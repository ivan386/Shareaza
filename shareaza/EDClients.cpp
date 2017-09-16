//
// EDClients.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2013.
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
#include "Transfers.h"
#include "EDClient.h"
#include "EDClients.h"
#include "EDPacket.h"

#include "Network.h"
#include "Security.h"
#include "Datagrams.h"
#include "Downloads.h"
#include "QueryHit.h"
#include "SearchManager.h"

#include "HostCache.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CEDClients EDClients;


//////////////////////////////////////////////////////////////////////
// CEDClients construction

CEDClients::CEDClients() :
	m_pFirst			( NULL )
,	m_pLast				( NULL )
,	m_nCount			( 0 )
,	m_tLastRun			( 0ul )
,	m_tLastMaxClients	( 0ul )
{
}

CEDClients::~CEDClients()
{
	Clear();
}

void CEDClients::Add(CEDClient* pClient)
{
	CQuickLock oLock( m_pSection );

	ASSERT( pClient->m_pEdPrev == NULL );
	ASSERT( pClient->m_pEdNext == NULL );

	pClient->m_pEdPrev = m_pLast;
	pClient->m_pEdNext = NULL;

	if ( m_pLast != NULL )
	{
		m_pLast->m_pEdNext = pClient;
		m_pLast = pClient;
	}
	else
	{
		m_pFirst = m_pLast = pClient;
	}

	++m_nCount;
}

void CEDClients::Remove(CEDClient* pClient)
{
	CQuickLock oLock( m_pSection );

	ASSERT( m_nCount > 0 );

	if ( pClient->m_pEdPrev != NULL )
		pClient->m_pEdPrev->m_pEdNext = pClient->m_pEdNext;
	else
		m_pFirst = pClient->m_pEdNext;

	if ( pClient->m_pEdNext != NULL )
		pClient->m_pEdNext->m_pEdPrev = pClient->m_pEdPrev;
	else
		m_pLast = pClient->m_pEdPrev;

	--m_nCount;
}

void CEDClients::Clear()
{
	CQuickLock oLock( m_pSection );

	for ( CEDClient* pClient = m_pFirst ; pClient ; )
	{
		CEDClient* pNext = pClient->m_pEdNext;
		pClient->Remove();
		pClient = pNext;
	}

	ASSERT( m_pFirst == NULL );
	ASSERT( m_pLast == NULL );
	ASSERT( m_nCount == 0 );
}

int CEDClients::GetCount() const
{
	CQuickLock oLock( m_pSection );

	return m_nCount;
}

//////////////////////////////////////////////////////////////////////
// CEDClients push connection setup
//
// This function is a callback from the Network thread. Destruction of the
// object is called from the Tranfers thread.

bool CEDClients::PushTo(DWORD nClientID, WORD nClientPort)
{
	// Lock this object until we are finished with it
	CQuickLock oCEDClientsLock( m_pSection );

	// Set up connection object
	CEDClient* pClient = Connect( nClientID, nClientPort, NULL, 0, Hashes::Guid() );

	// Set up failed
	if ( pClient == NULL )
		return false;

	// Signal that this client has requested a callback
	pClient->m_bCallbackRequested = true;

	// Log request
	theApp.Message( MSG_DEBUG, _T("[ED2K] Push request received for %s"), (LPCTSTR)CString( inet_ntoa( pClient->m_pHost.sin_addr ) ) );

	// Set up succeeded
	return true;
}

//////////////////////////////////////////////////////////////////////
// CEDClients connection setup

