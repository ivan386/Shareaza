//
// Handshake.h
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

#if !defined(AFX_HANDSHAKE_H__FC762B48_46E6_4BB1_8B25_BC783DA966A4__INCLUDED_)
#define AFX_HANDSHAKE_H__FC762B48_46E6_4BB1_8B25_BC783DA966A4__INCLUDED_

#pragma once

// CHandshake inherits from CConnection
#include "Connection.h"

// OnRead needs the eDonkey2000 packet formats to see if the very first bytes the remote computer sent are one
class CEDPacket;

// Inherit from CConnection to get a socket and methods to send data through it
// CHandshake adds methods to figure out what network the other computer is on, and authenticate GIV and PUSH requests
class CHandshake : public CConnection
{

// Construction
public:

	// New
	CHandshake();									// Make a new object with null and default values
	CHandshake(SOCKET hSocket, SOCKADDR_IN* pHost);	// Make a new one with this socket and IP address
	CHandshake(CHandshake* pCopy);					// Make a new object that is a copy of this given one

	// Delete
	virtual ~CHandshake(); // The CConnection destructor handles putting things away

// Attributes
public:

	// Member variables for push operations and the Gnutella index (do)
	BOOL	m_bPushing;	// True if we connected to the remote computer as part of a push
	DWORD	m_nIndex;	// (do)
	
// Operations
public:

	// Push open a connection to the remote computer with the given IP address
	virtual BOOL	Push(IN_ADDR* pAddress, WORD nPort, DWORD nIndex);

protected:

	// Send a push request, and look at the very start of the handshake
	virtual BOOL	OnRun();				// Make sure the connection hasn't been going on for too long
	virtual BOOL	OnConnected();			// Send GIV to the other computer
	virtual void	OnDropped(BOOL bError);	// Record our push didn't work
	virtual BOOL	OnRead();				// Figure out the remote computer's network from the first 7 bytes it tells us

protected:

	// Receive push requests
	BOOL	OnAcceptGive();			// The remote computer said GIV, see if we were expecting it
	BOOL	OnAcceptPush();			// The remote computer said PUSH, see if we were expecting it
	BOOL	OnPush(GGUID* pGUID);	// See if a child window recognizes the guid
};

#endif // !defined(AFX_HANDSHAKE_H__FC762B48_46E6_4BB1_8B25_BC783DA966A4__INCLUDED_)
