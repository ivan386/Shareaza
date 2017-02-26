//
// LibraryDictionary.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2017.
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

#include "LibraryDictionary.h"
#include "SharedFile.h"
#include "QueryHashMaster.h"
#include "QueryHashTable.h"
#include "QuerySearch.h"

#include "Library.h"
#include "LibraryMaps.h"
#include "Schema.h"
#include "Settings.h"
#include "UploadQueues.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CLibraryDictionary LibraryDictionary;


//////////////////////////////////////////////////////////////////////
// CLibraryDictionary construction

CLibraryDictionary::CLibraryDictionary() :
	m_pTable		( NULL )
,	m_bValid		( false )
,	m_nSearchCookie	( 1ul )
{
}

CLibraryDictionary::~CLibraryDictionary()
{
	ASSERT( m_oWordMap.IsEmpty() );
	delete m_pTable;
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary add and remove

void CLibraryDictionary::AddFile(const CLibraryFile* pFile)
{
	ASSUME_LOCK( Library.m_pSection );

	const bool bCanUpload = pFile->IsShared();

	ProcessFile( pFile, true, bCanUpload );

	if ( bCanUpload && m_bValid )
		m_pTable->AddHashes( *pFile );
}

void CLibraryDictionary::RemoveFile(const CLibraryFile* pFile)
{
	ASSUME_LOCK( Library.m_pSection );

	ProcessFile( pFile, false, pFile->IsShared() );

	// Always invalidate the table when removing a hashed file
	if ( pFile->HasHash() )
		Invalidate();
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary process file

void CLibraryDictionary::ProcessFile(const CLibraryFile* pFile, bool bAdd, bool bCanUpload)
{
	ProcessPhrase( pFile, pFile->GetSearchName(), bAdd, bCanUpload );
	ProcessPhrase( pFile, pFile->GetMetadataWords(), bAdd, bCanUpload );
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary phrase parser

void CLibraryDictionary::ProcessPhrase(const CLibraryFile* pFile, const CString& strPhrase, bool bAdd, bool bCanUpload)
{
	if ( strPhrase.IsEmpty() )
		return;

	CStringList oKeywords;
	CQueryHashTable::MakeKeywords( strPhrase, oKeywords );
	for ( POSITION pos = oKeywords.GetHeadPosition(); pos; )
	{
		ProcessWord( pFile, oKeywords.GetNext( pos ), bAdd, bCanUpload );
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary word add and remove

void CLibraryDictionary::ProcessWord(const CLibraryFile* pFile, const CString& strWord, bool bAdd, bool bCanUpload)
{
	ASSUME_LOCK( Library.m_pSection );

	CFileList* pList = NULL;
	if ( m_oWordMap.Lookup( strWord, pList ) )
	{
		if ( POSITION pos = pList->Find( const_cast< CLibraryFile* >( pFile ) ) )
		{
			if ( ! bAdd )
			{
				pList->RemoveAt( pos );
				if ( pList->IsEmpty() )
				{
					delete pList;

					VERIFY( m_oWordMap.RemoveKey( strWord ) );

					if ( bCanUpload && m_bValid )
						Invalidate();
				}
			}
		}
		else
		{
			if ( bAdd )
			{
				pList->AddTail( const_cast< CLibraryFile* >( pFile ) );

				if ( bCanUpload && m_bValid )
					m_pTable->AddExactString( strWord );
			}
		}
	}
	else if ( bAdd )
	{
		pList = new CFileList;
		if ( pList )
		{
			pList->AddTail( const_cast< CLibraryFile* >( pFile ) );
			m_oWordMap.SetAt( strWord, pList );

			if ( bCanUpload && m_bValid )
				m_pTable->AddExactString( strWord );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary build hash table

void CLibraryDictionary::BuildHashTable()
{
	ASSUME_LOCK( Library.m_pSection );

	if ( m_bValid )
		return;

	if ( !m_pTable )
	{
		m_pTable = new CQueryHashTable();
		if ( m_pTable )
			m_pTable->Create();
		else
			return;
	}

	m_pTable->Clear();

	// Add words to hash table
	//TRACE( _T("[LD] Dictionary size: %d words\n"), m_oWordMap.GetCount() );
	//TRACE( _T("[LD] Hash table size: %d\n"), m_oWordMap.GetHashTableSize() );
	for ( POSITION pos1 = m_oWordMap.GetStartPosition() ; pos1 ; )
	{
		CString strWord;
		CFileList* pList = NULL;
		m_oWordMap.GetNextAssoc( pos1, strWord, pList );

		//TRACE( _T("[LD] Word \"%hs\" found %d time(s) in %d file(s)\n"), (LPCSTR)CT2A( strWord ), oWord.m_nCount, oWord.m_pList->GetCount() );
		for ( POSITION pos2 = pList->GetHeadPosition() ; pos2 ; )
		{
			const CLibraryFile* pFile = pList->GetNext( pos2 );

			// Check if the file can be uploaded
			if ( pFile->IsShared() )
			{
				// Add the keyword to the table
				m_pTable->AddExactString( strWord );
				break;
			}
		}
	}

	// Add sha1/ed2k hashes to hash table
	for ( POSITION pos = LibraryMaps.GetFileIterator() ; pos ; )
	{
		const CLibraryFile* pFile = LibraryMaps.GetNextFile( pos );

		// Check if the file can be uploaded
		if ( pFile->IsShared() )
			m_pTable->AddHashes( *pFile );
	}

	m_bValid = true;
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary rebuild hash table
//
// Force hash table to re-build.

void CLibraryDictionary::Invalidate()
{
	m_bValid = false;

	QueryHashMaster.Invalidate();
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary retrieve hash table

const CQueryHashTable* CLibraryDictionary::GetHashTable()
{
	ASSUME_LOCK( Library.m_pSection );

	BuildHashTable();

	return m_pTable;
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary clear

void CLibraryDictionary::Clear()
{
	ASSUME_LOCK( Library.m_pSection );

	for ( POSITION pos = m_oWordMap.GetStartPosition() ; pos ; )
	{
		CString strWord;
		CFileList* pList = NULL;
		m_oWordMap.GetNextAssoc( pos, strWord, pList );
		delete pList;
	}
	m_oWordMap.RemoveAll();

	if ( m_pTable )
	{
		m_pTable->Clear();
		m_bValid = true;
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary search

CFileList* CLibraryDictionary::Search(
	const CQuerySearch* pSearch, const int nMaximum, const bool bLocal,
	const bool bAvailableOnly)
{
	ASSUME_LOCK( Library.m_pSection );

	if ( !m_bValid )
	{
		BuildHashTable();
		if ( !m_bValid )
			return NULL;
	}

	// Only check the hash when a search comes from other client.
	if ( !bLocal && !m_pTable->Check( pSearch ) )
		return NULL;

	++m_nSearchCookie;
	CLibraryFile* pHit = NULL;

	CQuerySearch::const_iterator pWordEntry = pSearch->begin();
	const CQuerySearch::const_iterator pLastWordEntry = pSearch->end();
	for ( ; pWordEntry != pLastWordEntry ; ++pWordEntry )
	{
		if ( pWordEntry->first[ 0 ] == _T('-') )
			continue;

		CString strWord( pWordEntry->first, static_cast< int >( pWordEntry->second ) );
		CFileList* pList = NULL;
		if ( m_oWordMap.Lookup( strWord, pList ) )
		{
			for ( POSITION pos = pList->GetHeadPosition() ; pos ; )
			{
				CLibraryFile* pFile = pList->GetNext( pos );

				if ( bAvailableOnly && ! pFile->IsAvailable() )
					continue;

				if ( ! bLocal && ! pFile->IsShared() )
					continue;

				if ( pFile->m_nSearchCookie == m_nSearchCookie )
				{
					++pFile->m_nSearchWords;
				}
				else
				{
					pFile->m_nSearchCookie	= m_nSearchCookie;
					pFile->m_nSearchWords	= 1;
					pFile->m_pNextHit		= pHit;
					pHit = pFile;
				}
			}
		}
	}

	size_t nLowerBound = ( pSearch->tableSize() >= 3 )
		? ( pSearch->tableSize() * 2 / 3 ) : pSearch->tableSize();

	CFileList* pHits = NULL;
	for ( ; pHit ; pHit = pHit->m_pNextHit )
	{
		ASSERT( pHit->m_nSearchCookie == m_nSearchCookie );

		if ( pHit->m_nSearchWords < nLowerBound )
			continue;

		if ( pSearch->Match( pHit->GetSearchName(),
			pHit->m_pSchema ? (LPCTSTR)pHit->m_pSchema->GetURI() : NULL,
			pHit->m_pMetadata, pHit ) )
		{
			if ( ! pHits )
				pHits = new CFileList;

			pHits->AddTail( pHit );

			if ( ! bLocal )
			{
				pHit->m_nHitsToday ++;
				pHit->m_nHitsTotal ++;
			}

			if ( pHit->m_nCollIndex )
			{
				CLibraryFile* pCollection = LibraryMaps.LookupFile(
					pHit->m_nCollIndex, !bLocal, bAvailableOnly );

				if ( pCollection )
				{
					if ( pCollection->m_nSearchCookie != m_nSearchCookie )
					{
						pCollection->m_nSearchCookie = m_nSearchCookie;
						pHits->AddHead( pCollection );
					}
				}
				else
				{
					// Collection removed without deleting indexes
					pHit->m_nCollIndex = 0ul;
				}
			}

			if ( nMaximum > 0 && pHits->GetCount() >= nMaximum )
				break;
		}
	}

	return pHits;
}

void CLibraryDictionary::Serialize(CArchive& ar, const int nVersion)
{
	ASSUME_LOCK( Library.m_pSection );

	if ( ar.IsStoring() )
	{
		ar << (DWORD)m_oWordMap.GetCount();
	}
	else
	{
		if ( nVersion >= 29 )
		{
			DWORD nWordsCount = 0u;
			ar >> nWordsCount;
			m_oWordMap.InitHashTable( GetBestHashTableSize( nWordsCount ) );
		}
	}
}
