//
// NeighboursBase.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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
	: m_nRunCookie    ( 5 ) // Start the run cookie as 5, OnRun will make it 6, 7, 8 and so on
{
	m_pNeighbours.InitHashTable( GetBestHashTableSize( 300 ) );
	m_pIndex.InitHashTable( GetBestHashTableSize( 300 ) );
}

CNeighboursBase::~CNeighboursBase()
{
	ASSERT( m_pNeighbours.IsEmpty() );
	ASSERT( m_pIndex.IsEmpty() );
}

//////////////////////////////////////////////////////////////////////
// CNeighboursBase list access

POSITION CNeighboursBase::GetIterator() const
{
	ASSUME_LOCK( Network.m_pSection );

	return m_pIndex.GetStartPosition();
}

CNeighbour* CNeighboursBase::GetNext(POSITION& pos) const
{
	ASSUME_LOCK( Network.m_pSection );

	DWORD_PTR nKey = 0;
	CNeighbour* pNeighbour = NULL;
	m_pIndex.GetNextAssoc( pos, nKey, pNeighbour );
	return pNeighbour;
}

//////////////////////////////////////////////////////////////////////
// CNeighboursBase lookup

CNeighbour* CNeighboursBase::Get(DWORD_PTR nUnique) const
{
	ASSUME_LOCK( Network.m_pSection );

	CNeighbour* pNeighbour;
	if ( m_pIndex.Lookup( nUnique, pNeighbour ) )
		return pNeighbour;

	return NULL;
}

CNeighbour* CNeighboursBase::Get(const IN_ADDR& pAddress) const
{
	ASSUME_LOCK( Network.m_pSection );

	CNeighbour* pNeighbour;
	if ( m_pNeighbours.Lookup( pAddress, pNeighbour ) )
		return pNeighbour;

	return NULL;
}

// Finds the newest neighbour object
CNeighbour* CNeighboursBase::GetNewest(PROTOCOLID nProtocol, int nState, int nNodeType) const
{
	ASSUME_LOCK( Network.m_pSection );

	DWORD tCurrent = GetTickCount();
	DWORD tMinTime = 0xffffffff;
	CNeighbour* pMinNeighbour = NULL;
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CNeighbour* pNeighbour = GetNext( pos );
		if ( ( nProtocol == PROTOCOL_ANY || nProtocol == pNeighbour->m_nProtocol ) &&
			 ( nState < 0 || nState == pNeighbour->m_nState ) &&
			 ( nNodeType < 0 || nNodeType == pNeighbour->m_nNodeType ) )
		{
			DWORD tTime = tCurrent - pNeighbour->m_tConnected;
			if ( tTime < tMinTime )
			{
				tMinTime = tTime;
				pMinNeighbour = pNeighbour;
			}
		}
	}
	return pMinNeighbour;
}

//////////////////////////////////////////////////////////////////////
// CNeighboursBase counting

// Takes a protocol, like Gnutella, a state, like connecting, and a node connection type, like we are both ultrapeers
// Counts the number of neighbours in the list that match these criteria, pass -1 to count them all
DWORD CNeighboursBase::GetCount(PROTOCOLID nProtocol, int nState, int nNodeType) const
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 100 ) )
		return 0;

	DWORD nCount = 0;
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		const CNeighbour* pNeighbour = GetNext( pos );

		// If this neighbour has the protocol we are looking for, or nProtocl is negative to count them all
		if ( nProtocol == PROTOCOL_ANY || nProtocol == pNeighbour->m_nProtocol )
		{
			// If this neighbour is currently in the state we are looking for, or nState is negative to count them all
			if ( nState < 0 || nState == pNeighbour->m_nState )
			{
				// If this neighbour is in the ultra or leaf role we are looking for, or nNodeType is null to count them all
				if ( nNodeType < 0 || nNodeType == pNeighbour->m_nNodeType )
				{
					nCount++;
				}
			}
		}
	}
	return nCount;
}

// NeighbourExists is faster than GetCount, use it if you don't care how many there are, you just want to know if there are any
// Takes a protocol, like Gnutella, a state, like connecting, and a node connection type, like we are both ultrapeers
// Determines if there is at least 1 neighbour in the list that matches these criteria
/*BOOL CNeighboursBase::NeighbourExists(PROTOCOLID nProtocol, int nState, int nNodeType) const
{
	CSingleLock pLock( &Network.m_pSection, FALSE );
	if ( pLock.Lock( 200 ) )
	{
		for ( POSITION pos = GetIterator() ; pos ; )
		{
			CNeighbour* pNeighbour = GetNext( pos );

			// If this neighbour has the protocol we are looking for, or nProtocl is negative to count them all
			if ( nProtocol == PROTOCOL_ANY || nProtocol == pNeighbour->m_nProtocol )
			{
				// If this neighbour is currently in the state we are looking for, or nState is negative to count them all
				if ( nState < 0 || nState == pNeighbour->m_nState )
				{
					// If this neighbour is in the ultra or leaf role we are looking for, or nNodeType is null to count them all
					if ( nNodeType < 0 || nNodeType == pNeighbour->m_nNodeType )
					{
						// Found one, return immediately
						return TRUE;
					}
				}
			}
		}
	}
	// Not found
	return FALSE;
}*/

