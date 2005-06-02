//
// BTTrackerRequest.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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
#include "Network.h"
#include "BENode.h"
#include "BTClients.h"
#include "BTTrackerRequest.h"
#include "Transfers.h"
#include "Downloads.h"
#include "Download.h"
#include "SHA.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CBTTrackerRequest, CWinThread)
	//{{AFX_MSG_MAP(CBTTrackerRequest)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBTTrackerRequest construction

CBTTrackerRequest::CBTTrackerRequest(CDownload* pDownload, LPCTSTR pszVerb, BOOL bProcess, WORD nNumWant)
{
	ASSERT( pDownload != NULL );
	ASSERT( pDownload->m_pTorrent.IsAvailable() ); 
	
	m_bAutoDelete	= TRUE;
	m_pDownload		= pDownload;
	m_bProcess		= bProcess;
	
	CString strURL;
	// Create the basic URL
	strURL.Format( _T("%s?info_hash=%s&peer_id=%s&port=%i&uploaded=%I64i&downloaded=%I64i&left=%I64i&compact=1"),
		(LPCTSTR)pDownload->m_pTorrent.m_sTracker,
		(LPCTSTR)Escape( &pDownload->m_pBTH ),
		(LPCTSTR)Escape( &m_pDownload->m_pPeerID ),
		Network.m_pHost.sin_port ? (int)htons( Network.m_pHost.sin_port ) : (int)Settings.Connection.InPort,
		(QWORD)pDownload->m_nTorrentUploaded,
		(QWORD)pDownload->m_nTorrentDownloaded,
		(QWORD)pDownload->GetVolumeRemaining() );
	
	// If the IP is valid, add it.
	if ( Network.m_pHost.sin_addr.S_un.S_addr != 0 )
	{	
		strURL += _T("&ip=");
		strURL += inet_ntoa( Network.m_pHost.sin_addr );
		// Note: Some trackers ignore this value and take the IP the request came from. (Usually the same)
	}
	
	// If an event was specified, add it.
	if ( pszVerb != NULL )
	{	
		strURL += _T("&event=");
		strURL += pszVerb;
		// Valid events: started, completed, stopped
	}

	// If a (valid) number of peers was specified, request that many.
	if ( nNumWant < 300 )
	{	
		CString strNumWant;
		strNumWant.Format( _T("&numwant=%i"), nNumWant );
		strURL += strNumWant;
		// Note: If this is omitted, trackers usually respond with 50, which is a good default.
		// Generally, it's not worth the bandwidth to send this. However, if we have plenty of 
		// sources, then ask for a lower number. (EG 5, just to ensure some sources are fresh)
	}	

	// If the TrackerKey is true and we have a valid key, then use it.
	if ( ( pDownload->m_sKey.GetLength() > 4 ) && ( Settings.BitTorrent.TrackerKey ) )
	{	
		ASSERT ( pDownload->m_sKey.GetLength() < 20 );		//Key too long

		strURL += _T("&key=");
		strURL += pDownload->m_sKey;
	}	
	
	m_pRequest.SetURL( strURL );
	//m_pRequest.AddHeader( _T("Accept"), _T("application/x-bittorrent") ); // This causes problems with some trackers
	m_pRequest.AddHeader( _T("Accept-Encoding"), _T("gzip") );
	
	if ( Settings.BitTorrent.StandardPeerID )
	{
		CString strUserAgent = Settings.SmartAgent( Settings.General.UserAgent );
		m_pRequest.SetUserAgent( strUserAgent );
	}

	//m_pRequest.AddHeader( _T("Cache-Control"), _T("no-cache") ); // Shouldn't be needed

	//theApp.Message( MSG_DEBUG, _T("Sending announce") );

	BTClients.Add( this );
	CreateThread();
}

CBTTrackerRequest::~CBTTrackerRequest()
{
	BTClients.Remove( this );
}

/////////////////////////////////////////////////////////////////////////////
// CBTTrackerRequest actions

