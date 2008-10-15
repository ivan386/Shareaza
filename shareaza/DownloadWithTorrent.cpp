//
// DownloadWithTorrent.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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
#include "Network.h"
#include "BTPacket.h"
#include "BTClient.h"
#include "BTClients.h"
#include "Download.h"
#include "DownloadTask.h"
#include "DownloadSource.h"
#include "DownloadWithTorrent.h"
#include "DownloadTransferBT.h"
#include "UploadTransferBT.h"
#include "BTTrackerRequest.h"
#include "Transfers.h"
#include "FragmentedFile.h"
#include "Buffer.h"
#include "LibraryFolders.h"
#include "GProfile.h"
#include "Uploads.h"
#include "UploadTransfer.h"
#include "Library.h"
#include "LibraryMaps.h"
#include "SharedFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent construction

CDownloadWithTorrent::CDownloadWithTorrent() :
	m_bTorrentRequested		( FALSE )
,	m_bTorrentStarted		( FALSE )
,	m_tTorrentTracker		( 0 )
,	m_nTorrentUploaded		( 0 )
,	m_nTorrentDownloaded	( 0 )
,	m_bTorrentEndgame		( FALSE )
,	m_bTorrentTrackerError	( FALSE )
	
,	m_pTorrentBlock			( NULL )
,	m_nTorrentBlock			( 0 )
,	m_nTorrentSize			( 0 )
,	m_nTorrentSuccess		( 0 )
,	m_bSeeding				( FALSE )
	
,	m_tTorrentChoke			( 0 )
,	m_tTorrentSources		( 0 )
{
	// Generate random Key value
	m_sKey = _T("");

	for ( int nChar = 1 ; nChar < 6 ; nChar++ ) 
	{
		m_sKey += GenerateCharacter();
	}
}

CDownloadWithTorrent::~CDownloadWithTorrent()
{
	if ( m_bTorrentRequested )
		SendStopped();

	{
		CSingleLock oLock( &m_pRequestsSection, TRUE );
		while( ! m_pRequests.IsEmpty() )
		{
			oLock.Unlock();
			Sleep( 100 );
			oLock.Lock();
		}
	}

	m_pPeerID.clear();

	CloseTorrentUploads();

	if ( m_pTorrentBlock )
		delete [] m_pTorrentBlock;
}

void CDownloadWithTorrent::Add(CBTTrackerRequest* pRequest)
{
	CQuickLock oLock( m_pRequestsSection );

	ASSERT( m_pRequests.Find( pRequest ) == NULL );
	m_pRequests.AddTail( pRequest );
}

void CDownloadWithTorrent::Remove(CBTTrackerRequest* pRequest)
{
	CQuickLock oLock( m_pRequestsSection );

	POSITION pos = m_pRequests.Find( pRequest );
	ASSERT( pos != NULL );
	m_pRequests.RemoveAt( pos );
}

