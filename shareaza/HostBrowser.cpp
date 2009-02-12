//
// HostBrowser.cpp
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
#include "Buffer.h"
#include "G1Packet.h"
#include "G2Packet.h"
#include "GProfile.h"
#include "Neighbours.h"
#include "HostBrowser.h"
#include "Transfers.h"
#include "QueryHit.h"
#include "VendorCache.h"
#include "WndBrowseHost.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CHostBrowser construction

CHostBrowser::CHostBrowser(CBrowseHostWnd* pNotify, IN_ADDR* pAddress, WORD nPort, BOOL bMustPush, const Hashes::Guid& oClientID) :
	m_nState		( hbsNull )
,	m_pNotify		( pNotify )
,	m_pProfile		( NULL )

,	m_bNewBrowse	( FALSE )
,	m_nPort			( nPort )
,	m_oClientID		( oClientID )
,	m_bMustPush		( bMustPush )
,	m_bCanPush		( m_oClientID.isValid() )
,	m_tPushed		( 0ul )
,	m_bConnect		( FALSE )
,	m_nHits			( 0 )
,	m_pVendor		( NULL )
,	m_bCanChat		( FALSE )

,	m_bDeflate		( FALSE )
,	m_nLength		( ~0ul )
,	m_nReceived		( 0ul )
,	m_pBuffer		( NULL )
,	m_pInflate		( NULL )
{
	if ( pAddress )
		m_pAddress = *pAddress;
	else
		m_pAddress.s_addr = INADDR_NONE;
}

CHostBrowser::~CHostBrowser()
{
	Stop();

	if ( m_pProfile ) delete m_pProfile;
}

//////////////////////////////////////////////////////////////////////
// CHostBrowser browse control

BOOL CHostBrowser::Browse()
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );

	if ( IsValid() ) return FALSE;

	m_sAddress = inet_ntoa( m_pAddress );

	if ( m_bMustPush )
	{
		if ( SendPush( FALSE ) )
		{
			theApp.Message( MSG_NOTICE, IDS_BROWSE_PUSHED_TO, m_sAddress );
		}
		else
		{
			theApp.Message( MSG_ERROR, IDS_BROWSE_CANT_PUSH_TO, m_sAddress );
			return FALSE;
		}
	}
	else
	{
		if ( ConnectTo( &m_pAddress, m_nPort ) )
		{
			theApp.Message( MSG_NOTICE, IDS_BROWSE_CONNECTING_TO, m_sAddress );
		}
		else
		{
			theApp.Message( MSG_ERROR, IDS_BROWSE_CANT_CONNECT_TO, m_sAddress );
			return FALSE;
		}
	}

	m_nState	= hbsConnecting;
	m_nHits		= 0;

	if ( m_pProfile != NULL ) delete m_pProfile;
	m_pProfile = NULL;

	return TRUE;
}

void CHostBrowser::Stop(BOOL /*bCompleted*/)
{
	CSingleLock pLock( &Transfers.m_pSection, TRUE );

	if ( IsValid() )
	{
		theApp.Message( MSG_INFO, IDS_BROWSE_CLOSED, m_sAddress );
	}

	CTransfer::Close();

	m_nState	= hbsNull;
	m_tPushed	= 0;

	if ( m_pBuffer )
	{
		delete m_pBuffer;
		m_pBuffer = NULL;
	}

	if ( m_pInflate )
		GetInput()->InflateStreamCleanup( m_pInflate );
}

BOOL CHostBrowser::IsBrowsing() const
{
	return m_nState != hbsNull;
}

float CHostBrowser::GetProgress() const
{
	if ( m_nState != hbsContent || m_nLength == 0ul || m_nLength == ~0ul ) return 0.0f;

	return (float)m_nReceived / (float)m_nLength;
}

//////////////////////////////////////////////////////////////////////
// CHostBrowser event handling

BOOL CHostBrowser::OnConnected()
{
	CTransfer::OnConnected();
	SendRequest();
	return TRUE;
}

