//
// Datagram.h
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

class CBuffer;
class CG2Packet;


class CDatagramIn
{
// Construction
public:
	CDatagramIn();
	virtual ~CDatagramIn();

// Attributes
public:
	CDatagramIn*	m_pNextHash;
	CDatagramIn**	m_pPrevHash;
	CDatagramIn*	m_pNextTime;
	CDatagramIn*	m_pPrevTime;
public:
	SOCKADDR_IN		m_pHost;
	BOOL			m_bCompressed;
	WORD			m_nSequence;
	BYTE			m_nCount;
	BYTE			m_nLeft;
	DWORD			m_tStarted;
public:
	CBuffer**		m_pBuffer;
	BOOL*			m_pLocked;
	BYTE			m_nBuffer;

// Operations
public:
	void		Create(const SOCKADDR_IN* pHost, BYTE nFlags, WORD nSequence, BYTE nCount);
	BOOL		Add(BYTE nPart, LPCVOID pData, DWORD nLength);
	CG2Packet*	ToG2Packet();

};
