//
// G1Packet.h
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

#if !defined(AFX_G1PACKET_H__6B611C29_56C1_4E2A_AA72_249AB7BD76D0__INCLUDED_)
#define AFX_G1PACKET_H__6B611C29_56C1_4E2A_AA72_249AB7BD76D0__INCLUDED_

#pragma once

#include "Packet.h"

#pragma pack(1)

typedef struct
{
	GGUID	m_pGUID;
	BYTE	m_nType;
	BYTE	m_nTTL;
	BYTE	m_nHops;
	LONG	m_nLength;
} GNUTELLAPACKET;


class CG1Packet : public CPacket
{
// Construction
protected:
	CG1Packet();
	virtual ~CG1Packet();
	
// Attributes
public:
	GGUID	m_pGUID;
	BYTE	m_nType;
	BYTE	m_nTTL;
	BYTE	m_nHops;
	int		m_nTypeIndex;
	DWORD	m_nHash;

// Operations
public:
	BOOL			Hop();
	void			CacheHash();
	virtual BOOL	GetRazaHash(SHA1* pHash, DWORD nLength = 0) const;
	virtual LPCTSTR	GetType() const;
	CString			GetGUID() const;
	virtual void	ToBuffer(CBuffer* pBuffer) const;
	virtual void	Debug(LPCTSTR pszReason) const;
public:
	static int		GnutellaTypeToIndex(BYTE nType);
	static LPCTSTR	m_pszPackets[9];

// Packet Pool
protected:
	class CG1PacketPool : public CPacketPool
	{
	public:
		virtual ~CG1PacketPool() { Clear(); }
	protected:
		virtual void NewPoolImpl(int nSize, CPacket*& pPool, int& nPitch);
		virtual void FreePoolImpl(CPacket* pPool);
	};
	
	static CG1PacketPool POOL;

// Construction
public:
	static CG1Packet* New(int nType = 0, DWORD nTTL = 0, GGUID* pGUID = NULL);
	
	inline static CG1Packet* New(GNUTELLAPACKET* pSource)
	{
		CG1Packet* pPacket		= (CG1Packet*)POOL.New();
		
		pPacket->m_pGUID		= pSource->m_pGUID;
		pPacket->m_nType		= pSource->m_nType;
		pPacket->m_nTTL			= pSource->m_nTTL;
		pPacket->m_nHops		= pSource->m_nHops;
		pPacket->m_nTypeIndex	= GnutellaTypeToIndex( pPacket->m_nType );
		
		pPacket->Write( &pSource[1], (DWORD)pSource->m_nLength );
		
		return pPacket;
	}
	
	inline virtual void Delete()
	{
		POOL.Delete( this );
	}
	
	friend class CG1Packet::CG1PacketPool;
};

inline void CG1Packet::CG1PacketPool::NewPoolImpl(int nSize, CPacket*& pPool, int& nPitch)
{
	nPitch	= sizeof(CG1Packet);
	pPool	= new CG1Packet[ nSize ];
}

inline void CG1Packet::CG1PacketPool::FreePoolImpl(CPacket* pPacket)
{
	delete [] (CG1Packet*)pPacket;
}

#pragma pack()

//
// Packet Types
//

#define G1_PACKET_PING			0x00
#define G1_PACKET_PONG			0x01
#define G1_PACKET_BYE			0x02
#define G1_PACKET_QUERY_ROUTE	0x30
#define G1_PACKET_VENDOR		0x31
#define G1_PACKET_VENDOR_APP	0x32
#define G1_PACKET_PUSH			0x40
#define G1_PACKET_QUERY			0x80
#define G1_PACKET_HIT			0x81

//
// Packet Type Indexes
//

#define G1_PACKTYPE_UNKNOWN		0
#define G1_PACKTYPE_PING		1
#define G1_PACKTYPE_PONG		2
#define G1_PACKTYPE_BYE			3
#define G1_PACKTYPE_QUERY_ROUTE	4
#define G1_PACKTYPE_VENDOR		5
#define G1_PACKTYPE_PUSH		6
#define G1_PACKTYPE_QUERY		7
#define G1_PACKTYPE_HIT			8
#define G1_PACKTYPE_MAX			9

//
// MinSpeed Flags
//

#define G1_QF_TAG			0x8000
#define G1_QF_FIREWALLED	0x4000
#define G1_QF_XML			0x2000
#define G1_QF_DYNAMIC		0x1000
#define G1_QF_BIN_HASH		0x800
#define G1_QF_OOB			0x400

//
// QHD Flags
//

#define G1_QHD_PUSH			0x01
#define G1_QHD_BAD			0x02
#define G1_QHD_BUSY			0x04
#define G1_QHD_STABLE		0x08
#define G1_QHD_SPEED		0x10
#define G1_QHD_GGEP			0x20
#define G1_QHD_MASK			0x3D

//
// GGEP
//

#define GGEP_MAGIC			0xC3


#endif // !defined(AFX_G1PACKET_H__6B611C29_56C1_4E2A_AA72_249AB7BD76D0__INCLUDED_)
