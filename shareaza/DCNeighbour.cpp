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
#include "DCNeighbour.h"
#include "HostCache.h"
#include "LibraryMaps.h"
#include "Network.h"
#include "Neighbours.h"
#include "GProfile.h"
#include "Settings.h"
#include "UploadQueue.h"
#include "UploadQueues.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CDCNeighbour::CDCNeighbour()
	: CNeighbour( PROTOCOL_DC )
	, m_bExtended( FALSE )
	, m_sNick( MyProfile.GetNick() )
{
	m_nNodeType = ntHub;

	// Make DC++ safe nick
	m_sNick.Replace( _T('|'), _T('_') );
	m_sNick.Replace( _T('$'), _T('_') );
	m_sNick.Replace( _T('<'), _T('_') );
}

CDCNeighbour::~CDCNeighbour()
{
}

BOOL CDCNeighbour::ConnectTo(const IN_ADDR* pAddress, WORD nPort, BOOL bAutomatic)
{
	CString sHost( inet_ntoa( *pAddress ) );

	if ( CNeighbour::ConnectTo( pAddress, nPort ) )
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

BOOL CDCNeighbour::OnRead()
{
	if ( ! CNeighbour::OnRead() )
		return FALSE;

	std::string strLine;
	if ( ! ReadCommand( strLine ) )
		return TRUE;

	m_nInputCount++;
	m_tLastPacket = GetTickCount();

	theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING,
		_T("%s >> %s"), (LPCTSTR)m_sAddress, (LPCTSTR)CA2CT( strLine.c_str() ) );

	if ( strLine.size() == 0 )
	{
		// Ping, i.e. received only "|" message
		return TRUE;
	}

	if ( strLine[ 0 ] != '$' )
	{
		// Chat
		return TRUE;
	}

	std::string::size_type nPos = strLine.find( ' ' );
	if ( nPos != std::string::npos )
		return OnCommand( strLine.substr( 0, nPos ), strLine.substr( nPos + 1 ) );
	else
		return OnCommand( strLine, std::string() );
}

BOOL CDCNeighbour::ReadCommand(std::string& strLine)
{
	CLockedBuffer pInput( GetInput() );

	if ( ! pInput->m_nLength )
		return FALSE;

	DWORD nLength = 0;
	for ( ; nLength < pInput->m_nLength ; nLength++ )
	{
		if ( pInput->m_pBuffer[ nLength ] == '|' )
			break;
	}

	if ( nLength >= pInput->m_nLength )
		return FALSE;

	strLine.append( (const char*)pInput->m_pBuffer, nLength );

	pInput->Remove( nLength + 1 );

	return TRUE;
}

BOOL CDCNeighbour::OnWrite()
{
	LogOutgoing();

	return CNeighbour::OnWrite();
}

BOOL CDCNeighbour::OnCommand(const std::string& strCommand, const std::string& strParams)
{
	if ( strCommand == "$Lock" )
	{
		if ( m_nNodeType == ntHub )
		{
			HostCache.DC.Add( &m_pHost.sin_addr, htons( m_pHost.sin_port ) );
		}

		if ( m_nState < nrsHandshake2 )
		{
			m_nState = nrsHandshake2;	// Send $Support, $Key and $ValidateNick
		}

		m_bExtended = ( strParams.substr( 0, 16 ) == "EXTENDEDPROTOCOL" );

		std::string strLock;
		std::string::size_type nPos = strParams.find( " Pk=" );
		if ( nPos != std::string::npos )
		{
			// Good way
			strLock = strParams.substr( 0, nPos );
			m_sUserAgent = strParams.substr( nPos + 4 ).c_str();
		}
		else
		{
			// Bad way
			nPos = strParams.find( ' ' );
			if ( nPos != std::string::npos )
				strLock = strParams.substr( 0, nPos );
			else
				// Very bad way
				strLock = strParams;
		}

		return SendKey( strLock );
	}
	else if ( strCommand == "$Supports" )
	{
		m_oFeatures.RemoveAll();
		for ( CString strFeatures( strParams.c_str() ); ! strFeatures.IsEmpty(); )
		{
			CString strFeature = strFeatures.SpanExcluding( _T(" ") );
			strFeatures = strFeatures.Mid( strFeature.GetLength() + 1 );
			if ( strFeature.IsEmpty() )
				continue;
			strFeature.MakeLower();
			if ( m_oFeatures.Find( strFeature ) == NULL )
			{
				m_oFeatures.AddTail( strFeature );
			}
			else
			{
				// Duplicate feature
			}				
		}
		return TRUE;
	}
	else if ( strCommand == "$HubName" )
	{
		m_nState = nrsConnected;

		m_sServerName = strParams.c_str();

		return TRUE;
	}
	else if ( strCommand == "$Hello" )
	{
		m_nState = nrsConnected;

		m_sNick = strParams.c_str();

		return SendVersion();
	}
	else if ( strCommand == "$OpList" )
	{
		// Hub operator list
		return TRUE;
	}
	else if ( strCommand == "$MyINFO" )
	{
		// Client info
		return TRUE;
	}
	else if ( strCommand == "$ConnectToMe" )
	{
		// Client connection request
		std::string::size_type nPos = strParams.rfind( ' ' );
		if ( nPos != std::string::npos )
		{
			std::string strNick = strParams.substr( 0, nPos );
			std::string strAddress = strParams.substr( nPos + 1 );
			nPos = strAddress.find( ':' );
			if ( nPos != std::string::npos )
			{
				DWORD nAddress = inet_addr( strAddress.substr( 0, nPos ).c_str() );
				WORD nPort = (WORD)atoi( strAddress.substr( nPos + 1 ).c_str() );

				// Ok
				nAddress, nPort;
			}
		}
		return TRUE;
	}

	// Unknown command - ignoring
	return TRUE;
}