CEDClient* CEDClients::Connect(DWORD nClientID, WORD nClientPort, IN_ADDR* pServerAddress, WORD nServerPort, const Hashes::Guid& oGUID)
{
	CEDClient* pClient = NULL;

	{
		CQuickLock oLock( m_pSection );

		if ( oGUID )
		{
			pClient = GetByGUID( oGUID );
			if ( pClient )
				return pClient;
		}

		if ( IsFull() )
			return NULL;

		if ( CEDPacket::IsLowID( nClientID ) )
		{
			if ( pServerAddress == NULL || nServerPort == 0 )
				return NULL;

			pClient = GetByID( nClientID, pServerAddress, oGUID );
		}
		else
		{
			if ( Security.IsDenied( (IN_ADDR*)&nClientID ) )
				return NULL;

			pClient = GetByID( nClientID, NULL, oGUID );
		}
	}

	if ( pClient == NULL )
	{
		pClient = new CEDClient();
		pClient->ConnectTo( nClientID, nClientPort, pServerAddress, nServerPort, oGUID );
	}

	return pClient;
}

//////////////////////////////////////////////////////////////////////
// CEDClients find by client ID and/or GUID

CEDClient* CEDClients::GetByIP(const IN_ADDR* pAddress) const
{
	CQuickLock oLock( m_pSection );

	for ( CEDClient* pClient = m_pFirst ; pClient ; pClient = pClient->m_pEdNext )
	{
		if ( pClient->m_pHost.sin_addr.S_un.S_addr == pAddress->S_un.S_addr )
			return pClient;
	}

	return NULL;
}

CEDClient* CEDClients::GetByID(DWORD nClientID, IN_ADDR* pServer, const Hashes::Guid& oGUID) const
{
	for ( CEDClient* pClient = m_pFirst ; pClient ; pClient = pClient->m_pEdNext )
	{
		if ( pServer && pClient->m_pServer.sin_addr.S_un.S_addr != pServer->S_un.S_addr )
			continue;

		if ( pClient->m_nClientID == nClientID )
		{
			if ( !oGUID || validAndEqual( pClient->m_oGUID, oGUID ) )
				return pClient;
		}
	}

	return NULL;
}

