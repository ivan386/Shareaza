//
// G1Packet.h
//
// Copyright (c) Shareaza Development Team, 2002-2013.
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
#include "Schema.h"


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

	// Change the packet's TTL and hop counts
	BOOL Hop(); // Make sure the TTL is 2 or more, and then make it one less and the hops count one more

	// Hash the packet
	void CacheHash();

	// Get the packet's type, GUID, and all its bytes
	virtual CString GetType() const;
	CString         GetGUID() const;

	virtual void	Reset();
	virtual void    ToBuffer(CBuffer* pBuffer, bool bTCP = true); // Adds the Gnutella packet header and payload into the given CBuffer object

#ifdef _DEBUG
	// Record information about the packet for debugging purposes
	virtual void Debug(LPCTSTR pszReason) const;
#endif // _DEBUG

	// Convert between the various ways the program expresses packet types, like ping and pong
	static int     GnutellaTypeToIndex(BYTE nType); // Turn a type byte, like 0x30, into index 4, both describe a query route packet
	static LPCTSTR m_pszPackets[9];                 // Turn a type index, like 4, into text like "QRP" for query route packet

	// Read IP/IPP/DIP/DIPP hosts from GGEP and add to cache.
	// Returns amount of successfully added or updated hosts and -1 on errors.
	static int GGEPReadCachedHosts(const CGGEPBlock& pGGEP);

	// Received SCP GGEP, send 5 random hosts from the cache
	// Since we do not provide leaves, ignore the preference data
	static void GGEPWriteRandomCache(CGGEPBlock& pGGEP, LPCTSTR pszID);

	// Read Gnutella HUGE extension
	bool ReadHUGE(CShareazaFile* pFile);
	// Read Gnutella XML extension
	bool ReadXML(CSchemaPtr& pSchema, CXMLElement*& pXML);

	// Decode metadata and Schema from text or XML deflated or plain
	static CXMLElement* AutoDetectSchema(LPCTSTR pszInfo);
	static CXMLElement* AutoDetectAudio(LPCTSTR pszInfo);

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
	BOOL OnQuery(const SOCKADDR_IN* pHost);
	BOOL OnCommonHit(const SOCKADDR_IN* pHost);
	BOOL OnPush(const SOCKADDR_IN* pHost);

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
#define G1_PACKET_PING			0x00	// Ping packet
#define G1_PACKET_PONG			0x01	// Pong packet, response to a ping
#define G1_PACKET_BYE			0x02	// Goodbye packet, the remote computer telling us why it's disconnecting
#define G1_PACKET_QUERY_ROUTE	0x30	// Packet about query routing table (do)
#define G1_PACKET_VENDOR		0x31	// Vendor-specific packets (do)
#define G1_PACKET_VENDOR_APP	0x32
#define G1_PACKET_PUSH			0x40	// Packet asking that we push open a connection to a remote computer 
										// that can't connect directly to us
#define G1_PACKET_RUDP			0x41	// Packet used for F2F RUDP transfers
#define G1_PACKET_QUERY			0x80	// Search query
#define G1_PACKET_HIT			0x81	// Response to search query, a hit

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
#define G1_PACKTYPE_MAX			9		// There are 9 packet type indices, with values 0 through 8

// MinSpeed Flags (do)
#define G1_QF_TAG				0x80	// If the bit 15 is 0, then this is a query with the deprecated minspeed semantic. If the bit 15 is set to 1, then this is a query with the new minimum speed semantic.
#define G1_QF_FIREWALLED		0x40	// Firewalled indicator. This flag can be used by the remote servent to avoid returning queryHits if it is itself firewalled, as the requesting servent won't be able to download the files.
#define G1_QF_XML				0x20	// XML Metadata. Set this bit to 1 if you want the servent to receive XML Metadata. This flag has been set to spare bandwidth, returning metadata in queryHits only if the requester asks for it.
#define G1_QF_DYNAMIC			0x10	// Leaf Guided Dynamic Query. When the bit is set to 1, this means that the query is sent by a leaf which wants to control the dynamic query mechanism. This is part of the Leaf guidance of dynamic queries proposal. This information is only used by the ultrapeers shielding this leave if they implement leaf guidance of dynamic queries.
#define G1_QF_BIN_HASH			0x08	// GGEP "H" allowed. If this bit is set to 1, then the sender is able to parse the GGEP "H" extension which is a replacement for the legacy HUGE GEM extension. This is meant to start replacing the GEM mechanism with GGEP extensions, as GEM extensions are now deprecated.
#define G1_QF_OOB				0x04	// OOB v2. Out of Band Query. This flag is used to recognize a Query which was sent using the Out Of Band query extension.
#define G1_QF_FWTRANS			0x02	// Firewalled transfers supported.

