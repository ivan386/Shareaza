//
// Buffer.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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

// CBuffer holds some memory, and takes care of allocating and freeing it itself
// http://shareazasecurity.be/wiki/index.php?title=Developers.Code.CBuffer

#include "StdAfx.h"
#include "Buffer.h"

#ifdef ZLIB_H
	#include "ZLibWarp.h"
#endif // ZLIB_H

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define BLOCK_SIZE  1024       // Change the allocated size of the buffer in 1 KB sized blocks
#define BLOCK_MASK  0xFFFFFC00 // Aids in rounding to the next biggest KB of size

///////////////////////////////////////////////////////////////////////////////
// CBuffer construction

CBuffer::CBuffer() :
	m_pNext		( NULL )	// This object isn't in a list yet
,	m_pBuffer	( NULL )	// No memory block has been allocated for this object yet
,	m_nBuffer	( 0 )		// The size of the memory block is 0
,	m_nLength	( 0 )		// No bytes have been written here yet
{
}

CBuffer::~CBuffer()
{
	// If the member variable points to some memory, free it
	if ( m_pBuffer ) free( m_pBuffer );
}

///////////////////////////////////////////////////////////////////////////////
// CBuffer add

// Add data to the buffer
void CBuffer::Add(const void* __restrict pData, const size_t nLength) throw()
{
	// If the text is blank, don't do anything
	if ( pData == NULL ) return;

	if ( ! EnsureBuffer( nLength ) ) return;

	// Copy the given memory into the end of the memory block
	CopyMemory( m_pBuffer + m_nLength, pData, nLength );

	// Add the length of the new memory to the total length in the buffer
	m_nLength += static_cast< DWORD >( nLength );
}

///////////////////////////////////////////////////////////////////////////////
// CBuffer insert

// Takes offset, a position in the memory block to insert some new memory at
// Inserts the memory there, shifting anything after it further to the right
void CBuffer::Insert(const DWORD nOffset, const void* __restrict pData, const size_t nLength)
{
	ASSERT( pData );
	if ( pData == NULL ) return;

	if ( ! EnsureBuffer( nLength ) ) return;

	// Cut the memory block sitting in the buffer in two, slicing it at offset and shifting that part forward nLength
	MoveMemory(
		m_pBuffer + nOffset + nLength, // Destination is the offset plus the length of the memory block to insert
		m_pBuffer + nOffset,           // Source is at the offset
		m_nLength - nOffset );         // Length is the size of the memory block beyond the offset

	// Now that there is nLength of free space in the buffer at nOffset, copy the given memory to fill it
	CopyMemory(
		m_pBuffer + nOffset, // Destination is at the offset in the buffer
		pData,               // Source is the given pointer to the memory to insert
		nLength );           // Length is the length of that memory

	// Add the length of the new memory to the total length in the buffer
	m_nLength += static_cast< DWORD >( nLength );
}

///////////////////////////////////////////////////////////////////////////////
// CBuffer remove

// Takes a number of bytes
// Removes this number from the start of the buffer, shifting the memory after it to the start
void CBuffer::Remove(const size_t nLength) throw()
{
	if ( nLength >= m_nLength )
	{
		ASSERT( nLength == m_nLength );
		m_nLength = 0;
	}
	else if ( nLength )
	{
		m_nLength -= static_cast< DWORD >( nLength );
		MoveMemory( m_pBuffer, m_pBuffer + nLength, m_nLength );
	}
}

///////////////////////////////////////////////////////////////////////////////
// CBuffer add utilities

// Takes another CBuffer object, and a number of bytes there to copy, or the default -1 to copy the whole thing
// Moves the memory from pBuffer into this one
// Returns the number of bytes moved
DWORD CBuffer::AddBuffer(CBuffer* pBuffer, const size_t nLength)
{
	ASSERT( pBuffer && pBuffer != this );
	if ( pBuffer == NULL || pBuffer == this )
		return 0;

	// primitive overflow protection (relevant for 64bit)
	if ( nLength > INT_MAX ) return 0;

	// If the call specified a length, use it, otherwise use the length of pBuffer
	if( pBuffer->m_nLength < nLength )
	{
		Add( pBuffer->m_pBuffer, pBuffer->m_nLength );	// Copy the memory across
		pBuffer->Clear();								// Remove the memory from the source buffer
		return pBuffer->m_nLength;						// Report how many bytes we moved
	}
	else
	{
		Add( pBuffer->m_pBuffer, nLength );				// Copy the memory across
		pBuffer->Remove( nLength );						// Remove the memory from the source buffer
		return static_cast< DWORD >( nLength );			// Report how many bytes we moved
	}
}

