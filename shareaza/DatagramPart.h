//
// DatagramPart.h
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

#pragma once

class CBuffer;
class CG2Packet;


class CDatagramOut
{
public:
	CDatagramOut();
	~CDatagramOut();

	CDatagramOut*	m_pNextHash;
	CDatagramOut**	m_pPrevHash;
	CDatagramOut*	m_pNextTime;
	CDatagramOut*	m_pPrevTime;

	SOCKADDR_IN		m_pHost;
	WORD			m_nSequence;
	CBuffer*		m_pBuffer;
	LPVOID			m_pToken;
	DWORD			m_tSent;
	BOOL			m_bAck;

	void	Create(const SOCKADDR_IN* pHost, CG2Packet* pPacket, WORD nSequence, CBuffer* pBuffer, BOOL bAck);
	BOOL	GetPacket(DWORD tNow, BYTE** ppPacket, DWORD* pnPacket, BOOL bResend);
	BOOL	Acknowledge(BYTE nPart);

protected:
	BOOL			m_bCompressed;
	DWORD			m_nPacket;
	BYTE			m_nCount;
	BYTE			m_nAcked;
	DWORD*			m_pLocked;
	BYTE			m_nLocked;
};
