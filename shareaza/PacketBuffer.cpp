//
// PacketBuffer.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

// CG1PacketBuffer holds arrays of packets to send, organized by their type
// http://shareazasecurity.be/wiki/index.php?title=Developers.Code.CG1PacketBuffer

// Copy in the contents of these files here before compiling
#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "PacketBuffer.h"
#include "G1Packet.h"
#include "Neighbours.h"
#include "Buffer.h"
#include "Statistics.h"

// If we are compiling in debug mode, replace the text "THIS_FILE" in the code with the name of this file
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CG1PacketBuffer construction

// Takes access to a buffer to write packets to directly instead of keeping them in this new object
// Makes a new CG1PacketBuffer object that holds 9 arrays of 64 packets each, one array for each packet type
CG1PacketBuffer::CG1PacketBuffer(CBuffer* pBuffer)
{
	// Make an array of 9 new CG1PacketBufferType objects, each of which are an array of 64 packets
	m_pType = new CG1PacketBufferType[ G1_PACKTYPE_MAX ]; // One array for each packet type, like ping or pong

	// Save given pointer to a buffer in the object
	m_pBuffer  = pBuffer;

	// Initalize values
	m_nTotal   = 0; // The total number of packets we've added to empty spots in arrays
	m_nCycle   = 1; // What kind of packet to send next, like 1 ping, 2 pong, and so on
	m_nIterate = 0; // Start out the count of packets of a type we've sent as 0
	m_nDropped = 0; // The total number of packets we've overwritten in the arrays because of lack of room
}

// Delete this CG1PacketBuffer object
CG1PacketBuffer::~CG1PacketBuffer()
{
	// Tell all the packets in the arrays for each type that we're not linked to them from here anymore
	Clear();

	// Delete the array of 9 CG1PacketBufferType objects
	delete [] m_pType;
}

//////////////////////////////////////////////////////////////////////
// CG1PacketBuffer add

// Takes a Gnutella packet and true to put it in the buffer, false to put it in the arrays
void CG1PacketBuffer::Add(CG1Packet* pPacket, BOOL bBuffered)
{
	// If the packet isn't buffered yet, or doesn't have a type like ping or pong yet
	if ( ! bBuffered || ! pPacket->m_nTypeIndex )
	{
		// Write the packet directly into the buffer, and we're done
		pPacket->ToBuffer( m_pBuffer );
		return;
	}

	// If the packet is already buffered, or it has a known type like ping or pong, add it to the appropriate array
	BOOL bFresh =                 // Add will tell us if it added the packet in an empty spot, or overwrote a packet
		m_pType                   // The array of packet arrays for each type
		[ pPacket->m_nTypeIndex ] // The type of the packet, like ping or pong, brings us to the array for that type
		.Add( pPacket );          // Add the packet to that array

	// Add had an empty spot in the array for the new packet
	if ( bFresh )
	{
		// Add the added packet to the total
		m_nTotal++;

	} // Add had to overwrite a packet because there are already 64 of that type there
	else
	{
		// Count the packet that was overwritten as dropped and lost
		m_nDropped++;
		Statistics.Current.Gnutella1.Lost++; // Our count of how many packets we lost before being able to send
	}
}

//////////////////////////////////////////////////////////////////////
// CG1PacketBuffer packet selection

