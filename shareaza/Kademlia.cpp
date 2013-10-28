//
// Kademlia.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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
#include "Statistics.h"
#include "Network.h"
#include "Datagrams.h"
#include "Transfers.h"
#include "Buffer.h"
#include "GProfile.h"
#include "EDPacket.h"
#include "Kademlia.h"
#include "HostCache.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CKademlia Kademlia;


BOOL CKademlia::Send(const SOCKADDR_IN* pHost, CEDPacket* pPacket)
{
	// TODO: Kademlia packets statistics

	return Datagrams.Send( pHost, pPacket );
}

BOOL CKademlia::Send(const SOCKADDR_IN* pHost, BYTE nType)
{
	return Send( pHost, CEDPacket::New( nType, ED2K_PROTOCOL_KAD ) );
}

BOOL CKademlia::Bootstrap(const SOCKADDR_IN* pHost, bool bKad2)
{
	if ( bKad2 )
	{
		return Send( pHost, KADEMLIA2_BOOTSTRAP_REQ );
	}
	else
	{
		return SendMyDetails( pHost, KADEMLIA_BOOTSTRAP_REQ, false );
	}
}

BOOL CKademlia::SendMyDetails(const SOCKADDR_IN* pHost, BYTE nType, bool bKad2)
{
	CEDPacket* pPacket = CEDPacket::New( nType, ED2K_PROTOCOL_KAD );
	if ( ! pPacket )
		return FALSE;

	Hashes::Guid oGUID = MyProfile.oGUID;

	if ( bKad2 )
	{
		pPacket->Write( oGUID );
		pPacket->WriteShortLE( htons( Network.m_pHost.sin_port ) );	// TCP
		pPacket->WriteByte( KADEMLIA_VERSION );
		pPacket->WriteByte( 0 );
		return Send( pHost, pPacket );
	}
	else
	{
		pPacket->Write( oGUID );
		pPacket->WriteLongLE( Network.m_pHost.sin_addr.S_un.S_addr );
		pPacket->WriteShortLE( htons( Network.m_pHost.sin_port ) );	// UDP
		pPacket->WriteShortLE( htons( Network.m_pHost.sin_port ) );	// TCP
		pPacket->WriteByte( 0 );
		return Send( pHost, pPacket );
	}
}

