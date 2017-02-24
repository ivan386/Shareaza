//
// NeighboursWithConnect.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2017.
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "BTPacket.h"
#include "DCNeighbour.h"
#include "Datagrams.h"
#include "DiscoveryServices.h"
#include "EDNeighbour.h"
#include "HostCache.h"
#include "Kademlia.h"
#include "Neighbours.h"
#include "NeighboursWithConnect.h"
#include "Network.h"
#include "Security.h"
#include "ShakeNeighbour.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CNeighboursWithConnect construction

CNeighboursWithConnect::CNeighboursWithConnect()
	: m_nBandwidthIn	( 0 )
	, m_nBandwidthOut	( 0 )
	, m_bG2Leaf			( FALSE )
	, m_bG2Hub			( FALSE )
	, m_bG1Leaf			( FALSE )
	, m_bG1Ultrapeer	( FALSE )
	, m_nStableCount	( 0 )
	, m_tHubG2Promotion	( 0 )
	, m_tPresent		()
	, m_tPriority		()
	, m_tLastConnect	( 0 )
{
}

CNeighboursWithConnect::~CNeighboursWithConnect()
{
}

void CNeighboursWithConnect::Close()
{
	CNeighboursWithRouting::Close();

	m_nBandwidthIn	= 0;
	m_nBandwidthOut	= 0;
	m_bG2Leaf		= FALSE;
	m_bG2Hub		= FALSE;
	m_bG1Leaf		= FALSE;
	m_bG1Ultrapeer	= FALSE;
	m_nStableCount	= 0;
	m_tHubG2Promotion = 0;
	ZeroMemory( &m_tPresent, sizeof( m_tPresent ) );
	ZeroMemory( &m_tPriority, sizeof( m_tPriority ) );
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithConnect connection initiation

// Maintain calls CHostCacheHost::ConnectTo, which calls this
// Takes an IP address and port number from the host cache, and connects to it
// Returns a pointer to the new neighbour in the connected list, or null if no connection was made
CNeighbour* CNeighboursWithConnect::ConnectTo(
	const IN_ADDR& pAddress, // IP address from the host cache to connect to, like 67.163.208.23
	WORD       nPort,        // Port number that goes with that IP address, like 6346
	PROTOCOLID nProtocol,    // Protocol name, like PROTOCOL_G1 for Gnutella
	BOOL       bAutomatic,   // True to (do)
	BOOL       bNoUltraPeer) // By default, false to not (do)
{
	// Don't connect to self
	if ( Settings.Connection.IgnoreOwnIP && Network.IsSelfIP( pAddress ) )
		return NULL;

	// Don't connect to blocked addresses
	if ( Security.IsDenied( &pAddress ) )
	{
		// If automatic (do) leave without making a note of the error
		if ( bAutomatic ) return NULL;

		// Report that this address is on the block list, and return no new connection made
		theApp.Message( MSG_ERROR, IDS_SECURITY_OUTGOING, (LPCTSTR)CString( inet_ntoa( pAddress ) ) );
		return NULL;
	}

	// If automatic (do) and the network object knows this IP address is firewalled and can't receive connections, give up
	if ( bAutomatic && Network.IsFirewalledAddress( &pAddress, TRUE ) )
		return NULL;
	
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 100 ) )
		return NULL;

	// If the list of connected computers already has this IP address
	if ( Get( pAddress ) )
	{
		// If automatic (do) leave without making a note of the error
		if ( bAutomatic ) return NULL;

		// Report that we're already connected to that computer, and return no new connection made
		theApp.Message( MSG_ERROR, IDS_CONNECTION_ALREADY_ABORT, (LPCTSTR)CString( inet_ntoa( pAddress ) ) );
		return NULL;
	}

	// If the caller wants automatic behavior, then make this connection request also connect the network it is for
	if ( ! bAutomatic )
	{
		switch ( nProtocol )
		{
		case PROTOCOL_G1:
			Settings.Gnutella1.EnableToday = true;
			break;

		case PROTOCOL_G2:
			Settings.Gnutella2.EnableToday = true;
			break;

		case PROTOCOL_ED2K:
			Settings.eDonkey.EnableToday = true;
			CloseDonkeys();
			break;

		case PROTOCOL_BT:
			Settings.BitTorrent.EnableToday = true;
			break;

		case PROTOCOL_KAD:
			Settings.eDonkey.EnableToday = true;
			break;

		case PROTOCOL_DC:
			Settings.DC.EnableToday = true;
			break;

		default:
			;
		}
	}

	// Run network connect (do), and leave if it reports an error
	if ( ! Network.Connect() )
		return NULL;

	// The computer at the IP address we have is running eDonkey2000 software
	switch ( nProtocol )
	{
	case PROTOCOL_ED2K:
		{
			auto_ptr< CEDNeighbour > pNeighbour( new CEDNeighbour() );
			if ( pNeighbour->ConnectTo( &pAddress, nPort, bAutomatic ) )
			{
				return pNeighbour.release();
			}
		}
		break;

	case PROTOCOL_BT:
		{
			DHT.Ping( &pAddress, nPort );
		}
		break;

	case PROTOCOL_KAD:
		{
			SOCKADDR_IN pHost = { AF_INET, htons( nPort ), pAddress };
			Kademlia.Bootstrap( &pHost );
		}
		break;

	case PROTOCOL_DC:
		{
			auto_ptr< CDCNeighbour > pNeighbour( new CDCNeighbour() );
			if ( pNeighbour->ConnectTo( &pAddress, nPort, bAutomatic ) )
			{
				return pNeighbour.release();
			}
		}
		break;

	default:
		{
			auto_ptr< CShakeNeighbour > pNeighbour( new CShakeNeighbour() );
			if ( pNeighbour->ConnectTo( &pAddress, nPort, bAutomatic, bNoUltraPeer ) )
			{
				if ( Settings.Gnutella.SpecifyProtocol )
				{
					pNeighbour->m_nProtocol = nProtocol;
				}
				return pNeighbour.release();
			}
		}
	}

	return NULL; // Not able to connect
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithConnect accept a connection

