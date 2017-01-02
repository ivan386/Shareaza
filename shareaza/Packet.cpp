//
// Packet.cpp
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

// CPacket represents a packet on a peer-to-peer network, and CPacketPool keeps lists of them
// http://shareazasecurity.be/wiki/index.php?title=Developers.Code.CPacket

#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "Network.h"
#include "Packet.h"
#include "ZLibWarp.h"
#include "Buffer.h"
#include "WndMain.h"
#include "WndPacket.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CPacket construction

// Takes a protocol id, like PROTOCOL_G1 for Gnutella
// Makes a new CPacket object to represent a packet
CPacket::CPacket(PROTOCOLID nProtocol)
	// Save the given protocol id in the object
	: m_nProtocol  ( nProtocol )
	// This packet isn't in a list yet, and isn't being used at all yet
	, m_pNext      ( NULL ) // No packet next in a list
	, m_nReference ( 0 )    // No one needs this packet yet, the reference count starts at 0
	// Start out memory pointers and lengths at null and 0
	, m_pBuffer    ( NULL ) // This is just a pointer to allocated bytes, not a CBuffer object that would take care of itself
	, m_nBuffer    ( 0 )
	, m_nLength    ( 0 )
	, m_nPosition  ( 0 )
	// Assume the bytes of the packet are in big endian order
	, m_bBigEndian ( TRUE )
	, m_bUDP	   ( FALSE )
	, m_bOutgoing  ( FALSE )
	, m_nNeighbourUnique	( NULL )
{
}

// Delete this CPacket object
CPacket::~CPacket()
{
	// If the packet points to some memory, delete it
	delete [] m_pBuffer;
}

//////////////////////////////////////////////////////////////////////
// CPacket reset

// Clear and reset the values of this packet object to use it again, just like it came from the constructor
void CPacket::Reset()
{
	// Make sure Reset is only called when nothing is referencing this packet object
	ASSERT( m_nReference == 0 );

	// Reset the member variables to null and 0 defaults
	m_pNext      = NULL;
	m_nLength    = 0;
	m_nPosition  = 0;
	m_bBigEndian = TRUE;
	m_bUDP		 = FALSE;
	m_bOutgoing	 = FALSE;
	m_nNeighbourUnique = NULL;
}

//////////////////////////////////////////////////////////////////////
// CPacket position and seeking

// Takes a distance in bytes into the packet, and seekStart if that's forwards from the front, seekEnd if that's backwards from the end
// Moves the position this packet object remembers to that distance from the start or end
void CPacket::Seek(DWORD nPosition, int nRelative)
{
	// Set the position forwards from the start
	if ( nRelative == seekStart )
	{
		// Move the position in the object to the given position, making sure it's in the data
		m_nPosition = min( m_nLength, nPosition );

	} // Set the position backwards from the end
	else if ( nRelative == seekEnd )
	{
		// Move the position in the object to m_nLength - nPosition from the start, which is nPosition from the end
		m_nPosition = min( m_nLength, m_nLength - nPosition );
	}
	else if ( nRelative == seekCurrent )
	{
		m_nPosition = min( m_nLength, m_nPosition + nPosition );
	}
}

// Takes a number of bytes
// Shortens the packet to that length
void CPacket::Shorten(DWORD nLength)
{
	// If the given length is within the data of the packet, shorten the packet and our position in it
	m_nLength	= min( m_nLength, nLength );     // Record that there are only nLength bytes of packet data written in the buffer
	m_nPosition	= min( m_nPosition, m_nLength ); // Make sure this doesn't move our position beyond the bytes of the packet
}

void CPacket::Remove(DWORD nLength)
{
	if ( nLength >= m_nLength )
	{
		m_nPosition = 0;
		m_nLength = 0;
	}
	else if ( nLength )
	{
		if ( m_nPosition > nLength )
			m_nPosition -= nLength;
		else
			m_nPosition = 0;
		m_nLength -= nLength;
		MoveMemory( m_pBuffer, m_pBuffer + nLength, m_nLength );
	}
}

