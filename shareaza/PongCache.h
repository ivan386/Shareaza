//
// PongCache.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

#if !defined(AFX_PONGCACHE_H__0F9B689A_5132_49EB_8F13_670563C80A1D__INCLUDED_)
#define AFX_PONGCACHE_H__0F9B689A_5132_49EB_8F13_670563C80A1D__INCLUDED_

#pragma once

class CPongItem;
class CNeighbour;
class CG1Packet;


class CPongCache
{
// Construction
public:
	CPongCache();
	virtual ~CPongCache();

// Attributes
protected:
	CList< CPongItem* >	m_pCache;
	DWORD		m_nTime;

// Operations
public:
	void		Clear();
	void		ClearNeighbour(CNeighbour* pNeighbour);
	BOOL		ClearIfOld();
	CPongItem*	Add(CNeighbour* pNeighbour, IN_ADDR* pAddress, WORD nPort, BYTE nHops, DWORD nFiles, DWORD nVolume);
	CPongItem*	Lookup(CNeighbour* pNotFrom, BYTE nHops, CList< CPongItem* >* pIgnore);
	CPongItem*	Lookup(CNeighbour* pFrom);
public:
	POSITION	GetIterator() const;
	CPongItem*	GetNext(POSITION& pos) const;

};


class CPongItem
{
// Construction
public:
	CPongItem(CNeighbour* pNeighbour, IN_ADDR* pAddress, WORD nPort, BYTE nHops, DWORD nFiles, DWORD nVolume);
	virtual ~CPongItem();

// Attributes
public:
	CNeighbour*		m_pNeighbour;
	IN_ADDR			m_pAddress;
	WORD			m_nPort;
	BYTE			m_nHops;
	DWORD			m_nFiles;
	DWORD			m_nVolume;

// Operations
public:
	CG1Packet*		ToPacket(int nTTL, const Hashes::Guid& oGUID);

};

#endif // !defined(AFX_PONGCACHE_H__0F9B689A_5132_49EB_8F13_670563C80A1D__INCLUDED_)
