//
// NeighboursWithRouting.h
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

// Adds methods that send packets and Gnutella queries to all the computers we're connected to
// http://shareazasecurity.be/wiki/index.php?title=Developers.Code.CNeighboursWithRouting

// Make the compiler only include the lines here once, this is the same thing as pragma once
#if !defined(AFX_NEIGHBOURSWITHROUTING_H__F6307BC0_A35F_43A3_A6BE_98C69D9A351E__INCLUDED_)
#define AFX_NEIGHBOURSWITHROUTING_H__F6307BC0_A35F_43A3_A6BE_98C69D9A351E__INCLUDED_

// Only include the lines beneath this one once
#pragma once

// Copy in the contents of these files here before compiling
#include "NeighboursWithED2K.h"

// Tell the compiler these classes exist, and it will find out more about them soon
class CPacket;
class CQuerySearch;

// Add methods to broadcast packets and Gnutella queries to all the neighbours in the list
class CNeighboursWithRouting : public CNeighboursWithED2K // Continue the inheritance column CNeighbours : CNeighboursWithConnect : Routing : ED2K : G2 : G1 : CNeighboursBase
{
protected:
	CNeighboursWithRouting(); // The constructor and destructor don't do anything
	virtual ~CNeighboursWithRouting();

public:

	// Send a packet to all the computers we're connected to
	int Broadcast(CPacket* pPacket, CNeighbour* pExcept = NULL, BOOL bGGEP = FALSE);

	// Send a query packet to all the computers we're connected to, translating it to Gnutella and Gnutella2 for computers running that software
	int RouteQuery(const CQuerySearch* pSearch, CPacket* pPacket, CNeighbour* pFrom, BOOL bToHubs);
};

// End the group of lines to only include once, pragma once doesn't require an endif at the bottom
#endif // !defined(AFX_NEIGHBOURSWITHROUTING_H__F6307BC0_A35F_43A3_A6BE_98C69D9A351E__INCLUDED_)
