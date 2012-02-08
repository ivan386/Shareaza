//
// Network.cpp
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
#include "ChatCore.h"
#include "CrawlSession.h"
#include "Datagrams.h"
#include "DiscoveryServices.h"
#include "Downloads.h"
#include "Firewall.h"
#include "G1Neighbour.h"
#include "G1Packet.h"
#include "G2Packet.h"
#include "GProfile.h"
#include "Handshakes.h"
#include "HostCache.h"
#include "Library.h"
#include "LocalSearch.h"
#include "Neighbours.h"
#include "Network.h"
#include "QueryHashMaster.h"
#include "QueryHit.h"
#include "QueryKeys.h"
#include "RouteCache.h"
#include "SearchManager.h"
#include "Settings.h"
#include "Statistics.h"
#include "Transfers.h"
#include "UPnPFinder.h"
#include "WndHitMonitor.h"
#include "WndMain.h"
#include "WndSearch.h"
#include "WndSearchMonitor.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CNetwork Network;

IMPLEMENT_DYNCREATE(CNetwork, CComObject)

//////////////////////////////////////////////////////////////////////
// CNetwork construction

CNetwork::CNetwork()
	: NodeRoute				( new CRouteCache() )
	, QueryRoute			( new CRouteCache() )
	, QueryKeys				( new CQueryKeys() )
	, UPnPFinder			( new CUPnPFinder() )
	, Firewall				( new CFirewall() )
	, m_pHost				()
	, m_bAutoConnect		( FALSE )
	, m_bConnected			( false )
	, m_tStartedConnecting	( 0 )
	, m_tLastConnect		( 0 )
	, m_tLastED2KServerHop	( 0 )
	, m_nSequence			( 0 )
	, m_bUPnPPortsForwarded	( TRI_UNKNOWN )
	, m_nUPnPExternalAddress()
	, m_tUPnPMap			( 0 )
{
	m_pHost.sin_family = AF_INET;
}

CNetwork::~CNetwork()
{
}

BOOL CNetwork::Init()
{
	// Start Windows Sockets 1.1
	WSADATA wsaData = {};
	for ( int i = 1; i <= 2; i++ )
	{
		if ( WSAStartup( MAKEWORD( 1, 1 ), &wsaData ) )
			return FALSE;
		if ( MAKEWORD( HIBYTE ( wsaData.wVersion ), LOBYTE( wsaData.wVersion ) ) >= MAKEWORD( 1, 1 ) )
			break;
		if ( i == 2 )
			return FALSE;
		Cleanup();
	}

	// Setup Windows Firewall for Shareaza itself and for UPnP service
	if ( Firewall->Init() &&
		 ( Settings.Connection.EnableUPnP ||
		   Settings.Connection.EnableFirewallException ) )
	{
		if ( Firewall->AreExceptionsAllowed() )
		{
			if ( Settings.Connection.EnableUPnP )
			{
				if ( Firewall->SetupService( NET_FW_SERVICE_UPNP ) )
					theApp.Message( MSG_INFO, _T("Windows Firewall setup for UPnP succeeded.") );
				else
					theApp.Message( MSG_ERROR, _T("Windows Firewall setup for UPnP failed.") );
			}

			if ( Settings.Connection.EnableFirewallException )
			{
				if ( Firewall->SetupProgram( theApp.m_strBinaryPath, CLIENT_NAME_T, FALSE ) )
					theApp.Message( MSG_INFO, _T("Windows Firewall setup for application succeeded.") );
				else
					theApp.Message( MSG_ERROR, _T("Windows Firewall setup for application failed.") );
			}
		}
		else
			theApp.Message( MSG_INFO, _T("Windows Firewall is not available.") );
	}

	return TRUE;
}

void CNetwork::Clear()
{
	// Remove port mappings
	DeletePorts();

	// Remove application from the firewall exception list
	if ( Settings.Connection.DeleteFirewallException )
		Firewall->SetupProgram( theApp.m_strBinaryPath, CLIENT_NAME_T, TRUE );

	Firewall.Free();
	UPnPFinder.Free();
	QueryKeys.Free();
	QueryRoute.Free();
	NodeRoute.Free();

	Cleanup();
}

//////////////////////////////////////////////////////////////////////
// CNetwork attributes

BOOL CNetwork::IsSelfIP(const IN_ADDR& nAddress) const
{
	if ( nAddress.s_addr == INADDR_ANY ||
		 nAddress.s_addr == INADDR_NONE )
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
	if ( m_nUPnPExternalAddress.s_addr != INADDR_NONE &&
		 m_nUPnPExternalAddress.s_addr == nAddress.s_addr )
	{
		return TRUE;
	}

	CQuickLock oHALock( m_pHASection );
	return ( m_pHostAddresses.Find( nAddress.s_addr ) != NULL );
}

bool CNetwork::InternetConnect()
{
	__try
	{
		return ( InternetAttemptConnect( 0 ) == ERROR_SUCCESS );
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		// Something blocked WinAPI (for example application level firewall)
	}

	return false;
}

bool CNetwork::IsAvailable() const
{
	DWORD dwState = 0ul;

	__try
	{
		if ( InternetGetConnectedState( &dwState, 0 ) )
		{
			if ( !( dwState & INTERNET_CONNECTION_OFFLINE ) )
				return true;
		}
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		// Something blocked WinAPI (for example application level firewall)
	}

	return false;
}

bool CNetwork::IsConnected() const
{
	return m_bConnected;
}

bool CNetwork::IsListening() const
{
	return ( IsConnected() )
		&& ( m_pHost.sin_addr.S_un.S_addr != 0 )
		&& ( m_pHost.sin_port != 0 )
		&& ( Handshakes.IsValid() )
		&& ( Datagrams.IsValid() );
}

