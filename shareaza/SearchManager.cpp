//
// SearchManager.cpp
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "HostCache.h"
#include "G2Packet.h"
#include "ManagedSearch.h"
#include "Neighbour.h"
#include "Neighbours.h"
#include "Network.h"
#include "QuerySearch.h"
#include "QueryHit.h"
#include "SearchManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CSearchManager SearchManager;


//////////////////////////////////////////////////////////////////////
// CSearchManager construction

CSearchManager::CSearchManager()
	: m_tLastTick( 0 )
	, m_nPriorityClass( 0 )
	, m_nPriorityCount( 0 )
{
}

CSearchManager::~CSearchManager()
{
	ASSERT( m_pList.IsEmpty() );
}

//////////////////////////////////////////////////////////////////////
// CSearchManager add and remove

bool CSearchManager::Add(CManagedSearch* pSearch)
{
	ASSUME_LOCK( m_pSection );

	POSITION pos = m_pList.Find( pSearch );
	if ( pos )
		// Already started
		return false;

	pSearch->ComAddRef( NULL );
	m_pList.AddHead( pSearch );

	return true;
}

bool CSearchManager::Remove(CManagedSearch* pSearch)
{
	ASSUME_LOCK( m_pSection );

	POSITION pos = m_pList.Find( pSearch );
	if ( ! pos )
		// Already stopped
		return false;

	m_pList.RemoveAt( pos );
	pSearch->ComRelease( NULL );

	return true;
}

//////////////////////////////////////////////////////////////////////
// CSearchManager list access

CSearchPtr CSearchManager::Find(const Hashes::Guid& oGUID) const
{
	ASSUME_LOCK( m_pSection );

	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CSearchPtr pManaged = m_pList.GetNext( pos );

		if ( pManaged->IsEqualGUID( oGUID ) )
			return pManaged;
	}

	return CSearchPtr();
}

//////////////////////////////////////////////////////////////////////
// CSearchManager run event (FROM CNetwork THREAD)

void CSearchManager::OnRun()
{
	// Don't run too often to avoid excess CPU use (and router flooding)
	DWORD tNow = GetTickCount();
	if ( ( tNow - m_tLastTick ) < 125 ) return;
	m_tLastTick = tNow;

	// Don't run if we aren't connected
	if ( ! Network.IsWellConnected() ) return;

	CSingleLock pLock( &m_pSection );
	if ( ! pLock.Lock( 100 ) )
		return;

	const int nPriorityFactor[ 3 ] = { 8, 4, 1 };

	if ( m_nPriorityCount >= nPriorityFactor[ m_nPriorityClass ] )
	{
		m_nPriorityCount = 0;
		m_nPriorityClass = ( m_nPriorityClass + 1 ) % CManagedSearch::spMax;
	}

	for ( int nClass = 0 ; nClass <= CManagedSearch::spMax ; nClass++ )
	{
		for ( POSITION pos = m_pList.GetHeadPosition(); pos ; )
		{
			POSITION posCur = pos;
			CManagedSearch* pSearch = m_pList.GetNext( pos );

			if ( pSearch->Execute( m_nPriorityClass ) )
			{
				m_pList.RemoveAt( posCur );
				m_pList.AddTail( pSearch );
				m_nPriorityCount++;
				return;
			}
		}

		m_nPriorityCount = 0;
		m_nPriorityClass = ( m_nPriorityClass + 1 ) % CManagedSearch::spMax;
	}
}

//////////////////////////////////////////////////////////////////////
// CSearchManager query acknowledgment

