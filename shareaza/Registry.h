//
// Registry.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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


struct CRegistry
{
	static CString GetString(LPCTSTR pszSection, LPCTSTR pszName, LPCTSTR pszDefault = NULL, LPCTSTR pszSubKey = NULL, BOOL bIgnoreHKCU = FALSE);

	static DWORD GetDword(LPCTSTR pszSection, LPCTSTR pszName, DWORD dwDefault = 0, LPCTSTR pszSubKey = NULL);

	inline static bool GetBool(LPCTSTR pszSection, LPCTSTR pszName, bool bDefault = false, LPCTSTR pszSubKey = NULL)
	{
		return ( GetDword( pszSection, pszName, ( bDefault ? 1 : 0 ), pszSubKey ) != 0 );
	}

	inline static int GetInt(LPCTSTR pszSection, LPCTSTR pszName, int nDefault = 0, LPCTSTR pszSubKey = NULL)
	{
		return (int)GetDword( pszSection, pszName, (DWORD)nDefault, pszSubKey );
	}

	inline static double GetFloat(LPCTSTR pszSection, LPCTSTR pszName, double fDefault = 0.0f, LPCTSTR pszSubKey = NULL)
	{
		CString buf;
		buf.Format( _T("%lg"), fDefault );
		return _tstof( GetString( pszSection, pszName, buf, pszSubKey ) );
	}

	static BOOL	SetString(LPCTSTR pszSection, LPCTSTR pszName, LPCTSTR pszValue, LPCTSTR pszSubKey = NULL);

	static BOOL	SetDword(LPCTSTR pszSection, LPCTSTR pszName, DWORD nValue, LPCTSTR pszSubKey = NULL);

	inline static BOOL SetBool(LPCTSTR pszSection, LPCTSTR pszName, bool bValue, LPCTSTR pszSubKey = NULL)
	{
		return SetDword( pszSection, pszName, ( bValue ? 1 : 0 ), pszSubKey );
	}

	inline static BOOL SetInt(LPCTSTR pszSection, LPCTSTR pszName, int nValue, LPCTSTR pszSubKey = NULL)
	{
		return SetDword( pszSection, pszName, (DWORD)nValue, pszSubKey );
	}

	inline static BOOL SetFloat(LPCTSTR pszSection, LPCTSTR pszName, double fValue, LPCTSTR pszSubKey = NULL)
	{
		CString buf;
		buf.Format( _T("%lg"), fValue );
		return SetString( pszSection, pszName, buf, pszSubKey );
	}
};
