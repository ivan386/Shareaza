//
// Connection.cpp
//
//	Date:			"$Date: 2005/04/03 22:01:36 $"
//	Revision:		"$Revision: 1.20 $"
//  Last change by:	"$Author: rolandas $"
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "Connection.h"
#include "Network.h"
#include "Security.h"
#include "Buffer.h"
#include "Statistics.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define TEMP_BUFFER			10240
#define METER_SECOND		1000
#define METER_MINIMUM		100
#define METER_LENGTH		64		// Used to be 24, now 64
#define METER_PERIOD		6000	// Used to be 2000, now 6000

//////////////////////////////////////////////////////////////////////
// CConnection construction

// Make a new CConnection object
CConnection::CConnection()
{
	// Initialize member variables with null or default values
	m_hSocket		= INVALID_SOCKET;	// Set the actual Windows socket to the invalid value
	m_pInput		= NULL;				// Null pointers to input and output CBuffer objects
	m_pOutput		= NULL;
	m_bInitiated	= FALSE;			// This connection hasn't been initiated or connected yet
	m_bConnected	= FALSE;
	m_tConnected	= 0;				// This will be the tick count when the connection is made
	m_nQueuedRun	= 0;				// DoRun sets it to 1, QueueRun sets it to 2 (do)

	// Zero the memory of the input and output TCPBandwidthMeter objects
	ZeroMemory( &m_mInput, sizeof(m_mInput) );
	ZeroMemory( &m_mOutput, sizeof(m_mOutput) );
}

// Delete this CConnection object
CConnection::~CConnection()
{
	// Close the socket before the system deletes the object
	CConnection::Close();
}

//////////////////////////////////////////////////////////////////////
// CConnection connect to

// Connect this CConnection object to a remote computer on the Internet
// Takes pHost, a pointer to a SOCKADDR_IN structure, which is MFC's way of holding an IP address and port number
// Returns true if connected
BOOL CConnection::ConnectTo(SOCKADDR_IN* pHost)
{
	// Call the next ConnectTo method, and return the result
	return ConnectTo( &pHost->sin_addr, htons( pHost->sin_port ) );
}