void CBTTrackerRequest::SendStarted(CDownloadBase* pDownload, WORD nNumWant)
{
	if ( ((CDownload*)pDownload)->m_pTorrent.m_sTracker.IsEmpty() ) return;
	new CBTTrackerRequest( (CDownload*)pDownload, _T("started"), TRUE, nNumWant );
}

void CBTTrackerRequest::SendUpdate(CDownloadBase* pDownload, WORD nNumWant)
{
	if ( ((CDownload*)pDownload)->m_pTorrent.m_sTracker.IsEmpty() ) return;
	new CBTTrackerRequest( (CDownload*)pDownload,  NULL , TRUE, nNumWant );
}

void CBTTrackerRequest::SendCompleted(CDownloadBase* pDownload)
{
	if ( ((CDownload*)pDownload)->m_pTorrent.m_sTracker.IsEmpty() ) return;
	new CBTTrackerRequest( (CDownload*)pDownload, _T("completed"), TRUE, 0 );
}

void CBTTrackerRequest::SendStopped(CDownloadBase* pDownload)
{
	if ( ((CDownload*)pDownload)->m_pTorrent.m_sTracker.IsEmpty() ) return;
	new CBTTrackerRequest( (CDownload*)pDownload, _T("stopped"), FALSE, 0xFFFF );
}

/////////////////////////////////////////////////////////////////////////////
// CBTTrackerRequest escaper

