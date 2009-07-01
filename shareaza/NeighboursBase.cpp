//
// NeighboursBase.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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

// Keeps a list of CNeighbour objects, with methods to go through them, add and remove them, and count them
// http://shareazasecurity.be/wiki/index.php?title=Developers.Code.CNeighboursBase

// Copy in the contents of these files here before compiling
#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "Network.h"
#include "NeighboursBase.h"
#include "Neighbour.h"
#include "RouteCache.h"

// If we are compiling in debug mode, replace the text "THIS_FILE" in the code with the name of this file
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CNeighboursBase construction

CNeighboursBase::CNeighboursBase()
{
	m_nRunCookie    = 5; // Start the run cookie as 5, OnRun will make it 6, 7, 8 and so on
	m_nStableCount  = 0; // We don't have any stable connections to remote computers yet
	m_nLeafCount    = 0; // We aren't connected to any leaves yet
	m_nLeafContent  = 0; // When we do have some leaf connetions, the total size of the files they are sharing will go here
	m_nBandwidthIn  = 0; // Start the bandwidth totals at 0
	m_nBandwidthOut = 0;
}

CNeighboursBase::~CNeighboursBase()
{
	CNeighboursBase::Close();
}

//////////////////////////////////////////////////////////////////////
// CNeighboursBase list access

POSITION CNeighboursBase::GetIterator() const
{
	ASSUME_LOCK( Network.m_pSection );

	return m_pNeighbours.GetHeadPosition();
}

CNeighbour* CNeighboursBase::GetNext(POSITION& pos) const
{
	ASSUME_LOCK( Network.m_pSection );

	return m_pNeighbours.GetNext( pos );
}

//////////////////////////////////////////////////////////////////////
// CNeighboursBase lookup

CNeighbour* CNeighboursBase::Get(DWORD_PTR nUnique) const
{
	ASSUME_LOCK( Network.m_pSection );

	if ( POSITION pos = m_pNeighbours.Find( (CNeighbour*)nUnique ) )
		return m_pNeighbours.GetAt( pos );

	return NULL;
}

CNeighbour* CNeighboursBase::Get(IN_ADDR* pAddress) const
{
	ASSUME_LOCK( Network.m_pSection );

	for ( POSITION pos = m_pNeighbours.GetHeadPosition() ; pos ; )
	{
		CNeighbour* pNeighbour = m_pNeighbours.GetNext( pos );

		// If this neighbour object has the IP address we are looking for, return it
		if ( pNeighbour->m_pHost.sin_addr.S_un.S_addr == pAddress->S_un.S_addr )
			return pNeighbour;
	}

	return NULL;
}