void CBuffer::Attach(CBuffer* pBuffer)
{
	ASSERT( pBuffer && pBuffer != this );
	if ( pBuffer == NULL || pBuffer == this )
		return;

	if ( m_pBuffer ) free( m_pBuffer );
	m_pBuffer = pBuffer->m_pBuffer;
	pBuffer->m_pBuffer = NULL;

	m_nBuffer = pBuffer->m_nBuffer;
	pBuffer->m_nBuffer = 0;

	m_nLength = pBuffer->m_nLength;
	pBuffer->m_nLength = 0;
}

// Takes a pointer to some memory, and the number of bytes we can read there
// Adds them to this buffer, except in reverse order
void CBuffer::AddReversed(const void *pData, const size_t nLength)
{
	ASSERT( pData );
	if ( pData == NULL ) return;

	// Make sure this buffer has enough memory allocated to hold another nLength bytes
	if ( ! EnsureBuffer( nLength ) ) return;

	// Copy nLength bytes from pData to the end of the buffer, except in reverse order
	ReverseBuffer( pData, m_pBuffer + m_nLength, nLength );

	// Record the new length
	m_nLength += static_cast< DWORD >( nLength );
}

// Takes a number of new bytes we're about to add to this buffer
// Makes sure the buffer will be big enough to hold them, allocating more memory if necessary
bool CBuffer::EnsureBuffer(const size_t nLength) throw()
{
	// Limit buffer size to a signed int. This is the most that can be sent/received from a socket in one call.
	if ( nLength > 0xffffffff - m_nBuffer ) return false;

	// If the size of the buffer minus the size filled is bigger than or big enough for the given length, do nothing
	if ( m_nBuffer - m_nLength >= nLength )
	{
		// There is enough room to write nLength bytes without allocating anything

		// If the buffer is larger than 512 KB, but what it needs to hold is less than 256 KB
		if ( m_nBuffer > 0x80000 && m_nLength + nLength < 0x40000 )
		{
			// Reallocate it to make it half as big
			const DWORD nBuffer = 0x40000;
			BYTE* pBuffer = (BYTE*)realloc( m_pBuffer, nBuffer ); // This may move the block, returning a different pointer
			if ( ! pBuffer )
				// Out of memory - original block is left unchanged. It's ok.
				return true;
			m_nBuffer = nBuffer;
			m_pBuffer = pBuffer;
		}
		return true;
	}

	// Make m_nBuffer the size of what's written plus what's requested
	DWORD nBuffer = m_nLength + static_cast< DWORD >( nLength );

	// Round that up to the nearest multiple of 1024, or 1 KB
	nBuffer = ( nBuffer + BLOCK_SIZE - 1 ) & BLOCK_MASK;

	// Reallocate the memory block to this size
	BYTE* pBuffer = (BYTE*)realloc( m_pBuffer, nBuffer ); // May return a different pointer
	if ( ! pBuffer ) 
		// Out of memory - original block is left unchanged. Error.
		return false;
	m_nBuffer = nBuffer;
	m_pBuffer = pBuffer;
	return true;
}

// Convert Unicode text to ASCII and add it to the buffer
void CBuffer::Print(const LPCWSTR pszText, const size_t nLength, const UINT nCodePage)
{
	// primitive overflow protection (relevant for 64bit)
	if ( nLength > INT_MAX ) return;

	// If the text is blank or no memory, don't do anything
	ASSERT( pszText );
	if ( pszText == NULL ) return;

	// Find out the required buffer size, in bytes, for the translated string
	int nBytes = WideCharToMultiByte( nCodePage, 0, pszText,
		static_cast< int >( nLength ), NULL, 0, NULL, NULL );

	// Make sure the buffer is big enough for this, making it larger if necessary
	if ( ! EnsureBuffer( nBytes ) ) return;

	// Convert the Unicode string into ASCII characters in the buffer
	WideCharToMultiByte( nCodePage, 0, pszText, static_cast< int >( nLength ),
		(LPSTR)( m_pBuffer + m_nLength ), nBytes, NULL, NULL );
	m_nLength += nBytes;
}

