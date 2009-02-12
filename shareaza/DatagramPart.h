//
// DatagramPart.h
//
// Copyright © Shareaza Development Team, 2002-2009.
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

#if !defined(AFX_DATAGRAMPART_H__293C0502_E9B1_4C85_9E8C_9C641318B939__INCLUDED_)
#define AFX_DATAGRAMPART_H__293C0502_E9B1_4C85_9E8C_9C641318B939__INCLUDED_

#pragma once

class CBuffer;
class CG2Packet;


class CDatagramOut
{
// Construction
public:
	CDatagramOut();
	virtual ~CDatagramOut();

// Attributes
public:
	CDatagramOut*	m_pNextHash;
	CDatagramOut**	m_pPrevHash;
	CDatagramOut*	m_pNextTime;
	CDatagramOut*	m_pPrevTime;
public:
	SOCKADDR_IN		m_pHost;
	WORD			m_nSequence;
	CBuffer*		m_pBuffer;
	LPVOID			m_pToken;
	BOOL			m_bCompressed;
	DWORD			m_nPacket;
	BYTE			m_nCount;
	BYTE			m_nAcked;
	DWORD*			m_pLocked;
	BYTE			m_nLocked;
	DWORD			m_tSent;
	BOOL			m_bAck;

// Operations
public:
	void	Create(SOCKADDR_IN* pHost, CG2Packet* pPacket, WORD nSequence, CBuffer* pBuffer, BOOL bAck);
	BOOL	GetPacket(DWORD tNow, BYTE** ppPacket, DWORD* pnPacket, BOOL bResend);
	BOOL	Acknowledge(BYTE nPart);

};

#endif // !defined(AFX_DATAGRAMPART_H__293C0502_E9B1_4C85_9E8C_9C641318B939__INCLUDED_)
