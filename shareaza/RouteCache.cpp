//
// RouteCache.cpp
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
#include "RouteCache.h"
#include "Packet.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define HASH_SIZE			1024
#define HASH_MASK			0x3FF

#define MIN_BUFFER_SIZE		1024
#define MAX_BUFFER_SIZE		40960
#define BUFFER_BLOCK_SIZE	1024


//////////////////////////////////////////////////////////////////////
// CRouteCache construction

CRouteCache::CRouteCache()
{
	m_nSeconds	= 60 * 20;
	m_pRecent	= &m_pTable[0];
	m_pHistory	= &m_pTable[1];
}

CRouteCache::~CRouteCache()
{
}

//////////////////////////////////////////////////////////////////////
// CRouteCache operations

void CRouteCache::SetDuration(DWORD nSeconds)
{
	m_nSeconds = nSeconds;
	Clear();
}

BOOL CRouteCache::Add(const GGUID* pGUID, const CNeighbour* pNeighbour)
{
	if ( CRouteCacheItem* pItem = Lookup( pGUID ) )
	{
		pItem->m_pNeighbour = pNeighbour;
		return FALSE;
	}
	
	if ( m_pRecent->IsFull() )
	{
		CRouteCacheTable* pTemp = m_pRecent;
		m_pRecent = m_pHistory;
		m_pHistory = pTemp;
		
		m_pRecent->Resize( m_pHistory->GetNextSize( m_nSeconds ) );
	}
	
	m_pRecent->Add( pGUID, pNeighbour, NULL );
	
	return TRUE;
}

BOOL CRouteCache::Add(const GGUID* pGUID, const SOCKADDR_IN* pEndpoint)
{
	if ( CRouteCacheItem* pItem = Lookup( pGUID ) )
	{
		pItem->m_pNeighbour	= NULL;
		pItem->m_pEndpoint	= *pEndpoint;
		return FALSE;
	}
	
	if ( m_pRecent->IsFull() )
	{
		CRouteCacheTable* pTemp = m_pRecent;
		m_pRecent = m_pHistory;
		m_pHistory = pTemp;
		
		m_pRecent->Resize( m_pHistory->GetNextSize( m_nSeconds ) );
	}
	
	m_pRecent->Add( pGUID, NULL, pEndpoint );
	
	return TRUE;
}

CRouteCacheItem* CRouteCache::Add(const GGUID* pGUID, const CNeighbour* pNeighbour, const SOCKADDR_IN* pEndpoint, DWORD tAdded)
{
	GGUID cGUID = *pGUID;
	SOCKADDR_IN cEndpoint;
	if ( pEndpoint != NULL ) cEndpoint = *pEndpoint;

	if ( m_pRecent->IsFull() )
	{
		CRouteCacheTable* pTemp = m_pRecent;
		m_pRecent = m_pHistory;
		m_pHistory = pTemp;

		m_pRecent->Resize( m_pHistory->GetNextSize( m_nSeconds ) );
	}

	return m_pRecent->Add( &cGUID, pNeighbour, pEndpoint != NULL ? &cEndpoint : NULL, tAdded );
}

CRouteCacheItem* CRouteCache::Lookup(const GGUID* pGUID, CNeighbour** ppNeighbour, SOCKADDR_IN* pEndpoint)
{
	CRouteCacheItem* pItem = m_pRecent->Find( pGUID );

	if ( pItem == NULL )
	{
		pItem = m_pHistory->Find( pGUID );

		if ( pItem == NULL )
		{
			if ( ppNeighbour ) *ppNeighbour = NULL;
			if ( pEndpoint ) ZeroMemory( pEndpoint, sizeof(*pEndpoint) );

			return NULL;
		}

		pItem = Add( &pItem->m_pGUID, pItem->m_pNeighbour,
			pItem->m_pNeighbour ? NULL : &pItem->m_pEndpoint, pItem->m_tAdded );
	}

	if ( ppNeighbour ) *ppNeighbour = (CNeighbour*)pItem->m_pNeighbour;
	if ( pEndpoint ) *pEndpoint = pItem->m_pEndpoint;

	return pItem;
}

void CRouteCache::Remove(CNeighbour* pNeighbour)
{
	m_pTable[0].Remove( pNeighbour );
	m_pTable[1].Remove( pNeighbour );
}

void CRouteCache::Clear()
{
	m_pTable[0].Clear();
	m_pTable[1].Clear();
}


//////////////////////////////////////////////////////////////////////
// CRouteCacheTable construction

