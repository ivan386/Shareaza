//
// ChatSession.cpp
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
#include "ChatCore.h"
#include "ChatSession.h"
#include "GProfile.h"
#include "G2Packet.h"
#include "Network.h"
#include "Buffer.h"
#include "XML.h"

#include "ChatWindows.h"
#include "CtrlChatFrame.h"
#include "CtrlPrivateChatFrame.h"
// #include "CtrlPublicChatFrame.h"

typedef unsigned int UINT_PTR;
#include <mmsystem.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CChatSession construction

CChatSession::CChatSession(CChatFrame* pFrame)
{
	m_bGUID		= FALSE;
	
	m_nState	= cssNull;
	m_bG2		= FALSE;
	m_bOld		= FALSE;
	m_bMustPush	= FALSE;
	m_tPushed	= 0;
	m_pProfile	= NULL;
	
	m_pWndPrivate	= NULL;
	m_pWndPublic	= NULL;
	
	if ( pFrame != NULL )
	{
		m_pWndPrivate = ( pFrame->IsKindOf( RUNTIME_CLASS(CPrivateChatFrame) ) )
			? reinterpret_cast<CPrivateChatFrame*>(pFrame) : NULL;
//		m_pWndPublic = ( pFrame->IsKindOf( RUNTIME_CLASS(CPublicChatFrame) ) )
//			? reinterpret_cast<CPublicChatFrame*>(pFrame) : NULL;
	}
	
	ChatCore.Add( this );
}

CChatSession::~CChatSession()
{
	ASSERT( m_hSocket == INVALID_SOCKET );
	
	if ( m_pProfile != NULL ) delete m_pProfile;
	
	ChatCore.Remove( this );
}

//////////////////////////////////////////////////////////////////////
// CChatSession setup

void CChatSession::Setup(GGUID* pGUID, SOCKADDR_IN* pHost, BOOL bMustPush)
{
	CSingleLock pLock( &ChatCore.m_pSection, TRUE );
	
	if ( pGUID != NULL )
	{
		m_bGUID = TRUE;
		m_pGUID = *pGUID;
	}
	
	m_pHost		= *pHost;
	m_bMustPush	= bMustPush;
	
	m_sUserNick = inet_ntoa( m_pHost.sin_addr );
}

//////////////////////////////////////////////////////////////////////
// CChatSession connect

BOOL CChatSession::Connect()
{
	CSingleLock pLock( &ChatCore.m_pSection, TRUE );
	
	if ( m_nState > cssNull ) return FALSE;
	
	if ( m_bMustPush )
	{
		if ( ! SendPush( FALSE ) )
		{
			StatusMessage( 1, IDS_CHAT_CANT_PUSH, (LPCTSTR)CString( inet_ntoa( m_pHost.sin_addr ) ) );
			return FALSE;
		}
	}
	else
	{
		if ( CConnection::ConnectTo( &m_pHost ) )
		{
			ChatCore.Add( this );
			StatusMessage( 0, IDS_CHAT_CONNECTING_TO, (LPCTSTR)m_sAddress );
		}
		else
		{
			StatusMessage( 1, IDS_CHAT_CANT_CONNECT, (LPCTSTR)m_sAddress );
			return FALSE;
		}
	}
	
	m_nState = cssConnecting;
	
	return TRUE;
}

TRISTATE CChatSession::GetConnectedState() const
{
	if ( m_nState == cssNull ) return TS_FALSE;
	if ( m_nState == cssActive ) return TS_TRUE;
	return TS_UNKNOWN;
}

//////////////////////////////////////////////////////////////////////
// CChatSession attach to (accept) an incoming connection

void CChatSession::AttachTo(CConnection* pConnection)
{
	CConnection::AttachTo( pConnection );
	
	m_nState = cssRequest1;
	ChatCore.Add( this );
}

//////////////////////////////////////////////////////////////////////
// CChatSession push functionality

