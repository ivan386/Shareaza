//
// NeighboursWithED2K.h
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

// Add methods helpful for eDonkey2000 that look at the list of neighbours
// http://shareazasecurity.be/wiki/index.php?title=Developers.Code.CNeighboursWithED2K

// Make the compiler only include the lines here once, this is the same thing as pragma once
#if !defined(AFX_NEIGHBOURSWITHED2K_H__0165D8B7_0351_4EF7_B669_73D0072F5107__INCLUDED_)
#define AFX_NEIGHBOURSWITHED2K_H__0165D8B7_0351_4EF7_B669_73D0072F5107__INCLUDED_

// Only include the lines beneath this one once
#pragma once

// Copy in the contents of these files here before compiling
#include "NeighboursWithG2.h"

// Tell the compiler these classes exist, and it will find out more about them soon
class CEDNeighbour;
class CDownload;

// Add methods helpful for eDonkey2000 that use the list of connected neighbours
class CNeighboursWithED2K : public CNeighboursWithG2 // Continue the inheritance column CNeighbours : CNeighboursWithConnect : Routing : ED2K : G2 : G1 : CNeighboursBase
{

public:

	// Set up and clean up anything CNeighboursWithED2K adds to the CNeighbours class
	CNeighboursWithED2K(); // Zero the memory of the sources arrays
	virtual ~CNeighboursWithED2K();

public:

	// Get an eDonkey2000 neighbour from the list that's through the handshake and has a client ID
	CEDNeighbour* GetDonkeyServer() const;

	// Do things to all the eDonkey2000 computers we're connected to
	void CloseDonkeys();                           // Disconnect from all the eDonkey2000 computers we're connected to
	void SendDonkeyDownload(CDownload* pDownload); // Tell all the connected eDonkey2000 computers about pDownload

	// Send eDonkey2000 packets
	BOOL PushDonkey(DWORD nClientID, IN_ADDR* pServerAddress, WORD nServerPort); // Send a callback request packet
    BOOL FindDonkeySources(const Hashes::Ed2kHash& oED2K, IN_ADDR* pServerAddress, WORD nServerPort);

// Classes that inherit from this one can get to protected members, but unrelated classes can't
protected:

	// Hash arrays used by FindDonkeySources
	DWORD            m_tEDSources[256]; // 256 MD4 hashes
    Hashes::Ed2kHash m_oEDSources[256];
};

// End the group of lines to only include once, pragma once doesn't require an endif at the bottom
#endif // !defined(AFX_NEIGHBOURSWITHED2K_H__0165D8B7_0351_4EF7_B669_73D0072F5107__INCLUDED_)
