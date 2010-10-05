//
// BTPacket.h
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

#pragma once

#include "Packet.h"

class CBENode;

//
// Packet Types
//

#define BT_PACKET_CHOKE				0
#define BT_PACKET_UNCHOKE			1
#define BT_PACKET_INTERESTED		2
#define BT_PACKET_NOT_INTERESTED	3
#define BT_PACKET_HAVE				4
#define BT_PACKET_BITFIELD			5
#define BT_PACKET_REQUEST			6
#define BT_PACKET_PIECE				7
#define BT_PACKET_CANCEL			8
#define BT_PACKET_DHT_PORT			9
#define BT_PACKET_EXTENSION			20 // Extension Protocol - http://www.bittorrent.org/beps/bep_0010.html

#define BT_PACKET_HANDSHAKE			128
#define BT_PACKET_SOURCE_REQUEST	129
#define BT_PACKET_SOURCE_RESPONSE	130

#define BT_PACKET_KEEPALIVE			255

// Packet extensions (for BT_PACKET_EXTENSION)

#define BT_EXTENSION_HANDSHAKE		0
#define BT_EXTENSION_UT_METADATA	1	// Extension for Peers to Send Metadata Files - http://www.bittorrent.org/beps/bep_0009.html 
#define BT_EXTENSION_UT_PEX			2
#define BT_EXTENSION_LT_TEX			3	// Tracker exchange extension - http://www.bittorrent.org/beps/bep_0028.html

#define BT_EXTENSION_NOP			255	// Packet without standard header i.e. bencoded data only

// Packet metadata type (for EXTENDED_PACKET_UT_METADATA)

#define UT_METADATA_REQUEST			0
#define UT_METADATA_DATA			1
#define UT_METADATA_REJECT			2

const LPCSTR BT_DICT_ADDED			= "added";
const LPCSTR BT_DICT_ADDED_F		= "added.f";
const LPCSTR BT_DICT_DATA			= "a";
const LPCSTR BT_DICT_DROPPED		= "dropped";
const LPCSTR BT_DICT_ERROR			= "e";
const LPCSTR BT_DICT_FAILURE		= "failure reason";
const LPCSTR BT_DICT_EXT_MSG		= "m";					// Dictionary of supported extension messages
const LPCSTR BT_DICT_ID				= "id";
const LPCSTR BT_DICT_INTERVAL		= "interval";
const LPCSTR BT_DICT_LT_TEX			= "lt_tex";
const LPCSTR BT_DICT_METADATA_SIZE	= "metadata_size";
const LPCSTR BT_DICT_MSG_TYPE		= "msg_type";
const LPCSTR BT_DICT_NICKNAME		= "nickname";
const LPCSTR BT_DICT_NODES			= "nodes";
const LPCSTR BT_DICT_PEER_ID		= "peer id";
const LPCSTR BT_DICT_PEER_IP		= "ip";
const LPCSTR BT_DICT_PEER_PORT		= "port";
const LPCSTR BT_DICT_PEER_URL		= "url";
const LPCSTR BT_DICT_PEERS			= "peers";
const LPCSTR BT_DICT_PIECE			= "piece";
const LPCSTR BT_DICT_PORT			= "p";					// Local TCP listen port
const LPCSTR BT_DICT_QUERY			= "q";
const LPCSTR BT_DICT_RESPONSE		= "r";
const LPCSTR BT_DICT_SRC_EXCHANGE	= "source-exchange";
const LPCSTR BT_DICT_TOKEN			= "token";
const LPCSTR BT_DICT_TOTAL_SIZE		= "total_size";
const LPCSTR BT_DICT_TRANSACT_ID	= "t";
const LPCSTR BT_DICT_TRACKERS		= "tr";					// Tracker List hash
const LPCSTR BT_DICT_TYPE			= "y";
const LPCSTR BT_DICT_YOURIP			= "yourip";				// External IP (IPv4 or IPv6)
const LPCSTR BT_DICT_USER_AGENT		= "user-agent";
const LPCSTR BT_DICT_UT_PEX			= "ut_pex";
const LPCSTR BT_DICT_UT_METADATA	= "ut_metadata";
const LPCSTR BT_DICT_VENDOR			= "v";					// Client name and version (as a utf-8 string)
const LPCSTR BT_DICT_VALUES			= "values";

//
// Packet
//

class CBTPacket : public CPacket
{
protected:
	CBTPacket();
	virtual ~CBTPacket();

public:
	BYTE				m_nType;
	BYTE				m_nExtension;	// Extension type if packet type is a BT_PACKET_EXTENSION
	auto_ptr< CBENode > m_pNode;		// Extension decoded data

	virtual void		Reset();
	virtual	void		ToBuffer(CBuffer* pBuffer, bool bTCP = true) const;
	static	CBTPacket*	ReadBuffer(CBuffer* pBuffer);
	virtual void		SmartDump(const SOCKADDR_IN* pAddress, BOOL bUDP, BOOL bOutgoing, DWORD_PTR nNeighbourUnique = 0) const;
	virtual CString		GetType() const;
	virtual CString		ToHex()   const;
	virtual CString		ToASCII() const;

	inline static bool IsEncoded(BYTE nType)
	{
		return
			nType == BT_PACKET_EXTENSION ||
			nType == BT_PACKET_HANDSHAKE ||
			nType == BT_PACKET_SOURCE_REQUEST ||
			nType == BT_PACKET_SOURCE_RESPONSE;
	}

	bool HasEncodedData() const;

// Packet Pool
protected:
	class CBTPacketPool : public CPacketPool
	{
	public:
		virtual ~CBTPacketPool() { Clear(); }
	protected:
		virtual void NewPoolImpl(int nSize, CPacket*& pPool, int& nPitch);
		virtual void FreePoolImpl(CPacket* pPool);
	};

	static CBTPacketPool POOL;

// Allocation
public:
	static CBTPacket* New(BYTE nType = BT_PACKET_EXTENSION, BYTE nExtension = BT_EXTENSION_NOP, const BYTE* pBuffer = NULL, DWORD nLength = 0);

	inline virtual void Delete()
	{
		POOL.Delete( this );
	}

	// Packet handler
	virtual BOOL OnPacket(const SOCKADDR_IN* pHost);

	BOOL OnPing(const SOCKADDR_IN* pHost);
	BOOL OnError(const SOCKADDR_IN* pHost);

	friend class CBTPacket::CBTPacketPool;

private:
	CBTPacket(const CBTPacket&);
	CBTPacket& operator=(const CBTPacket&);
};

inline void CBTPacket::CBTPacketPool::NewPoolImpl(int nSize, CPacket*& pPool, int& nPitch)
{
	nPitch	= sizeof(CBTPacket);
	pPool	= new CBTPacket[ nSize ];
}

inline void CBTPacket::CBTPacketPool::FreePoolImpl(CPacket* pPacket)
{
	delete [] (CBTPacket*)pPacket;
}

#pragma pack(1)

typedef struct
{
	DWORD	nLength;
	BYTE	nType;
	DWORD	nPiece;
	DWORD	nOffset;
} BT_PIECE_HEADER;

#pragma pack()
