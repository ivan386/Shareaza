//
// Network.cpp
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
#include "Network.h"
#include "Security.h"
#include "Handshakes.h"
#include "Neighbours.h"
#include "Neighbour.h"
#include "Datagrams.h"
#include "HostCache.h"
#include "RouteCache.h"
#include "QueryKeys.h"
#include "GProfile.h"
#include "Transfers.h"
#include "Downloads.h"
#include "Statistics.h"
#include "DiscoveryServices.h"

#include "CrawlSession.h"
#include "SearchManager.h"
#include "QueryHashMaster.h"
#include "QuerySearch.h"
#include "QueryHit.h"
#include "Buffer.h"
#include "G1Packet.h"
#include "G2Packet.h"
#include "G1Neighbour.h"

#include "WndMain.h"
#include "WndChild.h"
#include "WndSearchMonitor.h"
#include "WndHitMonitor.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CNetwork Network;


//////////////////////////////////////////////////////////////////////
// CNetwork construction

CNetwork::CNetwork()
{
	NodeRoute		= new CRouteCache();
	QueryRoute		= new CRouteCache();
	QueryKeys		= new CQueryKeys();
	
	m_bEnabled		= FALSE;
	m_bAutoConnect	= FALSE;
	m_nSequence		= 0;
	m_hThread		= NULL;
}

CNetwork::~CNetwork()
{
	delete QueryKeys;
	delete QueryRoute;
	delete NodeRoute;
}

//////////////////////////////////////////////////////////////////////
// CNetwork attributes

BOOL CNetwork::IsAvailable() const
{
	DWORD dwState = 0;
	// return InternetGetConnectedState( &dwState, 0 );
	return FALSE;
}

BOOL CNetwork::IsConnected() const
{
	return m_bEnabled;
}

BOOL CNetwork::IsListening() const
{
	return m_bEnabled
		&& ( m_pHost.sin_addr.S_un.S_addr != 0 )
		&& ( m_pHost.sin_port != 0 )
		&& ( Handshakes.IsListening() );
}

int CNetwork::IsWellConnected() const
{
	return Neighbours.m_nStableCount;
}

BOOL CNetwork::IsStable() const
{
	return IsListening() && ( Handshakes.m_nStableCount > 0 );
}

DWORD CNetwork::GetStableTime() const
{
	if ( ! IsStable() || ! Handshakes.m_tStableTime ) return 0;
	return (DWORD)time( NULL ) - Handshakes.m_tStableTime;
}

