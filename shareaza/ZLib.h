//
// ZLib.h
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

// CZLib makes it easier to use the zlib compression library
// http://wiki.shareaza.com/static/Developers.Code.CZLib

// Make the compiler only include the lines here once, this is the same thing as pragma once
#if !defined(AFX_ZLIB_H__3ABC5B39_501F_41B0_828A_B7CDFAD0F73B__INCLUDED_)
#define AFX_ZLIB_H__3ABC5B39_501F_41B0_828A_B7CDFAD0F73B__INCLUDED_

// Only include the lines beneath this one once
#pragma once

// Wraps the compress and decompress data compression functions of the ZLib compression library
class CZLib
{

public:

	// Compress and decompress nInput bytes at pInput to a new returned buffer of size pnOutput
	static LPBYTE Compress(LPCVOID pInput, DWORD nInput, DWORD* pnOutput, DWORD nSuggest = 0);
	static LPBYTE Decompress(LPCVOID pInput, DWORD nInput, DWORD* pnOutput, DWORD nSuggest = 0);
};

// End the group of lines to only include once, pragma once doesn't require an endif at the bottom
#endif // !defined(AFX_ZLIB_H__3ABC5B39_501F_41B0_828A_B7CDFAD0F73B__INCLUDED_)
