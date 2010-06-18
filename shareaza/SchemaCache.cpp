//
// SchemaCache.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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
#include "Zlib.h"

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
			CString strURI( pSchema->GetURI() );
			strURI.MakeLower();

			m_pURIs.SetAt( strURI, pSchema );
			
			CString strName( pSchema->m_sSingular );
			strName.MakeLower();

			m_pNames.SetAt( strName, pSchema );

			++nCount;
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

CXMLElement* CSchemaCache::Decode(BYTE* szData, DWORD nLength, CSchemaPtr& pSchema)
{
	auto_array< BYTE > pTmp;
	if ( nLength >= 9 && _strnicmp( (LPCSTR)szData, "{deflate}", 9 ) == 0 )
	{
		// Deflate data
		DWORD nRealSize;
		pTmp = CZLib::Decompress( (LPCSTR)szData + 9, nLength - 9, &nRealSize );
		if ( ! pTmp.get() )
			// Invalid data
			return NULL;
		szData = pTmp.get();
		nLength = nRealSize;
	}
	else if ( nLength >= 11 && _strnicmp( (LPCSTR)szData, "{plaintext}", 11 ) == 0 )
	{
		// Plain text with long header
		szData += 11;
		nLength -= 11;
	}
	else if ( nLength >= 2 && _strnicmp( (LPCSTR)szData, "{}", 2 ) == 0 )
	{
		// Plain text with short header
		szData += 2;
		nLength -= 2;
	}

	// Fix <tag attribute="valueZ/> -> <tag attribute="value"/>
	for ( DWORD i = 1; i + 2 < nLength ; i++ )
		if ( szData[ i ] == 0 && szData[ i + 1 ] == '/' && szData[ i + 2 ] == '>' )
			szData[ i ] = '\"';

	// Decode XML
	CXMLElement* pXML = CXMLElement::FromBytes( szData, nLength, FALSE );
	if ( ! pXML )
		// Reconstruct XML from non-XML legacy data
		pXML = AutoDetectSchema( CString( (LPCSTR)szData, nLength ) );
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