///////////////////////////////////////////////////////////////////////////////
// CBuffer read string helper

// Takes a maximum number of bytes to examine at the start of the buffer, and a code page which is ASCII by default
// Reads the given number of bytes as ASCII characters, copying them into a string
// Returns the text in a string, which will contain Unicode or ASCII characters depending on the compile
CString CBuffer::ReadString(const size_t nBytes, const UINT nCodePage) const
{
	// Make a new blank string to hold the text we find
	CString str;

	// Set nSource to whichever is smaller, the number of bytes in the buffer, or the number we can look at there
	int nSource = (int)( nBytes < m_nLength ? nBytes : m_nLength );

	// Find out how many wide characters a buffer must be able to hold to convert this text to Unicode, null terminator not included
	int nLength = MultiByteToWideChar( // If the bytes "hello" are in the buffer, and nSource is 5, nLength will be 5 also
		nCodePage,                     // Code page to use, CP_ACP ANSI code page for ASCII text, the default
		0,                             // No special options about difficult to translate characters
		(LPCSTR)m_pBuffer,             // Use the start of this buffer as the source, where the ASCII text is
		nSource,                       // Convert this number of bytes there
		NULL,                          // No output buffer, we want to find out how long one must be
		0 );

	// Convert the ASCII characters at the start of this buffer to Unicode
	MultiByteToWideChar(          // Convert ASCII text to Unicode
		nCodePage,                // Code page to use, CP_ACP ANSI code page for ASCII text, the default
		0,                        // No special options about difficult to translate characters
		(LPCSTR)m_pBuffer,        // Use the start of this buffer as the source, where the ASCII text is
		nSource,                  // Convert this number of bytes there
		str.GetBuffer( nLength ), // Get direct access to the memory buffer for the CString object, telling it to be able to hold nLength characters
		nLength );                // Size of the buffer in wide characters

	// Release our direct manipulation of the CString's buffer
	str.ReleaseBuffer( nLength ); // Tell it how many wide characters we wrote there, null terminator not included

	// Return the string
	return str;
}

///////////////////////////////////////////////////////////////////////////////
// CBuffer read helper

