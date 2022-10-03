//
// LibraryMaps.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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
#include "Library.h"
#include "LibraryMaps.h"
#include "Application.h"
#include "QuerySearch.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CLibraryMaps, CComObject)

BEGIN_INTERFACE_MAP(CLibraryMaps, CComObject)
	INTERFACE_PART(CLibraryMaps, IID_ILibraryFiles, LibraryFiles)
END_INTERFACE_MAP()

CLibraryMaps LibraryMaps;


//////////////////////////////////////////////////////////////////////
// CLibraryMaps construction

CLibraryMaps::CLibraryMaps()
	: m_pSHA1Map	()
	, m_pTigerMap	()
	, m_pED2KMap	()
	, m_pBTHMap		()
	, m_pMD5Map		()
	, m_nNextIndex	( 4 )
	, m_nFiles		( 0 )
	, m_nVolume		( 0 )
{
	EnableDispatch( IID_ILibraryFiles );
}

CLibraryMaps::~CLibraryMaps()
{
}

//////////////////////////////////////////////////////////////////////
// CLibraryMaps file list

POSITION CLibraryMaps::GetFileIterator() const
{
	return m_pIndexMap.GetStartPosition();
}

CLibraryFile* CLibraryMaps::GetNextFile(POSITION& pos) const
{
	DWORD pIndex;
	CLibraryFile* pFile;
	m_pIndexMap.GetNextAssoc( pos, pIndex, pFile );
	return pFile;
}

void CLibraryMaps::GetStatistics(DWORD* pnFiles, QWORD* pnVolume)
{
	if ( pnFiles ) *pnFiles = m_nFiles;
	if ( pnVolume ) *pnVolume = m_nVolume >> 10;
}

//////////////////////////////////////////////////////////////////////
// CLibraryMaps lookup file by index

CLibraryFile* CLibraryMaps::LookupFile(DWORD nIndex, BOOL bSharedOnly, BOOL bAvailableOnly) const
{
	if ( ! nIndex ) return NULL;

	CQuickLock oLock( Library.m_pSection );

	CLibraryFile* pFile;
	return ( m_pIndexMap.Lookup( nIndex, pFile ) && pFile->CheckFileAttributes(
		SIZE_UNKNOWN, bSharedOnly, bAvailableOnly ) ) ? pFile : NULL;
}

//////////////////////////////////////////////////////////////////////
// CLibraryMaps lookup file by name and/or path

CLibraryFile* CLibraryMaps::LookupFileByName(LPCTSTR pszName, QWORD nSize, BOOL bSharedOnly, BOOL bAvailableOnly) const
{
	ASSERT_VALID( this );
	ASSERT( pszName && *pszName );

	CQuickLock oLock( Library.m_pSection );

	CLibraryFile* pFile;
	return ( m_pNameMap.Lookup( PathFindFileName( pszName ), pFile ) && pFile->CheckFileAttributes(
		nSize, bSharedOnly, bAvailableOnly ) ) ? pFile : NULL;
}

CLibraryFile* CLibraryMaps::LookupFileByPath(LPCTSTR pszPath, BOOL bSharedOnly, BOOL bAvailableOnly) const
{
	ASSERT_VALID( this );
	ASSERT( pszPath && *pszPath );

	CQuickLock oLock( Library.m_pSection );

	CLibraryFile* pFile;
	return ( m_pPathMap.Lookup( pszPath, pFile ) && pFile->CheckFileAttributes(
		SIZE_UNKNOWN, bSharedOnly, bAvailableOnly ) ) ? pFile : NULL;
}

//////////////////////////////////////////////////////////////////////
// CLibraryMaps lookup file by URN

