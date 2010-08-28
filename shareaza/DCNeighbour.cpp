//
// DCNeighbour.cpp
//
// Copyright (c) Shareaza Development Team, 20010.
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
#include "DCClient.h"
#include "DCNeighbour.h"
#include "HostCache.h"
#include "LibraryMaps.h"
#include "LocalSearch.h"
#include "Network.h"
#include "Neighbours.h"
#include "Security.h"
#include "Settings.h"
#include "UploadQueue.h"
#include "UploadQueues.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CDCNeighbour::CDCNeighbour()
{
	m_nNodeType = ntHub;
}

CDCNeighbour::~CDCNeighbour()
{
}

BOOL CDCNeighbour::ConnectTo(const IN_ADDR* pAddress, WORD nPort, BOOL bAutomatic)
{
	CString sHost( inet_ntoa( *pAddress ) );

	if ( __super::ConnectTo( pAddress, nPort ) )
	{
		WSAEventSelect( m_hSocket, Network.GetWakeupEvent(),
			FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE );

		theApp.Message( MSG_INFO, IDS_CONNECTION_ATTEMPTING,
			(LPCTSTR)sHost, htons( m_pHost.sin_port ) );
	}
	else
	{
		theApp.Message( MSG_ERROR, IDS_CONNECTION_CONNECT_FAIL,
			(LPCTSTR)sHost );
		return FALSE;
	}

	m_nState = nrsConnecting;

	m_bAutomatic = bAutomatic;

	Neighbours.Add( this );

	return TRUE;
}

BOOL CDCNeighbour::Send(CPacket* pPacket, BOOL bRelease, BOOL /*bBuffered*/)
{
	Write( pPacket );

	LogOutgoing();

	m_nOutputCount++;

	if ( bRelease ) pPacket->Release();

	return TRUE;
}

BOOL CDCNeighbour::OnConnected()
{
	if ( ! __super::OnConnected() )
		return FALSE;

	theApp.Message( MSG_INFO, IDS_CONNECTION_CONNECTED,
		(LPCTSTR)m_sAddress );

	m_nState = nrsHandshake1; // Waiting for $Lock

	return TRUE;
}

void CDCNeighbour::OnDropped()
{
	if ( m_nState == nrsConnecting )
	{
		Close( IDS_CONNECTION_REFUSED );
	}
	else
	{
		Close( IDS_CONNECTION_DROPPED );
	}
}

