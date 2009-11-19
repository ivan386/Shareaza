//
// SearchManager.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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
#include "Network.h"
#include "SearchManager.h"
#include "ManagedSearch.h"
#include "QuerySearch.h"
#include "QueryHit.h"
#include "HostCache.h"
#include "G2Packet.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CSearchManager SearchManager;


//////////////////////////////////////////////////////////////////////
// CSearchManager construction

CSearchManager::CSearchManager()
{
	m_tLastTick = 0;

	m_nPriorityClass = 0;
	m_nPriorityCount = 0;
}

CSearchManager::~CSearchManager()
{
}

//////////////////////////////////////////////////////////////////////
// CSearchManager add and remove

void CSearchManager::Add(CManagedSearch* pSearch)
{
	POSITION pos = m_pList.Find( pSearch );
	if ( pos == NULL ) m_pList.AddHead( pSearch );
}

void CSearchManager::Remove(CManagedSearch* pSearch)
{
	POSITION pos = m_pList.Find( pSearch );
	if ( pos != NULL ) m_pList.RemoveAt( pos );
}

//////////////////////////////////////////////////////////////////////
// CSearchManager list access

POSITION CSearchManager::GetIterator() const
{
	return m_pList.GetHeadPosition();
}

CManagedSearch* CSearchManager::GetNext(POSITION& pos) const
{
	return m_pList.GetNext( pos );
}

INT_PTR CSearchManager::GetCount() const
{
	return m_pList.GetCount();
}

CManagedSearch* CSearchManager::Find(const Hashes::Guid& oGUID)
{
	for ( POSITION pos = m_pList.GetHeadPosition() ; pos ; )
	{
		CManagedSearch* pManaged = m_pList.GetNext( pos );
		CQuerySearchPtr pSearch = pManaged->GetSearch();
		if ( pSearch && validAndEqual( pSearch->m_oGUID, oGUID ) )
			return pManaged;
	}

	return NULL;
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

	if ( Settings.Gnutella2.EnableToday )
		HostCache.Gnutella2.PruneByQueryAck();

	CSingleLock pLock( &m_pSection );
	if ( ! pLock.Lock( 100 ) ) return;

	static int nPriorityFactor[ 3 ] = { 8, 4, 1 };

	if ( m_nPriorityCount >= nPriorityFactor[ m_nPriorityClass ] )
	{
		m_nPriorityCount = 0;
		m_nPriorityClass = ( m_nPriorityClass + 1 ) % CManagedSearch::spMax;
	}

	for ( int nClass = 0 ; nClass <= CManagedSearch::spMax ; nClass++ )
	{
		for ( POSITION pos = GetIterator() ; pos ; )
		{
			POSITION posCur = pos;
			CManagedSearch* pSearch = GetNext( pos );

			if ( pSearch->m_nPriority == m_nPriorityClass && pSearch->Execute() )
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

BOOL CSearchManager::OnQueryAck(CG2Packet* pPacket, SOCKADDR_IN* pHost, Hashes::Guid& oGUID)
{
	if ( ! pPacket->m_bCompound )
		AfxThrowUserException();

	DWORD nFromIP = pHost->sin_addr.S_un.S_addr;
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
			else if ( nLength >= 2 )
			{
				nRetryAfter = pPacket->ReadShortBE();
			}

			CQuickLock oLock( HostCache.Gnutella2.m_pSection );

			if ( CHostCacheHost* pHost = HostCache.Gnutella2.Find( (IN_ADDR*)&nFromIP ) )
			{
				pHost->m_tRetryAfter = tNow + nRetryAfter;
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
		(LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ), tAdjust,
		nHubs, nLeaves, nSuggestedHubs, nRetryAfter );
	
	CSingleLock pLock( &m_pSection );

	if ( pLock.Lock( 100 ) )
	{
		// Is it our search?
		if ( CManagedSearch* pSearch = Find( oGUID ) )
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
		}
		else
			// Route it!
			return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CSearchManager query hits

BOOL CSearchManager::OnQueryHits(CQueryHit* pHits)
{
	CSingleLock pLock( &m_pSection );
	if ( ! pLock.Lock( 100 ) )
		return TRUE;

	CManagedSearch* pSearch = Find( pHits->m_oSearchID );
	if ( pSearch == NULL )
		return TRUE;

	pSearch->OnHostAcknowledge( *(DWORD*)&pHits->m_pAddress );

	while ( pHits != NULL )
	{
		pSearch->m_nHits ++;
		if ( pHits->m_nProtocol == PROTOCOL_G1 )
			pSearch->m_nG1Hits++;
		else if ( pHits->m_nProtocol == PROTOCOL_G2 )
			pSearch->m_nG2Hits++;
		else if ( pHits->m_nProtocol == PROTOCOL_ED2K )
			pSearch->m_nEDHits++;
		pHits = pHits->m_pNext;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CSearchManager query status request

WORD CSearchManager::OnQueryStatusRequest(const Hashes::Guid& oGUID)
{
	CSingleLock pLock( &m_pSection );
	if ( ! pLock.Lock( 100 ) ) return 0xFFFF;
	
	CManagedSearch* pSearch = Find( oGUID );
	if ( pSearch == NULL ) return 0xFFFF;

	return (WORD)min( DWORD(0xFFFE), pSearch->m_nHits );
}
