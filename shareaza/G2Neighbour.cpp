//
// G2Neighbour.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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
#include "Security.h"
#include "Statistics.h"
#include "Neighbours.h"
#include "G2Neighbour.h"
#include "G2Packet.h"
#include "G1Packet.h"
#include "Buffer.h"
#include "Handshakes.h"
#include "Datagrams.h"
#include "HostCache.h"
#include "RouteCache.h"
#include "VendorCache.h"
#include "QuerySearch.h"
#include "QueryHit.h"
#include "Library.h"
#include "LocalSearch.h"
#include "SearchManager.h"
#include "QueryHashTable.h"
#include "HubHorizon.h"
#include "GProfile.h"
#include "Uploads.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CG2Neighbour construction

CG2Neighbour::CG2Neighbour(CNeighbour* pBase) : CNeighbour( PROTOCOL_G2, pBase )
{
	theApp.Message( MSG_DEFAULT, IDS_HANDSHAKE_ONLINE_G2, (LPCTSTR)m_sAddress,
		m_sUserAgent.IsEmpty() ? _T("Unknown") : (LPCTSTR)m_sUserAgent );

	m_nLeafCount	= 0;
	m_nLeafLimit	= 0;
	m_bCachedKeys	= FALSE;

	m_pGUIDCache	= new CRouteCache();
	m_pHubGroup		= new CHubHorizonGroup();

	m_tAdjust		= 0;
	m_tLastPingIn	= 0;
	m_tLastPingOut	= 0;
	m_tLastPacket	= GetTickCount();
	m_tWaitLNI		= m_tLastPacket;
	m_tLastKHL		= m_tLastPacket - Settings.Gnutella2.KHLPeriod + 1000;
	m_tLastHAW		= m_tLastPacket;

	m_nQueryLimiter	= 40;
	m_tQueryTimer	= 0;
	m_bBlacklisted	= FALSE;

	SendStartups();
}