bool CNetwork::IsWellConnected() const
{
	return IsConnected() && ( Neighbours.GetStableCount() != 0 );
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
#ifdef LAN_MODE
	UNUSED_ALWAYS( nCheck );
	return FALSE;
#else // LAN_MODE
	if ( Settings.Connection.FirewallState == CONNECTION_OPEN )	// CHECK_BOTH, CHECK_TCP, CHECK_UDP
		return FALSE;		// We know we are not firewalled on both TCP and UDP
	else if ( nCheck == CHECK_IP )
		return IsFirewalledAddress( &Network.m_pHost.sin_addr ) || IsReserved( &Network.m_pHost.sin_addr );
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
#endif // LAN_MODE
}

DWORD CNetwork::GetStableTime() const
{
	return IsStable() ? Handshakes.GetStableTime() : 0;
}

BOOL CNetwork::IsConnectedTo(const IN_ADDR* pAddress) const
{
	return IsSelfIP( *pAddress ) ||
		Handshakes.IsConnectedTo( pAddress ) ||
		Neighbours.Get( *pAddress ) ||
		Transfers.IsConnectedTo( pAddress );
}

BOOL CNetwork::ReadyToTransfer(DWORD tNow) const
{
	if ( !IsConnected() )
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
	if ( theApp.m_bClosing )
		return FALSE;

	CSingleLock pLock( &m_pSection, TRUE );

	if ( bAutoConnect )
	{
		m_bAutoConnect = TRUE;
	}

	// If we are already connected exit.
	if ( IsConnected() )
		return TRUE;

	m_tStartedConnecting	= GetTickCount();
	BeginThread( "Network" );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CNetwork disconnect

void CNetwork::Disconnect()
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( !IsConnected() )
		return;

	theApp.Message( MSG_INFO, _T("") );
	theApp.Message( MSG_NOTICE, IDS_NETWORK_DISCONNECTING );

	m_bAutoConnect			= FALSE;
	m_tStartedConnecting	= 0;

	pLock.Unlock();

	CloseThread();
}

//////////////////////////////////////////////////////////////////////
// CNetwork host connection

BOOL CNetwork::ConnectTo(LPCTSTR pszAddress, int nPort, PROTOCOLID nProtocol, BOOL bNoUltraPeer)
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( ! IsConnected() && ! Connect() )
		return FALSE;

	if ( nPort <= 0 || nPort > USHRT_MAX )
	{
		nPort = protocolPorts[ ( nProtocol == PROTOCOL_ANY ) ? PROTOCOL_NULL : nProtocol ];
	}

	return AsyncResolve( pszAddress, (WORD)nPort, nProtocol, bNoUltraPeer ? 2 : 1 );
}

//////////////////////////////////////////////////////////////////////
// CNetwork local IP acquisition and sending

BOOL CNetwork::AcquireLocalAddress(SOCKET hSocket)
{
	// Ask the socket what it thinks our IP address on this end is
	SOCKADDR_IN pAddress = {};
	int nSockLen = sizeof( pAddress );
	if ( getsockname( hSocket, (SOCKADDR*)&pAddress, &nSockLen ) != 0 )
		return FALSE;

	return AcquireLocalAddress( pAddress.sin_addr );
}

BOOL CNetwork::AcquireLocalAddress(LPCTSTR pszHeader)
{
	int nIPb1, nIPb2, nIPb3, nIPb4;
	if ( _stscanf( pszHeader, _T("%i.%i.%i.%i"), &nIPb1, &nIPb2, &nIPb3, &nIPb4 ) != 4 ||
		nIPb1 < 0 || nIPb1 > 255 ||
		nIPb2 < 0 || nIPb2 > 255 ||
		nIPb3 < 0 || nIPb3 > 255 ||
		nIPb4 < 0 || nIPb4 > 255 )
		return FALSE;

	IN_ADDR pAddress;
	pAddress.S_un.S_un_b.s_b1 = (BYTE)nIPb1;
	pAddress.S_un.S_un_b.s_b2 = (BYTE)nIPb2;
	pAddress.S_un.S_un_b.s_b3 = (BYTE)nIPb3;
	pAddress.S_un.S_un_b.s_b4 = (BYTE)nIPb4;
	return AcquireLocalAddress( pAddress );
}

BOOL CNetwork::AcquireLocalAddress(const IN_ADDR& pAddress)
{
	if ( pAddress.s_addr == INADDR_ANY ||
		 pAddress.s_addr == INADDR_NONE )
		return FALSE;

	CQuickLock oHALock( m_pHASection );

	// Add new address to address list
	if ( ! m_pHostAddresses.Find( pAddress.s_addr ) )
		m_pHostAddresses.AddTail( pAddress.s_addr );

	if ( IsFirewalledAddress( &pAddress ) )
		return FALSE;

	// Allow real IP only
	if ( m_pHost.sin_addr.s_addr != pAddress.s_addr )
	{
		m_pHost.sin_addr.s_addr = pAddress.s_addr;
	}

	return TRUE;
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

BOOL CNetwork::Resolve(LPCTSTR pszHost, int nPort, SOCKADDR_IN* pHost, BOOL bNames)
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
	theApp.Message( MSG_INFO, IDS_NETWORK_RESOLVING, pszAddress );

	auto_ptr< ResolveStruct > pResolve( new ResolveStruct );
	if ( pResolve.get() )
	{
		pResolve->m_sAddress = pszAddress;
		pResolve->m_nProtocol = nProtocol;
		pResolve->m_nPort = nPort;
		pResolve->m_nCommand = nCommand;

		HANDLE hAsync = WSAAsyncGetHostByName( AfxGetMainWnd()->GetSafeHwnd(), WM_WINSOCK,
			CT2CA( pszAddress ), pResolve->m_pBuffer, MAXGETHOSTSTRUCT );
		if ( hAsync )
		{
			CQuickLock pLock( m_pLookupsSection );
			m_pLookups.SetAt( hAsync, pResolve.release() );
			DEBUG_ONLY( theApp.Message( MSG_DEBUG, _T("Network name resolver has %u pending requests."), m_pLookups.GetCount() ) );
			return TRUE;
		}
	}

	theApp.Message( MSG_ERROR, IDS_NETWORK_RESOLVE_FAIL, pszAddress );

	return FALSE;
}

