//
// Handshakes.h
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

#if !defined(AFX_HANDSHAKES_H__2314A7BE_5C51_4F8E_A4C6_6059A7621AE0__INCLUDED_)
#define AFX_HANDSHAKES_H__2314A7BE_5C51_4F8E_A4C6_6059A7621AE0__INCLUDED_

#pragma once

// Tell the compiler that a class named CHandshake exists, and is defined in detail elsewhere
class CHandshake; // Lets methods defined here take and return pointers to CHandshake objects

class CHandshakes  
{

// Construction
public:

	// Make the CHandshakes object, and later, delete it
	CHandshakes();
	virtual ~CHandshakes();

// Attributes
public:

	// How many connections our listening socket has received, and how long they've been connected
	DWORD m_nStableCount; // The number of connections our listening socket has received
	DWORD m_tStableTime;  // The time at least one has been connected (do)

protected:

	// The listening socket
	SOCKET m_hSocket; // Our one listening socket
	HANDLE m_hThread; // This thread waits for remote computers to call the listening socket
	CEvent m_pWakeup; // Fire this event when a remote computer calls our listening socket

	// The list of CHandshake objects
	CPtrList			m_pList;	// The list of pointers to CHandshake objects
	CCriticalSection	m_pSection;	// Use to make sure only one thread accesses the list at a time

// Operations
public:

	// Start and stop listening on the socket
	BOOL Listen();		// Listen on the socket
	void Disconnect();	// Stop listening

	// Connect to an IP, and determine if we are connected to one
	BOOL PushTo(IN_ADDR* pAddress, WORD nPort, DWORD nIndex = 0);	// Connect to the given IP
	BOOL IsConnectedTo(IN_ADDR* pAddress);							// Looks for the IP in the handshake objects list

protected:

	// Replace and remove CHandshake objects in the m_pList of them
	void Substitute(CHandshake* pOld, CHandshake* pNew);	// Replace an old CHandshake object in the list with a new one
	void Remove(CHandshake* pHandshake);					// Remove a CHandshake object from the list

protected:

	// Loop to listen for connections and accept them
	static UINT	ThreadStart(LPVOID pParam);	// The thread that waits while the socket listens starts here
	void		OnRun();					// Accept incoming connections from remote computers
	void		RunHandshakes();			// Send and receive data with each remote computer in the list
	BOOL		AcceptConnection();			// Accept a connection, making a new CHandshake object in the list for it
	void		CreateHandshake(SOCKET hSocket, SOCKADDR_IN* pHost); // Make the new CHandshake object for the new connection
	void		RunStableUpdate();			// Update the discovery services (do)

	// Tell WSAAccept if we want to accept a connection from a computer that just called us
	static int	CALLBACK AcceptCheck(IN LPWSABUF lpCallerId, IN LPWSABUF lpCallerData, IN OUT LPQOS lpSQOS, IN OUT LPQOS lpGQOS, IN LPWSABUF lpCalleeId, OUT LPWSABUF lpCalleeData, OUT GROUP FAR * g, IN DWORD dwCallbackData);

// Short methods defined inline here in the .h file
public:

	// Returns an iterator at the start of the list of handshake objects
	inline POSITION GetIterator() const
	{
		// Returns a MFC POSITION iterator, or null if the list is empty
		return m_pList.GetHeadPosition();
	}

	// Given a position in the handshake list, returns the next handshake object
	inline CHandshake* GetNext(POSITION& pos) const
	{
		// Returns the CHandshake object at the current position, and then moves the position iterator to the next one
		return (CHandshake*)m_pList.GetNext( pos ); // Does two things
	}

	// True if the socket is valid, false if its closed
	inline BOOL IsListening() const
	{
		// If the socket is not invalid, it is connected to the remote computer
		return m_hSocket != INVALID_SOCKET;
	}

	// Get complete access to CHandhsake member variables and methods (do)
	friend class CHandshake;
};

// When the program runs, it makes a single global CHandshakes object
extern CHandshakes Handshakes; // Access Handshakes externally here in Handshakes.h, even though it is defined in Handshakes.cpp

#endif // !defined(AFX_HANDSHAKES_H__2314A7BE_5C51_4F8E_A4C6_6059A7621AE0__INCLUDED_)
