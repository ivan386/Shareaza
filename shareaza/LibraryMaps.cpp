//
// LibraryMaps.cpp
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
#include "Library.h"
#include "LibraryMaps.h"
#include "SharedFile.h"

#include "Application.h"
#include "QuerySearch.h"

#include "SHA.h"
#include "MD5.h"
#include "ED2K.h"
#include "TigerTree.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CLibraryMaps, CComObject)

BEGIN_INTERFACE_MAP(CLibraryMaps, CComObject)
	INTERFACE_PART(CLibraryMaps, IID_ILibraryFiles, LibraryFiles)
END_INTERFACE_MAP()

#undef HASH_SIZE
#undef HASH_MASK
#define HASH_SIZE	512
#define HASH_MASK	0x1FF

CLibraryMaps LibraryMaps;


//////////////////////////////////////////////////////////////////////
// CLibraryMaps construction

CLibraryMaps::CLibraryMaps()
{
	EnableDispatch( IID_ILibraryFiles );
	
	m_pSHA1Map		= new CLibraryFile*[HASH_SIZE];
	m_pTigerMap		= new CLibraryFile*[HASH_SIZE];
	m_pED2KMap		= new CLibraryFile*[HASH_SIZE];
	m_pBTHMap		= new CLibraryFile*[HASH_SIZE];
	m_pMD5Map		= new CLibraryFile*[HASH_SIZE];
	
	ZeroMemory( m_pSHA1Map, HASH_SIZE * sizeof( CLibraryFile* ) );
	ZeroMemory( m_pTigerMap, HASH_SIZE * sizeof( CLibraryFile* ) );
	ZeroMemory( m_pED2KMap, HASH_SIZE * sizeof( CLibraryFile* ) );
	ZeroMemory( m_pBTHMap, HASH_SIZE * sizeof( CLibraryFile* ) );
	ZeroMemory( m_pMD5Map, HASH_SIZE * sizeof( CLibraryFile* ) );
	
	m_nNextIndex	= 4;
	m_nFiles		= 0;
	m_nVolume		= 0;
}

CLibraryMaps::~CLibraryMaps()
{
	delete [] m_pMD5Map;
	delete [] m_pBTHMap;
	delete [] m_pED2KMap;
	delete [] m_pTigerMap;
	delete [] m_pSHA1Map;
}

//////////////////////////////////////////////////////////////////////
// CLibraryMaps file list

POSITION CLibraryMaps::GetFileIterator() const
{
	return m_pIndexMap.GetStartPosition();
}

