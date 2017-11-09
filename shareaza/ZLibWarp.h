//
// ZLibWarp.h
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

// CZLib makes it easier to use the zlib compression library
// http://shareazasecurity.be/wiki/index.php?title=Developers.Code.CZLib

#pragma once

class CZLib  
{
public:
	// Compress nInput bytes at pInput to a new returned buffer of size pnOutput
	// Note: After use free memory by delete[] function
	static auto_array< BYTE > Compress(LPCVOID pInput, DWORD nInput, DWORD* pnOutput, DWORD nSuggest = 0);

	// Compress nInput bytes at pInput to a new returned buffer of size pnOutput
	// Note: After use free memory by free() function
	static BYTE* Compress2(LPCVOID pInput, DWORD nInput, DWORD* pnOutput, DWORD nSuggest = 0);

	// Decompress nInput bytes at pInput to a new returned buffer of size pnOutput
	// Note: After use free memory by delete[] function
	static auto_array< BYTE > Decompress(LPCVOID pInput, DWORD nInput, DWORD* pnOutput);

	// Decompress nInput bytes at pInput to a new returned buffer of size pnOutput
	// Note: After use free memory by free() function
	static BYTE* Decompress2(LPCVOID pInput, DWORD nInput, DWORD* pnOutput);
};
