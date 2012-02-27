//
// ChatWindows.cpp
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
#include "Buffer.h"
#include "ChatWindows.h"
#include "EDClient.h"
#include "EDClients.h"
#include "GProfile.h"
#include "Neighbours.h"
#include "Network.h"
#include "Transfers.h"
#include "WndChat.h"
#include "WndPrivateChat.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CChatWindows ChatWindows;

//////////////////////////////////////////////////////////////////////
// CChatWindows construction

CChatWindows::CChatWindows()
{
}

CChatWindows::~CChatWindows()
{
}

//////////////////////////////////////////////////////////////////////
// CChatWindows list access

POSITION CChatWindows::GetIterator() const
{
	return m_pList.GetHeadPosition();
}

CChatWnd* CChatWindows::GetNext(POSITION& pos) const
{
	return m_pList.GetNext( pos );
}

//////////////////////////////////////////////////////////////////////
// CChatWindows private chat windows

CPrivateChatWnd* CChatWindows::FindPrivate(const Hashes::Guid& oGUID, bool bLive) const
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CPrivateChatWnd* pFrame = static_cast<CPrivateChatWnd*>( GetNext( pos ) );

		if ( pFrame->IsKindOf( RUNTIME_CLASS(CPrivateChatWnd) ) )
		{
			if ( pFrame->Find( oGUID, bLive ) )
				return pFrame;
		}
	}

	return NULL;
}

CPrivateChatWnd* CChatWindows::FindPrivate(const SOCKADDR_IN* pAddress) const
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CPrivateChatWnd* pFrame = static_cast<CPrivateChatWnd*>( GetNext( pos ) );

		if ( pFrame->IsKindOf( RUNTIME_CLASS(CPrivateChatWnd) ) )
		{
			if ( pFrame->Find( pAddress ) )
				return pFrame;
		}
	}

	return NULL;
}

CPrivateChatWnd* CChatWindows::FindED2KFrame(const SOCKADDR_IN* pAddress) const
{
	// For High ID clients
	CString strHighID;
	strHighID.Format( _T("%s:%hu"), (LPCTSTR)CString( inet_ntoa( pAddress->sin_addr ) ), ntohs( pAddress->sin_port ) );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CPrivateChatWnd* pFrame = static_cast<CPrivateChatWnd*>( GetNext( pos ) );

		if ( pFrame->IsKindOf( RUNTIME_CLASS(CPrivateChatWnd) ) )
		{
			if ( pFrame->Find( strHighID ) )
				return pFrame;
		}
	}

	return NULL;
}

CPrivateChatWnd* CChatWindows::FindED2KFrame(DWORD nClientID, const SOCKADDR_IN* pServerAddress) const
{
	// For Low ID clients

	if ( ( nClientID > 0 ) && ( nClientID < 16777216 ) )  // ED2K Low ID
	{
		CString strLowID;
		strLowID.Format( _T("%u@%s:%hu"),
		nClientID,
		(LPCTSTR)CString( inet_ntoa( pServerAddress->sin_addr ) ),
		pServerAddress->sin_port );

		for ( POSITION pos = GetIterator() ; pos ; )
		{
			CPrivateChatWnd* pFrame = static_cast<CPrivateChatWnd*>( GetNext( pos ) );

			if ( pFrame->IsKindOf( RUNTIME_CLASS(CPrivateChatWnd) ) )
			{
				if ( pFrame->Find( strLowID ) )
					return pFrame;
			}
		}
	}

	return NULL;
}

CPrivateChatWnd* CChatWindows::OpenPrivate(const Hashes::Guid& oGUID, const IN_ADDR* pAddress, WORD nPort, BOOL bMustPush, PROTOCOLID nProtocol, IN_ADDR* pServerAddress, WORD nServerPort)
{
	SOCKADDR_IN pHost = {};

	pHost.sin_family	= PF_INET;
	pHost.sin_addr		= *pAddress;
	pHost.sin_port		= htons( nPort );

	if ( pServerAddress == NULL )
		return OpenPrivate( oGUID, &pHost, bMustPush, nProtocol, NULL );

	SOCKADDR_IN pServer = {};

	pServer.sin_family	= PF_INET;
	pServer.sin_addr	= *pServerAddress;
	pServer.sin_port	= htons( nServerPort );

	return OpenPrivate( oGUID, &pHost, bMustPush, nProtocol, &pServer );
}

