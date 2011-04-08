//
// DCNeighbour.cpp
//
// Copyright (c) Shareaza Development Team, 2010-2011.
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
#include "DCClient.h"
#include "DCClients.h"
#include "DCNeighbour.h"
#include "DCPacket.h"
#include "HostCache.h"
#include "GProfile.h"
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
	: CNeighbour	( PROTOCOL_DC )
	, m_bExtended	( FALSE )
	, m_bNickValid	( FALSE )
{
	m_nNodeType = ntHub;

	m_oUsers.InitHashTable( GetBestHashTableSize( 3000 ) );
}

CDCNeighbour::~CDCNeighbour()
{
	ChatCore.OnDropped( this );

	RemoveAllUsers();
}

void CDCNeighbour::RemoveAllUsers()
{
	for ( POSITION pos = m_oUsers.GetStartPosition(); pos; )
	{
		CString sNick;
		CDCUser* pUser;
		m_oUsers.GetNextAssoc( pos, sNick, pUser );
		delete pUser;
	}
	m_oUsers.RemoveAll();
}

BOOL CDCNeighbour::ConnectToMe(const CString& sNick)
{
	// $ConnectToMe RemoteNick SenderIp:SenderPort|

	if ( m_nState != nrsConnected )
		// Too early
		return FALSE;

	ASSERT( ! sNick.IsEmpty() );

	CString strRequest;
	strRequest.Format( _T("$ConnectToMe %s %s|"),
		(LPCTSTR)DCClients.CreateNick( sNick ), (LPCTSTR)HostToString( &Network.m_pHost ) );

	if ( CDCPacket* pPacket = CDCPacket::New() )
	{
		pPacket->WriteString( strRequest, FALSE );

		Send( pPacket );
	}

	return TRUE;
}

CDCUser* CDCNeighbour::GetUser(const CString& sNick) const
{
	CDCUser* pUser;
	return m_oUsers.Lookup( sNick, pUser ) ? pUser : NULL;
}

BOOL CDCNeighbour::ConnectTo(const IN_ADDR* pAddress, WORD nPort, BOOL bAutomatic)
{
	CString sHost( inet_ntoa( *pAddress ) );

	if ( CConnection::ConnectTo( pAddress, nPort ) )
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

	for ( ;; )
	{
		CDCPacket* pPacket;
		{
			CLockedBuffer pInputLocked( GetInput() );

			if ( m_pZInput )
			{
				if ( m_pZInput->m_nLength == 0 )
				{
					if ( m_bZInputEOS )
					{
						// Got "End of Stream" so turn decompression off
						m_bZInputEOS = FALSE;
						if ( m_pZSInput )
							m_pZInput->InflateStreamCleanup( m_pZSInput );
						delete m_pZInput;
						m_pZInput = NULL;
					}
					return TRUE;
				}

				pPacket = CDCPacket::ReadBuffer( m_pZInput );
			}
			else
				pPacket = CDCPacket::ReadBuffer( pInputLocked );
		}

		if ( ! pPacket )
			return TRUE;

		pPacket->SmartDump( &m_pHost, FALSE, FALSE, (DWORD_PTR)this );

		m_nInputCount++;
		m_tLastPacket = GetTickCount();

		BOOL bResult = OnPacket( pPacket );

		pPacket->Release();

		if ( ! bResult )
			return FALSE;
	}
}

BOOL CDCNeighbour::Send(CPacket* pPacket, BOOL bRelease, BOOL /*bBuffered*/)
{
	m_nOutputCount++;

	if ( m_pZOutput )
		pPacket->ToBuffer( m_pZOutput );
	else
		Write( pPacket );

	QueueRun();

	pPacket->SmartDump( &m_pHost, FALSE, TRUE, (DWORD_PTR)this );

	if ( bRelease ) pPacket->Release();

	return TRUE;
}