// Takes an expiration time
// Gets a packet to send, choosing one added recently and of a different type than the one before
// Returns the packet
CG1Packet* CG1PacketBuffer::GetPacketToSend(DWORD dwExpire)
{
	// Let the loop send as many as
	static int nVolumeByType[ G1_PACKTYPE_MAX ] = {
		0,   // 0 unknown packets at a time
		1,   // 1 pign packet at a time
		2,   // 2 pong packets at a time
		5,   // 5 bye packets at a time
		1,   // 1 query route packet at a time
		1,   // 1 vendor specific packet at a time
		5,   // 5 push packets at a time
		1,   // 1 query packet at a time
		4 }; // 4 query hit packets at a time

	// Loop
	for (

		// Start nCycle as 18
		int nCycle = G1_PACKTYPE_MAX * 2; // The local variable nCycle isn't used in the loop, just up here

		// Stop if nCycle is 0
		nCycle; // This would mean we've looped 18 times

		// After each loop, do all of the following
		nCycle--,        // nCycle will be like 18, 17, 16, ... 2, 1, and then 0 will break the loop
		m_nCycle++,      // Move to the next packet type array, like from 1 ping to 2 pong and so on
		m_nIterate = 0 ) // Reset the count of packets of this type we've sent to 0
	{
		// Cycle from ping 1, to pong 2, to bye 3, and so on
		m_nCycle %= G1_PACKTYPE_MAX; // Divide m_nCycle by 9 and take the remainder, which is 0 through 8
		if ( ! m_nCycle ) continue;  // If that produced 0 unknown packet type, skip the rest of the code in the loop and loop again

		// If we've sent more packets of this type than the list of limits above allows, skip the rest of the loop code and loop again
		if ( m_nIterate >= nVolumeByType[ m_nCycle ] ) continue; // How could m_nIterate ever get above 1? (do)

		// Get the most recently added and not expired packet of the type determined by cycle
		CG1Packet* pPacket =    // Get a pointer to a packet
			m_pType[ m_nCycle ] // The array of 64 packets of m_nCycle type, like 1 ping first, then 2 pong, and so on
			.Get(               // Get the most recently added and not expired packet from that array
				dwExpire,       // Tell Get the expiration time
				&m_nTotal,      // Get will update the total number of packets added in empty slots and dropped here
				&m_nDropped );

		// If we got a packet
		if ( pPacket )
		{
			// Increment iterate and return it
			m_nIterate++;   // Count that we've found a packet and the program will send it
			return pPacket; // Return the packet to the caller so it can send it
		}
	}

	// We don't have any more packets to send
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CG1PacketBuffer clear

// Clears all the packets from all the arrays
void CG1PacketBuffer::Clear()
{
	// Loop through the 9 arrays of 64 packets, one array for each packet type
	for ( int nType = 0 ; nType < G1_PACKTYPE_MAX ; nType++ )
	{
		// Clear this array
		m_pType[ nType ].Clear();
	}

	// Reset the count of how many packets we've added in empty array spots
	m_nTotal = 0;
}

//////////////////////////////////////////////////////////////////////
// CG1PacketBufferType construction

// Create a new array of packet pointers
CG1PacketBufferType::CG1PacketBufferType()
{
	// Set the number of Gnutella packets this CG1PacketBufferType object will point to from the program settings
	m_nCapacity	= Settings.Gnutella1.PacketBufferSize; // By default, it's 64

	// Make the arrays of packet pointers and times
	m_pBuffer = new CG1Packet*[ m_nCapacity ]; // Allocate an array of 64 pointers to Gnutella packets, and point m_pBuffer at this array
	m_pTime   = new DWORD[ m_nCapacity ];      // Make an array of 64 DWORDs to record the time each packet will expire

	// Start packet indices and counts at 0
	m_nHead  = 0; // This is the index where we will add a packet to the array, initialize to 0 even though we'll add them back to front
	m_nCount = 0; // We haven't added any packets to the array yet
}

// Delete this array of packet pointers
CG1PacketBufferType::~CG1PacketBufferType()
{
	// Clear all the packet pointers from the array, and reset the count and head values to 0
	Clear(); // Tells each packet that we're not pointing to it anymore

	// Free the memory of the pointer and DWORD arrays we allocated in the constructor
	delete [] m_pTime;
	delete [] m_pBuffer;
}

//////////////////////////////////////////////////////////////////////
// CG1PacketBufferType add

// Takes a pointer to a Gnutella packet
// Adds it and its expiration time to the arrays inside this CG1PacketBufferType object, going from end to start
// Returns false if the array was full and we overwrote a packet, true if we're still on the first sweep backwards
BOOL CG1PacketBufferType::Add(CG1Packet* pPacket)
{
	// Assume that there is room for one more packet
	BOOL bFresh = TRUE;

	// If the arrays are full, we'll put this packet in the last place again and fill backwards from there again
	if ( m_nCount == m_nCapacity ) // Count and capacity are both 64
	{
		// Record there is one fewer packet in the array
		m_nCount--; // Now count is 63

		// Call Release on the last packet in the buffer, then the one before that, then (do)
		m_pBuffer[                 // Look at an index in the buffer of arrays we are about to calculate
			( m_nHead + m_nCount ) // Head + count are usually 64, but we just decremented count so now they're 63
			% m_nCapacity ]        // Divide by 64 and take the remainder, which is 63
			->Release();           // Release the last packet in the array, the one at index 63

		// Note that no, there wasn't room for another packet, but we made room so it's OK
		bFresh = FALSE;
	}

	// Count head down each time this line runs like 63, 62, 61 ... 3, 2, 1, 0, 63, 62, 61 ... to sweep backwards down the array
	if ( ! m_nHead-- ) m_nHead += m_nCapacity; // If head is 0, add capacity to it, and also decrement it either way

	// Record there will be one more packet stored here
	m_nCount++; // If we are overwriting packets now, this will set the count back up to 64

	// Store a pointer to the packet and the time a minute from now when it will expire at the head index of the arrays
	m_pBuffer[ m_nHead ] = pPacket;          // Store the pointer to the packet
	m_pTime[ m_nHead ] =                     // A minute from now, the packet we're adding will expire
		GetTickCount() +                     // The tick count now
		Settings.Gnutella1.PacketBufferTime; // By default, this is 1 minute

	// Have the packet count one more pointer saved to it
	pPacket->AddRef();

	// Return true if we did that without making more room, or false if we did it after making more room
	return bFresh;
}

// Takes an expiration time, and access to 2 integers to write statistics
// Gets the packet we added most recently that hasn't expired, and moves head forward and decrements count
// Returns a pointer to it
CG1Packet* CG1PacketBufferType::Get(DWORD dwExpire, int* pnTotal, int* pnDropped)
{
	// Local variables
	CG1Packet* pPacket; // A pointer to a packet copied from the array of packet pointers
	DWORD dwPacket;     // The expiration time of that packet

	// Loop until we find a packet that hasn't expired
	do
	{
		// If there aren't any packet pointers in the array, we won't be able to find this one
		if ( ! m_nCount ) return NULL; // Not found

		// Get the packet pointer and time we most recently added to the arrays
		pPacket  = m_pBuffer[ m_nHead ]; // The pointer the Add method added the last time it ran
		dwPacket = m_pTime[ m_nHead ];   // The tick count 1 minute after it ran

		// Move forward in the array to the next most recently added packet
		m_nHead = ( m_nHead + 1 ) % m_nCapacity; // Add 1 to head unless it is 63, then take it back to 0

		// This packet is either expired, or will be returned, so we can record 1 less packet in the array
		m_nCount--;

		// This packet has expired
		if ( dwPacket < dwExpire ) // This packet's expiration date happened before the given expiration date
		{
			// Adjust the numbers the caller gave us access to
			if ( pnTotal )   (*pnTotal)--;   // This is one less packet total
			if ( pnDropped ) (*pnDropped)++; // This is one more packet dropped

			// Count this in statistics as a lost packet
			Statistics.Current.Gnutella1.Lost++;

			// Tell the packet we're no longer pointing to it from here
			pPacket->Release();
			pPacket = NULL; // Stay in the do while loop
		}
	}
	while ( pPacket == NULL ); // If the if statement didn't run, exit the do while loop

	// Return the pointer to the first packet we found that hasn't expired
	return pPacket;
}

// Clears all the packet pointers from the array, and resets the count and head values to 0
void CG1PacketBufferType::Clear()
{
	// If count is 5, loop for 5, 4, 3, 2, and 1, set count to 0, and don't enter the loop
	while ( m_nCount-- > 0 ) // Enter the loop if count is positive, and then decrement the count
	{
		// Tell each packet we're not pointing to it anymore
		m_pBuffer[ ( m_nHead + m_nCount ) % m_nCapacity ]->Release();
	}

	// Count is 0, reset head to that value also
	m_nHead = 0;
}

