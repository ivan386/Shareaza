//
// DownloadWithTransfers.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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
#include "Neighbours.h"
#include "EDClient.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CDownloadWithTransfers construction

CDownloadWithTransfers::CDownloadWithTransfers() :
	m_pTransferFirst	( NULL )
,	m_pTransferLast		( NULL )
,	m_nTransferCount	( 0 )
,	m_tTransferStart	( 0 )
{
}

CDownloadWithTransfers::~CDownloadWithTransfers()
{
	ASSUME_LOCK( Transfers.m_pSection );

	CloseTransfers();
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTransfers counting

bool CDownloadWithTransfers::HasActiveTransfers() const
{
	for ( CDownloadTransfer* pTransfer = m_pTransferFirst; pTransfer; pTransfer = pTransfer->m_pDlNext )
	{
		if ( pTransfer->m_nState == dtsDownloading )
		{
			return true;
		}
	}
	return false;
}

DWORD CDownloadWithTransfers::GetTransferCount() const
{
	DWORD nCount = 0;

	for ( CDownloadTransfer* pTransfer = m_pTransferFirst; pTransfer; pTransfer = pTransfer->m_pDlNext )
	{
		if ( ( pTransfer->m_nState > dtsNull ) &&
			 ( pTransfer->m_nProtocol != PROTOCOL_ED2K || pTransfer->m_nState != dtsQueued ) )
		{
			++nCount;
		}
	}
	return nCount;
}

bool CDownloadWithTransfers::ValidTransfer(const IN_ADDR* pAddress, const CDownloadTransfer* pTransfer) const
{
	return ( ! pAddress || pAddress->S_un.S_addr == pTransfer->m_pHost.sin_addr.S_un.S_addr ) &&
		( pTransfer->m_nState > dtsNull ) &&
		( pTransfer->m_nProtocol != PROTOCOL_ED2K || pTransfer->m_nState != dtsQueued );
}

DWORD CDownloadWithTransfers::GetTransferCount(int nState, const IN_ADDR* pAddress) const
{
	int nCount = 0;

	switch ( nState )
	{
	case dtsCountAll:
		for ( CDownloadTransfer* pTransfer = m_pTransferFirst; pTransfer; pTransfer = pTransfer->m_pDlNext )
		{
			if ( ValidTransfer( pAddress, pTransfer ) )
			{
				++nCount;
			}
		}
		return nCount;
	case dtsCountNotQueued:
		for ( CDownloadTransfer* pTransfer = m_pTransferFirst ; pTransfer ; pTransfer = pTransfer->m_pDlNext )
		{
			if ( ValidTransfer( pAddress, pTransfer ) && ( ( pTransfer->m_nState != dtsQueued ) &&
				( ! ( pTransfer->m_nState == dtsTorrent && static_cast< CDownloadTransferBT* >(pTransfer)->m_bChoked ) ) ) )

			{
				++nCount;
			}
		}
		return nCount;
	case dtsCountNotConnecting:
		for ( CDownloadTransfer* pTransfer = m_pTransferFirst ; pTransfer ; pTransfer = pTransfer->m_pDlNext )
		{
			if ( ( ! pAddress || pAddress->S_un.S_addr == pTransfer->m_pHost.sin_addr.S_un.S_addr ) &&
				 ( pTransfer->m_nState > dtsConnecting ) )
			{
				++nCount;
			}
		}
		return nCount;
	case dtsCountTorrentAndActive:
		for ( CDownloadTransfer* pTransfer = m_pTransferFirst ; pTransfer ; pTransfer = pTransfer->m_pDlNext )
		{
			if ( ValidTransfer( pAddress, pTransfer ) )
			{
				switch( pTransfer->m_nState )
				{
				case dtsTorrent:
				case dtsRequesting:
				case dtsDownloading:
					++nCount;
				}
			}
		}
		return nCount;
	default:
		for ( CDownloadTransfer* pTransfer = m_pTransferFirst ; pTransfer ; pTransfer = pTransfer->m_pDlNext )
		{
			if ( pTransfer->m_nState == nState )
			{
				++nCount;
			}
		}
		return nCount;
	}
}

//////////////////////////////////////////////////////////////////////
// GetAmountDownloadedFrom total volume from an IP

QWORD CDownloadWithTransfers::GetAmountDownloadedFrom(const IN_ADDR* pAddress) const
{
	QWORD nTotal = 0;

	for ( CDownloadTransfer* pTransfer = m_pTransferFirst ; pTransfer ; pTransfer = pTransfer->m_pDlNext )
	{
		if ( pAddress->S_un.S_addr == pTransfer->m_pHost.sin_addr.S_un.S_addr )
			nTotal += pTransfer->m_nDownloaded;
	}

	return nTotal;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTransfers consider starting more transfers

// This function checks if it's okay to try opening a new download. (Download throttle, etc)
BOOL CDownloadWithTransfers::CanStartTransfers(DWORD tNow)
{
	if ( tNow == 0 ) tNow = GetTickCount();

	if ( tNow - m_tTransferStart < 100 )
		return FALSE;
	m_tTransferStart = tNow;

	// Make sure the network is ready
	if ( ! Network.ReadyToTransfer( tNow ) )
		return FALSE;

	// Limit the connection rate
	if ( Settings.Downloads.ConnectThrottle && tNow <= Neighbours.LastConnect() + Settings.Downloads.ConnectThrottle )
		return FALSE;

	// Limit the amount of connecting (half-open) sources. (Very important for XP sp2)
	if ( Downloads.GetConnectingTransferCount() >= Settings.Downloads.MaxConnectingSources )
		return FALSE;

	return TRUE;
}

// This functions starts a new download transfer if needed and allowed.
BOOL CDownloadWithTransfers::StartTransfersIfNeeded(DWORD tNow)
{
	if ( tNow == 0 ) tNow = GetTickCount();

	// Check connection throttles, max open connections, etc
	if ( ! CanStartTransfers( tNow ) ) return FALSE;

	//BitTorrent limiting
	if ( static_cast< CDownloadWithTorrent* >( this )->IsTorrent() )
	{
		// Max connections
		if ( ( GetTransferCount( dtsCountTorrentAndActive ) ) > Settings.BitTorrent.DownloadConnections ) return FALSE;
	}

	DWORD nTransfers = GetTransferCount( dtsDownloading );

	if ( nTransfers < Settings.Downloads.MaxFileTransfers &&
		 ( ! Settings.Downloads.StaggardStart ||
		 nTransfers == GetTransferCount( dtsCountAll ) ) )
	{
		// If we can start new downloads, or this download is already running
		if ( Downloads.AllowMoreDownloads() || m_pTransferFirst != NULL )
		{
			// If we can start new transfers
			if ( Downloads.AllowMoreTransfers() )
			{
				// If download bandwidth isn't at max
				if ( ( ( tNow - Downloads.m_tBandwidthAtMax ) > 5000 ) )
				{
					// Start a new download
					if ( StartNewTransfer( tNow ) )
					{
						Downloads.UpdateAllows();
						return TRUE;
					}
				}
			}
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTransfers start a new transfer

BOOL CDownloadWithTransfers::StartNewTransfer(DWORD tNow)
{
	if ( tNow == 0 ) tNow = GetTickCount();

	BOOL bConnected = Network.IsConnected();
	CDownloadSource* pConnectHead = NULL;

	// If BT preferencing is on, check them first
	if ( static_cast< CDownloadWithTorrent* >( this )->IsTorrent() &&
		( Settings.BitTorrent.PreferenceBTSources ) )
	{
		for ( POSITION posSource = GetIterator(); posSource ; )
		{
			CDownloadSource* pSource = GetNext( posSource );

			if (   pSource->IsIdle() &&						// does not have a transfer
				 ( pSource->m_bPushOnly == FALSE ) &&		// Not push
				 ( pSource->m_nProtocol == PROTOCOL_BT ) &&	// Is a BT source
				 ( pSource->m_tAttempt == 0 ) )				// Is a "fresh" source from the tracker
			{
				if ( pSource->CanInitiate( bConnected, FALSE ) )
				{
					CDownloadTransfer* pTransfer = pSource->CreateTransfer();
					return pTransfer != NULL && pTransfer->Initiate();
				}
			}
		}
	}

	for ( POSITION posSource = GetIterator(); posSource ; )
	{
		CDownloadSource* pSource = GetNext( posSource );

		if ( ! pSource->IsIdle() )
		{
			// Already has a transfer
		}
		else if ( ( pSource->m_nProtocol == PROTOCOL_ED2K ) && ( ( tNow - Downloads.m_tBandwidthAtMaxED2K ) < 5000 ) )
		{
			// ED2K use (Ratio) is maxed out, no point in starting new transfers
		}
		else if ( pSource->m_bPushOnly == FALSE ||
			pSource->m_nProtocol == PROTOCOL_ED2K ||
			pSource->m_nProtocol == PROTOCOL_DC )
		{
			if ( pSource->m_tAttempt == 0 )
			{
				if ( pSource->CanInitiate( bConnected, FALSE ) )
				{
					pConnectHead = pSource;
					break;
				}
			}
			else if ( pSource->m_tAttempt > 0 && pSource->m_tAttempt <= tNow )
			{
				if ( pConnectHead == NULL && pSource->CanInitiate( bConnected, FALSE ) )
				{
					pConnectHead = pSource;
				}
			}
		}
		else if ( Network.GetStableTime() >= 15 )
		{
			if ( pSource->m_tAttempt == 0 )
			{
				if ( pSource->CanInitiate( bConnected, FALSE ) )
				{
					pConnectHead = pSource;
					break;
				}
			}
			else if ( pSource->m_tAttempt <= tNow )
			{
				pSource->Remove( TRUE, FALSE );
			}
		}
	}

	if ( pConnectHead != NULL )
	{
		if ( pConnectHead->m_bPushOnly &&
			pConnectHead->m_nProtocol != PROTOCOL_ED2K )
		{
			if ( pConnectHead->PushRequest() )
			{
				return TRUE;
			}
			else if ( ! Settings.Downloads.NeverDrop )
			{
				pConnectHead->Remove( TRUE, FALSE );
			}
			else
			{
				SortSource( pConnectHead, FALSE );
			}
		}
		else
		{
			CDownloadTransfer* pTransfer = pConnectHead->CreateTransfer();
			if ( pTransfer && pTransfer->Initiate() )
			{
				if ( CDownloadSource* pSource = pTransfer->GetSource() )
				{
					pSource->m_sCountry = pTransfer->m_sCountry;
					pSource->m_sCountryName = pTransfer->m_sCountryName;
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTransfers close

void CDownloadWithTransfers::CloseTransfers()
{
	ASSUME_LOCK( Transfers.m_pSection );

	bool bBackup = Downloads.m_bClosing;
	Downloads.m_bClosing = true;

	for ( CDownloadTransfer* pTransfer = m_pTransferFirst ; pTransfer ; )
	{
		CDownloadTransfer* pNext = pTransfer->m_pDlNext;
		pTransfer->Close( TRI_TRUE );
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

BOOL CDownloadWithTransfers::OnAcceptPush(const Hashes::Guid& oClientID, CConnection* pConnection)
{
	CDownload* pDownload = static_cast< CDownload* >( this );

	if ( pDownload->IsMoving() || pDownload->IsPaused() )
		return FALSE;

	CDownloadSource* pSource = NULL;

	for ( POSITION posSource = GetIterator(); posSource ; )
	{
		pSource = GetNext( posSource );

		if ( pSource->CheckPush( oClientID ) )
			break;

		pSource = NULL;
	}

	if ( pSource == NULL )
		return FALSE;

	if ( ! pSource->IsIdle() )
	{
		if ( pSource->IsConnected() )
			return FALSE;

		pSource->Close();
	}

	if ( ! pConnection->IsValid() )
		return FALSE;

	if ( CDownloadTransfer* pTransfer = pSource->CreateTransfer() )
	{
		pTransfer->AttachTo( pConnection );
		return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTransfers eDonkey2000 callback handler

BOOL CDownloadWithTransfers::OnDonkeyCallback(const CEDClient* pClient, CDownloadSource* pExcept)
{
	CDownload* pDownload = static_cast< CDownload* >( this );
	if ( pDownload->IsMoving() || pDownload->IsPaused() ) return FALSE;

	CDownloadSource* pSource = NULL;

	for ( POSITION posSource = GetIterator(); posSource ; )
	{
		pSource = GetNext( posSource );

		if ( pExcept != pSource && pSource->CheckDonkey( pClient ) )
			break;

		pSource = NULL;
	}

	if ( pSource == NULL ) return FALSE;

	if ( ! pSource->IsIdle() )
	{
		if ( pSource->IsConnected() )
			return FALSE;

		pSource->Close();
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
	ASSUME_LOCK( Transfers.m_pSection );

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
