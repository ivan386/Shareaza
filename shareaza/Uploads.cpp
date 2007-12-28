//
// Uploads.cpp
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
#include "Uploads.h"
#include "UploadQueue.h"
#include "UploadQueues.h"
#include "UploadTransfer.h"
#include "UploadTransferHTTP.h"
#include "UploadTransferED2K.h"
#include "UploadTransferBT.h"

#include "Buffer.h"
#include "Transfers.h"
#include "Neighbours.h"
#include "Statistics.h"
#include "Remote.h"

#include "WndMain.h"
#include "WndMedia.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CUploads Uploads;


//////////////////////////////////////////////////////////////////////
// CUploads construction

CUploads::CUploads()
{
	m_nCount		= 0;
	m_nBandwidth	= 0;
	m_nTorrentSpeed	= 0;
	m_bStable		= FALSE;
	m_nBestSpeed	= 0;
}

CUploads::~CUploads()
{
	Clear( FALSE );
}

//////////////////////////////////////////////////////////////////////
// CUploads clear

void CUploads::Clear(BOOL bMessage)
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		GetNext( pos )->Remove( bMessage );
	}
	
	ASSERT( GetCount( NULL, -1 ) == 0 );
}

//////////////////////////////////////////////////////////////////////
// CUploads counting

DWORD CUploads::GetCount(CUploadTransfer* pExcept, int nState) const
{
	if ( pExcept == NULL && nState == -1 ) return (DWORD)m_pList.GetCount();
	
	DWORD nCount = 0;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CUploadTransfer* pUpload = GetNext( pos );
		
		if ( pUpload != pExcept )
		{
			switch ( nState )
			{
			case -1:
				nCount++;
				break;
			case -2:
				if ( pUpload->m_nState > upsNull ) nCount++;
				break;
			default:
				if ( pUpload->m_nState == nState ) nCount++;
				break;
			}
		}
	}
	
	return nCount;
}

DWORD CUploads::GetTorrentCount(int nState) const
{
	DWORD nCount = 0;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CUploadTransfer* pUpload = GetNext( pos );

		if ( pUpload->m_nProtocol == PROTOCOL_BT )
		{
			switch ( nState )
			{
			case -1:
				nCount++;
				break;
			case -2:
				if ( pUpload->m_nState > upsNull ) nCount++;
				break;
			case -3:
				if ( pUpload->m_nState >= upsUploading ) nCount++;
				break;
			default:
				if ( pUpload->m_nState == nState ) nCount++;
				break;
			}
		}
	}
	
	return nCount;
}

//////////////////////////////////////////////////////////////////////
// CUploads status

void CUploads::SetStable(DWORD nSpeed)
{
	m_bStable		= TRUE;
	m_nBestSpeed	= max( m_nBestSpeed, nSpeed );
}

DWORD CUploads::GetBandwidth() const
{
	DWORD nTotal = 0;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		nTotal += GetNext( pos )->GetMeasuredSpeed();
	}
	
	return nTotal;
}

//////////////////////////////////////////////////////////////////////
// CUploads per-host limiting

BOOL CUploads::AllowMoreTo(IN_ADDR* pAddress) const
{
	DWORD nCount = 0;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CUploadTransfer* pUpload = GetNext( pos );
		
		if ( pUpload->m_nState == upsUploading ||
			 pUpload->m_nState == upsQueued )
		{
			if ( pUpload->m_pHost.sin_addr.S_un.S_addr == pAddress->S_un.S_addr )
				nCount++;
		}
	}
	
	return ( nCount <= Settings.Uploads.MaxPerHost );
}

BOOL CUploads::CanUploadFileTo(IN_ADDR* pAddress, const Hashes::Sha1Hash& oSHA1) const
{
	DWORD nCount = 0;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CUploadTransfer* pUpload = GetNext( pos );
		
		if ( pUpload->m_nState == upsUploading ||
			 pUpload->m_nState == upsQueued )
		{
			if ( pUpload->m_pHost.sin_addr.S_un.S_addr == pAddress->S_un.S_addr )
			{
				nCount++;

				// If we're already uploading this file to this client
				if ( ( pUpload->m_oSHA1 ) && ( oSHA1 ) && validAndEqual( pUpload->m_oSHA1, oSHA1 ) )
					return FALSE;
			}
		}
	}
	
	return ( nCount < Settings.Uploads.MaxPerHost );
}

