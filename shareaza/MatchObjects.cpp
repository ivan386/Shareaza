//
// MatchObjects.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "SchemaCache.h"
#include "Security.h"
#include "ShellIcons.h"
#include "CoolInterface.h"
#include "VendorCache.h"
#include "Downloads.h"
#include "Transfers.h"
#include "XML.h"
#include "Download.h"

#include "TigerTree.h"
#include "SHA.h"
#include "ED2K.h"

#include "CtrlMatch.h"
#include "LiveList.h"
#include "ResultFilters.h"

#include "CtrlSearchDetailPanel.h"

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
		m_bFilterDRM		= m_pResultFilters->m_pFilters[nDefaultFilter]->m_bFilterDRM;
		m_bFilterAdult		= m_pResultFilters->m_pFilters[nDefaultFilter]->m_bFilterAdult;
		m_bFilterSuspicious	= m_pResultFilters->m_pFilters[nDefaultFilter]->m_bFilterSuspicious;
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
		m_bFilterDRM		= ( Settings.Search.FilterMask & ( 1 << 6 ) ) > 0;
		m_bFilterAdult		= ( Settings.Search.FilterMask & ( 1 << 7 ) ) > 0;
		m_bFilterSuspicious	= ( Settings.Search.FilterMask & ( 1 << 8 ) ) > 0;
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
	m_pMapBTH	= new CMatchFile*[ MAP_SIZE ];
	m_pMapMD5	= new CMatchFile*[ MAP_SIZE ];
	m_pszFilter	= NULL;
	m_pColumns	= NULL;
	m_nColumns	= 0;
	
	ClearUpdated();
	
	ZeroMemory( m_pSizeMap, MAP_SIZE * sizeof *m_pSizeMap );
	ZeroMemory( m_pMapSHA1, MAP_SIZE * sizeof *m_pMapSHA1 );
	ZeroMemory( m_pMapTiger, MAP_SIZE * sizeof *m_pMapTiger );
	ZeroMemory( m_pMapED2K, MAP_SIZE * sizeof *m_pMapED2K );
	ZeroMemory( m_pMapBTH, MAP_SIZE * sizeof *m_pMapBTH );
	ZeroMemory( m_pMapMD5, MAP_SIZE * sizeof *m_pMapMD5 );
	
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
	delete [] m_pMapBTH;
	delete [] m_pMapMD5;
	delete [] m_pSizeMap;
	
	if ( m_pFiles ) delete [] m_pFiles;

	delete m_pResultFilters;
}

//////////////////////////////////////////////////////////////////////
// CMatchList add hits