BOOL CDCNeighbour::SendKey(const std::string& strLock)
{
	if ( m_bExtended )
	{
		// UserCommand	- This indicates support for $UserCommand, which is a standard way of adding hub-specific shortcuts to the client.
		// NoGetINFO	- This indicates that hub doesn't need to receive a $GetINFO from a client to send out $MyINFO.
		// NoHello		- This indicates that the client doesn't need either $Hello or $NickList to be sent to it when connecting to a hub.
		// UserIP2		- This signals support for v2 of the $UserIP command.
		// TTHSearch	- This indicates that the client supports searching for queued files by TTH.
		Write( _P("$Supports UserCommand NoGetINFO NoHello UserIP2 TTHSearch |") );

		m_nOutputCount ++;
	}

	std::string strKey = MakeKey( strLock );
	Write( _P("$Key ") );
	Write( strKey.c_str(), strKey.size() );
	Write( _P("|") );
	m_nOutputCount ++;

	Write( _P("$ValidateNick ") );
	Write( m_sNick );
	Write( _P("|") );
	m_nOutputCount ++;

	return TRUE;
}

BOOL CDCNeighbour::SendVersion()
{
	Write( _P("$Version 1,0091|") );
	m_nOutputCount++;

	//Write( _P("$GetNickList|") );
	//m_nOutputCount++;

	// $MyINFO $ALL nick description<tag>$ $connection$e-mail$sharesize$|

	Write( _P("$MyINFO $ALL ") );
	Write( m_sNick );
	Write( _P(" " WEB_SITE "<" CLIENT_NAME) );
	Write( _P(" V:") );
	Write( theApp.m_sVersion );

	// If the user is in active(A), passive(P), or SOCKS5(5) mode 
	Write( _P(",M:") );
	if ( Network.IsFirewalled( CHECK_BOTH ) )
		Write( _P("P") );
	else
		Write( _P("A") );

	// How many hubs the Shareaza is on and what is his status on the hubs.
	// The first number means a normal user, second means VIP/registered
	// hubs and the last one operator hubs
	Write( _P(",H:1/0/0") );

	CUploadQueue* pQueue = UploadQueues.SelectQueue(
		m_nProtocol, NULL, 0, CUploadQueue::ulqBoth, NULL );

	QWORD nMyVolume = 0;
	LibraryMaps.GetStatistics( NULL, &nMyVolume );
	CString sInfo;
	sInfo.Format( _T(",S:%u>$ $%.2f%c$%s$%I64u$|"),
		( pQueue ? pQueue->m_nMaxTransfers : 0 ),
		(float)Settings.Bandwidth.Uploads * Bytes / ( Kilobits * Kilobits ),
		1, // Normal
		MyProfile.GetContact( _T("Email") ),
		nMyVolume << 10 );
	Write( sInfo );
	m_nOutputCount++;

	return TRUE;
}