BOOL CNetwork::IsConnectedTo(IN_ADDR* pAddress)
{
	if ( pAddress->S_un.S_addr == m_pHost.sin_addr.S_un.S_addr ) return TRUE;
	if ( Handshakes.IsConnectedTo( pAddress ) ) return TRUE;
	if ( Neighbours.Get( pAddress ) != NULL ) return TRUE;
	if ( Transfers.IsConnectedTo( pAddress ) ) return TRUE;
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CNetwork connection

BOOL CNetwork::Connect(BOOL bAutoConnect)
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	if ( bAutoConnect ) m_bAutoConnect = TRUE;
	Settings.Live.AutoClose = FALSE;
	
	if ( m_bEnabled )
	{
		if ( bAutoConnect ) DiscoveryServices.Execute();
		return TRUE;
	}
	
	theApp.Message( MSG_SYSTEM, IDS_NETWORK_STARTUP );
	
	Resolve( Settings.Connection.InHost, Settings.Connection.InPort, &m_pHost );
	
	if ( Settings.Connection.Firewalled )
		theApp.Message( MSG_DEFAULT, IDS_NETWORK_FIREWALLED );
	
	SOCKADDR_IN pOutgoing;

	if ( Resolve( Settings.Connection.OutHost, 0, &pOutgoing ) )
	{
		theApp.Message( MSG_DEFAULT, IDS_NETWORK_OUTGOING,
			(LPCTSTR)CString( inet_ntoa( pOutgoing.sin_addr ) ),
			htons( pOutgoing.sin_port ) );
	}
	else if ( Settings.Connection.OutHost.GetLength() )
	{
		theApp.Message( MSG_ERROR, IDS_NETWORK_CANT_OUTGOING,
			(LPCTSTR)Settings.Connection.OutHost );
	}
	
	Handshakes.Listen();
	Datagrams.Listen();
	Neighbours.Connect();
	
	NodeRoute->SetDuration( Settings.Gnutella.RouteCache );
	QueryRoute->SetDuration( Settings.Gnutella.RouteCache );
	
	m_bEnabled	= TRUE;
	m_hThread	= AfxBeginThread( ThreadStart, this, THREAD_PRIORITY_NORMAL )->m_hThread;
	
	// if ( m_bAutoConnect && bAutoConnect ) DiscoveryServices.Execute();
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CNetwork disconnect

void CNetwork::Disconnect()
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	if ( ! m_bEnabled ) return;
	
	theApp.Message( MSG_DEFAULT, _T("") );
	theApp.Message( MSG_SYSTEM, IDS_NETWORK_DISCONNECTING );
	
	m_bEnabled		= FALSE;
	m_bAutoConnect	= FALSE;
	
	Neighbours.Close();
	
	pLock.Unlock();
	
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
			theApp.Message( MSG_DEBUG, _T("WARNING: Terminating CNetwork thread.") );
			Sleep( 100 );
		}
		
		m_hThread = NULL;
	}
	
	Handshakes.Disconnect();
	pLock.Lock();
	
	Neighbours.Close();
	Datagrams.Disconnect();
	
	NodeRoute->Clear();
	QueryRoute->Clear();
	
	if ( TRUE )
	{
		for ( POSITION pos = m_pLookups.GetStartPosition() ; pos ; )
		{
			LPBYTE pAsync, pBuffer;
			m_pLookups.GetNextAssoc( pos, (VOID*&)pAsync, (VOID*&)pBuffer );
			WSACancelAsyncRequest( (HANDLE)pAsync );
			delete *(CString**)pBuffer;
			free( pBuffer );
		}
		
		m_pLookups.RemoveAll();
	}
	
	pLock.Unlock();
	
	DiscoveryServices.Stop();
	
	theApp.Message( MSG_SYSTEM, IDS_NETWORK_DISCONNECTED ); 
	theApp.Message( MSG_DEFAULT, _T("") );
}

//////////////////////////////////////////////////////////////////////
// CNetwork host connection

BOOL CNetwork::ConnectTo(LPCTSTR pszAddress, int nPort, PROTOCOLID nProtocol, BOOL bNoUltraPeer)
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	if ( ! m_bEnabled && ! Connect() ) return FALSE;
	
	if ( nPort == 0 ) nPort = GNUTELLA_DEFAULT_PORT;
	theApp.Message( MSG_DEFAULT, IDS_NETWORK_RESOLVING, pszAddress );
	
	if ( AsyncResolve( pszAddress, (WORD)nPort, nProtocol, bNoUltraPeer ? 2 : 1 ) ) return TRUE;
	
	theApp.Message( MSG_ERROR, IDS_NETWORK_RESOLVE_FAIL, pszAddress );
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CNetwork local IP aquisition and sending

void CNetwork::AcquireLocalAddress(LPCTSTR pszHeader)
{
	int nIP[4];
	
	if ( _stscanf( pszHeader, _T("%i.%i.%i.%i"), &nIP[0], &nIP[1], &nIP[2], &nIP[3] ) != 4 ) return;
	
	IN_ADDR pAddress;
	
	pAddress.S_un.S_un_b.s_b1 = (BYTE)nIP[0];
	pAddress.S_un.S_un_b.s_b2 = (BYTE)nIP[1];
	pAddress.S_un.S_un_b.s_b3 = (BYTE)nIP[2];
	pAddress.S_un.S_un_b.s_b4 = (BYTE)nIP[3];
	
	if ( IsFirewalledAddress( &pAddress ) ) return;
	
	m_pHost.sin_addr = pAddress;

	//Security.SessionBan( &pAddress, 0 );		// Ban self
}

