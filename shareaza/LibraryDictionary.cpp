//
// LibraryDictionary.cpp
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
#include "Library.h"
#include "LibraryMaps.h"
#include "LibraryDictionary.h"
#include "SharedFile.h"

#include "QueryHashTable.h"
#include "QuerySearch.h"

#include "Schema.h"
#include "XML.h"

#include "SHA.h"
#include "ED2K.h"
#include "TigerTree.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CLibraryDictionary LibraryDictionary;


//////////////////////////////////////////////////////////////////////
// CLibraryDictionary construction

CLibraryDictionary::CLibraryDictionary() : m_pWords( 64 )
{
	m_pTable = NULL;
	m_bTable = FALSE;
	
	m_nSearchCookie = 1;
}

CLibraryDictionary::~CLibraryDictionary()
{
	Clear();
	if ( m_pTable ) delete m_pTable;
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary add and remove

void CLibraryDictionary::Add(CLibraryFile* pFile)
{
	ProcessFile( pFile, TRUE );
	
	if ( ( pFile->m_bSHA1 || pFile->m_bED2K ) && ! BuildHashTable() )
	{
		if ( pFile->m_bSHA1 )
		{
			m_pTable->AddString( CSHA::HashToString( &pFile->m_pSHA1, TRUE ) );
		}
		if ( pFile->m_bED2K )
		{
			m_pTable->AddString( CED2K::HashToString( &pFile->m_pED2K, TRUE ) );
		}
	}
}

void CLibraryDictionary::Remove(CLibraryFile* pFile)
{
	ProcessFile( pFile, FALSE );
	
	// TODO: Always invalidate the table when removing a hashed
	// file... is this wise???  It will happen all the time.
	
	if ( pFile->m_bSHA1 || pFile->m_bED2K ) m_bTable = FALSE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary file and and remove

void CLibraryDictionary::ProcessFile(CLibraryFile* pFile, BOOL bAdd)
{
	ASSERT(pFile != NULL);
	ProcessPhrase( pFile, pFile->GetSearchName(), bAdd, FALSE );
	
	if ( pFile->m_pMetadata && pFile->m_pSchema )
	{
		ProcessWord( pFile, pFile->m_pSchema->m_sURI, bAdd );
		ProcessPhrase( pFile, pFile->GetMetadataWords(), bAdd );
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary phrase parser

int CLibraryDictionary::ProcessPhrase(CLibraryFile* pFile, const CString& strPhrase, BOOL bAdd, BOOL bLowercase)
{
	LPCTSTR pszPtr = strPhrase;
	CString strWord;
	int nCount = 0;
	
	for ( int nStart = 0, nPos = 0 ; *pszPtr ; nPos++, pszPtr++ )
	{
		if ( ! IsCharacter( *pszPtr ) )
		{
			if ( nStart < nPos && IsWord( strPhrase, nStart, nPos - nStart ) )
			{
				strWord = strPhrase.Mid( nStart, nPos - nStart );
				if ( bLowercase ) strWord = CharLower( strWord.GetBuffer() );
				ProcessWord( pFile, strWord, bAdd );
				nCount++;
				
				if ( nPos - nStart >= 5 && Settings.Library.PartialMatch )
				{
					strWord = strPhrase.Mid( nStart, nPos - nStart - 1 );
					if ( bLowercase ) strWord = CharLower( strWord.GetBuffer() );
					ProcessWord( pFile, strWord, bAdd );
					nCount++;
					
					strWord = strPhrase.Mid( nStart, nPos - nStart - 2 );
					if ( bLowercase ) strWord = CharLower( strWord.GetBuffer() );
					ProcessWord( pFile, strWord, bAdd );
					nCount++;
				}
			}
			nStart = nPos + 1;
		}
	}
	
	if ( nStart < nPos && IsWord( strPhrase, nStart, nPos - nStart ) )
	{
		strWord = strPhrase.Mid( nStart, nPos - nStart );
		if ( bLowercase ) strWord = CharLower( strWord.GetBuffer() );
		ProcessWord( pFile, strWord, bAdd );
		nCount++;
		
		if ( nPos - nStart >= 5 && Settings.Library.PartialMatch )
		{
			strWord = strPhrase.Mid( nStart, nPos - nStart - 1 );
			if ( bLowercase ) strWord = CharLower( strWord.GetBuffer() );
			ProcessWord( pFile, strWord, bAdd );
			nCount++;
			
			strWord = strPhrase.Mid( nStart, nPos - nStart - 2 );
			if ( bLowercase ) strWord = CharLower( strWord.GetBuffer() );
			ProcessWord( pFile, strWord, bAdd );
			nCount++;
		}
	}
	
	return nCount;
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary word add and remove

void CLibraryDictionary::ProcessWord(CLibraryFile* pFile, const CString& strWord, BOOL bAdd)
{
	CLibraryWord* pWord;
	
	if ( m_pWords.Lookup( strWord, (void*&)pWord ) )
	{
		if ( bAdd )
		{
			pWord->Add( pFile );
		}
		else
		{
			if ( ! pWord->Remove( pFile ) )
			{
				m_pWords.RemoveKey( strWord );
				delete pWord;
				m_bTable = FALSE;
			}
		}
	}
	else if ( bAdd )
	{
		pWord = new CLibraryWord();
		pWord->Add( pFile );
		m_pWords.SetAt( strWord, pWord );

		if ( ! BuildHashTable() ) m_pTable->AddString( strWord );
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary build hash table

BOOL CLibraryDictionary::BuildHashTable()
{
	if ( m_pTable == NULL )
	{
		m_pTable = new CQueryHashTable();
		m_pTable->Create();
	}
	
	if ( m_bTable ) return FALSE;
	
	m_pTable->Clear();
	
	for ( POSITION pos = m_pWords.GetStartPosition() ; pos ; )
	{
		CLibraryWord* pWord;
		CString strWord;
		
		m_pWords.GetNextAssoc( pos, strWord, (void*&)pWord );
		m_pTable->AddString( strWord );
	}
	
	for ( pos = LibraryMaps.GetFileIterator() ; pos ; )
	{
		CLibraryFile* pFile = LibraryMaps.GetNextFile( pos );
		
		if ( pFile->m_bSHA1 )
		{
			m_pTable->AddString( CSHA::HashToString( &pFile->m_pSHA1, TRUE ) );
		}
		if ( pFile->m_bED2K )
		{
			m_pTable->AddString( CED2K::HashToString( &pFile->m_pED2K, TRUE ) );
		}
	}
	
	m_bTable = TRUE;
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary retreive hash table

CQueryHashTable* CLibraryDictionary::GetHashTable()
{
	if ( ! Library.Lock( 500 ) ) return NULL;
	
	BuildHashTable();
	
	return m_pTable;
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary clear

void CLibraryDictionary::Clear()
{
	for ( POSITION pos = m_pWords.GetStartPosition() ; pos ; )
	{
		CLibraryWord* pWord;
		CString strWord;
		
		m_pWords.GetNextAssoc( pos, strWord, (void*&)pWord );
		delete pWord;
	}
	
	m_pWords.RemoveAll();
	
	if ( m_pTable != NULL )
	{
		m_pTable->Clear();
		m_bTable = TRUE;
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary search

CPtrList* CLibraryDictionary::Search(CQuerySearch* pSearch, int nMaximum, BOOL bLocal)
{
	BuildHashTable();
	
	if ( ! m_pTable->Check( pSearch ) ) return NULL;
	
	DWORD nCookie = m_nSearchCookie++;
	
	CLibraryFile* pHit = NULL;
	
	LPCTSTR* pWordPtr	= pSearch->m_pWordPtr;
	DWORD* pWordLen		= pSearch->m_pWordLen;
	
	for ( int nWord = pSearch->m_nWords ; nWord > 0 ; nWord--, pWordPtr++, pWordLen++ )
	{
		if ( **pWordPtr == '-' ) continue;
		
		LPTSTR pszNull = (LPTSTR)(*pWordPtr) + *pWordLen;
		TCHAR cNull = *pszNull;
		*pszNull = 0;
		
		CLibraryWord* pWord;
		
		if ( m_pWords.Lookup( *pWordPtr, (void*&)pWord ) )
		{
			CLibraryFile** pFiles	= pWord->m_pList;
			CLibraryFile* pLastFile	= NULL;
			
			for ( DWORD nFileCount = pWord->m_nCount ; nFileCount ; nFileCount--, pFiles++ )
			{
				CLibraryFile* pFile = *pFiles;
				
				if ( pFile == pLastFile ) continue;
				pLastFile = pFile;
				
				if ( ! bLocal && ! pFile->IsShared() ) continue;
				
				if ( pFile->m_nSearchCookie == nCookie )
				{
					pFile->m_nSearchWords ++;
				}
				else
				{
					pFile->m_nSearchCookie	= nCookie;
					pFile->m_nSearchWords	= 1;
					pFile->m_pNextHit		= pHit;
					pHit = pFile;
				}
			}
		}
		
		*pszNull = cNull;
	}
	
	DWORD nLowerBound = pSearch->m_nWords >= 3 ? pSearch->m_nWords * 2 / 3 : pSearch->m_nWords;
	
	CPtrList* pHits = NULL;
	int nCount = 0;
	
	for ( ; pHit ; pHit = pHit->m_pNextHit )
	{
		if ( pHit->m_nSearchCookie == nCookie && pHit->m_nSearchWords >= nLowerBound )
		{
			if ( pSearch->Match( pHit->GetSearchName(), pHit->m_nSize,
					pHit->m_pSchema ? (LPCTSTR)pHit->m_pSchema->m_sURI : NULL,
					pHit->m_pMetadata,
					pHit->m_bSHA1 ? &pHit->m_pSHA1 : NULL,
					pHit->m_bTiger ? &pHit->m_pTiger : NULL,
					pHit->m_bED2K ? &pHit->m_pED2K : NULL ) )
			{
				if ( ! pHits ) pHits = new CPtrList();
				pHits->AddTail( pHit );
				
				if ( ! bLocal )
				{
					pHit->m_nHitsToday++;
					pHit->m_nHitsTotal++;
				}
				
				if ( pHit->m_nCollIndex )
				{
					if ( CLibraryFile* pCollection = LibraryMaps.LookupFile( pHit->m_nCollIndex, FALSE, ! bLocal, TRUE ) )
					{
						if ( pCollection->m_nSearchCookie != nCookie )
						{
							pCollection->m_nSearchCookie = nCookie;
							pHits->AddHead( pCollection );
						}
					}
					else
					{
						pHit->m_nCollIndex = 0;
					}
				}
				
				if ( nMaximum && ++nCount >= nMaximum ) break;
			}
		}
	}
	
	return pHits;
}


//////////////////////////////////////////////////////////////////////
// CLibraryWord construction

CLibraryWord::CLibraryWord()
{
	m_pList		= NULL;
	m_nCount	= 0;
}

CLibraryWord::~CLibraryWord()
{
	if ( m_pList ) delete [] m_pList;
}

//////////////////////////////////////////////////////////////////////
// CLibraryWord add and remove

void CLibraryWord::Add(CLibraryFile* pFile)
{
	CLibraryFile** pList = new CLibraryFile*[ m_nCount + 1 ];
	
	if ( m_pList )
	{
		CopyMemory( pList, m_pList, m_nCount * sizeof(CLibraryFile*) );
		delete [] m_pList;
	}
	
	m_pList = pList;
	m_pList[ m_nCount++ ] = pFile;
}

BOOL CLibraryWord::Remove(CLibraryFile* pFile)
{
	CLibraryFile** pSearch = m_pList;
	
	for ( DWORD nSearch = m_nCount ; nSearch ; nSearch--, pSearch++ )
	{
		if ( *pSearch == pFile )
		{
			for ( m_nCount--, nSearch-- ; nSearch ; nSearch--, pSearch++ )
			{
				*pSearch = pSearch[1];
			}
			break;
		}
	}
	
	return ( m_nCount > 0 );
}

