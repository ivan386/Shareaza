//
// NeighboursWithED2K.cpp
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
#include "NeighboursWithED2K.h"
#include "EDNeighbour.h"
#include "EDPacket.h"
#include "Datagrams.h"
#include "Network.h"
#include "ED2K.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CNeighboursWithED2K construction

CNeighboursWithED2K::CNeighboursWithED2K()
{
	ZeroMemory( m_pEDSources, sizeof(MD4)   * 256 );
	ZeroMemory( m_tEDSources, sizeof(DWORD) * 256 );
}

CNeighboursWithED2K::~CNeighboursWithED2K()
{
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithED2K server lookup

CEDNeighbour* CNeighboursWithED2K::GetDonkeyServer() const
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CEDNeighbour* pNeighbour = (CEDNeighbour*)GetNext( pos );

		if ( pNeighbour->m_nProtocol == PROTOCOL_ED2K )
		{
			if ( pNeighbour->m_nState == nrsConnected &&
				 pNeighbour->m_nClientID != 0 )
			{
				return pNeighbour;
			}
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithED2K server lookup

void CNeighboursWithED2K::CloseDonkeys()
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CEDNeighbour* pNeighbour = (CEDNeighbour*)GetNext( pos );

		if ( pNeighbour->m_nProtocol == PROTOCOL_ED2K ) pNeighbour->Close();
	}
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithED2K advertise a new download

void CNeighboursWithED2K::SendDonkeyDownload(CDownload* pDownload)
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CEDNeighbour* pNeighbour = (CEDNeighbour*)GetNext( pos );

		if ( pNeighbour->m_nProtocol == PROTOCOL_ED2K )
		{
			pNeighbour->SendSharedDownload( pDownload );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithED2K server push

BOOL CNeighboursWithED2K::PushDonkey(DWORD nClientID, IN_ADDR* pServerAddress, WORD nServerPort)
{
	if ( ! Network.IsListening() ) return FALSE;

	CEDNeighbour* pNeighbour = (CEDNeighbour*)Get( pServerAddress );

	if ( pNeighbour != NULL && pNeighbour->m_nProtocol == PROTOCOL_ED2K )
	{
		CEDPacket* pPacket = CEDPacket::New( ED2K_C2S_CALLBACKREQUEST );
		pPacket->WriteLongLE( nClientID );
		pNeighbour->Send( pPacket );
		return TRUE;
	}

	/*
	lugdunum requests no more of this
	CEDPacket* pPacket = CEDPacket::New( ED2K_C2SG_CALLBACKREQUEST );
	pPacket->WriteLongLE( Network.m_pHost.sin_addr.S_un.S_addr );
	pPacket->WriteShortLE( htons( Network.m_pHost.sin_port ) );
	pPacket->WriteLongLE( nClientID );
	Datagrams.Send( pServerAddress, nServerPort + 4, pPacket );
	return TRUE;
	*/

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithED2K quick source lookup

BOOL CNeighboursWithED2K::FindDonkeySources(MD4* pED2K, IN_ADDR* pServerAddress, WORD nServerPort)
{
	if ( ! Network.IsListening() ) return FALSE;

	int nHash = (int)pServerAddress->S_un.S_un_b.s_b4 & 255;
	DWORD tNow = GetTickCount();

	if ( nHash < 0 ) nHash = 0;
	else if ( nHash > 255 ) nHash = 255;

	if ( m_pEDSources[ nHash ] == *pED2K )
	{
		if ( tNow - m_tEDSources[ nHash ] < 3600000 ) return FALSE;
	}
	else
	{
		if ( tNow - m_tEDSources[ nHash ] < 15000 ) return FALSE;
		m_pEDSources[ nHash ] = *pED2K;
	}

	m_tEDSources[ nHash ] = tNow;

	CEDPacket* pPacket = CEDPacket::New( ED2K_C2SG_GETSOURCES );
	pPacket->Write( pED2K, sizeof(MD4) );
	Datagrams.Send( pServerAddress, nServerPort + 4, pPacket );

	return TRUE;
}
