//
// DownloadWithTransfers.cpp
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
#include "Download.h"
#include "Downloads.h"
#include "Transfers.h"
#include "DownloadWithTransfers.h"
#include "DownloadSource.h"
#include "DownloadTransferHTTP.h"
//#include "DownloadTransferFTP.h"
#include "DownloadTransferED2K.h"
#include "DownloadTransferBT.h"
#include "Network.h"
#include "EDClient.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CDownloadWithTransfers construction

CDownloadWithTransfers::CDownloadWithTransfers()
{
	m_pTransferFirst	= NULL;
	m_pTransferLast		= NULL;
	m_nTransferCount	= 0;
	m_tTransferStart	= 0;
}

CDownloadWithTransfers::~CDownloadWithTransfers()
{
	CloseTransfers();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTransfers counting

int CDownloadWithTransfers::GetTransferCount(int nState, IN_ADDR* pAddress) const
{
	// if ( nState == -1 && pAddress == NULL ) return m_pTransfers.GetCount();
	int nCount = 0;
	
	for ( CDownloadTransfer* pTransfer = m_pTransferFirst ; pTransfer ; pTransfer = pTransfer->m_pDlNext )
	{	
		if ( pAddress == NULL || pAddress->S_un.S_addr == pTransfer->m_pHost.sin_addr.S_un.S_addr )
		{
			if ( pTransfer->m_nProtocol == PROTOCOL_ED2K && nState != dtsCountNotConnecting )
			{
				CDownloadTransferED2K* pED2K = (CDownloadTransferED2K*)pTransfer;
				if ( pED2K->m_pClient == NULL || pED2K->m_pClient->m_bConnected == FALSE ) continue;
			}
			
			if ( nState == dtsCountAll )
			{
				nCount++;
			}
			else if ( nState == dtsCountNotQueued )
			{
				if ( pTransfer->m_nState == dtsTorrent )
				{
					CDownloadTransferBT* pBT = (CDownloadTransferBT*)pTransfer;
					if ( ! pBT->m_bChoked ) nCount++;
				}
				else if ( pTransfer->m_nState != dtsQueued )
				{
					nCount++;
				}
			}
			else if ( nState == dtsCountNotConnecting )
			{
				if ( pTransfer->m_nState > dtsConnecting ) nCount++;
			}
			else
			{
				if ( pTransfer->m_nState == nState ) nCount++;
			}
		}
	}
	
	return nCount;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTransfers consider starting more transfers

BOOL CDownloadWithTransfers::StartTransfersIfNeeded(DWORD tNow)
{
	if ( tNow == 0 ) tNow = GetTickCount();
	
	if ( tNow - m_tTransferStart < 100 ) return FALSE;
	m_tTransferStart = tNow;
	
	if ( Settings.Downloads.ConnectThrottle != 0 )
	{
		if ( tNow <= Downloads.m_tLastConnect ) return FALSE;
		if ( tNow - Downloads.m_tLastConnect <= Settings.Downloads.ConnectThrottle ) return FALSE;
	}

	if ( Downloads.GetConnectingTransferCount() >= Settings.Downloads.MaxConnectingSources )
	{
		return FALSE;
	}
	
	int nTransfers = GetTransferCount( dtsDownloading );

	if ( m_bBTH )
	{
		if ( nTransfers > Settings.BitTorrent.DownloadConnections ) return FALSE;
		//if ( ( m_pTransferFirst == NULL ) &&  ( Downloads.GetTryingCount(TRUE) >= Settings.BitTorrent.DownloadTorrents ) ) return FALSE;
	}
	
	if ( nTransfers < Settings.Downloads.MaxFileTransfers &&
		 ( ! Settings.Downloads.StaggardStart ||
		 nTransfers == GetTransferCount( dtsCountAll ) ) )
	{
		if ( Downloads.m_bAllowMoreDownloads || m_pTransferFirst != NULL )
		{
			if ( Downloads.m_bAllowMoreTransfers )
			{
				if ( StartNewTransfer( tNow ) )
				{
					Downloads.UpdateAllows( TRUE );
					return TRUE;
				}
			}
		}
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadSource check (INLINE)

BOOL CDownloadSource::CanInitiate(BOOL bNetwork, BOOL bEstablished) const
{
	if ( Settings.Connection.RequireForTransfers )
	{
		if ( m_nProtocol == PROTOCOL_ED2K )
		{
			if ( ! Settings.eDonkey.EnableToday ) return FALSE;
			if ( ! bNetwork ) return FALSE;
		}
		else if ( m_nProtocol == PROTOCOL_BT )
		{
			if ( ! bNetwork ) return FALSE;
		}
		else if ( m_nProtocol == PROTOCOL_HTTP )
		{
			if ( m_nGnutella == 2 )
			{
				if ( ! Settings.Gnutella2.EnableToday ) return FALSE;
			}
			else if ( m_nGnutella == 1 )
			{
				if ( ! Settings.Gnutella1.EnableToday ) return FALSE;
			}
			else
			{
				if ( ! Settings.Gnutella1.EnableToday &&
					 ! Settings.Gnutella2.EnableToday ) return FALSE;
			}
		}
	}
	
	return bEstablished || Downloads.AllowMoreTransfers( (IN_ADDR*)&m_pAddress );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTransfers start a new transfer

BOOL CDownloadWithTransfers::StartNewTransfer(DWORD tNow)
{
	if ( tNow == 0 ) tNow = GetTickCount();
	
	BOOL bConnected = Network.IsConnected();
	CDownloadSource* pConnectHead = NULL;
	CDownloadSource* pPushHead = NULL;
	
	for ( CDownloadSource* pSource = m_pSourceFirst ; pSource ; )
	{
		CDownloadSource* pNext = pSource->m_pNext;
		
		if ( pSource->m_pTransfer != NULL )
		{
			// Already has a transfer
		}
		else if ( pSource->m_bPushOnly == FALSE )
		{
			if ( pSource->m_tAttempt == 0 )
			{
				if ( pSource->CanInitiate( bConnected, FALSE ) )
				{
					CDownloadTransfer* pTransfer = pSource->CreateTransfer();
					return pTransfer != NULL && pTransfer->Initiate();
				}
			}
			else if ( pSource->m_tAttempt > 0 && pSource->m_tAttempt <= tNow )
			{
				if ( pConnectHead == NULL || ( pConnectHead->m_nProtocol != PROTOCOL_HTTP && pSource->m_nProtocol == PROTOCOL_HTTP ) )
				{
					if ( pSource->CanInitiate( bConnected, FALSE ) ) pConnectHead = pSource;
				}
			}
		}
		else
		{
			if ( pSource->m_tAttempt == 0 )
			{
				if ( pPushHead == NULL && pSource->CanInitiate( bConnected, FALSE ) ) pPushHead = pSource;
			}
			else if ( pSource->m_tAttempt <= tNow )
			{
				if ( ! Settings.Downloads.NeverDrop ) pSource->Remove( TRUE, TRUE );
			}
		}
		
		pSource = pNext;
	}
	
	if ( pConnectHead != NULL )
	{
		CDownloadTransfer* pTransfer = pConnectHead->CreateTransfer();
		return pTransfer != NULL && pTransfer->Initiate();
	}
	
	if ( pPushHead != NULL )
	{
		if ( pPushHead->PushRequest() ) return FALSE;
		if ( ! Settings.Downloads.NeverDrop ) pPushHead->Remove( TRUE, TRUE );
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTransfers close

void CDownloadWithTransfers::CloseTransfers()
{
	BOOL bBackup = Downloads.m_bClosing;
	Downloads.m_bClosing = TRUE;
	
	for ( CDownloadTransfer* pTransfer = m_pTransferFirst ; pTransfer ; )
	{
		CDownloadTransfer* pNext = pTransfer->m_pDlNext;
		pTransfer->Close( TS_TRUE );
		pTransfer = pNext;
	}
	
	ASSERT( m_nTransferCount == 0 );
	
	Downloads.m_bClosing = bBackup;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTransfers average speed

DWORD CDownloadWithTransfers::GetAverageSpeed() const
{
	DWORD nSpeed = 0;
	
	for ( CDownloadTransfer* pTransfer = m_pTransferFirst ; pTransfer ; pTransfer = pTransfer->m_pDlNext )
	{
		if ( pTransfer->m_nState == dtsDownloading ) nSpeed += pTransfer->GetAverageSpeed();
	}
	
	return nSpeed;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTransfers measured speed

DWORD CDownloadWithTransfers::GetMeasuredSpeed() const
{
	DWORD nSpeed = 0;
	
	for ( CDownloadTransfer* pTransfer = m_pTransferFirst ; pTransfer ; pTransfer = pTransfer->m_pDlNext )
	{
		if ( pTransfer->m_nState == dtsDownloading )
			nSpeed += pTransfer->GetMeasuredSpeed();
	}
	
	return nSpeed;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTransfers push handler

BOOL CDownloadWithTransfers::OnAcceptPush(GGUID* pClientID, CConnection* pConnection)
{
	CDownload* pDownload = (CDownload*)this;
	if ( pDownload->IsMoving() || pDownload->IsPaused() ) return FALSE;
	
	CDownloadSource* pSource = NULL;
	
	for ( pSource = GetFirstSource() ; pSource ; pSource = pSource->m_pNext )
	{
		if ( pSource->m_nProtocol == PROTOCOL_HTTP && pSource->CheckPush( pClientID ) ) break;
	}
	
	if ( pSource == NULL ) return FALSE;
	
	if ( pSource->m_pTransfer != NULL )
	{
		if ( pSource->m_pTransfer->m_nState > dtsConnecting ) return FALSE;
		pSource->m_pTransfer->Close( TS_TRUE );
	}
	
	if ( pConnection->m_hSocket == INVALID_SOCKET ) return FALSE;
	
	CDownloadTransferHTTP* pTransfer = (CDownloadTransferHTTP*)pSource->CreateTransfer();
	ASSERT( pTransfer->m_nProtocol == PROTOCOL_HTTP );
	return pTransfer->AcceptPush( pConnection );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTransfers eDonkey2000 callback handler

BOOL CDownloadWithTransfers::OnDonkeyCallback(CEDClient* pClient, CDownloadSource* pExcept)
{
	CDownload* pDownload = (CDownload*)this;
	if ( pDownload->IsMoving() || pDownload->IsPaused() ) return FALSE;
	
	CDownloadSource* pSource = NULL;
	DWORD tNow = GetTickCount();
	
	for ( pSource = GetFirstSource() ; pSource ; pSource = pSource->m_pNext )
	{
		if ( pExcept != pSource && pSource->CheckDonkey( pClient ) ) break;
	}
	
	if ( pSource == NULL ) return FALSE;
	
	if ( pSource->m_pTransfer != NULL )
	{
		if ( pSource->m_pTransfer->m_nState > dtsConnecting ) return FALSE;
		pSource->m_pTransfer->Close( TS_TRUE );
	}
	
	CDownloadTransferED2K* pTransfer = (CDownloadTransferED2K*)pSource->CreateTransfer();
	ASSERT( pTransfer->m_nProtocol == PROTOCOL_ED2K );
	return pTransfer->Initiate();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTransfers add and remove transfers

void CDownloadWithTransfers::AddTransfer(CDownloadTransfer* pTransfer)
{
	m_nTransferCount ++;
	pTransfer->m_pDlPrev = m_pTransferLast;
	pTransfer->m_pDlNext = NULL;
	
	if ( m_pTransferLast != NULL )
	{
		m_pTransferLast->m_pDlNext = pTransfer;
		m_pTransferLast = pTransfer;
	}
	else
	{
		m_pTransferFirst = m_pTransferLast = pTransfer;
	}
}

void CDownloadWithTransfers::RemoveTransfer(CDownloadTransfer* pTransfer)
{
	ASSERT( m_nTransferCount > 0 );
	m_nTransferCount --;
	
	if ( pTransfer->m_pDlPrev != NULL )
		pTransfer->m_pDlPrev->m_pDlNext = pTransfer->m_pDlNext;
	else
		m_pTransferFirst = pTransfer->m_pDlNext;
	
	if ( pTransfer->m_pDlNext != NULL )
		pTransfer->m_pDlNext->m_pDlPrev = pTransfer->m_pDlPrev;
	else
		m_pTransferLast = pTransfer->m_pDlPrev;
	
	delete pTransfer;
}
