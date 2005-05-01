//
// G1Packet.cpp
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

// CG1Packet represents a Gnutella packet, and CG1PacketPool keeps lists of them
// http://wiki.shareaza.com/static/Developers.Code.CG1Packet

// Copy in the contents of these files here before compiling
#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "G1Packet.h"
#include "Network.h"
#include "Buffer.h"
#include "SHA.h"

// If we are compiling in debug mode, replace the text "THIS_FILE" in the code with the name of this file
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// When the program runs, create a single, global CG1PacketPool called POOL to hold a bunch of Gnutella packets
CG1Packet::CG1PacketPool CG1Packet::POOL;

//////////////////////////////////////////////////////////////////////
// CG1Packet construction

// Make a new CG1Packet object
CG1Packet::CG1Packet() : CPacket( PROTOCOL_G1 ) // Before running the code here, call the CPacket constructor, giving it Gnutella as the protocol
{
	// Start out with the type and type index 0, later they will be set to ping or pong
	m_nType      = 0;
	m_nTypeIndex = 0;

	// Set the time to live and hops counts to 0
	m_nTTL = m_nHops = 0;

	// No hash yet
	m_nHash = 0;
}

// Delete this CG1Packet object
CG1Packet::~CG1Packet()
{
	// The CPacket destructor will take care of freeing memory, so there is nothing to do here (do)
}

//////////////////////////////////////////////////////////////////////
// CG1Packet new

// Takes a packet type like ping, a TTL number, and a GUID for the packet
// Makes a new packet with these values
// Returns a pointer to it
CG1Packet* CG1Packet::New(int nType, DWORD nTTL, GGUID* pGUID)
{
	// Get a pointer to a packet in the single global Gnutella packet pool
	CG1Packet* pPacket = (CG1Packet*)POOL.New(); // Calls CPacketPool::New, defined in Packet.h

	// Copy the given type and corresponding type index into it
	pPacket->m_nType      = (BYTE)nType;
	pPacket->m_nTypeIndex = GnutellaTypeToIndex( pPacket->m_nType );

	// Set the TTL and hops counts
	pPacket->m_nTTL  = (BYTE)( nTTL > 0 ? nTTL : Settings.Gnutella1.DefaultTTL ); // If the given TTL is 0, use the default instead
	pPacket->m_nHops = 0; // This packet hasn't travelled across the Internet at all yet

	// No hash yet
	pPacket->m_nHash = 0;

	// If the caller has given us a GUID to use in the packet
	if ( pGUID )
	{
		// Copy the GUID into the packet
		pPacket->m_pGUID = *pGUID;

	} // If the caller didn't give us a GUID to use
	else
	{
		// Create a GUID for this packet
		Network.CreateID( pPacket->m_pGUID );
	}

	// Return a pointer to the packet
	return pPacket;
}

//////////////////////////////////////////////////////////////////////
// CG1Packet type conversion

// Takes the byte in a Gnutella packet that describes what type it is, like ping or pong
// Returns the corresponding enumerantion index number, like 0, 1, 2, the program uses to mean the same thing
int CG1Packet::GnutellaTypeToIndex(BYTE nType)
{
	// Sort by the type byte, and return the corresponding index number which means the same thing
	switch ( nType )
	{
	case G1_PACKET_PING:        return G1_PACKTYPE_PING;        // Byte 0x00 is index 1, ping
	case G1_PACKET_PONG:        return G1_PACKTYPE_PONG;        // Byte 0x01 is index 2, pong
	case G1_PACKET_BYE:         return G1_PACKTYPE_BYE;         // Byte 0x02 is index 3, bye
	case G1_PACKET_QUERY_ROUTE: return G1_PACKTYPE_QUERY_ROUTE; // Byte 0x30 is index 4, query route
	case G1_PACKET_VENDOR:                                      // Bytes 0x31 and 0x32 are index 5, vendor
	case G1_PACKET_VENDOR_APP:  return G1_PACKTYPE_VENDOR;
	case G1_PACKET_PUSH:        return G1_PACKTYPE_PUSH;        // Byte 0x40 is index 6, push
	case G1_PACKET_QUERY:       return G1_PACKTYPE_QUERY;       // Byte 0x80 is index 7, query
	case G1_PACKET_HIT:         return G1_PACKTYPE_HIT;         // Byte 0x81 is index 8, hit
	default:                    return G1_PACKTYPE_UNKNOWN;     // All other bytes are index 0, unknown
	}
}

