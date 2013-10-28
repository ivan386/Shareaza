//
// RegExp.h 
//
// Copyright (c) Shareaza Development Team, 2010.
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

#ifdef REGEXP_EXPORTS
#define REGEXP_API __declspec(dllexport)
#else
#define REGEXP_API __declspec(dllimport)
#endif

namespace RegExp
{

// Returns TRUE if szContent matches szRegExp regular expression (case insensitive)
REGEXP_API BOOL Match(LPCTSTR szRegExp, LPCTSTR szContent);

// Splits szContent according szRegExp regular expression (case insensitive)
// Returns number of string in function allocated pszResult (array of strings)
// pszResult must be freed by GlobalFree() function
REGEXP_API size_t Split(LPCTSTR szRegExp, LPCTSTR szContent, LPTSTR* pszResult);

};
