//
// MatchObjects.cpp
//
//	Date:			"$Date: 2005/06/19 10:00:18 $"
//	Revision:		"$Revision: 1.17 $"
//  Last change by:	"$Author: spooky23 $"
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "MatchObjects.h"
#include "QuerySearch.h"
#include "QueryHit.h"
#include "Library.h"
#include "SharedFile.h"
#include "Schema.h"
#include "Security.h"
#include "ShellIcons.h"
#include "VendorCache.h"
#include "Downloads.h"
#include "Transfers.h"
#include "XML.h"

#include "TigerTree.h"
#include "SHA.h"
#include "ED2K.h"

#include "CtrlMatch.h"
#include "LiveList.h"
#include "ResultFilters.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define MAP_SIZE		256
#define BUFFER_GROW		64


//////////////////////////////////////////////////////////////////////
// CMatchList construction

CMatchList::CMatchList()
{
	m_pResultFilters = new CResultFilters;
	m_pResultFilters->Load();
	
	int nDefaultFilter = m_pResultFilters->m_nDefault;

	if ( ( nDefaultFilter != NONE ) && ( (int)m_pResultFilters->m_nFilters >= nDefaultFilter + 1 ) )
	{
		m_sFilter			= m_pResultFilters->m_pFilters[nDefaultFilter]->m_sFilter;
		m_bFilterBusy		= m_pResultFilters->m_pFilters[nDefaultFilter]->m_bFilterBusy;
		m_bFilterPush		= m_pResultFilters->m_pFilters[nDefaultFilter]->m_bFilterPush;
		m_bFilterUnstable	= m_pResultFilters->m_pFilters[nDefaultFilter]->m_bFilterUnstable;
		m_bFilterReject		= m_pResultFilters->m_pFilters[nDefaultFilter]->m_bFilterReject;
		m_bFilterLocal		= m_pResultFilters->m_pFilters[nDefaultFilter]->m_bFilterLocal;
		m_bFilterBogus		= m_pResultFilters->m_pFilters[nDefaultFilter]->m_bFilterBogus;
		m_nFilterMinSize	= m_pResultFilters->m_pFilters[nDefaultFilter]->m_nFilterMinSize;
		m_nFilterMaxSize	= m_pResultFilters->m_pFilters[nDefaultFilter]->m_nFilterMaxSize;
		m_nFilterSources	= m_pResultFilters->m_pFilters[nDefaultFilter]->m_nFilterSources;
	}
	else
	{
		m_bFilterBusy		= ( Settings.Search.FilterMask & ( 1 << 0 ) ) > 0;
		m_bFilterPush		= ( Settings.Search.FilterMask & ( 1 << 1 ) ) > 0;
		m_bFilterUnstable	= ( Settings.Search.FilterMask & ( 1 << 2 ) ) > 0;
		m_bFilterReject		= ( Settings.Search.FilterMask & ( 1 << 3 ) ) > 0;
		m_bFilterLocal		= ( Settings.Search.FilterMask & ( 1 << 4 ) ) > 0;
		m_bFilterBogus		= ( Settings.Search.FilterMask & ( 1 << 5 ) ) > 0;
		m_nFilterMinSize	= 1;
		m_nFilterMaxSize	= 0;
		m_nFilterSources	= 1;
	}

	m_nSortColumn		= -1;
	m_bSortDir			= 1;
	m_pSchema			= NULL;
	m_bNew				= FALSE;
	
	m_pFiles			= NULL;
	m_nFiles			= 0;
	m_nItems			= 0;
	m_nFilteredFiles	= 0;
	m_nFilteredHits		= 0;
	m_nGnutellaHits		= 0;
	m_nED2KHits			= 0;
	
	m_nBuffer	= 0;
	m_pSizeMap	= new CMatchFile*[ MAP_SIZE ];
	m_pMapSHA1	= new CMatchFile*[ MAP_SIZE ];
	m_pMapTiger	= new CMatchFile*[ MAP_SIZE ];
	m_pMapED2K	= new CMatchFile*[ MAP_SIZE ];
	m_pszFilter	= NULL;
	m_pColumns	= NULL;
	m_nColumns	= 0;
	
	ClearUpdated();
	
	ZeroMemory( m_pSizeMap, 4 * MAP_SIZE );
	ZeroMemory( m_pMapSHA1, 4 * MAP_SIZE );
	ZeroMemory( m_pMapTiger, 4 * MAP_SIZE );
	ZeroMemory( m_pMapED2K, 4 * MAP_SIZE );
	
	SetSortColumn( MATCH_COL_COUNT, TRUE );
}

CMatchList::~CMatchList()
{
	Clear();
	
	if ( m_pColumns ) delete [] m_pColumns;
	
	if ( m_pszFilter ) delete [] m_pszFilter;
	
	delete [] m_pMapED2K;
	delete [] m_pMapTiger;
	delete [] m_pMapSHA1;
	delete [] m_pSizeMap;
	
	if ( m_pFiles ) delete [] m_pFiles;

	delete m_pResultFilters;
}

//////////////////////////////////////////////////////////////////////
// CMatchList add hits

