//
// LibraryDictionary.cpp
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

#include "UploadQueues.h"

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
	
	if ( ( pFile->m_oSHA1 || pFile->m_oED2K ) && ! BuildHashTable() )
	{
		if ( pFile->m_oSHA1 )
		{
			m_pTable->AddString( pFile->m_oSHA1.toUrn() );
		}
		if ( pFile->m_oED2K )
		{
			m_pTable->AddString( pFile->m_oED2K.toUrn() );
		}
	}
}

void CLibraryDictionary::Remove(CLibraryFile* pFile)
{
	ProcessFile( pFile, FALSE );
	
	// TODO: Always invalidate the table when removing a hashed
	// file... is this wise???  It will happen all the time.
	
	if ( pFile->m_oSHA1 || pFile->m_oED2K ) m_bTable = FALSE;
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
		ProcessPhrase( pFile, pFile->GetMetadataWords(), bAdd, FALSE );
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary phrase parser

int CLibraryDictionary::ProcessPhrase(CLibraryFile* pFile, const CString& strPhrase, BOOL bAdd, BOOL bLowercase)
{
	CString strTransformed( strPhrase );
	if ( bLowercase )
		ToLower( strTransformed );

	LPCTSTR pszPtr = strTransformed;
	CString strWord;
	int nCount = 0;
	ScriptType boundary[ 2 ] = { sNone, sNone };
    int nPos = 0;
	int nPrevWord = 0, nNextWord = 0;

	for ( ; *pszPtr ; nPos++, pszPtr++ )
	{
		// boundary[ 0 ] -- previous character;
		// boundary[ 1 ] -- current character;
		boundary[ 0 ] = boundary[ 1 ];

		if ( IsKanji( *pszPtr ) )
			boundary[ 1 ] = sKanji;
		else if ( IsKatakana( *pszPtr ) && IsHiragana( *pszPtr ) )
		{
			if ( boundary[ 0 ] == sKatakana || boundary[ 0 ] == sHiragana )
				boundary[ 1 ] = boundary[ 0 ];
		}
		else if ( IsKatakana( *pszPtr ) )
			boundary[ 1 ] = sKatakana;
		else if ( IsHiragana( *pszPtr ) )
			boundary[ 1 ] = sHiragana;
		else
			boundary[ 1 ] = sRegular;

		bool bCharacter = IsCharacter( *pszPtr );
		int nDistance = !bCharacter ? 1 : 0;

		if ( !bCharacter || boundary[ 0 ] != boundary[ 1 ] && nPos )
		{
			// Join two adjacent script phrases
			// nNextWord == nPrevWord when previous word was regular
			if ( nPos > nNextWord && nNextWord > nPrevWord )
			{
				strWord = strTransformed.Mid( nPrevWord, nPos - nPrevWord );
				nCount += MakeKeywords( pFile, strWord, bAdd );
			}
			if ( nPos > nNextWord )
			{
				strWord = strTransformed.Mid( nNextWord, nPos - nNextWord );
				nCount += MakeKeywords( pFile, strWord, bAdd );
				if ( nNextWord > nPrevWord )
				{
					strWord = strTransformed.Mid( nPrevWord, nNextWord - nPrevWord );
					nCount += MakeKeywords( pFile, strWord, bAdd );
				}
			}
			if ( nNextWord > nPrevWord )
				nPrevWord = nNextWord;
			nNextWord = nPos + nDistance;
			if ( boundary[ 0 ] < sKanji && boundary[ 1 ] < sKanji || !bCharacter ||
				 boundary[ 0 ] < sKanji && boundary[ 1 ] > sRegular ||
				 boundary[ 0 ] > sRegular && boundary[ 1 ] < sKanji )
				nPrevWord = nNextWord;
		}
	}
	
	strWord = strTransformed.Mid( nPrevWord, nPos - nPrevWord );
	nCount += MakeKeywords( pFile, strWord, bAdd );
	return nCount;
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary keyword maker
int CLibraryDictionary::MakeKeywords(CLibraryFile* pFile, const CString& strWord, BOOL bAdd)
{
	int nCount = 0;
	int nLength = strWord.GetLength();
	size_t nWindow = 0;
	CString strKeyword( strWord );

	if ( nLength && IsWord( strKeyword, 0, nLength ) )
	{
		// If start and end characters are from the same asian script, assign the window size
		if ( nLength > 2 )
		{
			if ( IsKanji( strKeyword.GetAt( 0 ) ) && IsKanji( strKeyword.GetAt( nLength - 1 ) ) ||
				 IsKatakana( strKeyword.GetAt( 0 ) ) && IsKatakana( strKeyword.GetAt( nLength - 1 ) ) ||
				 IsHiragana( strKeyword.GetAt( 0 ) ) && IsHiragana( strKeyword.GetAt( nLength - 1 ) ) )
			{
				nWindow = 2;
				// add original word
				ProcessWord( pFile, strKeyword, bAdd );
				nCount++;
			}
		}
	
		if ( nWindow == 0 )
		{
			ProcessWord( pFile, strKeyword, bAdd );
			nCount++;
		}
		else // make keywords using a sliding window
		{
			LPCTSTR pszPhrase = strKeyword.GetBuffer();
			TCHAR* pszToken = new TCHAR[ nWindow + 1 ];
			while ( _tcslen( pszPhrase ) >= nWindow )
			{
				_tcsncpy( pszToken, pszPhrase, nWindow );
				pszToken[ nWindow ] = 0;
				ProcessWord( pFile, (LPCTSTR)pszToken, bAdd );
				nCount++;
				pszPhrase++;
			}
			delete [] pszToken;
			return nCount; // we don't need to remove endings
		}
		
		if ( nLength >= 5 && Settings.Library.PartialMatch )
		{
			strKeyword = strWord.Left( nLength - 1 );
			ProcessWord( pFile, strKeyword, bAdd );
			nCount++;
			
			strKeyword = strWord.Left( nLength - 2 );
			ProcessWord( pFile, strKeyword, bAdd );
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
	
	if ( m_pWords.Lookup( strWord, pWord ) )
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
	
	//Add words to hash table
	for ( POSITION pos = m_pWords.GetStartPosition() ; pos ; )
	{
		CLibraryWord* pWord;
		CString strWord;
		
		m_pWords.GetNextAssoc( pos, strWord, pWord );
		
		CLibraryFile* pFileTemp = *(pWord->m_pList); 

		if ( pFileTemp->IsShared() )	// Check if the file is shared
		{
			if ( ( pFileTemp->IsGhost() ) || (UploadQueues.CanUpload( PROTOCOL_HTTP, pFileTemp, FALSE ) ) ) // Check if a queue exists
			{
				//Add the keywords to the table
				m_pTable->AddString( strWord );
/*
				CString str;
				str.Format( _T("Word Added: %s"), strWord );
				theApp.Message( MSG_DEFAULT, str );
			}
			else
			{
				CString str;
				str.Format( _T("Word not added: %s"), strWord );
				theApp.Message( MSG_DEFAULT, str );
*/
			}
		}
	}
	
	//Add sha1/ed2k hashes to hash table
	for ( POSITION pos = LibraryMaps.GetFileIterator() ; pos ; )
	{
		CLibraryFile* pFile = LibraryMaps.GetNextFile( pos );
		
		if (pFile->IsShared())	// Check if the file is shared
		{		
			if ( ( pFile->IsGhost() ) || ( UploadQueues.CanUpload( PROTOCOL_HTTP, pFile, FALSE ) ) ) // Check if a queue exists
			{
				//Add the hashes to the table
				if ( pFile->m_oSHA1 )
				{
					m_pTable->AddString( pFile->m_oSHA1.toUrn() );
				}
				if ( pFile->m_oED2K )
				{
					m_pTable->AddString( pFile->m_oED2K.toUrn() );
				}
/*
				CString str;
				str.Format( _T("File added: %s"), pFile->m_sName );
				theApp.Message( MSG_DEFAULT, str );
			}
			else
			{
				CString str;
				str.Format( _T("File not added: %s"), pFile->m_sName );
				theApp.Message( MSG_DEFAULT, str );
*/
			}
		}
	}
	
	m_bTable = TRUE;
	
	return TRUE;
}

void CLibraryDictionary::RebuildHashTable()	//Force table to re-build. (If queues changed, etc)
{
	m_bTable = FALSE;
	BuildHashTable();
}


//////////////////////////////////////////////////////////////////////
// CLibraryDictionary retreive hash table

CQueryHashTable* CLibraryDictionary::GetHashTable()
{
	CQuickLock oLock( Library.m_pSection );
	
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
		
		m_pWords.GetNextAssoc( pos, strWord, pWord );
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

CList< CLibraryFile* >* CLibraryDictionary::Search(CQuerySearch* pSearch, int nMaximum, BOOL bLocal)
{
	BuildHashTable();

	// Only check the hash when a search comes from other client. 
	if ( ! bLocal && ! m_pTable->Check( pSearch ) ) return NULL;

	DWORD nCookie = m_nSearchCookie++;

	CLibraryFile* pHit = NULL;

	for ( CQuerySearch::const_iterator pWordEntry = pSearch->begin(); pWordEntry != pSearch->end(); ++pWordEntry )
	{
		if ( pWordEntry->first[ 0 ] == '-' ) continue;

		CString sWord( pWordEntry->first, (int)pWordEntry->second );

		CLibraryWord* pWord;

		if ( m_pWords.Lookup( sWord, pWord ) )
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

	}

	size_t nLowerBound = pSearch->tableSize() >= 3
		? pSearch->tableSize() * 2 / 3
		: pSearch->tableSize();

	CList< CLibraryFile* >* pHits = NULL;
	int nCount = 0;

	for ( ; pHit ; pHit = pHit->m_pNextHit )
	{
		if ( pHit->m_nSearchCookie == nCookie && pHit->m_nSearchWords >= nLowerBound )
		{
			if ( pSearch->Match( pHit->GetSearchName(), pHit->m_nSize,
					pHit->m_pSchema ? (LPCTSTR)pHit->m_pSchema->m_sURI : NULL,
					pHit->m_pMetadata,
					pHit->m_oSHA1,
					pHit->m_oTiger,
					pHit->m_oED2K ) )
			{
				if ( ! pHits ) pHits = new CList< CLibraryFile* >;
				pHits->AddTail( pHit );

				if ( ! bLocal )
				{
					pHit->m_nHitsToday++;
					pHit->m_nHitsTotal++;
				}

				if ( pHit->m_nCollIndex )
				{
					if ( CLibraryFile* pCollection = LibraryMaps.LookupFile( pHit->m_nCollIndex, ! bLocal, TRUE ) )
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

