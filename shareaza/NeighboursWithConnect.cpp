//
// NeighboursWithConnect.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "Network.h"
#include "Datagrams.h"
#include "Security.h"
#include "HostCache.h"
#include "Downloads.h"
#include "DiscoveryServices.h"
#include "NeighboursWithConnect.h"
#include "ShakeNeighbour.h"
#include "EDNeighbour.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CNeighboursWithConnect construction

CNeighboursWithConnect::CNeighboursWithConnect()
{
	ZeroMemory( m_tPresent, sizeof(m_tPresent) );
}

CNeighboursWithConnect::~CNeighboursWithConnect()
{
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithConnect connection initiation

CNeighbour* CNeighboursWithConnect::ConnectTo(IN_ADDR* pAddress, WORD nPort, PROTOCOLID nProtocol, BOOL bAutomatic, BOOL bNoUltraPeer)
{
	CSingleLock pLock( &Network.m_pSection, TRUE );
	
	if ( Get( pAddress ) )
	{
		if ( bAutomatic ) return NULL;
		theApp.Message( MSG_ERROR, IDS_CONNECTION_ALREADY_ABORT,
			(LPCTSTR)CString( inet_ntoa( *pAddress ) ) );
		return NULL;
	}
	
	if ( Security.IsDenied( pAddress ) )
	{
		if ( bAutomatic ) return NULL;
		theApp.Message( MSG_ERROR, IDS_NETWORK_SECURITY_OUTGOING,
			(LPCTSTR)CString( inet_ntoa( *pAddress ) ) );
		return NULL;
	}
	
	if ( bAutomatic && Network.IsFirewalledAddress( &pAddress, TRUE ) ) return NULL;
	
	if ( ! Network.Connect() ) return NULL;
	
	if ( ! bAutomatic )
	{
		switch ( nProtocol )
		{
		case PROTOCOL_G1:
			Settings.Gnutella1.EnableToday = TRUE;
			break;
		case PROTOCOL_G2:
			Settings.Gnutella2.EnableToday = TRUE;
			break;
		case PROTOCOL_ED2K:
			Settings.eDonkey.EnableToday = TRUE;
			CloseDonkeys();
			break;
		}
	}
	
	if ( nProtocol == PROTOCOL_ED2K )
	{
		CEDNeighbour* pNeighbour = new CEDNeighbour();
		if ( pNeighbour->ConnectTo( pAddress, nPort, bAutomatic ) ) return pNeighbour;
		delete pNeighbour;
	}
	else
	{
		CShakeNeighbour* pNeighbour = new CShakeNeighbour();
		if ( pNeighbour->ConnectTo( pAddress, nPort, bAutomatic, bNoUltraPeer ) ) return pNeighbour;
		delete pNeighbour;
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithConnect accept a connection

CNeighbour* CNeighboursWithConnect::OnAccept(CConnection* pConnection)
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return NULL;
	
	CShakeNeighbour* pNeighbour = new CShakeNeighbour();
	pNeighbour->AttachTo( pConnection );
	
	return pNeighbour;
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithConnect

void CNeighboursWithConnect::PeerPrune()
{
	BOOL bNeedMoreG1 = NeedMoreHubs( FALSE ), bNeedMoreG2 = NeedMoreHubs( TRUE );
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CNeighbour* pNeighbour = GetNext( pos );

		if ( pNeighbour->m_nNodeType != ntHub )
		{
			switch ( pNeighbour->m_nProtocol )
			{
			case PROTOCOL_G1:
				if ( pNeighbour->m_nState == nrsConnected || ! bNeedMoreG1 )
					pNeighbour->Close( IDS_CONNECTION_PEERPRUNE );
				break;
			case PROTOCOL_G2:
				if ( pNeighbour->m_nState == nrsConnected || ! bNeedMoreG2 )
					pNeighbour->Close( IDS_CONNECTION_PEERPRUNE );
				break;
			}
		}
		/*
		if ( pNeighbour->m_nNodeType != ntHub && ( pNeighbour->m_nState == nrsConnected || ! bNeedMore ) )
		{
			if ( pNeighbour->m_nProtocol != PROTOCOL_ED2K )
			{
				pNeighbour->Close( IDS_CONNECTION_PEERPRUNE );
			}
		}
		*/
	}
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithConnect hub states

BOOL CNeighboursWithConnect::IsG2Leaf()
{
	//return Network.m_bEnabled && GetCount( -2, nrsConnected, ntHub ) > 0;
	return Network.m_bEnabled && GetCount( PROTOCOL_G2, nrsConnected, ntHub ) > 0;
}

BOOL CNeighboursWithConnect::IsG2Hub()
{
	//return Network.m_bEnabled && GetCount( -2, nrsConnected, ntLeaf ) > 0;
	return Network.m_bEnabled && GetCount( PROTOCOL_G2, nrsConnected, ntLeaf ) > 0;
}

DWORD CNeighboursWithConnect::IsG2HubCapable(BOOL bDebug)
{	//Determine if this client can be a G2 hub
	
	DWORD nRating = 0; //Zero means this node can not be a hub.
	//Higher numbers indicate it is likely to be a better hub.

	if ( bDebug ) theApp.Message( MSG_DEBUG, _T("IsHubCapable():") );

	// Can't be a G2 hub if you don't connect to G2
	if ( ! Settings.Gnutella2.EnableToday )
	{	
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: G2 not enabled") );
		return FALSE;
	}
	//
	
	//Win95, Win98 and WinMe cannot support enough connections to make a good hub
	if ( ! theApp.m_bNT )
	{	
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: OS is not NT based") );
		return FALSE;
	}
	else
	{
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("OK: OS is NT based") );
	}
	//
	
	//User can disable hub mode G2 settings
	if ( Settings.Gnutella2.ClientMode == MODE_LEAF )
	{	
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: hub mode disabled") );
		return FALSE;
	}
	
	//Check if we are a leaf
	if ( IsG2Leaf() )
	{
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: leaf") );
		return FALSE;
	}
	else
	{
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("OK: not a leaf") );
	}
	//
	
	//Check if hub mode is forced in the G2 settings.
	if ( Settings.Gnutella2.ClientMode == MODE_HUB )
	{	
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("YES: hub mode forced") );
	}
	else
	{
		//Additional checks (if user has *not* forced hub mode)

		//Check amount of memory installed in the machine
		if ( theApp.m_nPhysicalMemory < 250*1024*1024 )
		{	
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: less than 250 MB RAM") );
			return FALSE;
		}
		else
		{
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("OK: more than 250 MB RAM") );
		}
		//
		
		//Check the connection
		if ( Settings.Connection.InSpeed < 200 )
		{
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: less than 200 Kb/s in") );
			return FALSE;
		}	
		if ( Settings.Connection.OutSpeed < 200 )
		{
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: less than 200 Kb/s out") );
			return FALSE;
		}
		//
		
		//Check the G2 hub settings
		if ( Settings.Gnutella2.NumPeers < 4 )
		{
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: less than 4x G2 hub to hub") );
			return FALSE;
		}
		if ( Settings.Gnutella2.NumLeafs < 20 )
		{
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: less than 20x G2 hub to leaf") );
			return FALSE;
		}
		//

		//Confirm how long the node has been running.
		if ( Network.GetStableTime() < 7200 )
		{
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: not stable for 2 hours") );
			return FALSE;
		}
		else
		{
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("OK: stable for 2 hours") );
		}
		if ( ! Datagrams.IsStable() )
		{
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: datagram not stable") );
			return FALSE;
		}
		else
		{
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("OK: datagram stable") );
		}
		//

		//Check the scheduler
		if ( Settings.Scheduler.Enable && ! Settings.Scheduler.AllowHub )
		{	
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: scheduler active") );
			return FALSE;
		}
		else
		{	
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("OK: scheduler OK") );
		}
		//
		
		//This node meets the minimum requirements to be a hub.
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("YES: hub capable by test") );
	}
	nRating = 1;

	//Now, evaluate how good a hub it's likely to be
	//The higher the rating, the better the hub.

	//Check amount of memory installed in the machine
	if ( theApp.m_nPhysicalMemory > 600*1024*1024 )
	{
		nRating++;	// More than half a gig of RAM is good
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("More than 600 MB RAM") );
	}
	//

	//Check connection type
	if ( Settings.Connection.InSpeed > 1000 )
	{
		nRating++;	// More than 1Mb inbound
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("More than 1 Mb/s in") );
	}
	if ( Settings.Connection.OutSpeed > 1000 )
	{
		nRating++;	// More than 1Mb outbound
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("More than 1 Mb/s out") );
	}
	//

	//Check how many other networks are connected
	if ( ! Settings.eDonkey.EnableToday )
	{
		nRating++;	// Not running ed2k improves hub performance
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("eDonkey not enabled") );
	}
	if ( ! Settings.BitTorrent.AdvancedInterface )
	{
		nRating++;	// This user hasn't ever used BitTorrent, so probably won't be using bandwidth for that
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("BT is not in use") );
	}
	if ( ! Settings.Gnutella1.EnableToday )
	{
		nRating++;	// No G1 means more resources for a hub
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("G1 not enabled") );
	}
	//

	//Check how long the node has been up
	if ( Network.GetStableTime() > 28800 )
	{
		nRating++;	// 8 hours uptime is pretty good.
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("Stable for 8 hours") );
	}
	//

	//Check scheduler.
	if ( ! Settings.Scheduler.Enable )
	{
		//ToDo : If this node is scheduled to shut down at any time, it's not going to be a great
		//choice for a hub. If it's going down in the next few hours, don't be a hub at all.
		nRating++;
	}
	//

	//Check CPU.
		//ToDo: Add a CPU check. Faster CPU is better
	
	//Check if behind a router.
		//ToDo: Add a behind router check. (Some routers have problems with the traffic caused)

	//Check how much is shared and upload/download usage.
		//ToDo : Clients not uploading much make better hubs.

	//If debug mode is enabled, display the Hub rating in the log/system window
	if ( bDebug )
	{
		CString strRating;
		strRating.Format( _T("Hub rating: %d"), nRating );
		theApp.Message( MSG_DEBUG, strRating );
	}
	//

	return nRating;
}