BOOL CDCNeighbour::OnConnected()
{
	if ( ! CNeighbour::OnConnected() )
		return FALSE;

	theApp.Message( MSG_INFO, IDS_CONNECTION_CONNECTED,
		(LPCTSTR)HostToString( &m_pHost ) );

	m_nState = nrsHandshake1; // Waiting for $Lock

	return TRUE;
}

void CDCNeighbour::OnDropped()
{
	RemoveAllUsers();

	if ( m_nState == nrsConnecting )
	{
		Close( IDS_CONNECTION_REFUSED );
	}
	else
	{
		Close( IDS_CONNECTION_DROPPED );
	}
}

BOOL CDCNeighbour::OnPacket(CDCPacket* pPacket)
{
	if ( pPacket->m_nLength < 2  )
	{
		// Ping, i.e. received only one char "|"
		return TRUE;
	}
	else if ( *pPacket->m_pBuffer == '<' )
	{
		// Chat message
		// <Nick> Message|

		if ( LPCSTR szMessage = strchr( (LPCSTR)pPacket->m_pBuffer, '>' ) )
		{
			int nNickLen = szMessage - (LPCSTR)pPacket->m_pBuffer - 1;
			CString sNick( UTF8Decode( (LPCSTR)&pPacket->m_pBuffer[ 1 ], nNickLen ) );

			if ( nNickLen > 0 && m_sNick != sNick )
			{
				ChatCore.OnMessage( this, pPacket );
			}
		}
		return TRUE;
	}
	else if ( *pPacket->m_pBuffer != '$' )
	{
		// Unknown message

		theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING, _T("%s >> UNKNOWN MESSAGE: %s"),
			(LPCTSTR)HostToString( &m_pHost ),
			(LPCTSTR)CString( (LPCSTR)pPacket->m_pBuffer, (int)pPacket->m_nLength ) );

		return TRUE;
	}

	LPCSTR szCommand = (LPCSTR)pPacket->m_pBuffer;
	
	if ( strncmp( szCommand, _P("$Search ") ) == 0 )
	{
		// Search request
		// $Search SenderIP:SenderPort (F|T)?(F|T)?Size?Type?String|

		return OnQuery( pPacket );
	}

	// Convert '|' to '\0' (make ASCIIZ)
	pPacket->m_pBuffer[ pPacket->m_nLength - 1 ] = 0;

	// Split off parameters
	LPSTR szParams = strchr( (LPSTR)pPacket->m_pBuffer, ' ' );
	if ( szParams )
	{
		*szParams++ = 0;
	}

	if ( strcmp( szCommand, "$MyINFO" ) == 0 )
	{
		// User info
		// $MyINFO $ALL nick description<tag>$ $connection$e-mail$sharesize$|

		if ( strncmp( szParams, _P("$ALL ") ) == 0 )
		{
			LPSTR szNick = szParams + 5;
			if ( LPSTR szDescription = strchr( szNick, ' ' ) )
			{
				*szDescription++ = 0;
				if ( LPSTR szConnection = strchr( szDescription, '$' ) )
				{
					*szConnection++ = 0;
					CString sNick( UTF8Decode( szNick ) );

					CDCUser* pUser;
					if ( ! m_oUsers.Lookup( sNick, pUser ) )
					{
						pUser = new CDCUser;
						m_oUsers.SetAt( sNick, pUser );
					}
					pUser->m_sNick = sNick;
					pUser->m_sDescription = UTF8Decode( szDescription );
				}
			}
		}

		return TRUE;
	}
	else if ( strcmp( szCommand, "$Quit" ) == 0 )
	{
		// User leave hub
		// $Quit nick|

		CString sNick = UTF8Decode( szParams );
		CDCUser* pUser;
		if ( m_oUsers.Lookup( sNick, pUser ) )
		{
			m_oUsers.RemoveKey( sNick );
			delete pUser;
		}

		return TRUE;
	}
	else if ( strcmp( szCommand, "$To:" ) == 0 )
	{
		// Private chat message
		// $To: my_nick From: nick$<nick> message|

		if ( LPCSTR szChat = strchr( szParams, '$' ) )
		{
			if ( szChat[ 1 ] == '<' )
			{
				if ( LPCSTR szMessage = strchr( szChat, '>' ) )
				{
					// Cut off chat packet and restore ending "|"
					pPacket->Remove( szChat - (LPCSTR)pPacket->m_pBuffer + 1 );
					pPacket->m_pBuffer[ pPacket->m_nLength - 1 ] = '|';

					ChatCore.OnMessage( this, pPacket );
				}
			}
		}
		return TRUE;
	}
	else if ( strcmp( szCommand, "$Lock" ) == 0 )
	{
		// $Lock [EXTENDEDPROTOCOL]Challenge Pk=Vendor|

		if  ( LPSTR szLock = szParams )
		{
			m_bExtended = ( strncmp( szParams, _P("EXTENDEDPROTOCOL") ) == 0 );

			if ( LPSTR szUserAgent = strstr( szParams, " Pk=" ) )
			{
				// Good way
				*szUserAgent = 0;
				szUserAgent += 4;
				m_sUserAgent = UTF8Decode( szUserAgent );
			}
			else
			{
				// Bad way
				if ( LPSTR szUserAgent = strchr( szParams, ' ' ) )
				{
					*szUserAgent++ = 0;
					m_sUserAgent = UTF8Decode( szUserAgent );
				}
			}
			return OnLock( szLock );
		}
	}
	else if ( strcmp( szCommand, "$Supports" ) == 0 )
	{
		// $Supports [option1]...[optionN]|

		m_bExtended = TRUE;

		m_oFeatures.RemoveAll();
		for ( CString strFeatures( szParams ); ! strFeatures.IsEmpty(); )
		{
			CString strFeature = strFeatures.SpanExcluding( _T(" ") );
			strFeatures = strFeatures.Mid( strFeature.GetLength() + 1 );
			if ( strFeature.IsEmpty() )
				continue;
			if ( m_oFeatures.Find( strFeature ) == NULL )
			{
				m_oFeatures.AddTail( strFeature );
			}				
		}

		return TRUE;
	}
	else if ( strcmp( szCommand, "$Hello" ) == 0 )
	{
		// User logged-in
		// $Hello Nick|

		m_nState = nrsConnected;

		m_bNickValid = TRUE;
		m_sNick = UTF8Decode( szParams );

		if ( CHostCacheHostPtr pServer = HostCache.DC.Find( &m_pHost.sin_addr ) )
		{
			pServer->m_sUser = m_sNick;
		}

		return OnHello();
	}
	else if ( strcmp( szCommand, "$HubName" ) == 0 )
	{
		// Name of hub
		// $HubName Title [Description]|

		if ( LPSTR szHubName = szParams )
		{
			if ( LPSTR szHubInfo = strchr( szHubName, ' ' ) )
			{
				*szHubInfo++ = 0;
			}

			m_sServerName = UTF8Decode( szHubName );

			if ( CHostCacheHostPtr pServer = HostCache.DC.Find( &m_pHost.sin_addr ) )
			{
				pServer->m_sName = m_sServerName;
			}

			ChatCore.OnMessage( this );

			return TRUE;
		}
	}
	else if ( strcmp( szCommand, "$OpList" ) == 0 )
	{
		// Hub operators list
		// $OpList operator1|

		return TRUE;
	}
	else if ( strcmp( szCommand, "$ConnectToMe" ) == 0 )
	{
		// Client connection request
		// $ConnectToMe MyNick SenderIp:SenderPort|
		// or
		// $ConnectToMe SenderNick MyNick SenderIp:SenderPort|

		if ( LPSTR szSenderNick = szParams )
		{
			if ( LPSTR szMyNick = strchr( szSenderNick, ' ' ) )
			{
				*szMyNick++ = 0;
				LPSTR szAddress = strchr( szMyNick, ' ' );
				if ( szAddress )
				{
					*szAddress++ = 0;
				}
				else
				{
					szAddress = szMyNick;
					szMyNick = szSenderNick;
					szSenderNick = "";
				}

				CString sMyNick( UTF8Decode( szMyNick) );
				CString sSenderNick( UTF8Decode( szSenderNick ) );

				if ( LPSTR szPort = strchr( szAddress, ':' ) )
				{
					*szPort++ = 0;
					int nPort = atoi( szPort );
					IN_ADDR nAddress;
					nAddress.s_addr = inet_addr( szAddress );
					if ( m_sNick == sMyNick )
					{
						// Ok
						DCClients.ConnectTo( &nAddress, (WORD)nPort, m_sNick, sSenderNick );
					}
					else
					{
						// Wrong nick, bad IP
					}
				}
			}
			return TRUE;
		}
	}
	else if ( strcmp( szCommand, "$ForceMove " ) == 0 )
	{
		// User redirection
		// $ForceMove IP:Port|

		if ( LPSTR szAddress = szParams )
		{
			int nPort = DC_DEFAULT_PORT;
			if ( LPSTR szPort = strchr( szAddress, ':' ) )
			{
				*szPort++ = 0;
				nPort = atoi( szPort );
			}

			Network.ConnectTo( UTF8Decode( szAddress ), nPort, PROTOCOL_DC );

			return TRUE;
		}
	}
	else if ( strcmp( szCommand, "$ValidateDenide" ) == 0 ||
		// TODO: Add registered user support - for now just change nick
		strcmp( szCommand, "$GetPass" ) == 0 )
	{
		// Bad user nick
		// $ValidateDenide Nick|

		m_bNickValid = FALSE;
		m_sNick.Format( CLIENT_NAME_T _T("%04u"), GetRandomNum( 0, 9999 ) );

		if ( CHostCacheHostPtr pServer = HostCache.DC.Find( &m_pHost.sin_addr ) )
		{
			pServer->m_sUser = m_sNick;
		}

		if ( CDCPacket* pPacket = CDCPacket::New() )
		{
			pPacket->Write( _P("$ValidateNick ") );
			pPacket->WriteString( m_sNick, FALSE );
			pPacket->Write( _P("|") );
			Send( pPacket );
		}

		return TRUE;
	}
	else if ( strcmp( szCommand, "$UserIP" ) == 0 )
	{
		// User address
		// $UserIP MyNick IP|

		if ( LPSTR szNick = szParams )
		{
			if ( LPSTR szAddress = strchr( szParams, ' ' ) )
			{
				*szAddress++ = 0;

				CString sNick( UTF8Decode( szNick ) );

				if ( m_sNick == sNick && m_bNickValid )
				{
					IN_ADDR nAddress;
					nAddress.s_addr = inet_addr( szAddress );
					Network.AcquireLocalAddress( nAddress );
				}
				return TRUE;
			}
		}
	}
	else if ( strcmp( szCommand, "$ZOn" ) == 0 )
	{
		// ZLib stream compression enabled
		// $ZOn|

		ASSERT( m_pZInput == NULL );
		m_pZInput  = new CBuffer();
		return TRUE;
	}

	// Unknown command - ignoring

	theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING, _T("%s >> UNKNOWN COMMAND: %s"),
		(LPCTSTR)HostToString( &m_pHost ),
		(LPCTSTR)CString( (LPCSTR)pPacket->m_pBuffer, (int)pPacket->m_nLength ) );

	return TRUE;
}

