//
// Handshakes.cpp
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
#include "Handshakes.h"
#include "Connection.h"
#include "Handshake.h"
#include "Network.h"
#include "Security.h"
#include "Datagrams.h"
#include "DiscoveryServices.h"
#include "Transfers.h"
#include "Uploads.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CHandshakes Handshakes;


//////////////////////////////////////////////////////////////////////
// CHandshakes construction

CHandshakes::CHandshakes()
{
	m_nStableCount	= 0;
	m_tStableTime	= 0;
	m_hSocket		= INVALID_SOCKET;
	m_hThread		= NULL;
}

CHandshakes::~CHandshakes()
{
	Disconnect();
	
	ASSERT( m_hSocket == INVALID_SOCKET );
	ASSERT( m_hThread == NULL );
}

//////////////////////////////////////////////////////////////////////
// CHandshakes listen

BOOL CHandshakes::Listen()
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	if ( m_hSocket != INVALID_SOCKET ) return FALSE;
	
	m_hSocket = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( m_hSocket == INVALID_SOCKET ) return FALSE;
	
	SOCKADDR_IN saListen = Network.m_pHost;
	if ( ! Settings.Connection.InBind ) saListen.sin_addr.S_un.S_addr = 0;
	
	BOOL bBound = FALSE;
	
	for ( int nAttempt = 0 ; nAttempt < 5 ; nAttempt++ )
	{
		bBound = bind( m_hSocket, (SOCKADDR*)&saListen, sizeof(saListen) ) == 0;
		if ( bBound ) break;
		
		theApp.Message( MSG_ERROR, IDS_NETWORK_CANT_LISTEN,
			(LPCTSTR)CString( inet_ntoa( saListen.sin_addr ) ),
			htons( saListen.sin_port ) );
		
		if ( nAttempt )
		{
			int nPort = Network.RandomPort();
			Network.m_pHost.sin_port = saListen.sin_port = htons( nPort );
			//if ( Settings.Connection.InPort != 6346 ) Settings.Connection.InPort = nPort;
			//Don't reset the port- users with port forwarding have problems.
		}
		else
		{
			saListen.sin_addr.S_un.S_addr = 0;
		}
	}
	
	if ( Network.m_pHost.sin_addr.S_un.S_addr == 0 )
	{
		int nSockLen = sizeof(SOCKADDR_IN);
		getsockname( m_hSocket, (SOCKADDR*)&Network.m_pHost, &nSockLen );
	}
	
	if ( bBound )
	{
		theApp.Message( MSG_DEFAULT, IDS_NETWORK_LISTENING_TCP,
			(LPCTSTR)CString( inet_ntoa( Network.m_pHost.sin_addr ) ),
			htons( Network.m_pHost.sin_port ) );
	}
	
	WSAEventSelect( m_hSocket, m_pWakeup, FD_ACCEPT );
	listen( m_hSocket, 256 );
	
	m_hThread = AfxBeginThread( ThreadStart, this )->m_hThread;
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CHandshakes disconnect

