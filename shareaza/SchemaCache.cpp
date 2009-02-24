//
// SchemaCache.cpp
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
#include "Settings.h"
#include "SchemaCache.h"
#include "Schema.h"
#include "XML.h"

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
}

CSchemaCache::~CSchemaCache()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CSchemaCache load

int CSchemaCache::Load()
{
	WIN32_FIND_DATA pFind;
	CString strPath;
	HANDLE hSearch;
	int nCount;

#ifdef _DEBUG
	__int64 nStartTotal = GetMicroCount();
#endif

	Clear();

	strPath.Format( _T("%s\\Schemas\\*.xsd"), (LPCTSTR)Settings.General.Path );
	hSearch = FindFirstFile( strPath, &pFind );
	if ( hSearch == INVALID_HANDLE_VALUE ) return 0;
	nCount = 0;

	do
	{
#ifdef _DEBUG
		__int64 nStart = GetMicroCount();
#endif
		strPath.Format( _T("%s\\Schemas\\%s"), (LPCTSTR)Settings.General.Path, pFind.cFileName );
		
		CSchema* pSchema = new CSchema();
		if ( pSchema && pSchema->Load( strPath ) )
		{
			CString strURI( pSchema->GetURI() );
			ToLower( strURI );

			m_pURIs.SetAt( strURI, pSchema );
			
			CString strName( pSchema->m_sSingular );
			ToLower( strName );

			m_pNames.SetAt( strName, pSchema );
		}
		else
		{
			delete pSchema;
			pSchema = NULL;
		}

#ifdef _DEBUG
		__int64 nEnd = GetMicroCount();
		TRACE( _T("Schema \"%s\" load time : %I64i ms : %s\n"), strPath,
			( nEnd - nStart ) / 1000, pSchema ? _T("SUCCESS") : _T("FAILED") );
#endif
	}
	while ( FindNextFile( hSearch, &pFind ) );

	FindClose( hSearch );

#ifdef _DEBUG
	__int64 nEndTotal = GetMicroCount();
	TRACE( _T("Schemas load time : %I64i ms\n"),
		( nEndTotal - nStartTotal ) / 1000 );
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
}

CXMLElement* CSchemaCache::Decode(LPSTR pszData, int nLength, CSchema*& pSchema)
{
	// HACK: Fix embedded zeros
	char* p = pszData;
	for ( int i = 0; i < nLength - 2; ++i, ++p )
		// Fix <tag attribute="valueZ/> -> <tag attribute="value"/>
		if ( *p == _T('\0') && *(p + 1) == _T('/') && *(p + 2) == _T('>') )
			*p = _T('"');

	CString strXML = UTF8Decode( pszData, nLength );

	CXMLElement* pXML = CXMLElement::FromString( strXML );
	if ( ! pXML )
		pXML = AutoDetectSchema( strXML );

	if ( pXML )
	{
		pSchema = Get( pXML->GetAttributeValue( CXMLAttribute::schemaName, NULL ) );
		if ( ! pSchema )
		{
			// Schemas do not match by URN, get first element to compare
			// with names map of schemas (which are singulars)
			if ( CXMLElement* pElement = pXML->GetFirstElement() )
			{
				pSchema = SchemaCache.Guess( pElement->GetName() );
			}
			else // has no plural envelope
			{
				pSchema = SchemaCache.Guess( pXML->GetName() );
			}
			if ( ! pSchema )
			{
				pXML->Delete();
				pXML = NULL;
			}
		}
	}

	return pXML;
}

CXMLElement* CSchemaCache::AutoDetectSchema(LPCTSTR pszInfo)
{
	if ( _tcsstr( pszInfo, _T(" Kbps") ) != NULL &&
		 _tcsstr( pszInfo, _T(" kHz ") ) != NULL )
	{
		return AutoDetectAudio( pszInfo );
	}

	return NULL;
}

CXMLElement* CSchemaCache::AutoDetectAudio(LPCTSTR pszInfo)
{
	int nBitrate	= 0;
	int nFrequency	= 0;
	int nMinutes	= 0;
	int nSeconds	= 0;
	BOOL bVariable	= FALSE;

	if ( _stscanf( pszInfo, _T("%i Kbps %i kHz %i:%i"), &nBitrate, &nFrequency,
		&nMinutes, &nSeconds ) != 4 )
	{
		bVariable = TRUE;
		if ( _stscanf( pszInfo, _T("%i Kbps(VBR) %i kHz %i:%i"), &nBitrate, &nFrequency,
			&nMinutes, &nSeconds ) != 4 )
			return NULL;
	}

	CXMLElement* pXML = new CXMLElement( NULL, _T("audio") );

	CString strValue;
	strValue.Format( _T("%lu"), nMinutes * 60 + nSeconds );
	pXML->AddAttribute( _T("seconds"), strValue );

	strValue.Format( bVariable ? _T("%lu~") : _T("%lu"), nBitrate );
	pXML->AddAttribute( _T("bitrate"), strValue );

	return pXML;
}
