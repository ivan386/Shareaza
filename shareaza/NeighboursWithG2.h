//
// NeighboursWithG2.h
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

// Adds 2 methods helpful for Gnutella2 that look at the list of neighbours
// http://shareazasecurity.be/wiki/index.php?title=Developers.Code.CNeighboursWithG2

#pragma once

#include "NeighboursWithG1.h"

class CG2Neighbour;
class CG2Packet;

// Add methods helpful for Gnutella that need to look at the list of computers we're connected to
class CNeighboursWithG2 : public CNeighboursWithG1 // Continue the inheritance column CNeighbours : CNeighboursWithConnect : Routing : ED2K : G2 : G1 : CNeighboursBase
{
protected:
	CNeighboursWithG2();
	virtual ~CNeighboursWithG2();

public:
	virtual void Connect(); // Set the ping route duration and setup the hub horizon pool

public:

	// Make and return a query web packet with IP addresses from the neighbours list and the Gnutella2 host cache
	CG2Packet* CreateQueryWeb(const Hashes::Guid& oGUID, bool bWithHubs, CNeighbour* pExcept = NULL, bool bDone = true);

	// Return a random Gnutella2 hub neighbour that isn't pExcept and doesn't know about pGUID
	CG2Neighbour* GetRandomHub(CG2Neighbour* pExcept, const Hashes::Guid& oGUID);
};
