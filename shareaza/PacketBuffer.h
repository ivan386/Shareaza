//
// PacketBuffer.h
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

#if !defined(AFX_PACKETBUFFER_H__7FE2F4C5_B0E8_444C_9B26_C95CCB344615__INCLUDED_)
#define AFX_PACKETBUFFER_H__7FE2F4C5_B0E8_444C_9B26_C95CCB344615__INCLUDED_

#pragma once

class CG1Packet;
class CBuffer;
class CG1PacketBufferType;


class CG1PacketBuffer  
{
// Construction
public:
	CG1PacketBuffer(CBuffer* pBuffer);
	virtual ~CG1PacketBuffer();
	
// Attributes
public:
	int			m_nTotal;
	int			m_nDropped;
protected:
	CBuffer*				m_pBuffer;
	int						m_nCycle;
	int						m_nIterate;
	CG1PacketBufferType*	m_pType;

// Operations
public:
	void		Add(CG1Packet* pPacket, BOOL bBuffered = TRUE);
	void		Clear();
	CG1Packet*	GetPacketToSend(DWORD dwExpire = 0);

};


class CG1PacketBufferType
{
// Construction
public:
	CG1PacketBufferType();
	virtual ~CG1PacketBufferType();

// Attributes
protected:
	CG1Packet**	m_pBuffer;
	DWORD*		m_pTime;
	int			m_nHead;
	int			m_nCount;
	int			m_nCapacity;

public:
	BOOL		Add(CG1Packet* pPacket);
	CG1Packet*	Get(DWORD dwExpire = 0, int* pnTotal = NULL, int* pnDropped = NULL);
	void		Clear();

};

#endif // !defined(AFX_PACKETBUFFER_H__7FE2F4C5_B0E8_444C_9B26_C95CCB344615__INCLUDED_)
