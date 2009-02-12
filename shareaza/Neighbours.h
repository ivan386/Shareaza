//
// Neighbours.h
//
// Copyright © Shareaza Development Team, 2002-2009.
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

// Complete the CNeighbours inheritance column, calling Close on each neighbour when the program exits
// http://shareazasecurity.be/wiki/index.php?title=Developers.Code.CNeighbours

// Make the compiler only include the lines here once, this is the same thing as pragma once
#if !defined(AFX_NEIGHBOURS_H__864DFBBB_7F72_429B_96FD_A98E720FD0D0__INCLUDED_)
#define AFX_NEIGHBOURS_H__864DFBBB_7F72_429B_96FD_A98E720FD0D0__INCLUDED_

// Only include the lines beneath this one once
#pragma once

// Copy in the contents of these files here before compiling
#include "NeighboursWithConnect.h"

// Complete the CNeighbours inheritance column, calling Close on each neighbour when the program exits
class CNeighbours : public CNeighboursWithConnect // End the inheritance column CNeighbours : CNeighboursWithConnect : Routing : ED2K : G2 : G1 : CNeighboursBase
{

public:

	// Set up and clean up anything CNeighbours adds to the CNeighbours class
	CNeighbours();
	virtual ~CNeighbours();

public:

	// Methods implimented by several classes in the CNeighbours inheritance column
	virtual void Connect(); // Set the ping route duration and setup the hub horizon pool
	virtual void Close();   // Call Close on each neighbour in the list, 0 member variables, and clear the ping route and pong caches
	virtual void OnRun();   // Call DoRun on each neighbour in the list, and maintain the network auto connection

	// Let CNeighbour and CShakeNeighbour look at the private members of the CNeighbours class
	friend class CNeighbour;
	friend class CShakeNeighbour;
};

// Access the single global Neighbours object that holds the list of neighbour computers we are connected to
extern CNeighbours Neighbours; // When Shareaza starts running, this line creates a single global instance of a CNeighbours object, called Neighbours

// End the group of lines to only include once, pragma once doesn't require an endif at the bottom
#endif // !defined(AFX_NEIGHBOURS_H__864DFBBB_7F72_429B_96FD_A98E720FD0D0__INCLUDED_)