CString CBTTrackerRequest::Escape(SHA1* pSHA1)
{
	static LPCTSTR pszHex = _T("0123456789ABCDEF");
	
	CString str;
	LPTSTR psz = str.GetBuffer( sizeof(SHA1) * 3 + 1 );
	
	for ( int nByte = 0 ; nByte < sizeof(SHA1) ; nByte++ )
	{
		int nValue = (int)(unsigned char)pSHA1->n[ nByte ];
		
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

BOOL CBTTrackerRequest::InitInstance() 
{
	return TRUE;
}

int CBTTrackerRequest::Run() 
{	
	if ( m_bProcess )
	{
		Process( m_pRequest.Execute( FALSE ) );
	}
	else
	{
		m_pRequest.Execute( FALSE );
	}
	
	return 0;
}

void CBTTrackerRequest::Process(BOOL bRequest)
{
	CSingleLock pLock( &Transfers.m_pSection );
	
	if ( ! pLock.Lock( 250 ) ) return;
	if ( ! Downloads.Check( m_pDownload ) ) return;
	if ( ! m_pDownload->m_bTorrentRequested ) return;
	
	if ( ! bRequest )
	{
		m_pDownload->OnTrackerEvent( FALSE );
		return;
	}
	
	if ( ! m_pRequest.InflateResponse() )
	{
		theApp.Message( MSG_ERROR, IDS_BT_TRACK_PARSE_ERROR );
		return;
	}
	
	CBuffer* pBuffer = m_pRequest.GetResponseBuffer();
	if ( pBuffer == NULL ) return;

	CBENode* pRoot = CBENode::Decode( pBuffer );
	
	if ( pRoot->IsType( CBENode::beDict ) )
	{
		Process( pRoot );
	}
	else if ( pRoot->IsType( CBENode::beString ) )
	{
		CString str = pRoot->GetString();
		theApp.Message( MSG_ERROR, IDS_BT_TRACK_ERROR,
			(LPCTSTR)m_pDownload->GetDisplayName(), (LPCTSTR)str );
		m_pDownload->OnTrackerEvent( FALSE, str );
	}
	else
	{
		theApp.Message( MSG_ERROR, IDS_BT_TRACK_PARSE_ERROR );
		m_pDownload->OnTrackerEvent( FALSE );
	}
	
	if ( pRoot != NULL ) delete pRoot;
}

BOOL CBTTrackerRequest::Process(CBENode* pRoot)
{
	CString strError;

	// Check for failure
	if ( CBENode* pError = pRoot->GetNode( "failure reason" ) )
	{
		strError = pError->GetString();
		theApp.Message( MSG_ERROR, IDS_BT_TRACK_ERROR,
			(LPCTSTR)m_pDownload->GetDisplayName(), (LPCTSTR)strError );
		m_pDownload->OnTrackerEvent( FALSE, strError );
		return FALSE;
	}
	
	// Get the interval (next tracker contact)
	CBENode* pInterval = pRoot->GetNode( "interval" );
	if ( ! pInterval->IsType( CBENode::beInt ) ) 
	{
		LoadString( strError, IDS_BT_TRACK_PARSE_ERROR );
		m_pDownload->OnTrackerEvent( FALSE, strError );
		return FALSE;
	}
	int nInterval = (int)(DWORD)pInterval->GetInt();

	// Verify interval is valid
	nInterval = max( nInterval, 60*2 );
	nInterval = min( nInterval, 60*60 );
	
	m_pDownload->m_tTorrentTracker = GetTickCount() + 1000 * nInterval;
	m_pDownload->m_bTorrentStarted = TRUE;
	
	// Get list of peers
	CBENode* pPeers = pRoot->GetNode( "peers" );
	int nCount = 0;
	
	if ( pPeers->IsType( CBENode::beList )  && ( ( ! m_pDownload->IsMoving() ) || ( Settings.Connection.Firewalled ) ) )
	{
		for ( int nPeer = 0 ; nPeer < pPeers->GetCount() ; nPeer++ )
		{
			CBENode* pPeer = pPeers->GetNode( nPeer );
			if ( ! pPeer->IsType( CBENode::beDict ) ) continue;
			
			CBENode* pID = pPeer->GetNode( "peer id" );
			
			CBENode* pIP = pPeer->GetNode( "ip" );
			if ( ! pIP->IsType( CBENode::beString ) ) continue;
			
			CBENode* pPort = pPeer->GetNode( "port" );
			if ( ! pPort->IsType( CBENode::beInt ) ) continue;
			
			SOCKADDR_IN saPeer;
			if ( ! Network.Resolve( pIP->GetString(), (int)pPort->GetInt(), &saPeer ) ) continue;
			
			theApp.Message( MSG_DEBUG, _T("Tracker: %s:%i"),
				(LPCTSTR)CString( inet_ntoa( saPeer.sin_addr ) ), htons( saPeer.sin_port ) );
			
			if ( pID->IsType( CBENode::beString ) && pID->m_nValue == sizeof(SHA1) )
			{
				nCount += m_pDownload->AddSourceBT( (SHA1*)pID->m_pValue,
					&saPeer.sin_addr, ntohs( saPeer.sin_port ) );
			}
			else
			{
				nCount += m_pDownload->AddSourceBT( NULL,
					&saPeer.sin_addr, ntohs( saPeer.sin_port ) );
			}
		}
	}
	else if ( pPeers->IsType( CBENode::beString ) && ( ( ! m_pDownload->IsMoving() ) || ( Settings.Connection.Firewalled ) ) )
	{
		if ( 0 == ( pPeers->m_nValue % 6 ) )
		{
			BYTE* pPointer = (BYTE*)pPeers->m_pValue;
			
			for ( int nPeer = (int)pPeers->m_nValue / 6 ; nPeer > 0 ; nPeer --, pPointer += 6 )
			{
				IN_ADDR* pAddress = (IN_ADDR*)pPointer;
				WORD nPort = *(WORD*)( pPointer + 4 );
				
				nCount += m_pDownload->AddSourceBT( NULL, pAddress, ntohs( nPort ) );
			}
		}
	}

	// Okay, clear any errors and continue
	m_pDownload->OnTrackerEvent( TRUE );

	theApp.Message( MSG_DEFAULT, IDS_BT_TRACK_SUCCESS,
		(LPCTSTR)m_pDownload->GetDisplayName(), nCount );	
	return TRUE;
}