TCHAR CDownloadWithTorrent::GenerateCharacter() const
{
	switch ( GetRandomNum( 0, 2 ) )
	{
	case 0:
		return static_cast< TCHAR >( 'a' + ( GetRandomNum( 0, 25 ) ) );
	case 1:
		return static_cast< TCHAR >( 'A' + ( GetRandomNum( 0, 25 ) ) );
	default:
		return static_cast< TCHAR >( '0' + ( GetRandomNum( 0, 9 ) ) );
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent serialize

void CDownloadWithTorrent::Serialize(CArchive& ar, int nVersion)
{
	CDownloadWithFile::Serialize( ar, nVersion );
	
	if ( nVersion < 22 ) return;
	
	m_pTorrent.Serialize( ar );
	
	if ( ar.IsLoading() && IsTorrent() )
	{
		m_oBTH = m_pTorrent.m_oBTH;
        m_bBTHTrusted = true;
		if ( ! m_oTiger && m_pTorrent.m_oTiger )
		{
			m_oTiger = m_pTorrent.m_oTiger;
			m_bTigerTrusted = true;
		}
		if ( ! m_oSHA1 && m_pTorrent.m_oSHA1 )
		{
			m_oSHA1 = m_pTorrent.m_oSHA1;
			m_bSHA1Trusted = true;
		}
		if ( ! m_oED2K && m_pTorrent.m_oED2K )
		{
			m_oED2K = m_pTorrent.m_oED2K;
			m_bED2KTrusted = true;
		}
		if ( ! m_oMD5 && m_pTorrent.m_oMD5 )
		{
			m_oMD5 = m_pTorrent.m_oMD5;
			m_bMD5Trusted = true;
		}
	}

	if ( nVersion >= 23 && IsTorrent() )
	{
		if ( ar.IsStoring() )
		{
			ar << m_nTorrentSuccess;
			ar.Write( m_pTorrentBlock, sizeof(BYTE) * m_nTorrentBlock );
			ar << BOOL( m_bSeeding && Settings.BitTorrent.AutoSeed );
			ar << m_sServingFileName;
		}
		else
		{
			m_nTorrentSize	= m_pTorrent.m_nBlockSize;
			m_nTorrentBlock	= m_pTorrent.m_nBlockCount;
			
			ar >> m_nTorrentSuccess;
			m_pTorrentBlock = new BYTE[ m_nTorrentBlock ];
			ReadArchive( ar, m_pTorrentBlock, sizeof(BYTE) * m_nTorrentBlock );
			if ( nVersion >= 34 )
			{
				ar >> m_bSeeding;
				ar >> m_sServingFileName;
			}
			GenerateTorrentDownloadID();
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent set torrent

BOOL CDownloadWithTorrent::SetTorrent(CBTInfo* pTorrent)
{
	if ( pTorrent == NULL ) return FALSE;
	if ( IsTorrent() ) return FALSE;
	if ( ! pTorrent->IsAvailable() ) return FALSE;
	
	m_pTorrent.Copy( pTorrent );
	
	m_oBTH = m_pTorrent.m_oBTH;
	m_bBTHTrusted = true;
	if ( ! m_oTiger && m_pTorrent.m_oTiger )
	{
		m_oTiger = m_pTorrent.m_oTiger;
		m_bTigerTrusted = true;
	}
	if ( ! m_oSHA1 && m_pTorrent.m_oSHA1 )
	{
		m_oSHA1 = m_pTorrent.m_oSHA1;
		m_bSHA1Trusted = true;
	}
	if ( ! m_oED2K && m_pTorrent.m_oED2K )
	{
		m_oED2K = m_pTorrent.m_oED2K;
		m_bED2KTrusted = true;
	}
	if ( ! m_oMD5 && m_pTorrent.m_oMD5 )
	{
		m_oMD5 = m_pTorrent.m_oMD5;
		m_bMD5Trusted = true;
	}
	
	m_nTorrentSize	= m_pTorrent.m_nBlockSize;
	m_nTorrentBlock	= m_pTorrent.m_nBlockCount;
	m_pTorrentBlock	= new BYTE[ m_nTorrentBlock ];
	
	ZeroMemory( m_pTorrentBlock, sizeof(BYTE) * m_nTorrentBlock );
	SetModified();
	
	CreateDirectory( Settings.Downloads.TorrentPath );
	LibraryFolders.AddFolder( Settings.Downloads.TorrentPath, FALSE );
	pTorrent->SaveTorrentFile( Settings.Downloads.TorrentPath );

	if ( ! Settings.BitTorrent.AdvancedInterfaceSet )
	{
		// If this is the first time the user has downloaded a torrent, turn the extra interface on.
		Settings.BitTorrent.AdvancedInterfaceSet	= TRUE;
		Settings.BitTorrent.AdvancedInterface		= TRUE;
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent run

bool CDownloadWithTorrent::RunTorrent(DWORD tNow)
{
	// Return if this isn't a torrent
	if ( !IsTorrent() )
		return true;

	// Return if disk is full
	if ( m_bDiskFull )
		return false;

	// Choke torrents every 10 seconds
	if ( tNow > m_tTorrentChoke && tNow - m_tTorrentChoke >= 10000ul )
		ChokeTorrent( tNow );

	// Check if the torrent file exists and has been opened
	if ( m_pFile && m_pFile->IsOpen() == FALSE )
	{
		// Check if file has been created on the HDD
		bool bAllocated = ( !m_sPath.IsEmpty()
			&& GetFileAttributes( m_sPath ) != INVALID_FILE_ATTRIBUTES );

		// Try to create and/or open the file
		if ( !PrepareFile() )
			return false;

		// If file needed to be created, allocate disk space for it
		if ( !bAllocated )
		{
			ASSERT( m_pTask == NULL );
			m_pTask = new CDownloadTask( static_cast< CDownload* >( this ),
				CDownloadTask::dtaskAllocate );
		}
	}

	// Return if this download is waiting for a download task to finish
	if ( m_pTask )
		return false;

	// Can't send an announce for a trackerless torrent
	if ( !m_pTorrent.m_pAnnounceTracker )
		return true;

	// Generate a peerid if there isn't one
	if ( !m_pPeerID )
		GenerateTorrentDownloadID();

	// Store some values for later
	DWORD nSourcesCount = 0ul;
	DWORD nSourcesMax = 0ul;
	DWORD nSourcesWanted = 0ul;

	// Check if a tracker has already been locked onto
	if ( !m_bTorrentStarted )
	{
		// Check if download is active, isn't already waiting for a request
		// reply and is allowed to try and contact this tracker
		if ( !IsPaused() && IsTrying() && !m_bTorrentRequested
			&& tNow > m_tTorrentTracker )
		{
			// Get the # of sources that can be connected to
			nSourcesCount = GetBTSourceCount( TRUE );

			// Calculate how many new sources are wanted,
			// expect a high failure rate
			nSourcesMax = Settings.BitTorrent.DownloadConnections * 4ul;
			if ( nSourcesCount < nSourcesMax )
				nSourcesWanted = nSourcesMax - nSourcesCount;

			// Initial announce to tracker
			SendStarted( nSourcesWanted );
		}

		// Report that the torrent checks have run successfully
		return true;
	}

	// Store if this is a regular update or not
	bool bRegularUpdate = tNow > m_tTorrentTracker;

	// Check if an update needs to be sent to the tracker. This can either be a
	// regular update or a request for more sources if the number of known
	// sources is getting too low.
	if ( bRegularUpdate
		|| tNow - m_tTorrentSources > Settings.BitTorrent.DefaultTrackerPeriod )
	{
		// Check if the torrent is seeding
		if ( IsSeeding() )
		{
			// Use the upload count values
			nSourcesCount = Uploads.GetTorrentUploadCount();
			nSourcesMax = Settings.BitTorrent.UploadCount;
		}
		else
		{
			// Use the download count values
			nSourcesCount = GetBTSourceCount();
			nSourcesMax = Settings.BitTorrent.DownloadConnections;
		}

		// Request more sources, more often, for regular updates
		if ( bRegularUpdate )
			nSourcesMax *= 4ul;

		// Calculate the # of sources needed
		if ( nSourcesCount < nSourcesMax )
			nSourcesWanted = nSourcesMax - nSourcesCount;

		// Check if an update needs to be sent
		if ( bRegularUpdate || nSourcesWanted )
		{
			// Send tracker update
			SendUpdate( nSourcesWanted );
		}
		else
		{
			// Record the time that source counts checked even if no update
			// was sent
			m_tTorrentSources = tNow;
		}
	}

	// Report that the torrent checks have run successfully
	return true;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent Create Peer ID

// The 'Peer ID' is different for each download and is not retained between
// sessions.

BOOL CDownloadWithTorrent::GenerateTorrentDownloadID()
{
	theApp.Message( MSG_DEBUG, _T("Creating BitTorrent Peer ID") );

	//Check ID is not in use
	ASSERT( !m_pPeerID );
	if ( m_pPeerID )
	{
		theApp.Message( MSG_ERROR, _T("Attempted to re-create an in-use Peer ID") );
		return FALSE;
	}

	// Client ID
	// Azureus style "-SSVVVV-" http://bittorrent.org/beps/bep_0020.html
	m_pPeerID[ 0 ] = '-';
	m_pPeerID[ 1 ] = BT_ID1;
	m_pPeerID[ 2 ] = BT_ID2;
	m_pPeerID[ 3 ] = static_cast< BYTE >( '0' + theApp.m_nVersion[0] );
	m_pPeerID[ 4 ] = static_cast< BYTE >( '0' + theApp.m_nVersion[1] );
	m_pPeerID[ 5 ] = static_cast< BYTE >( '0' + theApp.m_nVersion[2] );
	m_pPeerID[ 6 ] = static_cast< BYTE >( '0' + theApp.m_nVersion[3] );
	m_pPeerID[ 7 ] = '-';

	// Random characters for the rest of the Client ID
	for ( int nByte = 8 ; nByte < 20 ; nByte++ )
	{
		m_pPeerID[ nByte ] = static_cast< BYTE >( rand() & 0xFF );
	}
	m_pPeerID.validate();
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Send BT Tracker Request actions

void CDownloadWithTorrent::SendStarted(DWORD nNumWant)
{
	// Return if there is no tracker
	if ( m_pTorrent.m_sTracker.IsEmpty() )
		return;

	// Log the 'start' event
	theApp.Message( MSG_INFO,
		_T("[BT] Sending initial tracker announce for %s"),
		m_pTorrent.m_sName );

	// Record that the start request has been sent
	m_bTorrentRequested = TRUE;
	m_tTorrentTracker = m_tTorrentSources = GetTickCount();
	m_tTorrentTracker += Settings.BitTorrent.DefaultTrackerPeriod;
	m_nTorrentDownloaded = m_nTorrentUploaded = 0ull;

	// Create and run tracker request
	new CBTTrackerRequest( this, _T("started"), nNumWant, TRUE );
}

void CDownloadWithTorrent::SendUpdate(DWORD nNumWant)
{
	// Return if there is no tracker
	if ( m_pTorrent.m_sTracker.IsEmpty() )
		return;

	// Log the 'update' event
	theApp.Message( MSG_INFO,
		_T("[BT] Sending update tracker announce for %s"),
		m_pTorrent.m_sName );

	// Record that an update has been sent
	m_tTorrentTracker = m_tTorrentSources = GetTickCount();
	m_tTorrentTracker += Settings.BitTorrent.DefaultTrackerPeriod;

	// Create and run tracker request
	new CBTTrackerRequest( this, NULL, nNumWant, TRUE );
}

void CDownloadWithTorrent::SendCompleted()
{
	// Return if there is no tracker
	if ( m_pTorrent.m_sTracker.IsEmpty() )
		return;

	// Log the 'complete' event
	theApp.Message( MSG_INFO,
		_T("[BT] Sending completed tracker announce for %s"),
		m_pTorrent.m_sName );

	// Create and run tracker request
	new CBTTrackerRequest( this, _T("completed"), 0ul, TRUE );
}

void CDownloadWithTorrent::SendStopped()
{
	// Return if there is no tracker
	if ( m_pTorrent.m_sTracker.IsEmpty() )
		return;

	// Log the 'stop' event
	theApp.Message( MSG_INFO,
		_T("[BT] Sending final tracker announce for %s"),
		m_pTorrent.m_sName );

	// Update download to indicate it has been stopped
	m_bTorrentStarted = m_bTorrentRequested = FALSE;

	// Create and run tracker request
	new CBTTrackerRequest( this, _T("stopped"), 0ul, FALSE );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent tracker event handler

void CDownloadWithTorrent::OnTrackerEvent(bool bSuccess, LPCTSTR pszReason, LPCTSTR pszTip)
{
	CSingleLock oLock( &Transfers.m_pSection );
	if ( ! oLock.Lock( 500 ) )
		// Abort probably due download cancel
		return;

	DWORD tNow = GetTickCount();

	if ( bSuccess )
	{
		// Success! Reset error conditions
		m_bTorrentTrackerError = FALSE;
		m_sTorrentTrackerError.Empty();
		m_pTorrent.SetTrackerSucceeded(tNow);

		theApp.Message( MSG_INFO, _T("%s"), pszReason );

		// Lock on this tracker if we were searching for one
		if ( m_pTorrent.m_nTrackerMode == tMultiFinding )
		{
			theApp.Message( MSG_DEBUG , _T("[BT] Locked onto tracker %s"),
				m_pTorrent.m_sTracker );
			m_pTorrent.m_nTrackerMode = tMultiFound;
		}
	}
	else
	{
		// There was a problem with the tracker
		m_bTorrentTrackerError = TRUE;
		m_sTorrentTrackerError = ( pszTip ? pszTip : pszReason );
		m_pTorrent.m_pAnnounceTracker->m_nFailures++;
		m_bTorrentRequested = m_bTorrentStarted = FALSE;
		m_tTorrentTracker = tNow + GetRetryTime();
		m_pTorrent.SetTrackerRetry( m_tTorrentTracker );

		theApp.Message( MSG_ERROR, _T("%s"), pszReason );

		if ( m_pTorrent.IsMultiTracker() )
		{
			// Try the next one
			m_pTorrent.SetTrackerNext( tNow );

			// Set retry time
			m_tTorrentTracker = m_pTorrent.m_pAnnounceTracker->m_tNextTry;
			
			// Load the error message string
			CString strFormat, strErrorMessage;
			LoadString( strFormat, IDS_BT_TRACKER_MULTI );
			strErrorMessage.Format( strFormat, m_pTorrent.m_nTrackerIndex + 1, m_pTorrent.m_pTrackerList.GetCount() );
			m_sTorrentTrackerError = m_sTorrentTrackerError + _T(" | ") + strErrorMessage;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent tracker retry calculation

DWORD CDownloadWithTorrent::GetRetryTime() const
{
	// 0..2 - 1 min, 3..5 - 4 min, 6..8 - 9 min, ..., > 20 - 1 hour
	DWORD tRetryTime = m_pTorrent.GetTrackerFailures() / 3ul + 1ul;
	tRetryTime *= tRetryTime * 60ul * 1000ul;
	if ( tRetryTime > 60ul * 60ul * 1000ul )
		tRetryTime = 60ul * 60ul * 1000ul;
	return tRetryTime;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent download transfer linking

CDownloadTransferBT* CDownloadWithTorrent::CreateTorrentTransfer(CBTClient* pClient)
{
	if ( IsMoving() || IsPaused() ) return NULL;
	
	CDownloadSource* pSource = NULL;
	
	Hashes::Guid tmp = transformGuid( pClient->m_oGUID );
	for ( pSource = GetFirstSource() ; pSource ; pSource = pSource->m_pNext )
	{
		if ( pSource->m_nProtocol == PROTOCOL_BT &&
			validAndEqual( pSource->m_oGUID, tmp ) ) break;
	}
	
	if ( pSource == NULL )
	{
		pSource = new CDownloadSource( (CDownload*)this, pClient->m_oGUID,
			&pClient->m_pHost.sin_addr, htons( pClient->m_pHost.sin_port ) );
		pSource->m_bPushOnly = !(pClient->m_bInitiated);
		
		if ( ! AddSourceInternal( pSource ) ) return NULL;
	}
		
	if ( pSource->m_pTransfer != NULL ) 
	{
		// A download transfer already exists
		return NULL;
	}
	
	pSource->m_pTransfer = new CDownloadTransferBT( pSource, pClient );
	
	return (CDownloadTransferBT*)pSource->m_pTransfer;
}

void CDownloadWithTorrent::OnFinishedTorrentBlock(DWORD nBlock)
{
	for ( CDownloadTransferBT* pTransfer = (CDownloadTransferBT*)GetFirstTransfer() ; pTransfer ; pTransfer = (CDownloadTransferBT*)pTransfer->m_pDlNext )
	{
		if ( pTransfer->m_nProtocol == PROTOCOL_BT )
		{
			pTransfer->SendFinishedBlock( nBlock );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent create bitfield

CBTPacket* CDownloadWithTorrent::CreateBitfieldPacket()
{
	ASSERT( IsTorrent() );
	
	CBTPacket* pPacket = CBTPacket::New( BT_PACKET_BITFIELD );
	int nCount = 0;
	
	for ( QWORD nBlock = 0 ; nBlock < m_nTorrentBlock ; )
	{
		BYTE nByte = 0;
		
		for ( int nBit = 7 ; nBit >= 0 && nBlock < m_nTorrentBlock ; nBit--, nBlock++ )
		{
			if ( m_pTorrentBlock[ nBlock ] == TRI_TRUE )
			{
				nByte |= ( 1 << nBit );
				nCount++;
			}
		}
		
		pPacket->WriteByte( nByte );
	}
	
	if ( nCount > 0 ) return pPacket;
	pPacket->Release();
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent upload linking

void CDownloadWithTorrent::AddUpload(CUploadTransferBT* pUpload)
{
	if ( m_pTorrentUploads.Find( pUpload ) == NULL )
		m_pTorrentUploads.AddTail( pUpload );
}

void CDownloadWithTorrent::RemoveUpload(CUploadTransferBT* pUpload)
{
	if ( POSITION pos = m_pTorrentUploads.Find( pUpload ) )
		m_pTorrentUploads.RemoveAt( pos );
}

void CDownloadWithTorrent::CloseTorrentUploads()
{
	for ( POSITION pos = m_pTorrentUploads.GetHeadPosition() ; pos ; )
	{
		CUploadTransferBT* pUpload = m_pTorrentUploads.GetNext( pos );
		pUpload->Close();
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent choking

void CDownloadWithTorrent::ChokeTorrent(DWORD tNow)
{
	BOOL bChooseRandom = TRUE;
	int nTotalRandom = 0;
	CList< void* > pSelected;
	
	if ( ! tNow ) tNow = GetTickCount();
	if ( tNow > m_tTorrentChoke && tNow - m_tTorrentChoke < 2000 ) return;
	m_tTorrentChoke = tNow;

	// Check if a seeding torrent needs to start some new connections
	if ( IsSeeding() )
	{
		// We might need to 'push' a connection if we don't have enough upload connections
		if ( (DWORD)m_pTorrentUploads.GetCount() < Settings.BitTorrent.UploadCount * 2 &&
			(DWORD)m_pTorrentUploads.GetCount() != GetBTSourceCount() &&
			CanStartTransfers( tNow ) )
		{
			theApp.Message( MSG_DEBUG, _T("Attempting to push-start a BitTorrent upload for %s"),
				m_pTorrent.m_sName );
			StartNewTransfer( tNow );
		}
	}

	
	for ( POSITION pos = m_pTorrentUploads.GetHeadPosition() ; pos ; )
	{
		CUploadTransferBT* pTransfer = m_pTorrentUploads.GetNext( pos );
		if ( pTransfer->m_nProtocol != PROTOCOL_BT ) continue;
		
		if ( pTransfer->m_nRandomUnchoke == 2 )
		{
			if ( tNow - pTransfer->m_tRandomUnchoke >= Settings.BitTorrent.RandomPeriod )
			{
				pTransfer->m_nRandomUnchoke = 1;
			}
			else
			{
				bChooseRandom = FALSE;
			}
		}
		
		if ( pTransfer->m_bInterested )
			nTotalRandom += ( pTransfer->m_nRandomUnchoke == 0 ) ? 3 : 1;
	}
	
	if ( bChooseRandom && nTotalRandom > 0 )
	{
		nTotalRandom = GetRandomNum( 0, nTotalRandom - 1 );
		
		for ( POSITION pos = m_pTorrentUploads.GetHeadPosition() ; pos ; )
		{
			CUploadTransferBT* pTransfer = m_pTorrentUploads.GetNext( pos );
			if ( pTransfer->m_nProtocol != PROTOCOL_BT ) continue;
			if ( pTransfer->m_bInterested == FALSE ) continue;
			
			int nWeight = ( pTransfer->m_nRandomUnchoke == 0 ) ? 3 : 1;
			
			if ( nTotalRandom < nWeight )
			{
				pTransfer->m_nRandomUnchoke = 2;
				pTransfer->m_tRandomUnchoke = tNow;
				pSelected.AddTail( pTransfer );
				break;
			}
			else
			{
				nTotalRandom -= nWeight;
			}
		}
	}
	
	while ( (DWORD)pSelected.GetCount() < Settings.BitTorrent.UploadCount )
	{
		CUploadTransferBT* pBest = NULL;
		DWORD nBest = 0;
		
		for ( POSITION pos = m_pTorrentUploads.GetHeadPosition() ; pos ; )
		{
			CUploadTransferBT* pTransfer = m_pTorrentUploads.GetNext( pos );
			
			if (	pTransfer->m_nProtocol == PROTOCOL_BT &&
					pTransfer->m_bInterested &&
					pSelected.Find( pTransfer->m_pClient ) == NULL &&
					pTransfer->GetAverageSpeed() >= nBest )
			{
				pBest = pTransfer;
				nBest = pTransfer->GetAverageSpeed();
			}
		}
		
		if ( pBest == NULL ) break;
		pSelected.AddTail( pBest->m_pClient );
	}
	
	while ( (DWORD)pSelected.GetCount() < Settings.BitTorrent.UploadCount )
	{
		CDownloadTransferBT* pBest = NULL;
		DWORD nBest = 0;
		
		for ( CDownloadTransferBT* pTransfer = (CDownloadTransferBT*)GetFirstTransfer()
				; pTransfer ; pTransfer = (CDownloadTransferBT*)pTransfer->m_pDlNext )
		{
			if (	pTransfer->m_nProtocol == PROTOCOL_BT &&
					pSelected.Find( pTransfer->m_pClient ) == NULL &&
					pTransfer->m_nState == dtsDownloading &&
					pTransfer->m_pClient->m_pUpload->m_bInterested &&
					pTransfer->GetAverageSpeed() >= nBest )
			{
				pBest = pTransfer;
				nBest = pTransfer->GetAverageSpeed();
			}
		}
		
		if ( pBest == NULL ) break;
		pSelected.AddTail( pBest->m_pClient );
	}
	
	for ( POSITION pos = m_pTorrentUploads.GetHeadPosition() ; pos ; )
	{
		CUploadTransferBT* pTransfer = m_pTorrentUploads.GetNext( pos );
		if ( pTransfer->m_nProtocol != PROTOCOL_BT ) continue;
		
		pTransfer->SetChoke(	pTransfer->m_bInterested == TRUE &&
								pSelected.Find( pTransfer->m_pClient ) == NULL );
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent search -> tracker link

BOOL CDownloadWithTorrent::FindMoreSources()
{
	if ( m_pFile != NULL && m_bTorrentRequested )
	{
		ASSERT( IsTorrent() );
		
		if ( GetTickCount() - m_tTorrentSources > 15000 )
		{
			SendUpdate( min( Settings.BitTorrent.DownloadConnections * 4ul, 100ul ) );
			return TRUE;
		}
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent seed

BOOL CDownloadWithTorrent::SeedTorrent(LPCTSTR pszTarget)
{
	if ( IsMoving() || IsCompleted() )
		return FALSE;

	if ( m_sPath == pszTarget )
		return FALSE;
	
	ASSERT( m_pFile != NULL );
	if ( m_pFile == NULL )
		return FALSE;

	ASSERT( m_pFile->IsOpen() == FALSE );
	if ( m_pFile->IsOpen() )
		return FALSE;

	delete m_pFile;
	m_pFile = NULL;

	GenerateTorrentDownloadID();
	
	CDownload* pDownload	= static_cast< CDownload* >( this );
	pDownload->m_bSeeding	= TRUE;
	pDownload->m_bComplete	= TRUE;
	pDownload->m_tCompleted	= GetTickCount();
	pDownload->m_bVerify	= TRI_TRUE;
	
	memset( m_pTorrentBlock, TRI_TRUE, m_nTorrentBlock );
	m_nTorrentSuccess = m_nTorrentBlock;
	
	if ( m_sPath.GetLength() > 0 )
	{
		ASSERT( FALSE );
		::DeleteFile( m_sPath );
		::DeleteFile( m_sPath + _T(".sd") );
	}
	
	m_sPath = pszTarget;
	SetModified();
	
	SendStarted( Settings.BitTorrent.UploadCount * 4ul );	

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent Close

void CDownloadWithTorrent::CloseTorrent()
{
	if ( m_bTorrentRequested ) 
		SendStopped();

	CloseTorrentUploads();
}


//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent stats

float CDownloadWithTorrent::GetRatio() const
{
	if ( m_pTorrent.m_nTotalUpload == 0 || m_pTorrent.m_nTotalDownload == 0 ) return 0;
	return float( m_pTorrent.m_nTotalUpload * 10000 / m_pTorrent.m_nTotalDownload ) / 100.0f;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent Check if it's okay to start a new download transfer

BOOL CDownloadWithTorrent::CheckTorrentRatio() const
{
	if ( ! IsTorrent() ) return TRUE;									// Not a torrent
	
	if ( m_pTorrent.m_nStartDownloads == dtAlways ) return TRUE;// Torrent is set to download as needed

	if ( m_pTorrent.m_nStartDownloads == dtWhenRatio )			// Torrent is set to download only when ratio is okay
	{
		if ( m_nTorrentUploaded > m_nTorrentDownloaded ) return TRUE;	// Ratio OK
		if ( GetVolumeComplete() < 5 * 1024 * 1024 ) return TRUE;		// Always get at least 5 MB so you have something to upload	
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent check if upload exists

BOOL CDownloadWithTorrent::UploadExists(in_addr* pIP) const
{
	for ( POSITION pos = m_pTorrentUploads.GetHeadPosition() ; pos ; )
	{
		CUploadTransferBT* pTransfer = m_pTorrentUploads.GetNext( pos );

		if ( ( pTransfer->m_nProtocol == PROTOCOL_BT ) &&
			 ( pTransfer->m_nState != upsNull ) &&
			 ( pTransfer->m_pHost.sin_addr.S_un.S_addr == pIP->S_un.S_addr ) )
			return TRUE;
	}
	return FALSE;
}

BOOL CDownloadWithTorrent::UploadExists(const Hashes::BtGuid& oGUID) const
{
	for ( POSITION pos = m_pTorrentUploads.GetHeadPosition() ; pos ; )
	{
		CUploadTransferBT* pTransfer = m_pTorrentUploads.GetNext( pos );

		if ( ( pTransfer->m_nProtocol == PROTOCOL_BT ) &&
			 ( pTransfer->m_nState != upsNull ) &&
			 validAndEqual( oGUID, pTransfer->m_pClient->m_oGUID ) )
			return TRUE;
	}
	return FALSE;
}

CString CDownloadWithTorrent::FindTorrentFile(CBTInfo::CBTFile* pFile)
{
	CString strFile;
	
	CString strPath = pFile->GetPath();
	int nSlash = strPath.ReverseFind( '\\' );
	if ( nSlash >= 0 ) strPath = strPath.Left( nSlash + 1 );

	if ( pFile->IsHashed() )
	{
		CSingleLock oLibraryLock( &Library.m_pSection, TRUE );
		if ( CLibraryFile* pShared = LibraryMaps.LookupFileByHash(
			pFile->m_oSHA1, pFile->m_oTiger, pFile->m_oED2K, pFile->m_oBTH,
			pFile->m_oMD5, pFile->m_nSize, pFile->m_nSize, FALSE, TRUE ) )
		{
			// Refill missed hashes
			if ( ! pFile->m_oSHA1 && pShared->m_oSHA1 )
				pFile->m_oSHA1 = pShared->m_oSHA1;
			if ( ! pFile->m_oTiger && pShared->m_oTiger )
				pFile->m_oTiger = pShared->m_oTiger;
			if ( ! pFile->m_oED2K && pShared->m_oED2K )
				pFile->m_oED2K = pShared->m_oED2K;
			if ( ! pFile->m_oBTH && pShared->m_oBTH )
				pFile->m_oBTH = pShared->m_oBTH;
			if ( ! pFile->m_oMD5 && pShared->m_oMD5 )
				pFile->m_oMD5 = pShared->m_oMD5;

			strFile = pShared->GetPath();
			oLibraryLock.Unlock();

			if ( GetFileAttributes( strFile ) != INVALID_FILE_ATTRIBUTES )
				return strFile;
		}
	}

	strFile = Settings.Downloads.CompletePath + "\\" + pFile->m_sPath;
	if ( GetFileAttributes( strFile ) != INVALID_FILE_ATTRIBUTES ) return strFile;

	strFile = strPath + pFile->m_sPath;
	if ( GetFileAttributes( strFile ) != INVALID_FILE_ATTRIBUTES ) return strFile;
	
	//Try removing the outer directory in case of multi-file torrent oddities
	LPCTSTR pszName = _tcsrchr( pFile->m_sPath, '\\' );
	if ( pszName == NULL ) pszName = pFile->m_sPath; else pszName ++;

	strFile = Settings.Downloads.CompletePath + "\\" + pszName;
	if ( GetFileAttributes( strFile ) != INVALID_FILE_ATTRIBUTES ) return strFile;

	strFile = strPath + pszName;
	if ( GetFileAttributes( strFile ) != INVALID_FILE_ATTRIBUTES ) return strFile;

	strFile.Empty();
	return strFile;
}
