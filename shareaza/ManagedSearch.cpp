//
// ManagedSearch.cpp
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
#include "SearchManager.h"
#include "ManagedSearch.h"
#include "QuerySearch.h"
#include "Network.h"
#include "HostCache.h"
#include "Neighbours.h"
#include "Datagrams.h"
#include "G1Neighbour.h"
#include "G2Neighbour.h"
#include "G1Packet.h"
#include "G2Packet.h"
#include "EDPacket.h"
#include "EDNeighbour.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CManagedSearch construction

CManagedSearch::CManagedSearch(CQuerySearch* pSearch, int nPriority)
{
	m_pSearch		= pSearch ? pSearch : new CQuerySearch();
	m_nPriority		= nPriority;
	m_bAllowG2		= TRUE;
	m_bAllowG1		= TRUE;
	m_bAllowED2K	= TRUE;
	
	m_bActive		= FALSE;
	m_bReceive		= TRUE;
	m_tStarted		= 0;
	m_nHubs			= 0;
	m_nLeaves		= 0;
	m_nHits			= 0;
	m_tLastED2K		= 0;
	m_tMoreResults	= 0;
	m_nQueryCount	= 0;

}

CManagedSearch::~CManagedSearch()
{
	Stop();
	if ( m_pSearch ) delete m_pSearch;
}

//////////////////////////////////////////////////////////////////////
// CManagedSearch serialize