// CHandshake::OnRead gets an incoming socket connection, looks at the first 7 bytes, and passes Gnutella and Gnutella2 here
// Takes a pointer to the CHandshake object the program made when it accepted the new connection from the listening socket
// Makes a new CShakeNeighbour object, and calls AttachTo to have it take this incoming connection
// Returns a pointer to the CShakeNeighbour object
BOOL CNeighboursWithConnect::OnAccept(CConnection* pConnection)
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 100 ) )
		// Try later
		return TRUE;

	if ( Neighbours.Get( pConnection->m_pHost.sin_addr ) )
	{
		pConnection->Write( _P("GNUTELLA/0.6 503 Duplicate connection\r\n\r\n") );
		pConnection->LogOutgoing();
		pConnection->DelayClose( IDS_CONNECTION_ALREADY_REFUSE );
		return TRUE;
	}

	if ( CShakeNeighbour* pNeighbour = new CShakeNeighbour() )
	{
		pNeighbour->AttachTo( pConnection );
		return FALSE;
	}

	// Out of memory
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithConnect

// If we've been demoted to the leaf role for a protocol, this function trims peers after we get a hub (do)
// Takes a protocol like PROTOCOL_G1 or PROTOCOL_G2
// If we don't need any more hub connections, closes them all (do)
void CNeighboursWithConnect::PeerPrune(PROTOCOLID nProtocol)
{
	// True if we need more hub connections for the requested protocol
	BOOL bNeedMore = NeedMoreHubs( nProtocol );

	// True if we need more hub connections for either Gnutella or Gnutella2
	BOOL bNeedMoreAnyProtocol = NeedMoreHubs( PROTOCOL_NULL );

	// Loop through all the neighbours in the list
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		// Get the neighbour at this position in the list, and move to the next one
		CNeighbour* pNeighbour = GetNext( pos );

		// This neighbour is on the network the caller wants us to prune, and
		if ( pNeighbour->m_nProtocol == nProtocol )
		{
			// Our connection to this neighbour is not up to a hub, and
			if ( pNeighbour->m_nNodeType != ntHub )
			{
				// Either we don't need any more hubs, or we're done with the handshake so we know it wont' be a hub, then
				if ( ! bNeedMore || pNeighbour->m_nState == nrsConnected )
				{
					// Drop this connection
					pNeighbour->Close( IDS_CONNECTION_PEERPRUNE );
				}
			}

		} // This must be a Gnutella or Gnutella2 computer in the middle of the handshake
		else if ( pNeighbour->m_nProtocol == PROTOCOL_NULL )
		{
			// If we initiated the connection, we know it's not a leaf trying to contact us, it's probably a hub
			if ( pNeighbour->m_bInitiated )
			{
				// If we don't need any more hubs, on any protocol, drop this connection
				if ( !bNeedMoreAnyProtocol ) pNeighbour->Close( IDS_CONNECTION_PEERPRUNE );
			}
		}
	}
}

// Determines if we are a leaf on the Gnutella2 network right now
// Returns true or false
bool CNeighboursWithConnect::IsG2Leaf() const
{
	// If the network is enabled (do) and we have at least 1 connection up to a hub, then we're a leaf
	return ( Network.IsConnected() && ( Settings.Gnutella2.ClientMode == MODE_LEAF || m_bG2Leaf ) );
}

// Determines if we are a hub on the Gnutella2 network right now
// Returns true or false
bool CNeighboursWithConnect::IsG2Hub() const
{
	// If the network is enabled (do) and we have at least 1 connection down to a leaf, then we're a hub
	return ( Network.IsConnected() && ( Settings.Gnutella2.ClientMode == MODE_HUB || m_bG2Hub ) );
}

