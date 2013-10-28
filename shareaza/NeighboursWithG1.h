//
// NeighboursWithG1.h
//
// Copyright (c) Shareaza Development Team, 2002-2011.
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

#pragma once

#include "NeighboursBase.h"
#include "PongCache.h"
#include "QuerySearch.h"
#include "RouteCache.h"


class CNeighbour;
class CG1Neighbour;

// Add the ping route and pong caches to the CNeighbours object
class CNeighboursWithG1 : public CNeighboursBase
{
protected:
	CNeighboursWithG1();          // Setup the ping route and pong caches
	virtual ~CNeighboursWithG1(); // Delete the ping route and pong cache objects

	virtual void Remove(CNeighbour* pNeighbour); // Remove a neighbour from the ping route and pong caches, network object, and the list
	virtual void Connect(); // Sets the ping route duration from settings
	virtual void Close();   // Call Close on each neighbour in the list, reset member variables to 0, and clear the ping route and pong caches
	virtual void OnRun();

private:
	// The ping route and pong caches
	CRouteCache m_oPingRoute;
	CPongCache  m_oPongCache;
	DWORD		m_tLastPingOut; // When we last sent a multicast ping packet

public:
	// Relay ping and pong packets to other neighbours
	void OnG1Ping();
	void OnG1Pong(CG1Neighbour* pFrom, IN_ADDR* pAddress, WORD nPort, BYTE nHops, DWORD nFiles, DWORD nVolume);

	// Send multicast ping
	void SendPing();
	// Send multicast query
	void SendQuery(CQuerySearchPtr pSearch);

	BOOL AddPingRoute(const Hashes::Guid& oGUID, const CG1Neighbour* pNeighbour);
	CG1Neighbour* GetPingRoute(const Hashes::Guid& oGUID);

	CPongItem*	AddPong(CNeighbour* pNeighbour, IN_ADDR* pAddress, WORD nPort, BYTE nHops, DWORD nFiles, DWORD nVolume);
	CPongItem*	LookupPong(CNeighbour* pNotFrom, BYTE nHops, CList< CPongItem* >* pIgnore);
};
