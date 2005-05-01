//
// VendorCache.h
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

#if !defined(AFX_VENDORCACHE_H__D5534D6B_0819_4C8F_B62B_9DD5CB3468AD__INCLUDED_)
#define AFX_VENDORCACHE_H__D5534D6B_0819_4C8F_B62B_9DD5CB3468AD__INCLUDED_

#pragma once

class CVendor;
class CXMLElement;


class CVendorCache
{
// Construction
public:
	CVendorCache();
	virtual ~CVendorCache();

// Attributes
public:
	CVendor*		m_pNull;
	CVendor*		m_pShareaza;
	CVendor*		m_pED2K;
protected:
	CMapStringToPtr	m_pMap;

// Operations
public:
	POSITION		GetIterator() const;
	CVendor*		GetNext(POSITION& pos) const;
	int				GetCount() const;
	CVendor*		Lookup(LPCSTR pszCode, BOOL bCreate = TRUE);
	CVendor*		Lookup(LPCWSTR pszCode, BOOL bCreate = TRUE);
	CVendor*		LookupByName(LPCTSTR pszName) const;
	void			Clear();
	BOOL			Load();
protected:
	BOOL			LoadFrom(CXMLElement* pXML);

};


class CVendor
{
// Construction
public:
	CVendor(LPCTSTR pszCode = NULL);
	virtual ~CVendor();

// Attributes
public:
	CString		m_sCode;
	CString		m_sName;
	CString		m_sLink;
	BOOL		m_bAuto;
public:
	BOOL		m_bChatFlag;
	BOOL		m_bHTMLBrowse;

// Operations
protected:
	BOOL		LoadFrom(CXMLElement* pXML);

	friend class CVendorCache;

};

extern CVendorCache VendorCache;


#endif // !defined(AFX_VENDORCACHE_H__D5534D6B_0819_4C8F_B62B_9DD5CB3468AD__INCLUDED_)
