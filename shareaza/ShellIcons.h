//
// ShellIcons.h
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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

#if !defined(AFX_SHELLICONS_H__D01FAB3B_B7C7_46AE_B4AF_A1451B322A27__INCLUDED_)
#define AFX_SHELLICONS_H__D01FAB3B_B7C7_46AE_B4AF_A1451B322A27__INCLUDED_

#pragma once


class CShellIcons  
{
// Construction
public:
	CShellIcons();
	virtual ~CShellIcons();
	
// Operations
public:
	void	Clear();
	int		Get(LPCTSTR pszFile, int nSize);
	int		Add(HICON hIcon, int nSize);
	HICON	ExtractIcon(int nIndex, int nSize);
public:
	BOOL	Lookup(LPCTSTR pszType, HICON* phSmallIcon, HICON* phLargeIcon, CString* psName, CString* psMIME, HICON* phHugeIcon = NULL);
	CString	GetTypeString(LPCTSTR pszFile);
public:
	void	Draw(CDC* pDC, int nIcon, int nSize, int nX, int nY, COLORREF crBack = CLR_NONE, BOOL bSelected = FALSE);

// Inlines
public:
	inline CImageList* GetObject(int nSize) const
	{
		switch ( nSize )
		{
		case 16:
			return (CImageList*)&m_i16;
		case 32:
			return (CImageList*)&m_i32;
		case 48:
			return (CImageList*)&m_i48;
		default:
			return NULL;
		}
	}

	inline HIMAGELIST GetHandle(int nSize) const
	{
		switch ( nSize )
		{
		case 16:
			return m_i16.m_hImageList;
		case 32:
			return m_i32.m_hImageList;
		case 48:
			return m_i48.m_hImageList;
		default:
			return NULL;
		}
	}

// Attributes
protected:
	CImageList		m_i16;
	CImageList		m_i32;
	CImageList		m_i48;
protected:
	CMapStringToPtr	m_m16;
	CMapStringToPtr	m_m32;
	CMapStringToPtr	m_m48;

	HINSTANCE m_hUser;
	UINT (WINAPI *m_pfnPrivate)(LPCTSTR, int, int, int, HICON*, UINT*, UINT, UINT);

};

extern CShellIcons ShellIcons;

enum
{
	SHI_FILE,
	SHI_PLUS,
	SHI_MINUS,
	SHI_TICK,
	SHI_BUSY,
	SHI_FIREWALL,
	SHI_UNSTABLE,
	SHI_COMPUTER,
	SHI_EXECUTABLE,
	SHI_CHAT,
	SHI_BROWSE,
	SHI_FOLDER_CLOSED,
	SHI_FOLDER_OPEN,
	SHI_LOCKED,
	SHI_SEARCH,
	SHI_PARTIAL,
	SHI_CHEVRON,
	SHI_STAR,
	SHI_PREVIEW,
	SHI_COLLECTION,
	SHI_FAKE,
	SHI_COMMERCIAL,
	SHI_MAX
};

enum
{
	SHI_O_NULL,
	SHI_O_LOCKED,
	SHI_O_PARTIAL,
	SHI_O_COLLECTION,
	SHI_O_COMMERCIAL,
	SHI_O_MAX
};

#endif // !defined(AFX_SHELLICONS_H__D01FAB3B_B7C7_46AE_B4AF_A1451B322A27__INCLUDED_)