BOOL CPacket::Compare(const void* szString, DWORD nLength, DWORD nOffset) const
{
	if ( nOffset + nLength > m_nLength )
		return FALSE;

	return ( memcmp( m_pBuffer + nOffset, szString, nLength ) == 0 );
}

int CPacket::Find(BYTE c, DWORD nOffset) const
{
	if ( nOffset >= m_nLength )
		return -1;

	BYTE* p = (BYTE*)memchr( m_pBuffer + nOffset, c, m_nLength - nOffset );
	if ( ! p )
		return -1;

	return (int)( p - m_pBuffer );
}

BYTE CPacket::GetAt(DWORD nOffset) const
{
	if ( nOffset >= m_nLength )
		return 0;

	return m_pBuffer[ nOffset ];
}

//////////////////////////////////////////////////////////////////////
// CPacket strings

// Takes the number of bytes to look at from our position in the packet as ASCII text
// Reads those up to the next null terminator as text
// Returns a string
CString CPacket::ReadString(UINT cp, DWORD nMaximum)
{
	// We'll convert the ANSI text in the packet into wide characters, and return them in this string
	CString strString;

	// If maximum would have us read beyond the end of the packet, make it smaller to read to the end of the packet
	nMaximum = min( nMaximum, m_nLength - m_nPosition );
	if ( ! nMaximum ) return strString; // If that would have us read nothing, return the new blank string

	// Setup pointers to look at bytes in the packet
	LPCSTR pszInput	= (LPCSTR)m_pBuffer + m_nPosition; // Point pszInput at our position inside the buffer
	LPCSTR pszScan  = pszInput;                        // Start out pszScan at the same spot, it will find the next null terminator

	// Loop for each byte in the packet at and beyond our position in it, searching for a null terminator
    DWORD nLength = 0; // When this loop is done, nLength will be the number of ANSI bytes we moved over before finding the null terminator
	for ( ; nLength < nMaximum ; nLength++ )
	{
		// Move the position pointer in this CPacket object to the next byte
		m_nPosition++;

		// If pszScan points to a 0 byte, exit the loop, otherwise move the pointer forward and keep going
		if ( ! *pszScan++ ) break;
	}

	// Find out how many wide characters the ANSI bytes will become when converted
	int nWide = MultiByteToWideChar(
		cp,   // Use the code page for ANSI
		0,        // No character type options
		pszInput, // Pointer to ASCII text in the packet
		nLength,  // Number of bytes to read there, the number of bytes before we found the null terminator
		NULL,     // No output buffer, we just want to know how many wide characters one would need to hold
		0 );

	// Convert the ASCII bytes into wide characters
	MultiByteToWideChar(
		cp,                       // Use the UTF8 code page
		0,                            // No character type options
		pszInput,                     // Pointer to ANSI text
		nLength,                      // Number of bytes to read there, the number of bytes before we found the null terminator
		strString.GetBuffer( nWide ), // Get access to the string's buffer, telling it we will write in nWide wide characters
		nWide );                      // Tell MultiByteToWideChar it can write nWide characters in the buffer

	// Close the string and return it
	strString.ReleaseBuffer( nWide );
	return strString;
}

// Takes the number of bytes to look at from our position in the packet as ANSI text
// Reads those up to the next null terminator as text
// Returns a string
CString CPacket::ReadStringASCII(DWORD nMaximum)
{
	return ReadString(CP_ACP, nMaximum);
}

// Takes text, and true to also write a null terminator
// Converts the text into ANSI bytes using the ANSI code page, and writes them into the end of the packet
void CPacket::WriteString(LPCTSTR pszString, BOOL bNull)
{
	// Find out how many ANSI bytes the wide characters will become when converted
	int nByte = WideCharToMultiByte( CP_ACP, 0, pszString, -1, NULL, 0, NULL, NULL );

	auto_array< CHAR > pszByte( new CHAR[ nByte ] );
	if ( pszByte.get() == NULL )
		return;

	// Convert the wide characters into bytes of ANSI text
	WideCharToMultiByte( CP_ACP, 0, pszString, -1, pszByte.get(), nByte, NULL, NULL );

	// Write the ANSI text into the end of the packet
	Write( pszByte.get(), nByte - ( bNull ? 0 : 1 ) ); // If bNull is true, also write the null terminator which got converted
}