void CMatchList::AddHits(CQueryHit* pHit, CQuerySearch* pFilter, BOOL bRequire)
{
	CSingleLock pLock( &m_pSection, TRUE );
	CMatchFile **pMap;
	
	while ( pHit )
	{
		CQueryHit* pNext = pHit->m_pNext;

		// Empty file names mean a hit for the currently downloading file.
		// We just clicked the search result while the search was in progress.
		// The size may be zero or match the size of the file.
		// Empty file names are catched by the next clause and deleted.

		if ( Security.IsDenied( &pHit->m_pAddress, pHit->m_sName ) || pHit->m_sName.IsEmpty() )
		{
			delete pHit;
			pHit = pNext;
			continue;
		}
		
		pHit->m_bNew = m_bNew;
		
		if ( pFilter != NULL )
		{
			if ( BOOL bName = _tcsistr( pFilter->m_sKeywords, pHit->m_sName ) == 0 )
				pHit->m_bExactMatch = TRUE;

			pHit->m_bMatched = pFilter->Match(
				pHit->m_sName, pHit->m_nSize, pHit->m_sSchemaURI, pHit->m_pXML,
				pHit->m_oSHA1,
				pHit->m_oTiger,
				pHit->m_oED2K,
				pHit->m_oBTH,
				pHit->m_oMD5);
			
			if ( bRequire && ! pHit->m_bMatched )
			{
				delete pHit;
				pHit = pNext;
				continue;
			}
			
			// ToDo: Change to pHit->m_bMatched when we will be able to get folder name
			// from hits. Raza sends hits if folder name matches the search keywords too.
			// For now, just move such files to bogus.
			if ( Settings.Search.SchemaTypes && pFilter->m_pSchema )
			{
				if ( !pHit->m_bMatched && pFilter->m_pSchema->CheckURI( pHit->m_sSchemaURI ) )
				{
					pHit->m_bBogus = TRUE;
				}
				else
				{
					pHit->m_bBogus = !pFilter->m_pSchema->FilterType( pHit->m_sName, TRUE );
				}
			}
		}
		else
		{
			pHit->m_bMatched = TRUE;
		}
		
		FilterHit( pHit );
		
		CMatchFile* pFile	= NULL;
		PROTOCOLID nProtocol= pHit->m_nProtocol;
		FILESTATS Stats = {};

		if ( pHit->m_oSHA1 )
		{
			pFile = FindFileAndAddHit( pHit, fSHA1, &Stats );
		}
		if ( pFile == NULL && pHit->m_oTiger )
		{
			pFile = FindFileAndAddHit( pHit, fTiger, &Stats );
		}
		if ( pFile == NULL && pHit->m_oED2K )
		{
			pFile = FindFileAndAddHit( pHit, fED2K, &Stats );
		}
		if ( pFile == NULL && pHit->m_oBTH )
		{
			pFile = FindFileAndAddHit( pHit, fBTH, &Stats );
		}
		if ( pFile == NULL && pHit->m_oMD5 )
		{
			pFile = FindFileAndAddHit( pHit, fMD5, &Stats );
		}
		
		if ( pFile == NULL
            && ( ( ! pHit->m_oSHA1 && ! pHit->m_oTiger && ! pHit->m_oED2K && ! pHit->m_oBTH && ! pHit->m_oMD5 )
                || !Settings.General.HashIntegrity ) )
		{
			pFile = FindFileAndAddHit( pHit, fSize, &Stats );
		}
		
		if ( pFile != NULL ) // New hit for the existing file
		{
			pMap = m_pFiles;

			for ( DWORD nCount = m_nFiles ; nCount ; nCount--, pMap++ )
			{
				if ( *pMap == pFile )
				{
					if ( m_nSortColumn >= 0 )
					{
						UpdateRange( m_nFiles - nCount );
						MoveMemory( pMap, pMap + 1, ( nCount - 1 ) * sizeof *pMap );
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
			
			if ( Stats.nHadCount )
			{
				ASSERT( m_nItems >= (UINT)Stats.nHadCount );
				m_nItems -= Stats.nHadCount;
				ASSERT( m_nFilteredFiles );
				m_nFilteredFiles --;
				ASSERT( m_nFilteredHits >= (UINT)Stats.nHadFiltered );
				m_nFilteredHits -= Stats.nHadFiltered;

				switch ( nProtocol )
				{
				case PROTOCOL_G1:
				case PROTOCOL_G2:
					ASSERT( m_nGnutellaHits >= (UINT)Stats.nHadFiltered );
					m_nGnutellaHits -= Stats.nHadFiltered;
					break;
				case PROTOCOL_ED2K:
					ASSERT( m_nED2KHits >= (UINT)Stats.nHadFiltered );
					m_nED2KHits -= Stats.nHadFiltered;
					break;
				case PROTOCOL_BT:
				case PROTOCOL_FTP:
				case PROTOCOL_HTTP:
				case PROTOCOL_NULL:
				case PROTOCOL_ANY:
				default:
//					ASSERT( 0 )
					;
				}
			}
		}
		else // New file hit
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
					CopyMemory( pFiles, m_pFiles, m_nFiles * sizeof( CMatchFile* ) );
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
		
		if ( ! Stats.bHadSHA1 && pFile->m_oSHA1 )
		{
			pMap = m_pMapSHA1 + pFile->m_oSHA1[ 0 ];
			pFile->m_pNextSHA1 = *pMap;
			*pMap = pFile;
		}
		if ( ! Stats.bHadTiger && pFile->m_oTiger )
		{
			pMap = m_pMapTiger + pFile->m_oTiger[ 0 ];
			pFile->m_pNextTiger = *pMap;
			*pMap = pFile;
		}
		if ( ! Stats.bHadED2K && pFile->m_oED2K )
		{
			pMap = m_pMapED2K + pFile->m_oED2K[ 0 ];
			pFile->m_pNextED2K = *pMap;
			*pMap = pFile;
		}
		if ( ! Stats.bHadBTH && pFile->m_oBTH )
		{
			pMap = m_pMapBTH + pFile->m_oBTH[ 0 ];
			pFile->m_pNextBTH = *pMap;
			*pMap = pFile;
		}
		if ( ! Stats.bHadMD5 && pFile->m_oMD5 )
		{
			pMap = m_pMapMD5 + pFile->m_oMD5[ 0 ];
			pFile->m_pNextMD5 = *pMap;
			*pMap = pFile;
		}
		
		Stats.nHadCount = pFile->GetItemCount();
		
		if ( Stats.nHadCount )
		{
			m_nItems += Stats.nHadCount;
			m_nFilteredFiles ++;
			m_nFilteredHits += pFile->m_nFiltered;

			switch ( nProtocol )
			{
			case PROTOCOL_G1:
			case PROTOCOL_G2:
				m_nGnutellaHits += pFile->m_nFiltered;
				break;
			case PROTOCOL_ED2K:
				m_nED2KHits += pFile->m_nFiltered;
				break;
			case PROTOCOL_BT:
			case PROTOCOL_FTP:
			case PROTOCOL_HTTP:
			case PROTOCOL_NULL:
			case PROTOCOL_ANY:
			default:
//				ASSERT( 0 )
				;
			}

		}
		
		pHit = pNext;
	}
}

CMatchFile* CMatchList::FindFileAndAddHit(CQueryHit* pHit, findType nFindFlag, FILESTATS FAR* Stats)
{
	CMatchFile **pMap, *pSeek;
	CMatchFile* pFile = NULL;

	bool bSHA1	= nFindFlag == fSHA1 && pHit->m_oSHA1;
	bool bTiger	= nFindFlag == fTiger && pHit->m_oTiger;
	bool bED2K	= nFindFlag == fED2K && pHit->m_oED2K;
	bool bBTH	= nFindFlag == fBTH && pHit->m_oBTH;
	bool bMD5	= nFindFlag == fMD5 && pHit->m_oMD5;
	bool bSize	= nFindFlag == fSize;

	if ( bSHA1 )
		pMap = m_pMapSHA1 + pHit->m_oSHA1[ 0 ];
	else if ( bTiger )
		pMap = m_pMapTiger + pHit->m_oTiger[ 0 ];
	else if ( bED2K )
		pMap = m_pMapED2K + ( pHit->m_oED2K[ 0 ] );
	else if ( bBTH )
		pMap = m_pMapBTH + ( pHit->m_oBTH[ 0 ] );
	else if ( bMD5 )
		pMap = m_pMapMD5 + ( pHit->m_oMD5[ 0 ] );
	else if ( bSize )
		pMap = m_pSizeMap + (DWORD)( pHit->m_nSize & 0xFF );
	else
	{
		ZeroMemory( Stats, sizeof(Stats) );
		return NULL;
	}

	bool bValid = false;

	for ( pSeek = *pMap ; pSeek ; )
	{
		if ( bSHA1 )
			bValid = validAndEqual( pSeek->m_oSHA1, pHit->m_oSHA1 );
		else if ( bTiger )
			bValid = validAndEqual( pSeek->m_oTiger, pHit->m_oTiger );
		else if ( bED2K )
			bValid = validAndEqual( pSeek->m_oED2K, pHit->m_oED2K );
		else if ( bBTH )
			bValid = validAndEqual( pSeek->m_oBTH, pHit->m_oBTH );
		else if ( bMD5 )
			bValid = validAndEqual( pSeek->m_oMD5, pHit->m_oMD5 );
		else if ( bSize )
			bValid = pSeek->m_nSize == pHit->m_nSize;

		if ( bValid )
		{

			Stats->nHadCount	= pSeek->GetItemCount();
			Stats->nHadFiltered	= pSeek->m_nFiltered;

			if ( bSize )
			{
				Stats->bHadSHA1		= bool( pSeek->m_oSHA1 );
				Stats->bHadTiger	= bool( pSeek->m_oTiger );
				Stats->bHadED2K		= bool( pSeek->m_oED2K );
				Stats->bHadBTH		= bool( pSeek->m_oBTH );
				Stats->bHadMD5		= bool( pSeek->m_oMD5 );

				// ToDo: Fixme. 
				// pSeek->Add( pHit ) returns pHit->m_pNext with a bad memory address sometimes.
				// Dangerous!!!
				if ( pSeek->Add( pHit ) )
				{
					pFile = pSeek;
					break;
				}
			}
			else
			{
				Stats->bHad[0] = bool( pSeek->m_oSHA1 );
				Stats->bHad[1] = bool( pSeek->m_oTiger );
				Stats->bHad[2] = bool( pSeek->m_oED2K );
				Stats->bHad[3] = bool( pSeek->m_oBTH );
				Stats->bHad[4] = bool( pSeek->m_oMD5 );

				if ( pSeek->Add( pHit, TRUE ) )
				{
					pFile = pSeek;
					Stats->bHadSHA1	 |= Stats->bHad[0];
					Stats->bHadTiger |= Stats->bHad[1];
					Stats->bHadED2K	 |= Stats->bHad[2];
					Stats->bHadBTH	 |= Stats->bHad[3];
					Stats->bHadMD5	 |= Stats->bHad[4];
					break;
				}
			}
		}

		if ( bSize && !pFile ) 
			Stats->bHadSHA1 = Stats->bHadTiger = Stats->bHadED2K = Stats->bHadBTH = Stats->bHadMD5 = FALSE;

		if ( bSHA1 )
			pSeek = pSeek->m_pNextSHA1;
		else if ( bTiger )
			pSeek = pSeek->m_pNextTiger;
		else if ( bED2K )
			pSeek = pSeek->m_pNextED2K;
		else if ( bBTH )
			pSeek = pSeek->m_pNextBTH;
		else if ( bMD5 )
			pSeek = pSeek->m_pNextMD5;
		else if ( bSize )
			pSeek = pSeek->m_pNextSize;
	}

	return pFile;
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

	MoveMemory( m_pFiles + nFirst + 1, m_pFiles + nFirst, ( m_nFiles - nFirst ) * sizeof *m_pFiles );
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

	ZeroMemory( m_pSizeMap, MAP_SIZE * sizeof *m_pSizeMap );
	ZeroMemory( m_pMapSHA1, MAP_SIZE * sizeof *m_pMapSHA1 );
	ZeroMemory( m_pMapTiger, MAP_SIZE * sizeof *m_pMapTiger );
	ZeroMemory( m_pMapED2K, MAP_SIZE * sizeof *m_pMapED2K );
	ZeroMemory( m_pMapBTH, MAP_SIZE * sizeof *m_pMapBTH );
	ZeroMemory( m_pMapMD5, MAP_SIZE * sizeof *m_pMapMD5 );

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
	
	return m_pSelectedFiles.GetHead();
}

CQueryHit* CMatchList::GetSelectedHit() const
{
	if ( m_pSelectedHits.GetCount() != 1 )
	{
		if ( m_pSelectedFiles.GetCount() != 1 ) return NULL;
		CMatchFile* pFile = m_pSelectedFiles.GetHead();
		return ( pFile->m_nFiltered == 1 ) ? pFile->GetHits() : NULL;
	}
	
	return m_pSelectedHits.GetHead();
}

INT_PTR CMatchList::GetSelectedCount() const
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
		bChanged = (*pLoop)->ClearSelection();
		if ( bChanged )
		{
			UpdateRange( nCount, nCount );
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
	if ( m_bFilterDRM )			Settings.Search.FilterMask |= ( 1 << 6 );
	if ( m_bFilterAdult	)		Settings.Search.FilterMask |= ( 1 << 7 );
	if ( m_bFilterSuspicious )	Settings.Search.FilterMask |= ( 1 << 8 );
	
	if ( m_pszFilter ) delete [] m_pszFilter;
	m_pszFilter = NULL;
	
	if ( m_sFilter.GetLength() )
	{
		LPCTSTR pszPtr = m_sFilter;
		BOOL bQuote = FALSE;
		BOOL bNot = FALSE;
		int nWordLen = 3;

		CList< CString > pWords;
		
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
		if ( ( nItems = (*pLoop)->Filter() ) != 0 )
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

	// Global adult filter and Local adult filter
	if ( Settings.Search.AdultFilter || m_bFilterAdult )
	{
		if ( AdultFilter.IsHitAdult( pHit->m_sName ) )
			return FALSE;
	}
	
	return ( pHit->m_bFiltered = TRUE );
}

//////////////////////////////////////////////////////////////////////
// CMatchList schema selection

void CMatchList::SelectSchema(CSchema* pSchema, CList< CSchemaMember* >* pColumns)
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
		m_pColumns[ m_nColumns++ ] = pColumns->GetNext( pos );
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
		if ( m_pFiles[ nFile ] ) m_pFiles[ nFile ]->ClearNew();
	}
	
	m_bNew = TRUE;
}

//////////////////////////////////////////////////////////////////////
// CMatchList serialize

void CMatchList::Serialize(CArchive& ar)
{
	int nVersion = 14;
	
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
		ar << m_bFilterDRM;
		ar << m_bFilterAdult;
		ar << m_bFilterSuspicious;
		ar << FALSE; // Temp Placeholder

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

		if ( nVersion >= 12 )
		{
			BOOL bTemp;
			ar >> m_bFilterDRM;
			ar >> m_bFilterAdult;
			ar >> m_bFilterSuspicious;
			ar >> bTemp; // Temp Placeholder
		}
		
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
		
		m_nFiles = m_nBuffer = static_cast< DWORD >( ar.ReadCount() );
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
			
			if ( pFile->m_oSHA1 )
			{
				pMap = m_pMapSHA1 + pFile->m_oSHA1[ 0 ];
				pFile->m_pNextSHA1 = *pMap;
				*pMap = pFile;
			}
			if ( pFile->m_oTiger )
			{
				pMap = m_pMapTiger + pFile->m_oTiger[ 0 ];
				pFile->m_pNextTiger = *pMap;
				*pMap = pFile;
			}
			if ( pFile->m_oED2K )
			{
				pMap = m_pMapED2K + ( pFile->m_oED2K[ 0 ] );
				pFile->m_pNextED2K = *pMap;
				*pMap = pFile;
			}
			if ( pFile->m_oBTH )
			{
				pMap = m_pMapBTH + ( pFile->m_oBTH[ 0 ] );
				pFile->m_pNextBTH = *pMap;
				*pMap = pFile;
			}
			if ( pFile->m_oMD5 )
			{
				pMap = m_pMapMD5 + ( pFile->m_oMD5[ 0 ] );
				pFile->m_pNextMD5 = *pMap;
				*pMap = pFile;
			}
		}
		
		Filter();
	}
}


