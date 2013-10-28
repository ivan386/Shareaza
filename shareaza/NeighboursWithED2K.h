//
// NeighboursWithED2K.h
//
// Copyright (c) Shareaza Development Team, 2002-2013.
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

// Only include the lines beneath this one once
#pragma once

#include "NeighboursWithG2.h"

class CEDNeighbour;
class CDownloadWithTiger;


// Add methods helpful for eDonkey2000 that use the list of connected neighbours
class CNeighboursWithED2K : public CNeighboursWithG2 // Continue the inheritance column CNeighbours : CNeighboursWithConnect : Routing : ED2K : G2 : G1 : CNeighboursBase
{
protected:
	CNeighboursWithED2K();
	virtual ~CNeighboursWithED2K();

public:
	DWORD				m_tLastED2KServerHop;	// The last time the ed2k server was changed due low ID (ticks)
	DWORD				m_nLowIDCount;			// Counts the amount of ed2k server low IDs we got (resets on high ID)

	virtual void OnRun();

	// Get an eDonkey2000 neighbour from the list that's through the handshake and has a client ID
	CEDNeighbour* GetDonkeyServer() const;

	// Do things to all the eDonkey2000 computers we're connected to
	void CloseDonkeys();                           // Disconnect from all the eDonkey2000 computers we're connected to
	void SendDonkeyDownload(const CDownloadWithTiger* pDownload); // Tell all the connected eDonkey2000 computers about pDownload

	// Send eDonkey2000 packets
	BOOL PushDonkey(DWORD nClientID, const IN_ADDR& pServerAddress, WORD nServerPort); // Send a callback request packet
    BOOL FindDonkeySources(const Hashes::Ed2kHash& oED2K, IN_ADDR* pServerAddress, WORD nServerPort);

// Classes that inherit from this one can get to protected members, but unrelated classes can't
protected:
	// Hash arrays used by FindDonkeySources
	DWORD				m_tEDSources[256];		// 256 MD4 hashes
    Hashes::Ed2kHash	m_oEDSources[256];

	void RunGlobalStatsRequests();
};