// Takes true if we are running the program in debug mode, and this method should write out debug information
// Determines if the computer and Internet connection here are strong enough for this program to run as a Gnutella2 hub
// Returns false, which is 0, if we can't be a hub, or a number 1+ that is higher the better hub we'd be
DWORD CNeighboursWithConnect::IsG2HubCapable(BOOL bIgnoreTime, BOOL bDebug) const
{
	// Start the rating at 0, which means we can't be a hub
	DWORD nRating = 0; // We'll make this number bigger if we find signs we can be a hub

	// If the caller wants us to report debugging information, start out with a header line
	if ( bDebug ) theApp.Message( MSG_DEBUG, _T("Is %s hub capable?"), protocolNames[ PROTOCOL_G2 ] );

	// We can't be a Gnutella2 hub if the user has not chosen to connect to Gnutella2 in the program settings
	if ( !Network.IsConnected() || !Settings.Gnutella2.EnableToday )
	{
		// Finish the lines of debugging information, and report no, we can't be a hub
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: %s not enabled"), protocolNames[ PROTOCOL_G2 ] );
		return FALSE;
	}

	// The user can go into settings and check a box to make the program never run in hub mode, even if it could
	if ( Settings.Gnutella2.ClientMode == MODE_LEAF ) // If the user disalbled hub mode for the program in settings
	{
		// Finish the lines of debugging information, and report no, we can't be a hub
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: hub mode disabled") );
		return FALSE;
	}

	// We are running as a Gnutella2 leaf right now
	if ( IsG2Leaf() )
	{
		// We can never be a hub because we are a leaf (do)
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: leaf") );
		return FALSE;

	} // We are not running as a Gnutella2 leaf right now (do)
	else
	{
		// Note this and keep going
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("OK: not a leaf") );
	}

	// The user can check a box in settings to let the program become a hub without passing the additional tests below
	if ( Settings.Gnutella2.ClientMode == MODE_HUB )
	{
		// Make a note about this and keep going
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("YES: hub mode forced") );

	} // The user didn't check the force hub box in settings, so the program will have to pass the additional tests below
	else
	{
		MEMORYSTATUSEX pMemory = { sizeof( MEMORYSTATUSEX ) };
		GlobalMemoryStatusEx( &pMemory );

		// See how much physical memory the computer we are running on has
		if ( pMemory.ullTotalPhys < 250 * 1024 * 1024 ) // It's less than 250 MB
		{
			// Not enough memory to be a hub
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: less than 250 MB RAM") );
			return FALSE;

		} // The computer has 250 MB or more RAM
		else
		{
			// Make a note we passed this test, and keep going
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("OK: more than 250 MB RAM") );
		}

		// Check the connection speed, make sure the download speed is fast enough
		if ( Settings.Connection.InSpeed < 200 ) // If the inbound speed is less than 200 (do)
		{
			// Download speed too slow
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: less than 200 Kb/s in") );
			return FALSE;
		}

		// Make sure the upload speed is fast enough
		if ( Settings.Connection.OutSpeed < 200 )
		{
			// Upload speed too slow
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: less than 200 Kb/s out") );
			return FALSE;
		}

		// Confirm how long the node has been running.
		if ( ! bIgnoreTime )
		{
			if ( Settings.Gnutella2.HubVerified )
			{
				// Systems that have been good hubs before can promote in 2 hours
				if ( Network.GetStableTime() < 7200 )
				{
					if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: not stable for 2 hours") );
					return FALSE;
				}
				else
				{
					if ( bDebug ) theApp.Message( MSG_DEBUG, _T("OK: stable for 2 hours") );
				}
			}
			else
			{
				// Untested hubs need 3 hours uptime to be considered
				if ( Network.GetStableTime() < 10800 )
				{
					if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: not stable for 3 hours") );
					return FALSE;
				}
				else
				{
					if ( bDebug ) theApp.Message( MSG_DEBUG, _T("OK: stable for 3 hours") );
				}
			}
		}

		// Make sure the datagram is stable (do)
		if ( Network.IsFirewalled(CHECK_UDP) )
		{
			// Record this is why we can't be a hub, and return no
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: datagram not stable") );
			return FALSE;

		} // The datagram is stable (do)
		else
		{
			// Make a note we passed this test, and keep going
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("OK: datagram stable") );
		}

		// Report that we meet the minimum requirements to be a hub
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("YES: hub capable by test") );
	}

	// If we've made it this far, change the rating number from 0 to 1
	nRating = 1 + CalculateSystemPerformanceScore(bDebug); // The higher it is, the better a hub we can be


	if ( ! Settings.Gnutella1.EnableToday )
	{
		nRating++;
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("%s not enabled"), protocolNames[ PROTOCOL_G1 ] );
	}

	if ( ! Settings.eDonkey.EnableToday ) 
	{
		nRating++;
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("%s not enabled"), protocolNames[ PROTOCOL_ED2K ]);
	}

	if ( ! Settings.BitTorrent.EnableToday )
	{
		nRating++;
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("%s not enabled"), protocolNames[ PROTOCOL_BT ] );
	}

	if ( ! Settings.DC.EnableToday )
	{
		nRating++;
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("%s not enabled"), protocolNames[ PROTOCOL_DC ] );
	}

	if ( bDebug )
	{
		CString strRating;
		strRating.Format( _T("Hub rating: %u"), nRating );
		theApp.Message( MSG_DEBUG, strRating );
	}

	// Return 0 if we can't be a Gnutella2 hub, or 1+ if we can, a higher number means we'd be a better hub
	return nRating;
}

// Determines if we are a leaf on the Gnutella network right now
// Returns true or false
bool CNeighboursWithConnect::IsG1Leaf() const
{
	// If the network is enabled (do) and we have at least 1 connection up to an ultrapeer, then we're a leaf
	return ( Network.IsConnected() && ( Settings.Gnutella1.ClientMode == MODE_LEAF || m_bG1Leaf ) );
}

// Determines if we are an ultrapeer on the Gnutella network right now
// Returns true or false
bool CNeighboursWithConnect::IsG1Ultrapeer() const
{
	// If the network is enabled (do) and we have at least 1 connection down to a leaf, then we're an ultrapeer
	return ( Network.IsConnected() && ( Settings.Gnutella1.ClientMode == MODE_ULTRAPEER || m_bG1Ultrapeer ) );
}

