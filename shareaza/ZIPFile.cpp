//
// ZIPFile.cpp
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "ZIPFile.h"
#include "Buffer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CZIPFile construction

CZIPFile::CZIPFile(HANDLE hAttach)
{
	m_bAttach	= FALSE;
	m_hFile		= INVALID_HANDLE_VALUE;
	m_pFile		= NULL;
	m_nFile		= 0;

	if ( hAttach != INVALID_HANDLE_VALUE ) Attach( hAttach );
}

CZIPFile::~CZIPFile()
{
	Close();
}

/////////////////////////////////////////////////////////////////////////////
// CZIPFile open

BOOL CZIPFile::Open(LPCTSTR pszFile)
{
	ASSERT( pszFile != NULL );

	Close();

	m_bAttach = FALSE;
	m_hFile = CreateFile( pszFile, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( m_hFile == INVALID_HANDLE_VALUE ) return FALSE;

	if ( LocateCentralDirectory() )
	{
		return TRUE;
	}
	else
	{
		Close();
		return FALSE;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CZIPFile attach

BOOL CZIPFile::Attach(HANDLE hFile)
{
	ASSERT( hFile != INVALID_HANDLE_VALUE );

	Close();

	m_bAttach	= TRUE;
	m_hFile		= hFile;

	if ( LocateCentralDirectory() )
	{
		return TRUE;
	}
	else
	{
		Close();
		return FALSE;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CZIPFile open test

BOOL CZIPFile::IsOpen() const
{
	return m_hFile != INVALID_HANDLE_VALUE;
}

/////////////////////////////////////////////////////////////////////////////
// CZIPFile close

void CZIPFile::Close()
{
	if ( m_hFile != INVALID_HANDLE_VALUE )
	{
		if ( ! m_bAttach ) CloseHandle( m_hFile );
		m_hFile = INVALID_HANDLE_VALUE;
	}

	if ( m_pFile != NULL ) delete [] m_pFile;
	m_pFile = NULL;
	m_nFile = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CZIPFile get the file count

int CZIPFile::GetCount() const
{
	return m_nFile;
}

/////////////////////////////////////////////////////////////////////////////
// CZIPFile get a particular file

CZIPFile::File* CZIPFile::GetFile(int nFile) const
{
	return ( nFile < 0 || nFile >= m_nFile ) ? NULL : m_pFile + nFile;
}

/////////////////////////////////////////////////////////////////////////////
// CZIPFile lookup a file by name

CZIPFile::File* CZIPFile::GetFile(LPCTSTR pszFile, BOOL bSmartSearch) const
{
	File* pFile = m_pFile;
	bool bFound = false;

	for ( int nFile = m_nFile ; nFile ; nFile--, pFile++ )
	{
		if ( _tcsicoll( pFile->m_sName, pszFile ) == 0 ) 
		{
			bFound = true;
			break;
		}
	}

	if ( bFound )
		return pFile;
	else if ( !bSmartSearch )
		return NULL;
	else
		pFile = m_pFile;

	for ( int nFile = m_nFile ; nFile ; nFile--, pFile++ )
	{
		LPCTSTR pszName = _tcsrchr( pFile->m_sName, '/' );
		pszName = pszName ? pszName + 1 : (LPCTSTR)pFile->m_sName;
		if ( _tcsicoll( pszName, pszFile ) == 0 ) return pFile;
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CZIPFile locate the central directory

#pragma pack(1)
typedef struct
{
	DWORD	nSignature;			// 0x06054b50
	WORD	nThisDisk;
	WORD	nDirectoryDisk;
	WORD	nFilesThisDisk;
	WORD	nTotalFiles;
	DWORD	nDirectorySize;
	DWORD	nDirectoryOffset;
	WORD	nCommentLen;
} ZIP_DIRECTORY_LOC;
#pragma pack()

BOOL CZIPFile::LocateCentralDirectory()
{
	auto_array< BYTE > pBuffer( new BYTE[ 4096 ] );
	if ( ! pBuffer.get() )
		return FALSE;

	DWORD nBuffer = 0;

	SetFilePointer( m_hFile, -4096, NULL, FILE_END );
	if ( ! ReadFile( m_hFile, pBuffer.get(), 4096, &nBuffer, NULL ) )
		return FALSE;
	if ( nBuffer < sizeof(ZIP_DIRECTORY_LOC) )
		return FALSE;

	ZIP_DIRECTORY_LOC* pLoc = NULL;

	for ( DWORD nScan = 4 ; nScan < nBuffer ; nScan++ )
	{
		DWORD* pnSignature = (DWORD*)( pBuffer.get() + nBuffer - nScan  );

		if ( *pnSignature == 0x06054b50 )
		{
			pLoc = (ZIP_DIRECTORY_LOC*)pnSignature;
			break;
		}
	}

	if ( pLoc == NULL )
		return FALSE;
	ASSERT( pLoc->nSignature == 0x06054b50 );

	if ( GetFileSize( m_hFile, NULL ) < pLoc->nDirectorySize )
		return FALSE;

	if ( SetFilePointer( m_hFile, pLoc->nDirectoryOffset, NULL, FILE_BEGIN )
		 != pLoc->nDirectoryOffset )
		 return FALSE;

	auto_array< BYTE > pDirectory( new BYTE[ pLoc->nDirectorySize ] );
	if ( ! pDirectory.get() )
		return FALSE;

	nBuffer = 0;
	if ( ! ReadFile( m_hFile, pDirectory.get(), pLoc->nDirectorySize, &nBuffer, NULL ) )
		return FALSE;

	if ( nBuffer == pLoc->nDirectorySize )
	{
		m_nFile = (int)pLoc->nTotalFiles;
		m_pFile = new File[ m_nFile ];

		if ( ! ParseCentralDirectory( pDirectory.get(), pLoc->nDirectorySize ) )
		{
			delete [] m_pFile;
			m_pFile = NULL;
			m_nFile = 0;
		}
	}

	return ( m_nFile > 0 );
}

/////////////////////////////////////////////////////////////////////////////
// CZIPFile parse the central directory

#pragma pack(1)
typedef struct
{
	DWORD	nSignature;		// 0x02014b50
	WORD	nWriteVersion;
	WORD	nReadVersion;
	WORD	nFlags;
	WORD	nCompression;
	WORD	nFileTime;
	WORD	nFileDate;
	DWORD	nCRC;
	DWORD	nCompressedSize;
	DWORD	nActualSize;
	WORD	nNameLen;
	WORD	nExtraLen;
	WORD	nCommentLen;
	WORD	nStartDisk;
	WORD	nInternalAttr;
	DWORD	nExternalAttr;
	DWORD	nLocalOffset;
} ZIP_CENTRAL_FILE;
#pragma pack()

BOOL CZIPFile::ParseCentralDirectory(BYTE* pDirectory, DWORD nDirectory)
{
	for ( int nFile = 0 ; nFile < m_nFile ; nFile++ )
	{
		ZIP_CENTRAL_FILE* pRecord = (ZIP_CENTRAL_FILE*)pDirectory;

		if ( nDirectory < sizeof(*pRecord) ) return FALSE;
		if ( pRecord->nSignature != 0x02014b50 ) return FALSE;

		pDirectory += sizeof(*pRecord);
		nDirectory -= sizeof(*pRecord);

		int nTailLen = (int)pRecord->nNameLen + (int)pRecord->nExtraLen + (int)pRecord->nCommentLen;
		if ( nDirectory < (DWORD)nTailLen ) return FALSE;

		m_pFile[ nFile ].m_pZIP				= this;
		m_pFile[ nFile ].m_nSize			= pRecord->nActualSize;
		m_pFile[ nFile ].m_nLocalOffset		= pRecord->nLocalOffset;
		m_pFile[ nFile ].m_nCompressedSize	= pRecord->nCompressedSize;
		m_pFile[ nFile ].m_nCompression		= pRecord->nCompression;

		LPTSTR pszName = m_pFile[ nFile ].m_sName.GetBuffer( pRecord->nNameLen );

		for ( WORD nChar = 0 ; nChar < pRecord->nNameLen ; nChar++ )
		{
			pszName[ nChar ] = (TCHAR)pDirectory[ nChar ];
			if ( pszName[ nChar ] == '\\' ) pszName[ nChar ] = '/';
		}

		m_pFile[ nFile ].m_sName.ReleaseBuffer( pRecord->nNameLen );

		pDirectory += (DWORD)nTailLen;
		nDirectory -= (DWORD)nTailLen;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CZIPFile::File seek to a file

#pragma pack(1)
typedef struct
{
	DWORD	nSignature;		// 0x04034b50
	WORD	nVersion;
	WORD	nFlags;
	WORD	nCompression;
	WORD	nFileTime;
	WORD	nFileDate;
	DWORD	nCRC;
	DWORD	nCompressedSize;
	DWORD	nActualSize;
	WORD	nNameLen;
	WORD	nExtraLen;
} ZIP_LOCAL_FILE;
#pragma pack()

BOOL CZIPFile::SeekToFile(File* pFile)
{
	ASSERT( this != NULL );
	ASSERT( pFile != NULL );
	ASSERT( pFile->m_pZIP == this );

	if ( m_hFile == INVALID_HANDLE_VALUE ) return FALSE;

	if ( SetFilePointer( m_hFile, (DWORD)pFile->m_nLocalOffset, NULL, FILE_BEGIN )
		 != pFile->m_nLocalOffset )
		 return FALSE;

	ZIP_LOCAL_FILE pLocal;
	DWORD nRead = 0;

	if ( ! ReadFile( m_hFile, &pLocal, sizeof(pLocal), &nRead, NULL ) )
		return FALSE;
	if ( nRead != sizeof(pLocal) )
		return FALSE;

	if ( pLocal.nSignature != 0x04034b50 )
		return FALSE;
	if ( pLocal.nCompression != Z_DEFLATED && pLocal.nCompression != 0 )
		return FALSE;

	SetFilePointer( m_hFile, pLocal.nNameLen + pLocal.nExtraLen, NULL, FILE_CURRENT );

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CZIPFile::File prepare to decompress

BOOL CZIPFile::File::PrepareToDecompress(LPVOID pStream)
{
	ZeroMemory( pStream, sizeof(z_stream) );

	if ( ! m_pZIP->SeekToFile( this ) ) return FALSE;

	if ( m_nCompression == 0 )
	{
		return ( m_nSize == m_nCompressedSize );
	}
	else
	{
		ASSERT( m_nCompression == Z_DEFLATED );
		return Z_OK == inflateInit2( (z_stream*)pStream, -MAX_WBITS );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CZIPFile::File decompress to memory

CBuffer* CZIPFile::File::Decompress()
{
	if ( m_nSize > 32*1024*1024 )
		return NULL;

	CAutoPtr< z_stream > pStream( new z_stream );

	if ( ! PrepareToDecompress( pStream ) )
		return NULL;

	auto_ptr< CBuffer > pTarget( new CBuffer() );
	if ( ! pTarget.get() )
		return NULL;

	if ( m_nCompression == 0 )
	{
		pTarget->EnsureBuffer( (DWORD)m_nSize );

		if ( ! ReadFile( m_pZIP->m_hFile, pTarget->m_pBuffer, (DWORD)m_nSize, &pTarget->m_nLength, NULL ) )
			return NULL;
		if ( pTarget->m_nLength != (DWORD)m_nSize )
			return NULL;

		return pTarget.release();
	}

	DWORD nSource = (DWORD)m_nCompressedSize;
	auto_array< BYTE > pSource( new BYTE[ nSource ] );
	if ( ! pSource.get() )
		return NULL;
	if ( ! ReadFile( m_pZIP->m_hFile, pSource.get(), nSource, &nSource, NULL ) )
		return NULL;

	if ( nSource != (DWORD)m_nCompressedSize )
	{
		inflateEnd( pStream );
		return NULL;
	}

	pTarget->EnsureBuffer( (DWORD)m_nSize );
	pTarget->m_nLength = (DWORD)m_nSize;

	pStream->next_in	= pSource.get();
	pStream->avail_in	= (DWORD)m_nCompressedSize;
	pStream->next_out	= pTarget->m_pBuffer;
	pStream->avail_out	= pTarget->m_nLength;

	CBuffer::Inflate( pStream, Z_FINISH );

	if ( pStream->avail_out != 0 )
	{
		pTarget.reset();
	}

	inflateEnd( pStream );

	return pTarget.release();
}

/////////////////////////////////////////////////////////////////////////////
// CZIPFile::File decompress to disk

const DWORD BUFFER_IN_SIZE = 64 * 1024u;
const DWORD BUFFER_OUT_SIZE = 128 * 1024u;

BOOL CZIPFile::File::Extract(LPCTSTR pszFile)
{
	HANDLE hFile = CreateFile( pszFile, GENERIC_WRITE, 0, NULL, CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
		return FALSE;

	CAutoPtr< z_stream > pStream( new z_stream );

	if ( ! PrepareToDecompress( pStream ) )
		return FALSE;

	QWORD nCompressed = 0, nUncompressed = 0;

	if ( m_nCompression == Z_DEFLATED )
	{
		auto_array< BYTE > pBufferIn( new BYTE[ BUFFER_IN_SIZE ] );
		auto_array< BYTE > pBufferOut( new BYTE[ BUFFER_OUT_SIZE ] );

		while ( pBufferIn.get() && pBufferOut.get() &&
			( nCompressed < m_nCompressedSize || nUncompressed < m_nSize ) )
		{
			if ( pStream->avail_in == 0 )
			{
				pStream->avail_in	= (uInt)min( m_nCompressedSize - nCompressed, (QWORD)BUFFER_IN_SIZE );
				pStream->next_in	= pBufferIn.get();

				DWORD nRead = 0;
				if ( ! ReadFile( m_pZIP->m_hFile, pBufferIn.get(), pStream->avail_in, &nRead, NULL ) )
					break;
				if ( nRead != pStream->avail_in )
					break;
				nCompressed += nRead;
			}

			pStream->avail_out	= BUFFER_OUT_SIZE;
			pStream->next_out	= pBufferOut.get();

			CBuffer::Inflate( pStream, Z_SYNC_FLUSH );

			if ( pStream->avail_out < BUFFER_OUT_SIZE )
			{
				DWORD nWrite = BUFFER_OUT_SIZE - pStream->avail_out;
				if ( ! WriteFile( hFile, pBufferOut.get(), nWrite, &nWrite, NULL ) )
					break;
				if ( nWrite != BUFFER_OUT_SIZE - pStream->avail_out )
					break;
				nUncompressed += nWrite;
			}
		}

		inflateEnd( pStream );
	}
	else
	{
		auto_array< BYTE > pBufferOut( new BYTE[ BUFFER_OUT_SIZE ] );

		while ( pBufferOut.get() && nUncompressed < m_nSize )
		{
			DWORD nChunk = (DWORD)min( m_nSize - nUncompressed, (QWORD)BUFFER_OUT_SIZE );
			DWORD nProcess = 0;

			if ( ! ReadFile( m_pZIP->m_hFile, pBufferOut.get(), nChunk, &nProcess, NULL ) )
				break;
			if ( nChunk != nProcess )
				break;
			if ( ! WriteFile( hFile, pBufferOut.get(), nChunk, &nProcess, NULL ) )
				break;
			if ( nChunk != nProcess )
				break;

			nCompressed += nChunk;
			nUncompressed += nChunk;
		}
	}

	CloseHandle( hFile );

	if ( nUncompressed >= m_nSize )
		return TRUE;

	DeleteFileEx( pszFile, FALSE, FALSE, TRUE );

	return FALSE;
}

