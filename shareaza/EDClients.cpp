//
// EDClients.cpp
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

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CEDClients EDClients;


//////////////////////////////////////////////////////////////////////
// CEDClients construction

CEDClients::CEDClients()
{
	m_pFirst	= NULL;
	m_pLast		= NULL;
	m_nCount	= 0;
	m_tLastRun	= 0;
}

CEDClients::~CEDClients()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CEDClients add and remove

void CEDClients::Add(CEDClient* pClient)
{
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
	
	m_nCount++;
}

void CEDClients::Remove(CEDClient* pClient)
{
	ASSERT( m_nCount > 0 );
	
	if ( pClient->m_pEdPrev != NULL )
		pClient->m_pEdPrev->m_pEdNext = pClient->m_pEdNext;
	else
		m_pFirst = pClient->m_pEdNext;
	
	if ( pClient->m_pEdNext != NULL )
		pClient->m_pEdNext->m_pEdPrev = pClient->m_pEdPrev;
	else
		m_pLast = pClient->m_pEdPrev;
	
	m_nCount --;
}

//////////////////////////////////////////////////////////////////////
// CEDClients clear

void CEDClients::Clear()
{
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

//////////////////////////////////////////////////////////////////////
// CEDClients push connection setup

BOOL CEDClients::PushTo(DWORD nClientID, WORD nClientPort)
{
	CEDClient* pClient = Connect( nClientID, nClientPort, NULL, 0, NULL );
	if ( pClient == NULL ) return FALSE;
	return pClient->Connect();
}

//////////////////////////////////////////////////////////////////////
// CEDClients connection setup

CEDClient* CEDClients::Connect(DWORD nClientID, WORD nClientPort, IN_ADDR* pServerAddress, WORD nServerPort, GGUID* pGUID)
{
	if ( pGUID != NULL )
	{
		if ( CEDClient* pClient = GetByGUID( pGUID ) ) return pClient;
	}
	
	if ( IsFull() ) return NULL;
	
	CEDClient* pClient = NULL;
	
	if ( CEDPacket::IsLowID( nClientID ) )
	{
		if ( pServerAddress == NULL || nServerPort == 0 ) return NULL;
		pClient = GetByID( nClientID, pServerAddress, pGUID );
	}
	else
	{
		if ( Security.IsDenied( (IN_ADDR*)&nClientID ) ) return NULL;
		pClient = GetByID( nClientID, NULL, pGUID );
	}
	
	if ( pClient == NULL )
	{
		pClient = new CEDClient();
		pClient->ConnectTo( nClientID, nClientPort, pServerAddress, nServerPort, pGUID );
	}
	
	return pClient;
}

//////////////////////////////////////////////////////////////////////
// CEDClients find by client ID and/or GUID

CEDClient* CEDClients::GetByIP(IN_ADDR* pAddress)
{
	for ( CEDClient* pClient = m_pFirst ; pClient ; pClient = pClient->m_pEdNext )
	{
		if ( pClient->m_pHost.sin_addr.S_un.S_addr == pAddress->S_un.S_addr )
			return pClient;
	}
	
	return NULL;
}

CEDClient* CEDClients::GetByID(DWORD nClientID, IN_ADDR* pServer, GGUID* pGUID)
{
	for ( CEDClient* pClient = m_pFirst ; pClient ; pClient = pClient->m_pEdNext )
	{
		if ( pServer && pClient->m_pServer.sin_addr.S_un.S_addr != pServer->S_un.S_addr ) continue;
		
		if ( pClient->m_nClientID == nClientID )
		{
			if ( pGUID == NULL || pClient->m_pGUID == *pGUID ) return pClient;
		}
	}
	
	return NULL;
}

CEDClient* CEDClients::GetByGUID(GGUID* pGUID)
{
	for ( CEDClient* pClient = m_pFirst ; pClient ; pClient = pClient->m_pEdNext )
	{
		if ( pClient->m_bGUID && pClient->m_pGUID == *pGUID ) return pClient;
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CEDClients merge

BOOL CEDClients::Merge(CEDClient* pClient)
{
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

BOOL CEDClients::IsFull(CEDClient* pCheckThis)
{
	int nCount = 0;
	
	if ( pCheckThis != NULL )
	{
		for ( CEDClient* pClient = m_pFirst ; pClient ; pClient = pClient->m_pEdNext )
		{
			if ( pClient->m_hSocket != INVALID_SOCKET ) nCount++;
			if ( pClient == pCheckThis ) pCheckThis = NULL;
		}
	}
	else
	{
		for ( CEDClient* pClient = m_pFirst ; pClient ; pClient = pClient->m_pEdNext )
		{
			if ( pClient->m_hSocket != INVALID_SOCKET ) nCount++;
		}
	}
	
	ASSERT( pCheckThis == NULL );
	return ( nCount >= Settings.eDonkey.MaxLinks ) || ( pCheckThis != NULL );
}

//////////////////////////////////////////////////////////////////////
// CEDClients run

void CEDClients::OnRun()
{
	// Delay to keep ed2k transfers under 10 KB/s per source
	DWORD tNow = GetTickCount();
	if ( tNow - m_tLastRun < Settings.eDonkey.PacketThrottle ) return;
	m_tLastRun = tNow;
	
	for ( CEDClient* pClient = m_pFirst ; pClient ; )
	{
		CEDClient* pNext = pClient->m_pEdNext;
		pClient->OnRunEx( tNow );
		pClient = pNext;
	}
}

//////////////////////////////////////////////////////////////////////
// CEDClients accept new connections

BOOL CEDClients::OnAccept(CConnection* pConnection)
{
	ASSERT( pConnection != NULL );
	
	if ( Settings.Connection.RequireForTransfers && ! Settings.eDonkey.EnableToday )
	{
		theApp.Message( MSG_ERROR, IDS_ED2K_CLIENT_DISABLED,
			(LPCTSTR)pConnection->m_sAddress );
		return FALSE;
	}
	
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return FALSE;
	
	CEDClient* pClient = new CEDClient();
	pClient->AttachTo( pConnection );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CEDClients process UDP Packets

BOOL CEDClients::OnUDP(SOCKADDR_IN* pHost, CEDPacket* pPacket)
{
	CSingleLock pLock( &Transfers.m_pSection );
	
	switch ( pPacket->m_nType )
	{
	case ED2K_C2C_UDP_REASKFILEPING:
		if ( ! pLock.Lock( 100 ) ) return FALSE;
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
		if ( ! pLock.Lock( 100 ) ) return FALSE;
		if ( CEDClient* pClient = GetByIP( &pHost->sin_addr ) )
		{
			pClient->m_nUDP = ntohs( pHost->sin_port );
			pClient->OnUdpReaskAck( pPacket );
		}
		break;
	case ED2K_C2C_UDP_QUEUEFULL:
		if ( ! pLock.Lock( 100 ) ) return FALSE;
		if ( CEDClient* pClient = GetByIP( &pHost->sin_addr ) )
		{
			pClient->m_nUDP = ntohs( pHost->sin_port );
			pClient->OnUdpQueueFull( pPacket );
		}
		break;
	case ED2K_C2C_UDP_FILENOTFOUND:
		if ( ! pLock.Lock( 100 ) ) return FALSE;
		if ( CEDClient* pClient = GetByIP( &pHost->sin_addr ) )
		{
			pClient->m_nUDP = ntohs( pHost->sin_port );
			pClient->OnUdpFileNotFound( pPacket );
		}
		break;
	case ED2K_S2CG_SEARCHRESULT:
	case ED2K_S2CG_FOUNDSOURCES:
		pHost->sin_port = htons( ntohs( pHost->sin_port ) - 4 );
		if ( CQueryHit* pHits = CQueryHit::FromPacket( pPacket, pHost, Settings.eDonkey.DefaultServerFlags ) )
		{
			Downloads.OnQueryHits( pHits );
			
			if ( pPacket->m_nType == ED2K_S2CG_SEARCHRESULT )
				Network.OnQueryHits( pHits );
			else
				pHits->Delete();
		}
		break;
	}
	
	return TRUE;
}
