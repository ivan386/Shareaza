//
// ZLib.cpp
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "ZLib.h"

#include <zlib.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CZLib compression

LPBYTE CZLib::Compress(LPCVOID pInput, DWORD nInput, DWORD* pnOutput, DWORD nSuggest)
{
	*pnOutput = nSuggest ? nSuggest : nInput * 2;
	
	BYTE* pBuffer = new BYTE[ *pnOutput ];
	
	if ( compress( pBuffer, pnOutput, (const BYTE*)pInput, nInput ) )
	{
		delete [] pBuffer;
		return NULL;
	}
	
	BYTE* pOutput = new BYTE[ *pnOutput ];
	CopyMemory( pOutput, pBuffer, *pnOutput );
	delete [] pBuffer;
	
	return pOutput;
}

//////////////////////////////////////////////////////////////////////
// CZLib decompression

LPBYTE CZLib::Decompress(LPCVOID pInput, DWORD nInput, DWORD* pnOutput, DWORD nSuggest)
{
	*pnOutput = nSuggest ? nSuggest : nInput * 6;
	
	BYTE* pBuffer = new BYTE[ *pnOutput ];
	
	if ( int nError = uncompress( pBuffer, pnOutput, (const BYTE*)pInput, nInput ) )
	{
		delete [] pBuffer;
		return NULL;
	}
	
	BYTE* pOutput = new BYTE[ *pnOutput ];
	CopyMemory( pOutput, pBuffer, *pnOutput );
	delete [] pBuffer;
	
	return pOutput;
}

