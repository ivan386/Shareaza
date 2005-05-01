//
// G1Neighbour.h
//
// Copyright (c) Shareaza Development Team, 2002-2005.
// This file is part of SHAREAZA (www.shareaza.com)
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
// http://wiki.shareaza.com/static/Developers.Code.CG1Neighbour

// Make the compiler only include the lines here once, this is the same thing as pragma once
#if !defined(AFX_G1NEIGHBOUR_H__BF099C28_0FD5_4A9A_B36E_6490DA6FB62F__INCLUDED_)
#define AFX_G1NEIGHBOUR_H__BF099C28_0FD5_4A9A_B36E_6490DA6FB62F__INCLUDED_

// Only include the lines beneath this one once
#pragma once

// Copy in the contents of these files here before compiling
#include "Neighbour.h"

// Tell the compiler these classes exist, and it will find out more about them soon
class CG1Packet;
class CG1PacketBuffer;
class CPongItem;

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
	DWORD m_tLastOutPing; // When we last sent a ping packet to the remote computer
	DWORD m_tClusterHost; // When we last called SendClusterAdvisor (do)
	DWORD m_tClusterSent; // When that method last sent a vendor specific cluster advisor packet

protected:

	// (do)
	BYTE m_nPongNeeded[PONG_NEEDED_BUFFER]; // This is just an array of 32 bytes

	// Information about the most recent ping packet the remote computer has sent us
	GGUID m_pLastPingID;   // The GUID of the most recent ping packet the remote computer has sent us
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
	BOOL SendPing(DWORD dwNow = 0, GGUID* pGUID = NULL);
	void OnNewPong(CPongItem* pPong);

	// Query packet
	virtual BOOL SendQuery(CQuerySearch* pSearch, CPacket* pPacket, BOOL bLocal);

	// Push packet
	void SendG2Push(GGUID* pGUID, CPacket* pPacket);

protected:

	// Send and recieve packets
	virtual BOOL OnRead();  // Read in data from the socket, decompress it, and call ProcessPackets
	virtual BOOL OnWrite(); // Sends all the packets from the outbound packet buffer to the remote computer
	virtual BOOL OnRun();   // Makes sure the remote computer hasn't been silent too long, and sends a ping every so often

protected:

	// Read and respond to packets from the remote computer
	BOOL ProcessPackets();             // Cuts up the recieved data into packets, and calls OnPacket for each one
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

// End the group of lines to only include once, pragma once doesn't require an endif at the bottom
#endif // !defined(AFX_G1NEIGHBOUR_H__BF099C28_0FD5_4A9A_B36E_6490DA6FB62F__INCLUDED_)