UINT CNetwork::GetResolveCount() const
{
	CQuickLock pLock( m_pLookupsSection );

	return m_pLookups.GetCount();
}

CNetwork::ResolveStruct* CNetwork::GetResolve(HANDLE hAsync)
{
	CQuickLock pLock( m_pLookupsSection );

	ResolveStruct* pResolve = NULL;
	if ( m_pLookups.Lookup( hAsync, pResolve ) )
		m_pLookups.RemoveKey( hAsync );

	DEBUG_ONLY( theApp.Message( MSG_DEBUG, _T("Network name resolver has %u pending requests."), m_pLookups.GetCount() ) );

	return pResolve;
}

void CNetwork::ClearResolve()
{
	CQuickLock pLock( m_pLookupsSection );

	for ( POSITION pos = m_pLookups.GetStartPosition() ; pos ; )
	{
		HANDLE pAsync;
		ResolveStruct* pResolve;
		m_pLookups.GetNextAssoc( pos, pAsync, pResolve );
		WSACancelAsyncRequest( pAsync );
		delete pResolve;
	}
	m_pLookups.RemoveAll();
}

// CNetwork firewalled address checking

BOOL CNetwork::IsFirewalledAddress(const IN_ADDR* pAddress, BOOL bIncludeSelf) const
{
	if ( ! pAddress ) return TRUE;
	if ( bIncludeSelf && IsSelfIP( *pAddress ) ) return TRUE;
	if ( ! pAddress->S_un.S_addr ) return TRUE;							// 0.0.0.0
#ifdef LAN_MODE
	if ( ( pAddress->S_un.S_addr & 0xFFFF ) == 0xA8C0 ) return FALSE;	// 192.168.0.0/16
	if ( ( pAddress->S_un.S_addr & 0xFFFF ) == 0xFEA9 ) return FALSE;	// 169.254.0.0/16
	if ( ( pAddress->S_un.S_addr & 0xF0FF ) == 0x10AC ) return FALSE;	// 172.16.0.0/12
	if ( ( pAddress->S_un.S_addr & 0xFF ) == 0x0A ) return FALSE;		// 10.0.0.0/8
	return TRUE;
#else // LAN_MODE
	if ( ! Settings.Connection.IgnoreLocalIP ) return FALSE;
	if ( ( pAddress->S_un.S_addr & 0xFFFF ) == 0xA8C0 ) return TRUE;	// 192.168.0.0/16
	if ( ( pAddress->S_un.S_addr & 0xFFFF ) == 0xFEA9 ) return TRUE;	// 169.254.0.0/16
	if ( ( pAddress->S_un.S_addr & 0xF0FF ) == 0x10AC ) return TRUE;	// 172.16.0.0/12
	if ( ( pAddress->S_un.S_addr & 0xFF ) == 0x0A ) return TRUE;		// 10.0.0.0/8
	if ( ( pAddress->S_un.S_addr & 0xFF ) == 0x7F ) return TRUE;		// 127.0.0.0/8
	return FALSE;
#endif // LAN_MODE
}

// Get external incoming port (in host byte order)

WORD CNetwork::GetPort() const
{
	return m_pHost.sin_port ? ntohs( m_pHost.sin_port ) : (WORD)Settings.Connection.InPort;
}

// Returns TRUE if the IP address is reserved.
// Private addresses are treated as reserved when Connection.IgnoreLocalIP = TRUE.
// The code is based on nmap code and updated according to
// http://www.cymru.com/Documents/bogon-bn-nonagg.txt
// and http://www.iana.org/assignments/ipv4-address-space

BOOL CNetwork::IsReserved(const IN_ADDR* pAddress) const
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
		case 55:        // misc. USA Armed forces
		case 127:       // 127/8 is reserved for loopback
			return TRUE;
		default:
			break;
	}

	// 192.0.2.0/24 is reserved for documentation and examples
	// 192.88.99.0/24 is used as 6to4 Relay anycast prefix by RFC3068
	if ( i1 == 192 )
	{
		if ( i2 == 0 && i3 == 2 ) return TRUE;
		if ( i2 == 88 && i3 == 99 ) return TRUE;
	}

	// 198.18.0.0/15 is used for benchmark tests by RFC2544
	if ( i1 == 198 && i2 == 18 && i3 >= 1 && i3 <= 64 ) return TRUE;

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
	return GetRandomNum( 10000ui16, 60000ui16 );
}

//////////////////////////////////////////////////////////////////////
// CNetwork thread run