BOOL CChatSession::SendPush(BOOL bAutomatic)
{
	if ( ! m_bGUID ) return FALSE;
	
	if ( Network.SendPush( &m_pGUID, 0 ) )
	{
		m_nState = cssNull;
		CConnection::Close();
		
		m_tConnected = m_tPushed = GetTickCount();
		StatusMessage( 0, IDS_CHAT_PUSH_SENT, (LPCTSTR)CString( inet_ntoa( m_pHost.sin_addr ) ) );
		
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CChatSession::OnPush(GGUID* pGUID, CConnection* pConnection)
{
	if ( m_tPushed == 0 ) return FALSE;
	if ( ! m_bGUID || m_pGUID != *pGUID ) return FALSE;
	if ( m_nState > cssConnecting ) return FALSE;
	
	if ( m_nState > cssNull ) Close();
	
	CConnection::AttachTo( pConnection );
	
	StatusMessage( 0, IDS_CHAT_PUSH_DONE, (LPCTSTR)m_sAddress );
	ChatCore.Add( this );
	OnConnected();
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CChatSession close

void CChatSession::Close()
{
	CSingleLock pLock( &ChatCore.m_pSection );
	pLock.Lock( 250 );
	
	if ( m_nState != cssNull )
	{
		m_nState = cssNull;
		StatusMessage( 0, IDS_CHAT_CLOSED );
		StatusMessage( 0, 0 );
	}
	
	CConnection::Close();
	
	if ( m_pWndPrivate == NULL && m_pWndPublic == NULL ) delete this;
}

//////////////////////////////////////////////////////////////////////
// CChatSession connection handler

BOOL CChatSession::OnConnected()
{
	CConnection::OnConnected();
	
	StatusMessage( 0, IDS_CHAT_CONNECTED );
	
	m_pOutput->Print( "CHAT CONNECT/0.2\r\n" );
	m_pOutput->Print( "Accept: text/plain,application/x-gnutella2\r\n" );
	m_pOutput->Print( "User-Agent: " );
	m_pOutput->Print( Settings.SmartAgent( Settings.General.UserAgent ) );
	m_pOutput->Print( "\r\n" );
	if ( m_bInitiated ) SendMyAddress();
	m_pOutput->Print( "\r\n" );
	
	m_nState		= cssRequest2;
	m_tConnected	= GetTickCount();
	
	OnWrite();
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CChatSession disconnection handler

void CChatSession::OnDropped(BOOL bError)
{
	if ( m_hSocket == INVALID_SOCKET ) return;
	
	if ( m_nState == cssConnecting )
	{
		StatusMessage( 1, IDS_CHAT_CANT_CONNECT, (LPCTSTR)m_sAddress );
		if ( m_tPushed == 0 && SendPush( TRUE ) ) return;
	}
	else
	{
		StatusMessage( 1, IDS_CHAT_DROPPED, (LPCTSTR)m_sAddress );
	}
	
	Close();
}

//////////////////////////////////////////////////////////////////////
// CChatSession run handler

BOOL CChatSession::OnRun()
{
	if ( m_nState > cssNull && m_nState < cssActive )
	{
		DWORD nDelay = GetTickCount() - m_tConnected;
		
		if ( nDelay >= ( m_nState == cssConnecting ?
			Settings.Connection.TimeoutConnect : Settings.Connection.TimeoutHandshake ) )
		{
			theApp.Message( MSG_ERROR, IDS_HANDSHAKE_TIMEOUT, (LPCTSTR)m_sAddress );
			Close();
			return FALSE;
		}
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CChatSession read handler

BOOL CChatSession::OnRead()
{
	CConnection::OnRead();
	
	switch ( m_nState )
	{
	case cssRequest1:
	case cssRequest2:
	case cssRequest3:
		return ReadHandshake();
	case cssHeaders1:
	case cssHeaders2:
	case cssHeaders3:
		return ReadHeaders();
	case cssHandshake:
	case cssActive:
		if ( m_bG2 )
			return ReadPackets();
		else
			return ReadText();
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CChatSession handshake processer

BOOL CChatSession::ReadHandshake()
{
	CString strLine;
	
	if ( ! m_pInput->ReadLine( strLine ) ) return TRUE;
	if ( strLine.IsEmpty() ) return TRUE;
	
	theApp.Message( MSG_DEBUG, _T("CHAT HANDSHAKE: %s: %s"),
		(LPCTSTR)m_sAddress, (LPCTSTR)strLine );
	
	m_bOld = strLine.Find( _T("/0.1") ) > 0;
	
	if ( StartsWith( strLine, _T("CHAT CONNECT/") ) && m_nState == cssRequest1 )
	{
		m_nState = cssHeaders1;
		return TRUE;
	}
	else if ( StartsWith( strLine, _T("CHAT/") ) )
	{
		if ( _tcsistr( strLine, _T("200 OK") ) )
		{
			if ( m_nState == cssRequest2 )
			{
				m_nState = cssHeaders2;
				return TRUE;
			}
			else if ( m_nState == cssRequest3 )
			{
				m_nState = cssHeaders3;
				return TRUE;
			}
		}
		else
		{
			// Rejected
		}
	}
	
	StatusMessage( 1, IDS_CHAT_PRIVATE_REFUSED, (LPCTSTR)m_sAddress );
	Close();
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CChatSession header processer

BOOL CChatSession::OnHeaderLine(CString& strHeader, CString& strValue)
{
	theApp.Message( MSG_DEBUG, _T("CHAT HEADER: %s: %s: %s"),
		(LPCTSTR)m_sAddress, (LPCTSTR)strHeader, (LPCTSTR)strValue );
	
	if ( strHeader.CompareNoCase( _T("User-Agent") ) == 0 )
	{
		m_sUserAgent = strValue;
	}
	else if ( strHeader.CompareNoCase( _T("Accept") ) == 0 )
	{
		m_bG2 |= ( strValue.Find( _T("application/x-gnutella2") ) >= 0 );
	}
	else if ( strHeader.CompareNoCase( _T("X-Nickname") ) == 0 )
	{
		m_sUserNick = strValue;
	}
	else if (	strHeader.CompareNoCase( _T("X-My-Address") ) == 0 ||
				strHeader.CompareNoCase( _T("Listen-IP") ) == 0 ||
				strHeader.CompareNoCase( _T("Node") ) == 0 )
	{
		int nColon = strValue.Find( ':' );
		
		if ( ! m_bInitiated && nColon > 0 )
		{
			int nPort = GNUTELLA_DEFAULT_PORT;
			
			if ( _stscanf( strValue.Mid( nColon + 1 ), _T("%lu"), &nPort ) == 1 && nPort != 0 )
			{
				m_pHost.sin_port = htons( nPort );
			}
		}
	}
	
	return TRUE;
}

BOOL CChatSession::OnHeadersComplete()
{
	if ( m_nState != cssHeaders3 )
	{
		m_pOutput->Print( "CHAT/0.2 200 OK\r\n" );
		
		if ( m_bG2 )
		{
			m_pOutput->Print( "Accept: application/x-gnutella2\r\n" );
			m_pOutput->Print( "Content-Type: application/x-gnutella2\r\n" );
		}
		else if ( MyProfile.IsValid() )
		{
			m_pOutput->Print( "X-Nickname: " );
			m_pOutput->Print( MyProfile.GetNick().Left( 255 ) );
			m_pOutput->Print( "\r\n" );
		}
		
		m_pOutput->Print( "User-Agent: " );
		m_pOutput->Print( Settings.SmartAgent( Settings.General.UserAgent ) );
		m_pOutput->Print( "\r\n" );
		m_pOutput->Print( "\r\n" );
		
		OnWrite();
	}
	
	if ( m_nState == cssHeaders1 )
	{
		// Sent second handshake
		m_nState = cssRequest3;
		return TRUE;
	}
	else if ( m_nState == cssHeaders2 )
	{
		// Sent third handshake
		m_nState = cssHandshake;
		return OnEstablished();
	}
	else if ( m_nState == cssHeaders3 )
	{
		// Connected
		m_nState = cssHandshake;
		return OnEstablished();
	}
	
	ASSERT( FALSE );
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CChatSession startup

BOOL CChatSession::OnEstablished()
{
	m_tConnected = GetTickCount();
	
	if ( m_bG2 )
	{
		StatusMessage( 0, IDS_CHAT_HANDSHAKE_G2 );
		Send( CG2Packet::New( G2_PACKET_PROFILE_CHALLENGE ) );
	}
	else
	{
		m_nState = cssActive;
		StatusMessage( 2, IDS_CHAT_HANDSHAKE_G1, m_bOld ? _T("0.1") : _T("0.2") );
		if ( m_pWndPrivate != NULL ) m_pWndPrivate->OnProfileReceived();
		StatusMessage( 0, 0 );
		PostOpenWindow();
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CChatSession text interface

void CChatSession::Print(LPCTSTR pszString)
{
	ASSERT( ! m_bG2 );
	ASSERT( m_nState >= cssHandshake );
	
	m_pOutput->Print( pszString );
	OnWrite();
}

BOOL CChatSession::ReadText()
{
	CString strLine;
	
	while ( m_pInput->ReadLine( strLine ) )
	{
		if ( ! OnText( strLine ) )
		{
			Close();
			return FALSE;
		}
	}
	
	return TRUE;
}

BOOL CChatSession::OnText(const CString& str)
{
	if ( m_pWndPrivate == NULL ) return TRUE;
	
	if ( m_bOld )
	{
		if ( StartsWith( str, _T("\001ACTION ") ) )
		{
			m_pWndPrivate->OnRemoteMessage( TRUE, str.Mid( 8 ) );
		}
		else
		{
			m_pWndPrivate->OnRemoteMessage( FALSE, str );
		}
	}
	else if ( StartsWith( str, _T("MESSAGE ") ) )
	{
		if ( StartsWith( str, _T("MESSAGE \001ACTION ") ) )
		{
			m_pWndPrivate->OnRemoteMessage( TRUE, str.Mid( 16 ) );
		}
		else
		{
			m_pWndPrivate->OnRemoteMessage( FALSE, str.Mid( 8 ) );
		}
	}
	else if ( StartsWith( str, _T("NICK ") ) )
	{
		// New nick is : str.Mid( 5 )
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CChatSession Gnutella2 packet interface

void CChatSession::Send(CG2Packet* pPacket, BOOL bRelease)
{
	ASSERT( m_bG2 );
	ASSERT( pPacket != NULL );
	ASSERT( m_nState >= cssHandshake );
	
	pPacket->ToBuffer( m_pOutput );
	if ( bRelease ) pPacket->Release();
	
	OnWrite();
}

BOOL CChatSession::ReadPackets()
{
	for ( BOOL bSuccess = TRUE ; bSuccess && m_pInput->m_nLength ; )
	{
		BYTE nInput = *( m_pInput->m_pBuffer );
		
		if ( nInput == 0 )
		{
			m_pInput->Remove( 1 );
			continue;
		}
		
		BYTE nLenLen	= ( nInput & 0xC0 ) >> 6;
		BYTE nTypeLen	= ( nInput & 0x38 ) >> 3;
		BYTE nFlags		= ( nInput & 0x07 );
		
		if ( nLenLen == 0 )
		{
			Close();
			return FALSE;
		}
		
		if ( (DWORD)m_pInput->m_nLength < (DWORD)nLenLen + nTypeLen + 2 ) break;
		
		DWORD nLength = 0;
		
		if ( nFlags & G2_FLAG_BIG_ENDIAN )
		{
			BYTE* pLenIn = m_pInput->m_pBuffer + 1;
			
			for ( BYTE nIt = nLenLen ; nIt ; nIt-- )
			{
				nLength <<= 8;
				nLength |= *pLenIn++;
			}
		}
		else
		{
			BYTE* pLenIn	= m_pInput->m_pBuffer + 1;
			BYTE* pLenOut	= (BYTE*)&nLength;
			for ( BYTE nLenCnt = nLenLen ; nLenCnt-- ; ) *pLenOut++ = *pLenIn++;
		}
		
		if ( nLength >= Settings.Gnutella1.MaximumPacket )
		{
			Close();
			return FALSE;
		}
		
		if ( (DWORD)m_pInput->m_nLength < (DWORD)nLength + nLenLen + nTypeLen + 2 ) break;
		
		CG2Packet* pPacket = CG2Packet::New( m_pInput->m_pBuffer );
		
		m_pInput->Remove( nLength + nLenLen + nTypeLen + 2 );
		
		try
		{
			bSuccess = OnPacket( pPacket );
		}
		catch ( CException* pException )
		{
			pException->Delete();
			bSuccess = TRUE;
		}
		
		pPacket->Release();
	}
	
	if ( ! bSuccess ) Close();
	
	return bSuccess;
}

//////////////////////////////////////////////////////////////////////
// CChatSession Gnutella2 packet handlers

BOOL CChatSession::OnPacket(CG2Packet* pPacket)
{
	if ( pPacket->IsType( "CMSG" ) )
	{
		return OnChatMessage( pPacket );
	}
	else if ( pPacket->IsType( G2_PACKET_PROFILE_CHALLENGE ) )
	{
		return OnProfileChallenge( pPacket );
	}
	else if ( pPacket->IsType( G2_PACKET_PROFILE_DELIVERY ) )
	{
		return OnProfileDelivery( pPacket );
	}
	else if ( pPacket->IsType( "CHATREQ" ) )
	{
		return OnChatRequest( pPacket );
	}
	else if ( pPacket->IsType( "CHATANS" ) )
	{
		return OnChatAnswer( pPacket );
	}
	
	return TRUE;
}

BOOL CChatSession::OnProfileChallenge(CG2Packet* pPacket)
{
	if ( ! MyProfile.IsValid() ) return TRUE;
	
	CG2Packet* pProfile = CG2Packet::New( G2_PACKET_PROFILE_DELIVERY, TRUE );
	CString strXML = MyProfile.GetXML( NULL, TRUE )->ToString( TRUE );
	
	pProfile->WritePacket( "XML", pProfile->GetStringLen( strXML ) );
	pProfile->WriteString( strXML, FALSE );
	
	Send( pProfile, TRUE );
	
	return TRUE;
}

BOOL CChatSession::OnProfileDelivery(CG2Packet* pPacket)
{
	if ( ! pPacket->m_bCompound ) return TRUE;
	
	if ( m_pProfile != NULL ) delete m_pProfile;
	m_pProfile = NULL;
	
	CHAR szType[9];
	DWORD nLength;
	
	while ( pPacket->ReadPacket( szType, nLength ) )
	{
		DWORD nOffset = pPacket->m_nPosition + nLength;
		
		if ( strcmp( szType, "XML" ) == 0 )
		{
			CXMLElement* pXML = CXMLElement::FromString( pPacket->ReadString( nLength ), TRUE );
			
			if ( pXML != NULL )
			{
				m_pProfile = new CGProfile();
				
				if ( ! m_pProfile->FromXML( pXML ) || ! m_pProfile->IsValid() )
				{
					delete pXML;
					delete m_pProfile;
					m_pProfile = NULL;
				}
			}
		}
		
		pPacket->m_nPosition = nOffset;
	}
	
	if ( m_pProfile == NULL ) return TRUE;
	
	m_sUserNick = m_pProfile->GetNick();
	
	if ( m_bGUID )
	{
		if ( m_pGUID != m_pProfile->GUID )
		{
			// ERROR: Its someone else !!
			m_pGUID = m_pProfile->GUID;
		}
	}
	else
	{
		m_bGUID = TRUE;
		m_pGUID = m_pProfile->GUID;
	}
	
	if ( m_pWndPrivate != NULL )
	{
		m_pWndPrivate->OnProfileReceived();
		
		CG2Packet* pPacket = CG2Packet::New( "CHATREQ", TRUE );
		pPacket->WritePacket( "USERGUID", 16 );
		pPacket->Write( &m_pGUID, 16 );
		
		Send( pPacket, TRUE );
	}
	
	return TRUE;
}

BOOL CChatSession::OnChatRequest(CG2Packet* pPacket)
{
	if ( ! pPacket->m_bCompound ) return TRUE;
	
	BOOL bGUID = FALSE;
	GGUID pGUID;
	
	CHAR szType[9];
	DWORD nLength;
	
	while ( pPacket->ReadPacket( szType, nLength ) )
	{
		DWORD nOffset = pPacket->m_nPosition + nLength;
		
		if ( strcmp( szType, "USERGUID" ) == 0 && nLength >= 16 )
		{
			pPacket->Read( &pGUID, 16 );
			bGUID = TRUE;
		}
		
		pPacket->m_nPosition = nOffset;
	}
	
	pPacket = CG2Packet::New( "CHATANS", TRUE );
	
	pPacket->WritePacket( "USERGUID", 16 );
	pPacket->Write( &MyProfile.GUID, 16 );
	
	if ( bGUID && pGUID == MyProfile.GUID )
	{
		pPacket->WritePacket( "ACCEPT", 0 );
		PostOpenWindow();
	}
	else
	{
		pPacket->WritePacket( "DENY", 0 );
	}
	
	Send( pPacket, TRUE );
	
	return TRUE;
}

BOOL CChatSession::OnChatAnswer(CG2Packet* pPacket)
{
	if ( ! pPacket->m_bCompound ) return TRUE;
	
	CHAR szType[9];
	DWORD nLength;
	
	while ( pPacket->ReadPacket( szType, nLength ) )
	{
		DWORD nOffset = pPacket->m_nPosition + nLength;
		
		if ( strcmp( szType, "ACCEPT" ) == 0 )
		{
			m_nState = cssActive;
			StatusMessage( 2, IDS_CHAT_PRIVATE_ONLINE, (LPCTSTR)m_sUserNick );
			StatusMessage( 0, 0 );
			return TRUE;
		}
		else if ( strcmp( szType, "DENY" ) == 0 )
		{
			StatusMessage( 1, IDS_CHAT_PRIVATE_REFUSED, (LPCTSTR)m_sUserNick );
		}
		else if ( strcmp( szType, "AWAY" ) == 0 )
		{
			CString strAway = pPacket->ReadString( nLength );
			StatusMessage( 1, IDS_CHAT_PRIVATE_AWAY, (LPCTSTR)m_sUserNick,
				(LPCTSTR)strAway );
		}
		
		pPacket->m_nPosition = nOffset;
	}
	
	Close();
	
	return FALSE;
}

BOOL CChatSession::OnChatMessage(CG2Packet* pPacket)
{
	if ( ! pPacket->m_bCompound ) return TRUE;
	
	BOOL bAction = FALSE;
	CString strBody;
	CHAR szType[9];
	DWORD nLength;
	
	while ( pPacket->ReadPacket( szType, nLength ) )
	{
		DWORD nOffset = pPacket->m_nPosition + nLength;
		
		if ( strcmp( szType, "BODY" ) == 0 )
		{
			strBody = pPacket->ReadString( nLength );
		}
		else if ( strcmp( szType, "ACT" ) == 0 )
		{
			bAction = TRUE;
		}
		
		pPacket->m_nPosition = nOffset;
	}
	
	if ( strBody.IsEmpty() ) return TRUE;
	
	if ( m_pWndPrivate != NULL ) m_pWndPrivate->OnRemoteMessage( bAction, strBody );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CChatSession message interface

BOOL CChatSession::SendPrivateMessage(BOOL bAction, LPCTSTR pszText)
{
	CSingleLock pLock( &ChatCore.m_pSection, TRUE );
	
	if ( m_nState != cssActive ) return FALSE;
	
	if ( m_bG2 )
	{
		CG2Packet* pPacket = CG2Packet::New( "CMSG", TRUE );
		
		if ( bAction ) pPacket->WritePacket( "ACT", 0 );
		
		pPacket->WritePacket( "BODY", pPacket->GetStringLen( pszText ) );
		pPacket->WriteString( pszText, FALSE );
		
		Send( pPacket, TRUE );
	}
	else
	{
		CString str;
		
		if ( ! m_bOld ) str += _T("MESSAGE ");
		if ( bAction ) str += _T("\001ACTION ");
		str += pszText;
		str += _T("\r\n");
		
		Print( str );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CChatSession status message

void CChatSession::StatusMessage(int nFlags, UINT nID, ...)
{
	CString strFormat;
	va_list pArgs;
	
	if ( nID )
		LoadString( strFormat, nID );
	else
		strFormat = _T("-");
	
	va_start( pArgs, nID );
	
	if ( strFormat.Find( _T("%1") ) >= 0 )
	{
		LPTSTR lpszTemp;
		if ( ::FormatMessage( FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ALLOCATE_BUFFER,
			strFormat, 0, 0, (LPTSTR)&lpszTemp, 0, &pArgs ) != 0 && lpszTemp != NULL )
		{
			if ( m_pWndPrivate != NULL ) m_pWndPrivate->OnStatusMessage( nFlags, lpszTemp );
			LocalFree( lpszTemp );
		}
	}
	else
	{
		TCHAR szMessageBuffer[1024];
		_vstprintf( szMessageBuffer, strFormat, pArgs );
		if ( m_pWndPrivate != NULL ) m_pWndPrivate->OnStatusMessage( nFlags, szMessageBuffer );
	}
	
	va_end( pArgs );
}

//////////////////////////////////////////////////////////////////////
// CChatSession chat window interface

void CChatSession::PostOpenWindow()
{
	if ( m_pWndPrivate != NULL || m_pWndPublic != NULL ) return;
	
	if ( CWnd* pWnd = (CWnd*)theApp.SafeMainWnd() )
	{
		pWnd->PostMessage( WM_OPENCHAT, (WPARAM)this );
	}
}

void CChatSession::OnOpenWindow()
{
	ASSERT( m_pWndPrivate == NULL && m_pWndPublic == NULL );
	
	if ( m_bGUID )
	{
		m_pWndPrivate = ChatWindows.FindPrivate( &m_pGUID );
	}
	else
	{
		m_pWndPrivate = ChatWindows.FindPrivate( &m_pHost.sin_addr );
	}
	
	if ( m_pWndPrivate == NULL )
	{
		m_pWndPrivate = new CPrivateChatFrame();
	}
	
	if ( ! m_pWndPrivate->Accept( this ) )
	{
		m_pWndPrivate = new CPrivateChatFrame();
		m_pWndPrivate->Accept( this );
	}
	
	m_pWndPrivate->OnProfileReceived();
	
	StatusMessage( 2, IDS_CHAT_PRIVATE_ONLINE, (LPCTSTR)m_sUserNick );
	StatusMessage( 0, 0 );
	
	PlaySound( _T("RAZA_IncomingChat"), NULL, SND_APPLICATION|SND_ALIAS|SND_ASYNC );
	
	m_nState = cssActive;
	
	// Hack to open it
	
	CWnd* pParent = m_pWndPrivate->GetParent();
	if ( pParent->IsIconic() ) pParent->ShowWindow( SW_SHOWNORMAL );
	pParent->BringWindowToTop();
	pParent->SetForegroundWindow();
}

void CChatSession::OnCloseWindow()
{
	m_pWndPrivate = NULL;
	m_pWndPublic = NULL;
	
	Close();
}

