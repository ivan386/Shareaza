//
// RelatedSearch.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2011.
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
#include "MatchObjects.h"
#include "SharedFile.h"
#include "RelatedSearch.h"
#include "QuerySearch.h"
#include "QueryHit.h"
#include "Schema.h"
#include "SchemaCache.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define MIN_LENGTH		4


//////////////////////////////////////////////////////////////////////
// CRelatedSearch construction

CRelatedSearch::CRelatedSearch(CMatchFile* pFile)
{
	if ( pFile != NULL )
	{
		m_oSHA1		= pFile->m_oSHA1;
		m_oTiger	= pFile->m_oTiger;
		m_oED2K		= pFile->m_oED2K;
		m_oBTH		= pFile->m_oBTH;
		m_oMD5		= pFile->m_oMD5;
		m_sName		= pFile->m_sName;

		m_pSchema	= pFile->GetBestSchema();
		m_pXML		= NULL;
		m_bXML		= FALSE;

		if ( m_pSchema != NULL )
		{
			m_pXML = new CXMLElement( NULL, m_pSchema->m_sSingular );
			m_bXML = TRUE;

			pFile->AddHitsToXML( m_pXML );
		}
	}
	else
	{
		m_bXML = FALSE;
		m_oSHA1.clear();
		m_oTiger.clear();
		m_oED2K.clear();
		m_oBTH.clear();
		m_oMD5.clear();
		m_pSchema = NULL;
		m_pXML = NULL;
	}
}

CRelatedSearch::CRelatedSearch(CLibraryFile* pFile)
{
	if ( pFile != NULL )
	{
		m_oSHA1		= pFile->m_oSHA1;
		m_oTiger	= pFile->m_oTiger;
		m_oED2K		= pFile->m_oED2K;
		m_oBTH		= pFile->m_oBTH;
		m_oMD5		= pFile->m_oMD5;
		m_sName		= pFile->m_sName;
		m_pSchema	= pFile->m_pSchema;
		m_bXML		= ( pFile->m_pMetadata != NULL );
		m_pXML		= m_bXML ? pFile->m_pMetadata->Clone() : NULL;
	}
	else
	{
		m_bXML = FALSE;
		m_oSHA1.clear();
		m_oTiger.clear();
		m_oED2K.clear();
		m_oBTH.clear();
		m_oMD5.clear();
		m_pSchema = NULL;
		m_pXML = NULL;
	}
}

CRelatedSearch::~CRelatedSearch()
{
	if ( m_bXML ) delete m_pXML;
}

//////////////////////////////////////////////////////////////////////
// CRelatedSearch search for this

BOOL CRelatedSearch::CanSearchForThis()
{
	return HasHash();
}

BOOL CRelatedSearch::RunSearchForThis()
{
	if ( ! CanSearchForThis() ) return FALSE;
	CQuerySearchPtr pSearch = new CQuerySearch();
	pSearch->m_oSHA1	= m_oSHA1;
	pSearch->m_oTiger	= m_oTiger;
	pSearch->m_oED2K	= m_oED2K;
	pSearch->m_oBTH		= m_oBTH;
	pSearch->m_oMD5		= m_oMD5;
	return CQuerySearch::OpenWindow( pSearch ) != NULL;
}

//////////////////////////////////////////////////////////////////////
// CRelatedSearch search for similar

BOOL CRelatedSearch::CanSearchForSimilar()
{
	return m_sName.GetLength() >= MIN_LENGTH;
}

BOOL CRelatedSearch::RunSearchForSimilar()
{
	if ( ! CanSearchForSimilar() ) return FALSE;
	CQuerySearchPtr pSearch = new CQuerySearch();
	pSearch->m_sSearch = Tokenise( m_sName );

	// Support "Related Search" in ed2k
	pSearch->m_oSimilarED2K	= m_oED2K;

	return CQuerySearch::OpenWindow( pSearch ) != NULL;
}

//////////////////////////////////////////////////////////////////////
// CRelatedSearch search for artist

BOOL CRelatedSearch::CanSearchForArtist()
{
	if ( m_pSchema == NULL || m_pXML == NULL ) return FALSE;
	CString str = m_pXML->GetAttributeValue( _T("artist") );
	return str.GetLength() >= MIN_LENGTH;
}

