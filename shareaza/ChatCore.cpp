//
// ChatCore.cpp
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
#include "EDClient.h"
#include "ChatSession.h"
#include "ChatCore.h"
#include "Buffer.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CChatCore ChatCore;


//////////////////////////////////////////////////////////////////////
// CChatCore construction

CChatCore::CChatCore()
{
	m_hThread = NULL;
	m_bThread = FALSE;
}

CChatCore::~CChatCore()
{
	Close();
}

//////////////////////////////////////////////////////////////////////
// CChatCore session access

POSITION CChatCore::GetIterator() const
{
	return m_pSessions.GetHeadPosition();
}

CChatSession* CChatCore::GetNext(POSITION& pos) const
{
	return (CChatSession*)m_pSessions.GetNext( pos );
}

int CChatCore::GetCount() const
{
	return m_pSessions.GetCount();
}

BOOL CChatCore::Check(CChatSession* pSession) const
{
	return m_pSessions.Find( pSession ) != NULL;
}

//////////////////////////////////////////////////////////////////////
// CChatCore accept new connections

void CChatCore::OnAccept(CConnection* pConnection, PROTOCOLID nProtocol)
{
	CSingleLock pLock( &m_pSection );
	if ( ! pLock.Lock( 250 ) ) return;
	
	CChatSession* pSession = new CChatSession();

	pSession->m_nProtocol = nProtocol;

	pSession->AttachTo( pConnection );
}

BOOL CChatCore::OnPush(GGUID* pGUID, CConnection* pConnection)
{
	CSingleLock pLock( &m_pSection );
	if ( ! pLock.Lock( 250 ) ) return FALSE;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CChatSession* pSession = GetNext( pos );
		if ( pSession->OnPush( pGUID, pConnection ) ) return TRUE;
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CChatCore ED2K chat handling

void CChatCore::OnED2KMessage(CEDClient* pClient, CEDPacket* pPacket)
{
	ASSERT ( pClient != NULL );
	// Note: Null packet is valid- in that case we have no pending message, but want to open
	// a chat window.

	CSingleLock pLock( &m_pSection );
	if ( ! pLock.Lock( 250 ) ) return;
	
	CChatSession* pSession = FindSession( pClient );

	pSession->OnED2KMessage( pPacket );
}

CChatSession* CChatCore::FindSession(CEDClient* pClient)
{
	CChatSession* pSession;

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		pSession = GetNext( pos );

		// If we already have a session
		if ( ( ( ! pSession->m_bGUID ) || ( pSession->m_pGUID == pClient->m_pGUID ) ) &&
			 ( pSession->m_pHost.sin_addr.S_un.S_addr == pClient->m_pHost.sin_addr.S_un.S_addr ) &&
			 ( pSession->m_nProtocol == PROTOCOL_ED2K ) )
		{
			// Update details
			pSession->m_bGUID		= pClient->m_bGUID;
			pSession->m_pGUID		= pClient->m_pGUID;
			pSession->m_pHost		= pClient->m_pHost;
			pSession->m_sAddress	= pClient->m_sAddress;
			pSession->m_sUserNick	= pClient->m_sNick;
			pSession->m_sUserAgent	= pClient->m_sUserAgent;
			pSession->m_bUnicode	= pClient->m_bEmUnicode;
			pSession->m_nClientID	= pClient->m_nClientID;
			pSession->m_pServer		= pClient->m_pServer;

			pSession->m_bMustPush	= ( ( pClient->m_nClientID > 0 ) && ( pClient->m_nClientID < 16777216 ) );

			// return existing session
			return pSession;
		}
	}

	// Create a new chat session
	pSession = new CChatSession();

	pSession->m_nProtocol	= PROTOCOL_ED2K;
	pSession->m_hSocket		= INVALID_SOCKET;			// Should always remain invalid- has no real connection
	pSession->m_nState		= cssActive;
	pSession->m_bConnected	= TRUE;
	pSession->m_tConnected	= GetTickCount();

	// Set details
	pSession->m_bGUID		= pClient->m_bGUID;
	pSession->m_pGUID		= pClient->m_pGUID;
	pSession->m_pHost		= pClient->m_pHost;
	pSession->m_sAddress	= pClient->m_sAddress;
	pSession->m_sUserNick	= pClient->m_sNick;
	pSession->m_sUserAgent	= pClient->m_sUserAgent;
	pSession->m_bUnicode	= pClient->m_bEmUnicode;
	pSession->m_nClientID	= pClient->m_nClientID;
	pSession->m_pServer		= pClient->m_pServer;

	pSession->m_bMustPush	= ( ( pClient->m_nClientID > 0 ) && ( pClient->m_nClientID < 16777216 ) );

	// Make new input and output buffer objects
	DWORD nLimit = 0;
	pSession->m_pInput		= new CBuffer( &nLimit );
	pSession->m_pOutput		= new CBuffer( &nLimit );

	Add( pSession );

	return pSession;
}

