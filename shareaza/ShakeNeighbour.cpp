//
// ShakeNeighbour.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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

// CShakeNeighbour reads and sends handshake headers to negotiate the Gnutella or Gnutella2 handshake
// http://shareazasecurity.be/wiki/index.php?title=Developers.Code.CShakeNeighbour

#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "Security.h"
#include "Network.h"
#include "Buffer.h"
#include "HostCache.h"
#include "Neighbours.h"
#include "ShakeNeighbour.h"
#include "G1Neighbour.h"
#include "G2Neighbour.h"
#include "Packet.h"
#include "VendorCache.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CShakeNeighbour construction

// Make a new CShakeNeighbour object
CShakeNeighbour::CShakeNeighbour() : CNeighbour( PROTOCOL_NULL ), // Call the CNeighbour constructor first, with no protocol
	// Set member variables that record headers to false
	m_bSentAddress(FALSE),			// We haven't told the remote computer "Listen-IP: 1.2.3.4:5"
	m_bG1Send(FALSE),				// The remote computer hasn't said "Content-Type: application/x-gnutella-packets" yet
	m_bG1Accept(FALSE),				// The remote computer hasn't said "Accept: application/x-gnutella-packets" yet
	m_bG2Send(FALSE),				// The remote computer hasn't said "Content-Type: application/x-gnutella2" yet
	m_bG2Accept(FALSE),				// The remote computer hasn't said "Accept: application/x-gnutella2" yet
	m_bDeflateSend(FALSE),			// The remote computer hasn't said "Content-Encoding: deflate" yet
	m_bDeflateAccept(FALSE),		// The remote computer hasn't said "Accept-Encoding: deflate" yet
	// Start out ultrapeer settings as unknown
	m_bUltraPeerSet(TRI_UNKNOWN),	// The remote computer hasn't told us if it's ultra or not yet
	m_bUltraPeerNeeded(TRI_UNKNOWN),	// The remote computer hasn't told us if it needs more ultra connections yet
	m_bUltraPeerLoaded(TRI_UNKNOWN),	// May not be in use (do)
		//ToDo: Check this - G1 setting?
		// Set m_bCanDeflate to true if the checkboxes in Shareaza Settings allow us to send and receive compressed data
	m_bCanDeflate( Neighbours.IsG2Leaf() ?
		( Settings.Gnutella.DeflateHub2Hub || Settings.Gnutella.DeflateLeaf2Hub ) :
		( Settings.Gnutella.DeflateHub2Hub || Settings.Gnutella.DeflateHub2Leaf ) ),
	m_bDelayClose( 0 )
{
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
BOOL CShakeNeighbour::ConnectTo(const IN_ADDR* pAddress, WORD nPort, BOOL bAutomatic, BOOL bNoUltraPeer)
{
	// Connect the socket in this object to the given ip address and port number
	if ( CConnection::ConnectTo( pAddress, nPort ) )
	{
		// Have Windows signal our event when the state of the socket changes
		WSAEventSelect(                                   // Associate an event object with a specified set of network events
			m_hSocket,                                    // The socket
			Network.GetWakeupEvent(),                     // Signal this event when the following network events happen
			FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE ); // Connection completed, ready to read or write, or socket closed

		// Report that we are attempting this connection
		theApp.Message( MSG_INFO, IDS_CONNECTION_ATTEMPTING, (LPCTSTR)m_sAddress, htons( m_pHost.sin_port ) );
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
	m_bUltraPeerSet	= bNoUltraPeer ? TRI_FALSE : TRI_UNKNOWN; // Set m_bUltraPeerSet to false or unknown (do)

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
		Network.GetWakeupEvent(),        // Signal this event when the following network events happen
		FD_READ | FD_WRITE | FD_CLOSE ); // Signal it when the socket is ready to read or write, or closed

	// Put this CShakeNeighbour object into the Handshake1 state (do)
	m_nState = nrsHandshake1; // We are waiting the request message

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
	bool bRemove = false;
	bool bFail = true;

	switch ( nError )
	{
		case 0:
			bFail = false;
		case IDS_HANDSHAKE_TIMEOUT:
			bRemove = m_nState == nrsRejected && m_bInitiated;
			break;
		case IDS_CONNECTION_DROPPED:
			bRemove = true;
			break;
		case IDS_CONNECTION_REFUSED:
			bRemove = m_nState == nrsConnecting && m_bInitiated;
			break;
		case IDS_HANDSHAKE_REJECTED:
			bRemove = true;
			break;
		case IDS_CONNECTION_TIMEOUT_CONNECT:
			break;
		case IDS_HANDSHAKE_FAIL:
			bRemove = true;
			break;
		case IDS_CONNECTION_PEERPRUNE:
			bRemove = true;
			bFail = false;
			break;
	}

	if ( bFail && m_bInitiated )
		HostCache.OnFailure( &m_pHost.sin_addr, htons( m_pHost.sin_port ), m_nProtocol, bRemove );

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
	if ( ! IsOutputExist() ) return FALSE;

	// This call does nothing (do)
	CConnection::OnConnected();

	// Report that this connection was made
	theApp.Message( MSG_INFO, IDS_CONNECTION_CONNECTED, (LPCTSTR)m_sAddress );

	// We initiated the connection to this computer, send it our first block of handshake headers
	Write( _P("GNUTELLA CONNECT/0.6\r\n") ); // Ask to connect
	SendPublicHeaders();                            // User agent, ip addresses, Gnutella2 and deflate, advanced Gnutella
													// features
	// POSSIBLE POLLUTION ALERT:
	// This SendHostHeaders() should not be here. because remote node is not known either G1 or G2.
	// calling this function here sends Cached G1 address to remote host, but since RAZA tells the remote that it is G2 capable
	// and if the remote host is RAZA, it will store GNUTELLA1 nodes into GNUTELLA2 cache.
	// SendHostHeaders();                              // Try ultrapeers

	Write( _P("\r\n") );                     // A blank line ends this first block of headers

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
void CShakeNeighbour::OnDropped()
{
	// We tried to connect the socket, but are still waiting for the socket connection to be made
	if ( m_nState == nrsConnecting )
		Close( IDS_CONNECTION_REFUSED );
	else
	{
		// We are somewhere else in the chain of connecting and doing the handshake
		// Connection to node has succeeded but was dropped.
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

	// If the remote computer has sent the first line of its final group of headers, keep reading them
	if ( m_nState == nrsHandshake3 && ! ReadHeaders() ) return FALSE;

	// If the remote computer started with "GNUTELLA/0.6", but did not say "200 OK" (do)
	if ( m_nState == nrsRejected && ! ReadHeaders() ) return FALSE;

	// Keep talking to the remote computer
	return TRUE;
}

BOOL CShakeNeighbour::OnWrite()
{
	LogOutgoing();

	return CNeighbour::OnWrite();
}

//////////////////////////////////////////////////////////////////////
// CShakeNeighbour handshake header dispatch

// Tell the remote computer we are Shareaza, and setup Gnutella2 or just Gnutella communications
void CShakeNeighbour::SendMinimalHeaders()
{
	// Tell the remote we are running Shareaza
	Write( _P("User-Agent: ") );
	Write( Settings.SmartAgent() );
	Write( _P("\r\n") );

	if ( m_bInitiated )
	{
		if ( m_nProtocol == PROTOCOL_G1 )
		{
			// Tell the remote computer we accept Gnutella packets
			Write( _P("Accept: application/x-gnutella-packets\r\n") );
		}
		else if ( m_nProtocol == PROTOCOL_G2 )
		{
			// Tell the remote computer we accept Gnutella2 packets
			Write( _P("Accept: application/x-gnutella2\r\n") );
		}
		else if ( m_nProtocol == PROTOCOL_NULL )
		{
			if ( Settings.Gnutella1.EnableToday && Settings.Gnutella2.EnableToday )
			{
				// Tell the remote computer we know how to read Gnutella and Gnutella2 packets
				Write( _P("Accept: application/x-gnutella2,application/x-gnutella-packets\r\n") );
			}
			else if ( Settings.Gnutella2.EnableToday )
			{
				Write( _P("Accept: application/x-gnutella2\r\n") );
			}
			else if ( Settings.Gnutella1.EnableToday )
			{
				Write( _P("Accept: application/x-gnutella-packets\r\n") );
			}
		}
	}
	else
	{
		// The remote computer contacted us, and accepts Gnutella2 packets
		if ( m_bG2Accept && Settings.Gnutella2.EnableToday &&
			( m_nProtocol != PROTOCOL_G1 ) )
		{
			Write( _P("Accept: application/x-gnutella2\r\n") );					// We can read Gnutella2 packets
			Write( _P("Content-Type: application/x-gnutella2\r\n") );			// You will be getting them from us
		}
		// The remote computer contacted us, and accepts Gnutella packets
		else if ( m_bG1Accept && Settings.Gnutella1.EnableToday &&
			( m_nProtocol != PROTOCOL_G2 ) )
		{
			Write( _P("Accept: application/x-gnutella-packets\r\n") );			// We can read Gnutella1 packets
			Write( _P("Content-Type: application/x-gnutella-packets\r\n") );	// You will be getting them from us
		}
	}

	SendXUltrapeerHeaders();
}

// Sends Gnutella headers to the other computer
void CShakeNeighbour::SendPublicHeaders()
{
	CString strHeader;

	SendMinimalHeaders();

//  TODO: We currently do not use standard language codes
//	strHeader.Format( _T("X-Locale-Pref: %s\r\n"), Settings.General.Language );
//	Write( strHeader );

	// Header "X-Version: n.n" is part of LimeWire's automatic software update feature. Skip.

	// Tell the remote computer our IP address with a header like "Listen-IP: 67.176.34.172:6346"
	m_bSentAddress |= SendMyAddress(); // Returns true if the header is sent, set m_bSentAddress true once its sent

	// Tell the remote computer what IP address it has from here with a header like "Remote-IP: 81.103.192.245"
	Write( _P("Remote-IP: ") );
	Write( CString( inet_ntoa( m_pHost.sin_addr ) ) );
	Write( _P("\r\n") );

	// Shareaza Settings allow us to exchange compressed data with this computer
	if ( m_bCanDeflate )
	{
		// Tell the remote computer we can accept compressed data
		Write( _P("Accept-Encoding: deflate\r\n") );

		// If the remote computer connected to us and accepts compressed data
		if ( ! m_bInitiated && m_bDeflateAccept )
		{
			// Tell it we will be sending compressed data
			Write( _P("Content-Encoding: deflate\r\n") );
		}
	}

	// If the settings say connect to Gnutella and this function got passed Gnutella or the unknown network
	if ( Settings.Gnutella1.EnableToday && m_nProtocol != PROTOCOL_G2 )
	{
		Write( _P("X-Requeries: False\r\n") );

		// Tell the remote computer all the Gnutella features we support
		if ( Settings.Gnutella1.EnableGGEP ) Write( _P("GGEP: 0.5\r\n") );			// We support GGEP blocks
		Write( _P("Pong-Caching: 0.1\r\n") );										// We support pong caching
		if ( Settings.Gnutella1.VendorMsg ) Write( _P("Vendor-Message: 0.1\r\n") );	// We support vendor-specific messages

		Write( _P("X-Query-Routing: 0.1\r\n") );										// We support the query routing protocol
		Write( _P("X-Ultrapeer-Query-Routing: 0.1\r\n") );

		Write( _P("X-Dynamic-Querying: 0.1\r\n") );
		Write( _P("X-Ext-Probes: 0.1\r\n") );

		strHeader.Format( _T("X-Degree: %u\r\n"), Settings.Gnutella1.NumPeers );
		Write( strHeader );

		strHeader.Format( _T("X-Max-TTL: %u\r\n"), Settings.Gnutella1.SearchTTL );
		Write( strHeader );
	}
}

void CShakeNeighbour::SendXUltrapeerHeaders()
{
	bool bXUltrapeer = false;
	bool bXUltrapeerNeeded = false;

	if ( m_nProtocol == PROTOCOL_G1 ) // This protocol ID this method got passed is Gnutella1
	{
		// Find out if we are an ultrapeer or at least eligible to become one soon
		bXUltrapeer = ( Settings.Gnutella1.ClientMode == MODE_ULTRAPEER ||
			Neighbours.IsG1Ultrapeer() || Neighbours.IsG1UltrapeerCapable() );
		bXUltrapeerNeeded = Neighbours.NeedMoreHubs( PROTOCOL_G1 ) == TRUE;
	}
	else if ( m_nProtocol == PROTOCOL_G2 ) // This protocol ID this method got passed is Gnutella2
	{
		// Find out if we are a Gnutella2 hub, or at least eligible to become one soon
		bXUltrapeer = ( Settings.Gnutella2.ClientMode == MODE_HUB ||
			Neighbours.IsG2Hub() || Neighbours.IsG2HubCapable() );
		bXUltrapeerNeeded = Neighbours.NeedMoreHubs( PROTOCOL_G2 ) == TRUE;
	}
	else if ( m_nProtocol == PROTOCOL_NULL ) // This protocol ID this method got passed is both Gnutella1 and Gnutella2
	{
		if ( Settings.Gnutella1.EnableToday && Settings.Gnutella2.EnableToday &&
			( m_bInitiated || ( ( m_bG2Send || m_bG2Accept) && ( m_bG1Send || m_bG1Accept) ) ) )
		{
			// Find out if we are a Gnutella1 Ultrapeer or Gnutella2 hub already,
			// or at least eligible to become one soon
			bXUltrapeer =
				( ( Settings.Gnutella1.ClientMode == MODE_ULTRAPEER ||
					Neighbours.IsG1Ultrapeer() || Neighbours.IsG1UltrapeerCapable() ) &&
				  ( Settings.Gnutella2.ClientMode == MODE_HUB ||
					Neighbours.IsG2Hub() || Neighbours.IsG2HubCapable() ) );
			bXUltrapeerNeeded = Neighbours.NeedMoreHubs( PROTOCOL_G1 ) &&
				 Neighbours.NeedMoreHubs( PROTOCOL_G2 );
		}
		else if ( Settings.Gnutella1.EnableToday &&
			( m_bInitiated || ( m_bG1Send || m_bG1Accept ) ) )
		{
			// Find out if we are a Gnutella1 Ultrapeer, or at least eligible to become one soon
			bXUltrapeer = ( Settings.Gnutella1.ClientMode == MODE_ULTRAPEER ||
				Neighbours.IsG1Ultrapeer() || Neighbours.IsG1UltrapeerCapable() );
			bXUltrapeerNeeded = Neighbours.NeedMoreHubs( PROTOCOL_G1 ) == TRUE;
		}
		else if ( Settings.Gnutella2.EnableToday &&
			( m_bInitiated || ( m_bG2Send || m_bG2Accept ) ) )
		{
			// Find out if we are a Gnutella2 hub, or at least eligible to become one soon
			bXUltrapeer = ( Settings.Gnutella2.ClientMode == MODE_HUB ||
				Neighbours.IsG2Hub() || Neighbours.IsG2HubCapable() );
			bXUltrapeerNeeded = Neighbours.NeedMoreHubs( PROTOCOL_G2 ) == TRUE;
		}
	}

	if ( bXUltrapeer )
		Write( _P("X-Ultrapeer: True\r\n") );
	else
		Write( _P("X-Ultrapeer: False\r\n") );

	if ( bXUltrapeerNeeded )
		Write( _P("X-Ultrapeer-Needed: True\r\n") );
	else
		Write( _P("X-Ultrapeer-Needed: False\r\n") );
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
		Write( _P("Accept: application/x-gnutella2\r\n") );
		Write( _P("Content-Type: application/x-gnutella2\r\n") );
	}
	else if ( m_bInitiated && m_bG1Send && m_bG1Accept && ( m_nProtocol != PROTOCOL_G2 ) )
	{
		// Tell it we also accept gnutella1 packets, and we're also going to send them
		Write( _P("Accept: application/x-gnutella-packets\r\n") );
		Write( _P("Content-Type: application/x-gnutella-packets\r\n") );
	}

	// If we initiated the connection to the remote computer
	if ( m_bInitiated )
	{
		// All the data from the remote computer is going to be compressed
		if ( m_bDeflateSend )
		{
			// Tell it we accept compressed data too
			Write( _P("Accept-Encoding: deflate\r\n") );
		}

		// The remote computer accepts compressed data
		if ( m_bDeflateAccept )
		{
			// Tell it all the data we send it will be compressed
			Write( _P("Content-Encoding: deflate\r\n") );
		}
	}
}

// Takes a line like "GNUTELLA/0.6 503 Need an Ultrapeer"
// Sends it with the X-Try-Hubs/X-Try-Ultrapeers header
void CShakeNeighbour::SendHostHeaders(LPCSTR pszMessage, size_t nLength)
{
	// Local variables
	DWORD nTime = static_cast< DWORD >( time( NULL ) );	// The number of seconds since midnight on January 1, 1970

	// If this method was given a message
	if ( pszMessage )
	{
		// Send it to the remote computer, along with the minimal headers
		Write( pszMessage, nLength );
		Write( _P("\r\n") );
		SendMinimalHeaders(); // Say we are Shareaza, and understand Gnutella2 packets
	}

	// Compose text with IP address and online time information to help the remote computer find hosts
	if ( m_bBadClient )
	{
		// Send nothing
	}
	else if ( ( m_bG2Accept || m_bG2Send || m_bClientExtended ) && m_nProtocol != PROTOCOL_G1 )
	{
		// The remote computer accepts Gnutella2 packets, sends them, or is Shareaza too

		int nCount = Settings.Gnutella2.HostCount;		// Set max length of list

		CQuickLock oLock( HostCache.Gnutella2.m_pSection );

		// Loop through the Gnutella2 host cache from newest to oldest
		CString strHosts;
		for ( CHostCacheIterator i = HostCache.Gnutella2.Begin() ;
			i != HostCache.Gnutella2.End() && nCount > 0 ; ++i )
		{
			CHostCacheHostPtr pHost = (*i);

			if ( pHost->CanQuote( nTime ) )		// if host is still recent enough
			{
				if ( strHosts.GetLength() ) strHosts += _T(",");	// Separate each computer's info with a comma
				strHosts += pHost->ToString();						// Add this computer's info to the string
				nCount--;											// Decrement counter
			}
		}

		// If we have any G2 hosts to tell the remote computer about
		if ( strHosts.GetLength() )
		{
			Write( _P("X-Try-Hubs: ") );
			Write( strHosts );
			Write( _P("\r\n") );
		}
	}
	else if ( ( m_bG1Accept || m_bG1Send ) && m_nProtocol != PROTOCOL_G2 )
	{
		// This computer is running Gnutella

		int nCount = Settings.Gnutella1.HostCount;		// Set max length of list

		CQuickLock oLock( HostCache.Gnutella1.m_pSection );

		// Loop through the Gnutella host cache from newest to oldest
		CString strHosts;
		for ( CHostCacheIterator i = HostCache.Gnutella1.Begin() ;
			i != HostCache.Gnutella1.End() && nCount > 0 ; ++i )
		{
			CHostCacheHostPtr pHost = (*i);

			// This host is still recent enough to tell another computer about
			if ( pHost->CanQuote( nTime ) )
			{
				if ( strHosts.GetLength() ) strHosts += _T(",");
				strHosts += pHost->ToString( m_bClientExtended != FALSE );
				nCount--;
			}
		}

		// If we have any G1 hosts to tell the remote computer about
		if ( strHosts.GetLength() )
		{
			Write( _P("X-Try-Ultrapeers: ") );
			Write( strHosts );
			Write( _P("\r\n") );
		}
	}

	// If this method started the handshake with a message line, end it with the blank line
	if ( pszMessage ) Write( _P("\r\n") ); // Sends a blank line, marking the end of the handshake
}

//////////////////////////////////////////////////////////////////////
// CShakeNeighbour handshake response processor

// Reads the first line of a new group of handshake headers from the remote computer, and sets the state to have OnRead read more
// Returns true to keep reading the handshake, false if its over or you should stop
BOOL CShakeNeighbour::ReadResponse()
{
	// Read one header line from the handshake the remote computer has sent us
	CString strLine; // The line
	if ( ! Read( strLine ) ) return TRUE; // The line is still arriving, return true to try this method again
	if ( strLine.GetLength() > HTTP_HEADER_MAX_LINE ) strLine = _T("#LINE_TOO_LONG#"); // Make sure the line isn't too long

	theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING, _T("%s >> HANDSHAKE: %s"), (LPCTSTR)m_sAddress, (LPCTSTR)strLine ); // Report handshake lines

	// If we initiated the connection to the remote computer, and now it's responding with "GNUTELLA OK"
	if ( strLine == _T("GNUTELLA OK") && m_bInitiated )
	{
		// (do)
		OnHandshakeComplete();	// Deletes this CShakeNeighbour object
		return FALSE;			// Tell the method that called this one not to call us again, we're done here
	}

	// The line starts "GNUTELLA/0.6", and says something else after that
	if ( strLine.GetLength() > 13 && strLine.Left( 12 ) == _T("GNUTELLA/0.6") )
	{
		// It does not say "200 OK" after that
		if ( strLine != _T("GNUTELLA/0.6 200 OK") )
		{
			// Clip out just the part that says why we were rejected, document it, and set the state to rejected
			strLine = strLine.Mid( 13, 3 );
			m_nState = nrsRejected; // Set the neighbour state in this CShakeNeighbour object to rejected
			if ( strLine == _T("503") )
			{
				m_bDelayClose = IDS_HANDSHAKE_REJECTED;
			}
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
	theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING, _T("%s >> %s: %s"), (LPCTSTR)m_sAddress, (LPCTSTR)strHeader, (LPCTSTR)strValue );

	// It's the "User-Agent" header, which will tell the name and version of the remote program
	if ( strHeader.CompareNoCase( _T("User-Agent") ) == 0 )
	{
		// Save the name and version of the remote program
		m_sUserAgent = strValue;

		// Record that the remote computer is running Shareaza
		m_bClientExtended = VendorCache.IsExtended( m_sUserAgent );

		// If the remote computer is running a client that is breaking GPL, causing problems, etc.
		// We don't actually ban these clients, but we don't accept them as a leaf. Can still upload, though.
		if ( Security.IsClientBad( m_sUserAgent ) )
		{
			// Remember this is a bad client.
			m_bBadClient = TRUE;
		}

		// Actual leechers and hostile clients. (We do ban these)
		if ( Security.IsClientBanned( m_sUserAgent ) )
		{
			m_nState = nrsRejected;
			m_bBadClient = TRUE;
			m_bDelayClose = IDS_SECURITY_BANNED_USERAGENT;
		}

		// If the remote computer is running a client the user has blocked
		if ( Security.IsAgentBlocked( m_sUserAgent ) )
		{
			m_nState = nrsRejected;
			m_bBadClient = TRUE;
			m_bDelayClose = IDS_HANDSHAKE_REJECTED;
		}
	} // The remote computer is telling us our IP address
	else if ( strHeader.CompareNoCase( _T("Remote-IP") ) == 0 )
	{
		// Give the value, which is text like "1.2.3.4", to the Network object
		Network.AcquireLocalAddress( strValue );
	} // The remote computer is telling us its IP address
	else if (	strHeader.CompareNoCase( _T("X-My-Address") ) == 0 ||
				strHeader.CompareNoCase( _T("Listen-IP") ) == 0 ||
				strHeader.CompareNoCase( _T("X-Node") ) == 0 ||
				strHeader.CompareNoCase( _T("Node") ) == 0 )
	{
		// Find the index of the first colon in the text
		int nColon = strValue.Find( ':' );
		if ( nColon > 0 ) // There is a colon and it's not at the start of the text
		{
			// Save the default Gnutella port, 6346, in nPort to use it if we can't read the port number from the header value text
			int nPort = protocolPorts[ PROTOCOL_G2 ];

			// Mid clips the strValue text from beyond the colon to the end
			// _stscanf is like scanf, and %1u means read the text as a long unsigned number
			// The If block makes sure that _stscanf successfully reads 1 item, and the number it read isn't 0
			if (_stscanf( strValue.Mid( nColon + 1 ), _T("%d"), &nPort ) == 1 && nPort != 0 )
			{
				// Save the remote computer port number in the connection object's m_pHost member variable
				m_pHost.sin_port = htons( u_short( nPort ) ); // Call htons to go from PC little-endian to Internet big-endian byte order
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
		m_bUltraPeerSet = ( strValue.CompareNoCase( _T("True") ) == 0 ) ? TRI_TRUE : TRI_FALSE;
	} // The remote computer is telling us if it needs more connections to ultrapeers or not
	else if (	strHeader.CompareNoCase( _T("X-Ultrapeer-Needed") ) == 0 ||
				strHeader.CompareNoCase( _T("X-Hub-Needed") ) == 0 )
	{
		// If the value is the text "True", set m_bUltraPeerNeeded to true, otherwise set it to false
		m_bUltraPeerNeeded = ( strValue.CompareNoCase( _T("True") ) == 0 ) ? TRI_TRUE : TRI_FALSE;
	} // The remote computer is telling us the "X-Ultrapeer-Loaded" header, which may not be in use (do)
	else if (	strHeader.CompareNoCase( _T("X-Ultrapeer-Loaded") ) == 0 ||
				strHeader.CompareNoCase( _T("X-Hub-Loaded") ) == 0 )
	{
		// If the value is the text "True", set m_bUltraPeerLoaded to true, otherwise set it to false
		m_bUltraPeerLoaded = ( strValue.CompareNoCase( _T("True") ) == 0 ) ? TRI_TRUE : TRI_FALSE;
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
	else if ( strHeader.CompareNoCase( _T("Accept") ) == 0 ) // And we're connected to Gnutella2
	{
		// Set m_bG2Accept to true if it accepts Gnutella2 or Shareaza packets
		m_bG1Accept |= ( strValue.Find( _T("application/x-gnutella-packets") ) >= 0 );
		m_bG2Accept |= ( strValue.Find( _T("application/x-gnutella2") ) >= 0 );
		m_bG2Accept |= ( strValue.Find( _T("application/x-shareaza") ) >= 0 );
		if ( !m_bG1Accept && !m_bG2Accept )
		{
			theApp.Message( MSG_DEBUG, L"Unknown app accept header: %s", (LPCTSTR)strHeader );
		}
	} // The remote computer is telling us it is sending a kind of packets
	else if ( strHeader.CompareNoCase( _T("Content-Type") ) == 0 ) // And we're connected to Gnutella2
	{
		// Set m_bG2Send to true if it is sending Gnutella2 or Shareaza packets
		m_bG1Send |= ( strValue.Find( _T("application/x-gnutella-packets") ) >= 0 );
		m_bG2Send |= ( strValue.Find( _T("application/x-gnutella2") ) >= 0 );
		m_bG2Send |= ( strValue.Find( _T("application/x-shareaza") ) >= 0 );
		if ( !m_bG1Send && !m_bG2Send )
		{
			theApp.Message( MSG_DEBUG, L"Unknown app content-type header: %s", (LPCTSTR)strHeader );
		}
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
	}
	else if ( strHeader.CompareNoCase( _T("X-Degree") ) == 0 )
	{
		int nValue = _tstoi( strValue );
		if ( nValue > 0 && nValue < 256 )
			m_nDegree = (DWORD)nValue;
	}
	else if ( strHeader.CompareNoCase( _T("X-Max-TTL") ) == 0 )
	{
		int nValue = _tstoi( strValue );
		if ( nValue > 0 && nValue < 10 )
			m_nMaxTTL = (DWORD)nValue;
	}
	else if ( strHeader.CompareNoCase( _T("X-Dynamic-Querying") ) == 0 )
	{
		m_bDynamicQuerying = ( strValue != _T("0") && strValue != _T("0.0") );
	}
	else if ( strHeader.CompareNoCase( _T("X-Ultrapeer-Query-Routing") ) == 0 )
	{
		m_bUltrapeerQueryRouting = ( strValue != _T("0") && strValue != _T("0.0") );
	}
	else if ( strHeader.CompareNoCase( _T("X-Locale-Pref") ) == 0 )
	{
		m_sLocalePref = strValue.MakeLower();
	}
	else if ( strHeader.CompareNoCase( _T("X-Requeries") ) == 0 )
	{
		m_bRequeries = ( strValue.CompareNoCase( _T("False") ) != 0 );
	}
	else if ( strHeader.CompareNoCase( _T("X-Ext-Probes") ) == 0 )
	{
		m_bExtProbes = ( strValue != _T("0") && strValue != _T("0.0") );
	}
	else if ( m_bBadClient )
	{
		// We don't want to accept Hubs or UltraPeers from clients that have bugs that pollute
		// the host cache, so stop here.
	}
	else if ( !m_bInitiated )
	{
		// if we accept connection from remote and is header for first request, any of "X-Try" headers should not be
		// processed, because no guarantee the given list of nodes are not old.
	}
	else if ( strHeader.CompareNoCase( _T("X-Try-DNA-Hubs") ) == 0 )
	{	// The remote computer is giving us a list GnucDNA G2 hubs
		m_sTryDNAHubs = strValue;
	}
	else if ( strHeader.CompareNoCase( _T("X-Try-Hubs") ) == 0 )
	{	// The remote computer is giving us a list G2 hubs
		m_sTryHubs = strValue;
	}
	else if (	strHeader.CompareNoCase( _T("X-Try-Ultrapeers") ) == 0 )
	{	// This header has been used for several things. In general, it's giving us a list of
		// Gnutella Ultrapeers, however some older versions of Shareaza can send G2 hubs in it,
		// if the client advertises G2 capability
		// However this should not be used for G2 because Shareaza 2.2.1.0 and earlier may give
		// G1 host list with "accept: application/x-gnutella2" specified, if the node has
		// Gnutella1 enabled
		m_sTryUltrapeers = strValue;
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
	BOOL bResult = FALSE;

	// determine what kind of Network this handshake is for.
	if ( m_bInitiated )
	{
		if ( ! m_bG1Accept && ! m_bG2Accept ) m_bG1Accept = TRUE;
		if ( ! m_bG1Send && ! m_bG2Send ) m_bG1Send = TRUE;
	}
	else
	{
		if ( m_nState == nrsHandshake2 && ! m_bG1Accept && ! m_bG2Accept ) m_bG1Accept = TRUE;
		if ( m_nState == nrsHandshake3 && ! m_bG1Send && ! m_bG2Send ) m_bG1Send = TRUE;
	}

	// These codes for HostCache should be here and ones in HeaderLine should just store these info in String array for this part of Process.
	if ( ! m_bInitiated || m_bBadClient )
	{
		// We don't want accept Hubs or UltraPeers from clients that have bugs that pollute
		// the host cache, not even the cache from incoming connections. this part should be
		// ignored
	}
	else if ( m_sTryDNAHubs.GetLength() )
	{	// The remote computer is giving us a list GnucDNA G2 hubs
		int nCount = 0;
		for ( m_sTryDNAHubs += ',' ; ; )
		{
			int nPos = m_sTryDNAHubs.Find( ',' );		// Set nPos to the distance in characters from the start to the comma
			if ( nPos < 0 ) break;					// If no comma was found, leave the loop
			CString strHost = m_sTryDNAHubs.Left( nPos );// Copy the text up to the comma into strHost
			m_sTryDNAHubs = m_sTryDNAHubs.Mid( nPos + 1 );    // Clip that text and the comma off the start of strValue

			// Add the host to the Gnutella2 host cache, noting it's a DNA hub
			// adding host with "GDNA" will not help a lot, if the name is blank, raza might get the name through
			// KHL over linked Neighbours.
			if ( HostCache.Gnutella2.Add( strHost, 0, NULL ) )
				nCount++;
		}
		m_sTryDNAHubs.Empty();
	}
	else if ( m_sTryHubs.GetLength() )
	{	// The remote computer is giving us a list G2 hubs
		int nCount = 0;
		for ( m_sTryHubs += ',' ; ; )
		{
			int nPos = m_sTryHubs.Find( ',' );		// Set nPos to the distance in characters from the start to the comma
			if ( nPos < 0 ) break;					// If no comma was found, leave the loop
			CString strHost = m_sTryHubs.Left( nPos );// Copy the text up to the comma into strHost
			m_sTryHubs = m_sTryHubs.Mid( nPos + 1 );    // Clip that text and the comma off the start of strValue

			// since there is no clever way to detect what the given Hosts' vender codes are, just add then as NULL
			// in order to prevent HostCache/KHL pollution done by wrong assumptions.
			if ( HostCache.Gnutella2.Add( strHost, 0, NULL ) ) nCount++; // Count it
		}
		m_sTryHubs.Empty();
	}
	else if ( m_sTryUltrapeers.GetLength() )
	{	// This header has been used for several things. In general, it's giving us a list of
		// Gnutella Ultrapeers, however some older versions of Shareaza can send G2 hubs in it,
		// if the client advertises G2 capability

		// TODO: Clean this up once there are very few of the older clients around

		int nCount = 0;
		// Append a comma onto the end of the value text once, and then loop forever
		for ( m_sTryUltrapeers += ',' ; ; ) // for (;;) is the same thing as forever
		{
			// Find the first comma in the value text
			int nPos = m_sTryUltrapeers.Find( ',' ); // Set nPos to the distance in characters from the start to the comma
			if ( nPos < 0 ) break;           // If no comma was found, leave the loop

			// Move the text before the comma from the value string to a new string for the host
			CString strHost = m_sTryUltrapeers.Left( nPos ); // Copy the text up to the comma into strHost
			m_sTryUltrapeers = m_sTryUltrapeers.Mid( nPos + 1 );     // Clip that text and the comma off the start of strValue

			// The remote computer accepts Gnutella2 connection.
			if ( m_bG2Accept || m_bG2Send )
			{
				// since there is no clever way to detect what the given Hosts' vender codes are, just add then as NULL
				// in order to prevent HostCache/KHL pollution done by wrong assumptions.
				//if ( HostCache.Gnutella2.Add( strHost, 0, NULL ) ) nCount++; // Count it
			}
			else if ( m_bG1Accept || m_bG1Send )	// This is a Gnutella connection, not Gnutella2
			{
				// Add the host to the Gnutella host cache
				if ( HostCache.Gnutella1.Add( strHost, 0, NULL ) ) nCount++;
			}
		}
		m_sTryUltrapeers.Empty();
	}

	// The remote computer called us and it hasn't said Ultrapeer or not.
	if ( !m_bInitiated && m_bUltraPeerSet == TRI_UNKNOWN )
	{
		// Really, we don't know if it's an ultrapeer or not, so assume remote client is leaf.
		m_bUltraPeerSet = TRI_FALSE;
	}

	if ( m_bDelayClose )
	{
		DelayClose( m_bDelayClose );
	}
	else if ( ( ( ! m_bInitiated && m_bG2Accept ) || ( m_bInitiated && m_bG2Send ) ) &&
			Settings.Gnutella2.EnableToday && m_nProtocol != PROTOCOL_G1 )
	{
		// This is a G2 connection
		m_nProtocol = PROTOCOL_G2;
		bResult = OnHeadersCompleteG2();
	}
	else if ( ( ( ! m_bInitiated && m_bG1Accept ) || ( m_bInitiated && m_bG1Send ) ) &&
			Settings.Gnutella1.EnableToday && m_nProtocol != PROTOCOL_G2 )
	{	// If the remote computer doesn't accept Gnutella2 packets, or it does but we contacted it and it's not going to send them
		// This is a Gnutella connection
		m_nProtocol = PROTOCOL_G1;
		bResult = OnHeadersCompleteG1();
	}
	else
	{
		SendHostHeaders( _P("GNUTELLA/0.6 503 Wrong Protocol\r\n") );
		DelayClose(IDS_HANDSHAKE_REJECTED);
	}
	return bResult;
}

// Called when CConnection::ReadHeaders calls ReadLine and gets a blank line, meaning a group of headers from the remote computer is done
// Responds to the group of headers from the remote computer by closing the connection, sending response headers, or turning this into a Gnutella2 object
// Returns false to delete this object, or true to keep reading headers and negotiating the handshake
BOOL CShakeNeighbour::OnHeadersCompleteG2()
{
	// Report that a set of Gnutella2 headers from a remote computer are complete
	theApp.Message( MSG_DEBUG, _T("Headers Complete: %s client"), protocolNames[ PROTOCOL_G2 ] );

	if ( m_bUltraPeerSet == TRI_TRUE )
	{
		HostCache.Gnutella2.Add( &m_pHost.sin_addr, htons( m_pHost.sin_port ) );
	}

	// The remote computer replied to our headers with something other than "200 OK"
	if ( m_nState == nrsRejected )
	{
		Close( 0 );   // Don't specify an error
		return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
	} // We've been reading response headers from a remote computer that contacted us
	else if ( m_nState == nrsHandshake3 ) // We were reading the final header group from the remote computer
	{
		// (do)
		if ( m_bUltraPeerSet == TRI_FALSE                                 // The remote computer told us it's a leaf
			&& ( Neighbours.IsG2Hub() || Neighbours.IsG2HubCapable() ) ) // And we're either a hub or capable of becoming one
		{
			// Report this case
			theApp.Message( MSG_INFO, IDS_HANDSHAKE_BACK2LEAF, (LPCTSTR)m_sAddress );

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

		// We are a Gnutella2 hub, or at least we are capable of becoming one
		if ( Neighbours.IsG2Hub() || Neighbours.IsG2HubCapable() )
		{
			// The remote computer sent us a header like "X-Ultrapeer: False"
			if ( m_bUltraPeerSet == TRI_FALSE )
			{
				// This connection is to a leaf below us
				m_nNodeType = ntLeaf;
			} // The remote computer sent us headers like "X-Ultrapeer: True" and "X-Ultrapeer-Needed: True"
			else if ( m_bUltraPeerSet == TRI_TRUE && m_bUltraPeerNeeded != TRI_FALSE )
			{// seems like GnucDNA type node do not give "X-Ultrapeer-needed:" header
				// Record that we are both hubs
				m_nNodeType = ntNode;
			} // The remote computer is an hub that doesn't need connections to more hubs
			else if ( m_bUltraPeerSet == TRI_TRUE && m_bUltraPeerNeeded == TRI_FALSE )
			{
				// If we are connected to any leaves
				if ( Neighbours.GetCount( PROTOCOL_G2, nrsConnected, ntLeaf ) > 0 )
				{
					// Tell the remote computer we can't connect (do)
					SendHostHeaders( _P("GNUTELLA/0.6 503 I have leaves") );
					DelayClose( IDS_HANDSHAKE_CANTBEPEER ); // Send the buffer then close the socket
					return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
				}

				// If we are a Gnutella2 hub
				if ( Settings.Gnutella2.ClientMode == MODE_HUB )
				{
					// We are a hub, and the remote computer doesn't need any more hub connections, tell it we can't connect
					SendHostHeaders( _P("GNUTELLA/0.6 503 Ultrapeer disabled") );
					DelayClose( IDS_HANDSHAKE_NOULTRAPEER ); // Send the buffer then close the socket
					return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
				}

				// We are or can become an ultrapeer, and the remote computer is an ultrapeer that doesn't need any more ultrapeer connections
				m_nNodeType = ntHub; // Pretend this connection is to a hub above us
				bFallback = TRUE;    // We'll tell the remote computer we're a leaf so we can still connect
			}
		} // The remote computer is a hub
		else if ( m_bUltraPeerSet == TRI_TRUE )
		{
			// And so are we
			if ( Settings.Gnutella2.ClientMode == MODE_HUB )
			{
				// (do)
				SendHostHeaders( _P("GNUTELLA/0.6 503 Ultrapeer disabled") );
				DelayClose( IDS_HANDSHAKE_NOULTRAPEER ); // Send the buffer then close the socket
				return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
			}

			// This connection is to a hub above us
			m_nNodeType = ntHub;
		} // The remote computer is a leaf
		else if ( m_bUltraPeerSet == TRI_FALSE )
		{
			m_nNodeType = ntLeaf;
			if ( Settings.Gnutella2.ClientMode == MODE_LEAF )
			{
				// Tell the remote computer we can't connect because we need a hub
				SendHostHeaders( _P("GNUTELLA/0.6 503 Need an Ultrapeer") );
				DelayClose( IDS_HANDSHAKE_NEEDAPEER ); // Send the buffer then close the socket
				return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
			}
		}
		else if ( m_bUltraPeerSet == TRI_UNKNOWN )
		{
			// We are a leaf
			if ( Settings.Gnutella2.ClientMode == MODE_LEAF )
			{
				// Tell the remote computer we can't connect because we need a hub
				SendHostHeaders( _P("GNUTELLA/0.6 503 Need an Ultrapeer") );
				DelayClose( IDS_HANDSHAKE_NEEDAPEER ); // Send the buffer then close the socket
				return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
			}
			m_nNodeType = ntNode;
		}

		// If the connection doesn't fall into any of those error cases, accept it with a batch of reply headers
		Write( _P("GNUTELLA/0.6 200 OK\r\n") );                 // Accept the connection
		SendPrivateHeaders();                                          // Respond to each header, setting up Gnutella2 packets and compression
		if ( bFallback ) Write( _P("X-Ultrapeer: False\r\n") ); // Tell it we're a leaf so we can still connect
		Write( _P("\r\n") );                                    // Send a blank line to end this group of headers

		// Turn this CShakeNeighbour object into a CG2Neighbour one, and delete this one
		OnHandshakeComplete();
		return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
	} // not rejected, not handshake3, not initiated (do)
	else
	{
		// We are a leaf
		if ( Neighbours.IsG2Leaf() )
		{
			// The remote computer sent us the header "X-Ultrapeer: False"
			if ( m_bUltraPeerSet == TRI_FALSE )
			{
				HostCache.Gnutella2.Remove( &m_pHost.sin_addr );
			}
			// Tell the remote computer we can't connect because we are a shielded leaf right now
			SendHostHeaders( _P("GNUTELLA/0.6 503 Shielded leaf node") );
			DelayClose( IDS_HANDSHAKE_IAMLEAF ); // Send the buffer then close the socket
			return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
		} // We are a Gnutella2 hub, or at least we are capable of becoming one
		else if ( Neighbours.IsG2Hub() || Neighbours.IsG2HubCapable() )
		{
			// The remote computer sent us the header "X-Ultrapeer: False"
			if ( m_bUltraPeerSet == TRI_FALSE )
			{
				// This connection is to a leaf below us
				m_nNodeType = ntLeaf;
			} // The remote computer sent us the header "X-Ultrapeer: True"
			else if ( m_bUltraPeerSet == TRI_TRUE )
			{
				// Record that we are both hubs
				m_nNodeType = ntNode;
			}
			else if ( m_bUltraPeerSet == TRI_UNKNOWN )
			{
				// Record that remote Node type is Unknown
				m_nNodeType = ntLeaf;
			}
		} // The remote computer is an ultrapeer, and we are not running in hub mode
		else if ( m_bUltraPeerSet == TRI_TRUE && ( Settings.Gnutella2.ClientMode != MODE_HUB ) )
		{
			// This connection is to a hub above us
			m_nNodeType = ntHub;
		}

		// If we don't need this connection
		if ( ( m_nNodeType == ntLeaf && m_bBadClient ) ||
			 ( m_nNodeType == ntLeaf && ! Neighbours.NeedMoreHubs( PROTOCOL_G2 ) && ! Neighbours.NeedMoreLeafs( PROTOCOL_G2 ) ) ||
			 ( m_nNodeType != ntLeaf && ! Neighbours.NeedMoreHubs( PROTOCOL_G2 ) ) ||
			 ( m_nNodeType == ntLeaf && ! m_bClientExtended && Neighbours.GetCount(PROTOCOL_G2, nrsConnected, ntLeaf ) > Settings.Gnutella2.NumLeafs - 5 ) )
		{
			// Tell the remote computer that we can't connect because we have too many connections already, and close the connection
			SendHostHeaders( _P("GNUTELLA/0.6 503 Maximum connections reached") ); // Send this error code along with some more IP addresses it can try
			DelayClose( IDS_HANDSHAKE_SURPLUS );
			return FALSE;
		} // Or, if we're both in the same role on the network
		else if ( ( m_nNodeType == ntHub && ( Settings.Gnutella2.ClientMode == MODE_HUB ) ) ||  // We're both hubs
				  ( m_nNodeType == ntLeaf && ( Settings.Gnutella2.ClientMode == MODE_LEAF ) ) ) // We're both leaves
		{
			// Tell the remote computer that we can't connect
			SendHostHeaders( _P("GNUTELLA/0.6 503 Ultrapeer disabled") ); // Send the error code along with more IP addresses the remote computer can try
			DelayClose( IDS_HANDSHAKE_NOULTRAPEER ); // Close the connection, but not until we've written the buffered outgoing data first
			return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
		} // This connection isn't to a hub above us, and we're a leaf
		else if ( m_nNodeType != ntHub && ( Settings.Gnutella2.ClientMode == MODE_LEAF ) )
		{
			// Tell the remote computer we can't connect because we need a hub
			SendHostHeaders( _P("GNUTELLA/0.6 503 Need an Ultrapeer") );
			DelayClose( IDS_HANDSHAKE_NEEDAPEER ); // Send the buffer then close the socket
			return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
		} // Otherwise, the connection is probably alright
		else
		{
			// Send reply headers to the remote computer
			Write( _P("GNUTELLA/0.6 200 OK\r\n") );	// Start our group of response headers to the other computer with the 200 OK message
			SendPublicHeaders();							// Send the initial Gnutella2 headers
			SendPrivateHeaders();							// Send headers in response to those we got from the remote computer
			// maybe good idea, but it should not send Host headers if negotiated successfully.
			//SendHostHeaders();								// Send the "X-Try-Ultrapeers" header with a list of other IP addresses running Gnutella

			// End this block of headers with a blank line
			Write( _P("\r\n") );
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
	theApp.Message( MSG_DEBUG, _T("Headers Complete: %s client"), protocolNames[ PROTOCOL_G1 ] );

	if ( m_bUltraPeerSet == TRI_TRUE )
	{
		HostCache.Gnutella1.Add( &m_pHost.sin_addr, htons( m_pHost.sin_port ) );
	}

	// Check if Gnutella1 is enabled before connecting to a gnutella client
	if ( ! Settings.Gnutella1.EnableToday && m_nState < nrsRejected ) // And make sure the state is before getting rejected
	{
		// Tell the remote computer that we're only connecting to Gnutella2 today
		Write( _P("GNUTELLA/0.6 503 G2 Required\r\n") );
		SendMinimalHeaders();       // Tell the remote computer we're Shareaza and we can exchange Gnutella2 packets
		Write( _P("\r\n") ); // End the group of headers with a blank line

		// Close the connection citing not Gnutella2 as the reason, but send the departing buffer first
		DelayClose( IDS_HANDSHAKE_NOTG2 );

		// Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
		return FALSE;
	}

	// The remote computer started with "GNUTELLA/0.6", but did not say "200 OK"
	if ( m_nState == nrsRejected )
	{
		Close( 0 );   // Don't specify an error
		return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
	} // We've been reading response headers from a remote computer that contacted us
	else if ( m_nState == nrsHandshake3 ) // We're reading the final header group from the remote computer
	{
		// If we're both leaves, and yet somehow we're also either an ultrapeer or can become one (do)
		if ( m_bUltraPeerSet == TRI_FALSE && m_nNodeType == ntNode &&               // The remote computer is a hub and so are we
			 ( Neighbours.IsG1Ultrapeer() || Neighbours.IsG1UltrapeerCapable() ) ) // And, we're either a Gnutella ultrapeer or we could become one
		{
			// Report that the handshake is back to a leaf (do), and consider this connection to be to a leaf below us
			theApp.Message( MSG_INFO, IDS_HANDSHAKE_BACK2LEAF, (LPCTSTR)m_sAddress );
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

		// We are an ultrapeer or at least we are capable of becoming one
		if ( Neighbours.IsG1Ultrapeer() || Neighbours.IsG1UltrapeerCapable() )
		{
			// The remote computer told us "X-Ultrapeer: False"
			if ( m_bUltraPeerSet == TRI_FALSE )
			{
				// This connection is to a leaf below us
				m_nNodeType = ntLeaf;
			} // The remote computer told us it's an ultrapeer and that it needs connections to more ultrapeers
			else if ( m_bUltraPeerSet == TRI_TRUE && m_bUltraPeerNeeded != TRI_FALSE )
			{// seems like GnucDNA type node do not give "X-Ultrapeer-needed:" header
				// Record that we are both ultrapeers
				m_nNodeType = ntNode;
			} // The remote computer is an ultrapeer that doesn't need any more ultrapeer connections
			else if ( m_bUltraPeerSet == TRI_TRUE && m_bUltraPeerNeeded == TRI_FALSE )
			{
				// (do)
				if ( Neighbours.GetCount( PROTOCOL_G1, nrsConnected, ntLeaf ) > 0 )
				{
					// Tell the remote computer we can't connect (do)
					SendHostHeaders( _P("GNUTELLA/0.6 503 I have leaves") );
					DelayClose( IDS_HANDSHAKE_CANTBEPEER ); // Send the buffer then close the socket
					return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
				}

				// We are an ultrapeer
				if ( Settings.Gnutella1.ClientMode == MODE_ULTRAPEER )
				{
					// Tell the remote computer we can't connect
					SendHostHeaders( _P("GNUTELLA/0.6 503 Ultrapeer disabled") );
					DelayClose( IDS_HANDSHAKE_NOULTRAPEER ); // Send the buffer then close the socket
					return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
				}

				// We are or can become an ultrapeer, and the remote computer is an ultrapeer that doesn't need any more ultrapeer connections
				m_nNodeType = ntHub; // Pretend this connection is to a hub above us
				bFallback = TRUE;    // We'll tell the remote computer we're a leaf so we can still connect
			}
		} // We're a leaf, and the remote computer is an ultrapeer
		else if ( m_bUltraPeerSet == TRI_TRUE )
		{
			// We are an ultrapeer
			if ( Settings.Gnutella1.ClientMode == MODE_ULTRAPEER )
			{
				// Tell the remote computer we can't connect
				SendHostHeaders( _P("GNUTELLA/0.6 503 Ultrapeer disabled") );
				DelayClose( IDS_HANDSHAKE_NOULTRAPEER ); // Send the buffer then close the socket
				return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
			}

			// This connection is to a hub above us
			m_nNodeType = ntHub;
		} // The remote computer is a leaf
		else if ( m_bUltraPeerSet != TRI_TRUE )
		{
			// And so are we
			if ( Settings.Gnutella1.ClientMode == MODE_LEAF )
			{
				// Tell the remote computer that we can't connect because we're both leaves
				SendHostHeaders( _P("GNUTELLA/0.6 503 Need an Ultrapeer") );
				DelayClose( IDS_HANDSHAKE_NEEDAPEER ); // Send the buffer, close the socket, cite need a peer as the reason
				return FALSE;                          // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
			}
		}

		// Tell the remote computer that we accept the connection, and send reply headers
		Write( _P("GNUTELLA/0.6 200 OK\r\n") ); // Begin our response headers with the OK message
		SendPrivateHeaders();                          // Respond to each header, setting up Gnutella2 packets and compression

		// If we are an ultrapeer or could become one, but the remote computer is an ultrapeer that doesn't need any more ultrapeer connections
		if ( bFallback ) Write( _P("X-Ultrapeer: False\r\n") ); // Tell it we're a leaf so we can still connect

		// Send a blank line to end this group of headers
		Write( _P("\r\n") );

		// Turn this CShakeNeighbour object into a CG1Neighbour one, and delete this one
		OnHandshakeComplete();
		return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
	} // not rejected, not handshake3, not initiated (do)
	else
	{
		// We are a leaf
		if ( Neighbours.IsG1Leaf() )
		{
			// The remote computer told us it is a leaf
			if ( m_bUltraPeerSet == TRI_FALSE )
			{
				HostCache.Gnutella1.Remove( &m_pHost.sin_addr );
			}
			// Tell the remote computer we can't connect because we are a shielded leaf right now
			SendHostHeaders( _P("GNUTELLA/0.6 503 Shielded leaf node") );
			DelayClose( IDS_HANDSHAKE_IAMLEAF ); // Send the buffer and then close the socket citing our being a leaf as the reason
			return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
		} // We are an ultrapeer, or at least we are capable of becoming one
		else if ( Neighbours.IsG1Ultrapeer() || Neighbours.IsG1UltrapeerCapable() )
		{
			// The remote computer told us it is a leaf
			if ( m_bUltraPeerSet == TRI_FALSE )
			{
				// This connection is to a leaf below us
				m_nNodeType = ntLeaf;
			} // The remote computer told us it is an ultrapeer
			else if ( m_bUltraPeerSet == TRI_TRUE )
			{
				// Record that we are both ultrapeers
				m_nNodeType = ntNode;
			}
		} // The remote computer is an ultrapeer, but we are just a leaf
		else if ( m_bUltraPeerSet == TRI_TRUE && ( Settings.Gnutella1.ClientMode != MODE_ULTRAPEER ) )
		{
			// This connection is to a hub above us
			m_nNodeType = ntHub;
		}

		// If we don't need this connection
		if ( ( m_nNodeType == ntLeaf && m_bBadClient ) ||
			 ( m_nNodeType == ntLeaf && ! Neighbours.NeedMoreHubs( PROTOCOL_G1 ) && ! Neighbours.NeedMoreLeafs( PROTOCOL_G1 ) ) ||
			 ( m_nNodeType != ntLeaf && ! Neighbours.NeedMoreHubs( PROTOCOL_G1 ) ) )
		{
			// Tell the remote computer we can't connect because we already have too many connections
			SendHostHeaders( _P("GNUTELLA/0.6 503 Maximum connections reached") );
			DelayClose( IDS_HANDSHAKE_SURPLUS );
			return FALSE;
		} // Or, if we're both in the same role on the network
		else if ( ( m_nNodeType == ntHub && ( Settings.Gnutella1.ClientMode == MODE_ULTRAPEER ) ) || // This connection is up to a hub, and we are a hub
				  ( m_nNodeType == ntLeaf && ( Settings.Gnutella1.ClientMode == MODE_LEAF ) ) )      // Or, this connection is down to a leaf like us
		{
			// Tell the remote computer we can't connect
			SendHostHeaders( _P("GNUTELLA/0.6 503 Ultrapeer disabled") );
			DelayClose( IDS_HANDSHAKE_NOULTRAPEER ); // Send the buffer then close the socket
			return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
		} // Weed out another nonsense combination that should never happen
		else if ( m_nNodeType != ntHub && ( Settings.Gnutella1.ClientMode == MODE_LEAF ) )
		{
			// Tell the remote computer we can't connect
			SendHostHeaders( _P("GNUTELLA/0.6 503 Need an Ultrapeer") );
			DelayClose( IDS_HANDSHAKE_NEEDAPEER ); // Send the buffer then close the socket
			return FALSE; // Return false all the way back to CHandshakes::RunHandshakes, which will delete this object
		} // Otherwise, the connection is probably alright
		else
		{
			// Send reply headers to the remote computer
			Write( _P("GNUTELLA/0.6 200 OK\r\n") );	// Start our group of response headers to the other computer with the 200 OK message
			SendPublicHeaders();							// Send the initial Gnutella headers
			SendPrivateHeaders();							// Send headers in response to those we got from the remote computer
			// maybe good idea, but it should not send Host headers if negotiated successfully.
			//SendHostHeaders();								// Send the "X-Try-Ultrapeers" header with a list of other IP addresses running Gnutella

			// End this block of headers with a blank line
			Write( _P("\r\n") );
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
	m_bAutoDelete = FALSE;

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
	if ( Settings.Connection.SendBuffer )
	{
		setsockopt( m_hSocket, SOL_SOCKET, SO_SNDBUF, (char*)&Settings.Connection.SendBuffer, 4 );
	}

	// Make a pointer for a new object that will be a copy of this one, but in a different place on the CConnection inheritance tree
	CNeighbour* pNeighbour = NULL;

	// Remove leaf from host cache
	if ( m_nNodeType == ntLeaf )
	{
		HostCache.OnFailure( &m_pHost.sin_addr, htons( m_pHost.sin_port ), m_nProtocol, true );
	}

	// If the remote computer is G2, or can send and understand Gnutella2 packets and isn't G1
	if ( m_bG2Send && m_bG2Accept && m_nProtocol != PROTOCOL_G1 )
	{
		// Add good hub to host cache
		if ( m_nNodeType == ntHub || m_nNodeType == ntNode )
		{
			HostCache.OnSuccess( &m_pHost.sin_addr, htons( m_pHost.sin_port ), m_nProtocol, true );
		}

		// check if this connection is still needed at this point
		if ( ! m_bAutomatic &&
			( m_nNodeType == ntHub || m_nNodeType == ntNode ) &&
			! Neighbours.NeedMoreHubs( PROTOCOL_G2 ) )
		{
			// Free slot for this neighbour
			CNeighbour* pOldNeighbour = Neighbours.GetNewest( PROTOCOL_G2, nrsConnected, m_nNodeType );
			if ( pOldNeighbour )
				pOldNeighbour->Close( IDS_CONNECTION_CLOSED );
		}
		if ( m_nNodeType == ntLeaf && ! Neighbours.NeedMoreLeafs( PROTOCOL_G2 ) )
		{
			delete this;
			return;
		}

		// Record that the remote computer supports query routing
		m_bQueryRouting = TRUE;

		// Make a new Gnutella2 neighbour object by copying values from this ShakeNeighbour one
		pNeighbour = new CG2Neighbour( this );
	}
	else if (  m_bG1Send && m_bG1Accept && m_nProtocol != PROTOCOL_G2 )
	{
		// Add good hub to host cache
		if ( m_nNodeType == ntHub || m_nNodeType == ntNode )
		{
			HostCache.OnSuccess( &m_pHost.sin_addr, htons( m_pHost.sin_port ), m_nProtocol, true );
		}

		// check if this connection is still needed at this point
		if ( ! m_bAutomatic &&
			( m_nNodeType == ntHub || m_nNodeType == ntNode ) &&
			! Neighbours.NeedMoreHubs( PROTOCOL_G1 ) )
		{
			// Free slot for this neighbour
			CNeighbour* pOldNeighbour = Neighbours.GetNewest( PROTOCOL_G1, nrsConnected, m_nNodeType );
			if ( pOldNeighbour )
				pOldNeighbour->Close( IDS_CONNECTION_CLOSED );
		}
		if ( m_nNodeType == ntLeaf && ! Neighbours.NeedMoreLeafs( PROTOCOL_G1 ) )
		{
			delete this;
			return;
		}

		// The remote computer is just Gnutella, not Gnutella2
		// Make a new Gnutella neighbour object by copying values from this ShakeNeighbour one
		pNeighbour = new CG1Neighbour( this );
	}

	if ( pNeighbour == NULL )
	{
	}// This connection is to a hub above us
	else if ( m_nNodeType == ntHub )
	{
		// Report that we got a ultrapeer connection
		theApp.Message( MSG_INFO, IDS_HANDSHAKE_GOTPEER );

		// (do)
		Neighbours.PeerPrune( pNeighbour->m_nProtocol );
	} // This connection is to a leaf below us
	else if ( m_nNodeType == ntLeaf )
	{
		// Report that we connected to a leaf
		theApp.Message( MSG_INFO, IDS_HANDSHAKE_GOTLEAF, (LPCTSTR)m_sAddress );
	}

	// Delete this CShakeNeighbour object now that it has been turned into a CG1Neighbour or CG2Neighbour object
	delete this;
}