BOOL CRelatedSearch::RunSearchForArtist()
{
	if ( ! CanSearchForArtist() ) return FALSE;
	CQuerySearchPtr pSearch = new CQuerySearch();
	pSearch->m_pSchema	= m_pSchema ? m_pSchema : SchemaCache.Get( CSchema::uriAudio );
	pSearch->m_pXML		= pSearch->m_pSchema->Instantiate();
	CXMLElement* pXML	= pSearch->m_pXML->AddElement( pSearch->m_pSchema->m_sSingular );
	pXML->AddAttribute( _T("artist"), Tokenise( m_pXML->GetAttributeValue( _T("artist") ) ) );
	return CQuerySearch::OpenWindow( pSearch ) != NULL;
}

//////////////////////////////////////////////////////////////////////
// CRelatedSearch search for album

BOOL CRelatedSearch::CanSearchForAlbum()
{
	if ( m_pSchema == NULL || m_pXML == NULL ) return FALSE;
	CString str = m_pXML->GetAttributeValue( _T("album") );
	return str.GetLength() >= MIN_LENGTH;
}

BOOL CRelatedSearch::RunSearchForAlbum()
{
	if ( ! CanSearchForAlbum() ) return FALSE;
	CQuerySearchPtr pSearch = new CQuerySearch();
	pSearch->m_pSchema	= m_pSchema ? m_pSchema : SchemaCache.Get( CSchema::uriAudio );
	pSearch->m_pXML		= pSearch->m_pSchema->Instantiate();
	CXMLElement* pXML	= pSearch->m_pXML->AddElement( pSearch->m_pSchema->m_sSingular );
	pXML->AddAttribute( _T("album"), Tokenise( m_pXML->GetAttributeValue( _T("album") ) ) );
	return CQuerySearch::OpenWindow( pSearch ) != NULL;
}

//////////////////////////////////////////////////////////////////////
// CRelatedSearch search for series

BOOL CRelatedSearch::CanSearchForSeries()
{
	if ( m_pSchema == NULL || m_pXML == NULL ) return FALSE;
	CString str = m_pXML->GetAttributeValue( _T("series") );
	return str.GetLength() >= MIN_LENGTH;
}

BOOL CRelatedSearch::RunSearchForSeries()
{
	if ( ! CanSearchForSeries() ) return FALSE;
	CQuerySearchPtr pSearch = new CQuerySearch();
	pSearch->m_pSchema	= m_pSchema ? m_pSchema : SchemaCache.Get( CSchema::uriVideo );
	pSearch->m_pXML		= pSearch->m_pSchema->Instantiate();
	CXMLElement* pXML	= pSearch->m_pXML->AddElement( pSearch->m_pSchema->m_sSingular );
	pXML->AddAttribute( _T("series"), Tokenise( m_pXML->GetAttributeValue( _T("series") ) ) );
	return CQuerySearch::OpenWindow( pSearch ) != NULL;
}

//////////////////////////////////////////////////////////////////////
// CRelatedSearch string token processing

CString CRelatedSearch::Tokenise(LPCTSTR psz)
{
	int nChars = 0;
	CString str, strTemp(psz);

	// remove diacritics
	int nSource = FoldString( MAP_COMPOSITE, psz, -1, NULL, 0 ); //_tcslen( psz );
	FoldString( MAP_COMPOSITE, psz, -1, strTemp.GetBuffer( nSource ), nSource );
	strTemp.ReleaseBuffer( nSource );
	psz = strTemp.GetBuffer( nSource );

	int nLastPoint = strTemp.ReverseFind( '.' );
	if ( nLastPoint > 0 )
		nLastPoint = strTemp.GetLength() - nLastPoint - 1;

	for ( ; *psz ; psz++ )
	{
		if ( *psz == '.' && int( _tcslen( psz ) ) == nLastPoint )
		{
			break;
		}
		else if ( _istalnum( *psz ) )
		{
			str += *psz;
			nChars ++;
		}
		else if ( nChars > 1 && !( *psz >= 0x02B0 && *psz <= 0x036F ) )
		{
			str += ' ';
			nChars = 0;
		}
		else if ( nChars == 1 && !( *psz >= 0x02B0 && *psz <= 0x036F ) )
		{
			str = str.Left( str.GetLength() - 1 );
			nChars = 0;
		}
	}

	return str;
}