BOOL CDCNeighbour::OnQuery(CDCPacket* pPacket)
{
	CQuerySearchPtr pSearch = CQuerySearch::FromPacket( pPacket, NULL, TRUE );
	if ( ! pSearch  || pSearch->m_bDropMe )
	{
		if ( ! pSearch )
		{
			DEBUG_ONLY( pPacket->Debug( _T("Malformed Query.") ) );
			theApp.Message( MSG_WARNING, IDS_PROTOCOL_BAD_QUERY, (LPCTSTR)m_sAddress );
		}
		m_nDropCount++;
		return TRUE;
	}

	pSearch->m_pMyHub = m_pHost;
	pSearch->m_sMyHub = m_sServerName;
	pSearch->m_sMyNick = m_sNick;

	if ( pSearch->m_bUDP )
	{
		if ( Security.IsDenied( &pSearch->m_pEndpoint.sin_addr ) )
		{
			m_nDropCount++;
			return TRUE;
		}

		Network.OnQuerySearch( new CLocalSearch( pSearch, PROTOCOL_DC ) );
	}
	else
	{
		Network.OnQuerySearch( new CLocalSearch( pSearch, this ) );
	}

	return TRUE;
}

BOOL CDCNeighbour::OnLock(LPSTR szLock)
{
	if ( m_nState < nrsHandshake2 )
	{
		m_nState = nrsHandshake2;	// Waiting for $Hello
	}

	if ( m_nNodeType == ntHub )
	{
		HostCache.DC.Add( &m_pHost.sin_addr, htons( m_pHost.sin_port ) );
	}

	if ( m_bExtended )
	{
		if ( CDCPacket* pPacket = CDCPacket::New() )
		{
			pPacket->Write( _P(DC_HUB_SUPPORTS) );
			Send( pPacket );
		}
	}

	std::string strKey = DCClients.MakeKey( szLock );
	if ( CDCPacket* pPacket = CDCPacket::New() )
	{
		pPacket->Write( _P("$Key ") );
		pPacket->Write( strKey.c_str(), (DWORD)strKey.size() );
		pPacket->Write( _P("|") );
		Send( pPacket );
	}

	m_bNickValid = FALSE;
	/*
	if ( CHostCacheHostPtr pServer = HostCache.DC.Find( &m_pHost.sin_addr ) )
	{
		m_sNick = pServer->m_sUser;
	}
	*/
	m_sNick = DCClients.CreateNick( m_sNick );

	if ( CDCPacket* pPacket = CDCPacket::New() )
	{
		pPacket->Write( _P("$ValidateNick ") );
		pPacket->WriteString( m_sNick, FALSE );
		pPacket->Write( _P("|") );
		Send( pPacket );
	}

	return TRUE;
}