bool CNetwork::PreRun()
{
	CQuickLock oLock( m_pSection );

	// Begin network startup
	theApp.Message( MSG_NOTICE, IDS_NETWORK_STARTUP );

	// Act like server application
	SetThreadExecutionState( ES_SYSTEM_REQUIRED | ES_CONTINUOUS );

	// Make sure WinINet is connected (IE is not in offline mode)
	if ( Settings.Connection.ForceConnectedState )
	{
		INTERNET_CONNECTED_INFO ici = {};
		HINTERNET hInternet = InternetOpen( Settings.SmartAgent(), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0 );

		ici.dwConnectedState = INTERNET_STATE_CONNECTED;
		InternetSetOption( hInternet, INTERNET_OPTION_CONNECTED_STATE, &ici, sizeof(ici) );
		InternetCloseHandle( hInternet );
	}

	if ( !InternetConnect() )
	{
		theApp.Message( MSG_ERROR, _T("Internet connection attempt failed.") );
		return false;
	}

	m_bConnected = true;

	Resolve( Settings.Connection.InHost, Settings.Connection.InPort, &m_pHost );

	// Get host name
	gethostname( m_sHostName.GetBuffer( 255 ), 255 );
	m_sHostName.ReleaseBuffer();

	// Get all IPs
	if ( hostent* h = gethostbyname( m_sHostName ) )
	{
		for ( char** p = h->h_addr_list ; p && *p ; p++ )
		{
			AcquireLocalAddress( *(IN_ADDR*)*p );
		}
	}

	// Map ports using NAT UPnP and Control Point UPnP methods and acquire external IP on success
	MapPorts();

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

	if ( ! Handshakes.Listen() || ! Datagrams.Listen() )
	{
		theApp.Message( MSG_ERROR, _T("The connection process is failed.") );
		return false;
	}

	Neighbours.Connect();

	NodeRoute->SetDuration( Settings.Gnutella.RouteCache );
	QueryRoute->SetDuration( Settings.Gnutella.RouteCache );

	Neighbours.IsG2HubCapable( FALSE, TRUE );
	Neighbours.IsG1UltrapeerCapable( FALSE, TRUE );

	// It will check if it is needed inside the function
	DiscoveryServices.Execute( TRUE, PROTOCOL_NULL, FALSE );

	return true;
}

void CNetwork::OnRun()
{
	if ( PreRun() )
	{
		while ( IsThreadEnabled() )
		{
			Doze( 100 );

			if ( ! theApp.m_bLive )
			{
				Sleep( 0 );
				continue;
			}

			if ( UPnPFinder && UPnPFinder->IsAsyncFindRunning() )
			{
				Sleep( 0 );
				continue;
			}

			// Refresh UPnP port mappings
			DWORD tNow = GetTickCount();
			if ( Settings.Connection.EnableUPnP &&
				 tNow > m_tUPnPMap + Settings.Connection.UPnPRefreshTime )
			{
				MapPorts();
				continue;
			}

			if ( m_pSection.Lock() )
			{
				Datagrams.OnRun();
				SearchManager.OnRun();
				QueryHashMaster.Build();
				CrawlSession.OnRun();

				m_pSection.Unlock();
			}

			Neighbours.OnRun();

			RunJobs();

			HostCache.PruneOldHosts();	// Every minute
		}
	}

	PostRun();
}

void CNetwork::PostRun()
{
	CQuickLock oLock( m_pSection );

	m_bConnected = false;

	Neighbours.Close();
	Handshakes.Disconnect();

	Neighbours.Close();
	Datagrams.Disconnect();

	NodeRoute->Clear();
	QueryRoute->Clear();

	ClearResolve();

	ClearJobs();

	DiscoveryServices.Stop();

	DeletePorts();

	CQuickLock oHALock( m_pHASection );
	m_pHostAddresses.RemoveAll();

	// Act like regular application
	SetThreadExecutionState( ES_CONTINUOUS );

	theApp.Message( MSG_NOTICE, IDS_NETWORK_DISCONNECTED );
	theApp.Message( MSG_NOTICE, _T("") );
}

//////////////////////////////////////////////////////////////////////
// CNetwork resolve callback