CLibraryFile* CLibraryMaps::LookupFileByURN(LPCTSTR pszURN, BOOL bSharedOnly, BOOL bAvailableOnly) const
{
	ASSERT_VALID( this );
	ASSERT( pszURN && *pszURN );

	CLibraryFile* pFile = NULL;
	Hashes::TigerHash oTiger;
	Hashes::Sha1Hash oSHA1;
	Hashes::Ed2kHash oED2K;
	Hashes::BtHash oBTH;
	Hashes::Md5Hash oMD5;

	if ( oSHA1.fromUrn( pszURN ) )
	{
		if ( ( pFile = LookupFileBySHA1( oSHA1, bSharedOnly, bAvailableOnly ) ) != NULL ) return pFile;
	}

	if ( oTiger.fromUrn( pszURN ) )
	{
		if ( ( pFile = LookupFileByTiger( oTiger, bSharedOnly, bAvailableOnly ) ) != NULL ) return pFile;
	}

	if ( oED2K.fromUrn( pszURN ) )
	{
		if ( ( pFile = LookupFileByED2K( oED2K, bSharedOnly, bAvailableOnly ) ) != NULL ) return pFile;
	}

	if ( oBTH.fromUrn( pszURN ) || oBTH.fromUrn< Hashes::base16Encoding >( pszURN ) )
	{
		if ( ( pFile = LookupFileByBTH( oBTH, bSharedOnly, bAvailableOnly ) ) != NULL ) return pFile;
	}

	if ( oMD5.fromUrn( pszURN ) )
	{
		if ( ( pFile = LookupFileByMD5( oMD5, bSharedOnly, bAvailableOnly ) ) != NULL ) return pFile;
	}

	return NULL;
}

CLibraryFile* CLibraryMaps::LookupFileByHash(const CShareazaFile* pFilter, BOOL bSharedOnly, BOOL bAvailableOnly) const
{
	CQuickLock oLock( Library.m_pSection );

	const CLibraryFile* pFile = NULL;

	if ( CFileList* pList = LookupFilesByHash( pFilter, bSharedOnly, bAvailableOnly ) )
	{
		pFile = pList->GetHead();
		delete pList;
	}

	return const_cast< CLibraryFile* >( pFile );
}

