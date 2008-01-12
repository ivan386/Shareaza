//
// Neighbour.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

// CNeighbour is in the middle of the CConnection inheritance tree, adding compression and a bunch of member variables
// http://wiki.shareaza.com/static/Developers.Code.CNeighbour

// Copy in the contents of these files here before compiling
#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "Network.h"
#include "Neighbours.h"
#include "Neighbour.h"
#include "Security.h"
#include "RouteCache.h"
#include "Library.h"
#include "Buffer.h"
#include "Packet.h"
#include "G1Packet.h"
#include "G2Packet.h"
#include "SearchManager.h"
#include "QueryHashTable.h"
#include "QueryHashGroup.h"
#include "QueryHashMaster.h"
#include "QueryHit.h"
#include "GProfile.h"
#include "Statistics.h"

// If we are compiling in debug mode, replace the text "THIS_FILE" in the code with the name of this file
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// Set Z_TIMER to 1/5th of a second
#define Z_TIMER 200

//////////////////////////////////////////////////////////////////////
// CNeighbour construction

// Make a new CNeighbour object for a certain network
// Takes a protocol ID, like PROTOCOL_G1
CNeighbour::CNeighbour(PROTOCOLID nProtocol) :
	m_nRunCookie( 0 ),
	m_nUnique( 0 ),
	// Copy the given network protocol into this new object
	m_nProtocol( nProtocol ),	// nProtocol is like PROTOCOL_ED2K
	// Set null and default values for connection state, software vendor, guid, and user profile
	m_nState( nrsNull ),		// No state now, soon we'll connect and do the handshake
	m_pVendor( NULL ),			// We don't know what brand software the remote computer is running yet
	m_pProfile( NULL ),			// No profile on the person running the remote computer yet
	// Set handshake values to defaults
	m_bAutomatic( FALSE ),		// Automatic setting used to maintain the connection
	m_bShareaza( FALSE ),		// Expect the remote computer to not be running Shareaza
	m_nNodeType( ntNode ),		// Start out assuming that we and the remote computer are both hubs
	m_bQueryRouting( FALSE ),	// Don't start query routing or pong caching yet
	m_bPongCaching( FALSE ),	//
	m_bVendorMsg( FALSE ),		// The remote computer hasn't told us it supports the vendor-specific messages yet
	m_bGGEP( FALSE ),			// The remote computer hasn't told us it supports the GGEP block yet
	m_bBadClient( FALSE ),		//
	m_nDegree( (DWORD)-1 ),
	m_nMaxTTL( (DWORD)-1 ),
	m_bDynamicQuerying( FALSE ),
	m_bUltrapeerQueryRouting( FALSE ),
	m_sLocalePref( _T("") ),
	m_bRequeries( TRUE ),
	m_bExtProbes( FALSE ),
	// Start out time variables as 0
	m_tLastQuery( 0 ),			// We'll set these to the current tick or seconds count when we get a query or packet
	m_tLastPacket( 0 ),			//
	// Zero packet and file counts
	m_nInputCount( 0 ),
	m_nOutputCount( 0 ),
	m_nDropCount( 0 ),
	m_nLostCount( 0 ),
	m_nOutbound( 0 ),
	m_nFileCount( 0 ),
	m_nFileVolume( 0 ),
	// The local and remote query tables aren't set up yet, make the pointers to them start out null
	m_pQueryTableLocal( NULL ),
	m_pQueryTableRemote( NULL ),
	// Null pointers and zero counts for zlib compression
	m_pZInput( NULL ),
	m_pZOutput( NULL ),
	m_nZInput( 0 ),
	m_nZOutput( 0 ),
	m_pZSInput( NULL ),
	m_pZSOutput( NULL ),
	m_bZFlush( FALSE ),
	m_tZOutput( 0 )
{
}