void CManagedSearch::Serialize(CArchive& ar)
{
	int nVersion = 3;

	if ( ar.IsStoring() )
	{
		ar << nVersion;

		m_pSearch->Serialize( ar );
		ar << m_nPriority;
		ar << m_bActive;
		ar << m_bReceive;
		ar << m_bAllowG2;
		ar << m_bAllowG1;
		ar << m_bAllowED2K;
	}
	else
	{
		ar >> nVersion;
		if ( nVersion < 2 ) AfxThrowUserException();
		
		if ( m_pSearch ) delete m_pSearch;
		m_pSearch = new CQuerySearch();
		m_pSearch->Serialize( ar );
		
		ar >> m_nPriority;
		ar >> m_bActive;
		ar >> m_bReceive;
		
		m_bActive = m_bReceive = FALSE;
		
		if ( nVersion >= 3 )
		{
			ar >> m_bAllowG2;
			ar >> m_bAllowG1;
			ar >> m_bAllowED2K;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CManagedSearch start and stop

void CManagedSearch::Start()
{
	if ( m_bActive ) return;
	m_bActive = TRUE;
	
	CSingleLock pLock( &SearchManager.m_pSection );
	pLock.Lock( 1000 );
	
	m_tStarted		= time( NULL );
	m_tExecute		= 0;
	m_tLastED2K		= 0;
	m_tMoreResults	= 0;
	m_nQueryCount	= 0;
	
	m_pNodes.RemoveAll();
	
	SearchManager.Add( this );
}

void CManagedSearch::Stop()
{
	if ( m_bActive )
	{
		m_bActive = FALSE;
	    Datagrams.PurgeToken( this );
	}
	
	CSingleLock pLock( &SearchManager.m_pSection );
	pLock.Lock( 1000 );
	SearchManager.Remove( this );
}

//////////////////////////////////////////////////////////////////////
// CManagedSearch execute

BOOL CManagedSearch::Execute()
{
	if ( ! m_bActive || ! m_pSearch ) return FALSE;
	
	DWORD tTicks	= GetTickCount();
	DWORD tSecs		= time( NULL );
	
	if ( m_nPriority == spLowest )
	{
		if ( tTicks - m_tExecute < 30000 ) return FALSE;
		m_tExecute = tTicks;
	}
	else if ( m_nPriority == spMedium )
	{
		if ( tTicks - m_tExecute < 1000 ) return FALSE;
		m_tExecute = tTicks;
	}
	else
	{
		if ( tTicks - m_tExecute < 200 ) return FALSE;
		m_tExecute = tTicks;
	}
	
	BOOL bSuccess = ExecuteNeighbours( tTicks, tSecs );
	
	if ( Settings.Gnutella2.EnableToday && m_bAllowG2 )
	{
		bSuccess |= ExecuteG2Mesh( tTicks, tSecs );
	}
	
	if ( Settings.eDonkey.EnableToday && Settings.eDonkey.ServerWalk && m_bAllowED2K && Network.IsListening() )
	{
		if ( tTicks > m_tLastED2K && tTicks - m_tLastED2K >= Settings.eDonkey.QueryGlobalThrottle )
		{
			bSuccess |= ExecuteDonkeyMesh( tTicks, tSecs );
			m_tLastED2K = tTicks;
		}
	}

	if(bSuccess) m_nQueryCount++;
	
	return bSuccess;
}

//////////////////////////////////////////////////////////////////////
// CManagedSearch execute the search on G1 and G2 neighbours

BOOL CManagedSearch::ExecuteNeighbours(DWORD tTicks, DWORD tSecs)
{
	BOOL bIsOld = ( tSecs - m_tStarted ) >= 5;
	
	int nCount = 0;
	
	for ( POSITION pos = Neighbours.GetIterator() ; pos ; )
	{
		CNeighbour* pNeighbour = Neighbours.GetNext( pos );
		DWORD nPeriod;
		
		// Must be connected
		
		if ( pNeighbour->m_nState != nrsConnected ) continue;
		
		// Check network flags

		switch ( pNeighbour->m_nProtocol )
		{
		case PROTOCOL_G1:
			if ( ! m_bAllowG1 ) continue;
			break;
		case PROTOCOL_G2:
			if ( ! m_bAllowG2 ) continue;
			break;
		case PROTOCOL_ED2K:
			if ( ! m_bAllowED2K ) continue;
			break;
		}
		
		// Must be stable for 15 seconds, or longer for G1 low priority searches
		
		nPeriod = bIsOld ? 15 : 5;
		if ( bIsOld && pNeighbour->m_nProtocol == PROTOCOL_G1 ) nPeriod += 120 + ( 15 * m_nPriority );
		
		if ( tTicks - pNeighbour->m_tConnected < nPeriod * 1000 ) continue;
		
		// Do not hammer neighbours for search results
		
		if ( bIsOld )
		{
			if ( pNeighbour->m_nProtocol == PROTOCOL_G1 )
			{
				if ( tSecs - pNeighbour->m_tLastQuery <
					 Settings.Gnutella1.QueryThrottle ) continue;
			}
			else if ( pNeighbour->m_nProtocol == PROTOCOL_G2 )
			{
				if ( tSecs - pNeighbour->m_tLastQuery <
					 Settings.Gnutella2.QueryHostThrottle / 4 ) continue;
			}
		}
		
		// Lookup the host
		
		DWORD nAddress = pNeighbour->m_pHost.sin_addr.S_un.S_addr;
		
		if ( m_pNodes.Lookup( (LPVOID)nAddress, (LPVOID&)nPeriod ) )
		{
			DWORD nFrequency = 0;
			
			if ( pNeighbour->m_nProtocol == PROTOCOL_G1 )
			{
				nFrequency = Settings.Gnutella1.RequeryDelay;
				nFrequency *= ( m_nPriority + 1 );
			}
			else if ( pNeighbour->m_nProtocol == PROTOCOL_G2 )
			{
				nFrequency = Settings.Gnutella2.RequeryDelay;
				nFrequency *= ( m_nPriority + 1 );
			}
			else if ( pNeighbour->m_nProtocol == PROTOCOL_ED2K )
				nFrequency = 86400;
			
			if ( tSecs - nPeriod < nFrequency ) // If we've queried this neighbour 'recently'
			{
				// Request more ed2k results (if appropriate)
				if ( ( pNeighbour->m_nProtocol == PROTOCOL_ED2K ) && ( pNeighbour->m_pMoreResultsGUID != NULL ) ) // If it's an ed2k server and has more results
				{	
					if ( ( m_pSearch->m_pGUID == *pNeighbour->m_pMoreResultsGUID ) && // and this search is the one with results waiting
						( m_tMoreResults + 10000 < tTicks ) )		// and we've waited a little while (to ensure the search is still active)
					{
						// Request more results
						pNeighbour->Send( CEDPacket::New(  ED2K_C2S_MORERESULTS ) );
						((CEDNeighbour*)pNeighbour)->m_pQueries.AddTail( pNeighbour->m_pMoreResultsGUID );
						// Reset "more results" indicator
						pNeighbour->m_pMoreResultsGUID = NULL;
						// Set timer
						m_tMoreResults = tTicks;
						// Display message in system window
						theApp.Message( MSG_DEBUG, _T("Asking ed2k neighbour for additional search results") );
					}
				}

				// Don't search this neighbour again.
				continue; 
			}
		}
		
		// Set the last query time for this host for this search
		
		m_pNodes.SetAt( (LPVOID)nAddress, (LPVOID)tSecs );
		
		// Create the appropriate packet type
		
		CPacket* pPacket = NULL;
		
		if ( pNeighbour->m_nProtocol == PROTOCOL_G1 )
		{
			pPacket = m_pSearch->ToG1Packet();
		}
		else if ( pNeighbour->m_nProtocol == PROTOCOL_G2 )
		{
			m_pSearch->m_bAndG1 = ( Settings.Gnutella1.EnableToday && m_bAllowG1 );
			pPacket = m_pSearch->ToG2Packet( Datagrams.IsStable() ? &Network.m_pHost : NULL, 0 );
		}
		else if ( pNeighbour->m_nProtocol == PROTOCOL_ED2K )
		{
			pPacket = m_pSearch->ToEDPacket( FALSE, ((CEDNeighbour*)pNeighbour)->m_nFlags );
		}
		else
		{
			ASSERT( FALSE );
		}
		
		// Try to send the search
		
		if ( pPacket != NULL && pNeighbour->SendQuery( m_pSearch, pPacket, TRUE ) )
		{
			// Reset the last "search more" sent to this neighbour (if applicable)
			if ( pNeighbour->m_pMoreResultsGUID != NULL )
			{
				delete pNeighbour->m_pMoreResultsGUID;
				pNeighbour->m_pMoreResultsGUID = NULL;
			}
			m_tMoreResults = 0;

			//Display message in system window
			theApp.Message( MSG_DEFAULT, IDS_NETWORK_SEARCH_SENT,
				m_pSearch->m_sSearch.GetLength() ? (LPCTSTR)m_pSearch->m_sSearch : _T("URN"),
				(LPCTSTR)CString( inet_ntoa( pNeighbour->m_pHost.sin_addr ) ) );
		}
		pPacket->Release();
		
		nCount++;
	}
	
	return ( nCount > 0 );
}

//////////////////////////////////////////////////////////////////////
// CManagedSearch execute the search on the G2 mesh

BOOL CManagedSearch::ExecuteG2Mesh(DWORD tTicks, DWORD tSecs)
{
	// Look at all known Gnutella2 hubs, newest first
	
	for ( CHostCacheHost* pHost = HostCache.Gnutella2.GetNewest() ; pHost ; pHost = pHost->m_pPrevTime )
	{
		// Must be Gnutella2
		
		ASSERT( pHost->m_nProtocol == PROTOCOL_G2 );
		if ( pHost->m_nProtocol != PROTOCOL_G2 ) continue;
		
		// If this host is a neighbour, don't UDP to it
		
		if ( NULL != Neighbours.Get( &pHost->m_pAddress ) ) continue;
		
		// If this host can't be queried now, don't query it
		
		if ( ! pHost->CanQuery( tSecs ) ) continue;
		
		// Check if we have an appropriate query key for this host, and if so,
		// record the receiver address
		
		SOCKADDR_IN* pReceiver = NULL;
		
		if ( pHost->m_nKeyValue == 0 )
		{
			// Well, we already know we don't have a key.. pretty simple
		}
		else if ( Datagrams.IsStable() )
		{
			// If we are "stable", we have to TX/RX our own UDP traffic,
			// so we must have a query key for the local addess
			
			if ( pHost->m_nKeyHost == Network.m_pHost.sin_addr.S_un.S_addr )
				pReceiver = &Network.m_pHost;
			else
				pHost->m_nKeyValue = 0;
		}
		else
		{
			// Make sure we have a query key via one of our neighbours,
			// and ensure we have queried this neighbour
			
			if ( CNeighbour* pNeighbour = Neighbours.Get( (IN_ADDR*)&pHost->m_nKeyHost ) )
			{
				if ( m_pNodes.Lookup( (LPVOID)pHost->m_nKeyHost, (LPVOID&)pReceiver ) )
					pReceiver = &pNeighbour->m_pHost;
				else
					continue;
			}
			else
			{
				pHost->m_nKeyValue = 0;
			}
		}
		
		// Now, if we still have a query key, send the query
		
		if ( pHost->m_nKeyValue != 0 )
		{
			DWORD tLastQuery, nAddress = pHost->m_pAddress.S_un.S_addr;
			ASSERT( pReceiver != NULL );
			
			// Lookup the host
			
			if ( m_pNodes.Lookup( (LPVOID)nAddress, (LPVOID&)tLastQuery ) )
			{
				// Check per-hub requery time
				DWORD nFrequency = Settings.Gnutella2.RequeryDelay;
				nFrequency *= ( m_nPriority + 1 );
				if ( tSecs - tLastQuery < nFrequency ) continue;
			}
			
			// Set the last query time for this host for this search
			
			m_pNodes.SetAt( (LPVOID)nAddress, (LPVOID)tSecs );
			
			// Record the query time on the host, for all searches
			
			pHost->m_tQuery = tSecs;
			if ( pHost->m_tAck == 0 ) pHost->m_tAck = tSecs;
			
			// Try to create a packet
			
			m_pSearch->m_bAndG1 = ( Settings.Gnutella1.EnableToday && m_bAllowG1 );
			CPacket* pPacket = m_pSearch->ToG2Packet( pReceiver, pHost->m_nKeyValue );
			
			// Send the packet if it was created
			
			if ( pPacket != NULL )
			{
				Datagrams.Send( &pHost->m_pAddress, pHost->m_nPort, pPacket, TRUE, this, TRUE );
				
				theApp.Message( MSG_DEBUG, _T("Querying %s"),
					(LPCTSTR)CString( inet_ntoa( pHost->m_pAddress ) ) );
				
				return TRUE;
			}
		}
		else if ( tSecs - pHost->m_tKeyTime >= max( Settings.Gnutella2.QueryHostThrottle * 5, 5*60 ) )
		{
			// Timing wise, we can request a query key now -- but first we must figure
			// out who should be the receiver
			
			CNeighbour* pCacheHub = NULL;
			pReceiver = NULL;
			
			if ( Datagrams.IsStable() )
			{
				// If we are stable, we must be the receiver
				pReceiver = &Network.m_pHost;
			}
			else
			{
				// Otherwise, we need to find a neighbour G2 hub who has acked
				// this query already
				
				for ( POSITION pos = Neighbours.GetIterator() ; pos ; pCacheHub = NULL )
				{
					pCacheHub = Neighbours.GetNext( pos );
					LPVOID pTemp;
					
					if ( m_pNodes.Lookup( (LPVOID)pCacheHub->m_pHost.sin_addr.S_un.S_addr, pTemp ) )
					{
						if ( pCacheHub->m_nProtocol == PROTOCOL_G2 &&
							 pCacheHub->m_nNodeType == ntHub )
						{
							pReceiver = &pCacheHub->m_pHost;
							if ( ! ((CG2Neighbour*)pCacheHub)->m_bCachedKeys ) pCacheHub = NULL;
							break;
						}
					}
				}
			}
			
			// If we found a receiver, we can ask for the query key
			
			if ( pCacheHub != NULL )
			{
				// The receiver is a cache-capable hub, so we ask it to return
				// a cached key, or fetch a fresh one
				
				CG2Packet* pPacket = CG2Packet::New( G2_PACKET_QUERY_KEY_REQ, TRUE );
				pPacket->WritePacket( "QNA", 6 );
				pPacket->WriteLongLE( pHost->m_pAddress.S_un.S_addr );
				pPacket->WriteShortBE( pHost->m_nPort );
				pCacheHub->Send( pPacket );
				
				// Report
				
				CString strReceiver = CString( inet_ntoa( pReceiver->sin_addr ) );
				theApp.Message( MSG_DEBUG, _T("Requesting query key from %s through %s"),
					(LPCTSTR)CString( inet_ntoa( pHost->m_pAddress ) ), (LPCTSTR)strReceiver );
				
				if ( pHost->m_tAck == 0 ) pHost->m_tAck = tSecs;
				pHost->m_tKeyTime = tSecs;
				pHost->m_nKeyValue = 0;
				
				return TRUE;
			}
			else if ( pReceiver != NULL )
			{
				// We need to transmit directly to the remote query host
				
				CG2Packet* pPacket = CG2Packet::New( G2_PACKET_QUERY_KEY_REQ, TRUE );
				
				if ( pReceiver == &Network.m_pHost )
				{
					theApp.Message( MSG_DEBUG, _T("Requesting query key from %s"),
						(LPCTSTR)CString( inet_ntoa( pHost->m_pAddress ) ) );
				}
				else
				{
					// We are not the receiver, so include receiver address
					pPacket->WritePacket( "RNA", 6 );
					pPacket->WriteLongLE( pReceiver->sin_addr.S_un.S_addr );
					pPacket->WriteShortBE( ntohs( pReceiver->sin_port ) );
					
					CString strReceiver = CString( inet_ntoa( pReceiver->sin_addr ) );
					theApp.Message( MSG_DEBUG, _T("Requesting query key from %s for %s"),
						(LPCTSTR)CString( inet_ntoa( pHost->m_pAddress ) ), (LPCTSTR)strReceiver );
				}
				
				// Send
				
				Datagrams.Send( &pHost->m_pAddress, pHost->m_nPort, pPacket, TRUE, NULL, FALSE );
				
				if ( pHost->m_tAck == 0 ) pHost->m_tAck = tSecs;
				pHost->m_tKeyTime = tSecs;
				pHost->m_nKeyValue = 0;
				
				return TRUE;
			}
		}
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CManagedSearch execute the search on eDonkey2000 servers

BOOL CManagedSearch::ExecuteDonkeyMesh(DWORD tTicks, DWORD tSecs)
{
	for ( CHostCacheHost* pHost = HostCache.eDonkey.GetNewest() ; pHost ; pHost = pHost->m_pPrevTime )
	{
		ASSERT( pHost->m_nProtocol == PROTOCOL_ED2K );
		
		// If this host is a neighbour, don't UDP to it
		
		if ( Neighbours.Get( &pHost->m_pAddress ) ) continue;
		
		// Make sure this host can be queried (now)
		
		if ( pHost->CanQuery( tSecs ) )
		{
			DWORD nAddress = pHost->m_pAddress.S_un.S_addr;
			DWORD tLastQuery;
			
			// Never requery eDonkey2000 servers
			
			if ( m_pNodes.Lookup( (LPVOID)nAddress, (LPVOID&)tLastQuery ) ) continue;
			
			// Set the last query time for this host for this search
			
			m_pNodes.SetAt( (LPVOID)nAddress, (LPVOID)tSecs );
			
			// Record the query time on the host, for all searches
			
			pHost->m_tQuery = tSecs;
			if ( pHost->m_tAck == 0 ) pHost->m_tAck = tSecs;
			
			// Create a packet in the appropriate format
			
			CPacket* pPacket = NULL;
			
			if ( pHost->m_nProtocol == PROTOCOL_ED2K )
			{
				pPacket = m_pSearch->ToEDPacket( TRUE, 0 );
			}
			else
			{
				ASSERT( FALSE );
			}
			
			// Send the datagram if possible
			
			if ( pPacket != NULL ) 
			{
				Datagrams.Send( &pHost->m_pAddress, pHost->m_nPort + 4, pPacket, TRUE );
				
				theApp.Message( MSG_DEBUG, _T("Sending query to %s"),
					(LPCTSTR)CString( inet_ntoa( pHost->m_pAddress ) ) );
				
				return TRUE;
			}
		}
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CManagedSearch host acknowledgement

void CManagedSearch::OnHostAcknowledge(DWORD nAddress)
{
	DWORD tSecs = time( NULL );
	m_pNodes.SetAt( (LPVOID)nAddress, (LPVOID)tSecs );
}