BOOL CBuffer::Read(void* pData, const size_t nLength) throw()
{
	if ( nLength > m_nLength )
		return FALSE;

	CopyMemory( pData, m_pBuffer, nLength );
	Remove( nLength );

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// CBuffer read line helper

// Takes access to a string, default peek false to move a line from the buffer to the string, and default CP_ACP to read ASCII text
// Looks for bytes like "line\r\n" in the buffer, and moves them from the buffer to the string, throwing away the "\r\n" part
// Returns true if a line was found and moved from the buffer to the string, false if there isn't a '\n' in the buffer right now
BOOL CBuffer::ReadLine(CString& strLine, BOOL bPeek)
{
	// Empty the string, making it blank
	strLine.Empty();

	// If this buffer is empty, tell the caller we didn't find a complete line
	if ( ! m_nLength ) return FALSE;

	// Scan down each byte in the buffer
	DWORD nLength = 0;
	for ( ; nLength < m_nLength ; nLength++ )
	{
		// If the byte at this length is the newline character '\n', exit the loop
		if ( m_pBuffer[ nLength ] == '\n' ) break;
	}

	// If the loop didn't find a '\n' and instead stopped because nLength grew to equal m_nLength
	if ( nLength >= m_nLength ) return FALSE; // There isn't an '\n' in the buffer, tell the caller we didn't find a complete line

	strLine = UTF8Decode( (LPCSTR)m_pBuffer, nLength );

	strLine.TrimRight( _T("\r") );

	// Now that the line has been copied into the string, remove it and the '\n' from the buffer
	if ( ! bPeek )
	{
		Remove( nLength + 1 ); // Unless we're peeking, then leave it in the buffer
	}

	// Report that we found a line and moved it from the buffer to the string
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// CBuffer starts with helper

// Takes a pointer to ASCII text, and the option to remove these characters from the start of the buffer if they are found there
// Looks at the bytes at the start of the buffer, and determines if they are the same as the given ASCII text
// Returns true if the text matches, false if it doesn't
BOOL CBuffer::StartsWith(LPCSTR pszString, const size_t nLength, const BOOL bRemove) throw()
{
	// If the buffer isn't long enough to contain the given string, report the buffer doesn't start with it
	if ( m_nLength < nLength ) return FALSE;

	// If the first characters in the buffer don't match those in the ASCII string, return false
	if ( strncmp( (LPCSTR)m_pBuffer, (LPCSTR)pszString, nLength ) != 0 )
		return FALSE;

	// If we got the option to remove the string if it matched, do it
	if ( bRemove ) Remove( nLength );

	// Report that the buffer does start with the given ASCII text
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CBuffer deflate and inflate compression

#ifdef ZLIB_H

// Takes an option to avoid compressing a small buffer and to make sure compressing didn't actually make it bigger
// Compresses the data in this buffer in place
// Returns true if the data is compressed, false if there was an error
BOOL CBuffer::Deflate(BOOL bIfSmaller)
{
	// If the caller requested we check for small buffers, and this one contains less than 45 bytes, return false
	if ( bIfSmaller && m_nLength < 45 )
		return FALSE;

	DWORD nCompress = 0;
	BYTE* pCompress = CZLib::Compress2( m_pBuffer, m_nLength, &nCompress );
	if ( ! pCompress )
		return FALSE;

	// If compressing the data actually made it bigger, and we were told to watch for this happening
	if ( bIfSmaller && nCompress >= m_nLength )
	{
		free( pCompress );
		return FALSE;
	}

	if ( m_pBuffer ) free( m_pBuffer );
	m_pBuffer = pCompress;
	m_nBuffer = m_nLength = nCompress;

	return TRUE;
}

// Decompress the data in this buffer in place
// Returns true if the data is decompressed, false if there was an error
//
// Side Effect: This function assumes that all of the data in the buffer needs
// be decompressed, existing contents will be replaced by the decompression.
BOOL CBuffer::Inflate()
{
	DWORD nCompress = 0;
	BYTE* pCompress = CZLib::Decompress2( m_pBuffer, m_nLength, &nCompress );
	if ( ! pCompress )
		return FALSE;

	if ( m_pBuffer ) free( m_pBuffer );
	m_pBuffer = pCompress;
	m_nBuffer = m_nLength = nCompress;

	return TRUE;
}

// If the contents of this buffer are between headers and compressed with gzip, this method can remove all that
// Returns false on error
BOOL CBuffer::Ungzip()
{
	// Make sure there are at least 10 bytes in this buffer
	if ( m_nLength < 10 ) return FALSE;

	// Make sure the first 3 bytes are not 1f8b08
	if ( m_pBuffer[0] != 0x1F || m_pBuffer[1] != 0x8B || m_pBuffer[2] != 8 ) return FALSE;

	// At a distance of 3 bytes into the buffer, read the byte there and call it nFlags
	BYTE nFlags = m_pBuffer[3];

	// Remove the first 10 bytes of the buffer
	Remove( 10 );

	// If there is a 1 in position 0000 0100 in the flags byte
	if ( nFlags & 0x04 )
	{
		// Make sure the buffer has 2 or more bytes
		if ( m_nLength < 2 ) return FALSE;

		// Look at the first 2 bytes in the buffer as a word, this says how long the data it beyond it
		WORD nLen = *reinterpret_cast< WORD* >( m_pBuffer );

		// If the buffer has less data than it should, return false
		if ( m_nLength - 2 < nLen ) return FALSE;

		// Remove the length word and the length it describes from the front of the buffer
		Remove( nLen + 2 );
	}

	// If there is a 1 in position 0000 1000 in the flags byte
	if ( nFlags & 0x08 )
	{
		DWORD i = 0;
		for( ; m_pBuffer[i] && i < m_nLength; ++i );
		if ( i == m_nLength ) return FALSE;
		Remove( i + 1 );
	}

	// If there is a 1 in position 0001 0000 in the flags byte
	if ( nFlags & 0x10 )
	{
		DWORD i = 0;
		for( ; m_pBuffer[i] && i < m_nLength; ++i );
		if ( i == m_nLength ) return FALSE;
		Remove( i + 1 );
	}

	// If there is a 1 in position 0000 0010 in the flags byte
	if ( nFlags & 0x02 )
	{
		// Make sure the buffer has at least 2 bytes, and then remove them
		if ( m_nLength < 2 ) return FALSE;
		Remove( 2 );
	}

	// After removing all that header information from the front, remove the last 8 bytes from the end
	if ( m_nLength <= 8 ) return FALSE; // Make sure the buffer has more than 8 bytes
	m_nLength -= 8;                     // Remove the last 8 bytes in the buffer

	// Make a new buffer for the output.
	// Guess that inflating the data won't make it more than 6 times as big
	CBuffer pOutput;
	DWORD nLength = m_nLength * 6;
	for (;;)
	{
		if ( ! pOutput.EnsureBuffer( nLength ) )
		{
			// Out of memory
			return FALSE;
		}

		// Setup a z_stream structure to perform a raw inflate
		CAutoPtr< z_stream > pStream ( new z_stream );
		ZeroMemory( pStream, sizeof( z_stream ) );
		if ( Z_OK != inflateInit2(	// Initialize a stream inflation with more options than just inflateInit
			pStream,				// Stream structure to initialize
			-MAX_WBITS ) )			// Window bits value of -15 to perform a raw inflate
		{
			// The Zlib function inflateInit2 returned something other than Z_OK, report error
			return FALSE;
		}

		// Tell the z_stream structure where to work
		pStream->next_in   = m_pBuffer;									// Decompress the memory here
		pStream->avail_in  = static_cast< uInt >( m_nLength );			// There is this much of it
		pStream->next_out  = pOutput.m_pBuffer;							// Write decompressed data here
		pStream->avail_out = static_cast< uInt >( pOutput.GetBufferSize() );	// Tell ZLib it has this much space, it makes this smaller to show how much space is left

		// Call ZLib inflate to decompress all the data, and see if it returns Z_STREAM_END
		int nRes = Inflate( pStream, Z_FINISH );

		// The inflate call returned Z_STREAM_END
		if ( Z_STREAM_END == nRes )
		{
			// Move the decompressed data from the output buffer into this one
			Clear();                   // Record there are no bytes stored here, doesn't change the allocated block size
			Add(pOutput.m_pBuffer,     // Add the memory at the start of the output buffer
				pOutput.GetBufferSize()      // The amount of space the buffer had when we gave it to Zlib
				- pStream->avail_out ); // Minus the amount it said it left, this is the number of bytes it wrote

			// Close ZLib and report success
			inflateEnd( pStream );
			return TRUE;
		}
		// Buffer too small
		else if ( Z_BUF_ERROR == nRes )
		{
			nLength *= 2;
			inflateEnd( pStream );
		}
		// The inflate call returned something else
		else
		{
			// Close ZLib and report error
			inflateEnd( pStream );
			return FALSE;
		}
	}
}

int CBuffer::Inflate(z_streamp pStream, int nFlush)
{
	__try
	{
		return inflate( pStream, nFlush );
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		return Z_STREAM_ERROR;
	}
}

int CBuffer::Deflate(z_streamp pStream, int nFlush)
{
	__try
	{
		return deflate( pStream, nFlush );
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		return Z_STREAM_ERROR;
	}
}

// Decompress the data in this buffer into another buffer
// Returns true if the data is decompressed, false if there was an error
//
// Side Effect: This function allocates a new z_stream structure that gets
// cleaned up when the stream is finished. Call InflateStreamCleanup() to close
// the stream and delete the z_stream structure before the stream has finished.
bool CBuffer::InflateStreamTo(CBuffer& oBuffer, z_streamp& pStream, BOOL* pbEndOfStream)
{
	// Report success if there was nothing to decompress
	if ( ! m_nLength )
		return true;

	// Check if a z_stream structure has been allocated
	if ( ! pStream )
	{
		// Create a new z_stream structure to store state information
		pStream = new z_stream;

		// Initialise it to zero
		ZeroMemory( pStream, sizeof(z_stream) );

		// Initialise ZLib
		if ( inflateInit( pStream ) != Z_OK )
		{
			delete pStream;	// delete the z_stream structure
			pStream = NULL;	// and null the pointer
			return false;	// Report failure
		}
	}

	// Tell Zlib how much data is available to try and decompress
	pStream->avail_in  = (uInt)min( (size_t)m_nLength, (size_t)UINT_MAX );

	// Record inflation state
	int nResult = Z_OK;

	// Run inflate until all compressed data is consumed or there is an error
	do
	{
		// Limit nLength to the free buffer space or the maximum chunk size
		size_t nLength = max( GetBufferFree(), 1024ul ); // Chunk size for ZLib compression/decompression

		// Make sure the receiving buffer is large enough to hold at least 1KB
		if ( ! oBuffer.EnsureBuffer( nLength ) )
			break;

		// Tell the z_stream structure where to work
		pStream->next_in   = m_pBuffer;				// Decompress the data from here
		pStream->next_out  = oBuffer.GetDataEnd();	// Write decompressed data here
		pStream->avail_out = (uInt)nLength;			// There is this much room to
													// decompress data to

		// Call ZLib inflate to decompress all the available data
		nResult = Inflate( pStream, Z_SYNC_FLUSH );

		switch ( nResult )
		{
		// Check for errors
		case Z_NEED_DICT:
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
			break;

		// Something was decompressed
		default:
			// Tell the buffer it has additional data in it
			oBuffer.m_nLength += pStream->total_out;

			// Remove the consumed uncompressed data from the buffer
			Remove( pStream->total_in );

			// Adjust the state information
			pStream->total_out = 0ul;
			pStream->total_in = 0ul;
		}
	} while ( pStream->avail_out == 0u );	// Check if everything available
											// was decompressed

	// If there was not enough data to decompress anything inflate() returns
	// Z_BUF_ERROR, ignore this error and try again next time.
	if ( nResult == Z_BUF_ERROR )
		nResult = Z_OK;

	// Check if the stream needs to be closed
	if ( nResult != Z_OK )
		InflateStreamCleanup( pStream );

	if ( pbEndOfStream )
		*pbEndOfStream = ( nResult == Z_STREAM_END );

	return ( nResult == Z_OK || nResult == Z_STREAM_END );
}

void CBuffer::InflateStreamCleanup(z_streamp& pStream)
{
	if ( pStream )
	{
		__try
		{
			inflateEnd( pStream );	// Close the stream
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
		{
		}
		delete pStream;
		pStream = NULL;
	}
}

void CBuffer::DeflateStreamCleanup(z_streamp& pStream)
{
	if ( pStream )
	{
		__try
		{
			deflateEnd( pStream );	// Close the stream
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
		{
		}
		delete pStream;
		pStream = NULL;
	}
}

#endif // ZLIB_H

#ifdef _BZLIB_H

BOOL CBuffer::BZip()
{
	// Compress to temporary buffer first
	CBuffer pOutBuf;
	UINT nOutSize = m_nLength / 2;
	for (;;)
	{
		if ( ! pOutBuf.EnsureBuffer( nOutSize ) )
			// Out of memory
			return FALSE;

		int err = BZ2_bzBuffToBuffCompress( (char*)pOutBuf.m_pBuffer, &nOutSize,
			(char*)m_pBuffer, m_nLength, 9, 0, 30 );

		if ( err == BZ_OK )
		{
			pOutBuf.m_nLength = nOutSize;
			break;
		}
		else if ( err == BZ_OUTBUFF_FULL )
			// Insufficient output buffer 
			nOutSize *= 2;
		else
			// Compression error
			return FALSE;
	}

	Attach( &pOutBuf );

	return TRUE;
}

BOOL CBuffer::UnBZip()
{
	// Uncompress to temporary buffer first
	CBuffer pOutBuf;
	UINT nOutSize = m_nLength * 3;
	for (;;)
	{
		if ( ! pOutBuf.EnsureBuffer( nOutSize ) )
			// Out of memory
			return FALSE;

		int err = BZ2_bzBuffToBuffDecompress( (char*)pOutBuf.m_pBuffer, &nOutSize,
			(char*)m_pBuffer, m_nLength, 0, 0 );

		if ( err == BZ_OK )
		{
			pOutBuf.m_nLength = nOutSize;
			break;
		}
		else if ( err == BZ_OUTBUFF_FULL )
			// Insufficient output buffer 
			nOutSize *= 2;
		else
			// Decompression error
			return FALSE;
	}

	Attach( &pOutBuf );

	return TRUE;
}

#endif // _BZLIB_H

//////////////////////////////////////////////////////////////////////
// CBuffer reverse buffer

// This method is static, which means you can call it like CBuffer::ReverseBuffer() without having a CBuffer object at all
// Takes pointers to input memory and an output buffer, and a length, which is both the memory in input and the space in output
// Copies the bytes from input to output, but in reverse order
void CBuffer::ReverseBuffer(const void* pInput, void* pOutput, size_t nLength)
{
	// Point pInputWords at the end of the input memory block
	const DWORD* pInputWords = (const DWORD*)( (const BYTE*)pInput + nLength ); // This is a DWORD pointer, so it will move in steps of 4

	// Point pOutputWords at the start of the output buffer
	DWORD* pOutputWords      = (DWORD*)( pOutput );

	// Make a new local DWORD called nTemp, and request that Visual Studio place it in a machine register
	register DWORD nTemp; // The register keyword asks that nTemp be a machine register, making it really fast

	// Loop while nLength is bigger than 4, grabbing bytes 4 at a time and reversing them
	while ( nLength > 4 )
	{
		// Move pInputWords back 4 bytes, then copy the 4 bytes there into nTemp, the fast machine register DWORD
		nTemp = *--pInputWords;

		// Reverse the order of the 4 bytes, copy them under pOutputWords, and then move that pointer 4 bytes forward
		*pOutputWords++ = swapEndianess( nTemp );

		// We've just reverse 4 bytes, subtract the length to reflect this
		nLength -= 4;
	}

	// If there are still some input bytes to add reversed
	if ( nLength )
	{
		// Point pInputBytes and pOutputBytes at the same places
		const BYTE* pInputBytes	= (const BYTE*)pInputWords; // This is a byte pointer, so it will move in steps of 1
		BYTE* pOutputBytes		= (BYTE*)pOutputWords;

		// Loop until there are no more bytes to copy over
		while ( nLength-- )
		{
			// Move pInputBytes back to grab a byte, copy it under pOutputBytes, then move pOutputBytes forward
			*pOutputBytes++ = *--pInputBytes;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CBuffer DIME handling

// DIME is a specification for sending and receiving SOAP messages along with additional attachments, like binary files or XML fragments
// Takes information to create a DIME message
// Composes the DIME message and writes it into this buffer
void CBuffer::WriteDIME(
	DWORD nFlags,   // 0, 1, or 2
	LPCSTR pszID,   // Blank, or a GUID in hexadecimal encoding
	size_t nIDLength,
	LPCSTR pszType, // "text/xml" or a URI to an XML specification
	size_t nTypeLength,
	LPCVOID pBody,  // The XML fragment we're wrapping
	size_t nBody)    // How long it is
{
	// Format lengths into the bytes of the DIME header
	if ( ! EnsureBuffer( 12 ) ) return;

	BYTE* pOut = m_pBuffer + m_nLength;                               // Point pOut at the end of the memory block in this buffer
	*pOut++ = 0x08 | ( nFlags & 1 ? 4 : 0 ) | ( nFlags & 2 ? 2 : 0 ); // *pOut++ = 0x08 sets the byte at pOut and then moves the pointer forward
	*pOut++ = strchr( pszType, ':' ) ? 0x20 : 0x10;
	*pOut++ = 0x00;
	*pOut++ = 0x00;
	*pOut++ = BYTE( ( nIDLength & 0xFF00 ) >> 8 );
	*pOut++ = BYTE( nIDLength & 0xFF );
	*pOut++ = BYTE( ( nTypeLength & 0xFF00 ) >> 8 );
	*pOut++ = BYTE( nTypeLength & 0xFF );
	*pOut++ = (BYTE)( ( nBody & 0xFF000000 ) >> 24 );
	*pOut++ = (BYTE)( ( nBody & 0x00FF0000 ) >> 16 );
	*pOut++ = (BYTE)( ( nBody & 0x0000FF00 ) >> 8 );
	*pOut++ = (BYTE)( nBody & 0x000000FF );
	m_nLength += 12;                                                  // Record that we wrote 12 bytes

	// Print pszID, which is blank or a GUID in hexadecimal encoding, and bytes of 0 until the total length we added is a multiple of 4
	Print( pszID, nIDLength );
	for ( size_t nPad = nIDLength ; nPad & 3 ; nPad++ ) Add( "", 1 ); // If we added "a", add "000" to get to the next group of 4

	// Print pszType, which is "text/xml" or a URI to an XML specification, and bytes of 0 until the total length we added is a multiple of 4
	Print( pszType, nTypeLength );
	for ( size_t nPad = nTypeLength ; nPad & 3 ; nPad++ ) Add( "", 1 ); // If we added "abcdef", add "00" to get to the next group of 4

	// If there is body text
	if ( pBody != NULL )
	{
		// Add it, followed by bytes of 0 until the total length we added is a multiple of 4
		Add( pBody, nBody );
		for ( size_t nPad = nBody ; nPad & 3 ; nPad++ ) Add( "", 1 );
	}
}

// DIME is a specification for sending and receiving SOAP messages along with additional attachments, like binary files or XML fragments
// If there is a DIME message sitting in this buffer, this method can read it
// Takes DWORD and CString pointers to fill with information from the DIME message
// Returns false if the DIME message wasn't formatted correctly
BOOL CBuffer::ReadDIME(
	DWORD* pnFlags,  // Writes the flags byte from the DIME message
	CString* psID,   // Writes a GUID in hexadecimal encoding from the DIME message
	CString* psType, // Writes "text/xml" or a URI to an XML specification
	DWORD* pnBody)   // Writes how long the body of the DIME message is
{
	// Make sure the buffer has at least 12 bytes
	if ( m_nLength < 12 ) return FALSE;

	// Point pIn at the start of this buffer
	BYTE* pIn = m_pBuffer;

	// The first 5 bits of the first byte, 00000---, must not be 00001---
	if ( ( *pIn & 0xF8 ) != 0x08 ) return FALSE;

	// If this method was passed a pnFlags DWORD
	if ( pnFlags != NULL )
	{
		// Write it for the caller
		*pnFlags = 0;                  // Start it out as 0
		if ( *pIn & 4 ) *pnFlags |= 1; // If the first byte in the buffer has a bit here -----1--, put one here -------1 in pnFlags
		if ( *pIn & 2 ) *pnFlags |= 2; // If the first byte in the buffer has a bit here ------1-, put one here ------1- in pnFlags
	}

	// Move the pIn pointer to the second byte in the buffer, and make sure it's not 00001010 or 00010100
	pIn++;
	if ( *pIn != 0x10 && *pIn != 0x20 ) return FALSE;

	// Make sure bytes 3 and 4 in the buffer aren't 0, and move pIn a distance of 4 bytes into the buffer, pointing at the 5th byte
	pIn++;
	if ( *pIn++ != 0x00 ) return FALSE;
	if ( *pIn++ != 0x00 ) return FALSE;

	// Read nID, nType, and pnBody from the buffer, and move the pointer forward 8 bytes
	WORD nID   = ( pIn[0] << 8 ) + pIn[1]; pIn += 2;
	WORD nType = ( pIn[0] << 8 ) + pIn[1]; pIn += 2;
	*pnBody    = ( pIn[0] << 24 ) + ( pIn[1] << 16 ) + ( pIn[2] << 8 ) + pIn[3]; // Write the body length in the DWORD from the caller
	pIn += 4; // Move forward another 4 bytes to total 8 bytes for this section

	// Skip forward a distance determined by the lengths we just read
	DWORD nSkip = 12 + ( ( nID + 3 ) & ~3 ) + ( ( nType + 3 ) & ~3 );
	if ( m_nLength < nSkip + ( ( *pnBody + 3 ) & ~3 ) ) return FALSE; // Make sure the buffer is big enough to skip this far forward

	// Read psID, a GUID in hexadecimal encoding
	*psID = CString( reinterpret_cast< char* >( pIn ), nID );
	pIn += ( nID + 3 ) & ~3;           // Move pIn forward beyond the psID text and align at 4 bytes

	// Read psType, a GUID in hexadecimal encoding
	*psType = CString( reinterpret_cast< char* >( pIn ), nType );
	pIn += ( nType + 3 ) & ~3;           // Move pIn forward beyond the pszType text and align at 4 bytes

	// Remove the first part of the DIME message from the buffer, and report success
	Remove( nSkip );
	return TRUE;
}
