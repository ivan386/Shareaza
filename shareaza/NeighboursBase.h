//
// NeighboursBase.h
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

class CNeighbour;

class CNeighboursBase
{
protected:
	CNeighboursBase();
	virtual ~CNeighboursBase();

	virtual void Connect(); // Does nothing, but inheriting classes have Connect methods with code in them
	virtual void Close();   // Calls Close on all the neighbours in the list, and resets member variables back to 0
	virtual void OnRun();   // Calls DoRun on each neighbour in the list, making them send and receive data

public:
	POSITION    GetIterator()          const;	// Call GetIterator to get the POSITION value
	CNeighbour* GetNext(POSITION& pos) const;	// Give the POSITION to GetNext to get the neighbour beneath it and move to the next one
	CNeighbour* Get(DWORD_PTR nUnique) const;	// Lookup a neighbour by its unique number, like 2, 3, 4, and so on
	CNeighbour* Get(const IN_ADDR& pAddress) const;	// Lookup a neighbour by the remote computer's IP address
	CNeighbour* GetNewest(PROTOCOLID nProtocol, int nState, int nNodeType) const;	// Finds the newest neighbour object

	// Count how many computers we are connected to, specifying various filtering characteristics
	// pass -1 to not filter by protocol, state, or node type
	DWORD GetCount(PROTOCOLID nProtocol, int nState, int nNodeType) const;
	//BOOL NeighbourExists(PROTOCOLID nProtocol, int nState, int nNodeType) const; // Use this if you just want to know if there are any or not

	// Add and remove neighbour objects from the list
	virtual void Add(CNeighbour* pNeighbour);
	virtual void Remove(CNeighbour* pNeighbour);

private:
	typedef CMap< IN_ADDR, const IN_ADDR&, CNeighbour*, CNeighbour*& > CAMap;
	typedef CMap< DWORD_PTR, const DWORD_PTR&, CNeighbour*, CNeighbour*& > CNMap;

	CAMap	m_pNeighbours;	// The list of remote computers we are connected to
	CNMap	m_pIndex;		// Additional index
	DWORD	m_nRunCookie;	// OnRun uses this to run each neighbour once even if GetNext returns the same one more than once in the loop
};