void CMatchList::AddHits(CQueryHit* pHit, CQuerySearch* pFilter, BOOL bRequire)
{
	CSingleLock pLock( &m_pSection, TRUE );
	CMatchFile **pMap, *pSeek;
	
	while ( pHit )
	{
		CQueryHit* pNext = pHit->m_pNext;
		
		if ( Security.IsDenied( &pHit->m_pAddress, pHit->m_sName ) )
		{
			delete pHit;
			pHit = pNext;
			continue;
		}
		
		pHit->m_bNew = m_bNew;
		
		if ( pFilter != NULL )
		{
			pHit->m_bMatched = pFilter->Match(
				pHit->m_sName, pHit->m_nSize, pHit->m_sSchemaURI, pHit->m_pXML,
				pHit->m_bSHA1 ? &pHit->m_pSHA1 : NULL,
				pHit->m_bTiger ? &pHit->m_pTiger : NULL,
				pHit->m_bED2K ? &pHit->m_pED2K : NULL );
			
			if ( bRequire && ! pHit->m_bMatched )
			{
				delete pHit;
				pHit = pNext;
				continue;
			}
			
			if ( Settings.Search.SchemaTypes && pFilter->m_pSchema && ! pHit->m_bBogus )
			{
				if ( pFilter->m_pSchema->CheckURI( pHit->m_sSchemaURI ) )
				{
					pHit->m_bBogus = FALSE;
				}
				else
				{
					pHit->m_bBogus = ! pFilter->m_pSchema->FilterType( pHit->m_sName, TRUE );
				}
			}
		}
		else
		{
			pHit->m_bMatched = TRUE;
		}
		
		FilterHit( pHit );
		
		CMatchFile* pFile	= NULL;
		BOOL bHadSHA1		= FALSE;
		BOOL bHadTiger		= FALSE;
		BOOL bHadED2K		= FALSE;
		int nHadCount		= 0;
		int nHadFiltered	= 0;
		BOOL bHad[3];
		
		if ( pHit->m_bSHA1 )
		{
			pMap = m_pMapSHA1 + ( pHit->m_pSHA1.n[0] );
			
			for ( pSeek = *pMap ; pSeek ; pSeek = pSeek->m_pNextSHA1 )
			{
				if ( pSeek->m_pSHA1 == pHit->m_pSHA1 )
				{
					nHadCount		= pSeek->GetItemCount();
					nHadFiltered	= pSeek->m_nFiltered;
					
					bHad[0] = pSeek->m_bSHA1; bHad[1] = pSeek->m_bTiger; bHad[2] = pSeek->m_bED2K;
					
					if ( pSeek->Add( pHit, TRUE ) )
					{
						pFile		 = pSeek;
						bHadSHA1	|= bHad[0];
						bHadTiger	|= bHad[1];
						bHadED2K	|= bHad[2];
						break;
					}
				}
			}
		}
		if ( pFile == NULL && pHit->m_bTiger )
		{
			pMap = m_pMapTiger + ( pHit->m_pTiger.n[0] );
			
			for ( pSeek = *pMap ; pSeek ; pSeek = pSeek->m_pNextTiger )
			{
				if ( pSeek->m_pTiger == pHit->m_pTiger )
				{
					nHadCount		= pSeek->GetItemCount();
					nHadFiltered	= pSeek->m_nFiltered;
					
					bHad[0] = pSeek->m_bSHA1; bHad[1] = pSeek->m_bTiger; bHad[2] = pSeek->m_bED2K;
					
					if ( pSeek->Add( pHit, TRUE ) )
					{
						pFile		 = pSeek;
						bHadSHA1	|= bHad[0];
						bHadTiger	|= bHad[1];
						bHadED2K	|= bHad[2];
						break;
					}
				}
			}
		}
		if ( pFile == NULL && pHit->m_bED2K )
		{
			pMap = m_pMapED2K + ( pHit->m_pED2K.n[0] );
			
			for ( pSeek = *pMap ; pSeek ; pSeek = pSeek->m_pNextED2K )
			{
				if ( pSeek->m_pED2K == pHit->m_pED2K )
				{
					nHadCount		= pSeek->GetItemCount();
					nHadFiltered	= pSeek->m_nFiltered;
					
					bHad[0] = pSeek->m_bSHA1; bHad[1] = pSeek->m_bTiger; bHad[2] = pSeek->m_bED2K;
					
					if ( pSeek->Add( pHit, TRUE ) )
					{
						pFile		 = pSeek;
						bHadSHA1	|= bHad[0];
						bHadTiger	|= bHad[1];
						bHadED2K	|= bHad[2];

						break;
					}
				}
			}
		}
		
		if ( pFile == NULL && ( ( ! pHit->m_bSHA1 && ! pHit->m_bTiger && ! pHit->m_bED2K ) || ! Settings.General.HashIntegrity ) )
		{
			pMap = m_pSizeMap + (DWORD)( pHit->m_nSize & 0xFF );

			for ( pSeek = *pMap ; pSeek ; pSeek = pSeek->m_pNextSize )
			{
				if ( pSeek->m_nSize == pHit->m_nSize )
				{
					bHadSHA1		= pSeek->m_bSHA1;
					bHadTiger		= pSeek->m_bTiger;
					bHadED2K		= pSeek->m_bED2K;
					nHadCount		= pSeek->GetItemCount();
					nHadFiltered	= pSeek->m_nFiltered;

					if ( pSeek->Add( pHit ) )
					{
						pFile = pSeek;
						break;
					}
				}
			}
			if ( ! pFile ) bHadSHA1 = bHadTiger = bHadED2K = FALSE;
		}
		
		if ( pFile != NULL )
		{
			pMap = m_pFiles;
			
			for ( DWORD nCount = m_nFiles ; nCount ; nCount--, pMap++ )
			{
				if ( *pMap == pFile )
				{
					if ( m_nSortColumn >= 0 )
					{
						UpdateRange( m_nFiles - nCount );
						MoveMemory( pMap, pMap + 1, ( nCount - 1 ) << 2 );
						m_nFiles--;
						InsertSorted( pFile );
					}
					else
					{
						UpdateRange( m_nFiles - nCount, m_nFiles - nCount );
					}
					
					break;
				}
			}
			
			if ( nHadCount )
			{
				m_nItems -= nHadCount;
				m_nFilteredFiles --;
				m_nFilteredHits -= nHadFiltered;

				switch ( pHit->m_nProtocol )
				{
				case PROTOCOL_G1:
				case PROTOCOL_G2:
					m_nGnutellaHits -= nHadFiltered;
					break;
				case PROTOCOL_ED2K:
					m_nED2KHits -= nHadFiltered;
					break;
				}
			}
		}
		else
		{
			pFile = new CMatchFile( this, pHit );
			pFile->m_bNew = m_bNew;
			
			pMap = m_pSizeMap + (DWORD)( pFile->m_nSize & 0xFF );
			pFile->m_pNextSize = *pMap;
			*pMap = pFile;
			
			if ( m_nFiles + 1 > m_nBuffer )
			{
				m_nBuffer += BUFFER_GROW;
				CMatchFile** pFiles = new CMatchFile*[ m_nBuffer ];
				
				if ( m_pFiles )
				{
					CopyMemory( pFiles, m_pFiles, m_nFiles << 2 );
					delete [] m_pFiles;
				}
				
				m_pFiles = pFiles;
			}
			
			if ( m_nSortColumn >= 0 )
			{
				InsertSorted( pFile );
			}
			else
			{
				UpdateRange( m_nFiles );
				m_pFiles[ m_nFiles++ ] = pFile;
			}
		}
		
		if ( ! bHadSHA1 && pFile->m_bSHA1 )
		{
			pMap = m_pMapSHA1 + ( pFile->m_pSHA1.n[0] );
			pFile->m_pNextSHA1 = *pMap;
			*pMap = pFile;
		}
		if ( ! bHadTiger && pFile->m_bTiger )
		{
			pMap = m_pMapTiger + ( pFile->m_pTiger.n[0] );
			pFile->m_pNextTiger = *pMap;
			*pMap = pFile;
		}
		if ( ! bHadED2K && pFile->m_bED2K )
		{
			pMap = m_pMapED2K + ( pFile->m_pED2K.n[0] );
			pFile->m_pNextED2K = *pMap;
			*pMap = pFile;
		}
		
		nHadCount = pFile->GetItemCount();
		
		if ( nHadCount )
		{
			m_nItems += nHadCount;
			m_nFilteredFiles ++;
			m_nFilteredHits += pFile->m_nFiltered;

			switch ( pHit->m_nProtocol )
			{
			case PROTOCOL_G1:
			case PROTOCOL_G2:
				m_nGnutellaHits += pFile->m_nFiltered;
				break;
			case PROTOCOL_ED2K:
				m_nED2KHits += pFile->m_nFiltered;
				break;
			}

		}
		
		pHit = pNext;
	}
}