CPrivateChatWnd* CChatWindows::OpenPrivate(const Hashes::Guid& oGUID, const SOCKADDR_IN* pHost, BOOL bMustPush, PROTOCOLID nProtocol, SOCKADDR_IN* pServer)
{
	if ( ! MyProfile.IsValid() )
	{
		CString strMessage;
		LoadString( strMessage, IDS_CHAT_NEED_PROFILE );
		if ( AfxMessageBox( strMessage, MB_YESNO|MB_ICONQUESTION ) == IDYES )
			PostMainWndMessage( WM_COMMAND, ID_TOOLS_PROFILE );
		return NULL;
	}

	switch ( nProtocol )
	{
	case PROTOCOL_G1:
		Settings.Gnutella1.EnableToday = true;
		return OpenPrivateGnutella( oGUID, pHost, bMustPush, PROTOCOL_ANY );

	case PROTOCOL_G2:
	case PROTOCOL_HTTP:
		Settings.Gnutella2.EnableToday = true;
		return OpenPrivateGnutella( oGUID, pHost, bMustPush, PROTOCOL_ANY );

	case PROTOCOL_ED2K:
		Settings.eDonkey.EnableToday = true;
		return OpenPrivateED2K( oGUID, pHost, bMustPush, pServer );

	default:
		return NULL;
	}
}

CPrivateChatWnd* CChatWindows::OpenPrivateGnutella(const Hashes::Guid& oGUID, const SOCKADDR_IN* pHost, BOOL bMustPush, PROTOCOLID nProtocol)
{
	CPrivateChatWnd* pFrame = NULL;
	if ( oGUID )
	{
		pFrame = FindPrivate( oGUID, false );
		if ( pFrame == NULL ) pFrame = FindPrivate( oGUID, true );
	}
	if ( pFrame == NULL ) pFrame = FindPrivate( pHost );

	if ( pFrame == NULL )
	{
		pFrame = new CPrivateChatWnd();
		pFrame->Setup( oGUID, pHost, bMustPush, nProtocol );
	}

	pFrame->PostMessage( WM_COMMAND, ID_CHAT_CONNECT );

	pFrame->Open();

	return pFrame;
}

CPrivateChatWnd* CChatWindows::OpenPrivateED2K(const Hashes::Guid& oGUID, const SOCKADDR_IN* pHost, BOOL bMustPush, SOCKADDR_IN* pServer)
{
	// First, check if it's a low ID user on another server.
	if ( bMustPush && pServer )
	{
		// It's a firewalled user (Low ID). If they are using another server, we
		// can't (shouldn't) contact them. (It places a heavy load on the ed2k servers)
		CSingleLock pLock1( &Network.m_pSection );
		if ( ! pLock1.Lock( 250 ) ) return NULL;
		if ( Neighbours.Get( pServer->sin_addr ) == NULL ) return NULL;
		pLock1.Unlock();
	}

	// ED2K chat is handled by the EDClient section. (Transfers)
	// We need to find (or create) an EDClient to handle this chat session, since everything
	// on ed2k shares a TCP link.

	// First, lock the section to prevent a problem with other threads
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return NULL;

	// We need to connect to them, so either find or create an EDClient
	CEDClient* pClient;
	if ( pServer )
		pClient = EDClients.Connect(pHost->sin_addr.s_addr, ntohs( pHost->sin_port ), &pServer->sin_addr, ntohs( pServer->sin_port ), oGUID );
	else
		pClient = EDClients.Connect(pHost->sin_addr.s_addr, ntohs( pHost->sin_port ), NULL, 0, oGUID );
	// If we weren't able to create a client (Low-id and no server), then exit.
	if ( ! pClient ) return NULL;
	// Have it connect (if it isn't)
	if ( ! pClient->m_bConnected ) pClient->Connect();
	// Tell it to start a chat session as soon as it's able
	pClient->OpenChat();
	pLock.Unlock();

	// Check for / make active any existing window
	CPrivateChatWnd* pFrame = FindPrivate( pHost );
	// Check for an empty frame
	if ( pFrame == NULL )
	{
		if ( bMustPush ) pFrame = FindED2KFrame( pHost->sin_addr.s_addr, pServer );
		else pFrame = FindED2KFrame( pHost );
	}
	if ( pFrame != NULL )
	{
		// Open window if we found one
		pFrame->Open();

		// And exit
		return pFrame;
	}

	// Set name (Also used to match incoming connection)
	CString sNick;
	if ( bMustPush && pServer ) // Firewalled user (Low ID)
	{
		sNick.Format( _T("%lu@%s:%hu"),
			pHost->sin_addr.s_addr,
			(LPCTSTR)CString( inet_ntoa( pServer->sin_addr ) ),
			ntohs( pServer->sin_port ) );
	}
	else	// Regular user (High ID)
	{
		sNick.Format( _T("%s:%hu"), (LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ), ntohs( pHost->sin_port ) );
	}

	// Open an empty (blank) chat frame. This is totally unnecessary- The EDClient will open
	// one as required, but it looks better to open one here.
	pFrame = new CPrivateChatWnd();
	pFrame->Setup( sNick );

	return pFrame;
}

//////////////////////////////////////////////////////////////////////
// CChatWindows add and remove

void CChatWindows::Add(CChatWnd* pFrame)
{
	if ( m_pList.Find( pFrame ) == NULL )
		m_pList.AddTail( pFrame );
}

void CChatWindows::Remove(CChatWnd* pFrame)
{
	if ( POSITION pos = m_pList.Find( pFrame ) )
		m_pList.RemoveAt( pos );
}