CRouteCacheTable::CRouteCacheTable()
{
	m_pBuffer	= NULL;
	m_nBuffer	= 0;
	m_nUsed		= 0;
	Clear();
}

CRouteCacheTable::~CRouteCacheTable()
{
	if ( m_pBuffer && m_nBuffer ) delete [] m_pBuffer;
}

//////////////////////////////////////////////////////////////////////
// CRouteCacheTable operations

CRouteCacheItem* CRouteCacheTable::Find(const GGUID* pGUID)
{
	WORD nGUID = 0, *ppGUID = (WORD*)pGUID;
	for ( int nIt = 8 ; nIt ; nIt-- ) nGUID += *ppGUID++;
	
	CRouteCacheItem* pItem = *( m_pHash + ( nGUID & HASH_MASK ) );
	
	for ( ; pItem ; pItem = pItem->m_pNext )
	{
		if ( *pGUID == pItem->m_pGUID ) return pItem;
	}
	
	return NULL;
}

CRouteCacheItem* CRouteCacheTable::Add(const GGUID* pGUID, const CNeighbour* pNeighbour, const SOCKADDR_IN* pEndpoint, DWORD nTime)
{
	if ( m_nUsed == m_nBuffer || ! m_pFree ) return NULL;

	WORD nGUID = 0, *ppGUID = (WORD*)pGUID;
	for ( int nIt = 8 ; nIt ; nIt-- ) nGUID += *ppGUID++;
	
	CRouteCacheItem** pHash = m_pHash + ( nGUID & HASH_MASK );

	CRouteCacheItem* pItem = m_pFree;
	m_pFree = m_pFree->m_pNext;

	pItem->m_pNext = *pHash;
	*pHash = pItem;

	pItem->m_pGUID			= *pGUID;
	pItem->m_tAdded			= nTime ? nTime : GetTickCount();
	pItem->m_pNeighbour		= pNeighbour;
	if ( pEndpoint ) pItem->m_pEndpoint = *pEndpoint;
	
	if ( ! m_nUsed++ ) 
	{
		m_tFirst = GetTickCount();
	}
	else if ( m_nUsed == m_nBuffer )
	{
		m_tLast = GetTickCount();
	}
	
	return pItem;
}

void CRouteCacheTable::Remove(CNeighbour* pNeighbour)
{
	CRouteCacheItem** pHash = m_pHash;

	for ( int nHash = 0 ; nHash < HASH_SIZE ; nHash++, pHash++ )
	{
		CRouteCacheItem** pLast = pHash;

		for ( CRouteCacheItem* pItem = *pLast ; pItem ; )
		{
			CRouteCacheItem* pNext = pItem->m_pNext;

			if ( pItem->m_pNeighbour == pNeighbour )
			{
				*pLast = pNext;
				pItem->m_pNext = m_pFree;
				m_pFree = pItem;
				m_nUsed--;
			}
			else
			{
				pLast = &pItem->m_pNext;
			}

			pItem = pNext;
		}
	}
}

void CRouteCacheTable::Resize(DWORD nSize)
{
	nSize = min( max( nSize, DWORD(MIN_BUFFER_SIZE) ), DWORD(MAX_BUFFER_SIZE) );
	nSize = ( ( nSize + BUFFER_BLOCK_SIZE - 1 ) / BUFFER_BLOCK_SIZE * BUFFER_BLOCK_SIZE );

	if ( nSize != m_nBuffer )
	{
		if ( m_pBuffer && m_nBuffer ) delete [] m_pBuffer;

		m_nBuffer = nSize;
		m_pBuffer = nSize ? ( new CRouteCacheItem[ m_nBuffer ] ) : NULL;
	}

	ZeroMemory( m_pHash, sizeof(CRouteCacheItem*) * HASH_SIZE );

	m_pFree		= m_pBuffer;
	m_nUsed		= 0;
	m_tFirst	= 0;
	m_tLast		= 0;

	CRouteCacheItem* pItem = m_pBuffer;

	for ( DWORD nPos = m_nBuffer ; nPos ; nPos--, pItem++ )
	{
		if ( nPos == 1 )
		{
			pItem->m_pNext = NULL;
		}
		else
		{
			pItem->m_pNext = pItem + 1;
		}
	}
}

DWORD CRouteCacheTable::GetNextSize(DWORD nDesired)
{
	DWORD nSeconds = ( m_tLast - m_tFirst ) / 1000;
	if ( ! nSeconds ) nSeconds = 1;

	return m_nBuffer * nDesired / nSeconds;
}

void CRouteCacheTable::Clear()
{
	Resize( 0 );
}
