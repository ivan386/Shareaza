//
// NeighboursWithG2.cpp
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
#include "NeighboursWithG2.h"
#include "G2Neighbour.h"
#include "G2Packet.h"
#include "HubHorizon.h"
#include "RouteCache.h"
#include "HostCache.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CNeighboursWithG2 construction

CNeighboursWithG2::CNeighboursWithG2()
{
}

CNeighboursWithG2::~CNeighboursWithG2()
{
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithG2 connect

void CNeighboursWithG2::Connect()
{
	CNeighboursWithG1::Connect();
	HubHorizonPool.Setup();
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithG2 create query web packet

CG2Packet* CNeighboursWithG2::CreateQueryWeb(GGUID* pGUID, CNeighbour* pExcept)
{
	CG2Packet* pPacket = CG2Packet::New( G2_PACKET_QUERY_ACK, TRUE );
	
	DWORD tNow = time( NULL );
	
	pPacket->WritePacket( "TS", 4 );
	pPacket->WriteLongBE( tNow );
	
	theApp.Message( MSG_DEBUG, _T("Creating a query acknowledgement:") );
	
	pPacket->WritePacket( "D", 8 );
	pPacket->WriteLongLE( Network.m_pHost.sin_addr.S_un.S_addr );
	pPacket->WriteShortBE( htons( Network.m_pHost.sin_port ) );
	pPacket->WriteShortBE( GetCount( PROTOCOL_G2, nrsConnected, ntLeaf ) );
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CG2Neighbour* pNeighbour = (CG2Neighbour*)GetNext( pos );

		if (	pNeighbour->m_nProtocol == PROTOCOL_G2 &&
				pNeighbour->m_nNodeType != ntLeaf &&
				pNeighbour->m_nState >= nrsConnected &&
				pNeighbour != pExcept )
		{
			pPacket->WritePacket( "D", 8 );
			pPacket->WriteLongLE( pNeighbour->m_pHost.sin_addr.S_un.S_addr );
			pPacket->WriteShortBE( htons( pNeighbour->m_pHost.sin_port ) );
			pPacket->WriteShortBE( (WORD)pNeighbour->m_nLeafCount );
			
			theApp.Message( MSG_DEBUG, _T("  Done neighbour %s"),
				(LPCTSTR)pNeighbour->m_sAddress );
		}
	}
	
	int nCount = ( pExcept == NULL ) ? 3 : 25;
	
	for ( CHostCacheHost* pHost = HostCache.Gnutella2.GetNewest() ; pHost ; pHost = pHost->m_pPrevTime )
	{
		if ( pHost->CanQuote( tNow ) &&
			 Get( &pHost->m_pAddress ) == NULL && 
			 HubHorizonPool.Find( &pHost->m_pAddress ) == NULL )
		{
			pPacket->WritePacket( "S", 10 );
			pPacket->WriteLongLE( pHost->m_pAddress.S_un.S_addr );
			pPacket->WriteShortBE( pHost->m_nPort );
			pPacket->WriteLongBE( pHost->m_tSeen );
			
			theApp.Message( MSG_DEBUG, _T("  Try cached hub %s"),
				(LPCTSTR)CString( inet_ntoa( pHost->m_pAddress ) ) );
			
			if ( ! --nCount ) break;
		}
	}
	
	HubHorizonPool.AddHorizonHubs( pPacket );
	
	pPacket->WriteByte( 0 );
	pPacket->Write( pGUID, sizeof(GGUID) );
	
	return pPacket;
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithG2 random hub selector

CG2Neighbour* CNeighboursWithG2::GetRandomHub(CG2Neighbour* pExcept, GGUID* pGUID)
{
	CPtrArray pRandom;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CG2Neighbour* pNeighbour = (CG2Neighbour*)GetNext( pos );
		
		if (	pNeighbour->m_nState == nrsConnected &&
				pNeighbour->m_nProtocol == PROTOCOL_G2 &&
				pNeighbour->m_nNodeType != ntLeaf &&
				pNeighbour != pExcept )
		{
			if ( pNeighbour->m_pGUIDCache->Lookup( pGUID ) == NULL )
			{
				pRandom.Add( pNeighbour );
			}
		}
	}
	
	int nSize = pRandom.GetSize();
	if ( ! nSize ) return NULL;
	
	nSize = rand() % nSize;
	
	return (CG2Neighbour*)pRandom.GetAt( nSize );
}