// Takes true if we are running the program in debug mode, and this method should write out debug information
// Determines if the computer and Internet connection here are strong enough for this program to run as a Gnutella ultrapeer
// Returns false, which is 0, if we can't be an ultrapeer, or a number 1+ that is higher the better ultrapeer we'd be
DWORD CNeighboursWithConnect::IsG1UltrapeerCapable(BOOL bIgnoreTime, BOOL bDebug) const
{
	// Start out the rating as 0, meaning we can't be a Gnutella ultrapeer
	DWORD nRating = 0; // If we can be an ultrapeer, we'll set this to 1, and then make it higher if we'd be an even better ultrapeer

	// If the caller requested we write out debugging information, start out by titling that the messages that follow come from this method
	if ( bDebug ) theApp.Message( MSG_DEBUG, _T("Is %s ultrapeer capable?"), protocolNames[ PROTOCOL_G1 ] );

	// We can't be a Gnutella ultrapeer if we're not connected to the Gnutella network
	if ( !Network.IsConnected() || !Settings.Gnutella1.EnableToday )
	{
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: %s not enabled"), protocolNames[ PROTOCOL_G1 ] );
		return FALSE;
	}

	// The user can go into settings and check a box to make the program never become an ultrapeer, even if it could
	if ( Settings.Gnutella1.ClientMode == MODE_LEAF )
	{
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: ultrapeer mode disabled") );
		return FALSE;
	}

	// We are running as a Gnutella leaf right now
	if ( IsG1Leaf() )
	{
		// We can never be an ultrapeer because we are a leaf (do)
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: leaf") );
		return FALSE;

	} // We are running as a Gnutella ultrapeer already (do)
	else
	{
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("OK: not a leaf") );
	}

	// The user can check a box in settings to let the program become an ultrapeer without passing the additional tests below
	if ( Settings.Gnutella1.ClientMode == MODE_ULTRAPEER )
	{
		// Note this and keep going
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("YES: ultrapeer mode forced") );

	} // The user didn't check the force ultrapeer box in settings, so the program will have to pass the additional tests below
	else
	{
		// If we are a Gnutella2 hub, then we shouldn't also be a Gnutella ultrapeer
		if ( IsG2Hub() )
		{
			// ToDo: Check what sort of machine could handle being both a Gnutella ultrapeer and a Gnutella2 hub at the same time

			// Report the reason we can't be an ultrapeer, and return no
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: Acting as a %s hub"), protocolNames[ PROTOCOL_G2 ] );
			return FALSE;

		} // We aren't a Gnutella2 hub right now
		else
		{
			// Make a note we passed this test, and keep going
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("OK: not a %s hub"), protocolNames[ PROTOCOL_G2 ] );
		}

		MEMORYSTATUSEX pMemory = { sizeof( MEMORYSTATUSEX ) };
		GlobalMemoryStatusEx( &pMemory );

		// See how much physical memory the computer we are running on has
		if ( pMemory.ullTotalPhys < 250 * 1024 * 1024 ) // It's less than 250 MB
		{
			// Not enough memory to become a Gnutella ultrapeer
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: less than 250 MB RAM") );
			return FALSE;

		} // This computer has 256 MB of memory or more
		else
		{
			// Make a note we passed this test, and keep going
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("OK: more than 250 MB RAM") );
		}

		// Check the connection speed, make sure the download speed is fast enough
		if ( Settings.Connection.InSpeed < 200 ) // If the inbound speed is less than 200 (do)
		{
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: less than 200 Kb/s in") );
			return FALSE;
		}

		// Make sure the upload speed is fast enough
		if ( Settings.Connection.OutSpeed < 200 )
		{
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: less than 200 Kb/s out") );
			return FALSE;
		}

		// Make sure settings aren't limiting the upload speed too much
		if ( ( Settings.Bandwidth.Uploads <= 4096 ) && ( Settings.Bandwidth.Uploads != 0 ) )
		{
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: Upload limit set too low") );
			return FALSE;
		}

		// We can only become an ultrapeer if we've been connected for 4 hours or more, it takes awhile for ultrapeers to get leaves, so stability is important
		if ( ! bIgnoreTime )
		{
			if ( Network.GetStableTime() < 14400 ) // 14400 seconds is 4 hours
			{
				if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: not stable for 4 hours") );
				return FALSE;

			} // We have been connected to the Gnutella network for more than 4 hours
			else
			{
				if ( bDebug ) theApp.Message( MSG_DEBUG, _T("OK: stable for 4 hours") );
			}
		}

		// Make sure the datagram is stable (do)
		if ( Network.IsFirewalled(CHECK_UDP) )
		{
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: datagram not stable") );
			return FALSE;

		} // The datagram is stable (do)
		else
		{
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("OK: datagram stable") );
		}
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("YES: ultrapeer capable by test") );
	}

	// If we've made it this far, change the rating number from 0 to 1
	// The higher it is, the better an ultrapeer we can be
	nRating = 1 + CalculateSystemPerformanceScore(bDebug); 

	if ( ! Settings.Gnutella2.EnableToday )
	{
		nRating++;
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("%s not enabled"), protocolNames[ PROTOCOL_G2 ] );
	}

	if ( ! Settings.eDonkey.EnableToday )
	{
		nRating++;
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("%s not enabled"), protocolNames[ PROTOCOL_ED2K] );
	}

	if ( ! Settings.BitTorrent.EnableToday )
	{
		nRating++;
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("%s not enabled"), protocolNames[ PROTOCOL_BT ] );
	}

	if ( ! Settings.DC.EnableToday )
	{
		nRating++;
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("%s not enabled"), protocolNames[ PROTOCOL_DC ] );
	}

	// If debug mode is enabled, display the ultrapeer rating in the system window log (do)
	if ( bDebug )
	{
		// Compose text that includes the rating, and show it in the user interface
		CString strRating;
		strRating.Format( _T("Ultrapeer rating: %u"), nRating );
		theApp.Message( MSG_DEBUG, strRating );
	}

	// Return 0 if we can't be a Gnutella ultrapeer, or 1+ if we can, a higher number means we'd be a better ultrapeer
	return nRating;
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithConnect connection capacity

bool CNeighboursWithConnect::NeedMoreHubs(PROTOCOLID nProtocol) const
{
	if ( ! Network.IsConnected() ) return false;

	switch ( nProtocol )
	{
	case PROTOCOL_NULL:
		if ( ! Settings.Gnutella1.EnableToday && ! Settings.Gnutella2.EnableToday ) return false;
		break;

	case PROTOCOL_G1:
		if ( ! Settings.Gnutella1.EnableToday ) return false;
		break;

	case PROTOCOL_G2:
		if ( ! Settings.Gnutella2.EnableToday ) return false;
		break;

	default:
		return false;
	}

	DWORD nConnected[ PROTOCOL_LAST ] = {};
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		const CNeighbour* pNeighbour = GetNext( pos );
		if ( pNeighbour->m_nState == nrsConnected && pNeighbour->m_nNodeType != ntLeaf )
		{
			nConnected[ pNeighbour->m_nProtocol ]++;
		}
	}

	switch ( nProtocol )
	{
	case PROTOCOL_NULL:
		return nConnected[ PROTOCOL_G1 ] < ( IsG1Leaf() ? Settings.Gnutella1.NumHubs : Settings.Gnutella1.NumPeers ) ||
			   nConnected[ PROTOCOL_G2 ] < ( IsG2Leaf() ? Settings.Gnutella2.NumHubs : Settings.Gnutella2.NumPeers );

	case PROTOCOL_G1:
		return nConnected[ PROTOCOL_G1 ] < ( IsG1Leaf() ? Settings.Gnutella1.NumHubs : Settings.Gnutella1.NumPeers );

	case PROTOCOL_G2:
		return nConnected[ PROTOCOL_G2 ] < ( IsG2Leaf() ? Settings.Gnutella2.NumHubs : Settings.Gnutella2.NumPeers );

	default:
		return false;
	}
}