// Finds the newest neighbour object
CNeighbour* CNeighboursBase::GetNewest(PROTOCOLID nProtocol, int nState, int nNodeType) const
{
	ASSUME_LOCK( Network.m_pSection );

	DWORD tCurrent = GetTickCount();
	DWORD tMinTime = 0xffffffff;
	CNeighbour* pMinNeighbour = NULL;
	for ( POSITION pos = m_pNeighbours.GetHeadPosition() ; pos ; )
	{
		CNeighbour* pNeighbour = m_pNeighbours.GetNext( pos );
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
	DWORD nCount = 0;

	CSingleLock pLock( &Network.m_pSection, FALSE );
	if ( pLock.Lock( 200 ) )
	{
		for ( POSITION pos = m_pNeighbours.GetHeadPosition() ; pos ; )
		{
			CNeighbour* pNeighbour = m_pNeighbours.GetNext( pos );

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
	}
	return nCount;
}

// NeighbourExists is faster than GetCount, use it if you don't care how many there are, you just want to know if there are any
// Takes a protocol, like Gnutella, a state, like connecting, and a node connection type, like we are both ultrapeers
// Determines if there is at least 1 neighbour in the list that matches these criteria
BOOL CNeighboursBase::NeighbourExists(PROTOCOLID nProtocol, int nState, int nNodeType) const
{
	CSingleLock pLock( &Network.m_pSection, FALSE );
	if ( pLock.Lock( 200 ) )
	{
		for ( POSITION pos = m_pNeighbours.GetHeadPosition() ; pos ; )
		{
			CNeighbour* pNeighbour = m_pNeighbours.GetNext( pos );

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
}

//////////////////////////////////////////////////////////////////////
// CNeighboursBase connect

void CNeighboursBase::Connect()
{
}

//////////////////////////////////////////////////////////////////////
// CNeighboursBase close

// Calls Close on all the neighbours in the list, and resets member variables back to 0
void CNeighboursBase::Close()
{
	for ( POSITION pos = m_pNeighbours.GetHeadPosition() ; pos ; )
	{
		m_pNeighbours.GetNext( pos )->Close();
	}

	// Reset member variables back to 0
	m_nStableCount  = 0;
	m_nLeafCount    = 0;
	m_nLeafContent  = 0;
	m_nBandwidthIn  = 0;
	m_nBandwidthOut = 0;
}

//////////////////////////////////////////////////////////////////////
// CNeighboursBase run callback

// The program calls OnRun on a regular interval
// Calls DoRun on neighbours in the list, and totals statistics from them
void CNeighboursBase::OnRun()
{
	// Calculate time from now
	DWORD tNow       = GetTickCount(); // The tick count now, the number of milliseconds since the user turned the computer on
	DWORD tEstablish = tNow - 1500;    // The tick count 1.5 seconds ago

	// Make local variables to mirror member variables, and start them all out as 0
	int nStableCount    = 0;
	int nLeafCount      = 0;
	DWORD nLeafContent  = 0;
	DWORD nBandwidthIn  = 0;
	DWORD nBandwidthOut = 0;

	// Have the loop test each neighbour's run cookie count against the next number
	m_nRunCookie++;			// The first time this runs, it will take the value from 5 to 6
	bool bUpdated = true;	// Indicate if stats were updated

	// Loop until all updates have been processed
	while ( bUpdated )
	{
		// Make sure this thread is the only one accessing the network object
		CSingleLock pLock( &Network.m_pSection, FALSE );
		if ( ! pLock.Lock( 200 ) )
			continue;

		// Indicate if stats were updated
		bUpdated = false;

		// Loop through the neighbours in the list
		for ( POSITION pos = m_pNeighbours.GetHeadPosition() ; pos ; )
		{
			// Get the neighbour at this position, and move pos to the next position in the m_pUniques map
			CNeighbour* pNeighbour = m_pNeighbours.GetNext( pos );

			// If this neighbour doesn't have the new run cookie count yet, we need to run it
			if ( pNeighbour->m_nRunCookie != m_nRunCookie )
			{
				// Give it the current run cookie count so we don't run it twice, even if GetNext is weird or broken
				pNeighbour->m_nRunCookie = m_nRunCookie;

				// If the socket has been open for 1.5 seconds and we've finished the handshake, count this neighbour as stable
				if ( pNeighbour->m_nState == nrsConnected && // This neighbour has finished the handshake with us, and
					 pNeighbour->m_tConnected < tEstablish ) // Our socket to it connected before 1.5 seconds ago
					 nStableCount++;                         // Count this as a stable connection

				// If this connection is down to a leaf
				if ( pNeighbour->m_nNodeType == ntLeaf )
				{
					// Count the leaf and add the number of bytes it's sharing to the total we're keeping of that
					nLeafCount++;                              // Count one more leaf
					nLeafContent += pNeighbour->m_nFileVolume; // Add the number of bytes of files this leaf is sharing to the total
				}

				// Have this neighbour measure its bandwidth, compute the totals, and then we'll save them in the member variables
				pNeighbour->Measure(); // Calls CConnection::Measure, which edits the values in the TCP bandwidth meter structure
				nBandwidthIn   += pNeighbour->m_mInput.nMeasure;  // We started these out as 0, and will save the totals in the CNeighbours object
				nBandwidthOut  += pNeighbour->m_mOutput.nMeasure;

				// Send and receive data with this remote computer through the socket
				pNeighbour->DoRun(); // Calls CConnection::DoRun

				// We found a neighbour with a nonmatching run cookie count, updated it, and processed it
				// Defer any other updates until next run through the loop, allowing the network object to be unlocked
				bUpdated = true;	// Set bUpdated to true
				break;				// Break out of the for loop
			}
		}
	}

	// Save the values we calculated in local variables into the corresponding member variables
	m_nStableCount  = nStableCount;
	m_nLeafCount    = nLeafCount;
	m_nLeafContent  = nLeafContent;
	m_nBandwidthIn  = nBandwidthIn;
	m_nBandwidthOut = nBandwidthOut;
}

//////////////////////////////////////////////////////////////////////
// CNeighboursBase add and remove

// Takes a neighbour object, and true to find it an unused m_nUnique number
// Adds it to the m_pUniques map using pNeighbour->m_nUnique as the key
void CNeighboursBase::Add(CNeighbour* pNeighbour)
{
	ASSUME_LOCK( Network.m_pSection );

	if ( POSITION pos = m_pNeighbours.Find( pNeighbour ) )
	{
		// Dup?
	}
	else
	{
		m_pNeighbours.AddTail( pNeighbour );
	}
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
	if ( POSITION pos = m_pNeighbours.Find( pNeighbour ) )
	{
		m_pNeighbours.RemoveAt( pos );
	}
}