CLibraryFile* CLibraryMaps::GetNextFile(POSITION& pos) const
{
	DWORD_PTR pIndex;
	CLibraryFile* pFile = NULL;
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

CLibraryFile* CLibraryMaps::LookupFile(DWORD_PTR nIndex, BOOL bSharedOnly, BOOL bAvailableOnly) const
{
	if ( ! nIndex ) return NULL;
	
	CLibraryFile* pFile = NULL;
	
	CQuickLock oLock( Library.m_pSection );
	
	if ( m_pIndexMap.Lookup( nIndex, pFile ) && ( ! bSharedOnly || pFile->IsShared() ) && ( ! bAvailableOnly || pFile->IsAvailable() ) )
	{
		return pFile;
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CLibraryMaps lookup file by name and/or path

CLibraryFile* CLibraryMaps::LookupFileByName(LPCTSTR pszName, BOOL bSharedOnly, BOOL bAvailableOnly)
{
	ASSERT_VALID( this );
	ASSERT( pszName && *pszName );

	CLibraryFile* pFile = NULL;
	CString strName( pszName );
	ToLower( strName );
	
	CQuickLock oLock( Library.m_pSection );
	
	if ( m_pNameMap.Lookup( strName, pFile ) && ( ! bSharedOnly || pFile->IsShared() ) && ( ! bAvailableOnly || pFile->IsAvailable() ) )
	{
		return pFile;
	}
	
	return NULL;
}

CLibraryFile* CLibraryMaps::LookupFileByPath(LPCTSTR pszPath, BOOL bSharedOnly, BOOL bAvailableOnly)
{
	ASSERT_VALID( this );
	ASSERT( pszPath && *pszPath );

	CLibraryFile* pFile = NULL;
	CString strPath( pszPath );
	ToLower( strPath );
	
	CQuickLock oLock( Library.m_pSection );
	
	if ( m_pPathMap.Lookup( strPath, pFile ) && ( ! bSharedOnly || pFile->IsShared() ) && ( ! bAvailableOnly || pFile->IsAvailable() ) )
	{
		return pFile;
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CLibraryMaps lookup file by URN

CLibraryFile* CLibraryMaps::LookupFileByURN(LPCTSTR pszURN, BOOL bSharedOnly, BOOL bAvailableOnly)
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

	if ( oBTH.fromUrn( pszURN ) )
	{
		if ( ( pFile = LookupFileByBTH( oBTH, bSharedOnly, bAvailableOnly ) ) != NULL ) return pFile;
	}

	if ( oMD5.fromUrn( pszURN ) )
	{
		if ( ( pFile = LookupFileByMD5( oMD5, bSharedOnly, bAvailableOnly ) ) != NULL ) return pFile;
	}
	
	return NULL;
}

CLibraryFile* CLibraryMaps::LookupFileByHash(const Hashes::Sha1Hash& oSHA1, const Hashes::TigerHash& oTiger,
											 const Hashes::Ed2kHash& oED2K, const Hashes::BtHash& oBTH,
											 const Hashes::Md5Hash& oMD5, QWORD nMinSize,
											 QWORD nMaxSize, BOOL bSharedOnly, BOOL bAvailableOnly) const
{
	CQuickLock oLock( Library.m_pSection );

	CLibraryFile* pFile = NULL;
	bool bMinSize = false, bMaxSize = false;

	bMinSize = !(nMinSize == SIZE_UNKNOWN) && !(nMinSize == 0);
	bMaxSize = !(nMaxSize == SIZE_UNKNOWN) && !(nMaxSize == 0);

	// Do not make conditions below with "if/else". There is no guarantee that a
	// Ghost rated file has all the hashes thus look up should go through all the
	// maps individually.
	if ( oSHA1.isValid() )
	{
		pFile = m_pSHA1Map[ oSHA1[ 0 ] & HASH_MASK ];
		if ( pFile != NULL )
		{
			for ( ; pFile ; pFile = pFile->m_pNextSHA1 )
			{
				if ( ( ( pFile->m_oSHA1 && oSHA1) ||
					( pFile->m_oTiger && oTiger) ||
					( pFile->m_oED2K && oED2K) ||
					( pFile->m_oMD5 && oMD5) ) &&
					( !validAndUnequal( pFile->m_oSHA1, oSHA1 ) ) &&
					( !validAndUnequal( pFile->m_oTiger, oTiger ) ) &&
					( !validAndUnequal( pFile->m_oED2K, oED2K ) ) &&
					( !validAndUnequal( pFile->m_oMD5, oMD5 ) ) )
				{
					if ( CheckFileAttributes( pFile, bMinSize, bMaxSize, nMinSize, nMaxSize, bSharedOnly, bAvailableOnly ) )
						return pFile;
				}
			}
		}
	}
	if ( oED2K.isValid() )
	{
		pFile = m_pED2KMap[ oED2K[ 0 ] & HASH_MASK ];
		if ( pFile != NULL )
		{
			for ( ; pFile ; pFile = pFile->m_pNextED2K )
			{
				if ( ( ( pFile->m_oED2K && oED2K) ||
					( pFile->m_oTiger && oTiger) ||
					( pFile->m_oMD5 && oMD5) ) &&
					( !validAndUnequal( pFile->m_oED2K, oED2K ) ) &&
					( !validAndUnequal( pFile->m_oTiger, oTiger ) ) &&
					( !validAndUnequal( pFile->m_oMD5, oMD5 ) ) )
				{
					if ( CheckFileAttributes( pFile, bMinSize, bMaxSize, nMinSize, nMaxSize, bSharedOnly, bAvailableOnly ) )
						return pFile;
				}
			}
		}
	}
	if ( oTiger.isValid() )
	{
		pFile = m_pTigerMap[ oTiger[ 0 ] & HASH_MASK ];
		if ( pFile != NULL )
		{
			for ( ; pFile ; pFile = pFile->m_pNextTiger )
			{
				if ( ( ( pFile->m_oTiger && oTiger) ||
					( pFile->m_oMD5 && oMD5) ) &&
					( !validAndUnequal( pFile->m_oTiger, oTiger ) ) &&
					( !validAndUnequal( pFile->m_oMD5, oMD5 ) ) )
				{
					if ( CheckFileAttributes( pFile, bMinSize, bMaxSize, nMinSize, nMaxSize, bSharedOnly, bAvailableOnly ) )
						return pFile;
				}
			}
		}
	}
	if ( oMD5.isValid() )
	{
		pFile = m_pMD5Map[ oMD5[ 0 ] & HASH_MASK ];
		if ( pFile != NULL )
		{
			for ( ; pFile ; pFile = pFile->m_pNextMD5 )
			{
				if ( validAndEqual( pFile->m_oMD5, oMD5 ) )
				{
					if ( CheckFileAttributes( pFile, bMinSize, bMaxSize, nMinSize, nMaxSize, bSharedOnly, bAvailableOnly ) )
						return pFile;
				}
			}
		}
	}

	if ( oBTH.isValid() )
	{
		pFile = m_pBTHMap[ oBTH[ 0 ] & HASH_MASK ];
		if ( pFile != NULL )
		{
			for ( ; pFile ; pFile = pFile->m_pNextBTH )
			{
				if ( validAndEqual( pFile->m_oBTH, oBTH ) )
				{
					if ( CheckFileAttributes( pFile, bMinSize, bMaxSize, nMinSize, nMaxSize, bSharedOnly, bAvailableOnly ) )
						return pFile;
				}
			}
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CLibraryMaps lookup file by individual hash types

CLibraryFile* CLibraryMaps::LookupFileBySHA1(const Hashes::Sha1Hash& oSHA1, BOOL bSharedOnly, BOOL bAvailableOnly)
{
	if ( !oSHA1 ) return NULL;

	CQuickLock oLock( Library.m_pSection );
	
	CLibraryFile* pFile = m_pSHA1Map[ oSHA1[ 0 ] & HASH_MASK ];
	
	for ( ; pFile ; pFile = pFile->m_pNextSHA1 )
	{
		if ( validAndEqual( oSHA1, pFile->m_oSHA1 ) )
		{
			if ( ( ! bSharedOnly || pFile->IsShared() ) && ( ! bAvailableOnly || pFile->IsAvailable() ) )
			{
				return pFile;
			}
			else
			{
				return NULL;
			}
		}
	}
	
	return NULL;
}

CLibraryFile* CLibraryMaps::LookupFileByTiger(const Hashes::TigerHash& oTiger, BOOL bSharedOnly, BOOL bAvailableOnly)
{
	if ( !oTiger ) return NULL;

	CQuickLock oLock( Library.m_pSection );
	
	CLibraryFile* pFile = m_pTigerMap[ oTiger[ 0 ] & HASH_MASK ];
	
	for ( ; pFile ; pFile = pFile->m_pNextTiger )
	{
		if ( validAndEqual( oTiger, pFile->m_oTiger ) )
		{
			if ( ( ! bSharedOnly || pFile->IsShared() ) && ( ! bAvailableOnly || pFile->IsAvailable() ) )
			{
				return pFile;
			}
			else
			{
				return NULL;
			}
		}
	}
	
	return NULL;
}

CLibraryFile* CLibraryMaps::LookupFileByED2K(const Hashes::Ed2kHash& oED2K, BOOL bSharedOnly, BOOL bAvailableOnly)
{
    if ( !oED2K ) return NULL;

	CQuickLock oLock( Library.m_pSection );
	
	CLibraryFile* pFile = m_pED2KMap[ oED2K[ 0 ] & HASH_MASK ];
	
	for ( ; pFile ; pFile = pFile->m_pNextED2K )
	{
		if ( validAndEqual( oED2K, pFile->m_oED2K ) )
		{
			if ( ( ! bSharedOnly || pFile->IsShared() ) && ( ! bAvailableOnly || pFile->IsAvailable() ) )
			{
				return pFile;
			}
			else
			{
				return NULL;
			}
		}
	}
	
	return NULL;
}

CLibraryFile* CLibraryMaps::LookupFileByBTH(const Hashes::BtHash& oBTH, BOOL bSharedOnly, BOOL bAvailableOnly)
{
	if ( !oBTH ) return NULL;

	CQuickLock oLock( Library.m_pSection );

	CLibraryFile* pFile = m_pBTHMap[ oBTH[ 0 ] & HASH_MASK ];

	for ( ; pFile ; pFile = pFile->m_pNextBTH )
	{
		if ( validAndEqual( oBTH, pFile->m_oBTH ) )
		{
			if ( ( ! bSharedOnly || pFile->IsShared() ) && ( ! bAvailableOnly || pFile->IsAvailable() ) )
			{
				return pFile;
			}
			else
			{
				return NULL;
			}
		}
	}

	return NULL;
}

CLibraryFile* CLibraryMaps::LookupFileByMD5(const Hashes::Md5Hash& oMD5, BOOL bSharedOnly, BOOL bAvailableOnly)
{
	if ( !oMD5 ) return NULL;

	CQuickLock oLock( Library.m_pSection );

	CLibraryFile* pFile = m_pMD5Map[ oMD5[ 0 ] & HASH_MASK ];

	for ( ; pFile ; pFile = pFile->m_pNextMD5 )
	{
		if ( validAndEqual( oMD5, pFile->m_oMD5 ) )
		{
			if ( ( ! bSharedOnly || pFile->IsShared() ) && ( ! bAvailableOnly || pFile->IsAvailable() ) )
			{
				return pFile;
			}
			else
			{
				return NULL;
			}
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CLibraryMaps clear

void CLibraryMaps::Clear()
{
	for ( POSITION pos = GetFileIterator() ; pos ; ) delete GetNextFile( pos );
	
	ASSERT( m_pIndexMap.IsEmpty() );
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
#endif
	
	ZeroMemory( m_pSHA1Map, HASH_SIZE * sizeof *m_pSHA1Map );
	ZeroMemory( m_pTigerMap, HASH_SIZE * sizeof *m_pTigerMap );
	ZeroMemory( m_pED2KMap, HASH_SIZE * sizeof *m_pED2KMap );
	ZeroMemory( m_pBTHMap, HASH_SIZE * sizeof *m_pBTHMap );
	ZeroMemory( m_pMD5Map, HASH_SIZE * sizeof *m_pMD5Map );
	
	m_nFiles  = 0;
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

	if ( ( pFile->m_pFolder != NULL ) && ( ! bSkipStats ) )
	{
		m_nVolume += pFile->m_nSize;
		m_nFiles ++;
	}
	
	m_pNameMap.SetAt( pFile->GetNameLC(), pFile );
	
	if ( pFile->m_pFolder != NULL )
	{
		CString strPath( pFile->GetPath() );
		ToLower( strPath );
		m_pPathMap.SetAt( strPath, pFile );
	}
	else if ( m_pDeleted.Find( pFile ) == NULL )
	{
		m_pDeleted.AddTail( pFile );
	}
	
	if ( pFile->m_oSHA1 )
	{
		CLibraryFile** pHash = &m_pSHA1Map[ pFile->m_oSHA1[ 0 ] & HASH_MASK ];
		pFile->m_pNextSHA1 = *pHash;
		*pHash = pFile;
	}
	
	if ( pFile->m_oTiger )
	{
		CLibraryFile** pHash = &m_pTigerMap[ pFile->m_oTiger[ 0 ] & HASH_MASK ];
		pFile->m_pNextTiger = *pHash;
		*pHash = pFile;
	}
	
	if ( pFile->m_oED2K )
	{
		CLibraryFile** pHash = &m_pED2KMap[ pFile->m_oED2K[ 0 ] & HASH_MASK ];
		pFile->m_pNextED2K = *pHash;
		*pHash = pFile;
	}

	if ( pFile->m_oBTH )
	{
		CLibraryFile** pHash = &m_pBTHMap[ pFile->m_oBTH[ 0 ] & HASH_MASK ];
		pFile->m_pNextBTH = *pHash;
		*pHash = pFile;
	}

	if ( pFile->m_oMD5 )
	{
		CLibraryFile** pHash = &m_pMD5Map[ pFile->m_oMD5[ 0 ] & HASH_MASK ];
		pFile->m_pNextMD5 = *pHash;
		*pHash = pFile;
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryMaps remove a file from the maps

void CLibraryMaps::OnFileRemove(CLibraryFile* pFile)
{
	CLibraryFile* pOld;
	
	if ( pFile->m_nIndex )
	{
		pOld = LookupFile( pFile->m_nIndex );
		
		if ( pOld == pFile )
		{
			m_pIndexMap.RemoveKey( pFile->m_nIndex );
			
			if ( pOld->m_pFolder != NULL )
			{
				m_nFiles --;
				m_nVolume -= pFile->m_nSize;
			}
		}
	}
	
	pOld = LookupFileByName( pFile->GetNameLC() );
	if ( pOld == pFile ) m_pNameMap.RemoveKey( pFile->GetNameLC() );
	
	if ( pFile->m_pFolder != NULL )
	{
		CString strPath( pFile->GetPath() );
		ToLower( strPath );
		pOld = LookupFileByPath( strPath );
		if ( pOld == pFile ) m_pPathMap.RemoveKey( strPath );
	}
	
	if ( POSITION pos = m_pDeleted.Find( pFile ) )
		m_pDeleted.RemoveAt( pos );
	
	if ( pFile->m_oSHA1 )
	{
		CLibraryFile** pPrev = &m_pSHA1Map[ pFile->m_oSHA1[ 0 ] & HASH_MASK ];
		
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
		CLibraryFile** pPrev = &m_pTigerMap[ pFile->m_oTiger[ 0 ] & HASH_MASK ];
		
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
		CLibraryFile** pPrev = &m_pED2KMap[ pFile->m_oED2K[ 0 ] & HASH_MASK ];
		
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
		CLibraryFile** pPrev = &m_pBTHMap[ pFile->m_oBTH[ 0 ] & HASH_MASK ];

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
		CLibraryFile** pPrev = &m_pMD5Map[ pFile->m_oMD5[ 0 ] & HASH_MASK ];

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
	CSingleLock oLock( &Library.m_pSection );
	if ( !oLock.Lock( 100 ) ) return;
	CLibraryFile* pFile;
	
	if ( pMatch->m_oSHA1 )
	{
		if ( ( pFile = LookupFileBySHA1( pMatch->m_oSHA1 ) ) != NULL )
		{
			if ( ! pFile->IsAvailable() ) pFile->Delete();
		}
	}
	
	if ( pMatch->m_oTiger )
	{
		if ( ( pFile = LookupFileByTiger( pMatch->m_oTiger ) ) != NULL )
		{
			if ( ! pFile->IsAvailable() ) pFile->Delete();
		}
	}
	
	if ( pMatch->m_oED2K )
	{
		if ( ( pFile = LookupFileByED2K( pMatch->m_oED2K ) ) != NULL )
		{
			if ( ! pFile->IsAvailable() ) pFile->Delete();
		}
	}

	if ( pMatch->m_oBTH )
	{
		if ( ( pFile = LookupFileByBTH( pMatch->m_oBTH ) ) != NULL )
		{
			if ( ! pFile->IsAvailable() ) pFile->Delete();
		}
	}

	if ( pMatch->m_oMD5 )
	{
		if ( ( pFile = LookupFileByMD5( pMatch->m_oMD5 ) ) != NULL )
		{
			if ( ! pFile->IsAvailable() ) pFile->Delete();
		}
	}
	
}

//////////////////////////////////////////////////////////////////////
// CLibraryMaps search

CList< CLibraryFile* >* CLibraryMaps::Search(CQuerySearch* pSearch, int nMaximum, BOOL bLocal, BOOL bAvailableOnly)
{
	CList< CLibraryFile* >* pHits = NULL;
	if ( pSearch == NULL )
	{
		for ( POSITION pos = GetFileIterator() ; pos ; )
		{
			CLibraryFile* pFile = GetNextFile( pos );
			
			if ( pFile->IsAvailable() )
			{
				if ( bLocal || ( pFile->IsShared() && pFile->m_oSHA1 ) )
				{
					if ( ! pHits ) pHits = new CList< CLibraryFile* >( 64 );
					pHits->AddTail( pFile );
					if ( nMaximum && pHits->GetCount() >= nMaximum ) break;
				}
			}
		}
	}
	else if ( pSearch->m_oSHA1 )
	{
		if ( CLibraryFile* pFile = LookupFileBySHA1( pSearch->m_oSHA1, ! bLocal, bAvailableOnly ) )
		{
			if ( ! pHits ) pHits = new CList< CLibraryFile* >( 64 );
			pHits->AddTail( pFile );
			if ( ! bLocal )
			{
				pFile->m_nHitsToday++;
				pFile->m_nHitsTotal++;
			}
		}
	}
	else if ( pSearch->m_oTiger )
	{
		if ( CLibraryFile* pFile = LookupFileByTiger( pSearch->m_oTiger, ! bLocal, bAvailableOnly ) )
		{
			if ( ! pHits ) pHits = new CList< CLibraryFile* >( 64 );
			pHits->AddTail( pFile );
			if ( ! bLocal )
			{
				pFile->m_nHitsToday++;
				pFile->m_nHitsTotal++;
			}
		}
	}
	else if ( pSearch->m_oED2K )
	{
		if ( CLibraryFile* pFile = LookupFileByED2K( pSearch->m_oED2K, ! bLocal, bAvailableOnly ) )
		{
			if ( ! pHits ) pHits = new CList< CLibraryFile* >( 64 );
			pHits->AddTail( pFile );
			if ( ! bLocal )
			{
				pFile->m_nHitsToday++;
				pFile->m_nHitsTotal++;
			}
		}
	}
	else if ( pSearch->m_oBTH )
	{
		if ( CLibraryFile* pFile = LookupFileByBTH( pSearch->m_oBTH, ! bLocal, bAvailableOnly ) )
		{
			if ( ! pHits ) pHits = new CList< CLibraryFile* >( 64 );
			pHits->AddTail( pFile );
			if ( ! bLocal )
			{
				pFile->m_nHitsToday++;
				pFile->m_nHitsTotal++;
			}
		}
	}
	else if ( pSearch->m_oMD5 )
	{
		if ( CLibraryFile* pFile = LookupFileByMD5( pSearch->m_oMD5, ! bLocal, bAvailableOnly ) )
		{
			if ( ! pHits ) pHits = new CList< CLibraryFile* >( 64 );
			pHits->AddTail( pFile );
			if ( ! bLocal )
			{
				pFile->m_nHitsToday++;
				pFile->m_nHitsTotal++;
			}
		}
	}
	return pHits;
}

BOOL CLibraryMaps::CheckFileAttributes(CLibraryFile* pFile, bool bMinSize, bool bMaxSize, QWORD nMinSize, QWORD nMaxSize, BOOL bSharedOnly, BOOL bAvailableOnly) const
{
	if ( ( ! bSharedOnly || pFile->IsShared() ) && ( ! bAvailableOnly || pFile->IsAvailable() ) )
	{
		if ( ( !bMinSize && !bMaxSize ) ||
			( !bMaxSize && nMinSize <= pFile->m_nSize ) ||
			( !bMinSize && nMaxSize >= pFile->m_nSize ) ||
			( nMinSize <= pFile->m_nSize && nMaxSize >= pFile->m_nSize ) )
		{
			return TRUE;
		}
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryMaps serialize

void CLibraryMaps::Serialize1(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		ar << m_nNextIndex;

		ar << (UINT)m_pIndexMap.GetCount();
		ar << (UINT)m_pNameMap.GetCount();
		ar << (UINT)m_pPathMap.GetCount();
	}
	else
	{
		DWORD nNextIndex = 0;
		ar >> nNextIndex;
		m_nNextIndex = nNextIndex;

		if ( nVersion >= 28 )
		{
			UINT nIndexMapCount = 0;
			ar >> nIndexMapCount;
			m_pIndexMap.InitHashTable( GetBestHashTableSize( nIndexMapCount ) );

			UINT nNameMapCount = 0;
			ar >> nNameMapCount;
			m_pNameMap.InitHashTable( GetBestHashTableSize( nNameMapCount ) );

			UINT nPathMapCount = 0;
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
			CLibraryFile* pFile = m_pDeleted.GetNext( pos );
			pFile->Serialize( ar, nVersion );
		}
	}
	else
	{
		for ( DWORD_PTR nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
		{
			CLibraryFile* pFile = new CLibraryFile( NULL );
			pFile->Serialize( ar, nVersion );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CLibrary ILibraryFiles

IMPLEMENT_DISPATCH(CLibraryMaps, LibraryFiles)

STDMETHODIMP CLibraryMaps::XLibraryFiles::get_Application(IApplication FAR* FAR* ppApplication)
{
	METHOD_PROLOGUE( CLibraryMaps, LibraryFiles )
	*ppApplication = Application.GetApp();
	return S_OK;
}

STDMETHODIMP CLibraryMaps::XLibraryFiles::get_Library(ILibrary FAR* FAR* ppLibrary)
{
	METHOD_PROLOGUE( CLibraryMaps, LibraryFiles )
	*ppLibrary = (ILibrary*)Library.GetInterface( IID_ILibrary, TRUE );
	return S_OK;
}

STDMETHODIMP CLibraryMaps::XLibraryFiles::get__NewEnum(IUnknown FAR* FAR* /*ppEnum*/)
{
	METHOD_PROLOGUE( CLibraryMaps, LibraryFiles )
	return E_NOTIMPL;
}

STDMETHODIMP CLibraryMaps::XLibraryFiles::get_Item(VARIANT vIndex, ILibraryFile FAR* FAR* ppFile)
{
	METHOD_PROLOGUE( CLibraryMaps, LibraryFiles )

	CLibraryFile* pFile = NULL;
	*ppFile = NULL;
	
	if ( vIndex.vt == VT_BSTR )
	{
		CString strName( vIndex.bstrVal );
		if ( strName.Find( '\\' ) >= 0 )
			pFile = pThis->LookupFileByPath( strName );
		else
			pFile = pThis->LookupFileByName( strName );
	}
	else
	{
		VARIANT va;
		VariantInit( &va );

		if ( FAILED( VariantChangeType( &va, (VARIANT FAR*)&vIndex, 0, VT_I4 ) ) )
			return E_INVALIDARG;
		if ( va.lVal < 0 || va.lVal >= pThis->GetFileCount() )
			return E_INVALIDARG;
		
		for ( POSITION pos = pThis->GetFileIterator() ; pos ; )
		{
			pFile = pThis->GetNextFile( pos );
			if ( va.lVal-- == 0 ) break;
			pFile = NULL;
		}
	}
	
	*ppFile = pFile ? (ILibraryFile*)pFile->GetInterface( IID_ILibraryFile, TRUE ) : NULL;
	
	return S_OK;
}

STDMETHODIMP CLibraryMaps::XLibraryFiles::get_Count(LONG FAR* pnCount)
{
	METHOD_PROLOGUE( CLibraryMaps, LibraryFiles )
	*pnCount = static_cast< int >( pThis->GetFileCount() );
	return S_OK;
}