BOOL CUploads::EnforcePerHostLimit(CUploadTransfer* pHit, BOOL bRequest)
{
	DWORD nCount = 0;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CUploadTransfer* pUpload = GetNext( pos );

		if ( pUpload->m_nState == upsUploading ||
			 pUpload->m_nState == upsQueued ||
			 pUpload->m_nState == upsPreQueue )
		{
			if ( pUpload->m_pHost.sin_addr.S_un.S_addr == pHit->m_pHost.sin_addr.S_un.S_addr )
				nCount++;
		}
	}
	
	if ( nCount <= Settings.Uploads.MaxPerHost ) return FALSE;
	
	while ( nCount > Settings.Uploads.MaxPerHost )
	{
		CUploadTransfer* pNewest = NULL;
		
		for ( POSITION pos = GetIterator() ; pos ; )
		{
			CUploadTransfer* pUpload = GetNext( pos );
			
			if ( pUpload->m_nState == upsUploading ||
				 pUpload->m_nState == upsQueued ||
				 pUpload->m_nState == upsPreQueue )
			{
				if ( pUpload != pHit && pUpload->m_pHost.sin_addr.S_un.S_addr == pHit->m_pHost.sin_addr.S_un.S_addr )
				{
					if ( bRequest && pUpload->m_pBaseFile == pHit->m_pBaseFile )
					{
						pNewest = pUpload;
						break;
					}
					
					if ( pNewest == NULL || pUpload->m_tConnected > pNewest->m_tConnected )
						pNewest = pUpload;
				}
			}
		}
		
		if ( pNewest == NULL ) break;
		
		if ( bRequest )
		{
			if ( pNewest->m_pBaseFile == pHit->m_pBaseFile && ! pNewest->m_bLive )
			{
				UploadQueues.StealPosition( pHit, pNewest );
				
				theApp.Message( MSG_ERROR, IDS_UPLOAD_DROPPED_OLDER,
					(LPCTSTR)pNewest->m_sAddress );
				
				pNewest->Close( FALSE );
			}
			else
			{
				return FALSE;
			}
		}
		else
		{
			theApp.Message( MSG_ERROR, IDS_UPLOAD_DROPPED_NEWER, (LPCTSTR)pNewest->m_sAddress );
			pNewest->Close( FALSE );
		}
		
		nCount--;
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CUploads run

void CUploads::OnRun()
{
	CSingleLock pLock( &UploadQueues.m_pSection, TRUE );
	int nCountTorrent = 0;
	POSITION pos;
	
	m_nCount		= 0;
	m_nBandwidth	= 0;
	
	//Set measured queue speeds to 0
	for ( pos = UploadQueues.GetIterator() ; pos ; )
	{
		UploadQueues.GetNext( pos )->m_nMeasured = 0;
	}
	UploadQueues.m_pTorrentQueue->m_nMeasured = 0;
	UploadQueues.m_pTorrentQueue->m_nMinTransfers = 0;
	UploadQueues.m_pTorrentQueue->m_nMaxTransfers = 0;
	
	for ( pos = GetIterator() ; pos ; )
	{
		CUploadTransfer* pTransfer = GetNext( pos );

		if ( ( pTransfer->m_nProtocol == PROTOCOL_BT ) && ( pTransfer->m_nState != upsNull ) )	
		{
			// This is a torrent transfer
			CUploadTransferBT* pBT = (CUploadTransferBT*)pTransfer;
			if ( ( ! pBT->m_bInterested ) || ( pBT->m_bChoked ) )
			{
				// Choked- Increment appropriate torrent counter
				UploadQueues.m_pTorrentQueue->m_nMaxTransfers ++;
			}
			else
			{
				// Active torrent. (Uploading or requesting)

				// Only call this when we are going to use the returned value
				DWORD nMeasured = pTransfer->GetMeasuredSpeed();

				// Increment normal counters
				m_nCount ++;
				m_nBandwidth += nMeasured;

				// Increment torrent counters
				nCountTorrent ++;
				UploadQueues.m_pTorrentQueue->m_nMinTransfers ++;
				UploadQueues.m_pTorrentQueue->m_nMeasured += nMeasured;

				//theApp.Message( MSG_SYSTEM, pTransfer->m_sAddress );
				//theApp.Message( MSG_SYSTEM, _T("Port: %i "), pTransfer->m_pHost.sin_port );
			}
		}
		else if ( pTransfer->m_nState == upsUploading )
		{
			// Regular transfer that's uploading

			// Only call this when we are going to use the returned value
			DWORD nMeasured = pTransfer->GetMeasuredSpeed();

			m_nCount ++;
			m_nBandwidth += nMeasured;

			if ( pTransfer->m_pQueue != NULL && UploadQueues.Check( pTransfer->m_pQueue ) )
				pTransfer->m_pQueue->m_nMeasured += nMeasured;
		}
	}
	
	if ( nCountTorrent > 0 )	//If there are any active torrents
	{	//Assign bandwidth to BT (4/5ths by default)
		m_nTorrentSpeed = Settings.Bandwidth.Uploads;
		if ( m_nTorrentSpeed == 0 ) m_nTorrentSpeed = 0xFFFFFFFF;
		m_nTorrentSpeed = min( m_nTorrentSpeed, Settings.Connection.OutSpeed * 128 );
		m_nTorrentSpeed = m_nTorrentSpeed * Settings.BitTorrent.BandwidthPercentage / 100;
		
		m_nTorrentSpeed = m_nTorrentSpeed / nCountTorrent;
		m_nTorrentSpeed = max( m_nTorrentSpeed, Settings.Bandwidth.Request );
	}
	else
	{	//If there are no torrents, set to zero
		m_nTorrentSpeed = 0;
	}
}

//////////////////////////////////////////////////////////////////////
// CUploads get connection

BOOL CUploads::OnAccept(CConnection* pConnection)
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return FALSE;

	if ( Settings.Remote.Enable &&
		( pConnection->StartsWith( _P("GET /remote/") ) ||
		// The user entered the remote page into a browser, but forgot the trailing '/'
		  pConnection->StartsWith( _P("GET /remote HTTP") ) ) )
	{
		new CRemote( pConnection );
		return TRUE;
	}
	else if ( Settings.Uploads.MaxPerHost > 0 )
	{
		CUploadTransfer* pUpload = new CUploadTransferHTTP();
		
		for ( POSITION pos = GetIterator() ; pos ; )
		{
			CUploadTransfer* pTest = GetNext( pos );
			
			if (	pTest->m_pHost.sin_addr.S_un.S_addr ==
					pConnection->m_pHost.sin_addr.S_un.S_addr )
			{
				pTest->m_bLive = FALSE;
			}
		}
		
		pUpload->AttachTo( pConnection );
		
		return TRUE;
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CUploads rename handler

void CUploads::OnRename(LPCTSTR pszSource, LPCTSTR pszTarget, BOOL bRemoving)
{
	if ( pszSource == NULL ) return;
	CSingleLock pLock( &Transfers.m_pSection );
	
	if ( pLock.Lock( 500 ) )
	{
		for ( POSITION pos = GetIterator() ; pos ; )
		{
			GetNext( pos )->OnRename( pszSource, pszTarget );
		}
		
		pLock.Unlock();
	}
	
	if ( ! bRemoving ) return;

	CSingleLock pLock2( &theApp.m_pSection );
	
	if ( pLock2.Lock( 500 ) )
	{
		if ( CMainWnd* pMainWnd = (CMainWnd*)theApp.m_pSafeWnd )
		{
			CMediaWnd* pMediaWnd = (CMediaWnd*)pMainWnd->m_pWindows.Find(
				RUNTIME_CLASS(CMediaWnd) );
			
			if ( pMediaWnd != NULL )
			{
				pMediaWnd->OnFileDelete( pszSource );
			}
		}
		
		pLock2.Unlock();
	}
}

//////////////////////////////////////////////////////////////////////
// CUploads add and remove

void CUploads::Add(CUploadTransfer* pUpload)
{
	POSITION pos = m_pList.Find( pUpload );
	ASSERT( pos == NULL );
	m_pList.AddHead( pUpload );
	UNUSED_ALWAYS( pos );
}

void CUploads::Remove(CUploadTransfer* pUpload)
{
	POSITION pos = m_pList.Find( pUpload );
	ASSERT( pos != NULL );
	m_pList.RemoveAt( pos );
	UNUSED_ALWAYS( pos );
}

