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
// http://wiki.shareaza.com/static/Developers.Code.CNeighboursBase

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

// When the program makes the CNeighbours object, this constructor initializes the member variables defined in CNeighboursBase
CNeighboursBase::CNeighboursBase()
{
	m_nUnique       = 2; // Give the first neighbour which needs a unique number the number 2, the one after that will get 3, and so on
	m_nRunCookie    = 5; // Start the run cookie as 5, OnRun will make it 6, 7, 8 and so on
	m_nStableCount  = 0; // We don't have any stable connections to remote computers yet
	m_nLeafCount    = 0; // We aren't connected to any leaves yet
	m_nLeafContent  = 0; // When we do have some leaf connetions, the total size of the files they are sharing will go here
	m_nBandwidthIn  = 0; // Start the bandwidth totals at 0
	m_nBandwidthOut = 0;
}

// When the program deletes the CNeighbours object, each destructor runs in succession
CNeighboursBase::~CNeighboursBase()
{
	// Call the Close method on each neighbour object in the list, and reset member variables back to 0
	CNeighboursBase::Close();
}

//////////////////////////////////////////////////////////////////////
// CNeighboursBase list access

// Call to get a POSITION you can use with GetNextAssoc to loop through the contents of m_pUniques
// Calls GetStartPosition on the MFC CMapPtrToPtr collection class
// Returns a POSITION value, or null if the map is empty
POSITION CNeighboursBase::GetIterator() const
{
	// Return the POSITION that GetStartPosition gets from the m_pUniques map
	return m_pUniques.GetStartPosition(); // Returns a POSITION value that we can use to loop through the contents
}

// Call repeatedly to loop through the contents of the m_pUniques map
// Takes access to the POSITION value that GetIterator returned
// Calls GetNextAssoc to get the next thing in the map
// Casts it back to a CNeighbour object, and returns a pointer to it
CNeighbour* CNeighboursBase::GetNext(POSITION& pos) const
{
	// Local variables for the key and value under pos
	DWORD_PTR   nUnique;    // The key, a pointer
	CNeighbour* pNeighbour; // The value, a pointer to an object that inherits from CNeighbour

	// Retrieves the map element at pos, and then updates pos to refer to the next element in the map
	m_pUniques.GetNextAssoc(
		pos,                  // A reference to the POSITION value that is used, and then updated
		nUnique,              // GetNextAssoc writes the key of the element here, we don't need it
		pNeighbour );		  // GetNextAssoc writes the value of the element here

	// Return the pointer to the object that inherits from CNeighbour that we got from the m_pUniques map
	return pNeighbour;
}

//////////////////////////////////////////////////////////////////////
// CNeighboursBase lookup

// Takes a key value as a DWORD (do)
// Finds the CNeighbour object that was entered into m_pUniques with that value
// Returns it, or null if not found
CNeighbour* CNeighboursBase::Get(DWORD_PTR nUnique) const
{
	// Make a local pointer to a CNeighbour object and start it out pointing at null
	CNeighbour* pNeighbour = NULL;

	// Lookup the CNeighbour object with that value in the m_pUniques map
	if ( ! m_pUniques.Lookup( // If Lookup can't find the element, it will return 0, and the if will execute
		nUnique,      // Cast the DWORD as a pointer and use it as the key value
		              // This is a reference to a pointer, letting Lookup change where the pointer points
		pNeighbour ) )        // Lookup will write the value pointer it finds in pNeighbour

		// Lookup couldn't find the element, make sure the local pointer we return will be null
		pNeighbour = NULL;

	// Return a pointer to the CNeighbours object Lookup found, or null if not found
	return pNeighbour;
}

// Takes an IP address
// Finds the neighbour object in the m_pUniques map that represents the remote computer with that address
// Returns it, or null if not found
CNeighbour* CNeighboursBase::Get(IN_ADDR* pAddress) const // Saying const here means this method won't change any member variables
{
	// Loop through each neighbour in the m_pUniques map
	for ( POSITION pos = GetIterator() ; pos ; ) // Get a POSITION iterator, and leave when calling GetNext made it null
	{
		// Get the neighbour object at the current position, and move pos to the next position
		CNeighbour* pNeighbour = GetNext( pos );

		// If this neighbour object has the IP address we are looking for, return it
		if ( pNeighbour->m_pHost.sin_addr.S_un.S_addr == pAddress->S_un.S_addr ) return pNeighbour;
	}

	// None of the neighbour objects in the map had the IP address we are looking for
	return NULL; // Not found
}