CFileList* CLibraryMaps::LookupFilesByHash(const CShareazaFile* pFilter, BOOL bSharedOnly, BOOL bAvailableOnly, int nMaximum) const
{
	CQuickLock oLock( Library.m_pSection );

	CFileList* pFiles = NULL;

	if ( pFilter->m_oSHA1 )
	{
		for ( CLibraryFile* pFile = m_pSHA1Map[ FILE_INDEX( pFilter->m_oSHA1 ) ] ;
			pFile ; pFile = pFile->m_pNextSHA1 )
		{
			if ( validAndEqual( pFile->m_oSHA1, pFilter->m_oSHA1 ) &&
				 *pFile == *pFilter &&
				 pFile->CheckFileAttributes( pFilter->m_nSize, bSharedOnly, bAvailableOnly ) )
			{
				if ( ! pFiles )
					pFiles = new CFileList;
				if ( pFiles->Find( pFile ) == NULL )
				{
					if ( bSharedOnly )
					{
						pFile->m_nHitsToday++;
						pFile->m_nHitsTotal++;
					}
					pFiles->AddTail( pFile );
					if ( nMaximum && pFiles->GetCount() >= nMaximum )
						break;
				}
			}
		}
		return pFiles;
	}
	else if ( pFilter->m_oED2K )
	{
		for ( CLibraryFile* pFile = m_pED2KMap[ FILE_INDEX( pFilter->m_oED2K ) ] ;
			pFile ; pFile = pFile->m_pNextED2K )
		{
			if ( validAndEqual( pFile->m_oED2K, pFilter->m_oED2K ) &&
				 *pFile == *pFilter &&
				 pFile->CheckFileAttributes( pFilter->m_nSize, bSharedOnly, bAvailableOnly ) )
			{
				if ( ! pFiles )
					pFiles = new CFileList;
				if ( pFiles->Find( pFile ) == NULL )
				{
					if ( bSharedOnly )
					{
						pFile->m_nHitsToday++;
						pFile->m_nHitsTotal++;
					}
					pFiles->AddTail( pFile );
					if ( nMaximum && pFiles->GetCount() >= nMaximum )
						break;
				}
			}
		}
		return pFiles;
	}
	else if ( pFilter->m_oTiger )
	{
		for ( CLibraryFile* pFile = m_pTigerMap[ FILE_INDEX( pFilter->m_oTiger ) ] ;
			pFile ; pFile = pFile->m_pNextTiger )
		{
			if ( validAndEqual( pFile->m_oTiger, pFilter->m_oTiger ) &&
				 *pFile == *pFilter &&
				 pFile->CheckFileAttributes( pFilter->m_nSize, bSharedOnly, bAvailableOnly ) )
			{
				if ( ! pFiles )
					pFiles = new CFileList;
				if ( pFiles->Find( pFile ) == NULL )
				{
					if ( bSharedOnly )
					{
						pFile->m_nHitsToday++;
						pFile->m_nHitsTotal++;
					}
					pFiles->AddTail( pFile );
					if ( nMaximum && pFiles->GetCount() >= nMaximum )
						break;
				}
			}
		}
		return pFiles;
	}
	else if ( pFilter->m_oMD5 )
	{
		for ( CLibraryFile* pFile = m_pMD5Map[ FILE_INDEX( pFilter->m_oMD5 ) ] ;
			pFile ; pFile = pFile->m_pNextMD5 )
		{
			if ( validAndEqual( pFile->m_oMD5, pFilter->m_oMD5 ) &&
				 *pFile == *pFilter &&
				 pFile->CheckFileAttributes( pFilter->m_nSize, bSharedOnly, bAvailableOnly ) )
			{
				if ( ! pFiles )
					pFiles = new CFileList;
				if ( pFiles->Find( pFile ) == NULL )
				{
					if ( bSharedOnly )
					{
						pFile->m_nHitsToday++;
						pFile->m_nHitsTotal++;
					}
					pFiles->AddTail( pFile );
					if ( nMaximum && pFiles->GetCount() >= nMaximum )
						break;
				}
			}
		}
		return pFiles;
	}
	else if ( pFilter->m_oBTH )
	{
		for ( CLibraryFile* pFile = m_pBTHMap[ FILE_INDEX( pFilter->m_oBTH ) ] ;
			pFile ; pFile = pFile->m_pNextBTH )
		{
			if ( validAndEqual( pFile->m_oBTH, pFilter->m_oBTH ) &&
				 *pFile == *pFilter &&
				 pFile->CheckFileAttributes( pFilter->m_nSize, bSharedOnly, bAvailableOnly ) )
			{
				if ( ! pFiles )
					pFiles = new CFileList;
				if ( pFiles->Find( pFile ) == NULL )
				{
					pFiles->AddTail( pFile );
					if ( bSharedOnly )
					{
						pFile->m_nHitsToday++;
						pFile->m_nHitsTotal++;
					}
					if ( nMaximum && pFiles->GetCount() >= nMaximum )
						break;
				}
			}
		}
		return pFiles;
	}
	else if ( pFilter->m_sName.GetLength() &&
		pFilter->m_nSize != SIZE_UNKNOWN && pFilter->m_nSize != 0 )
	{
		if ( CLibraryFile* pFile = LibraryMaps.LookupFileByName( pFilter->m_sName, pFilter->m_nSize, bSharedOnly, bAvailableOnly ) )
		{
			if ( ! pFiles )
				pFiles = new CFileList;
			if ( pFiles->Find( pFile ) == NULL )
			{
				if ( bSharedOnly )
				{
					pFile->m_nHitsToday++;
					pFile->m_nHitsTotal++;
				}
				pFiles->AddTail( pFile );
			}
		}
	}

	return pFiles;
}

//////////////////////////////////////////////////////////////////////
// CLibraryMaps lookup file by individual hash types

