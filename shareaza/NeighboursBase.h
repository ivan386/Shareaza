//
// NeighboursBase.h
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

// Keeps a list of CNeighbour objects, with methods to go through them, add and remove them, and count them
// http://wiki.shareaza.com/static/Developers.Code.CNeighboursBase

// Make the compiler only include the lines here once, this is the same thing as pragma once
#if !defined(AFX_NEIGHBOURSBASE_H__EE5DBF25_1EC6_40A7_9343_7373A2A882AD__INCLUDED_)
#define AFX_NEIGHBOURSBASE_H__EE5DBF25_1EC6_40A7_9343_7373A2A882AD__INCLUDED_

// Only include the lines beneath this one once
#pragma once

// Tell the compiler these classes exist, and it will find out more about them soon
class CNeighbour;

// Keeps a list of CNeighbour objects, with methods to go through them, add and remove them, and count them
class CNeighboursBase // Begin the inheritance column CNeighbours : CNeighboursWithConnect : Routing : ED2K : G2 : G1 : CNeighboursBase
{

public:

	// Make the CNeighbours object, and delete it
	CNeighboursBase();
	virtual ~CNeighboursBase();

protected:

	// The list of remote computers we are connected to
	CMap< DWORD_PTR, DWORD_PTR, CNeighbour*, CNeighbour* > m_pUniques;   // A MFC CMapPtrToPtr map collection class, the list of neighbour objects
	DWORD        m_nUnique;    // The unique key number we'll try to assign a neighbour object for the map next, like 2, 3, 4, and so on
	DWORD        m_nRunCookie; // OnRun uses this to run each neighbour once even if GetNext returns the same one more than once in the loop

public:

	// Counts and totals from the list of remote computers we have connections to
	DWORD m_nStableCount;  // The number of connections we have older than 1.5 seconds and finished with the handshake
	DWORD m_nLeafCount;    // The number of connections we have that are down to leaf nodes below us
	DWORD m_nLeafContent;  // The total size in bytes of all of the files all of these leaves are sharing
	DWORD m_nBandwidthIn;  // The total number of bytes that we've transferred through all the sockets, in each direction
	DWORD m_nBandwidthOut;

public:

	// Loop through each neighbour in the list
	POSITION    GetIterator()          const; // Call GetIterator to get the POSITION value
	CNeighbour* GetNext(POSITION& pos) const; // Give the POSITION to GetNext to get the neighbour beneath it and move to the next one

	// Lookup a neighbour in the list
	CNeighbour* Get(DWORD_PTR nUnique)     const; // By its unique number, like 2, 3, 4, and so on
	CNeighbour* Get(IN_ADDR* pAddress) const; // By the remote computer's IP address

	// Count how many computers we are connected to, specifying various filtering characteristics
	DWORD GetCount(PROTOCOLID nProtocol, int nState, int nNodeType)        const; // Pass -1 to not filter by protocol, state, or node type
	BOOL NeighbourExists(PROTOCOLID nProtocol, int nState, int nNodeType) const; // Use this if you just want to know if there are any or not

public:

	// Methods implimented by several classes in the CNeighbours inheritance column
	virtual void Connect(); // Does nothing, but inheriting classes have Connect methods with code in them
	virtual void Close();   // Calls Close on all the neighbours in the list, and resets member variables back to 0
	virtual void OnRun();   // Calls DoRun on each neighbour in the list, making them send and receive data

protected:

	// Add and remove neighbour objects from the list
	virtual void Add(CNeighbour* pNeighbour, BOOL bAssignUnique = TRUE);
	virtual void Remove(CNeighbour* pNeighbour);

	// Let the methods of CNeighbour, CShakeNeighbour, and CEDNeighbour access the private members of this class
	friend class CNeighbour;
	friend class CShakeNeighbour;
	friend class CEDNeighbour;
};

// End the group of lines to only include once, pragma once doesn't require an endif at the bottom
#endif // !defined(AFX_NEIGHBOURSBASE_H__EE5DBF25_1EC6_40A7_9343_7373A2A882AD__INCLUDED_)
