//
// BTTrackerRequest.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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
#include "BENode.h"
#include "BTPacket.h"
#include "BTTrackerRequest.h"
#include "Datagrams.h"
#include "Download.h"
#include "Downloads.h"
#include "HttpRequest.h"
#include "Network.h"
#include "Transfers.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static LPCTSTR szEvents[] =
{
	NULL,									// BTE_TRACKER_UPDATE
	_T("completed"),						// BTE_TRACKER_COMPLETED
	_T("started"),							// BTE_TRACKER_STARTED
	_T("stopped")							// BTE_TRACKER_STOPPED
};

static LPCTSTR szEventInfo[] =
{
	_T("update tracker announce"),			// BTE_TRACKER_UPDATE
	_T("completed tracker announce"),		// BTE_TRACKER_COMPLETED
	_T("initial tracker announce"),			// BTE_TRACKER_STARTED
	_T("final tracker announce"),			// BTE_TRACKER_STOPPED
	_T("tracker scrape")					// BTE_TRACKER_SCRAPE
};

static const DWORD nMinResponseSize[] =
{
	sizeof( bt_udp_connecting_response_t ),	// BTA_TRACKER_CONNECT
	sizeof( bt_udp_announcing_response_t ),	// BTA_TRACKER_ANNOUNCE
	sizeof( bt_udp_scraping_request_t ),	// BTA_TRACKER_SCRAPE
	sizeof( bt_udp_error_response_t )		// BTA_TRACKER_ERROR
};

CBTTrackerPacket::CBTTrackerPacketPool CBTTrackerPacket::POOL;

CBTTrackerRequests TrackerRequests;


//////////////////////////////////////////////////////////////////////
// CBTTrackerPacket construction

CBTTrackerPacket::CBTTrackerPacket()
	: CPacket			( PROTOCOL_BT )		// TODO: Make new one
	, m_nAction			( 0 )
	, m_nTransactionID	( 0 )
	, m_nConnectionID	( 0 )
{
}

CBTTrackerPacket::~CBTTrackerPacket()
{
}

void CBTTrackerPacket::Reset()
{
	CPacket::Reset();

	m_nAction			= 0;
	m_nTransactionID	= 0;
	m_nConnectionID		= 0;
}

CBTTrackerPacket* CBTTrackerPacket::New(DWORD nAction, DWORD nTransactionID, QWORD nConnectionID, const BYTE* pBuffer, DWORD nLength)
{
	CBTTrackerPacket* pPacket = (CBTTrackerPacket*)POOL.New();
	if ( pPacket )
	{
		pPacket->m_nAction = nAction;
		pPacket->m_nTransactionID = nTransactionID;
		pPacket->m_nConnectionID = nConnectionID;

		if ( pBuffer && nLength )
		{
			if ( ! pPacket->Write( pBuffer, nLength ) )
			{
				pPacket->Release();
				return NULL;
			}
		}
	}
	return pPacket;
}

CBTTrackerPacket* CBTTrackerPacket::New(const BYTE* pBuffer, DWORD nLength)
{
	ASSERT( pBuffer && nLength );

	DWORD nAction = ntohl( ((DWORD*)pBuffer)[ 0 ] );
	if ( nAction > BTA_TRACKER_ERROR || nLength < nMinResponseSize[ nAction ] )
	{
		// Unknown or too short packet
		return NULL;
	}

	DWORD nTransactionID = ntohl( ((DWORD*)pBuffer)[ 1 ] );
	if ( ! TrackerRequests.Check( nTransactionID ) )
	{
		// Unknown transaction ID
		return NULL;
	}

	CBTTrackerPacket* pPacket = (CBTTrackerPacket*)POOL.New();
	if ( pPacket )
	{
		pPacket->m_nAction = nAction;
		pPacket->m_nTransactionID = nTransactionID;
		if ( pBuffer && nLength )
		{
			if ( ! pPacket->Write( pBuffer + 8, nLength - 8 ) )
			{
				pPacket->Release();
				return NULL;
			}
		}
	}

	return pPacket;
}


