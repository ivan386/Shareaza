//
// Connection.cpp
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

// CConnection holds a socket used to communicate with a remote computer, and is the root of a big inheritance tree
// http://shareazasecurity.be/wiki/index.php?title=Developers.Code.CConnection

#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "Connection.h"
#include "Network.h"
#include "Security.h"
#include "Buffer.h"
#include "Statistics.h"
#include "VendorCache.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CConnection construction

// Make a new CConnection object
CConnection::CConnection(PROTOCOLID nProtocol)
	: m_bInitiated			( FALSE )
	, m_bConnected			( FALSE )
	, m_tConnected			( 0 )
	, m_hSocket				( INVALID_SOCKET )
	, m_pInput				( NULL )
	, m_pOutput				( NULL )
	, m_bClientExtended		( FALSE )
	, m_nQueuedRun			( 0 )
	, m_nProtocol			( nProtocol )
	, m_nDelayCloseReason	( 0 )
	, m_bAutoDelete			( FALSE )
{
	ZeroMemory( &m_pHost, sizeof( m_pHost ) );
	m_pHost.sin_family = AF_INET;
	ZeroMemory( &m_mInput, sizeof( m_mInput ) );
	ZeroMemory( &m_mOutput, sizeof( m_mOutput ) );
	m_pInputSection.reset( new CCriticalSection() );
	m_pOutputSection.reset( new CCriticalSection() );
}

//////////////////////////////////////////////////////////////////////
// CConnection attach to an existing connection

// Call to copy a connection object to this one (do)
// Takes another connection object
void CConnection::AttachTo(CConnection* pConnection)
{
	// Make sure the socket isn't valid yet
	ASSERT( ! IsValid() );								// Make sure the socket here isn't valid yet
	ASSERT( AfxIsValidAddress( pConnection, sizeof( *pConnection ) ) );
	ASSERT( pConnection->IsValid() );					// And make sure its socket exists

	DestroyBuffers();

	CQuickLock oOutputLock( *pConnection->m_pOutputSection );
	CQuickLock oInputLock( *pConnection->m_pInputSection );

	// Copy values from the given CConnection object to this one
	m_pHost				= pConnection->m_pHost;
	m_sAddress			= pConnection->m_sAddress;
	m_sCountry			= pConnection->m_sCountry;
	m_sCountryName		= pConnection->m_sCountryName;
	m_hSocket			= pConnection->m_hSocket;
	m_bInitiated		= pConnection->m_bInitiated;
	m_bConnected		= pConnection->m_bConnected;
	m_tConnected		= pConnection->m_tConnected;
	m_pInputSection		= pConnection->m_pInputSection;
	m_pInput			= pConnection->m_pInput;
	m_pOutputSection	= pConnection->m_pOutputSection;
	m_pOutput			= pConnection->m_pOutput;
	m_sUserAgent		= pConnection->m_sUserAgent;
	m_bClientExtended	= pConnection->m_bClientExtended;
	m_nQueuedRun		= pConnection->m_nQueuedRun;
	if ( m_nProtocol <= PROTOCOL_NULL && pConnection->m_nProtocol > PROTOCOL_NULL )
		m_nProtocol		= pConnection->m_nProtocol;
	m_nDelayCloseReason	= pConnection->m_nDelayCloseReason;

	// Record the current time in the input and output TCP bandwidth meters
	m_mInput.tLast = m_mOutput.tLast = GetTickCount();

	// Invalidate the socket in the given connection object so it's just here now
	pConnection->m_hSocket	= INVALID_SOCKET;

	// Null the input and output pointers
	pConnection->m_pInput	= NULL;
	pConnection->m_pOutput	= NULL;

	// Zero the memory of the input and output TCPBandwidthMeter objects
	ZeroMemory( &pConnection->m_mInput, sizeof( m_mInput ) );
	ZeroMemory( &pConnection->m_mOutput, sizeof( m_mOutput ) );
}

// Delete this CConnection object
CConnection::~CConnection()
{
	m_bAutoDelete = FALSE;

	CConnection::Close();

	// Delete and mark null the input and output buffers
	DestroyBuffers();
}

