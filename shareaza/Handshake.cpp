//
// Handshake.cpp
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

// CHandshake figures out what the remote computer wants from the first 7 bytes it sends us
// http://shareazasecurity.be/wiki/index.php?title=Developers.Code.CHandshake

#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "BTClients.h"
#include "BTPacket.h"
#include "Buffer.h"
#include "ChatCore.h"
#include "DCClients.h"
#include "DCPacket.h"
#include "EDClients.h"
#include "EDPacket.h"
#include "GProfile.h"
#include "Handshake.h"
#include "Handshakes.h"
#include "Neighbours.h"
#include "Network.h"
#include "UploadTransfer.h"
#include "Uploads.h"
#include "WndChat.h"
#include "WndMain.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CHandshake construction

CHandshake::CHandshake()
	: m_bPushing( FALSE )
	, m_nIndex	( 0 )
{
	// Set pointers so the input and output bandwidth limits are read from the DWORD in settings
	m_mInput.pLimit = m_mOutput.pLimit = &Settings.Bandwidth.Request;
}

// Called when a remote computer wants to connect to us
// Make a new CHanshake object given a socket, and a MFC SOCKADDR_IN structure which contains an IP address and port number
// Uses AcceptFrom to make a new object with this socket and ip address
CHandshake::CHandshake(SOCKET hSocket, SOCKADDR_IN* pHost)
	: m_bPushing( FALSE )
	, m_nIndex	( 0 )
{
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
	CConnection::OnConnected();

	// copy Profile's GUID
	Hashes::Guid oID( MyProfile.oGUID );

	// Compose the GIV string, which is like "GIV index:guid/" with two newlines at the end (do)
	CString strGIV;
	strGIV.Format(
		_T("GIV %u:%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X/\n\n"),
		m_nIndex,
		int( oID[0] ),  int( oID[1] ),  int( oID[2] ),  int( oID[3] ),
		int( oID[4] ),  int( oID[5] ),  int( oID[6] ),  int( oID[7] ),
		int( oID[8] ),  int( oID[9] ),  int( oID[10] ), int( oID[11] ),
		int( oID[12] ), int( oID[13] ), int( oID[14] ), int( oID[15] ) );

	Write( strGIV );

	LogOutgoing();

	OnWrite();

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
	if ( ! CConnection::OnRead() )
		return FALSE;

	DWORD nLength = GetInputLength();

	if ( nLength < 3 )
		// Not enough information yet, leave now returning true
		return TRUE;

	// Determine if the remote computer has sent an eDonkey2000 hello packet
	if ( nLength >= 7							&& // 7 or more bytes have arrived
		 PeekAt( 0 ) == ED2K_PROTOCOL_EDONKEY	&& // The first byte is "e3", indicating eDonkey2000
		 PeekAt( 5 ) == ED2K_C2C_HELLO			&& // 5 bytes in is "01", a hello for that network
		 PeekAt( 6 ) == 0x10 )					   // And after that is "10"
	{
		return EDClients.OnAccept( this );
	}

	// See if the remote computer is speaking BitTorrent
	if ( nLength >= BT_PROTOCOL_HEADER_LEN && // We have at least 20 bytes
		 StartsWith( BT_PROTOCOL_HEADER, BT_PROTOCOL_HEADER_LEN ) ) // They are "\023BitTorrent protocol"
	{
		return BTClients.OnAccept( this );
	}

	// See if the remote computer is speaking DC++
	if ( nLength >= DC_PROTOCOL_MIN_LEN &&
		 PeekAt( 0 ) == '$' &&
		 PeekAt( nLength - 1 ) == '|' )
	{
		return DCClients.OnAccept( this );
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

	if ( StartsWith( _P("GNUTELLA") ) )
	{
		// Gnutella handshake
		return Neighbours.OnAccept( this );
	}
	else if ( StartsWith( _P("CHAT") ) )
	{
		return ChatCore.OnAccept( this );
	}

	// Record this handshake line as an application message
	theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING, _T("%s >> HANDSHAKE: %s"), (LPCTSTR)m_sAddress, (LPCTSTR)strLine );

	// The first header starts "GET" or "HEAD"
	if (     StartsWith( _P("GET ") ) ||
		     StartsWith( _P("HEAD ") ) )
	{
		// The remote computer wants a file from us, accept the connection as an upload
		return Uploads.OnAccept( this );
	}
	else if ( StartsWith( _P("PUSH ") ) )
	{
		// Gnutella2-style push
		return OnAcceptPush();
	}
	else if ( StartsWith( _P("GIV ") ) )
	{
		// Gnutella giv
		return OnAcceptGive();
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
	// The first line should be like "PUSH guid:GUIDinHEXguidINhexGUIDinHEXguidI", which has 10 characters before the 32 for the guid
	CString strLine;
	if ( ! Read( strLine ) )
		return TRUE;

	if ( strLine.GetLength() != 10 + 32 )
	{
		Write( _P("GNUTELLA/0.6 503 Bad push\r\n\r\n") );
		LogOutgoing();
		DelayClose( IDS_DOWNLOAD_BAD_PUSH );
		return TRUE;
	}

	// Read the 16 hexadecimal digits of the GUID, copying it into pGUID
	Hashes::Guid oGUID;
	for ( int nByte = 0 ; nByte < 16 ; nByte++ )
	{
		int nValue;
		if ( _stscanf( strLine.Mid( 10 + nByte * 2, 2 ), _T("%X"), &nValue ) != 1 )
		{
			Write( _P("GNUTELLA/0.6 503 Bad push\r\n\r\n") );
			LogOutgoing();
			DelayClose( IDS_DOWNLOAD_BAD_PUSH );
			return TRUE;
		}
		oGUID[ nByte ] = (BYTE)nValue;
	}
	oGUID.validate();

	// If a child window recognizes the GUID, accept the push
	if ( Network.OnPush( oGUID, this ) )
		return FALSE;

	Write( _P("GNUTELLA/0.6 503 Unknown push\r\n\r\n") );
	LogOutgoing();
	DelayClose( IDS_DOWNLOAD_UNKNOWN_PUSH );
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CHandshake accept Gnutella1-style GIV

// If the first thing the remote computer says to us is a Gnutella "GIV ", this method gets called
// Checks if a child window recognizes the guid
// Returns true or false
BOOL CHandshake::OnAcceptGive()
{
	// The first line should be like "GIV 124:d51dff817f895598ff0065537c09d503/my%20song.mp3"
	CString strLine;
	if ( ! Read( strLine ) ) 
		return TRUE;

	// If there is a slash in the line
	CString strClient, strFile;
	int nPos = strLine.Find( _T('/') );
	if ( nPos > 0 )
	{
		// Clip out the part of the line after the slash, URL decode it to turn %20 into spaces, and save that in strFile
		strFile	= URLDecode( strLine.Mid( nPos + 1 ) );	// Mid takes part after the slash
		strLine	= strLine.Left( nPos );					// Left removes that part and the slash from strLine
	}

	// If there is a colon in the line more than 4 character widths in, like "GIV 123:client32characterslong----------"
	DWORD nFileIndex = 0xFFFFFFFF;
	nPos = strLine.Find( _T(':') );
	if ( nPos > 4 )
	{
		// Read the number before and text after the colon
		strClient	= strLine.Mid( nPos + 1 );			// Clip out just the "client" part of the example shown above
		strLine		= strLine.Mid( 4, nPos - 4 );		// Starting at 4 to get beyond the "GIV ", clip out "123", any text before the colon at nPos
		_stscanf( strLine, _T("%lu"), &nFileIndex );	// Read "123" as a number, this is the file index
	}

	// If there was no colon and the file index was not read, or the client text isn't exactly 32 characters long
	if ( nFileIndex == 0xFFFFFFFF || strClient.GetLength() != 32 )
	{
		Write( _P("GNUTELLA/0.6 503 Bad push\r\n\r\n") );
		LogOutgoing();
		DelayClose( IDS_DOWNLOAD_BAD_PUSH );
		return TRUE;
	}

	// The client id text is a 16-byte guid written in 32 characters with text like "00" through "ff" for each byte, read it into pClientID
	Hashes::Guid oGUID;
	for ( int nByte = 0 ; nByte < 16 ; nByte++ )
	{
		// Convert one set of characters like "00" or "ff" into that byte in pClientID
		if ( _stscanf( strClient.Mid( nByte * 2, 2 ), _T("%X"), &nPos ) != 1 )
		{
			Write( _P("GNUTELLA/0.6 503 Bad push\r\n\r\n") );
			LogOutgoing();
			DelayClose( IDS_DOWNLOAD_BAD_PUSH );
			return TRUE;
		}
		oGUID[ nByte ] = (BYTE)nPos;
	}
	oGUID.validate();

	theApp.Message( MSG_DEBUG, _T( "Got push answer from %s : %s..." ), (LPCTSTR)CString( inet_ntoa( m_pHost.sin_addr ) ), (LPCTSTR)oGUID.toString< Hashes::base16Encoding >().MakeUpper() );

	// If a child window recognizes this guid, return true
	if ( Network.OnPush( oGUID, this ) )
		return FALSE;

	Write( _P("GNUTELLA/0.6 503 Unknown push\r\n\r\n") );
	LogOutgoing();
	DelayClose( IDS_DOWNLOAD_UNKNOWN_PUSH );
	return TRUE;
}