//////////////////////////////////////////////////////////////////////
// CBTTrackerPacket serialize

void CBTTrackerPacket::ToBuffer(CBuffer* pBuffer, bool /*bTCP*/)
{
	pBuffer->AddReversed( &m_nConnectionID, sizeof( m_nConnectionID ) );
	pBuffer->AddReversed( &m_nAction, sizeof( m_nAction ) );
	pBuffer->AddReversed( &m_nTransactionID, sizeof( m_nTransactionID ) );

	pBuffer->Add( m_pBuffer, m_nLength );
}

//////////////////////////////////////////////////////////////////////
// CBTTrackerPacket un-serialize

CBTTrackerPacket* CBTTrackerPacket::ReadBuffer(CBuffer* pBuffer)
{
	CBTTrackerPacket* pPacket = CBTTrackerPacket::New( pBuffer->m_pBuffer, pBuffer->m_nLength );
	if ( pPacket )
	{
		pBuffer->Clear();
	}
	return pPacket;
}

//////////////////////////////////////////////////////////////////////
// CBTTrackerPacket debugging

CString CBTTrackerPacket::GetType() const
{
	CString sType;
	switch ( m_nAction )
	{
	case BTA_TRACKER_CONNECT:
		sType = _T("Connect");
		break;

	case BTA_TRACKER_ANNOUNCE:
		sType = _T("Announce");
		break;

	case BTA_TRACKER_SCRAPE:
		sType = _T("Scrape");
		break;

	case BTA_TRACKER_ERROR:
		sType = _T("Error");
		break;

	default:
		sType.Format( _T("%lu"), m_nAction );
	}
	return sType;
}

CString CBTTrackerPacket::ToHex() const
{
	return CPacket::ToHex();
}

CString CBTTrackerPacket::ToASCII() const
{
	CString strHeader;
	if ( m_nConnectionID )
		strHeader.Format( _T("{tid=0x%08x cid=0x%016I64x} "), m_nTransactionID, m_nConnectionID );
	else
		strHeader.Format( _T("{tid=0x%08x} "), m_nTransactionID );
	return strHeader + CPacket::ToASCII();
}