#define OLD_LW_MAX_QUERY_FIELD_LEN	30
#define WHAT_IS_NEW_QUERY_STRING	"whatisnewxoxo"
#define DEFAULT_URN_QUERY			"\\"
#define DEFAULT_G1_MCAST_ADDRESS	"234.21.81.1"
#define DEFAULT_G1_MCAST_PORT		6347

#define QUERY_KEY_LIFETIME		2 * 60 * 60
#define MIN_QK_SIZE_IN_BYTES	4
#define MAX_QK_SIZE_IN_BYTES	16

// QueryHit Flags (inside Public data)
#define G1_QHD_PUSH				0x01
#define G1_QHD_BAD				0x02
#define G1_QHD_BUSY				0x04
#define G1_QHD_STABLE			0x08
#define G1_QHD_SPEED			0x10
#define G1_QHD_GGEP				0x20
#define G1_QHD_MASK				0x3D

#define G1_QHD_CHAT				0x01	// Chat flag

#define G1_PACKET_HIT_SEP		0x1C	// Query hit extension separator

// Known GGEP Extension Blocks table:
// http://gnutella-specs.rakjar.de/index.php/Known_GGEP_Extension_Blocks

// Browse Host (no value)
const LPCTSTR GGEP_HEADER_BROWSE_HOST			= _T("BH");
// Average daily uptime (seconds, 1-3 bytes)
const LPCTSTR GGEP_HEADER_DAILY_AVERAGE_UPTIME	= _T("DU");
// Unicast protocol support
const LPCTSTR GGEP_HEADER_UNICAST_SUPPORT		= _T("GUE");
// Vendor info. The value is like "LIME#". The first 4 bytes are the ASCII characters of the 4-character vendor code. The 5th byte has the major and minor version number squashed into a single byte.
const LPCTSTR GGEP_HEADER_VENDOR_INFO			= _T("VC");
// Ultrapeer support
const LPCTSTR GGEP_HEADER_UP_SUPPORT			= _T("UP");
// AddressSecurityToken support
const LPCTSTR GGEP_HEADER_QUERY_KEY_SUPPORT		= _T("QK");
// OOB v3 Security Token support
const LPCTSTR GGEP_HEADER_SECURE_OOB			= _T("SO");
// AddressSecurityToken support
const LPCTSTR GGEP_HEADER_MULTICAST_RESPONSE	= _T("MCAST");
// PushProxy support
const LPCTSTR GGEP_HEADER_PUSH_PROXY			= _T("PUSH");
// PushProxy TLS indexes
const LPCTSTR GGEP_HEADER_PUSH_PROXY_TLS		= _T("PUSH_TLS");
// AlternateLocation support
const LPCTSTR GGEP_HEADER_ALTS					= _T("ALT");
// AlternateLocations that support TLS
const LPCTSTR GGEP_HEADER_ALTS_TLS				= _T("ALT_TLS");
// IpPort request
const LPCTSTR GGEP_HEADER_IPPORT				= _T("IP");
// UDP HostCache pongs
const LPCTSTR GGEP_HEADER_UDP_HOST_CACHE		= _T("UDPHC");
// Indicating support for packed ip/ports & udp host caches
const LPCTSTR GGEP_HEADER_SUPPORT_CACHE_PONGS	= _T("SCP");
// Packed IP/Ports
const LPCTSTR GGEP_HEADER_PACKED_IPPORTS		= _T("IPP");
// Which packed IP/Ports support TLS
const LPCTSTR GGEP_HEADER_PACKED_IPPORTS_TLS	= _T("IPP_TLS");
// Packed UDP Host Caches
const LPCTSTR GGEP_HEADER_PACKED_HOSTCACHES		= _T("PHC");
// SHA1 URNs
const LPCTSTR GGEP_HEADER_SHA1					= _T("S1");
// Tiger Tree Root URNs (24 bytes)
const LPCTSTR GGEP_HEADER_TTROOT				= _T("TT");
// Determine if a SHA1 is valid
const LPCTSTR GGEP_HEADER_SHA1_VALID			= _T("SV");
// TLS support
const LPCTSTR GGEP_HEADER_TLS_SUPPORT			= _T("TLS");
// DHT support
const LPCTSTR GGEP_HEADER_DHT_SUPPORT			= _T("DHT");
// DHT IPP requests
const LPCTSTR GGEP_HEADER_DHT_IPPORTS			= _T("DHTIPP");
// A feature query. This is 'WH' for legacy reasons, because 'What is New' was the first
const LPCTSTR GGEP_HEADER_FEATURE_QUERY			= _T("WH");
// The extension header disabling OOB proxying
const LPCTSTR GGEP_HEADER_NO_PROXY				= _T("NP");
// MetaType query support
const LPCTSTR GGEP_HEADER_META					= _T("M");
// Client locale
const LPCTSTR GGEP_HEADER_CLIENT_LOCALE			= _T("LOC");
// Network wide file creation time (4 bytes)(in seconds)
const LPCTSTR GGEP_HEADER_CREATE_TIME			= _T("CT");
// Firewalled Transfer support in Hits (1 byte - version, supported if version > 0)
const LPCTSTR GGEP_HEADER_FW_TRANS				= _T("FW");
// The extension header (key) indicating the GGEP block is the 'secure' block
const LPCTSTR GGEP_HEADER_SECURE_BLOCK			= _T("SB");
// The extension header (key) indicating the value has a signature in it
const LPCTSTR GGEP_HEADER_SIGNATURE				= _T("SIG");
// Chat support
const LPCTSTR GGEP_HEADER_CHAT					= _T("CHAT");
// Equivalent of GGEP SCP but for GnucDNA peers only. Unlike SCP, it's also used as acknowledgment
const LPCTSTR GGEP_HEADER_SUPPORT_GDNA			= _T("DNA");
// Legacy buggy version of GnucDNA DIPP
const LPCTSTR GGEP_HEADER_GDNA_PACKED_IPPORTS_x	= _T("DIP");
// Equivalent of GGEP IPP but contains GnucDNA peers only
const LPCTSTR GGEP_HEADER_GDNA_PACKED_IPPORTS	= _T("DIPP");
// File hash. SHA1 only or SHA1 + Tiger
const LPCTSTR GGEP_HEADER_HASH					= _T("H");
// URN but without "urn:" prefix
const LPCTSTR GGEP_HEADER_URN					= _T("u");
// Indicating the size of the file is 64 bit
const LPCTSTR GGEP_HEADER_LARGE_FILE			= _T("LF");
// The prefix of the extension header indicating support for partial results
const LPCTSTR GGEP_HEADER_PARTIAL_RESULT_PREFIX	= _T("PR");
// Determine if the encoded ranges are unverified
const LPCTSTR GGEP_HEADER_PARTIAL_RESULT_UNVERIFIED	= _T("PRU");
// To support queries longer than previous length limit on query string fields
const LPCTSTR GGEP_HEADER_EXTENDED_QUERY		= _T("XQ");
// Various information contained in a return path entry GGEP block
const LPCTSTR GGEP_HEADER_RETURN_PATH_SOURCE	= _T("RPS");
const LPCTSTR GGEP_HEADER_RETURN_PATH_HOPS		= _T("RPH");
const LPCTSTR GGEP_HEADER_RETURN_PATH_ME		= _T("RPI");
const LPCTSTR GGEP_HEADER_RETURN_PATH_TTL		= _T("RPT");