BOOL CNeighboursWithConnect::IsG1Leaf()
{
	return Network.m_bEnabled && GetCount( PROTOCOL_G1, nrsConnected, ntHub ) > 0;
}

BOOL CNeighboursWithConnect::IsG1Ultrapeer()
{
	return Network.m_bEnabled && GetCount( PROTOCOL_G1, nrsConnected, ntLeaf ) > 0;
}

DWORD CNeighboursWithConnect::IsG1UltrapeerCapable(BOOL bDebug) 
{	//Check if this node can be a G1 Ultrapeer

	DWORD nRating = 0; //Zero means this node can not be an ultrapeer.
	//Higher numbers indicate it is likely to be a better ultrapeer.

	if ( bDebug ) theApp.Message( MSG_DEBUG, _T("IsUltrapeerCapable():") );

	// Can't be an ultrapeer if you don't connect to Gnutella1
	if ( ! Settings.Gnutella1.EnableToday )
	{	
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: Gnutella1 not enabled") );
		return FALSE;
	}
	//
	
	//Win95, Win98 and WinMe cannot support enough connections to make a good ultrapeer
	if ( ! theApp.m_bNT )
	{	
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: OS is not NT based") );
		return FALSE;
	}
	else
	{
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("OK: OS is NT based") );
	}
	//
	
	//Check if has disabled ultrapeer mode in gnutella settings
	if ( Settings.Gnutella1.ClientMode == MODE_LEAF )
	{	
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: ultrapeer mode disabled") );
		return FALSE;
	}
	//

	//Check if node is a leaf
	if ( IsG1Leaf() )
	{
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: leaf") );
		return FALSE;
	}
	else
	{
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("OK: not a leaf") );
	}
	//

	//Check if node is already a G2 hub (ToDo: Maybe this should not be a requirement)
	if ( IsG2Hub() )
	{
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: Acting as a G2 hub") );
		return FALSE;
	}
	else
	{
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("OK: not a G2 hub") );
	}
	//
	
	//Check if this node has ultrapeer mode forced in the gnutella settings.
	if ( Settings.Gnutella1.ClientMode == MODE_ULTRAPEER )
	{
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("YES: ultrapeer mode forced") );
	}
	else
	{
		//Additional checks (if user has *not* forced ultrapeer mode)

		//Check amount of memory installed in the machine
		if ( theApp.m_nPhysicalMemory < 250*1024*1024 )
		{	
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: less than 250 MB RAM") );
			return FALSE;
		}
		else
		{
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("OK: more than 250 MB RAM") );
		}
		//
		
		//Check the connection
		if ( Settings.Connection.InSpeed < 200 )
		{
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: less than 200 Kb/s in") );
			return FALSE;
		}
		if ( Settings.Connection.OutSpeed < 200 )
		{
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: less than 200 Kb/s out") );
			return FALSE;
		}
		//
		
		//Check the various UP settings. (They should always be higher than this anyway)
		if ( Settings.Gnutella1.NumPeers < 4 )
		{
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: less than 4x G1 peer to peer") );
			return FALSE;
		}
		if ( Settings.Gnutella1.NumLeafs < 5 )
		{
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: less than 5x G1 ultrapeer to leaf") );
			return FALSE;
		}
		//
		
		//Confirm how long the node has been running. (Takes a while for UPs to get leafs- stability is important)
		if ( Network.GetStableTime() < 14400 )
		{
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: not stable for 4 hours") );
			return FALSE;
		}
		else
		{
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("OK: stable for 2 hours") );
		}
		if ( ! Datagrams.IsStable() )
		{
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: datagram not stable") );
			return FALSE;
		}
		else
		{
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("OK: datagram stable") );
		}
		//

		//Check scheduler
		if ( Settings.Scheduler.Enable && ! Settings.Scheduler.AllowHub )
		{	
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("NO: scheduler active") );
			return FALSE;
		}
		else
		{	
			if ( bDebug ) theApp.Message( MSG_DEBUG, _T("OK: scheduler OK") );
		}
		//
			
		//This node meets the minimum requirements to be an ultrapeer.
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("YES: ultrapeer capable by test") );
	}
	nRating = 1;

	//Now, evaluate how good an ultrapeer it's likely to be
	//The higher the rating, the better the UP.

	//Check amount of memory installed in the machine
	if ( theApp.m_nPhysicalMemory > 600*1024*1024 )
	{
		nRating++;	// More than half a gig of RAM is good
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("More than 600 MB RAM") );
	}
	//

	//Check connection type
	if ( Settings.Connection.InSpeed > 1000 )
	{
		nRating++;	// More than 1Mb inbound
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("More than 1 Mb/s in") );
	}
	if ( Settings.Connection.OutSpeed > 1000 )
	{
		nRating++;	// More than 1Mb outbound
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("More than 1 Mb/s out") );
	}
	//

	//Check how many other networks are connected
	if ( ! Settings.eDonkey.EnableToday )
	{
		nRating++;	// Not running ed2k improves ultrapeer performance
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("eDonkey not enabled") );
	}
	if ( ! Settings.BitTorrent.AdvancedInterface )
	{
		nRating++;	// This user hasn't ever used BitTorrent, so probably won't be using bandwidth for that
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("BT is not in use") );
	}
	if ( ! Settings.Gnutella2.EnableToday )
	{
		nRating++;	// No G2 means more resources for an ultrapeer
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("G2 not enabled") );
	}
	//

	//Check how long the node has been up
	if ( Network.GetStableTime() > 28800 )
	{
		nRating++;	// 8 hours uptime is pretty good.
		if ( bDebug ) theApp.Message( MSG_DEBUG, _T("Stable for 8 hours") );
	}
	//

	//Check scheduler.
	if ( ! Settings.Scheduler.Enable )
	{
		//ToDo : If this node is scheduled to shut down at any time, it's not going to be a great
		//choice for an ultrapeer. If it's going down in the next few hours, don't be an UP at all.
		nRating++;
	}
	//

	//Check CPU.
		//ToDo: Add a CPU check. Faster CPU is better
	
	//Check if behind a router.
		//ToDo: Add a behind router check. (Some routers have problems with the traffic caused)
		//Note: This check *may* only be necessary for G2, due to the UDP traffic. Check that.

	//Check how much is shared and upload/download usage.
		//ToDo : Clients not uploading much make better ultrapeers.

	//If debug mode is enabled, display the Ultrapeer rating in the log/system window
	if ( bDebug )
	{
		CString strRating;
		strRating.Format( _T("Ultrapeer rating: %d"), nRating );
		theApp.Message( MSG_DEBUG, strRating );
	}
	//

	return nRating;
}