BOOL CDCNeighbour::OnCommand(const std::string& strCommand, const std::string& strParams)
{
	m_nInputCount++;
	m_tLastPacket = GetTickCount();

	if ( strCommand == "$Search" )
	{
		// Search request
		// $Search SenderIP:SenderPort (F|T)?(F|T)?Size?Type?String|
	
		std::string::size_type nPos = strParams.find( ' ' );
		if ( nPos != std::string::npos )
		{
			std::string strAddress = strParams.substr( 0, nPos );
			std::string strSearch = strParams.substr( nPos + 1 );
			nPos = strAddress.find( ':' );
			if ( nPos != std::string::npos )
			{
				DWORD nAddress = inet_addr( strAddress.substr( 0, nPos ).c_str() );
				int nPort = atoi( strAddress.substr( nPos + 1 ).c_str() );
				if ( nPort > 0 && nPort <= USHRT_MAX && nAddress != INADDR_NONE &&
					! Network.IsFirewalledAddress( (const IN_ADDR*)&nAddress ) &&
					! Network.IsReserved( (const IN_ADDR*)&nAddress ) &&
					! Security.IsDenied( (const IN_ADDR*)&nAddress ) )
				{
					OnSearch( (const IN_ADDR*)&nAddress, (WORD)nPort, strSearch );
				}
			}
		}

		return TRUE;
	}
	else if ( strCommand == "$MyINFO" )
	{
		// User info
		// $MyINFO $ALL nick description<tag>$ $connection$e-mail$sharesize$|

		m_nState = nrsConnected;

		return TRUE;
	}
	else if ( strCommand == "$Quit" )
	{
		// User leave hub
		// $Quit nick|

		return TRUE;
	}
	else if ( strCommand == "$HubName" )
	{
		// Name of hub
		// $HubName RemoteNick [Version]|

		m_nState = nrsConnected;

		m_sServerName = strParams.c_str();

		std::string::size_type nPos = strParams.find( ' ' );
		if ( nPos != std::string::npos )
			// Cut-off version
			m_sRemoteNick = strParams.substr( 0, nPos).c_str();
		else
			m_sRemoteNick = strParams.c_str();

		return TRUE;
	}
	else if ( strCommand == "$OpList" )
	{
		// Hub operators list
		// $OpList operator1|

		m_nState = nrsConnected;

		return TRUE;
	}
	else if ( strCommand == "$ConnectToMe" )
	{
		// Client connection request
		// $ConnectToMe RemoteNick SenderIp:SenderPort|
		// or
		// $ConnectToMe SenderNick RemoteNick SenderIp:SenderPort|

		std::string::size_type nPos = strParams.rfind( ' ' );
		if ( nPos != std::string::npos )
		{
			std::string strAddress = strParams.substr( nPos + 1 );
			std::string strSenderNick, strRemoteNick = strParams.substr( 0, nPos );
			nPos = strRemoteNick.find( ' ' );
			if ( nPos != std::string::npos )
			{
				strSenderNick = strRemoteNick.substr( nPos + 1 );
				strRemoteNick = strRemoteNick.substr( 0, nPos );
			}
			nPos = strAddress.find( ':' );
			if ( nPos != std::string::npos )
			{
				DWORD nAddress = inet_addr( strAddress.substr( 0, nPos ).c_str() );
				int nPort = atoi( strAddress.substr( nPos + 1 ).c_str() );
				if ( nPort > 0 && nPort <= USHRT_MAX && nAddress != INADDR_NONE &&
					m_sNick == strRemoteNick.c_str() &&
					! Network.IsFirewalledAddress( (const IN_ADDR*)&nAddress ) &&
					! Network.IsReserved( (const IN_ADDR*)&nAddress ) &&
					! Security.IsDenied( (const IN_ADDR*)&nAddress ) )
				{
					// Ok
					CDCClient* pClient = new CDCClient();
					pClient->m_sNick = m_sNick;
					pClient->ConnectTo( (const IN_ADDR*)&nAddress, (WORD)nPort );
				}
				else
				{
					// Wrong nick, bad IP
				}
			}
		}
		return TRUE;
	}
	else if ( strCommand == "$ForceMove" )
	{
		// User redirection
		// $ForceMove IP:Port|

		CString strAddress;
		int nPort = 0;
		std::string::size_type nPos = strParams.rfind( ':' );
		if ( nPos != std::string::npos )
		{
			strAddress = strParams.substr( 0, nPos ).c_str();
			nPort = atoi( strParams.substr( nPos + 1 ).c_str() );
		}
		else
			strAddress = strParams.c_str();

		Network.ConnectTo( strAddress, nPort, PROTOCOL_DC );

		return TRUE;
	}

	// Unknown command - ignoring
	return TRUE;
}

BOOL CDCNeighbour::OnSearch(const IN_ADDR* pAddress, WORD nPort, std::string& strSearch)
{
	std::string::size_type nPos = strSearch.find( '?' );
	if ( nPos == std::string::npos )
		return TRUE;

	std::string strSizeRestriced = strSearch.substr( 0, nPos );
	bool bSizeRestriced = ( strSizeRestriced == "T" );
	strSearch = strSearch.substr( nPos + 1 );
	nPos = strSearch.find( '?' );
	if ( nPos == std::string::npos || ( ! bSizeRestriced && strSizeRestriced != "F" ) )
		return TRUE;

	std::string strIsMaxSize = strSearch.substr( 0, nPos );
	bool bIsMaxSize = ( strIsMaxSize == "T" );
	strSearch = strSearch.substr( nPos + 1 );
	nPos = strSearch.find( '?' );
	if ( nPos == std::string::npos || ( ! bIsMaxSize && strIsMaxSize != "F" ) )
		return TRUE;

	QWORD nSize = 0;
	std::string strSize = strSearch.substr( 0, nPos );
	strSearch = strSearch.substr( nPos + 1 );
	nPos = strSearch.find( '?' );
	if ( nPos == std::string::npos ||
		sscanf_s( strSize.c_str(), "%I64u", &nSize ) != 1 )
		return TRUE;

	std::string strType = strSearch.substr( 0, nPos );
	int nType = atoi( strType.c_str() );
	strSearch = strSearch.substr( nPos + 1 );
	if ( nType < 1 || nType > 9 )
		// Unknown search type
		return TRUE;
	
	CQuerySearchPtr pSearch = new CQuerySearch( TRUE );

	pSearch->m_pEndpoint = m_pHost;

	if ( bSizeRestriced )
	{
		if ( bIsMaxSize )
			pSearch->m_nMaxSize = nSize;
		else
			pSearch->m_nMinSize = nSize;
	}
	else if ( nSize )
	{
		pSearch->m_nMinSize = pSearch->m_nMaxSize = nSize;
	}

	if ( nType == 9 )
	{
		// Hash search

		if ( strSearch.substr( 0, 4 ) != "TTH:" )
			// Unknown hash prefix
			return TRUE;

		if ( ! pSearch->m_oTiger.fromString( CA2W( strSearch.substr( 4 ).c_str() ) ) )
			// Invalid TigerTree hash encoding
			return TRUE;
	}
	else
	{
		// Keywords search

		pSearch->m_sSearch = strSearch.c_str();
		pSearch->m_sSearch.Replace( _T('$'), _T(' ') );
	}

	if ( ! pSearch->CheckValid( true ) )
		// Invalid search
		return TRUE;

	CLocalSearch* pLocalSearch = new CLocalSearch( pSearch, this );

	// Send result (if any) directly to client via UDP
	pLocalSearch->m_pEndpoint.sin_addr = *pAddress;
	pLocalSearch->m_pEndpoint.sin_port = htons( nPort );
	pLocalSearch->m_bUDP = TRUE;

	Network.OnQuerySearch( pLocalSearch );

	return TRUE;
}