CG2Neighbour::~CG2Neighbour()
{
	delete m_pHubGroup;
	delete m_pGUIDCache;

	for ( POSITION pos = m_pOutbound.GetHeadPosition() ; pos ; )
	{
		CG2Packet* pOutbound = (CG2Packet*)m_pOutbound.GetNext( pos );
		pOutbound->Release();
	}
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour read and write events

BOOL CG2Neighbour::OnRead()
{
	CNeighbour::OnRead();
	return ProcessPackets();
}

BOOL CG2Neighbour::OnWrite()
{
	CBuffer* pOutput = m_pZOutput ? m_pZOutput : m_pOutput;

	while ( pOutput->m_nLength == 0 && m_nOutbound > 0 )
	{
		CG2Packet* pPacket = (CG2Packet*)m_pOutbound.RemoveHead();
		m_nOutbound--;

		pPacket->ToBuffer( pOutput );
		pPacket->Release();

		CNeighbour::OnWrite();
	}

	CNeighbour::OnWrite();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour run event

BOOL CG2Neighbour::OnRun()
{
	if ( ! CNeighbour::OnRun() ) return FALSE;

	DWORD tNow = GetTickCount();

	if ( m_tWaitLNI > 0 && tNow - m_tWaitLNI > Settings.Gnutella2.KHLPeriod * 3 )
	{
		Close( IDS_CONNECTION_TIMEOUT_TRAFFIC );
		return FALSE;
	}
	else if ( tNow - m_tLastPingOut >= Settings.Gnutella1.PingRate )
	{
		BOOL bNeedStable = Network.IsListening() && ! Datagrams.IsStable();
		CG2Packet* pPing = CG2Packet::New( G2_PACKET_PING, TRUE );

		if ( bNeedStable )
		{
			pPing->WritePacket( "UDP", 6 );
			pPing->WriteLongLE( Network.m_pHost.sin_addr.S_un.S_addr );
			pPing->WriteShortBE( htons( Network.m_pHost.sin_port ) );
		}

		Send( pPing );
		Datagrams.Send( &m_pHost, CG2Packet::New( G2_PACKET_PING ) );
		m_tLastPingOut = tNow;
	}

	if ( tNow - m_tLastKHL > Settings.Gnutella2.KHLPeriod )
	{
		SendLNI();
		SendKHL();
	}
	else if ( tNow - m_tLastHAW > Settings.Gnutella2.HAWPeriod )
	{
		SendHAW();
	}

	// Update allowed queries based on the node type
	if ( m_nNodeType == ntLeaf )
	{
		if ( ( tNow - m_tQueryTimer ) > ( 5*60*1000 ) )
		{
			if ( m_nQueryLimiter < 60 ) m_nQueryLimiter ++;
			m_tQueryTimer = tNow;
		}
	}
	else
	{
		if ( ( tNow - m_tQueryTimer ) > ( 1000 ) )
		{
			if ( m_nQueryLimiter < 240 ) m_nQueryLimiter += 10;
			m_tQueryTimer = tNow;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour send packet

BOOL CG2Neighbour::Send(CPacket* pPacket, BOOL bRelease, BOOL bBuffered)
{
	BOOL bSuccess = FALSE;

	if ( m_nState >= nrsConnected && pPacket->m_nProtocol == PROTOCOL_G2 )
	{
		m_nOutputCount++;
		Statistics.Current.Gnutella2.Outgoing++;

		if ( bBuffered )
		{
			if ( m_nOutbound >= Settings.Gnutella1.PacketBufferSize )
			{
				CG2Packet* pRemove = (CG2Packet*)m_pOutbound.RemoveTail();
				pRemove->Release();
				m_nOutbound--;
				Statistics.Current.Gnutella2.Lost++;
			}

			pPacket->AddRef();
			m_pOutbound.AddHead( pPacket );
			m_nOutbound++;
		}
		else
		{
			pPacket->ToBuffer( m_pZOutput ? m_pZOutput : m_pOutput );
		}

		QueueRun();

		pPacket->SmartDump( this, NULL, TRUE );

		bSuccess = TRUE;
	}

	if ( bRelease ) pPacket->Release();

	return bSuccess;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour startup events

void CG2Neighbour::SendStartups()
{
	CG2Packet* pPing = CG2Packet::New( G2_PACKET_PING, TRUE );

	if ( Network.IsListening() )
	{
		pPing->WritePacket( "UDP", 6 );
		pPing->WriteLongLE( Network.m_pHost.sin_addr.S_un.S_addr );
		pPing->WriteShortBE( htons( Network.m_pHost.sin_port ) );
	}

	Send( pPing, TRUE, TRUE );
	m_tLastPingOut = GetTickCount();

	Datagrams.Send( &m_pHost, CG2Packet::New( G2_PACKET_PING ) );

	Send( CG2Packet::New( G2_PACKET_PROFILE_CHALLENGE ), TRUE, TRUE );
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour packet dispatch

BOOL CG2Neighbour::ProcessPackets()
{
	CBuffer* pInput = m_pZInput ? m_pZInput : m_pInput;

    BOOL bSuccess = TRUE;
	for ( ; bSuccess && pInput->m_nLength ; )
	{
		BYTE nInput = *(pInput->m_pBuffer);

		if ( nInput == 0 )
		{
			pInput->Remove( 1 );
			continue;
		}

		BYTE nLenLen	= ( nInput & 0xC0 ) >> 6;
		BYTE nTypeLen	= ( nInput & 0x38 ) >> 3;
		BYTE nFlags		= ( nInput & 0x07 );

		if ( (DWORD)pInput->m_nLength < (DWORD)nLenLen + nTypeLen + 2 ) break;

		DWORD nLength = 0;

		if ( nFlags & G2_FLAG_BIG_ENDIAN )
		{
			BYTE* pLenIn = pInput->m_pBuffer + 1;

			for ( BYTE nIt = nLenLen ; nIt ; nIt-- )
			{
				nLength <<= 8;
				nLength |= *pLenIn++;
			}
		}
		else
		{
			BYTE* pLenIn	= pInput->m_pBuffer + 1;
			BYTE* pLenOut	= (BYTE*)&nLength;
			for ( BYTE nLenCnt = nLenLen ; nLenCnt-- ; ) *pLenOut++ = *pLenIn++;
		}

		if ( nLength >= Settings.Gnutella1.MaximumPacket )
		{
			Close( IDS_PROTOCOL_TOO_LARGE );
			return FALSE;
		}

		if ( (DWORD)pInput->m_nLength < (DWORD)nLength + nLenLen + nTypeLen + 2 ) break;

		CG2Packet* pPacket = CG2Packet::New( pInput->m_pBuffer );

		try
		{
			bSuccess = OnPacket( pPacket );
		}
		catch ( CException* pException )
		{
			pException->Delete();
			// bSuccess = FALSE;		// SHOULD THIS BE LIKE THIS?
			bSuccess = TRUE;
		}

		pPacket->Release();

		pInput->Remove( nLength + nLenLen + nTypeLen + 2 );
	}

	if ( bSuccess ) return TRUE;

	Close( 0 );
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour packet handler

BOOL CG2Neighbour::OnPacket(CG2Packet* pPacket)
{
	m_nInputCount++;
	m_tLastPacket = GetTickCount();
	Statistics.Current.Gnutella2.Incoming++;

	pPacket->SmartDump( this, NULL, FALSE );

	if ( Network.RoutePacket( pPacket ) ) return TRUE;

	if ( pPacket->IsType( G2_PACKET_PING ) )
	{
		return OnPing( pPacket );
	}
	else if ( pPacket->IsType( G2_PACKET_LNI ) )
	{
		return OnLNI( pPacket );
	}
	else if ( pPacket->IsType( G2_PACKET_KHL ) )
	{
		return OnKHL( pPacket );
	}
	else if ( pPacket->IsType( G2_PACKET_HAW ) )
	{
		return OnHAW( pPacket );
	}
	else if ( pPacket->IsType( G2_PACKET_QUERY ) || pPacket->IsType( G2_PACKET_QUERY_WRAP ) )
	{
		return OnQuery( pPacket );
	}
	else if ( pPacket->IsType( G2_PACKET_HIT ) || pPacket->IsType( G2_PACKET_HIT_WRAP ) )
	{
		return OnCommonHit( pPacket );
	}
	else if ( pPacket->IsType( G2_PACKET_QUERY_ACK ) )
	{
		return OnQueryAck( pPacket );
	}
	else if ( pPacket->IsType( G2_PACKET_QUERY_KEY_REQ ) )
	{
		return OnQueryKeyReq( pPacket );
	}
	else if ( pPacket->IsType( G2_PACKET_QUERY_KEY_ANS ) )
	{
		return OnQueryKeyAns( pPacket );
	}
	else if ( pPacket->IsType( G2_PACKET_QHT ) )
	{
		return OnCommonQueryHash( pPacket );
	}
	else if ( pPacket->IsType( G2_PACKET_PUSH ) )
	{
		return OnPush( pPacket );
	}
	else if ( pPacket->IsType( G2_PACKET_PROFILE_CHALLENGE ) )
	{
		return OnProfileChallenge( pPacket );
	}
	else if ( pPacket->IsType( G2_PACKET_PROFILE_DELIVERY ) )
	{
		return OnProfileDelivery( pPacket );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour PING packet handler

BOOL CG2Neighbour::OnPing(CG2Packet* pPacket)
{
	Send( CG2Packet::New( G2_PACKET_PONG ) );

	if ( ! pPacket->m_bCompound ) return TRUE;

	BOOL bRelay = FALSE;
	DWORD nAddress = 0;
	WORD nPort = 0;
	CHAR szType[9];
	DWORD nLength;

	while ( pPacket->ReadPacket( szType, nLength ) )
	{
		DWORD nNext = pPacket->m_nPosition + nLength;

		if ( strcmp( szType, "UDP" ) == 0 && nLength >= 6 )
		{
			nAddress	= pPacket->ReadLongLE();
			nPort		= pPacket->ReadShortBE();
		}
		else if ( strcmp( szType, "RELAY" ) == 0 )
		{
			bRelay = TRUE;
		}

		pPacket->m_nPosition = nNext;
	}

	if ( ! nPort || Network.IsFirewalledAddress( &nAddress ) ) return TRUE;

	if ( bRelay )
	{
		CG2Packet* pPong = CG2Packet::New( G2_PACKET_PONG, TRUE );
		pPong->WritePacket( "RELAY", 0 );
		Datagrams.Send( (IN_ADDR*)&nAddress, nPort, pPong );
	}
	else
	{
		DWORD tNow = GetTickCount();
		if ( tNow - m_tLastPingIn < Settings.Gnutella1.PingFlood ) return TRUE;
		m_tLastPingIn = tNow;

		BYTE* pRelay = pPacket->WriteGetPointer( 7, 0 );
		*pRelay++ = 0x60;
		*pRelay++ = 0;
		*pRelay++ = 'R'; *pRelay++ = 'E'; *pRelay++ = 'L';
		*pRelay++ = 'A'; *pRelay++ = 'Y';

		CPtrArray pG2Nodes;

		for ( POSITION pos = Neighbours.GetIterator() ; pos ; )
		{
			CG2Neighbour* pNeighbour = (CG2Neighbour*)Neighbours.GetNext( pos );

			if (	pNeighbour->m_nState == nrsConnected &&
					pNeighbour->m_nProtocol == PROTOCOL_G2 &&
					pNeighbour != this)
			{
				pG2Nodes.Add( pNeighbour );
 			}
		}

		int nRelayTo = Settings.Gnutella2.PingRelayLimit;

		int nCount = pG2Nodes.GetCount();

		for ( int nCur = 0; (nCur < nCount && nCur < nRelayTo); nCur++ )
		{
			int nRand = rand() % pG2Nodes.GetCount();

			CG2Neighbour* pNeighbour = (CG2Neighbour*)pG2Nodes.GetAt( nRand );
			// Remove this debug message later
			theApp.Message( MSG_DEBUG, _T("Ping Relay iteration %i picked random index %i as %s"),
			nCur, nRand, (LPCTSTR)pNeighbour->m_sAddress  );

			pNeighbour->Send( pPacket, FALSE );
			pG2Nodes.RemoveAt( nRand );
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour LOCAL NODE INFO : send

void CG2Neighbour::SendLNI()
{
	CG2Packet* pPacket = CG2Packet::New( G2_PACKET_LNI, TRUE );

	QWORD nMyVolume = 0;
	DWORD nMyFiles = 0;
	LibraryMaps.GetStatistics( &nMyFiles, &nMyVolume );

	WORD nLeafs = 0;

	for ( POSITION pos = Neighbours.GetIterator() ; pos ; )
	{
		CNeighbour* pNeighbour = Neighbours.GetNext( pos );

		if (	pNeighbour != this &&
				pNeighbour->m_nState == nrsConnected &&
				pNeighbour->m_nNodeType == ntLeaf )
		{
			nMyFiles += pNeighbour->m_nFileCount;
			nMyVolume += pNeighbour->m_nFileVolume;
			nLeafs++;
		}
	}

	pPacket->WritePacket( "NA", 6 );
	pPacket->WriteLongLE( Network.m_pHost.sin_addr.S_un.S_addr );
	pPacket->WriteShortBE( htons( Network.m_pHost.sin_port ) );

	pPacket->WritePacket( "GU", 16 );
	GGUID tmp( MyProfile.GUID );
	pPacket->Write( &tmp, 16 );

	pPacket->WritePacket( "V", 4 );
	pPacket->WriteString( SHAREAZA_VENDOR_A, FALSE );

	pPacket->WritePacket( "LS", 8 );
	pPacket->WriteLongBE( (DWORD)nMyFiles );
	pPacket->WriteLongBE( (DWORD)nMyVolume );

	if ( ! Neighbours.IsG2Leaf() )
	{
		pPacket->WritePacket( "HS", 4 );
		pPacket->WriteShortBE( nLeafs );
		pPacket->WriteShortBE( Settings.Gnutella2.NumLeafs );

		pPacket->WritePacket( "QK", 0 );
	}

	Send( pPacket, TRUE, FALSE );
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour LOCAL NODE INFO : receive

BOOL CG2Neighbour::OnLNI(CG2Packet* pPacket)
{
	if ( ! pPacket->m_bCompound ) return TRUE;

	DWORD tNow = time( NULL );
	CHAR szType[9];
	DWORD nLength;

	m_nLeafCount = 0;

	while ( pPacket->ReadPacket( szType, nLength ) )
	{
		DWORD nNext = pPacket->m_nPosition + nLength;

		if ( strcmp( szType, "NA" ) == 0 && nLength >= 6 )
		{
			m_pHost.sin_addr.S_un.S_addr = pPacket->ReadLongLE();
			m_pHost.sin_port = htons( pPacket->ReadShortBE() );
		}
		else if ( strcmp( szType, "GU" ) == 0 && nLength >= 16 )
		{
			pPacket->Read( &m_pGUID, sizeof(GGUID) );
			m_bGUID = TRUE;
		}
		else if ( strcmp( szType, "V" ) == 0 && nLength >= 4 )
		{
			CHAR szVendor[5] = { 0, 0, 0, 0, 0 };
			pPacket->Read( szVendor, 4 );
			m_pVendor = VendorCache.Lookup( szVendor );
		}
		else if ( strcmp( szType, "LS" ) == 0 && nLength >= 8 )
		{
			m_nFileCount	= pPacket->ReadLongBE();
			m_nFileVolume	= pPacket->ReadLongBE();
		}
		else if ( strcmp( szType, "HS" ) == 0 && nLength >= 2 )
		{
			m_nLeafCount = pPacket->ReadShortBE();
			m_nLeafLimit = pPacket->ReadShortBE();
		}
		else if ( strcmp( szType, "QK" ) == 0 )
		{
			m_bCachedKeys = TRUE;
		}

		pPacket->m_nPosition = nNext;
	}

	if ( ! Network.IsFirewalledAddress( &m_pHost.sin_addr, TRUE ) &&
		   m_pVendor != NULL && m_nNodeType != ntLeaf )
	{
		HostCache.Gnutella2.Add( &m_pHost.sin_addr, htons( m_pHost.sin_port ),
			0, m_pVendor->m_sCode );
	}

	m_tWaitLNI = 0;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour KNOWN HUB LIST : send

void CG2Neighbour::SendKHL()
{
	CG2Packet* pPacket = CG2Packet::New( G2_PACKET_KHL, TRUE );

	DWORD nBase = pPacket->m_nPosition;

	for ( POSITION pos = Neighbours.GetIterator() ; pos ; )
	{
		CG2Neighbour* pNeighbour = (CG2Neighbour*)Neighbours.GetNext( pos );

		if (	pNeighbour != this &&
				pNeighbour->m_nProtocol == PROTOCOL_G2 &&
				pNeighbour->m_nState == nrsConnected &&
				pNeighbour->m_nNodeType != ntLeaf &&
				pNeighbour->m_pHost.sin_addr.S_un.S_addr != Network.m_pHost.sin_addr.S_un.S_addr )
		{
			if ( pNeighbour->m_pVendor && pNeighbour->m_pVendor->m_sCode.GetLength() == 4 )
			{
				pPacket->WritePacket( "NH", 14 + 6, TRUE );					// 4
				pPacket->WritePacket( "HS", 2 );							// 4
				pPacket->WriteShortBE( (WORD)pNeighbour->m_nLeafCount );	// 2
				pPacket->WritePacket( "V", 4 );								// 3
				pPacket->WriteString( pNeighbour->m_pVendor->m_sCode );		// 5
			}
			else
			{
				pPacket->WritePacket( "NH", 7 + 6, TRUE );					// 4
				pPacket->WritePacket( "HS", 2 );							// 4
				pPacket->WriteShortBE( (WORD)pNeighbour->m_nLeafCount );	// 2
				pPacket->WriteByte( 0 );									// 1
			}

			pPacket->WriteLongLE( pNeighbour->m_pHost.sin_addr.S_un.S_addr );	// 4
			pPacket->WriteShortBE( htons( pNeighbour->m_pHost.sin_port ) );		// 2
		}
	}

	int nCount = Settings.Gnutella2.KHLHubCount;
	DWORD tNow = time( NULL );

	pPacket->WritePacket( "TS", 4 );
	pPacket->WriteLongBE( time( NULL ) );

	for ( CHostCacheHost* pHost = HostCache.Gnutella2.GetNewest() ; pHost && nCount > 0 ; pHost = pHost->m_pPrevTime )
	{
		if (	pHost->CanQuote( tNow ) &&
				Neighbours.Get( &pHost->m_pAddress ) == NULL &&
				pHost->m_pAddress.S_un.S_addr != Network.m_pHost.sin_addr.S_un.S_addr )
		{
			int nLength = 10;

			if ( pHost->m_pVendor && pHost->m_pVendor->m_sCode.GetLength() == 4 )
				nLength += 7;
			if ( m_nNodeType == ntLeaf && pHost->m_nKeyValue != 0 && pHost->m_nKeyHost == Network.m_pHost.sin_addr.S_un.S_addr )
				nLength += 8;
			if ( nLength > 10 )
				nLength ++;

			pPacket->WritePacket( "CH", nLength, nLength > 10 );

			if ( pHost->m_pVendor && pHost->m_pVendor->m_sCode.GetLength() == 4 )
			{
				pPacket->WritePacket( "V", 4 );								// 3
				pPacket->WriteString( pHost->m_pVendor->m_sCode, FALSE );	// 4
			}

			if ( m_nNodeType == ntLeaf && pHost->m_nKeyValue != 0 && pHost->m_nKeyHost == Network.m_pHost.sin_addr.S_un.S_addr )
			{
				pPacket->WritePacket( "QK", 4 );							// 4
				pPacket->WriteLongBE( pHost->m_nKeyValue );					// 4
			}

			if ( nLength > 10 ) pPacket->WriteByte( 0 );					// 1

			pPacket->WriteLongLE( pHost->m_pAddress.S_un.S_addr );			// 4
			pPacket->WriteShortBE( pHost->m_nPort );						// 2
			pPacket->WriteLongBE( pHost->m_tSeen );							// 4

			nCount--;
		}
	}

	Send( pPacket, TRUE, TRUE );

	m_tLastKHL = GetTickCount();
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour KNOWN HUB LIST : receive

BOOL CG2Neighbour::OnKHL(CG2Packet* pPacket)
{
	if ( ! pPacket->m_bCompound ) return TRUE;

	CHAR szType[9], szInner[9];
	DWORD nLength, nInner;
	BOOL bCompound;

	DWORD tNow = time( NULL );

	m_pHubGroup->Clear();

	while ( pPacket->ReadPacket( szType, nLength, &bCompound ) )
	{
		DWORD nNext = pPacket->m_nPosition + nLength;

		if (	strcmp( szType, "NH" ) == 0 ||
				strcmp( szType, "CH" ) == 0 )
		{
			DWORD nAddress = 0, nKey = 0, tSeen = tNow;
			WORD nPort = 0, nLeafs = 0;
			CString strVendor;

			if ( bCompound || 0 == strcmp( szType, "NH" ) )
			{
				while ( pPacket->m_nPosition < nNext && pPacket->ReadPacket( szInner, nInner ) )
				{
					DWORD nNextX = pPacket->m_nPosition + nInner;

					if ( strcmp( szInner, "NA" ) == 0 && nInner >= 6 )
					{
						nAddress = pPacket->ReadLongLE();
						nPort = pPacket->ReadShortBE();
					}
					else if ( strcmp( szInner, "V" ) == 0 && nInner >= 4 )
					{
						strVendor = pPacket->ReadString( 4 );
					}
					else if ( strcmp( szInner, "QK" ) == 0 && nInner >= 4 )
					{
						nKey = pPacket->ReadLongBE();
						m_bCachedKeys = TRUE;
					}
					else if ( strcmp( szInner, "TS" ) == 0 && nInner >= 4 )
					{
						tSeen = pPacket->ReadLongBE() + m_tAdjust;
					}

					pPacket->m_nPosition = nNextX;
				}

				nLength = nNext - pPacket->m_nPosition;
			}

			if ( nLength >= 6 )
			{
				nAddress = pPacket->ReadLongLE();
				nPort = pPacket->ReadShortBE();
				if ( nLength >= 10 ) tSeen = pPacket->ReadLongBE() + m_tAdjust;
			}

			if ( FALSE == Network.IsFirewalledAddress( &nAddress, TRUE ) )
			{
				CHostCacheHost* pCached = HostCache.Gnutella2.Add(
					(IN_ADDR*)&nAddress, nPort, tSeen, strVendor );

				if ( pCached != NULL && m_nNodeType == ntHub )
				{
					if ( pCached->m_nKeyValue == 0 ||
						 pCached->m_nKeyHost != Network.m_pHost.sin_addr.S_un.S_addr )
					{
						pCached->SetKey( nKey, &m_pHost.sin_addr );
					}
				}

				if ( strcmp( szType, "NH" ) == 0 )
				{
					m_pHubGroup->Add( (IN_ADDR*)&nAddress, nPort );
				}
			}
		}
		else if ( strcmp( szType, "TS" ) == 0 && nLength >= 4 )
		{
			m_tAdjust = (LONG)tNow - (LONG)pPacket->ReadLongBE();
		}

		pPacket->m_nPosition = nNext;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour HUB ADVERTISEMENT WALKER : send

void CG2Neighbour::SendHAW()
{
	m_tLastHAW = GetTickCount();

	if ( m_nNodeType == ntLeaf || Neighbours.IsG2Leaf() ) return;

	CG2Packet* pPacket = CG2Packet::New( G2_PACKET_HAW, TRUE );

	WORD nLeafs = 0;
	GGUID pGUID;

	Network.CreateID( pGUID );

	for ( POSITION pos = Neighbours.GetIterator() ; pos ; )
	{
		CNeighbour* pNeighbour = Neighbours.GetNext( pos );

		if (	pNeighbour != this &&
				pNeighbour->m_nState == nrsConnected &&
				pNeighbour->m_nNodeType == ntLeaf )
		{
			nLeafs++;
		}
	}

	pPacket->WritePacket( "NA", 6 );
	pPacket->WriteLongLE( Network.m_pHost.sin_addr.S_un.S_addr );
	pPacket->WriteShortBE( htons( Network.m_pHost.sin_port ) );

	pPacket->WritePacket( "HS", 2 );
	pPacket->WriteShortBE( nLeafs );

	pPacket->WritePacket( "V", 4 );
	pPacket->WriteString( SHAREAZA_VENDOR_A );	// 5 bytes

	pPacket->WriteByte( 100 );
	pPacket->WriteByte( 0 );
	pPacket->Write( &pGUID, sizeof(GGUID) );

	Send( pPacket, TRUE, TRUE );

	m_pGUIDCache->Add( &pGUID, this );
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour HUB ADVERTISEMENT WALKER : receive

BOOL CG2Neighbour::OnHAW(CG2Packet* pPacket)
{
	if ( ! pPacket->m_bCompound ) return TRUE;

	CString strVendor;
	CHAR szType[9];
	DWORD nLength;

	DWORD nAddress	= 0;
	WORD nPort		= 0;

	while ( pPacket->ReadPacket( szType, nLength ) )
	{
		DWORD nNext = pPacket->m_nPosition + nLength;

		if ( strcmp( szType, "V" ) == 0 && nLength >= 4 )
		{
			strVendor = pPacket->ReadString( 4 );
		}
		else if ( strcmp( szType, "NA" ) == 0 && nLength >= 6 )
		{
			nAddress	= pPacket->ReadLongLE();
			nPort		= pPacket->ReadShortBE();
		}

		pPacket->m_nPosition = nNext;
	}

	if ( pPacket->GetRemaining() < 2 + 16 ) return TRUE;
	if ( nAddress == 0 || nPort == 0 ) return TRUE;
	if ( Network.IsFirewalledAddress( &nAddress, TRUE ) ) return TRUE;

	BYTE* pPtr	= pPacket->m_pBuffer + pPacket->m_nPosition;
	BYTE nTTL	= pPacket->ReadByte();
	BYTE nHops	= pPacket->ReadByte();

	GGUID pGUID;
	pPacket->Read( &pGUID, sizeof(GGUID) );

	HostCache.Gnutella2.Add( (IN_ADDR*)&nAddress, nPort, 0, strVendor );

	if ( nTTL > 0 && nHops < 255 )
	{
		m_pGUIDCache->Add( &pGUID, this );

		pPtr[0] = nTTL  - 1;
		pPtr[1] = nHops + 1;

		if ( CG2Neighbour* pNeighbour = Neighbours.GetRandomHub( this, &pGUID ) )
		{
			pNeighbour->Send( pPacket, FALSE, TRUE );
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour QUERY packet handler

BOOL CG2Neighbour::SendQuery(CQuerySearch* pSearch, CPacket* pPacket, BOOL bLocal)
{
	if ( m_nState != nrsConnected )
	{
		return FALSE;
	}
	else if ( pPacket == NULL || pPacket->m_nProtocol != PROTOCOL_G2 )
	{
		return FALSE;
	}
	else if ( m_nNodeType == ntHub && ! bLocal )
	{
		return FALSE;
	}
	else if ( m_pQueryTableRemote != NULL && m_pQueryTableRemote->m_bLive && ( m_nNodeType == ntLeaf || pSearch->m_bUDP ) )
	{
		if ( ! m_pQueryTableRemote->Check( pSearch ) ) return FALSE;
	}
	else if ( m_nNodeType == ntLeaf && ! bLocal )
	{
		return FALSE;
	}

	Send( pPacket, FALSE, ! bLocal );

	return TRUE;
}

BOOL CG2Neighbour::OnQuery(CG2Packet* pPacket)
{
	CQuerySearch* pSearch = CQuerySearch::FromPacket( pPacket );

	// Check for invalid / blocked searches
	if ( pSearch == NULL )
	{
		theApp.Message( MSG_DEFAULT, IDS_PROTOCOL_BAD_QUERY, (LPCTSTR)m_sAddress );
		Statistics.Current.Gnutella2.Dropped++;
		m_nDropCount++;
		return TRUE;
	}

	// Check for excessive source searching
	if ( ( pSearch->m_bSHA1 ) || ( pSearch->m_bBTH ) || ( pSearch->m_bED2K ) )
	{

		// Update allowed query operations, check for bad client
		if ( m_nQueryLimiter > -60 ) 
		{
			m_nQueryLimiter--;
		}
		else if ( ! m_bBlacklisted && ( m_nNodeType == ntLeaf ) )
		{
			// Abusive client
			m_bBlacklisted = TRUE;
			theApp.Message( MSG_SYSTEM, _T("Blacklisting %s due to excess traffic"), (LPCTSTR)m_sAddress );
			//Security.TempBlock( m_pHost.sin_addr );

		}

		if ( ( m_bBlacklisted ) || ( m_nQueryLimiter < 0 ) )
		{
			// Too many FMS operations
			if ( ! m_bBlacklisted )
				theApp.Message( MSG_DEBUG, _T("Dropping excess query traffic from %s"), (LPCTSTR)m_sAddress );

			delete pSearch;
			Statistics.Current.Gnutella2.Dropped++;
			m_nDropCount++;
			return TRUE;
		}
	}

	if ( m_nNodeType == ntLeaf && pSearch->m_bUDP &&
		 pSearch->m_pEndpoint.sin_addr.S_un.S_addr != m_pHost.sin_addr.S_un.S_addr )
	{
		delete pSearch;
		Statistics.Current.Gnutella2.Dropped++;
		m_nDropCount++;
		return TRUE;
	}

	if ( ! Network.QueryRoute->Add( &pSearch->m_pGUID, this ) )
	{
		delete pSearch;
		Statistics.Current.Gnutella2.Dropped++;
		m_nDropCount++;
		return TRUE;
	}

	if ( m_nNodeType != ntHub )
	{
		if ( pPacket->IsType( G2_PACKET_QUERY_WRAP ) )
		{
			if ( ! pPacket->SeekToWrapped() ) return TRUE;
			GNUTELLAPACKET* pG1 = (GNUTELLAPACKET*)( pPacket->m_pBuffer + pPacket->m_nPosition );

			if ( pG1->m_nTTL > 1 )
			{
				pG1->m_nTTL--;
				pG1->m_nHops++;
				Neighbours.RouteQuery( pSearch, pPacket, this, TRUE );
			}
		}
		else
		{
			Neighbours.RouteQuery( pSearch, pPacket, this, m_nNodeType == ntLeaf );
		}
	}

	Network.OnQuerySearch( pSearch );

	if ( pSearch->m_bUDP && /* Network.IsStable() && Datagrams.IsStable() && */
		 pSearch->m_pEndpoint.sin_addr.S_un.S_addr != m_pHost.sin_addr.S_un.S_addr )
	{
		CLocalSearch pLocal( pSearch, &pSearch->m_pEndpoint );
		pLocal.Execute();
	}
	else
	{
		BOOL bIsG1 = pPacket->IsType( G2_PACKET_QUERY_WRAP );

		if ( ! bIsG1 || Settings.Gnutella1.EnableToday )
		{
			CLocalSearch pLocal( pSearch, this, bIsG1 );
			pLocal.Execute();
		}
	}

	if ( m_nNodeType == ntLeaf ) Send( Neighbours.CreateQueryWeb( &pSearch->m_pGUID, this ), TRUE, FALSE );

	delete pSearch;
	Statistics.Current.Gnutella2.Queries++;

	return TRUE;
}

BOOL CG2Neighbour::OnQueryAck(CG2Packet* pPacket)
{
	HostCache.Gnutella2.Add( &m_pHost.sin_addr, htons( m_pHost.sin_port ) );
	SearchManager.OnQueryAck( pPacket, &m_pHost, NULL );
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour QUERY KEY REQUEST packet handler

BOOL CG2Neighbour::OnQueryKeyReq(CG2Packet* pPacket)
{
	if ( ! pPacket->m_bCompound ) return TRUE;
	if ( m_nNodeType != ntLeaf ) return TRUE;

	DWORD nLength, nAddress = 0;
	BOOL bCacheOkay = TRUE;
	CHAR szType[9];
	WORD nPort = 0;

	while ( pPacket->ReadPacket( szType, nLength ) )
	{
		DWORD nOffset = pPacket->m_nPosition + nLength;

		if ( strcmp( szType, "QNA" ) == 0 && nLength >= 6 )
		{
			nAddress	= pPacket->ReadLongLE();
			nPort		= pPacket->ReadShortBE();
		}
		else if ( strcmp( szType, "REF" ) == 0 )
		{
			bCacheOkay = FALSE;
		}

		pPacket->m_nPosition = nOffset;
	}

	if ( Network.IsFirewalledAddress( &nAddress, TRUE ) || 0 == nPort ) return TRUE;

	CHostCacheHost* pCached = bCacheOkay ? HostCache.Gnutella2.Find( (IN_ADDR*)&nAddress ) : NULL;

	if ( pCached != NULL && pCached->m_nKeyValue != 0 &&
		 pCached->m_nKeyHost == Network.m_pHost.sin_addr.S_un.S_addr )
	{
		CG2Packet* pAnswer = CG2Packet::New( G2_PACKET_QUERY_KEY_ANS, TRUE );
		pAnswer->WritePacket( "QNA", 6 );
		pAnswer->WriteLongLE( nAddress );
		pAnswer->WriteShortBE( nPort );
		pAnswer->WritePacket( "QK", 4 );
		pAnswer->WriteLongBE( pCached->m_nKeyValue );
		pAnswer->WritePacket( "CACHED", 0 );
		Send( pAnswer );
	}
	else
	{
		CG2Packet* pRequest = CG2Packet::New( G2_PACKET_QUERY_KEY_REQ, TRUE );
		pRequest->WritePacket( "SNA", 4 );
		pRequest->WriteLongLE( m_pHost.sin_addr.S_un.S_addr );
		Datagrams.Send( (IN_ADDR*)&nAddress, nPort, pRequest, TRUE, NULL, FALSE );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour QUERY KEY ANSWER packet handler

BOOL CG2Neighbour::OnQueryKeyAns(CG2Packet* pPacket)
{
	if ( ! pPacket->m_bCompound ) return TRUE;
	if ( m_nNodeType != ntHub ) return TRUE;

	DWORD nKey = 0, nAddress = 0;
	WORD nPort = 0;

	CHAR szType[9];
	DWORD nLength;

	while ( pPacket->ReadPacket( szType, nLength ) )
	{
		DWORD nOffset = pPacket->m_nPosition + nLength;

		if ( strcmp( szType, "QK" ) == 0 && nLength >= 4 )
		{
			nKey = pPacket->ReadLongBE();
		}
		else if ( strcmp( szType, "QNA" ) == 0 && nLength >= 6 )
		{
			nAddress	= pPacket->ReadLongLE();
			nPort		= pPacket->ReadShortBE();
		}
		else if ( strcmp( szType, "CACHED" ) == 0 )
		{
			m_bCachedKeys = TRUE;
		}

		pPacket->m_nPosition = nOffset;
	}

	theApp.Message( MSG_DEBUG, _T("Got a query key for %s:%i via neighbour %s: 0x%x"),
		(LPCTSTR)CString( inet_ntoa( *(IN_ADDR*)&nAddress ) ), nPort, (LPCTSTR)m_sAddress, nKey );

	if ( Network.IsFirewalledAddress( &nAddress ) || 0 == nPort ) return TRUE;

	CHostCacheHost* pCache = HostCache.Gnutella2.Add( (IN_ADDR*)&nAddress, nPort );
	if ( pCache != NULL ) pCache->SetKey( nKey, &m_pHost.sin_addr );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour PUSH packet handler

BOOL CG2Neighbour::OnPush(CG2Packet* pPacket)
{
	if ( ! pPacket->m_bCompound ) return TRUE;

	DWORD nLength = pPacket->GetRemaining();

	if ( ! pPacket->SkipCompound( nLength, 6 ) )
	{
		pPacket->Debug( _T("BadPush") );
		Statistics.Current.Gnutella2.Dropped++;
		return TRUE;
	}

	DWORD nAddress	= pPacket->ReadLongLE();
	WORD nPort		= pPacket->ReadShortBE();

	if ( Security.IsDenied( (IN_ADDR*)&nAddress ) )
	{
		Statistics.Current.Gnutella2.Dropped++;
		m_nDropCount++;
		return TRUE;
	}
	else if ( ! nPort || Network.IsFirewalledAddress( &nAddress ) )
	{
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_ZERO_PUSH, (LPCTSTR)m_sAddress );
		Statistics.Current.Gnutella2.Dropped++;
		m_nDropCount++;
		return TRUE;
	}

	Handshakes.PushTo( (IN_ADDR*)&nAddress, nPort );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour USER PROFILE CHALLENGE packet handler

BOOL CG2Neighbour::OnProfileChallenge(CG2Packet* pPacket)
{
	if ( ! MyProfile.IsValid() ) return TRUE;

	CG2Packet* pProfile = CG2Packet::New( G2_PACKET_PROFILE_DELIVERY, TRUE );

	CString strXML = MyProfile.GetXML( NULL, TRUE )->ToString( TRUE );

	pProfile->WritePacket( "XML", pProfile->GetStringLen( strXML ) );
	pProfile->WriteString( strXML, FALSE );

	Send( pProfile, TRUE, TRUE );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG2Neighbour USER PROFILE DELIVERY packet handler

BOOL CG2Neighbour::OnProfileDelivery(CG2Packet* pPacket)
{
	if ( ! pPacket->m_bCompound ) return TRUE;

	CHAR szType[9];
	DWORD nLength;

	while ( pPacket->ReadPacket( szType, nLength ) )
	{
		DWORD nOffset = pPacket->m_nPosition + nLength;

		if ( strcmp( szType, "XML" ) == 0 )
		{
			CXMLElement* pXML = CXMLElement::FromString( pPacket->ReadString( nLength ), TRUE );

			if ( pXML )
			{
				if ( m_pProfile == NULL ) m_pProfile = new CGProfile();
				if ( ! m_pProfile->FromXML( pXML ) ) delete pXML;
			}
		}

		pPacket->m_nPosition = nOffset;
	}

	return TRUE;
}