// Make a new CNeighbour object, copying values from a base one
// Takes a protocol ID and a base neighbour to copy information from
CNeighbour::CNeighbour(PROTOCOLID nProtocol, CNeighbour* pBase)
	: CConnection( *pBase )
	, m_nRunCookie( 0 )
	, m_nUnique(           pBase->m_nUnique )
	, m_nProtocol( nProtocol )
	, m_nState( nrsConnected )
	, m_pVendor(           pBase->m_pVendor )
	, m_oGUID(             pBase->m_oGUID )
	, m_pProfile( NULL )
	, m_oMoreResultsGUID()
	, m_bAutomatic(        pBase->m_bAutomatic )
	, m_bShareaza(         pBase->m_bShareaza )
	, m_nNodeType(         pBase->m_nNodeType )
	, m_bQueryRouting(     pBase->m_bQueryRouting )
	, m_bPongCaching(      pBase->m_bPongCaching )
	, m_bVendorMsg(        pBase->m_bVendorMsg )
	, m_bGGEP(             pBase->m_bGGEP )
	, m_tLastQuery(        pBase->m_tLastQuery )
	, m_bBadClient(			pBase->m_bBadClient )
	, m_nDegree(			pBase->m_nDegree )
	, m_nMaxTTL(			pBase->m_nMaxTTL )
	, m_bDynamicQuerying(	pBase->m_bDynamicQuerying )
	, m_bUltrapeerQueryRouting(	pBase->m_bUltrapeerQueryRouting )
	, m_sLocalePref(		pBase->m_sLocalePref )
	, m_bRequeries(			pBase->m_bRequeries )
	, m_bExtProbes(			pBase->m_bExtProbes )
	, m_nInputCount(       pBase->m_nInputCount )
	, m_nOutputCount(      pBase->m_nOutputCount )
	, m_nDropCount(        pBase->m_nDropCount )
	, m_nLostCount(        pBase->m_nLostCount )
	, m_nOutbound(         pBase->m_nOutbound )
	, m_nFileCount(        pBase->m_nFileCount )
	, m_nFileVolume(       pBase->m_nFileVolume )
	// If this connected computer is sending and receiving Gnutella2 packets, it will also support query routing
	, m_pQueryTableRemote( m_bQueryRouting ? new CQueryHashTable : NULL )
	, m_pQueryTableLocal( m_bQueryRouting ? new CQueryHashTable : NULL )
	, m_tLastPacket( GetTickCount() )
	, m_pZInput(           pBase->m_pZInput )	// transfer of ownership
	, m_pZOutput(          pBase->m_pZOutput )
	, m_nZInput(           pBase->m_nZInput )
	, m_nZOutput(          pBase->m_nZOutput )
	, m_pZSInput( NULL )
	, m_pZSOutput( NULL )
	, m_bZFlush(           pBase->m_bZFlush )
	, m_tZOutput(          pBase->m_tZOutput )
{
	m_tConnected = m_tLastPacket;
	pBase->m_pZInput  = NULL;
	pBase->m_pZOutput = NULL;
	// Call CNeighboursBase::Add to keep track of this newly created CNeighbours object
	Neighbours.Add( this, FALSE );
}

// Delete this CNeighbour object
CNeighbour::~CNeighbour()
{
	// If m_pZSOutput isn't NULL, we are probably in the middle of a zlib compression operation
	if ( z_streamp pStream = (z_streamp)m_pZSOutput ) // This is correctly assignment, not comparison, in an if statement
	{
		// End the compression and delete the pStream object
		deflateEnd( pStream );
		delete pStream;
	}

	// Same thing, except for decompressing input
	if ( z_streamp pStream = (z_streamp)m_pZSInput )
	{
		// End the decompression and delete the pStream object
		inflateEnd( pStream );
		delete pStream;
	}

	// If any of these objects exist, delete them
	if ( m_pProfile )			delete m_pProfile;
	if ( m_pZOutput )			delete m_pZOutput;
	if ( m_pZInput )			delete m_pZInput;
	if ( m_pQueryTableRemote )	delete m_pQueryTableRemote;
	if ( m_pQueryTableLocal )	delete m_pQueryTableLocal;
}

