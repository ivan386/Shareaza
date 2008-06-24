//
// NeighboursWithG1.h
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

// Adds the ping route and pong caches to the CNeighbours object, and methods to route Gnutella ping and pong packets
// http://shareazasecurity.be/wiki/index.php?title=Developers.Code.CNeighboursWithG1

// Make the compiler only include the lines here once, this is the same thing as pragma once
#if !defined(AFX_NEIGHBOURSWITHG1_H__6CDF87CC_1439_4C48_84E5_D44ADE9141A7__INCLUDED_)
#define AFX_NEIGHBOURSWITHG1_H__6CDF87CC_1439_4C48_84E5_D44ADE9141A7__INCLUDED_

// Only include the lines beneath this one once
#pragma once

// Copy in the contents of these files here before compiling
#include "NeighboursBase.h"

// Tell the compiler these classes exist, and it will find out more about them soon
class CG1Neighbour;
class CRouteCache;
class CPongCache;

// Add the ping route and pong caches to the CNeighbours object
class CNeighboursWithG1 : public CNeighboursBase // Continue the inheritance column CNeighbours : CNeighboursWithConnect : Routing : ED2K : G2 : G1 : CNeighboursBase
{

public:

	// Called when the program runs and creates the CNeighbours object, and when it closes and deletes it
	CNeighboursWithG1();          // Setup the ping route and pong caches
	virtual ~CNeighboursWithG1(); // Delete the ping route and pong cache objects

public:

	// The ping route and pong caches
	CRouteCache* m_pPingRoute;
	CPongCache*  m_pPongCache;

public:

	// Methods implimented by several classes in the CNeighbours inheritance column
	virtual void Connect(); // Sets the ping route duration from settings
	virtual void Close();   // Call Close on each neighbour in the list, reset member variables to 0, and clear the ping route and pong caches

protected:

	// Remove a neighbour from the ping route and pong caches, network object, and the list
	virtual void Remove(CNeighbour* pNeighbour);

public:

	// Relay ping and pong packets to other neighbours
	void OnG1Ping();
	void OnG1Pong(CG1Neighbour* pFrom, IN_ADDR* pAddress, WORD nPort, BYTE nHops, DWORD nFiles, DWORD nVolume);
};

// End the group of lines to only include once, pragma once doesn't require an endif at the bottom
#endif // !defined(AFX_NEIGHBOURSWITHG1_H__6CDF87CC_1439_4C48_84E5_D44ADE9141A7__INCLUDED_)
