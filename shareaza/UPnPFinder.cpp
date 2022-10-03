//
// UPnPFinder.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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
#include "UPnPFinder.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CUPnPFinder::CUPnPFinder()
:	m_pDevices(),
	m_pServices(),
	m_nAsyncFindHandle( 0 ),
	m_bAsyncFindRunning( false ),
	m_bADSL( false ),
	m_bADSLFailed( false ),
	m_bPortIsFree( true ),
	m_sLocalIP(),
	m_sExternalIP(),
	m_tLastEvent( GetTickCount() ),
	m_bInited( false ),
	m_bSecondTry( false ),
	m_bDisableWANIPSetup( false ),
	m_bDisableWANPPPSetup( false )
{
}

bool CUPnPFinder::Init()
{
	if ( ! m_bInited )
	{
		m_bDisableWANIPSetup =  Settings.Connection.SkipWANIPSetup;
		m_bDisableWANPPPSetup = Settings.Connection.SkipWANPPPSetup;
		m_pDeviceFinder = CreateFinderInstance();
		m_pServiceCallback = new CServiceCallback( *this );
		m_pDeviceFinderCallback = new CDeviceFinderCallback( *this );
		m_bInited = ( m_pDeviceFinder && m_pServiceCallback && m_pDeviceFinderCallback );
	}

	return m_bInited;
}

CUPnPFinder::FinderPointer CUPnPFinder::CreateFinderInstance() throw()
{
	FinderPointer pNewDeviceFinder;
	if ( FAILED( pNewDeviceFinder.CoCreateInstance( CLSID_UPnPDeviceFinder ) ) )
	{
		// Should we ask to disable auto-detection?
		theApp.Message( MSG_INFO, L"UPnP discovery is not supported or not installed." );
	}
	return pNewDeviceFinder;
}

CUPnPFinder::~CUPnPFinder()
{
	StopAsyncFind();

	m_pDevices.clear();
	m_pServices.clear();
}

