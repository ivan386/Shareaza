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

// Takes a pointer to memory and how many bytes are there
// Compresses the memory into a new buffer this function allocates
// Returns a pointer to the new buffer, and writes its size under pnOutput
LPBYTE CZLib::Compress(LPCVOID pInput, DWORD nInput, DWORD* pnOutput, DWORD nSuggest)
{
	// If we were given nSuggest, use it as the output buffer size, otherwise call compressBound to set it
	*pnOutput = nSuggest ? nSuggest : compressBound( nInput ); // compressBound just uses math to guess, it doesn't look at the data

	// Allocate a new buffer of pnOutput bytes
	BYTE* pBuffer = new BYTE[ *pnOutput ];

	// Compress the data at pInput into pBuffer, putting how many bytes it wrote under pnOutput
	if ( compress(            // Compress data from one buffer to another, returns Z_OK 0 false if it works
		pBuffer,              // The output buffer where ZLib can write compressed data
		pnOutput,             // Reads how much space it has there, writes how much space it used
		(const BYTE *)pInput, // The source buffer with data to compress
		nInput ) )            // The number of bytes there
	{
		// The compress function reported error
		delete [] pBuffer; // Delete our temporary buffer
		return NULL;       // Report error
	}

	// The pBuffer buffer is too big, make a new one exactly the right size, copy the data, delete the first, and return the second
	BYTE* pOutput = new BYTE[ *pnOutput ];     // Allocate a new buffer exactly big enough to hold the bytes compress wrote
	CopyMemory( pOutput, pBuffer, *pnOutput ); // Copy the compressed bytes from the old buffer to the new one
	delete [] pBuffer;                         // Delete the old buffer
	return pOutput;                            // Return the new one
}

//////////////////////////////////////////////////////////////////////
// CZLib decompression

// Takes a pointer to compressed input bytes, and how many are there
// Decompresses the memory into a new buffer this function allocates
// Returns a pointer to the new buffer, and writes its size under pnOutput
LPBYTE CZLib::Decompress(LPCVOID pInput, DWORD nInput, DWORD* pnOutput, DWORD nSuggest)
{
	// Guess how big the data will be decompressed, use nSuggest, or just guess it will be 6 times as big
	*pnOutput = nSuggest ? nSuggest : nInput * 6;

	// Allocate a buffer that big
	BYTE* pBuffer = new BYTE[ *pnOutput ];

	// Uncompress the data from pInput into pBuffer, writing how big it is now in pnOutput
	if ( int nError = uncompress( // Uncompress data
		pBuffer,                  // Destination buffer where uncompress can write uncompressed data
		pnOutput,                 // Reads how much space it has there, and writes how much space it used
		(const BYTE *)pInput,     // Source buffer of compressed data
		nInput ) )                // Number of bytes there
	{
		// The uncompress function returned an error, delete the buffer we allocated and return error
		delete [] pBuffer;
		return NULL;
	}

	// The pBuffer buffer is bigger than necessary, move its bytes into one perfectly sized, and return it
	BYTE* pOutput = new BYTE[ *pnOutput ];     // Make a new buffer exactly the right size
	CopyMemory( pOutput, pBuffer, *pnOutput ); // Copy the data from the one that's too big
	delete [] pBuffer;                         // Delete the one that's too big
	return pOutput;                            // Return a pointer to the perfectly sized one
}
