//
// ShakeNeighbour.cpp
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

// CShakeNeighbour reads and sends handshake headers to negotiate the Gnutella or Gnutella2 handshake
// http://wiki.shareaza.com/static/Developers.Code.CShakeNeighbour

// Copy in the contents of these files here before compiling
#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "Network.h"
#include "Buffer.h"
#include "HostCache.h"
#include "DiscoveryServices.h"
#include "Neighbours.h"
#include "ShakeNeighbour.h"
#include "G1Neighbour.h"
#include "G2Neighbour.h"
#include "Packet.h"

// If we are compiling in debug mode, replace the text "THIS_FILE" in the code with the name of this file
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CShakeNeighbour construction

// Make a new CShakeNeighbour object
CShakeNeighbour::CShakeNeighbour() : CNeighbour( PROTOCOL_NULL ) // Call the CNeighbour constructor first, with no protocol
{
	// Set member variables that record headers to false
	m_bSentAddress   = FALSE; // We haven't told the remote computer "Listen-IP: 1.2.3.4:5"
	m_bG2Send        = FALSE; // The remote computer hasn't said "Content-Type: application/x-gnutella2" yet
	m_bG2Accept      = FALSE; // The remote computer hasn't said "Accept: application/x-gnutella2" yet
	m_bDeflateSend   = FALSE; // The remote computer hasn't said "Content-Encoding: deflate" yet
	m_bDeflateAccept = FALSE; // The remote computer hasn't said "Accept-Encoding: deflate" yet

	//ToDo: Check this - G1 setting?
	// Set m_bCanDeflate to true if the checkboxes in Shareaza Settings allow us to send and receive compressed data
	m_bCanDeflate = Neighbours.IsG2Leaf() ? ( Settings.Gnutella.DeflateHub2Hub || Settings.Gnutella.DeflateLeaf2Hub ) : ( Settings.Gnutella.DeflateHub2Hub || Settings.Gnutella.DeflateHub2Hub );

	// Start out ultrapeer settings as unknown
	m_bUltraPeerSet    = TS_UNKNOWN; // The remote computer hasn't told us if it's ultra or not yet
	m_bUltraPeerNeeded = TS_UNKNOWN; // The remote computer hasn't told us if it needs more ultra connections yet
	m_bUltraPeerLoaded = TS_UNKNOWN; // May not be in use (do)
}

// Delete this CShakeNeighbour object
CShakeNeighbour::~CShakeNeighbour()
{
	// This virtual method will be redefined by a class that inherits from CShakeNeighbour
}

//////////////////////////////////////////////////////////////////////
// CShakeNeighbour connect to