CLibraryFile* CLibraryMaps::LookupFileBySHA1(const Hashes::Sha1Hash& oSHA1, BOOL bSharedOnly, BOOL bAvailableOnly) const
{
	if ( !oSHA1 ) return NULL;

	CQuickLock oLock( Library.m_pSection );

	CLibraryFile* pFile = m_pSHA1Map[ FILE_INDEX( oSHA1 ) ];

	for ( ; pFile ; pFile = pFile->m_pNextSHA1 )
	{
		if ( validAndEqual( oSHA1, pFile->m_oSHA1 ) )
		{
			if ( pFile->CheckFileAttributes( SIZE_UNKNOWN, bSharedOnly, bAvailableOnly ) )
				return pFile;
		}
	}

	return NULL;
}

CLibraryFile* CLibraryMaps::LookupFileByTiger(const Hashes::TigerHash& oTiger, BOOL bSharedOnly, BOOL bAvailableOnly) const
{
	if ( !oTiger ) return NULL;

	CQuickLock oLock( Library.m_pSection );

	CLibraryFile* pFile = m_pTigerMap[ FILE_INDEX( oTiger ) ];

	for ( ; pFile ; pFile = pFile->m_pNextTiger )
	{
		if ( validAndEqual( oTiger, pFile->m_oTiger ) )
		{
			if ( pFile->CheckFileAttributes( SIZE_UNKNOWN, bSharedOnly, bAvailableOnly ) )
				return pFile;
		}
	}

	return NULL;
}

CLibraryFile* CLibraryMaps::LookupFileByED2K(const Hashes::Ed2kHash& oED2K, BOOL bSharedOnly, BOOL bAvailableOnly) const
{
	if ( !oED2K ) return NULL;

	CQuickLock oLock( Library.m_pSection );

	CLibraryFile* pFile = m_pED2KMap[ FILE_INDEX( oED2K ) ];

	for ( ; pFile ; pFile = pFile->m_pNextED2K )
	{
		if ( validAndEqual( oED2K, pFile->m_oED2K ) )
		{
			if ( pFile->CheckFileAttributes( SIZE_UNKNOWN, bSharedOnly, bAvailableOnly ) )
				return pFile;
		}
	}

	return NULL;
}

CLibraryFile* CLibraryMaps::LookupFileByBTH(const Hashes::BtHash& oBTH, BOOL bSharedOnly, BOOL bAvailableOnly) const
{
	if ( !oBTH ) return NULL;

	CQuickLock oLock( Library.m_pSection );

	CLibraryFile* pFile = m_pBTHMap[ FILE_INDEX( oBTH ) ];

	for ( ; pFile ; pFile = pFile->m_pNextBTH )
	{
		if ( validAndEqual( oBTH, pFile->m_oBTH ) )
		{
			if ( pFile->CheckFileAttributes( SIZE_UNKNOWN, bSharedOnly, bAvailableOnly ) )
				return pFile;
		}
	}

	return NULL;
}

