//
// G1Packet.h
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

// CG1Packet represents a Gnutella packet, and CG1PacketPool keeps lists of them
// http://shareazasecurity.be/wiki/index.php?title=Developers.Code.CG1Packet

#pragma once

#include "Packet.h"
#include "GGEP.h"

#pragma pack(1)

// We can cast a pointer as a GNUTELLAPACKET structure to easily read the parts of the Gnutella packet header
typedef struct
{
	// These are the parts of a Gnutella packet header, in the right order, with each part the right size
	Hashes::Guid::RawStorage m_oGUID; // At  0, length 16, the globally unique identifier of this packet
	BYTE  m_nType;   // At 16, the byte that identifies what kind of packet this is, like ping or pong
	BYTE  m_nTTL;    // At 17, the number of hops this packet can travel across the Internet from here
	BYTE  m_nHops;   // At 18, the number of hops this packet has traveled across the Internet to get here
	LONG  m_nLength; // At 19, length 4, for a total size 23 bytes, the length of the packet payload

} GNUTELLAPACKET;

// Each CG1Packet object represents a received or preparing to send Gnutella packet
class CG1Packet : public CPacket // Inherit from CPacket to get memory management, and methods to read 
								 // and write ASCII text, bytes, and DWORDs
{

protected:

	// Make a new CG1Packet object, and delete this one
	CG1Packet();
	virtual ~CG1Packet(); // Why is this virtual, it's at the top of the inheritance tree (do)

public:

	// Data in the packet
	Hashes::Guid m_oGUID; // The globally unique identifier of this packet
	BYTE  m_nType; // The type of this packet, like ping or pong
	BYTE  m_nTTL;  // The number of hops this packet can travel across the Internet from here
	BYTE  m_nHops; // The number of hops this packet has travelled across the Internet to get here

	// Data about the packet
	int   m_nTypeIndex; // Packet type like ping or pong, except as an enumeration this program defines instead of the byte code 
						// used by the packet itself
	DWORD m_nHash;      // Used by CacheHash, but doesn't seem to ever get a hash written into it (do)

public:

	// Change the packet's TTL and hop counts
	BOOL Hop(); // Make sure the TTL is 2 or more, and then make it one less and the hops count one more

	// Hash the packet
	void         CacheHash();                                       // Calculate a simple hash of the packet payload in m_nHash
    // ????????????????????????????????? redefinition of default Parameter!!!
	virtual BOOL GetRazaHash(Hashes::Sha1Hash& oHash, DWORD nLength = 0) const; // Compute the SHA hash of the packet GUID, 
																				// type byte, and payload

	// Get the packet's type, GUID, and all its bytes
	virtual CString GetType()                  const; // Returns a pointer to a text literal like "Ping" or "Pong"
	CString         GetGUID()                  const; // Returns the packet's GUID encoded into text in base 16

	virtual void	Reset();
	virtual void    ToBuffer(CBuffer* pBuffer, bool bTCP = true) const; // Adds the Gnutella packet header and payload into the given CBuffer object

	// Record information about the packet for debugging purposes
	virtual void Debug(LPCTSTR pszReason) const; // Writes debug information about the packet into the Shareaza.log file

public:

	// Convert between the various ways the program expresses packet types, like ping and pong
	static int     GnutellaTypeToIndex(BYTE nType); // Turn a type byte, like 0x30, into index 4, both describe a query route packet
	static LPCTSTR m_pszPackets[9];                 // Turn a type index, like 4, into text like "QRP" for query route packet

	// Read IP/IPP/DIP/DIPP hosts from GGEP and add to cache.
	// Returns amount of successfully added or updated hosts and -1 on errors.
	static int GGEPReadCachedHosts(const CGGEPBlock& pGGEP);

	// Received SCP GGEP, send 5 random hosts from the cache
	// Since we do not provide leaves, ignore the preference data
	static int GGEPWriteRandomCache(CGGEPItem* pItem);

	// Is Out of Band queries enabled?
	static bool IsOOBEnabled();

	// Is we firewalled in terms of Gnutella 1?
	static bool IsFirewalled();

protected:

	// Create a nested class, CG1PacketPool, that holds arrays of Gnutella packets we can use quickly
	class CG1PacketPool : public CPacketPool // Inherit from CPacketPool to get methods to create arrays of 
											 // packets and break them off for speedy use
	{

	public:

		// Delete this CG1PacketPool object
		virtual ~CG1PacketPool() { Clear(); } // Call the Clear method to free all the arrays of packets

	protected:

		// Create a new array of packets, and free one
		virtual void NewPoolImpl(int nSize, CPacket*& pPool, int& nPitch); // Allocate a new array of 256 packets
		virtual void FreePoolImpl(CPacket* pPool);                         // Free an array of 256 packets
	};

	// Separate from objects made from this CG1Packet class, allow a single CG1PacketPool called POOL to be made
	static CG1PacketPool POOL;

public:

	// Get a new packet from the global packet pool called POOL, fill it with these values, and return a pointer to it
	static CG1Packet* New(int nType = 0, DWORD nTTL = 0, const Hashes::Guid& oGUID = Hashes::Guid());

	// Takes a Gnutella packet header structure
	// Gets a new packet from the pool and fills it with values from the header structure
	// Returns a pointer to the prepared packet in the pool
	inline static CG1Packet* New(const GNUTELLAPACKET* pSource)
	{
		// Get a blank packet from the pool
		CG1Packet* pPacket = (CG1Packet*)POOL.New();

		// Fill it with information from the given Gnutella packet header structure
		pPacket->m_oGUID = pSource->m_oGUID;
		pPacket->m_nType = pSource->m_nType;
		pPacket->m_nTTL  = pSource->m_nTTL;
		pPacket->m_nHops = pSource->m_nHops;

		// Also record the type as an index
		pPacket->m_nTypeIndex = GnutellaTypeToIndex( pPacket->m_nType );

		// Copy the bytes of the payload from beyond the gnutella packet structure into the buffer of the packet object
		pPacket->Write(                  // Have the packet write these bytes into its buffer for the packet payload
			&pSource[1],                 // The 1 moves forward 1 structure size, to the bytes beyond the structure
			(DWORD)pSource->m_nLength ); // The number of bytes there is the payload size according to the header structure

		// Return a pointer to the packet, sitting in the pool, filled with the given header values and payload
		return pPacket;
	}

	// Delete this packet
	inline virtual void Delete()
	{
		// Tell the pool to delete this packet
		POOL.Delete( this ); // All it will really do is link it back into the list of packets we can use later
	}

	// Packet handler
	virtual BOOL OnPacket(const SOCKADDR_IN* pHost);

protected:
	BOOL OnPing(const SOCKADDR_IN* pHost);
	BOOL OnPong(const SOCKADDR_IN* pHost);
	BOOL OnVendor(const SOCKADDR_IN* pHost);

	// Let the nested CG1PacketPool class access the private members of this CG1Packet class
	friend class CG1Packet::CG1PacketPool;

private:
	CG1Packet(const CG1Packet&);
	CG1Packet& operator=(const CG1Packet&);
};

