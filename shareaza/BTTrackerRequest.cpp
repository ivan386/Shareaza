//
// BTTrackerRequest.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2011.
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
#include "BENode.h"
#include "BTPacket.h"
#include "BTTrackerRequest.h"
#include "Transfers.h"
#include "Datagrams.h"
#include "Downloads.h"
#include "Download.h"
#include "ShareazaURL.h"

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
	if ( ! TrackerRequests.Lookup( nTransactionID ) )
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

void CBTTrackerPacket::ToBuffer(CBuffer* pBuffer, bool /*bTCP*/) const
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

void CBTTrackerPacket::SmartDump(const SOCKADDR_IN* pAddress, BOOL bUDP, BOOL bOutgoing, DWORD_PTR nNeighbourUnique) const
{
	CPacket::SmartDump( pAddress, bUDP, bOutgoing, nNeighbourUnique );
}

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
		sType.Format( _T("%d"), m_nAction );
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

	CSingleLock oLock( &Transfers.m_pSection, FALSE );
	if ( oLock.Lock( 100 ) )
	{
		if ( CBTTrackerRequest* pRequest = TrackerRequests.Lookup( m_nTransactionID ) )
		{
			switch ( m_nAction )
			{
			case BTA_TRACKER_CONNECT:
				// Assume UDP is stable
				Datagrams.SetStable();

				m_nConnectionID = ReadInt64();
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
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CBTTrackerRequest construction

CBTTrackerRequest::CBTTrackerRequest(CDownloadWithTorrent* pDownload, DWORD nEvent, DWORD nNumWant)
	: m_bHTTP			( false )
	, m_pDownload		( pDownload )
	, m_pCancel			( FALSE, TRUE )
	, m_nEvent			( nEvent )
	, m_nNumWant		( nNumWant )
	, m_nConnectionID	( 0 )
	, m_nTransactionID	( 0 )
{
	ASSERT( nEvent <= BTE_TRACKER_STOPPED );
	ASSERT( m_pDownload != NULL );
	ASSERT( m_pDownload->IsTorrent() ); 

	ZeroMemory( &m_pHost, sizeof( m_pHost ) );
	m_pHost.sin_family = AF_INET;

	// Generate unique transaction ID
	m_nTransactionID = TrackerRequests.Add( this );

	m_pDownload->Add( this );

	CString strAddress = m_pDownload->m_pTorrent.GetTrackerAddress();

	if ( StartsWith( strAddress, _PT("http:") ) )
	{
		// Create the basic URL (http://wiki.theory.org/BitTorrentSpecification#Tracker_HTTP.2FHTTPS_Protocol)

		m_bHTTP = true;

		QWORD nLeft = m_pDownload->GetVolumeRemaining();
		if ( nLeft == SIZE_UNKNOWN )
			nLeft = 0;

		m_sURL.Format( _T("%s%cinfo_hash=%s&peer_id=%s&port=%u&uploaded=%I64u&downloaded=%I64u&left=%I64u&compact=1"),
			(LPCTSTR)strAddress.TrimRight( _T('&') ),
			( ( strAddress.Find( _T('?') ) != -1 ) ? _T('&') : _T('?') ),
			(LPCTSTR)Escape( m_pDownload->m_oBTH ),
			(LPCTSTR)Escape( m_pDownload->m_pPeerID ),
			Network.m_pHost.sin_port ? (DWORD)htons( Network.m_pHost.sin_port ) : Settings.Connection.InPort,
			m_pDownload->m_nTorrentUploaded,
			m_pDownload->m_nTorrentDownloaded,
			nLeft );

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

		// If the TrackerKey is true and we have a valid key, then use it.
		if ( ( m_pDownload->m_sKey.GetLength() > 4 ) && ( Settings.BitTorrent.TrackerKey ) )
		{	
			ASSERT ( m_pDownload->m_sKey.GetLength() < 20 );		//Key too long

			m_sURL += _T("&key=");
			m_sURL += m_pDownload->m_sKey;
		}	
	}
	else if ( StartsWith( strAddress, _PT("udp:") ) )
	{
		// UDP Tracker Protocol for BitTorrent (http://bittorrent.org/beps/bep_0015.html)

		m_sURL = strAddress.Mid( 4 ).Trim( _T("/") ); // Skip 'udp://'
		int nSlash = m_sURL.Find( _T('/') );
		if ( nSlash != -1 )
		{
			m_sURL = m_sURL.Left( nSlash );
		}
	}

	theApp.Message( MSG_DEBUG | MSG_FACILITY_OUTGOING,
		_T("[BT] Sending BitTorrent tracker announce: %s"), m_sURL );

	BeginThread( "BT Tracker Request", ThreadStart, this );
}

CBTTrackerRequest::~CBTTrackerRequest()
{
	TrackerRequests.Remove( m_nTransactionID );

	m_pDownload->Remove( this );
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

/////////////////////////////////////////////////////////////////////////////
// CBTTrackerRequest run

UINT CBTTrackerRequest::ThreadStart(LPVOID pParam)
{
	( (CBTTrackerRequest*)pParam )->OnRun();

	return 0;
}

void CBTTrackerRequest::OnRun()
{
	if ( m_bHTTP )
	{
		ProcessHTTP();
	}
	else
	{
		ProcessUDP();
	}

	delete this;
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
	m_pRequest->SetUserAgent( Settings.SmartAgent() );

	bool bSuccess = m_pRequest->Execute( false );

	// Check if the request return needs to be parsed
	if ( m_nEvent == BTE_TRACKER_STOPPED )
		return;

	CSingleLock oLock( &Transfers.m_pSection, FALSE );
	while ( ! oLock.Lock( 100 ) ) { if ( WaitForSingleObject( m_pCancel, 0 ) != WAIT_TIMEOUT ) return; }

	// Abort if the download has been paused after the request was sent but
	// before a reply was received
	if ( ! m_pDownload->m_bTorrentRequested )
		return;

	if ( ! bSuccess )
	{
		m_pDownload->OnTrackerEvent( false, LoadString( IDS_BT_TRACKER_DOWN ) );
		return;
	}

	if ( ! m_pRequest->InflateResponse() )
	{
		m_pDownload->OnTrackerEvent( false, LoadString( IDS_BT_TRACK_PARSE_ERROR ) );
		return;
	}

	const CBuffer* pBuffer = m_pRequest->GetResponseBuffer();

	if ( pBuffer == NULL )
	{
		m_pDownload->OnTrackerEvent( false, LoadString( IDS_BT_TRACKER_DOWN ) );
		return;
	}

	if ( pBuffer->m_pBuffer == NULL )
	{
		m_pDownload->OnTrackerEvent( false, LoadString(IDS_BT_TRACK_PARSE_ERROR ) );
		return;
	}

	const CBENode* pRoot = CBENode::Decode( pBuffer );

	if ( pRoot && pRoot->IsType( CBENode::beDict ) )
	{
		Process( pRoot );
	}
	else if ( pRoot && pRoot->IsType( CBENode::beString ) )
	{
		CString strError;
		strError.Format( LoadString( IDS_BT_TRACK_ERROR ), m_pDownload->GetDisplayName(), pRoot->GetString() );
		m_pDownload->OnTrackerEvent( false, strError, pRoot->GetString() );
	}
	else
	{
		m_pDownload->OnTrackerEvent( false, LoadString( IDS_BT_TRACK_PARSE_ERROR ) );

		CString strData( (const char*)pBuffer->m_pBuffer, pBuffer->m_nLength );
		theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING,
			_T("[BT] Recieved BitTorrent tracker response: %s"), strData.Trim() );
	}

	delete pRoot;
}

void CBTTrackerRequest::Process(const CBENode* pRoot)
{
	theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING,
		_T("[BT] Recieved BitTorrent tracker response: %s"), pRoot->Encode() );

	// Check for failure
	if ( const CBENode* pError = pRoot->GetNode( BT_DICT_FAILURE ) )
	{
		CString strError;
		strError.Format( LoadString( IDS_BT_TRACK_ERROR ), m_pDownload->GetDisplayName(), pError->GetString() );
		m_pDownload->OnTrackerEvent( false, strError, pError->GetString() );
		return;
	}

	// Get the interval (next tracker contact)
	const CBENode* pInterval = pRoot->GetNode( BT_DICT_INTERVAL );
	if ( ! pInterval->IsType( CBENode::beInt ) )
	{
		m_pDownload->OnTrackerEvent( false, LoadString( IDS_BT_TRACK_PARSE_ERROR ) );
		return;
	}
	QWORD nInterval = pInterval->GetInt();

	// Verify interval is valid
	nInterval = max( nInterval, 60ull * 2ull );
	nInterval = min( nInterval, 60ull * 60ull );

	// nInterval is now between 120 - 3600 so this cast is safe
	m_pDownload->m_tTorrentTracker = static_cast< DWORD >( nInterval ) * 1000ul;
	m_pDownload->m_tTorrentTracker += GetTickCount();
	m_pDownload->m_bTorrentStarted = TRUE;

	// Get list of peers
	const CBENode* pPeers = pRoot->GetNode( BT_DICT_PEERS );
	int nCount = 0;

	if ( pPeers->IsType( CBENode::beList ) )
	{
		for ( int nPeer = 0 ; nPeer < pPeers->GetCount() ; nPeer++ )
		{
			const CBENode* pPeer = pPeers->GetNode( nPeer );
			if ( ! pPeer->IsType( CBENode::beDict ) )
				continue;

			const CBENode* pID = pPeer->GetNode( BT_DICT_PEER_ID );

			const CBENode* pIP = pPeer->GetNode( BT_DICT_PEER_IP );
			if ( ! pIP->IsType( CBENode::beString ) )
				continue;

			const CBENode* pPort = pPeer->GetNode( BT_DICT_PEER_PORT );
			if ( ! pPort->IsType( CBENode::beInt ) )
				continue;

			SOCKADDR_IN saPeer = {};
			if ( ! Network.Resolve( pIP->GetString(), (int)pPort->GetInt(), &saPeer ) )
				continue;

			if ( pID->IsType( CBENode::beString ) && pID->m_nValue == Hashes::BtGuid::byteCount )
			{
				Hashes::BtGuid tmp( *static_cast< Hashes::BtGuid::RawStorage* >(
					pID->m_pValue ) );
				if ( validAndUnequal( tmp, m_pDownload->m_pPeerID ) )
					nCount += m_pDownload->AddSourceBT( tmp,
						&saPeer.sin_addr, ntohs( saPeer.sin_port ) );
			}
			else
			{
				// Self IP is checked later although if bound to 0.0.0.0
				// this will add self too
				nCount += m_pDownload->AddSourceBT( Hashes::BtGuid(),
					&saPeer.sin_addr, ntohs( saPeer.sin_port ) );
			}
		}
	}
	else if ( pPeers->IsType( CBENode::beString ) )
	{
		if ( 0 == ( pPeers->m_nValue % 6 ) )
		{
			const BYTE* pPointer = (const BYTE*)pPeers->m_pValue;

			for ( int nPeer = (int)pPeers->m_nValue / 6 ; nPeer > 0 ; nPeer --, pPointer += 6 )
			{
				const IN_ADDR* pAddress = (const IN_ADDR*)pPointer;
				WORD nPort = *(const WORD*)( pPointer + 4 );

				nCount += m_pDownload->AddSourceBT( Hashes::BtGuid(), pAddress, ntohs( nPort ) );
			}
		}
	}

	// Okay, clear any errors and continue
	CString strError;
	strError.Format( LoadString( IDS_BT_TRACK_SUCCESS ), m_pDownload->GetDisplayName(), nCount );
	m_pDownload->OnTrackerEvent( true, strError );
}

void CBTTrackerRequest::ProcessUDP()
{
	if ( ! Network.Resolve( m_sURL, INTERNET_DEFAULT_HTTP_PORT, &m_pHost, TRUE ) )
	{
		CSingleLock oLock( &Transfers.m_pSection, FALSE );
		while ( ! oLock.Lock( 100 ) ) { if ( WaitForSingleObject( m_pCancel, 0 ) != WAIT_TIMEOUT ) return; }
		m_pDownload->OnTrackerEvent( false, LoadString( IDS_BT_TRACKER_DOWN ) );
		return;
	}

	// Send connect packet
	bool bSuccess = false;
	if ( Datagrams.Send( &m_pHost, CBTTrackerPacket::New( BTA_TRACKER_CONNECT, m_nTransactionID, bt_connection_magic ) ) )
	{
		// Wait for response
		bSuccess = ( WaitForSingleObject( m_pCancel, 5000 ) == WAIT_OBJECT_0 );
	}

	if ( ! bSuccess )
	{
		// Connecion errors
		CSingleLock oLock( &Transfers.m_pSection, FALSE );
		while ( ! oLock.Lock( 100 ) ) { if ( WaitForSingleObject( m_pCancel, 0 ) != WAIT_TIMEOUT ) return; }
		m_pDownload->OnTrackerEvent( false, LoadString( IDS_BT_TRACKER_DOWN ) );
	}
}

BOOL CBTTrackerRequest::OnConnect(CBTTrackerPacket* pPacket)
{
	m_nConnectionID = pPacket->m_nConnectionID;

	if ( CBTTrackerPacket* pResponse = CBTTrackerPacket::New( BTA_TRACKER_ANNOUNCE, m_nTransactionID, m_nConnectionID ) )
	{
		if ( m_nEvent == BTE_TRACKER_SCRAPE )
		{
			pResponse->Write( m_pDownload->m_oBTH );
		}
		else
		{
			QWORD nLeft = m_pDownload->GetVolumeRemaining();
			if ( nLeft == SIZE_UNKNOWN )
				nLeft = 0;

			WORD nPort = Network.m_pHost.sin_port ? (DWORD)htons( Network.m_pHost.sin_port ) : Settings.Connection.InPort;

			pResponse->Write( m_pDownload->m_oBTH );
			pResponse->Write( m_pDownload->m_pPeerID );
			pResponse->WriteInt64( m_pDownload->m_nTorrentDownloaded );
			pResponse->WriteInt64( nLeft );
			pResponse->WriteInt64( m_pDownload->m_nTorrentUploaded );
			pResponse->WriteLongBE( m_nEvent );
			pResponse->WriteLongBE( 0 );			// Owr IP (same)
			pResponse->WriteLongBE( 0 );			// Key
			pResponse->WriteLongBE( m_nNumWant ? m_nNumWant : (DWORD)-1 );
			pResponse->WriteShortBE( nPort );
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
			m_pDownload->OnTrackerEvent( false, LoadString( IDS_BT_TRACKER_DOWN ) );
			Cancel();
		}
	}
	else
	{
		// Out of memory
		m_pDownload->OnTrackerEvent( false, LoadString( IDS_BT_TRACKER_DOWN ) );
		Cancel();
	}

	return TRUE;
}

BOOL CBTTrackerRequest::OnAnnounce(CBTTrackerPacket* pPacket)
{
	int nCount = 0;

	DWORD nInterval = pPacket->ReadLongBE();
	nInterval = min( max( nInterval, 60ul * 2ul ), 60ul * 60ul );
	m_pDownload->m_tTorrentTracker = GetTickCount() + nInterval * 1000;
	m_pDownload->m_bTorrentStarted = TRUE;

	m_nLeechers = pPacket->ReadLongBE();
	m_nSeeders = pPacket->ReadLongBE();

	while ( pPacket->GetRemaining() >= sizeof( bt_peer_t ) )
	{
		DWORD nAddress = pPacket->ReadLongLE();
		WORD nPort = pPacket->ReadShortBE();
		nCount += m_pDownload->AddSourceBT( Hashes::BtGuid(), (IN_ADDR*)&nAddress, nPort );
	}

	CString strError;
	strError.Format( LoadString( IDS_BT_TRACK_SUCCESS ), m_pDownload->GetDisplayName(), nCount );
	m_pDownload->OnTrackerEvent( true, strError );
	Cancel();

	return TRUE;
}

BOOL CBTTrackerRequest::OnScrape(CBTTrackerPacket* pPacket)
{
	// Assume only one scrape
	if ( pPacket->GetRemaining() >= sizeof( bt_scrape_t ) )
	{
		m_nSeeders = pPacket->ReadLongBE();
		m_nDownloaded = pPacket->ReadLongBE();
		m_nLeechers = pPacket->ReadLongBE();
	}

	CString strError;
	strError.Format( LoadString( IDS_BT_TRACK_SUCCESS ), m_pDownload->GetDisplayName(), 0 );
	m_pDownload->OnTrackerEvent( true, strError );
	Cancel();

	return TRUE;
}

BOOL CBTTrackerRequest::OnError(CBTTrackerPacket* pPacket)
{
	CString strErrorMsg = pPacket->ReadStringUTF8();

	CString strError;
	strError.Format( LoadString( IDS_BT_TRACK_ERROR ), m_pDownload->GetDisplayName(), strErrorMsg );
	m_pDownload->OnTrackerEvent( false, strError, strErrorMsg );
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
}

DWORD CBTTrackerRequests::Add(CBTTrackerRequest* pRequest)
{
	CQuickLock oLock( m_pSection );

	for (;;)
	{
		DWORD nTransactionID = GetRandomNum( 0ui32, _UI32_MAX );
		if ( m_pTrackerRequests.PLookup( nTransactionID ) == NULL )
		{
			m_pTrackerRequests.SetAt( nTransactionID, pRequest );
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
	m_pTrackerRequests.Lookup( nTransactionID, pRequest );
	return pRequest;
}
