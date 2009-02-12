//
// Handshake.cpp
//
// Copyright © Shareaza Development Team, 2002-2009.
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

// CHandshake figures out what the remote computer wants from the first 7 bytes it sends us
// http://shareazasecurity.be/wiki/index.php?title=Developers.Code.CHandshake

// Copy in the contents of these files here before compiling
#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "Handshakes.h"
#include "Handshake.h"
#include "Neighbours.h"
#include "Downloads.h"
#include "Uploads.h"
#include "UploadTransfer.h"
#include "ChatCore.h"
#include "Network.h"
#include "Buffer.h"
#include "GProfile.h"
#include "BTClients.h"
#include "EDClients.h"
#include "EDPacket.h"
#include "WndMain.h"
#include "WndChat.h"

// If we are compiling in debug mode, replace the text "THIS_FILE" in the code with the name of this file
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CHandshake construction

// Make a new CHandshake object
// Initializes the member variables with default values
CHandshake::CHandshake()
{
	// We did not connect to the remote computer as part of a push
	m_bPushing = FALSE;

	// Set pointers so the input and output bandwidth limits are read from the DWORD in settings
	m_mInput.pLimit = m_mOutput.pLimit = &Settings.Bandwidth.Request;
}

// Called when a remote computer wants to connect to us
// Make a new CHanshake object given a socket, and a MFC SOCKADDR_IN structure which contains an IP address and port number
// Uses AcceptFrom to make a new object with this socket and ip address
CHandshake::CHandshake(SOCKET hSocket, SOCKADDR_IN* pHost)
{
	// We did not connect to the remote computer as part of a push
	m_bPushing = FALSE;

	// Call CConnection::AcceptFrom to setup this object with the socket and
	AcceptFrom( hSocket, pHost );

	// Set pointers so the input and output bandwidth limits are read from the DWORD in settings
	m_mInput.pLimit = m_mOutput.pLimit = &Settings.Bandwidth.Request;

	// Record that the program accepted this connection
	theApp.Message( MSG_INFO, IDS_CONNECTION_ACCEPTED, (LPCTSTR)m_sAddress, htons( m_pHost.sin_port ) );
}

// Copy a CHandshake object
// Takes pCopy, a pointer to the CHandshake object to make this new one a copy of
CHandshake::CHandshake(CHandshake* pCopy)
{
	// Call CConnection::AttachTo to copy the CConnection core of pCopy over to this new CHandshake object
	AttachTo( pCopy );

	// Then, copy across the CHandshake member variables, since AttachTo just does the CConnection ones
	m_bPushing		= pCopy->m_bPushing;	// Copy in whether or not we connected to the remote computer as part of a push
	m_nIndex		= pCopy->m_nIndex;		// Copy across the handshake index (do)

	// Set pointers so the input and output bandwidth limits are read from the DWORD in settings
	m_mInput.pLimit = m_mOutput.pLimit = &Settings.Bandwidth.Request;
}

// Delete this CHandshake object
CHandshake::~CHandshake()
{
	// There is nothing the CConnection destructor won't take care of
}

//////////////////////////////////////////////////////////////////////
// CHandshake push

