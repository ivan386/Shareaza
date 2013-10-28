//
// ManagedSearch.h
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

#include "QuerySearch.h"

class CPacket;
class CNeighbour;
class CManagedSearch;


typedef CComObjectPtr< CManagedSearch > CSearchPtr;


class CManagedSearch : public CComObject
{
	DECLARE_DYNAMIC(CManagedSearch)

public:
	CManagedSearch(CQuerySearch* pSearch = NULL, int nPriority = 0);

	typedef CMap< DWORD, DWORD, DWORD, DWORD > CDwordDwordMap;
	
	enum { spHighest, spMedium, spLowest, spMax };

	inline CQuerySearchPtr GetSearch() const
	{
		return m_pSearch;
	}

	inline bool IsEqualGUID(const Hashes::Guid& oGUID) const
	{
		return m_pSearch && validAndEqual( m_pSearch->m_oGUID, oGUID );
	}

	inline CSchemaPtr GetSchema() const
	{
		return m_pSearch ? m_pSearch->m_pSchema : NULL;
	}

	inline bool IsActive() const
	{
		return ( m_bActive != FALSE );
	}

	inline void SetActive(BOOL bActive)
	{
		InterlockedExchange( (LONG*)&m_bActive, bActive );
	}

	inline void SetPriority(int nPriority)
	{
		InterlockedExchange( (LONG*)&m_nPriority, nPriority );
	}

	void	Serialize(CArchive& ar);
	void	Start();
	void	Stop();
	BOOL	Execute(int nPriorityClass);		// Run search of specified priority class
	void	OnHostAcknowledge(DWORD nAddress);
	BOOL	IsLastSearch();						// Check GUID of latest text search on ED2K and DC++ networks
	void	CreateGUID();

	BOOL			m_bAllowG1;					// Gnutella search enabled
	BOOL			m_bAllowG2;					// Gnutella2 search enabled
	BOOL			m_bAllowED2K;				// eDonkey2000 search enabled
	BOOL			m_bAllowDC;					// DC++ search enabled
	BOOL			m_bReceive;
	DWORD			m_nHits;					// Total hits
	DWORD			m_nHubs;					// Number of G2 hubs searched
	DWORD			m_nLeaves;					// Number of G2 leaves searched
	DWORD			m_nQueryCount;				// Total Gnutella2 queries sent
	DWORD			m_tLastED2K;				// Time an ed2k server was last searched
	DWORD			m_tMoreResults;				// Time more results were requested from an ed2k server

protected:
	virtual ~CManagedSearch();

	int				m_nPriority;
	BOOL			m_bActive;
	DWORD			m_tLastG1;					// Time a G1 multicast search was sent
	DWORD			m_tLastG2;					// Time a G2 hub was last searched
	CQuerySearchPtr m_pSearch;					// Search handler	
	CDwordDwordMap	m_pNodes;					// Pair of IP and query time (s)
	CDwordDwordMap	m_pG1Nodes;					// Pair of IP and last sent packet TTL
	DWORD			m_tExecute;					// Search execute time (ticks)

	BOOL	ExecuteNeighbours(const DWORD tTicks, const DWORD tSecs);
	BOOL	ExecuteG1Mesh(const DWORD tTicks, const DWORD tSecs);
	BOOL	ExecuteG2Mesh(const DWORD tTicks, const DWORD tSecs);
	BOOL	ExecuteDonkeyMesh(const DWORD tTicks, const DWORD tSecs);

private:
	CManagedSearch(const CManagedSearch&);
	CManagedSearch& operator=(const CManagedSearch&);
};