// Takes text
// Determines how many bytes of ANSI text it would turn into when converted
// Returns the number, 5 for "hello", that does not include a null terminator
int CPacket::GetStringLen(LPCTSTR pszString) const
{
	// If the text is blank, the length is 0
	if ( *pszString == 0 ) return 0;

	// Find the number of characters in the text
	int nLength = static_cast< int >( _tcslen( pszString ) ); // Same as lstrlen, doesn't include null terminator

	// Find out how many ANSI bytes the text would convert into, and return that number
	nLength = WideCharToMultiByte( CP_ACP, 0, pszString, nLength, NULL, 0, NULL, NULL );
	return nLength;
}

//////////////////////////////////////////////////////////////////////
// CPacket UTF-8 strings

// Takes the number of bytes to look at from our position in the packet as ASCII text
// Converts those up to the next null terminator into wide characters using the UTF8 code page
// Returns the string of converted Unicode wide characters
CString CPacket::ReadStringUTF8(DWORD nMaximum)
{
	return ReadString(CP_UTF8, nMaximum);
}

// Takes Unicode text, and true to also write a null terminator into the packet
// Converts it into ASCII bytes using the UTF8 code page, and writes them into the end of the packet
void CPacket::WriteStringUTF8(LPCTSTR pszString, BOOL bNull)
{
	// Find out how many bytes of ASCII text the wide characters will become when converted with the UTF8 code page
	int nByte = WideCharToMultiByte( CP_UTF8, 0, pszString, -1, NULL, 0, NULL, NULL );

	auto_array< CHAR > pszByte( new CHAR[ nByte ] );
	if ( pszByte.get() == NULL )
		return;

	// Convert the wide characters into bytes of ASCII text
	WideCharToMultiByte( CP_UTF8, 0, pszString, -1, pszByte.get(), nByte, NULL, NULL );

	// Write the ASCII text into the end of the packet
	Write( pszByte.get(), nByte - ( bNull ? 0 : 1 ) ); // If bNull is true, also write the null terminator which got converted
}

// Takes text
// Determines how many bytes of ASCII text it would turn into when converted with the UTF8 code page
// Returns the number, which does not include a null terminator
int CPacket::GetStringLenUTF8(LPCTSTR pszString) const
{
	// If the text is blank, the length is 0
	if ( *pszString == 0 ) return 0;

	// Find the number of characters in the text
	int nLength = static_cast< int >( _tcslen( pszString ) ); // Same as lstrlen, doesn't include null terminator

	// Find out how many ASCII bytes the text would convert into using the UTF8 code page, and return that number
	nLength = WideCharToMultiByte( CP_UTF8, 0, pszString, nLength, NULL, 0, NULL, NULL );
	return nLength;
}

//////////////////////////////////////////////////////////////////////
// CPacket pointer access

// Takes the number of bytes to write, nLength, and where we want to write them, nOffset
// Increases the size of the buffer and makes a gap to hold that many bytes there
// Returns a pointer to the gap where the caller can insert the new data
BYTE* CPacket::WriteGetPointer(DWORD nLength, DWORD nOffset)
{
	// If the caller didn't specify an nOffset, the method's default is -1, make nOffset the end of the packet
	if ( nOffset == 0xFFFFFFFF ) nOffset = m_nLength;

	// If adding nLength would go beyond the buffer, we need to make it bigger
	if ( m_nLength + nLength > m_nBuffer )
	{
		// Increase the size of the buffer by the needed length, or 128 bytes, whichever is bigger
		m_nBuffer += max( nLength, PACKET_GROW );		// Packet grow is 128 bytes
		LPBYTE pNew = new BYTE[ m_nBuffer ];			// Allocate a new buffer of that size
		if ( pNew == NULL )
			return NULL;
		if ( m_pBuffer )
		{
			CopyMemory( pNew, m_pBuffer, m_nLength );	// Copy all the memory of the old buffer into the new bigger one
			delete [] m_pBuffer;						// Free the old buffer
		}
		m_pBuffer = pNew;								// Point this packet object at its new, bigger buffer
	}

	// If the offset isn't at the very end of the buffer
	if ( nOffset != m_nLength )
	{
		// Shift the fragment beyond the offset into the buffer further to make a gap m_nLength big to hold the new data
		MoveMemory(
			m_pBuffer + nOffset + nLength, // Destination is beyond the offset and the length of what we're going to insert
			m_pBuffer + nOffset,           // Source is at the offset
			m_nLength - nOffset );         // Size is the number of bytes written in the buffer beyond the offset
	}

	// Record there are going to be nLength more bytes stored in the buffer
	m_nLength += nLength;

	// Return a pointer to where the caller can write the nLength bytes
	return m_pBuffer + nOffset;
}