//////////////////////////////////////////////////////////////////////
// CG1Packet hopping

// Adjusts the TTL and hops counts of this packet to record a trip across the Internet
// If the packet's TTL starts out less than 2, returns false without changing the numbers (do)
BOOL CG1Packet::Hop()
{
	// Make sure the packet's TTL is 2 or more
	if ( m_nTTL < 2 ) return FALSE;

	// Record a trip across the Internet
	m_nTTL--;  // One less trip to live
	m_nHops++; // One more hop made

	// Report that we checked and changed the numbers
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CG1Packet hashing

// Calculate a simple hash of the bytes of the packet, and store it in the member DWORD m_nHash
void CG1Packet::CacheHash()
{
	// Point pInput at the packet's buffer, where the bytes of the packet are stored
	BYTE* pInput = m_pBuffer;

	// Start out with the packet's hash set to 0
	m_nHash = 0;

	// Loop once for every byte in the packet
	for ( DWORD nPosition = m_nLength ; nPosition ; nPosition-- ) // If there are 10 bytes in the packet, loops 10 times, the loop doesn't use this index
	{
		// Use the byte under pInput to adjust a simple 4 byte hash of the packet
		m_nHash = ( m_nHash << 8 ) | ( ( m_nHash >> 24 ) ^ *pInput++ ); // After reading the byte under pInput, move to the next one
	}

	// If the last bit in the hash is 0, make it a 1
	m_nHash |= 1;
}

// Takes the number of bytes in the packet to hash, or leave that blank to hash all of them
// Computes the SHA hash of the packet
// Returns true and writes the hash under pHash, or returns false on error
BOOL CG1Packet::GetRazaHash(SHA1* pHash, DWORD nLength) const
{
	// If the caller didn't specify a length, use the length of the packet
	if ( nLength == 0xFFFFFFFF ) nLength = m_nLength;
	if ( (DWORD)m_nLength < nLength ) return FALSE; // Make sure the caller didn't ask for more than that

	// Hash the data, writing the hash under pHash, and report success
	CSHA pSHA;                             // Make a local CSHA object which will compute the hash
	pSHA.Add( &m_pGUID, sizeof(m_pGUID) ); // Start by hashing the GUID of the packet
	pSHA.Add( &m_nType, sizeof(m_nType) ); // Then throw in the type byte
	pSHA.Add( m_pBuffer, nLength );        // After that, hash the bytes of the packet
	pSHA.Finish();                         // Tell the object that is all
	pSHA.GetHash( pHash );                 // Have the object write the hash under pHash
	return TRUE;                           // Report success
}

//////////////////////////////////////////////////////////////////////
// CG1Packet string conversion

// Use this array to lookup a packet index and get title text, like CG1Packet::m_pszPackets[ 1 ] resolves to "Ping"
LPCTSTR CG1Packet::m_pszPackets[ G1_PACKTYPE_MAX ] = // There are 9 packet types, with values 0 through 8
{
	// 0 is unknown, 1 Ping, 2 Pong, and so on
	_T("Unknown"), _T("Ping"), _T("Pong"), _T("Bye"), _T("QRP"), _T("Vendor"), _T("Push"), _T("Query"), _T("Hit")
};

// Uses the packet type index in this packet to look up text that describes that type
// Returns text like "Ping" or "Pong"
LPCTSTR CG1Packet::GetType() const
{
	// Return the pointer to the text literal defined above this method
	return m_pszPackets[ m_nTypeIndex ];
}

// Describes the GUID of this packet as text using base 16 notation
// Returns a string
CString CG1Packet::GetGUID() const
{
	// Compose a string like "0001020304050607080910111213141516" with two characters to describe each of the 16 bytes of the GUID
	CString strOut;
	strOut.Format( _T("%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X"),
		m_pGUID.n[0],  m_pGUID.n[1],  m_pGUID.n[2],  m_pGUID.n[3],
		m_pGUID.n[4],  m_pGUID.n[5],  m_pGUID.n[6],  m_pGUID.n[7],
		m_pGUID.n[8],  m_pGUID.n[9],  m_pGUID.n[10], m_pGUID.n[11],
		m_pGUID.n[12], m_pGUID.n[13], m_pGUID.n[14], m_pGUID.n[15] );

	// Return it
	return strOut;
}

//////////////////////////////////////////////////////////////////////
// CG1Packet buffer output

// Takes a pointer to a buffer
// Writes this Gnutella packet into it, composing a Gnutella packet header and then adding the payload from the packet's buffer
void CG1Packet::ToBuffer(CBuffer* pBuffer) const
{
	// Compose a Gnutella packet header with values from this CG1Packet object
	GNUTELLAPACKET pHeader;              // Make a local GNUTELLAPACKET structure called pHeader
	pHeader.m_pGUID   = m_pGUID;         // Copy in the GUID
	pHeader.m_nType   = m_nType;         // Copy in the type byte
	pHeader.m_nTTL    = m_nTTL;          // Copy in the TTL and hops counts
	pHeader.m_nHops   = m_nHops;
	pHeader.m_nLength = (LONG)m_nLength; // Copy in the payload length number of bytes

	// Add the Gnutella packet header and packet payload to the buffer
	pBuffer->Add( &pHeader, sizeof(pHeader) ); // First, copy the bytes of the Gnutella packet header structure we made right here
	pBuffer->Add( m_pBuffer, m_nLength );      // This packet object's buffer is the payload, copy that in after the header
}

//////////////////////////////////////////////////////////////////////
// CG1Packet debug

// Takes text that describes something that happened when debugging the program
// Writes it into a line at the bottom of the file Shareaza.log
void CG1Packet::Debug(LPCTSTR pszReason) const
{

// Only include these lines in the program if it is being compiled in debug mode
#ifdef _DEBUG

	// If settings have turned off debugging, do nothing and leave now
	if ( ! Settings.General.Debug ) return;

	// Local objects
	CString strOutput; // We'll compose text that describes what happened here
	CFile   pFile;     // Then, we'll write that text into the bottom of this file

	// Open a file named Shareaza.log with the ability to read and write from it
	if ( pFile.Open( _T("\\Shareaza.log"), CFile::modeReadWrite ) )
	{
		pFile.Seek( 0, CFile::end );

	} // Opening the file didn't work, probably because it doesn't exist yet
	else
	{
		// Create it and request read only access
		if ( ! pFile.Open( _T("\\Shareaza.log"), CFile::modeWrite | CFile::modeCreate ) ) return; // If that still didn't work, leave now
	}

	// Compose the given reason into more complete and descriptive text
	strOutput.Format( _T("%s: %s [%i/%i] %s\r\n\r\n"), pszReason, GetType(), m_nTTL, m_nHops, (LPCTSTR)ToASCII() );

	// Set up the ATL string conversion macros, like T2CA
	USES_CONVERSION; // This is a macro that alerts the compiler to add a lot of code here for the macros used beneath it

	// Convert the string of TCHARs into ASCII, and get a pointer to the constant text
	LPCSTR pszOutput = T2CA( (LPCTSTR)strOutput ); // T2CA means text to constant ASCII

	// Write that text into the file
	pFile.Write( pszOutput, strlen(pszOutput) );

	// Loop the index i down each byte in the packet buffer
	for ( DWORD i = 0 ; i < m_nLength ; i++ )
	{
		// Read the byte there as an int called nChar
		int nChar = m_pBuffer[i];

		// Encode it as two base 16 characters followed by an ASCII character, like "00(.) " or "41(A) "
		strOutput.Format( _T("%.2X(%c) "), nChar, ( nChar >= 32 ? nChar : '.' ) );

		// Convert that little string into ASCII, and write it into the log file
		pszOutput = T2CA( (LPCTSTR)strOutput );
		pFile.Write( pszOutput, strlen(pszOutput) );
	}

	// End the line with two newlines, and close the file
	pFile.Write( "\r\n\r\n", 4 ); // Each newline is the 2 bytes "\r\n"
	pFile.Close();

// Go back to including all the lines in the program
#endif

}