bool CNeighboursWithConnect::NeedMoreLeafs(PROTOCOLID nProtocol) const
{
	if ( ! Network.IsConnected() ) return false;

	switch ( nProtocol )
	{
	case PROTOCOL_NULL:
		if ( ! Settings.Gnutella1.EnableToday && ! Settings.Gnutella2.EnableToday ) return false;
		break;

	case PROTOCOL_G1:
		if ( ! Settings.Gnutella1.EnableToday ) return false;
		break;

	case PROTOCOL_G2:
		if ( ! Settings.Gnutella2.EnableToday ) return false;
		break;

	default:
		return false;
	}

	DWORD nConnected[ PROTOCOL_LAST ] = {};
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		const CNeighbour* pNeighbour = GetNext( pos );

		if ( pNeighbour->m_nState == nrsConnected && pNeighbour->m_nNodeType == ntLeaf )
		{
			nConnected[ pNeighbour->m_nProtocol ]++;
		}
	}

	switch ( nProtocol )
	{
	case PROTOCOL_NULL:
		return nConnected[ PROTOCOL_G1 ] < Settings.Gnutella1.NumLeafs ||
			   nConnected[ PROTOCOL_G2 ] < Settings.Gnutella2.NumLeafs;

	case PROTOCOL_G1:
		return nConnected[ PROTOCOL_G1 ] < Settings.Gnutella1.NumLeafs;

	case PROTOCOL_G2:
		return nConnected[ PROTOCOL_G2 ] < Settings.Gnutella2.NumLeafs;

	default:
		return false;
	}
}

/*bool CNeighboursWithConnect::IsHubLoaded(PROTOCOLID nProtocol) const
{
	if ( ! Network.IsConnected() ) return false;

	switch ( nProtocol )
	{
	case PROTOCOL_NULL:
		if ( ! Settings.Gnutella1.EnableToday && ! Settings.Gnutella2.EnableToday ) return false;
		break;

	case PROTOCOL_G1:
		if ( ! Settings.Gnutella1.EnableToday ) return false;
		break;

	case PROTOCOL_G2:
		if ( ! Settings.Gnutella2.EnableToday ) return false;
		break;

	default:
		return false;
	}

	DWORD nConnected[ PROTOCOL_LAST ] = {};
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		const CNeighbour* pNeighbour = GetNext( pos );
		if ( pNeighbour->m_nState == nrsConnected && pNeighbour->m_nNodeType == ntLeaf )
		{
			nConnected[ pNeighbour->m_nProtocol ]++;
		}
	}

	switch ( nProtocol )
	{
	case PROTOCOL_NULL:
		// If the total connection number is bigger than 75% of the totals from settings, say yes
		return nConnected[ PROTOCOL_G1 ] + nConnected[ PROTOCOL_G2 ] >= ( Settings.Gnutella1.NumLeafs + Settings.Gnutella2.NumLeafs ) * 3 / 4;

	case PROTOCOL_G1:
		// If we've got more than 75% of the leaf number from settings, say yes
		return nConnected[ PROTOCOL_G1 ] > Settings.Gnutella1.NumLeafs * 3 / 4;

	case PROTOCOL_G2:
		// If we've got more than 75% of the leaf number from settings, say yes
		return nConnected[ PROTOCOL_G2 ] > Settings.Gnutella2.NumLeafs * 3 / 4;

	default:
		return false;
	}
}*/

//////////////////////////////////////////////////////////////////////
// CNeighboursWithConnect run event

// Call DoRun on each neighbour in the list, and maintain the network auto connection
void CNeighboursWithConnect::OnRun()
{
	CNeighboursWithRouting::OnRun();

	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 100 ) )
		return;

	MaintainNodeStatus();

	// Maintain the network (do)
	if ( Network.IsConnected() && Network.m_bAutoConnect )
	{
		// Count how many connections of each type we have, calculate how many we should have, and close and open connections accordingly
		Maintain(); // Makes new connections by getting IP addresses from the host caches for each network
	}
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithConnect maintain connection
	
void CNeighboursWithConnect::MaintainNodeStatus()
{
	BOOL bG2Leaf		= FALSE;
	BOOL bG2Hub			= FALSE;
	BOOL bG1Leaf		= FALSE;
	BOOL bG1Ultrapeer	= FALSE;
	DWORD nStableCount	= 0;
	DWORD tEstablish	= GetTickCount() - 1500;
	DWORD nBandwidthIn	= 0;
	DWORD nBandwidthOut	= 0;

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CNeighbour* pNeighbour = GetNext( pos );

		// We're done with the handshake with this neighbour
		if ( pNeighbour->m_nState == nrsConnected )
		{
			pNeighbour->Measure();
			nBandwidthIn += pNeighbour->m_mInput.nMeasure;
			nBandwidthOut += pNeighbour->m_mOutput.nMeasure;

			if ( pNeighbour->m_tConnected < tEstablish )
				nStableCount++;

			// We're connected to this neighbour and exchanging Gnutella2 packets
			if ( pNeighbour->m_nProtocol == PROTOCOL_G2 )
			{
				// If our connection to this remote computer is up to a hub, we are a leaf, if it's down to a leaf, we are a hub
				if ( pNeighbour->m_nNodeType == ntHub )
					bG2Leaf = TRUE;
				else
					bG2Hub  = TRUE;

			} // We're connected to this neighbours and exchanging Gnutella packets
			else if ( pNeighbour->m_nProtocol == PROTOCOL_G1 )
			{
				// If our connection to this remote computer is up to a hub, we are a leaf, if it's down to a leaf, we are an ultrapeer
				if ( pNeighbour->m_nNodeType == ntHub )
					bG1Leaf      = TRUE;
				else
					bG1Ultrapeer = TRUE;
			}
		}
	}

	m_bG2Leaf		= bG2Leaf;
	m_bG2Hub		= bG2Hub;
	m_bG1Leaf		= bG1Leaf;
	m_bG1Ultrapeer	= bG1Ultrapeer;
	m_nStableCount	= nStableCount;
	m_nBandwidthIn	= nBandwidthIn;
	m_nBandwidthOut	= nBandwidthOut;
}