BOOL CKademlia::OnPacket(const SOCKADDR_IN* pHost, CEDPacket* pPacket)
{
	pPacket->SmartDump( pHost, TRUE, FALSE );

	// TODO: Kademlia packets statistics

	CQuickLock oLock( m_pSection );

	switch ( pPacket->m_nType )
	{
	case KADEMLIA_BOOTSTRAP_REQ:
//		return OnPacket_KADEMLIA_BOOTSTRAP_REQ( pHost, pPacket );
	case KADEMLIA2_BOOTSTRAP_REQ:
//		return OnPacket_KADEMLIA2_BOOTSTRAP_REQ( pHost, pPacket );
		break;
	case KADEMLIA_BOOTSTRAP_RES:
		return OnPacket_KADEMLIA_BOOTSTRAP_RES( pHost, pPacket );
	case KADEMLIA2_BOOTSTRAP_RES:
		return OnPacket_KADEMLIA2_BOOTSTRAP_RES( pHost, pPacket );
	case KADEMLIA_HELLO_REQ:
//		return OnPacket_KADEMLIA_HELLO_REQ( pHost, pPacket );
	case KADEMLIA2_HELLO_REQ:
//		return OnPacket_KADEMLIA2_HELLO_REQ( pHost, pPacket );
	case KADEMLIA_HELLO_RES:
//		return OnPacket_KADEMLIA_HELLO_RES( pHost, pPacket );
	case KADEMLIA2_HELLO_RES:
//		return OnPacket_KADEMLIA2_HELLO_RES( pHost, pPacket );
	case KADEMLIA_REQ:
//		return OnPacket_KADEMLIA_REQ( pHost, pPacket );
	case KADEMLIA2_REQ:
//		return OnPacket_KADEMLIA2_REQ( pHost, pPacket );
	case KADEMLIA_RES:
//		return OnPacket_KADEMLIA_RES( pHost, pPacket );
	case KADEMLIA2_RES:
//		return OnPacket_KADEMLIA2_RES( pHost, pPacket );
	case KADEMLIA_SEARCH_REQ:
//		return OnPacket_KADEMLIA_SEARCH_REQ( pHost, pPacket );
	case KADEMLIA_SEARCH_NOTES_REQ:
//		return OnPacket_KADEMLIA_SEARCH_NOTES_REQ( pHost, pPacket );
	case KADEMLIA2_SEARCH_NOTES_REQ:
//		return OnPacket_KADEMLIA2_SEARCH_NOTES_REQ( pHost, pPacket );
	case KADEMLIA2_SEARCH_KEY_REQ:
//		return OnPacket_KADEMLIA2_SEARCH_KEY_REQ( pHost, pPacket );
	case KADEMLIA2_SEARCH_SOURCE_REQ:
//		return OnPacket_KADEMLIA2_SEARCH_SOURCE_REQ( pHost, pPacket );
	case KADEMLIA_SEARCH_RES:
//		return OnPacket_KADEMLIA_SEARCH_RES( pHost, pPacket );
	case KADEMLIA_SEARCH_NOTES_RES:
//		return OnPacket_KADEMLIA_SEARCH_NOTES_RES( pHost, pPacket );
	case KADEMLIA2_SEARCH_RES:
//		return OnPacket_KADEMLIA2_SEARCH_RES( pHost, pPacket );
	case KADEMLIA_PUBLISH_REQ:
//		return OnPacket_KADEMLIA_PUBLISH_REQ( pHost, pPacket );
	case KADEMLIA_PUBLISH_NOTES_REQ:
//		return OnPacket_KADEMLIA_PUBLISH_NOTES_REQ( pHost, pPacket );
	case KADEMLIA2_PUBLISH_KEY_REQ:
//		return OnPacket_KADEMLIA2_PUBLISH_KEY_REQ( pHost, pPacket );
	case KADEMLIA2_PUBLISH_SOURCE_REQ:
//		return OnPacket_KADEMLIA2_PUBLISH_SOURCE_REQ( pHost, pPacket );
	case KADEMLIA2_PUBLISH_NOTES_REQ:
//		return OnPacket_KADEMLIA2_PUBLISH_NOTES_REQ( pHost, pPacket );
	case KADEMLIA_PUBLISH_RES:
//		return OnPacket_KADEMLIA_PUBLISH_RES( pHost, pPacket );
	case KADEMLIA_PUBLISH_NOTES_RES:
//		return OnPacket_KADEMLIA_PUBLISH_NOTES_RES( pHost, pPacket );
	case KADEMLIA2_PUBLISH_RES:
//		return OnPacket_KADEMLIA2_PUBLISH_RES( pHost, pPacket );
	case KADEMLIA_FIREWALLED_REQ:
//		return OnPacket_KADEMLIA_FIREWALLED_REQ( pHost, pPacket );
	case KADEMLIA_FIREWALLED_RES:
//		return OnPacket_KADEMLIA_FIREWALLED_RES( pHost, pPacket );
	case KADEMLIA_FIREWALLED_ACK_RES:
//		return OnPacket_KADEMLIA_FIREWALLED_ACK_RES( pHost, pPacket );
	case KADEMLIA_FINDBUDDY_REQ:
//		return OnPacket_KADEMLIA_FINDBUDDY_REQ( pHost, pPacket );
	case KADEMLIA_FINDBUDDY_RES:
//		return OnPacket_KADEMLIA_FINDBUDDY_RES( pHost, pPacket );
	case KADEMLIA_CALLBACK_REQ:
//		return OnPacket_KADEMLIA_CALLBACK_REQ( pHost, pPacket );
		break;
	case KADEMLIA2_PING:
		return OnPacket_KADEMLIA2_PING( pHost, pPacket );
	case KADEMLIA2_PONG:
		return OnPacket_KADEMLIA2_PONG( pHost, pPacket );

#ifdef _DEBUG
	default:
		CString tmp;
		tmp.Format( _T("Unknown packet from %s:%u."),
			(LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ),
			htons( pHost->sin_port ) );
		pPacket->Debug( tmp );
#endif // _DEBUG
	}
	return FALSE;
}