// Finds the newest neighbour object
// Returns it, or null if not found
CNeighbour* CNeighboursBase::GetNewest(PROTOCOLID nProtocol, int nState, int nNodeType) const
{
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
// Returns the number found
DWORD CNeighboursBase::GetCount(PROTOCOLID nProtocol, int nState, int nNodeType) const
{
	// Start out the count at 0
	DWORD nCount = 0;

	// Get exclusive access to the network object (do)
	if ( ( Network.m_pSection.Lock( 200 ) ) ) // If we're waiting more than a fifth of a second for it, give up
	{
		// Loop through each neighbour in the m_pUniques map
		for ( POSITION pos = m_pUniques.GetStartPosition() ; pos ; ) // Get a POSITION iterator, and leave when calling GetNext made it null
		{
			// Get a key and value from the m_pUniques map
			CNeighbour* pNeighbour;   // GetNextAssoc will put the neighbour pointer here
			DWORD_PTR nUnique;        // GetNextAssoc will put the key to that neighbour here
			m_pUniques.GetNextAssoc(  // Get a key and value from the map, and move pos to the next one for the next time this runs
				pos,                  // Get the neighbour at this position, and move pos to the next one
				nUnique,              // GetNextAssoc will write the key here
				pNeighbour ); // And the corresponding neighbour pointer here

			// If this neighbour has the protocol we are looking for, or nProtocl is negative to count them all
			if ( nProtocol == PROTOCOL_ANY || nProtocol == pNeighbour->m_nProtocol )
			{
				// If this neighbour is currently in the state we are looking for, or nState is negative to count them all
				if ( nState < 0 || nState == pNeighbour->m_nState )
				{
					// If this neighbour is in the ultra or leaf role we are looking for, or nNodeType is null to count them all
					if ( nNodeType < 0 || nNodeType == pNeighbour->m_nNodeType )
					{
						// Count it
						nCount++;
					}
				}
			}
		}

		// Let another thread gain exclusive access to the network object (do)
		Network.m_pSection.Unlock();
	}

	// Return the number of neighbours we found in the list that match the given qualities
	return nCount;
}

// NeighbourExists is faster than GetCount, use it if you don't care how many there are, you just want to know if there are any
// Takes a protocol, like Gnutella, a state, like connecting, and a node connection type, like we are both ultrapeers
// Determines if there is at least 1 neighbour in the list that matches these criteria
// Returns true if it finds one
BOOL CNeighboursBase::NeighbourExists(PROTOCOLID nProtocol, int nState, int nNodeType) const
{
	// Get exclusive access to the network object, giving up after a fifth of a second of waiting
	CSingleLock pLock( &Network.m_pSection );
	if ( pLock.Lock( 200 ) ) return FALSE;

	// Loop through each neighbour in the m_pUniques map
	for ( POSITION pos = m_pUniques.GetStartPosition() ; pos ; )
	{
		// Get a key and value from the m_pUniques map
		CNeighbour* pNeighbour;   // GetNextAssoc will put the neighbour pointer here
		DWORD_PTR nUnique;        // GetNextAssoc will put the key to that neighbour here
		m_pUniques.GetNextAssoc(  // Get a key and value from the map, and move pos to the next one for the next time this runs
			pos,                  // Get the neighbour at this position, and move pos to the next one
			nUnique,              // GetNextAssoc will write the key here
			pNeighbour ); // And the corresponding neighbour pointer here

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

	// Not found
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CNeighboursBase connect

// Both CNeighboursWithG1 and CNeighboursWithG2 have Connect methods that do something
void CNeighboursBase::Connect()
{
	// Do nothing
}

//////////////////////////////////////////////////////////////////////
// CNeighboursBase close

// Calls Close on all the neighbours in the list, and resets member variables back to 0
void CNeighboursBase::Close()
{
	// Loop through each neighbour in the list
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		// Call the close method on it
		GetNext( pos )->Close(); // GetNext gets the neighbour at the current position, and then moves pos forward
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
		Network.m_pSection.Lock();

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

		// We're done with the network object for right now, let another thread access it
		Network.m_pSection.Unlock();
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
void CNeighboursBase::Add(CNeighbour* pNeighbour, BOOL bAssignUnique)
{
	// The caller wants us to assign this neighbour a unique number for the map
	if ( bAssignUnique )
	{
		// Start out pExisting as null, nothing found yet
		CNeighbour* pExisting = NULL;

		// Assign this neighbour a number like 2, 3, 4 and so on that no neighbour in the list has already
		do
		{
			// Give the number in this CNeighbours object to this neighbour
			pNeighbour->m_nUnique = m_nUnique++; // Then, move to the next number for the next neighbour

		} // Look for this number in the map already, and break if it's not found
		while (

			// This neighbour has a unique number less than 2, or
			pNeighbour->m_nUnique < 2 // Not really possible, because we just set it to 2, 3, 4 or so on (do)

			// Lookup can find this number in use as a key in the map already
			|| m_pUniques.Lookup(              // Given a key, find the corresponding value in the map
				pNeighbour->m_nUnique, // The key is this neighbour's unique number
				pExisting ) );         // If there is a value for that key, Lookup will write it in pExisting
	}

	// Now that we've given this neighbour a unique key, insert it with that key into the map
	m_pUniques.SetAt(                  // Insert an element into the map
		pNeighbour->m_nUnique, // Under this key
		pNeighbour );                  // Put this element
}

// Takes a pointer to a neighbour object
// Removes it from the list
void CNeighboursBase::Remove(CNeighbour* pNeighbour)
{
	// Tell the network object to remove this neighbour also
	Network.QueryRoute->Remove( pNeighbour );
	Network.NodeRoute->Remove( pNeighbour );

	// Remove the neighbour object from the map
	m_pUniques.RemoveKey( pNeighbour->m_nUnique ); // Remove it by its key
}