// Takes nSize, the number of CG1Packet objects we want
// Sets nPitch to the size of each one, and points pPool at a new array of that many of them
inline void CG1Packet::CG1PacketPool::NewPoolImpl(int nSize, CPacket*& pPool, int& nPitch)
{
	// Set nPitch to the size in bytes of each CG1Packet object
	nPitch = sizeof(CG1Packet);

	// Allocate a new array of nSize CG1Packet objects, and point pPool at it
	pPool = new CG1Packet[ nSize ];
}

// Takes a pointer to an array of packets, which is called a packet pool
// Deletes the packet pool, freeing the memory of all the packets in it
inline void CG1Packet::CG1PacketPool::FreePoolImpl(CPacket* pPacket)
{
	// Delete the array of packets
	delete [] (CG1Packet*)pPacket;
}

// Those are all the structures we need special alignment for
#pragma pack() // Same as pragma pack(pop)

// Gnutella packet type codes, m_nType in the header will be one of these values to show the type
#define G1_PACKET_PING			0x00 // Ping packet
#define G1_PACKET_PONG			0x01 // Pong packet, response to a ping
#define G1_PACKET_BYE			0x02 // Goodbye packet, the remote computer telling us why it's disconnecting
#define G1_PACKET_QUERY_ROUTE	0x30 // Packet about query routing table (do)
#define G1_PACKET_VENDOR		0x31 // Vendor-specific packets (do)
#define G1_PACKET_VENDOR_APP	0x32
#define G1_PACKET_PUSH			0x40 // Packet asking that we push open a connection to a remote computer 
									 // that can't connect directly to us
