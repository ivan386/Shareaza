//
// G2Packet.h
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

#if !defined(AFX_G2PACKET_H__1790A2FA_5A60_4846_A7BD_629747EEC1C5__INCLUDED_)
#define AFX_G2PACKET_H__1790A2FA_5A60_4846_A7BD_629747EEC1C5__INCLUDED_

#pragma once

#include "GUID.h"
#include "Packet.h"

class CG1Packet;


class CG2Packet : public CPacket
{
// Construction
protected:
	CG2Packet();
	virtual ~CG2Packet();
	
// Attributes
public:
	CHAR	m_sType[9];
	BOOL	m_bCompound;

#ifdef _UNICODE
	CString	m_sTypeCache;
#endif
	
// Operations
public:
	void	WritePacket(CG2Packet* pPacket);
	void	WritePacket(LPCSTR pszType, DWORD nLength, BOOL bCompound = FALSE);
	BOOL	ReadPacket(LPSTR pszType, DWORD& nLength, BOOL* pbCompound = NULL);
	BOOL	SkipCompound();
	BOOL	SkipCompound(DWORD& nLength, DWORD nRemaining = 0);
	BOOL	GetTo(CGUID* pGUID);
	BOOL	SeekToWrapped();
public:
	virtual void	Reset();
	CG2Packet*		Clone() const;
	virtual CString	ReadString(DWORD nMaximum = 0xFFFFFFFF);
	virtual void	WriteString(LPCTSTR pszString, BOOL bNull = TRUE);
	virtual int		GetStringLen(LPCTSTR pszString) const;
	virtual void	ToBuffer(CBuffer* pBuffer) const;
	virtual void	Debug(LPCTSTR pszReason) const;
public:
	static CG2Packet* ReadBuffer(CBuffer* pBuffer);
	
#ifdef _UNICODE
	virtual void	WriteString(LPCSTR pszString, BOOL bNull = TRUE);
#endif

// Inlines
public:
	inline BOOL IsType(LPCSTR pszType) const
	{
		return strcmp( pszType, m_sType ) == 0;
	}

	virtual inline LPCTSTR GetType() const
	{
#ifdef _UNICODE
		if ( m_sTypeCache.IsEmpty() ) ((CG2Packet*)this)->m_sTypeCache = m_sType;
		return m_sTypeCache;
#else
		return m_sType;
#endif
	}

// Packet Pool
protected:
	class CG2PacketPool : public CPacketPool
	{
	public:
		virtual ~CG2PacketPool() { Clear(); }
	protected:
		virtual void NewPoolImpl(int nSize, CPacket*& pPool, int& nPitch);
		virtual void FreePoolImpl(CPacket* pPool);
	};
	
	static CG2PacketPool POOL;

// Construction
public:
	inline static CG2Packet* New(LPCSTR pszType = NULL, BOOL bCompound = FALSE)
	{
		CG2Packet* pPacket = (CG2Packet*)POOL.New();
		
		if ( pszType != NULL )
		{
			strncpy( pPacket->m_sType, pszType, 9 );
			pPacket->m_sType[8] = 0;
		}
		
		pPacket->m_bCompound = bCompound;
#ifdef _UNICODE
		pPacket->m_sTypeCache.Empty();
#endif
		
		return pPacket;
	}
	
	static CG2Packet* New(BYTE* pSource);
	static CG2Packet* New(LPCSTR pszType, CG1Packet* pWrap, int nMinTTL = 255);
	
	inline virtual void Delete()
	{
		POOL.Delete( this );
	}
	
	friend class CG2Packet::CG2PacketPool;
};

inline void CG2Packet::CG2PacketPool::NewPoolImpl(int nSize, CPacket*& pPool, int& nPitch)
{
	nPitch	= sizeof(CG2Packet);
	pPool	= new CG2Packet[ nSize ];
}

inline void CG2Packet::CG2PacketPool::FreePoolImpl(CPacket* pPacket)
{
	delete [] (CG2Packet*)pPacket;
}

//
// G2 Packet Flags
//

#define G2_FLAG_COMPOUND	0x04
#define G2_FLAG_BIG_ENDIAN	0x02

//
// G2 Packet Types
//

#define G2_PACKET_TO				"TO"
#define G2_PACKET_PING				"PI"
#define G2_PACKET_PONG				"PO"
#define G2_PACKET_LNI				"LNI"
#define G2_PACKET_KHL				"KHL"
#define G2_PACKET_HAW				"HAW"
#define G2_PACKET_QHT				"QHT"
#define G2_PACKET_QUERY_KEY_REQ		"QKR"
#define G2_PACKET_QUERY_KEY_ANS		"QKA"
#define G2_PACKET_QUERY				"Q2"
#define G2_PACKET_HIT				"QH2"
#define G2_PACKET_QUERY_WRAP		"Q1"
#define G2_PACKET_HIT_WRAP			"QH1"
#define G2_PACKET_QUERY_ACK			"QA"
#define G2_PACKET_PUSH				"PUSH"
#define G2_PACKET_PROFILE_CHALLENGE	"UPROC"
#define G2_PACKET_PROFILE_DELIVERY	"UPROD"
#define G2_PACKET_PROFILE_AVATAR	"UPROAVTR"
#define G2_PACKET_CRAWL_REQ			"CRAWLR"
#define G2_PACKET_CRAWL_ANS			"CRAWLA"
#define G2_PACKET_PHYSICAL_FOLDER	"PF"
#define G2_PACKET_VIRTUAL_FOLDER	"VF"
#define G2_PACKET_DISCOVERY_REQ		"DISCR"
#define G2_PACKET_DISCOVERY_ANS		"DISCA"
#define G2_PACKET_DISCOVERY_HUB		"DISCH"
#define G2_PACKET_DISCOVERY_LOG		"DISCL"

//
// G2 SS
//

#define G2_SS_PUSH		0x01
#define G2_SS_BUSY		0x02
#define G2_SS_STABLE	0x04

#endif // !defined(AFX_G2PACKET_H__1790A2FA_5A60_4846_A7BD_629747EEC1C5__INCLUDED_)