//////////////////////////////////////////////////////////////////////
// CNeighboursWithConnect connection capacity

BOOL CNeighboursWithConnect::NeedMoreHubs(TRISTATE bG2)
{
	if ( ! Network.IsConnected() ) return FALSE;
	
	int nConnected[4] = { 0, 0, 0, 0 };
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CNeighbour* pNeighbour = GetNext( pos );

		if ( pNeighbour->m_nState == nrsConnected && pNeighbour->m_nNodeType != ntLeaf )
		{
			nConnected[ pNeighbour->m_nProtocol ] ++;
		}
	}
	
	switch ( bG2 )
	{
	case TS_UNKNOWN://Do we need more hubs/UPs on either protocol?
		return ( ( Settings.Gnutella1.EnableToday ) && ( ( nConnected[1] ) < ( IsG1Leaf() ? Settings.Gnutella1.NumHubs : Settings.Gnutella1.NumPeers ) ) ||
				 ( Settings.Gnutella2.EnableToday ) && ( ( nConnected[2] ) < ( IsG2Leaf() ? Settings.Gnutella2.NumHubs : Settings.Gnutella2.NumPeers ) ) );
	case TS_FALSE:	//Do we need more Gnutella 1 Ultrapeers?
		if ( Settings.Gnutella1.EnableToday == FALSE ) return FALSE;
		return ( nConnected[1] ) < ( IsG1Leaf() ? Settings.Gnutella1.NumHubs : Settings.Gnutella1.NumPeers );
	case TS_TRUE:	//Do we need more Gnutella 2 Hubs?
		if ( Settings.Gnutella2.EnableToday == FALSE ) return FALSE;
		return ( nConnected[2] ) < ( IsG2Leaf() ? Settings.Gnutella2.NumHubs : Settings.Gnutella2.NumPeers );
	default:
		return FALSE;
	}
}

