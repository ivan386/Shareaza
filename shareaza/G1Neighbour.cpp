//
// G1Neighbour.cpp
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
#include "Network.h"
#include "Buffer.h"
#include "Statistics.h"
#include "Neighbours.h"
#include "Handshakes.h"
#include "G1Neighbour.h"
#include "G1Packet.h"
#include "G2Packet.h"
#include "HostCache.h"
#include "RouteCache.h"
#include "PacketBuffer.h"
#include "Security.h"
#include "GProfile.h"
#include "PongCache.h"
#include "VendorCache.h"
#include "QuerySearch.h"
#include "QueryHit.h"
#include "QueryHashTable.h"
#include "LocalSearch.h"
#include "SearchManager.h"
#include "DiscoveryServices.h"
#include "Downloads.h"
#include "Uploads.h"
#include "Library.h"
#include "SHA.h"

#include "WndMain.h"
#include "WndChild.h"
#include "WndSearchMonitor.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CG1Neighbour construction

CG1Neighbour::CG1Neighbour(CNeighbour* pBase) : CNeighbour( PROTOCOL_G1, pBase )
{
	ZeroMemory( m_nPongNeeded, PONG_NEEDED_BUFFER );

	m_tLastOutPing	= m_tLastPacket;
	m_nHopsFlow		= 0xFF;
	m_pOutbound		= new CG1PacketBuffer( m_pZOutput ? m_pZOutput : m_pOutput );

	theApp.Message( MSG_DEFAULT, IDS_HANDSHAKE_ONLINE, (LPCTSTR)m_sAddress,
				0, 6, m_sUserAgent.IsEmpty() ? _T("Unknown") : (LPCTSTR)m_sUserAgent );
				//0, m_bShake06 ? 6 : 4, m_sUserAgent.IsEmpty() ? _T("Unknown") : (LPCTSTR)m_sUserAgent );

	Send( CG1Packet::New( G1_PACKET_PING ) );
	
	if ( Settings.Gnutella1.VendorMsg && m_bVendorMsg )
	{
		CG1Packet* pVendor = CG1Packet::New( G1_PACKET_VENDOR, 1 );
		pVendor->WriteLongLE( 0 );
		pVendor->WriteShortLE( 0 );
		pVendor->WriteShortLE( 0 );
		pVendor->WriteShortLE( 6 );
		pVendor->WriteLongLE( 'RAEB' );
		pVendor->WriteShortLE( 0x0004 );
		pVendor->WriteShortLE( 1 );
		pVendor->WriteLongLE( 'RAEB' );
		pVendor->WriteShortLE( 0x000B );
		pVendor->WriteShortLE( 1 );
		pVendor->WriteLongLE( 'RAEB' );
		pVendor->WriteShortLE( 0x000C );
		pVendor->WriteShortLE( 1 );
		pVendor->WriteLongLE( 'AZAR' );
		pVendor->WriteShortLE( 0x0001 );
		pVendor->WriteShortLE( 1 );
		pVendor->WriteLongLE( 'AZAR' );
		pVendor->WriteShortLE( 0x0002 );
		pVendor->WriteShortLE( 1 );
		pVendor->WriteLongLE( 'AZAR' );
		pVendor->WriteShortLE( 0x0003 );
		pVendor->WriteShortLE( 1 );
		Send( pVendor );
	}
}

CG1Neighbour::~CG1Neighbour()
{
	if ( m_pOutbound ) delete m_pOutbound;
}

//////////////////////////////////////////////////////////////////////
// CG1Neighbour read and write events

BOOL CG1Neighbour::OnRead()
{
	CNeighbour::OnRead();
	return ProcessPackets();
}

