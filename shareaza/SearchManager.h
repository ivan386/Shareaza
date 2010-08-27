//
// SearchManager.h
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

#include "ManagedSearch.h"

class CG2Packet;
class CQueryHit;


class CSearchManager
{
public:
	CSearchManager();
	~CSearchManager();

	void			OnRun();
	BOOL			OnQueryAck(CG2Packet* pPacket, const SOCKADDR_IN* pAddress, Hashes::Guid& oGUID);
	BOOL			OnQueryHits(const CQueryHit* pHits);
	WORD			OnQueryStatusRequest(const Hashes::Guid& oGUID);

protected:
	typedef CList< CManagedSearch* > CSearchList;

	CMutexEx		m_pSection;
	CSearchList		m_pList;
	DWORD			m_tLastTick;
	int				m_nPriorityClass;
	int				m_nPriorityCount;
	Hashes::Guid	m_oLastSearch;

	bool			Add(CManagedSearch* pSearch);
	bool			Remove(CManagedSearch* pSearch);
	CSearchPtr		Find(const Hashes::Guid& oGUID) const;

	friend class CManagedSearch;	// m_pSection, m_oLastSearch, Add(), Remove()
};

extern CSearchManager SearchManager;