BOOL CNeighboursWithConnect::NeedMoreLeafs(TRISTATE bG2)
{
	if ( ! Network.IsConnected() ) return FALSE;
	
	int nConnected[4] = { 0, 0, 0, 0 };
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CNeighbour* pNeighbour = GetNext( pos );
		
		if ( pNeighbour->m_nState == nrsConnected && pNeighbour->m_nNodeType == ntLeaf )
		{
			nConnected[ pNeighbour->m_nProtocol ] ++;
		}
	}
	
	switch ( bG2 )
	{
	case TS_UNKNOWN://Do we need more Leafs of either sort?
		return ( ( ( Settings.Gnutella1.EnableToday ) && ( ( nConnected[1] ) < Settings.Gnutella1.NumLeafs ) ) ||
				 ( ( Settings.Gnutella2.EnableToday ) && ( ( nConnected[2] ) < Settings.Gnutella2.NumLeafs ) ) );
	case TS_FALSE:	//Do we need more Gnutella 1 Leafs?
		if ( Settings.Gnutella1.EnableToday == FALSE ) return FALSE;
		return ( nConnected[1] ) < Settings.Gnutella1.NumLeafs;
	case TS_TRUE:	//Do we need more Gnutella 2 Leafs?
		if ( Settings.Gnutella2.EnableToday == FALSE ) return FALSE;
		return ( nConnected[2] ) < Settings.Gnutella2.NumLeafs;
	default:
		return FALSE;
	}
}

