//
// LibraryMaps.cpp
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
#include "Library.h"
#include "LibraryMaps.h"
#include "SharedFile.h"

#include "Application.h"
#include "QuerySearch.h"

#include "SHA.h"
#include "MD5.h"
#include "ED2K.h"
#include "TigerTree.h"

IMPLEMENT_DYNAMIC(CLibraryMaps, CComObject)

BEGIN_INTERFACE_MAP(CLibraryMaps, CComObject)
	INTERFACE_PART(CLibraryMaps, IID_ILibraryFiles, LibraryFiles)
END_INTERFACE_MAP()

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
	
	ZeroMemory( m_pSHA1Map, HASH_SIZE * 4 );
	ZeroMemory( m_pTigerMap, HASH_SIZE * 4 );
	ZeroMemory( m_pED2KMap, HASH_SIZE * 4 );
	
	m_nNextIndex	= 4;
	m_nFiles		= 0;
	m_nVolume		= 0;
}

CLibraryMaps::~CLibraryMaps()
{
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
	LPVOID pIndex;
	CLibraryFile* pFile = NULL;
	m_pIndexMap.GetNextAssoc( pos, pIndex, (void*&)pFile );
	return pFile;
}

int CLibraryMaps::GetFileCount() const
{
	return m_pIndexMap.GetCount();
}

void CLibraryMaps::GetStatistics(DWORD* pnFiles, QWORD* pnVolume)
{
	if ( pnFiles ) *pnFiles = m_nFiles;
	if ( pnVolume ) *pnVolume = m_nVolume;
}

//////////////////////////////////////////////////////////////////////
// CLibraryMaps lookup file by index