BOOL CDCNeighbour::OnHello()
{
	// NMDC version
	if ( CDCPacket* pPacket = CDCPacket::New() )
	{
		pPacket->Write( _P("$Version 1,0091|") );
		Send( pPacket );
	}

	// $MyINFO $ALL nick description<tag>$ $connection$e-mail$sharesize$|

	CUploadQueue* pQueue = UploadQueues.SelectQueue(
		PROTOCOL_DC, NULL, 0, CUploadQueue::ulqBoth, NULL );

	QWORD nMyVolume = 0;
	LibraryMaps.GetStatistics( NULL, &nMyVolume );

	CString sInfo;
	sInfo.Format( _T("$MyINFO $ALL %s %s<%s V:%s,M:%c,H:%u/%u/%u,S:%u>$ $%.2f%c$%s$%I64u$|"),
		// Registered nick
		(LPCTSTR)m_sNick,
		// Description
		WEB_SITE_T,
		// Client name
		CLIENT_NAME_T,
		// Client version
		(LPCTSTR)theApp.m_sVersion,
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
		(LPCTSTR)MyProfile.GetContact( _T("Email") ),
		// Share size (bytes)
		nMyVolume << 10 );
	if ( CDCPacket* pPacket = CDCPacket::New() )
	{
		pPacket->WriteString( sInfo, FALSE );
		Send( pPacket );
	}

	/*/ Switch to compression stream
	if ( ! m_pZOutput && m_oFeatures.Find( _T("ZPipe0") ) )
	{
		if ( CDCPacket* pPacket = CDCPacket::New() )
		{
			pPacket->Write( _P("$ZOn|") );
			Send( pPacket );

			m_pZOutput  = new CBuffer();
		}
	}*/

	// Request nick list
	if ( CDCPacket* pPacket = CDCPacket::New() )
	{
		pPacket->Write( _P("$GetNickList|") );
		Send( pPacket );
	}

	return TRUE;
}