BOOL CHostBrowser::OnRead()
{
	if ( ! IsInputExist() || ! IsOutputExist() ) return TRUE;

	CTransfer::OnRead();

	switch ( m_nState )
	{
	case hbsRequesting:
		if ( ! ReadResponseLine() ) return FALSE;
		if ( m_nState != hbsHeaders ) break;

	case hbsHeaders:
		if ( ! ReadHeaders() ) return FALSE;
		if ( m_nState != hbsContent ) break;

	case hbsContent:
		return ReadContent();

	}

	return TRUE;
}

void CHostBrowser::OnDropped()
{
	if ( ! IsValid() ) return;

	if ( m_nState == hbsConnecting )
	{
		theApp.Message( MSG_ERROR, IDS_BROWSE_CANT_CONNECT_TO, m_sAddress );
		if ( ! m_tPushed && SendPush( TRUE ) ) return;
	}
	else
	{
		if ( m_nLength == ~0ul )
		{
			m_nLength = GetInputLength();
			ReadContent();
			return;
		}

		theApp.Message( MSG_ERROR, IDS_BROWSE_DROPPED, m_sAddress );
	}

	Stop();
}

BOOL CHostBrowser::OnRun()
{
	CTransfer::OnRun();

	DWORD nNow = GetTickCount();

	switch ( m_nState )
	{
	case hbsConnecting:
		if ( nNow - m_tConnected > Settings.Connection.TimeoutConnect * 2 )
		{
			OnDropped();
			return FALSE;
		}
		break;
	case hbsRequesting:
	case hbsHeaders:
		if ( nNow - m_tConnected > Settings.Connection.TimeoutHandshake * 3 )
		{
			theApp.Message( MSG_ERROR, IDS_BROWSE_TIMEOUT, m_sAddress );
			Stop();
			return FALSE;
		}
		break;
	case hbsContent:
		if ( nNow - m_mInput.tLast > Settings.Connection.TimeoutTraffic )
		{
			theApp.Message( MSG_ERROR, IDS_BROWSE_TIMEOUT, m_sAddress );
			Stop();
			return FALSE;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CHostBrowser push handling

BOOL CHostBrowser::SendPush(BOOL bMessage)
{
	if ( ! m_bCanPush ) return FALSE;

	if ( Network.SendPush( m_oClientID, 0 ) )
	{
		CTransfer::Close();
		m_tPushed = GetTickCount();

		if ( bMessage )
			theApp.Message( MSG_INFO, IDS_BROWSE_PUSHED_TO, m_sAddress );

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CHostBrowser::OnPush(const Hashes::Guid& oClientID, CConnection* pConnection)
{
	if ( m_tPushed == 0 ) return FALSE;
	if ( IsValid() ) return FALSE;

	if ( !validAndEqual( m_oClientID, oClientID ) ) return FALSE;

	AttachTo( pConnection );

	m_pAddress	= m_pHost.sin_addr;
	m_nPort		= htons( m_pHost.sin_port );

	SendRequest();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CHostBrowser send request

void CHostBrowser::SendRequest()
{
	if ( ! IsValid() ) return;

	if ( m_bNewBrowse )
	{
		Write( _P("GET /gnutella/browse/v1 HTTP/1.1\r\n") );
	}
	else
	{
		if ( Settings.Downloads.RequestHTTP11 )
			Write( _P("GET / HTTP/1.1\r\n") );
		else
			Write( _P("GET / HTTP/1.0\r\n") );
	}

	CString strHeader = Settings.SmartAgent();

	if ( strHeader.GetLength() )
	{
		Write( _P("User-Agent: ") );
		Write( strHeader );
		Write( _P("\r\n") );
	}

	Write( _P("Accept: text/html, application/x-gnutella-packets, application/x-gnutella2\r\n"
						 "Accept-Encoding: deflate\r\n"
						 "Connection: close\r\n") );

	strHeader.Format( _T("Host: %s:%lu\r\n\r\n"), m_sAddress,
		htons( m_pHost.sin_port ) );
	Write( strHeader );

	CTransfer::OnWrite();

	m_nState	= hbsRequesting;
	m_nProtocol	= PROTOCOL_ANY;
	m_bDeflate	= FALSE;
	m_nLength	= ~0ul;
	m_bConnect	= TRUE;

	m_mInput.pLimit = m_mOutput.pLimit = &Settings.Bandwidth.Downloads;

	theApp.Message( MSG_INFO, IDS_BROWSE_SENT_REQUEST, m_sAddress );
}

//////////////////////////////////////////////////////////////////////
// CHostBrowser read response line

BOOL CHostBrowser::ReadResponseLine()
{
	CString strLine, strCode, strMessage;

	if ( ! Read( strLine ) ) return TRUE;
	if ( strLine.IsEmpty() ) return TRUE;

	if ( strLine.GetLength() > 256 * 1024 ) strLine = _T("#LINE_TOO_LONG#");

	if ( strLine.GetLength() >= 12 && strLine.Left( 9 ) == _T("HTTP/1.1 ") )
	{
		strCode		= strLine.Mid( 9, 3 );
		strMessage	= strLine.Mid( 12 );
	}
	else if ( strLine.GetLength() >= 12 && strLine.Left( 9 ) == _T("HTTP/1.0 ") )
	{
		strCode		= strLine.Mid( 9, 3 );
		strMessage	= strLine.Mid( 12 );
	}
	else if ( strLine.GetLength() >= 8 && strLine.Left( 4 ) == _T("HTTP") )
	{
		strCode		= strLine.Mid( 5, 3 );
		strMessage	= strLine.Mid( 8 );
	}
	else
	{
		theApp.Message( MSG_DEBUG, _T("UNKNOWN BROWSE RESPONSE: %s: %s"),
			m_sAddress, strLine );
		theApp.Message( MSG_ERROR, IDS_BROWSE_NOT_HTTP, m_sAddress );
		Stop();
		return FALSE;
	}

	if ( strCode == _T("200") || strCode == _T("206") )
	{
		m_nState = hbsHeaders;
	}
	else
	{
		strMessage.TrimLeft();
		if ( strMessage.GetLength() > 256 ) strMessage = _T("No Message");

		theApp.Message( MSG_ERROR, IDS_BROWSE_HTTPCODE, m_sAddress, strCode,
			strMessage );

		Stop();
		return FALSE;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CHostBrowser read headers

BOOL CHostBrowser::OnHeaderLine(CString& strHeader, CString& strValue)
{
	if ( ! CTransfer::OnHeaderLine( strHeader, strValue ) )
		return FALSE;

	if ( strHeader.CompareNoCase( _T("Server") ) == 0 )
	{
		m_pVendor = VendorCache.LookupByName( strValue );
		m_sServer = strValue;
		if ( m_sServer.GetLength() > 64 ) m_sServer = m_sServer.Left( 64 );
	}
	else if ( strHeader.CompareNoCase( _T("Content-Type") ) == 0 )
	{
		if ( strValue.CompareNoCase( _T("application/x-gnutella-packets") ) == 0 )
			m_nProtocol = PROTOCOL_G1;
		else if ( strValue.CompareNoCase( _T("application/x-gnutella2") ) == 0 )
			m_nProtocol = PROTOCOL_G2;
		else if ( strValue.CompareNoCase( _T("application/x-shareaza") ) == 0 )
			m_nProtocol = PROTOCOL_G2;
		else if ( strValue.Left(9).CompareNoCase( _T("text/html") ) == 0 )
			m_nProtocol = PROTOCOL_NULL;
	}
	else if ( strHeader.CompareNoCase( _T("Content-Encoding") ) == 0 )
	{
		m_bDeflate = strValue.CompareNoCase( _T("deflate") ) == 0;
	}
	else if ( strHeader.CompareNoCase( _T("Content-Length") ) == 0 )
	{
		_stscanf( strValue, _T("%lu"), &m_nLength );
	}

	return TRUE;
}

BOOL CHostBrowser::OnHeadersComplete()
{
	if ( m_nProtocol == PROTOCOL_ANY || m_nLength == 0 )
	{
		theApp.Message( MSG_ERROR, IDS_BROWSE_BAD_RESPONSE, m_sAddress );
		Stop();
		return FALSE;
	}

	m_nState		= hbsContent;
	m_nReceived		= 0ul;
	m_pBuffer		= new CBuffer();
	m_mInput.tLast	= GetTickCount();

	switch ( m_nProtocol )
	{
	case PROTOCOL_NULL:
		theApp.Message( MSG_INFO, IDS_BROWSE_DOWNLOADING_FROM,
			m_sAddress, _T("HTML") );
		break;
	case PROTOCOL_G1:
		theApp.Message( MSG_INFO, IDS_BROWSE_DOWNLOADING_FROM,
			m_sAddress, _T("Gnutella-1") );
		break;
	case PROTOCOL_G2:
		theApp.Message( MSG_INFO, IDS_BROWSE_DOWNLOADING_FROM,
			m_sAddress, _T("Gnutella-2") );
		break;
	default:
		theApp.Message( MSG_ERROR,
			_T("CHostBrowser::OnHeadersComplete(): Unknown Browse Protocol") );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CHostBrowser read content

BOOL CHostBrowser::ReadContent()
{
	if ( m_nReceived < m_nLength )
	{
		CLockedBuffer pInput( GetInput() );

		DWORD nVolume = min( m_nLength - m_nReceived, pInput->m_nLength );
		m_nReceived += nVolume;

		if ( m_bDeflate )
		{
			// Try to decompress the stream
			if( !pInput->InflateStreamTo( *m_pBuffer, m_pInflate ) )
			{
				Stop();			// Clean up
				return FALSE;	// Report failure
			}
		}
		else
		{
			ASSERT( m_pBuffer );
			m_pBuffer->AddBuffer( pInput, nVolume );
		}
	}

	switch ( m_nProtocol )
	{
	case PROTOCOL_NULL:
		if ( ! StreamHTML() ) return FALSE;
		break;
	case PROTOCOL_G1:
		if ( ! StreamPacketsG1() ) return FALSE;
		break;
	case PROTOCOL_G2:
		if ( ! StreamPacketsG2() ) return FALSE;
		break;
	default:
		theApp.Message( MSG_ERROR,
			_T("CHostBrowser::ReadContent(): Unknown Browse Protocol") );
	}

	if ( m_nReceived < m_nLength ) return TRUE;

	Stop( TRUE );

	if ( m_pProfile->IsValid() && m_pNotify != NULL ) m_pNotify->OnProfileReceived();

	theApp.Message( MSG_NOTICE, IDS_BROWSE_FINISHED, m_sAddress, m_nHits );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CHostBrowser packet streaming

BOOL CHostBrowser::StreamPacketsG1()
{
	BOOL bSuccess = TRUE;
	for ( ; bSuccess ; )
	{
		GNUTELLAPACKET* pPacket = (GNUTELLAPACKET*)m_pBuffer->m_pBuffer;
		if ( m_pBuffer->m_nLength < sizeof(*pPacket) ) break;

		DWORD nLength = sizeof(*pPacket) + pPacket->m_nLength;

		if ( pPacket->m_nLength < 0 || nLength >= (DWORD)Settings.Gnutella1.MaximumPacket * 8 )
		{
			theApp.Message( MSG_ERROR, IDS_BROWSE_PACKET_ERROR, m_sAddress );
			Stop();
			return FALSE;
		}

		if ( m_pBuffer->m_nLength < nLength ) break;

		CG1Packet* pPacketObject = CG1Packet::New( pPacket );

		try
		{
			bSuccess = OnPacket( pPacketObject );
		}
		catch ( CException* pException )
		{
			pException->Delete();
		}

		pPacketObject->Release();

		m_pBuffer->Remove( nLength );
	}

	if ( ! bSuccess ) Stop();

	return bSuccess;
}

BOOL CHostBrowser::StreamPacketsG2()
{
	while ( CG2Packet* pPacket = CG2Packet::ReadBuffer( m_pBuffer ) )
	{
		BOOL bSuccess = FALSE;

		try
		{
			bSuccess = OnPacket( pPacket );
		}
		catch ( CException* pException )
		{
			pException->Delete();
		}

		pPacket->Release();

		if ( ! bSuccess )
		{
			Stop();
			return FALSE;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CHostBrowser packet interpreter

BOOL CHostBrowser::OnPacket(CG1Packet* pPacket)
{
	if ( pPacket->m_nType != G1_PACKET_HIT || pPacket->m_nLength <= 27 )
		return TRUE;

	CQueryHit* pHits = CQueryHit::FromG1Packet( pPacket );

	if ( pHits == NULL )
	{
		theApp.Message( MSG_ERROR, IDS_BROWSE_PACKET_ERROR, m_sAddress );
		return FALSE;
	}

	m_bCanPush	= TRUE;
	m_oClientID	= pHits->m_oClientID;

	for ( CQueryHit* pCount = pHits ; pCount ; pCount = pCount->m_pNext ) m_nHits++;

	if ( ! m_bCanChat && pHits->m_bChat ) m_bCanChat = TRUE;

	if ( m_pNotify != NULL )
		m_pNotify->OnBrowseHits( pHits );

	Network.OnQueryHits( pHits );

	return TRUE;
}

BOOL CHostBrowser::OnPacket(CG2Packet* pPacket)
{
	if ( pPacket->IsType( G2_PACKET_HIT ) )
	{
		CQueryHit* pHits = CQueryHit::FromG2Packet( pPacket );

		if ( pHits == NULL )
		{
			theApp.Message( MSG_ERROR, IDS_BROWSE_PACKET_ERROR, m_sAddress );
			return FALSE;
		}

		m_bCanPush	= TRUE;
		m_oClientID	= pHits->m_oClientID;

		for ( CQueryHit* pCount = pHits ; pCount ; pCount = pCount->m_pNext )
		{
			m_nHits++;
		}

		if ( ! m_bCanChat && pHits->m_bChat )
		{
			m_bCanChat = TRUE;
			if ( m_pNotify && m_pProfile != NULL ) m_pNotify->OnProfileReceived();
		}

		if ( m_pNotify != NULL )
			m_pNotify->OnBrowseHits( pHits );

		Network.OnQueryHits( pHits );
	}
	else if ( pPacket->IsType( G2_PACKET_PHYSICAL_FOLDER ) )
	{
		if ( m_pNotify != NULL ) m_pNotify->OnPhysicalTree( pPacket );
	}
	else if ( pPacket->IsType( G2_PACKET_VIRTUAL_FOLDER ) )
	{
		if ( m_pNotify != NULL ) m_pNotify->OnVirtualTree( pPacket );
	}
	else if ( pPacket->IsType( G2_PACKET_PROFILE_DELIVERY ) )
	{
		OnProfilePacket( pPacket );

		if ( m_pProfile != NULL && m_pNotify != NULL )
		{
			m_pNotify->OnProfileReceived();
		}
	}
	else if ( pPacket->IsType( G2_PACKET_PROFILE_AVATAR ) )
	{
		if ( m_pNotify != NULL ) m_pNotify->OnHeadPacket( pPacket );
	}

	return TRUE;
}

void CHostBrowser::OnProfilePacket(CG2Packet* pPacket)
{
	G2_PACKET nType;
	DWORD nLength;

	while ( pPacket->ReadPacket( nType, nLength ) )
	{
		DWORD nOffset = pPacket->m_nPosition + nLength;

		if ( nType == G2_PACKET_XML )
		{
			CXMLElement* pXML = CXMLElement::FromString( pPacket->ReadString( nLength ), TRUE );

			if ( pXML != NULL )
			{
				if ( m_pProfile == NULL ) m_pProfile = new CGProfile();
				if ( ! m_pProfile->FromXML( pXML ) ) delete pXML;
				if ( m_pProfile != NULL && ! m_pProfile->IsValid() )
				{
					delete m_pProfile;
					m_pProfile = NULL;
				}
			}
		}

		pPacket->m_nPosition = nOffset;
	}
}

//////////////////////////////////////////////////////////////////////
// CHostBrowser HTML streaming

BOOL CHostBrowser::StreamHTML()
{
	CString strLine;

	CQueryHit* pHits = NULL;

	while ( m_pBuffer->ReadLine( strLine ) )
	{
		int nPosHTTP = strLine.Find( _T("http://") );

		while ( nPosHTTP >= 0 && strLine.Find( _T("/get/") ) > nPosHTTP )
		{
			CString strURI = strLine.Mid( nPosHTTP ).SpanExcluding( _T("?&\"'<>") );
			CString strName;
			DWORD nSize = 0;

			int nPosSize = strLine.Find( _T("<TD NOWRAP>") );

			if ( nPosSize >= 0 && nPosSize < nPosHTTP )
			{
				CString strSize = strLine.Mid( nPosSize + 11 ).SpanExcluding( _T("</") );
				float nFloat = 0;

				if ( _stscanf( strSize, _T("%f"), &nFloat ) == 1 && nFloat > 0 )
				{
					if ( strSize.Find( _T(" GB") ) >= 0 )
						nFloat *= 1024*1024*1024;
					else if ( strSize.Find( _T(" MB") ) >= 0 )
						nFloat *= 1024*1024;
					else if ( strSize.Find( _T(" KB") ) >= 0 )
						nFloat *= 1024;

					nSize = (DWORD)nFloat;
				}
			}

			strLine = strLine.Mid( nPosHTTP + strURI.GetLength() );

			int nPosName = strLine.Find( _T(">") );

			if ( nPosName >= 0 )
			{
				strName = strLine.Mid( nPosName + 1 ).SpanExcluding( _T("<>") );
			}

			if ( strName.IsEmpty() && ( nPosName = strURI.ReverseFind( '/' ) ) > 0 )
			{
				strName = URLDecode( strURI.Mid( nPosName + 1 ) );
			}

			CQueryHit* pHit = new CQueryHit( PROTOCOL_NULL );

			pHit->m_pAddress	= m_pHost.sin_addr;
			pHit->m_nPort		= htons( m_pHost.sin_port );
			pHit->m_pVendor		= m_pVendor ? m_pVendor : VendorCache.m_pNull;
			pHit->m_bPush		= ( m_tPushed ) ? TRI_TRUE : TRI_FALSE;
			pHit->m_bBrowseHost	= TRUE;
			pHit->m_nSize		= nSize;
			pHit->m_bSize		= TRUE;
			pHit->m_sName		= strName;
			pHit->m_sURL		= strURI;

			if ( m_bCanPush ) pHit->m_oClientID = m_oClientID;

			pHit->Resolve();

			pHit->m_pNext = pHits;
			pHits = pHit;

			m_nHits ++;

			nPosHTTP = strLine.Find( _T("http://") );
		}
	}

	if ( pHits != NULL )
	{
		if ( m_pNotify != NULL )
			m_pNotify->OnBrowseHits( pHits );

		Network.OnQueryHits( pHits );
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CHostBrowser serialize

void CHostBrowser::Serialize(CArchive& ar)
{
	BOOL bProfilePresent = FALSE;

	if ( ar.IsStoring() )
	{
		ar << m_bNewBrowse;
		ar << m_pAddress.S_un.S_addr;
		ar << m_nPort;
		ar << m_bMustPush;
		ar << m_bCanPush;
		SerializeOut( ar, m_oPushID );
		SerializeOut( ar, m_oClientID );
		ar << m_nHits;
		ar << m_bCanChat;
		ar << m_sServer;
		ar << m_nProtocol;
		ar << m_nLength;
		ar << m_nReceived;

		bProfilePresent = ( m_pProfile != NULL );
		ar << bProfilePresent;
	}
	else
	{
		Stop();

		ar >> m_bNewBrowse;
		ar >> m_pAddress.S_un.S_addr;
		ar >> m_nPort;
		ar >> m_bMustPush;
		ar >> m_bCanPush;
		SerializeIn( ar, m_oPushID, 31 );
		SerializeIn( ar, m_oClientID, 31 );
		ar >> m_nHits;
		ar >> m_bCanChat;
		ar >> m_sServer;
		ar >> m_nProtocol;
		ar >> m_nLength;
		ar >> m_nReceived;

		m_pVendor = VendorCache.LookupByName( m_sServer );

		ar >> bProfilePresent;
		if ( m_pProfile ) delete m_pProfile;
		m_pProfile = new CGProfile();
	}
	if ( m_pProfile && bProfilePresent )
		m_pProfile->Serialize( ar );
}