BOOL CDCNeighbour::OnLock()
{
	if ( m_nState < nrsHandshake2 )
	{
		m_nState = nrsHandshake2;	// Waiting for $Hello
	}

	m_nInputCount++;
	m_tLastPacket = GetTickCount();

	if ( m_nNodeType == ntHub )
	{
		HostCache.DC.Add( &m_pHost.sin_addr, htons( m_pHost.sin_port ) );
	}

	if ( m_bExtended )
	{
		Write( _P("$Supports NoGetINFO NoHello UserIP2 TTHSearch |") );
	}

	Write( _P("$Key ") );
	Write( m_strKey.c_str(), m_strKey.size() );
	Write( _P("|") );

	Write( _P("$ValidateNick ") );
	Write( m_sNick );
	Write( _P("|") );

	LogOutgoing();

	m_nOutputCount++;

	return TRUE;
}

BOOL CDCNeighbour::OnSupport()
{
	m_nInputCount++;
	m_tLastPacket = GetTickCount();

	return TRUE;
}

BOOL CDCNeighbour::OnChat(const std::string& strMessage)
{
	m_nInputCount++;
	m_tLastPacket = GetTickCount();

	return TRUE;
}

BOOL CDCNeighbour::OnHello()
{
	m_nState = nrsConnected;

	// NMDC version
	Write( _P("$Version 1,0091|") );

	// Request nick list
	//Write( _P("$GetNickList|") );

	// $MyINFO $ALL nick description<tag>$ $connection$e-mail$sharesize$|

	CUploadQueue* pQueue = UploadQueues.SelectQueue(
		PROTOCOL_DC, NULL, 0, CUploadQueue::ulqBoth, NULL );

	QWORD nMyVolume = 0;
	LibraryMaps.GetStatistics( NULL, &nMyVolume );

	CString sInfo;
	sInfo.Format( _T("$MyINFO $ALL %s %s<%s V:%s,M:%c,H:%u/%u/%u,S:%u>$ $%.2f%c$%s$%I64u$|"),
		// Registered nick
		m_sNick,
		// Description
		WEB_SITE_T,
		// Client name
		CLIENT_NAME_T,
		// Client version
		theApp.m_sVersion,
		// User is in active(A), passive(P), or SOCKS5(5) mode 
		( Network.IsFirewalled( CHECK_BOTH ) ? _T('P') : _T('A') ),
		// Number of connected hubs as regular user
		Neighbours.GetCount( PROTOCOL_DC, nrsConnected, ntHub ),
		// Number of connected hubs as VIP
		0,
		// Number of connected hubs as operator
		0,
		// Number of upload slots
		( pQueue ? pQueue->m_nMaxTransfers : 0 ),
		// Upload speed (Mbit/s)
		(float)Settings.Bandwidth.Uploads * Bytes / ( Kilobits * Kilobits ),
		// User status: Normal(1), Away(2,3), Server(4,5), Server Away(6,7)
		1,
		// E-mail
		MyProfile.GetContact( _T("Email") ),
		// Share size (bytes)
		nMyVolume << 10 );
	Write( sInfo );

	LogOutgoing();

	m_nOutputCount++;

	return TRUE;
}
