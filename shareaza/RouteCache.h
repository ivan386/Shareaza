//
// RouteCache.h
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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

#define ROUTE_HASH_SIZE		1024
#define ROUTE_HASH_MASK		1023

class CNeighbour;


class CRouteCacheItem
{
public:
	CRouteCacheItem();

	CRouteCacheItem*	m_pNext;
	DWORD				m_tAdded;
	Hashes::Guid		m_oGUID;
	const CNeighbour*	m_pNeighbour;
	SOCKADDR_IN			m_pEndpoint;
};


class CRouteCacheTable
{
// Construction
public:
	CRouteCacheTable();
	virtual ~CRouteCacheTable();

// Attributes
protected:
	CRouteCacheItem*	m_pHash[ ROUTE_HASH_SIZE ];
	CRouteCacheItem*	m_pFree;
	CRouteCacheItem*	m_pBuffer;
	DWORD				m_nBuffer;
	DWORD				m_nUsed;
	DWORD				m_tFirst;
	DWORD				m_tLast;

// Operations
public:
	CRouteCacheItem*	Find(const Hashes::Guid& oGUID);
	CRouteCacheItem*	Add(const Hashes::Guid& oGUID, const CNeighbour* pNeighbour, const SOCKADDR_IN* pEndpoint, DWORD nTime = 0);
	void				Remove(CNeighbour* pNeighbour);
	void				Resize(DWORD nSize);
	DWORD				GetNextSize(DWORD nDesired);
	void				Clear();

	inline BOOL IsFull() const
	{
		return m_nUsed == m_nBuffer;
	}
};


class CRouteCache
{
// Construction
public:
	CRouteCache();
	virtual ~CRouteCache();

// Attributes
protected:
	DWORD				m_nSeconds;
	CRouteCacheTable	m_pTable[2];
	CRouteCacheTable*	m_pRecent;
	CRouteCacheTable*	m_pHistory;

	CRouteCacheItem*	Add(const Hashes::Guid& oGUID, const CNeighbour* pNeighbour, const SOCKADDR_IN* pEndpoint, DWORD tAdded);

// Operations
public:
	void		SetDuration(DWORD nSeconds);
	BOOL		Add(const Hashes::Guid& oGUID, const CNeighbour* pNeighbour);
	BOOL		Add(const Hashes::Guid& oGUID, const SOCKADDR_IN* pEndpoint);
	void		Remove(CNeighbour* pNeighbour);
	void		Clear();

	CRouteCacheItem*	Lookup(const Hashes::Guid& oGUID, CNeighbour** ppNeighbour = NULL, SOCKADDR_IN* pEndpoint = NULL);
};