//////////////////////////////////////////////////////////////////////
// CPacket string conversion

// Ask this packet what type it is
// Returns text
CString	CPacket::GetType() const // Saying const indicates this method doesn't change the values of any member variables
{
	// This is just a CPacket, not a G1Packet which would have a type
	return CString(); // Return blank
}

// Express all the bytes of the packet as base 16 digits separated by spaces, like "08 C0 12 AF"
// Returns a string
CString CPacket::ToHex() const
{
	// Setup the alphabet to use when endoing each byte in two hexadecimal characters, 0-9 and A-F
	const LPCTSTR pszHex = _T("0123456789ABCDEF");

	const DWORD nLength = min( m_nLength, 84ul );

	// Make a string and open it to write the characters in it directly, for speed
	CString strDump;
	LPTSTR pszDump = strDump.GetBuffer( nLength * 3 + 4 ); // Each byte will become 3 characters

	// Loop i down each byte in the packet
	for ( DWORD i = 0 ; i < nLength ; i++ )
	{
		// Copy the byte at i into an integer called nChar
		int nChar = m_pBuffer[i];

		// If this isn't the very start, write a space into the text
		if ( i ) *pszDump++ = ' '; // Write a space at pszDump, then move the pszDump pointer forward to the next character

		// Express the byte as two characters in the text, "00" through "FF"
		*pszDump++ = pszHex[ nChar >> 4 ];
		*pszDump++ = pszHex[ nChar & 0x0F ];
	}

	if ( nLength < m_nLength )
	{
		*pszDump++ = '.';
		*pszDump++ = '.';
		*pszDump++ = '.';
	}

	// Write a null terminator beyond the characters we wrote, close direct memory access to the string, and return it
	*pszDump = 0;
	strDump.ReleaseBuffer();
	return strDump;
}

// Read the bytes of the packet as though they are ASCII characters, placing periods for those that aren't
// Returns a string like "abc..fgh.i"
CString CPacket::ToASCII() const
{
	const DWORD nLength = min( m_nLength, 252ul );

	// Make a string and get direct access to its memory buffer
	CString strDump;
	LPTSTR pszDump = strDump.GetBuffer( nLength + 4 ); // We'll write a character for each byte, and 1 more for the null terminator

	// Loop i down each byte in the packet
	for ( DWORD i = 0 ; i < nLength ; i++ )
	{
		// Copy the byte at i into an integer called nChar
		int nChar = m_pBuffer[i];

		// If the byte is 32 or greater, read it as an ASCII character and copy that character into the string
		*pszDump++ = TCHAR( nChar >= 32 ? nChar : '.' ); // If it's 0-31, copy in a period instead
	}

	if ( nLength < m_nLength )
	{
		*pszDump++ = '.';
		*pszDump++ = '.';
		*pszDump++ = '.';
	}

	// Write a null terminator beyond the characters we wrote, close direct memory access to the string, and return it
	*pszDump = 0;
	strDump.ReleaseBuffer();
	return strDump;
}

//////////////////////////////////////////////////////////////////////
// CPacket debugging

#ifdef _DEBUG

void CPacket::Debug(LPCTSTR pszReason) const
{
	if ( m_nLength )
		theApp.Message( MSG_DEBUG, _T("%s Size: %u bytes ASCII: %s HEX: %s"), pszReason, m_nLength, (LPCTSTR)ToASCII(), (LPCTSTR)ToHex() );
	else
		theApp.Message( MSG_DEBUG, _T("%s Size: %u bytes"), pszReason, m_nLength );
}