// Helper function for processing the AsyncFind search
void CUPnPFinder::ProcessAsyncFind(BSTR bsSearchType) throw()
{
	HRESULT hr;

	// We have to start the AsyncFind.
	if ( ! m_pDeviceFinderCallback || ! m_pDeviceFinder )
	{
		Network.OnMapFailed();
		return;
	}

	__try
	{
		hr = m_pDeviceFinder->CreateAsyncFind( bsSearchType, NULL,
			m_pDeviceFinderCallback, &m_nAsyncFindHandle );
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		hr = E_FAIL;
	}
	if ( FAILED( hr ) )
	{
		Network.OnMapFailed();
		return;
	}

	m_bAsyncFindRunning = true;
	m_tLastEvent = GetTickCount();

	__try
	{
		hr = m_pDeviceFinder->StartAsyncFind( m_nAsyncFindHandle );
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		hr = E_FAIL;
	}
	if ( FAILED( hr ) )
	{
		__try
		{
			m_pDeviceFinder->CancelAsyncFind( m_nAsyncFindHandle );
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
		{
		}
		m_bAsyncFindRunning = false;
		Network.OnMapFailed();
	}
}

void CUPnPFinder::StartDiscovery()
{
	StartDiscovery( false );
}

// Helper function for stopping the async find if proceeding
void CUPnPFinder::StopAsyncFind()
{
	// This will stop the async find if it is in progress
	// ToDo: Locks up in WinME, cancelling is required <- critical

	if ( m_bInited && IsAsyncFindRunning() )
	{
		__try
		{
			m_pDeviceFinder->CancelAsyncFind( m_nAsyncFindHandle );
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
		{
		}
	}

	if ( m_bSecondTry )
		m_bAsyncFindRunning = false;
}

// Start the discovery of the UPnP gateway devices
void CUPnPFinder::StartDiscovery(bool bSecondTry)
{
	if ( bSecondTry && m_bSecondTry ) // already did 2 tries
		return;

	if ( ! Init() )
		return;

	// On tests, in some cases the search for WANConnectionDevice had no results and only a search for InternetGatewayDevice
	// showed up the UPnP root Device which contained the WANConnectionDevice as a child. I'm not sure if there are cases
	// where search for InternetGatewayDevice only would have similar bad effects, but to be sure we do "normal" search first
	// and one for InternetGateWayDevice as fallback
	static const CString strDeviceType1( L"urn:schemas-upnp-org:device:WANConnectionDevice:1");
	static const CString strDeviceType2( L"urn:schemas-upnp-org:device:InternetGatewayDevice:1");

	StopAsyncFind();		// If AsyncFind is in progress, stop it
	m_bSecondTry = bSecondTry;

	m_bPortIsFree = true;

	// We have to process the AsyncFind
	ProcessAsyncFind( CComBSTR( bSecondTry ? strDeviceType2 : strDeviceType1 ) );

	// We should not release the device finder object
	return;
}

// Helper function for adding devices to the list
// This is called by the devicefinder callback object (DeviceAdded func)
void CUPnPFinder::AddDevice(DevicePointer device, bool bAddChilds, int nLevel)
{
	if ( nLevel > 10 )
	{
		ASSERT( false );
		return;
	}

	//We are going to add a device
	CComBSTR bsFriendlyName, bsUniqueName;

	m_tLastEvent = GetTickCount();
	HRESULT hr = device->get_FriendlyName( &bsFriendlyName );

	if ( FAILED( hr ) )
		return (void)UPnPMessage( hr );

	hr = device->get_UniqueDeviceName( &bsUniqueName );

	if ( FAILED( hr ) )
		return (void)UPnPMessage( hr );

	// Add the item at the end of the device list if not found
	std::vector< CAdapt< DevicePointer > >::iterator deviceSet
		= std::find_if( m_pDevices.begin(), m_pDevices.end(), FindDevice( bsUniqueName ) );

	if ( deviceSet == m_pDevices.end() )
	{
		m_pDevices.push_back( device );
		theApp.Message( MSG_DEBUG, L"Found UPnP device: %s (ChildLevel: %i, UID: %s)", (LPCTSTR)bsFriendlyName, nLevel, (LPCTSTR)bsUniqueName );
	}

	if ( ! bAddChilds )
		return;

	// Recursive add any child devices, see comment on StartDiscovery
	CComPtr< IUPnPDevices > pChildDevices;
	if ( SUCCEEDED( device->get_Children( &pChildDevices ) ) )
	{
		if ( ! pChildDevices )
			return;

		CComPtr< IUnknown > pIUnknown;
		if ( FAILED( hr = pChildDevices->get__NewEnum( &pIUnknown ) ) )
			return (void)UPnPMessage( hr );

		CComQIPtr< IEnumVARIANT > pEnumVar( pIUnknown );
		if ( ! pEnumVar )
			return (void)UPnPMessage( E_NOINTERFACE );

		CComVariant var;
		ULONG lFetch;
		hr = pEnumVar->Next( 1, &var, &lFetch );
		while ( hr == S_OK )
		{
			if ( lFetch == 1 )
			{
				CComQIPtr< IUPnPDevice > pChildDevice( V_DISPATCH( &var ) );
				if ( ! pChildDevice )
					return (void)UPnPMessage( E_NOINTERFACE );

				if ( SUCCEEDED(pChildDevice->get_FriendlyName( &bsFriendlyName )) &&
					 SUCCEEDED(pChildDevice->get_UniqueDeviceName( &bsUniqueName )) )
					AddDevice( pChildDevice, true, nLevel + 1 );
			}

			hr = pEnumVar->Next( 1, &var, &lFetch );
		};
	}
}

// Helper function for removing device from the list
// This is called by the devicefinder callback object (DeviceRemoved func)
void CUPnPFinder::RemoveDevice(CComBSTR bsUDN)
{
	theApp.Message( MSG_DEBUG, L"Finder asked to remove: %s", (LPCTSTR)bsUDN );

	std::vector< CAdapt< DevicePointer > >::iterator device
		= std::find_if( m_pDevices.begin(), m_pDevices.end(), FindDevice( bsUDN ) );

	if ( device != m_pDevices.end() )
	{
		theApp.Message( MSG_DEBUG, L"Device removed: %s", (LPCTSTR)bsUDN );
		m_pDevices.erase( device );
	}
}

bool CUPnPFinder::OnSearchComplete()
{
	if ( m_pDevices.empty() )
	{
		if ( m_bSecondTry )
		{
			Network.OnMapFailed();
		}
		else
			theApp.Message( MSG_INFO, L"Found no UPnP gateway devices - will retry with different parameters..." );
		return false; // no devices found
	}

	for ( std::size_t pos = 0; pos < m_pDevices.size(); pos++ )
	{
		GetDeviceServices( m_pDevices[ pos ] );
		StartPortMapping();

		if ( ! m_bPortIsFree ) // warn only once
		{
			// Add more descriptive explanation!!!
			CString strMessage;
			LoadString( strMessage, IDS_NETWORK_UPNP_FAILED );
			AfxMessageBox( strMessage, MB_OK|MB_ICONEXCLAMATION );
			break;
		}
	}
	return true;
}

// Function to populate the service list for the device
HRESULT	CUPnPFinder::GetDeviceServices(DevicePointer pDevice)
{
	if ( !pDevice )
		return E_POINTER;

	HRESULT hr = S_OK;

	m_pServices.clear();
	CComPtr< IUPnPServices > pServices;
	if ( FAILED( hr = pDevice->get_Services( &pServices ) ) )
		return UPnPMessage( hr ), hr;

	LONG nCount = 0;
	if ( FAILED( hr = pServices->get_Count( &nCount ) ) )
		return UPnPMessage( hr ), hr;

	if ( nCount == 0 )
	{
		// Should we ask a user to disable auto-detection?
		theApp.Message( MSG_INFO, L"Found no services for the current UPnP device." );
		return hr;
	}

	// We have to get a IEnumUnknown pointer
	CComPtr< IUnknown > pIUnknown;
	if ( FAILED( hr = pServices->get__NewEnum( &pIUnknown ) ) )
		return UPnPMessage( hr ), hr;

	CComQIPtr< IEnumUnknown > pEU( pIUnknown );
	if ( ! pEU )
		return UPnPMessage( E_NOINTERFACE ), E_NOINTERFACE;

	hr = SaveServices( pEU, nCount );

	return hr;
}

// Saves services from enumeration to member m_pServices
HRESULT CUPnPFinder::SaveServices(CComPtr< IEnumUnknown > pEU, const LONG nTotalItems)
{
	HRESULT hr = S_OK;
	CComBSTR bsServiceId;

	for ( LONG nIndex = 0 ; nIndex < nTotalItems ; nIndex++ )
	{
		CComPtr< IUnknown > punkService;
		hr = pEU->Next( 1, &punkService, NULL );
		if ( FAILED( hr ) )
		{
			// Happens with MS ICS sometimes when the device is disconnected, reboot fixes that
			theApp.Message( MSG_ERROR, L"Traversing the service list of UPnP device failed." );
			return UPnPMessage( hr ), hr;
		}

		// Get a IUPnPService pointer to the service just got
		CComQIPtr< IUPnPService > pService( punkService );
		if ( ! pService )
			return UPnPMessage( E_NOINTERFACE ), E_NOINTERFACE;

		if ( FAILED( hr = pService->get_Id( &bsServiceId ) ) )
			return UPnPMessage( hr ), hr;

		theApp.Message( MSG_DEBUG, L"Found UPnP service: %s", (LPCTSTR)bsServiceId );
		m_pServices.push_back( pService );
		bsServiceId.Empty();
	}

	return hr;
}

HRESULT CUPnPFinder::MapPort(const ServicePointer& service)
{
	CComBSTR bsServiceId;

	HRESULT hr = service->get_Id( &bsServiceId );
	if ( FAILED( hr ) )
		return UPnPMessage( hr );

	CString strServiceId( bsServiceId );

	if ( m_bADSL ) // not a very reliable way to detect ADSL, since WANEthLinkC* is optional
	{
		if ( Network.m_bUPnPPortsForwarded == TRI_TRUE ) // another physical device or the setup was ran again manually
		{
			// Reset settings and recheck ( is there a better solution? )
			Settings.Connection.SkipWANIPSetup  = FALSE;
			Settings.Connection.SkipWANPPPSetup = FALSE;
			m_bADSL = false;
			m_bADSLFailed = false;
		}
		else if ( !m_bADSLFailed )
		{
			theApp.Message( MSG_DEBUG, L"WANEthLinkC service detected. Disabling WANIPConn setup..." );
			Settings.Connection.SkipWANIPSetup  = TRUE;
			Settings.Connection.SkipWANPPPSetup = FALSE;
		}
	}

	// ADSL routers have WANEthernetLinkConfig and WANPPPConnection services.
	// They may have WANIPConnection service too. Seems the right way to forward
	// ports is to use WANPPPConnection. If we hadn't skipped WANIPConnection,
	// some routers could remove the previously forwarded ports as if WANIPConnection
	// and WANPPPConnection are mutually exclusive.
	// The problem is that it's unclear if the order of services is always the same...
	// But looks like it is.
	// WANEthernetLinkConfig is not a good criterion to detect an ADSL device. Thus,
	// we first skip WANIPConnection service and if we fail afterwards, we would try
	// to use it as a last resort.
	if ( !m_bADSL )
	{
		m_bADSL = !( strServiceId.Find( L"urn:upnp-org:serviceId:WANEthLinkC" ) == -1 ) ||
				  !( strServiceId.Find( L"urn:upnp-org:serviceId:WANDSLLinkC" ) == -1 );
	}

	bool bPPP = !( strServiceId.Find( L"urn:upnp-org:serviceId:WANPPPConn" ) == -1 );
	bool bIP  = !( strServiceId.Find( L"urn:upnp-org:serviceId:WANIPConn" ) == -1 );

	if ( bIP ) m_pWANIPService = service;

	if ( ( Settings.Connection.SkipWANPPPSetup || m_bDisableWANPPPSetup ) && bPPP ||
		 ( Settings.Connection.SkipWANIPSetup || m_bDisableWANIPSetup ) && bIP ||
		 !bPPP && !bIP )
		return S_OK;

	// For ICS we can query variables, for router devices we need to use
	// actions to get the ConnectionStatus state variable; recommended to use actions
	// "GetStatusInfo" returns state variables:
	//		|ConnectionStatus|LastConnectionError|Uptime|

	CString strResult;
	hr = InvokeAction( service, L"GetStatusInfo", NULL, strResult );

	if ( strResult.IsEmpty() )
		return hr;

	theApp.Message( MSG_DEBUG, L"Got status info from the service %s: %s", (LPCTSTR)strServiceId, (LPCTSTR)strResult );

	if ( _tcsistr( strResult, L"|VT_BSTR=Connected|" ) != NULL )
	{
		// Add a callback to detect device status changes
		// ??? How it will work if two devices are active ???

		hr = service->AddCallback( m_pServiceCallback );
		// Marshaller adds a ref here, so we should release it
		// m_pServiceCallback->Release();
		if ( FAILED( hr ) )
			UPnPMessage( hr );
		else
			theApp.Message( MSG_DEBUG, L"Callback added for the service %s", (LPCTSTR)strServiceId );

		// Delete old and add new port mappings
		m_sLocalIP = GetLocalRoutableIP( service );
		if ( ! m_sLocalIP.IsEmpty() )
		{
			DeleteExistingPortMappings( service );
			CreatePortMappings( service );
		}
	}
	else if ( _tcsistr( strResult, L"|VT_BSTR=Disconnected|" ) != NULL && m_bADSL && bPPP )
	{
		theApp.Message( MSG_DEBUG, L"Disconnected PPP service in ADSL device..." );
		Settings.Connection.SkipWANIPSetup  = FALSE;
		Settings.Connection.SkipWANPPPSetup = TRUE;
		m_bADSLFailed = true;
	}
	else if ( _tcsistr( strResult, L"|VT_BSTR=Disconnected|" ) != NULL && m_bADSL && bIP )
	{
		theApp.Message( MSG_DEBUG, L"Disconnected IP service in ADSL device..." );
		Settings.Connection.SkipWANIPSetup  = TRUE;
		Settings.Connection.SkipWANPPPSetup = FALSE;
		m_bADSLFailed = true;
	}
	return S_OK;
}

void CUPnPFinder::StartPortMapping()
{
	std::for_each( m_pServices.begin(), m_pServices.end(), boost::bind( &CUPnPFinder::MapPort, this, _1 ) );
	if ( m_bADSL && !Settings.Connection.SkipWANIPSetup &&
		( Network.m_bUPnPPortsForwarded == TRI_UNKNOWN || m_bADSLFailed ) && m_pWANIPService != NULL )
	{
		m_bADSLFailed = true;
		theApp.Message( MSG_DEBUG, L"Configuration failed or it wasn't an ADSL device. Retrying with WANIPConn setup..." );
		m_bDisableWANIPSetup = false;
		m_bDisableWANPPPSetup = true;
		MapPort( m_pWANIPService );
	}
}

void CUPnPFinder::DeletePorts()
{
   if ( !m_bInited )
	   return;
	std::for_each( m_pServices.begin(), m_pServices.end(), boost::bind( &CUPnPFinder::DeleteExistingPortMappings, this, _1 ) );
}

bool CUPnPFinder::IsAsyncFindRunning()
{
	if ( m_pDeviceFinder && m_bAsyncFindRunning )
	{
		if ( GetTickCount() > m_tLastEvent + Settings.Connection.UPnPTimeout )
		{
			__try
			{
				m_pDeviceFinder->CancelAsyncFind( m_nAsyncFindHandle );
			}
			__except( EXCEPTION_EXECUTE_HANDLER )
			{
			}
			m_bAsyncFindRunning = false;
			Network.OnMapFailed();
		}
	}
	return m_bAsyncFindRunning;
}

// Finds a local IP address routable from UPnP device
CString CUPnPFinder::GetLocalRoutableIP(ServicePointer pService)
{
	CString strExternalIP;
	HRESULT hr = InvokeAction( pService, L"GetExternalIPAddress", NULL, strExternalIP );
	int nEqualPos = strExternalIP.Find( '=' );
	strExternalIP = strExternalIP.Mid( nEqualPos + 1 ).Trim( '|' );

	if ( FAILED( hr ) || strExternalIP.IsEmpty() )
		return CString();

	DWORD nInterfaceIndex = 0;
	DWORD ip = inet_addr( (LPCSTR)CT2A( (LPCTSTR)strExternalIP ) );

	// Get the interface through which the UPnP device has a route
	HRESULT hrRes = GetBestInterface( ip, &nInterfaceIndex );

	if ( ip == INADDR_NONE || hrRes != NO_ERROR )
		return CString();

	MIB_IFROW ifRow = {};
	ifRow.dwIndex = nInterfaceIndex;
	hrRes = GetIfEntry( &ifRow );

	if ( hrRes != NO_ERROR )
		return CString();

	// Take an IP address table
	char mib[ sizeof(MIB_IPADDRTABLE) + 32 * sizeof(MIB_IPADDRROW) ];
	ULONG nSize = sizeof(mib);
	PMIB_IPADDRTABLE ipAddr = (PMIB_IPADDRTABLE)mib;

	hrRes = GetIpAddrTable( ipAddr, &nSize, FALSE );

	if ( hrRes != NO_ERROR )
		return CString();

	DWORD nCount = ipAddr->dwNumEntries;
	CString strLocalIP;

	// Look for IP associated with the interface in the address table
	// Loopback addresses are functional for ICS? (at least Windows maps them fine)
	for ( DWORD nIf = 0 ; nIf < nCount ; nIf++ )
	{
		if ( ipAddr->table[ nIf ].dwIndex == nInterfaceIndex )
		{
			strLocalIP = inet_ntoa( *(IN_ADDR*)&ipAddr->table[ nIf ].dwAddr );
			break;
		}
	}

	if ( ! strLocalIP.IsEmpty() && ! strExternalIP.IsEmpty() )
	{
		theApp.Message( MSG_INFO, L"UPnP route: %s->%s", (LPCTSTR)strLocalIP, (LPCTSTR)strExternalIP );

		Network.AcquireLocalAddress( strExternalIP );
	}

	return strLocalIP;
}

// Walks through all port mappings and searches for "Shareaza" string.
// Deletes when it has the same IP as local, otherwise quits and sets
// m_bPortIsFree to false after 10 attempts to use a random port;
// this member will be used to determine if we have to create new port maps.
void CUPnPFinder::DeleteExistingPortMappings(ServicePointer pService)
{
	// Port mappings are numbered starting from 0 without gaps between;
	// So, we will loop until we get an empty string or failure as a result.
	CString strInArgs;
	USHORT nEntry = 0; // PortMappingNumberOfEntries is of type VT_UI2
	if ( m_sLocalIP.IsEmpty() )
		return;

	HRESULT hr = S_OK;
	int nAttempts = 10;

	// ICS returns computer name instead of IP, thus we need to compare not IPs
	CString strComputerName;
	DWORD nMaxLen = MAX_COMPUTERNAME_LENGTH + 1;
	GetComputerName( strComputerName.GetBuffer( nMaxLen ), &nMaxLen );
	strComputerName.ReleaseBuffer();

	CString strActionResult;
	do
	{
		HRESULT hrDel = E_FAIL;
		strInArgs.Format( _T("|VT_UI2=%hu|"), nEntry );
		hr = InvokeAction( pService,
			 L"GetGenericPortMappingEntry", strInArgs, strActionResult );

		if ( SUCCEEDED( hr ) && ! strActionResult.IsEmpty() )
		{
			// It returned in the following format and order:
			//
			// VT_BSTR	RemoteHost = "" (i.e. any)
			// VT_UI2	ExternalPort = 6346
			// VT_BSTR	PortMappingProtocol = "TCP"
			// VT_UI2	InternalPort = 6346
			// VT_BSTR	InternalClient = "192.168.0.1"
			// VT_BOOL	PortMappingEnabled = True
			// VT_BSTR	PortMappingDescription = "Shareaza TCP"
			// VT_UI4	PortMappingLeaseDuration = 0 (i.e. any)

			// DeletePortMapping action takes 3 arguments:
			//		RemoteHost, ExternalPort and PortMappingProtocol

			CString strHost, strPort, strProtocol;

			if ( _tcsistr( strActionResult, L"|VT_BSTR=Shareaza TCP|" ) != NULL ||
				_tcsistr( strActionResult, L"|VT_BSTR=Shareaza UDP|" ) != NULL )
			{
				CStringArray oTokens;
				Split( strActionResult, _T('|'), oTokens );

				if ( oTokens.GetCount() != 8 )
					break;

				strHost		= '|' + oTokens[ 0 ];
				strPort		= '|' + oTokens[ 1 ];
				strProtocol	= '|' + oTokens[ 2 ] + '|';

				// verify types
				if ( _tcsistr( strHost, L"VT_BSTR" ) == NULL
						|| _tcsistr( strPort, L"VT_UI2" ) == NULL
						|| _tcsistr( strProtocol, L"VT_BSTR" ) == NULL )
					break;

				if ( _tcsstr( oTokens[ 4 ], m_sLocalIP ) != NULL ||
					 _tcsistr( oTokens[ 4 ], (LPCTSTR)strComputerName ) != NULL )
				{
					CString str;
					hrDel = InvokeAction( pService, L"DeletePortMapping",
						strHost + strPort + strProtocol, str );
					if ( FAILED( hrDel ) )
						UPnPMessage( hrDel );
					else
					{
						theApp.Message( MSG_DEBUG, L"Old port mapping deleted: %s", (LPCTSTR)( strPort + strProtocol ) );
					}
				}
				else // different IP found in the port mapping entry
				{
					theApp.Message( MSG_DEBUG, L"Port %s is used by %s, trying random port.",
							(LPCTSTR)oTokens[ 1 ], (LPCTSTR)oTokens[ 4 ] );
					CString str;
					str.Format( L"%hu", Settings.Connection.InPort );
					if ( _tcsstr( strPort, str ) != NULL ) // ports are equal
					{
						Settings.Connection.InPort = Network.RandomPort();
						// start from the beginning
						nEntry = 0;
						hrDel = hr = S_OK;
						// cancel after the defined number of attempts
						if ( ! ( nAttempts-- ) )
						{
							m_bPortIsFree = false;
							break;
						}
					}
				}
			}
		}

		if ( FAILED( hrDel ) )
			nEntry++; // Entries are pushed from bottom to top after success
		if ( nEntry > 30 )
		{
			// FIXME: this is a sanitize check, since some routers seem to reponse to invalid GetGenericPortMappingEntry numbers
			// proper way would be to get the actualy portmapping count, but needs testing before
			theApp.Message( MSG_INFO, L"GetGenericPortMappingEntry maximal count exceeded, quiting." );
			break;
		}
	}
	while ( SUCCEEDED( hr ) && strActionResult.GetLength() );
}

// Creates TCP and UDP port mappings
void CUPnPFinder::CreatePortMappings(ServicePointer pService)
{
	if ( m_sLocalIP.IsEmpty() || !m_bPortIsFree )
		return;

	CString strPort, strInArgs, strResult;

	LPCTSTR szFormatString = L"|VT_BSTR=|VT_UI2=%s|VT_BSTR=%s|VT_UI2=%s|VT_BSTR=%s|VT_BOOL=True|VT_BSTR=%s at %s:%s|VT_UI4=0|";

	if ( Settings.Connection.InPort == 0 ) // random port
	{
		Settings.Connection.InPort = Network.RandomPort();
	}
	strPort.Format( L"%hu", Settings.Connection.InPort );

	// First map UDP if some buggy router overwrites TCP on top
	strInArgs.Format( szFormatString, (LPCTSTR)strPort, L"UDP", (LPCTSTR)strPort, (LPCTSTR)m_sLocalIP,
		CLIENT_NAME_T _T(" UDP"), (LPCTSTR)m_sLocalIP, (LPCTSTR)strPort );
	HRESULT hr = InvokeAction( pService, L"AddPortMapping", strInArgs, strResult );
	if ( FAILED( hr ) )
		return (void)UPnPMessage( hr );

	strInArgs.Format( szFormatString, (LPCTSTR)strPort, L"TCP", (LPCTSTR)strPort, (LPCTSTR)m_sLocalIP,
		CLIENT_NAME_T _T(" TCP"), (LPCTSTR)m_sLocalIP, (LPCTSTR)strPort );
	hr = InvokeAction( pService, L"AddPortMapping", strInArgs, strResult );
	if ( FAILED( hr ) )
		return (void)UPnPMessage( hr );

	// Leave the message loop, since events may take more time.
	// Assuming that the user doesn't use several devices

	m_bAsyncFindRunning = false;
	Network.OnMapSuccess();
}

// Invoke the action for the selected service.
// OUT arguments or return value is packed in strResult.
HRESULT CUPnPFinder::InvokeAction(ServicePointer pService,
	CComBSTR action, LPCTSTR pszInArgString, CString& strResult)
{
	if ( !pService || !action )
		return E_POINTER;

	m_tLastEvent = GetTickCount();
	CString strInArgs;
	strInArgs.SetString( pszInArgString ? pszInArgString : _T("") );

	HRESULT hr = S_OK;

	CComVariant	vaActionArgs, vaArray, vaOutArgs, vaRet;
	VARIANT**  ppVars = NULL;
	SAFEARRAY* psaArgs = NULL;
	LONG nPos = 0;

	INT_PTR nArgs = CreateVarFromString( strInArgs, &ppVars );
	if ( nArgs < 0 ) return E_FAIL;

	hr = CreateSafeArray( VT_VARIANT, (ULONG)nArgs, &psaArgs );
	if ( FAILED( hr ) ) return hr;

	vaArray.vt = VT_VARIANT | VT_ARRAY | VT_BYREF;
	vaArray.pparray = &psaArgs;

	vaActionArgs.vt = VT_VARIANT | VT_BYREF;
	vaActionArgs.pvarVal = &vaArray;

	vaArray.pparray = &psaArgs;

	for( INT_PTR nArg = 0 ; nArg < nArgs ; nArg++ )
	{
		nPos = (LONG)( nArg + 1 );
		hr = SafeArrayPutElement( psaArgs, &nPos, ppVars[ nArg ] );
		if ( FAILED( hr ) ) return hr;
	}

	hr = pService->InvokeAction( action, vaActionArgs, &vaOutArgs, &vaRet);

	if ( SUCCEEDED( hr ) )
	{
		// In connection services return value is empty
		// when OUT arguments are returned
		if ( vaRet.vt != VT_EMPTY )
		{
			bool bInvalid = false;

			if ( vaRet.vt == VT_BSTR )
				strResult = L"|VT_BSTR=";
			else if ( vaRet.vt == VT_UI2 )
				strResult = L"|VT_UI2=";
			else if ( vaRet.vt == VT_UI4 )
				strResult = L"|VT_UI4=";
			else if ( vaRet.vt == VT_BOOL )
				strResult = L"|VT_BOOL=";
			else
				bInvalid = true;

			if ( ! bInvalid )
			{
				hr = VariantChangeType( &vaRet, &vaRet, VARIANT_ALPHABOOL, VT_BSTR );
				if ( SUCCEEDED( hr ) )
				{
					CString str( vaRet.bstrVal );
					strResult += str;
					strResult += L"|";
				}
				else strResult.Empty();
			}
		}
		else
			GetStringFromOutArgs( &vaOutArgs, strResult );
	}

	if ( ppVars != NULL ) DestroyVars( nArgs, &ppVars );
	if ( psaArgs != NULL ) SafeArrayDestroy( psaArgs );

	return hr;
}

// Creates a SafeArray
// vt--VariantType
// nArgs--Number of Arguments
// ppsa--Created safearray
HRESULT CUPnPFinder::CreateSafeArray(const VARTYPE vt, const ULONG nArgs, SAFEARRAY** ppsa)
{
	SAFEARRAYBOUND aDim[ 1 ];

	if ( nArgs == 0 )
	{
		aDim[ 0 ].lLbound = 0;
		aDim[ 0 ].cElements = 0;
	}
	else
	{
		aDim[ 0 ].lLbound = 1;
		aDim[ 0 ].cElements = nArgs;
	}

	*ppsa = SafeArrayCreate( vt, 1, aDim );

	if( NULL == *ppsa ) return E_OUTOFMEMORY;
	return S_OK;
}

// Creates argument variants from the string
// The string format is "|variant_type1=value1|variant_type2=value2|"
// The most common types used for UPnP values are:
//		VT_BSTR, VT_UI2, VT_UI4, VT_BOOL
// Returns: number of arguments or -1 if invalid string/values.

INT_PTR CUPnPFinder::CreateVarFromString(const CString& strArgs, VARIANT*** pppVars)
{
	if ( strArgs.IsEmpty() )
	{
		*pppVars = NULL;
		return 0;
	}

	CStringArray oTokens;
	CString strToken, strType, strValue;
	BOOL bInvalid = FALSE;

	Split( strArgs, _T('|'), oTokens );

	INT_PTR nArgs = oTokens.GetCount();
	*pppVars = new VARIANT* [ nArgs ]();

	for ( INT_PTR nArg = 0 ; nArg < nArgs ; nArg++ )
	{
		strToken = oTokens.GetAt( nArg );
		int nEqualPos = strToken.Find( '=' );

		// Malformatted string test
		if ( nEqualPos == -1 ) { bInvalid = TRUE; break; }

		strType.SetString( strToken.Left( nEqualPos ).Trim() );
		strValue.SetString( strToken.Mid( nEqualPos + 1 ).Trim() );

		(*pppVars)[ nArg ] = new VARIANT;
		VariantInit( (*pppVars)[ nArg ] );

		// Assign value
		if ( strType == _T("VT_BSTR") )
		{
			(*pppVars)[ nArg ]->vt = VT_BSTR;
			(*pppVars)[ nArg ]->bstrVal = CComBSTR( strValue ).Detach();
		}
		else if ( strType == _T("VT_UI2") )
		{
			USHORT nValue = 0;
			bInvalid = _stscanf( strValue, _T("%hu"), &nValue ) != 1;
			if ( bInvalid ) break;

			(*pppVars)[ nArg ]->vt = VT_UI2;
			(*pppVars)[ nArg ]->uiVal = nValue;
		}
		else if ( strType == _T("VT_UI4") )
		{
			ULONG nValue = 0;
			bInvalid = _stscanf( strValue, _T("%lu"), &nValue ) != 1;
			if ( bInvalid ) break;

			(*pppVars)[ nArg ]->vt = VT_UI4;
			(*pppVars)[ nArg ]->ulVal = nValue;
		}
		else if ( strType == _T("VT_BOOL") )
		{
			VARIANT_BOOL va = 1;
			if ( strValue.CompareNoCase( _T("true") ) == 0 )
				va = VARIANT_TRUE;
			else if ( strValue.CompareNoCase( _T("false") ) == 0 )
				va = VARIANT_FALSE;
			else
				bInvalid = TRUE;
			if ( bInvalid ) break;

			(*pppVars)[ nArg ]->vt = VT_BOOL;
			(*pppVars)[ nArg ]->boolVal = va;
		}
		else
		{
			bInvalid = TRUE; // no other types are supported
			break;
		}
	}

	if ( bInvalid ) // cleanup if invalid
	{
		DestroyVars( nArgs, pppVars );
		return -1;
	}
	return nArgs;
}

// Creates a string in format "|variant_type1=value1|variant_type2=value2|"
// from OUT variant returned by service.
// Returns: number of arguments or -1 if not applicable.

INT_PTR	CUPnPFinder::GetStringFromOutArgs(const VARIANT* pvaOutArgs, CString& strArgs)
{
	LONG nLBound = 0L, nUBound = 0L;
	HRESULT hr = GetSafeArrayBounds( pvaOutArgs->parray, &nLBound, &nUBound );
	bool bInvalid = FAILED( hr );
	CString strResult, strToken;

	if ( ! bInvalid ) // We have got the bounds of the arguments
	{
		CComVariant vaOutElement;
		strResult = '|';

		for ( LONG nIndex = nLBound ; nIndex <= nUBound && ! bInvalid ; ++nIndex )
		{
			vaOutElement.Clear();
			hr = GetVariantElement( pvaOutArgs->parray, nIndex, &vaOutElement );

			if ( SUCCEEDED( hr ) )
			{
				if ( vaOutElement.vt == VT_BSTR )
					strToken = L"VT_BSTR=";
				else if ( vaOutElement.vt == VT_UI2 )
					strToken = L"VT_UI2=";
				else if ( vaOutElement.vt == VT_UI4 )
					strToken = L"VT_UI4=";
				else if ( vaOutElement.vt == VT_BOOL )
					strToken = L"VT_BOOL=";
				else
				{
					bInvalid = true;
					break;
				}

				hr = VariantChangeType( &vaOutElement, &vaOutElement,
							VARIANT_ALPHABOOL, VT_BSTR );
				if ( SUCCEEDED( hr ) )
				{
					CString str( vaOutElement.bstrVal );
					strToken += str;
					strToken += L"|";
					strResult += strToken;
				}
				else bInvalid = true;
			}
			else
				bInvalid = true;
		} // For loop
	}

	if ( bInvalid || nLBound > nUBound ) return -1;

	strArgs = strResult;
	return  nUBound - nLBound + 1;
}

// Get SafeArray bounds
HRESULT CUPnPFinder::GetSafeArrayBounds(SAFEARRAY* psa, LONG* pLBound, LONG* pUBound)
{
	ASSERT( psa != NULL );

	HRESULT hr = SafeArrayGetLBound( psa, 1, pLBound );
	if ( FAILED( hr ) )
		return hr;

	return SafeArrayGetUBound( psa, 1, pUBound );
}

// Get Variant Element
// psa--SafeArray; nPosition--Position in the array; pvar--Variant Element being set
HRESULT CUPnPFinder::GetVariantElement(SAFEARRAY* psa, LONG pos, VARIANT* pvar)
{
	ASSERT( psa != NULL );

	return SafeArrayGetElement( psa, &pos, pvar );
}


// Destroys argument variants
void CUPnPFinder::DestroyVars(const INT_PTR nCount, VARIANT*** pppVars)
{
	VARIANT* pVar = NULL;

	ASSERT( pppVars && *pppVars );

	if( nCount == 0 ) return;

	for ( INT_PTR nArg = 0 ; nArg < nCount ; nArg++ )
	{
		pVar = (*pppVars)[ nArg ];
		if ( pVar != NULL )
		{
			VariantClear( pVar );
			delete pVar;
			pVar = NULL;
		}
	}

	delete [] *pppVars;
	*pppVars = NULL;
}

///////////////////////////////////////////////////////////////////
//   CDeviceFinderCallback
///////////////////////////////////////////////////////////////////

// Called when a device is added
// nFindData--AsyncFindHandle; pDevice--COM interface pointer of the device being added
HRESULT CDeviceFinderCallback::DeviceAdded(LONG /*nFindData*/, IUPnPDevice* pDevice)
{
	m_instance.AddDevice( pDevice, true );
	return S_OK;
}

// Called when a device is removed
// nFindData--AsyncFindHandle; bsUDN--UDN of the device being removed
HRESULT CDeviceFinderCallback::DeviceRemoved(LONG /*nFindData*/, BSTR bsUDN)
{
	m_instance.RemoveDevice( bsUDN );
	return S_OK;
}

// Called when the search is complete; nFindData--AsyncFindHandle
HRESULT CDeviceFinderCallback::SearchComplete(LONG /*nFindData*/)
{
	// StopAsyncFind must be here, do not move to OnSearchComplete
	// Otherwise, "Service died" message is shown, and it means
	// that the service still was active.
	bool bRetry = !m_instance.OnSearchComplete();
	m_instance.StopAsyncFind();
	if ( bRetry )
	{
		try
		{
			m_instance.StartDiscovery( true );
		}
		catch ( CException* e ) { e->Delete(); }
	}
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
//   CServiceCallback
////////////////////////////////////////////////////////////////////////////////

//! Called when the state variable is changed
//! \arg pus             COM interface pointer of the service;
//! \arg pszStateVarName State Variable Name;
//! \arg varValue        State Variable Value
HRESULT CServiceCallback::StateVariableChanged(IUPnPService* pService,
			LPCWSTR pszStateVarName, VARIANT varValue)
{
	CComBSTR bsServiceId;
	m_instance.m_tLastEvent = GetTickCount();

	HRESULT hr = pService->get_Id( &bsServiceId );
	if ( FAILED( hr ) )
		return CUPnPFinder::UPnPMessage( hr );
	if ( FAILED( hr = VariantChangeType( &varValue, &varValue, VARIANT_ALPHABOOL, VT_BSTR ) ) )
		return CUPnPFinder::UPnPMessage( hr );

	CString strValue( varValue.bstrVal );

	// Re-examine state variable change only when discovery was finished
	// We are not interested in the initial values; we will request them explicitly
	if ( !m_instance.IsAsyncFindRunning() )
	{
		if ( _wcsicmp( pszStateVarName, L"ExternalIPAddress" ) == 0 )
		{
			Network.AcquireLocalAddress( strValue.Trim() );
		}
	}

	theApp.Message( MSG_DEBUG, L"UPnP device state variable %s changed to %s in %s",
		pszStateVarName, strValue.IsEmpty()? L"NULL" : strValue.GetString(), bsServiceId.m_str );

	return hr;
}

//! Called when the service dies
HRESULT CServiceCallback::ServiceInstanceDied(IUPnPService* pService)
{
	CComBSTR bsServiceId;

	HRESULT hr = pService->get_Id( &bsServiceId );
	if ( SUCCEEDED( hr ) )
	{
		theApp.Message( MSG_ERROR, L"UPnP service %s died.", (LPCTSTR)bsServiceId );
		return hr;
	}

	return CUPnPFinder::UPnPMessage( hr );
}

////////////////////////////////////////////////////////////////////////////////
// Prints the appropriate UPnP error text

CString CUPnPFinder::translateUPnPResult(HRESULT hr)
{
	static std::map<HRESULT, std::string> messages;

	if ( hr >= UPNP_E_ACTION_SPECIFIC_BASE && hr <= UPNP_E_ACTION_SPECIFIC_MAX )
	{
		CString strResult;
		strResult.Format( L"Non-Standard Device Error: %i",
						 ( hr - UPNP_E_ACTION_SPECIFIC_BASE ) + FAULT_ACTION_SPECIFIC_BASE );
		return strResult;
	}

	messages[ 0 ] = "";
	messages[ UPNP_E_ROOT_ELEMENT_EXPECTED ] =      "Root Element Expected";
	messages[ UPNP_E_DEVICE_ELEMENT_EXPECTED ] =    "Device Element Expected";
	messages[ UPNP_E_SERVICE_ELEMENT_EXPECTED ] =   "Service Element Expected";
	messages[ UPNP_E_SERVICE_NODE_INCOMPLETE ] =    "Service Node Incomplete";
	messages[ UPNP_E_DEVICE_NODE_INCOMPLETE ] =     "Device Node Incomplete";
	messages[ UPNP_E_ICON_ELEMENT_EXPECTED ] =      "Icon Element Expected";
	messages[ UPNP_E_ICON_NODE_INCOMPLETE ] =       "Icon Node Incomplete";
	messages[ UPNP_E_INVALID_ACTION ] =             "Invalid Action";
	messages[ UPNP_E_INVALID_ARGUMENTS ] =          "Invalid Arguments";
	messages[ UPNP_E_OUT_OF_SYNC ] =                "Out of Sync";
	messages[ UPNP_E_ACTION_REQUEST_FAILED ] =      "Action Request Failed";
	messages[ UPNP_E_TRANSPORT_ERROR ] =            "Transport Error";
	messages[ UPNP_E_VARIABLE_VALUE_UNKNOWN ] =     "Variable Value Unknown";
	messages[ UPNP_E_INVALID_VARIABLE ] =           "Invalid Variable";
	messages[ UPNP_E_DEVICE_ERROR ] =               "Device Error";
	messages[ UPNP_E_PROTOCOL_ERROR ] =             "Protocol Error";
	messages[ UPNP_E_ERROR_PROCESSING_RESPONSE ] =  "Error Processing Response";
	messages[ UPNP_E_DEVICE_TIMEOUT ] =             "Device Timeout";
	messages[ UPNP_E_INVALID_DOCUMENT ] =           "Invalid Document";
	messages[ UPNP_E_EVENT_SUBSCRIPTION_FAILED ] =  "Event Subscription Failed";
	messages[ E_FAIL ] =                            "Generic failure";

	return CString( messages[ hr ].c_str() );
}

HRESULT CUPnPFinder::UPnPMessage(HRESULT hr)
{
	CString strError = translateUPnPResult( hr );
	if ( ! strError.IsEmpty() )
		theApp.Message( MSG_DEBUG, L"upnp: " + strError );
	return hr;
}
