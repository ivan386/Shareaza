//
// NeighboursWithG1.cpp
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
#include "NeighboursWithG1.h"
#include "G1Neighbour.h"
#include "RouteCache.h"
#include "PongCache.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CNeighboursWithG1 construction

CNeighboursWithG1::CNeighboursWithG1()
{
	m_pPingRoute	= new CRouteCache();
	m_pPongCache	= new CPongCache();
}

CNeighboursWithG1::~CNeighboursWithG1()
{
	delete m_pPongCache;
	delete m_pPingRoute;
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithG1 connect

void CNeighboursWithG1::Connect()
{
	CNeighboursBase::Connect();
	m_pPingRoute->SetDuration( Settings.Gnutella.RouteCache );
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithG1 close

void CNeighboursWithG1::Close()
{
	CNeighboursBase::Close();
	
	m_pPingRoute->Clear();
	m_pPongCache->Clear();
}

void CNeighboursWithG1::Remove(CNeighbour* pNeighbour)
{
	m_pPingRoute->Remove( pNeighbour );
	CNeighboursBase::Remove( pNeighbour );
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithG1 G1 ping handler

void CNeighboursWithG1::OnG1Ping()
{
	if ( m_pPongCache->ClearIfOld() )
	{
		DWORD dwNow = GetTickCount();
		CGUID pGUID;
		
		Network.CreateID( &pGUID );
		
		for ( POSITION pos = GetIterator() ; pos ; )
		{
			CG1Neighbour* pNeighbour = (CG1Neighbour*)GetNext( pos );
			
			if ( pNeighbour->m_nProtocol == PROTOCOL_G1 && pNeighbour->m_bPongCaching )
			{
				pNeighbour->SendPing( dwNow, &pGUID );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithG1 G1 pong handler

void CNeighboursWithG1::OnG1Pong(CG1Neighbour* pFrom, IN_ADDR* pAddress, WORD nPort, BYTE nHops, DWORD nFiles, DWORD nVolume)
{
	CPongItem* pPongCache = m_pPongCache->Add( pFrom, pAddress, nPort, nHops, nFiles, nVolume );
	
	if ( pPongCache == NULL ) return;
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CG1Neighbour* pNeighbour = (CG1Neighbour*)GetNext( pos );
		
		if ( pNeighbour->m_nProtocol == PROTOCOL_G1 && pNeighbour != pFrom )
		{
			pNeighbour->OnNewPong( pPongCache );
		}
	}
}
