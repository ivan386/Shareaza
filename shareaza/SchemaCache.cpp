//
// SchemaCache.cpp
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
#include "Settings.h"
#include "SchemaCache.h"
#include "Schema.h"
#include "XML.h"
#include "ZLibWarp.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CSchemaCache SchemaCache;


//////////////////////////////////////////////////////////////////////
// CSchemaCache construction

CSchemaCache::CSchemaCache()
{
	// experimental values
	m_pURIs.InitHashTable( 61 );
	m_pNames.InitHashTable( 61 );
	m_pTypeFilters.InitHashTable( 1021 );
}

CSchemaCache::~CSchemaCache()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CSchemaCache load

int CSchemaCache::Load()
{
#ifdef _DEBUG
	__int64 nStartTotal = GetMicroCount();
#endif

	Clear();

	CString strPath;
	strPath.Format( _T("%s\\Schemas\\*.xsd"), (LPCTSTR)Settings.General.Path );

	WIN32_FIND_DATA pFind = {};
	HANDLE hSearch = FindFirstFile( strPath, &pFind );
	if ( hSearch == INVALID_HANDLE_VALUE )
		return 0;

	int nCount = 0;
	do
	{
#ifdef _DEBUG
		__int64 nStart = GetMicroCount();
#endif
		strPath.Format( _T("%s\\Schemas\\%s"), (LPCTSTR)Settings.General.Path, pFind.cFileName );

		CSchema* pSchema = new CSchema();
		if ( pSchema && pSchema->Load( strPath ) )
		{
			m_pURIs.SetAt( pSchema->GetURI(), pSchema );
			m_pNames.SetAt( pSchema->m_sSingular, pSchema );

			for ( POSITION pos = pSchema->GetFilterIterator(); pos; )
			{
				CString sType;
				BOOL bResult;
				pSchema->GetNextFilter( pos, sType, bResult );
				if ( bResult )
				{
					m_pTypeFilters.SetAt( sType, pSchema );
				}
			}

			++nCount;
		}
		else
		{
			delete pSchema;
			pSchema = NULL;
		}

#ifdef _DEBUG
		__int64 nEnd = GetMicroCount();
		TRACE( "Schema \"%s\" load time : %I64i ms : %s\n", (LPCSTR)CT2A( strPath ),
			( nEnd - nStart ) / 1000, pSchema ? "SUCCESS" : "FAILED" );
#endif
	}
	while ( FindNextFile( hSearch, &pFind ) );

	FindClose( hSearch );

#ifdef _DEBUG
	__int64 nEndTotal = GetMicroCount();
	TRACE( _T("Schemas load time : %I64i ms. Found %d types.\n"),
		( nEndTotal - nStartTotal ) / 1000, m_pTypeFilters.GetCount() );
#endif

	return nCount;
}

//////////////////////////////////////////////////////////////////////
// CSchemaCache clear

void CSchemaCache::Clear()
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		delete GetNext( pos );
	}

	m_pURIs.RemoveAll();
	m_pNames.RemoveAll();
	m_pTypeFilters.RemoveAll();
}

bool CSchemaCache::Normalize(CSchemaPtr& pSchema, CXMLElement*& pXML) const
{
	pSchema = NULL;

	if ( pXML )
	{
		pSchema = SchemaCache.Get( pXML->GetAttributeValue( CXMLAttribute::schemaName ) );
		if ( ! pSchema )
		{
			// Schemas do not match by URN, get first element to compare
			// with names map of schemas (which are singulars)
			if ( CXMLElement* pElement = pXML->GetFirstElement() )
			{
				pSchema = Guess( pElement->GetName() );
				if ( pSchema )
				{
					// Strip envelope
					pElement->Detach();
					pXML->Delete();
					pXML = pElement;
				}
			}
			if ( ! pSchema ) // has no plural envelope
			{
				pSchema = Guess( pXML->GetName() );
			}
		}
	}

	return ( pSchema != NULL );
}

CString CSchemaCache::GetFilter(LPCTSTR pszURI) const
{
	if ( CSchemaPtr pSchema = Get( pszURI ) )
	{
		LPCTSTR pszURIType;
		if ( pszURI == CSchema::uriImageAll )
			pszURIType = CSchema::uriImage;
		else if ( pszURI == CSchema::uriVideoAll )
			pszURIType = CSchema::uriVideo;
		else if ( pszURI == CSchema::uriMusicAll )
			pszURIType = CSchema::uriAudio;
		else if ( pszURI == CSchema::uriApplicationAll )
			pszURIType = CSchema::uriApplication;
		else if ( pszURI == CSchema::uriAllFiles )
			pszURIType = NULL;
		else
		{
			ASSERT( FALSE );
			return CString();
		}

		CString sTypes;
		if ( CSchemaPtr pSchemaType = Get( pszURIType ) )
		{
			for ( POSITION pos = pSchemaType->GetFilterIterator(); pos; )
			{
				CString sType;
				BOOL bResult;
				pSchemaType->GetNextFilter( pos, sType, bResult );
				if ( bResult )
				{
					if ( sTypes.GetLength() )
						sTypes += _T(";*.");
					else
						sTypes += _T("|*.");
					sTypes += sType;
				}
			}
		}

		if ( sTypes.IsEmpty() )
			return pSchema->m_sHeaderTitle + _T("|*.*|");
		else
			return pSchema->m_sHeaderTitle + sTypes + _T("|");
	}

	return CString();
}