CEDClient* CEDClients::GetByGUID(const Hashes::Guid& oGUID) const
{
	for ( CEDClient* pClient = m_pFirst ; pClient ; pClient = pClient->m_pEdNext )
	{
		if ( validAndEqual( pClient->m_oGUID, oGUID ) ) return pClient;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CEDClients merge

BOOL CEDClients::Merge(CEDClient* pClient)
{
	CQuickLock oLock( m_pSection );

	ASSERT( pClient != NULL );

	for ( CEDClient* pOther = m_pFirst ; pOther ; pOther = pOther->m_pEdNext )
	{
		if ( pOther != pClient && pOther->Equals( pClient ) )
		{
			pClient->Merge( pOther );
			pOther->Remove();
			return TRUE;
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CEDClients full test

bool CEDClients::IsFull(const CEDClient* pCheckThis)
{
	CQuickLock oLock( m_pSection );

	// Count the number of connected clients
	DWORD nCount = 0;
	for ( CEDClient* pClient = m_pFirst ; pClient ; pClient = pClient->m_pEdNext )
	{
		if ( pClient->IsValid() )
			++nCount;
	}

	// Get the current time
	DWORD tNow = GetTickCount();

	// If there are more clients current connected than there should be, set the full timer
	if ( nCount >= Settings.eDonkey.MaxLinks )
		m_tLastMaxClients = tNow;

	// If we have not been full in the past 2 seconds, then we're okay to start new connections
	if ( tNow - m_tLastMaxClients > 2ul * 1000ul )
		return false;

	// If we're checking a client that's already connected, say we aren't full. (don't drop it)
	if ( pCheckThis && pCheckThis->IsValid() )
		return false;

	// We're too full to start new connections
	return true;
}

BOOL CEDClients::IsOverloaded() const
{
	CQuickLock oLock( m_pSection );

	DWORD nCount = 0;

	for ( CEDClient* pClient = m_pFirst ; pClient ; pClient = pClient->m_pEdNext )
	{
		if ( pClient->IsValid() ) nCount++;
	}

	return ( nCount >= ( Settings.eDonkey.MaxLinks + 25 ) );
}

BOOL CEDClients::IsMyDownload(const CDownloadTransferED2K* pDownload) const
{
	CQuickLock oLock( m_pSection );

	for ( CEDClient* pClient = m_pFirst ; pClient ; pClient = pClient->m_pEdNext )
	{
		if( pClient->m_pDownloadTransfer == pDownload )
			return TRUE;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CEDClients run

void CEDClients::OnRun()
{
	DWORD tNow = GetTickCount();

	// Delay to limit the rate of ed2k packets being sent.
	// keep ed2k transfers under 10 KB/s per source
	if ( tNow - m_tLastRun < Settings.eDonkey.PacketThrottle )
		return;

	CSingleLock oCTranfersLock( &Transfers.m_pSection );
	if ( ! oCTranfersLock.Lock( 250 ) )
		return;

	CSingleLock oCEDClientsLock( &m_pSection );
	if ( ! oCEDClientsLock.Lock( 250 ) )
		return;

	for ( CEDClient* pClient = m_pFirst ; pClient ; )
	{
		CEDClient* pNext = pClient->m_pEdNext;
		pClient->OnRunEx( tNow );
		pClient = pNext;
	}

	m_tLastRun = tNow;
}

//////////////////////////////////////////////////////////////////////
// CEDClients accept new connections

BOOL CEDClients::OnAccept(CConnection* pConnection)
{
	if ( ! Network.IsConnected() || ( Settings.Connection.RequireForTransfers && ! Settings.eDonkey.EnableToday ) )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_DISABLED, (LPCTSTR)pConnection->m_sAddress );
		return FALSE;
	}

	CSingleLock oTransfersLock( &Transfers.m_pSection );
	if ( oTransfersLock.Lock( 250 ) )
	{
		CSingleLock oEDClientsLock( &m_pSection );
		if ( oEDClientsLock.Lock( 250 ) )
		{
			if ( IsFull() )
			{
				// Even if we're full, we still need to accept connections from clients we have queued, etc
				if ( ( GetByIP( &pConnection->m_pHost.sin_addr ) == NULL ) || ( IsOverloaded() ) )
				{
					theApp.Message( MSG_ERROR, _T("Rejecting %s connection from %s, max client connections reached."), protocolNames[ PROTOCOL_ED2K ], (LPCTSTR)pConnection->m_sAddress );
					return FALSE;
				}
				else
				{
					theApp.Message( MSG_DEBUG, _T("Accepting %s connection from %s despite client connection limit."), protocolNames[ PROTOCOL_ED2K ], (LPCTSTR)pConnection->m_sAddress );
				}
			}

			if ( CEDClient* pClient = new CEDClient() )
			{
				pClient->AttachTo( pConnection );
			}
		}
		else
			theApp.Message( MSG_ERROR, _T("Rejecting %s connection from %s, network core overloaded."), protocolNames[ PROTOCOL_ED2K ], (LPCTSTR)pConnection->m_sAddress );
	}
	else
		theApp.Message( MSG_ERROR, _T("Rejecting %s connection from %s, network core overloaded."), protocolNames[ PROTOCOL_ED2K ], (LPCTSTR)pConnection->m_sAddress );


	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CEDClients process UDP Packets

BOOL CEDClients::OnPacket(const SOCKADDR_IN* pHost, CEDPacket* pPacket)
{
	pPacket->SmartDump( pHost, TRUE, FALSE );

	if ( pPacket->m_nEdProtocol == ED2K_PROTOCOL_EDONKEY )
	{
		switch ( pPacket->m_nType )
		{
		case ED2K_S2CG_SERVERSTATUS:
			return OnServerStatus( pHost, pPacket );

		case ED2K_S2CG_SEARCHRESULT:
		case ED2K_S2CG_FOUNDSOURCES:
			return OnServerSearchResult( pHost, pPacket );

#ifdef _DEBUG
		default:
			CString tmp;
			tmp.Format( _T("Unknown packet from %s:%u."), (LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ), htons( pHost->sin_port ) );
			pPacket->Debug( tmp );
	#endif // _DEBUG
		}
	}
	else
	{
		CSingleLock pLock( &Transfers.m_pSection );
		if ( ! pLock.Lock( 250 ) )
		{
			theApp.Message( MSG_ERROR, _T("Rejecting %s connection from %s, network core overloaded."), protocolNames[ PROTOCOL_ED2K ], (LPCTSTR)CString( inet_ntoa( (IN_ADDR&)pHost->sin_addr ) ) );
			return FALSE;
		}

		CQuickLock oLock( m_pSection );

		switch ( pPacket->m_nType )
		{
		case ED2K_C2C_UDP_REASKFILEPING:
			if ( CEDClient* pClient = GetByIP( &pHost->sin_addr ) )
			{
				pClient->m_nUDP = ntohs( pHost->sin_port );

				if ( ! pClient->OnUdpReask( pPacket ) )
				{
					Datagrams.Send( pHost, CEDPacket::New( ED2K_C2C_UDP_FILENOTFOUND, ED2K_PROTOCOL_EMULE ) );
				}
			}
			else
			{
				Datagrams.Send( pHost, CEDPacket::New( ED2K_C2C_UDP_FILENOTFOUND, ED2K_PROTOCOL_EMULE ) );
			}
			break;

		case ED2K_C2C_UDP_REASKACK:
			if ( CEDClient* pClient = GetByIP( &pHost->sin_addr ) )
			{
				pClient->m_nUDP = ntohs( pHost->sin_port );
				pClient->OnUdpReaskAck( pPacket );
			}
			break;

		case ED2K_C2C_UDP_QUEUEFULL:
			if ( CEDClient* pClient = GetByIP( &pHost->sin_addr ) )
			{
				pClient->m_nUDP = ntohs( pHost->sin_port );
				pClient->OnUdpQueueFull( pPacket );
			}
			break;

		case ED2K_C2C_UDP_FILENOTFOUND:
			if ( CEDClient* pClient = GetByIP( &pHost->sin_addr ) )
			{
				pClient->m_nUDP = ntohs( pHost->sin_port );
				pClient->OnUdpFileNotFound( pPacket );
			}
			break;

	#ifdef _DEBUG
		default:
			CString tmp;
			tmp.Format( _T("Unknown packet from %s:%u."), (LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ), htons( pHost->sin_port ) );
			pPacket->Debug( tmp );
	#endif // _DEBUG
		}
	}
	return TRUE;
}

BOOL CEDClients::OnServerStatus(const SOCKADDR_IN* pHost, CEDPacket* pPacket)
{
	DWORD nLen = pPacket->GetRemaining();
	if ( nLen < 4 )
		return FALSE;

	// Read in and check the key value to make sure we requested this update
	DWORD nKey = pPacket->ReadLongLE();

	CQuickLock oLock( HostCache.eDonkey.m_pSection );

	CHostCacheHostPtr pServer = HostCache.eDonkey.Find( &pHost->sin_addr );
	if ( pServer == NULL )
	{
		theApp.Message( MSG_WARNING, _T("eDonkey server %s:%u status received, but server not found in host cache"), (LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ), htons( pHost->sin_port ) );
		return FALSE;
	}
	if ( pServer->m_nKeyValue != nKey )
	{
		theApp.Message( MSG_WARNING, _T("eDonkey server %s:%u status received, but server key does not match"), (LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ), htons( pHost->sin_port ) );
		return FALSE;
	}

	// Assume UDP is stable
	Datagrams.SetStable();

	// Read in the status packet
	DWORD nUsers = 0, nFiles = 0, nMaxUsers = 0, nFileLimit = 1000, nUDPFlags = 0;
	if ( nLen >= 12 )
	{
		// Current users and files indexed
		nUsers = pPacket->ReadLongLE();
		nFiles = pPacket->ReadLongLE();
	}
	if ( nLen >= 16 )
	{
		// Maximum users allowed
		nMaxUsers = pPacket->ReadLongLE();
	}
	if ( nLen >= 24 )
	{
		// Client file limit. (Maximum files you can send to the server)
		nFileLimit = pPacket->ReadLongLE();	// Soft limit. (Files over this are ignored)
		pPacket->ReadLongLE();				// 'Hard' limit. (Obey previous, it saves bandwidth)
	}
	if ( nLen >= 28 )
	{
		// UDP Flags. (This is important, it determines search types, etc)
		nUDPFlags = pPacket->ReadLongLE();
	}
	if ( nLen >= 32 )
	{
		// Low ID users.
		pPacket->ReadLongLE(); // We don't use this
	}
	if ( nLen >= 40 )
	{
		// UDP Obfuscation Port
		pPacket->ReadShortLE(); // We don't use this
		// TCP Obfuscation Port
		pPacket->ReadShortLE(); // We don't use this
		// Server Key
		pPacket->ReadLongLE(); // We don't use this
	}

	// Update the server variables
	pServer->m_tAck			= 0;
	pServer->m_nFailures	= 0;
	pServer->m_tFailure		= 0;
	pServer->m_bCheckedLocally	= TRUE;
	pServer->m_nUserCount	= nUsers;
	pServer->m_nUserLimit	= nMaxUsers;
	pServer->m_nFileLimit	= nFileLimit;
	pServer->m_nUDPFlags	= nUDPFlags;

	if ( pServer->Seen() < pServer->m_tStats )
		HostCache.eDonkey.Update( pServer, 0, pServer->m_tStats );
	if ( nUDPFlags & ED2K_SERVER_UDP_UNICODE )
		pServer->m_nTCPFlags |= ED2K_SERVER_TCP_UNICODE;
	if ( nUDPFlags & ED2K_SERVER_UDP_GETSOURCES2 )
		pServer->m_nTCPFlags |= ED2K_SERVER_TCP_GETSOURCES2;

	HostCache.eDonkey.Update( pServer );

	theApp.Message( MSG_DEBUG, _T("eDonkey server %s:%u UDP flags: %s"), (LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ), htons( pHost->sin_port ), (LPCTSTR)GetED2KServerUDPFlags( nUDPFlags ) );

	return TRUE;
}

BOOL CEDClients::OnServerSearchResult(const SOCKADDR_IN* pHost, CEDPacket* pPacket)
{
	CQuickLock oLock( HostCache.eDonkey.m_pSection );

	CHostCacheHostPtr pServer = HostCache.eDonkey.Find( &pHost->sin_addr );
	if ( pServer == NULL )
	{
		theApp.Message( MSG_WARNING, _T("eDonkey server %s:%u search result received, but server not found in host cache"), (LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ), htons( pHost->sin_port ) );
		return FALSE;
	}

	SOCKADDR_IN pAddress = {};
	pAddress.sin_addr = pServer->m_pAddress;
	pAddress.sin_port = htons( pServer->m_nPort + 4 );

	// Check server details in host cache
	DWORD nServerFlags = ( pServer && pServer->m_nUDPFlags ) ? pServer->m_nUDPFlags : Settings.eDonkey.DefaultServerFlags;

	// Decode packet and create hits
	if ( CQueryHit* pHits = CQueryHit::FromEDPacket( pPacket, &pAddress, nServerFlags ) )
	{
		// Assume UDPis stable
		Datagrams.SetStable();

		// Update the server variables
		pServer->m_tAck			= 0;
		pServer->m_nFailures	= 0;
		pServer->m_tFailure		= 0;
		pServer->m_bCheckedLocally	= TRUE;

		HostCache.eDonkey.Update( pServer );

		if ( pPacket->m_nType == ED2K_S2CG_SEARCHRESULT )
			Network.OnQueryHits( pHits );
		else
		{
			Downloads.OnQueryHits( pHits );
			pHits->Delete();
		}

		return TRUE;
	}

	return FALSE;
}