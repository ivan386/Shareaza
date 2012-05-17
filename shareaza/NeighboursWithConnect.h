//
// NeighboursWithConnect.h
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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

#pragma once

#include "NeighboursWithRouting.h"

class CConnection;


class CNeighboursWithConnect : public CNeighboursWithRouting
{
protected:
	CNeighboursWithConnect();
	virtual ~CNeighboursWithConnect();

public:
	// All neighbours average incoming speed (bytes/second)
	DWORD BandwidthIn() const { return m_nBandwidthIn; }

	// All neighbours average outgoing speed (bytes/second)
	DWORD BandwidthOut() const { return m_nBandwidthOut; }

	// Connect to a computer at an IP address, and accept a connection from a computer that has connected to us
	CNeighbour* ConnectTo(const IN_ADDR& pAddress, WORD nPort, PROTOCOLID nProtocol, BOOL bAutomatic = FALSE, BOOL bNoUltraPeer = FALSE);
	BOOL OnAccept(CConnection* pConnection);

	virtual void Close();
	virtual void OnRun();

	// Determine our role on the Gnutella2 network
	bool  IsG2Leaf() const;							// Returns true if we are acting as a Gnutella2 leaf on at least one connection
	bool  IsG2Hub() const;							// Returns true if we are acting as a Gnutella2 hub on at least one connection
	DWORD IsG2HubCapable(BOOL bIgnoreTime = FALSE, BOOL bDebug = FALSE) const; // Returns true if we have a computer and Internet connection powerful enough to become a Gnutella2 hub

	// Determine our role on the Gnutella network
	bool  IsG1Leaf() const;							// Returns true if we are acting as a Gnutella leaf on at least one connection
	bool  IsG1Ultrapeer() const;					// Returns true if we are acting as a Gnutella ultrapeer on at least one connection
	DWORD IsG1UltrapeerCapable(BOOL bIgnoreTime = FALSE, BOOL bDebug = FALSE) const; // Returns true if we have a computer and Internet connection powerful enough to become a Gnutella ultrapeer

	// The number of connections we have older than 1.5 seconds and finished with the handshake
	DWORD GetStableCount() const { return m_nStableCount; }

	// The last time a neighbour connection attempt was made (in ticks)
	DWORD LastConnect() const { return m_tLastConnect; }

	// Determine our needs on the given network, Gnutella or Gnutella2
	bool NeedMoreHubs(PROTOCOLID nProtocol) const;	// Returns true if we need more hub connections on the given network
	bool NeedMoreLeafs(PROTOCOLID nProtocol) const;	// Returns true if we need more leaf connections on the given network
	//BOOL IsHubLoaded(PROTOCOLID nProtocol) const;	// Returns true if we have more than 75% of the number of hub connections settings says is our limit

	void PeerPrune(PROTOCOLID nProtocol);	// Close hub to hub connections when we get demoted to the leaf role (do)

private:
	DWORD m_nBandwidthIn;					// All neighbours average incoming speed (bytes/second)
	DWORD m_nBandwidthOut;					// All neighbours average outgoing speed (bytes/second)
	// Member variables that tell our current role on the Gnutella and Gnutella2 networks
	BOOL m_bG2Leaf;							// True if we are a leaf to at least one computer on the Gnutella2 network
	BOOL m_bG2Hub;							// True if we are a hub to at least one computer on the Gnutella2 network
	BOOL m_bG1Leaf;							// True if we are a leaf to at least one computer on the Gnutella network
	BOOL m_bG1Ultrapeer;					// True if we are an ultrapeer to at least one computer on the Gnutella network
	DWORD m_nStableCount;					// The number of connections we have older than 1.5 seconds and finished with the handshake
	DWORD m_tHubG2Promotion;				// Time we were promoted to a G2 hub (in seconds)
	DWORD m_tPresent[ PROTOCOL_LAST ];		// The time when we last connected to a hub for each network (in seconds)
	DWORD m_tPriority[ PROTOCOL_LAST ];		// The time when we last connected to priority server to make delay between priority and regular servers (in seconds)
	DWORD m_tLastConnect;					// The last time a neighbour connection attempt was made (in ticks)

	// Make new connections and close existing ones
	void MaintainNodeStatus();				// Determine our node status
	void Maintain();						// Count how many connections we have, and initiate or close them to match the ideal numbers in settings
	DWORD CalculateSystemPerformanceScore(BOOL bDebug) const;
};