BOOL CSearchManager::OnQueryAck(CG2Packet* pPacket, const SOCKADDR_IN* pAddress, Hashes::Guid& oGUID)
{
	if ( ! pPacket->m_bCompound )
		AfxThrowUserException();

	DWORD nFromIP = pAddress->sin_addr.S_un.S_addr;
	LONG tAdjust = 0;
	DWORD tNow = static_cast< DWORD >( time( NULL ) );
	DWORD nHubs = 0, nLeaves = 0, nSuggestedHubs = 0;
	DWORD nRetryAfter = 0;
	CArray< DWORD > pDone;

	G2_PACKET nType;
	DWORD nLength;

	while ( pPacket->ReadPacket( nType, nLength ) )
	{
		DWORD nOffset = pPacket->m_nPosition + nLength;

		if ( nType == G2_PACKET_QUERY_DONE && nLength >= 4 )
		{
			DWORD nAddress = pPacket->ReadLongLE();
			pDone.Add( nAddress );

			if ( nLength >= 6 )
			{
				WORD nPort = pPacket->ReadShortBE();

				HostCache.Gnutella2.Add( (IN_ADDR*)&nAddress, nPort, tNow );
			}

			if ( nLength >= 8 )
				nLeaves += pPacket->ReadShortBE();
			nHubs ++;
		}
		else if ( nType == G2_PACKET_QUERY_SEARCH && nLength >= 6 )
		{
			DWORD nAddress	= pPacket->ReadLongLE();
			WORD nPort		= pPacket->ReadShortBE();
			DWORD tSeen		= ( nLength >= 10 ) ?
				(DWORD)( (LONG)pPacket->ReadLongBE() + tAdjust ) : tNow;

			HostCache.Gnutella2.Add( (IN_ADDR*)&nAddress, nPort, min( tNow, tSeen ) );

			nSuggestedHubs ++;
		}
		else if ( nType == G2_PACKET_TIMESTAMP && nLength >= 4 )
		{
			tAdjust = (LONG)tNow - (LONG)pPacket->ReadLongBE();
		}
		else if ( nType == G2_PACKET_RETRY_AFTER && nLength >= 2 )
		{
			if ( nLength >= 4 )
			{
				nRetryAfter = pPacket->ReadLongBE();
			}
			else
			{
				nRetryAfter = pPacket->ReadShortBE();
			}
		}
		else if ( nType == G2_PACKET_FROM_ADDRESS && nLength >= 4 )
		{
			nFromIP = pPacket->ReadLongLE();
		}

		pPacket->m_nPosition = nOffset;
	}

	if ( pPacket->GetRemaining() < 16 )
		AfxThrowUserException();

	pPacket->Read( oGUID );

	theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH,
		_T("Processing query acknowledge from %s (time adjust %+d seconds): %d hubs, %d leaves, %d suggested hubs, retry after %d seconds."),
		(LPCTSTR)CString( inet_ntoa( pAddress->sin_addr ) ), tAdjust,
		nHubs, nLeaves, nSuggestedHubs, nRetryAfter );

	// Update host cache
	if ( nFromIP && nRetryAfter )
	{
		CQuickLock oLock( HostCache.Gnutella2.m_pSection );
		if ( CHostCacheHostPtr pHost = HostCache.Gnutella2.Find( (IN_ADDR*)&nFromIP ) )
		{
			pHost->m_tRetryAfter = tNow + nRetryAfter;
		}
	}

	// Update neighbours
	if ( nFromIP && nRetryAfter )
	{
		CQuickLock oLock( Network.m_pSection );
		if ( CNeighbour* pNeighbour = Neighbours.Get( *(IN_ADDR*)&nFromIP ) )
		{
			pNeighbour->m_tLastQuery = max( pNeighbour->m_tLastQuery, tNow + nRetryAfter );
		}
	}
	
	CSingleLock oLock( &m_pSection );
	if ( ! oLock.Lock( 100 ) )
	{
		theApp.Message( MSG_ERROR | MSG_FACILITY_SEARCH,
			_T("Rejecting query ack operation, search manager overloaded.") );
		return FALSE;
	}

	// Is it our search?
	if ( CSearchPtr pSearch = Find( oGUID ) )
	{
		pSearch->m_nHubs += nHubs;
		pSearch->m_nLeaves += nLeaves;

		// (technically not required, but..)
		pSearch->OnHostAcknowledge( nFromIP );

		for ( int nItem = 0 ; nItem < pDone.GetSize() ; nItem++ )
		{
			DWORD nAddress = pDone.GetAt( nItem );
			pSearch->OnHostAcknowledge( nAddress );
		}

		return FALSE;
	}

	// Route it!
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSearchManager query hits

BOOL CSearchManager::OnQueryHits(const CQueryHit* pHits)
{
	CSingleLock oLock( &m_pSection );
	if ( ! oLock.Lock( 100 ) )
	{
		theApp.Message( MSG_ERROR | MSG_FACILITY_SEARCH,
			_T("Rejecting query hit operation, search manager overloaded.") );
		return FALSE;
	}

	if ( CSearchPtr pSearch = Find( pHits->m_oSearchID ) )
	{
		pSearch->OnHostAcknowledge( *(DWORD*)&pHits->m_pAddress );

		while ( pHits != NULL )
		{
			++pSearch->m_nHits;
			pHits = pHits->m_pNext;
		}

		return FALSE;
	}

	// Route it!
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSearchManager query status request

WORD CSearchManager::OnQueryStatusRequest(const Hashes::Guid& oGUID)
{
	CSingleLock pLock( &m_pSection );
	if ( pLock.Lock( 100 ) )
	{
		if ( CSearchPtr pSearch = Find( oGUID ) )
		{
			return (WORD)min( DWORD(0xFFFE), pSearch->m_nHits );
		}
	}
	return 0xFFFF;
}