std::string CDCNeighbour::MakeKey(const std::string& aLock)
{
	if ( aLock.size() < 3 )
		return std::string();

	auto_array< BYTE > temp( new BYTE[ aLock.size() ] );
	size_t extra = 0;
	BYTE v1 = (BYTE)( (BYTE)aLock[ 0 ] ^ 5 );
	v1 = (BYTE)( ( ( v1 >> 4 ) | ( v1 << 4 ) ) & 0xff );
	temp[ 0 ] = v1;
	for ( size_t i = 1; i < aLock.size(); i++ )
	{
		v1 = (BYTE)( (BYTE)aLock[ i ] ^ (BYTE)aLock[ i - 1 ] );
		v1 = (BYTE)( ( ( v1 >> 4 ) | ( v1 << 4 ) ) & 0xff );
		temp[ i ] = v1;
		if ( IsExtra( temp[ i ] ) )
			extra++;
	}
	temp[ 0 ] = (BYTE)( temp[ 0 ] ^ temp[ aLock.size() - 1 ] );
	if ( IsExtra( temp[ 0 ] ) )
		extra++;

	return KeySubst( &temp[ 0 ], aLock.size(), extra );
}

std::string CDCNeighbour::KeySubst(const BYTE* aKey, size_t len, size_t n)
{
	auto_array< BYTE > temp( new BYTE[ len + n * 9 ] );
	size_t j = 0;	
	for ( size_t i = 0; i < len; i++ )
	{
		if ( IsExtra( aKey[ i ] ) )
		{
			temp[ j++ ] = '/'; temp[ j++ ] = '%'; temp[ j++ ] = 'D';
			temp[ j++ ] = 'C'; temp[ j++ ] = 'N';
			switch ( aKey[ i ] )
			{
			case 0:   temp[ j++ ] = '0'; temp[ j++ ] = '0'; temp[ j++ ] = '0'; break;
			case 5:   temp[ j++ ] = '0'; temp[ j++ ] = '0'; temp[ j++ ] = '5'; break;
			case 36:  temp[ j++ ] = '0'; temp[ j++ ] = '3'; temp[ j++ ] = '6'; break;
			case 96:  temp[ j++ ] = '0'; temp[ j++ ] = '9'; temp[ j++ ] = '6'; break;
			case 124: temp[ j++ ] = '1'; temp[ j++ ] = '2'; temp[ j++ ] = '4'; break;
			case 126: temp[ j++ ] = '1'; temp[ j++ ] = '2'; temp[ j++ ] = '6'; break;
			}
			temp[ j++ ] = '%'; temp[ j++ ] = '/';
		}
		else
			temp[ j++ ] = aKey[ i ];
	}
	return std::string( (const char*)&temp[ 0 ], j );
}

BOOL CDCNeighbour::IsExtra(BYTE b) const
{
	return ( b == 0 || b == 5 || b == 124 || b == 96 || b == 126 || b == 36 );
}

BOOL CDCNeighbour::OnRun()
{
	DWORD tNow = GetTickCount();

	if ( m_nState <= nrsConnecting )
	{
		if ( tNow - m_tConnected > Settings.Connection.TimeoutConnect )
		{
			Close( IDS_CONNECTION_TIMEOUT_CONNECT );
			return FALSE;
		}
	}
	else if ( m_nState < nrsConnected )
	{
		if ( tNow - m_tConnected > Settings.Connection.TimeoutHandshake )
		{
			Close( IDS_HANDSHAKE_TIMEOUT );
			return FALSE;
		}
	}
	/*else
	{
		if ( tNow - m_tLastPacket > Settings.Connection.TimeoutTraffic )
		{
			Close( IDS_CONNECTION_TIMEOUT_TRAFFIC );
			return FALSE;
		}
	}*/

	return TRUE;
}

BOOL CDCNeighbour::OnConnected()
{
	if ( ! CNeighbour::OnConnected() )
		return FALSE;

	theApp.Message( MSG_INFO, IDS_CONNECTION_CONNECTED,
		(LPCTSTR)m_sAddress );

	m_nState = nrsHandshake1; // Waiting for $Lock

	return TRUE;
}

void CDCNeighbour::OnDropped()
{
	if ( m_nState < nrsConnected )
	{
		HostCache.OnFailure( &m_pHost.sin_addr, htons( m_pHost.sin_port ),
			PROTOCOL_DC, false );
	}

	if ( m_nState == nrsConnecting )
	{
		Close( IDS_CONNECTION_REFUSED );
	}
	else
	{
		Close( IDS_CONNECTION_DROPPED );
	}
}
