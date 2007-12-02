//
// ThumbCache.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

#if !defined(AFX_THUMBCACHE_H__EDC6DF1A_071C_4171_A709_C7CCD7FAE56A__INCLUDED_)
#define AFX_THUMBCACHE_H__EDC6DF1A_071C_4171_A709_C7CCD7FAE56A__INCLUDED_

#pragma once

class CImageFile;

typedef struct
{
	DWORD		nIndex;
	DWORD		nOffset;
	DWORD		nLength;
	FILETIME	pTime;
} THUMB_INDEX;


class CThumbCache
{
// Construction
public:
	CThumbCache();
	virtual ~CThumbCache();

// Attributes
protected:
	CCriticalSection m_pSection;
protected:
	CString		m_sPath;
	CFile		m_pFile;
	BOOL		m_bOpen;
	CSize		m_szThumb;
protected:
	DWORD			m_nOffset;
	THUMB_INDEX*	m_pIndex;
	DWORD			m_nIndex;
	DWORD			m_nBuffer;

// Operations
public:
	BOOL	Load(LPCTSTR pszPath, CSize* pszThumb, DWORD nIndex, CImageFile* pImage);
	BOOL	Store(LPCTSTR pszPath, CSize* pszThumb, DWORD nIndex, CImageFile* pImage);
	static BOOL Cache(LPCTSTR pszPath, CSize* pszThumb, DWORD nIndex, CImageFile* pImage = NULL);
	void	Close();
protected:
	BOOL	Prepare(LPCTSTR pszPath, CSize* pszThumb, BOOL bCreate);
	BOOL	GetFileTime(LPCTSTR pszPath, FILETIME* pTime);
public:
	static BOOL Purge(LPCTSTR pszPath);

};

#endif // !defined(AFX_THUMBCACHE_H__EDC6DF1A_071C_4171_A709_C7CCD7FAE56A__INCLUDED_)