// As the program runs, CNetwork::OnRun calls this method periodically and repeatedly
// Counts how many connections we have for each network and in each role, and connects to more from the host cache or disconnects from some
void CNeighboursWithConnect::Maintain()
{
	// Get the time
	DWORD tTimer = GetTickCount();							// The time (in ticks)
	DWORD tNow   = static_cast< DWORD >( time( NULL ) );	// The time (in seconds)

	// Don't initiate neighbour connections too quickly if connections are limited
	if ( Settings.Connection.ConnectThrottle && tTimer >= m_tLastConnect && tTimer <= m_tLastConnect + Settings.Connection.ConnectThrottle )
		return;

	DWORD nCount[ PROTOCOL_LAST ][3] = {}, nLimit[ PROTOCOL_LAST ][3] = {};

	// Loop down the list of connected neighbours, sorting each by network and role and counting it
	for ( POSITION pos = GetIterator() ; pos ; ) // We'll also prune leaf to leaf connections, which shouldn't happen
	{
		// Get the next neighbour in the list
		CNeighbour* pNeighbour = GetNext( pos );

		// We're done with the handshake and connected to this remote computer
		if ( pNeighbour->m_nState == nrsConnected )
		{
			// We're both Gnutella2 leaves
			if ( pNeighbour->m_nNodeType != ntHub && m_bG2Leaf && pNeighbour->m_nProtocol == PROTOCOL_G2 )
			{
				// Two leaves shouldn't connect, disconnect
				pNeighbour->Close( IDS_CONNECTION_PEERPRUNE );

			} // We're both Gnutella leaves
			else if ( pNeighbour->m_nNodeType != ntHub && m_bG1Leaf && pNeighbour->m_nProtocol == PROTOCOL_G1 )
			{
				// Two leaves shouldn't connect, disconnect
				pNeighbour->Close( IDS_CONNECTION_PEERPRUNE );

			} // This connection is to a hub above us, or a hub just like us
			else if ( pNeighbour->m_nNodeType != ntLeaf )
			{
				// If we've been connected for more than 8 seconds
				if ( pNeighbour->m_nProtocol == PROTOCOL_ED2K || pNeighbour->m_nProtocol == PROTOCOL_DC || tTimer > pNeighbour->m_tConnected + 8000 )
				{
					// Count one more hub for this connection's protocol
					nCount[ pNeighbour->m_nProtocol ][ ntHub ]++;
				}
				else
					nCount[ pNeighbour->m_nProtocol ][ ntNode ]++;

			} // We must be a hub, and this connection must be down to a leaf
			else
			{
				// Count one more leaf for this connection's protocol
				nCount[ pNeighbour->m_nProtocol ][ ntLeaf ]++;
			}

		} // We're still going through the handshake with this remote computer
		else
		{
			// Count one more connection in the 0 column for this protocol
			nCount[ pNeighbour->m_nProtocol ][ ntNode ]++;
		}
	}

	// Set our "promoted to hub" timer
	if ( m_bG2Hub == FALSE )
		m_tHubG2Promotion = 0;			// If we're not a hub, time promoted is 0
	else if ( m_tHubG2Promotion == 0 )
		m_tHubG2Promotion = tNow;		// If we've just been promoted, set the timer

	// Check if we have verified if we make a good G2 hub
	if ( ! Settings.Gnutella2.HubVerified && m_tHubG2Promotion )
	{
		// If we have been a hub for at least 8 hours
		if ( tNow > m_tHubG2Promotion + 8 * 60 * 60 )
		{
			// And we're loaded ( 75% capacity )
			if ( ( nCount[ PROTOCOL_G2 ][ ntHub ] ) > ( Settings.Gnutella2.NumLeafs * 3 / 4 ) )
			{
				// Then we probably make a pretty good hub
				Settings.Gnutella2.HubVerified = true;
			}
		}
	}

	if ( ! Settings.Gnutella1.EnableToday )
	{
		// Set the limit as no Gnutella hub or leaf connections allowed at all
		nLimit[ PROTOCOL_G1 ][ ntHub ] = nLimit[ PROTOCOL_G1 ][ ntLeaf ] = 0;

	} // We're a leaf on the Gnutella network
	else if ( m_bG1Leaf )
	{
		nLimit[ PROTOCOL_G1 ][ ntHub ] = Settings.Gnutella1.NumHubs;

	} // We're an ultrapeer on the Gnutella network
	else
	{
		// Set the limit for Gnutella ultrapeer connections as whichever number from settings is bigger, peers or hubs
		nLimit[ PROTOCOL_G1 ][ ntHub ] = max( Settings.Gnutella1.NumPeers, Settings.Gnutella1.NumHubs ); // Defaults are 0 and 2

		// Set the limit for Gnutella leaf connections from settings
		nLimit[ PROTOCOL_G1 ][ ntLeaf ] = Settings.Gnutella1.NumLeafs; // 0 by default
	}

	if ( ! Settings.Gnutella2.EnableToday )
	{
		// Set the limit as no Gnutella2 hub or leaf connections allowed at all
		nLimit[ PROTOCOL_G2 ][ ntHub ] = nLimit[ PROTOCOL_G2 ][ ntLeaf ] = 0;
	}
	else if ( m_bG2Leaf )
	{	// We're a leaf on the Gnutella2 network
		// Set the limit for Gnutella2 hub connections as whichever is smaller, the number from settings, or 3
		nLimit[ PROTOCOL_G2 ][ ntHub ] = Settings.Gnutella2.NumHubs; // NumHubs is 2 by default

	}
	else
	{	// We're a hub on the Gnutella2 network
		// Set the limit for Gnutella2 hub connections as whichever number from settings is bigger, peers or hubs
		nLimit[ PROTOCOL_G2 ][ ntHub ] = max( Settings.Gnutella2.NumPeers, Settings.Gnutella2.NumHubs ); // Defaults are 6 and 2

		// Set the limit for Gnutella2 leaf connections from settings
		nLimit[ PROTOCOL_G2 ][ ntLeaf ] = Settings.Gnutella2.NumLeafs; // 1024 by default
	}

	if ( ! Settings.eDonkey.EnableToday )
	{
		nLimit[ PROTOCOL_ED2K ][ ntHub ] =  0;
	}
	else
	{
		// Set the limit for eDonkey2000 hub connections as whichever is smaller, 1, or the number from settings
		nLimit[ PROTOCOL_ED2K ][ ntHub ] = Settings.eDonkey.NumServers; // NumServers is 1 by default
	}

	if ( ! Settings.DC.EnableToday )
	{
		nLimit[ PROTOCOL_DC ][ ntHub ] =  0;
	}
	else
	{
		nLimit[ PROTOCOL_DC ][ ntHub ] = Settings.DC.NumServers; // NumServers is 1 by default
	}

	// Add the count of connections where we don't know the network yet to the 0 column of both Gnutella and Gnutella2
	nCount[ PROTOCOL_G1 ][ ntNode ] += nCount[ PROTOCOL_NULL ][ ntNode ];
	nCount[ PROTOCOL_G2 ][ ntNode ] += nCount[ PROTOCOL_NULL ][ ntNode ];

	// Connect to more computers or disconnect from some to get the connection counts where settings wants them to be
	for ( int nProtocol = PROTOCOL_NULL ; nProtocol < PROTOCOL_LAST ; ++nProtocol ) // Loop once for each protocol, eDonkey2000, Gnutella2, then Gnutella
	{
		// If we're connected to a hub of this protocol, store the tick count now in m_tPresent for this protocol
		if ( nCount[ nProtocol ][ ntHub ] > 0 ) m_tPresent[ nProtocol ] = tNow;

		// If we don't have enough hubs for this protocol
		if ( nCount[ nProtocol ][ ntHub ] < nLimit[ nProtocol ][ ntHub ] )
		{
			// Don't try to connect to G1 right away, wait a few seconds to reduce the number of connections
			if ( nProtocol != PROTOCOL_G2 && Settings.Gnutella2.EnableToday )
			{
				if ( ! Network.ReadyToTransfer( tTimer ) )
					return;
			}

			// We are going to try to connect to a computer running Gnutella or Gnutella2 software
			DWORD nAttempt;
			if ( nProtocol == PROTOCOL_ED2K )
			{
				// For ed2k we try one attempt at a time to begin with, but we can step up to
				// 2 at a time after a few seconds if the FastConnect option is selected.
				if ( Settings.eDonkey.FastConnect && Network.ReadyToTransfer( tTimer ) )
					nAttempt = 2;
				else
					nAttempt = 1;
			}
			else if ( nProtocol == PROTOCOL_DC )
			{
				// DC++ Slow connecting
				nAttempt = 1;
			}
			else
			{
				// For Gnutella and Gnutella2, try connection to the number of free slots multiplied by the connect factor from settings
				nAttempt = ( nLimit[ nProtocol ][ ntHub ] - nCount[ nProtocol ][ ntHub ] );
				nAttempt *=  Settings.Gnutella.ConnectFactor;
			}

			// Lower the needed hub number to avoid hitting Windows XP Service Pack 2's half open connection limit
			nAttempt = min( nAttempt, Settings.Downloads.MaxConnectingSources );

			CHostCacheList* pCache = HostCache.ForProtocol( (PROTOCOLID)nProtocol );

			CSingleLock oLock( &pCache->m_pSection, FALSE );
			if ( ! oLock.Lock( 250 ) )
				return;

			// Handle priority servers
			for ( CHostCacheIterator i = pCache->Begin() ;
				i != pCache->End() && nCount[ nProtocol ][ ntNode ] < nAttempt;
				++i )
			{
				CHostCacheHostPtr pHost = (*i);

				if ( pHost->m_bPriority &&
					pHost->CanConnect( tNow ) &&
					pHost->ConnectTo( TRUE ) )
				{
					m_tPriority[ nProtocol ] = tNow;

					pHost->m_nFailures = 0;
					pHost->m_tFailure = 0;
					pHost->m_bCheckedLocally = TRUE;

					++ nCount[ nProtocol ][ ntNode ];

					// Prevent queries while we connect with this computer (do)
					pHost->m_tQuery = tNow;

					// If settings wants to limit how frequently this method can run
					if ( Settings.Connection.ConnectThrottle )
					{
						m_tLastConnect = tTimer;
						return;
					}
				}
			}

			if ( tNow > m_tPriority[ nProtocol ] + 10 ) // 10 seconds delay between priority and regular servers
			{
				// Handle regular servers
				for ( CHostCacheIterator i = pCache->Begin() ;
					i != pCache->End() && nCount[ nProtocol ][ ntNode ] < nAttempt;
					++i )
				{
					CHostCacheHostPtr pHost = (*i);

					if ( ! pHost->m_bPriority &&
						pHost->CanConnect( tNow ) &&
						pHost->ConnectTo( TRUE ) )
					{
						pHost->m_nFailures = 0;
						pHost->m_tFailure = 0;
						pHost->m_bCheckedLocally = TRUE;

						++ nCount[ nProtocol ][ ntNode ];

						// Prevent queries while we log on (do)
						pHost->m_tQuery = tNow;

						// If settings wants to limit how frequently this method can run
						if ( Settings.Connection.ConnectThrottle )
						{
							m_tLastConnect = tTimer;
							return;
						}
					}
				}
			}

			// If we don't have any handshaking connections for this network, and we've been connected to a hub for more than 30 seconds
			if ( nCount[ nProtocol ][ ntNode ] == 0 || tNow > m_tPresent[ nProtocol ] + 30 )
			{
				DWORD tDiscoveryLastExecute = DiscoveryServices.LastExecute();

				// We're looping for Gnutella2 right now
				if ( nProtocol == PROTOCOL_G2 && Settings.Gnutella2.EnableToday )
				{
					// Execute the discovery services (do)
					if ( pCache->IsEmpty() && tNow >= tDiscoveryLastExecute + 8 )
						DiscoveryServices.Execute( PROTOCOL_G2, 1 );
				} // We're looping for Gnutella right now
				else if ( nProtocol == PROTOCOL_G1 && Settings.Gnutella1.EnableToday )
				{
					// If the Gnutella host cache is empty (do), execute discovery services (do)
					if ( pCache->IsEmpty() && tNow >= tDiscoveryLastExecute + 8 )
						DiscoveryServices.Execute( PROTOCOL_G1, 1 );
				}
			}

		} // We have too many hub connections for this protocol
		else if ( nCount[ nProtocol ][ ntHub ] > nLimit[ nProtocol ][ ntHub ] ) // We're over the limit we just calculated
		{
			// Find the hub we connected to most recently for this protocol
			CNeighbour* pNewest = NULL;
			for ( POSITION pos = GetIterator() ; pos ; )
			{
				// Loop through the list of neighbours
				CNeighbour* pNeighbour = GetNext( pos );

				// If this is a hub connection that connected to us recently
				if (
					// If this connection isn't down to a leaf, and
					( pNeighbour->m_nNodeType != ntLeaf ) &&
					// This connection is for the protocol we're looping on right now, and
					( pNeighbour->m_nProtocol == nProtocol ) &&
					// If the neighbour connected to us
					( pNeighbour->m_bAutomatic              || // The neighbour is automatic, or
					  !pNeighbour->m_bInitiated             || // The neighbour connected to us, or
					  nLimit[ nProtocol ][ ntHub ] == 0 ) )    // We're not supposed to be connected to this network at all
				{
					// If this is the newest hub, remember it.
					if ( pNewest == NULL || pNeighbour->m_tConnected > pNewest->m_tConnected )
						pNewest = pNeighbour;
				}
			}

			// Disconnect from one hub
			if ( pNewest != NULL ) pNewest->Close(); // Close the connection
		}

		// If we're over our leaf connection limit for this network
		if ( nCount[ nProtocol ][ ntLeaf ] > nLimit[ nProtocol ][ ntLeaf ] )
		{
			// Find the leaf we most recently connected to
			CNeighbour* pNewest = NULL;
			for ( POSITION pos = GetIterator() ; pos ; )
			{
				// Loop for each neighbour in the list
				CNeighbour* pNeighbour = GetNext( pos );

				// This connection is down to a leaf and the protocol is correct
				if ( pNeighbour->m_nNodeType == ntLeaf && pNeighbour->m_nProtocol == nProtocol )
				{
					// If we haven't found the newest yet, or this connection is younger than the current newest, this is it
					if ( pNewest == NULL || pNeighbour->m_tConnected > pNewest->m_tConnected ) pNewest = pNeighbour;
				}
			}

			// Disconnect from one leaf
			if ( pNewest != NULL ) pNewest->Close(); // Close the connection
		}
	}
}