//////////////////////////////////////////////////////////////////////
// CMatchList insert to a sorted array

void CMatchList::InsertSorted(CMatchFile* pFile)
{
    int nFirst = 0;
	for ( int nLast = m_nFiles - 1 ; nLast >= nFirst ; )
	{
		DWORD nMiddle = ( nFirst + nLast ) >> 1;

		if ( pFile->Compare( m_pFiles[ nMiddle ] ) == m_bSortDir )
		{
			nFirst = nMiddle + 1;
		}
		else
		{
			nLast = nMiddle - 1;
		}
	}

	MoveMemory( m_pFiles + nFirst + 1, m_pFiles + nFirst, ( m_nFiles - nFirst ) << 2 );
	m_pFiles[ nFirst ] = pFile;
	m_nFiles++;
	UpdateRange( nFirst );
}

//////////////////////////////////////////////////////////////////////
// CMatchList searching

DWORD CMatchList::FileToItem(CMatchFile* pFile)
{
	CSingleLock pLock( &m_pSection, TRUE );

	CMatchFile** pLoop = m_pFiles;

	for ( DWORD nCount = 0 ; nCount < m_nFiles ; nCount++, pLoop++ )
	{
		if ( *pLoop == pFile ) return nCount;
	}

	return 0xFFFFFFFF;
}

//////////////////////////////////////////////////////////////////////
// CMatchList clear

void CMatchList::Clear()
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( m_pFiles )
	{
		CMatchFile** pLoop = m_pFiles;

		for ( DWORD nCount = m_nFiles ; nCount ; nCount--, pLoop++ )
		{
			if ( *pLoop ) delete (*pLoop);
		}
	}

	m_nFiles			= 0;
	m_nItems			= 0;
	m_nFilteredFiles	= 0;
	m_nFilteredHits		= 0;

	m_pSelectedFiles.RemoveAll();
	m_pSelectedHits.RemoveAll();

	ZeroMemory( m_pSizeMap, 4 * MAP_SIZE );
	ZeroMemory( m_pMapSHA1, 4 * MAP_SIZE );
	ZeroMemory( m_pMapTiger, 4 * MAP_SIZE );
	ZeroMemory( m_pMapED2K, 4 * MAP_SIZE );

	UpdateRange();
}

//////////////////////////////////////////////////////////////////////
// CMatchList selection

BOOL CMatchList::Select(CMatchFile* pFile, CQueryHit* pHit, BOOL bSelected)
{
	if ( pHit != NULL )
	{
		if ( pHit->m_bSelected == bSelected ) return FALSE;
		pHit->m_bSelected = bSelected;

		if ( bSelected )
			m_pSelectedHits.AddTail( pHit );
		else
			m_pSelectedHits.RemoveAt( m_pSelectedHits.Find( pHit ) );
	}
	else
	{
		if ( pFile->m_bSelected == bSelected ) return FALSE;
		pFile->m_bSelected = bSelected;

		if ( bSelected )
			m_pSelectedFiles.AddTail( pFile );
		else
			m_pSelectedFiles.RemoveAt( m_pSelectedFiles.Find( pFile ) );
	}
	
	if ( pFile != NULL )
	{
		DWORD nIndex = FileToItem( pFile );
		UpdateRange( nIndex, nIndex );
	}
	
	return TRUE;
}

CMatchFile* CMatchList::GetSelectedFile(BOOL bFromHit) const
{
	if ( m_pSelectedFiles.GetCount() != 1 )
	{
		if ( bFromHit == FALSE ) return NULL;
		
		CQueryHit* pHit = GetSelectedHit();
		if ( pHit == NULL ) return NULL;
		
		CMatchFile** pLoop = m_pFiles;
		
		for ( DWORD nCount = m_nFiles ; nCount ; nCount--, pLoop++ )
		{
			if ( (*pLoop)->Check( pHit ) ) return *pLoop;
		}
		
		return NULL;
	}
	
	return (CMatchFile*)m_pSelectedFiles.GetHead();
}

CQueryHit* CMatchList::GetSelectedHit() const
{
	if ( m_pSelectedHits.GetCount() != 1 )
	{
		if ( m_pSelectedFiles.GetCount() != 1 ) return NULL;
		CMatchFile* pFile = (CMatchFile*)m_pSelectedFiles.GetHead();
		return pFile->m_nFiltered == 1 ? pFile->m_pHits : NULL;
	}
	
	return (CQueryHit*)m_pSelectedHits.GetHead();
}

int CMatchList::GetSelectedCount() const
{
	return m_pSelectedFiles.GetCount() + m_pSelectedHits.GetCount();
}

BOOL CMatchList::ClearSelection()
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	CMatchFile** pLoop = m_pFiles;
	BOOL bChanged = FALSE;
	
	for ( DWORD nCount = 0 ; nCount < m_nFiles ; nCount++, pLoop++ )
	{
		if ( (*pLoop)->m_bSelected )
		{
			UpdateRange( nCount, nCount );
			(*pLoop)->m_bSelected = FALSE;
			bChanged = TRUE;
		}
		
		for ( CQueryHit* pHit = (*pLoop)->m_pHits ; pHit ; pHit = pHit->m_pNext )
		{
			if ( pHit->m_bSelected )
			{
				UpdateRange( nCount, nCount );
				pHit->m_bSelected = FALSE;
				bChanged = TRUE;
			}
		}
	}
	
	m_pSelectedFiles.RemoveAll();
	m_pSelectedHits.RemoveAll();
	
	return bChanged;
}

//////////////////////////////////////////////////////////////////////
// CMatchList filter

