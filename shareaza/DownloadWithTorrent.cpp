//
// DownloadWithTorrent.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2017.
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
#include "BTClient.h"
#include "BTPacket.h"
#include "DlgProgressBar.h"
#include "DownloadGroups.h"
#include "DownloadSource.h"
#include "DownloadTransferBT.h"
#include "DownloadWithTorrent.h"
#include "Downloads.h"
#include "FragmentedFile.h"
#include "GProfile.h"
#include "HostCache.h"
#include "Library.h"
#include "LibraryFolders.h"
#include "Network.h"
#include "UploadTransferBT.h"
#include "Uploads.h"

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
,	m_bTorrentEndgame		( false )
,	m_bTorrentTrackerError	( FALSE )

,	m_nTorrentBlock			( 0 )
,	m_nTorrentSize			( 0 )
,	m_nTorrentSuccess		( 0 )
,	m_bSeeding				( FALSE )

,	m_tTorrentChoke			( 0 )
,	m_tTorrentSources		( 0 )
{
	// Generate random Key value
	for ( int nChar = 1 ; nChar < 6 ; nChar++ )
	{
		m_sKey += GenerateCharacter();
	}
}

CDownloadWithTorrent::~CDownloadWithTorrent()
{
	if ( m_bTorrentRequested )
		SendStopped();

	m_pPeerID.clear();

	CloseTorrentUploads();
}

bool CDownloadWithTorrent::IsSeeding() const
{
	return m_bSeeding != 0;
}

bool CDownloadWithTorrent::IsTorrent() const
{
	return m_pTorrent.IsAvailable();
}

