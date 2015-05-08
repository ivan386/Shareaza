//
// Transfers.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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
#include "Datagrams.h"
#include "Handshakes.h"
#include "Network.h"
#include "Transfers.h"
#include "Transfer.h"
#include "TransferFile.h"
#include "Downloads.h"
#include "Uploads.h"
#include "EDClients.h"
#include "DCClients.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CTransfers Transfers;


//////////////////////////////////////////////////////////////////////
// CTransfers construction

CTransfers::CTransfers() :
	m_nRunCookie( 0 )
{
}

CTransfers::~CTransfers()
{
}

//////////////////////////////////////////////////////////////////////
// CTransfers list tests

INT_PTR CTransfers::GetActiveCount() const
{
	return Downloads.GetCount( TRUE ) + Uploads.GetTransferCount();
}

BOOL CTransfers::IsConnectedTo(const IN_ADDR* pAddress) const
{
	CSingleLock pLock( &m_pSection );
	if ( ! pLock.Lock( 250 ) )
		return FALSE;

	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		if ( m_pList.GetNext( pos )->m_pHost.sin_addr.S_un.S_addr == pAddress->S_un.S_addr )
			return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CTransfers thread start and stop

BOOL CTransfers::StartThread()
{
	if ( theApp.m_bClosing )
		return FALSE;

	return BeginThread( "Transfers" );
}

void CTransfers::StopThread()
{
	CloseThread();

	Downloads.m_nComplete	= 0;
	Downloads.m_nTotal		= 0;
	Downloads.m_nTransfers	= 0;
	Downloads.m_nBandwidth	= 0;
	Uploads.m_nCount		= 0;
	Uploads.m_nBandwidth	= 0;
}

//////////////////////////////////////////////////////////////////////
// CTransfers registration

void CTransfers::Add(CTransfer* pTransfer)
{
	CQuickLock oLock( m_pSection );

	ASSERT( pTransfer->IsValid() );
	WSAEventSelect( pTransfer->m_hSocket, GetWakeupEvent(), FD_CONNECT|FD_READ|FD_WRITE|FD_CLOSE );

	POSITION pos = m_pList.Find( pTransfer );
	ASSERT( pos == NULL );
	if ( pos == NULL )
		m_pList.AddHead( pTransfer );

	StartThread();
}

void CTransfers::Remove(CTransfer* pTransfer)
{
	CQuickLock oLock( m_pSection );

	if ( pTransfer->IsValid() )
		WSAEventSelect( pTransfer->m_hSocket, GetWakeupEvent(), 0 );

	if ( POSITION pos = m_pList.Find( pTransfer ) )
		m_pList.RemoveAt( pos );
}

//////////////////////////////////////////////////////////////////////
// CTransfers thread run

void CTransfers::OnRun()
{
	while ( IsThreadEnabled() )
	{
		Doze( Settings.General.MinTransfersRest );

		if ( ! theApp.m_bLive || ! Handshakes.IsValid() || ! Datagrams.IsValid() )
		{
			Sleep( 0 );
			continue;
		}

		EDClients.OnRun();

		if ( ! IsThreadEnabled() )
			break;

		DCClients.OnRun();

		if ( ! IsThreadEnabled() )
			break;

		OnRunTransfers();

		if ( ! IsThreadEnabled() )
			break;

		Downloads.OnRun();

		if ( ! IsThreadEnabled() )
			break;

		Uploads.OnRun();
		
		OnCheckExit();

		TransferFiles.CommitDeferred();
	}

	Downloads.m_nComplete	= 0;
	Downloads.m_nTotal		= 0;
	Downloads.m_nTransfers	= 0;
	Downloads.m_nBandwidth	= 0;
	Uploads.m_nCount		= 0;
	Uploads.m_nBandwidth	= 0;
}

void CTransfers::OnRunTransfers()
{
	// Quick check to avoid locking
	if ( m_pList.IsEmpty() )
		return;

	// Spend here no more than 250 ms at once
	DWORD nBegin = GetTickCount();
	CSingleLock oLock( &m_pSection, FALSE );
	if ( ! oLock.Lock( 250 ) )
		return;

	++m_nRunCookie;

	while ( ! m_pList.IsEmpty() && GetTickCount() - nBegin < 250 && m_pList.GetHead()->m_nRunCookie != m_nRunCookie )
	{
		CTransfer* pTransfer = m_pList.RemoveHead();
		m_pList.AddTail( pTransfer );
		pTransfer->m_nRunCookie = m_nRunCookie;
		pTransfer->DoRun();
	}
}

void CTransfers::OnCheckExit()
{
	// Quick check to avoid locking
	if ( m_pList.IsEmpty() )
	{
		CSingleLock oLock( &m_pSection, FALSE );
		if ( oLock.Lock( 250 ) )
		{
			if ( m_pList.GetCount() == 0 &&
				Downloads.GetCount() == 0 &&
				EDClients.GetCount() == 0 &&
				DCClients.GetCount() == 0 )
			{
				Exit();
			}
		}
	}

	if ( Settings.Live.AutoClose && GetActiveCount() == 0 )
	{
		Settings.Live.AutoClose = false;
		PostMainWndMessage( WM_CLOSE );
	}
}
