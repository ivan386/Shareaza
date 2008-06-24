//
// Neighbours.cpp
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

// Complete the CNeighbours inheritance column, calling Close on each neighbour when the program exits
// http://shareazasecurity.be/wiki/index.php?title=Developers.Code.CNeighbours

// Copy in the contents of these files here before compiling
#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "Neighbours.h"

// If we are compiling in debug mode, replace the text "THIS_FILE" in the code with the name of this file
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// Create the single global Neighbours object that holds the list of neighbour computers we are connected to
CNeighbours Neighbours; // When Shareaza starts running, this line creates a single global instance of a CNeighbours object, called Neighbours

//////////////////////////////////////////////////////////////////////
// CNeighbours construction

// The line above creates the one CNeighbours object named Neighbours
// Creating that object calls this constructor, then the CNeighboursWithConnect constructor, and so on all the way down to CNeighboursBase
// CNeighbours doesn't add anything to the inheritance column that needs to be set up
CNeighbours::CNeighbours()
{
}

// Delete the CNeighbours object
CNeighbours::~CNeighbours()
{
	// Call close on each neighbour in the list, reset member variables to 0, and clear the ping route and pong caches
	Close();
}

//////////////////////////////////////////////////////////////////////
// CNeighbours connect

// Set the ping route duration and setup the hub horizon pool
void CNeighbours::Connect()
{
	// Since there is no Connect method in CNeighboursWithConnect, this calls the highest one in the inheritance column
	CNeighboursWithConnect::Connect(); // Calls CNeighboursWithG2::Connect()
}

//////////////////////////////////////////////////////////////////////
// CNeighbours close

// Call Close on each neighbour in the list, reset member variables here to 0, and clear the ping and pong route caches
void CNeighbours::Close()
{
	// There isn't a Close method in CNeighbours with connect, so this calls CNeighboursWithG1::Close()
	CNeighboursWithConnect::Close();
}

//////////////////////////////////////////////////////////////////////
// CNeighbours run callback

// Call DoRun on each neighbour in the list, and maintain the network auto connection
void CNeighbours::OnRun()
{
	// Calls CNeighboursBase::OnRun(), and uses the network object to maintain the connection
	CNeighboursWithConnect::OnRun();
}
