//
// NeighboursWithG1.cpp
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

// Adds the ping route and pong caches to the CNeighbours object, and methods to route Gnutella ping and pong packets
// http://shareazasecurity.be/wiki/index.php?title=Developers.Code.CNeighboursWithG1

// Copy in the contents of these files here before compiling
#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "Statistics.h"
#include "Network.h"
#include "NeighboursWithG1.h"
#include "G1Neighbour.h"
#include "RouteCache.h"
#include "PongCache.h"

// If we are compiling in debug mode, replace the text "THIS_FILE" in the code with the name of this file
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CNeighboursWithG1 construction

// When the program makes the single global CNeighbours object, this constructor runs to setup the Gnutella part of it
CNeighboursWithG1::CNeighboursWithG1()
{
	// Create the ping route and pong caches, and have the CNeighbours object point to them
	m_pPingRoute = new CRouteCache();
	m_pPongCache = new CPongCache();
}

// When the program closes, the single global CNeighbours object is destroyed, and this code cleans up the Gnutella parts
CNeighboursWithG1::~CNeighboursWithG1()
{
	// Delete the ping route and pong cache objects
	delete m_pPongCache;
	delete m_pPingRoute;
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithG1 connect

// Set the ping route cache duration from settings
void CNeighboursWithG1::Connect()
{
	// Does nothing
	CNeighboursBase::Connect();

	// Tell the route cache object to set its duration from the program settings
	m_pPingRoute->SetDuration( Settings.Gnutella.RouteCache );
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithG1 close

// Call Close on each neighbour in the list, reset the member variables here, and clear the ping route and pong caches
void CNeighboursWithG1::Close()
{
	// Call Close on each neighbour in the list, and reset the member variables of this CNeighbours object back to 0
	CNeighboursBase::Close();

	// Clear the ping route and pong caches
	m_pPingRoute->Clear();
	m_pPongCache->Clear();
}

// Takes a neighbour object
// Removes it from the ping route cache, network object, and the list
void CNeighboursWithG1::Remove(CNeighbour* pNeighbour)
{
	// Remove this neighbour from the ping route cache
	m_pPingRoute->Remove( pNeighbour );

	// Remove the neighbour from the list
	CNeighboursBase::Remove( pNeighbour ); // Also tells the network object to remove the neighbour
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithG1 G1 ping handler

// CG1Neighbour::OnPing calls this after it has received, broadcasted, and responded to a ping packet from a remote computer
// Loops through the list of neighbours, pinging those that are running Gnutella software that supports pong caching
void CNeighboursWithG1::OnG1Ping()
{
	// Clear the old (do) from the pong cache, and make sure that works (do)
	if ( m_pPongCache->ClearIfOld() )
	{
		// Prepare data for a new packet we might send
		DWORD dwNow = GetTickCount(); // The time now
		Hashes::Guid oGUID;           // A new GUID for the packet (do)
		Network.CreateID( oGUID );

		CSingleLock pLock( &Network.m_pSection, TRUE );

		// Loop for each neighbour we're connected to
		for ( POSITION pos = GetIterator() ; pos ; )
		{
			// Get the neighbour at this position, and move pos to the next one
			CG1Neighbour* pNeighbour = (CG1Neighbour*)GetNext( pos );

			// If this neighbour is running a Gnutella program that supports pong caching
			if ( pNeighbour->m_nProtocol == PROTOCOL_G1 && pNeighbour->m_bPongCaching )
			{
				// Send a ping packet to it (do)
				pNeighbour->SendPing( dwNow, oGUID );
				Statistics.Current.Gnutella1.PingsSent++;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithG1 G1 pong handler

// CG1Neighbour::OnPong calls this when a remote computer has sent a pong packet, and we've added it to the Gnutella host cache
// Takes information from the pong packet
// Sends the pong to other remote computers we're connected to that need it according to their pong needed arrays
void CNeighboursWithG1::OnG1Pong(CG1Neighbour* pFrom, IN_ADDR* pAddress, WORD nPort, BYTE nHops, DWORD nFiles, DWORD nVolume)
{
	// Add the information from the pong packet to the pong cache (do)
	CPongItem* pPongCache = m_pPongCache->Add( pFrom, pAddress, nPort, nHops, nFiles, nVolume );
	if ( pPongCache == NULL ) return; // If Add didn't return a CPongItem, (do)

	CSingleLock pLock( &Network.m_pSection, TRUE );

	// Loop through each neighbour we're connected to
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		// Get the neighbour at this position, and move post forward
		CG1Neighbour* pNeighbour = (CG1Neighbour*)GetNext( pos );

		// If this neighbour is running Gnutella, and it's not the computer we got the pong packet from
		if ( pNeighbour->m_nProtocol == PROTOCOL_G1 && pNeighbour != pFrom )
		{
			// Send the pong to this remote computer, if it needs it according to its pong needed array
			pNeighbour->OnNewPong( pPongCache );
		}
	}
}
