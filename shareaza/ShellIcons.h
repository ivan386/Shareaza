//
// ShellIcons.h
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


class CShellIcons
{
// Construction
public:
	CShellIcons();

// Operations
public:
	void	Clear();
	int		Get(LPCTSTR pszFile, int nSize);
	int		Add(HICON hIcon, int nSize);
	HICON	ExtractIcon(int nIndex, int nSize);
	BOOL	Lookup(LPCTSTR pszType, HICON* phSmallIcon, HICON* phLargeIcon, CString* psName, CString* psMIME, HICON* phHugeIcon = NULL);
	CString	GetTypeString(LPCTSTR pszFile);
	void	AttachTo(CListCtrl* const pList, int nSize) const;
	void	AttachTo(CTreeCtrl* const pTree) const;
	BOOL	Draw(CDC* pDC, int nIcon, int nSize, int nX, int nY, COLORREF crBack = CLR_NONE, BOOL bSelected = FALSE) const;

private:
	CImageList		m_i16;
	CImageList		m_i32;
	CImageList		m_i48;
	CMap< CString, const CString&, int, int > m_m16;
	CMap< CString, const CString&, int, int > m_m32;
	CMap< CString, const CString&, int, int > m_m48;

	CShellIcons(const CShellIcons&);
	CShellIcons& operator=(const CShellIcons&);
};

extern CShellIcons ShellIcons;

// Predefined icons
enum
{
	SHI_FILE = 0,
	SHI_EXECUTABLE = 1,
	SHI_COMPUTER = 2,
	SHI_FOLDER_CLOSED = 3,
	SHI_FOLDER_OPEN = 4,
	SHI_LOCKED = 5
};

enum
{
	SHI_O_LOCKED = 1
};