// GGEP_HEADER_SUPPORT_CACHE_PONGS - Support Cache Pongs(SCP)
#define GGEP_SCP_LEAF		0x00 // If we are a leaf
#define GGEP_SCP_ULTRAPEER	0x01 // If we are a ultrapeer
#define GGEP_SCP_TLS		0x02 // If we support incoming TLS

// GGEP_HEADER_HASH - Hash query
#define GGEP_H_SHA1			0x01 // Binary SHA1
#define GGEP_H_BITPRINT		0x02 // Bitprint (SHA1 + Tiger tree root)
#define GGEP_H_MD5			0x03 // Binary MD5
#define GGEP_H_UUID			0x04 // Binary UUID (GUID-like)
#define GGEP_H_MD4			0x05 // Binary MD4

// GGEP_HEADER_META - MetaType query support
#define GGEP_META_RESERVED1	0x01 // Reserved
#define GGEP_META_RESERVED2	0x02 // Reserved
#define GGEP_META_AUDIO		0x04 // Audio
#define GGEP_META_VIDEO		0x08 // Video
#define GGEP_META_DOCUMENTS	0x10 // Documents
#define GGEP_META_IMAGES	0x20 // Images
#define GGEP_META_WINDOWS	0x40 // Windows Programs/Packages 
#define GGEP_META_UNIX		0x80 // Linux/Unix/Mac Programs/Packages