void CHandshakes::Disconnect()
{
	if ( m_hSocket != INVALID_SOCKET )
	{
		closesocket( m_hSocket );
		m_hSocket = INVALID_SOCKET;
	}
	
	if ( m_hThread != NULL )
	{
		m_pWakeup.SetEvent();
		
		for ( int nAttempt = 10 ; nAttempt > 0 ; nAttempt-- )
		{
			DWORD nCode;
			if ( ! GetExitCodeThread( m_hThread, &nCode ) ) break;
			if ( nCode != STILL_ACTIVE ) break;
			Sleep( 100 );
		}
		
		if ( nAttempt == 0 )
		{
			TerminateThread( m_hThread, 0 );
			theApp.Message( MSG_DEBUG, _T("WARNING: Terminating CHandshakes thread.") );
			Sleep( 100 );
		}
		
		m_hThread = NULL;
	}
	
	CSingleLock pLock( &m_pSection, TRUE );
	
	m_nStableCount	= 0;
	m_tStableTime	= 0;
	
	for ( POSITION pos = GetIterator() ; pos ; ) delete GetNext( pos );
	m_pList.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// CHandshakes push connection

BOOL CHandshakes::PushTo(IN_ADDR* pAddress, WORD nPort, DWORD nIndex)
{
	CSingleLock pLock1( &Transfers.m_pSection );
	
	if ( pLock1.Lock( 250 ) )
	{
		if ( ! Uploads.AllowMoreTo( pAddress ) )
		{
			CString strAddress = inet_ntoa( *pAddress );
			theApp.Message( MSG_ERROR, IDS_UPLOAD_PUSH_BUSY, (LPCTSTR)strAddress );
			return FALSE;
		}
		
		pLock1.Unlock();
	}
	
	CSingleLock pLock2( &m_pSection, TRUE );
	
	CHandshake* pHandshake = new CHandshake();
	
	if ( pHandshake->Push( pAddress, nPort, nIndex ) )
	{
		m_pList.AddTail( pHandshake );
		return TRUE;
	}
	else
	{
		delete pHandshake;
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// CHandshakes connection test

BOOL CHandshakes::IsConnectedTo(IN_ADDR* pAddress)
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CHandshake* pHandshake = GetNext( pos );
		if ( pHandshake->m_pHost.sin_addr.S_un.S_addr == pAddress->S_un.S_addr ) return TRUE;
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CHandshakes list modification

void CHandshakes::Substitute(CHandshake* pOld, CHandshake* pNew)
{
	POSITION pos = m_pList.Find( pOld );
	ASSERT( pos != NULL );
	m_pList.SetAt( pos, pNew );
	delete pOld;
}

void CHandshakes::Remove(CHandshake* pHandshake)
{
	POSITION pos = m_pList.Find( pHandshake );
	ASSERT( pos != NULL );
	m_pList.RemoveAt( pos );
	delete pHandshake;
}

//////////////////////////////////////////////////////////////////////
// CHandshakes thread run

UINT CHandshakes::ThreadStart(LPVOID pParam)
{
	((CHandshakes*)pParam)->OnRun();
	return 0;
}

void CHandshakes::OnRun()
{
	while ( m_hSocket != INVALID_SOCKET )
	{
		WaitForSingleObject( m_pWakeup, 1000 );
		while ( AcceptConnection() );
		RunHandshakes();
		while ( AcceptConnection() );
		RunStableUpdate();
	}
}

//////////////////////////////////////////////////////////////////////
// CHandshakes run the handshake
	
void CHandshakes::RunHandshakes()
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	for ( POSITION posNext = GetIterator() ; posNext ; )
	{
		POSITION posThis = posNext;		
		CHandshake* pHandshake = GetNext( posNext );
		
		if ( ! pHandshake->DoRun() && m_pList.GetAt( posThis ) == pHandshake )
		{
			delete pHandshake;
			m_pList.RemoveAt( posThis );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CHandshakes accept a connection

BOOL CHandshakes::AcceptConnection()
{
	/*
	WSANETWORKEVENTS pEvents;
	WSAEnumNetworkEvents( m_hSocket, NULL, &pEvents );
	if ( ( pEvents.lNetworkEvents & FD_ACCEPT ) == 0 ) return FALSE;
	*/
	
	SOCKADDR_IN pHost;
	int nHost = sizeof(pHost);
	
	SOCKET hSocket = WSAAccept( m_hSocket, (SOCKADDR*)&pHost, &nHost, AcceptCheck, (DWORD)this );
	if ( hSocket == INVALID_SOCKET ) return FALSE;
	
	InterlockedIncrement( (PLONG)&m_nStableCount );
	
	if ( Security.IsDenied( &pHost.sin_addr ) )
	{
		closesocket( hSocket );
		CString strHost = inet_ntoa( pHost.sin_addr );
		theApp.Message( MSG_ERROR, IDS_NETWORK_SECURITY_DENIED, (LPCTSTR)strHost );
	}
	else
	{
		CreateHandshake( hSocket, &pHost );
	}
	
	return TRUE;
}

void CHandshakes::CreateHandshake(SOCKET hSocket, SOCKADDR_IN* pHost)
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	WSAEventSelect( hSocket, m_pWakeup, FD_READ|FD_WRITE|FD_CLOSE );
	m_pList.AddTail( new CHandshake( hSocket, pHost ) );
}

//////////////////////////////////////////////////////////////////////
// CHandshakes inbound connection security check callback

int CALLBACK CHandshakes::AcceptCheck(IN LPWSABUF lpCallerId, IN LPWSABUF lpCallerData, IN OUT LPQOS lpSQOS, IN OUT LPQOS lpGQOS, IN LPWSABUF lpCalleeId, OUT LPWSABUF lpCalleeData, OUT GROUP FAR * g, IN DWORD dwCallbackData)
{
	if ( lpCallerId == NULL ) return CF_REJECT;
	if ( lpCallerId->len < sizeof(SOCKADDR_IN) ) return CF_REJECT;
	
	SOCKADDR_IN* pHost = (SOCKADDR_IN*)lpCallerId->buf;
	
	if ( Security.IsDenied( &pHost->sin_addr ) )
	{
		CString strHost = inet_ntoa( pHost->sin_addr );
		theApp.Message( MSG_ERROR, IDS_NETWORK_SECURITY_DENIED, (LPCTSTR)strHost );
		return CF_REJECT;
	}
	else
	{
		return CF_ACCEPT;
	}
}

//////////////////////////////////////////////////////////////////////
// CHandshakes update stable state

void CHandshakes::RunStableUpdate()
{
	if ( m_nStableCount > 0 )
	{
		if ( m_tStableTime == 0 ) m_tStableTime = (DWORD)time( NULL );
		DiscoveryServices.Update();
	}
}
