//
// Transfers.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2004.
// This file is part of SHAREAZA (www.shareaza.com)
//
// Shareaza is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Shareaza is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Shareaza; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "Transfers.h"
#include "Transfer.h"
#include "TransferFile.h"
#include "Downloads.h"
#include "Uploads.h"
#include "EDClients.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CTransfers Transfers;


//////////////////////////////////////////////////////////////////////
// CTransfers construction

CTransfers::CTransfers()
{
	m_nBuffer		= 256*1024;
	m_pBuffer		= new BYTE[ m_nBuffer ];
	m_hThread		= NULL;
	m_bThread		= FALSE;
	m_nRunCookie	= 0;
}

CTransfers::~CTransfers()
{
	StopThread();
	delete [] m_pBuffer;
}

//////////////////////////////////////////////////////////////////////
// CTransfers list tests

int CTransfers::GetActiveCount() const
{
	return Downloads.GetCount( TRUE ) + Uploads.GetTransferCount();
}

BOOL CTransfers::IsConnectedTo(IN_ADDR* pAddress)
{
	CSingleLock pLock( &m_pSection );
	if ( ! pLock.Lock( 100 ) ) return FALSE;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		if ( GetNext( pos )->m_pHost.sin_addr.S_un.S_addr == pAddress->S_un.S_addr ) return TRUE;
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CTransfers thread start and stop

BOOL CTransfers::StartThread()
{
	if ( m_hThread != NULL && m_bThread ) return TRUE;
	if ( GetCount() == 0 && Downloads.GetCount() == 0 ) return FALSE;
	
	m_hThread	= NULL;
	m_bThread	= TRUE;
	
	CWinThread* pThread = AfxBeginThread( ThreadStart, this, THREAD_PRIORITY_NORMAL );
	m_hThread = pThread->m_hThread;
	
	return TRUE;
}

void CTransfers::StopThread()
{
	if ( m_hThread == NULL ) return;
	
	m_bThread = FALSE;
	m_pWakeup.SetEvent();
	
	for ( int nAttempt = 40 ; nAttempt > 0 ; nAttempt-- )
	{
		DWORD nCode;
		if ( ! GetExitCodeThread( m_hThread, &nCode ) ) break;
		if ( nCode != STILL_ACTIVE ) break;
		Sleep( 100 );
	}
	
	if ( nAttempt == 0 )
	{
		TerminateThread( m_hThread, 0 );
		theApp.Message( MSG_DEBUG, _T("WARNING: Terminating CTransfers thread.") );
		Sleep( 100 );
	}
	
	m_hThread = NULL;
	
	Downloads.m_nTransfers	= 0;
	Downloads.m_nBandwidth	= 0;
	Uploads.m_nCount		= 0;
	Uploads.m_nBandwidth	= 0;
}

//////////////////////////////////////////////////////////////////////
// CTransfers registration

void CTransfers::Add(CTransfer* pTransfer)
{
	ASSERT( pTransfer->m_hSocket != INVALID_SOCKET );
	WSAEventSelect( pTransfer->m_hSocket, m_pWakeup, FD_CONNECT|FD_READ|FD_WRITE|FD_CLOSE );
	
	POSITION pos = m_pList.Find( pTransfer );
	ASSERT( pos == NULL );
	if ( pos == NULL ) m_pList.AddHead( pTransfer );
	
	if ( Settings.General.Debug && Settings.General.DebugLog ) theApp.Message( MSG_DEBUG, _T("CTransfers::Add(): %x"), pTransfer );
	
	StartThread();
}

void CTransfers::Remove(CTransfer* pTransfer)
{
	if ( Settings.General.Debug && Settings.General.DebugLog ) theApp.Message( MSG_DEBUG, _T("CTransfers::Remove(): %x"), pTransfer );
	
	if ( pTransfer->m_hSocket != INVALID_SOCKET )
		WSAEventSelect( pTransfer->m_hSocket, m_pWakeup, 0 );
	
	if ( POSITION pos = m_pList.Find( pTransfer ) )
		m_pList.RemoveAt( pos );
}

//////////////////////////////////////////////////////////////////////
// CTransfers thread run

UINT CTransfers::ThreadStart(LPVOID pParam)
{
	CTransfers* pTransfers = (CTransfers*)pParam;
	pTransfers->OnRun();
	return 0;
}

void CTransfers::OnRun()
{
	CSingleLock pLock( &m_pSection );
	
	while ( m_bThread )
	{
		WaitForSingleObject( m_pWakeup, 50 );
		
		pLock.Lock();
		EDClients.OnRun();
		pLock.Unlock();
		if ( ! m_bThread ) break;
		
		OnRunTransfers();
		if ( ! m_bThread ) break;
		Downloads.OnRun();
		if ( ! m_bThread ) break;
		
		pLock.Lock();
		Uploads.OnRun();
		OnCheckExit();
		pLock.Unlock();
		
		TransferFiles.CommitDeferred();
	}
	
	Downloads.m_nTransfers = Downloads.m_nBandwidth = 0;
	Uploads.m_nCount = Uploads.m_nBandwidth = 0;
}

void CTransfers::OnRunTransfers()
{
	CSingleLock pLock( &m_pSection );
	m_nRunCookie ++;
	
	while ( TRUE )
	{
		BOOL bWorked = FALSE;
		pLock.Lock();
		
		for ( POSITION pos = GetIterator() ; pos ; )
		{
			CTransfer* pTransfer = GetNext( pos );
			
			if ( pTransfer->m_nRunCookie != m_nRunCookie )
			{
				pTransfer->m_nRunCookie = m_nRunCookie;
				pTransfer->DoRun();
				bWorked = TRUE;
				break;
			}
		}
		
		pLock.Unlock();
		if ( ! bWorked ) break;
	}
}

void CTransfers::OnCheckExit()
{
	if ( GetCount() == 0 && Downloads.GetCount() == 0 ) m_bThread = FALSE;
	
	if ( Settings.Live.AutoClose && GetActiveCount() == 0 )
	{
		CSingleLock pLock( &theApp.m_pSection );
		
		if ( pLock.Lock( 250 ) )
		{
			if ( CWnd* pWnd = (CWnd*)theApp.SafeMainWnd() )
			{
				Settings.Live.AutoClose = FALSE;
				pWnd->PostMessage( WM_CLOSE );
			}
		}
	}
}