void CNetwork::OnWinsock(WPARAM wParam, LPARAM lParam)
{
	auto_ptr< ResolveStruct > pResolve( GetResolve( (HANDLE)wParam ) );
	if ( ! pResolve.get() )
		// Out of memory
		return;

	CHostCacheList* pCache = HostCache.ForProtocol( pResolve->m_nProtocol );

	CQuickLock oNetworkLock( m_pSection );

	if ( WSAGETASYNCERROR( lParam ) == 0 )
	{
		IN_ADDR& pAddress = *(IN_ADDR*)pResolve->m_pHost.h_addr;

		if ( pResolve->m_nCommand == 0 )
		{
			if ( pCache )
				pCache->OnResolve( pResolve->m_sAddress, &pAddress, pResolve->m_nPort );
		}
		else if ( pResolve->m_nCommand == 1 || pResolve->m_nCommand == 2 )
		{
			if ( pCache )
				pCache->OnResolve( pResolve->m_sAddress, &pAddress, pResolve->m_nPort );

			BOOL bNoUltraPeer = ( pResolve->m_nCommand == 2 );

			Neighbours.ConnectTo( pAddress, pResolve->m_nPort, pResolve->m_nProtocol, FALSE, bNoUltraPeer );
		}
		else if ( pResolve->m_nCommand == 3 )
		{
			// code to invoke UDPHC/UDPKHL Sender.
			if ( pResolve->m_nProtocol == PROTOCOL_G1 ||
				 pResolve->m_nProtocol == PROTOCOL_G2 )
			{
				CString strAddress = ( ( pResolve->m_nProtocol == PROTOCOL_G1 ) ?
					L"uhc:" : L"ukhl:" ) + pResolve->m_sAddress;
				CDiscoveryService* pService = DiscoveryServices.GetByAddress( strAddress );
				if ( pService == NULL )
				{
					strAddress.AppendFormat(_T(":%u"), pResolve->m_nPort );
					pService = DiscoveryServices.GetByAddress( strAddress );
				}
				if ( pService != NULL )
				{
					pService->m_pAddress = pAddress;
					pService->m_nPort = pResolve->m_nPort;
				}
				if ( pResolve->m_nProtocol == PROTOCOL_G1 )
					UDPHostCache((IN_ADDR*)pResolve->m_pHost.h_addr, pResolve->m_nPort);
				else
					UDPKnownHubCache( &pAddress, pResolve->m_nPort );
			}
		}
	}
	else
	{
		if ( pResolve->m_nCommand == 0 )
		{
			theApp.Message( MSG_ERROR, IDS_NETWORK_RESOLVE_FAIL, pResolve->m_sAddress );
		}
		else if ( pResolve->m_nCommand == 1 || pResolve->m_nCommand == 2 )
		{
			theApp.Message( MSG_ERROR, IDS_NETWORK_RESOLVE_FAIL, pResolve->m_sAddress );

			if ( pCache )
				pCache->OnFailure( pResolve->m_sAddress, false );
		}
		else if ( pResolve->m_nCommand == 3 )
		{
			if ( pResolve->m_nProtocol == PROTOCOL_G1 ||
				 pResolve->m_nProtocol == PROTOCOL_G2 )
			{
				CString strAddress = ( ( pResolve->m_nProtocol == PROTOCOL_G1 ) ?
					L"uhc:" : L"ukhl:" ) + pResolve->m_sAddress;
				CDiscoveryService* pService = DiscoveryServices.GetByAddress( strAddress );
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
}

//////////////////////////////////////////////////////////////////////
// CNetwork get node route

BOOL CNetwork::GetNodeRoute(const Hashes::Guid& oGUID, CNeighbour** ppNeighbour, SOCKADDR_IN* pEndpoint)
{
	if ( validAndEqual( oGUID, Hashes::Guid( MyProfile.oGUID ) ) ) return FALSE;

	if ( NodeRoute->Lookup( oGUID, ppNeighbour, pEndpoint ) ) return TRUE;
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
	CSingleLock pLock( &m_pSection );
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
		if ( IsSelfIP( pEndpoint.sin_addr ) ) return FALSE;
		Datagrams.Send( &pEndpoint, pPacket, FALSE );
	}
	else
	{
		if ( IsSelfIP( pEndpoint.sin_addr ) ) return FALSE;
		pPacket = CG2Packet::New( G2_PACKET_HIT_WRAP, (CG1Packet*)pPacket );
		Datagrams.Send( &pEndpoint, pPacket, TRUE );
	}

	if ( pPacket->m_nProtocol == PROTOCOL_G1 )
		Statistics.Current.Gnutella1.Routed++;
	else if ( pPacket->m_nProtocol == PROTOCOL_G2 )
		Statistics.Current.Gnutella2.Routed++;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CNetwork common handler functions

BOOL CNetwork::OnPush(const Hashes::Guid& oGUID, CConnection* pConnection)
{
	if ( Downloads.OnPush( oGUID, pConnection ) )
		return TRUE;

	if ( ChatCore.OnPush( oGUID, pConnection ) )
		return TRUE;

	CSingleLock oAppLock( &theApp.m_pSection );
	if ( ! oAppLock.Lock( 250 ) )
		return FALSE;

	if ( CMainWnd* pMainWnd = theApp.SafeMainWnd() )
	{
		CChildWnd* pChildWnd = NULL;
		while ( ( pChildWnd = pMainWnd->m_pWindows.Find( NULL, pChildWnd ) ) != NULL )
		{
			pChildWnd->OnPush( oGUID, pConnection );
		}
	}

	return FALSE;
}

void CNetwork::OnQuerySearch(CLocalSearch* pSearch)
{
	CQuickLock oLock( m_pSection );

	// TODO: Add overload protection code

	m_oJobs.AddTail( CJob( CJob::Search, pSearch ) );
}

void CNetwork::OnQueryHits(CQueryHit* pHits)
{
	CQuickLock oLock( m_pSection );

	// TODO: Add overload protection code

	m_oJobs.AddTail( CJob( CJob::Hit, pHits ) );
}

void CNetwork::RunJobs()
{
	// Quick check to avoid locking
	if ( m_oJobs.IsEmpty() )
		return;

	// Spend here no more than 250 ms at once
	DWORD nBegin = GetTickCount();
	CSingleLock oLock( &m_pSection, FALSE );
	if ( ! oLock.Lock( 250 ) )
		return;
	while ( ! m_oJobs.IsEmpty() && GetTickCount() - nBegin < 250 )
	{
		CJob oJob = m_oJobs.RemoveHead();

		oLock.Unlock();

		bool bKeep = false;
		switch ( oJob.GetType() )
		{
		case CJob::Hit:
			bKeep = ProcessQueryHits( oJob );
			break;

		case CJob::Search:
			bKeep = ProcessQuerySearch( oJob );
			break;

		default:
			ASSERT( FALSE );
		}

		oLock.Lock();

		if ( bKeep )
		{
			// Go to next iteration
			m_oJobs.AddTail( oJob );
		}
	}
}

void CNetwork::ClearJobs()
{
	CQuickLock oLock( m_pSection );

	while ( ! m_oJobs.IsEmpty() )
	{
		CJob oJob = m_oJobs.RemoveHead();

		switch ( oJob.GetType() )
		{
		case CJob::Search:
			delete (CLocalSearch*)oJob.GetData();
			break;

		case CJob::Hit:
			((CQueryHit*)oJob.GetData())->Delete();
			break;

		default:
			ASSERT( FALSE );
		}
	}
}

bool CNetwork::ProcessQuerySearch(CNetwork::CJob& oJob)
{
	CLocalSearch* pSearch = (CLocalSearch*)oJob.GetData();

	switch( oJob.GetStage() )
	{
	case 0:
		// Send searches to monitor window
		{
			CSingleLock oAppLock( &theApp.m_pSection );
			if ( oAppLock.Lock( 250 ) )
			{
				if ( CMainWnd* pMainWnd = theApp.SafeMainWnd() )
				{
					CRuntimeClass* pClass = RUNTIME_CLASS(CSearchMonitorWnd);
					CChildWnd* pChildWnd = NULL;
					while ( ( pChildWnd = pMainWnd->m_pWindows.Find( pClass, pChildWnd ) ) != NULL )
					{
						pChildWnd->OnQuerySearch( pSearch->GetSearch() );
					}
				}

				oAppLock.Unlock();

				oJob.Next();
			}
		}
		break;

	case 1:
		// Downloads search
		if ( pSearch->Execute( -1, true, false ) )
		{
			oJob.Next();
		}
		break;

	case 2:
		// Library search
		if ( pSearch->Execute( -1, false, true ) )
		{
			oJob.Next();
		}
		break;
	}

	if ( oJob.GetStage() == 3 )
	{
		// Clean-up
		delete pSearch;

		return false;
	}

	return true;
}

bool CNetwork::ProcessQueryHits(CNetwork::CJob& oJob)
{
	CQueryHit* pHits = (CQueryHit*)oJob.GetData();

	switch( oJob.GetStage() )
	{
	case 0:
		// Update downloads
		if ( Downloads.OnQueryHits( pHits ) )
		{
			oJob.Next();
		}
		break;

	case 1:
		// Update library files alternate sources
		if ( Library.OnQueryHits( pHits ) )
		{
			oJob.Next();
		}
		break;

	case 2:
		// Send hits to search windows
		{
			CSingleLock oAppLock( &theApp.m_pSection );
			if ( oAppLock.Lock( 250 ) )
			{
				if ( CMainWnd* pMainWnd = theApp.SafeMainWnd() )
				{
					// Update search window(s)
					BOOL bHandled = FALSE;
					CChildWnd* pMonitorWnd = NULL;
					CChildWnd* pChildWnd = NULL;
					while ( ( pChildWnd = pMainWnd->m_pWindows.Find( NULL, pChildWnd ) ) != NULL )
					{
						if ( pChildWnd->IsKindOf( RUNTIME_CLASS( CSearchWnd ) ) )
						{
							if ( pChildWnd->OnQueryHits( pHits ) )
								bHandled = TRUE;
						}
						else if ( pChildWnd->IsKindOf( RUNTIME_CLASS( CHitMonitorWnd ) ) )
						{
							pMonitorWnd = pChildWnd;
						}
					}

					// Drop rest to hit window
					if ( ! bHandled && pMonitorWnd )
						pMonitorWnd->OnQueryHits( pHits );
				}
				oAppLock.Unlock();

				oJob.Next();
			}
		}
		break;
	}

	if ( oJob.GetStage() == 3 )
	{
		// Clean-up
		pHits->Delete();

		return false;
	}

	return true;
}

void CNetwork::UDPHostCache(IN_ADDR* pAddress, WORD nPort)
{
	if ( CG1Packet* pPing = CG1Packet::New( G1_PACKET_PING, 1, Hashes::Guid( MyProfile.oGUID ) ) )
	{
		CGGEPBlock pBlock;
		if ( CGGEPItem* pItem = pBlock.Add( GGEP_HEADER_SUPPORT_CACHE_PONGS ) )
		{
			pItem->WriteByte( Neighbours.IsG1Ultrapeer() ? GGEP_SCP_ULTRAPEER : GGEP_SCP_LEAF );
		}
		pBlock.Write( pPing );
		Datagrams.Send( pAddress, nPort, pPing, TRUE, NULL, FALSE );
	}
}

void CNetwork::UDPKnownHubCache(IN_ADDR* pAddress, WORD nPort)
{
	if ( CG2Packet* pKHLR = CG2Packet::New( G2_PACKET_KHL_REQ ) )
	{
		Datagrams.Send( pAddress, nPort, pKHLR, TRUE, NULL, FALSE );
	}
}

SOCKET CNetwork::AcceptSocket(SOCKET hSocket, SOCKADDR_IN* addr, LPCONDITIONPROC lpfnCondition, DWORD_PTR dwCallbackData)
{
	__try	// Fix against stupid firewalls like (iS3 Anti-Spyware or Norman Virus Control)
	{
		int len = sizeof( SOCKADDR_IN );
		return WSAAccept( hSocket, (SOCKADDR*)addr, &len, lpfnCondition, dwCallbackData );
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		return INVALID_SOCKET;
	}
}

void CNetwork::CloseSocket(SOCKET& hSocket, const bool bForce)
{
	if ( hSocket != INVALID_SOCKET )
	{
		__try	// Fix against stupid firewalls like (iS3 Anti-Spyware or Norman Virus Control)
		{
			if ( bForce )
			{
				const LINGER ls = { 1, 0 };
				setsockopt( hSocket, SOL_SOCKET, SO_LINGER, (char*)&ls, sizeof( ls ) );
			}
			else
			{
				shutdown( hSocket, SD_BOTH );
			}
			closesocket( hSocket );
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
		{
		}
		hSocket = INVALID_SOCKET;
	}
}

int CNetwork::Send(SOCKET s, const char* buf, int len)
{
	__try	// Fix against stupid firewalls like (iS3 Anti-Spyware or Norman Virus Control)
	{
		return send( s, buf, len, 0 );
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		return -1;
	}
}

int CNetwork::SendTo(SOCKET s, const char* buf, int len, const SOCKADDR_IN* pTo)
{
	__try	// Fix against stupid firewalls like (iS3 Anti-Spyware or Norman Virus Control)
	{
		return sendto( s, buf, len, 0, (const SOCKADDR*)pTo, sizeof( SOCKADDR_IN ) );
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		return -1;
	}
}

int CNetwork::Recv(SOCKET s, char* buf, int len)
{
	__try	// Fix against stupid firewalls like (iS3 Anti-Spyware or Norman Virus Control)
	{
		return recv( s, buf, len, 0 );
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		return -1;
	}
}

int CNetwork::RecvFrom(SOCKET s, char* buf, int len, SOCKADDR_IN* pFrom)
{
	__try	// Fix against stupid firewalls like (iS3 Anti-Spyware or Norman Virus Control)
	{
		int nFromLen = sizeof( SOCKADDR_IN );
		return recvfrom( s, buf, len, 0, (SOCKADDR*)pFrom, &nFromLen );
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		return -1;
	}
}

HINTERNET CNetwork::InternetOpenUrl(HINTERNET hInternet, LPCWSTR lpszUrl, LPCWSTR lpszHeaders, DWORD dwHeadersLength, DWORD dwFlags)
{
	__try	// Fix against stupid firewalls like (iS3 Anti-Spyware or Norman Virus Control)
	{
		return ::InternetOpenUrl( hInternet, lpszUrl, lpszHeaders, dwHeadersLength, dwFlags, NULL );
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		return NULL;
	}
}

void CNetwork::Cleanup()
{
	__try	// Fix against stupid firewalls like (iS3 Anti-Spyware or Norman Virus Control)
	{
		WSACleanup();
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
	}
}

void CNetwork::MapPorts()
{
	HRESULT hr;

	m_tUPnPMap = GetTickCount();

	if ( ! Settings.Connection.EnableUPnP )
		return;

	m_bUPnPPortsForwarded = TRI_UNKNOWN;

	// UPnP Device Host Service
	if ( ! AreServiceHealthy( _T("upnphost") ) )
	{
		theApp.Message( MSG_ERROR, L"UPnP Device Host and SSDP Discovery services are not running, skipping UPnP setup." );
		m_bUPnPPortsForwarded = TRI_FALSE;
		return;
	}

	// Simple Service Discovery Protocol Service
	AreServiceHealthy( _T("SSDPSRV") );

	BOOL bSuccess = FALSE;

	// (Re)Create an instance of the NAT UPnP
	theApp.Message( MSG_INFO, L"Trying to setup port mappings using NAT UPnP...");
	m_pNatManager.Release();
	m_pNat.Release();
	hr = m_pNat.CoCreateInstance( __uuidof( UPnPNAT ) );
	if ( SUCCEEDED( hr ) && m_pNat )
	{
		// Retrieve the NAT manager interface and register callbacks
		hr = m_pNat->get_NATEventManager( &m_pNatManager );
		if ( SUCCEEDED( hr ) && m_pNatManager )
		{
			m_pNatManager->put_ExternalIPAddressCallback(
				GetInterface( IID_INATExternalIPAddressCallback ) );
			m_pNatManager->put_NumberOfEntriesCallback(
				GetInterface( IID_INATNumberOfEntriesCallback ) );

			// Retrieve the mappings collection
			CComPtr< IStaticPortMappingCollection >	pCollection;
			for ( int i = 0; i < 5; ++i )
			{
				pCollection.Release();
				hr = m_pNat->get_StaticPortMappingCollection( &pCollection );
				if ( SUCCEEDED( hr ) && pCollection )
					break;
				Sleep( 1000 );
			}
			if ( SUCCEEDED( hr ) && pCollection )
			{
				// Retrieve local IP address
				ULONG ulOutBufLen = sizeof( IP_ADAPTER_INFO );
				auto_array< char > pAdapterInfo( new char[ ulOutBufLen ] );
				ULONG ret = GetAdaptersInfo( (PIP_ADAPTER_INFO)pAdapterInfo.get(), &ulOutBufLen );
				if ( ret == ERROR_BUFFER_OVERFLOW )
				{
					pAdapterInfo.reset( new char[ ulOutBufLen ] );
					ret = GetAdaptersInfo( (PIP_ADAPTER_INFO)pAdapterInfo.get(), &ulOutBufLen );
				}
				if ( ret == NO_ERROR )
				{
					IP_ADAPTER_INFO& pInfo = *(PIP_ADAPTER_INFO)pAdapterInfo.get();
					CString strLocalIP( (LPCSTR)( pInfo.IpAddressList.IpAddress.String ) );

					DWORD nPort = Settings.Connection.InPort;
					bool bRandomPort = Settings.Connection.RandomPort;

					if ( nPort == 0 ) // random port
						nPort = Network.RandomPort();

					// Try five times to map both ports
					for ( int i = 0; i < 5; ++i )
					{
						bSuccess = MapPort( pCollection, strLocalIP, nPort, L"TCP", CLIENT_NAME_T ) &&
							MapPort( pCollection, strLocalIP, nPort, L"UDP", CLIENT_NAME_T );
						if ( bSuccess )
						{
							Settings.Connection.InPort = nPort;
							Settings.Connection.RandomPort = bRandomPort;
							break;
						}
						Sleep( 200 );

						// Change port to random
						nPort = Network.RandomPort();
						bRandomPort = true;
					}
				}
			}
		}
	}

	if ( bSuccess )
	{
		OnMapSuccess();
	}
	else
	{
		// Using Control Point UPnP methods
		theApp.Message( MSG_INFO, L"Trying to setup port mappings using Control Point UPnP...");

		if ( UPnPFinder )
			UPnPFinder->StartDiscovery();
	}
}

BOOL CNetwork::MapPort(IStaticPortMappingCollection* pCollection, LPCWSTR szLocalIP, long nPort, LPCWSTR szProtocol, LPCWSTR szDescription)
{
	HRESULT hr;

	// Retrieving existing mapping
	CComPtr< IStaticPortMapping > pMapping;
	hr = pCollection->get_Item( nPort, CComBSTR( szProtocol ), &pMapping );
	if ( FAILED( hr ) || ! pMapping )
	{
		// Create new mapping
		hr = pCollection->Add( nPort, CComBSTR( szProtocol ), nPort, CComBSTR( szLocalIP ),
			VARIANT_TRUE, CComBSTR( szDescription ), &pMapping );
		if ( FAILED( hr ) || ! pMapping )
			return FALSE;
	}

	// Checking port number
	long nInternalPort = 0;
	hr = pMapping->get_InternalPort( &nInternalPort );
	if ( FAILED( hr ) || nInternalPort != nPort )
	{
		hr = pMapping->EditInternalPort( nPort );
		if ( FAILED( hr ) )
			return FALSE;
	}

	// Checking enable state
	VARIANT_BOOL bEnabled = VARIANT_FALSE;
	hr = pMapping->get_Enabled( &bEnabled );
	if ( FAILED( hr ) || bEnabled == VARIANT_FALSE )
	{
		hr = pMapping->Enable( VARIANT_TRUE );
		if ( FAILED( hr ) )
			return FALSE;
	}

	return TRUE;
}

void CNetwork::DeletePorts()
{
	HRESULT hr;

	if ( Settings.Connection.DeleteUPnPPorts && Settings.Connection.InPort )
	{
		if ( m_pNat )
		{
			// Retrieve the mappings collection
			CComPtr< IStaticPortMappingCollection >	pCollection;
			hr = m_pNat->get_StaticPortMappingCollection( &pCollection );
			if ( SUCCEEDED( hr ) && pCollection )
			{
				// Unmap
				pCollection->Remove( Settings.Connection.InPort, CComBSTR( L"TCP" ) );
				pCollection->Remove( Settings.Connection.InPort, CComBSTR( L"UDP" ) );
			}
		}

		// Legacy way
		if ( UPnPFinder )
		{
			UPnPFinder->StopAsyncFind();
			UPnPFinder->DeletePorts();
		}
	}

	m_nUPnPExternalAddress.s_addr = INADDR_ANY;
	m_bUPnPPortsForwarded = TRI_UNKNOWN;
	m_tUPnPMap = 0;

	m_pNatManager.Release();
	m_pNat.Release();
}

void CNetwork::OnNewExternalIPAddress(const IN_ADDR& pAddress)
{
	if ( pAddress.s_addr != INADDR_ANY &&
		 pAddress.s_addr != INADDR_NONE &&
		 pAddress.s_addr != m_nUPnPExternalAddress.s_addr )
	{
		theApp.Message( MSG_INFO, _T("UPnP gateway device reports external IP address: %s"),
			(LPCTSTR)CA2CT( inet_ntoa( pAddress ) ) );

		m_nUPnPExternalAddress = pAddress;

		AcquireLocalAddress( pAddress );
	}
}

void CNetwork::OnMapSuccess()
{
	theApp.Message( MSG_INFO, _T("UPnP port mapping succeeded.") );

	m_bUPnPPortsForwarded = TRI_TRUE;
	m_tUPnPMap = GetTickCount();
}

void CNetwork::OnMapFailed()
{
	theApp.Message( MSG_ERROR, _T("UPnP port mapping failed.") );

	m_bUPnPPortsForwarded = TRI_FALSE;
	m_tUPnPMap = GetTickCount();
}

BEGIN_INTERFACE_MAP( CNetwork, CComObject )
	INTERFACE_PART( CNetwork, IID_INATNumberOfEntriesCallback, NATNumberOfEntriesCallback )
	INTERFACE_PART( CNetwork, IID_INATExternalIPAddressCallback, NATExternalIPAddressCallback )
END_INTERFACE_MAP()

IMPLEMENT_UNKNOWN( CNetwork, NATNumberOfEntriesCallback )

STDMETHODIMP CNetwork::XNATNumberOfEntriesCallback::NewNumberOfEntries(/* [in] */ long /* lNewNumberOfEntries */)
{
	METHOD_PROLOGUE( CNetwork, NATNumberOfEntriesCallback )

	TRACE( _T("[UPnP] Got new number of entries\n") );

	DWORD tNow = GetTickCount();
	if ( tNow > pThis->m_tUPnPMap + 5*1000 )	// 5 sec delay to avoid self-notification
	{
		pThis->m_tUPnPMap = 0;	// Force port mappings
	}

	return S_OK;
}

IMPLEMENT_UNKNOWN( CNetwork, NATExternalIPAddressCallback )

STDMETHODIMP CNetwork::XNATExternalIPAddressCallback::NewExternalIPAddress(/* [in] */ BSTR bstrNewExternalIPAddress)
{
	METHOD_PROLOGUE( CNetwork, NATExternalIPAddressCallback )

	IN_ADDR pAddress;
	pAddress.s_addr = inet_addr( (LPCSTR)CW2A( (LPCWSTR)bstrNewExternalIPAddress ) );

	pThis->OnNewExternalIPAddress( pAddress );

	return S_OK;
}