//////////////////////////////////////////////////////////////////////
// CNetwork GGUID generation

void CNetwork::CreateID(GGUID* pID)
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	*pID = MyProfile.GUID;
	
	DWORD *pNum = (DWORD*)pID;
	pNum[0] += GetTickCount();
	pNum[1] += ( m_nSequence++ );
	pNum[2] += rand() * rand();
	pNum[3] += rand() * rand();
}

//////////////////////////////////////////////////////////////////////
// CNetwork firewalled address checking

BOOL CNetwork::IsFirewalledAddress(LPVOID pAddress, BOOL bIncludeSelf)
{
	if ( ! pAddress ) return TRUE;
	if ( ! Settings.Connection.IgnoreLocalIP ) return FALSE;
	
	DWORD nAddress = *(DWORD*)pAddress;
	
	if ( ! nAddress ) return TRUE;
	if ( ( nAddress & 0xFFFF ) == 0xA8C0 ) return TRUE;
	if ( ( nAddress & 0xF0AC ) == 0x08AC ) return TRUE;
	if ( ( nAddress & 0xFF ) == 0x0A ) return TRUE;
	if ( ( nAddress & 0xFF ) == 0x7F ) return TRUE;		// 127.*
	
	if ( bIncludeSelf && nAddress == *(DWORD*)(&m_pHost.sin_addr) ) return TRUE;
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CNetwork name resolution

BOOL CNetwork::Resolve(LPCTSTR pszHost, int nPort, SOCKADDR_IN* pHost, BOOL bNames) const
{
	ZeroMemory( pHost, sizeof(*pHost) );
	pHost->sin_family	= PF_INET;
	pHost->sin_port		= htons( nPort );
	
	if ( pszHost == NULL || *pszHost == 0 ) return FALSE;
	
	CString strHost( pszHost );
	
	int nColon = strHost.Find( ':' );
	
	if ( nColon >= 0 )
	{
		if ( _stscanf( strHost.Mid( nColon + 1 ), _T("%i"), &nPort ) == 1 )
		{
			pHost->sin_port = htons( nPort );
		}
		
		strHost = strHost.Left( nColon );
	}
	
	USES_CONVERSION;
	LPCSTR pszaHost = T2CA( (LPCTSTR)strHost );
	
	DWORD dwIP = inet_addr( pszaHost );
	
	if ( dwIP == INADDR_NONE )
	{
		if ( ! bNames ) return TRUE;
		
		HOSTENT* pLookup = gethostbyname( pszaHost );
		
		if ( pLookup == NULL ) return FALSE;
		
		CopyMemory( &pHost->sin_addr, pLookup->h_addr, 4 );
	}
	else
	{
		CopyMemory( &pHost->sin_addr, &dwIP, 4 );
	}
	
	return TRUE;
}

BOOL CNetwork::AsyncResolve(LPCTSTR pszAddress, WORD nPort, PROTOCOLID nProtocol, BYTE nCommand)
{
	CSingleLock pLock( &m_pSection );
	if ( ! pLock.Lock( 250 ) ) return FALSE;
	
	BYTE* pResolve = (BYTE*)malloc( MAXGETHOSTSTRUCT + 8 );
	
	USES_CONVERSION;
	
	HANDLE hAsync = WSAAsyncGetHostByName( AfxGetMainWnd()->GetSafeHwnd(), WM_WINSOCK,
		T2CA(pszAddress), (LPSTR)pResolve + 8, MAXGETHOSTSTRUCT );
	
	if ( hAsync != NULL )
	{
		*((CString**)&pResolve[0])	= new CString( pszAddress );
		*((WORD*)&pResolve[4])		= nPort;
		*((BYTE*)&pResolve[6])		= nProtocol;
		*((BYTE*)&pResolve[7])		= nCommand;
		
		m_pLookups.SetAt( (LPVOID)hAsync, (LPVOID)pResolve );
		return TRUE;
	}
	else
	{
		free( pResolve );
		return FALSE;
	}
}

WORD CNetwork::RandomPort() const
{
	return 10000 + ( rand() % 50000 );
}

//////////////////////////////////////////////////////////////////////
// CNetwork thread run

UINT CNetwork::ThreadStart(LPVOID pParam)
{
	CNetwork* pNetwork = (CNetwork*)pParam;
	pNetwork->OnRun();
	return 0;
}

void CNetwork::OnRun()
{
	while ( m_bEnabled )
	{
		Sleep( 50 );
		WaitForSingleObject( m_pWakeup, 100 );
		
		if ( m_bEnabled && m_pSection.Lock() )
		{
			Datagrams.OnRun();
			SearchManager.OnRun();
			QueryHashMaster.Build();
			
			if ( CrawlSession.m_bActive ) CrawlSession.OnRun();
			
			m_pSection.Unlock();
		}
		
		Neighbours.OnRun();
	}
}

//////////////////////////////////////////////////////////////////////
// CNetwork resolve callback

void CNetwork::OnWinsock(WPARAM wParam, LPARAM lParam)
{
	CSingleLock pLock( &m_pSection, TRUE );

	LPBYTE pBuffer = NULL;
	if ( ! m_pLookups.Lookup( (LPVOID)wParam, (LPVOID&)pBuffer ) ) return;
	m_pLookups.RemoveKey( (LPVOID)wParam );

	CString* psHost	= *(CString**)pBuffer;
	WORD nPort		= *(WORD*)(pBuffer + 4);
	BYTE nProtocol	= *(BYTE*)(pBuffer + 6);
	BYTE nCommand	= *(BYTE*)(pBuffer + 7);
	HOSTENT* pHost	= (HOSTENT*)(pBuffer + 8);

	if ( WSAGETASYNCERROR(lParam) == 0 )
	{
		if ( nCommand == 0 )
		{
			HostCache.ForProtocol( nProtocol )->Add( (IN_ADDR*)pHost->h_addr, nPort );
		}
		else
		{
			Neighbours.ConnectTo( (IN_ADDR*)pHost->h_addr, nPort, nProtocol, FALSE, nCommand == 2 );
		}
	}
	else if ( nCommand > 0 )
	{
		theApp.Message( MSG_ERROR, IDS_NETWORK_RESOLVE_FAIL, (LPCTSTR)*psHost );
	}
	
	delete psHost;
	free( pBuffer );
}

//////////////////////////////////////////////////////////////////////
// CNetwork get node route

BOOL CNetwork::GetNodeRoute(GGUID* pGUID, CNeighbour** ppNeighbour, SOCKADDR_IN* pEndpoint)
{
	if ( *pGUID == MyProfile.GUID ) return FALSE;
	
	if ( Network.NodeRoute->Lookup( pGUID, ppNeighbour, pEndpoint ) ) return TRUE;
	if ( ppNeighbour == NULL ) return FALSE;
	
	for ( POSITION pos = Neighbours.GetIterator() ; pos ; )
	{
		CNeighbour* pNeighbour = Neighbours.GetNext( pos );
		
		if ( pNeighbour->m_bGUID && pNeighbour->m_pGUID == *pGUID )
		{
			*ppNeighbour = pNeighbour;
			return TRUE;
		}
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CNetwork route generic packets

BOOL CNetwork::RoutePacket(CG2Packet* pPacket)
{
	GGUID pGUID;
	
	if ( ! pPacket->GetTo( &pGUID ) || pGUID == MyProfile.GUID ) return FALSE;
	
	CNeighbour* pOrigin = NULL;
	SOCKADDR_IN pEndpoint;
	
	if ( GetNodeRoute( &pGUID, &pOrigin, &pEndpoint ) )
	{
		if ( pOrigin != NULL )
		{
			if ( pOrigin->m_nProtocol == PROTOCOL_G1 &&
				 pPacket->IsType( G2_PACKET_PUSH ) )
			{
				CG1Neighbour* pG1 = (CG1Neighbour*)pOrigin;
				pPacket->SkipCompound();
				pG1->SendG2Push( &pGUID, pPacket );
			}
			else
			{
				pOrigin->Send( pPacket, FALSE, TRUE );
			}
		}
		else
		{
			Datagrams.Send( &pEndpoint, pPacket, FALSE );
		}
		
		Statistics.Current.Gnutella2.Routed++;
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CNetwork send a push request

BOOL CNetwork::SendPush(GGUID* pGUID, DWORD nIndex)
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return TRUE;

	if ( ! IsListening() ) return FALSE;
	
	GGUID pGUID2 = *pGUID;
	SOCKADDR_IN pEndpoint;
	CNeighbour* pOrigin;
	int nCount = 0;
	
	while ( GetNodeRoute( &pGUID2, &pOrigin, &pEndpoint ) )
	{
		if ( pOrigin != NULL && pOrigin->m_nProtocol == PROTOCOL_G1 )
		{
			CG1Packet* pPacket = CG1Packet::New( G1_PACKET_PUSH,
				Settings.Gnutella1.MaximumTTL - 1 );
			
			pPacket->Write( pGUID, 16 );
			pPacket->WriteLongLE( nIndex );
			pPacket->WriteLongLE( m_pHost.sin_addr.S_un.S_addr );
			pPacket->WriteShortLE( htons( m_pHost.sin_port ) );
			
			pOrigin->Send( pPacket );
		}
		else
		{
			CG2Packet* pPacket = CG2Packet::New( G2_PACKET_PUSH, TRUE );
			
			pPacket->WritePacket( G2_PACKET_TO, 16 );
			pPacket->Write( pGUID, 16 );
			
			pPacket->WriteByte( 0 );
			pPacket->WriteLongLE( m_pHost.sin_addr.S_un.S_addr );
			pPacket->WriteShortBE( htons( m_pHost.sin_port ) );
			
			if ( pOrigin != NULL )
			{
				pOrigin->Send( pPacket );
			}
			else
			{
				Datagrams.Send( &pEndpoint, pPacket );
			}
		}
		
		pGUID2.n[15] ++;
		nCount++;
	}
	
	return nCount > 0;
}

//////////////////////////////////////////////////////////////////////
// CNetwork hit routing

BOOL CNetwork::RouteHits(CQueryHit* pHits, CPacket* pPacket)
{
	SOCKADDR_IN pEndpoint;
	CNeighbour* pOrigin;
	
	if ( ! QueryRoute->Lookup( &pHits->m_pSearchID, &pOrigin, &pEndpoint ) ) return FALSE;
	
	BOOL bWrapped = FALSE;
	
	if ( pPacket->m_nProtocol == PROTOCOL_G1 )
	{
		CG1Packet* pG1 = (CG1Packet*)pPacket;
		if ( ! pG1->Hop() ) return FALSE;
	}
	else if ( pPacket->m_nProtocol == PROTOCOL_G2 )
	{
		CG2Packet* pG2 = (CG2Packet*)pPacket;

		if ( pG2->IsType( G2_PACKET_HIT ) && pG2->m_nLength > 17 )
		{
			BYTE* pHops = pG2->m_pBuffer + pG2->m_nLength - 17;
			if ( *pHops > Settings.Gnutella1.MaximumTTL ) return FALSE;
			(*pHops) ++;
		}
		else if ( pG2->IsType( G2_PACKET_HIT_WRAP ) )
		{
			if ( ! pG2->SeekToWrapped() ) return FALSE;
			GNUTELLAPACKET* pG1 = (GNUTELLAPACKET*)( pPacket->m_pBuffer + pPacket->m_nPosition );
			if ( pG1->m_nTTL == 0 ) return FALSE;
			pG1->m_nTTL --;
			pG1->m_nHops ++;
			bWrapped = TRUE;
		}
	}
	
	if ( pOrigin != NULL )
	{
		if ( pOrigin->m_nProtocol == pPacket->m_nProtocol )
		{
			pOrigin->Send( pPacket, FALSE, FALSE );	// Dont buffer
		}
		else if ( pOrigin->m_nProtocol == PROTOCOL_G1 && pPacket->m_nProtocol == PROTOCOL_G2 )
		{
			if ( ! bWrapped ) return FALSE;
			pPacket = CG1Packet::New( (GNUTELLAPACKET*)( pPacket->m_pBuffer + pPacket->m_nPosition ) );
			pOrigin->Send( pPacket, TRUE, TRUE );
		}
		else if ( pOrigin->m_nProtocol == PROTOCOL_G2 && pPacket->m_nProtocol == PROTOCOL_G1 )
		{
			pPacket = CG2Packet::New( G2_PACKET_HIT_WRAP, (CG1Packet*)pPacket );
			pOrigin->Send( pPacket, TRUE, FALSE );	// Dont buffer
		}
		else
		{
			// Should not happen either (logic flaw)
			return FALSE;
		}
	}
	else if ( pPacket->m_nProtocol == PROTOCOL_G2 )
	{
		if ( pEndpoint.sin_addr.S_un.S_addr == Network.m_pHost.sin_addr.S_un.S_addr ) return FALSE;
		Datagrams.Send( &pEndpoint, (CG2Packet*)pPacket, FALSE );
	}
	else
	{
		if ( pEndpoint.sin_addr.S_un.S_addr == Network.m_pHost.sin_addr.S_un.S_addr ) return FALSE;
		pPacket = CG2Packet::New( G2_PACKET_HIT_WRAP, (CG1Packet*)pPacket );
		Datagrams.Send( &pEndpoint, (CG2Packet*)pPacket, TRUE );
	}
	
	if ( pPacket->m_nProtocol == PROTOCOL_G1 )
		Statistics.Current.Gnutella1.Routed++;
	else if ( pPacket->m_nProtocol == PROTOCOL_G2 )
		Statistics.Current.Gnutella2.Routed++;
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CNetwork common handler functions

void CNetwork::OnQuerySearch(CQuerySearch* pSearch)
{
	CSingleLock pLock( &theApp.m_pSection );
	
	if ( pLock.Lock( 10 ) )
	{
		if ( CMainWnd* pMainWnd = theApp.SafeMainWnd() )
		{
			CWindowManager* pWindows	= &pMainWnd->m_pWindows;
			CChildWnd* pChildWnd		= NULL;
			CRuntimeClass* pClass		= RUNTIME_CLASS(CSearchMonitorWnd);

			while ( pChildWnd = pWindows->Find( pClass, pChildWnd ) )
			{
				pChildWnd->OnQuerySearch( pSearch );
			}
		}

		pLock.Unlock();
	}
}

void CNetwork::OnQueryHits(CQueryHit* pHits)
{
	Downloads.OnQueryHits( pHits );

	CSingleLock pLock( &theApp.m_pSection );

	if ( pLock.Lock( 250 ) )
	{
		if ( CMainWnd* pMainWnd = theApp.SafeMainWnd() )
		{
			CWindowManager* pWindows	= &pMainWnd->m_pWindows;
			CChildWnd* pChildWnd		= NULL;
			CChildWnd* pMonitorWnd		= NULL;
			CRuntimeClass* pMonitorType	= RUNTIME_CLASS(CHitMonitorWnd);

			while ( pChildWnd = pWindows->Find( NULL, pChildWnd ) )
			{
				if ( pChildWnd->GetRuntimeClass() == pMonitorType )
				{
					pMonitorWnd = pChildWnd;
				}
				else
				{
					if ( pChildWnd->OnQueryHits( pHits ) ) return;
				}
			}

			if ( pMonitorWnd != NULL )
			{
				if ( pMonitorWnd->OnQueryHits( pHits ) ) return;
			}
		}

		pLock.Unlock();
	}

	pHits->Delete();
}