void CMatchList::Filter()
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	Settings.Search.FilterMask = 0;
	
	if ( m_bFilterBusy )		Settings.Search.FilterMask |= ( 1 << 0 );
	if ( m_bFilterPush )		Settings.Search.FilterMask |= ( 1 << 1 );
	if ( m_bFilterUnstable )	Settings.Search.FilterMask |= ( 1 << 2 );
	if ( m_bFilterReject )		Settings.Search.FilterMask |= ( 1 << 3 );
	if ( m_bFilterLocal )		Settings.Search.FilterMask |= ( 1 << 4 );
	if ( m_bFilterBogus	)		Settings.Search.FilterMask |= ( 1 << 5 );
	
	if ( m_pszFilter ) delete [] m_pszFilter;
	m_pszFilter = NULL;
	
	if ( m_sFilter.GetLength() )
	{
		LPCTSTR pszPtr = m_sFilter;
		BOOL bQuote = FALSE;
		BOOL bNot = FALSE;
		int nWordLen = 3;

		CStringList pWords;
		
        int nStart = 0, nPos = 0;
		for ( ; *pszPtr ; nPos++, pszPtr++ )
		{
			if ( *pszPtr == '\"' || ( ! bQuote && ( *pszPtr == ' ' || *pszPtr == '\t' || *pszPtr == '-' ) ) )
			{
				if ( nStart < nPos )
				{
					pWords.AddTail( ( bNot ? '-' : '+' ) + m_sFilter.Mid( nStart, nPos - nStart ) );
					nWordLen += ( nPos - nStart ) + 2;
				}
				
				nStart = nPos + 1;
				
				if ( *pszPtr == '\"' )
				{
					bQuote = ! bQuote;
				}
				else if ( *pszPtr == '-' && ! bQuote )
				{
					bNot = TRUE;
				}

				if ( bNot && ! bQuote && *pszPtr != '-' ) bNot = FALSE;
			}
		}
		
		if ( nStart < nPos )
		{
			pWords.AddTail( ( bNot ? '-' : '+' ) + m_sFilter.Mid( nStart, nPos - nStart ) );
			nWordLen += ( nPos - nStart ) + 2;
		}
		
		m_pszFilter = new TCHAR[ nWordLen ];
		LPTSTR pszFilter = m_pszFilter;
		
		for ( POSITION pos = pWords.GetHeadPosition() ; pos ; )
		{
			CString strWord = pWords.GetNext( pos );
			CopyMemory( pszFilter, (LPCTSTR)strWord, sizeof(TCHAR) * ( strWord.GetLength() + 1 ) );
			pszFilter += strWord.GetLength() + 1;
		}
		
		*pszFilter++ = 0;
		*pszFilter++ = 0;
	}
	
	CMatchFile** pLoop = m_pFiles;
	
	m_nItems			= 0;
	m_nFilteredFiles	= 0;
	m_nFilteredHits		= 0;
	
	for ( DWORD nCount = m_nFiles, nItems = 0 ; nCount ; nCount--, pLoop++ )
	{
		if ( nItems = (*pLoop)->Filter() )
		{
			m_nItems += nItems;
			m_nFilteredFiles ++;
			m_nFilteredHits += (*pLoop)->m_nFiltered;
		}
		else
		{
			if ( (*pLoop)->m_bSelected ) Select( *pLoop, NULL, FALSE );
		}
	}
	
	SetSortColumn( m_nSortColumn, m_bSortDir < 0 );
	UpdateRange();
}

//////////////////////////////////////////////////////////////////////
// CMatchList hit filtering

BOOL CMatchList::FilterHit(CQueryHit* pHit)
{
	pHit->m_bFiltered = FALSE;
	
	if ( m_bFilterBusy && pHit->m_bBusy == TS_TRUE ) return FALSE;
	//if ( m_bFilterPush && pHit->m_bPush == TS_TRUE && pHit->m_nProtocol != PROTOCOL_ED2K ) return FALSE;
	if ( m_bFilterPush && pHit->m_bPush == TS_TRUE ) return FALSE;
	if ( m_bFilterUnstable && pHit->m_bStable == TS_FALSE ) return FALSE;
	if ( m_bFilterReject && pHit->m_bMatched == FALSE ) return FALSE;
	if ( m_bFilterBogus && pHit->m_bBogus ) return FALSE;
	
	if ( m_nFilterMinSize > 0 && pHit->m_nSize < m_nFilterMinSize ) return FALSE;
	if ( m_nFilterMaxSize > 0 && pHit->m_nSize > m_nFilterMaxSize ) return FALSE;
	
	if ( m_pszFilter )
	{
		LPCTSTR pszName = pHit->m_sName;
		
		for ( LPCTSTR pszFilter = m_pszFilter ; *pszFilter ; )
		{
			if ( *pszFilter == '-' )
			{
				if ( _tcsistr( pszName, pszFilter + 1 ) != NULL ) return FALSE;
			}
			else
			{
				if ( _tcsistr( pszName, pszFilter + 1 ) == NULL ) return FALSE;
			}

			pszFilter += _tcslen( pszFilter + 1 ) + 2;
		}
	}
	
	if ( pHit->m_nSpeed )
		pHit->m_sSpeed = Settings.SmartVolume( pHit->m_nSpeed, TRUE, TRUE );
	else
		pHit->m_sSpeed.Empty();

	if ( AdultFilter.IsSearchFiltered( pHit->m_sName ) ) return FALSE;
	
	return ( pHit->m_bFiltered = TRUE );
}

//////////////////////////////////////////////////////////////////////
// CMatchList schema selection

void CMatchList::SelectSchema(CSchema* pSchema, CPtrList* pColumns)
{
	CSingleLock pLock( &m_pSection, TRUE );

	CMatchFile** pLoop = m_pFiles;

	for ( DWORD nCount = m_nFiles ; nCount ; nCount--, pLoop++ )
	{
		if ( (*pLoop)->m_pColumns )
		{
			delete [] (*pLoop)->m_pColumns;
			(*pLoop)->m_pColumns = NULL;
			(*pLoop)->m_nColumns = 0;
		}
	}
	
	if ( m_pColumns ) delete [] m_pColumns;
	m_pColumns	= NULL;
	m_nColumns	= 0;
	m_pSchema	= pSchema;

	if ( ! pSchema || ! pColumns ) return;

	m_pColumns = new CSchemaMember*[ pColumns->GetCount() ];

	for ( POSITION pos = pColumns->GetHeadPosition() ; pos ; )
	{
		m_pColumns[ m_nColumns++ ] = (CSchemaMember*)pColumns->GetNext( pos );
	}

	Filter();
}

//////////////////////////////////////////////////////////////////////
// CMatchList sort column selection

