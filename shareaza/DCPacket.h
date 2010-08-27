//
// DCPacket.h
//
// Copyright (c) Shareaza Development Team, 2010.
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

class CDCPacket : public CPacket
{
protected:
	CDCPacket();
	virtual ~CDCPacket();

public:
	virtual void ToBuffer(CBuffer* pBuffer) const;

// Packet Pool
protected:
	class CDCPacketPool : public CPacketPool
	{
	public:
		virtual ~CDCPacketPool() { Clear(); }
	protected:
		virtual void NewPoolImpl(int nSize, CPacket*& pPool, int& nPitch);
		virtual void FreePoolImpl(CPacket* pPool);
	};

	static CDCPacketPool POOL;

// Allocation
public:
	inline static CDCPacket* New()
	{
		return (CDCPacket*)POOL.New();
	}

	inline virtual void Delete()
	{
		POOL.Delete( this );
	}

	// Packet handler
	virtual BOOL OnPacket(SOCKADDR_IN* pHost);

protected:
	BOOL OnCommonHit(SOCKADDR_IN* pHost);

	friend class CDCPacket::CDCPacketPool;
};

inline void CDCPacket::CDCPacketPool::NewPoolImpl(int nSize, CPacket*& pPool, int& nPitch)
{
	nPitch	= sizeof(CDCPacket);
	pPool	= new CDCPacket[ nSize ];
}

inline void CDCPacket::CDCPacketPool::FreePoolImpl(CPacket* pPacket)
{
	delete [] (CDCPacket*)pPacket;
}