BOOL CKademlia::OnPacket_KADEMLIA_BOOTSTRAP_RES(const SOCKADDR_IN* /*pHost*/, CEDPacket* pPacket)
{
	Hashes::Guid oGUID;
	IN_ADDR pAddress;
	WORD nUDPPort, nTCPPort;
	WORD nCount;

	if ( pPacket->GetRemaining() < 2 )
		return FALSE;

	nCount = pPacket->ReadShortLE();

	if ( pPacket->GetRemaining() < nCount * ( 16u + 4 + 2 + 2 + 1 ) )
		return FALSE;

	while( nCount-- )
	{
		pPacket->Read( oGUID );
		*(DWORD*)&pAddress = ntohl( pPacket->ReadLongLE() );
		nUDPPort = pPacket->ReadShortLE();
		nTCPPort = pPacket->ReadShortLE();
		pPacket->ReadByte();	// skip

		CHostCacheHostPtr pCache = HostCache.Kademlia.Add( &pAddress, nTCPPort );
		if ( pCache )
		{
			pCache->m_oGUID = oGUID;
			pCache->m_nUDPPort = nUDPPort;
			pCache->m_sDescription = oGUID.toString();
		}
	}

	HostCache.Kademlia.m_nCookie++;

	return TRUE;
}

BOOL CKademlia::OnPacket_KADEMLIA2_BOOTSTRAP_RES(const SOCKADDR_IN* pHost, CEDPacket* pPacket)
{
	Hashes::Guid oGUID;
	IN_ADDR pAddress;
	WORD nUDPPort, nTCPPort;
	BYTE nVersion;
	WORD nCount;

	if ( pPacket->GetRemaining() < ( 16 + 2 + 1 + 2 ) )
		return FALSE;

	pPacket->Read( oGUID );
	nTCPPort = pPacket->ReadShortLE();
	nVersion = pPacket->ReadByte();
	nCount = pPacket->ReadShortLE();

	if ( pPacket->GetRemaining() < nCount * ( 16u + 4 + 2 + 2 + 1 ) )
		return FALSE;

	// TODO: Packet track check

	CQuickLock oLock( HostCache.Kademlia.m_pSection );

	CHostCacheHostPtr pCache = HostCache.Kademlia.Add( &pHost->sin_addr, nTCPPort );
	if ( ! pCache )
		return FALSE;

	pCache->m_oGUID = oGUID;
	pCache->m_nUDPPort = htons( pHost->sin_port );
	pCache->m_nKADVersion = nVersion;
	pCache->m_sDescription = oGUID.toString();
	pCache->m_tFailure = 0;
	pCache->m_nFailures = 0;
	pCache->m_bCheckedLocally = TRUE;

	while( nCount-- )
	{
		pPacket->Read( oGUID );
		*(DWORD*)&pAddress = ntohl( pPacket->ReadLongLE() );
		nUDPPort = pPacket->ReadShortLE();
		nTCPPort = pPacket->ReadShortLE();
		nVersion = pPacket->ReadByte();

		pCache = HostCache.Kademlia.Add( &pAddress, nTCPPort );
		if ( pCache )
		{
			pCache->m_oGUID = oGUID;
			pCache->m_nUDPPort = nUDPPort;
			pCache->m_nKADVersion = nVersion;
			pCache->m_sDescription = oGUID.toString();
		}
	}

	HostCache.Kademlia.m_nCookie++;

	return TRUE;
}

BOOL CKademlia::OnPacket_KADEMLIA2_PING(const SOCKADDR_IN* pHost, CEDPacket* /*pPacket*/)
{
	return Send( pHost, KADEMLIA2_PONG ); 
}

BOOL CKademlia::OnPacket_KADEMLIA2_PONG(const SOCKADDR_IN* /*pHost*/, CEDPacket* /*pPacket*/)
{
	// TODO: Implement Kademlia Pong packet handling

	return TRUE;
}