CLibraryFile* CLibraryMaps::LookupFileByMD5(const Hashes::Md5Hash& oMD5, BOOL bSharedOnly, BOOL bAvailableOnly) const
{
	if ( !oMD5 ) return NULL;

	CQuickLock oLock( Library.m_pSection );

	CLibraryFile* pFile = m_pMD5Map[ FILE_INDEX( oMD5 ) ];

	for ( ; pFile ; pFile = pFile->m_pNextMD5 )
	{
		if ( validAndEqual( oMD5, pFile->m_oMD5 ) )
		{
			if ( pFile->CheckFileAttributes( SIZE_UNKNOWN, bSharedOnly, bAvailableOnly ) )
				return pFile;
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CLibraryMaps clear

void CLibraryMaps::Clear()
{
	while ( POSITION pos = GetFileIterator() ) delete GetNextFile( pos );

	ASSERT( m_pNameMap.IsEmpty() );
	ASSERT( m_pPathMap.IsEmpty() );
#ifdef _DEBUG
	for ( POSITION p = m_pPathMap.GetStartPosition() ; p ; )
	{
		CString k;
		CLibraryFile* v;
		m_pPathMap.GetNextAssoc( p, k, v );
		TRACE ( _T("m_pPathMap lost : %ls = 0x%08x\n"), (LPCTSTR)k, v );
	}
	for ( POSITION p = m_pNameMap.GetStartPosition(); p; )
	{
		CString k;
		CLibraryFile* v;
		m_pNameMap.GetNextAssoc( p, k, v );
		TRACE( _T( "m_pNameMap lost : %ls = 0x%08x\n" ), (LPCTSTR)k, v );
	}
	for ( int i = 0; i < FILE_HASH_SIZE; ++i )
	{
		ASSERT( m_pSHA1Map[ i ] == NULL );
		ASSERT( m_pTigerMap[ i ] == NULL );
		ASSERT( m_pED2KMap[ i ] == NULL );
		ASSERT( m_pBTHMap[ i ] == NULL );
		ASSERT( m_pMD5Map[ i ] == NULL );
	}
#endif

	ZeroMemory( m_pSHA1Map, sizeof( m_pSHA1Map ) );
	ZeroMemory( m_pTigerMap, sizeof( m_pTigerMap ) );
	ZeroMemory( m_pED2KMap, sizeof( m_pED2KMap ) );
	ZeroMemory( m_pBTHMap, sizeof( m_pBTHMap ) );
	ZeroMemory( m_pMD5Map, sizeof( m_pMD5Map ) );

	ASSERT( m_nFiles == 0 );
	m_nFiles  = 0;
	ASSERT( m_nVolume == 0 );
	m_nVolume = 0;
}

//////////////////////////////////////////////////////////////////////
// CLibraryMaps index manager

DWORD CLibraryMaps::AllocateIndex()
{
	while ( ( m_nNextIndex & 3 ) == 0 || LookupFile( m_nNextIndex ) ) m_nNextIndex++;
	return m_nNextIndex;
}

//////////////////////////////////////////////////////////////////////
// CLibraryMaps add a file to the maps

void CLibraryMaps::OnFileAdd(CLibraryFile* pFile)
{
	BOOL bSkipStats = FALSE;
	if ( pFile->m_nIndex )
	{
		if ( CLibraryFile* pOld = LookupFile( pFile->m_nIndex ) )
		{
			if ( pOld != pFile )
			{
				pFile->m_nIndex = AllocateIndex();
				m_pIndexMap.SetAt( pFile->m_nIndex, pFile );
			}
			else
			{
				bSkipStats = TRUE;
			}
		}
		else
		{
			m_pIndexMap.SetAt( pFile->m_nIndex, pFile );
		}
	}
	else
	{
		pFile->m_nIndex = AllocateIndex();
		m_pIndexMap.SetAt( pFile->m_nIndex, pFile );
	}

	if ( pFile->IsAvailable() && ! bSkipStats )
	{
		m_nVolume += pFile->m_nSize;
		m_nFiles ++;
	}

	// Prefer real files
	CLibraryFile* pDupNameFile;
	if ( ! m_pNameMap.Lookup( pFile->m_sName, pDupNameFile ) || ! pDupNameFile->IsAvailable() )
	{
		m_pNameMap.SetAt( pFile->m_sName, pFile );
	}

	if ( pFile->IsAvailable() )
	{
		m_pPathMap.SetAt( pFile->GetPath(), pFile );
	}
	else if ( m_pDeleted.Find( pFile ) == NULL )
	{
		m_pDeleted.AddTail( pFile );
	}

	if ( pFile->m_oSHA1 )
	{
		CLibraryFile** pHash = &m_pSHA1Map[ FILE_INDEX( pFile->m_oSHA1 ) ];
		pFile->m_pNextSHA1 = *pHash;
		*pHash = pFile;
	}

	if ( pFile->m_oTiger )
	{
		CLibraryFile** pHash = &m_pTigerMap[ FILE_INDEX( pFile->m_oTiger ) ];
		pFile->m_pNextTiger = *pHash;
		*pHash = pFile;
	}

	if ( pFile->m_oED2K )
	{
		CLibraryFile** pHash = &m_pED2KMap[ FILE_INDEX( pFile->m_oED2K ) ];
		pFile->m_pNextED2K = *pHash;
		*pHash = pFile;
	}

	if ( pFile->m_oBTH )
	{
		CLibraryFile** pHash = &m_pBTHMap[ FILE_INDEX( pFile->m_oBTH ) ];
		pFile->m_pNextBTH = *pHash;
		*pHash = pFile;
	}

	if ( pFile->m_oMD5 )
	{
		CLibraryFile** pHash = &m_pMD5Map[ FILE_INDEX( pFile->m_oMD5 ) ];
		pFile->m_pNextMD5 = *pHash;
		*pHash = pFile;
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryMaps remove a file from the maps

void CLibraryMaps::OnFileRemove(CLibraryFile* pFile)
{
	CLibraryFile* pOld = LookupFile( pFile->m_nIndex );
	if ( pOld == pFile )
	{
		VERIFY( m_pIndexMap.RemoveKey( pFile->m_nIndex ) );
		if ( pOld->IsAvailable() )
		{
			m_nFiles --;
			m_nVolume -= pFile->m_nSize;
		}
	}

	pOld = LookupFileByName( pFile->m_sName, pFile->m_nSize );
	if ( pOld == pFile ) VERIFY( m_pNameMap.RemoveKey( pFile->m_sName ) );

	if ( pFile->IsAvailable() )
	{
		const CString strPath( pFile->GetPath() );
		pOld = LookupFileByPath( strPath );
		if ( pOld == pFile ) VERIFY( m_pPathMap.RemoveKey( strPath ) );
	}

	if ( POSITION pos = m_pDeleted.Find( pFile ) )
		m_pDeleted.RemoveAt( pos );

	if ( pFile->m_oSHA1 )
	{
		CLibraryFile** pPrev = &m_pSHA1Map[ FILE_INDEX( pFile->m_oSHA1 ) ];

		for ( CLibraryFile* pOther = *pPrev ; pOther ; pOther = pOther->m_pNextSHA1 )
		{
			if ( pOther == pFile )
			{
				*pPrev = pOther->m_pNextSHA1;
				break;
			}
			pPrev = &pOther->m_pNextSHA1;
		}

		pFile->m_pNextSHA1 = NULL;
	}

	if ( pFile->m_oTiger )
	{
		CLibraryFile** pPrev = &m_pTigerMap[ FILE_INDEX( pFile->m_oTiger ) ];

		for ( CLibraryFile* pOther = *pPrev ; pOther ; pOther = pOther->m_pNextTiger )
		{
			if ( pOther == pFile )
			{
				*pPrev = pOther->m_pNextTiger;
				break;
			}
			pPrev = &pOther->m_pNextTiger;
		}

		pFile->m_pNextTiger = NULL;
	}

	if ( pFile->m_oED2K )
	{
		CLibraryFile** pPrev = &m_pED2KMap[ FILE_INDEX( pFile->m_oED2K ) ];

		for ( CLibraryFile* pOther = *pPrev ; pOther ; pOther = pOther->m_pNextED2K )
		{
			if ( pOther == pFile )
			{
				*pPrev = pOther->m_pNextED2K;
				break;
			}
			pPrev = &pOther->m_pNextED2K;
		}

		pFile->m_pNextED2K = NULL;
	}

	if ( pFile->m_oBTH )
	{
		CLibraryFile** pPrev = &m_pBTHMap[ FILE_INDEX( pFile->m_oBTH ) ];

		for ( CLibraryFile* pOther = *pPrev ; pOther ; pOther = pOther->m_pNextBTH )
		{
			if ( pOther == pFile )
			{
				*pPrev = pOther->m_pNextBTH;
				break;
			}
			pPrev = &pOther->m_pNextBTH;
		}

		pFile->m_pNextBTH = NULL;
	}

	if ( pFile->m_oMD5 )
	{
		CLibraryFile** pPrev = &m_pMD5Map[ FILE_INDEX( pFile->m_oMD5 ) ];

		for ( CLibraryFile* pOther = *pPrev ; pOther ; pOther = pOther->m_pNextMD5 )
		{
			if ( pOther == pFile )
			{
				*pPrev = pOther->m_pNextMD5;
				break;
			}
			pPrev = &pOther->m_pNextMD5;
		}

		pFile->m_pNextMD5 = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryMaps cull deleted files

void CLibraryMaps::CullDeletedFiles(CLibraryFile* pMatch)
{
	ASSUME_LOCK( Library.m_pSection );

	if ( CFileList* pList = LookupFilesByHash( pMatch, FALSE, FALSE, 0 ) )
	{
		for ( POSITION pos = pList->GetHeadPosition() ; pos ; )
		{
			CLibraryFile* pFile = pList->GetNext( pos );
			if ( ! pFile->IsAvailable() )
			{
				// Restore metadata from the ghost file
				pMatch->AddMetadata( pFile );

				// Delete ghost file
				pFile->Delete( TRUE );
			}
		}
		delete pList;
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryMaps search

CFileList* CLibraryMaps::Browse(int nMaximum) const
{
	ASSUME_LOCK( Library.m_pSection );

	CFileList* pHits = NULL;
	for ( POSITION pos = GetFileIterator() ; pos ; )
	{
		CLibraryFile* pFile = GetNextFile( pos );

		if ( pFile->IsAvailable() && pFile->IsShared() && pFile->m_oSHA1 )
		{
			if ( ! pHits )
				pHits = new CFileList;
			pHits->AddTail( pFile );
			if ( nMaximum && pHits->GetCount() >= nMaximum )
				break;
		}
	}
	return pHits;
}

CFileList* CLibraryMaps::WhatsNew(const CQuerySearch* pSearch, int nMaximum) const
{
	ASSUME_LOCK( Library.m_pSection );

	DWORD tNow = static_cast< DWORD >( time( NULL ) );
	CFileList* pHits = NULL;
	for ( POSITION pos = GetFileIterator() ; pos ; )
	{
		CLibraryFile* pFile = GetNextFile( pos );

		if ( pFile->IsAvailable() && pFile->IsShared() && pFile->m_oSHA1 &&
			( ! pSearch->m_pSchema || pSearch->m_pSchema->Equals( pFile->m_pSchema ) ) )
		{
			DWORD nTime = pFile->GetCreationTime();
			if ( nTime && nTime + 12 * 60 * 60 > tNow  ) // 12 hours
			{
				pFile->m_nHitsToday++;
				pFile->m_nHitsTotal++;

				if ( ! pHits )
					pHits = new CFileList;
				pHits->AddTail( pFile );
				if ( nMaximum && pHits->GetCount() >= nMaximum )
					break;
			}
		}
	}
	return pHits;
}

//////////////////////////////////////////////////////////////////////
// CLibraryMaps serialize

void CLibraryMaps::Serialize1(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		ar << m_nNextIndex;

		ar << (DWORD)m_pIndexMap.GetCount();
		ar << (DWORD)m_pNameMap.GetCount();
		ar << (DWORD)m_pPathMap.GetCount();
	}
	else
	{
		DWORD nNextIndex = 0;
		ar >> nNextIndex;
		m_nNextIndex = nNextIndex;

		if ( nVersion >= 28 )
		{
			DWORD nIndexMapCount = 0;
			ar >> nIndexMapCount;
			m_pIndexMap.InitHashTable( GetBestHashTableSize( nIndexMapCount ) );

			DWORD nNameMapCount = 0;
			ar >> nNameMapCount;
			m_pNameMap.InitHashTable( GetBestHashTableSize( nNameMapCount ) );

			DWORD nPathMapCount = 0;
			ar >> nPathMapCount;
			m_pPathMap.InitHashTable( GetBestHashTableSize( nPathMapCount ) );
		}
	}
}

void CLibraryMaps::Serialize2(CArchive& ar, int nVersion)
{
	if ( nVersion < 18 ) return;

	if ( ar.IsStoring() )
	{
		ar.WriteCount( m_pDeleted.GetCount() );

		for ( POSITION pos = m_pDeleted.GetHeadPosition() ; pos ; )
		{
			m_pDeleted.GetNext( pos )->Serialize( ar, nVersion );
		}
	}
	else
	{
		for ( DWORD_PTR nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
		{
			CAutoPtr< CLibraryFile > pFile( new CLibraryFile( NULL ) );
			if ( ! pFile )
				AfxThrowMemoryException();

			pFile->Serialize( ar, nVersion );

			const CLibraryFile* pRealFile = LibraryMaps.LookupFileByHash( pFile );
			if ( ! pRealFile )
				Library.AddFile( pFile.Detach() );
			else
			{
				TRACE( "Ignored ghost file \"%s\" (%I64u bytes) due real file \"%s\" (%I64u bytes)\n", (LPCSTR)CT2A( (LPCTSTR)pFile->m_sName ), pFile->m_nSize, (LPCSTR)CT2A( (LPCTSTR)pRealFile->m_sName ), pRealFile->m_nSize );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CLibrary ILibraryFiles

IMPLEMENT_DISPATCH(CLibraryMaps, LibraryFiles)

STDMETHODIMP CLibraryMaps::XLibraryFiles::get_Application(IApplication FAR* FAR* ppApplication)
{
	METHOD_PROLOGUE( CLibraryMaps, LibraryFiles )
	return CApplication::GetApp( ppApplication );
}

STDMETHODIMP CLibraryMaps::XLibraryFiles::get_Library(ILibrary FAR* FAR* ppLibrary)
{
	METHOD_PROLOGUE( CLibraryMaps, LibraryFiles )
	*ppLibrary = (ILibrary*)Library.GetInterface( IID_ILibrary, TRUE );
	return *ppLibrary ? S_OK : E_NOINTERFACE;
}

STDMETHODIMP CLibraryMaps::XLibraryFiles::get__NewEnum(IUnknown FAR* FAR* /*ppEnum*/)
{
	METHOD_PROLOGUE( CLibraryMaps, LibraryFiles )
	return E_NOTIMPL;
}

STDMETHODIMP CLibraryMaps::XLibraryFiles::get_Item(VARIANT vIndex, ILibraryFile FAR* FAR* ppFile)
{
	METHOD_PROLOGUE( CLibraryMaps, LibraryFiles )
	*ppFile = NULL;
	CQuickLock oLock( Library.m_pSection );

	CLibraryFile* pFile = NULL;
	if ( vIndex.vt == VT_BSTR )
	{
		CString strName( vIndex.bstrVal );
		if ( strName.Find( '\\' ) >= 0 )
			pFile = pThis->LookupFileByPath( strName );
		else
			pFile = pThis->LookupFileByName( strName, SIZE_UNKNOWN, FALSE, FALSE );
	}
	else
	{
		CComVariant va( vIndex );
		if ( FAILED( va.ChangeType( VT_I4 ) ) || va.lVal < 0 || va.lVal >= pThis->GetFileCount() )
			return E_INVALIDARG;

		for ( POSITION pos = pThis->GetFileIterator() ; pos ; )
		{
			pFile = pThis->GetNextFile( pos );
			if ( va.lVal-- == 0 ) break;
			pFile = NULL;
		}
	}

	*ppFile = pFile ? (ILibraryFile*)pFile->GetInterface( IID_ILibraryFile, TRUE ) : NULL;

	return *ppFile ? S_OK : E_INVALIDARG;
}

STDMETHODIMP CLibraryMaps::XLibraryFiles::get_Count(LONG FAR* pnCount)
{
	METHOD_PROLOGUE( CLibraryMaps, LibraryFiles )
	CQuickLock oLock( Library.m_pSection );
	*pnCount = static_cast< LONG >( pThis->GetFileCount() );
	return S_OK;
}