bool CDownloadWithTorrent::IsSingleFileTorrent() const
{
	return IsTorrent() && ( m_pTorrent.GetCount() == 1 );
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

	if ( nVersion < 22 )
		return;

	m_pTorrent.Serialize( ar );

	if ( nVersion < 23 )
		return;

	if ( ! IsTorrent() )
		return;

	if ( ar.IsStoring() )
	{
		ar << m_nTorrentSuccess;
		if ( m_pTorrentBlock )
		{
			ar.Write( m_pTorrentBlock, sizeof(BYTE) * m_nTorrentBlock );
		}
		ar << BOOL( m_bSeeding && Settings.BitTorrent.AutoSeed );
	}
	else
	{
		CString sServingFileName;

		m_nTorrentSize	= m_pTorrent.m_nBlockSize;
		m_nTorrentBlock	= m_pTorrent.m_nBlockCount;

		ar >> m_nTorrentSuccess;
		m_pTorrentBlock.Free();
		if ( m_nTorrentBlock )
		{
			m_pTorrentBlock.Attach( new BYTE[ m_nTorrentBlock ] );
			memset( m_pTorrentBlock, TRI_UNKNOWN, m_nTorrentBlock );
			ReadArchive( ar, m_pTorrentBlock, sizeof(BYTE) * m_nTorrentBlock );
		}
		if ( nVersion >= 34 )
		{
			ar >> m_bSeeding;

			if ( nVersion < 41 )
			{
				ar >> sServingFileName;
			}
		}
		GenerateTorrentDownloadID();

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
		if ( ! m_oBTH && m_pTorrent.m_oBTH )
		{
			m_oBTH = m_pTorrent.m_oBTH;
			m_bBTHTrusted = true;
		}

		if ( nVersion < 40 )
		{
			// Convert old multifile torrent to new file system

			// Get old file name
			ASSERT( GetFileCount() == 1 );
			CString sPath = GetPath( 0 );
			if ( sServingFileName.IsEmpty() )
				sServingFileName = sPath;

			CProgressBarDlg oProgress( AfxGetMainWnd() );
			oProgress.SetWindowText( LoadString( IDS_BT_UPDATE_TITLE ) );
			oProgress.SetActionText( LoadString( IDS_BT_UPDATE_CONVERTING ) );
			oProgress.SetEventText( m_sName );
			oProgress.SetEventRange( 0, (int)( m_pTorrent.m_nSize / 1024ull ) );
			oProgress.SetSubEventText( sServingFileName );
			oProgress.SetSubEventRange( 0, (int)( m_pTorrent.m_nSize / 1024ull ) );

			oProgress.CenterWindow();
			oProgress.ShowWindow( SW_SHOW );
			oProgress.UpdateWindow();
			oProgress.UpdateData( FALSE );

			// Close old files
			ClearFile();

			// Create a bunch of new empty files
			auto_ptr< CFragmentedFile > pFragFile( GetFile() );
			if ( ! pFragFile.get() )
				AfxThrowMemoryException();
			if ( ! pFragFile->Open( m_pTorrent, ! IsSeeding() ) )
				AfxThrowFileException( CFileException::genericException );

			if ( ! IsSeeding() )
			{
				// Check for free space
				if ( ! Downloads.IsSpaceAvailable( m_pTorrent.m_nSize,
					Downloads.dlPathIncomplete ) )
					AfxThrowFileException( CFileException::diskFull );

				// Open old file
				CFile oSource( sServingFileName,
					CFile::modeRead | CFile::shareDenyWrite | CFile::osSequentialScan );

				// Copy data from old file to new files
				const QWORD BUFFER_SIZE = 4ul * 1024ul * 1024ul;	// 4 MB
				auto_array< BYTE > pBuffer( new BYTE[ BUFFER_SIZE ] );
				if ( ! pBuffer.get() )
					AfxThrowMemoryException();
				// TODO: Optimize this by reading only available data
				QWORD nTotal = 0ull;
				for ( QWORD nLength = m_pTorrent.m_nSize; nLength; )
				{
					DWORD nBuffer = (DWORD)min( nLength, BUFFER_SIZE );
					DWORD nRead = oSource.Read( pBuffer.get(), nBuffer );
					if ( nRead )
					{
						if ( ! pFragFile->Write( nTotal, pBuffer.get(), nRead ) )
							AfxThrowFileException( CFileException::genericException );
					}
					if ( nRead != nBuffer )
						// EOF
						break;
					nLength -= nBuffer;
					nTotal += nBuffer;

					CString strText;
					strText.Format( _T("%s %s %s"),
						(LPCTSTR)Settings.SmartVolume( nTotal, KiloBytes ),
						(LPCTSTR)LoadString( IDS_GENERAL_OF ),
						(LPCTSTR)Settings.SmartVolume( m_pTorrent.m_nSize, KiloBytes ) );
					oProgress.SetSubActionText( strText );
					oProgress.StepSubEvent( (int)( nBuffer / 1024ul ) );
					oProgress.SetEventPos( (int)( nTotal / 1024ull ) );
					oProgress.UpdateWindow();

					AfxGetMainWnd()->UpdateWindow();

					Sleep( 50 );
				}
			}

			// Delete old files
			DeleteFileEx( sServingFileName, FALSE, FALSE, TRUE );
			if ( sServingFileName != sPath )
				DeleteFileEx( sPath, FALSE, FALSE, TRUE );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent submit data

BOOL CDownloadWithTorrent::SubmitData(QWORD nOffset, LPBYTE pData, QWORD nLength)
{
	if ( IsTorrent() )
	{
		CSingleLock oLock( &Transfers.m_pSection );
		if ( oLock.Lock( 250 ) )
		{
			for ( CDownloadTransfer* pTransfer = GetFirstTransfer() ; pTransfer ; pTransfer = pTransfer->m_pDlNext )
			{
				if ( pTransfer->m_nProtocol == PROTOCOL_BT )
					pTransfer->UnrequestRange( nOffset, nLength );
			}
		}
	}

	return CDownloadWithFile::SubmitData( nOffset, pData, nLength );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent set torrent

BOOL CDownloadWithTorrent::SetTorrent(const CBTInfo* pTorrent)
{
	ASSUME_LOCK( Transfers.m_pSection );

	if ( IsMoving() || IsCompleted() )
		return FALSE;

	if ( pTorrent )
	{
		if ( ! pTorrent->IsAvailable() )
			// Something wrong
			return FALSE;

		m_pTorrent = *pTorrent;
	}

	if ( m_nSize != SIZE_UNKNOWN && // Single file download
		 m_pTorrent.IsAvailableInfo() && ( m_pTorrent.m_nSize == SIZE_UNKNOWN || m_pTorrent.m_nSize != m_nSize ) )
		return FALSE;

	if ( m_bBTHTrusted && validAndUnequal( m_oBTH, m_pTorrent.m_oBTH ) )
		return FALSE;

	if ( m_bTigerTrusted && validAndUnequal( m_oTiger, m_pTorrent.m_oTiger ) )
		return FALSE;

	if ( m_bSHA1Trusted && validAndUnequal( m_oSHA1, m_pTorrent.m_oSHA1 ) )
		return FALSE;

	if ( m_bED2KTrusted && validAndUnequal( m_oED2K, m_pTorrent.m_oED2K ) )
		return FALSE;

	if ( m_bMD5Trusted && validAndUnequal( m_oMD5, m_pTorrent.m_oMD5 ) )
		return FALSE;

	if ( m_pTorrent.m_nSize != SIZE_UNKNOWN )
	{
		m_nSize = m_pTorrent.m_nSize;
	}

	if ( m_pTorrent.m_oTiger )
	{
		m_oTiger = m_pTorrent.m_oTiger;
		m_bTigerTrusted = true;
	}

	if ( m_pTorrent.m_oSHA1 )
	{
		m_oSHA1 = m_pTorrent.m_oSHA1;
		m_bSHA1Trusted = true;
	}

	if ( m_pTorrent.m_oED2K )
	{
		m_oED2K = m_pTorrent.m_oED2K;
		m_bED2KTrusted = true;
	}

	if ( m_pTorrent.m_oMD5 )
	{
		m_oMD5 = m_pTorrent.m_oMD5;
		m_bMD5Trusted = true;
	}

	if ( m_pTorrent.m_oBTH )
	{
		m_oBTH = m_pTorrent.m_oBTH;
		m_bBTHTrusted = true;
	}

	if ( m_pTorrent.m_sName.GetLength() )
	{
		Rename( m_pTorrent.m_sName );
	}

	if ( m_pTorrent.m_nBlockSize )
	{
		m_nTorrentSize = m_pTorrent.m_nBlockSize;
	}

	if ( m_pTorrent.m_nBlockCount )
	{
		m_nTorrentBlock = m_pTorrent.m_nBlockCount;

		if ( m_nTorrentBlock )
		{
			m_pTorrentBlock.Free();
			m_pTorrentBlock.Attach( new BYTE[ m_nTorrentBlock ] );
			memset( m_pTorrentBlock, TRI_UNKNOWN, m_nTorrentBlock );
		}
	}

	m_nTorrentSuccess = 0;

	if ( CreateDirectory( Settings.Downloads.TorrentPath ) )
	{
		LibraryFolders.AddFolder( Settings.Downloads.TorrentPath, FALSE );
		m_pTorrent.SaveTorrentFile( Settings.Downloads.TorrentPath );
	}

	// Add sources from torrents - DWK
	for ( POSITION pos = m_pTorrent.m_sURLs.GetHeadPosition() ; pos ; )
	{
		AddSourceURLs( m_pTorrent.m_sURLs.GetNext( pos ) );
	}

	// Add DHT nodes to host cache
	for ( POSITION pos = m_pTorrent.m_oNodes.GetHeadPosition() ; pos ; )
	{
		HostCache.BitTorrent.Add( m_pTorrent.m_oNodes.GetNext( pos ) );
	}

	// Re-link
	DownloadGroups.Link( static_cast< CDownload* >( this ) );

	SetModified();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent run

void CDownloadWithTorrent::RunTorrent(DWORD tNow)
{
	if ( ! IsTorrent() || ! Network.IsConnected() || ! Settings.BitTorrent.EnableToday )
		return;

	// Choke torrents every 10 seconds
	if ( tNow > m_tTorrentChoke && tNow - m_tTorrentChoke >= 10000ul )
		ChokeTorrent( tNow );

	// Generate a peerid if there isn't one
	if ( !m_pPeerID )
		GenerateTorrentDownloadID();

	// Store some values for later
	DWORD nSourcesCount = 0ul;
	DWORD nSourcesMax = 0ul;
	DWORD nSourcesWanted = 0ul;

	// Check if a tracker has already been locked onto
	if ( ! m_bTorrentStarted )
	{
		// Check if download is active, isn't already waiting for a request
		// reply and is allowed to try and contact this tracker
		if ( ! m_bTorrentRequested && ( tNow > m_tTorrentTracker ) )
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

		return;
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

	return;
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
		m_pPeerID[ nByte ] = GetRandomNum( 0ui8, _UI8_MAX );
	}
	m_pPeerID.validate();
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Send BT Tracker Request actions

void CDownloadWithTorrent::SendStarted(DWORD nNumWant)
{
	if ( ! Network.IsConnected() || ! Settings.BitTorrent.EnableToday )
		return;

	// Record that the start request has been sent
	m_bTorrentRequested = TRUE;
	m_tTorrentTracker = m_tTorrentSources = GetTickCount();
	m_tTorrentTracker += Settings.BitTorrent.DefaultTrackerPeriod;
	m_nTorrentDownloaded = m_nTorrentUploaded = 0ull;

	DHT.Search( m_oBTH );

	// Return if there is no tracker
	if ( ! m_pTorrent.HasTracker() )
		return;

	// Create and run tracker request
	TrackerRequests.Request( static_cast< CDownload* >( this ), BTE_TRACKER_STARTED, nNumWant, this );
}

void CDownloadWithTorrent::SendUpdate(DWORD nNumWant)
{
	if ( ! Network.IsConnected() || ! Settings.BitTorrent.EnableToday )
		return;

	// Record that an update has been sent
	m_tTorrentTracker = m_tTorrentSources = GetTickCount();
	m_tTorrentTracker += Settings.BitTorrent.DefaultTrackerPeriod;

	DHT.Search( m_oBTH );

	// Return if there is no tracker
	if ( ! m_pTorrent.HasTracker() )
		return;

	// Create and run tracker request
	TrackerRequests.Request( static_cast< CDownload* >( this ), BTE_TRACKER_UPDATE, nNumWant, this );
}

void CDownloadWithTorrent::SendCompleted()
{
	if ( ! Network.IsConnected() || ! Settings.BitTorrent.EnableToday )
		return;

	// Return if there is no tracker
	if ( ! m_pTorrent.HasTracker() )
		return;

	// Create and run tracker request
	TrackerRequests.Request( static_cast< CDownload* >( this ), BTE_TRACKER_COMPLETED, 0, this );
}

void CDownloadWithTorrent::SendStopped()
{
	if ( ! Network.IsConnected() || ! Settings.BitTorrent.EnableToday )
		return;

	// Return if there is no tracker
	if ( ! m_pTorrent.HasTracker() )
		return;

	// Update download to indicate it has been stopped
	m_bTorrentStarted = m_bTorrentRequested = FALSE;

	// Create and run tracker request
	TrackerRequests.Request( static_cast< CDownload* >( this ), BTE_TRACKER_STOPPED );
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent tracker event handler

void CDownloadWithTorrent::OnTrackerEvent(bool bSuccess, LPCTSTR pszReason, LPCTSTR pszTip, CBTTrackerRequest* pEvent)
{
	ASSUME_LOCK( Transfers.m_pSection );

	DWORD tNow = GetTickCount();

	if ( bSuccess )
	{
		// Success! Reset error conditions
		if ( pEvent->GetEvent() == BTE_TRACKER_STARTED ||
			 pEvent->GetEvent() == BTE_TRACKER_UPDATE ||
			 pEvent->GetEvent() == BTE_TRACKER_COMPLETED )
		{
			if ( ! m_bTorrentRequested )
				// Abort if the download has been paused after the request was sent but before a reply was received
				return;

			m_bTorrentStarted = TRUE;
			m_tTorrentTracker = tNow + pEvent->GetInterval() * 1000;
		}
		m_bTorrentTrackerError = FALSE;
		m_sTorrentTrackerError.Empty();
		m_pTorrent.SetTrackerSucceeded( tNow );

		// Get new sources
		for ( POSITION pos = pEvent->GetSources(); pos; )
		{
			const CBTTrackerSource& pSource = pEvent->GetNextSource( pos );
			AddSourceBT( pSource.m_pPeerID, &pSource.m_pAddress.sin_addr, ntohs( pSource.m_pAddress.sin_port ) );
		}

		// Lock on this tracker if we were searching for one
		if ( m_pTorrent.GetTrackerMode() == CBTInfo::tMultiFinding )
		{
			theApp.Message( MSG_DEBUG , _T("[BT] Locked onto tracker %s"), (LPCTSTR)m_pTorrent.GetTrackerAddress() );
			m_pTorrent.SetTrackerMode( CBTInfo::tMultiFound );
		}
	}
	else
	{
		// There was a problem with the tracker
		m_bTorrentTrackerError = TRUE;
		m_sTorrentTrackerError = ( pszTip ? pszTip : pszReason );
		m_pTorrent.OnTrackerFailure();
		m_bTorrentRequested = m_bTorrentStarted = FALSE;
		m_tTorrentTracker = tNow + GetRetryTime();
		m_pTorrent.SetTrackerRetry( m_tTorrentTracker );

		if ( m_pTorrent.IsMultiTracker() )
		{
			// Try the next one
			m_pTorrent.SetTrackerNext( tNow );

			// Set retry time
			m_tTorrentTracker = m_pTorrent.GetTrackerNextTry();

			// Load the error message string
			CString strFormat, strErrorMessage;
			LoadString( strFormat, IDS_BT_TRACKER_MULTI );
			strErrorMessage.Format( strFormat, m_pTorrent.GetTrackerIndex() + 1, m_pTorrent.GetTrackerCount() );
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
	for ( POSITION posSource = GetIterator(); posSource ; )
	{
		pSource = GetNext( posSource );

		if ( pSource->m_nProtocol == PROTOCOL_BT &&
			validAndEqual( pSource->m_oGUID, tmp ) ) break;

		pSource = NULL;
	}

	if ( pSource == NULL )
	{
		pSource = new CDownloadSource( static_cast< CDownload* >( this ),
			pClient->m_oGUID, &pClient->m_pHost.sin_addr, htons( pClient->m_pHost.sin_port ) );
		pSource->m_bPushOnly = !(pClient->m_bInitiated);

		if ( ! AddSourceInternal( pSource ) ) return NULL;
	}

	if ( ! pSource->IsIdle() )
		// A download transfer already exists
		return NULL;

	return (CDownloadTransferBT*)pSource->CreateTransfer( pClient );
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
	if ( ! m_pTorrentBlock )
		return NULL;

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
			theApp.Message( MSG_DEBUG, _T("Attempting to push-start a BitTorrent upload for %s"), (LPCTSTR)m_pTorrent.m_sName );
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
					pTransfer->m_pClient->m_pUploadTransfer->m_bInterested &&
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
	if ( m_bTorrentRequested )
	{
		ASSERT( IsTorrent() );

		if ( GetTickCount() > m_tTorrentSources + 15000 )
		{
			SendUpdate( min( Settings.BitTorrent.DownloadConnections * 4ul, 100ul ) );
			return TRUE;
		}
	}

	return FALSE;
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
	return float( m_pTorrent.m_nTotalUpload * 10000 /
		( m_pTorrent.m_nTotalDownload ? m_pTorrent.m_nTotalDownload :
		m_pTorrent.m_nSize ) ) / 100.0f;
}

//////////////////////////////////////////////////////////////////////
// CDownloadWithTorrent Check if it's okay to start a new download transfer

BOOL CDownloadWithTorrent::CheckTorrentRatio() const
{
	if ( ! IsTorrent() ) return TRUE;									// Not a torrent

	if ( m_pTorrent.m_nStartDownloads == CBTInfo::dtAlways ) return TRUE;// Torrent is set to download as needed

	if ( m_pTorrent.m_nStartDownloads == CBTInfo::dtWhenRatio )			// Torrent is set to download only when ratio is okay
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
