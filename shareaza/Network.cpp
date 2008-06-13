//
// Network.cpp
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
#include "HttpRequest.h"
#include "UPnPFinder.h"

#include "CrawlSession.h"
#include "SearchManager.h"
#include "QueryHashMaster.h"
#include "QuerySearch.h"
#include "QueryHit.h"
#include "Buffer.h"
#include "G1Packet.h"
#include "G2Packet.h"
#include "GGEP.h"
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

CNetwork::CNetwork() :
	NodeRoute				( new CRouteCache() ),
	QueryRoute				( new CRouteCache() ),
	QueryKeys				( new CQueryKeys() ),
	m_bAutoConnect			( FALSE ),
	m_bTCPListeningReady	( FALSE ),
	m_bUDPListeningReady	( FALSE ),
	m_tStartedConnecting	( 0 ),
	m_tLastConnect			( 0 ),
	m_tLastED2KServerHop	( 0 ),
	m_nSequence				( 0 )
{
	ZeroMemory( &m_pHost, sizeof( m_pHost ) );
	m_pHost.sin_family		= AF_INET;
}

CNetwork::~CNetwork()
{
	delete QueryKeys;
	delete QueryRoute;
	delete NodeRoute;
}

//////////////////////////////////////////////////////////////////////
// CNetwork attributes

BOOL CNetwork::IsSelfIP(IN_ADDR nAddress) const
{
	IN_ADDR* nUPnPAddr = (IN_ADDR*)&theApp.m_nUPnPExternalAddress;
	if ( nUPnPAddr->S_un.S_addr == nAddress.S_un.S_addr )
		return TRUE;

	if ( nAddress.s_addr == INADDR_ANY || nAddress.s_addr == INADDR_NONE )
	{
		return FALSE;
	}
	if ( nAddress.s_addr == m_pHost.sin_addr.s_addr )
	{
		return TRUE;
	}
	if ( nAddress.s_net == 127 )
	{
		return TRUE;
	}
	return ( m_pHostAddresses.Find( nAddress.s_addr ) != NULL );
}

