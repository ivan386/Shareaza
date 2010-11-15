//
// NeighboursWithRouting.h
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

#include "NeighboursWithED2K.h"

class CPacket;
class CQuerySearch;

class CNeighboursWithRouting : public CNeighboursWithED2K
{
protected:
	CNeighboursWithRouting();
	virtual ~CNeighboursWithRouting();

	typedef struct
	{
		IN_ADDR	m_pAddress;
		DWORD	m_nTime;
	} CIPTime;

	CList< CIPTime > m_pQueries;

public:
	virtual void Connect();

	// Send a packet to all the computers we're connected to
	int Broadcast(CPacket* pPacket, CNeighbour* pExcept = NULL, BOOL bGGEP = FALSE);

	// Limit queries by endpoint addresses
	bool CheckQuery(const CQuerySearch* pSearch);

	// Send a query packet to all the computers we're connected to, translating it to Gnutella and Gnutella2 for computers running that software
	int RouteQuery(const CQuerySearch* pSearch, CPacket* pPacket, CNeighbour* pFrom, BOOL bToHubs);
};
