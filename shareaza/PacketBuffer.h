//
// PacketBuffer.h
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

// Make the compiler only include the lines here once, this is the same thing as pragma once
#if !defined(AFX_PACKETBUFFER_H__7FE2F4C5_B0E8_444C_9B26_C95CCB344615__INCLUDED_)
#define AFX_PACKETBUFFER_H__7FE2F4C5_B0E8_444C_9B26_C95CCB344615__INCLUDED_

// Only include the lines beneath this one once
#pragma once

// Tell the compiler these classes exist, and it will find out more about them soon
class CG1Packet;
class CBuffer;
class CG1PacketBufferType;

// Holds 9 arrays of 64 Gnutella packets each, one array for each packet type, like ping or pong
class CG1PacketBuffer
{

public:

	// Make a new set of arrays, and delete this set
	CG1PacketBuffer(CBuffer* pBuffer); // Takes a buffer to write packets to instead of putting them in the arrays
	virtual ~CG1PacketBuffer();

public:

	// Total counts of packets in the arrays
	int m_nTotal;   // The number of packets Add added without overwriting any
	int m_nDropped; // The number of packets Add overwrote

protected:

	// The buffer where we can write packets directly, instead of putting them in arrays
	CBuffer* m_pBuffer;

	// Arrays of packets, one for each type
	int                  m_nCycle;   // The type of packet to send next, like 1 ping array, 2 pong array, and so on
	int                  m_nIterate; // The number of packets of this type we've recently sent
	CG1PacketBufferType* m_pType;    // An array of 9 pointers to arrays of 64 packets each, one array for each packet type

public:

	// Add a packet, and clear them all
	void Add(CG1Packet* pPacket, BOOL bBuffered = TRUE); // Add a packet to the array of its type
	void Clear();                                        // Clear all the packets from all the arrays

	// Get a packet to send, it chooses one added recently, not expired, and keeps the type mixed up
	CG1Packet* GetPacketToSend(DWORD dwExpire = 0);
};

// Holds an array of 64 pointers to packets to send, all of one type, and the time each expires 1 minute after they were added
class CG1PacketBufferType
{

public:

	// Create a new packet buffer type object, and delete this one
	CG1PacketBufferType();
	virtual ~CG1PacketBufferType();

protected:

	// Two arrays that hold information about 64 Gnutella packets
	CG1Packet** m_pBuffer; // This is a pointer to an array of 64 pointers to Gnutella packets, and has nothing to do with a CBuffer object
	DWORD*      m_pTime;   // The time each packet will expire, which is 1 minute after it is added

	// Indices and counts for those arrays
	int m_nHead;     // The index in the array where we added information about a packet most recently
	int m_nCount;    // The number of spots in the array filled with a packet pointer and time
	int m_nCapacity; // The size of the arrays we allocated from settings, 64 by default

public:

	// Add a packet to the array, get one out, and clear them all
	BOOL       Add(CG1Packet* pPacket);
	CG1Packet* Get(DWORD dwExpire = 0, int* pnTotal = NULL, int* pnDropped = NULL);
	void       Clear();
};

// End the group of lines to only include once, pragma once doesn't require an endif at the bottom
#endif // !defined(AFX_PACKETBUFFER_H__7FE2F4C5_B0E8_444C_9B26_C95CCB344615__INCLUDED_)
