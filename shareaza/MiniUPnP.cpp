//
// MiniUPnP.cpp
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
#include "MiniUPnP.h"

// MiniUPnPc library
// Copyright (c) 2005-2015 Thomas Bernard
#include "..\MiniUPnPc\miniupnpc\miniupnpc.h"
#include "..\MiniUPnPc\miniupnpc\upnpcommands.h"
#pragma comment( lib, "miniupnpc" )

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CMiniUPnP::CMiniUPnP()
	: m_nExternalTCPPort	( 0 )
	, m_nExternalUDPPort	( 0 )
{
}

CMiniUPnP::~CMiniUPnP()
{
	StopAsyncFind();
}

void CMiniUPnP::StartDiscovery()
{
	BeginThread( "MiniUPnP" );
}

void CMiniUPnP::StopAsyncFind()
{
	CloseThread();
}

void CMiniUPnP::DeletePorts()
{
	int result;
	CStringA sPort;

	if ( m_nExternalTCPPort )
	{
		sPort.Format( "%u", m_nExternalTCPPort );
		result = UPNP_DeletePortMapping( m_sControlURL, m_sServiceType, sPort, "TCP", 0 );
		if ( result == UPNPCOMMAND_SUCCESS )
			theApp.Message( MSG_DEBUG, _T("UPnP successfully unmapped TCP port %u."), m_nExternalTCPPort );
		else
			theApp.Message( MSG_DEBUG, _T("UPnP failed to unmap TCP port %u, error %d."), m_nExternalTCPPort, result );
		m_nExternalTCPPort = 0;
	}

	if ( m_nExternalUDPPort )
	{
		sPort.Format( "%u", m_nExternalUDPPort );
		result = UPNP_DeletePortMapping( m_sControlURL, m_sServiceType, sPort, "UDP", 0 );
		if ( result == UPNPCOMMAND_SUCCESS )
			theApp.Message( MSG_DEBUG, _T("UPnP successfully unmapped UDP port %u."), m_nExternalUDPPort );
		else
			theApp.Message( MSG_DEBUG, _T("UPnP failed to unmap UDP port %u, error %d."), m_nExternalUDPPort, result );
		m_nExternalUDPPort = 0;
	}
}

bool CMiniUPnP::IsAsyncFindRunning()
{
	return IsThreadAlive();
}