void CMatchList::SetSortColumn(int nColumn, BOOL bDirection)
{
	m_nSortColumn	= nColumn;
	m_bSortDir		= bDirection ? -1 : 1;

	if ( m_nSortColumn < 0 || ! m_nFiles ) return;

	int nFirst		= 0;
	int nLast		= m_nFiles - 1;
	DWORD nStack	= 0;

	int nStackFirst[128];
	int nStackLast[128];

	for ( ; ; )
	{
		if ( nLast - nFirst <= 16 )
		{
			int nIndex;
			CMatchFile* pPrevious = m_pFiles[ nFirst ];
			CMatchFile* pCurrent;

			for ( nIndex = nFirst + 1 ; nIndex <= nLast ; ++nIndex )
			{
				pCurrent = m_pFiles[ nIndex ];

				if ( pPrevious->Compare( pCurrent ) == m_bSortDir )
				{
					int nIndex2;
					m_pFiles[ nIndex ] = pPrevious;

					for ( nIndex2 = nIndex - 1 ; nIndex2 > nFirst ; )
					{
						CMatchFile* pTemp = m_pFiles[ nIndex2 - 1 ];

						if ( pTemp->Compare( pCurrent ) == m_bSortDir )
						{
							m_pFiles[ nIndex2-- ] = pTemp;
						}
						else break;
					}
					m_pFiles[ nIndex2 ] = pCurrent;
				}
				else
				{
					pPrevious = pCurrent;
				}
			}
		}
		else
		{
			CMatchFile* pPivot;
			{
				CMatchFile* pTemp;
				DWORD nMiddle = ( nFirst + nLast ) >> 1;

				pTemp = m_pFiles[ nFirst ];
				if ( pTemp->Compare( m_pFiles[ nLast ] ) == m_bSortDir )
				{
					m_pFiles[ nFirst ] = m_pFiles[ nLast ]; m_pFiles[ nLast ] = pTemp;
				}

				pTemp = m_pFiles[ nMiddle ];
				if ( m_pFiles[ nFirst ]->Compare( pTemp ) == m_bSortDir )
				{
					m_pFiles[ nMiddle ] = m_pFiles[ nFirst ]; m_pFiles[ nFirst ] = pTemp;
				}

				pTemp = m_pFiles[ nLast ];
				if ( m_pFiles[ nMiddle ]->Compare( pTemp ) == m_bSortDir )
				{
					m_pFiles[ nLast ] = m_pFiles[ nMiddle ]; m_pFiles[ nMiddle ] = pTemp;
				}
				pPivot = m_pFiles[ nMiddle ];
			}
			{
				DWORD nUp;
				{
					DWORD nDown = nFirst;
					nUp = nLast;

					for ( ; ; )
					{
						do
						{
							++nDown;
						} while ( pPivot->Compare( m_pFiles[ nDown ] ) == m_bSortDir );
						do
						{
							--nUp;
						} while ( m_pFiles[ nUp ]->Compare( pPivot ) == m_bSortDir );

						if ( nUp > nDown )
						{
							CMatchFile* pTemp = m_pFiles[ nDown ];
							m_pFiles[ nDown ] = m_pFiles[ nUp ];
							m_pFiles[ nUp ] = pTemp;
						}
						else
							break;
					}
				}
				{
					if ( ( nUp - nFirst + 1 ) >= ( nLast - nUp ) )
					{
						nStackFirst[ nStack ]	= nFirst;
						nStackLast[ nStack++ ]	= nUp;

						nFirst = nUp + 1;
					}
					else
					{
						nStackFirst[ nStack ]	= nUp + 1;
						nStackLast[ nStack++ ]	= nLast;

						nLast = nUp;
					}
				}
				continue;
			}
		}

		if ( nStack > 0 )
		{
			nFirst = nStackFirst[ --nStack ];
			nLast = nStackLast[ nStack ];
		}
		else
			break;
	}

	UpdateRange();
}

//////////////////////////////////////////////////////////////////////
// CMatchList updates

void CMatchList::UpdateRange(DWORD nMin, DWORD nMax)
{
	m_nUpdateMin = min( m_nUpdateMin, nMin );
	m_nUpdateMax = max( m_nUpdateMax, nMax );
	m_bUpdated = TRUE;
}

void CMatchList::ClearUpdated()
{
	m_bUpdated		= FALSE;
	m_nUpdateMin	= 0xFFFFFFFF;
	m_nUpdateMax	= 0;
}

//////////////////////////////////////////////////////////////////////
// CMatchList clear new

void CMatchList::ClearNew()
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	for ( DWORD nFile = 0 ; nFile < m_nFiles ; nFile++ )
	{
		m_pFiles[ nFile ]->ClearNew();
	}
	
	m_bNew = TRUE;
}

//////////////////////////////////////////////////////////////////////
// CMatchList serialize

void CMatchList::Serialize(CArchive& ar)
{
	int nVersion = 11;
	
	if ( ar.IsStoring() )
	{
		ar << nVersion;
		
		ar << m_sFilter;
		ar << m_bFilterBusy;
		ar << m_bFilterPush;
		ar << m_bFilterUnstable;
		ar << m_bFilterReject;
		ar << m_bFilterLocal;
		ar << m_bFilterBogus;
		ar << m_nFilterMinSize;
		ar << m_nFilterMaxSize;
		ar << m_nFilterSources;
		ar << m_nSortColumn;
		ar << m_bSortDir;
		
		ar.WriteCount( m_nFiles );
		
		for ( DWORD nFile = 0 ; nFile < m_nFiles ; nFile++ )
		{
			CMatchFile* pFile = m_pFiles[ nFile ];
			pFile->Serialize( ar, nVersion );
		}
	}
	else
	{
		ar >> nVersion;
		if ( nVersion < 8 ) AfxThrowUserException();
		
		ar >> m_sFilter;
		ar >> m_bFilterBusy;
		ar >> m_bFilterPush;
		ar >> m_bFilterUnstable;
		ar >> m_bFilterReject;
		ar >> m_bFilterLocal;
		ar >> m_bFilterBogus;
		
		if ( nVersion >= 10 )
		{
			ar >> m_nFilterMinSize;
			ar >> m_nFilterMaxSize;
		}
		else
		{
			DWORD nInt32;
			ar >> nInt32; m_nFilterMinSize = nInt32;
			ar >> nInt32; m_nFilterMaxSize = nInt32;
		}
		
		ar >> m_nFilterSources;
		ar >> m_nSortColumn;
		ar >> m_bSortDir;
		
		m_nFiles = m_nBuffer = ar.ReadCount();
		m_pFiles = new CMatchFile*[ m_nFiles ];
		ZeroMemory( m_pFiles, sizeof(CMatchFile*) * m_nFiles );
		
		for ( DWORD nFile = 0 ; nFile < m_nFiles ; nFile++ )
		{
			CMatchFile* pFile = new CMatchFile( this );
			m_pFiles[ nFile ] = pFile;
			pFile->Serialize( ar, nVersion );
			
			CMatchFile** pMap = m_pSizeMap + (DWORD)( pFile->m_nSize & 0xFF );
			pFile->m_pNextSize = *pMap;
			*pMap = pFile;
			
			if ( pFile->m_bSHA1 )
			{
				pMap = m_pMapSHA1 + ( pFile->m_pSHA1.n[0] );
				pFile->m_pNextSHA1 = *pMap;
				*pMap = pFile;
			}
			if ( pFile->m_bTiger )
			{
				pMap = m_pMapTiger + ( pFile->m_pTiger.n[0] );
				pFile->m_pNextTiger = *pMap;
				*pMap = pFile;
			}
			if ( pFile->m_bED2K )
			{
				pMap = m_pMapED2K + ( pFile->m_pED2K.n[0] );
				pFile->m_pNextED2K = *pMap;
				*pMap = pFile;
			}
		}
		
		Filter();
	}
}


//////////////////////////////////////////////////////////////////////
// CMatchFile construction

