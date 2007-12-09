//
// Registry.h
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

#pragma once


class CRegistry
{
// Construction
public:
	CRegistry();
	~CRegistry();

// Operations
public:
	CString GetString(LPCTSTR pszSection, LPCTSTR pszName, LPCTSTR pszDefault = NULL, HKEY hMainKey = HKEY_CURRENT_USER, LPCTSTR pszSubKey = NULL);
	int		GetInt(LPCTSTR pszSection, LPCTSTR pszName, int nDefault = 0, HKEY hMainKey = HKEY_CURRENT_USER, LPCTSTR pszSubKey = NULL);
	BOOL	GetBool(LPCTSTR pszSection, LPCTSTR pszName, BOOL nDefault = FALSE, HKEY hMainKey = HKEY_CURRENT_USER, LPCTSTR pszSubKey = NULL);
	DWORD	GetDword(LPCTSTR pszSection, LPCTSTR pszName, DWORD dwDefault = 0);
	double	GetFloat(LPCTSTR pszSection, LPCTSTR pszName, double fDefault = 0.0f);
public:
	BOOL	SetString(LPCTSTR pszSection, LPCTSTR pszName, LPCTSTR pszValue, HKEY hMainKey = HKEY_CURRENT_USER, LPCTSTR pszSubKey = NULL);
	BOOL	SetInt(LPCTSTR pszSection, LPCTSTR pszName, int nValue, HKEY hMainKey = HKEY_CURRENT_USER, LPCTSTR pszSubKey = NULL);

// Implementation
protected:
	void	DisplayErrorMessageBox(LPCTSTR pszName, DWORD nErrorCode);
};
