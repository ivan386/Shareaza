//
// Handshakes.h
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

#include "ThreadImpl.h"

class CHandshake;


class CHandshakes :
	public CThreadImpl
{
public:
	CHandshakes();
	virtual ~CHandshakes();

	BOOL Listen();						// Listen on the socket
	void Disconnect();					// Stop listening

	BOOL PushTo(IN_ADDR* pAddress, WORD nPort, DWORD nIndex = 0);	// Connect to the given IP
	BOOL IsConnectedTo(const IN_ADDR* pAddress) const;				// Looks for the IP in the handshake objects list

protected:
	DWORD m_nStableCount;				// The number of connections our listening socket has received
	DWORD m_tStableTime;				// The time at least one has been connected (do)
	SOCKET m_hSocket;					// Our one listening socket
	CList< CHandshake* > m_pList;		// The list of pointers to CHandshake objects
	mutable CMutex m_pSection;			// Use to make sure only one thread accesses the list at a time

	void Add(CHandshake* pHandshake);	// Add a CHandshake object to the list
	void Remove(CHandshake* pHandshake);// Remove a CHandshake object from the list
	void OnRun();						// Accept incoming connections from remote computers
	void RunHandshakes();				// Send and receive data with each remote computer in the list
	BOOL AcceptConnection();			// Accept a connection, making a new CHandshake object in the list for it
	void RunStableUpdate();				// Update the discovery services (do)

	// Tell WSAAccept if we want to accept a connection from a computer that just called us
	static int CALLBACK AcceptCheck(IN LPWSABUF lpCallerId, IN LPWSABUF lpCallerData, IN OUT LPQOS lpSQOS, IN OUT LPQOS lpGQOS, IN LPWSABUF lpCalleeId, IN LPWSABUF lpCalleeData, OUT GROUP FAR * g, IN DWORD_PTR dwCallbackData);

public:
	// True if the socket is valid, false if its closed
	inline BOOL IsValid() const throw()
	{
		return ( m_hSocket != INVALID_SOCKET );
	}

	// The time at least one has been connected (seconds)
	inline DWORD GetStableTime() const
	{
		const DWORD tNow = static_cast< DWORD >( time( NULL ) );
		return ( m_tStableTime && tNow > m_tStableTime ) ? ( tNow - m_tStableTime ) : 0;
	}
};

extern CHandshakes Handshakes;