BOOL CNetwork::IsAvailable() const
{
	BOOL bIsAvailable = FALSE;
	HINSTANCE hWininet = LoadLibrary( _T("wininet.dll") );
	BOOL (WINAPI *pfnInternetGetConnectedState)(LPDWORD, DWORD);
	pfnInternetGetConnectedState = NULL;

	if ( hWininet != NULL )
		(FARPROC&)pfnInternetGetConnectedState = GetProcAddress( hWininet, "InternetGetConnectedState" );

	if ( pfnInternetGetConnectedState )
	{
		DWORD dwState = 0;

		__try
		{
			if ( (*pfnInternetGetConnectedState)( &dwState, 0 ) )
			{
				if ( ! ( dwState & INTERNET_CONNECTION_OFFLINE ) ) bIsAvailable = TRUE;
			}
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
		{
			// Something blocked WinAPI (for example application level firewall)
			bIsAvailable = FALSE;
		}
	}
	else
		bIsAvailable = TRUE; // Return always TRUE if the version of IE installed is too old and it doesn't support InternetGetConnectedState()

	if ( hWininet != NULL ) FreeLibrary( hWininet );

	return bIsAvailable;
}

bool CNetwork::IsConnected() const
{
	return IsThreadAlive();
}

bool CNetwork::IsListening() const
{
	return ( IsConnected() )
		&& ( m_pHost.sin_addr.S_un.S_addr != 0 )
		&& ( m_pHost.sin_port != 0 )
		&& ( Handshakes.IsListening() );
}

bool CNetwork::IsWellConnected() const
{
	return IsConnected() && ( Neighbours.m_nStableCount != 0 );
}

bool CNetwork::IsStable() const
{
#ifdef LAN_MODE
	return IsListening();
#else // LAN_MODE
	return IsListening() && IsWellConnected();
#endif // LAN_MODE
}

BOOL CNetwork::IsFirewalled(int nCheck) const
{
	if ( Settings.Connection.FirewallState == CONNECTION_OPEN )	// CHECK_BOTH, CHECK_TCP, CHECK_UDP
		return FALSE;		// We know we are not firewalled on both TCP and UDP
	else if ( Settings.Connection.FirewallState == CONNECTION_OPEN_TCPONLY && nCheck == CHECK_TCP )
		return FALSE;		// We know we are not firewalled on TCP port
	else if ( Settings.Connection.FirewallState == CONNECTION_OPEN_UDPONLY && nCheck == CHECK_UDP )
		return FALSE;		// We know we are not firewalled on UDP port
	else if ( Settings.Connection.FirewallState == CONNECTION_AUTO )
	{
		BOOL bTCPOpened = IsStable();
		BOOL bUDPOpened = Datagrams.IsStable();
		if( nCheck == CHECK_BOTH && bTCPOpened && bUDPOpened )
			return FALSE;	// We know we are not firewalled on both TCP and UDP
		else if ( nCheck == CHECK_TCP && bTCPOpened )
			return FALSE;	// We know we are not firewalled on TCP port
		else if ( nCheck == CHECK_UDP && bUDPOpened )
			return FALSE;	// We know we are not firewalled on UDP port
	}

	return TRUE;			// We know we are firewalled
}

DWORD CNetwork::GetStableTime() const
{
	if ( ! IsStable() || ! Handshakes.m_tStableTime ) return 0;
	return (DWORD)time( NULL ) - Handshakes.m_tStableTime;
}

BOOL CNetwork::IsConnectedTo(IN_ADDR* pAddress) const
{
	return IsSelfIP( *pAddress ) ||
		Handshakes.IsConnectedTo( pAddress ) ||
		Neighbours.Get( pAddress ) ||
		Transfers.IsConnectedTo( pAddress );
}

BOOL CNetwork::ReadyToTransfer(DWORD tNow) const
{
	if ( !Network.IsConnected() )
		return FALSE;

	// If a connection isn't needed for transfers, we can start any time
	if ( !Settings.Connection.RequireForTransfers )
		return TRUE;

	// If we have not started connecting, we're not ready to transfer.
	if ( m_tStartedConnecting == 0 )
		return FALSE;

	// We should wait a short time after starting the connection sequence before starting downloads
	if ( Settings.Connection.SlowConnect )
		return ( ( tNow - m_tStartedConnecting ) > 8000 );		// 8 seconds for XPsp2 users
	else
		return ( ( tNow - m_tStartedConnecting ) > 4000 );		// 4 seconds for others
}

//////////////////////////////////////////////////////////////////////
// CNetwork connection

BOOL CNetwork::Connect(BOOL bAutoConnect)
{
	CSingleLock pLock( &m_pSection, TRUE );

	Settings.Live.AutoClose = FALSE;

	if ( bAutoConnect ) 
	{
		m_bAutoConnect = TRUE;
	}

	// Make sure WinINet is connected (IE is not in offline mode)
	if ( Settings.Connection.ForceConnectedState )
	{
		INTERNET_CONNECTED_INFO ici = { 0 };
		HINTERNET hInternet = InternetOpen( Settings.SmartAgent(), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0 );

		ici.dwConnectedState = INTERNET_STATE_CONNECTED;
		InternetSetOption( hInternet, INTERNET_OPTION_CONNECTED_STATE, &ici, sizeof(ici) );
		InternetCloseHandle( hInternet );
	}

	// If we are already connected exit.
	if ( IsConnected() )
		return TRUE;

	// Begin network startup
	theApp.Message( MSG_NOTICE, IDS_NETWORK_STARTUP );

	gethostname( m_sHostName.GetBuffer( 255 ), 255 );
	m_sHostName.ReleaseBuffer();
	if( hostent* h = gethostbyname( m_sHostName ) )
	{
		for ( char** p = h->h_addr_list ; p && *p ; p++ )
		{
			m_pHostAddresses.AddTail( *(ULONG*)*p );
		}
	}

	Resolve( Settings.Connection.InHost, Settings.Connection.InPort, &m_pHost );

	if ( /*IsFirewalled()*/Settings.Connection.FirewallState == CONNECTION_FIREWALLED ) // Temp disable
		theApp.Message( MSG_INFO, IDS_NETWORK_FIREWALLED );

	SOCKADDR_IN pOutgoing;

	if ( Resolve( Settings.Connection.OutHost, 0, &pOutgoing ) )
	{
		theApp.Message( MSG_INFO, IDS_NETWORK_OUTGOING,
			(LPCTSTR)CString( inet_ntoa( pOutgoing.sin_addr ) ),
			htons( pOutgoing.sin_port ) );
	}
	else if ( Settings.Connection.OutHost.GetLength() )
	{
		theApp.Message( MSG_ERROR, IDS_NETWORK_CANT_OUTGOING,
			(LPCTSTR)Settings.Connection.OutHost );
	}

	m_bTCPListeningReady = Handshakes.Listen();
	m_bUDPListeningReady = Datagrams.Listen();

	if ( !m_bTCPListeningReady || !m_bUDPListeningReady )
	{
		theApp.Message( MSG_ERROR, _T("The connection process is failed.") );
		Handshakes.Disconnect();
		Datagrams.Disconnect();
		m_pHostAddresses.RemoveAll();
		return FALSE;
	}

	Neighbours.Connect();

	NodeRoute->SetDuration( Settings.Gnutella.RouteCache );
	QueryRoute->SetDuration( Settings.Gnutella.RouteCache );

	m_tStartedConnecting	= GetTickCount();
	BeginThread( "Network" );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CNetwork disconnect

void CNetwork::Disconnect()
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( ! IsConnected() ) return;

	theApp.Message( MSG_INFO, _T("") );
	theApp.Message( MSG_NOTICE, IDS_NETWORK_DISCONNECTING );

	m_bAutoConnect			= FALSE;
	m_bTCPListeningReady	= FALSE;
	m_bUDPListeningReady	= FALSE;
	m_tStartedConnecting	= 0;

	Neighbours.Close();

	pLock.Unlock();

	CloseThread();

	Handshakes.Disconnect();
	pLock.Lock();

	Neighbours.Close();
	Datagrams.Disconnect();

	NodeRoute->Clear();
	QueryRoute->Clear();

	{
		for ( POSITION pos = m_pLookups.GetStartPosition() ; pos ; )
		{
			HANDLE pAsync;
			ResolveStruct* pBuffer;
			m_pLookups.GetNextAssoc( pos, pAsync, pBuffer );
			WSACancelAsyncRequest( pAsync );
			delete pBuffer->m_sAddress;
			delete pBuffer;
		}

		m_pLookups.RemoveAll();
	}

	m_pHostAddresses.RemoveAll();

	pLock.Unlock();

	DiscoveryServices.Stop();

	theApp.Message( MSG_NOTICE, IDS_NETWORK_DISCONNECTED ); 
	theApp.Message( MSG_NOTICE, _T("") );
}

//////////////////////////////////////////////////////////////////////
// CNetwork host connection

BOOL CNetwork::ConnectTo(LPCTSTR pszAddress, int nPort, PROTOCOLID nProtocol, BOOL bNoUltraPeer)
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	if ( ! IsConnected() && ! Connect() ) return FALSE;
	
	if ( nPort == 0 ) nPort = GNUTELLA_DEFAULT_PORT;
	theApp.Message( MSG_INFO, IDS_NETWORK_RESOLVING, pszAddress );
	
	if ( AsyncResolve( pszAddress, (WORD)nPort, nProtocol, bNoUltraPeer ? 2 : 1 ) ) return TRUE;
	
	theApp.Message( MSG_ERROR, IDS_NETWORK_RESOLVE_FAIL, pszAddress );
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CNetwork local IP acquisition and sending

void CNetwork::AcquireLocalAddress(LPCTSTR pszHeader)
{
	int nIPb1, nIPb2, nIPb3, nIPb4;
	
	if ( _stscanf( pszHeader, _T("%i.%i.%i.%i"), &nIPb1, &nIPb2, &nIPb3, &nIPb4 ) != 4 ||
		nIPb1 < 0 || nIPb1 > 255 ||
		nIPb2 < 0 || nIPb2 > 255 ||
		nIPb3 < 0 || nIPb3 > 255 ||
		nIPb4 < 0 || nIPb4 > 255 )
		return;
	
	IN_ADDR pAddress;
	
	pAddress.S_un.S_un_b.s_b1 = (BYTE)nIPb1;
	pAddress.S_un.S_un_b.s_b2 = (BYTE)nIPb2;
	pAddress.S_un.S_un_b.s_b3 = (BYTE)nIPb3;
	pAddress.S_un.S_un_b.s_b4 = (BYTE)nIPb4;
	
	if ( IsFirewalledAddress( &pAddress, FALSE, TRUE ) ) return;
	
	// Add new address to address list
	if ( ! m_pHostAddresses.Find( pAddress.s_addr ) )
		m_pHostAddresses.AddTail( pAddress.s_addr );
	
	m_pHost.sin_addr = pAddress;
}

//////////////////////////////////////////////////////////////////////
// CNetwork GGUID generation

void CNetwork::CreateID(Hashes::Guid& oID)
{
	VERIFY( SUCCEEDED( CoCreateGuid( reinterpret_cast<GUID*>( &oID[ 0 ] ) ) ) );
	VERIFY( oID.validate() );
}

//////////////////////////////////////////////////////////////////////
// CNetwork name resolution

BOOL CNetwork::Resolve(LPCTSTR pszHost, int nPort, SOCKADDR_IN* pHost, BOOL bNames) const
{
	ZeroMemory( pHost, sizeof(*pHost) );
	pHost->sin_family	= PF_INET;
	pHost->sin_port		= htons( u_short( nPort ) );
	
	if ( pszHost == NULL || *pszHost == 0 ) return FALSE;
	
	CString strHost( pszHost );
	
	int nColon = strHost.Find( ':' );
	
	if ( nColon >= 0 )
	{
		if ( _stscanf( strHost.Mid( nColon + 1 ), _T("%i"), &nPort ) == 1 )
		{
			pHost->sin_port = htons( u_short( nPort ) );
		}
		
		strHost = strHost.Left( nColon );
	}
	
	CT2CA pszaHost( (LPCTSTR)strHost );
	
	DWORD dwIP = inet_addr( pszaHost );
	
	if ( dwIP == INADDR_NONE )
	{
		if ( ! bNames ) return TRUE;
		
		HOSTENT* pLookup = gethostbyname( pszaHost );
		
		if ( pLookup == NULL ) return FALSE;
		
		CopyMemory( &pHost->sin_addr, pLookup->h_addr, sizeof pHost->sin_addr );
	}
	else
	{
		CopyMemory( &pHost->sin_addr, &dwIP, sizeof pHost->sin_addr );
	}
	
	return TRUE;
}

BOOL CNetwork::AsyncResolve(LPCTSTR pszAddress, WORD nPort, PROTOCOLID nProtocol, BYTE nCommand)
{
	CSingleLock pLock( &m_pSection );
	if ( ! pLock.Lock( 250 ) ) return FALSE;
	
	ResolveStruct* pResolve = new ResolveStruct;
	
	HANDLE hAsync = WSAAsyncGetHostByName( AfxGetMainWnd()->GetSafeHwnd(), WM_WINSOCK,
		CT2CA(pszAddress), pResolve->m_pBuffer, MAXGETHOSTSTRUCT );
	
	if ( hAsync != NULL )
	{
		pResolve->m_sAddress = new CString( pszAddress );
		pResolve->m_nProtocol = nProtocol;
		pResolve->m_nPort = nPort;
		pResolve->m_nCommand = nCommand;

		m_pLookups.SetAt( hAsync, pResolve );
		return TRUE;
	}
	else
	{
		delete pResolve;
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// CNetwork firewalled address checking

BOOL CNetwork::IsFirewalledAddress(LPVOID pAddress, BOOL bIncludeSelf, BOOL bForceCheck)
{
	if ( ! pAddress ) return TRUE;
	if ( bIncludeSelf && IsSelfIP( *(IN_ADDR*)pAddress ) ) return TRUE;
	if ( ! *(DWORD*)pAddress ) return TRUE;							// 0.0.0.0
#ifdef LAN_MODE
	UNUSED_ALWAYS( bForceCheck );
	if ( ( *(DWORD*)pAddress & 0xFFFF ) == 0xA8C0 ) return FALSE;	// 192.168.0.0/16
	if ( ( *(DWORD*)pAddress & 0xF0FF ) == 0x10AC ) return FALSE;	// 172.16.0.0/12
	if ( ( *(DWORD*)pAddress & 0xFF ) == 0x0A ) return FALSE;		// 10.0.0.0/8
	return TRUE;
#else // LAN_MODE
	if ( ! bForceCheck && ! Settings.Connection.IgnoreLocalIP ) return FALSE;
	if ( ( *(DWORD*)pAddress & 0xFFFF ) == 0xA8C0 ) return TRUE;	// 192.168.0.0/16
	if ( ( *(DWORD*)pAddress & 0xF0FF ) == 0x10AC ) return TRUE;	// 172.16.0.0/12
	if ( ( *(DWORD*)pAddress & 0xFF ) == 0x0A ) return TRUE;		// 10.0.0.0/8
	if ( ( *(DWORD*)pAddress & 0xFF ) == 0x7F ) return TRUE;		// 127.0.0.0/8
	return FALSE;
#endif // LAN_MODE
}

// Returns TRUE if the IP address is reserved.
// Private addresses are treated as reserved when Connection.IgnoreLocalIP = TRUE.
// The code is based on nmap code and updated according to
// http://www.cymru.com/Documents/bogon-bn-nonagg.txt
// and http://www.iana.org/assignments/ipv4-address-space

BOOL CNetwork::IsReserved(IN_ADDR* pAddress, bool bCheckLocal)
{
	char *ip = (char*)&(pAddress->s_addr);
	unsigned char i1 = ip[ 0 ], i2 = ip[ 1 ], i3 = ip[ 2 ], i4 = ip[ 3 ];

	switch ( i1 )
	{
		case 0:         // 000/8 is IANA reserved
		case 1:         // 001/8 is IANA reserved       
		case 2:         // 002/8 is IANA reserved       
		case 5:         // 005/8 is IANA reserved       
		case 6:         // USA Army ISC                 
		case 7:         // used for BGP protocol        
		case 23:        // 023/8 is IANA reserved       
		case 27:        // 027/8 is IANA reserved       
		case 31:        // 031/8 is IANA reserved       
		case 36:        // 036/8 is IANA reserved       
		case 37:        // 037/8 is IANA reserved       
		case 39:        // 039/8 is IANA reserved       
		case 42:        // 042/8 is IANA reserved       
		case 49:        // 049/8 is IANA reserved       
		case 50:        // 050/8 is IANA reserved       
		case 55:        // misc. USA Armed forces    
		case 127:       // 127/8 is reserved for loopback 
		case 197:       // 197/8 is IANA reserved       
		case 223:       // 223/8 is IANA reserved       
			return TRUE;
		case 10:        // Private addresses
			return bCheckLocal && Settings.Connection.IgnoreLocalIP;
		default:
			break;
	}

	// 077-079/8 is IANA reserved 
	if ( i1 >= 77 && i1 <= 79 ) return TRUE;

	// 092-123/8 is IANA reserved 
	if ( i1 >= 92 && i1 <= 120 ) return TRUE;

	// 172.16.0.0/12 is reserved for private nets by RFC1819 
	if ( i1 == 172 && i2 >= 16 && i2 <= 31 ) 
		return bCheckLocal && Settings.Connection.IgnoreLocalIP;

	// 173-187/8 is IANA reserved 
	if ( i1 >= 173 && i1 <= 187 ) return TRUE;

	// 192.168.0.0/16 is reserved for private nets by RFC1819 
	// 192.0.2.0/24 is reserved for documentation and examples 
	// 192.88.99.0/24 is used as 6to4 Relay anycast prefix by RFC3068 
	if ( i1 == 192 )
	{
		if ( i2 == 168 ) return bCheckLocal && Settings.Connection.IgnoreLocalIP;
		if ( i2 == 0 && i3 == 2 ) return TRUE;
		if ( i2 == 88 && i3 == 99 ) return TRUE;
	}

	// 198.18.0.0/15 is used for benchmark tests by RFC2544 
	if ( i1 == 198 && i2 == 18 && i3 >= 1 && i3 <= 64 ) return TRUE;

	// reserved for DHCP clients seeking addresses, not routable outside LAN 
	if ( i1 == 169 && i2 == 254 ) return TRUE;

	// 204.152.64.0/23 is some Sun proprietary clustering thing 
	if ( i1 == 204 && i2 == 152 && ( i3 == 64 || i3 == 65 ) )
		return TRUE;

	// 224-239/8 is all multicast stuff 
	// 240-255/8 is IANA reserved 
	if ( i1 >= 224 ) return TRUE;

	// 255.255.255.255, we already tested for i1 
	if ( i2 == 255 && i3 == 255 && i4 == 255 ) return TRUE;

	return FALSE;
}

WORD CNetwork::RandomPort() const
{
	return WORD( 10000 + ( rand() % 50000 ) );
}

//////////////////////////////////////////////////////////////////////
// CNetwork thread run

void CNetwork::OnRun()
{
	Neighbours.IsG2HubCapable( TRUE );
	Neighbours.IsG1UltrapeerCapable( TRUE );

	// It will check if it is needed inside the function
	DiscoveryServices.Execute( TRUE, PROTOCOL_NULL, FALSE );

	while ( IsThreadEnabled() )
	{
		HostCache.PruneOldHosts();

		Sleep( 50 );
		Doze( 100 );
	
		if ( !theApp.m_bLive )
			continue;

		if ( theApp.m_pUPnPFinder && theApp.m_pUPnPFinder->IsAsyncFindRunning() )
			continue;

		if ( IsThreadEnabled() && m_pSection.Lock() )
		{
			Datagrams.OnRun();
			SearchManager.OnRun();
			QueryHashMaster.Build();
			
			if ( CrawlSession.m_bActive )
				CrawlSession.OnRun();
			
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

	ResolveStruct* pResolve = NULL;
	if ( ! m_pLookups.Lookup( (HANDLE)wParam, pResolve ) ) return;
	m_pLookups.RemoveKey( (HANDLE)wParam );

	CString strAddress;
	CDiscoveryService* pService;

	if ( WSAGETASYNCERROR(lParam) == 0 )
	{
		if ( pResolve->m_nCommand == 0 )
		{
			HostCache.ForProtocol( pResolve->m_nProtocol )->Add( (IN_ADDR*)pResolve->m_pHost.h_addr, pResolve->m_nPort );
		}
		else if ( pResolve->m_nCommand == 1 || pResolve->m_nCommand == 2 )
		{
			Neighbours.ConnectTo( (IN_ADDR*)pResolve->m_pHost.h_addr, pResolve->m_nPort, pResolve->m_nProtocol, FALSE, pResolve->m_nCommand );
		}
		else if ( pResolve->m_nCommand == 3 )
		{
			// code to invoke UDPHC/UDPKHL Sender.
			if ( pResolve->m_nProtocol == PROTOCOL_G1 )
			{
				strAddress = L"uhc:" + *(pResolve->m_sAddress);
				pService = DiscoveryServices.GetByAddress( strAddress );
				if ( pService == NULL )
				{
					strAddress.AppendFormat(_T(":%u"), pResolve->m_nPort );
					pService = DiscoveryServices.GetByAddress( strAddress );
				}

				if ( pService != NULL )
				{
					pService->m_pAddress = *((IN_ADDR*)pResolve->m_pHost.h_addr);
					pService->m_nPort =  pResolve->m_nPort;
				}
				UDPHostCache((IN_ADDR*)pResolve->m_pHost.h_addr, pResolve->m_nPort);
			}
			else if ( pResolve->m_nProtocol == PROTOCOL_G2 )
			{
				strAddress = L"ukhl:" + *(pResolve->m_sAddress);
				pService = DiscoveryServices.GetByAddress( strAddress );
				if ( pService == NULL )
				{
					strAddress.AppendFormat(_T(":%u"), pResolve->m_nPort );
					pService = DiscoveryServices.GetByAddress( strAddress );
				}

				if ( pService != NULL )
				{
					pService->m_pAddress =  *((IN_ADDR*)pResolve->m_pHost.h_addr);
					pService->m_nPort =  pResolve->m_nPort;
				}
				UDPKnownHubCache((IN_ADDR*)pResolve->m_pHost.h_addr, pResolve->m_nPort);
			}
		}
	}
	else if ( pResolve->m_nCommand == 0 )
	{
		theApp.Message( MSG_ERROR, IDS_NETWORK_RESOLVE_FAIL, LPCTSTR( *pResolve->m_sAddress ) );
	}
	else
	{
		if ( pResolve->m_nCommand == 3 )
		{
			if ( pResolve->m_nProtocol == PROTOCOL_G1 )
			{
				strAddress = L"uhc:" + *(pResolve->m_sAddress);
				pService = DiscoveryServices.GetByAddress( strAddress );
				if ( pService == NULL )
				{
					strAddress.AppendFormat(_T(":%u"), pResolve->m_nPort );
					pService = DiscoveryServices.GetByAddress( strAddress );
				}

				if ( pService != NULL )
				{
					pService->OnFailure();
				}
			}
			else if ( pResolve->m_nProtocol == PROTOCOL_G2 )
			{
				strAddress = L"ukhl:" + *(pResolve->m_sAddress);
				pService = DiscoveryServices.GetByAddress( strAddress );
				if ( pService == NULL )
				{
					strAddress.AppendFormat(_T(":%u"), pResolve->m_nPort );
					pService = DiscoveryServices.GetByAddress( strAddress );
				}

				if ( pService != NULL )
				{
					pService->OnFailure();
				}
			}
		}

	}

	delete pResolve->m_sAddress;
	delete pResolve;
}

//////////////////////////////////////////////////////////////////////
// CNetwork get node route

BOOL CNetwork::GetNodeRoute(const Hashes::Guid& oGUID, CNeighbour** ppNeighbour, SOCKADDR_IN* pEndpoint)
{
	if ( validAndEqual( oGUID, Hashes::Guid( MyProfile.oGUID ) ) ) return FALSE;
	
	if ( Network.NodeRoute->Lookup( oGUID, ppNeighbour, pEndpoint ) ) return TRUE;
	if ( ppNeighbour == NULL ) return FALSE;
	
	for ( POSITION pos = Neighbours.GetIterator() ; pos ; )
	{
		CNeighbour* pNeighbour = Neighbours.GetNext( pos );
		
		if ( validAndEqual( pNeighbour->m_oGUID, oGUID ) )
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
	Hashes::Guid oGUID;
	
	if ( ! pPacket->GetTo( oGUID ) || validAndEqual( oGUID, Hashes::Guid( MyProfile.oGUID ) ) ) return FALSE;
	
	CNeighbour* pOrigin = NULL;
	SOCKADDR_IN pEndpoint;
	
	if ( GetNodeRoute( oGUID, &pOrigin, &pEndpoint ) )
	{
		if ( pOrigin != NULL )
		{
			if ( pOrigin->m_nProtocol == PROTOCOL_G1 &&
				 pPacket->IsType( G2_PACKET_PUSH ) )
			{
				CG1Neighbour* pG1 = (CG1Neighbour*)pOrigin;
				pPacket->SkipCompound();
				pG1->SendG2Push( oGUID, pPacket );
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

BOOL CNetwork::SendPush(const Hashes::Guid& oGUID, DWORD nIndex)
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return TRUE;

	if ( ! IsListening() ) return FALSE;
	
	Hashes::Guid oGUID2 = oGUID;
	SOCKADDR_IN pEndpoint;
	CNeighbour* pOrigin;
	int nCount = 0;
	
	while ( GetNodeRoute( oGUID2, &pOrigin, &pEndpoint ) )
	{
		if ( pOrigin != NULL && pOrigin->m_nProtocol == PROTOCOL_G1 )
		{
			CG1Packet* pPacket = CG1Packet::New( G1_PACKET_PUSH,
				Settings.Gnutella1.MaximumTTL - 1 );
			
			pPacket->Write( oGUID );
			pPacket->WriteLongLE( nIndex );
			pPacket->WriteLongLE( m_pHost.sin_addr.S_un.S_addr );
			pPacket->WriteShortLE( htons( m_pHost.sin_port ) );
			
			pOrigin->Send( pPacket );
		}
		else
		{
			CG2Packet* pPacket = CG2Packet::New( G2_PACKET_PUSH, TRUE );
			
			pPacket->WritePacket( G2_PACKET_TO, 16 );
			pPacket->Write( oGUID );
			
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
		
		oGUID2[15] ++;
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
	
	if ( ! QueryRoute->Lookup( pHits->m_oSearchID, &pOrigin, &pEndpoint ) ) return FALSE;
	
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
		if ( Network.IsSelfIP( pEndpoint.sin_addr ) ) return FALSE;
		Datagrams.Send( &pEndpoint, (CG2Packet*)pPacket, FALSE );
	}
	else
	{
		if ( Network.IsSelfIP( pEndpoint.sin_addr ) ) return FALSE;
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
			CRuntimeClass* pClass		= RUNTIME_CLASS(CSearchMonitorWnd);
			CChildWnd* pChildWnd		= NULL;

			while ( ( pChildWnd = pWindows->Find( pClass, pChildWnd ) ) != NULL )
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
			CChildWnd* pMonitorWnd		= NULL;
			CRuntimeClass* pMonitorType	= RUNTIME_CLASS(CHitMonitorWnd);
			CChildWnd* pChildWnd		= NULL;

			while ( ( pChildWnd = pWindows->Find( NULL, pChildWnd ) ) != NULL )
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

void CNetwork::UDPHostCache(IN_ADDR* pAddress, WORD nPort)
{
	CG1Packet* pPing = CG1Packet::New( G1_PACKET_PING, 1, Hashes::Guid( MyProfile.oGUID ) );

	CGGEPBlock pBlock;
	CGGEPItem* pItem;
	
	pItem = pBlock.Add( GGEP_HEADER_SUPPORT_CACHE_PONGS );
	pItem->UnsetCOBS();
	pItem->UnsetSmall();
	pItem->WriteByte( Neighbours.IsG1Ultrapeer() ? 1 : 0 );

	pBlock.Write( pPing );
	Datagrams.Send( pAddress, nPort, pPing, TRUE, NULL, FALSE );
}

void CNetwork::UDPKnownHubCache(IN_ADDR* pAddress, WORD nPort)
{
	CG2Packet* pKHLR = CG2Packet::New( G2_PACKET_KHL_REQ );
	Datagrams.Send( pAddress, nPort, pKHLR, TRUE, NULL, FALSE );
}