#endif // _DEBUG

//////////////////////////////////////////////////////////////////////
// CPacket smart dumping

// Takes a CNeighbour object, an IP address without a port number, and true if we are sending the packet, false if we received it
// Gives this packet and related objects to each window in the tab bar for them to process it
void CPacket::SmartDump(const SOCKADDR_IN* pAddress, BOOL bUDP, BOOL bOutgoing, DWORD_PTR nNeighbourUnique)
{
	m_bUDP = bUDP;
	m_bOutgoing = bOutgoing;
	m_nNeighbourUnique = nNeighbourUnique;

	if ( theApp.m_pPacketWnd )
	{
		theApp.m_pPacketWnd->SmartDump( this, pAddress, bUDP, bOutgoing, nNeighbourUnique );
	}
}

// Does nothing, and is not overriden by any inheritng class (do)
void CPacket::RazaSign()
{
	// Do nothing (do)
}

// Does nothing, and is not overriden by any inheritng class (do)
BOOL CPacket::RazaVerify() const
{
	// Always return false (do)
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CPacketPool construction

// Make a new packet pool
CPacketPool::CPacketPool()
{
	// Set member variables to null and 0 defaults
	m_pFree = NULL; // No pointer to a CPacket object
	m_nFree = 0;    // Start the count at 0 (do)
}

// Delete this packet pool
CPacketPool::~CPacketPool()
{
	// Free all the packets in this pool before the destructor frees this packet pool object itself
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CPacketPool clear

// Delete all the packet objects that this packet pool points to, and return the member variables to defaults
void CPacketPool::Clear()
{
	// Loop from the end of the pointer array back to the start
	for (
		INT_PTR nIndex = m_pPools.GetSize() - 1; // GetSize returns the number of pointers in the array, start nIndex on the last one
		nIndex >= 0;                         // If nIndex reaches 0, loop one more time, when it's -1, don't do the loop anymore
		nIndex-- )                           // Move back one index in the pointer array
	{
		// Point pPool at the packet pool at that position in the array
		CPacket* pPool = m_pPools.GetAt( nIndex );

		// Delete the packet pool, freeing the memory of the 256 packets in it
		FreePoolImpl( pPool ); // Calls up higher on the inheritance tree
	}

	// Clear all the member variables of this packet pool object
	m_pPools.RemoveAll(); // Remove all the pointers from the MFC CPtrArray structure
	m_pFree = NULL;       // There are no packets to point to anymore
	m_nFree = 0;          // This packet pool has 0 packets now
}

//////////////////////////////////////////////////////////////////////
// CPacketPool new pool setup

// Create a new array of 256 packets, called a packet pool, and add it to this CPacketPool object's list of them
void CPacketPool::NewPool()
{
	// Allocate an array of 256 packets, this is a new packet pool
	CPacket* pPool = NULL;
	int nPitch = 0, nSize = 256;
	NewPoolImpl(  // NewPoolImpl allocates an array of packets for this packet pool object
		nSize,    // The number of packets the array will hold, we want 256
		pPool,    // NewPoolImpl will save a pointer to the allocated array here
		nPitch ); // NewPoolImpl will save the size in bytes of each packet in the array here

	// Add the new packet pool to m_pPools, this CPacketPool object's list of them
	m_pPools.Add( pPool );

	// Link the packets in the new pool so m_pFree points at the last one, they each point to the one before, and the first points to what m_pFree used to
	BYTE* pBytes = (BYTE*)pPool; // Start the pBytes pointer at the start of our new packet pool
	while ( nSize-- > 0 )        // Loop 256 times, once for each packet in the new pool
	{
		// Link this packet to the one before it
		pPool = (CPacket*)pBytes; // Point pPool at the new packet pool we just created and added to the list
		pBytes += nPitch;         // Move pBytes to the next packet in the pool
		pPool->m_pNext = m_pFree; // Set the next pointer in the first packet in the pool
		m_pFree = pPool;          // Point m_pFree at the next packet
		m_nFree++;                // Record one more packet is linked into the list
	}
}