void CConnection::LogOutgoing()
{
	if ( ! theApp.IsLogDisabled( MSG_DEBUG | MSG_FACILITY_OUTGOING ) )
	{
		CLockedBuffer pOutput( GetOutput() );
		if ( pOutput->m_nLength )
		{
			CStringA msg( (const char*)pOutput->m_pBuffer, pOutput->m_nLength );
			theApp.Message( MSG_DEBUG | MSG_FACILITY_OUTGOING, _T("%s << %s"), (LPCTSTR)m_sAddress, (LPCTSTR)CString( msg ) );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CConnection connect to

// Connect this CConnection object to a remote computer on the Internet
// Takes pHost, a pointer to a SOCKADDR_IN structure, which is MFC's way of holding an IP address and port number
// Returns true if connected
BOOL CConnection::ConnectTo(const SOCKADDR_IN* pHost)
{
	// Call the next ConnectTo method, and return the result
	return ConnectTo( &pHost->sin_addr, htons( pHost->sin_port ) );
}

// Connect this CConnection object to a remote computer on the Internet
// Takes pAddress, a Windows Sockets structure that holds an IP address, and takes the port number seprately
// Returns true if connected
BOOL CConnection::ConnectTo(const IN_ADDR* pAddress, WORD nPort)
{
	// Make sure the socket isn't already connected somehow
	if ( IsValid() )
		return FALSE;

	// Make sure we have an address and a nonzero port number
	if ( pAddress == NULL || nPort == 0 )
		return FALSE;

	// S_un.S_addr is the IP as a single unsigned 4-byte long
	if ( pAddress->S_un.S_addr == 0 )
		return FALSE;

	// The IP address is in the security list of government and corporate addresses we want to avoid
	if ( Security.IsDenied( pAddress ) )
	{
		// Report that we aren't connecting to this IP address and return false
		theApp.Message( MSG_ERROR, IDS_SECURITY_OUTGOING, (LPCTSTR)CString( inet_ntoa( *pAddress ) ) );
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
	m_pHost.sin_family	= AF_INET;							// PF_INET means just normal IPv4, not IPv6 yet
	m_pHost.sin_port	= htons( nPort );					// Copy the port number into the m_pHost structure
	m_sAddress			= inet_ntoa( m_pHost.sin_addr );	// Save the IP address as a string of text
	UpdateCountry();

	// Create a socket and store it in m_hSocket
	m_hSocket = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( ! IsValid() )	// Now, make sure it has been created
	{
		theApp.Message( MSG_ERROR, _T("Failed to create socket. (1st Try)") );
		// Second attempt
		m_hSocket = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
		if ( ! IsValid() )
		{
			theApp.Message( MSG_ERROR, _T("Failed to create socket. (2nd Try)") );
			return FALSE;
		}
	}

	// Disables the Nagle algorithm for send coalescing
	VERIFY( setsockopt( m_hSocket, IPPROTO_TCP, TCP_NODELAY, "\x01", 1 ) == 0 );

	// Allows the socket to be bound to an address that is already in use
	VERIFY( setsockopt( m_hSocket, SOL_SOCKET, SO_REUSEADDR, "\x01", 1 ) == 0 );

	// Choose asynchronous, non-blocking reading and writing on our new socket
	DWORD dwValue = 1;
	ioctlsocket( m_hSocket, FIONBIO, &dwValue );

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

	DestroyBuffers();

	// Try to connect to the remote computer
	if ( WSAConnect(
		m_hSocket,					// Our socket
		(SOCKADDR*)&m_pHost,		// The remote IP address and port number
		sizeof(SOCKADDR_IN),		// How many bytes the function can read
		NULL, NULL, NULL, NULL ) )	// No advanced features
	{
		// If no error occurs, WSAConnect returns 0, so if we're here an error happened
		int nError = WSAGetLastError(); // Get the last Windows Sockets error number

		// An error of "would block" is normal because connections can't be made instantly and this is a non-blocking socket
		if ( nError != WSAEWOULDBLOCK )
		{
			CNetwork::CloseSocket( m_hSocket, true );

			if ( nError != 0 ) 
				Statistics.Current.Connections.Errors++;

			return FALSE;
		}
	}

	CreateBuffers();

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
	ASSERT( ! IsValid() );

	// Record the connection information here
	m_hSocket		= hSocket;							// Keep the socket here
	m_pHost			= *pHost;							// Copy the remote IP address into this object
	m_sAddress		= inet_ntoa( m_pHost.sin_addr );	// Store it as a string also
	UpdateCountry();

	// Make new input and output buffer objects
	ASSERT( m_pInput == NULL );
	ASSERT( m_pOutput == NULL );
	CreateBuffers();

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
// CConnection close

// Call to close the connection represented by this object
void CConnection::Close(UINT nError)
{
	// Make sure this object exists, and is located entirely within the program's memory space
	ASSERT( this != NULL );
	ASSERT( AfxIsValidAddress( this, sizeof(*this) ) );

	if ( nError )
	{
		if ( nError == IDS_SECURITY_BANNED_USERAGENT )
		{
			CString sComment;
			sComment.Format( IDS_SECURITY_BANNED_USERAGENT, (LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );
			Security.Ban( &m_pHost.sin_addr, ban2Hours, FALSE, sComment );

			theApp.Message( MSG_ERROR, IDS_SECURITY_BANNED_USERAGENT, (LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );
		}
		else if ( nError == IDS_HANDSHAKE_REJECTED )
			theApp.Message( MSG_ERROR, IDS_HANDSHAKE_REJECTED, (LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent );
		else
		{
			BOOL bInfo = ( nError == IDS_CONNECTION_CLOSED || nError == IDS_CONNECTION_PEERPRUNE );
			theApp.Message( bInfo ? MSG_INFO : MSG_ERROR, nError, (LPCTSTR)m_sAddress );
		}
	}

	CNetwork::CloseSocket( m_hSocket, false );

	// This connection object isn't connected any longer
	m_bConnected = FALSE;

	if ( m_bAutoDelete )
		delete this;
}

// Close the connection, but not until we've written the buffered outgoing data first
// Takes the reason we're closing the connection, or 0 by default
void CConnection::DelayClose(UINT nError)
{
	ASSERT( nError );

	// Clear input buffer
	{
		CQuickLock oInputLock( *m_pInputSection );
		m_pInput->Clear();
	}

	// Disable incoming data
	shutdown( m_hSocket, SD_RECEIVE );

	if ( ! m_bInitiated )
		// Prolong ban (if any) for income connection
		Security.Complain( &m_pHost.sin_addr );

	m_nDelayCloseReason = nError;

	// Have the connection object write all the outgoing data soon
	QueueRun();
}

//////////////////////////////////////////////////////////////////////
// CConnection run function

// Talk to the other computer, write the output buffer to the socket and read from the socket to the input buffer
// Return true if this worked, false if we've lost the connection
BOOL CConnection::DoRun()
{
	// If this socket is invalid, call OnRun and return the result (do)
	if ( ! IsValid() )
		return OnRun();

	// Setup pEvents to store the socket's internal information about network events
	WSANETWORKEVENTS pEvents = {};
	if ( WSAEnumNetworkEvents( m_hSocket, NULL, &pEvents ) != 0 )
		return FALSE;

	// If the FD_CONNECT network event has occurred
	if ( pEvents.lNetworkEvents & FD_CONNECT )
	{
		// If there is a nonzero error code for the connect operation
		if ( pEvents.iErrorCode[ FD_CONNECT_BIT ] != 0 )
		{
			Statistics.Current.Connections.Errors++;

			// This connection was dropped
			OnDropped();
			return FALSE;
		}

		// The socket is now connected
		m_bConnected = TRUE;
		m_tConnected = m_mInput.tLast = m_mOutput.tLast = GetTickCount(); // Store the time 3 places

		// Call CShakeNeighbour::OnConnected to start reading the handshake
		if ( !OnConnected() )
			return FALSE;

		Network.AcquireLocalAddress( m_hSocket );
	}

	// If the FD_CLOSE network event has occurred, set bClosed to true, otherwise set it to false
	BOOL bClosed = ( pEvents.lNetworkEvents & FD_CLOSE ) ? TRUE : FALSE;

	// If the close event happened, null a pointer within the TCP bandwidth meter for input (do)
	if ( bClosed )
		m_mInput.pLimit = NULL;

	// Change the queued run state to 1 (do)
	m_nQueuedRun = 1;

	// Write the contents of the output buffer to the remote computer, and read in data it sent us
	if ( !OnWrite() )
		return FALSE;
	if ( !OnRead() )
		return FALSE;

	// If the close event happened
	if ( bClosed )
	{
		// theApp.Message( MSG_DEBUG, _T("socket close() error %i"), pEvents.iErrorCode[ FD_CLOSE_BIT ] );
		// Call OnDropped, telling it true if there is a close error
		OnDropped(); // True if there is an nonzero error code for the close bit
		return FALSE;
	}

	// Make sure the handshake doesn't take too long
	if ( !OnRun() )
		return FALSE;

	// If the queued run state is 2 and OnWrite returns false, leave here with false also
	if ( m_nQueuedRun == 2 && !OnWrite() )
		return FALSE;

	// Change the queued run state back to 0 and report success (do)
	m_nQueuedRun = 0;
	return TRUE;
}

// Call OnWrite if DoRun has just succeeded (do)
void CConnection::QueueRun()
{
	// If the queued run state is 1 or 2, make it 2, if it's 0, call OnWrite now (do)
	if ( m_nQueuedRun )
		m_nQueuedRun = 2;	// The queued run state is not 0, make it 2 and do nothing else here
	else
		OnWrite();			// The queued run state is 0, do a write (do)
}

//////////////////////////////////////////////////////////////////////
// CConnection socket event handlers

// Objects that inherit from CConnection have OnConnected methods that do things, unlike this one
BOOL CConnection::OnConnected()
{
	// Just return true
	return TRUE;
}

// Objects that inherit from CConnection have OnDropped methods that do things, unlike this one
void CConnection::OnDropped()
{
	// Do nothing
}

// Objects that inherit from CConnection have OnRun methods that do things, unlike this one
BOOL CConnection::OnRun()
{
	if ( m_nDelayCloseReason )
	{
		CLockedBuffer pOutputLocked( GetOutput() );

		// If there is nothing to send
		if ( pOutputLocked->m_nLength == 0 )
		{
			Close( m_nDelayCloseReason );
			return FALSE;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CConnection read event handler

// Read data waiting in the socket into the input buffer
BOOL CConnection::OnRead()
{
	CQuickLock oInputLock( *m_pInputSection );

	// Make sure the socket is valid
	if ( ! IsValid() )
		return FALSE;

	if ( m_nDelayCloseReason )
		return TRUE;

	DWORD tNow		= GetTickCount();	// The time right now
	DWORD nLimit	= ~0ul;				// Make the limit huge

	// If we need to worry about throttling bandwidth, calculate nLimit, the number of bytes we are allowed to read now
	if ( m_mInput.pLimit							// If there is a limit
		&& *m_mInput.pLimit							// And that limit isn't 0
		&& Settings.Live.BandwidthScale <= 100 )	// And the bandwidth scale isn't at MAX
	{
		// Work out what the bandwidth limit is
		nLimit = m_mInput.CalculateLimit( tNow );
	}

	nLimit = min( nLimit, (DWORD)INT_MAX );

	// Start the total at 0
	DWORD nTotal = 0ul;

	// Read bytes from the socket until the limit has run out
	while ( nLimit )
	{
		// Limit nLength to the free buffer space or the maximum size of an int
		DWORD nLength = m_pInput->GetBufferFree();

		if ( nLength )
			// Limit nLength to the speed limit
			nLength = min( nLimit, nLength );
		else
			// Limit nLength to the maximum receive size
			nLength = min( nLimit, 16384ul );	// Receive up to 16KB blocks from the socket

		// Exit loop if the buffer isn't big enough to hold the data
		if ( ! m_pInput->EnsureBuffer( nLength ) )
			break;

		// Read the bytes from the socket and record how many are actually read
		int nRead = CNetwork::Recv( m_hSocket, (char*)m_pInput->GetDataEnd(), (int)nLength );

		// Exit loop if nothing is left or an error occurs
		if ( nRead <= 0 )
			break;

		m_pInput->m_nLength	+= nRead;	// Add to the buffer size
		nTotal				+= nRead;	// Add to the total
		nLimit				-= nRead;	// Adjust the limit
	}

	// Add the total to the statistics
	Statistics.Current.Bandwidth.Incoming += nTotal;

	// If some bytes were read, add # bytes to bandwidth meter
	if ( nTotal ) m_mInput.Add( nTotal, tNow );

	// Report success
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CConnection write event handler

// Call to send the contents of the output buffer to the remote computer
BOOL CConnection::OnWrite()
{
	CQuickLock oOutputLock( *m_pOutputSection );

	// Make sure the socket is valid
	if ( ! IsValid() )
		return FALSE;

	// If there is nothing to send, we succeed without doing anything
	if ( m_pOutput->m_nLength == 0 )
		return TRUE;

	DWORD tNow		= GetTickCount();	// The time right now
	DWORD nLimit	= ~0ul;				// Make the limit huge

	// If we need to worry about throttling bandwidth, calculate nLimit, the number of bytes we are allowed to write now
	if ( m_mOutput.pLimit							// If there is a limit
		&& *m_mOutput.pLimit						// And that limit isn't 0
		&& Settings.Live.BandwidthScale <= 100 )	// And the bandwidth scale isn't at MAX
	{
		// Work out what the bandwidth limit is
		nLimit = m_mOutput.CalculateLimit( tNow, Settings.Uploads.ThrottleMode == 0 );
	}

	nLimit = min( min( nLimit, m_pOutput->GetCount() ), (DWORD)INT_MAX );

	// Point to the data to write
	const BYTE* pData = m_pOutput->GetData();

	// Start the total at 0
	DWORD nTotal = 0ul;

	// Write bytes to the socket until our limit has run out
	while ( nLimit )
	{
		// Send the bytes to the socket and record how many are actually sent
		int nSend = CNetwork::Send( m_hSocket, (const char*)pData, (int)nLimit );

		// Exit loop if nothing is left or an error occurs
		if ( nSend <= 0 )
			break;

		pData	+= nSend;	// Move forward past the sent data
		nTotal	+= nSend;	// Add to the total
		nLimit	-= nSend;	// Adjust the limit
	}

	// Remove sent bytes from the buffer
	m_pOutput->Remove( nTotal );

	// Add the total to statistics
	Statistics.Current.Bandwidth.Outgoing += nTotal;

	// If some bytes were sent, add # bytes to bandwidth meter
	if ( nTotal ) m_mOutput.Add( nTotal, tNow );

	// Report success
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CConnection measure

// Calculate the input and output speeds for this connection
void CConnection::Measure()
{
	// Time period for bytes
	DWORD tCutoff = GetTickCount() - METER_PERIOD;
	
	// Calculate Input and Output seperately
	m_mInput.nMeasure  = m_mInput.CalculateUsage( tCutoff )  / ( METER_PERIOD / METER_SECOND );
	m_mOutput.nMeasure = m_mOutput.CalculateUsage( tCutoff ) / ( METER_PERIOD / METER_SECOND );
}

// Calculate the input speed for this connection
void CConnection::MeasureIn()
{
	// Time period for bytes
	DWORD tCutoff = GetTickCount() - METER_PERIOD;
	
	// Calculate input speed
	m_mInput.nMeasure  = m_mInput.CalculateUsage( tCutoff )  / ( METER_PERIOD / METER_SECOND );
}

// Calculate the output speed for this connection
void CConnection::MeasureOut()
{
	// Time period for bytes
	DWORD tCutoff = GetTickCount() - METER_PERIOD;
	
	// Calculate output speed
	m_mOutput.nMeasure = m_mOutput.CalculateUsage( tCutoff ) / ( METER_PERIOD / METER_SECOND );
}

//////////////////////////////////////////////////////////////////////
// CConnection HTML header reading

// Remove the headers from the input buffer, handing each to OnHeaderLine
BOOL CConnection::ReadHeaders()
{
	CString strLine;
	while ( Read( strLine ) ) // ReadLine will return false when there are no more lines
	{
		// If the line is more than 256 KB, change it to the line too long error code 
		if ( strLine.GetLength() > 256 * 1024 ) strLine = _T("#LINE_TOO_LONG#");

		// Find the first colon in the line
		int nPos = strLine.Find( _T(":") );

		// The line is empty, it's just a \n character
		if ( strLine.IsEmpty() )
		{
			// Empty the last header member variable (do)
			m_sLastHeader.Empty();

			// Call the OnHeadersComplete method for the most advanced class that inherits from CConnection
			return OnHeadersComplete();
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
					if ( !OnHeaderLine( m_sLastHeader, strLine ) )
						return FALSE;
				}
			}

		} // The colon is at a distance greater than 1 and less than 64
		else if ( nPos > 1 && nPos < 64 ) // ":a" is 0 and "a:a" is 1, but "aa:a" is greater than 1
		{
			// The line is like "header:value", copy out both parts
			CString strHeader	= strLine.Left( nPos );
			CString strValue	= strLine.Mid( nPos + 1 );
			m_sLastHeader = strHeader;

			// Trim spaces from both ends of the value, and see if it still has length
			strValue.TrimLeft();
			strValue.TrimRight();

			// Give OnHeaderLine this last header, and its value
			if ( !OnHeaderLine( strHeader, strValue ) )
				return FALSE;
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
	theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING, _T("%s >> %s: %s"), (LPCTSTR)m_sAddress, (LPCTSTR)strHeader, (LPCTSTR)strValue );

	// It's the user agent header
	if ( strHeader.CompareNoCase( _T("User-Agent") ) == 0 )
	{
		// Copy the value into the user agent member string
		m_sUserAgent = strValue; // This tells what software the remote computer is running
		m_bClientExtended = VendorCache.IsExtended( m_sUserAgent );

	} // It's the remote IP header
	else if ( strHeader.CompareNoCase( _T("Remote-IP") ) == 0 )
	{
		// Add this address to our record of them
		Network.AcquireLocalAddress( strValue );
	
	} // It's the x my address, listen IP, or node header, like "X-My-Address: 10.254.0.16:6349"
	else if (  strHeader.CompareNoCase( _T("X-My-Address") ) == 0
			|| strHeader.CompareNoCase( _T("Listen-IP") ) == 0
			|| strHeader.CompareNoCase( _T("X-Node") ) == 0
			|| strHeader.CompareNoCase( _T("Node") ) == 0 )
	{
		// Find another colon in the value
		int nColon = strValue.Find( ':' );

		// If the remote computer first contacted us and the colon is there but not first
		if ( ! m_bInitiated && nColon > 0 )
		{
			// Read the number after the colon into nPort
			int nPort = protocolPorts[ PROTOCOL_G2 ];
			if ( _stscanf( strValue.Mid( nColon + 1 ), _T("%d"), &nPort ) == 1 // Make sure 1 number was found
				&& nPort != 0 ) // Make sure the found number isn't 0
			{
				// Save the found port number in m_pHost
				m_pHost.sin_port = htons( u_short( nPort ) ); // Convert Windows little endian to big for the Internet with htons
			}
		}
	}
	else if ( strHeader.CompareNoCase( _T("Accept") ) == 0 )
	{
		if ( _tcsistr( strValue, _T("application/x-gnutella-packets") ) &&
			m_nProtocol != PROTOCOL_G2 )
			m_nProtocol = PROTOCOL_G1;
		if ( _tcsistr( strValue, _T("application/x-gnutella2") ) ||
			 _tcsistr( strValue, _T("application/x-shareaza") ) )
			m_nProtocol = PROTOCOL_G2;
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

		strHeader.Format(
			_T("Listen-IP: %s:%hu\r\n"),								// Make it like "Listen-IP: 67.176.34.172:6346\r\n"
			(LPCTSTR)CString( inet_ntoa( Network.m_pHost.sin_addr ) ),	// Insert the IP address like "67.176.34.172"
			htons( Network.m_pHost.sin_port ) );						// Our port number in big endian

		// Print the line into the bottom of the output buffer
		Write( strHeader ); // It will be sent to the remote computer on the next write

		// Report that we are listening on a port, and the header is sent
		return TRUE;
	}

	// We're not even listening on a port
	return FALSE;
}

void CConnection::UpdateCountry()
{
	m_sCountry     = theApp.GetCountryCode( m_pHost.sin_addr );
	m_sCountryName = theApp.GetCountryName( m_pHost.sin_addr );
}

void CConnection::SendHTML(UINT nResourceID)
{
	CString strResponse;
	CString strBody = LoadRichHTML( nResourceID, strResponse );

	if ( strResponse.IsEmpty() )
		Write( _P("HTTP/1.1 200 OK\r\n") );
	else
		Write( _T("HTTP/1.1 ") + strResponse );

	if ( nResourceID == IDR_HTML_BUSY )
		Write( _P("Retry-After: 30\r\n") );

	Write( _P("Content-Type: text/html\r\n") );

	CStringA strBodyUTF8 = UTF8Encode( strBody );

	CString strLength;
	strLength.Format( _T("Content-Length: %i\r\n\r\n"), strBodyUTF8.GetLength() );
	Write( strLength );

	LogOutgoing();

	Write( (LPCSTR)strBodyUTF8, strBodyUTF8.GetLength() );
}

//////////////////////////////////////////////////////////////////////
// TCPBandwidthMeter Utility routines

// Calculate the number of bytes available for use
DWORD CConnection::TCPBandwidthMeter::CalculateLimit( DWORD tNow, bool bMaxMode ) const
{
	DWORD tCutoff = tNow - METER_SECOND;			// Time period for bytes
	if ( bMaxMode ) tCutoff += METER_MINIMUM;		// Adjust time period for Max mode
	DWORD nData = CalculateUsage( tCutoff, true );	// #bytes in the time period

	// nLimit is the speed limit (bytes/second)
	DWORD nLimit = *pLimit;						// Get the speed limit
	if ( Settings.Live.BandwidthScale < 100 )	// The scale is turned down and we should use it
	{
		// Adjust limit based on the scale percentage
		nLimit = nLimit * Settings.Live.BandwidthScale / 100;
	}

	// nLimit - nData is the number of bytes still available for this time period
	// Set nData to this, or 0 if we're over the limit
	nData >= nLimit ? nData = 0 : nData = nLimit - nData;

	// Is this running in max mode
	if ( bMaxMode )
	{
		// Adjust limit for the time elapsed since last time
		nLimit = nLimit * ( tNow - tLastLimit ) / 1000;

		// nData = speed limit in bytes per second - bytes we read in the last second
		// nLimit = speed limit in bytes per second * elapsed time
		// return the smaller of the two
		nLimit = min( nLimit, nData );
	}
	else
	{
		// Set limit to the number of bytes still available for this time period
		nLimit = nData;
	}
	tLastLimit = tNow;	// The time of this limit calculation
	return nLimit;		// Return the new limit
}

// Count the #bytes used for a given time period ( optimal for time periods more than METER_LENGTH / 2 )
DWORD CConnection::TCPBandwidthMeter::CalculateUsage( DWORD tTime ) const
{
	// Exit early if the last slot used is older than the time limit
	if ( tLastSlot <= tTime ) return 0;

	// Loop across the times and histories stored
	DWORD nData = 0;	// #bytes in the time period
	DWORD slot = 0;		// Start at the first slot
	
	// Find the first reading in the time limit
	while ( slot <= nPosition && pTimes[ slot ] <= tTime ) slot++;

	// Add history up to the latest reading
	while ( slot <= nPosition ) nData += pHistory[ slot++ ];

	// Did we start with a reading inside the time limit
	if ( pTimes[ 0 ] > tTime )
	{
		// Find the next reading in the time limit
		while ( slot < METER_LENGTH && pTimes[ slot ] <= tTime ) slot++;

		// Add history up to the end of the meter
		while ( slot < METER_LENGTH ) nData += pHistory[ slot++ ];
	}
	
	// return the #bytes in time period
	return nData;
}

// Count the #bytes used for a given time period ( optimal for time periods less than METER_LENGTH / 2 )
DWORD CConnection::TCPBandwidthMeter::CalculateUsage( DWORD tTime, bool /*bShortPeriod*/ ) const
{
	// Exit early if the last slot used is older than the time limit
	if ( tLastSlot <= tTime ) return 0;

	// Loop across the times and histories stored
	// Granularity is 1/10th ( METER_MINIMUM ) of a second, so at the most we only
	// need to read 12 ( METER_MINIMUM / METER_SECOND + 2 )records instead of all of them
	DWORD nData = 0;			// #bytes in the time period
	DWORD slot = METER_LENGTH;	// Start at the last slot
	while ( slot-- )
	{
		if ( pTimes[ slot ] > tTime )	// Is this within the time period
			nData += pHistory[ slot ];	//   It was, add it to #bytes
		else if ( slot > nPosition )	// It wasn't, did we start with the latest reading
			slot = nPosition + 1;		//   We didn't, jump to our latest reading and continue
		else
			break;						//   We did, no need to check the rest
	}

	// return the #bytes in time period
	return nData;
}

// Add #bytes to history
void CConnection::TCPBandwidthMeter::Add( const DWORD nBytes, const DWORD tNow )
{
	if ( tNow - tLastSlot < METER_MINIMUM )
	{
		// Less than the minimum time interval
		// Use the same place in the array as before
		pHistory[ nPosition ] += nBytes;
	}
	else
	{
		// More than the minimum time interval
		// Store the time and total in a new array slot
		nPosition = ( nPosition + 1 ) % METER_LENGTH;	// Move to the next position in the array
		pTimes	[ nPosition ]	= tNow;					// Record the new time
		pHistory[ nPosition ]	= nBytes;				// Store the #bytes next to it
		tLastSlot = tNow;								// We just wrote some history information
	}
	nTotal += nBytes;	// Add the #bytes to the total
	tLast = tNow;		// The time of this add
}