// Open a connection to a new remote computer
// Takes the IP address and port number of the remote computer, as well as the index (do)
// Connects this socket to the new computer, but does not make any communications yet
BOOL CHandshake::Push(IN_ADDR* pAddress, WORD nPort, DWORD nIndex)
{
	// Report we are about to push open a connection to the given IP address and port number now
	theApp.Message( MSG_INFO, IDS_UPLOAD_CONNECT, (LPCTSTR)CString( inet_ntoa( *pAddress ) ), _T("") );

	// Connect the socket in this CHandshake object to the IP address and port number the method got passed
	if ( ! ConnectTo( pAddress, nPort ) ) return FALSE; // If the connection was not made, leave now

	// Tell Windows to give us a m_pWakeup event if this socket connects, gets data, is ready for writing, or closes
	WSAEventSelect(				// Specify our event object Handshakes.m_pWakeup to the FD_CONNECT FD_READ FD_WRITE and FD_CLOSE events
		m_hSocket,				// The socket in this CHandshake object
		Handshakes.GetWakeupEvent(),	// The MFC CEvent object we've made for the wakeup event
		FD_CONNECT	|			// The connection has been made
		FD_READ		|			// There is data from the remote computer on our end of the socket waiting for us to read it
		FD_WRITE	|			// The remote computer is ready for some data from us (do)
		FD_CLOSE );				// The socket has been closed

	// Record the push in the member variables
	m_bPushing		= TRUE;				// We connected to this computer as part of a push
	m_tConnected	= GetTickCount();	// Record that we connected right now
	m_nIndex		= nIndex;			// Copy the given index number into this object (do)

	// Report success
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CHandshake run event

// Determine the handshake has been going on for too long
// Returns true or false
BOOL CHandshake::OnRun()
{
	// If we've been connected for longer than the handshake timeout from settings
	if ( GetTickCount() - m_tConnected > Settings.Connection.TimeoutHandshake )
	{
		// Report this, and return false
		theApp.Message( MSG_ERROR, IDS_HANDSHAKE_TIMEOUT, (LPCTSTR)m_sAddress );
		return FALSE;
	}

	// We still have time for the handshake
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CHandshake connection events

// Send the remote computer our client index and guid in a header like "GIV index:guid/"
BOOL CHandshake::OnConnected()
{
	// Call CConnection's OnConnected method first, even though it does nothing (do)
	CConnection::OnConnected();

	// copy Profile's GUID
	Hashes::Guid oID( MyProfile.oGUID );

	// Compose the GIV string, which is like "GIV index:guid/" with two newlines at the end (do)
	CString strGIV;
	strGIV.Format( // MFC's CString::Format is like sprintf, "%.2X" formats a byte into 2 hexidecimal characters like "ff"
		_T("GIV %u:%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X/\n\n"),
		m_nIndex,											// Our index on the Gnutella network (do)
		int( oID[0] ),  int( oID[1] ),  int( oID[2] ),  int( oID[3] ),		// Our GUID
		int( oID[4] ),  int( oID[5] ),  int( oID[6] ),  int( oID[7] ),
		int( oID[8] ),  int( oID[9] ),  int( oID[10] ), int( oID[11] ),
		int( oID[12] ), int( oID[13] ), int( oID[14] ), int( oID[15] ) );

	// Print the string into the output buffer, and write the output buffer to the remote computer
	Write( strGIV );
	OnWrite();

	// Record that we uploaded the giv, and report success
	theApp.Message( MSG_INFO, IDS_UPLOAD_GIV, (LPCTSTR)m_sAddress );
	return TRUE;
}

// If we connected to the remote computer as part of a push, record that we couldn't connect to do the upload
void CHandshake::OnDropped()
{
	// If we connected to the remote computer as part of a push
	if ( m_bPushing )
	{
		// Record a upload connect error
		theApp.Message( MSG_ERROR, IDS_UPLOAD_CONNECT_ERROR, (LPCTSTR)m_sAddress );
	}
}

//////////////////////////////////////////////////////////////////////
// CHandshake read event

// Reads the first few bytes from the other computer to figure out what it wants
// Returns true if it needs more information, false if it's done
BOOL CHandshake::OnRead()
{
	// Read data waiting in the socket into the input buffer
	CConnection::OnRead();

	// We need at least 7 bytes of headers from the remote compuer to figure out what network its talking about
	if ( GetInputLength() < 7 ) return TRUE; // Not enough information yet, leave now returning true

	// Determine if the remote computer has sent an eDonkey2000 hello packet
	if ( GetInputLength() >= 7							&& // 7 or more bytes have arrived
		 PeekAt( 0 ) == ED2K_PROTOCOL_EDONKEY	&& // The first byte is "e3", indicating eDonkey2000
		 PeekAt( 5 ) == ED2K_C2C_HELLO			&& // 5 bytes in is "01", a hello for that network
		 PeekAt( 6 ) == 0x10 )					   // And after that is "10"
	{
		// Have the EDClients object accept this CHandshake as a new eDonkey2000 computer
		EDClients.OnAccept( this );
		return FALSE; // Return false to indicate that we are done sorting the handshake
	}

	// See if the remote computer is speaking BitTorrent
	if ( GetInputLength() >= BT_PROTOCOL_HEADER_LEN && // We have at least 20 bytes
		 StartsWith( BT_PROTOCOL_HEADER, BT_PROTOCOL_HEADER_LEN ) ) // They are "\023BitTorrent protocol"
	{
		// Have the BTClients object accept this CHandshake as a new BitTorrent computer
		BTClients.OnAccept( this );
		return FALSE; // Done sorting the handshake
	}

	// With eDonkey2000 and BitTorrent out of the way, now we can look for text-based handshakes

	// Read the first header line
	CString strLine;
	if ( ! Read( strLine, TRUE ) ) // Read characters until \n, returning false if there is no \n
	{
		// The remote computer hasn't sent a \n yet, if there are more than 2048 bytes in the first line, abort
		return ( GetInputLength() < 2048 ) ? TRUE : FALSE; // Return false to signal we are done sorting the handshake
	}

	// The first line the remote computer sent was blank
	if ( strLine.IsEmpty() )
	{
		// Eat blank lines, just read the next line into strLine and return true
		Read( strLine ); // But strLine is never used? (do)
		return TRUE; // Keep trying to sort the handshake
	}

	// Record this handshake line as an application message
	theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING, _T("%s >> HANDSHAKE: %s"), (LPCTSTR)m_sAddress, (LPCTSTR)strLine );

	// The first header starts "GET" or "HEAD"
	if (     StartsWith( _P("GET ") ) ||
		     StartsWith( _P("HEAD ") ) )
	{
		// The remote computer wants a file from us, accept the connection as an upload
		Uploads.OnAccept( this );
	}
	else if ( StartsWith( _P("GNUTELLA") ) )
	{
		// Gnutella handshake
		Neighbours.OnAccept( this );
	}
	else if ( StartsWith( _P("PUSH ") ) )
	{
		// Gnutella2-style push
		OnAcceptPush();
	}
	else if ( StartsWith( _P("GIV ") ) )
	{
		// Gnutella giv
		OnAcceptGive();
	}
	else if ( StartsWith( _P("CHAT") ) )
	{
		// If the user has setup a valid profile and enabeled chat in the program settings
		if ( MyProfile.IsValid() && Settings.Community.ChatEnable )
		{
			// Have the chat system accept this connection
			ChatCore.OnAccept( this );
		}
		else
		{
			// Otherwise, tell the other computer we can't chat
			Write( _P("CHAT/0.2 404 Unavailable\r\n\r\n") );
			OnWrite();
		}
	}
	else
	{
		// The first header starts with something else, report that we couldn't figure out the handshake
		theApp.Message( MSG_ERROR, IDS_HANDSHAKE_FAIL, (LPCTSTR)m_sAddress );
	}

	// Return false to indicate that we are done sorting the handshake
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CHandshake accept Gnutella2-style PUSH

// When the first thing a remote computer says is a Gnutella2 "PUSH", this method gets called
// Checks if a child window recognizes the guid
// Returns true or false
BOOL CHandshake::OnAcceptPush()
{
	// Make a string for the header line, and variables to hold the GUID in string and binary forms
	CString strLine, strGUID;
	Hashes::Guid oGUID;

	// Read the first line from the input buffer, this doesn't remove it so we can call it over and over again
	if ( ! Read( strLine ) ) return FALSE;

	// Make sure the line has the format "PUSH guid:GUIDinHEXguidINhexGUIDinHEXguidI", which has 10 characters before the 32 for the guid
	if ( strLine.GetLength() != 10 + 32 )
	{
		// Report the bad push and return reporting error
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_BAD_PUSH, (LPCTSTR)CString( inet_ntoa( m_pHost.sin_addr ) ) );
		return FALSE;
	}

	// Read the 16 hexidecimal digits of the GUID, copying it into pGUID
	for ( int nByte = 0 ; nByte < 16 ; nByte++ )
	{
		int nValue;
		_stscanf( strLine.Mid( 10 + nByte * 2, 2 ), _T("%X"), &nValue );
		oGUID[ nByte ] = (BYTE)nValue;
	}
	oGUID.validate();

	// If a child window recongizes the GUID, accept the push
	if ( OnPush( oGUID ) ) return TRUE;

	// Record the fact that we got a push we knew nothing about, and return false to not accept it
	theApp.Message( MSG_ERROR, IDS_DOWNLOAD_UNKNOWN_PUSH, (LPCTSTR)CString( inet_ntoa( m_pHost.sin_addr ) ), _T("Gnutella2") );
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CHandshake accept Gnutella1-style GIV

// If the first thing the remote computer says to us is a Gnutella "GIV ", this method gets called
// Checks if a child window recognizes the guid
// Returns true or false
BOOL CHandshake::OnAcceptGive()
{
	// Local variables for the searching and parsing
	CString strLine, strClient, strFile;	// Strings for the whole line, the client guid hexidecimal characters, and the file name within it
	DWORD nFileIndex = 0xFFFFFFFF;			// Start out the file index as -1 to detect if we were able to read it
	Hashes::Guid oClientID;					// We will translate the GUID into binary here
	int nPos;								// The distance from the start of the string to a colon or slash we will look for

	// The first line should be like "GIV 124:d51dff817f895598ff0065537c09d503/my%20song.mp3"
	if ( ! Read( strLine ) ) return FALSE; // If the line isn't all there yet, return false to try again later

	// If there is a slash in the line
	if ( ( nPos = strLine.Find( '/' ) ) > 0 ) // Find returns the 0-based index in the string, -1 if not found
	{
		// Clip out the part of the line after the slash, URL decode it to turn %20 into spaces, and save that in strFile
		strFile	= URLDecode( strLine.Mid( nPos + 1 ) );	// Mid takes part after the slash
		strLine	= strLine.Left( nPos );					// Left removes that part and the slash from strLine
	}

	// If there is a colon in the line more than 4 character widths in, like "GIV 123:client32characterslong----------"
	if ( ( nPos = strLine.Find( ':' ) ) > 4 )
	{
		// Read the number before and text after the colon
		strClient	= strLine.Mid( nPos + 1 );			// Clip out just the "client" part of the example shown above
		strLine		= strLine.Mid( 4, nPos - 4 );		// Starting at 4 to get beyond the "GIV ", clip out "123", any text before the colon at nPos
		_stscanf( strLine, _T("%lu"), &nFileIndex );	// Read "123" as a number, this is the file index
	}

	// If there was no colon and the file index was not read, or the client text isn't exactly 32 characters long
	if ( nFileIndex == 0xFFFFFFFF || strClient.GetLength() != 32 )
	{
		// Make a record of this bad push and leave now
		theApp.Message( MSG_ERROR, IDS_DOWNLOAD_BAD_PUSH, (LPCTSTR)CString( inet_ntoa( m_pHost.sin_addr ) ) );
		return FALSE;
	}

	// The client id text is a 16-byte guid written in 32 characters with text like "00" through "ff" for each byte, read it into pClientID
	for ( int nByte = 0 ; nByte < 16 ; nByte++ )
	{
		// Convert one set of characters like "00" or "ff" into that byte in pClientID
		_stscanf( strClient.Mid( nByte * 2, 2 ), _T("%X"), &nPos );
		oClientID[ nByte ] = (BYTE)nPos;
	}
	oClientID.validate();

	// If a child window recognizes this guid, return true
	if ( OnPush( oClientID ) ) return TRUE;

	// If the file name is longer than 256 characters, change it to the text "Invalid Filename"
	if ( strFile.GetLength() > 256 ) strFile = _T("Invalid Filename");

	// Log this unexpected push, and return false
	theApp.Message( MSG_ERROR, IDS_DOWNLOAD_UNKNOWN_PUSH, (LPCTSTR)CString( inet_ntoa( m_pHost.sin_addr ) ), (LPCTSTR)strFile );
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CHandshake accept a push from a GUID

// Takes the GUID of a remote computer which has sent us a Gnutella2-style PUSH handshake
// Sees if any child windows recognize the GUID
// Returns true or false
BOOL CHandshake::OnPush(const Hashes::Guid& oGUID)
{
	// Make sure the socket is valid
	if ( ! IsValid() ) return FALSE;

	// Look for the remote computer's GUID in our list of downloads and the chat interface
	if ( Downloads.OnPush( oGUID, this ) ) return TRUE; // Return true if it's found
	if ( ChatCore.OnPush( oGUID, this ) ) return TRUE;

	// Make sure this is the only thread doing this right now
	CSingleLock pWindowLock( &theApp.m_pSection );
	if ( pWindowLock.Lock( 250 ) ) // Don't wait here for more than a quarter second, Lock will return false and so will OnPush
	{
		// Access granted, get a pointer to the windowing system
		if ( CMainWnd* pMainWnd = theApp.SafeMainWnd() )
		{
			// Get a pointer to the main Shareaza window
			CWindowManager* pWindows	= &pMainWnd->m_pWindows;
			CChildWnd* pChildWnd		= NULL;

			// Loop through all of Shareaza's child windows
			while ( ( pChildWnd = pWindows->Find( NULL, pChildWnd ) ) != NULL )
			{
				// If a child window recognizes this push request, return true
				if ( pChildWnd->OnPush( oGUID, this ) ) return TRUE;
			}
		}

		// Let other threads use theApp.m_pSection
		pWindowLock.Unlock();
	}

	// No child window recognized a push request, or we waited more than a quarter second and gave up
	return FALSE;
}