BOOL CNeighboursWithConnect::IsHubLoaded(TRISTATE bG2)
{
	int nConnected[4] = { 0, 0, 0, 0 };
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CNeighbour* pNeighbour = GetNext( pos );
		
		if ( pNeighbour->m_nState == nrsConnected && pNeighbour->m_nNodeType == ntLeaf )
		{
			nConnected[ pNeighbour->m_nProtocol ] ++;
		}
	}
	
	switch ( bG2 )
	{
	case TS_UNKNOWN:
		return ( nConnected[1] + nConnected[2] ) >= ( Settings.Gnutella1.NumLeafs + Settings.Gnutella2.NumLeafs ) * 3 / 4;
	case TS_FALSE:
		if ( Settings.Gnutella1.EnableToday == FALSE ) return FALSE;
		return ( nConnected[1] ) > Settings.Gnutella1.NumLeafs * 3 / 4;
	case TS_TRUE:
		if ( Settings.Gnutella2.EnableToday == FALSE ) return FALSE;
		return ( nConnected[2] ) > Settings.Gnutella2.NumLeafs * 3 / 4;
	default:
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithConnect run event

void CNeighboursWithConnect::OnRun()
{
	CNeighboursWithRouting::OnRun();

	if ( Network.m_pSection.Lock( 50 ) )
	{
		if ( Network.m_bEnabled && Network.m_bAutoConnect )
		{
			Maintain();
		}
		
		Network.m_pSection.Unlock();
	}
}

//////////////////////////////////////////////////////////////////////
// CNeighboursWithConnect maintain connection

void CNeighboursWithConnect::Maintain()
{
	int nCount[4][3], nLimit[4][3];
	DWORD tTimer = GetTickCount();
	DWORD tNow = time( NULL );
	BOOL bIsG1Leaf = IsG1Leaf(), bIsG2Leaf = IsG2Leaf();
	
	//Check connection throttle
	if ( Settings.Connection.ConnectThrottle != 0 )
	{
		if ( tTimer < Network.m_tLastConnect ) return;
		if ( tTimer - Network.m_tLastConnect < Settings.Connection.ConnectThrottle ) return;
	}
	
	//Initialise counters
	ZeroMemory( nCount, sizeof(int) * 4 * 3 );
	ZeroMemory( nLimit, sizeof(int) * 4 * 3 );
	

	//Count (and prune) neighbours
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CNeighbour* pNeighbour = GetNext( pos );
		
		if ( pNeighbour->m_nState == nrsConnected )
		{
			if ( pNeighbour->m_nNodeType != ntHub && bIsG2Leaf && pNeighbour->m_nProtocol == PROTOCOL_G2 )
			{
				pNeighbour->Close( IDS_CONNECTION_PEERPRUNE );
			}
			else if ( pNeighbour->m_nNodeType != ntHub && bIsG1Leaf && pNeighbour->m_nProtocol == PROTOCOL_G1 )
			{
				pNeighbour->Close( IDS_CONNECTION_PEERPRUNE );
			}
			else if ( pNeighbour->m_nNodeType != ntLeaf )
			{
				if ( tNow - pNeighbour->m_tConnected > 8000 )
				{
					nCount[ pNeighbour->m_nProtocol ][ ntHub ] ++;
				}
			}
			else
			{
				nCount[ pNeighbour->m_nProtocol ][ ntLeaf ] ++;
			}
		}
		else if ( pNeighbour->m_nState < nrsConnected )
		{
			nCount[ pNeighbour->m_nProtocol ][ 0 ] ++;
		}
	}
	
	//Set numbers of neighbours
	if ( bIsG1Leaf )
	{
		nLimit[ PROTOCOL_G1 ][ ntHub ]	= min( Settings.Gnutella1.NumHubs, 2 );
	}
	else
	{
		nLimit[ PROTOCOL_G1 ][ ntHub ]	= max( Settings.Gnutella1.NumPeers, Settings.Gnutella1.NumHubs );
		nLimit[ PROTOCOL_G1 ][ ntLeaf ]	= Settings.Gnutella1.NumLeafs;
	}

	if ( bIsG2Leaf )
	{
		nLimit[ PROTOCOL_G2 ][ ntHub ]	= min( Settings.Gnutella2.NumHubs, 3 );
	}
	else
	{
		nLimit[ PROTOCOL_G2 ][ ntHub ]	= max( Settings.Gnutella2.NumPeers, Settings.Gnutella2.NumHubs );
		nLimit[ PROTOCOL_G2 ][ ntLeaf ]	= Settings.Gnutella2.NumLeafs;
	}
	
	if ( Settings.Gnutella2.EnableToday == FALSE )
	{
		nLimit[ PROTOCOL_G2 ][ ntHub ] = nLimit[ PROTOCOL_G2 ][ ntLeaf ] = 0;
	}
	
	if ( Settings.Gnutella1.EnableToday == FALSE )
	{
		nLimit[ PROTOCOL_G1 ][ ntHub ] = nLimit[ PROTOCOL_G1 ][ ntLeaf ] = 0;
	}
	
	if ( Settings.eDonkey.EnableToday )
	{
		nLimit[ PROTOCOL_ED2K ][ ntHub ] = min( 1, Settings.eDonkey.NumServers );
	}
	
	nCount[ PROTOCOL_G1 ][0] += nCount[ PROTOCOL_NULL ][0];
	nCount[ PROTOCOL_G2 ][0] += nCount[ PROTOCOL_NULL ][0];
	
	//Maintain neighbour connections
	for ( int nProtocol = 3 ; nProtocol >= 1 ; nProtocol-- )
	{
		if ( nCount[ nProtocol ][ ntHub ] > 0 ) m_tPresent[ nProtocol ] = tNow;
		
		if ( nCount[ nProtocol ][ ntHub ] < nLimit[ nProtocol ][ ntHub ] )
		{	//We don't have enough hubs

			//If connections are limited (XP sp2), then don't try to connect to G1 until G2 is ok.
			if ( ( nProtocol == PROTOCOL_G1 ) && ( Settings.Gnutella2.EnableToday == TRUE )
											  && ( Settings.Downloads.MaxConnectingSources < 10 ) )
			{
				if ( (nCount[ PROTOCOL_G2 ][ ntHub ] == 0) || ( Network.GetStableTime() < 15 ) )
					return;
			}

			CHostCacheList* pCache = HostCache.ForProtocol( nProtocol );
			
			int nAttempt = ( nLimit[ nProtocol ][ ntHub ] - nCount[ nProtocol ][ ntHub ] );
			nAttempt *= ( nProtocol != PROTOCOL_ED2K ) ? Settings.Gnutella.ConnectFactor : 2;
			//Prevent XP sp2 from maxing out half open connections
			nAttempt = min(nAttempt, ( Settings.Downloads.MaxConnectingSources - 2 ) ); 
			
			//Handle priority ed2k servers
			if ( nProtocol == PROTOCOL_ED2K )
			{
				for (	CHostCacheHost* pHost = pCache->GetNewest() ;
						pHost && nCount[ nProtocol ][0] < nAttempt ; pHost = pHost->m_pPrevTime )
				{
					if ( pHost->m_bPriority && pHost->CanConnect( tNow ) && pHost->ConnectTo( TRUE ) )
					{
						ASSERT( pHost->m_nProtocol == nProtocol );
						nCount[ nProtocol ][0] ++;
						
						if ( Settings.Connection.ConnectThrottle != 0 )
						{
							Network.m_tLastConnect = tTimer;
							Downloads.m_tLastConnect = tTimer;
							return;
						}
					}
				}
			}
			
			//Connect to regular hosts (neighbours)
			for (	CHostCacheHost* pHost = pCache->GetNewest() ;
					pHost && nCount[ nProtocol ][0] < nAttempt ; pHost = pHost->m_pPrevTime )
			{
				if ( pHost->CanConnect( tNow ) && pHost->ConnectTo( TRUE ) )
				{
					ASSERT( pHost->m_nProtocol == nProtocol );
					nCount[ nProtocol ][0] ++;
					
					if ( Settings.Connection.ConnectThrottle != 0 )
					{
						Network.m_tLastConnect = tTimer;
						Downloads.m_tLastConnect = tTimer;
						return;
					}
				}
			}
			
			if ( Network.m_bAutoConnect )
			{
				if ( nCount[ nProtocol ][ 0 ] == 0 || tNow - m_tPresent[ nProtocol ] >= 30 )
				{
					if ( nProtocol == PROTOCOL_G2 )
					{
						DiscoveryServices.Execute( TRUE );
					}
					else if ( nProtocol == PROTOCOL_G1 )
					{
						if ( pCache->GetOldest() == NULL ) DiscoveryServices.Execute( TRUE );
					}
				}
			}
		}
		else if ( nCount[ nProtocol ][ ntHub ] > nLimit[ nProtocol ][ ntHub ] )
		{	//We have too many hubs
			CNeighbour* pNewest = NULL;
			
			//Find the most recently added
			for ( POSITION pos = GetIterator() ; pos ; )
			{
				CNeighbour* pNeighbour = GetNext( pos );
				
				if ( pNeighbour->m_nNodeType != ntLeaf &&
					 pNeighbour->m_nProtocol == nProtocol &&
					 ( pNeighbour->m_bAutomatic || ! pNeighbour->m_bInitiated || nLimit[ nProtocol ][ ntHub ] == 0 ) )
				{
					if ( pNewest == NULL || pNeighbour->m_tConnected > pNewest->m_tConnected )
						pNewest = pNeighbour;
				}
			}

			//Drop it
			if ( pNewest != NULL ) pNewest->Close();
		}
		
		if ( nCount[ nProtocol ][ ntLeaf ] > nLimit[ nProtocol ][ ntLeaf ] )
		{	//We have too many Leafs
			CNeighbour* pNewest = NULL;
			
			//Find the most recently added
			for ( POSITION pos = GetIterator() ; pos ; )
			{
				CNeighbour* pNeighbour = GetNext( pos );
				
				if ( pNeighbour->m_nNodeType == ntLeaf && pNeighbour->m_nProtocol == nProtocol )
				{
					if ( pNewest == NULL || pNeighbour->m_tConnected > pNewest->m_tConnected )
						pNewest = pNeighbour;
				}
			}
			
			//Drop it
			if ( pNewest != NULL ) pNewest->Close();
		}
	}
}
