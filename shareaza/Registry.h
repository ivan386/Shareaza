//
// Registry.h
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

#if !defined(AFX_REGISTRY_H____INCLUDED_)
#define AFX_REGISTRY_H____INCLUDED_

#pragma once

class CRegistry 
{
// Construction
public:
	CRegistry();
	virtual ~CRegistry();

public:
	void	DisplayErrorMessageBox(DWORD);
	CString GetString(LPCTSTR pLocation, LPCTSTR pKeyName, LPCTSTR pDefault);
	int		GetInt(LPCTSTR pLocation, LPCTSTR pKeyName, int iDefault);
	DWORD	GetDword(LPCTSTR pLocation, LPCTSTR pKeyName, DWORD dwDefault);
	double	GetFloat(LPCTSTR pLocation, LPCTSTR pKeyName, double fDefault);
	BOOL	SetInt(LPCTSTR pLocation, LPCTSTR pKeyName, int iValue);
	BOOL	SetString(LPCTSTR pLocation, LPCTSTR pKeyName, LPCTSTR sValue);
};

#endif