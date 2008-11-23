//
// BTTrackerRequest.cpp
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
#include "BENode.h"
#include "BTTrackerRequest.h"
#include "Transfers.h"
#include "Downloads.h"
#include "Download.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CBTTrackerRequest construction

CBTTrackerRequest::CBTTrackerRequest(CDownloadWithTorrent* pDownload, LPCTSTR pszVerb, DWORD nNumWant, bool bProcess) :
	m_pDownload( pDownload ),
	m_bProcess( bProcess )
{
	ASSERT( pDownload != NULL );
	ASSERT( pDownload->IsTorrent() ); 
		
	CString strURL;
	// Create the basic URL
	CString strAddress = pDownload->m_pTorrent.GetTrackerAddress();
	strURL.Format( _T("%s%cinfo_hash=%s&peer_id=%s&port=%i&uploaded=%I64i&downloaded=%I64i&left=%I64i&compact=1"),
		strAddress.TrimRight( _T('&') ),
		( ( strAddress.Find( _T('?') ) != -1 ) ? _T('&') : _T('?') ),
		Escape( pDownload->m_oBTH ),
		Escape( m_pDownload->m_pPeerID ),
		Network.m_pHost.sin_port ? (int)htons( Network.m_pHost.sin_port ) : (int)Settings.Connection.InPort,
		pDownload->m_nTorrentUploaded,
		pDownload->m_nTorrentDownloaded,
		pDownload->GetVolumeRemaining() );
	
	// If an event was specified, add it.
	if ( pszVerb != NULL )
	{	
		// Valid events: started, completed, stopped
		strURL += _T("&event=");
		strURL += pszVerb;

		// If event is 'started' and the IP is valid, add it.
		if ( !_tcscmp( pszVerb, _T("started") ) && Network.m_pHost.sin_addr.s_addr != INADDR_ANY )
		{	
			// Note: Some trackers ignore this value and take the IP the request came from. (Usually the same)
			strURL += _T("&ip=");
			strURL += inet_ntoa( Network.m_pHost.sin_addr );
		}
	}

	// Add the # of peers to request
	CString strNumWant;
	strNumWant.Format( _T("&numwant=%i"), nNumWant );
	strURL += strNumWant;

	// If the TrackerKey is true and we have a valid key, then use it.
	if ( ( pDownload->m_sKey.GetLength() > 4 ) && ( Settings.BitTorrent.TrackerKey ) )
	{	
		ASSERT ( pDownload->m_sKey.GetLength() < 20 );		//Key too long

		strURL += _T("&key=");
		strURL += pDownload->m_sKey;
	}	
	
	m_pRequest.SetURL( strURL );
	m_pRequest.AddHeader( _T("Accept-Encoding"), _T("deflate, gzip") );
	m_pRequest.EnableCookie( false );
	m_pRequest.SetUserAgent( Settings.SmartAgent() );

	theApp.Message( MSG_DEBUG | MSG_FACILITY_OUTGOING,
		_T("[BT] Sending BitTorrent tracker announce: %s"), strURL );

	m_pDownload->Add( this );

	BeginThread( "BT Tracker Request", ThreadStart, this );
}

CBTTrackerRequest::~CBTTrackerRequest()
{
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
	// Check if the request return needs to be parsed
	if ( m_bProcess )
	{
		// Parse the result if there is one
		Process( m_pRequest.Execute( false ) );
	}
	else
	{
		// Don't wait for a result, just send the request
		m_pRequest.Execute( false );
	}

	delete this;
}