//////////////////////////////////////////////////////////////////////
// CNeighbour close

// Close the connection to the remote computer
// Takes the reason we're closing the connection, IDS_CONNECTION_CLOSED by default
void CNeighbour::Close(UINT nError)
{
	// Make sure that the socket stored in this CNeighbour object is valid
	ASSERT( m_hSocket != INVALID_SOCKET );

	// If nError is the default closed or a result of peer pruning, we're closing the connection voluntarily
	BOOL bVoluntary = ( nError == IDS_CONNECTION_CLOSED || nError == IDS_CONNECTION_PEERPRUNE );

	// Remove this neighbour from the list of them
	Neighbours.Remove( this );

	// Actually close the socket connection to the remote computer
	CConnection::Close();

	// If this Close method was called with an error, among which IDS_CONNECTION_CLOSED counts
	if ( nError && nError != IDS_HANDSHAKE_REJECTED )
	{
		// Report a voluntary default close, or an error
		theApp.Message( bVoluntary ? MSG_DEFAULT : MSG_ERROR, nError, (LPCTSTR)m_sAddress );
	}

	// Delete this CNeighbour object, calling its destructor right now
	delete this;
}

// Close the connection, but not until we've written the buffered outgoing data first
// Takes the reason we're closing the connection, or 0 by default
void CNeighbour::DelayClose(UINT nError)
{
	// If this method got passed a close reason error, report it
	if ( nError ) theApp.Message( MSG_ERROR, nError, (LPCTSTR)m_sAddress );

	// Change this object's state to closing
	m_nState = nrsClosing;

	// Have the connection object write all the outgoing data soon
	QueueRun();
}

//////////////////////////////////////////////////////////////////////
// CNeighbour send

// Send a packet to the remote computer (do)
// Takes the packet, bRelease to release it, and bBuffered
BOOL CNeighbour::Send(CPacket* pPacket, BOOL bRelease, BOOL /*bBuffered*/)
{
	// If we should release the packet, call its release method (do)
	if ( bRelease ) pPacket->Release();

	// Always return false (do)
	return FALSE;
}