BOOL CBTTrackerPacket::OnPacket(const SOCKADDR_IN* pHost)
{
	SmartDump( pHost, TRUE, FALSE );

	CAutoPtr< CBTTrackerRequest > pRequest( TrackerRequests.Lookup( m_nTransactionID ) );
	if ( pRequest )
	{
		switch ( m_nAction )
		{
		case BTA_TRACKER_CONNECT:
			return pRequest->OnConnect( this );

		case BTA_TRACKER_ANNOUNCE:
			return pRequest->OnAnnounce( this );

		case BTA_TRACKER_SCRAPE:
			return pRequest->OnScrape( this );

		case BTA_TRACKER_ERROR:
			return pRequest->OnError( this );

		default:
			ASSERT( FALSE );
		}
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CBTTrackerRequest construction

CBTTrackerRequest::CBTTrackerRequest(CDownload* pDownload, BTTrackerEvent nEvent, DWORD nNumWant, CTrackerEvent* pOnTrackerEvent)
	: m_dwRef				( 1 )
	, m_bHTTP				( false )
	, m_oBTH				( pDownload->m_oBTH )
	, m_pPeerID				( pDownload->m_pPeerID )
	, m_nTorrentDownloaded	( pDownload->m_nTorrentDownloaded )
	, m_nTorrentUploaded	( pDownload->m_nTorrentUploaded )
	, m_nTorrentLeft		( pDownload->GetVolumeRemaining() )
	, m_pDownload			( pDownload )
	, m_sName				( pDownload->GetDisplayName() )
	, m_sAddress			( pDownload->m_pTorrent.GetTrackerAddress() )
	, m_nEvent				( nEvent )
	, m_nNumWant			( nNumWant )
	, m_nConnectionID		( 0 )
	, m_nTransactionID		( 0 )
	, m_pOnTrackerEvent		( pOnTrackerEvent )
	, m_nComplete			( 0 )
	, m_nDownloaded			( 0 )
	, m_nIncomplete			( 0 )
	, m_nInterval			( 0 )
{
	ASSUME_LOCK( Transfers.m_pSection );
	ASSERT( nEvent <= BTE_TRACKER_SCRAPE );
	ASSERT( pDownload && pDownload->IsTorrent() );

	ZeroMemory( &m_pHost, sizeof( m_pHost ) );
	m_pHost.sin_family = AF_INET;

	if ( Settings.BitTorrent.PeerID.GetLength() >= 6 )
	{
		// uTorrent 1.6.1.0
		for ( int i = 0; i < 6; ++i )
			m_pPeerID[ i + 1 ] = (BYTE)Settings.BitTorrent.PeerID.GetAt( i );
	}

	if ( StartsWith( m_sAddress, _PT("http:") ) )
	{
		// Create the basic URL (http://wiki.theory.org/BitTorrentSpecification#Tracker_HTTP.2FHTTPS_Protocol)

		m_bHTTP = true;

		if ( m_nEvent == BTE_TRACKER_SCRAPE )
		{
			if ( m_sAddress.Replace( _T("/announce"), _T("/scrape") ) == 1 )
			{
				m_sURL.Format( _T("%s%cinfo_hash=%s&peer_id=%s"),
					(LPCTSTR)CString( m_sAddress ).TrimRight( _T('&') ),
					( ( m_sAddress.Find( _T('?') ) != -1 ) ? _T('&') : _T('?') ),
					(LPCTSTR)Escape( m_oBTH ),
					(LPCTSTR)Escape( m_pPeerID ) );
			}
			else
			{
				// Scrape not supported
			}
		}
		else
		{
			m_sURL.Format( _T("%s%cinfo_hash=%s&peer_id=%s&port=%u&uploaded=%I64u&downloaded=%I64u&left=%I64u&compact=1"),
				(LPCTSTR)CString( m_sAddress ).TrimRight( _T('&') ),
				( ( m_sAddress.Find( _T('?') ) != -1 ) ? _T('&') : _T('?') ),
				(LPCTSTR)Escape( m_oBTH ),
				(LPCTSTR)Escape( m_pPeerID ),
				Network.GetPort(),
				m_nTorrentUploaded,
				m_nTorrentDownloaded,
				( m_nTorrentLeft == SIZE_UNKNOWN ) ? 0 : m_nTorrentLeft );

			// If an event was specified, add it.
			if ( m_nEvent != BTE_TRACKER_UPDATE )
			{
				// Valid events: started, completed, stopped
				m_sURL += _T("&event=");
				m_sURL += szEvents[ m_nEvent ];

				// If event is 'started' and the IP is valid, add it.
				if ( m_nEvent == BTE_TRACKER_STARTED && Network.m_pHost.sin_addr.s_addr != INADDR_ANY )
				{
					// Note: Some trackers ignore this value and take the IP the request came from. (Usually the same)
					m_sURL += _T("&ip=");
					m_sURL += inet_ntoa( Network.m_pHost.sin_addr );
				}
			}

			// Add the # of peers to request
			CString strNumWant;
			strNumWant.Format( _T("&numwant=%u"), m_nNumWant );
			m_sURL += strNumWant;
		}

		// If the TrackerKey is true and we have a valid key, then use it.
		if ( ! m_sURL.IsEmpty() && ! pDownload->m_sKey.IsEmpty() && Settings.BitTorrent.TrackerKey )
		{
			m_sURL += _T("&key=");
			m_sURL += pDownload->m_sKey;
		}
	}
	else if ( StartsWith( m_sAddress, _PT("udp:") ) )
	{
		// UDP Tracker Protocol for BitTorrent (http://bittorrent.org/beps/bep_0015.html)

		m_sURL = m_sAddress.Mid( 4 ).Trim( _T("/") ); // Skip 'udp://'
		int nSlash = m_sURL.Find( _T('/') );
		if ( nSlash != -1 )
		{
			m_sURL = m_sURL.Left( nSlash );
		}
	}
	// else Unsupported protocol

	BeginThread( "BT Tracker Request" );
}

CBTTrackerRequest::~CBTTrackerRequest()
{
	ASSERT( m_dwRef == 0 );

	TrackerRequests.Remove( m_nTransactionID );
}

ULONG CBTTrackerRequest::AddRef()
{
	return (ULONG)InterlockedIncrement( &m_dwRef );
}

ULONG CBTTrackerRequest::Release()
{
	ULONG ref_count = (ULONG)InterlockedDecrement( &m_dwRef );
	if ( ref_count )
		return ref_count;
	delete this;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CBTTrackerRequest escaper

CString CBTTrackerRequest::Escape(const Hashes::BtHash& oBTH)
{
	static LPCTSTR pszHex = _T("0123456789ABCDEF");

	CString str;
    LPTSTR psz = str.GetBuffer( Hashes::BtHash::byteCount * 3 + 1 );

	for ( int nByte = 0 ; nByte < Hashes::BtHash::byteCount ; nByte++ )
	{
		int nValue = oBTH[ nByte ];

		if (	( nValue >= '0' && nValue <= '9' ) ||
				( nValue >= 'a' && nValue <= 'z' ) ||
				( nValue >= 'A' && nValue <= 'Z' ) )
		{
			*psz++ = (TCHAR)nValue;
		}
		else
		{
			*psz++ = '%';
			*psz++ = pszHex[ ( nValue >> 4 ) & 15 ];
			*psz++ = pszHex[ nValue & 15 ];
		}
	}

	*psz = 0;
	str.ReleaseBuffer();

	return str;
}

CString CBTTrackerRequest::Escape(const Hashes::BtGuid& oGUID)
{
	static LPCTSTR pszHex = _T("0123456789ABCDEF");

	CString str;
    LPTSTR psz = str.GetBuffer( Hashes::BtGuid::byteCount * 3 + 1 );

	for ( int nByte = 0 ; nByte < Hashes::BtGuid::byteCount ; nByte++ )
	{
		int nValue = oGUID[ nByte ];

		if (	( nValue >= '0' && nValue <= '9' ) ||
				( nValue >= 'a' && nValue <= 'z' ) ||
				( nValue >= 'A' && nValue <= 'Z' ) )
		{
			*psz++ = (TCHAR)nValue;
		}
		else
		{
			*psz++ = '%';
			*psz++ = pszHex[ ( nValue >> 4 ) & 15 ];
			*psz++ = pszHex[ nValue & 15 ];
		}
	}

	*psz = 0;
	str.ReleaseBuffer();

	return str;
}

void CBTTrackerRequest::Cancel()
{
	m_pOnTrackerEvent = NULL; // Disable notification

	Exit();

	if ( m_pRequest )
		m_pRequest->Cancel();
}

/////////////////////////////////////////////////////////////////////////////
// CBTTrackerRequest run

void CBTTrackerRequest::OnRun()
{
	if ( ! m_sURL.IsEmpty() )
	{
		theApp.Message( MSG_DEBUG | MSG_FACILITY_OUTGOING, _T("[BT] Sending BitTorrent %s for \"%s\": %s"), szEventInfo[ m_nEvent ], (LPCTSTR)m_sName, (LPCTSTR)m_sAddress );

		if ( m_bHTTP )
			ProcessHTTP();
		else
			ProcessUDP();
	}
	else
		theApp.Message( MSG_DEBUG, _T("[BT] BitTorrent %s for \"%s\" is not supported: %s"), szEventInfo[ m_nEvent ], (LPCTSTR)m_sName, (LPCTSTR)m_sAddress );

	Release();
}

void CBTTrackerRequest::OnTrackerEvent(bool bSuccess, LPCTSTR pszReason, LPCTSTR pszTip)
{
	theApp.Message( ( bSuccess ? MSG_INFO : MSG_ERROR ), _T("%s \"%s\": %s"), pszReason, (LPCTSTR)m_sName, (LPCTSTR)m_sAddress );

	if ( m_pOnTrackerEvent )
	{
		CSingleLock oLock( &Transfers.m_pSection, FALSE );
		while ( ! oLock.Lock( 100 ) ) { if ( ! IsThreadEnabled() ) return; }
		if ( ! IsThreadEnabled() || ! Downloads.Check( m_pDownload ) || ! m_pOnTrackerEvent ) return;

		m_pOnTrackerEvent->OnTrackerEvent( bSuccess, pszReason, pszTip, this );
	}
}

void CBTTrackerRequest::ProcessHTTP()
{
	m_pRequest.Attach( new CHttpRequest );
	if ( ! m_pRequest )
		// Out of memory
		return;

	if ( ! m_pRequest->SetURL( m_sURL ) )
		// Bad URL or pending request
		return;

	m_pRequest->AddHeader( _T("Accept-Encoding"), _T("deflate, gzip") );
	m_pRequest->EnableCookie( false );

	bool bSuccess = m_pRequest->Execute( false );

	// Check if the request return needs to be parsed
	if ( m_nEvent == BTE_TRACKER_STOPPED )
		return;

	if ( ! bSuccess )
	{
		OnTrackerEvent( false, LoadString( IDS_BT_TRACKER_DOWN ) );
		return;
	}

	if ( ! m_pRequest->InflateResponse() )
	{
		OnTrackerEvent( false, LoadString( IDS_BT_TRACK_PARSE_ERROR ) );
		return;
	}

	const CBuffer* pBuffer = m_pRequest->GetResponseBuffer();

	if ( pBuffer == NULL )
	{
		OnTrackerEvent( false, LoadString( IDS_BT_TRACKER_DOWN ) );
		return;
	}

	if ( pBuffer->m_pBuffer == NULL )
	{
		OnTrackerEvent( false, LoadString( IDS_BT_TRACK_PARSE_ERROR ) );
		return;
	}

	CAutoPtr< const CBENode > pRoot( CBENode::Decode( pBuffer ) );

	if ( pRoot && pRoot->IsType( CBENode::beDict ) )
	{
		Process( pRoot );
	}
	else if ( pRoot && pRoot->IsType( CBENode::beString ) )
	{
		CString strError;
		strError.Format( LoadString( IDS_BT_TRACK_ERROR ), (LPCTSTR)m_sName, (LPCTSTR)pRoot->GetString() );
		OnTrackerEvent( false, strError, pRoot->GetString() );
	}
	else
	{
		CString strData( (const char*)pBuffer->m_pBuffer, pBuffer->m_nLength );
		theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING, _T("[BT] Recieved bad BitTorrent tracker response: %s"), (LPCTSTR)strData.Trim() );

		OnTrackerEvent( false, LoadString( IDS_BT_TRACK_PARSE_ERROR ) );
	}
}

void CBTTrackerRequest::Process(const CBENode* pRoot)
{
	theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING, _T("[BT] Recieved BitTorrent tracker response: %s"), (LPCTSTR)pRoot->Encode() );

	// Check for failure
	if ( const CBENode* pError = pRoot->GetNode( BT_DICT_FAILURE ) )
	{
		CString strError;
		strError.Format( LoadString( IDS_BT_TRACK_ERROR ), (LPCTSTR)m_sName, (LPCTSTR)pError->GetString() );
		OnTrackerEvent( false, strError, pError->GetString() );
		return;
	}

	if ( m_nEvent == BTE_TRACKER_SCRAPE )
	{
		if ( ! pRoot->IsType( CBENode::beDict ) )
		{
			OnTrackerEvent( false, LoadString( IDS_BT_TRACK_PARSE_ERROR ) );
			return;
		}

		const CBENode* pFiles = pRoot->GetNode( BT_DICT_FILES );
		if ( ! pFiles || ! pFiles->IsType( CBENode::beDict ) )
		{
			OnTrackerEvent( false, LoadString( IDS_BT_TRACK_PARSE_ERROR ) );
			return;
		}

		const LPBYTE nKey = &m_oBTH[ 0 ];

		const CBENode* pFile = pFiles->GetNode( nKey, Hashes::BtHash::byteCount );
		if ( ! pFile || ! pFile->IsType( CBENode::beDict ) )
		{
			OnTrackerEvent( false, LoadString( IDS_BT_TRACK_PARSE_ERROR ) );
			return;
		}

		// Since we read QWORDs, make sure we won't get negative values;
		// Some buggy trackers send very huge numbers, so let's leave them as
		// the max int.

		const CBENode* pComplete = pFile->GetNode( BT_DICT_COMPLETE );
		if ( pComplete && pComplete->IsType( CBENode::beInt ) )
		{
			m_nComplete = (DWORD)( pComplete->GetInt() & ~0xFFFF0000 );
		}

		const CBENode* pIncomplete = pFile->GetNode( BT_DICT_INCOMPLETE );
		if ( pIncomplete && pIncomplete->IsType( CBENode::beInt ) )
		{
			m_nIncomplete = (DWORD)( pIncomplete->GetInt() & ~0xFFFF0000 );
		}

		const CBENode* pDownloaded = pFile->GetNode( BT_DICT_DOWNLOADED );
		if ( pDownloaded && pDownloaded->IsType( CBENode::beInt ) )
		{
			m_nDownloaded = (DWORD)( pDownloaded->GetInt() & ~0xFFFF0000 );
		}
	}
	else
	{
		SOCKADDR_IN saPeer = { AF_INET };

		// Get the interval (next tracker contact)
		m_nInterval = 0;
		const CBENode* pInterval = pRoot->GetNode( BT_DICT_INTERVAL );
		if ( pInterval && pInterval->IsType( CBENode::beInt ) )
		{
			m_nInterval = (DWORD)pInterval->GetInt();
		}

		// Get list of peers
		const CBENode* pPeers = pRoot->GetNode( BT_DICT_PEERS );

		if ( pPeers && pPeers->IsType( CBENode::beList ) )
		{
			for ( int nPeer = 0 ; nPeer < pPeers->GetCount() ; nPeer++ )
			{
				const CBENode* pPeer = pPeers->GetNode( nPeer );
				if ( ! pPeer || ! pPeer->IsType( CBENode::beDict ) )
					continue;

				const CBENode* pID = pPeer->GetNode( BT_DICT_PEER_ID );

				const CBENode* pIP = pPeer->GetNode( BT_DICT_PEER_IP );
				if ( ! pIP || ! pIP->IsType( CBENode::beString ) )
					continue;

				const CBENode* pPort = pPeer->GetNode( BT_DICT_PEER_PORT );
				if ( ! pPort || ! pPort->IsType( CBENode::beInt ) )
					continue;

				if ( ! Network.Resolve( pIP->GetString(), (int)pPort->GetInt(), &saPeer ) )
					continue;

				if ( pID && pID->IsType( CBENode::beString ) && pID->m_nValue == Hashes::BtGuid::byteCount )
				{
					Hashes::BtGuid tmp( *static_cast< const Hashes::BtGuid::RawStorage* >( pID->m_pValue ) );
					if ( validAndUnequal( tmp, m_pPeerID ) )
						m_pSources.AddTail( CBTTrackerSource( tmp, saPeer ) );
				}
				else
				{
					m_pSources.AddTail( CBTTrackerSource( Hashes::BtGuid(), saPeer ) );
				}
			}
		}
		else if ( pPeers && pPeers->IsType( CBENode::beString ) )
		{
			if ( 0 == ( pPeers->m_nValue % 6 ) )
			{
				const BYTE* pPointer = (const BYTE*)pPeers->m_pValue;

				for ( int nPeer = (int)pPeers->m_nValue / 6 ; nPeer > 0 ; nPeer --, pPointer += 6 )
				{
					saPeer.sin_addr = *(const IN_ADDR*)pPointer;
					saPeer.sin_port = *(const WORD*)( pPointer + 4 );
					m_pSources.AddTail( CBTTrackerSource( Hashes::BtGuid(), saPeer ) );
				}
			}
		}
	}

	CString strError;
	strError.Format( LoadString( IDS_BT_TRACK_SUCCESS ), (LPCTSTR)m_sName, m_pSources.GetCount() );
	OnTrackerEvent( true, strError );
}

void CBTTrackerRequest::ProcessUDP()
{
	if ( ! Network.Resolve( m_sURL, INTERNET_DEFAULT_HTTP_PORT, &m_pHost, TRUE ) )
	{
		// Bad tracker name
		OnTrackerEvent( false, LoadString( IDS_BT_TRACKER_DOWN ) );
		return;
	}

	// Send connect UDP-packet to tracker
	bool bSuccess = false;
	if ( Datagrams.Send( &m_pHost, CBTTrackerPacket::New( BTA_TRACKER_CONNECT, m_nTransactionID, bt_connection_magic ) ) )
	{
		// Wait for UDP response from tracker
		bSuccess = ! IsThreadEnabled( Settings.Connection.TimeoutConnect ); // Success if Cancel() was called
	}

	if ( ! bSuccess )
	{
		// Time-out
		OnTrackerEvent( false, LoadString( IDS_BT_TRACKER_DOWN ) );
	}
}

BOOL CBTTrackerRequest::OnConnect(CBTTrackerPacket* pPacket)
{
	// Assume UDP is stable
	Datagrams.SetStable();

	m_nConnectionID = pPacket->m_nConnectionID = pPacket->ReadInt64();

	if ( CBTTrackerPacket* pResponse = CBTTrackerPacket::New(
		( ( m_nEvent == BTE_TRACKER_SCRAPE ) ? BTA_TRACKER_SCRAPE : BTA_TRACKER_ANNOUNCE ), m_nTransactionID, m_nConnectionID ) )
	{
		if ( m_nEvent == BTE_TRACKER_SCRAPE )
		{
			pResponse->Write( m_oBTH );
		}
		else
		{
			pResponse->Write( m_oBTH );
			pResponse->Write( m_pPeerID );
			pResponse->WriteInt64( m_nTorrentDownloaded );
			pResponse->WriteInt64( ( m_nTorrentLeft == SIZE_UNKNOWN ) ? 0 : m_nTorrentLeft );
			pResponse->WriteInt64( m_nTorrentUploaded );
			pResponse->WriteLongBE( m_nEvent );		// update = 0; completed = 1; started = 2; stopped = 3
			pResponse->WriteLongBE( 0 );			// Own IP (same)
			pResponse->WriteLongBE( 0 );			// Key
			pResponse->WriteLongBE( m_nNumWant ? m_nNumWant : (DWORD)-1 );
			pResponse->WriteShortBE( Network.GetPort() );
			pResponse->WriteShortBE( 0 );			// Extensions
		}

		if ( Datagrams.Send( &m_pHost, pResponse ) )
		{
			// Ok
			if ( m_nEvent == BTE_TRACKER_STOPPED )
			{
				Cancel();
			}
		}
		else
		{
			// Network error
			OnTrackerEvent( false, LoadString( IDS_BT_TRACKER_DOWN ) );
			Cancel();
		}
	}
	else
	{
		// Out of memory
		OnTrackerEvent( false, LoadString( IDS_BT_TRACKER_DOWN ) );
		Cancel();
	}

	return TRUE;
}

BOOL CBTTrackerRequest::OnAnnounce(CBTTrackerPacket* pPacket)
{
	m_nInterval = pPacket->ReadLongBE();
	m_nIncomplete = pPacket->ReadLongBE();
	m_nComplete = pPacket->ReadLongBE();

	SOCKADDR_IN saPeer = { AF_INET };
	while ( pPacket->GetRemaining() >= sizeof( bt_peer_t ) )
	{
		saPeer.sin_addr.s_addr = pPacket->ReadLongLE();
		saPeer.sin_port = pPacket->ReadShortLE();
		m_pSources.AddTail( CBTTrackerSource( Hashes::BtGuid(), saPeer ) );
	}

	CString strError;
	strError.Format( LoadString( IDS_BT_TRACK_SUCCESS ), (LPCTSTR)m_sName, m_pSources.GetCount() );
	OnTrackerEvent( true, strError );
	Cancel();

	return TRUE;
}

BOOL CBTTrackerRequest::OnScrape(CBTTrackerPacket* pPacket)
{
	// Assume only one scrape
	if ( pPacket->GetRemaining() >= sizeof( bt_scrape_t ) )
	{
		m_nComplete = pPacket->ReadLongBE();
		m_nDownloaded = pPacket->ReadLongBE();
		m_nIncomplete = pPacket->ReadLongBE();
	}

	CString strError;
	strError.Format( LoadString( IDS_BT_TRACK_SUCCESS ), (LPCTSTR)m_sName, 0 );
	OnTrackerEvent( true, strError );
	Cancel();

	return TRUE;
}

BOOL CBTTrackerRequest::OnError(CBTTrackerPacket* pPacket)
{
	CString strErrorMsg = pPacket->ReadStringUTF8();

	CString strError;
	strError.Format( LoadString( IDS_BT_TRACK_ERROR ), (LPCTSTR)m_sName, (LPCTSTR)strErrorMsg );
	OnTrackerEvent( false, strError, strErrorMsg );
	Cancel();

	return TRUE;
}


//////////////////////////////////////////////////////////////////////
// CBTTrackerRequests construction

CBTTrackerRequests::CBTTrackerRequests()
{
}

CBTTrackerRequests::~CBTTrackerRequests()
{
	Clear();
}

void CBTTrackerRequests::Clear()
{
	for ( ;; )
	{
		CAutoPtr< CBTTrackerRequest > pRequest( GetFirst() );
		if ( ! pRequest )
			break;

		pRequest->Cancel();

		pRequest->CloseThread();
	}
}

DWORD CBTTrackerRequests::Request(CDownload* pDownload, BTTrackerEvent nEvent, DWORD nNumWant, CTrackerEvent* pOnTrackerEvent)
{
	CQuickLock oLock( m_pSection );

	CAutoPtr< CBTTrackerRequest > pRequest ( new CBTTrackerRequest( pDownload, nEvent, nNumWant, pOnTrackerEvent ) );
	if ( ! pRequest )
		// Out of memory
		return 0;

	for (;;)
	{
		DWORD nTransactionID = GetRandomNum( 1ui32, _UI32_MAX );
		if ( m_pTrackerRequests.PLookup( nTransactionID ) == NULL )
		{
			pRequest->m_nTransactionID = nTransactionID;
			m_pTrackerRequests.SetAt( nTransactionID, pRequest.Detach() );
			return nTransactionID;
		}
	}
}

void CBTTrackerRequests::Remove(DWORD nTransactionID)
{
	CQuickLock oLock( m_pSection );

	m_pTrackerRequests.RemoveKey( nTransactionID );
}

CBTTrackerRequest* CBTTrackerRequests::Lookup(DWORD nTransactionID) const
{
	CQuickLock oLock( m_pSection );

	CBTTrackerRequest* pRequest = NULL;
	if ( m_pTrackerRequests.Lookup( nTransactionID, pRequest ) )
		pRequest->AddRef();

	return pRequest;
}

CBTTrackerRequest* CBTTrackerRequests::GetFirst() const
{
	CQuickLock oLock( m_pSection );

	CBTTrackerRequest* pRequest = NULL;
	if ( POSITION pos = m_pTrackerRequests.GetStartPosition() )
	{
		DWORD nTransactionID;
		m_pTrackerRequests.GetNextAssoc( pos, nTransactionID, pRequest );
		pRequest->AddRef();
	}

	return pRequest;
}

BOOL CBTTrackerRequests::Check(DWORD nTransactionID) const
{
	CQuickLock oLock( m_pSection );

	return ( m_pTrackerRequests.PLookup( nTransactionID ) != NULL );
}

void CBTTrackerRequests::Cancel(DWORD nTransactionID)
{
	CAutoPtr< CBTTrackerRequest > pRequest( Lookup( nTransactionID ) );
	if ( pRequest )
	{
		pRequest->Cancel();
	}
}
