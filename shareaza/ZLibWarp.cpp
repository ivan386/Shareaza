//
// ZLibWarp.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "ZLibWarp.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

auto_array< BYTE > CZLib::Compress(LPCVOID pInput, DWORD nInput, DWORD* pnOutput, DWORD nSuggest)
{
	if ( ! nInput )
	{
		*pnOutput = 0;
		return auto_array< BYTE >();
	}

	*pnOutput = nSuggest ? nSuggest : compressBound( nInput );

	// Allocate a new buffer of pnOutput bytes
	auto_array< BYTE >pBuffer( new BYTE[ *pnOutput ] );
	if ( ! pBuffer.get() )
	{
		*pnOutput = 0;
		return auto_array< BYTE >();
	}

	// Compress the data at pInput into pBuffer, putting how many bytes it wrote under pnOutput
	int nRes = compress2( pBuffer.get(), pnOutput, (const BYTE *)pInput, nInput, Settings.Connection.ZLibCompressionLevel );
	if ( nRes != Z_OK )
	{
		// The compress function reported error
		ASSERT( Z_BUF_ERROR != nRes );	// TODO
		*pnOutput = 0;
		return auto_array< BYTE >();
	}

	return pBuffer;
}

BYTE* CZLib::Compress2(LPCVOID pInput, DWORD nInput, DWORD* pnOutput, DWORD nSuggest)
{
	if ( ! nInput )
	{
		*pnOutput = 0;
		return NULL;
	}

	*pnOutput = nSuggest ? nSuggest : compressBound( nInput );

	// Allocate a new buffer of pnOutput bytes
	BYTE* pBuffer = (BYTE*)malloc( *pnOutput );
	if ( ! pBuffer )
	{
		*pnOutput = 0;
		return NULL;
	}

	// Compress the data at pInput into pBuffer, putting how many bytes it wrote under pnOutput
	int nRes = compress2( pBuffer, pnOutput, (const BYTE *)pInput, nInput, Settings.Connection.ZLibCompressionLevel );
	if ( nRes != Z_OK )
	{
		// The compress function reported error
		ASSERT( Z_BUF_ERROR != nRes );	// TODO
		free( pBuffer );
		*pnOutput = 0;
		return NULL;
	}

	return pBuffer;
}

auto_array< BYTE > CZLib::Decompress(LPCVOID pInput, DWORD nInput, DWORD* pnOutput)
{
	// Guess how big the data will be decompressed, use nSuggest, or just guess it will be 3 times as big
	for ( DWORD nSuggest = nInput * 3; ; nSuggest *= 2 )
	{
		*pnOutput = nSuggest;

		auto_array< BYTE > pBuffer( new BYTE[ *pnOutput ] );
		if ( ! pBuffer.get() )
		{
			// Out of memory
			*pnOutput = 0;
			return auto_array< BYTE >();
		}

		// Uncompress the data from pInput into pBuffer, writing how big it is now in pnOutput
		int nRes = uncompress( pBuffer.get(), pnOutput, (const BYTE *)pInput, nInput );
		if ( Z_OK == nRes )
			return pBuffer;

		if ( Z_BUF_ERROR != nRes )
		{
			// Decompression error
			*pnOutput = 0;
			return auto_array< BYTE >();
		}
	}
}

BYTE* CZLib::Decompress2(LPCVOID pInput, DWORD nInput, DWORD* pnOutput)
{
	BYTE* pBuffer = NULL;

	// Guess how big the data will be decompressed, use nSuggest, or just guess it will be 3 times as big
	for ( DWORD nSuggest = nInput * 3; ; nSuggest *= 2 )
	{
		*pnOutput = nSuggest;

		BYTE* pNewBuffer = (BYTE*)realloc( pBuffer, *pnOutput );
		if ( ! pNewBuffer )
		{
			// Out of memory
			free( pBuffer );
			*pnOutput = 0;
			return NULL;
		}
		pBuffer = pNewBuffer;

		// Uncompress the data from pInput into pBuffer, writing how big it is now in pnOutput
		int nRes = uncompress( pBuffer, pnOutput, (const BYTE *)pInput, nInput );
		if ( Z_OK == nRes )
			return pBuffer;

		if ( Z_BUF_ERROR != nRes )
		{
			// Decompression error
			free( pBuffer );
			*pnOutput = 0;
			return NULL;
		}
	}
}
