//
// UPnPNAT.cpp
//
// Copyright (c) Shareaza Development Team, 2014-2015.
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
#include "UPnPNAT.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CUPnPNAT::CUPnPNAT()
	: m_nExternalTCPPort	( 0 )
	, m_nExternalUDPPort	( 0 )
{
}

CUPnPNAT::~CUPnPNAT()
{
	StopAsyncFind();
}

void CUPnPNAT::StartDiscovery()
{
	BeginThread( "UPnPNAT" );
}

void CUPnPNAT::StopAsyncFind()
{
	CloseThread();
}

void CUPnPNAT::DeletePorts()
{
	if ( m_nExternalTCPPort || m_nExternalUDPPort )
	{
		CComPtr< IUPnPNAT > pNat;
		HRESULT hr = pNat.CoCreateInstance( __uuidof( UPnPNAT ) );
		if ( SUCCEEDED( hr ) && pNat )
		{
			// Retrieve the mappings collection
			CComPtr< IStaticPortMappingCollection >	pCollection;
			hr = pNat->get_StaticPortMappingCollection( &pCollection );
			if ( SUCCEEDED( hr ) && pCollection )
			{
				// Unmap
				if ( m_nExternalTCPPort )
				{
					hr = pCollection->Remove( m_nExternalTCPPort, CComBSTR( L"TCP" ) );
					if ( SUCCEEDED( hr ) )
						theApp.Message( MSG_DEBUG, _T("UPnP successfully unmapped TCP port %u."), m_nExternalTCPPort );
					else
						theApp.Message( MSG_DEBUG, _T("UPnP failed to unmap TCP port %u, error %d."), m_nExternalTCPPort, hr );
					m_nExternalTCPPort = 0;
				}
				if ( m_nExternalUDPPort )
				{
					hr = pCollection->Remove( m_nExternalUDPPort, CComBSTR( L"UDP" ) );
					if ( SUCCEEDED( hr ) )
						theApp.Message( MSG_DEBUG, _T("UPnP successfully unmapped UDP port %u."), m_nExternalUDPPort );
					else
						theApp.Message( MSG_DEBUG, _T("UPnP failed to unmap UDP port %u, error %d."), m_nExternalUDPPort, hr );
					m_nExternalUDPPort = 0;
				}
			}
			else
				theApp.Message( MSG_DEBUG, _T("UPnP failed to get mapping collection, error %d."), hr );
		}
		else
			theApp.Message( MSG_DEBUG, _T("UPnP failed to connect to UPnPNAT subsystem, error %d."), hr );
	}
}

bool CUPnPNAT::IsAsyncFindRunning()
{
	return IsThreadAlive();
}

void CUPnPNAT::OnRun()
{
	BOOL bSuccess = FALSE;

	CComPtr< IUPnPNAT > pNat;
	HRESULT hr = pNat.CoCreateInstance( __uuidof( UPnPNAT ) );
	if ( SUCCEEDED( hr ) && pNat )
	{
		// Retrieve the mappings collection
		CComPtr< IStaticPortMappingCollection >	pCollection;
		for ( int i = 0; IsThreadEnabled() && i < 5; ++i )
		{
			pCollection.Release();
			hr = pNat->get_StaticPortMappingCollection( &pCollection );
			if ( SUCCEEDED( hr ) && pCollection )
			{
				break;
			}
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

				WORD nPort = (WORD)Settings.Connection.InPort;
				bool bRandomPort = Settings.Connection.RandomPort;

				if ( nPort == 0 ) // random port
					nPort = Network.RandomPort();

				// Try to map both ports
				CString strInfo;
				for ( int i = 0; IsThreadEnabled() && i < 5; ++i )
				{
					strInfo.Format( _T("%s at %s:%u"), CLIENT_NAME_T _T(" TCP"), (LPCTSTR)strLocalIP, nPort );
					m_nExternalTCPPort = MapPort( pCollection, strLocalIP, nPort, L"TCP", strInfo );
					if ( m_nExternalTCPPort  )
						theApp.Message( MSG_DEBUG, _T("UPnP successfully mapped TCP port %u."), m_nExternalTCPPort );

					strInfo.Format( _T("%s at %s:%u"), CLIENT_NAME_T _T(" UDP"), (LPCTSTR)strLocalIP, nPort );
					m_nExternalUDPPort = MapPort( pCollection, strLocalIP, nPort, L"UDP", strInfo );
					if ( m_nExternalUDPPort  )
						theApp.Message( MSG_DEBUG, _T("UPnP successfully mapped UDP port %u."), m_nExternalUDPPort );

					bSuccess =  ( m_nExternalTCPPort != 0 ) &&
								( m_nExternalUDPPort != 0 ) &&
								( m_nExternalTCPPort == m_nExternalUDPPort ) &&
								( m_nExternalTCPPort == nPort );
					if ( bSuccess )
					{
						Network.AcquireLocalAddress( m_sExternalAddress, nPort );

						Settings.Connection.InPort = nPort;
						Settings.Connection.RandomPort = bRandomPort;
						break;
					}

					DeletePorts();

					Sleep( 200 );

					// Change port to random
					nPort = Network.RandomPort();
					bRandomPort = true;
				}
			}
			else
				theApp.Message( MSG_DEBUG, _T("UPnP failed to get local IP address, error %u."), ret );
		}
		else
			theApp.Message( MSG_DEBUG, _T("UPnP failed to get mapping collection, error %d."), hr );
	}
	else
		theApp.Message( MSG_DEBUG, _T("UPnP failed to connect to UPnPNAT subsystem, error %d."), hr );

	if ( bSuccess )
	{
		Network.OnMapSuccess();
	}
	else
	{
		Network.OnMapFailed();
	}
}

WORD CUPnPNAT::MapPort(IStaticPortMappingCollection* pCollection, LPCWSTR szLocalIP, WORD nPort, LPCWSTR szProtocol, LPCWSTR szDescription)
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
			return 0;
	}

	CComBSTR bstrAddress;
	hr = pMapping->get_ExternalIPAddress( &bstrAddress );
	m_sExternalAddress = bstrAddress;

	long nExternalPort = 0;
	hr = pMapping->get_ExternalPort( &nExternalPort );

	// Checking port number
	long nInternalPort = 0;
	hr = pMapping->get_InternalPort( &nInternalPort );
	if ( FAILED( hr ) || (WORD)nInternalPort != nPort )
	{
		hr = pMapping->EditInternalPort( nPort );
		if ( FAILED( hr ) )
			return (WORD)nExternalPort;
	}

	// Checking enable state
	VARIANT_BOOL bEnabled = VARIANT_FALSE;
	hr = pMapping->get_Enabled( &bEnabled );
	if ( FAILED( hr ) || bEnabled == VARIANT_FALSE )
	{
		hr = pMapping->Enable( VARIANT_TRUE );
		if ( FAILED( hr ) )
			return (WORD)nExternalPort;
	}

	return (WORD)nExternalPort;
}
