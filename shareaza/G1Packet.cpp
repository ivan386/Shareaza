//
// G1Packet.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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

// CG1Packet represents a Gnutella packet, and CG1PacketPool keeps lists of them
// http://shareazasecurity.be/wiki/index.php?title=Developers.Code.CG1Packet

#include "StdAfx.h"
#include "Shareaza.h"
#include "Buffer.h"
#include "Settings.h"
#include "Datagrams.h"
#include "DiscoveryServices.h"
#include "G1Packet.h"
#include "HostCache.h"
#include "LibraryMaps.h"
#include "Network.h"
#include "Security.h"
#include "Statistics.h"
#include "VendorCache.h"

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
CG1Packet* CG1Packet::New(int nType, DWORD nTTL, const Hashes::Guid& oGUID)
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
	if ( oGUID )
	{
		// Copy the GUID into the packet
		pPacket->m_oGUID = oGUID;

	} // If the caller didn't give us a GUID to use
	else
	{
		// Create a GUID for this packet
		Network.CreateID( pPacket->m_oGUID );
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
BOOL CG1Packet::GetRazaHash(Hashes::Sha1Hash& oHash, DWORD nLength) const
{
	// If the caller didn't specify a length, use the length of the packet
	if ( nLength == 0xFFFFFFFF ) nLength = m_nLength;
	if ( (DWORD)m_nLength < nLength ) return FALSE; // Make sure the caller didn't ask for more than that

	// Hash the data, writing the hash under pHash, and report success
	CSHA pSHA;                             // Make a local CSHA object which will compute the hash
	pSHA.Add( &m_oGUID[ 0 ], Hashes::Guid::byteCount ); // Start by hashing the GUID of the packet
	pSHA.Add( &m_nType, sizeof(m_nType) ); // Then throw in the type byte
	pSHA.Add( m_pBuffer, nLength );        // After that, hash the bytes of the packet
	pSHA.Finish();                         // Tell the object that is all
	pSHA.GetHash( &oHash[ 0 ] );           // Have the object write the hash under pHash
	oHash.validate();
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
CString CG1Packet::GetType() const
{
	// Return the pointer to the text literal defined above this method
	return m_pszPackets[ m_nTypeIndex ];
}

// Describes the GUID of this packet as text using base 16 notation
// Returns a string
CString CG1Packet::GetGUID() const
{
	// Compose a string like "0001020304050607080910111213141516" with two characters to describe each of the 16 bytes of the GUID
	return m_oGUID.toString< Hashes::base16Encoding >();
}

//////////////////////////////////////////////////////////////////////
// CG1Packet buffer output

// Takes a pointer to a buffer
// Writes this Gnutella packet into it, composing a Gnutella packet header and then adding the payload from the packet's buffer
void CG1Packet::ToBuffer(CBuffer* pBuffer) const
{
	// Compose a Gnutella packet header with values from this CG1Packet object
	GNUTELLAPACKET pHeader;						// Make a local GNUTELLAPACKET structure called pHeader
	pHeader.m_oGUID		= m_oGUID.storage();	// Copy in the GUID
	pHeader.m_nType		= m_nType;				// Copy in the type byte
	pHeader.m_nTTL		= m_nTTL;				// Copy in the TTL and hops counts
	pHeader.m_nHops		= m_nHops;
	pHeader.m_nLength	= (LONG)m_nLength;		// Copy in the payload length number of bytes

	// Abort if the buffer isn't big enough for the packet
	if ( !pBuffer->EnsureBuffer( sizeof(pHeader) + m_nLength ) ) return;

	// Add the Gnutella packet header and packet payload to the buffer
	pBuffer->Add( &pHeader, sizeof(pHeader) );	// First, copy the bytes of the Gnutella packet header structure we made right here
	pBuffer->Add( m_pBuffer, m_nLength );		// This packet object's buffer is the payload, copy that in after the header
}

//////////////////////////////////////////////////////////////////////
// CG1Packet debug

// Takes text that describes something that happened when debugging the program
// Writes it into a line at the bottom of the file Shareaza.log
void CG1Packet::Debug(LPCTSTR pszReason) const
{

// Only include these lines in the program if it is being compiled in debug mode
#ifdef _DEBUG

	// Local objects
	CString strOutput; // We'll compose text that describes what happened here
	strOutput.Format( L"[G1] %s Type: %s [%i/%i]", pszReason, GetType(), m_nTTL, m_nHops );
	CPacket::Debug( strOutput );
#else
	UNUSED_ALWAYS(pszReason);
// Go back to including all the lines in the program
#endif

}

int CG1Packet::GGEPReadCachedHosts(const CGGEPBlock& pGGEP)
{
	int nCount = -1;

	CGGEPItem* pIPPs = pGGEP.Find( GGEP_HEADER_PACKED_IPPORTS, 6 );
	if ( pIPPs && ( pIPPs->m_nLength - pIPPs->m_nPosition ) % 6 == 0 )
	{
		nCount = 0;
		while ( pIPPs->m_nPosition != pIPPs->m_nLength )
		{
			DWORD nAddress = 0;
			WORD nPort = 0;
			pIPPs->Read( (void*)&nAddress, 4 );
			pIPPs->Read( (void*)&nPort, 2 );
			theApp.Message( MSG_DEBUG, _T("Got G1 host %s:%i"), 
				(LPCTSTR)CString( inet_ntoa( *(IN_ADDR*)&nAddress ) ), nPort ); 
			CHostCacheHostPtr pCachedHost =
				HostCache.Gnutella1.Add( (IN_ADDR*)&nAddress, nPort );
			if ( pCachedHost ) nCount++;
		}
	}

	if ( Settings.Experimental.EnableDIPPSupport )
	{
		CGGEPItem* pGDNAs = pGGEP.Find( GGEP_HEADER_GDNA_PACKED_IPPORTS, 6 );
		if ( ! pGDNAs )
			// GDNA has a bug in their code; they send DIP but receive DIPP (fixed in the latest versions)
			pGDNAs = pGGEP.Find( GGEP_HEADER_GDNA_PACKED_IPPORTS_x, 6 );
		if ( pGDNAs && ( pGDNAs->m_nLength - pGDNAs->m_nPosition ) % 6 == 0 )
		{
			if ( nCount == -1 ) nCount = 0;
			while ( pGDNAs->m_nPosition != pGDNAs->m_nLength )
			{
				DWORD nAddress = 0;
				WORD nPort = 0;
				pGDNAs->Read( (void*)&nAddress, 4 );
				pGDNAs->Read( (void*)&nPort, 2 );
				theApp.Message( MSG_DEBUG, _T("Got GDNA host %s:%i"), 
					(LPCTSTR)CString( inet_ntoa( *(IN_ADDR*)&nAddress ) ), nPort ); 
				CHostCacheHostPtr pCachedHost =
					HostCache.Gnutella1.Add( (IN_ADDR*)&nAddress, nPort );
				if ( pCachedHost ) nCount++;
				HostCache.G1DNA.Add( (IN_ADDR*)&nAddress, nPort, 0, _T("GDNA") );
			}
		}
	}

	return nCount;
}

int CG1Packet::GGEPWriteRandomCache(CGGEPItem* pItem)
{
	if ( !pItem ) return 0;

	bool bIPP = false;
	if ( pItem->IsNamed( GGEP_HEADER_PACKED_IPPORTS ) )
		bIPP = true;
	else if ( !pItem->IsNamed( GGEP_HEADER_GDNA_PACKED_IPPORTS ) &&
		!pItem->IsNamed( GGEP_HEADER_GDNA_PACKED_IPPORTS_x ) )
	return 0;

	DWORD nCount = min( 50ul,
		( bIPP ? HostCache.Gnutella1.CountHosts() : HostCache.G1DNA.CountHosts() ) );
	WORD nPos = 0;

	// Create 5 random positions from 0 to 50 in the descending order
	std::vector< WORD > pList;
	pList.reserve( Settings.Gnutella1.MaxHostsInPongs );
	for ( WORD nNo = 0 ; nNo < Settings.Gnutella1.MaxHostsInPongs ; nNo++ )
	{
		pList.push_back( GetRandomNum( 0ui16, 50ui16 ) );
	}
	std::sort( pList.begin(), pList.end(), CompareNums() );

	nCount = Settings.Gnutella1.MaxHostsInPongs;

	if ( bIPP )
	{
		CQuickLock oLock( HostCache.Gnutella1.m_pSection );

		for ( CHostCacheIterator i = HostCache.Gnutella1.Begin() ;
			i != HostCache.Gnutella1.End() && nCount && ! pList.empty() ; ++i )
		{
			CHostCacheHostPtr pHost = (*i);

			nPos = pList.back();	// take the smallest value;
			pList.pop_back();		// remove it
			for ( ; i != HostCache.Gnutella1.End() && nPos-- ; ++i ) pHost = (*i);

			// We won't provide Shareaza hosts for G1 cache, since users may disable
			// G1 and it will pollute the host caches ( ??? )
			if ( i != HostCache.Gnutella1.End() &&
				pHost->m_nFailures == 0 && pHost->m_bCheckedLocally &&
				! ( pHost->m_pVendor && pHost->m_pVendor->m_sCode == L"GDNA" ) )
			{
				pItem->Write( (void*)&pHost->m_pAddress, 4 );
				pItem->Write( (void*)&pHost->m_nPort, 2 );
				theApp.Message( MSG_DEBUG, _T("Sending G1 host through pong (%s:%i)"),
					(LPCTSTR)CString( inet_ntoa( *(IN_ADDR*)&pHost->m_pAddress ) ), pHost->m_nPort );
				nCount--;
			}

			if ( i == HostCache.Gnutella1.End() )
				break;
		}
	}
	else
	{
		CQuickLock oLock( HostCache.G1DNA.m_pSection );

		for ( CHostCacheIterator i = HostCache.G1DNA.Begin() ;
			i != HostCache.G1DNA.End() && nCount && ! pList.empty() ; ++i )
		{
			CHostCacheHostPtr pHost = (*i);

			nPos = pList.back();	// take the smallest value;
			pList.pop_back();		// remove it
			for ( ; i != HostCache.G1DNA.End() && nPos-- ; ++i ) pHost = (*i);

			// We won't provide Shareaza hosts for G1 cache, since users may disable
			// G1 and it will pollute the host caches ( ??? )
			if ( i != HostCache.G1DNA.End() &&
				pHost->m_nFailures == 0 && pHost->m_bCheckedLocally &&
				( pHost->m_pVendor && pHost->m_pVendor->m_sCode == L"GDNA" ) )
			{
				pItem->Write( (void*)&pHost->m_pAddress, 4 );
				pItem->Write( (void*)&pHost->m_nPort, 2 );
				theApp.Message( MSG_DEBUG, _T("Sending GDNA host through pong (%s:%i)"),
					(LPCTSTR)CString( inet_ntoa( *(IN_ADDR*)&pHost->m_pAddress ) ), pHost->m_nPort );
				nCount--;
			}

			if ( i == HostCache.G1DNA.End() )
				break;
		}
	}
	return Settings.Gnutella1.MaxHostsInPongs - nCount;
}

bool CG1Packet::IsOOBEnabled()
{
	return ( Network.IsFirewalled( CHECK_UDP ) == FALSE && Settings.Gnutella1.EnableOOB );
}

bool CG1Packet::IsFirewalled()
{
	return ( Network.IsFirewalled( CHECK_TCP ) != FALSE );
}

//////////////////////////////////////////////////////////////////////
// UDP packet handler

BOOL CG1Packet::OnPacket(const SOCKADDR_IN* pHost)
{
	SmartDump( pHost, TRUE, FALSE );

	switch ( m_nType )
	{
	case G1_PACKET_PING:
		return OnPing( pHost );
	case G1_PACKET_PONG:
		return OnPong( pHost );
	case G1_PACKET_VENDOR:
		return OnVendor( pHost );
	default:
		CString tmp;
		tmp.Format( _T("Received unexpected UDP packet from %s:%u"),
			(LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ),
			htons( pHost->sin_port ) );
		Debug( tmp );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// PING packet handler for G1UDP

BOOL CG1Packet::OnPing(const SOCKADDR_IN* pHost)
{
	Statistics.Current.Gnutella1.PingsReceived++;

	CString strAddress( inet_ntoa( pHost->sin_addr ) );

	// A ping packet is just a header, and shouldn't have length, if it does, and settings say to worry about stuff like this
	if ( m_nLength != 0 && Settings.Gnutella1.StrictPackets )
	{
		// Record the error, drop the packet, but stay connected
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_SIZE_PING, (LPCTSTR)strAddress );
		Statistics.Current.Gnutella1.Dropped++;
		return TRUE;

	} // The ping is just a header, or settings don't care, and the length is bigger than settings allow
	else if ( m_nLength > Settings.Gnutella1.MaximumQuery )
	{
		// Record the error, drop the packet, but stay connected
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_TOO_LARGE, (LPCTSTR)strAddress );
		Statistics.Current.Gnutella1.Dropped++;
		return TRUE;
	}

	bool bSCP = false;
	bool bDNA = false;

	// If this ping packet strangely has length, and the remote computer does GGEP blocks
	if ( m_nLength && Settings.Gnutella1.EnableGGEP )
	{
		// There is a GGEP block here, and checking and adjusting the TTL and hops counts worked
		CGGEPBlock pGGEP;
		if ( pGGEP.ReadFromPacket( this ) )
		{
			if ( CGGEPItem* pItem = pGGEP.Find( GGEP_HEADER_SUPPORT_CACHE_PONGS ) )
			{
				bSCP = true;
			}
			if ( CGGEPItem* pItem = pGGEP.Find( GGEP_HEADER_SUPPORT_GDNA ) )
			{
				bDNA = true;
			}
		}
		else
		{
			// It's not, drop the packet, but stay connected
			theApp.Message( MSG_ERROR, IDS_PROTOCOL_GGEP_REQUIRED, (LPCTSTR)strAddress );
			Statistics.Current.Gnutella1.Dropped++;
			return TRUE;
		}
	}

	CGGEPBlock pGGEP;
	if ( bSCP )
	{
		GGEPWriteRandomCache( pGGEP.Add( GGEP_HEADER_PACKED_IPPORTS ) );
	}
	if ( Settings.Experimental.EnableDIPPSupport )
	{
		if ( bDNA )
		{
			GGEPWriteRandomCache( pGGEP.Add( GGEP_HEADER_GDNA_PACKED_IPPORTS ) );
		}
	}

	// Make a new pong packet, the response to a ping
	CG1Packet* pPong = New(	// Gets it quickly from the Gnutella packet pool
		G1_PACKET_PONG,		// We're making a pong packet
		m_nHops,			// Give it TTL same as HOP count of received PING packet
		m_oGUID);			// Give it the same GUID as the ping

	// Get statistics about how many files we are sharing
	QWORD nMyVolume = 0;
	DWORD nMyFiles = 0;
	LibraryMaps.GetStatistics( &nMyFiles, &nMyVolume );

	// Start the pong's payload with the IP address and port number from the Network object (do)
	pPong->WriteShortLE( htons( Network.m_pHost.sin_port ) );
	pPong->WriteLongLE( Network.m_pHost.sin_addr.S_un.S_addr );

	// Then, write in the information about how many files we are sharing
	pPong->WriteLongLE( nMyFiles );
	pPong->WriteLongLE( (DWORD)nMyVolume );

	if ( ! pGGEP.IsEmpty() )
		pGGEP.Write( pPong );

	// Send the pong packet to the remote computer we are currently looping on
	Datagrams.Send( pHost, pPong );
	Statistics.Current.Gnutella1.PongsSent++;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// PONG packet handler

BOOL CG1Packet::OnPong(const SOCKADDR_IN* pHost)
{
	Statistics.Current.Gnutella1.PongsReceived++;

	// If the pong is too short, or the pong is too long and settings say we should watch that
	if ( m_nLength < 14 || ( m_nLength > 14 && Settings.Gnutella1.StrictPackets && ! Settings.Gnutella1.EnableGGEP ) )
	{
		// Pong packets should be 14 bytes long, drop this strangely sized one
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_SIZE_PONG, (LPCTSTR)inet_ntoa( pHost->sin_addr ) );
		Statistics.Current.Gnutella1.Dropped++;
		return TRUE; // Don't disconnect from the remote computer, though
	}

	// Read information from the pong packet
	WORD nPort     = ReadShortLE(); // 2 bytes, port number (do) of us? the remote computer? the computer that sent the packet?
	DWORD nAddress = ReadLongLE();  // 4 bytes, IP address
	DWORD nFiles   = ReadLongLE();  // 4 bytes, the number of files the source computer is sharing
	DWORD nVolume  = ReadLongLE();  // 4 bytes, the total size of all those files
	UNUSED_ALWAYS(nFiles);
	UNUSED_ALWAYS(nVolume);

	CDiscoveryService* pService = DiscoveryServices.GetByAddress(
		&(pHost->sin_addr) , ntohs( pHost->sin_port ), CDiscoveryService::dsGnutellaUDPHC );

	// If that IP address is in our list of computers to not talk to, except ones in UHC list in discovery
	if ( pService == NULL && Security.IsDenied( (IN_ADDR*)&nAddress ) )
	{
		// Record the packet as dropped, do nothing else, and leave now
		Statistics.Current.Gnutella1.Dropped++;
		return TRUE;
	}

	// If the pong is bigger than 14 bytes, and the remote compuer told us in the handshake it supports GGEP blocks
	if ( m_nLength > 14 && Settings.Gnutella1.EnableGGEP )
	{
		// There is a GGEP block here, and checking and adjusting the TTL and hops counts worked
		CGGEPBlock pGGEP;
		if ( pGGEP.ReadFromPacket( this ) )
		{
			// Read vendor code
			CString strVendorCode;
			if ( CGGEPItem* pVC = pGGEP.Find( GGEP_HEADER_VENDOR_INFO, 4 ) )
			{
				CHAR szaVendor[ 4 ] = {};
				pVC->Read( szaVendor,4 );
				TCHAR szVendor[5] = { szaVendor[0], szaVendor[1], szaVendor[2], szaVendor[3], 0 };
				strVendorCode = szVendor;
				strVendorCode.Trim();
			}

			// Read daily uptime
			DWORD nUptime = 0;
			if ( CGGEPItem* pDU = pGGEP.Find( GGEP_HEADER_DAILY_AVERAGE_UPTIME, 1 ) )
			{
				pDU->Read( (void*)&nUptime, 4 );
			}

			// Catch pongs and update host cache only from ultrapeers
			if ( CGGEPItem* pUP = pGGEP.Find( GGEP_HEADER_UP_SUPPORT ) )
			{
				HostCache.Gnutella1.Add( (IN_ADDR*)&nAddress, nPort, 0,
					( strVendorCode.IsEmpty() ? NULL : (LPCTSTR)strVendorCode ),
					nUptime );

				if ( CGGEPItem* pGDNA = pGGEP.Find( GGEP_HEADER_SUPPORT_GDNA ) )
				{
					HostCache.G1DNA.Add( (IN_ADDR*)&nAddress, nPort, 0,
						( strVendorCode.IsEmpty() ? NULL : (LPCTSTR)strVendorCode ),
						nUptime );
				}
			}

			int nCount = GGEPReadCachedHosts( pGGEP );

			// Update Gnutella UDPHC state
			if ( nCount != -1 && pService )
			{
				pService->OnSuccess();
				pService->m_nHosts = nCount;
				pService->OnCopyGiven();
			}
		}
		else
		{
			// It's not, drop the packet, but stay connected
			theApp.Message( MSG_ERROR, IDS_PROTOCOL_GGEP_REQUIRED, (LPCTSTR)inet_ntoa( pHost->sin_addr ) );
			Statistics.Current.Gnutella1.Dropped++;
			return TRUE;
		}
	}

	return TRUE;
}

BOOL CG1Packet::OnVendor(const SOCKADDR_IN* pHost)
{
	// If the packet payload is smaller than 8 bytes, or settings don't allow vendor messages
	if ( m_nLength < 8 || ! Settings.Gnutella1.VendorMsg )
	{
		Statistics.Current.Gnutella1.Dropped++;
		return TRUE;
	}

	// Read the vendor, function, and version numbers from the packet payload
	DWORD nVendor  = ReadLongBE();  // 4 bytes, vendor code in ASCII characters, like "RAZA" (do)
	WORD nFunction = ReadShortLE(); // 2 bytes, function (do)
	WORD nVersion  = ReadShortLE(); // 2 bytes, version (do)

	if ( nVendor == 'LIME' )
	{
		if ( nFunction == 23 && nVersion == 2 )
		{
			// TODO: HEAD ping
			return TRUE;
		}
	}

	CString tmp;
	tmp.Format( _T("Received vendor packet from %s:%u Function: %u Version: %u"),
		(LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ), htons( pHost->sin_port ),
		nFunction, nVersion );
	Debug( tmp );

	return TRUE;
}
