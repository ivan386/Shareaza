//
// Neighbours.cpp
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
#include "Neighbours.h"
#include "EDNeighbour.h"
#include "EDPacket.h"
#include "GProfile.h"
#include "Network.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CNeighbours Neighbours;

CNeighbours::CNeighbours()
{
}

CNeighbours::~CNeighbours()
{
}

CString CNeighbours::GetName(const CNeighbour* pNeighbour) const
{
	if ( pNeighbour->m_nProtocol == PROTOCOL_G1 )
	{
		switch ( pNeighbour->m_nNodeType )
		{
		case ntNode:
			return LoadString( IDS_NEIGHBOUR_G1PEER );

		case ntHub:
			return LoadString( IDS_NEIGHBOUR_G1ULTRA );

		case ntLeaf:
			return LoadString( IDS_NEIGHBOUR_G1LEAF );
		}
	}
	else if ( pNeighbour->m_nProtocol == PROTOCOL_G2 )
	{
		switch ( pNeighbour->m_nNodeType )
		{
		case ntNode:
			return LoadString( IDS_NEIGHBOUR_G2PEER );

		case ntHub:
			return LoadString( IDS_NEIGHBOUR_G2HUB );

		case ntLeaf:
			return LoadString( IDS_NEIGHBOUR_G2LEAF );
		}
	}

	return protocolNames[ pNeighbour->m_nProtocol ];
}

CString CNeighbours::GetAgent(const CNeighbour* pNeighbour) const
{
	if ( pNeighbour->m_nProtocol == PROTOCOL_ED2K )
	{
		const CEDNeighbour* pED2K = static_cast< const CEDNeighbour* >( pNeighbour );

		if ( pED2K->m_nClientID )
			return LoadString( CEDPacket::IsLowID( pED2K->m_nClientID ) ?
				IDS_NEIGHBOUR_ED2K_LOWID : IDS_NEIGHBOUR_ED2K_HIGHID );
		else
			return LoadString( IDS_NEIGHBOUR_ED2K_SERVER );
	}

	return pNeighbour->m_sUserAgent;
}

CString CNeighbours::GetNick(const CNeighbour* pNeighbour) const
{
	return pNeighbour->m_pProfile ? pNeighbour->m_pProfile->GetNick() : pNeighbour->m_sServerName;
}

CString CNeighbours::GetNeighbourList(LPCTSTR szFormat) const
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 100 ) )
		return CString();

	CString strOutput;
	DWORD tNow = GetTickCount();

	for ( POSITION pos = Neighbours.GetIterator() ; pos ; )
	{
		const CNeighbour* pNeighbour = Neighbours.GetNext( pos );

		if ( pNeighbour->m_nState == nrsConnected )
		{
			CString strNode;

			DWORD nTime = ( tNow - pNeighbour->m_tConnected ) / 1000;

			strNode.Format( szFormat,
				(LPCTSTR)pNeighbour->m_sAddress, htons( pNeighbour->m_pHost.sin_port ),
				(LPCTSTR)pNeighbour->m_sAddress, htons( pNeighbour->m_pHost.sin_port ),
				nTime / 3600, ( nTime % 3600 ) / 60, nTime % 60,
				(LPCTSTR)Neighbours.GetName( pNeighbour ),
				(LPCTSTR)Neighbours.GetAgent( pNeighbour ),
				(LPCTSTR)pNeighbour->m_sAddress, htons( pNeighbour->m_pHost.sin_port ) );

			strOutput += strNode;
		}
	}

	return strOutput;
}