CLibraryFile* CLibraryMaps::LookupFile(DWORD nIndex, BOOL bSharedOnly, BOOL bAvailableOnly)
{
	if ( ! nIndex ) return NULL;
	
	CLibraryFile* pFile = NULL;
	
	CQuickLock oLock( Library.m_pSection );
	
	if ( m_pIndexMap.Lookup( (LPVOID)nIndex, (void*&)pFile ) && ( ! bSharedOnly || pFile->IsShared() ) && ( ! bAvailableOnly || pFile->IsAvailable() ) )
	{
		return pFile;
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CLibraryMaps lookup file by name and/or path

CLibraryFile* CLibraryMaps::LookupFileByName(LPCTSTR pszName, BOOL bSharedOnly, BOOL bAvailableOnly)
{
	CLibraryFile* pFile = NULL;
	CString strName( pszName );
	
	CQuickLock oLock( Library.m_pSection );
	CharLower( strName.GetBuffer() );
	strName.ReleaseBuffer();
	
	if ( m_pNameMap.Lookup( strName, (CObject*&)pFile ) && ( ! bSharedOnly || pFile->IsShared() ) && ( ! bAvailableOnly || pFile->IsAvailable() ) )
	{
		return pFile;
	}
	
	return NULL;
}

CLibraryFile* CLibraryMaps::LookupFileByPath(LPCTSTR pszPath, BOOL bSharedOnly, BOOL bAvailableOnly)
{
	CLibraryFile* pFile = NULL;
	
	CQuickLock oLock( Library.m_pSection );
	
	if ( m_pPathMap.Lookup( pszPath, (CObject*&)pFile ) && ( ! bSharedOnly || pFile->IsShared() ) && ( ! bAvailableOnly || pFile->IsAvailable() ) )
	{
		return pFile;
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CLibraryMaps lookup file by URN

CLibraryFile* CLibraryMaps::LookupFileByURN(LPCTSTR pszURN, BOOL bSharedOnly, BOOL bAvailableOnly)
{
	CLibraryFile* pFile;
	TIGEROOT pTiger;
	SHA1 pSHA1;
	MD4 pED2K;
	
	if ( CSHA::HashFromURN( pszURN, &pSHA1 ) )
	{
		if ( pFile = LookupFileBySHA1( &pSHA1, bSharedOnly ) ) return pFile;
	}
	
	if ( CTigerNode::HashFromURN( pszURN, &pTiger ) )
	{
		if ( pFile = LookupFileByTiger( &pTiger, bSharedOnly ) ) return pFile;
	}
	
	if ( CED2K::HashFromURN( pszURN, &pED2K ) )
	{
		if ( pFile = LookupFileByED2K( &pED2K, bSharedOnly ) ) return pFile;
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CLibraryMaps lookup file by individual hash types

CLibraryFile* CLibraryMaps::LookupFileBySHA1(const SHA1* pSHA1,BOOL bSharedOnly, BOOL bAvailableOnly)
{
	CQuickLock oLock( Library.m_pSection );
	
	CLibraryFile* pFile = m_pSHA1Map[ *(WORD*)pSHA1 & HASH_MASK ];
	
	for ( ; pFile ; pFile = pFile->m_pNextSHA1 )
	{
		if ( *pSHA1 == pFile->m_pSHA1 )
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

CLibraryFile* CLibraryMaps::LookupFileByTiger(const TIGEROOT* pTiger, BOOL bSharedOnly, BOOL bAvailableOnly)
{
	CQuickLock oLock( Library.m_pSection );
	
	CLibraryFile* pFile = m_pTigerMap[ *(WORD*)pTiger & HASH_MASK ];
	
	for ( ; pFile ; pFile = pFile->m_pNextTiger )
	{
		if ( *pTiger == pFile->m_pTiger )
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

CLibraryFile* CLibraryMaps::LookupFileByED2K(const MD4* pED2K, BOOL bSharedOnly, BOOL bAvailableOnly)
{
	CQuickLock oLock( Library.m_pSection );
	
	CLibraryFile* pFile = m_pED2KMap[ *(WORD*)pED2K & HASH_MASK ];
	
	for ( ; pFile ; pFile = pFile->m_pNextED2K )
	{
		if ( *pED2K == pFile->m_pED2K )
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
	
	ASSERT( m_pIndexMap.GetCount() == 0 );
	ASSERT( m_pNameMap.GetCount() == 0 );
	ASSERT( m_pPathMap.GetCount() == 0 );
	
	ZeroMemory( m_pSHA1Map, HASH_SIZE * 4 );
	ZeroMemory( m_pTigerMap, HASH_SIZE * 4 );
	ZeroMemory( m_pED2KMap, HASH_SIZE * 4 );
	
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
	if ( pFile->m_nIndex )
	{
		if ( CLibraryFile* pOld = LookupFile( pFile->m_nIndex ) )
		{
			if ( pOld != pFile )
			{
				pFile->m_nIndex = AllocateIndex();
				m_pIndexMap.SetAt( (LPVOID)pFile->m_nIndex, pFile );
				m_nVolume += ( pFile->m_nSize >> 10 );
				m_nFiles ++;
			}
		}
		else
		{
			m_pIndexMap.SetAt( (LPVOID)pFile->m_nIndex, pFile );
			m_nVolume += ( pFile->m_nSize >> 10 );
			m_nFiles ++;
		}
	}
	else
	{
		pFile->m_nIndex = AllocateIndex();
		m_pIndexMap.SetAt( (LPVOID)pFile->m_nIndex, pFile );
		m_nVolume += ( pFile->m_nSize >> 10 );
		m_nFiles ++;
	}
	
	m_pNameMap.SetAt( pFile->GetNameLC(), pFile );
	
	if ( pFile->m_pFolder != NULL )
	{
		m_pPathMap.SetAt( pFile->GetPath(), pFile );
	}
	else if ( m_pDeleted.Find( pFile ) == NULL )
	{
		m_pDeleted.AddTail( pFile );
	}
	
	if ( pFile->m_bSHA1 )
	{
		CLibraryFile** pHash = &m_pSHA1Map[ *(WORD*)&pFile->m_pSHA1 & HASH_MASK ];
		pFile->m_pNextSHA1 = *pHash;
		*pHash = pFile;
	}
	
	if ( pFile->m_bTiger )
	{
		CLibraryFile** pHash = &m_pTigerMap[ *(WORD*)&pFile->m_pTiger & HASH_MASK ];
		pFile->m_pNextTiger = *pHash;
		*pHash = pFile;
	}
	
	if ( pFile->m_bED2K )
	{
		CLibraryFile** pHash = &m_pED2KMap[ *(WORD*)&pFile->m_pED2K & HASH_MASK ];
		pFile->m_pNextED2K = *pHash;
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
			m_pIndexMap.RemoveKey( (LPVOID)pFile->m_nIndex );
			
			m_nFiles --;
			m_nVolume -= ( pFile->m_nSize >> 10 );
		}
	}
	
	pOld = LookupFileByName( pFile->GetNameLC() );
	if ( pOld == pFile ) m_pNameMap.RemoveKey( pFile->GetNameLC() );
	
	if ( pFile->m_pFolder != NULL )
	{
		pOld = LookupFileByPath( pFile->GetPath() );
		if ( pOld == pFile ) m_pPathMap.RemoveKey( pFile->GetPath() );
	}
	
	if ( POSITION pos = m_pDeleted.Find( pFile ) )
		m_pDeleted.RemoveAt( pos );
	
	if ( pFile->m_bSHA1 )
	{
		CLibraryFile** pPrev = &m_pSHA1Map[ *(WORD*)&pFile->m_pSHA1 & HASH_MASK ];
		
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
	
	if ( pFile->m_bTiger )
	{
		CLibraryFile** pPrev = &m_pTigerMap[ *(WORD*)&pFile->m_pTiger & HASH_MASK ];
		
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
	
	if ( pFile->m_bED2K )
	{
		CLibraryFile** pPrev = &m_pED2KMap[ *(WORD*)&pFile->m_pED2K & HASH_MASK ];
		
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
}

//////////////////////////////////////////////////////////////////////
// CLibraryMaps cull deleted files

void CLibraryMaps::CullDeletedFiles(CLibraryFile* pMatch)
{
	CSingleLock oLock( &Library.m_pSection );
	if ( !oLock.Lock( 100 ) ) return;
	CLibraryFile* pFile;
	
	if ( pMatch->m_bSHA1 )
	{
		if ( pFile = LookupFileBySHA1( &pMatch->m_pSHA1 ) )
		{
			if ( ! pFile->IsAvailable() ) pFile->Delete();
		}
	}
	
	if ( pMatch->m_bTiger )
	{
		if ( pFile = LookupFileByTiger( &pMatch->m_pTiger ) )
		{
			if ( ! pFile->IsAvailable() ) pFile->Delete();
		}
	}
	
	if ( pMatch->m_bED2K )
	{
		if ( pFile = LookupFileByED2K( &pMatch->m_pED2K ) )
		{
			if ( ! pFile->IsAvailable() ) pFile->Delete();
		}
	}
	
}

//////////////////////////////////////////////////////////////////////
// CLibraryMaps search

CPtrList* CLibraryMaps::Search(CQuerySearch* pSearch, int nMaximum, BOOL bLocal)
{
	CPtrList* pHits = NULL;
	
	if ( pSearch == NULL )
	{
		for ( POSITION pos = GetFileIterator() ; pos ; )
		{
			CLibraryFile* pFile = GetNextFile( pos );
			
			if ( pFile->IsAvailable() )
			{
				if ( bLocal || ( pFile->IsShared() && pFile->m_bSHA1 ) )
				{
					if ( ! pHits ) pHits = new CPtrList( 64 );
					pHits->AddTail( pFile );
				}
			}
		}
	}
	else if ( pSearch->m_bSHA1 )
	{
		if ( CLibraryFile* pFile = LookupFileBySHA1( &pSearch->m_pSHA1 ) )
		{
			if ( bLocal || pFile->IsShared() )
			{
				pHits = new CPtrList();
				pHits->AddTail( pFile );
				
				if ( ! bLocal )
				{
					pFile->m_nHitsToday++;
					pFile->m_nHitsTotal++;
				}
			}
		}
	}
	else if ( pSearch->m_bTiger )
	{
		if ( CLibraryFile* pFile = LookupFileByTiger( &pSearch->m_pTiger ) )
		{
			if ( bLocal || pFile->IsShared() )
			{
				pHits = new CPtrList();
				pHits->AddTail( pFile );
				
				if ( ! bLocal )
				{
					pFile->m_nHitsToday++;
					pFile->m_nHitsTotal++;
				}
			}
		}
	}
	else if ( pSearch->m_bED2K )
	{
		for ( POSITION pos = GetFileIterator() ; pos ; )
		{
			CLibraryFile* pFile = GetNextFile( pos );
			
			if ( pFile->m_bED2K && pFile->m_pED2K == pSearch->m_pED2K )
			{
				if ( bLocal || ( pFile->IsShared() && pFile->m_bSHA1 ) )
				{
					if ( ! pHits ) pHits = new CPtrList( 64 );
					pHits->AddTail( pFile );
				}
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
	}
	else
	{
		ar >> m_nNextIndex;
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
			CLibraryFile* pFile = (CLibraryFile*)m_pDeleted.GetNext( pos );
			pFile->Serialize( ar, nVersion );
		}
	}
	else
	{
		for ( int nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
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

STDMETHODIMP CLibraryMaps::XLibraryFiles::get__NewEnum(IUnknown FAR* FAR* ppEnum)
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
	*pnCount = pThis->GetFileCount();
	return S_OK;
}
