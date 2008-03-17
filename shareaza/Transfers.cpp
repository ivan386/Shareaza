//
// Transfers.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
// This file is part of SHAREAZA (shareaza.sourceforge.net)
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

INT_PTR CTransfers::GetActiveCount() const
{
	return Downloads.GetCount( TRUE ) + Uploads.GetTransferCount();
}

BOOL CTransfers::IsConnectedTo(IN_ADDR* pAddress) const
{
	CSingleLock pLock( &m_pSection );
	if ( ! pLock.Lock( 100 ) )
		return FALSE;

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

	m_hThread = BeginThread( "Transfers", ThreadStart, this );

	return TRUE;
}

void CTransfers::StopThread()
{
	if ( m_hThread == NULL ) return;

	m_bThread = FALSE;
	m_pWakeup.SetEvent();

	CloseThread( (HANDLE*)&m_hThread );

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

	//if ( Settings.General.Debug && Settings.General.DebugLog ) 
	//	theApp.Message( MSG_DEBUG, _T("CTransfers::Add(): %x"), pTransfer );

	StartThread();
}

void CTransfers::Remove(CTransfer* pTransfer)
{
	//if ( Settings.General.Debug && Settings.General.DebugLog ) 
	//	theApp.Message( MSG_DEBUG, _T("CTransfers::Remove(): %x"), pTransfer );

	if ( pTransfer->m_hSocket != INVALID_SOCKET )
		WSAEventSelect( pTransfer->m_hSocket, m_pWakeup, 0 );

	CQuickLock oLock( m_pSection );
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
	while ( m_bThread )
	{
		Sleep( Settings.General.MinTransfersRest );
		WaitForSingleObject( m_pWakeup, 50 );

		{
			CQuickLock oLock( m_pSection );
			EDClients.OnRun();
		}
		if ( ! m_bThread ) break;

		OnRunTransfers();
		if ( ! m_bThread ) break;
		Downloads.OnRun();
		if ( ! m_bThread ) break;

		{
			CQuickLock oLock( m_pSection );
			Uploads.OnRun();
			OnCheckExit();
		}

		TransferFiles.CommitDeferred();
	}

	Downloads.m_nTransfers = Downloads.m_nBandwidth = 0;
	Uploads.m_nCount = Uploads.m_nBandwidth = 0;
}

void CTransfers::OnRunTransfers()
{
	CQuickLock oLock( m_pSection );

	++m_nRunCookie;

	while ( !m_pList.IsEmpty()
		&& m_pList.GetHead()->m_nRunCookie != m_nRunCookie )
	{
		CTransfer* pTransfer = m_pList.RemoveHead();
		m_pList.AddTail( pTransfer );
		pTransfer->m_nRunCookie = m_nRunCookie;
		pTransfer->DoRun();
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