CMatchFile::CMatchFile(CMatchList* pList, CQueryHit* pHit)
{
	m_pList		= pList;
	m_pHits		= NULL;
	m_pBest		= NULL;
	m_nTotal	= NULL;
	m_nFiltered	= 0;
	m_nSources	= 0;
	
	m_pNextSize		= NULL;
	m_pNextSHA1		= NULL;
	m_pNextTiger	= NULL;
	m_pNextED2K		= NULL;
	
	m_bSHA1		= FALSE;
	m_bTiger	= FALSE;
	m_bED2K		= FALSE;
	m_nSize		= pHit ? pHit->m_nSize : 0;
	m_sSize		= Settings.SmartVolume( m_nSize, FALSE );
	
	m_bBusy			= TS_UNKNOWN;
	m_bPush			= TS_UNKNOWN;
	m_bStable		= TS_UNKNOWN;
	m_bPreview		= FALSE;
	m_nSpeed		= 0;
	m_nRating		= 0;
	m_nRated		= 0;
	m_bDRM			= FALSE;
	m_bCollection	= FALSE;
	
	m_bExpanded		= Settings.Search.ExpandMatches;
	m_bSelected		= FALSE;
	m_bExisting		= FALSE;
	m_bDownload		= FALSE;
	m_bNew			= FALSE;
	m_bOneValid		= FALSE;
	m_nShellIndex	= -1;
	m_pColumns		= NULL;
	m_nColumns		= 0;
	m_pPreview		= NULL;
	m_nPreview		= 0;
	
	if ( pHit ) Add( pHit );
}

CMatchFile::~CMatchFile()
{
	while ( m_pHits )
	{
		CQueryHit* pNext = m_pHits->m_pNext;
		delete m_pHits;
		m_pHits = pNext;
	}
	
	if ( m_pColumns ) delete [] m_pColumns;
	if ( m_pPreview ) delete [] m_pPreview;
}

//////////////////////////////////////////////////////////////////////
// CMatchFile add

