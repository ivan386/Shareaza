//
// Neighbour.h
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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

#if !defined(AFX_NEIGHBOUR_H__7B1C7637_2718_4D5F_B39D_28894CC0669D__INCLUDED_)
#define AFX_NEIGHBOUR_H__7B1C7637_2718_4D5F_B39D_28894CC0669D__INCLUDED_

#pragma once

#include "Connection.h"

class CBuffer;
class CPacket;
class CVendor;
class CGProfile;
class CQuerySearch;
class CQueryHashTable;

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

typedef enum NeighbourNodeEnum
{
	// The remote computer can be a leaf, or an ultrapeer or hub, and so can we
	ntNode, // We are both Gnutella ultrapeers or Gnutella2 hubs
	ntHub,  // We are a leaf, and this connection is to a Gnutella ultrapeer or Gnutella2 hub above us
	ntLeaf  // We are a Gnutella ultrapeer or Gnutella2 hub, and this connection is to a leaf below us

} NrsNode;

#define PONG_NEEDED_BUFFER 32

// Define the CNeighbour class to inherit from CConnection, picking up a socket and methods to connect it and read data through it
class CNeighbour : public CConnection
{

// Construction
public:

	CNeighbour(PROTOCOLID nProtocol);
	CNeighbour(PROTOCOLID nProtocol, CNeighbour* pBase);
	virtual ~CNeighbour();
	
// Attributes: State
public:

	DWORD		m_nRunCookie;
	DWORD		m_zStart;
	DWORD		m_nUnique;
	PROTOCOLID	m_nProtocol;
	NrsState	m_nState;			// Neighbour state, like connecting, handshake 1, 2, or 3, or rejected
	CVendor*	m_pVendor;
	BOOL		m_bGUID;
	GGUID		m_pGUID;
	CGProfile*	m_pProfile;
	GGUID*		m_pMoreResultsGUID;		//Last search GUID- used to get more results

// Attributes: Capabilities
public:

	BOOL		m_bAutomatic;
	BOOL		m_bShareaza;     // True if the remote computer is running Shareaza also
	NrsNode		m_nNodeType;     // This connection is to a hub above us, ntHub, a leaf below us, ntLeaf, or a hub just like us, ntNode
	BOOL		m_bQueryRouting;
	BOOL		m_bPongCaching;
	BOOL		m_bVendorMsg;
	BOOL		m_bGGEP;
	DWORD		m_tLastQuery;

// Attributes: Statistics
public:

	DWORD		m_nInputCount;
	DWORD		m_nOutputCount;
	DWORD		m_nDropCount;
	DWORD		m_nLostCount;
	DWORD		m_nOutbound;
	DWORD		m_nFileCount;
	DWORD		m_nFileVolume;

// Attributes: Query Hash Tables
public:

	CQueryHashTable*	m_pQueryTableRemote;
	CQueryHashTable*	m_pQueryTableLocal;

// Attributes: Internals
protected:

	DWORD		m_tLastPacket;	// The time that we received the last packet
	CBuffer*	m_pZInput;		// Buffers that hold compressed data just received
	CBuffer*	m_pZOutput;		//    and ready to send
	DWORD		m_nZInput;		// The number of compressed bytes in the input buffer
	DWORD		m_nZOutput;		//    and in the output buffer
	LPVOID		m_pZSInput;		// Access the Zlib library for decompressing input
	LPVOID		m_pZSOutput;	//    and compressing output
	BOOL		m_bZFlush;		// True to flush the compressed output buffer to the remote computer
	DWORD		m_tZOutput;		// The time that Zlib last compressed something

protected:

	DWORD		m_zEnd;
	
// Operations
public:

	virtual BOOL	Send(CPacket* pPacket, BOOL bRelease = TRUE, BOOL bBuffered = FALSE);
	virtual void	Close(UINT nError = IDS_CONNECTION_CLOSED);
	void			DelayClose(UINT nError = 0); // Send the buffer then close the socket, record the error given
	virtual BOOL	SendQuery(CQuerySearch* pSearch, CPacket* pPacket, BOOL bLocal);

protected:

	virtual BOOL	OnRun();
	virtual void	OnDropped(BOOL bError);
	virtual BOOL	OnRead();
	virtual BOOL	OnWrite();
	virtual BOOL	OnCommonHit(CPacket* pPacket);
	virtual BOOL	OnCommonQueryHash(CPacket* pPacket);

public:

	void	GetCompression(float* pnInRate, float* pnOutRate);
};

#endif // !defined(AFX_NEIGHBOUR_H__7B1C7637_2718_4D5F_B39D_28894CC0669D__INCLUDED_)
