//
// SchemaCache.h
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

#if !defined(AFX_SCHEMACACHE_H__45A0432F_B5D1_4196_BCDC_BC0AF9B2296A__INCLUDED_)
#define AFX_SCHEMACACHE_H__45A0432F_B5D1_4196_BCDC_BC0AF9B2296A__INCLUDED_

#pragma once

class CSchema;


class CSchemaCache
{
// Construction
public:
	CSchemaCache();
	virtual ~CSchemaCache();

// Attributes
protected:
	CMapStringToPtr	m_pURIs;
	CMapStringToPtr	m_pNames;

// Operations
public:
	BOOL		Load();
	void		Clear();

// Inlines
public:
	POSITION GetIterator() const
	{
		return m_pURIs.GetStartPosition();
	}
	
	CSchema* GetNext(POSITION& pos) const
	{
		CSchema* pSchema = NULL;
		CString strURI;
		m_pURIs.GetNextAssoc( pos, strURI, (void*&)pSchema );
		return pSchema;
	}
	
	CSchema* Get(LPCTSTR pszURI) const
	{
		if ( ! pszURI || ! *pszURI ) return NULL;
		CString strURI( pszURI );
		CharLower( strURI.GetBuffer() );
		strURI.ReleaseBuffer();
		CSchema* pSchema = NULL;
		return ( m_pURIs.Lookup( strURI, (void*&)pSchema ) ) ? pSchema : NULL;
	}
	
	CSchema* Guess(LPCTSTR pszName) const
	{
		if ( ! pszName || ! *pszName ) return NULL;
		CString strName( pszName );
		CharLower( strName.GetBuffer() );
		strName.ReleaseBuffer();
		CSchema* pSchema = NULL;
		return m_pNames.Lookup( strName, (void*&)pSchema ) ? pSchema : NULL;
	}

};

extern CSchemaCache	SchemaCache;

#endif // !defined(AFX_SCHEMACACHE_H__45A0432F_B5D1_4196_BCDC_BC0AF9B2296A__INCLUDED_)
