//
// Connection.h
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

#if !defined(AFX_CONNECTION_H__6312EF26_B2C8_431F_93EF_243EA5E1A3DF__INCLUDED_)
#define AFX_CONNECTION_H__6312EF26_B2C8_431F_93EF_243EA5E1A3DF__INCLUDED_

#pragma once

class CBuffer;
class CConnection;

// Keep track of how fast we are reading or writing bytes to a socket
typedef struct
{
	// Options to limit bandwidth
	DWORD*	pLimit;			// Points to a DWORD that holds the limit for this bandwidth meter
	BOOL	bUnscaled;		// Set to true to worry about bandwidth even if the program settings don't

	// Transfer statistics
	DWORD	nTotal;			// The total number of bytes read or written
	DWORD	tLast;			// The time the last read or write happened
	DWORD	nMeasure;		// The average speed in bytes per second over the last 2 seconds

	// The arrays of byte counts and times
	DWORD	pHistory[64];	// 64 records of a number of bytes transferred
	DWORD	pTimes[64];		// The times each of these transfers happened
	DWORD	nPosition;		// The next spot in the array to use
	DWORD	tLastAdd;		// When we last recorded a transfer of some bytesThe last time something was recorded into pHistory and pTimes
	DWORD	tLastSlot;		// When we started using this time slot

} TCPBandwidthMeter;

// A socket connection to a remote compueter on the Internet running peer-to-peer software
class CConnection  
{

public:

	// Construction
	CConnection();			// Make a new CConnection object
	virtual ~CConnection();	// Delete a CConnection object

public:

	// The remote computer's IP address, who connected to the other, are we connected, and when it happened
	SOCKADDR_IN	m_pHost;		// The remote computer's IP address in Windows Sockets format
	CString		m_sAddress;		// The same IP address in a string like "1.2.3.4"
	BOOL		m_bInitiated;	// True if we initiated the connection, false if the remote computer connected to us
	BOOL		m_bConnected;	// True when the socket is connected
	DWORD		m_tConnected;	// The tick count when the connection was made

public:

	// The actual socket, buffers for reading and writing bytes, and some headers from the other computer
	SOCKET		m_hSocket;		// The actual Windows socket for the Internet connection to the remote computer
	CBuffer*	m_pInput;		// Pointers to input and output buffers
	CBuffer*	m_pOutput;
	CString		m_sUserAgent;	// The name of the program the remote computer is running
	CString		m_sLastHeader;	// The handshake header that ReadHeaders most recently read

public:

	// Structures to control bandwidth in each direction
	TCPBandwidthMeter	m_mInput;		// Input and output TCP bandwidth meters
	TCPBandwidthMeter	m_mOutput;
	int					m_nQueuedRun;	// The queued run state of 0, 1, or 2 (do)

public:

	// Make a connection, accept a connection, copy a connection, and close a connection
	virtual BOOL ConnectTo(SOCKADDR_IN* pHost);						// Connect to an IP address and port number
	virtual BOOL ConnectTo(IN_ADDR* pAddress, WORD nPort);
	virtual void AcceptFrom(SOCKET hSocket, SOCKADDR_IN* pHost);	// Accept a connection from a remote computer
	virtual void AttachTo(CConnection* pConnection);				// Copy a connection (do)
	virtual void Close();											// Disconnect from the remote computer

public:

	// Exchange data with the other computer, measure bandwidth, and work with headers
	BOOL	DoRun();			// Communicate with the other computer, reading and writing everything we can right now
	void	QueueRun();			// (do) may no longer be in use
	void	Measure();			// Measure the bandwidth, setting nMeasure in the bandwidth meters for each direction
	BOOL	ReadHeaders();		// Read text headers sitting in the input buffer
	BOOL	SendMyAddress();	// If we are listening on a port, tell the other computer our IP address and port number
	BOOL	IsAgentBlocked();	// Check the other computer's software title against our list of programs not to talk to
	
protected:

	// Read and write data through the socket, and look at headers
	virtual BOOL	OnRun();				// (do) just returns true
	virtual BOOL	OnConnected();			// (do) just returns true
	virtual BOOL	OnRead();				// Read data waiting in the socket into the input buffer
	virtual BOOL	OnWrite();				// Move the contents of the output buffer into the socket
	virtual void	OnDropped(BOOL bError);	// (do) empty
	virtual BOOL	OnHeaderLine(CString& strHeader, CString& strValue); // Processes a single line from the headers
	virtual BOOL	OnHeadersComplete();	// (do) just returns true

public:

	// Encode and decode URL text, and see if a string starts with a tag
	static CString	URLEncode(LPCTSTR pszInput);					// Encode "hello world" into "hello%20world"
	static CString	URLDecode(LPCTSTR pszInput);					// Decode "hello%20world" back to "hello world"
	static BOOL		StartsWith(LPCTSTR pszInput, LPCTSTR pszText);	// StartsWith("hello world", "hello") is true
};

#endif // !defined(AFX_CONNECTION_H__6312EF26_B2C8_431F_93EF_243EA5E1A3DF__INCLUDED_)
