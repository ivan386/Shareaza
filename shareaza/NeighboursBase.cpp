//
// NeighboursBase.cpp
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
#include "NeighboursBase.h"
#include "Neighbour.h"
#include "RouteCache.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CNeighboursBase construction

CNeighboursBase::CNeighboursBase()
{
	m_nUnique		= 2;
	m_nRunCookie	= 5;
	m_nStableCount	= 0;
	m_nLeafCount	= 0;
	m_nLeafContent	= 0;
	m_nBandwidthIn	= 0;
	m_nBandwidthOut	= 0;
}

CNeighboursBase::~CNeighboursBase()
{
	CNeighboursBase::Close();
}

//////////////////////////////////////////////////////////////////////
// CNeighboursBase list access

POSITION CNeighboursBase::GetIterator() const
{
	return m_pUniques.GetStartPosition();
}

CNeighbour* CNeighboursBase::GetNext(POSITION& pos) const
{
	LPVOID nUnique;
	CNeighbour* pNeighbour;
	m_pUniques.GetNextAssoc( pos, nUnique, (void*&)pNeighbour );
	return pNeighbour;
}

//////////////////////////////////////////////////////////////////////
// CNeighboursBase lookup

CNeighbour* CNeighboursBase::Get(DWORD nUnique) const
{
	CNeighbour* pNeighbour = NULL;
	if ( ! m_pUniques.Lookup( (LPVOID)nUnique, (void*&)pNeighbour ) ) pNeighbour = NULL;
	return pNeighbour;
}

CNeighbour* CNeighboursBase::Get(IN_ADDR* pAddress) const
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CNeighbour* pNeighbour = GetNext( pos );
		
		if ( pNeighbour->m_pHost.sin_addr.S_un.S_addr == pAddress->S_un.S_addr )
			return pNeighbour;
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CNeighboursBase counting

int CNeighboursBase::GetCount(PROTOCOLID nProtocol, int nState, int nNodeType) const
{
	int nCount = 0;
	
	for ( POSITION pos = m_pUniques.GetStartPosition() ; pos ; )
	{
		CNeighbour* pNeighbour;
		LPVOID nUnique;
		
		m_pUniques.GetNextAssoc( pos, nUnique, (void*&)pNeighbour );
		
		if ( nProtocol < 0 || nProtocol == pNeighbour->m_nProtocol )
		{
			// Hack to count only Gnutella (No longer needed)
			//if ( nProtocol == -2 && pNeighbour->m_nProtocol > PROTOCOL_G2 ) continue;
			
			if ( nState < 0 || nState == pNeighbour->m_nState )
			{
				if ( nNodeType < 0 || nNodeType == pNeighbour->m_nNodeType )
				{
					nCount++;
				}
			}
		}
	}
	
	return nCount;
}

//////////////////////////////////////////////////////////////////////
// CNeighboursBase connect

void CNeighboursBase::Connect()
{
}

//////////////////////////////////////////////////////////////////////
// CNeighboursBase close

void CNeighboursBase::Close()
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		GetNext( pos )->Close();
	}
	
	m_nStableCount	= 0;
	m_nLeafCount	= 0;
	m_nLeafContent	= 0;
	m_nBandwidthIn	= 0;
	m_nBandwidthOut	= 0;
}

//////////////////////////////////////////////////////////////////////
// CNeighboursBase run callback

void CNeighboursBase::OnRun()
{
	DWORD tNow			= GetTickCount();
	DWORD tEstablish	= tNow - 1500;
	
	int nStableCount	= 0;
	int nLeafCount		= 0;
	DWORD nLeafContent	= 0;
	DWORD nBandwidthIn	= 0;
	DWORD nBandwidthOut	= 0;
	
	m_nRunCookie++;
	
	while ( TRUE )
	{
		Network.m_pSection.Lock();
		BOOL bWorked = FALSE;
		
		for ( POSITION pos = GetIterator() ; pos ; )
		{
			CNeighbour* pNeighbour = GetNext( pos );
			
			if ( pNeighbour->m_nRunCookie != m_nRunCookie )
			{
				pNeighbour->m_nRunCookie = m_nRunCookie;
				
				if ( pNeighbour->m_nState == nrsConnected &&
					 pNeighbour->m_tConnected < tEstablish )
					 nStableCount ++;
				
				if ( pNeighbour->m_nNodeType == ntLeaf )
				{
					nLeafCount ++;
					nLeafContent += pNeighbour->m_nFileVolume;
				}
				
				pNeighbour->Measure();
				
				nBandwidthIn	+= pNeighbour->m_mInput.nMeasure;
				nBandwidthOut	+= pNeighbour->m_mOutput.nMeasure;
				
				pNeighbour->DoRun();
				
				bWorked = TRUE;
				break;
			}
		}
		
		Network.m_pSection.Unlock();
		
		if ( ! bWorked ) break;
	}
	
	m_nStableCount	= nStableCount;
	m_nLeafCount	= nLeafCount;
	m_nLeafContent	= nLeafContent;
	m_nBandwidthIn	= nBandwidthIn;
	m_nBandwidthOut	= nBandwidthOut;
}

//////////////////////////////////////////////////////////////////////
// CNeighboursBase add and remove

void CNeighboursBase::Add(CNeighbour* pNeighbour, BOOL bAssignUnique)
{
	if ( bAssignUnique )
	{
		CNeighbour* pExisting = NULL;
		
		do
		{
			pNeighbour->m_nUnique = m_nUnique++;
		}
		while (	pNeighbour->m_nUnique < 2 ||
				m_pUniques.Lookup( (LPVOID)pNeighbour->m_nUnique, (void*&)pExisting ) );
	}
	
	m_pUniques.SetAt( (LPVOID)pNeighbour->m_nUnique, pNeighbour );
}

void CNeighboursBase::Remove(CNeighbour* pNeighbour)
{
	Network.QueryRoute->Remove( pNeighbour );
	Network.NodeRoute->Remove( pNeighbour );
	
	m_pUniques.RemoveKey( (LPVOID)pNeighbour->m_nUnique );
}

