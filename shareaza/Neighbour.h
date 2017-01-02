//
// Neighbour.h
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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

class CBuffer;
class CPacket;
class CGProfile;
class CQuerySearch;
class CQueryHashTable;

#include "Connection.h"
#include "VendorCache.h"
#include "ZLibWarp.h"

// Keep track of what stage of communications we are in with the remote computer
typedef enum NeighbourStateEnum
{
	// One of these states describes what's happening with our connection to the remote computer right now
	nrsNull,       // No state recorded yet, the CNeighbour constructor sets m_nState to nrsNull
	nrsConnecting, // We called CConnection::ConnectTo, and are now waiting for the remote computer to do something
	nrsHandshake1, // We've finished sending a group of headers, and await the response
	nrsHandshake2, // We're reading the initial header group the remote computer has sent
	nrsHandshake3, // We're reading the final header group from the remote computer
	nrsRejected,   // The remote computer started with "GNUTELLA/0.6", but did not say "200 OK"
	nrsClosing,    // We called DelayClose to send buffered data and then close the socket connection
	nrsConnected   // The handshake is over, the CNeighbour copy constructor sets m_nState to nrsConnected

} NrsState;

// Record if the remote computer is in the same network role as us, or in a higher or lower one
typedef enum NeighbourNodeEnum
{
	// The remote computer can be a leaf, or an ultrapeer or hub, and so can we
	ntNode, // We are both Gnutella ultrapeers or Gnutella2 hubs
	ntHub,  // We are a leaf, and this connection is to a Gnutella ultrapeer or Gnutella2 hub above us
	ntLeaf  // We are a Gnutella ultrapeer or Gnutella2 hub, and this connection is to a leaf below us

} NrsNode;

// Make the m_nPongNeeded buffer an array of 32 bytes
const BYTE PONG_NEEDED_BUFFER = 32;

// Define the CNeighbour class to inherit from CConnection, picking up a socket and methods to connect it and read data through it
class CNeighbour : public CConnection
{

// Construction
protected:
	CNeighbour(PROTOCOLID nProtocol);
	CNeighbour(PROTOCOLID nProtocol, CNeighbour* pBase);
	virtual ~CNeighbour();

// Attributes: State
public:
	DWORD			m_nRunCookie;		// The number of times this neighbour has been run, CNeighboursBase::OnRun uses this to run each neighbour in the list once
	NrsState		m_nState;			// Neighbour state, like connecting, handshake 1, 2, or 3, or rejected
	CVendorPtr		m_pVendor;
	CString			m_sServerName;		// Server name primarily for eD2K and DC++ hubs
	Hashes::Guid	m_oGUID;
	CGProfile*		m_pProfile;
	Hashes::Guid	m_oMoreResultsGUID;	// GUID of the last search, used to get more results (do)

// Attributes: Capabilities
public:
	BOOL    m_bAutomatic;
	NrsNode m_nNodeType;       // This connection is to a hub above us, ntHub, a leaf below us, ntLeaf, or a hub just like us, ntNode
	BOOL    m_bQueryRouting;
	BOOL    m_bPongCaching;
	BOOL    m_bVendorMsg;      // True if the remote computer told us it supports vendor-specific messages
	BOOL    m_bGGEP;
	DWORD   m_tLastQuery;      // The time we last got a query packet, recorded as the number of seconds since 1970
	BOOL    m_bBadClient;		// Is the remote client running a 'bad' client- GPL rip, buggy, etc. (not banned, though)

	DWORD	m_nDegree;					// "X-Degree: n" (-1 if not set)
	DWORD	m_nMaxTTL;					// "X-Max-TTL: n" (-1 if not set)
	BOOL	m_bDynamicQuerying;			// "X-Dynamic-Querying: 0.1" (default: false)
	BOOL	m_bUltrapeerQueryRouting;	// "X-Ultrapeer-Query-Routing: 0.1" (default: false)
	CString	m_sLocalePref;				// "X-Locale-Pref: en" ("" if not set)
	BOOL	m_bRequeries;				// "X-Requeries: false" (default: true)
	BOOL	m_bExtProbes;				// "X-Ext-Probes: 0.1" (default: false)

// Attributes: Statistics
public:
	DWORD m_nInputCount;
	DWORD m_nOutputCount;
	DWORD m_nDropCount;
	DWORD m_nLostCount;
	DWORD m_nOutbound;

	// If the remote computer sends us a pong packet it made, copy the sharing statistics here
	DWORD m_nFileCount;  // The number of files the remote computer is sharing, according to the pong packet it sent us
	DWORD m_nFileVolume; // The total size of all of those files, according to the same pong packet

// Attributes: Query Hash Tables
public:
	CQueryHashTable* m_pQueryTableRemote;
	CQueryHashTable* m_pQueryTableLocal;

// Attributes: Internals
protected:
	DWORD		m_tLastPacket;	// The time that we received the last packet
	CBuffer*	m_pZInput;		// The remote computer is sending compressed data, we'll save it in m_pInput, and then decompress it to here
	CBuffer*	m_pZOutput;		// We are sending the remote computer compressed data, we're writing it here, and then compressing it to m_pOutput
	DWORD		m_nZInput;		// The number of decompressed bytes of data the remote computer sent us
	DWORD		m_nZOutput;		// The number of not yet compressed bytes of data we've sent the remote computer
	z_streamp	m_pZSInput;		// Pointer to the zlib z_stream structure for decompression
	z_streamp	m_pZSOutput;	// Pointer to the zlib z_stream structure for compression
	BOOL		m_bZFlush;		// True to flush the compressed output buffer to the remote computer
	DWORD		m_tZOutput;		// The time that Zlib last compressed something
	BOOL		m_bZInputEOS;	// Got End Of Stream while decompressing incoming data

// Operations
public:
	virtual BOOL	ConnectTo(const IN_ADDR* pAddress, WORD nPort, BOOL bAutomatic);
	virtual BOOL	Send(CPacket* pPacket, BOOL bRelease = TRUE, BOOL bBuffered = FALSE);
	virtual void	Close(UINT nError = IDS_CONNECTION_CLOSED);

	// Send the buffer then close the socket, record the error given
	virtual void	DelayClose(UINT nError);

	// Validate query
	virtual BOOL	SendQuery(const CQuerySearch* pSearch, CPacket* pPacket, BOOL bLocal);

	// Returns hub/server leaf/user count
	virtual DWORD	GetUserCount() const { return 0; }

	// Returns hub/server leaf/user limit
	virtual DWORD	GetUserLimit() const { return 0; }

	// Process packets from input buffer
	virtual BOOL	ProcessPackets(CBuffer* /*pInput*/) { return TRUE; }

protected:

	// Process packets from internal input buffer
	virtual BOOL	ProcessPackets() { return TRUE; }

	virtual BOOL OnRun();
	virtual void OnDropped();
	virtual BOOL OnRead();
	virtual BOOL OnWrite();
	virtual BOOL OnCommonHit(CPacket* pPacket);
	virtual BOOL OnCommonQueryHash(CPacket* pPacket);

public:
	// Get maximum TTL which is safe for both sides
	DWORD GetMaxTTL() const;
	// Calculate the average compression rate in either direction for this connection
	void GetCompression(float& nInRate, float& nOutRate) const;
};
