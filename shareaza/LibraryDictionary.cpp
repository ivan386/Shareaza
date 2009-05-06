//
// LibraryDictionary.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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
	Clear();
	delete m_pTable;
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary add and remove

void CLibraryDictionary::AddFile(const CLibraryFile& oFile)
{
	bool bCanUpload( CanUpload( oFile ) );

	ProcessFile( oFile, true, bCanUpload );

	if ( bCanUpload )
		AddHashes( oFile );
}

void CLibraryDictionary::RemoveFile(const CLibraryFile& oFile)
{
	bool bCanUpload( CanUpload( oFile ) );

	ProcessFile( oFile, false, bCanUpload );

	// TODO: Always invalidate the table when removing a hashed
	// file... is this wise???  It will happen all the time.

	if ( oFile.m_oSHA1 || oFile.m_oTiger || oFile.m_oED2K || oFile.m_oBTH
		|| oFile.m_oMD5 )
	{
		m_bValid = false;
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary process file

void CLibraryDictionary::ProcessFile(
	const CLibraryFile& oFile, const bool bAdd, const bool bCanUpload)
{
	ProcessPhrase( oFile, oFile.GetSearchName(), bAdd, bCanUpload );

	if ( oFile.m_pMetadata && oFile.m_pSchema )
	{
		ProcessWord( oFile, oFile.m_pSchema->GetURI(), bAdd, bCanUpload );
		ProcessPhrase( oFile, oFile.GetMetadataWords(), bAdd, bCanUpload );
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary phrase parser

void CLibraryDictionary::ProcessPhrase(
	const CLibraryFile& oFile, const CString& strPhrase, const bool bAdd,
	const bool bCanUpload)
{
	CString strWord;
	WORD boundary[ 2 ] = { C3_NOTAPPLICABLE, C3_NOTAPPLICABLE };
	WORD nKanaType[ 2 ] = { C3_NOTAPPLICABLE, C3_NOTAPPLICABLE };
	int nPos( 0 ), nPrevWord( 0 ), nNextWord( 0 );

	for ( ; nPos < strPhrase.GetLength() ; ++nPos )
	{
		// boundary[ 0 ] -- previous character;
		// boundary[ 1 ] -- current character;
		boundary[ 0 ] = boundary[ 1 ];
		const WCHAR nChar( strPhrase[ nPos ] );
		GetStringTypeW( CT_CTYPE3, &nChar, 1, &boundary[ 1 ] );

		nKanaType[ 1 ] = boundary[ 1 ] & ( C3_KATAKANA | C3_HIRAGANA );
		if ( nKanaType[ 1 ] == ( C3_KATAKANA | C3_HIRAGANA )
			&& nKanaType[ 0 ] )
		{
			boundary[ 1 ] = boundary[ 0 ];
			nKanaType[ 1 ] = nKanaType[ 0 ];
		}

		// Letter is a character if it is alpha-numeric
		// Work-around for WinXP: Katakana/Hiragana diacritic characters in
		// the NLS table aren't marked as being alpha (Win2000 & >Vista are OK)
		const bool bCharacter( ( boundary[ 1 ] & C3_ALPHA )
			|| iswdigit( nChar )
			|| ( nKanaType[ 1 ] && ( boundary[ 1 ] & C3_DIACRITIC ) ) );

		if ( !bCharacter || ( nPos && boundary[ 0 ] != boundary[ 1 ] ) )
		{
			// Join two phrases if the previous was a single character word.
			// Joining single characters breaks GDF compatibility completely,
			// but Shareaza 2.2 and above are not really following GDF about
			// word length limit for ASIAN chars, so merging needs to be done.
			if ( nPos > nNextWord )
			{
				strWord = strPhrase.Mid( nNextWord, nPos - nNextWord );
				MakeKeywords( oFile, strWord, boundary[ 0 ], bAdd, bCanUpload );
			}
			nPrevWord = nNextWord;
			nNextWord = nPos;
			nNextWord += ( bCharacter ? 0 : 1 );
		}
	}

	strWord = strPhrase.Mid( nPrevWord, nPos - nPrevWord );
	MakeKeywords( oFile, strWord, boundary[ 0 ], bAdd, bCanUpload );
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary keyword maker

void CLibraryDictionary::MakeKeywords(
	const CLibraryFile& oFile, const CString& strWord, const WORD nWordType,
	const bool bAdd, const bool bCanUpload)
{
	const int nLength( strWord.GetLength() );

	if ( !nLength )
		return;

	ProcessWord( oFile, strWord, bAdd, bCanUpload );

	if ( nWordType & C3_HIRAGANA )
	{
		// Continuous Hiragana string can be structured with a few prefix or
		// postfix or both. Assume Prefix/Postfix length is MAX 2 chars.
		// Note: according to GDF, minimum char length for Hiragana is 2 char
		if ( nLength < 3 )
			return;

		// take off last 1 char
		ProcessWord( oFile, strWord.Left( nLength - 1 ), bAdd, bCanUpload );

		// take off first 1 char
		ProcessWord( oFile, strWord.Right( nLength - 1 ), bAdd, bCanUpload );

		if ( nLength < 4 )
			return;

		// take off last 2 chars
		ProcessWord( oFile, strWord.Left( nLength - 2 ), bAdd, bCanUpload );

		// take off first 2 chars
		ProcessWord( oFile, strWord.Right( nLength - 2 ), bAdd, bCanUpload );

		// take off first 1 & last 1 chars
		ProcessWord( oFile, strWord.Mid( 1, nLength - 2 ), bAdd, bCanUpload );

		if ( nLength < 5 )
			return;

		// take off first 1 & last 2 chars
		ProcessWord( oFile, strWord.Mid( 1, nLength - 3 ), bAdd, bCanUpload );

		// take off first 2 & last 1 chars
		ProcessWord( oFile, strWord.Mid( 2, nLength - 3 ), bAdd, bCanUpload );

		if ( nLength < 6 )
			return;

		// take off first 2 & last 2 chars
		ProcessWord( oFile, strWord.Mid( 2, nLength - 4 ), bAdd, bCanUpload );

		return;
	}

	if ( nWordType & C3_KATAKANA )
	{
		// Continuous Katakana string does not have Prefix or postfix
		// but can contain a few words in one continuous string
		// Assume MAX number of Words contained in one continuous Katakana
		// string as Two words
		// Note: according to GDF, minimum char length for Katakana is 2 char
		// moreover, it is not known how long the prefix/postfix
		// not even the length of chars in one word.
		if ( nLength < 3 )
			return;

		for ( int nLen( 2 ) ; nLen < nLength ; ++nLen )
		{
			ProcessWord( oFile, strWord.Left( nLen ), bAdd, bCanUpload );
			ProcessWord( oFile, strWord.Right( nLen ), bAdd, bCanUpload );
		}

		return;
	}

	if ( nWordType & C3_IDEOGRAPH )
	{
		// Continuous Kanji string may have Prefix or postfix
		// but can contain a few words in one continuous string
		// Assume MAX number of Words contained in one continuous Kanji string
		// as Two words
		// Note: according to GDF, minimum char length for Kanji is 1 char
		// moreover, it is not known how long the prefix/postfix
		// not even the length of chars in one word.
		if ( nLength < 2 )
			return;

		for ( int nLen( 1 ) ; nLen < nLength ; ++nLen )
		{
			ProcessWord( oFile, strWord.Left( nLen ), bAdd, bCanUpload );
			ProcessWord( oFile, strWord.Right( nLen ), bAdd, bCanUpload );
		}

		return;
	}

	if ( Settings.Library.PartialMatch )
	{
		if ( nLength < 4 )
			return;

		ProcessWord( oFile, strWord.Left( nLength - 1 ), bAdd, bCanUpload );

		if ( nLength < 5 )
			return;

		ProcessWord( oFile, strWord.Left( nLength - 2 ), bAdd, bCanUpload );
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary word add and remove

void CLibraryDictionary::ProcessWord(
	const CLibraryFile& oFile, const CString& strWord, const bool bAdd,
	const bool bCanUpload)
{
	FilePtrList* pFilePtrList( NULL );

	if ( m_oWordMap.Lookup( strWord, pFilePtrList ) )
	{
		if ( bAdd )
		{
			if ( pFilePtrList->GetTail() != &oFile )
			{
				pFilePtrList->AddTail( &oFile );
				if ( bCanUpload && !m_pTable->CheckString( strWord ) )
					m_pTable->AddString( strWord );
			}
		}
		else
		{
			POSITION pos( pFilePtrList->Find( &oFile ) );
			if ( pos )
			{
				pFilePtrList->RemoveAt( pos );

				if ( pFilePtrList->IsEmpty() )
				{
					m_oWordMap.RemoveKey( strWord );
					delete pFilePtrList;

					if ( bCanUpload )
						m_bValid = false;
				}
			}
		}
	}
	else if ( bAdd )
	{
		pFilePtrList = new FilePtrList;
		pFilePtrList->AddTail( &oFile );
		m_oWordMap.SetAt( strWord, pFilePtrList );

		if ( bCanUpload && !BuildHashTable() )
			m_pTable->AddString( strWord );
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary check if file is shareable

const bool CLibraryDictionary::CanUpload(const CLibraryFile& oFile) const
{
	// Check if the file is shared and is a ghost file or can be uploaded
	if ( oFile.IsShared() )
	{
		if ( oFile.IsGhost()
			|| UploadQueues.CanUpload( PROTOCOL_HTTP, &oFile, false ) )
		{
			return true;
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary add hashes to query table

void CLibraryDictionary::AddHashes(const CLibraryFile& oFile)
{
	//Add the hashes to the table
	if ( oFile.m_oSHA1 )
		m_pTable->AddExactString( oFile.m_oSHA1.toUrn() );

	if ( oFile.m_oED2K )
		m_pTable->AddExactString( oFile.m_oED2K.toUrn() );

	if ( oFile.m_oBTH )
		m_pTable->AddExactString( oFile.m_oBTH.toUrn() );

	if ( oFile.m_oMD5 )
		m_pTable->AddExactString( oFile.m_oMD5.toUrn() );
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary build hash table

const bool CLibraryDictionary::BuildHashTable()
{
	if ( !m_pTable )
	{
		m_pTable = new CQueryHashTable();
		m_pTable->Create();
	}

	if ( m_bValid )
		return false;

	m_pTable->Clear();

	// Add words to hash table
	for ( POSITION pos( m_oWordMap.GetStartPosition() ) ; pos ; )
	{
		CString strWord;
		FilePtrList* pFilePtrList( NULL );
		m_oWordMap.GetNextAssoc( pos, strWord, pFilePtrList );

		for ( POSITION pos( pFilePtrList->GetHeadPosition() ) ; pos ; )
		{
			const CLibraryFile& oFile( *pFilePtrList->GetNext( pos ) );

			// Check if the file can be uploaded
			if ( CanUpload( oFile ) )
			{
				// Add the keyword to the table
				m_pTable->AddString( strWord );
				break;
			}
		}
	}

	// Add sha1/ed2k hashes to hash table
	for ( POSITION pos( LibraryMaps.GetFileIterator() ) ; pos ; )
	{
		const CLibraryFile& oFile( *LibraryMaps.GetNextFile( pos ) );

		// Check if the file can be uploaded
		if ( CanUpload( oFile ) )
			AddHashes( oFile );
	}

	m_bValid = true;

	return true;
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary rebuild hash table
//
// Force hash table to re-build. (If queues changed, etc)

void CLibraryDictionary::RebuildHashTable()
{
	m_bValid = false;
	BuildHashTable();
}


//////////////////////////////////////////////////////////////////////
// CLibraryDictionary retreive hash table

const CQueryHashTable& CLibraryDictionary::GetHashTable()
{
	BuildHashTable();

	return *m_pTable;
}

//////////////////////////////////////////////////////////////////////
// CLibraryDictionary clear

void CLibraryDictionary::Clear()
{
	for ( POSITION pos( m_oWordMap.GetStartPosition() ) ; pos ; )
	{
		CString strWord;
		FilePtrList* pFilePtrList( NULL );

		m_oWordMap.GetNextAssoc( pos, strWord, pFilePtrList );
		delete pFilePtrList;
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

FilePtrList* CLibraryDictionary::Search(
	const CQuerySearch& oSearch, const int nMaximum, const bool bLocal,
	const bool bAvailableOnly)
{
	// Only check the hash when a search comes from other client.
	if ( !bLocal && !m_pTable->Check( oSearch ) )
		return NULL;

	BuildHashTable();

	++m_nSearchCookie;
	const CLibraryFile* pHit( NULL );

	const CQuerySearch::const_iterator pLastWordEntry( oSearch.end() );
	for ( CQuerySearch::const_iterator pWordEntry( oSearch.begin() )
		; pWordEntry != pLastWordEntry ; ++pWordEntry )
	{
		if ( pWordEntry->first[ 0 ] == _T('-') )
			continue;

		CString strWord( pWordEntry->first, static_cast< int >( pWordEntry->second ) );
		FilePtrList* pFilePtrList( NULL );

		if ( m_oWordMap.Lookup( strWord, pFilePtrList ) )
		{
			for ( POSITION pos( pFilePtrList->GetHeadPosition() ) ; pos ; )
			{
				const CLibraryFile* pFile( pFilePtrList->GetNext( pos ) );

				if ( bAvailableOnly && pFile->IsGhost() )
					continue;

				if ( !bLocal && !pFile->IsShared() )
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

	size_t nLowerBound( oSearch.tableSize() >= 3
		? oSearch.tableSize() * 2 / 3 : oSearch.tableSize() );

	FilePtrList* pHits( NULL );
	for ( ; pHit ; pHit = pHit->m_pNextHit )
	{
		ASSERT( pHit->m_nSearchCookie == m_nSearchCookie );

		if ( pHit->m_nSearchWords < nLowerBound )
			continue;

		if ( oSearch.Match( pHit->GetSearchName(), pHit->m_nSize,
			pHit->m_pSchema ? (LPCTSTR)pHit->m_pSchema->GetURI() : NULL,
			pHit->m_pMetadata, pHit->m_oSHA1, pHit->m_oTiger, pHit->m_oED2K,
			pHit->m_oBTH, pHit->m_oMD5 ) )
		{
			if ( !pHits )
				pHits = new FilePtrList;

			pHits->AddTail( pHit );

			if ( !bLocal )
			{
				++pHit->m_nHitsToday;
				++pHit->m_nHitsTotal;
			}

			if ( pHit->m_nCollIndex )
			{
				const CLibraryFile* pCollection( LibraryMaps.LookupFile(
					pHit->m_nCollIndex, !bLocal, bAvailableOnly ) );

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
					ASSERT( false );
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
	if ( ar.IsStoring() )
	{
		ar << (UINT)m_oWordMap.GetCount();
	}
	else
	{
		if ( nVersion >= 29 )
		{
			UINT nWordsCount( 0u );
			ar >> nWordsCount;
			m_oWordMap.InitHashTable( GetBestHashTableSize( nWordsCount ) );
		}
	}
}
