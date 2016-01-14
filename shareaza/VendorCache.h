//
// VendorCache.h
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

#pragma once

class CVendor;
class CXMLElement;

typedef const CVendor* CVendorPtr;


class CVendorCache
{
public:
	CVendorCache();
	~CVendorCache();

	CVendorPtr		m_pNull;

	// Lookup 4-bytes vendor code (ASCII without terminating null)
	inline CVendorPtr Lookup(LPCSTR pszCode) const
	{
		ASSERT( pszCode );
		if ( pszCode && pszCode[ 0 ] && pszCode[ 1 ] && pszCode[ 2 ] && pszCode[ 3 ] )
		{
			const WCHAR szCode[5] = { (WCHAR)pszCode[0], (WCHAR)pszCode[1], (WCHAR)pszCode[2], (WCHAR)pszCode[3], 0 };
			CVendorPtr pVendor;
			if ( m_pCodeMap.Lookup( szCode, pVendor ) )
				return pVendor;
		}
		return NULL;
	}

	// Lookup 4-chars vendor code (with terminating null)
	inline CVendorPtr Lookup(LPCWSTR pszCode) const
	{
		ASSERT( pszCode );
		if ( pszCode && pszCode[0] && pszCode[1] && pszCode[2] && pszCode[3] && ! pszCode[4] )
		{
			CVendorPtr pVendor;
			if ( m_pCodeMap.Lookup( pszCode, pVendor ) )
				return pVendor;
		}
		return NULL;
	}

	// Lookup by code or by name
	CVendorPtr		LookupByName(LPCTSTR pszName) const;

	// Load data from Vendors.xml
	BOOL			Load();

	// Is specified vendor a Shareaza-powered vendor?
	bool			IsExtended(LPCTSTR pszCode) const;

protected:
	typedef CAtlMap< CString, CVendorPtr, CStringElementTraitsI< CString > > CVendorMap;

	// Vendor code map
	CVendorMap m_pCodeMap;
	// Name map
	CVendorMap m_pNameMap;

	void			Clear();
	BOOL			LoadFrom(const CXMLElement* pXML);
};


class CVendor
{
public:
	CVendor();
	CVendor(LPCTSTR pszCode);

	CString		m_sCode;
	CString		m_sName;
	CString		m_sLink;
	bool		m_bChatFlag;		// Support chatting
	bool		m_bBrowseFlag;		// Supports browsing
	bool		m_bExtended;		// Shareaza-powered

protected:
	BOOL		LoadFrom(const CXMLElement* pXML);

	friend class CVendorCache;
};

extern CVendorCache VendorCache;
