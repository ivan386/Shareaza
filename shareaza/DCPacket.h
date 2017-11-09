//
// DCPacket.h
//
// Copyright (c) Shareaza Development Team, 2010-2013.
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
	virtual CString GetType() const;
	virtual CString ToHex()   const;
	virtual CString ToASCII() const;
	virtual void Reset();
	virtual void ToBuffer(CBuffer* pBuffer, bool bTCP = true);
	static	CDCPacket*	ReadBuffer(CBuffer* pBuffer);

#ifdef _DEBUG
	virtual void Debug(LPCTSTR pszReason) const; // Writes debug information about the packet into the Shareaza.log file
#endif // _DEBUG

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
	inline static CDCPacket* New(const BYTE* pBuffer = NULL, DWORD nLength = 0)
	{
		if ( CDCPacket* pPacket = (CDCPacket*)POOL.New() )
		{
			if ( pBuffer == NULL || pPacket->Write( pBuffer, nLength ) )
				return pPacket;
			pPacket->Release();
		}
		return NULL;
	}

	inline virtual void Delete()
	{
		POOL.Delete( this );
	}

	// Packet handler
	virtual BOOL OnPacket(const SOCKADDR_IN* pHost);

protected:
	BOOL OnCommonHit(const SOCKADDR_IN* pHost);

	friend class CDCPacket::CDCPacketPool;

private:
	CDCPacket(const CDCPacket&);
	CDCPacket& operator=(const CDCPacket&);
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

#define DC_PROTOCOL_MIN_LEN	3