// Connect this CConnection object to a remote computer on the Internet
// Takes pAddress, a Windows Sockets structure that holds an IP address, and takes the port number seprately
// Returns true if connected
BOOL CConnection::ConnectTo(IN_ADDR* pAddress, WORD nPort)
{
	// Check inputs
	if ( m_hSocket != INVALID_SOCKET )		return FALSE; // Make sure the socket isn't already connected somehow
	if ( pAddress == NULL || nPort == 0 )	return FALSE; // Make sure we have an address and a nonzero port number
	if ( pAddress->S_un.S_addr == 0 )		return FALSE; // S_un.S_addr is the IP as a single unsigned 4-byte long

	// The IP address is in the security list of government and corporate addresses we want to avoid
	if ( Security.IsDenied( pAddress ) )
	{
		// Report that we aren't connecting to this IP address and return false
		theApp.Message( MSG_ERROR, IDS_NETWORK_SECURITY_OUTGOING, (LPCTSTR)CString( inet_ntoa( *pAddress ) ) );
		return FALSE;
	}

	// The IN_ADDR structure we just got passed isn't the same as the one already stored in this object
	if ( pAddress != &m_pHost.sin_addr )
	{
		// Zero the memory of the entire SOCKADDR_IN structure m_pHost, and then copy in the sin_addr part
		ZeroMemory( &m_pHost, sizeof(m_pHost) );
		m_pHost.sin_addr = *pAddress;
	}

	// Fill in more parts of the m_pHost structure
	m_pHost.sin_family	= PF_INET;							// PF_INET means just normal IPv4, not IPv6 yet
	m_pHost.sin_port	= htons( nPort );					// Copy the port number into the m_pHost structure
	m_sAddress			= inet_ntoa( m_pHost.sin_addr );	// Save the IP address as a string of text

	// Create a socket and store it in m_hSocket
	m_hSocket = socket(
		PF_INET,		// Normal IPv4, not IPv6
		SOCK_STREAM,	// The two-way sequenced reliable byte streams of TCP, not the datagrams of UDP
		IPPROTO_TCP );	// Again, we want TCP

	// Choose asynchronous, non-blocking reading and writing on our new socket
	DWORD dwValue = 1;
	ioctlsocket(	// Call Windows Sockets ioctlsocket to control the input/output mode of our new socket
		m_hSocket,	// Give it our new socket
		FIONBIO,	// Select the option for blocking i/o, should the program wait on read and write calls, or keep going?
		&dwValue ); // Nonzero, it should keep going

	// If the OutHost string in connection settings has an IP address written in it
	if ( Settings.Connection.OutHost.GetLength() )
	{
		// Read the text and copy the IP address and port into a new local MFC SOCKADDR_IN structure called pOutgoing
		SOCKADDR_IN pOutgoing;
		Network.Resolve( Settings.Connection.OutHost, 0, &pOutgoing );

		// S_addr is the IP address as a single long number, if it's not zero
		if ( pOutgoing.sin_addr.S_un.S_addr )
		{
			// Call bind in Windows Sockets to associate the local address with the socket
			bind(
				m_hSocket,				// Our socket
				(SOCKADDR*)&pOutgoing,	// The IP address this computer appears to have on the Internet (do)
				sizeof(SOCKADDR_IN) );	// Tell bind how many bytes it can read at the pointer
		}
	}

	// Try to connect to the remote computer
	if ( WSAConnect(
		m_hSocket,					// Our socket
		(SOCKADDR*)&m_pHost,		// The remote IP address and port number
		sizeof(SOCKADDR_IN),		// How many bytes the function can read
		NULL, NULL, NULL, NULL ) )	// No advanced features
	{
		// If no error occurs, WSAConnect returns 0, so if we're here an error happened
		UINT nError = WSAGetLastError(); // Get the last Windows Sockets error number

		// An error of "would block" is normal because connections can't be made instantly and this is a non-blocking socket
		if ( nError != WSAEWOULDBLOCK )
		{
			// The error is something else, record it, close the socket, set the value of m_hSocket, and leave
			theApp.Message( MSG_DEBUG, _T("connect() error 0x%x"), nError );
			closesocket( m_hSocket );
			m_hSocket = INVALID_SOCKET;
			return FALSE;
		}
	}

	// Delete the input and output CBuffer objects if they exist, and then create new ones
	if ( m_pInput ) delete m_pInput;
	if ( m_pOutput ) delete m_pOutput;
	m_pInput		= new CBuffer( &Settings.Bandwidth.PeerIn );
	m_pOutput		= new CBuffer( &Settings.Bandwidth.PeerOut );

	// Record that we initiated this connection, and when it happened
	m_bInitiated	= TRUE;
	m_tConnected	= GetTickCount();

	// Record one more outgoing connection in the statistics
	Statistics.Current.Connections.Outgoing++;

	// The connection was successfully attempted
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CConnection accept an incoming connection

// Called when a remote computer wants to connect to us
// When WSAAccept accepted the connection, it created a new socket hSocket for it and wrote the remote IP in pHost
void CConnection::AcceptFrom(SOCKET hSocket, SOCKADDR_IN* pHost)
{
	// Make sure the newly accepted socket is valid
	ASSERT( m_hSocket == INVALID_SOCKET );

	// Record the connection information here
	m_hSocket		= hSocket;							// Keep the socket here
	m_pHost			= *pHost;							// Copy the remote IP address into this object
	m_sAddress		= inet_ntoa( m_pHost.sin_addr );	// Store it as a string also

	// Make new input and output buffer objects
	m_pInput		= new CBuffer( &Settings.Bandwidth.PeerIn );
	m_pOutput		= new CBuffer( &Settings.Bandwidth.PeerOut );

	// Facts about the connection
	m_bInitiated	= FALSE;			// We didn't initiate this connection
	m_bConnected	= TRUE;				// We're connected right now
	m_tConnected	= GetTickCount();	// Record the time this happened

	// Choose asynchronous, non-blocking reading and writing on the new socket
	DWORD dwValue = 1;
	ioctlsocket( m_hSocket, FIONBIO, &dwValue );

	// Record one more incoming connection in the statistics
	Statistics.Current.Connections.Incoming++;
}

//////////////////////////////////////////////////////////////////////
// CConnection attach to an existing connection

// Call to copy a connection object to this one (do)
// Takes another connection object
void CConnection::AttachTo(CConnection* pConnection)
{
	// Make sure the socket isn't valid yet
	ASSERT( m_hSocket == INVALID_SOCKET );				// Make sure the socket here isn't valid yet
	ASSERT( pConnection != NULL );						// Make sure we got a CConnection object
	ASSERT( pConnection->m_hSocket != INVALID_SOCKET ); // And make sure its socket exists

	// Copy values from the given CConnection object to this one
	m_pHost			= pConnection->m_pHost;
	m_sAddress		= pConnection->m_sAddress;
	m_hSocket		= pConnection->m_hSocket;
	m_bInitiated	= pConnection->m_bInitiated;
	m_bConnected	= pConnection->m_bConnected;
	m_tConnected	= pConnection->m_tConnected;
	m_pInput		= pConnection->m_pInput;
	m_pOutput		= pConnection->m_pOutput;
	m_sUserAgent	= pConnection->m_sUserAgent; // But, we don't also copy across m_mInput and m_mOutput

	// Record the current time in the input and output TCP bandwidth meters
	m_mInput.tLast = m_mOutput.tLast = GetTickCount();

	// Invalidate the socket in the given connection object so it's just here now
	pConnection->m_hSocket	= INVALID_SOCKET;

	// Null the input and output pointers
	pConnection->m_pInput	= NULL;
	pConnection->m_pOutput	= NULL;

	// Zero the memory of the input and output TCPBandwidthMeter objects
	ZeroMemory( &pConnection->m_mInput, sizeof(m_mInput) );
	ZeroMemory( &pConnection->m_mOutput, sizeof(m_mOutput) );
}

//////////////////////////////////////////////////////////////////////
// CConnection close

// Call to close the connection represented by this object
void CConnection::Close()
{
	// Make sure this object exists, and is located entirely within the program's memory space
	ASSERT( this != NULL );
	ASSERT( AfxIsValidAddress( this, sizeof(*this) ) );

	// The socket is valid
	if ( m_hSocket != INVALID_SOCKET )
	{
		// Close it and mark it invalid
		closesocket( m_hSocket );
		m_hSocket = INVALID_SOCKET;
	}

	// Delete and mark null the input and output buffers
	if ( m_pOutput ) delete m_pOutput;
	m_pOutput = NULL;
	if ( m_pInput ) delete m_pInput;
	m_pInput = NULL;

	// This connection object isn't connected any longer
	m_bConnected = FALSE;
}

//////////////////////////////////////////////////////////////////////
// CConnection run function

// Talk to the other computer, write the output buffer to the socket and read from the socket to the input buffer
// Return true if this worked, false if we've lost the connection
BOOL CConnection::DoRun()
{
	// If this socket is invalid, call OnRun and return the result (do)
	if ( m_hSocket == INVALID_SOCKET ) return OnRun();

	// Setup pEvents to store the socket's internal information about network events
	WSANETWORKEVENTS pEvents;
	WSAEnumNetworkEvents( m_hSocket, NULL, &pEvents );

	// If the FD_CONNECT network event has occurred
	if ( pEvents.lNetworkEvents & FD_CONNECT )
	{
		// If there is a nonzero error code for the connect operation
		if ( pEvents.iErrorCode[ FD_CONNECT_BIT ] != 0 )
		{
			// This connection was dropped
			OnDropped( TRUE ); // Calls CShakeNeighbour::OnDropped
			return FALSE;
		}

		// The socket is now connected
		m_bConnected = TRUE;
		m_tConnected = m_mInput.tLast = m_mOutput.tLast = GetTickCount(); // Store the time 3 places

		// Call CShakeNeighbour::OnConnected to start reading the handshake
		if ( ! OnConnected() ) return FALSE;
	}

	// If the FD_CLOSE network event has occurred, set bClosed to true, otherwise set it to false
	BOOL bClosed = ( pEvents.lNetworkEvents & FD_CLOSE ) ? TRUE : FALSE;

	// If the close event happened, null a pointer within the TCP bandwidth meter for input (do)
	if ( bClosed ) m_mInput.pLimit = NULL;

	// Change the queued run state to 1 (do)
	m_nQueuedRun = 1;

	// Write the contents of the output buffer to the remote computer, and read in data it sent us
	if ( ! OnWrite() ) return FALSE; // If this is a CShakeNeighbour object, calls CShakeNeighbour::OnWrite
	if ( ! OnRead() ) return FALSE;

	// If the close event happened
	if ( bClosed )
	{
		// Call OnDropped, telling it true if there is a close error
		OnDropped( pEvents.iErrorCode[ FD_CLOSE_BIT ] != 0 ); // True if there is an nonzero error code for the close bit
		return FALSE;
	}

	// Make sure the handshake doesn't take too long
	if ( ! OnRun() ) return FALSE; // If this is a CShakeNeighbour object, calls CShakeNeighbour::OnRun

	// If the queued run state is 2 and OnWrite returns false, leave here with false also
	if ( m_nQueuedRun == 2 && ! OnWrite() ) return FALSE;

	// Change the queued run state back to 0 and report success (do)
	m_nQueuedRun = 0;
	return TRUE;
}

// (do)
void CConnection::QueueRun()
{
	// If the queued run state is 1 or 2, make it 2, if it's 0, call OnWrite now (do)
	if ( m_nQueuedRun )	m_nQueuedRun = 2;	// The queued run state is not 0, make it 2 and do nothing else here
	else				OnWrite();			// The queued run state is 0, do a write (do)
}

//////////////////////////////////////////////////////////////////////
// CConnection socket event handlers

BOOL CConnection::OnConnected()
{
	return TRUE;
}

void CConnection::OnDropped(BOOL bError)
{

}

BOOL CConnection::OnRun()
{
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CConnection read event handler

// Read data waiting in the socket into the input buffer
BOOL CConnection::OnRead()
{
	// Make sure the socket is valid
	if ( m_hSocket == INVALID_SOCKET ) return FALSE;

	// Setup local variables
	BYTE pData[TEMP_BUFFER];			// Make a 4 KB data buffer called pData
	DWORD tNow		= GetTickCount();	// The time right now
	DWORD nLimit	= 0xFFFFFFFF;		// Make the limit huge
	DWORD nTotal	= 0;				// Start the total at 0

	// If we need to worry about throttling bandwidth, calculate nLimit, the number of bytes we are allowed to read now
	if ( m_mInput.pLimit							// If the input bandwidth memter points to a limit
		&& *m_mInput.pLimit							// And that limit isn't 0
		&& ( Settings.Live.BandwidthScale <= 100	// And either the bandwidth meter in program settings is (do)
		|| m_mInput.bUnscaled ) )					// Or the input bandwidth meter in this object is unscaled (do)
	{
		// tCutoff is the tick count 1 second ago
		DWORD tCutoff = tNow - METER_SECOND; // METER_SECOND is 1 second

		// nLimit is the number of bytes we've read in the last second
		// This part of the code has been translated into assembly language to make it faster
/*
		// Loop across the first 24 times and histories stored in the input bandwidth meter
		DWORD* pHistory	= m_mInput.pHistory;	// Start the pointers at the beginning of the arrays
		DWORD* pTime	= m_mInput.pTimes;
		nLimit			= 0;					// Count up the limit from 0
		for ( int nSeek = METER_LENGTH ; nSeek ; nSeek--, pHistory++, pTime++ )
		{
			// If this time is within the last second, add its bytes to the limit
		    if ( *pTime >= tCutoff ) nLimit += *pHistory;
		}
*/
		// Here is the assembly language which does the same thing
		__asm
		{
			mov		ecx, this
			mov		ebx, -METER_LENGTH
			mov		edx, tCutoff
			xor		eax, eax
_loop:		cmp		edx, [ ecx + ebx * 4 ]CConnection.m_mInput.pTimes + METER_LENGTH * 4
			jnbe	_ignore
			add		eax, [ ecx + ebx * 4 ]CConnection.m_mInput.pHistory + METER_LENGTH * 4
_ignore:	inc		ebx
			jnz		_loop
			mov		nLimit, eax
		}

		// nActual is the speed limit, set by the input bandwidth meter and the bandwidth scale in settings
		DWORD nActual = *m_mInput.pLimit; // Get the speed limit from the input bandwidth meter
		if ( Settings.Live.BandwidthScale < 100 && ! m_mInput.bUnscaled ) // The scale is turned down and we should use it
		{
			// Adjust actual based on the scale
			nActual = nActual * Settings.Live.BandwidthScale / 100; // If the settings scale is 50, this cuts actual in half
		}

		// tCutoff is the number of bytes we can read now, multiply the speed limit with the elapsed time to get it
		tCutoff = nActual * ( tNow - m_mInput.tLastAdd ) / 1000; // Subtract tick counts and divide by 1000 to get seconds
		m_mInput.tLastAdd = tNow; // Record that the last add in the input bandwidth meter happened now

		// nActual is the speed limit in bytes per second
		// nLimit is how many bytes we read in the last second
		// So, nActual - nLimit is the number of bytes we can still read this second
		// Set nLimit to this, or 0 if we're over the limit
		nLimit = ( nLimit >= nActual ) ? 0 : ( nActual - nLimit );

		// If the cutoff is even more restrictive than the limit, make it the limit instead
		// nLimit = speed limit in bytes per second - bytes we read in the last second
		// tCutoff = speed limit in bytes per second * elapsed time
		nLimit = min( nLimit, tCutoff );
	}

	// Loop until the limit has run out
	while ( nLimit )
	{
		// Set nLength to nLimit or 4 KB, whichever is smaller
		int nLength = min( ( nLimit & 0xFFFFF ), DWORD(TEMP_BUFFER) );

		// Read the bytes from the socket
		nLength = recv(		// nLength is the number of bytes we received from the socket
			m_hSocket,		// Use the socket in this CConnection object
			(char*)pData,	// Tell recv to write the data here
			nLength,		// The size of the buffer, and how many bytes recv should write there
			0 );			// No special options
		if ( nLength <= 0 ) break; // Loop until nothing is left or an error occurs

		// Record the time of this read in the input bandwidth meter
		m_mInput.tLast = tNow;

		// If the length is positive and up to 4 KB
		if ( nLength > 0 && nLength <= TEMP_BUFFER )
		{
			// Add the data to the input buffer in the connection object
			m_pInput->Add( pData, nLength );
		}

		// Include these new bytes in the total
		nTotal += nLength;

		// If we are keeping track of the limit, subtract the bytes we just read from it
		if ( nLimit != 0xFFFFFFFF ) nLimit -= nLength;
	}

	// If we are keeping track of history and we just read some bytes
	if ( m_mInput.pHistory && nTotal )
	{
		// If it's less than 1/10 of a second since we last wrote some history information
		if ( tNow - m_mInput.tLastSlot < METER_MINIMUM )
		{
			// Use the same place in the array as before
			m_mInput.pHistory[ m_mInput.nPosition ] += nTotal;

		} // It's been more than a tenth of a second since we last recorded a read
		else
		{
			// Store the time and total in a new array slot
			m_mInput.nPosition = ( m_mInput.nPosition + 1 ) % METER_LENGTH;	// Move to the next position in the array
			m_mInput.pTimes[ m_mInput.nPosition ] = tNow;					// Record the new time
			m_mInput.pHistory[ m_mInput.nPosition ]	= nTotal;				// Store the bytes read next to it
			m_mInput.tLastSlot = tNow;										// We just wrote some history information
		}
	}

	// Add the bytes we read to the total in the bandwidth meter and the total in statistics
	m_mInput.nTotal += nTotal;
	Statistics.Current.Bandwidth.Incoming += nTotal;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CConnection write event handler

// Call to send the contents of the output buffer to the remote computer
BOOL CConnection::OnWrite()
{
	// Make sure the socket is valid and there is something to send
	if ( m_hSocket == INVALID_SOCKET ) return FALSE;
	if ( m_pOutput->m_nLength == 0 ) return TRUE; // There is nothing to send, so we succeed without doing anything

	// Setup local variables
	DWORD tNow		= GetTickCount();	// The time right now
	DWORD nLimit	= 0xFFFFFFFF;		// Make the limit huge

	// If we need to worry about throttling bandwidth, calculate nLimit, the number of bytes we are allowed to write now
	if ( m_mOutput.pLimit							// If the output bandwidth meter points to a limit
		&& *m_mOutput.pLimit						// And that limit isn't 0
		&& ( Settings.Live.BandwidthScale <= 100	// And either the bandwidth meter in program settings is (do)
		|| m_mOutput.bUnscaled ) )					// Or the input bandwidth meter in this object is unscaled (do)
	{
		// tCutoff is the tick count 1 second ago
		DWORD tCutoff = tNow - METER_SECOND; // METER_SECOND is 1 second

		// nUsed is the number of bytes we've written in the last second
		// This part of the code has been translated into assembly language to make it faster
/*
		// Loop across the first 24 times and histories stored in the output bandwidth meter
		DWORD* pHistory	= m_mOutput.pHistory;	// Start the pointers at the beginning of the arrays
		DWORD* pTime	= m_mOutput.pTimes;
		DWORD nUsed		= 0;					// Count up used from 0
		for ( int nSeek = METER_LENGTH ; nSeek ; nSeek--, pHistory++, pTime++ )
		{
			// If this time is within the last second, add its bytes to used
			if ( *pTime >= tCutoff ) nUsed += *pHistory;
		}
*/
		// Here is the assembly language which does the same thing
		DWORD nUsed;
		__asm
		{
			mov		ecx, this
			mov		ebx, -METER_LENGTH
			mov		edx, tCutoff
			xor		eax, eax
_loop:		cmp		edx, [ ecx + ebx * 4 ]CConnection.m_mOutput.pTimes + METER_LENGTH * 4
			jnbe	_ignore
			add		eax, [ ecx + ebx * 4 ]CConnection.m_mOutput.pHistory + METER_LENGTH * 4
_ignore:	inc		ebx
			jnz		_loop
			mov		nUsed, eax
		}

		// nLimit is the speed limit, set by the output bandwidth meter and the bandwidth scale in settings
		nLimit = *m_mOutput.pLimit; // Get the speed limit from the output bandwidth meter
		if ( Settings.Live.BandwidthScale < 100 && ! m_mOutput.bUnscaled ) // The scale is turned down and we should use it
		{
			// Adjust the limit lower based on the scale
			nLimit = nLimit * Settings.Live.BandwidthScale / 100;
		}

		// The program is running in throttle mode
		if ( Settings.Uploads.ThrottleMode )
		{
			// Set nLimit to the remaining bytes we're allowd to write this second
			nLimit = ( nUsed >= nLimit ) ? 0 : ( nLimit - nUsed );

		} // The program is not running in throttle mode
		else
		{
			// tCutoff is the number of bytes we can write now, multiply the speed limit with the elapsed time to get it
			tCutoff = nLimit * ( tNow - m_mOutput.tLastAdd ) / 1000;

			// Set nLimit to the remaining bytes we're allowd to write this second
			nLimit = ( nUsed >= nLimit ) ? 0 : ( nLimit - nUsed );

			// If the cutoff is even more restrictive than the limit, make it the limit instead
			// nLimit = speed limit in bytes per second - bytes we read in the last second
			// tCutoff = speed limit in bytes per second * elapsed time
			nLimit = min( nLimit, tCutoff );

			// Record that the last add in the input bandwidth meter happened now
			m_mOutput.tLastAdd = tNow;
		}
	}

	// Point pBuffer at the start of the output buffer, and set nBuffer to its length
	BYTE* pBuffer = m_pOutput->m_pBuffer;
	DWORD nBuffer = m_pOutput->m_nLength;

	// Loop while we're under our limit and there are still bytes to write
	while ( nLimit && nBuffer )
	{
		// nLength is the number of bytes we will write, set it to the limit or the length, whichever is smaller
		int nLength = (int)min( nLimit, nBuffer );

		// Send the bytes to the other computer through the socket
		nLength = send(		// The send function returns how many bytes it sent
			m_hSocket,		// Use the socket in this CConnection object
			(char*)pBuffer,	// Tell send to read the data here
			nLength,		// This is how many bytes are there to send
			0 );			// No special options
		if ( nLength <= 0 ) break; // Loop until nothing is left or an error occurs

		// Move the pointer forward past the sent bytes, and subtract their size from the length
		pBuffer += nLength;
		nBuffer -= nLength;

		// If we are keeping track of bandwidth, subtract the size of what we wrote from the limit
		if ( nLimit != 0xFFFFFFFF ) nLimit -= nLength;
	}

	// The total number of bytes written is the length of the output buffer minus any bytes left still to write
	DWORD nTotal = ( m_pOutput->m_nLength - nBuffer );

	// We wrote some bytes into the socket
	if ( nTotal )
	{
		// Remove them from the output buffer, otherwise they would sit there and get sent again the next time
		m_pOutput->Remove( nTotal );
		m_mOutput.tLast = tNow; // Record that we last wrote now

		// If it's less than 1/10 of a second since we last wrote some bandwidth history information
		if ( tNow - m_mOutput.tLastSlot < METER_MINIMUM )
		{
			// Just add the bytes in the same time slot as before
			m_mOutput.pHistory[ m_mOutput.nPosition ] += nTotal;

		} // It's been more than a tenth of a second since we last recorded a write
		else
		{
			// Store the time and total in a new array slot
			m_mOutput.nPosition = ( m_mOutput.nPosition + 1 ) % METER_LENGTH;	// Move to the next position in the array
			m_mOutput.pTimes[ m_mOutput.nPosition ] = tNow;						// Record the new time
			m_mOutput.pHistory[ m_mOutput.nPosition ] = nTotal;					// Store the bytes written next to it
			m_mOutput.tLastSlot = tNow;											// Mark down when we did this
		}

		// Add the bytes we read to the total in the bandwidth meter and the total in statistics
		m_mOutput.nTotal += nTotal;
		Statistics.Current.Bandwidth.Outgoing += nTotal;
	}

	// Report success
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CConnection measure

// Calculate the input and output speeds for this connection
void CConnection::Measure()
{
	// Set tCutoff to the tick count exactly 2 seconds ago
	DWORD tCutoff = GetTickCount() - METER_PERIOD; // METER_PERIOD is 2 seconds

	// This part of the code has been translated into assembly language to make it faster
/*
	// Point to the start of the input and output history and time arrays
	DWORD* pInHistory	= m_mInput.pHistory;
	DWORD* pInTime		= m_mInput.pTimes;
	DWORD* pOutHistory	= m_mOutput.pHistory;
	DWORD* pOutTime		= m_mOutput.pTimes;

	// Start counts at zero
	DWORD nInput	= 0;
	DWORD nOutput	= 0;

	// Loop down the arrays
	for ( int tNow = METER_LENGTH ; tNow ; tNow-- )
	{
		// If this record is from the last 2 seconds, add it to the input or output total
		if ( *pInTime >= tCutoff ) nInput += *pInHistory;
		if ( *pOutTime >= tCutoff ) nOutput += *pOutHistory;

		// Move pointers forward to the next spot in the array
		pInHistory++, pInTime++;
		pOutHistory++, pOutTime++;
	}

	// Set nMeasure for input and output as the average bytes read and written per second over the last 2 seconds
	m_mInput.nMeasure  = nInput  * 1000 / METER_PERIOD;
	m_mOutput.nMeasure = nOutput * 1000 / METER_PERIOD;
*/
	// Here is the assembly language which does the same thing
	__asm
	{
		mov		ebx, this
		mov		edx, tCutoff
		xor		eax, eax
		xor		esi, esi
		mov		ecx, -METER_LENGTH
_loop:	cmp		edx, [ ebx + ecx * 4 ]CConnection.m_mInput.pTimes + METER_LENGTH * 4
		jnbe	_ignoreIn
		add		eax, [ ebx + ecx * 4 ]CConnection.m_mInput.pHistory + METER_LENGTH * 4
_ignoreIn:cmp	edx, [ ebx + ecx * 4 ]CConnection.m_mOutput.pTimes + METER_LENGTH * 4
		jnbe	_ignoreOut
		add		esi, [ ebx + ecx * 4 ]CConnection.m_mOutput.pHistory + METER_LENGTH * 4
_ignoreOut:inc	ecx
		jnz		_loop
		xor		edx, edx
		mov		ecx, METER_PERIOD / 1000
		div		ecx
		mov		[ ebx ]CConnection.m_mInput.nMeasure, eax
		xor		edx, edx
		mov		eax, esi
		div		ecx
		mov		[ ebx ]CConnection.m_mOutput.nMeasure, eax
	}
}

//////////////////////////////////////////////////////////////////////
// CConnection HTML header reading

// Remove the headers from the input buffer, handing each to OnHeaderLine
BOOL CConnection::ReadHeaders()
{
	// Move the first line from the m_pInput buffer to strLine and do the contents of the while loop
	CString strLine;
	while ( m_pInput->ReadLine( strLine ) ) // ReadLine will return false when there are no more lines
	{
		// If the line is more than 20 KB, change it to the line too long error code 
		if ( strLine.GetLength() > 20480 ) strLine = _T("#LINE_TOO_LONG#");

		// Find the first colon in the line
		int nPos = strLine.Find( _T(":") );

		// The line is empty, it's just a \n character
		if ( strLine.IsEmpty() )
		{
			// Empty the last header member variable (do)
			m_sLastHeader.Empty();

			// Call the OnHeadersComplete method for the most advanced class that inherits from CConnection
			return OnHeadersComplete(); // Calls CShakeNeighbour::OnHeadersComplete()

		} // The line starts with a space
		else if ( _istspace( strLine.GetAt( 0 ) ) ) // Get the first character in the string, and see if its a space
		{
			// The last header has length
			if ( m_sLastHeader.GetLength() )
			{
				// Trim the spaces from both ends of the line, and see if it still has length
				strLine.TrimLeft();
				strLine.TrimRight();
				if ( strLine.GetLength() > 0 )
				{
					// Give OnHeaderLine the last header and this line
					if ( ! OnHeaderLine( m_sLastHeader, strLine ) ) return FALSE;
				}
			}

		} // The colon is at a distance greater than 1 and less than 64
		else if ( nPos > 1 && nPos < 64 ) // ":a" is 0 and "a:a" is 1, but "aa:a" is greater than 1
		{
			// The line is like "header:value", copy out both parts
			m_sLastHeader		= strLine.Left( nPos );
			CString strValue	= strLine.Mid( nPos + 1 );

			// Trim spaces from both ends of the value, and see if it still has length
			strValue.TrimLeft();
			strValue.TrimRight();
			if ( strValue.GetLength() > 0 )
			{
				// Give OnHeaderLine this last header, and its value
				if ( ! OnHeaderLine( m_sLastHeader, strValue ) ) return FALSE; // Calls CShakeNeighbour::OnHeaderLine
			}
		}
	}

	// Send the contents of the output buffer to the remote computer
	OnWrite();
	return TRUE;
}

// Takes a header and its value
// Reads and processes popular Gnutella headers
// Returns true to have ReadHeaders keep going
BOOL CConnection::OnHeaderLine(CString& strHeader, CString& strValue)
{
	// It's the user agent header
	if ( strHeader.CompareNoCase( _T("User-Agent") ) == 0 )
	{
		// Copy the value into the user agent member string
		m_sUserAgent = strValue; // This tells what software the remote computer is running
		return TRUE;             // Have ReadHeaders keep going
	
	} // It's the remote IP header
	else if ( strHeader.CompareNoCase( _T("Remote-IP") ) == 0 )
	{
		// Add this address to our record of them
		Network.AcquireLocalAddress( strValue );
	
	} // It's the x my address, listen IP, or node header, like "X-My-Address: 10.254.0.16:6349"
	else if (	strHeader.CompareNoCase( _T("X-My-Address") ) == 0 ||
				strHeader.CompareNoCase( _T("Listen-IP") ) == 0 ||
				strHeader.CompareNoCase( _T("Node") ) == 0 )
	{
		// Find another colon in the value
		int nColon = strValue.Find( ':' );

		// If the remote computer first contacted us and the colon is there but not first
		if ( ! m_bInitiated && nColon > 0 )
		{
			// Read the number after the colon into nPort
			int nPort = GNUTELLA_DEFAULT_PORT; // Start out nPort as the default value, 6346
			if ( _stscanf( strValue.Mid( nColon + 1 ), _T("%i"), &nPort ) == 1 // Make sure 1 number was found
				&& nPort != 0 ) // Make sure the found number isn't 0
			{
				// Save the found port number in m_pHost
				m_pHost.sin_port = htons( nPort ); // Convert Windows little endian to big for the Internet with htons
			}
		}
	}

	// Have ReadHeaders keep going
	return TRUE;
}

// Classes that inherit from CConnection override this virtual method, adding code specific to them
// Returns true
BOOL CConnection::OnHeadersComplete()
{
	// Just return true, it's CShakeNeighbour::OnHeadersComplete() that usually gets called instead of this method
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CConnection header output helpers

// Compose text like "Listen-IP: 1.2.3.4:5" and prints it into the output buffer
// Returns true if we are listening on a port and the header was sent, false otherwise
BOOL CConnection::SendMyAddress()
{
	// Only do something if we are listening on a port
	if ( Network.IsListening() )
	{
		// Compose header text
		CString strHeader;

		// Format works just like sprintf
		strHeader.Format(
			_T("Listen-IP: %s:%hu\r\n"),								// Make it like "Listen-IP: 67.176.34.172:6346\r\n"
			(LPCTSTR)CString( inet_ntoa( Network.m_pHost.sin_addr ) ),	// Insert the IP address like "67.176.34.172"
			htons( Network.m_pHost.sin_port ) );						// Our port number in big endian

		// Print the line into the bottom of the output buffer
		m_pOutput->Print( strHeader ); // It will be sent to the remote computer on the next write

		// Report that we are listening on a port, and the header is sent
		return TRUE;
	}

	// We're not even listening on a port
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CConnection blocked agent filter

// Call to determine if the remote computer is running software we'd rather not communicate with
// Returns true to block or false to allow the program
BOOL CConnection::IsAgentBlocked()
{
	// Eliminate some obvious block and don't block cases
	if ( m_sUserAgent == _T("Fake Shareaza") )		return TRUE;	// Block "Fake Shareaza"

	if ( m_sUserAgent.Trim().IsEmpty() )									// Blank user agent
	{
		if ( Settings.Gnutella.BlockBlankClients )
			return TRUE;
		else
			return FALSE;	
	}

	if ( Settings.Uploads.BlockAgents.IsEmpty() )	return FALSE;	// If the list of programs to block is empty, allow this

	// Get the list of blocked programs, and make a copy here of it all in lowercase letters
	CString strBlocked = Settings.Uploads.BlockAgents;
	CharLower( strBlocked.GetBuffer() );
	strBlocked.ReleaseBuffer();

	// Get the name of the program running on the other side of the connection, and make it lowercase also
	CString strAgent = m_sUserAgent;
	CharLower( strAgent.GetBuffer() );
	strAgent.ReleaseBuffer();

	// Loop through the list of programs to block
	for ( strBlocked += '|' ; strBlocked.GetLength() ; )
	{
		// Break off a blocked program name from the start of the list
		CString strBrowser	= strBlocked.SpanExcluding( _T("|;,") );		// Get the text before a puncutation mark
		strBlocked			= strBlocked.Mid( strBrowser.GetLength() + 1 );	// Remove that much text from the start

		// If the blocked list still exists and the blocked program and remote program match, block it
		if ( strBrowser.GetLength() > 0 && strAgent.Find( strBrowser ) >= 0 ) return TRUE;
	}

	// Allow it
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CConnection URL encodings

// Encodes unsafe characters in a string, turning "hello world" into "hello%20world", for instance
// Takes text and returns a string
CString CConnection::URLEncode(LPCTSTR pszInputT)
{
	// Setup two strings, one with all the hexidecimal digits, the other with all the characters to find and encode
	static LPCTSTR pszHex	= _T("0123456789ABCDEF");	// A string with all the hexidecimal digits
	static LPCSTR pszUnsafe	= "<>\"#%{}|\\^~[]+?&@=:,";	// A string with all the characters unsafe for a URL

	// The output string starts blank
	CString strOutput;

	// If the input character pointer points to null or points to the null terminator, just return the blank output string
	if ( pszInputT == NULL || *pszInputT == 0 ) return strOutput;

	// Map the wide character string to a new character set
	int nUTF8 = WideCharToMultiByte(
		CP_UTF8,	// Translate using UTF-8, the default encoding for Unicode
		0,			// Must be 0 for UTF-8 encoding
		pszInputT,	// Points to the wide character string to be converted
		-1,			// The string is null terminated
		NULL,		// We just want to find out how long the buffer for the output string needs to be
		0,
		NULL,		// Both must be NULL for UTF-8 encoding
		NULL );

	// If the converted text would take less than 2 bytes, which is 1 character, just return blank
	if ( nUTF8 < 2 ) return strOutput;

	// Make a new array of CHARs which is nUTF8 bytes long
	LPSTR pszUTF8 = new CHAR[ nUTF8 ];

	// Call WideCharToMultiByte again, this time it has the output buffer and will actually do the conversion
	WideCharToMultiByte( CP_UTF8, 0, pszInputT, -1, pszUTF8, nUTF8, NULL, NULL );

	// Set the null terminator in pszUTF8 to right where you think it should be, and point a new character pointer at it
	pszUTF8[ nUTF8 - 1 ] = 0;
	LPCSTR pszInput = pszUTF8;

	// Get the character buffer inside the output string, specifying how much larger to make it
	LPTSTR pszOutput = strOutput.GetBuffer( strlen( pszInput ) * 3 + 1 ); // Times 3 in case every character gets encoded

	// Loop for each character of input text
	for ( ; *pszInput ; pszInput++ )
	{
		// If the character code is 32 or less, more than 127, or in the unsafe list
		if ( *pszInput <= 32 || *pszInput > 127 || strchr( pszUnsafe, *pszInput ) != NULL )
		{
			// Write a three letter code for it like %20 in the output text
			*pszOutput++ = _T('%');
			*pszOutput++ = pszHex[ ( *pszInput >> 4 ) & 0x0F ];
			*pszOutput++ = pszHex[ *pszInput & 0x0F ];

		} // The character doesn't need to be encoded
		else
		{
			// Just copy it across
			*pszOutput++ = (TCHAR)*pszInput;
		}
	}

	// Null terminate the output text, and then close our direct manipulation of the string
	*pszOutput = 0;
	strOutput.ReleaseBuffer(); // This closes the string so Windows can again start managing its memory for us

	// Free the memory we allocated with the new keyword above
	delete [] pszUTF8;


	// Return the URL-encoded, %20-filled text
	return strOutput;
}


// Decodes unsafe characters in a string, turning "hello%20world" into "hello world", for instance
// Takes text and returns a string
CString CConnection::URLDecode(LPCTSTR pszInput)
{
	LPCTSTR pszLoop = pszInput;
	// Check each character of input text
	for ( ; *pszLoop ; pszLoop++ )
	{
		if ( *pszLoop > 255 )
		{
			// This URI is not properly encoded, and has unicode characters in it. URL-decode only
			return URLDecodeUnicode( pszInput );
		}
	}

	// This is a correctly formatted URI, which must be url-decoded, then UTF-8 decoded.
	return URLDecodeANSI( pszInput );
}

// Decodes a properly formatted URI, then UTF-8 decodes it
CString CConnection::URLDecodeANSI(LPCTSTR pszInput)
{
	// Setup local variables useful for the conversion
	TCHAR szHex[3] = { 0, 0, 0 };	// A 3 character long array filled with 3 null terminators
	CString strOutput;				// The output string, which starts out blank
	int nHex;						// The hex code of the character we found
	
	// Allocate a new CHAR array big enough to hold the input characters and a null terminator
	LPSTR pszBytes = new CHAR[ _tcslen( pszInput ) + 1 ];

	// Point the output string pointer at this array
	LPSTR pszOutput = pszBytes;
	
	// Loop for each character of input text
	for ( ; *pszInput ; pszInput++ )
	{
		// We hit a %, which might be the start of something like %20
		if ( *pszInput == '%' )
		{
			// Copy characters like "20" into szHex, making sure neither are null
			if ( ! ( szHex[0] = pszInput[1] ) ) break;
			if ( ! ( szHex[1] = pszInput[2] ) ) break;

			// Read the text like "20" as a number, and store it in nHex
			if ( _stscanf( szHex, _T("%x"), &nHex ) != 1 ) break;
			if ( nHex < 1 ) break; // Make sure the number isn't 0 or negative

			// That number is the code of a character, copy it into the output string
			*pszOutput++ = nHex; // And then move the output pointer to the next spot

			// Move the input pointer past the two characters of the "20"
			pszInput += 2;

		} // We hit a +, which is shorthand for space
		else if ( *pszInput == '+' )
		{
			// Add a space to the output text, and move the pointer forward
			*pszOutput++ = ' ';

		} // The input pointer is just on a normal character
		else
		{
			// Copy it across
			*pszOutput++ = (CHAR)*pszInput;
		}
	}

	// Cap off the output text with a null terminator
	*pszOutput = 0;

	// Copy the text from pszBytes into strOutput, converting it into Unicode
	int nLength = MultiByteToWideChar( CP_UTF8, 0, pszBytes, -1, NULL, 0 );
	MultiByteToWideChar( CP_UTF8, 0, pszBytes, -1, strOutput.GetBuffer( nLength ), nLength );

	// Close the output string, we are done editing its buffer directly
	strOutput.ReleaseBuffer();

	// Free the memory we allocated above
	delete [] pszBytes;

	// Return the output string
	return strOutput;
}


// Decodes encoded characters in a unicode string
CString CConnection::URLDecodeUnicode(LPCTSTR pszInput)
{
	// Setup local variables useful for the conversion
	TCHAR szHex[3] = { 0, 0, 0 };	// A 3 character long array filled with 3 null terminators
	CString strOutput;				// The output string, which starts out blank
	int nHex;						// The hex code of the character we found
	
	// Allocate a new CHAR array big enough to hold the input characters and a null terminator
	LPTSTR pszBytes = strOutput.GetBuffer( _tcslen( pszInput ) );

	// Point the output string pointer at this array
	LPTSTR pszOutput = pszBytes;
	
	// Loop for each character of input text
	for ( ; *pszInput ; pszInput++ )
	{
		// We hit a %, which might be the start of something like %20
		if ( *pszInput == '%' )
		{
			// Copy characters like "20" into szHex, making sure neither are null
			if ( ! ( szHex[0] = pszInput[1] ) ) break;
			if ( ! ( szHex[1] = pszInput[2] ) ) break;

			// Read the text like "20" as a number, and store it in nHex
			if ( _stscanf( szHex, _T("%x"), &nHex ) != 1 ) break;
			if ( nHex < 1 ) break; // Make sure the number isn't 0 or negative

			// That number is the code of a character, copy it into the output string
			*pszOutput++ = nHex; // And then move the output pointer to the next spot

			// Move the input pointer past the two characters of the "20"
			pszInput += 2;

		} // We hit a +, which is shorthand for space
		else if ( *pszInput == '+' )
		{
			// Add a space to the output text, and move the pointer forward
			*pszOutput++ = ' ';

		} // The input pointer is just on a normal character
		else
		{
			// Copy it across
			*pszOutput++ = (TCHAR)*pszInput;
		}
	}

	// End the output text with a null terminator
	*pszOutput = 0;
	// Put the output into a CString
	strOutput.ReleaseBuffer();
	// Return the output string
	return strOutput;
}

// Takes some input text like "hello world" and a text tag like "hello"
// Determines if the input starts with the text
// Returns true or false
BOOL CConnection::StartsWith(LPCTSTR pszInput, LPCTSTR pszText)
{
	// See if input starts with text
	return _tcsnicmp(
		pszInput,				// Compare input
		pszText,				// With text
		_tcslen( pszText ) )	// But just the first text-length part of both
		== 0;					// Return true if it matches, false if not
}
