//
// NeighboursWithRouting.cpp
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
#include "Statistics.h"
#include "Network.h"
#include "Datagrams.h"
#include "Neighbour.h"
#include "NeighboursWithRouting.h"
#include "QuerySearch.h"
#include "G1Packet.h"
#include "G2Packet.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CNeighboursWithRouting construction

CNeighboursWithRouting::CNeighboursWithRouting()
{
}

CNeighboursWithRouting::~CNeighboursWithRouting()
{
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithRouting packet broadcasting

int CNeighboursWithRouting::Broadcast(CPacket* pPacket, CNeighbour* pExcept, BOOL bGGEP)
{
	CSingleLock pLock( &Network.m_pSection, TRUE );
	int nCount = 0;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CNeighbour* pNeighbour = GetNext( pos );
		
		if ( pNeighbour != pExcept && pNeighbour->m_nState == nrsConnected )
		{
			if ( pNeighbour->Send( pPacket, FALSE, TRUE ) ) nCount++;
		}
	}
	
	return nCount;
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithRouting query dispatch

int CNeighboursWithRouting::RouteQuery(CQuerySearch* pSearch, CPacket* pPacket, CNeighbour* pFrom, BOOL bToHubs)
{
	BOOL bHubLoop = FALSE;
	int nCount = 0;
	POSITION pos;
	
	CG1Packet* pG1		= ( pPacket->m_nProtocol == PROTOCOL_G1 ) ? (CG1Packet*)pPacket : NULL;
	CG2Packet* pG2		= ( pPacket->m_nProtocol == PROTOCOL_G2 ) ? (CG2Packet*)pPacket : NULL;
	CG2Packet* pG2Q1	= NULL;
	CG2Packet* pG2Q2	= NULL;
	
	ASSERT( pG1 || pG2 );
	
	if ( pG2 )
	{
		if ( pG2->IsType( "Q2" ) ) pG2Q2 = pG2; else pG2Q1 = pG2;
	}
	
	for ( pos = GetIterator() ; pos ; )
	{
		CNeighbour* pNeighbour = (CNeighbour*)GetNext( pos );
		
		if ( pNeighbour == pFrom ) continue;
		if ( pNeighbour->m_nState < nrsConnected ) continue;
		
		if ( pNeighbour->m_nProtocol == PROTOCOL_G1 )
		{
			if ( bToHubs || pNeighbour->m_nNodeType == ntLeaf )
			{
				if ( pG1 == NULL )
				{
					if ( pG2Q1 != NULL )
					{
						if ( ! pG2Q1->SeekToWrapped() ) break;
						pG1 = CG1Packet::New( (GNUTELLAPACKET*)( pG2Q1->m_pBuffer + pG2Q1->m_nPosition ) );
					}
					else
					{
						pG1 = pSearch->ToG1Packet();
					}
				}
				
				// Dont buffer G2 naturals
				if ( pNeighbour->SendQuery( pSearch, pG1, pG2Q2 != NULL ) ) nCount++;
				// if ( pNeighbour->SendQuery( pSearch, pG1, FALSE ) ) nCount++;
			}
		}
		else if ( pNeighbour->m_nProtocol == PROTOCOL_G2 )
		{
			if ( pNeighbour->m_nNodeType == ntLeaf )
			{
				if ( pG2 == NULL )
				{
					pG2 = pG2Q1 = CG2Packet::New( G2_PACKET_QUERY_WRAP, pG1, Settings.Gnutella1.TranslateTTL );
				}

				if ( pNeighbour->SendQuery( pSearch, pG2, FALSE ) ) nCount++;
			}
			else if ( bToHubs )
			{
				if ( pG2Q2 == NULL )
				{
					if ( pG2 == NULL )
					{
						pG2 = pG2Q1 = CG2Packet::New( G2_PACKET_QUERY_WRAP, pG1, Settings.Gnutella1.TranslateTTL );
					}
					
					if ( pNeighbour->SendQuery( pSearch, pG2, FALSE ) ) nCount++;
				}
				else
				{
					bHubLoop = TRUE;
				}
			}
		}
	}
	
	if ( bHubLoop )
	{
		if ( pSearch->m_bUDP == FALSE && Datagrams.IsStable() )
		{
			pG2 = pG2->Clone();
			if ( pG2Q2 != pPacket ) pG2Q2->Release();
			pG2Q2 = pG2;
			
			BYTE* pPtr = pG2->WriteGetPointer( 5 + 6, 0 );
			*pPtr++ = 0x50;
			*pPtr++ = 6;
			*pPtr++ = 'U';
			*pPtr++ = 'D';
			*pPtr++ = 'P';
			*pPtr++ = Network.m_pHost.sin_addr.S_un.S_un_b.s_b1;
			*pPtr++ = Network.m_pHost.sin_addr.S_un.S_un_b.s_b2;
			*pPtr++ = Network.m_pHost.sin_addr.S_un.S_un_b.s_b3;
			*pPtr++ = Network.m_pHost.sin_addr.S_un.S_un_b.s_b4;
			if ( pPacket->m_bBigEndian )
			{
				*pPtr++ = (BYTE)( Network.m_pHost.sin_port & 0xFF );
				*pPtr++ = (BYTE)( ( Network.m_pHost.sin_port >> 8 ) & 0xFF );
			}
			else
			{
				*pPtr++ = (BYTE)( ( Network.m_pHost.sin_port >> 8 ) & 0xFF );
				*pPtr++ = (BYTE)( Network.m_pHost.sin_port & 0xFF );
			}
		}
		
		for ( pos = GetIterator() ; pos ; )
		{
			CNeighbour* pNeighbour = (CNeighbour*)GetNext( pos );
			
			if (	pNeighbour != pFrom &&
					pNeighbour->m_nState >= nrsConnected &&
					pNeighbour->m_nProtocol == PROTOCOL_G2 &&
					pNeighbour->m_nNodeType != ntLeaf )
			{
				if ( pNeighbour->SendQuery( pSearch, pG2, FALSE ) ) nCount++;
			}
		}
	}
	
	if ( pG1 && pG1 != pPacket ) pG1->Release();
	if ( pG2 && pG2 != pPacket ) pG2->Release();
	
	if ( nCount )
	{
		if ( pPacket->m_nProtocol == PROTOCOL_G1 )
			Statistics.Current.Gnutella1.Routed++;
		else if ( pPacket->m_nProtocol == PROTOCOL_G2 )
			Statistics.Current.Gnutella2.Routed++;
	}
	
	return nCount;
}
