//
// SchemaCache.h
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

#pragma once

#include "Schema.h"


class CSchemaCache
{
// Construction
public:
	CSchemaCache();
	virtual ~CSchemaCache();

// Attributes
protected:
	CMap< CString, const CString&, CSchemaPtr, CSchemaPtr > m_pURIs;
	CMap< CString, const CString&, CSchemaPtr, CSchemaPtr >	m_pNames;

// Operations
public:
	int			Load();
	void		Clear();

// Inlines
public:
	POSITION GetIterator() const
	{
		return m_pURIs.GetStartPosition();
	}
	
	CSchemaPtr GetNext(POSITION& pos) const
	{
		CSchemaPtr pSchema = NULL;
		CString strURI;
		m_pURIs.GetNextAssoc( pos, strURI, pSchema );
		return pSchema;
	}
	
	CSchemaPtr Get(LPCTSTR pszURI) const
	{
		if ( ! pszURI || ! *pszURI ) return NULL;
		CString strURI( pszURI );
		strURI.MakeLower();

		CSchemaPtr pSchema = NULL;
		return ( m_pURIs.Lookup( strURI, pSchema ) ) ? pSchema : NULL;
	}
	
	CSchemaPtr Guess(LPCTSTR pszName) const
	{
		if ( ! pszName || ! *pszName ) return NULL;
		CString strName( pszName );
		strName.MakeLower();

		CSchemaPtr pSchema = NULL;

		// A quick hack for Limewire documents schema
		// ToDo: Remove it when the full schema mapping is ready
		if ( strName == L"document" )
			return m_pNames.Lookup( L"wordprocessing", pSchema ) ? pSchema : NULL;

		return m_pNames.Lookup( strName, pSchema ) ? pSchema : NULL;
	}

	// Decode metadata and Schema from text or XML deflated or plain
	CXMLElement* Decode(BYTE* pszData, DWORD nLength, CSchemaPtr& pSchema);
	static CXMLElement* AutoDetectSchema(LPCTSTR pszInfo);
	static CXMLElement* AutoDetectAudio(LPCTSTR pszInfo);

private:
	CSchemaCache(const CSchemaCache&);
	CSchemaCache& operator=(const CSchemaCache&);
};

extern CSchemaCache	SchemaCache;

// Compare two schema URIs with schema mapping
inline bool CheckURI(const CString& strURI1, LPCTSTR szURI2)
{
	if ( strURI1.CompareNoCase( szURI2 ) == 0 )
		return true;
	CSchemaPtr pSchema1 = SchemaCache.Get( strURI1 );
	if ( pSchema1 && pSchema1->CheckURI( szURI2 ) )
		return true;
	CSchemaPtr pSchema2 = SchemaCache.Get( szURI2 );
	if ( pSchema2 && pSchema2->CheckURI( strURI1 ) )
		return true;
	return false;
}