// Called by CNeighboursWithConnect::ConnectTo
// Takes an IP address and port number to connect to, the automatic setting (do), and true if (do)
// Connects the socket in this object to the remote computer
// Returns false if the connection could not be made
BOOL CShakeNeighbour::ConnectTo(IN_ADDR* pAddress, WORD nPort, BOOL bAutomatic, BOOL bNoUltraPeer)
{
	// Connect the socket in this object to the given ip address and port number
	if ( CConnection::ConnectTo( pAddress, nPort ) )
	{
		// Have Windows signal our event when the state of the socket changes
		WSAEventSelect(                                   // Associate an event object with a specified set of network events
			m_hSocket,                                    // The socket
			Network.m_pWakeup,                            // Signal this event when the following network events happen
			FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE ); // Connection completed, ready to read or write, or socket closed

		// Report that we are attempting this connection
		theApp.Message( MSG_DEFAULT, IDS_CONNECTION_ATTEMPTING, (LPCTSTR)m_sAddress, htons( m_pHost.sin_port ) );

	} // ConnectTo reported that the socket could not be made
	else
	{
		// Report the connection failure
		theApp.Message( MSG_ERROR, IDS_CONNECTION_CONNECT_FAIL, (LPCTSTR)CString( inet_ntoa( m_pHost.sin_addr ) ) );
		return FALSE;
	}

	// Initialize more member variables
	m_nState		= nrsConnecting;                        // We've connected the socket, and are waiting for the connection to be made
	m_bAutomatic	= bAutomatic;                           // Copy the given automatic setting into the member variable (do)
	m_bUltraPeerSet	= bNoUltraPeer ? TS_FALSE : TS_UNKNOWN; // Set m_bUltraPeerSet to false or unknown (do)

	// Add this CShakeNeighbour object to the list of them
	Neighbours.Add( this );

	// The connection was made without error
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CShakeNeighbour accept from

// Takes pConnection, the source object to copy from
// Copies the the values from inside pConnection to this CShakeNeighbour object
void CShakeNeighbour::AttachTo(CConnection* pConnection)
{
	// Call CConnection::AttachTo to copy pConnection into the CConnection core of this CShakeNeighbour object
	CConnection::AttachTo( pConnection );

	// Have Windows signal our event when the state of the socket changes
	WSAEventSelect(                      // Associate an event object with a specified set of network events
		m_hSocket,                       // The socket
		Network.m_pWakeup,               // Signal this event when the following network events happen
		FD_READ | FD_WRITE | FD_CLOSE ); // Signal it when the socket is ready to read or write, or closed

	// Put this CShakeNeighbour object into the Handshake1 state (do)
	m_nState = nrsHandshake1; // We've finished sending a group of headers, and await the response

	// Add this CShakeNeighbour object to the list of them
	Neighbours.Add( this );
}

//////////////////////////////////////////////////////////////////////
// CShakeNeighbour close

// Called when the socket connection has been dropped
// Takes an error code that explains why
// Records the failure and puts everything away
void CShakeNeighbour::Close(UINT nError)
{
	// If we initiated the connection to the remote computer
	if ( m_bInitiated )
	{
		// And if the remote computer hasn't sent us any handshake headers yet
		if ( m_nState < nrsHandshake2 )
		{
			// Tell the host cache that we didn't get any data from this remote computer at all
			HostCache.OnFailure( &m_pHost.sin_addr, htons( m_pHost.sin_port ) );
		}
	}

	// Have CNeighbour remove this object from the list, and put away the socket
	CNeighbour::Close( nError );
}

//////////////////////////////////////////////////////////////////////
// CShakeNeighbour connection event

// CConnection::DoRun calls this when it has just opened a socket to a remote computer
// Sends the remote computer our first big block of Gnutella headers
// Always returns true
BOOL CShakeNeighbour::OnConnected()
{
	// This call does nothing (do)
	CConnection::OnConnected();

	// Report that this connection was made
	theApp.Message( MSG_DEFAULT, IDS_CONNECTION_CONNECTED, (LPCTSTR)m_sAddress );

	// We initiated the connection to this computer, send it our first block of handshake headers
	m_pOutput->Print( "GNUTELLA CONNECT/0.6\r\n" ); // Ask to connect
	SendPublicHeaders();                            // User agent, ip addresses, Gnutella2 and deflate, advanced Gnutella features
	SendHostHeaders();                              // Try ultrapeers
	m_pOutput->Print( "\r\n" );                     // A blank line ends this first block of headers

	// We've finished sending a group of headers, and await the response
	m_nState = nrsHandshake1;

	// Send the output buffer to the remote computer
	OnWrite();

	// Have CConnection::DoRun keep going
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CShakeNeighbour connection loss event

// CConnection::DoRun calls this when a socket connection has been refused or lost
// Documents what happened, and puts everything away
void CShakeNeighbour::OnDropped(BOOL bError)
{
	// We tried to connect the socket, but are still waiting for the socket connection to be made
	if ( m_nState == nrsConnecting )
	{
		// Close the connection, citing refused as the reason it didn't work out
		Close( IDS_CONNECTION_REFUSED );

	} // We are somewhere else in the chain of connecting and doing the handshake
	else
	{
		// Close the connection, citing dropped as the explination of what happened
		Close( IDS_CONNECTION_DROPPED );
	}
}

//////////////////////////////////////////////////////////////////////
// CShakeNeighbour read and write events

// Reads in data from the remote computer, looks at it as the handshake, and replies to it
// Returns true if we should keep talking to the remote computer, false if we should disconnect
BOOL CShakeNeighbour::OnRead()
{
	// Copy data from the remote computer that is waiting on our end of the socket into the input buffer
	CConnection::OnRead();

	// If we've finished sending a group of headers, read the remote computer's response
	if ( m_nState == nrsHandshake1 && ! ReadResponse() ) return FALSE;

	// If the remote computer has sent the first line of its initial group of headers, keep reading them
	if ( m_nState == nrsHandshake2 && ! ReadHeaders() ) return FALSE;

	// If we've finished sending a group of headers, read the remote computer's response
	if ( m_nState == nrsHandshake1 && ! ReadResponse() ) return FALSE;

	// If the remote computer has sent the first line of its final group of headers, keep reading them
	if ( m_nState == nrsHandshake3 && ! ReadHeaders() ) return FALSE;

	// If the remote computer started with "GNUTELLA/0.6", but did not say "200 OK" (do)
	if ( m_nState == nrsRejected && ! ReadHeaders() ) return FALSE;

	// Keep talking to the remote computer
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CShakeNeighbour run event

// CConnection::DoRun calls this
// Makes sure the handshake hasn't been taking too long
// Returns false to stop talking to this computer, true to keep talking to it
BOOL CShakeNeighbour::OnRun()
{
	// Get the number of milliseconds since the computer has been turned on
	DWORD nTimeNow = GetTickCount();

	// We connected the socket, and are waiting for the socket connection to be made
	switch ( m_nState ) {
	case nrsConnecting:

		// If we've been waiting for the connection to be made longer than the connection timeout in settings
		if ( nTimeNow - m_tConnected > Settings.Connection.TimeoutConnect )
		{
			// Tell discovery services that we're giving up on this one, and close the connection
			DiscoveryServices.OnGnutellaFailed( &m_pHost.sin_addr );
			Close( IDS_CONNECTION_TIMEOUT_CONNECT );
			return FALSE;
		}

		break;

	// We are exchanging handshake headers with the remote computer, and the most recent thing that's happened is
	case nrsHandshake1: // We've sent a complete group of headers
	case nrsHandshake2: // The remote computer sent the first line of its initial group of headers to us
	case nrsHandshake3: // The remote computer sent the first line of its final group of headers to us
	case nrsRejected:   // The remote computer sent us a line with a 503 error code

		// If the handshake has been going on for longer than the handshake timeout in settings
		if ( nTimeNow - m_tConnected > Settings.Connection.TimeoutHandshake )
		{
			// Close the connection
			Close( IDS_HANDSHAKE_TIMEOUT );
			return FALSE;
		}

		break;

	// DelayClose was called, it sends the write buffer to the remote computer before closing the socket
	case nrsClosing:

		// Close the connection
		Close( 0 );
		return FALSE;

		break;
	}

	// Have CConnection::DoRun keep talking to this remote computer
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CShakeNeighbour handshake header dispatch

// Tell the remote computer we are Shareaza, and setup Gnutella2 or just Gnutella communications
void CShakeNeighbour::SendMinimalHeaders()
{
	// Say what program we are with a line like "User-Agent: Shareaza 1.2.3.4\r\n"
	CString strHeader = Settings.SmartAgent( Settings.General.UserAgent );
	if ( strHeader.GetLength() )
	{
		strHeader = _T("User-Agent: ") + strHeader + _T("\r\n");
		m_pOutput->Print( strHeader );
	}

	// The user has checked the box to connect to Gnutella2 today
	if ( Settings.Gnutella2.EnableToday && ( m_nProtocol != PROTOCOL_G1 ) )
	{
		// If we initiated the connection to the remote computer
		if ( m_bInitiated )
		{
			// Tell the remote computer we know how to read Gnutella and Gnutella2 packets
			m_pOutput->Print( "Accept: application/x-gnutella2,application/x-gnutella-packets\r\n" );
		}
		else if ( m_bG2Accept ) // The remote computer contacted us, and accepts Gnutella2 packets
		{
			// Reply by saying we accept them also, and will be sending them
			m_pOutput->Print( "Accept: application/x-gnutella2\r\n" );
			m_pOutput->Print( "Content-Type: application/x-gnutella2\r\n" );
		}
	}
}

// Sends Gnutella headers to the other computer
void CShakeNeighbour::SendPublicHeaders()
{
	// Tell the remote we are running Shareza with a header like "User-Agent: Shareaza 2.1.0.97"
	CString strHeader = Settings.SmartAgent( Settings.General.UserAgent ); // UserAgent is just "."
	if ( strHeader.GetLength() )
	{
		strHeader = _T("User-Agent: ") + strHeader + _T("\r\n");
		m_pOutput->Print( strHeader );
	}

	// Tell the remote computer our IP address with a header like "Listen-IP: 67.176.34.172:6346"
	m_bSentAddress |= SendMyAddress(); // Returns true if the header is sent, set m_bSentAddress true once its sent

	// Tell the remote computer what IP address it has from here with a header like "Remote-IP: 81.103.192.245"
	strHeader.Format( _T("Remote-IP: %s\r\n"), (LPCTSTR)CString( inet_ntoa( m_pHost.sin_addr ) ) );
	m_pOutput->Print( strHeader );

	// If the settings say connect to Gnutella2 and this function got passed Gnutella2 or the unknown network
	if ( ( Settings.Gnutella2.EnableToday ) && ( m_nProtocol != PROTOCOL_G1 ) ) // The protocol ID is Gnutella2 or unknown
	{
		// If we initiated the connection to the remote computer
		if ( m_bInitiated )
		{
			// Tell the remote computer we accept Gnutella and Gnutella2 packets
			m_pOutput->Print( "Accept: application/x-gnutella2,application/x-gnutella-packets\r\n" );
		}
		else if ( m_bG2Accept ) // The remote computer contacted us, and accepts Gnutella2 packets
		{
			// Tell the remote computer we accept Gnutella2 packets and will be sending it Gnutella2 packets
			m_pOutput->Print( "Accept: application/x-gnutella2\r\n" );			// We can read Gnutella2 packets
			m_pOutput->Print( "Content-Type: application/x-gnutella2\r\n" );	// You will be getting them from us
		}
	}

	// Shareaza Settings allow us to exchange compressed data with this computer
	if ( m_bCanDeflate )
	{
		// Tell the remote computer we can accept compressed data
		m_pOutput->Print( "Accept-Encoding: deflate\r\n" );

		// If the remote computer connected to us and accepts compressed data
		if ( ! m_bInitiated && m_bDeflateAccept )
		{
			// Tell it we will be sending compressed data
			m_pOutput->Print( "Content-Encoding: deflate\r\n" );
		}
	}

	// If the settings say connect to Gnutella and this function got passed Gnutella or the unknown network
	if ( ( Settings.Gnutella1.EnableToday ) && ( m_nProtocol != PROTOCOL_G2 ) ) // The protocol ID is Gnutella or unknown
	{
		// Tell the remote computer all the Gnutella features we support
		if ( Settings.Gnutella1.EnableGGEP ) m_pOutput->Print( "GGEP: 0.5\r\n" );			// We support GGEP blocks
		m_pOutput->Print( "Pong-Caching: 0.1\r\n" );										// We support pong caching
		if ( Settings.Gnutella1.VendorMsg ) m_pOutput->Print( "Vendor-Message: 0.1\r\n" );	// We support vendor-specific messages
		m_pOutput->Print( "X-Query-Routing: 0.1\r\n" );										// We support the query routing protocol
	}

	// If we initiated the connection to the remote computer and it is not an ultrapeer
	if ( m_bInitiated && m_bUltraPeerSet == TS_FALSE )
	{
		// Really, we don't know if it's an ultrapeer or not yet
		m_bUltraPeerSet = TS_UNKNOWN;

	} // The remote computer called us, or we called them and it's an ultrapeer or hasn't said yet
	else
	{
		if ( m_nProtocol == PROTOCOL_G1 )
		{
			// Find out if we are an ultrapeer or at least eligible to become one soon
			if ( Neighbours.IsG1Ultrapeer() || Neighbours.IsG1UltrapeerCapable() )
			{
				// Tell the remote computer that we are an ultrapeer
				m_pOutput->Print( "X-Ultrapeer: True\r\n" );

			} // We are not an ultrapeer nor are we elegible, and the settings say so too
			else if ( Settings.Gnutella1.ClientMode != MODE_ULTRAPEER )
			{
				// Tell the remote computer that we are not an ultrapeer, we are just a Gnutella leaf node
				m_pOutput->Print( "X-Ultrapeer: False\r\n" );
			}
		}
		else // This protocol ID this method got passed is unknown or for something other than Gnutella
		{
			// Find out if we are a Gnutella2 hub, or at least eligible to become one soon
			if ( Neighbours.IsG2Hub() || Neighbours.IsG2HubCapable() )
			{
				// Tell the remote computer that we are a hub
				m_pOutput->Print( "X-Ultrapeer: True\r\n" );

			} // We are not a hub nor are we eligible, and the settings say so too
			else if ( Settings.Gnutella2.ClientMode != MODE_HUB )
			{
				// Tell the remote computer that we are a leaf
				m_pOutput->Print( "X-Ultrapeer: False\r\n" );
			}
		}
	}
}

// Reply to a remote computer's headers with more headers, confirming Gnutella2 packets and data compression
void CShakeNeighbour::SendPrivateHeaders()
{
	// If we haven't told the other computer our IP address yet, do it now with a header like "Listen-IP: 1.2.3.4:5"
	if ( ! m_bSentAddress ) m_bSentAddress = SendMyAddress();

	// We initiated the connection to the remote computer, it's going to send us Gnutella2 packets, and it accepts them
	if ( m_bInitiated && m_bG2Send && m_bG2Accept && ( m_nProtocol != PROTOCOL_G1 ) )
	{
		// Tell it we also accept gnutella2 packets, and we're also going to send them
		m_pOutput->Print( "Accept: application/x-gnutella2\r\n" );
		m_pOutput->Print( "Content-Type: application/x-gnutella2\r\n" );
	}

	// If we initiated the connection to the remote computer
	if ( m_bInitiated )
	{
		// All the data from the remote computer is going to be compressed
		if ( m_bDeflateSend ) 
		{
			// Tell it we accept compressed data too
			m_pOutput->Print( "Accept-Encoding: deflate\r\n" );
		}
		
		// The remote computer accepts compressed data
		if ( m_bDeflateAccept ) 
		{
			// Tell it all the data we send it will be compressed
			m_pOutput->Print( "Content-Encoding: deflate\r\n" );
		}
	}
}

// Takes a line like "GNUTELLA/0.6 503 Need an Ultrapeer"
// Sends it with the "X-Try-Ultrapeers:" header
void CShakeNeighbour::SendHostHeaders(LPCTSTR pszMessage)
{
	// Local variables
	DWORD nTime = time( NULL );	// The number of seconds since midnight on January 1, 1970
	CString strHosts, strHost;	// Text to describe other computers and just one
	CHostCacheHost* pHost;		// We'll use this to loop through host objects in the host cache

	// If this method was given a message
	if ( pszMessage )
	{
		// Send it to the remote computer, along with the minimal headers
		m_pOutput->Print( pszMessage );
		m_pOutput->Print( "\r\n" );
		SendMinimalHeaders(); // Say we are Shareaza, and understand Gnutella2 packets
	}

	// Compose text with IP address and online time information to help the remote computer find more Gnutella computers
	int nCount = Settings.Gnutella1.PongCount;		// The list can't be longer than the pong count
	if ( m_bG2Accept || m_bG2Send || m_bShareaza )	// The remote computer accepts Gnutella2 packets, sends them, or is Shareaza too
	{
		// Loop through the Gnutella2 host cache from newest to oldest
		for ( pHost = HostCache.Gnutella2.GetNewest() ; pHost && nCount > 0 ; pHost = pHost->m_pPrevTime )
		{
			// This host is still recent enough to tell another computer about
			if ( pHost->CanQuote( nTime ) )
			{
				// Compose text for one computer and add it to the string
				strHost = pHost->ToString();						// The host object composes text about itself
				if ( strHosts.GetLength() ) strHosts += _T(",");	// Separate each computer's info with a comma
				strHosts += strHost;								// Add this computer's info to the big string
				nCount--;											// Record that we documented another computer
			}
		}
	}
	else // The remote computer is running Gnutella
	{
		// Loop through the Gnutella host cache from newest to oldest
		for ( pHost = HostCache.Gnutella1.GetNewest() ; pHost && nCount > 0 ; pHost = pHost->m_pPrevTime )
		{
			// This host is still recent enough to tell another computer about
			if ( pHost->CanQuote( nTime ) )
			{
				// Compose text for one computer and add it to the string
				strHost = pHost->ToString();						// Like "24.98.97.155:6348 2004-12-18T23:47Z"
				if ( strHosts.GetLength() ) strHosts += _T(",");	// Separate each computer's info with a comma
				strHosts += strHost;								// Add this computer's info to the big string
				nCount--;											// Record that we documented another computer
			}
		}
	}

	// If we have any computers to tell the remote computer about
	if ( strHosts.GetLength() )
	{
		// Send the information in a header like "X-Try-Ultrapeers: 24.98.97.155:6348 2004-12-18T23:47Z," and so on
		m_pOutput->Print( "X-Try-Ultrapeers: " );
		m_pOutput->Print( strHosts );
		m_pOutput->Print( "\r\n" );
	}

	// If this method started the handshake with a message line, end it with the blank line
	if ( pszMessage ) m_pOutput->Print( "\r\n" ); // Sends a blank line, marking the end of the handshake
}

//////////////////////////////////////////////////////////////////////
// CShakeNeighbour handshake response processor

// Reads the first line of a new group of handshake headers from the remote computer, and sets the state to have OnRead read more
// Returns true to keep reading the handshake, false if its over or you should stop
BOOL CShakeNeighbour::ReadResponse()
{
	// Read one header line from the handshake the remote computer has sent us
	CString strLine; // The line
	if ( ! m_pInput->ReadLine( strLine ) ) return TRUE; // The line is still arriving, return true to try this method again
	if ( strLine.GetLength() > 1024 ) strLine = _T("#LINE_TOO_LONG#"); // Make sure the line isn't too long
	theApp.Message( MSG_DEBUG, _T("%s: HANDSHAKE: %s"), (LPCTSTR)m_sAddress, (LPCTSTR)strLine ); // Report handshake lines

	// If we initiated the connection to the remote computer, and now it's responding with "GNUTELLA OK"
	if ( strLine == _T("GNUTELLA OK") && m_bInitiated )
	{
		// (do)
		OnHandshakeComplete();	// Deletes this CShakeNeighbour object
		return FALSE;			// Tell the method that called this one not to call us again, we're done here

	} // The line starts "GNUTELLA/0.6", and says something else after that
	else if ( strLine.GetLength() > 13 && strLine.Left( 12 ) == _T("GNUTELLA/0.6") )
	{
		// It does not say "200 OK" after that
		if ( strLine != _T("GNUTELLA/0.6 200 OK") )
		{
			// Clip out just the part that says why we were rejected, document it, and set the state to rejected
			strLine = strLine.Mid( 13 );
			theApp.Message( MSG_ERROR, IDS_HANDSHAKE_REJECTED, (LPCTSTR)m_sAddress, (LPCTSTR)strLine );
			m_nState = nrsRejected; // Set the neigbour state in this CShakeNeighbour object to rejected

		} // It does say "200 OK", and the remote computer contacted us
		else if ( ! m_bInitiated )
		{
			// The remote computer connected to us and sent its headers, we replied with ours, and now it's sending the final group
			m_nState = nrsHandshake3; // We're reading the final header group from the remote computer

		} // It does say "200 OK", and we initiated the connection
		else
		{
			// We connected and sent our headers, now the remote computer is responding
			m_nState = nrsHandshake2; // We're reading the initial header group the remote computer has sent
		}

	} // The remote computer connected to us, and wants to talk with the old 0.4 protocol
	else if ( strLine == _T("GNUTELLA CONNECT/0.4") && ! m_bInitiated )
	{
		// Gnutella 0.4 is too old for us, close the connection
		DelayClose( IDS_HANDSHAKE_SURPLUS );	// Send the buffer then close the socket, citing the reason the connection didn't work out
		return FALSE;							// Tell the method that called this one to stop calling us

	} // The remote computer connected to us, and wants to talk Gnutella
	else if ( strLine.GetLength() > 17 && strLine.Left( 17 ) == _T("GNUTELLA CONNECT/") && ! m_bInitiated )
	{
		// We're reading the initial header group the remote computer has sent
		m_nState = nrsHandshake2;

	} // The remote computer connected to us, and said "SHAREAZABETA CONNECT/"
	else if ( strLine.GetLength() > 21 && strLine.Left( 21 ) == _T("SHAREAZABETA CONNECT/") && ! m_bInitiated )
	{
		// We're reading the initial header group the remote computer has sent
		m_nState = nrsHandshake2;

	} // The remote computer said something else that we aren't expecting here
	else
	{
		// Close the connection, citing the reason as we can't understand the handshake
		Close( IDS_HANDSHAKE_FAIL );
		return FALSE; // Tell the calling method to stop calling us
	}

	// Tell the calling method it can call us some more to read more lines from the handshake
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CShakeNeighbour handshake header processing

// Takes a handshake header and value parsed from a line sent by the remote computer
// Reads it and sets member variables to reflect the remote computer's capabilities
// Returns true to keep going, or false to indicate the handshake is over or we should stop trying to read it
BOOL CShakeNeighbour::OnHeaderLine(CString& strHeader, CString& strValue)
{
	// Record this header
	theApp.Message( MSG_DEBUG, _T("%s: GNUTELLA HEADER: %s: %s"), (LPCTSTR)m_sAddress, (LPCTSTR)strHeader, (LPCTSTR)strValue );

	// It's the "User-Agent" header, which will tell the name and version of the remote program
	if ( strHeader.CompareNoCase( _T("User-Agent") ) == 0 )
	{
		// Save the name and version of the remote program
		m_sUserAgent = strValue;

		// If the text contains "Shareaza"
		if ( _tcsistr( m_sUserAgent, _T("Shareaza") ) ) 
		{
			// Record that the remote computer is running Shareaza
			m_bShareaza = TRUE;
		} 

		// If the remote computer is running a program we don't want to talk to
		if ( IsAgentBlocked() )
		{
			// Record that we're rejecting this handshake, and set the state to rejected
			theApp.Message( MSG_ERROR, IDS_HANDSHAKE_REJECTED, (LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );
			m_nState = nrsRejected;
		}
				
		// Check if it's an old version of Shareaza
		m_bObsoleteClient = IsClientObsolete();

	} // The remote computer is telling us our IP address
	else if ( strHeader.CompareNoCase( _T("Remote-IP") ) == 0 )
	{
		// Give the value, which is text like "1.2.3.4", to the Network object
		Network.AcquireLocalAddress( strValue );

	} // The remote computer is telling us its IP address
	else if (	strHeader.CompareNoCase( _T("X-My-Address") ) == 0 ||
				strHeader.CompareNoCase( _T("Listen-IP") ) == 0 ||
				strHeader.CompareNoCase( _T("Node") ) == 0 )
	{
		// Find the index of the first colon in the text
		int nColon = strValue.Find( ':' );
		if ( nColon > 0 ) // There is a colon and it's not at the start of the text
		{
			// Save the default Gnutella port, 6346, in nPort to use it if we can't read the port number from the header value text
			int nPort = GNUTELLA_DEFAULT_PORT;

			// Mid clips the strValue text from beyond the colon to the end
			// _stscanf is like scanf, and %1u means read the text as a long unsigned number
			// The If block makes sure that _stscanf successfully reads 1 item, and the number it read isn't 0
			if (_stscanf( strValue.Mid( nColon + 1 ), _T("%lu"), &nPort ) == 1 && nPort != 0 )
			{
				// Save the remote computer port number in the connection object's m_pHost member variable
				m_pHost.sin_port = htons( nPort ); // Call htons to go from PC little-endian to Internet big-endian byte order
			}
		}

	} // The remote computer is telling us it supports Gnutella pong caching
	else if ( strHeader.CompareNoCase( _T("Pong-Caching") ) == 0 )
	{
		// Record this ability in the member variable
		m_bPongCaching = TRUE;

	} // The remote computer is telling us it supports vendor-specific Gnutella messages
	else if ( strHeader.CompareNoCase( _T("Vendor-Message") ) == 0 )
	{
		// Record this ability in the member variable
		m_bVendorMsg = TRUE;

	} // The remote computer is telling us it supports Gnutella query routing
	else if ( strHeader.CompareNoCase( _T("X-Query-Routing") ) == 0 )
	{
		// Only set m_bQueryRouting true if the value isn't "0" or "0.0"
		m_bQueryRouting = ( strValue != _T("0") && strValue != _T("0.0") );

	} // The remote computer is telling us if it is an hub or a leaf
	else if (	strHeader.CompareNoCase( _T("X-Ultrapeer") ) == 0 ||
				strHeader.CompareNoCase( _T("X-Hub") ) == 0 )
	{
		// If the value is the text "True", set m_bUltraPeerSet to true, otherwise set it to false
		m_bUltraPeerSet = ( strValue.CompareNoCase( _T("True") ) == 0 ) ? TS_TRUE : TS_FALSE;

	} // The remote computer is telling us if it needs more connections to ultrapeers or not
	else if (	strHeader.CompareNoCase( _T("X-Ultrapeer-Needed") ) == 0 ||
				strHeader.CompareNoCase( _T("X-Hub-Needed") ) == 0 )
	{
		// If the value is the text "True", set m_bUltraPeerNeeded to true, otherwise set it to false
		m_bUltraPeerNeeded = ( strValue.CompareNoCase( _T("True") ) == 0 ) ? TS_TRUE : TS_FALSE;

	} // The remote computer is telling us the "X-Ultrapeer-Loaded" header, which may not be in use (do)
	else if (	strHeader.CompareNoCase( _T("X-Ultrapeer-Loaded") ) == 0 ||
				strHeader.CompareNoCase( _T("X-Hub-Loaded") ) == 0 )
	{
		// If the value is the text "True", set m_bUltraPeerLoaded to true, otherwise set it to false
		m_bUltraPeerLoaded = ( strValue.CompareNoCase( _T("True") ) == 0 ) ? TS_TRUE : TS_FALSE;

	} // The remote computer is telling us it understands GGEP blocks
	else if ( strHeader.CompareNoCase( _T("GGEP") ) == 0 )
	{
		// If we also have GGEP blocks enabled
		if ( Settings.Gnutella1.EnableGGEP )
		{
			// And if the value the remote computer sent isn't "0" or "0.0", set m_bGGEP true
			m_bGGEP = ( strValue != _T("0") && strValue != _T("0.0") );
		}

	} // The remote computer is telling us it accepts and can understand a kind of packets
	else if ( strHeader.CompareNoCase( _T("Accept") ) == 0 && Settings.Gnutella2.EnableToday ) // And we're connected to Gnutella2
	{
		// Set m_bG2Accept to true if it accepts Gnutella2 or Shareaza packets
		m_bG2Accept |= ( strValue.Find( _T("application/x-gnutella2") ) >= 0 );
		m_bG2Accept |= ( strValue.Find( _T("application/x-shareaza") ) >= 0 );

	} // The remote computer is telling us it is sending a kind of packets
	else if ( strHeader.CompareNoCase( _T("Content-Type") ) == 0 && Settings.Gnutella2.EnableToday ) // And we're connected to Gnutella2
	{
		// Set m_bG2Send to true if it is sending Gnutella2 or Shareaza packets
		m_bG2Send |= ( strValue.Find( _T("application/x-gnutella2") ) >= 0 );
		m_bG2Send |= ( strValue.Find( _T("application/x-shareaza") ) >= 0 );

	} // The remote computer is telling us it can accept compressed data, and the settings allow us to do compression
	else if ( strHeader.CompareNoCase( _T("Accept-Encoding") ) == 0 && m_bCanDeflate ) // Settings allow us to do compression
	{
		// Look for the text "deflate", and make m_bDeflateAccept true if it's found
		m_bDeflateAccept |= ( strValue.Find( _T("deflate") ) >= 0 );

	} // The remote computer is telling us it will send compressed data, and the settings allow us to do compression
	else if ( strHeader.CompareNoCase( _T("Content-Encoding") ) == 0 && m_bCanDeflate ) // Settings allow us to do compression
	{
		// Look for the text "deflate", and make m_bDeflateSend true if it's found
		m_bDeflateSend |= ( strValue.Find( _T("deflate") ) >= 0 );

	} // The remote computer is giving us a list of IP addreses we can try to contact more ultrapeers
	else if (	strHeader.CompareNoCase( _T("X-Try-Ultrapeers") ) == 0 ||
				strHeader.CompareNoCase( _T("X-Try-Hubs") ) == 0 )
	{
		// Count how many hosts we add to the cache
		int nCount = 0;

		// Append a comma onto the end of the value text once, and then loop forever
		for ( strValue += ',' ; ; ) // for (;;) is the same thing as forever
		{
			// Get the port number we are listening on from settings, 6346 by default
			int nPort = Settings.Connection.InPort; // Not used (do)

			// Find the first comma in the value text
			int nPos = strValue.Find( ',' ); // Set nPos to the distance in characters from the start to the comma
			if ( nPos < 0 ) break;           // If no comma was found, leave the loop

			// Move the text before the comma from the value string to a new string for the host
			CString strHost = strValue.Left( nPos ); // Copy the text up to the comma into strHost
			strValue = strValue.Mid( nPos + 1 );     // Clip that text and the comma off the start of strValue

			// The remote computer accepts Gnutella2 packets, is sending them, or is Shareaza
			if ( m_bG2Accept || m_bG2Send || m_bShareaza )
			{
				// Add the host to the Gnutella2 host cache, sending the text "RAZA" along if the remote computer is Shareaza
				if ( HostCache.Gnutella2.Add( strHost, 0, m_bShareaza ? SHAREAZA_VENDOR_T : NULL ) ) nCount++; // Count it

			} // This is a Gnutella connection, not Gnutella2
			else
			{
				// Add the host to the Gnutella2 host cache, sending the text "RAZA" along if the remote computer is Shareaza
				if ( HostCache.Gnutella1.Add( strHost, 0, m_bShareaza ? SHAREAZA_VENDOR_T : NULL ) ) nCount++; // Count it
			}
		}

		// Tell discovery services the remote computer's IP address, and how many hosts it just told us about
		DiscoveryServices.OnGnutellaAdded( &m_pHost.sin_addr, nCount );
	}

	// Report success
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CShakeNeighbour handshake end of headers

// Called when CConnection::ReadHeaders calls ReadLine and gets a blank line, meaning a group of headers from the remote computer is done
// Responds to the group of headers from the remote computer by closing the connection, sending response headers, or copying this object for a protocol
// Returns false to delete this object, or true to keep reading headers and negotiating the handshake
BOOL CShakeNeighbour::OnHeadersComplete()
{
	// If the remote computer doesn't accept Gnutella2 packets, or it does but we contacted it and it's not going to send them
	if ( ! m_bG2Accept || ( m_bInitiated && ! m_bG2Send ) )
	{
		// This is a Gnutella connection
		m_nProtocol = PROTOCOL_G1;
		return OnHeadersCompleteG1();
	}
	else
	{
		// This is a G2 connection
		m_nProtocol = PROTOCOL_G2;
		return OnHeadersCompleteG2();
	}
}

// Called when CConnection::ReadHeaders calls ReadLine and gets a blank line, meaning a group of headers from the remote computer is done
// Responds to the group of headers from the remote computer by closing the connection, sending response headers, or turning this into a Gnutella2 object
// Returns false to delete this object, or true to keep reading headers and negotiating the handshake
BOOL CShakeNeighbour::OnHeadersCompleteG2()
{
	// Report that a set of Gnutella2 headers from a remote computer are complete
	theApp.Message( MSG_DEBUG, _T("Headers Complete: G2 client") );

	// The remote computer replied to our headers with something other than "200 OK"
	if ( m_nState == nrsRejected )
	{
		// Close the connection
		Close( 0 );   // Don't specify an error
		return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object

	} // We've been reading response headers from a remote computer that contacted us
	else if ( m_nState == nrsHandshake3 ) // We were reading the final header group from the remote computer
	{
		// (do)
		if ( m_bUltraPeerSet == TS_FALSE                                 // The remote computer told us it's a leaf
			&& m_nNodeType == ntNode                                     // And this is a connection to a hub like us
			&& ( Neighbours.IsG2Hub() || Neighbours.IsG2HubCapable() ) ) // And we're either a hub or capable of becoming one
		{
			// Report this case
			theApp.Message( MSG_DEFAULT, IDS_HANDSHAKE_BACK2LEAF, (LPCTSTR)m_sAddress );

			// This connection is to a leaf below us
			m_nNodeType = ntLeaf;
		}

		// Turn this CShakeNeighbour object into a CG2Neighbour object
		OnHandshakeComplete();
		return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object

	} // We initiated the connection
	else if ( m_bInitiated )
	{
		// We'll set this flag to true if we need to tell the remote computer we're a leaf so we can connect
		BOOL bFallback = FALSE;

		// We are a Gnutella2 hub, or at least we are capable of becomming one
		if ( Neighbours.IsG2Hub() || Neighbours.IsG2HubCapable() )
		{
			// The remote computer sent us a header like "X-Ultrapeer: False"
			if ( m_bUltraPeerSet == TS_FALSE )
			{
				// This connection is to a leaf below us
				m_nNodeType = ntLeaf;

			} // The remote computer sent us headers like "X-Ultrapeer: True" and "X-Ultrapeer-Needed: True"
			else if ( m_bUltraPeerSet == TS_TRUE && m_bUltraPeerNeeded == TS_TRUE ) // && Network.NeedMoreHubs() )
			{
				// Record that we are both hubs
				m_nNodeType = ntNode;

			} // The remote computer is an hub that doesn't need connections to more hubs
			else if ( m_bUltraPeerSet == TS_TRUE && m_bUltraPeerNeeded == TS_FALSE )
			{
				// If we are connected to any leaves
				if ( Neighbours.GetCount( PROTOCOL_G2, nrsConnected, ntLeaf ) > 0 )
				{
					// Tell the remote computer we can't connect (do)
					SendHostHeaders( _T("GNUTELLA/0.6 503 I have leaves") );
					DelayClose( IDS_HANDSHAKE_CANTBEPEER ); // Send the buffer then close the socket
					return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
				}

				// If we are a Gnutella2 hub
				if ( Settings.Gnutella2.ClientMode == MODE_HUB )
				{
					// We are a hub, and the remote computer doesn't need any more hub connections, tell it we can't connect
					SendHostHeaders( _T("GNUTELLA/0.6 503 Ultrapeer disabled") );
					DelayClose( IDS_HANDSHAKE_NOULTRAPEER ); // Send the buffer then close the socket
					return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
				}

				// We are or can become an ultrapeer, and the remote computer is an ultrapeer that doesn't need any more ultrapeer connections
				m_nNodeType = ntHub; // Pretend this connection is to a hub above us
				bFallback = TRUE;    // We'll tell the remote computer we're a leaf so we can still connect
			}

		} // The remote computer is a hub
		else if ( m_bUltraPeerSet == TS_TRUE )
		{
			// And so are we
			if ( Settings.Gnutella2.ClientMode == MODE_HUB )
			{
				// (do)
				SendHostHeaders( _T("GNUTELLA/0.6 503 Ultrapeer disabled") );
				DelayClose( IDS_HANDSHAKE_NOULTRAPEER ); // Send the buffer then close the socket
				return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
			}

			// This connection is to a hub above us
			m_nNodeType = ntHub;

		} // The remote computer is a leaf, or it hasn't told us yet
		else if ( m_bUltraPeerSet != TS_TRUE )
		{
			// We are a leaf
			if ( Settings.Gnutella2.ClientMode == MODE_LEAF )
			{
				// Tell the remote computer we can't connect because we need a hub
				SendHostHeaders( _T("GNUTELLA/0.6 503 Need an Ultrapeer") );
				DelayClose( IDS_HANDSHAKE_NEEDAPEER ); // Send the buffer then close the socket
				return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
			}
		}

		// If the connection doesn't fall into any of those error cases, accept it with a batch of reply headers
		m_pOutput->Print( "GNUTELLA/0.6 200 OK\r\n" );                 // Accept the connection
		SendPrivateHeaders();                                          // Respond to each header, setting up Gnutella2 packets and compression
		if ( bFallback ) m_pOutput->Print( "X-Ultrapeer: False\r\n" ); // Tell it we're a leaf so we can still connect
		m_pOutput->Print( "\r\n" );                                    // Send a blank line to end this group of headers

		// Turn this CShakeNeighbour object into a CG2Neighbour one, and delete this one
		OnHandshakeComplete();
		return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object

	} // not rejected, not handshake3, not initiated (do)
	else
	{
		// We are a leaf
		if ( Neighbours.IsG2Leaf() )
		{
			// Tell the remote computer we can't connect because we are a shielded leaf right now
			SendHostHeaders( _T("GNUTELLA/0.6 503 Shielded leaf node") );
			DelayClose( IDS_HANDSHAKE_IAMLEAF ); // Send the buffer then close the socket
			return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object

		} // We are a Gnutella2 hub, or at least we are capable of becomming one
		else if ( Neighbours.IsG2Hub() || Neighbours.IsG2HubCapable() )
		{
			// The remote computer sent us the header "X-Ultrapeer: False"
			if ( m_bUltraPeerSet == TS_FALSE )
			{
				// This connection is to a leaf below us
				m_nNodeType = ntLeaf;

			} // The remote computer sent us the header "X-Ultrapeer: True"
			else if ( m_bUltraPeerSet == TS_TRUE )
			{
				// Record that we are both hubs
				m_nNodeType = ntNode;
			}

		} // The remote computer is an ultrapeer, and we are not running in hub mode
		else if ( m_bUltraPeerSet == TS_TRUE && ( Settings.Gnutella2.ClientMode != MODE_HUB ) )
		{
			// This connection is to a hub above us
			m_nNodeType = ntHub;
		}

		// If it's a leaf and an old version
		if ( ( m_nNodeType == ntLeaf ) && ( m_bObsoleteClient ) )
		{
			// Check our loading. (Old clients consume more resources)
			if ( Neighbours.GetCount(PROTOCOL_G2, nrsConnected ,ntLeaf ) > ( Settings.Gnutella2.NumLeafs / 2 ) )
			{
				theApp.Message( MSG_ERROR, _T("Rejecting obsolete leaf %s (We are too full)") , (LPCTSTR)m_sUserAgent );
				SendHostHeaders( _T("GNUTELLA/0.6 503 Old client version, please update. www.shareaza.com") );
				DelayClose( IDS_HANDSHAKE_SURPLUS );
				return FALSE;
			}
		}

		// If we don't need another connection to the role the remote computer is acting in
		if ((
				// If the remote computer is a leaf and we don't need any more
				m_nNodeType == ntLeaf                        // This connection is to a leaf below us
				&& ! Neighbours.NeedMoreHubs( PROTOCOL_G2 )  // And the neighbours object says we don't need more Gnutella2 hubs or leaves
				&& ! Neighbours.NeedMoreLeafs( PROTOCOL_G2 )

			) || (

				// Or, if the remote computer is a hub and we don't need any more
				m_nNodeType != ntLeaf                        // This connection isn't to a leaf below us
				&& ! Neighbours.NeedMoreHubs( PROTOCOL_G2 )  // And the neighbours object says we don't need any more Gnutella2 hubs
			))
		{
			// Tell the remote computer that we can't connect because we have too many connections already, and close the connection
			SendHostHeaders( _T("GNUTELLA/0.6 503 Maximum connections reached") ); // Send this error code along with some more IP addresses it can try
			DelayClose( IDS_HANDSHAKE_SURPLUS ); // Close the connection, but not until we've written the buffered outgoing data first
			return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object

		} // Or, if we're both in the same role on the network
		else if ( ( m_nNodeType == ntHub && ( Settings.Gnutella2.ClientMode == MODE_HUB ) ) ||  // We're both hubs
				  ( m_nNodeType == ntLeaf && ( Settings.Gnutella2.ClientMode == MODE_LEAF ) ) ) // We're both leaves
		{
			// Tell the remote computer that we can't connect
			SendHostHeaders( _T("GNUTELLA/0.6 503 Ultrapeer disabled") ); // Send the error code along with more IP addresses the remote computer can try
			DelayClose( IDS_HANDSHAKE_NOULTRAPEER ); // Close the connection, but not until we've written the buffered outgoing data first
			return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object

		} // This connection isn't to a hub above us, and we're a leaf
		else if ( m_nNodeType != ntHub && ( Settings.Gnutella2.ClientMode == MODE_LEAF ) )
		{
			// Tell the remote computer we can't connect because we need a hub
			SendHostHeaders( _T("GNUTELLA/0.6 503 Need an Ultrapeer") );
			DelayClose( IDS_HANDSHAKE_NEEDAPEER ); // Send the buffer then close the socket
			return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object

		} // Otherwise, the connection is probably alright
		else
		{
			// Send reply headers to the remote computer
			m_pOutput->Print( "GNUTELLA/0.6 200 OK\r\n" );	// Start our group of response headers to the other computer with the 200 OK message
			SendPublicHeaders();							// Send the initial Gnutella2 headers
			SendPrivateHeaders();							// Send headers in response to those we got from the remote computer
			SendHostHeaders();								// Send the "X-Try-Ultrapeers" header with a list of other IP addresses running Gnutella

			// If this connection is up to a hub or to a hub like us, and we need more hubs
			if ( m_nNodeType != ntLeaf && Neighbours.NeedMoreHubs( PROTOCOL_G2 ) )
			{
				// And if we're a hub, so this connection must be to another hub
				if ( Settings.Gnutella2.ClientMode != MODE_LEAF )
				{
					// Tell the remote computer we want more connections to hubs
					m_pOutput->Print( "X-Ultrapeer-Needed: True\r\n" );
				}

			} // This connection must be down to a leaf
			else
			{
				// And we're a leaf
				if ( Settings.Gnutella2.ClientMode != MODE_HUB )
				{
					// Tell the remote computer we don't need any more hub connections
					m_pOutput->Print( "X-Ultrapeer-Needed: False\r\n" );
				}
			}

			// End this block of headers with a blank line
			m_pOutput->Print( "\r\n" );
			m_nState = nrsHandshake1; // We've finished sending a group of headers, and await the response
		}
	}

	// Have OnRead keep exchanging handshake headers with the remote computer
	return TRUE;
}

// Called when CConnection::ReadHeaders calls ReadLine and gets a blank line, meaning a group of headers from the remote computer is done
// Responds to the group of headers from the remote computer by closing the connection, sending response headers, or turning this into a Gnutella object
// Returns false to delete this object, or true to keep reading headers and negotiating the handshake
BOOL CShakeNeighbour::OnHeadersCompleteG1()
{
	// Report that a set of Gnutella headers from a remote computer are complete
	theApp.Message( MSG_DEBUG, _T("Headers Complete: G1 client") );

	// Check if Gnutella1 is enabled before connecting to a gnutella client
	if ( ! Settings.Gnutella1.EnableToday && m_nState < nrsRejected ) // And make sure the state is before getting rejected
	{
		// Tell the remote computer that we're only connecting to Gnutella2 today
		m_pOutput->Print( "GNUTELLA/0.6 503 G2 Required\r\n" );
		SendMinimalHeaders();       // Tell the remote computer we're Shareaza and we can exchange Gnutella2 packets
		m_pOutput->Print( "\r\n" ); // End the group of headers with a blank line

		// Tell the host cache that this didn't work out
		HostCache.OnFailure( &m_pHost.sin_addr, htons( m_pHost.sin_port ) );

		// Close the connection citing not Gnutella2 as the reason, but send the departing buffer first
		DelayClose( IDS_HANDSHAKE_NOTG2 );

		// Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
		return FALSE;
	}

	// The remote computer started with "GNUTELLA/0.6", but did not say "200 OK"
	if ( m_nState == nrsRejected )
	{
		// Close the connection
		Close( 0 );   // Don't specify an error
		return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object

	} // We've been reading response headers from a remote computer that contacted us
	else if ( m_nState == nrsHandshake3 ) // We're reading the final header group from the remote computer
	{
		// If we're both leaves, and yet somehow we're also either an ultrapeer or can become one (do)
		if ( m_bUltraPeerSet == TS_FALSE && m_nNodeType == ntNode &&               // The remote computer is a hub and so are we
			 ( Neighbours.IsG1Ultrapeer() || Neighbours.IsG1UltrapeerCapable() ) ) // And, we're either a Gnutella ultrapeer or we could become one
		{
			// Report that the handshake is back to a leaf (do), and consider this connection to be to a leaf below us
			theApp.Message( MSG_DEFAULT, IDS_HANDSHAKE_BACK2LEAF, (LPCTSTR)m_sAddress );
			m_nNodeType = ntLeaf; // This connection is to a leaf below us
		}

		// Turn this CShakeNeighbour object into a CG1Neighbour object
		OnHandshakeComplete();
		return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object

	} // We initiated the connection to the remote computer
	else if ( m_bInitiated )
	{
		// We'll set this flag to true if we need to tell the remote computer we're a leaf so we can connect
		BOOL bFallback = FALSE;

		// We are an ultrapeer or at least we are capable of becomming one
		if ( Neighbours.IsG1Ultrapeer() || Neighbours.IsG1UltrapeerCapable() )
		{
			// The remote computer told us "X-Ultrapeer: False"
			if ( m_bUltraPeerSet == TS_FALSE )
			{
				// This connection is to a leaf below us
				m_nNodeType = ntLeaf;

			} // The remote computer told us it's an ultrapeer and that it needs connections to more ultrapeers
			else if ( m_bUltraPeerSet == TS_TRUE && m_bUltraPeerNeeded == TS_TRUE )
			{
				// Record that we are both ultrapeers
				m_nNodeType = ntNode;

			} // The remote computer is an ultrapeer that doesn't need any more ultrapeer connections
			else if ( m_bUltraPeerSet == TS_TRUE && m_bUltraPeerNeeded == TS_FALSE )
			{
				// (do)
				if ( Neighbours.GetCount( PROTOCOL_G1, nrsConnected, ntLeaf ) > 0 )
				{
					// Tell the remote computer we can't connect (do)
					SendHostHeaders( _T("GNUTELLA/0.6 503 I have leaves") );
					DelayClose( IDS_HANDSHAKE_CANTBEPEER ); // Send the buffer then close the socket
					return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
				}

				// We are an ultrapeer
				if ( Settings.Gnutella1.ClientMode == MODE_ULTRAPEER )
				{
					// Tell the remote computer we can't connect
					SendHostHeaders( _T("GNUTELLA/0.6 503 Ultrapeer disabled") );
					DelayClose( IDS_HANDSHAKE_NOULTRAPEER ); // Send the buffer then close the socket
					return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
				}

				// We are or can become an ultrapeer, and the remote computer is an ultrapeer that doesn't need any more ultrapeer connections
				m_nNodeType = ntHub; // Pretend this connection is to a hub above us
				bFallback = TRUE;    // We'll tell the remote computer we're a leaf so we can still connect
			}

		} // We're a leaf, and the remote computer is an ultrapeer
		else if ( m_bUltraPeerSet == TS_TRUE )
		{
			// We are an ultrapeer
			if ( Settings.Gnutella1.ClientMode == MODE_ULTRAPEER )
			{
				// Tell the remote computer we can't connect
				SendHostHeaders( _T("GNUTELLA/0.6 503 Ultrapeer disabled") );
				DelayClose( IDS_HANDSHAKE_NOULTRAPEER ); // Send the buffer then close the socket
				return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
			}

			// This connection is to a hub above us
			m_nNodeType = ntHub;

		} // The remote computer is a leaf
		else if ( m_bUltraPeerSet != TS_TRUE )
		{
			// And so are we
			if ( Settings.Gnutella1.ClientMode == MODE_LEAF )
			{
				// Tell the remote computer that we can't connect because we're both leaves
				SendHostHeaders( _T("GNUTELLA/0.6 503 Need an Ultrapeer") );
				DelayClose( IDS_HANDSHAKE_NEEDAPEER ); // Send the buffer, close the socket, cite need a peer as the reason
				return FALSE;                          // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
			}
		}

		// Tell the remote computer that we accept the connection, and send reply headers
		m_pOutput->Print( "GNUTELLA/0.6 200 OK\r\n" ); // Begin our response headers with the OK message
		SendPrivateHeaders();                          // Respond to each header, setting up Gnutella2 packets and compression

		// If we are an ultrapeer or could become one, but the remote computer is an ultrapeer that doesn't need any more ultrapeer connections
		if ( bFallback ) m_pOutput->Print( "X-Ultrapeer: False\r\n" ); // Tell it we're a leaf so we can still connect

		// Send a blank line to end this group of headers
		m_pOutput->Print( "\r\n" );

		// Turn this CShakeNeighbour object into a CG1Neighbour one, and delete this one
		OnHandshakeComplete();
		return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object

	} // not rejected, not handshake3, not initiated (do)
	else
	{
		// We are a leaf
		if ( Neighbours.IsG1Leaf() )
		{
			// Tell the remote computer we can't connect because we are a shielded leaf right now
			SendHostHeaders( _T("GNUTELLA/0.6 503 Shielded leaf node") );
			DelayClose( IDS_HANDSHAKE_IAMLEAF ); // Send the buffer and then close the socket citing our being a leaf as the reason
			return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object

		} // We are an ultrapeer, or at least we are capable of becomming one
		else if ( Neighbours.IsG1Ultrapeer() || Neighbours.IsG1UltrapeerCapable() )
		{
			// The remote computer told us it is a leaf
			if ( m_bUltraPeerSet == TS_FALSE )
			{
				// This connection is to a leaf below us
				m_nNodeType = ntLeaf;

			} // The remote computer told us it is an ultrapeer
			else if ( m_bUltraPeerSet == TS_TRUE )
			{
				// Record that we are both ultrapeers
				m_nNodeType = ntNode;
			}

		} // The remote computer is an ultrapeer, but we are just a leaf
		else if ( m_bUltraPeerSet == TS_TRUE && ( Settings.Gnutella1.ClientMode != MODE_ULTRAPEER ) )
		{
			// This connection is to a hub above us
			m_nNodeType = ntHub;
		}

		// If we don't need this connection
		if ( ( m_nNodeType == ntLeaf && ! Neighbours.NeedMoreHubs( PROTOCOL_G1 ) &&  // This connection is to a leaf below us, and we don't need more hubs
			 ! Neighbours.NeedMoreLeafs( PROTOCOL_G1 ) ) ||                          // And we don't need more leaves
			 ( m_nNodeType != ntLeaf && ! Neighbours.NeedMoreHubs( PROTOCOL_G1 ) ) ) // All that, or this connection is to a hub and we don't need more hubs
		{
			// Tell the remote computer we can't connect because we already have too many connections
			SendHostHeaders( _T("GNUTELLA/0.6 503 Maximum connections reached") );
			DelayClose( IDS_HANDSHAKE_SURPLUS ); // Send the buffer then close the socket
			return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object

		} // Weed out this nonsense combination that should never happen
		else if ( ( m_nNodeType == ntHub && ( Settings.Gnutella1.ClientMode == MODE_ULTRAPEER ) ) || // This connection is up to a hub, and we are a hub
				  ( m_nNodeType == ntLeaf && ( Settings.Gnutella1.ClientMode == MODE_LEAF ) ) )      // Or, this connection is down to a leaf like us
		{
			// Tell the remote computer we can't connect
			SendHostHeaders( _T("GNUTELLA/0.6 503 Ultrapeer disabled") );
			DelayClose( IDS_HANDSHAKE_NOULTRAPEER ); // Send the buffer then close the socket
			return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object

		} // Weed out another nonsense combination that should never happen
		else if ( m_nNodeType != ntHub && ( Settings.Gnutella1.ClientMode == MODE_LEAF ) )
		{
			// Tell the remote computer we can't connect
			SendHostHeaders( _T("GNUTELLA/0.6 503 Need an Ultrapeer") );
			DelayClose( IDS_HANDSHAKE_NEEDAPEER ); // Send the buffer then close the socket
			return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object

		} // Otherwise, the connection is probably alright
		else
		{
			// Send reply headers to the remote computer
			m_pOutput->Print( "GNUTELLA/0.6 200 OK\r\n" );	// Start our group of response headers to the other computer with the 200 OK message
			SendPublicHeaders();							// Send the initial Gnutella headers
			SendPrivateHeaders();							// Send headers in response to those we got from the remote computer
			SendHostHeaders();								// Send the "X-Try-Ultrapeers" header with a list of other IP addresses running Gnutella

			// If this connection is up to a hub or to a hub like us, and we need more hubs
			if ( m_nNodeType != ntLeaf && Neighbours.NeedMoreHubs( PROTOCOL_G1 ) )
			{
				// And if we're a hub, so this connection must be to another hub
				if ( Settings.Gnutella1.ClientMode != MODE_LEAF )
				{
					// Tell the remote computer we want more connections to hubs
					m_pOutput->Print( "X-Ultrapeer-Needed: True\r\n" );
				}

			} // This connection must be down to a leaf
			else
			{
				// And we're a leaf
				if ( Settings.Gnutella1.ClientMode != MODE_ULTRAPEER )
				{
					// Tell the remote computer we don't need any more hub connections
					m_pOutput->Print( "X-Ultrapeer-Needed: False\r\n" );
				}
			}

			// End this block of headers with a blank line
			m_pOutput->Print( "\r\n" );
			m_nState = nrsHandshake1; // We've finished sending a group of headers, and await the response
		}
	}

	// Have OnRead keep exchanging handshake headers with the remote computer
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CShakeNeighbour handshake completed

// Creates a new GC1Neighbour or CG2Neighbour object based on this one, and deletes this one
void CShakeNeighbour::OnHandshakeComplete()
{
	// Remove this CShakeNeighbour object from the list of them the neighbours object keeps
	Neighbours.Remove( this );

	// Point the bandwidth meter limits at the numbers from Shareaza Settings
	switch ( m_nNodeType ) {
	case ntHub:

		// Point the limits at the settings for connections to hubs above us
		m_mInput.pLimit  = &Settings.Bandwidth.HubIn;
		m_mOutput.pLimit = &Settings.Bandwidth.HubOut;

		break;

	// This connection is to a leaf below us
	case ntLeaf:

		// Point the limits at the settings for connections to leaves below us
		m_mInput.pLimit  = &Settings.Bandwidth.LeafIn;
		m_mOutput.pLimit = &Settings.Bandwidth.LeafOut;

		break;

	// We are both hubs
	case ntNode:

		// Point the limits at the settings for peer hub-to-hub connections
		m_mInput.pLimit  = &Settings.Bandwidth.PeerIn;
		m_mOutput.pLimit = &Settings.Bandwidth.PeerOut;

		break;
	}

	// If the remote computer supports compression, setup buffers for arriving and departing compressed data
	if ( m_bDeflateSend )   m_pZInput  = new CBuffer(); // The remote computer said "Content-Encoding: deflate", make a buffer for compressed data coming in
	if ( m_bDeflateAccept ) m_pZOutput = new CBuffer(); // The remote computer said "Accept-Encoding: deflate", make a buffer for data to compress before sending

	// If Shareaza Settings specify a size for the send buffer
	if ( Settings.Connection.SendBuffer ) // By default, this is 2048, which is 2 KB of space
	{
		// Tell the socket to use a 2 KB buffer for sends
		setsockopt(                                 // Set a socket option
			m_hSocket,                              // The socket this CShakeNeighbour object inherited from CConnection
			SOL_SOCKET,                             // Define this option at the socket level
			SO_SNDBUF,                              // Specify the total per-socket buffer space reserved for sends
			(LPSTR)&Settings.Connection.SendBuffer, // Use the value from Shareaza Settings, 2 KB by default
			4 );                                    // SendBuffer is a DWORD, occupying 4 bytes of memory
	}

	// Make a pointer for a new object that will be a copy of this one, but in a different place on the CConnection inheritance tree
	CNeighbour* pNeighbour = NULL;

	// If the remote computer is G2, or can send and understand Gnutella2 packets and isn't G1
	if ( ( m_nProtocol == PROTOCOL_G2 ) || ( m_bG2Send && m_bG2Accept && ( m_nProtocol != PROTOCOL_G1 ) ) )
	{
		// Record that the remote computer supports query routing
		m_bQueryRouting = TRUE;

		// Make a new Gnutella2 neighbour object by copying values from this ShakeNeighbour one
		pNeighbour = new CG2Neighbour( this );

	}
	else	// The remote computer is just Gnutella, not Gnutella2
	{
		// Record that the remote computer is not running Shareaza
		m_bShareaza = FALSE;

		// Make a new Gnutella neighbour object by copying values from this ShakeNeighbour one
		pNeighbour = new CG1Neighbour( this );
	}

	// This connection is to a hub above us
	if ( m_nNodeType == ntHub )
	{
		// Report that we got a ultrapeer connection
		theApp.Message( MSG_DEFAULT, IDS_HANDSHAKE_GOTPEER );

		// (do)
		Neighbours.PeerPrune( pNeighbour->m_nProtocol );

	} // This connection is to a leaf below us
	else if ( m_nNodeType == ntLeaf )
	{
		// Report that we connected to a leaf
		theApp.Message( MSG_DEFAULT, IDS_HANDSHAKE_GOTLEAF, (LPCTSTR)m_sAddress );
	}

	// When we copied the object, the pointers to buffers were also copied, null them here so deleting this doesn't delete them
	m_pZInput  = NULL;
	m_pZOutput = NULL;

	// Delete this CShakeNeighbour object now that it has been turned into a CG1Neighbour or CG2Neighbour object
	delete this;
}


//////////////////////////////////////////////////////////////////////
// CShakeNeighbour IsClientObsolete

// Checks the user agent to see if it's an outdated client. (An old Shareaza beta, or something)
BOOL CShakeNeighbour::IsClientObsolete()
{
	if ( m_sUserAgent.IsEmpty() ) return FALSE;

	if ( ( _tcsistr( m_sUserAgent, _T("Shareaza 1."   ) ) ) ||	// Old Shareazas
		 ( _tcsistr( m_sUserAgent, _T("Shareaza 2.0." ) ) ) ||

		 ( _tcsistr( m_sUserAgent, _T("Shareaza 3.0"  ) ) ) ||	// Fake Shareaza
		 ( _tcsistr( m_sUserAgent, _T("Shareaza 6."   ) ) ) ||
		 ( _tcsistr( m_sUserAgent, _T("Shareaza 7."   ) ) ) ||

		 ( _tcsistr( m_sUserAgent, _T("K-Lite 2.1"	  ) ) ) ||	// Based on old Shareaza code
		 ( _tcsistr( m_sUserAgent, _T("SlingerX 2."   ) ) ) ||
		 ( _tcsistr( m_sUserAgent, _T("eTomi 2.0."    ) ) ) ||
		 ( _tcsistr( m_sUserAgent, _T("eTomi 2.1."    ) ) ) ||
		 ( _tcsistr( m_sUserAgent, _T("360Share"      ) ) ) )

		 return TRUE;

	return FALSE;
}

