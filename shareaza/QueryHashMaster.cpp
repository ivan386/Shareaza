//
// QueryHashMaster.cpp
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
#include "QueryHashMaster.h"
#include "QueryHashGroup.h"
#include "Neighbour.h"
#include "Library.h"
#include "LibraryDictionary.h"
#include "Transfers.h"
#include "Downloads.h"
#include "Download.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CQueryHashMaster QueryHashMaster;


//////////////////////////////////////////////////////////////////////
// CQueryHashMaster construction

CQueryHashMaster::CQueryHashMaster()
{
	m_nPerGroup = 0;
}

CQueryHashMaster::~CQueryHashMaster()
{
	ASSERT( GetCount() == 0 );
}

//////////////////////////////////////////////////////////////////////
// CQueryHashMaster create

void CQueryHashMaster::Create()
{
	CQueryHashTable::Create();
	
	m_nPerGroup			= 250;
	m_bValid			= FALSE;
	m_bLive				= FALSE;
	m_nCookie			= 0;
}

//////////////////////////////////////////////////////////////////////
// CQueryHashMaster add neighbour

void CQueryHashMaster::Add(CQueryHashTable* pTable)
{
	ASSERT( m_nPerGroup > 0 );
	ASSERT( pTable != NULL );
	ASSERT( pTable->m_nHash > 0 );
	ASSERT( pTable->m_pGroup == NULL );
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CQueryHashGroup* pGroup = GetNext( pos );
		
		if ( pGroup->m_nHash == pTable->m_nHash &&
			 pGroup->GetCount() < m_nPerGroup )
		{
			pGroup->Add( pTable );
			m_bValid = FALSE;
			return;
		}
	}
	
	CQueryHashGroup* pGroup = new CQueryHashGroup( pTable->m_nHash );
	m_pGroups.AddTail( pGroup );
	pGroup->Add( pTable );
	m_bValid = FALSE;
}

//////////////////////////////////////////////////////////////////////
// CQueryHashMaster remove neighbour

void CQueryHashMaster::Remove(CQueryHashTable* pTable)
{
	ASSERT( pTable != NULL );
	if( pTable->m_pGroup == NULL ) return;
	
	CQueryHashGroup* pGroup = pTable->m_pGroup;
	pGroup->Remove( pTable );
	
	if ( pGroup->GetCount() == 0 )
	{
		POSITION pos = m_pGroups.Find( pGroup );
		ASSERT( pos != NULL );
		m_pGroups.RemoveAt( pos );
		delete pGroup;
	}
	
	m_bValid = FALSE;
}

//////////////////////////////////////////////////////////////////////
// CQueryHashMaster build

void CQueryHashMaster::Build()
{
	DWORD tNow = GetTickCount();
	
	if ( m_bValid )
	{
		if ( tNow - m_nCookie < 600000 ) return;
	}
	else
	{
		if ( tNow - m_nCookie < 20000 ) return;
	}
	
	CQueryHashTable* pLocalTable = LibraryDictionary.GetHashTable();
	if ( pLocalTable == NULL ) return;
	
	Clear();
	Merge( pLocalTable );
	
	Library.Unlock();
	
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CQueryHashGroup* pGroup = GetNext( pos );
		Merge( pGroup );
	}
	
	if ( Transfers.m_pSection.Lock( 100 ) )
	{
		for ( POSITION pos = Downloads.GetIterator() ; pos ; )
		{
			CDownload* pDownload = Downloads.GetNext( pos );
			
			if ( pDownload->m_bSHA1 )
			{
				AddString( CSHA::HashToString( &pDownload->m_pSHA1, TRUE ) );
			}
			
			if ( pDownload->m_bED2K )
			{
				AddString( CED2K::HashToString( &pDownload->m_pED2K, TRUE ) );
			}
			
			if ( pDownload->m_bBTH )
			{
				AddString( _T("BTIH") );
				AddString( _T("urn:btih:") + CSHA::HashToString( &pDownload->m_pBTH, FALSE ) );
			}
		}
		
		Transfers.m_pSection.Unlock();
	}
	
	m_bValid	= TRUE;
	m_bLive		= TRUE;
	m_nCookie	= tNow;
}