BOOL CG1Neighbour::OnWrite()
{
	CBuffer* pOutput	= m_pZOutput ? m_pZOutput : m_pOutput;
	DWORD nExpire		= GetTickCount();

	CNeighbour::OnWrite();

	while ( pOutput->m_nLength == 0 && m_pOutbound->m_nTotal > 0 )
	{
		CG1Packet* pPacket = m_pOutbound->GetPacketToSend( nExpire );
		if ( ! pPacket ) break;

		pPacket->ToBuffer( pOutput );
		pPacket->Release();

		m_pOutbound->m_nTotal --;

		CNeighbour::OnWrite();
	}

	m_nOutbound		= m_pOutbound->m_nTotal;
	m_nLostCount	= m_pOutbound->m_nDropped;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG1Neighbour run event

BOOL CG1Neighbour::OnRun()
{
	if ( ! CNeighbour::OnRun() ) return FALSE;
	
	DWORD tNow = GetTickCount();
	
	SendPing( tNow, NULL );
	
	// if ( m_bShareaza && tNow - m_tClusterSent > 60000 ) SendClusterAdvisor();
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG1Neighbour send packet

BOOL CG1Neighbour::Send(CPacket* pPacket, BOOL bRelease, BOOL bBuffered)
{
	CG1Packet* pPacketG1 = (CG1Packet*)pPacket;
	BOOL bSuccess = FALSE;
	
	if ( m_nState >= nrsConnected && pPacket->m_nProtocol == PROTOCOL_G1 && pPacketG1->m_nTTL )
	{
		m_nOutputCount++;
		Statistics.Current.Gnutella1.Outgoing++;
		
		m_pOutbound->Add( pPacketG1, bBuffered );
		QueueRun();
		
		pPacketG1->SmartDump( this, NULL, TRUE );
		
		bSuccess = TRUE;
	}
	
	if ( bRelease ) pPacket->Release();
	
	return bSuccess;
}

//////////////////////////////////////////////////////////////////////
// CG1Neighbour packet dispatch

BOOL CG1Neighbour::ProcessPackets()
{
	CBuffer* pInput = m_pZInput ? m_pZInput : m_pInput;
	
    BOOL bSuccess = TRUE;
	for ( ; bSuccess ; )
	{
		GNUTELLAPACKET* pPacket = (GNUTELLAPACKET*)pInput->m_pBuffer;
		if ( pInput->m_nLength < sizeof(*pPacket) ) break;
		
		DWORD nLength = sizeof(*pPacket) + pPacket->m_nLength;
		
		if ( pPacket->m_nLength < 0 || nLength >= Settings.Gnutella1.MaximumPacket )
		{
			Close( IDS_PROTOCOL_TOO_LARGE );
			return FALSE;
		}
		
		if ( pInput->m_nLength < nLength ) break;
		
		CG1Packet* pPacketObject = CG1Packet::New( pPacket );
		bSuccess = OnPacket( pPacketObject );
		pPacketObject->Release();
		
		pInput->Remove( nLength );
	}
	
	if ( bSuccess ) return TRUE;
	
	Close( 0 );
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CG1Neighbour packet handler

BOOL CG1Neighbour::OnPacket(CG1Packet* pPacket)
{
	m_nInputCount++;
	m_tLastPacket = GetTickCount();
	Statistics.Current.Gnutella1.Incoming++;
	
	if ( pPacket->m_nTTL == 0 )
	{
		// This is valid- no need to worry the user
		//theApp.Message( MSG_DEBUG, IDS_PROTOCOL_NO_TTL, (LPCTSTR)m_sAddress );
	}
	else if (	(DWORD)pPacket->m_nTTL + pPacket->m_nHops > Settings.Gnutella1.MaximumTTL &&
				pPacket->m_nType != G1_PACKET_PUSH && pPacket->m_nType != G1_PACKET_HIT )
	{
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_HIGH_TTL, (LPCTSTR)m_sAddress,
			pPacket->m_nTTL, pPacket->m_nHops );
		pPacket->m_nTTL = 1;
	}
	
	pPacket->SmartDump( this, NULL, FALSE );

	switch ( pPacket->m_nType )
	{
	case G1_PACKET_PING:
		return OnPing( pPacket );

	case G1_PACKET_PONG:
		return OnPong( pPacket );

	case G1_PACKET_BYE:
		return OnBye( pPacket );

	case G1_PACKET_QUERY_ROUTE:
		return OnCommonQueryHash( pPacket );

	case G1_PACKET_VENDOR:
	case G1_PACKET_VENDOR_APP:
		return OnVendor( pPacket );

	case G1_PACKET_PUSH:
		return OnPush( pPacket );

	case G1_PACKET_QUERY:
		return OnQuery( pPacket );

	case G1_PACKET_HIT:
		return OnHit( pPacket );
	}
	
	theApp.Message( MSG_ERROR, IDS_PROTOCOL_UNKNOWN, (LPCTSTR)m_sAddress, pPacket->m_nType );
	
	return ! Settings.Gnutella1.StrictPackets;
}

//////////////////////////////////////////////////////////////////////
// CG1Neighbour PING packet handlers

BOOL CG1Neighbour::SendPing(DWORD dwNow, GGUID* pGUID)
{
	if ( m_nNodeType == ntLeaf && pGUID != NULL ) return FALSE;

	BOOL bNeedPeers =	Neighbours.NeedMoreHubs( PROTOCOL_G1 ) ||
						Neighbours.NeedMoreLeafs( PROTOCOL_G1 );

	if ( ! dwNow ) dwNow = GetTickCount();

	if ( m_bPongCaching )
	{
		if ( dwNow - m_tLastOutPing < Settings.Gnutella1.PingRate * 2 ) return FALSE;
		// if ( ! bNeedPeers && dwNow - m_tLastPacket < Settings.Gnutella1.PingRate ) return FALSE;
	}
	else
	{
		if ( dwNow - m_tLastOutPing < Settings.Gnutella1.PingRate * 2 ) return FALSE;
	}

	m_tLastOutPing = dwNow;

	Send( CG1Packet::New( G1_PACKET_PING, ( pGUID || bNeedPeers ) ? 0 : 1, pGUID ), TRUE, TRUE );

	return TRUE;
}

BOOL CG1Neighbour::OnPing(CG1Packet* pPacket)
{
	if ( ! Neighbours.m_pPingRoute->Add( &pPacket->m_pGUID, this ) )
	{
		Statistics.Current.Gnutella1.Dropped++;
		m_nDropCount++;
		return TRUE;
	}

	if ( pPacket->m_nLength != 0 && Settings.Gnutella1.StrictPackets )
	{
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_SIZE_PING, (LPCTSTR)m_sAddress );
		Statistics.Current.Gnutella1.Dropped++;
		m_nDropCount++;
		return TRUE;
	}
	else if ( pPacket->m_nLength > Settings.Gnutella1.MaximumQuery )
	{
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_TOO_LARGE, (LPCTSTR)m_sAddress );
		Statistics.Current.Gnutella1.Dropped++;
		m_nDropCount++;
		return TRUE;
	}

	BOOL bIsKeepAlive	= ( pPacket->m_nTTL == 1 && pPacket->m_nHops == 0 );
	DWORD dwNow			= GetTickCount();
	
	if ( dwNow - m_tLastInPing < Settings.Gnutella1.PingFlood && ! bIsKeepAlive )
	{
		Statistics.Current.Gnutella1.Dropped++;
		m_nDropCount++;
		return TRUE;
	}
	
	if ( pPacket->m_nLength && m_bGGEP )
	{
		if ( pPacket->ReadByte() != GGEP_MAGIC )
		{
			theApp.Message( MSG_ERROR, IDS_PROTOCOL_GGEP_REQUIRED, (LPCTSTR)m_sAddress );
			Statistics.Current.Gnutella1.Dropped++;
			m_nDropCount++;
			return TRUE;
		}
		else if ( pPacket->Hop() )
		{
			if ( Neighbours.Broadcast( pPacket, this, TRUE ) )
				Statistics.Current.Gnutella1.Routed ++;
			pPacket->m_nHops --; // Dehop
			pPacket->m_nTTL ++;
		}
	}
	
	m_tLastInPing	= dwNow;
	m_nLastPingHops	= pPacket->m_nHops + 1;
	m_pLastPingID	= pPacket->m_pGUID;
	
	if ( pPacket->m_nTTL == 2 && pPacket->m_nHops == 0 )
	{
		for ( POSITION pos = Neighbours.GetIterator() ; pos ; )
		{
			CNeighbour* pConnection = Neighbours.GetNext( pos );
			if ( pConnection->m_nState != nrsConnected ) continue;

			CG1Packet* pPong = CG1Packet::New( G1_PACKET_PONG, m_nLastPingHops, &m_pLastPingID );
			
			pPong->WriteShortLE( htons( pConnection->m_pHost.sin_port ) );
			pPong->WriteLongLE( pConnection->m_pHost.sin_addr.S_un.S_addr );
			pPong->WriteLongLE( 0 );
			pPong->WriteLongLE( 0 );

			Send( pPong );
		}
		
		// theApp.Message( MSG_DEFAULT, IDS_CONNECTION_CRAWLER, (LPCTSTR)m_sAddress );
		
		return TRUE;
	}
	
	if ( bIsKeepAlive || ( Network.IsListening() && ! Neighbours.IsG1Leaf() ) )
	{
		CG1Packet* pPong = CG1Packet::New( G1_PACKET_PONG, m_nLastPingHops, &m_pLastPingID );
		
		QWORD nMyVolume;
		DWORD nMyFiles;
		LibraryMaps.GetStatistics( &nMyFiles, &nMyVolume );
		
		pPong->WriteShortLE( htons( Network.m_pHost.sin_port ) );
		pPong->WriteLongLE( Network.m_pHost.sin_addr.S_un.S_addr );
		pPong->WriteLongLE( nMyFiles );
		pPong->WriteLongLE( (DWORD)nMyVolume );
		
		Send( pPong );
	}
	
	if ( bIsKeepAlive || m_nNodeType == ntHub || ! pPacket->Hop() ) return TRUE;
	
	Neighbours.OnG1Ping();
	
	CPtrList pIgnore;
	
	ZeroMemory( m_nPongNeeded, PONG_NEEDED_BUFFER ); 
	
	for ( BYTE nHops = 1 ; nHops <= pPacket->m_nTTL ; nHops++ )
	{
		m_nPongNeeded[ nHops ] = Settings.Gnutella1.PongCount / pPacket->m_nTTL;
		
		CPongItem* pCache = NULL;
		
		while (	( m_nPongNeeded[ nHops ] > 0 ) &&
				( pCache = Neighbours.m_pPongCache->Lookup( this, nHops, &pIgnore ) ) )
		{
			Send( pCache->ToPacket( m_nLastPingHops, &m_pLastPingID ) );

			pIgnore.AddTail( pCache );
			m_nPongNeeded[ nHops ]--;
		}
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG1Neighbour PONG packet handlers

BOOL CG1Neighbour::OnPong(CG1Packet* pPacket)
{
	if ( pPacket->m_nLength < 14 || ( pPacket->m_nLength > 14 && Settings.Gnutella1.StrictPackets ) )
	{
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_SIZE_PONG, (LPCTSTR)m_sAddress );
		Statistics.Current.Gnutella1.Dropped++;
		m_nDropCount++;
		return TRUE;
	}

	WORD nPort		= pPacket->ReadShortLE();
	DWORD nAddress	= pPacket->ReadLongLE();
	DWORD nFiles	= pPacket->ReadLongLE();
	DWORD nVolume	= pPacket->ReadLongLE();

	if ( Security.IsDenied( (IN_ADDR*)&nAddress ) )
	{
		Statistics.Current.Gnutella1.Dropped++;
		m_nDropCount++;
		return TRUE;
	}

	if ( pPacket->m_nLength > 14 && m_bGGEP )
	{
		if ( pPacket->ReadByte() != GGEP_MAGIC )
		{
			theApp.Message( MSG_ERROR, IDS_PROTOCOL_GGEP_REQUIRED, (LPCTSTR)m_sAddress );
			Statistics.Current.Gnutella1.Dropped++;
			m_nDropCount++;
			return TRUE;
		}
		else if ( pPacket->Hop() )
		{
			CG1Neighbour* pOrigin;
			
			Neighbours.m_pPingRoute->Lookup( &pPacket->m_pGUID, (CNeighbour**)&pOrigin );

			if ( pOrigin && pOrigin->m_bGGEP )
			{
				Statistics.Current.Gnutella1.Routed++;
				pOrigin->Send( pPacket, FALSE, TRUE );
			}

			pPacket->m_nHops--;	// Dehop
		}
	}

	BOOL bLocal = ! nPort || Network.IsFirewalledAddress( &nAddress );

	if ( pPacket->m_nHops != 0 && bLocal )
	{
		if ( pPacket->m_nHops ) theApp.Message( MSG_DEBUG, IDS_PROTOCOL_ZERO_PONG, (LPCTSTR)m_sAddress );
		Statistics.Current.Gnutella1.Dropped++;
		m_nDropCount++;
		return TRUE;
	}

	if ( ! bLocal && ! Network.IsFirewalledAddress( &nAddress, TRUE ) )
	{
		if ( pPacket->m_nHops == 0 && nAddress == m_pHost.sin_addr.S_un.S_addr )
		{
			m_nFileCount	= nFiles;
			m_nFileVolume	= nVolume;

			HostCache.Gnutella1.Add( (IN_ADDR*)&nAddress, nPort, 0, m_bShareaza ? SHAREAZA_VENDOR_T : NULL );
		}
		else
		{
			HostCache.Gnutella1.Add( (IN_ADDR*)&nAddress, nPort );
		}
	}
	
	BYTE nHops = pPacket->m_nHops + 1;
	nHops = min( nHops, BYTE(PONG_NEEDED_BUFFER - 1) );
	
	if ( ! bLocal ) Neighbours.OnG1Pong( this, (IN_ADDR*)&nAddress, nPort, nHops + 1, nFiles, nVolume );
	
	return TRUE;
}

void CG1Neighbour::OnNewPong(CPongItem* pPong)
{
	if ( m_nPongNeeded[ pPong->m_nHops ] > 0 )
	{
		Send( pPong->ToPacket( m_nLastPingHops, &m_pLastPingID ) );
		m_nPongNeeded[ pPong->m_nHops ] --;
	}
}

//////////////////////////////////////////////////////////////////////
// CG1Neighbour BYE packet handler

BOOL CG1Neighbour::OnBye(CG1Packet* pPacket)
{
	CString strReason;
	WORD nReason = 0;
	
	if ( pPacket->m_nLength >= 3 )
	{
		nReason		= pPacket->ReadShortLE();
		strReason	= pPacket->ReadString();
	}
	
	for ( int nChar = 0 ; nChar < strReason.GetLength() ; nChar++ )
	{
		if ( strReason[nChar] < 32 )
		{
			strReason = strReason.Left( nChar );
			break;
		}
	}

	if ( strReason.IsEmpty() || strReason.GetLength() > 128 ) strReason = _T("No Message");

	theApp.Message( MSG_ERROR, IDS_CONNECTION_BYE, (LPCTSTR)m_sAddress,
		nReason, (LPCTSTR)strReason );

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CG1Neighbour VENDOR packet handler

BOOL CG1Neighbour::OnVendor(CG1Packet* pPacket)
{
	if ( pPacket->m_nLength < 8 || ! Settings.Gnutella1.VendorMsg )
	{
		Statistics.Current.Gnutella1.Dropped++;
		m_nDropCount++;
		return TRUE;
	}

	DWORD nVendor	= pPacket->ReadLongLE();
	WORD nFunction	= pPacket->ReadShortLE();
	WORD nVersion	= pPacket->ReadShortLE();
	
	if ( nVendor == 0 && nFunction == 0 )
	{
		// Supported vendor messages array
	}
	else if ( nFunction == 0xFFFF )
	{
		if ( nVendor == 0 )
		{
			// Vendor code query
			CG1Packet* pReply = CG1Packet::New( pPacket->m_nType, 1, &pPacket->m_pGUID );
			pReply->WriteLongLE( 0 );
			pReply->WriteShortLE( 0xFFFE );
			pReply->WriteShortLE( 1 );
			pReply->WriteLongLE( 'AZAR' );
			pReply->WriteLongLE( 'RAEB' );
			Send( pReply );
		}
		else if ( nVendor == 'AZAR' )
		{
			// Function code query for "RAZA"
			CG1Packet* pReply = CG1Packet::New( pPacket->m_nType, 1, &pPacket->m_pGUID );
			pReply->WriteLongLE( 'AZAR' );
			pReply->WriteShortLE( 0xFFFE );
			pReply->WriteShortLE( 1 );
			pReply->WriteShortLE( 0x0001 );
			pReply->WriteShortLE( 1 );
			pReply->WriteShortLE( 0x0002 );
			pReply->WriteShortLE( 1 );
			pReply->WriteShortLE( 0x0003 );
			pReply->WriteShortLE( 1 );
			Send( pReply );
		}
		else if ( nVendor == 'RAEB' )
		{
			// Function code query for "BEAR"
			CG1Packet* pReply = CG1Packet::New( pPacket->m_nType, 1, &pPacket->m_pGUID );
			pReply->WriteLongLE( 'RAEB' );
			pReply->WriteShortLE( 0xFFFE );
			pReply->WriteShortLE( 1 );
			pReply->WriteShortLE( 0x0004 );
			pReply->WriteShortLE( 1 );
			pReply->WriteShortLE( 0x000B );
			pReply->WriteShortLE( 1 );
			pReply->WriteShortLE( 0x000C );
			pReply->WriteShortLE( 1 );
			Send( pReply );
		}
	}
	else if ( nVendor == 'AZAR' )
	{
		switch ( nFunction )
		{
		case 0x0001:
			// Version Query
			if ( nVersion <= 1 )
			{
				CG1Packet* pReply = CG1Packet::New( pPacket->m_nType, 1, &pPacket->m_pGUID );
				pReply->WriteLongLE( 'AZAR' );
				pReply->WriteShortLE( 0x0002 );
				pReply->WriteShortLE( 1 );
				pReply->WriteShortLE( theApp.m_nVersion[0] );
				pReply->WriteShortLE( theApp.m_nVersion[1] );
				pReply->WriteShortLE( theApp.m_nVersion[2] );
				pReply->WriteShortLE( theApp.m_nVersion[3] );
				Send( pReply );
			}
			break;
		case 0x0002:
			// Version Response
			if ( nVersion <= 1 && pPacket->GetRemaining() >= 8 )
			{
				WORD nVersion[4];
				nVersion[0] = pPacket->ReadShortLE();
				nVersion[1] = pPacket->ReadShortLE();
				nVersion[2] = pPacket->ReadShortLE();
				nVersion[3] = pPacket->ReadShortLE();
			}
			break;
		case 0x0003:
			// Cluster Advisor
			if ( nVersion <= 1 && pPacket->GetRemaining() >= 28 )
			{
				OnClusterAdvisor( pPacket );
			}
			break;
		}
	}
	else if ( nVendor == 'RAEB' )
	{
		switch ( nFunction )
		{
		case 0x0001:
			// Super Pong
			// (WORD)Count, [ (DWORD)IP, (WORD)Port ], SIG
			break;
		case 0x0003:
			// Product Identifiers
			break;
		case 0x0004:
			// Hops Flow
			if ( nVersion <= 1 && pPacket->GetRemaining() >= 1 ) 
			{
				m_nHopsFlow = pPacket->ReadByte();
			}
			break;
		case 0x0005:
			// Horizon Ping
			// (BYTE)TTL
			break;
		case 0x0006:
			// Horizon Pong
			// (BYTE)maxTTL, (DWORD)sharing, (DWORD)freepeers, (DWORD)auth'ed
			break;
		case 0x000B:
			// Query Status Request
			if ( nVersion <= 1 )
			{
				CG1Packet* pReply = CG1Packet::New( pPacket->m_nType, 1, &pPacket->m_pGUID );
				pReply->WriteLongLE( 'RAEB' );
				pReply->WriteShortLE( 0x000C );
				pReply->WriteShortLE( 1 );
				pReply->WriteShortLE( SearchManager.OnQueryStatusRequest( &pPacket->m_pGUID ) );
				Send( pReply );
			}
			break;
		case 0x000C:
			// Query Status Response
			// (WORD)hit_count
			break;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG1Neighbour VENDOR cluster handlers

void CG1Neighbour::SendClusterAdvisor()
{
	if ( ! m_bShareaza || ! Settings.Gnutella1.VendorMsg ) return;

	DWORD tNow = time( NULL );
	CG1Packet* pPacket = NULL;
	WORD nCount = 0;

	for ( CHostCacheHost* pHost = HostCache.Gnutella1.GetNewest() ; pHost && nCount < 20 ; pHost = pHost->m_pPrevTime )
	{
		if ( pHost->m_pVendor == VendorCache.m_pShareaza &&
			 pHost->m_tAdded > m_tClusterHost &&
			 pHost->CanConnect( tNow ) )
		{
			if ( ! pPacket )
			{
				pPacket = CG1Packet::New( G1_PACKET_VENDOR, 1 );
				pPacket->WriteLongLE( 'AZAR' );
				pPacket->WriteShortLE( 0x0003 );
				pPacket->WriteShortLE( 1 );
				pPacket->WriteShortLE( 0 );
			}
			pPacket->WriteLongLE( pHost->m_pAddress.S_un.S_addr );
			pPacket->WriteShortLE( pHost->m_nPort );
			nCount++;
		}
	}
	
	m_tClusterHost = GetTickCount();
	
	if ( pPacket && nCount )
	{
		m_tClusterSent = m_tClusterHost;
		((WORD*)pPacket->m_pBuffer)[4] = nCount;
		pPacket->RazaSign();
		Send( pPacket, TRUE, TRUE );
	}
}

BOOL CG1Neighbour::OnClusterAdvisor(CG1Packet* pPacket)
{
	if ( ! pPacket->RazaVerify() ) return FALSE;
	
	WORD nCount = pPacket->ReadShortLE();
	if ( pPacket->GetRemaining() < nCount * 6 + 20 ) return FALSE;
	
	SendClusterAdvisor();
	
	while ( nCount-- )
	{
		DWORD nAddress	= pPacket->ReadLongLE();
		WORD nPort		= pPacket->ReadShortLE();
		HostCache.Gnutella1.Add( (IN_ADDR*)&nAddress, nPort, 0, SHAREAZA_VENDOR_T );
	}
	
	m_tClusterHost = GetTickCount();
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG1Neighbour PUSH packet handler

BOOL CG1Neighbour::OnPush(CG1Packet* pPacket)
{
	if ( pPacket->m_nLength < 26 || ( pPacket->m_nLength > 26 && Settings.Gnutella1.StrictPackets ) )
	{
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_SIZE_PUSH, (LPCTSTR)m_sAddress );
		Statistics.Current.Gnutella1.Dropped++;
		m_nDropCount++;
		return TRUE;
	}
	
	GGUID pClientID;
	pPacket->Read( &pClientID, 16 );
	DWORD nFileIndex	= pPacket->ReadLongLE();
	DWORD nAddress		= pPacket->ReadLongLE();
	WORD nPort			= pPacket->ReadShortLE();
	BOOL bGGEP			= FALSE;
	
	if ( Security.IsDenied( (IN_ADDR*)&nAddress ) )
	{
		Statistics.Current.Gnutella1.Dropped++;
		m_nDropCount++;
		return TRUE;
	}
	
	if ( pPacket->m_nLength > 26 && m_bGGEP )
	{
		if ( pPacket->ReadByte() != GGEP_MAGIC )
		{
			theApp.Message( MSG_ERROR, IDS_PROTOCOL_GGEP_REQUIRED, (LPCTSTR)m_sAddress );
			Statistics.Current.Gnutella1.Dropped++;
			m_nDropCount++;
			return TRUE;
		}
		bGGEP = TRUE;
	}
	
	if ( ! nPort || Network.IsFirewalledAddress( &nAddress ) )
	{
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_ZERO_PUSH, (LPCTSTR)m_sAddress );
		Statistics.Current.Gnutella1.Dropped++;
		m_nDropCount++;
		return TRUE;
	}
	
	if ( pClientID == MyProfile.GUID )
	{
		Handshakes.PushTo( (IN_ADDR*)&nAddress, nPort, nFileIndex );
		return TRUE;
	}
	
	CNeighbour* pOrigin;
	
	Network.NodeRoute->Lookup( &pClientID, (CNeighbour**)&pOrigin );
	
	if ( pOrigin && pPacket->Hop() )
	{
		if ( pOrigin->m_nProtocol == PROTOCOL_G1 )
		{
			if ( bGGEP && ! pOrigin->m_bGGEP ) pPacket->Shorten( 26 );
			pOrigin->Send( pPacket, FALSE, TRUE );
		}
		else if ( pOrigin->m_nProtocol == PROTOCOL_G2 )
		{
			CG2Packet* pWrap = CG2Packet::New( G2_PACKET_PUSH, TRUE );
			pWrap->WritePacket( "TO", 16 );
			pWrap->Write( &pClientID, 16 );
			pWrap->WriteByte( 0 );
			pWrap->WriteLongLE( nAddress );
			pWrap->WriteShortLE( nPort );
			pOrigin->Send( pWrap, TRUE, TRUE );
		}
		Statistics.Current.Gnutella1.Routed++;
	}
	
	return TRUE;
}

void CG1Neighbour::SendG2Push(GGUID* pGUID, CPacket* pPacket)
{
	if ( pPacket->GetRemaining() < 6 ) return;
	
	DWORD nAddress	= pPacket->ReadLongLE();
	WORD nPort		= pPacket->ReadShortLE();
	
	pPacket = CG1Packet::New( G1_PACKET_PUSH, Settings.Gnutella1.MaximumTTL - 1 );

	pPacket->Write( pGUID, 16 );
	pPacket->WriteLongLE( 0 );
	pPacket->WriteLongLE( nAddress );
	pPacket->WriteShortLE( nPort );
	
	Send( pPacket, TRUE, TRUE );
}

//////////////////////////////////////////////////////////////////////
// CG1Neighbour QUERY packet handlers

BOOL CG1Neighbour::OnQuery(CG1Packet* pPacket)
{
	if ( pPacket->m_nLength <= 5 )
	{
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_BAD_QUERY, (LPCTSTR)m_sAddress );
		Statistics.Current.Gnutella1.Dropped++;
		m_nDropCount++;
		return TRUE;
	}
	else if ( pPacket->m_nLength > Settings.Gnutella1.MaximumQuery )
	{
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_TOO_LARGE, (LPCTSTR)m_sAddress );
		Statistics.Current.Gnutella1.Dropped++;
		m_nDropCount++;
		return FALSE;
	}

	if ( m_nNodeType == ntLeaf && pPacket->m_nHops > 0 )
	{
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_LEAF_FORWARD, (LPCTSTR)m_sAddress );
		Statistics.Current.Gnutella1.Dropped++;
		m_nDropCount++;
		return FALSE;
	}

	if ( ! Network.QueryRoute->Add( &pPacket->m_pGUID, this ) )
	{
		Statistics.Current.Gnutella1.Dropped++;
		m_nDropCount++;
		return TRUE;
	}

	CQuerySearch* pSearch = CQuerySearch::FromPacket( pPacket );

	if ( pSearch == NULL )
	{
		pPacket->Debug( _T("BadQuery") );
		theApp.Message( MSG_DEBUG, IDS_PROTOCOL_BAD_QUERY, (LPCTSTR)m_sAddress );
		Statistics.Current.Gnutella1.Dropped++;
		m_nDropCount++;
		return TRUE;
	}
	
	CLocalSearch pLocalSearch( pSearch, this );

	if ( m_nNodeType != ntHub && pPacket->Hop() )
	{
		Neighbours.RouteQuery( pSearch, pPacket, this, TRUE );
	}
	
	Network.OnQuerySearch( pSearch );
	
	if ( ! pSearch->m_bFirewall || Network.IsListening() ) pLocalSearch.Execute();
	
	delete pSearch;
	
	Statistics.Current.Gnutella1.Queries++;
	
	return TRUE;
}

BOOL CG1Neighbour::SendQuery(CQuerySearch* pSearch, CPacket* pPacket, BOOL bLocal)
{
	if ( m_nState != nrsConnected ) return FALSE;
	if ( pPacket == NULL || pPacket->m_nProtocol != PROTOCOL_G1 ) return FALSE;

	CG1Packet* pG1 = (CG1Packet*)pPacket;

	if ( pG1->m_nHops > m_nHopsFlow )
	{
		return FALSE;
	}
	else if ( m_nNodeType == ntHub && ! bLocal )
	{
		return FALSE;
	}
	else if ( ! pSearch->m_bAndG1 )
	{
		return FALSE;
	}
	else if ( m_pQueryTableRemote != NULL && m_pQueryTableRemote->m_bLive )
	{
		if ( ! m_pQueryTableRemote->Check( pSearch ) ) return FALSE;
	}
	else if ( m_nNodeType == ntLeaf && ! bLocal )
	{
		return FALSE;
	}
	
	if ( bLocal ) m_tLastQuery = time( NULL );

	Send( pPacket, FALSE, ! bLocal );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG1Neighbour QUERY HIT packet handler

BOOL CG1Neighbour::OnHit(CG1Packet* pPacket)
{
	if ( pPacket->m_nLength <= 27 )
	{
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_BAD_HIT, (LPCTSTR)m_sAddress );
		Statistics.Current.Gnutella1.Dropped++;
		m_nDropCount++;
		return TRUE;
	}

	return OnCommonHit( pPacket );
}

