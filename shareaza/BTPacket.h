//
// BTPacket.h
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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

#if !defined(AFX_BTPACKET_H__F3CA41D9_F0E8_4996_A551_F7EEE8D509B3__INCLUDED_)
#define AFX_BTPACKET_H__F3CA41D9_F0E8_4996_A551_F7EEE8D509B3__INCLUDED_

#pragma once

#include "Packet.h"

//
// Packet
//

class CBTPacket : public CPacket
{
// Construction
protected:
	CBTPacket();
	virtual ~CBTPacket();

// Attributes
public:
	BYTE	m_nType;

// Operations
public:
	virtual	void		ToBuffer(CBuffer* pBuffer) const;
	static	CBTPacket*	ReadBuffer(CBuffer* pBuffer);
public:
	virtual CString		GetType() const;

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
	inline static CBTPacket* New(BYTE nType)
	{
		CBTPacket* pPacket = (CBTPacket*)POOL.New();
		pPacket->m_nType = nType;
		return pPacket;
	}

	inline virtual void Delete()
	{
		POOL.Delete( this );
	}

	friend class CBTPacket::CBTPacketPool;
};

inline void CBTPacket::CBTPacketPool::NewPoolImpl(int nSize, CPacket*& pPool, int& nPitch)
{
	nPitch	= sizeof(CBTPacket);
	pPool	= new CBTPacket[ nSize ];
	if ( pPool == NULL )
	{
		theApp.Message( MSG_ERROR, _T("Memory allocation error in CBTPacket::CBTPacketPool::NewPoolImpl()") );
	}
}

inline void CBTPacket::CBTPacketPool::FreePoolImpl(CPacket* pPacket)
{
	delete [] (CBTPacket*)pPacket;
}

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

#define BT_PACKET_HANDSHAKE			128
#define BT_PACKET_SOURCE_REQUEST	129
#define BT_PACKET_SOURCE_RESPONSE	130

#define BT_PACKET_KEEPALIVE			255

#pragma pack(1)

typedef struct
{
	DWORD	nLength;
	BYTE	nType;
	DWORD	nPiece;
	DWORD	nOffset;
} BT_PIECE_HEADER;

#pragma pack()

#endif // !defined(AFX_BTPACKET_H__F3CA41D9_F0E8_4996_A551_F7EEE8D509B3__INCLUDED_)