//////////////////////////////////////////////////////////////////////
// CChatCore session add and remove

void CChatCore::Add(CChatSession* pSession)
{
	CSingleLock pLock( &m_pSection, TRUE );
	if ( m_pSessions.Find( pSession ) == NULL ) m_pSessions.AddTail( pSession );
	if ( pSession->m_hSocket != INVALID_SOCKET )
		WSAEventSelect( pSession->m_hSocket, m_pWakeup, FD_CONNECT|FD_READ|FD_WRITE|FD_CLOSE );
	StartThread();
}

void CChatCore::Remove(CChatSession* pSession)
{
	CSingleLock pLock( &m_pSection, TRUE );
	POSITION pos = m_pSessions.Find( pSession );
	if ( pos != NULL ) m_pSessions.RemoveAt( pos );
	if ( pSession->m_hSocket != INVALID_SOCKET )
		WSAEventSelect( pSession->m_hSocket, m_pWakeup, 0 );
}

void CChatCore::Close()
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		GetNext( pos )->Close();
	}
	
	StopThread();
}

//////////////////////////////////////////////////////////////////////
// CChatCore thread control

void CChatCore::StartThread()
{
	if ( m_hThread != NULL && m_bThread ) return;
	if ( GetCount() == 0 ) return;
	
	m_bThread = TRUE;
	CWinThread* pThread = AfxBeginThread( ThreadStart, this, THREAD_PRIORITY_NORMAL );
	m_hThread = pThread->m_hThread;
}

void CChatCore::StopThread()
{
	if ( m_hThread == NULL ) return;
	
	m_pWakeup.SetEvent();
	
    int nAttempt = 5;
	for ( ; nAttempt > 0 ; nAttempt-- )
	{
		DWORD nCode;
		if ( ! GetExitCodeThread( m_hThread, &nCode ) ) break;
		if ( nCode != STILL_ACTIVE ) break;
		Sleep( 100 );
	}
	
	if ( nAttempt == 0 )
	{
		TerminateThread( m_hThread, 0 );
		theApp.Message( MSG_DEBUG, _T("WARNING: Terminating CChatCore thread.") );
		Sleep( 100 );
	}
	
	m_hThread = NULL;
}

//////////////////////////////////////////////////////////////////////
// CChatCore thread run

UINT CChatCore::ThreadStart(LPVOID pParam)
{
	CChatCore* pChatCore = (CChatCore*)pParam;
	pChatCore->OnRun();
	return 0;
}

void CChatCore::OnRun()
{
	CSingleLock pLock( &m_pSection );
	
	while ( m_bThread )
	{
		Sleep( 50 );
		WaitForSingleObject( m_pWakeup, 100 );
		
		if ( pLock.Lock( 250 ) )
		{
			if ( GetCount() == 0 ) break;
			
			for ( POSITION pos = GetIterator() ; pos ; )
			{
				GetNext( pos )->DoRun();
			}
			
			pLock.Unlock();
		}
	}
	
	m_bThread = FALSE;
}