#define G1_PACKET_RUDP			0x41 // Packet used for F2F RUDP transfers
#define G1_PACKET_QUERY			0x80 // Search query
#define G1_PACKET_HIT			0x81 // Response to search query, a hit

// Packet type indices, another enumeration for Gnutella packets, GnutellaTypeToIndex translates from the byte code to this number
#define G1_PACKTYPE_UNKNOWN		0
#define G1_PACKTYPE_PING		1
#define G1_PACKTYPE_PONG		2
#define G1_PACKTYPE_BYE			3
#define G1_PACKTYPE_QUERY_ROUTE	4
#define G1_PACKTYPE_VENDOR		5
#define G1_PACKTYPE_PUSH		6
#define G1_PACKTYPE_QUERY		7
#define G1_PACKTYPE_HIT			8
#define G1_PACKTYPE_MAX			9 // There are 9 packet type indices, with values 0 through 8

// MinSpeed Flags (do)
#define G1_QF_TAG				0x8000	// If the bit 15 is 0, then this is a query with the deprecated minspeed semantic. If the bit 15 is set to 1, then this is a query with the new minimum speed semantic.
#define G1_QF_FIREWALLED		0x4000	// Firewalled indicator. This flag can be used by the remote servent to avoid returning queryHits if it is itself firewalled, as the requesting servent won't be able to download the files.
#define G1_QF_XML				0x2000	// XML Metadata. Set this bit to 1 if you want the servent to receive XML Metadata. This flag has been set to spare bandwidth, returning metadata in queryHits only if the requester asks for it.
#define G1_QF_DYNAMIC			0x1000	// Leaf Guided Dynamic Query. When the bit is set to 1, this means that the query is sent by a leaf which wants to control the dynamic query mechanism. This is part of the Leaf guidance of dynamic queries proposal. This information is only used by the ultrapeers shielding this leave if they implement leaf guidance of dynamic queries.
#define G1_QF_BIN_HASH			0x0800	// GGEP "H" allowed. If this bit is set to 1, then the sender is able to parse the GGEP "H" extension which is a replacement for the legacy HUGE GEM extension. This is meant to start replacing the GEM mechanism with GGEP extensions, as GEM extensions are now deprecated.
#define G1_QF_OOB				0x0400	// OOB v2. Out of Band Query. This flag is used to recognize a Query which was sent using the Out Of Band query extension.
#define G1_QF_FWTRANS			0x0200	// Firewalled transfers supported.

#define OLD_LW_MAX_QUERY_FIELD_LEN	30
#define WHAT_IS_NEW_QUERY_STRING	"WhatIsNewXOXO"
#define DEFAULT_URN_QUERY			"\\"

#define QUERY_KEY_LIFETIME		2 * 60 * 60
#define MIN_QK_SIZE_IN_BYTES	4
#define MAX_QK_SIZE_IN_BYTES	16

// QHD Flags (do)
#define G1_QHD_PUSH				0x01
#define G1_QHD_BAD				0x02
#define G1_QHD_BUSY				0x04
#define G1_QHD_STABLE			0x08
#define G1_QHD_SPEED			0x10
#define G1_QHD_GGEP				0x20
#define G1_QHD_MASK				0x3D

#define G1_PACKET_HIT_SEP		0x1C // Query hit extension separator