void CBTTrackerRequest::Process(bool bRequest)
{
	CString strError;

	// Abort if the download has been paused after the request was sent but
	// before a reply was received
	if ( !m_pDownload->m_bTorrentRequested )
		return;

	if ( !bRequest )
	{
		LoadString( strError, IDS_BT_TRACKER_DOWN );
		m_pDownload->OnTrackerEvent( false, strError );
		return;
	}

	if ( !m_pRequest.InflateResponse() )
	{
		LoadString( strError, IDS_BT_TRACK_PARSE_ERROR );
		m_pDownload->OnTrackerEvent( false, strError );
		return;
	}

	CBuffer* pBuffer = m_pRequest.GetResponseBuffer();

	if ( pBuffer == NULL )
	{
		LoadString( strError, IDS_BT_TRACKER_DOWN );
		m_pDownload->OnTrackerEvent( false, strError );
		return;
	}

	if ( pBuffer->m_pBuffer == NULL )
	{
		LoadString( strError, IDS_BT_TRACK_PARSE_ERROR );
		m_pDownload->OnTrackerEvent( false, strError );
		return;
	}

	CBENode* pRoot = CBENode::Decode( pBuffer );

	if ( pRoot && pRoot->IsType( CBENode::beDict ) )
	{
		theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING,
			_T("[BT] Recieved BitTorrent tracker response: %s"), pRoot->Encode() );
		Process( pRoot );
	}
	else if ( pRoot && pRoot->IsType( CBENode::beString ) )
	{
		CString strErrorFormat;
		LoadString( strErrorFormat, IDS_BT_TRACK_ERROR );
		strError.Format( strErrorFormat, m_pDownload->GetDisplayName(), pRoot->GetString() );
		m_pDownload->OnTrackerEvent( false, strError, pRoot->GetString() );
	}
	else
	{
		LoadString( strError, IDS_BT_TRACK_PARSE_ERROR );
		m_pDownload->OnTrackerEvent( false, strError );

		CString strData( (const char*)pBuffer->m_pBuffer, pBuffer->m_nLength );
		theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING,
			_T("[BT] Recieved BitTorrent tracker response: %s"), strData.Trim() );
	}

	delete pRoot;
}

void CBTTrackerRequest::Process(CBENode* pRoot)
{
	CString strError;

	// Check for failure
	if ( CBENode* pError = pRoot->GetNode( "failure reason" ) )
	{
		CString strErrorFormat;
		LoadString( strErrorFormat, IDS_BT_TRACK_ERROR );
		strError.Format( strErrorFormat, m_pDownload->GetDisplayName(), pError->GetString() );
		m_pDownload->OnTrackerEvent( false, strError, pError->GetString() );
		return;
	}

	// Get the interval (next tracker contact)
	CBENode* pInterval = pRoot->GetNode( "interval" );
	if ( ! pInterval->IsType( CBENode::beInt ) )
	{
		LoadString( strError, IDS_BT_TRACK_PARSE_ERROR );
		m_pDownload->OnTrackerEvent( false, strError );
		return;
	}
	QWORD nInterval = pInterval->GetInt();

	// Verify interval is valid
	nInterval = max( nInterval, 60ull * 2ull );
	nInterval = min( nInterval, 60ull * 60ull );

	{
		CQuickLock oLock( Transfers.m_pSection );

		// nInterval is now between 120 - 3600 so this cast is safe
		m_pDownload->m_tTorrentTracker = static_cast< DWORD >( nInterval ) * 1000ul;
		m_pDownload->m_tTorrentTracker += GetTickCount();
		m_pDownload->m_bTorrentStarted = TRUE;
	}

	// Get list of peers
	CBENode* pPeers = pRoot->GetNode( "peers" );
	int nCount = 0;

	if ( pPeers->IsType( CBENode::beList ) )
	{
		for ( int nPeer = 0 ; nPeer < pPeers->GetCount() ; nPeer++ )
		{
			CBENode* pPeer = pPeers->GetNode( nPeer );
			if ( ! pPeer->IsType( CBENode::beDict ) )
				continue;

			CBENode* pID = pPeer->GetNode( "peer id" );

			CBENode* pIP = pPeer->GetNode( "ip" );
			if ( ! pIP->IsType( CBENode::beString ) )
				continue;

			CBENode* pPort = pPeer->GetNode( "port" );
			if ( ! pPort->IsType( CBENode::beInt ) )
				continue;

			SOCKADDR_IN saPeer;
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
			BYTE* pPointer = (BYTE*)pPeers->m_pValue;

			for ( int nPeer = (int)pPeers->m_nValue / 6 ; nPeer > 0 ; nPeer --, pPointer += 6 )
			{
				IN_ADDR* pAddress = (IN_ADDR*)pPointer;
				WORD nPort = *(WORD*)( pPointer + 4 );

				nCount += m_pDownload->AddSourceBT( Hashes::BtGuid(), pAddress, ntohs( nPort ) );
			}
		}
	}

	// Okay, clear any errors and continue
	CString strErrorFormat;
	LoadString( strErrorFormat, IDS_BT_TRACK_SUCCESS );
	strError.Format( strErrorFormat, m_pDownload->GetDisplayName(), nCount );
	m_pDownload->OnTrackerEvent( true, strError );
}