//////////////////////////////////////////////////////////////////////
// CMatchFile construction

CMatchFile::CMatchFile(CMatchList* pList, CQueryHit* pHit) :
	m_pList			( pList ),
	m_pHits			( NULL ),
	m_pBest			( NULL ),
	m_nTotal		( NULL ),
	m_nFiltered		( 0 ),
	m_nSources		( 0 ),	
	m_pNextSize		( NULL ),
	m_pNextSHA1		( NULL ),
	m_pNextTiger	( NULL ),
	m_pNextED2K		( NULL ),
	m_pNextBTH		( NULL ),
	m_pNextMD5		( NULL ),
	m_bBusy			( TS_UNKNOWN ),
	m_bPush			( TS_UNKNOWN ),
	m_bStable		( TS_UNKNOWN ),
	m_bPreview		( FALSE ),
	m_nSpeed		( 0 ),
	m_nRating		( 0 ),
	m_nRated		( 0 ),
	m_bDRM			( FALSE ),
	m_bSuspicious	( FALSE ),
	m_bCollection	( FALSE ),
	m_bTorrent		( FALSE ),	
	m_bExpanded		( Settings.Search.ExpandMatches ),
	m_bSelected		( FALSE ),
	m_bExisting		( TS_UNKNOWN ),
	m_bDownload		( FALSE ),
	m_bNew			( FALSE ),
	m_bOneValid		( FALSE ),
	m_nShellIndex	( -1 ),
	m_pColumns		( NULL ),
	m_nColumns		( 0 ),
	m_pPreview		( NULL ),
	m_nPreview		( 0 ),
	m_pTime			( CTime::GetCurrentTime() )
{
	// TODO: Change to SIZE_UNKNOWN without the size
	m_nSize			= ( pHit && pHit->m_bSize ) ? pHit->m_nSize : 0;
	m_sSize			= Settings.SmartVolume( m_nSize, FALSE );	

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

void CMatchFile::RefreshStatus()
{
	CMap< CString, LPCTSTR, DWORD, DWORD > oNameRatings;
	DWORD nBestVote = 0;
	for ( CQueryHit* pHit = m_pHits ; pHit ; pHit = pHit->m_pNext )
	{
		DWORD nVote = 0;
		oNameRatings.Lookup( pHit->m_sName, nVote );
		oNameRatings [pHit->m_sName] = ++nVote;
		if ( nVote > nBestVote )
		{
			nBestVote = nVote;
			m_sName = pHit->m_sName;
			m_sURL = pHit->m_sURL;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CMatchFile add

BOOL CMatchFile::Add(CQueryHit* pHit, BOOL bForce)
{
	if ( m_nSize == 0 && pHit->m_bSize )
	{
		m_nSize = pHit->m_nSize;
	}
	else if ( m_nSize != 0 && pHit->m_bSize && m_nSize != pHit->m_nSize )
	{
		return FALSE;
	}
	
	if ( ! bForce )
	{
		if ( m_oSHA1 && ( pHit->m_oSHA1 || Settings.General.HashIntegrity ) )
		{
			if ( !pHit->m_oSHA1 ) return FALSE;
			if ( validAndUnequal( m_oSHA1, pHit->m_oSHA1 ) ) return FALSE;
			bForce = TRUE;
		}
		else if ( !m_oSHA1 && pHit->m_oSHA1 && Settings.General.HashIntegrity && m_pHits )
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
	
	if ( ! m_oSHA1 && pHit->m_oSHA1 )
	{
		m_oSHA1 = pHit->m_oSHA1;
	}
	
	if ( ! m_oTiger && pHit->m_oTiger )
	{
		m_oTiger = pHit->m_oTiger;
	}
	
	if ( ! m_oED2K && pHit->m_oED2K )
	{
		m_oED2K = pHit->m_oED2K;
	}

	if ( ! m_oBTH && pHit->m_oBTH )
	{
		m_oBTH = pHit->m_oBTH;
	}

	if ( ! m_oMD5 && pHit->m_oMD5 )
	{
		m_oMD5 = pHit->m_oMD5;
	}
	
	if ( ! m_bDownload && GetLibraryStatus() == TS_UNKNOWN && ( m_oSHA1 || m_oTiger || m_oED2K || m_oBTH || m_oMD5 ) )
	{
		CSingleLock pLock2( &Transfers.m_pSection );
		
		if ( pLock2.Lock( 50 ) )
		{
			if ( m_oSHA1 && Downloads.FindBySHA1( m_oSHA1 ) != NULL )
			{
				m_bDownload = TRUE;
			}
			else if ( m_oTiger && Downloads.FindByTiger( m_oTiger ) != NULL )
			{
				m_bDownload = TRUE;
			}
			else if ( m_oED2K && Downloads.FindByED2K( m_oED2K ) != NULL )
			{
				m_bDownload = TRUE;
			}
			else if ( m_oBTH && Downloads.FindByBTH( m_oBTH ) != NULL )
			{
				m_bDownload = TRUE;
			}
			else if ( m_oMD5 && Downloads.FindByMD5( m_oMD5 ) != NULL )
			{
				m_bDownload = TRUE;
			}
		}
	}
	
	if ( pHit->m_bFiltered ) Added( pHit );
	
	if ( ! m_bOneValid && ! pHit->m_bBogus && pHit->m_bMatched ) m_bOneValid = TRUE;
	
	RefreshStatus();

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
	m_bSuspicious	= FALSE;
	m_bCollection	= FALSE;
	m_bTorrent		= FALSE;
	
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
	if ( m_pList->m_bFilterLocal && GetLibraryStatus() == TS_FALSE ) return 0;
	if ( m_pList->m_bFilterDRM && m_bDRM ) return 0;
	if ( m_pList->m_bFilterSuspicious && m_bSuspicious ) return 0;

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
	
	m_bCollection	|= ( pHit->m_bCollection && ! pHit->m_bBogus );
	
	if ( pHit->m_nRating )
	{
		m_nRating += pHit->m_nRating;
		m_nRated ++;
	}
	
	if ( m_nShellIndex == -1 )
	{
		m_nShellIndex = ShellIcons.Get( pHit->m_sName, 16 );
	}

	BOOL bSchema;
	
	if ( m_pList->m_pSchema &&
		 ( bSchema = m_pList->m_pSchema->CheckURI( pHit->m_sSchemaURI ) || pHit->m_oSHA1 ) != FALSE )
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
				if ( pHit->m_oSHA1 )
				{
					m_pColumns[ nCount ] = m_oSHA1.toString();
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

	// cross-packet spam filtering
	DWORD nBogusCount = 0;
	DWORD nTotal = 0;
	for ( CQueryHit* pFileHits = m_pHits; pFileHits ; 
			pFileHits = pFileHits->m_pNext, nTotal++ )
	{
		if ( pFileHits->m_pNext && validAndEqual( pFileHits->m_oClientID, pFileHits->m_pNext->m_oClientID ) )
			pFileHits->m_bBogus = TRUE;
		if ( pFileHits->m_bBogus )
			nBogusCount++;
	}
	
	// Mark/unmark a file as suspicious depending on the percentage of the spam hits
	m_bSuspicious = (float)nBogusCount / nTotal > Settings.Search.SpamFilterThreshold / 100.0f;

	// Unshared files are suspicious. (A user is assumed to want to exclude these entirely)
	if ( CLibrary::IsBadFile( m_sName ) )
	{
		m_bSuspicious = TRUE;
	}

	// Get extention
	if ( int nExt = pHit->m_sName.ReverseFind( _T('.') ) + 1 )
	{
		LPCTSTR pszExt = (LPCTSTR)pHit->m_sName + nExt;

		// Set torrent bool
		if ( ( _tcsicmp( pszExt, _T("torrent") ) == 0 ) ) m_bTorrent = TRUE;

		// Check if file is suspicious
		if ( ! m_bSuspicious )
		{
			// These are basically always viral or useless
			if ( ( _tcsicmp( pszExt, _T("vbs") ) == 0 ) ||
				 ( _tcsicmp( pszExt, _T("lnk") ) == 0 ) ||
				 ( _tcsicmp( pszExt, _T("pif") ) == 0 ) )
			{
				m_bSuspicious = TRUE;
			}

			// Basic viral check. User still needs a virus scanner, but this may help. ;)
			if ( ( _tcsicmp( pszExt, _T("exe") ) == 0 ) ||
				 ( _tcsicmp( pszExt, _T("com") ) == 0 ) )
			{
				if ( m_nSize < 128 * 1024 ) 
				{
					// It's really likely to be viral.
					m_bSuspicious = TRUE;
				}
			}

			// Really common spam types
			if ( ( _tcsicmp( pszExt, _T("wmv") ) == 0 ) ||
				 ( _tcsicmp( pszExt, _T("wma") ) == 0 ) )
			{
				if ( m_nSize < 256 * 1024 ) 
				{
					// A movie file this small is very odd.
					m_bSuspicious = TRUE;
				}
			}

			// ZIP/RAR spam
			if ( ( _tcsicmp( pszExt, _T("zip") ) == 0 ) ||
				 ( _tcsicmp( pszExt, _T("rar") ) == 0 ) )
			{
				if ( m_nSize < 128 * 1024 ) 
				{
					m_bSuspicious = TRUE;
				}
				else if ( ( m_nSize < 512 * 1024 ) && ( pHit->m_bExactMatch ) )
				{
					m_bSuspicious = TRUE;
				}
			}
		}

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
		x = _tcsicoll( m_sName, pFile->m_sName );
		if ( ! x ) return 0;
		return x > 0 ? 1 : -1;

	case MATCH_COL_TYPE:
		{
			LPCTSTR pszType1 = _tcsrchr( m_sName, '.' );
			LPCTSTR pszType2 = _tcsrchr( pFile->m_sName, '.' );
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

	case MATCH_COL_TIME:
		return m_pTime == pFile->m_pTime ? 0 : ( m_pTime > pFile->m_pTime ? 1 : -1 );
		
	case MATCH_COL_SPEED:
		x = m_nFiltered ? m_nSpeed / m_nFiltered : 0;
		y = pFile->m_nFiltered ? pFile->m_nSpeed / pFile->m_nFiltered : 0;
		return x == y ? 0 : ( x > y ? 1 : -1 );
	case MATCH_COL_COUNTRY:
		{
			LPCTSTR pszType1 = ( m_nFiltered == 1 ) ? (LPCTSTR)m_pHits->m_sCountry : NULL;
			LPCTSTR pszType2 = ( pFile->m_nFiltered == 1 ) ? (LPCTSTR)pFile->m_pHits->m_sCountry : NULL;
			if ( ! pszType1 ) return ( pszType2 ? -1 : 0 );
			if ( ! pszType2 ) return 1;
			x = _tcsicmp( pszType1, pszType2 );
			if ( ! x ) return 0;
			return x > 0 ? 1 : -1;
		}
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
	
	if ( m_oSHA1 && m_oTiger )
	{
		strURN	= _T("urn:bitprint:")
				+ m_oSHA1.toString() + '.'
				+ m_oTiger.toString();
	}
	else if ( m_oSHA1 )
	{
		strURN = m_oSHA1.toUrn();
	}
	else if ( m_oTiger )
	{
		strURN = m_oTiger.toUrn();
	}
	else if ( m_oED2K )
	{
		strURN = m_oED2K.toUrn();
	}
	else if ( m_oBTH )
	{
		strURN = m_oBTH.toUrn();
	}
	else if ( m_oMD5 )
	{
		strURN = m_oMD5.toUrn();
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
        SerializeOut( ar, m_oSHA1 );
        SerializeOut( ar, m_oTiger );
        SerializeOut( ar, m_oED2K );
		SerializeOut( ar, m_oBTH );
		SerializeOut( ar, m_oMD5 );

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
		CArray< CQueryHit* > pHits;
		
        CQueryHit* pHit = m_pHits;
		for ( ; pHit ; pHit = pHit->m_pNext )
		{
			pHits.Add( pHit );
		}
		
		for ( int nHit = m_nTotal - 1 ; nHit >= 0 ; nHit-- )
		{
			pHit = pHits.GetAt( nHit );
			pHit->Serialize( ar, nVersion );
		}

		ar << m_pTime;
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
        SerializeIn( ar, m_oSHA1, nVersion );
        SerializeIn( ar, m_oTiger, nVersion );
        SerializeIn( ar, m_oED2K, nVersion );
		
		if ( nVersion >= 13 )
		{
			SerializeIn( ar, m_oBTH, nVersion  );
			SerializeIn( ar, m_oMD5, nVersion  );
		}

		ar >> m_bBusy;
		ar >> m_bPush;
		ar >> m_bStable;
		ar >> m_nSpeed;
		ar >> m_sSpeed;
		ar >> m_bExpanded;
		ar >> m_bExisting;
		ar >> m_bDownload;
		ar >> m_bOneValid;
		
		if ( ( m_nPreview = static_cast< DWORD >( ar.ReadCount() ) ) != 0 )
		{
			m_pPreview = new BYTE[ m_nPreview ];
			ReadArchive( ar, m_pPreview, m_nPreview );
		}
		
		m_nTotal = static_cast< DWORD >( ar.ReadCount() );
		
		for ( int nCount = m_nTotal ; nCount > 0 ; nCount-- )
		{
			CQueryHit* pNext = new CQueryHit( PROTOCOL_NULL );
			pNext->m_pNext = m_pHits;
			m_pHits = pNext;
			m_pHits->Serialize( ar, nVersion );
		}
		
		if ( nVersion >= 14 )
		{
			ar >> m_pTime;
		}

		RefreshStatus();
	}
}

CQueryHit* CMatchFile::GetHits() const
{
	return m_pHits;
}

CQueryHit*	CMatchFile::GetBest() const
{
	return m_pBest;
}

/*int CMatchFile::GetRating() const
{
	int nRating = 0;

	if ( m_bPush != TS_TRUE ) nRating += 4;
	if ( m_bBusy != TS_TRUE ) nRating += 2;
	if ( m_bStable == TS_TRUE ) nRating ++;

	return nRating;
}*/

DWORD CMatchFile::GetBogusHitsCount() const
{
	DWORD nBogusCount = 0;
	for ( CQueryHit* pHits = m_pHits; pHits ; pHits = pHits->m_pNext )
	{
		if ( pHits->m_bBogus )
			nBogusCount++;
	}
	return nBogusCount;
}

DWORD CMatchFile::GetTotalHitsCount() const
{
	DWORD nTotalCount = 0;
	for ( CQueryHit* pHits = m_pHits; pHits ; pHits = pHits->m_pNext, nTotalCount++ );
	return nTotalCount;
}

DWORD CMatchFile::GetTotalHitsSpeed() const
{
	DWORD nSpeed = 0;
	for ( CQueryHit* pHit = m_pHits ; pHit ; pHit = pHit->m_pNext )
	{
		nSpeed += pHit->m_nSpeed;
	}
	return nSpeed;
}

void CMatchFile::AddHitsToDownload(CDownload* pDownload, BOOL bForce) const
{
	// Best goes first if forced
	if ( bForce && m_pBest != NULL )
	{
		pDownload->AddSourceHit( m_pBest, TRUE );
	}

	for ( CQueryHit* pHit = m_pHits ; pHit ; pHit = pHit->m_pNext )
	{
		if ( bForce )
		{
			// Best already added
			if ( pHit != m_pBest )
			{
				pDownload->AddSourceHit( pHit, TRUE );
			}
		}
		else
		{
			pDownload->AddSourceHit( pHit );
		}

		// Send any reviews to the download, so they can be viewed later
		if ( pHit->m_nRating || ! pHit->m_sComments.IsEmpty() )
		{
			pDownload->AddReview( &pHit->m_pAddress, 2, pHit->m_nRating, pHit->m_sNick, pHit->m_sComments );
		}
	}
}

void CMatchFile::AddHitsToXML(CXMLElement* pXML) const
{
	for ( CQueryHit* pHit = m_pHits ; pHit ; pHit = pHit->m_pNext )
	{
		if ( pHit->m_pXML != NULL )
		{
			for ( POSITION pos = pHit->m_pXML->GetAttributeIterator() ; pos ; )
			{
				CXMLAttribute* pAttribute = pHit->m_pXML->GetNextAttribute( pos );
				pXML->AddAttribute( pAttribute->GetName(), pAttribute->GetValue() );
			}
		}
	}
}

CSchema* CMatchFile::GetHitsSchema() const
{
	CSchema* pSchema = NULL;
	for ( CQueryHit* pHit = m_pHits ; pHit ; pHit = pHit->m_pNext )
	{
		pSchema = SchemaCache.Get( pHit->m_sSchemaURI );
		if ( pSchema ) break;
	}
	return pSchema;
}

CSchema* CMatchFile::AddHitsToMetadata(CMetaList& oMetadata) const
{
	CSchema* pSchema = GetHitsSchema();
	if ( pSchema )
	{
		oMetadata.Setup( pSchema );

		for ( CQueryHit* pHit = m_pHits ; pHit ; pHit = pHit->m_pNext )
		{
			if ( pHit->m_pXML && pSchema->CheckURI( pHit->m_sSchemaURI ) )
			{
				oMetadata.Combine( pHit->m_pXML );
			}
		}
	}
	return pSchema;
}

BOOL CMatchFile::AddHitsToPreviewURLs(CList<CString>& oPreviewURLs) const
{
	BOOL bCanPreview = FALSE;
	for ( CQueryHit* pHit = m_pHits ; pHit ; pHit = pHit->m_pNext )
	{
		if ( pHit->m_oSHA1 && pHit->m_bPush == TS_FALSE )
		{
			if ( pHit->m_bPreview )
			{
				oPreviewURLs.AddTail( pHit->m_sPreview );
				bCanPreview = TRUE;
			}
#ifdef _DEBUG
			else if (	_tcsistr( pHit->m_sName, _T(".mpg") ) ||
						_tcsistr( pHit->m_sName, _T(".mpeg") ) ||
						_tcsistr( pHit->m_sName, _T(".avi") ) ||
						_tcsistr( pHit->m_sName, _T(".jpg") ) ||
						_tcsistr( pHit->m_sName, _T(".jpeg") ) )
			{
				CString strURL;
				strURL.Format( _T("http://%s:%i/gnutella/preview/v1?%s"),
					(LPCTSTR)CString( inet_ntoa( pHit->m_pAddress ) ), pHit->m_nPort,
					(LPCTSTR)pHit->m_oSHA1.toUrn() );
				oPreviewURLs.AddTail( strURL );
				bCanPreview = TRUE;
			}
#endif
		}
	}
	return bCanPreview;
}

void CMatchFile::AddHitsToReviews(CList < Review* >& oReviews) const
{
	for ( CQueryHit* pHit = m_pHits ; pHit ; pHit = pHit->m_pNext )
	{
		if ( pHit->m_nRating > 0 || pHit->m_sComments.GetLength() > 0 )
		{
			oReviews.AddTail( new Review( pHit->m_oClientID,
				&pHit->m_pAddress, pHit->m_sNick, pHit->m_nRating, pHit->m_sComments ) );
		}
	}
}

void CMatchFile::SetBogus( BOOL bBogus )
{
	for ( CQueryHit* pHit = m_pHits ; pHit ; pHit = pHit->m_pNext )
	{
		pHit->m_bBogus = bBogus;
	}
}

BOOL CMatchFile::ClearSelection()
{
	BOOL bChanged = FALSE;
	if ( m_bSelected )
	{
		m_bSelected = FALSE;
		bChanged = TRUE;
	}
	for ( CQueryHit* pHit = m_pHits ; pHit ; pHit = pHit->m_pNext )
	{
		if ( pHit->m_bSelected )
		{
			pHit->m_bSelected = FALSE;
			bChanged = TRUE;
		}
	}
	return bChanged;
}

BOOL CMatchFile::IsValid() const
{
	return ( m_pBest != NULL );
}

DWORD CMatchFile::GetBestPartial() const
{
	return ( m_pBest ? m_pBest->m_nPartial : 0 );
}

int CMatchFile::GetBestRating() const
{
	return ( m_pBest ? m_pBest->m_nRating : 0 );
}

IN_ADDR CMatchFile::GetBestAddress() const
{
	const IN_ADDR foo = { 0xff, 0xff, 0xff, 0xff };
	return ( m_pBest ? m_pBest->m_pAddress : foo );
}

LPCTSTR CMatchFile::GetBestVendorName() const
{
	return ( ( m_pBest && m_pBest->m_pVendor) ? m_pBest->m_pVendor->m_sName : _T("") );
}

LPCTSTR CMatchFile::GetBestCountry() const
{
	return ( m_pBest ? m_pBest->m_sCountry : _T("") );
}

LPCTSTR CMatchFile::GetBestSchemaURI() const
{
	return ( m_pBest ? m_pBest->m_sSchemaURI : _T("") );
}

TRISTATE CMatchFile::GetBestMeasured() const
{
	return ( m_pBest ? m_pBest->m_bMeasured : TS_UNKNOWN );
}

BOOL CMatchFile::GetBestBrowseHost() const
{
	return ( m_pBest && m_pBest->m_bBrowseHost );
}

void CMatchFile::GetPartialTip(CString& sPartial) const
{
	if ( m_nFiltered == 1 && m_pBest && m_pBest->m_nPartial )
	{
		CString strFormat;
		LoadString( strFormat, IDS_TIP_PARTIAL );
		sPartial.Format( strFormat, 100.0f * (float)m_pBest->m_nPartial / (float)m_nSize );
	}
	else
	{
		sPartial.Empty();
	}
}

void CMatchFile::GetQueueTip(CString& sQueue) const
{
	if ( m_nFiltered == 1 && m_pBest && m_pBest->m_nUpSlots )
	{
		CString strFormat;
		LoadString( strFormat, IDS_TIP_QUEUE );
		sQueue.Format( strFormat, m_pBest->m_nUpSlots,
			max( 0, m_pBest->m_nUpQueue - m_pBest->m_nUpSlots ) );
	}
	else
	{
		sQueue.Empty();
	}
}

void CMatchFile::GetUser(CString& sUser) const
{
	if ( m_nFiltered == 1 && m_pBest )
	{
		if ( m_pBest->m_sNick.GetLength() )
		{
			sUser.Format( _T("%s (%s - %s)"),
				(LPCTSTR)m_pBest->m_sNick,
				(LPCTSTR)CString( inet_ntoa( m_pBest->m_pAddress ) ),
				(LPCTSTR)m_pBest->m_pVendor->m_sName );
		}
		else
		{
			if ( ( m_pBest->m_nProtocol == PROTOCOL_ED2K ) && ( m_pBest->m_bPush == TS_TRUE ) )
			{
				sUser.Format( _T("%lu@%s - %s"), m_pBest->m_oClientID.begin()[2], 
					(LPCTSTR)CString( inet_ntoa( (IN_ADDR&)*m_pBest->m_oClientID.begin() ) ),
					(LPCTSTR)m_pBest->m_pVendor->m_sName );
			}
			else
			{
				sUser.Format( _T("%s - %s"),
					(LPCTSTR)CString( inet_ntoa( m_pBest->m_pAddress ) ),
					(LPCTSTR)m_pBest->m_pVendor->m_sName );
			}
		}
	}
	else
	{
		sUser.Empty();
	}
}

void CMatchFile::GetStatusTip( CString& sStatus, COLORREF& crStatus)
{
	sStatus.Empty();

	if ( GetLibraryStatus() != TS_UNKNOWN )
	{
		CLibraryFile* pExisting = NULL;

		CQuickLock oLock( Library.m_pSection );
		if ( pExisting == NULL && m_oSHA1 )
			pExisting = LibraryMaps.LookupFileBySHA1( m_oSHA1 );
		if ( pExisting == NULL && m_oTiger )
			pExisting = LibraryMaps.LookupFileByTiger( m_oTiger );
		if ( pExisting == NULL && m_oED2K )
			pExisting = LibraryMaps.LookupFileByED2K( m_oED2K );
		if ( pExisting == NULL && m_oBTH )
			pExisting = LibraryMaps.LookupFileByBTH( m_oBTH );
		if ( pExisting == NULL && m_oMD5 )
			pExisting = LibraryMaps.LookupFileByMD5( m_oMD5 );
		
		if ( pExisting != NULL )
		{
			if ( pExisting->IsAvailable() )
			{
				LoadString( sStatus, IDS_TIP_EXISTS_LIBRARY );
				crStatus = CoolInterface.m_crTextStatus ;
			}
			else
			{
				LoadString( sStatus, IDS_TIP_EXISTS_DELETED );
				crStatus = CoolInterface.m_crTextAlert ;

				if ( pExisting->m_sComments.GetLength() )
				{
					sStatus += L" (";
					sStatus += pExisting->m_sComments;
					sStatus.Replace( L"\r\n", L"; " );

					int nLen = sStatus.GetLength();
					if ( nLen > 150 )
					{
						// Truncate string including the last word 
						// but no more than 150 characters plus punctuation
						CString str( sStatus.Left( 150 ) );
						if ( IsCharacter( sStatus.GetAt( 151 ) ) )
						{
							nLen = str.ReverseFind( ' ' );
							sStatus = nLen == -1 ? str : str.Left( nLen );
						}
						sStatus += L"\x2026)";
					}
					else
						sStatus.Append( L")" );
				}
			}
		}
	}
	else if ( m_bDownload || m_pBest->m_bDownload )
	{
		LoadString( sStatus, IDS_TIP_EXISTS_DOWNLOAD );
		crStatus = CoolInterface.m_crSearchQueued ;
	}
	else if ( m_pBest->m_bBogus || ! m_bOneValid )
	{
		LoadString( sStatus, IDS_TIP_BOGUS );
		crStatus = CoolInterface.m_crTextAlert ;
	}
}

TRISTATE CMatchFile::GetLibraryStatus()
{
	// TODO: Is some cache needed?

	CSingleLock pLock( &Library.m_pSection );
	if (  pLock.Lock( 100 ) )
	{
		CLibraryFile* pExisting = NULL;
		if ( ( m_oSHA1 && ( pExisting = LibraryMaps.LookupFileBySHA1( m_oSHA1 ) ) != NULL ) ||
			 ( m_oTiger && ( pExisting = LibraryMaps.LookupFileByTiger( m_oTiger ) ) != NULL ) ||
			 ( m_oED2K && ( pExisting = LibraryMaps.LookupFileByED2K( m_oED2K ) ) != NULL ) ||
			 ( m_oBTH && ( pExisting = LibraryMaps.LookupFileByBTH( m_oBTH ) ) != NULL ) ||
			 ( m_oMD5 && ( pExisting = LibraryMaps.LookupFileByMD5( m_oMD5 ) ) != NULL ) )
		{
			m_bExisting = pExisting->IsAvailable() ? TS_FALSE : TS_TRUE;
		}
		else
		{
			if ( m_bExisting == TS_FALSE )
				m_bExisting = TS_TRUE;
		}
		pLock.Unlock();
	}
	return m_bExisting;
}