//////////////////////////////////////////////////////////////////////
// CNeighboursBase connect

void CNeighboursBase::Connect()
{
	ASSUME_LOCK( Network.m_pSection );
}

//////////////////////////////////////////////////////////////////////
// CNeighboursBase close

// Calls Close on all the neighbours in the list, and resets member variables back to 0
void CNeighboursBase::Close()
{
	ASSUME_LOCK( Network.m_pSection );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		GetNext( pos )->Close();
	}
}

//////////////////////////////////////////////////////////////////////
// CNeighboursBase run callback

// The program calls OnRun on a regular interval
// Calls DoRun on neighbours in the list, and totals statistics from them
void CNeighboursBase::OnRun()
{
	// Spend here no more than 100 ms at once
	DWORD nStop = GetTickCount() + 100;

	// Have the loop test each neighbour's run cookie count against the next number
	m_nRunCookie++;			// The first time this runs, it will take the value from 5 to 6
	bool bUpdated = true;	// Indicate if stats were updated

	// Loop until all updates have been processed
	while ( bUpdated && GetTickCount() < nStop )
	{
		// Make sure this thread is the only one accessing the network object
		CSingleLock pLock( &Network.m_pSection );
		if ( ! pLock.Lock( 100 ) )
			continue;

		// Indicate if stats were updated
		bUpdated = false;

		// Loop through the neighbours in the list
		for ( POSITION pos = GetIterator() ; pos ; )
		{
			// Get the neighbour at this position, and move pos to the next position in the m_pUniques map
			CNeighbour* pNeighbour = GetNext( pos );

			// If this neighbour doesn't have the new run cookie count yet, we need to run it
			if ( pNeighbour->m_nRunCookie != m_nRunCookie )
			{
				// Give it the current run cookie count so we don't run it twice, even if GetNext is weird or broken
				pNeighbour->m_nRunCookie = m_nRunCookie;

				// Send and receive data with this remote computer through the socket
				pNeighbour->DoRun(); // Calls CConnection::DoRun

				// We found a neighbour with a nonmatching run cookie count, updated it, and processed it
				// Defer any other updates until next run through the loop, allowing the network object to be unlocked
				bUpdated = true;	// Set bUpdated to true
				break;				// Break out of the for loop
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CNeighboursBase add and remove

// Takes a neighbour object, and true to find it an unused m_nUnique number
// Adds it to the m_pUniques map using pNeighbour->m_nUnique as the key
void CNeighboursBase::Add(CNeighbour* pNeighbour)
{
	ASSUME_LOCK( Network.m_pSection );
	ASSERT( pNeighbour->m_pHost.sin_addr.s_addr != INADDR_ANY );
	ASSERT( pNeighbour->m_pHost.sin_addr.s_addr != INADDR_NONE );
	ASSERT( Get( pNeighbour->m_pHost.sin_addr ) == NULL );
	ASSERT( Get( (DWORD_PTR)pNeighbour ) == NULL );

	m_pNeighbours.SetAt( pNeighbour->m_pHost.sin_addr, pNeighbour );
	m_pIndex.SetAt( (DWORD_PTR)pNeighbour, pNeighbour );
}

// Takes a pointer to a neighbour object
// Removes it from the list
void CNeighboursBase::Remove(CNeighbour* pNeighbour)
{
	ASSUME_LOCK( Network.m_pSection );

	// Tell the network object to remove this neighbour also
	Network.QueryRoute->Remove( pNeighbour );
	Network.NodeRoute->Remove( pNeighbour );

	// Remove the neighbour object from the map
	for ( POSITION pos = m_pNeighbours.GetStartPosition(); pos; )
	{
		CNeighbour* pCurNeighbour = NULL;
		IN_ADDR nCurAddress = {};
		m_pNeighbours.GetNextAssoc( pos, nCurAddress, pCurNeighbour );
		if ( pNeighbour == pCurNeighbour )
		{
			VERIFY( m_pNeighbours.RemoveKey( nCurAddress ) );
			break;
		}
	}
	VERIFY( m_pIndex.RemoveKey( (DWORD_PTR)pNeighbour ) );
}
