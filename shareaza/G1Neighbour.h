//
// G1Neighbour.h
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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

// A CG1Neighbour object represents a remote computer running Gnutella software with which we are exchanging Gnutella packets
// http://shareazasecurity.be/wiki/index.php?title=Developers.Code.CG1Neighbour

#pragma once

#include "Neighbour.h"

class CG1Packet;
class CG1PacketBuffer;
class CPongItem;
class CGGEPItem;

// A CG1Neighbour object represents a remote computer running Gnutella software with which we are exchanging Gnutella packets
class CG1Neighbour : public CNeighbour // Inherit from CNeighbour and from that CConnection to get compression features and the connection socket
{

public:

	// Make a new CG1Neighbour object, and delete this one
	CG1Neighbour(CNeighbour* pBase); // Takes a pointer to a CShakeNeighbour object that determined the remote computer is running Gnutella
	virtual ~CG1Neighbour();

protected:

	// The tick count when something last happened
	DWORD m_tLastInPing;  // When the remote computer last sent us a ping packet
	DWORD m_tLastPingOut; // When we last sent a ping packet to the remote computer
	DWORD m_tClusterHost; // When we last called SendClusterAdvisor (do)
	DWORD m_tClusterSent; // When that method last sent a vendor specific cluster advisor packet

protected:

	// (do)
	BYTE m_nPongNeeded[PONG_NEEDED_BUFFER]; // This is just an array of 32 bytes

	// Information about the most recent ping packet the remote computer has sent us
	Hashes::Guid m_pLastPingID;   // The GUID of the most recent ping packet the remote computer has sent us
	BYTE  m_nLastPingHops; // The number of hops that packet has travelled, adding 1 (do)

	// A hops flow byte specific to BearShare (do)
	BYTE  m_nHopsFlow;

protected:

	// Holds the packets we are going to send to the remote computer
	CG1PacketBuffer* m_pOutbound;

public:

	// Send a packet to the remote computer
	virtual BOOL Send(CPacket* pPacket, BOOL bRelease = TRUE, BOOL bBuffered = FALSE);

	// Ping and Pong packets
	BOOL SendPing(const Hashes::Guid& oGUID = Hashes::Guid());
	void OnNewPong(const CPongItem* pPong);

	// Query packet
	virtual BOOL SendQuery(const CQuerySearch* pSearch, CPacket* pPacket, BOOL bLocal);

	// Push packet
	void SendG2Push(const Hashes::Guid& oGUID, CPacket* pPacket);

	// Process packets from specified buffer
	virtual BOOL ProcessPackets(CBuffer* pInput);

protected:

	// Send and recieve packets
	virtual BOOL OnRead();  // Read in data from the socket, decompress it, and call ProcessPackets
	virtual BOOL OnWrite(); // Sends all the packets from the outbound packet buffer to the remote computer
	virtual BOOL OnRun();   // Makes sure the remote computer hasn't been silent too long, and sends a ping every so often

protected:

	// Process packets from internal input buffer
	virtual BOOL ProcessPackets();

	BOOL OnPacket(CG1Packet* pPacket); // Sorts the packet and calls one of the methods below

	// Ping and pong packets
	BOOL OnPing(CG1Packet* pPacket);
	BOOL OnPong(CG1Packet* pPacket);

	// Bye packet
	BOOL OnBye(CG1Packet* pPacket);

	// Vendor specific packet
	BOOL OnVendor(CG1Packet* pPacket);

	// Push packet
	BOOL OnPush(CG1Packet* pPacket);

	// Query and query hit packets
	BOOL OnQuery(CG1Packet* pPacket);
	BOOL OnHit(CG1Packet* pPacket);

	// Cluster advisor vendor specific packet
	void SendClusterAdvisor();
	BOOL OnClusterAdvisor(CG1Packet* pPacket);
};