// CG1Neighbour, which inherits from CNeighbour, overrides this method with its own version that does something
BOOL CNeighbour::SendQuery(CQuerySearch* pSearch, CPacket* pPacket, BOOL bLocal)
{
	// If we're still negotiating the handshake with this remote computer, leave now
	if ( m_nState != nrsConnected )
		return FALSE;

	// This neighbour is a hub above us, and this isn't a local search, leave now
	if ( m_nNodeType == ntHub && ! bLocal )
		return FALSE;

	// If QHT exsist
	if ( m_pQueryTableRemote != NULL && m_pQueryTableRemote->m_bLive )
	{
		// If QHT disables search, leave now
		if ( ! m_pQueryTableRemote->Check( pSearch ) )
			return FALSE;

	} // If QHT doesn't exist and this connection is to a leaf below us, leave now
	else if ( m_nNodeType == ntLeaf && ! bLocal )
		return FALSE;

	// If this is local (do), set the last query time this CG1Neighbour object remembers to now
	if ( bLocal )
		m_tLastQuery = static_cast< DWORD >( time( NULL ) );

	// Send the packet to the remote computer
	Send( pPacket, FALSE, ! bLocal ); // Submit !bLocal as the value for bBuffered (do)

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CNeighbour run event handler

// Check the connection hasn't gone silent too long, and send a query patch table if necessary
// Returns true if the connection is still open and there was no error sending the patch table
BOOL CNeighbour::OnRun()
{
	// Get the tick count right now
	DWORD tNow = GetTickCount();

	// If it's been awhile since the last packet came in through this connection
	if ( tNow - m_tLastPacket > Settings.Connection.TimeoutTraffic )
	{
		// Close the connection, citing traffic timeout as the reason, and return false
		Close( IDS_CONNECTION_TIMEOUT_TRAFFIC );
		return FALSE;
	}

	// If this connection is to a hub above us, or to a Gnutella2 hub like us
	if ( ( m_nNodeType == ntHub || ( m_nNodeType == ntNode && m_nProtocol == PROTOCOL_G2 ) ) &&
		// And if we have a local query table for this neighbour and its cookie isn't the master cookie (do)
	     ( m_pQueryTableLocal != NULL && m_pQueryTableLocal->m_nCookie != QueryHashMaster.m_nCookie ) &&
		// And it's been more than 60 seconds since the last update (do)
		 ( tNow - m_pQueryTableLocal->m_nCookie > 60000 ) &&
		// And it's been more than 30 seconds since the master cookie (do)
		 ( tNow - QueryHashMaster.m_nCookie > 30000 ||								
		// Or, the master cookie is a minute older than the local one (do)
			QueryHashMaster.m_nCookie - m_pQueryTableLocal->m_nCookie > 60000 ||
		// Or, the local query table is not live (do)
			! m_pQueryTableLocal->m_bLive ) )
	{
		// Then send the connected computer a query hash table patch
		if ( m_pQueryTableLocal->PatchTo( &QueryHashMaster, this ) )
		{
			// There was an error, record it
			theApp.Message( MSG_SYSTEM, IDS_PROTOCOL_QRP_SENT, (LPCTSTR)m_sAddress,
				m_pQueryTableLocal->m_nBits, m_pQueryTableLocal->m_nHash,
				m_pQueryTableLocal->m_nInfinity, m_pQueryTableLocal->GetPercent() );
		}
	}

	// Report success
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CNeighbour close event handler

// Call when the connection has been dropped
// Logs the error with some statistics about how long it was quiet
void CNeighbour::OnDropped(BOOL /*bError*/)
{
	// Find out how many seconds it's been since this neighbour connected, and since it received the last packet
	DWORD nTime1 = ( GetTickCount() - m_tConnected ) / 1000;	// Time since connected in seconds
	DWORD nTime2 = ( GetTickCount() - m_tLastPacket ) / 1000;	// Time since last packet received in seconds

	// Report these times in a message about the connection being dropped
	theApp.Message( MSG_DEBUG, _T("Dropped neighbour %s (%s), conn: %.2i:%.2i, packet: %.2i:%.2i"),
		(LPCTSTR)m_sAddress, (LPCTSTR)m_sUserAgent,
		nTime1 / 60, nTime1 % 60, nTime2 / 60, nTime2 % 60 );

	// Close the connection, citing the reason as dropped
	Close( IDS_CONNECTION_DROPPED );
}

//////////////////////////////////////////////////////////////////////
// CNeighbour compressed read and write handlerss

// Read data waiting in the socket into the input buffer, and decompress it into the zlib input buffer
// Return false if error
BOOL CNeighbour::OnRead()
{
	// Read the bytes waiting in our end of the socket into the input buffer
	CConnection::OnRead();

	CLockedBuffer pInput( GetInput() );

	// If compression is off, we're done, return true now
	// Otherwise, compression is on, the bytes we read into the input buffer are compressed, and we have to decompress them
	if ( m_pZInput == NULL || pInput->m_nLength == 0 ) return TRUE;

	// Start or continue using zlib with m_pZSInput and pStream pointers
	BOOL bNew = ( m_pZSInput == NULL );			// If the zlib input pointer is null, make bNew true, otherwise it's false
	if ( bNew ) m_pZSInput = new z_stream;		// If the zlib input pointer is null, make a new z_stream
	z_streamp pStream = (z_streamp)m_pZSInput;	// Cast the pointer as a z_streamp and call that pStream

	// If we are starting to use zlib now
	if ( bNew )
	{
		// Zero the z_stream structure and set it up for decompression
		ZeroMemory( pStream, sizeof(z_stream) );
		if ( inflateInit( pStream ) != Z_OK )
		{
			// There was an error setting up zlib, clean up and leave now
			delete pStream;
			delete m_pZInput;
			m_pZInput = NULL;
			m_pZSInput = NULL;
			return TRUE; // Report success anyway
		}
	}

	// Loop until there is no more data for zlib to decompress
	while ( pInput->m_nLength ||							// While the input buffer has data in it
			m_pZInput->m_nLength == m_pZInput->m_nBuffer || // Or the zlib buffer length is the same as the buffer (do)
			pStream->avail_out == 0 )						// Or there is no more data availiable for zlib to decompress
	{
		// Make sure the zlib buffer holds at least 1 KB (do)
		m_pZInput->EnsureBuffer( 1024 );
		if ( m_pZInput->m_nBuffer < 1024 ) return FALSE;

		// Tell zlib where the compressed data is, how much is there, where it can output bytes, and how much space it has there
		pStream->next_in	= pInput->m_pBuffer;							// Point zlib at the start of the input buffer
		pStream->avail_in	= pInput->m_nLength;							// Give it the entire block there
		pStream->next_out	= m_pZInput->m_pBuffer + m_pZInput->m_nLength;	// Have it put decompressed bytes in the empty space of the ZInput buffer
		pStream->avail_out	= m_pZInput->m_nBuffer - m_pZInput->m_nLength;	// This is how much space is left there

		// Call zlib inflate to decompress the contents of m_pInput into the end of m_pZInput
		inflate( pStream, Z_SYNC_FLUSH ); // Zlib adjusts next in, avail in, next out, and avail out to record what it did

		// Zlib decompressed something
		if ( pStream->avail_in < pInput->m_nLength ) // We set avail_in to nLength, but zlib shrunk it smaller
		{
			// Remove the portion in the start of the input buffer that zlib decompressed
			pInput->Remove( pInput->m_nLength - pStream->avail_in ); // nLength - avail_in is the size of the block that zlib decompressed
		}

		// Calculate the size of nBlock, the block of data in ZInput that Zlib just decompressed
		DWORD nBlock = ( m_pZInput->m_nBuffer - m_pZInput->m_nLength ) - pStream->avail_out; // Buffer size minus previous length and current empty space

		// Record tha this many more bytes are stored in the ZInput buffer
		m_pZInput->m_nLength += nBlock;

		// Add this number of bytes to the zlib count
		m_nZInput += nBlock;

		// If Zlib didn't decompress anything, leave the loop
		if ( ! nBlock ) break;
	}

	// Report success
	return TRUE;
}

// Compress data and send it to the connected computer
// Return false if error
BOOL CNeighbour::OnWrite()
{
	// If we're not sending compressed data to the remote computer, just call CConnection::OnWrite to send the output buffer
	if ( m_pZOutput == NULL ) return CConnection::OnWrite(); // Return the result and leave now

	// Start or continue using zlib with m_pZSInput and pStream pointers
	BOOL bNew = ( m_pZSOutput == NULL );		// Make bNew true if zlib compression isn't setup yet
	if ( bNew ) m_pZSOutput = new z_stream;		// Create a new z_stream structure and point m_pZSOutput and pStream at it
	z_streamp pStream = (z_streamp)m_pZSOutput;
	
	// If we are starting to use zlib now
	if ( bNew )
	{
		// Zero the z_stream structure and set it up for compression
		ZeroMemory( pStream, sizeof(z_stream) );
		if ( deflateInit( pStream, 6 ) != Z_OK )
		{
			// There was an error setting up zlib, clean up and leave now
			delete pStream;
			delete m_pZOutput;
			m_pZOutput = NULL;
			m_pZSOutput = NULL;
			return TRUE; // Report success anyway
		}
	}

	CLockedBuffer pOutput( GetOutput() );

	// If there is data in the output buffer already
	if ( pOutput->m_nLength ) 
	{
		// Send it to the other computer
		CConnection::OnWrite();
		if ( pOutput->m_nLength ) return TRUE; // Return true if there is still more to send after this (do)
	}

	// If it's been more than 2 seconds since we've flushed the compressed output buffer to the remote computer, set the flag to do it next
	DWORD tNow = GetTickCount();
	if ( tNow - m_tZOutput >= Z_TIMER ) m_bZFlush = TRUE;

	// Loop until all the data in ZOutput has been compressed into Output
	while ( ( m_pZOutput->m_nLength && ! pOutput->m_nLength )	// ZOutput has data to compress and Output is empty
		|| pStream->avail_out == 0 )							// Or, zlib says it has no more room left (do)
	{
		// Make sure the output buffer is 1 KB (do)
		pOutput->EnsureBuffer( 1024 );
		if ( pOutput->m_nBuffer < 1024 ) return FALSE;

		// Tell zlib where the data to compress is, and where it should put the compressed data
		pStream->next_in	= m_pZOutput->m_pBuffer; // Start next_in and avail_in on the data in ZOutput
		pStream->avail_in	= m_pZOutput->m_nLength;
		pStream->next_out	= pOutput->m_pBuffer + pOutput->m_nLength; // Start next_out and avail_out on the empty space in Output
		pStream->avail_out	= pOutput->m_nBuffer - pOutput->m_nLength;

		// Call zlib inflate to decompress the contents of m_pInput into the end of m_pZInput
		deflate( pStream, m_bZFlush ? Z_SYNC_FLUSH : 0 ); // Zlib adjusts next in, avail in, next out, and avail out to record what it did

		// Add the number of uncompressed bytes that zlib compressed to the m_nZOutput count
		m_nZOutput += m_pZOutput->m_nLength - pStream->avail_in;

		// Remove the chunk that zlib compressed from the start of ZOutput
		m_pZOutput->Remove( m_pZOutput->m_nLength - pStream->avail_in );

		// Set nOutput to the size of the new compressed block in the output buffer between the data already there, and the empty space afterwards
		int nOutput = ( pOutput->m_nBuffer - pOutput->m_nLength ) - pStream->avail_out;

		// Zlib compressed something
		if ( nOutput )
		{
			// Add the new block to the length, and record when this happened
			pOutput->m_nLength += nOutput;
			m_tZOutput = tNow;

		// Zlib didn't compress anything, and the flush flag is true
		}
		else if ( m_bZFlush )
		{
			// Make the flush flag false
			m_bZFlush = FALSE;
		}

		// Send the contents of the output buffer to the remote computer
		CConnection::OnWrite();
	}

	// Report success
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CNeighbour common hit routing code

BOOL CNeighbour::OnCommonHit(CPacket* pPacket)
{
	CQueryHit* pHits = NULL;
	int nHops = 0;

	if ( pPacket->m_nProtocol == PROTOCOL_G1 )
	{
		pHits = CQueryHit::FromPacket( (CG1Packet*)pPacket, &nHops );
	}
	else if ( pPacket->m_nProtocol == PROTOCOL_G2 )
	{
		pHits = CQueryHit::FromPacket( (CG2Packet*)pPacket, &nHops );
	}
	else
	{
		ASSERT( FALSE );
	}
	
	if ( pHits == NULL )
	{
		pPacket->Debug( _T("Malformed Hit") );
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_BAD_HIT, (LPCTSTR)m_sAddress );
		m_nDropCount++;
		if ( m_nProtocol == PROTOCOL_G1 )
			Statistics.Current.Gnutella1.Dropped++;
		else if ( m_nProtocol == PROTOCOL_G2 )
			Statistics.Current.Gnutella2.Dropped++;
		return TRUE;
	}

	if ( Security.IsDenied( &pHits->m_pAddress ) )
	{
		pPacket->Debug( _T("Security manager denied Hit") );
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_BAD_HIT, (LPCTSTR)m_sAddress );
		m_nDropCount++;
		if ( m_nProtocol == PROTOCOL_G1 )
			Statistics.Current.Gnutella1.Dropped++;
		else if ( m_nProtocol == PROTOCOL_G2 )
			Statistics.Current.Gnutella2.Dropped++;
		pHits->Delete();
		return FALSE;
	}

	Network.NodeRoute->Add( pHits->m_oClientID, this );

	// Don't route exceeded hits
	if ( nHops <= (int)Settings.Gnutella1.MaximumTTL &&
		SearchManager.OnQueryHits( pHits ) )
	{
		Network.RouteHits( pHits, pPacket );
	}

	Network.OnQueryHits( pHits );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CNeighbour common query hash table

BOOL CNeighbour::OnCommonQueryHash(CPacket* pPacket)
{
	if ( m_pQueryTableRemote == NULL )
	{
		m_pQueryTableRemote = new CQueryHashTable();
		theApp.Message( MSG_DEFAULT, IDS_PROTOCOL_QRP_UNEXPECTED, (LPCTSTR)m_sAddress );
	}
	
	BOOL bLive = m_pQueryTableRemote->m_bLive;
	
	if ( ! m_pQueryTableRemote->OnPacket( pPacket ) )
	{
		theApp.Message( MSG_ERROR, IDS_PROTOCOL_QRP_FAILED, (LPCTSTR)m_sAddress );
		return FALSE;
	}
	
	if ( m_pQueryTableRemote->m_bLive && ! bLive )
	{
		theApp.Message( MSG_DEFAULT, IDS_PROTOCOL_QRP_UPDATED, (LPCTSTR)m_sAddress,
			m_pQueryTableRemote->m_nBits, m_pQueryTableRemote->m_nHash,
			m_pQueryTableRemote->m_nInfinity, m_pQueryTableRemote->GetPercent() );
	}
	
	if ( m_nNodeType == ntLeaf && m_pQueryTableRemote->m_pGroup == NULL )
	{
		QueryHashMaster.Add( m_pQueryTableRemote );
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CNeighbour compression statistics

// Calculate the average compression rate in either direction for this connection
// Takes pointers to numbers to write the answers in
void CNeighbour::GetCompression(float* pnInRate, float* pnOutRate)
{
	// Set the compression ratios to -1 to indicate that compression is off
	*pnInRate = -1; *pnOutRate = -1;

	// If there is a buffer for decompressing read data and a count of bytes have been decompressed
	if ( m_pZInput != NULL && m_nZInput > 0.)
	{
		// Calculate the compression rate of the data coming in
		*pnInRate = 1.0f				// 1 minus, so if there were 80 compressed bytes that inflated to 100, its 1 - (80 / 100) = .2
			- (float)m_mInput.nTotal	// The total number of compressed bytes the bandwidth meter has counted come in from the socket
			/ (float)m_nZInput;			// Divided by the total number of not compressed bytes that OnRead has decompressed into the m_nZInput buffer

		// Keep the rate between 0 and 1
		if ( *pnInRate < 0 ) *pnInRate = 0;
		else if ( *pnInRate > 1 ) *pnInRate = 1;
	}

	// If there is a buffer for compressing data to write and a count of bytes have been compressed
	if ( m_pZOutput != NULL && m_mOutput.nTotal > 0.)
	{
		// Calculate the compressing rate of the data going out
		*pnOutRate = 1.0f				// 1 minus, so if there were 100 bytes that deflated down to 70, its 1 - (70 / 100) = .3
			- (float)m_mOutput.nTotal	// The total number of compressed bytes the bandwidth meter has counted go out through the socket
			/ (float)m_nZOutput;		// Divided by the total number of not compressed bytes that OnWrite has compressed from the m_nZOutput buffer

		// Keep the rate between 0 and 1
		if ( *pnOutRate < 0 ) *pnOutRate = 0;
		else if ( *pnOutRate > 1 ) *pnOutRate = 1;
	}
}

DWORD CNeighbour::GetMaxTTL() const
{
	return ( m_nMaxTTL != (DWORD)-1 ) ? min( m_nMaxTTL, Settings.Gnutella1.SearchTTL ) :
		Settings.Gnutella1.SearchTTL;
}