BOOL CMatchFile::Add(CQueryHit* pHit, BOOL bForce)
{
	if ( pHit->m_nSize != m_nSize ) return FALSE;
	
	if ( ! bForce )
	{
		if ( m_bSHA1 && ( pHit->m_bSHA1 || Settings.General.HashIntegrity ) )
		{
			if ( ! pHit->m_bSHA1 ) return FALSE;
			if ( m_pSHA1 != pHit->m_pSHA1 ) return FALSE;
			bForce = TRUE;
		}
		else if ( ! m_bSHA1 && pHit->m_bSHA1 && Settings.General.HashIntegrity && m_pHits )
		{
			return FALSE;
		}
	}
	
	BOOL bSubstituted = FALSE;
	
	if ( m_pHits )
	{
		for ( CQueryHit* pOld = m_pHits ; pOld ; pOld = pOld->m_pNext )
		{
			BOOL bName = _tcsicmp( pHit->m_sName, pOld->m_sName ) == 0;
			
			if ( ! bForce && bName ) bForce = TRUE;
			
			if ( bName && pHit->m_pAddress.S_un.S_addr == pOld->m_pAddress.S_un.S_addr &&
				 pHit->m_nPort == pOld->m_nPort )
			{
				if ( pOld->m_bFiltered )
				{
					m_nFiltered --;
					m_nSources -= pOld->GetSources();
					m_nSpeed -= pOld->m_nSpeed;
				}
				
				pOld->Copy( pHit );
				delete pHit;
				
				pHit = pOld;
				bForce = bSubstituted = TRUE;
				break;
			}
		}
		
		if ( ! bForce ) return FALSE;
	}
	
	if ( ! bSubstituted )
	{
		pHit->m_pNext = m_pHits;
		m_pHits = pHit;
		m_nTotal++;
	}
	
	CSingleLock pLock1( &Library.m_pSection );
	BOOL bLocked = FALSE;
	
	if ( ! m_bSHA1 && pHit->m_bSHA1 )
	{
		m_pSHA1 = pHit->m_pSHA1;
		m_bSHA1 = TRUE;
		
		if ( ! m_bExisting && pLock1.Lock( 100 ) )
		{
			if ( CLibraryFile* pExisting = LibraryMaps.LookupFileBySHA1( &m_pSHA1 ) )
				m_bExisting = pExisting->IsAvailable() ? 1 : 2;
			bLocked = TRUE;
		}
	}
	
	if ( ! m_bTiger && pHit->m_bTiger )
	{
		m_pTiger = pHit->m_pTiger;
		m_bTiger = TRUE;
		
		if ( ! m_bExisting && ( bLocked || pLock1.Lock( 100 ) ) )
		{
			if ( CLibraryFile* pExisting = LibraryMaps.LookupFileByTiger( &m_pTiger ) )
				m_bExisting = pExisting->IsAvailable() ? 1 : 2;
			bLocked = TRUE;
		}
	}
	
	if ( ! m_bED2K && pHit->m_bED2K )
	{
		m_pED2K = pHit->m_pED2K;
		m_bED2K = TRUE;
		
		if ( ! m_bExisting && ( bLocked || pLock1.Lock( 100 ) ) )
		{
			if ( CLibraryFile* pExisting = LibraryMaps.LookupFileByED2K( &m_pED2K ) )
				m_bExisting = pExisting->IsAvailable() ? 1 : 2;
			bLocked = TRUE;
		}
	}
	
	if ( bLocked ) pLock1.Unlock();
	
	if ( ! m_bDownload && ! m_bExisting && ( m_bSHA1 || m_bTiger || m_bED2K ) )
	{
		CSingleLock pLock2( &Transfers.m_pSection );
		
		if ( pLock2.Lock( 50 ) )
		{
			if ( m_bSHA1 && Downloads.FindBySHA1( &m_pSHA1 ) != NULL )
			{
				m_bDownload = TRUE;
			}
			else if ( m_bTiger && Downloads.FindByTiger( &m_pTiger ) != NULL )
			{
				m_bDownload = TRUE;
			}
			else if ( m_bED2K && Downloads.FindByED2K( &m_pED2K ) != NULL )
			{
				m_bDownload = TRUE;
			}
		}
	}
	
	if ( pHit->m_bFiltered ) Added( pHit );
	
	if ( ! m_bOneValid && ! pHit->m_bBogus && pHit->m_bMatched ) m_bOneValid = TRUE;
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CMatchFile check

BOOL CMatchFile::Check(CQueryHit* pHit) const
{
	for ( CQueryHit* pOld = m_pHits ; pOld ; pOld = pOld->m_pNext )
	{
		if ( pOld == pHit ) return TRUE;
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CMatchFile expand/collapse

BOOL CMatchFile::Expand(BOOL bExpand)
{
	if ( m_bExpanded == bExpand ) return FALSE;

	m_pList->m_nItems -= GetItemCount();
	m_bExpanded = bExpand;
	m_pList->m_nItems += GetItemCount();

	if ( ! m_bExpanded )
	{
		for ( CQueryHit* pHit = m_pHits ; pHit ; pHit = pHit->m_pNext )
		{
			if ( pHit->m_bSelected ) m_pList->Select( this, pHit, FALSE );
		}
	}

	m_pList->UpdateRange( m_pList->FileToItem( this ) );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CMatchFile filter

DWORD CMatchFile::Filter()
{
	m_bBusy			= TS_UNKNOWN;
	m_bPush			= TS_UNKNOWN;
	m_bStable		= TS_UNKNOWN;
	m_bPreview		= FALSE;
	m_nSpeed		= 0;
	m_nRating		= 0;
	m_nRated		= 0;
	m_bDRM			= FALSE;
	m_bCollection	= FALSE;
	
	m_nFiltered		= 0;
	m_nSources		= 0;
	m_pBest			= NULL;
	
	for ( CQueryHit* pHit = m_pHits ; pHit ; pHit = pHit->m_pNext )
	{
		if ( m_pList->FilterHit( pHit ) )
		{
			Added( pHit );
		}
		else if ( pHit->m_bSelected )
		{
			m_pList->Select( this, pHit, FALSE );
		}
	}

	if ( m_pBest == NULL ) return 0;	// If we filtered all hits, don't try to display
	if ( m_pList->m_bFilterLocal && m_bExisting ) return 0;
	if ( m_nSources < m_pList->m_nFilterSources ) return 0;
	// if ( m_nFiltered < m_pList->m_nFilterSources ) return 0;

	if ( m_nFiltered == 1 || ! m_bExpanded )
		return 1;
	else
		return m_nFiltered + 1;
}

//////////////////////////////////////////////////////////////////////
// CMatchFile averaging

void CMatchFile::Added(CQueryHit* pHit)
{
	m_pBest = pHit;
	
	m_nFiltered ++;
	m_nSources += pHit->GetSources();
	m_nSpeed += pHit->m_nSpeed;
	
	if ( m_nFiltered && m_nSpeed )
		m_sSpeed = Settings.SmartVolume( m_nFiltered ? m_nSpeed / m_nFiltered : 0, TRUE, TRUE );
	else
		m_sSpeed.Empty();
	
	if ( pHit->GetSources() > 0 )
	{
		if ( pHit->m_bPush == TS_FALSE )
			m_bPush = TS_FALSE;
		else if ( pHit->m_bPush == TS_TRUE && m_bPush == TS_UNKNOWN )
			m_bPush = TS_TRUE;
		
		if ( pHit->m_bBusy == TS_FALSE )
			m_bBusy = TS_FALSE;
		else if ( pHit->m_bBusy == TS_TRUE && m_bBusy == TS_UNKNOWN )
			m_bBusy = TS_TRUE;
		
		if ( pHit->m_bStable == TS_TRUE )
			m_bStable = TS_TRUE;
		else if ( pHit->m_bStable == TS_FALSE && m_bStable == TS_UNKNOWN )
			m_bStable = TS_FALSE;
		
		m_bPreview |= pHit->m_bPreview;
	}
	
	m_bCollection |= ( pHit->m_bCollection && ! pHit->m_bBogus );
	
	if ( pHit->m_nRating )
	{
		m_nRating += pHit->m_nRating;
		m_nRated ++;
	}
	
	if ( m_nShellIndex == -1 )
	{
		LPCTSTR pszExt = _tcsrchr( pHit->m_sName, '.' );
		m_nShellIndex = pszExt ? ShellIcons.Get( pszExt, 16 ) : 0;
	}
	
	BOOL bSchema;
	
	if ( m_pList->m_pSchema &&
		 ( bSchema = m_pList->m_pSchema->CheckURI( pHit->m_sSchemaURI ) || pHit->m_bSHA1 ) )
	{
		if ( m_pColumns == NULL )
		{
			m_nColumns = m_pList->m_nColumns;
			m_pColumns = new CString[ m_nColumns ];
		}
		
		CSchemaMember** pMember = m_pList->m_pColumns;
		CString strValue;
		
		for ( int nCount = 0 ; nCount < m_nColumns ; nCount ++, pMember ++ )
		{
			if ( _tcsicmp( (*pMember)->m_sName, _T("SHA1") ) == 0 )
			{
				if ( pHit->m_bSHA1 )
				{
					m_pColumns[ nCount ] = CSHA::HashToString( &m_pSHA1 );
				}
			}
			else if ( bSchema )
			{
				strValue = (*pMember)->GetValueFrom( pHit->m_pXML, NULL, TRUE );
				if ( strValue.GetLength() ) m_pColumns[ nCount ] = strValue;
			}
		}
	}
	else if ( m_pColumns != NULL )
	{
		delete [] m_pColumns;
		m_pColumns = NULL;
		m_nColumns = 0;
	}
	
	if ( ! m_bDRM && pHit->m_pXML != NULL )
	{
		if ( pHit->m_pXML->GetAttributeValue( _T("DRM") ).GetLength() > 0 )
			m_bDRM = TRUE;
	}
	
	if ( m_bDownload ) pHit->m_bDownload = TRUE;
}

//////////////////////////////////////////////////////////////////////
// CMatchFile clear new

void CMatchFile::ClearNew()
{
	m_bNew = FALSE;
	for ( CQueryHit* pHit = m_pHits ; pHit ; pHit = pHit->m_pNext )
		pHit->m_bNew = FALSE;
}

//////////////////////////////////////////////////////////////////////
// CMatchFile compare (for sort)

int CMatchFile::Compare(CMatchFile* pFile) const
{
	register int x, y;
	
	/*
	if ( m_bCollection != pFile->m_bCollection )
	{
		return m_bCollection ? -m_pList->m_bSortDir : m_pList->m_bSortDir;
	}
	*/
	
	switch ( m_pList->m_nSortColumn )
	{
	case MATCH_COL_NAME:
		x = _tcsicoll( m_pHits->m_sName, pFile->m_pHits->m_sName );
		if ( ! x ) return 0;
		return x > 0 ? 1 : -1;

	case MATCH_COL_TYPE:
		{
			LPCTSTR pszType1 = _tcsrchr( m_pHits->m_sName, '.' );
			LPCTSTR pszType2 = _tcsrchr( pFile->m_pHits->m_sName, '.' );
			if ( ! pszType1 ) return ( pszType2 ? -1 : 0 );
			if ( ! pszType2 ) return 1;
			x = _tcsicmp( pszType1, pszType2 );
			if ( ! x ) return 0;
			return x > 0 ? 1 : -1;
		}
		
	case MATCH_COL_SIZE:
		return m_nSize == pFile->m_nSize ? 0 : ( m_nSize > pFile->m_nSize ? 1 : -1 );
	
	case MATCH_COL_RATING:
		x = m_nRated ? m_nRating / m_nRated : 0;
		y = pFile->m_nRated ? pFile->m_nRating / pFile->m_nRated : 0;
		return x == y ? 0 : ( x > y ? 1 : -1 );
		
	case MATCH_COL_STATUS:
		x = y = 0;
		if ( m_bPush != TS_TRUE ) x += 4;
		if ( m_bBusy != TS_TRUE ) x += 2;
		if ( m_bStable == TS_TRUE ) x ++;
		if ( pFile->m_bPush != TS_TRUE ) y += 4;
		if ( pFile->m_bBusy != TS_TRUE ) y += 2;
		if ( pFile->m_bStable == TS_TRUE ) y ++;
		return x == y ? 0 : ( x > y ? 1 : -1 );
		
	case MATCH_COL_COUNT:
		return m_nSources == pFile->m_nSources ? 0 : ( m_nSources > pFile->m_nSources ? 1 : -1 );
		
	case MATCH_COL_CLIENT:
		{
			LPCTSTR pszType1 = ( m_nFiltered == 1 ) ? (LPCTSTR)m_pHits->m_pVendor->m_sName : NULL;
			LPCTSTR pszType2 = ( pFile->m_nFiltered == 1 ) ? (LPCTSTR)pFile->m_pHits->m_pVendor->m_sName : NULL;
			if ( ! pszType1 ) return ( pszType2 ? -1 : 0 );
			if ( ! pszType2 ) return 1;
			x = _tcsicmp( pszType1, pszType2 );
			if ( ! x ) return 0;
			return x > 0 ? 1 : -1;
		}
		
	case MATCH_COL_SPEED:
		x = m_nFiltered ? m_nSpeed / m_nFiltered : 0;
		y = pFile->m_nFiltered ? pFile->m_nSpeed / pFile->m_nFiltered : 0;
		return x == y ? 0 : ( x > y ? 1 : -1 );
		
	default:
		if ( ! m_pColumns ) return ( pFile->m_pColumns ? -1 : 0 );
		else if ( ! pFile->m_pColumns ) return 1;
		
		x = ( m_pList->m_nSortColumn - MATCH_COL_MAX >= m_nColumns );
		y = ( m_pList->m_nSortColumn - MATCH_COL_MAX >= pFile->m_nColumns );
		if ( x ) return ( y ? 0 : -1 );
		else if ( y ) return 1;
		
		{
			LPCTSTR pszA = m_pColumns[ m_pList->m_nSortColumn - MATCH_COL_MAX ];
			LPCTSTR pszB = pFile->m_pColumns[ m_pList->m_nSortColumn - MATCH_COL_MAX ];
			
#if 0
			if ( *pszA && *pszB &&
				( pszA[ _tcslen( pszA ) - 1 ] == 'k' || pszA[ _tcslen( pszA ) - 1 ] == '~' )
				&&
				( pszB[ _tcslen( pszB ) - 1 ] == 'k' || pszB[ _tcslen( pszB ) - 1 ] == '~' ) )
			{
				x = CLiveList::SortProc( pszA, pszB, TRUE );
			}
			else
			{
				x = _tcsicoll(	m_pColumns[ m_pList->m_nSortColumn - MATCH_COL_MAX ],
								pFile->m_pColumns[ m_pList->m_nSortColumn - MATCH_COL_MAX ] );
			}
#else
			x = CLiveList::SortProc( pszA, pszB );
#endif
		}
		if ( ! x ) return 0;
		return x > 0 ? 1 : -1;
	}
}

//////////////////////////////////////////////////////////////////////
// CMatchFile URN

CString CMatchFile::GetURN() const
{
	CString strURN;
	
	if ( m_bSHA1 && m_bTiger )
	{
		strURN	= _T("urn:bitprint:")
				+ CSHA::HashToString( &m_pSHA1 ) + '.'
				+ CTigerNode::HashToString( &m_pTiger );
	}
	else if ( m_bSHA1 )
	{
		strURN = CSHA::HashToString( &m_pSHA1, TRUE );
	}
	else if ( m_bTiger )
	{
		strURN = CTigerNode::HashToString( &m_pTiger, TRUE );
	}
	else if ( m_bED2K )
	{
		strURN = CED2K::HashToString( &m_pED2K, TRUE );
	}
	
	return strURN;
}

//////////////////////////////////////////////////////////////////////
// CMatchFile serialize

void CMatchFile::Serialize(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		ar << m_nSize;
		ar << m_sSize;
		ar << m_bSHA1;
		if ( m_bSHA1 ) ar.Write( &m_pSHA1, sizeof(SHA1) );
		ar << m_bTiger;
		if ( m_bTiger ) ar.Write( &m_pTiger, sizeof(TIGEROOT) );
		ar << m_bED2K;
		if ( m_bED2K ) ar.Write( &m_pED2K, sizeof(MD4) );

		ar << m_bBusy;
		ar << m_bPush;
		ar << m_bStable;
		ar << m_nSpeed;
		ar << m_sSpeed;
		ar << m_bExpanded;
		ar << m_bExisting;
		ar << m_bDownload;
		ar << m_bOneValid;
		
		if ( m_pPreview == NULL ) m_nPreview = 0;
		ar.WriteCount( m_nPreview );
		if ( m_nPreview > 0 ) ar.Write( m_pPreview, m_nPreview );
		
		ar.WriteCount( m_nTotal );
		CPtrArray pHits;
		
        CQueryHit* pHit = m_pHits;
		for ( ; pHit ; pHit = pHit->m_pNext )
		{
			pHits.Add( pHit );
		}
		
		for ( int nHit = m_nTotal - 1 ; nHit >= 0 ; nHit-- )
		{
			pHit = (CQueryHit*)pHits.GetAt( nHit );
			pHit->Serialize( ar, nVersion );
		}
	}
	else
	{
		if ( nVersion >= 10 )
		{
			ar >> m_nSize;
		}
		else
		{
			DWORD nSize;
			ar >> nSize;
			m_nSize = nSize;
		}
		
		ar >> m_sSize;
		ar >> m_bSHA1;
		if ( m_bSHA1 ) ar.Read( &m_pSHA1, sizeof(SHA1) );
		ar >> m_bTiger;
		if ( m_bTiger ) ar.Read( &m_pTiger, sizeof(TIGEROOT) );
		ar >> m_bED2K;
		if ( m_bED2K ) ar.Read( &m_pED2K, sizeof(MD4) );
		
		ar >> m_bBusy;
		ar >> m_bPush;
		ar >> m_bStable;
		ar >> m_nSpeed;
		ar >> m_sSpeed;
		ar >> m_bExpanded;
		ar >> m_bExisting;
		ar >> m_bDownload;
		ar >> m_bOneValid;
		
		if ( m_nPreview = ar.ReadCount() )
		{
			m_pPreview = new BYTE[ m_nPreview ];
			ar.Read( m_pPreview, m_nPreview );
		}
		
		m_nTotal = ar.ReadCount();
		
		for ( int nCount = m_nTotal ; nCount > 0 ; nCount-- )
		{
			CQueryHit* pNext = new CQueryHit( PROTOCOL_NULL );
			pNext->m_pNext = m_pHits;
			m_pHits = pNext;
			m_pHits->Serialize( ar, nVersion );
		}
	}
}
