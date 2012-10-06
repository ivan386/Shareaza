//
// PongCache.h
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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

class CPongItem;
class CNeighbour;
class CG1Packet;


class CPongCache
{
public:
	CPongCache();
	~CPongCache();

//	POSITION	GetIterator() const;
//	CPongItem*	GetNext(POSITION& pos) const;

	void		Clear();
	void		ClearNeighbour(CNeighbour* pNeighbour);
	BOOL		ClearIfOld();
	CPongItem*	Add(CNeighbour* pNeighbour, IN_ADDR* pAddress, WORD nPort, BYTE nHops, DWORD nFiles, DWORD nVolume);
	CPongItem*	Lookup(CNeighbour* pNotFrom, BYTE nHops, CList< CPongItem* >* pIgnore) const;
	CPongItem*	Lookup(CNeighbour* pFrom) const;

protected:
	CList< CPongItem* >	m_pCache;
	DWORD				m_nTime;
};


class CPongItem
{
public:
	CPongItem(CNeighbour* pNeighbour, IN_ADDR* pAddress, WORD nPort, BYTE nHops, DWORD nFiles, DWORD nVolume);

	CNeighbour*		m_pNeighbour;
	IN_ADDR			m_pAddress;
	WORD			m_nPort;
	BYTE			m_nHops;
	DWORD			m_nFiles;
	DWORD			m_nVolume;

	CG1Packet*		ToPacket(int nTTL, const Hashes::Guid& oGUID) const;
};