DWORD CNeighboursWithConnect::CalculateSystemPerformanceScore(BOOL bDebug) const
{
	DWORD nRating = 0;
	
	MEMORYSTATUSEX pMemory = { sizeof( MEMORYSTATUSEX ) };
	GlobalMemoryStatusEx( &pMemory );

	if ( pMemory.ullTotalPhys >= 1024 * 1024 * 1024 ) // The computer here has more than 1 GB of memory
	{
		nRating++;
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("More than 1 GB RAM") );
	}

	if ( pMemory.ullAvailPhys > 1024 * 1024 * 1024 ) // The computer has more than 1 GB of free memory
	{
		nRating++;
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("More than 1 GB free RAM") );
	}

	// Our Internet connection allows fast downloads
	if ( Settings.Connection.InSpeed > 1000 ) // More than 1 Mbps inbound (do)
	{
		nRating++;
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("More than 1 Mb/s in") );
	}

	// Our Internet connection allows fast uploads
	if ( Settings.Connection.OutSpeed > 1000 ) // More than 1 Mbps outbound (do)
	{
		nRating++;
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("More than 1 Mb/s out") );
	}

	// If the program has been connected (do) for more than 8 hours, give it another point
	if ( Network.GetStableTime() > 28800 ) // 28800 seconds is 8 hours
	{
		nRating++;
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("Stable for 8 hours") );
	}

	// Having more CPUs has significant effect on performance
	if ( System.dwNumberOfProcessors > 1 )
	{
		nRating += System.dwNumberOfProcessors / 2;
		if ( bDebug )
			theApp.Message( MSG_DEBUG, _T("Number of processors: %u"), System.dwNumberOfProcessors );
	}

	// 64-bit
#ifdef _WIN64
	{
		nRating++;
		if ( bDebug )
			theApp.Message( MSG_DEBUG, _T("Mode: 64-bit") );
	}
#endif // _WIN64

	// ToDo: Find out more about the computer to award more rating points
	// CPU: Add a CPU check, award a point if this computer has a fast processor
	// Router: Some routers can't handle the 100+ socket connections an ultrapeer maintains, check for a router and lower the score
	// File transfer: Check how many files are shared and the file transfer in both directions, the perfect ultrapeer isn't sharing or doing any file transfer at all

	return nRating;
}