void CMiniUPnP::OnRun()
{
	int result;
	BOOL bSuccess = FALSE;

	int error = 0;
	if ( UPNPDev* pDevList = upnpDiscover( Settings.Connection.UPnPTimeout, NULL, NULL, UPNP_LOCAL_PORT_ANY, FALSE, 2, &error ) )
	{
		for ( UPNPDev* pDevice = pDevList; ! bSuccess && pDevice && IsThreadEnabled(); pDevice = pDevice->pNext )
		{
			theApp.Message( MSG_DEBUG, _T("UPnP device: %s : %s"), (LPCTSTR)CA2T( pDevice->descURL ), (LPCTSTR)CA2T( pDevice->st ) );

			UPNPUrls urls = {};
			IGDdatas data = {};
			char internalIPAddress[ 16 ] = {};
			result = UPNP_GetValidIGD( pDevice, &urls, &data, internalIPAddress, sizeof( internalIPAddress ) );
			if ( result )
			{
				m_sServiceType = data.first.servicetype;
				m_sControlURL = urls.controlURL;
				FreeUPNPUrls( &urls );

				switch ( result )
				{
				case 1:
					theApp.Message( MSG_DEBUG, _T("UPnP IGD found (valid and connected) : %s"), (LPCTSTR)CA2T( m_sControlURL ) );
					break;
				case 2:
					theApp.Message( MSG_DEBUG, _T("UPnP IGD found (valid but not connected) . Trying to continue anyway... : %s"), (LPCTSTR)CA2T( m_sControlURL ) );
					break;
				default:
					theApp.Message( MSG_DEBUG, _T("UPnP IGD found (not valid). Trying to continue anyway... : %s"), (LPCTSTR)CA2T( m_sControlURL ) );
				}

				result = UPNP_GetExternalIPAddress( m_sControlURL, m_sServiceType, m_sExternalAddress.GetBuffer( 16 ) );
				m_sExternalAddress.ReleaseBuffer();
				if ( result == UPNPCOMMAND_SUCCESS && ! m_sExternalAddress.IsEmpty() )
				{
					WORD nPort = (WORD)Settings.Connection.InPort;
					bool bRandomPort = Settings.Connection.RandomPort;

					if ( nPort == 0 ) // random port
						nPort = Network.RandomPort();

					// Try to map both ports
					for ( int i = 0; IsThreadEnabled() && i < 5; ++i )
					{
						CStringA sPort;
						sPort.Format( "%u", nPort );

						CString strInfo;
						strInfo.Format( _T("%s at %s:%u"), CLIENT_NAME_T _T(" TCP"), (LPCTSTR)CA2T( internalIPAddress ), nPort );
						result = UPNP_AddPortMapping( m_sControlURL, m_sServiceType, sPort, sPort, internalIPAddress, (LPCSTR)CT2A( strInfo ), "TCP", NULL, NULL );
						if ( result == UPNPCOMMAND_SUCCESS )
						{
							char sRealPort[ 6 ] = {};
							result = UPNP_GetSpecificPortMappingEntry( m_sControlURL, m_sServiceType, sPort, "TCP", NULL, internalIPAddress, sRealPort, NULL, NULL, NULL );
							if ( result == UPNPCOMMAND_SUCCESS )
							{
								m_nExternalTCPPort = (WORD)atoi( sRealPort );
								theApp.Message( MSG_DEBUG, _T("UPnP successfully mapped TCP port %u."), m_nExternalTCPPort );

								strInfo.Format( _T("%s at %s:%u"), CLIENT_NAME_T _T(" UDP"), (LPCTSTR)CA2T( internalIPAddress ), nPort );
								result = UPNP_AddPortMapping( m_sControlURL, m_sServiceType, sPort, sPort, internalIPAddress, (LPCSTR)CT2A( strInfo ), "UDP", NULL, NULL );
								if ( result == UPNPCOMMAND_SUCCESS )
								{
									*sRealPort = '\0';
									result = UPNP_GetSpecificPortMappingEntry( m_sControlURL, m_sServiceType, sPort, "UDP", NULL, internalIPAddress, sRealPort, NULL, NULL, NULL );
									if ( result == UPNPCOMMAND_SUCCESS )
									{
										m_nExternalUDPPort = (WORD)atoi( sRealPort );
										theApp.Message( MSG_DEBUG, _T("UPnP successfully mapped UDP port %u."), m_nExternalUDPPort );

										bSuccess =  ( m_nExternalTCPPort != 0 ) &&
													( m_nExternalUDPPort != 0 ) &&
													( m_nExternalTCPPort == m_nExternalUDPPort ) &&
													( m_nExternalTCPPort == nPort );									
										if ( bSuccess )
										{
											Network.AcquireLocalAddress( (LPCTSTR)CA2T( m_sExternalAddress ), nPort );

											Settings.Connection.InPort = nPort;
											Settings.Connection.RandomPort = bRandomPort;
											break;
										}
									}
									else
										theApp.Message( MSG_DEBUG, _T("UPnP failed to get mapped UDP port %u, error %d."), nPort, result );
								}
								else
									theApp.Message( MSG_DEBUG, _T("UPnP failed to map UDP port %u, error %d."), nPort, result );

								DeletePorts();
							}
							else
								theApp.Message( MSG_DEBUG, _T("UPnP failed to get mapped TCP port %u, error %d."), nPort, result );
						}
						else
							theApp.Message( MSG_DEBUG, _T("UPnP failed to map TCP port %u, error %d."), nPort, result );

						Sleep( 200 );

						// Change port to random
						nPort = Network.RandomPort();
						bRandomPort = true;
					}
				}
				else
					theApp.Message( MSG_DEBUG, _T("UPnP failed to get external IP address, error %d."), result  );
			}
			else
				theApp.Message( MSG_DEBUG, _T("UPnP bad device.") );
		}
		freeUPNPDevlist( pDevList );
	}
	else
		theApp.Message( MSG_DEBUG, _T("UPnP found no devices.") );

	if ( bSuccess )
	{
		Network.OnMapSuccess();
	}
	else
	{
		Network.OnMapFailed();
	}
}
