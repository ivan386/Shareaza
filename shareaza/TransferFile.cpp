//
// TransferFile.cpp
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
#include "TransferFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CTransferFiles TransferFiles;


//////////////////////////////////////////////////////////////////////
// CTransferFiles construction

CTransferFiles::CTransferFiles()
{
}

CTransferFiles::~CTransferFiles()
{
	Close();
}

//////////////////////////////////////////////////////////////////////
// CTransferFiles open a file

CTransferFile* CTransferFiles::Open(LPCTSTR pszFile, BOOL bWrite, BOOL bCreate)
{
	CSingleLock pLock( &m_pSection, TRUE );
	CTransferFile* pFile = NULL;
	
	if ( m_pMap.Lookup( pszFile, (void*&)pFile ) )
	{
		if ( bWrite && ! pFile->EnsureWrite() ) return NULL;
	}
	else
	{
		pFile = new CTransferFile( pszFile );
		
		if ( ! pFile->Open( bWrite, bCreate ) )
		{
			delete pFile;
			return NULL;
		}
		
		m_pMap.SetAt( pFile->m_sPath, pFile );
	}
	
	pFile->AddRef();
	
	return pFile;
}

//////////////////////////////////////////////////////////////////////
// CTransferFiles close all files

void CTransferFiles::Close()
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	for ( POSITION pos = m_pMap.GetStartPosition() ; pos ; )
	{
		CTransferFile* pFile;
		CString strPath;
		
		m_pMap.GetNextAssoc( pos, strPath, (void*&)pFile );
		delete pFile;
	}
	
	m_pMap.RemoveAll();
	m_pDeferred.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// CTransferFiles commit deferred writes

void CTransferFiles::CommitDeferred()
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	for ( POSITION pos = m_pDeferred.GetHeadPosition() ; pos ; )
	{
		CTransferFile* pFile = (CTransferFile*)m_pDeferred.GetNext( pos );
		pFile->DeferredWrite( TRUE );
	}
	
	m_pDeferred.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// CTransferFiles queue for deferred write

void CTransferFiles::QueueDeferred(CTransferFile* pFile)
{
	if ( NULL == m_pDeferred.Find( pFile ) ) m_pDeferred.AddTail( pFile );
}

//////////////////////////////////////////////////////////////////////
// CTransferFiles remove a single file

void CTransferFiles::Remove(CTransferFile* pFile)
{
	m_pMap.RemoveKey( pFile->m_sPath );
	if ( POSITION pos = m_pDeferred.Find( pFile ) ) m_pDeferred.RemoveAt( pos );
}


//////////////////////////////////////////////////////////////////////
// CTransferFile construction

CTransferFile::CTransferFile(LPCTSTR pszPath)
{
	m_sPath				= pszPath;
	m_hFile				= INVALID_HANDLE_VALUE;
	m_nReference		= 0;
	m_bWrite			= FALSE;
	m_nDeferred			= 0;
}

CTransferFile::~CTransferFile()
{
	if ( m_hFile != INVALID_HANDLE_VALUE )
	{
		DeferredWrite();
		CloseHandle( m_hFile );
	}
}

//////////////////////////////////////////////////////////////////////
// CTransferFile reference counts

void CTransferFile::AddRef()
{
	CSingleLock pLock( &TransferFiles.m_pSection, TRUE );
	m_nReference++;
}

void CTransferFile::Release(BOOL bWrite)
{
	CSingleLock pLock( &TransferFiles.m_pSection, TRUE );
	
	if ( ! --m_nReference )
	{
		TransferFiles.Remove( this );
		delete this;
		return;
	}
	
	if ( m_bWrite && bWrite ) CloseWrite();
}

//////////////////////////////////////////////////////////////////////
// CTransferFile handle

HANDLE CTransferFile::GetHandle(BOOL bWrite)
{
	CSingleLock pLock( &TransferFiles.m_pSection, TRUE );
	
	if ( bWrite && ! m_bWrite ) return INVALID_HANDLE_VALUE;
	if ( m_nDeferred > 0 ) DeferredWrite();
	
	return m_hFile;
}

BOOL CTransferFile::IsOpen()
{
	return m_hFile != INVALID_HANDLE_VALUE;
}

//////////////////////////////////////////////////////////////////////
// CTransferFile open

BOOL CTransferFile::Open(BOOL bWrite, BOOL bCreate)
{
	if ( m_hFile != INVALID_HANDLE_VALUE ) return FALSE;
	
	DWORD dwDesiredAccess = GENERIC_READ;
	if ( bWrite ) dwDesiredAccess |= GENERIC_WRITE;
	
	DWORD dwShare = FILE_SHARE_READ|FILE_SHARE_WRITE;
	DWORD dwCreation = bCreate ? CREATE_ALWAYS : OPEN_EXISTING;
	
#if 1
	m_hFile = CreateFile( m_sPath, dwDesiredAccess, dwShare,
		NULL, dwCreation, FILE_ATTRIBUTE_NORMAL, NULL );
#else
	// Testing
	m_hFile = CreateFile( _T("C:\\Junk\\Incomplete.bin"), dwDesiredAccess,
		dwShare, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
#endif
	
	if ( m_hFile != INVALID_HANDLE_VALUE ) m_bWrite = bWrite;
	
	return m_hFile != INVALID_HANDLE_VALUE;
}

//////////////////////////////////////////////////////////////////////
// CTransferFile write access management

BOOL CTransferFile::EnsureWrite()
{
	if ( m_hFile == INVALID_HANDLE_VALUE ) return FALSE;
	if ( m_bWrite ) return TRUE;

	CloseHandle( m_hFile );
	m_hFile = INVALID_HANDLE_VALUE;

	if ( Open( TRUE, FALSE ) ) return TRUE;

	Open( FALSE, FALSE );

	return FALSE;
}

BOOL CTransferFile::CloseWrite()
{
	if ( m_hFile == INVALID_HANDLE_VALUE ) return FALSE;
	if ( ! m_bWrite ) return TRUE;
	
	DeferredWrite();
	
	CloseHandle( m_hFile );
	m_hFile = INVALID_HANDLE_VALUE;
	
	return Open( FALSE, FALSE );
}

//////////////////////////////////////////////////////////////////////
// CTransferFile read

BOOL CTransferFile::Read(QWORD nOffset, LPVOID pBuffer, QWORD nBuffer, QWORD* pnRead)
{
	CSingleLock pLock( &TransferFiles.m_pSection, TRUE );
	
	*pnRead = 0;
	if ( m_hFile == INVALID_HANDLE_VALUE ) return FALSE;
	if ( m_nDeferred > 0 ) DeferredWrite();
	
	DWORD nOffsetLow	= (DWORD)( nOffset & 0x00000000FFFFFFFF );
	DWORD nOffsetHigh	= (DWORD)( ( nOffset & 0xFFFFFFFF00000000 ) >> 32 );
	SetFilePointer( m_hFile, nOffsetLow, (PLONG)&nOffsetHigh, FILE_BEGIN );
	
	return ReadFile( m_hFile, pBuffer, (DWORD)nBuffer, (DWORD*)pnRead, NULL );
}

//////////////////////////////////////////////////////////////////////
// CTransferFile write (with deferred extension)

#define DEFERRED_THRESHOLD		(20*1024*1024)

BOOL CTransferFile::Write(QWORD nOffset, LPCVOID pBuffer, QWORD nBuffer, QWORD* pnWritten)
{
	CSingleLock pLock( &TransferFiles.m_pSection, TRUE );
	
	*pnWritten = 0;
	if ( m_hFile == INVALID_HANDLE_VALUE ) return FALSE;
	if ( ! m_bWrite ) return FALSE;
	
	if ( nOffset > DEFERRED_THRESHOLD )
	{
		DWORD nSizeHigh = 0;
		QWORD nSize = (QWORD)GetFileSize( m_hFile, &nSizeHigh );
		nSize |= ( (QWORD)nSizeHigh << 32 );
		
		if ( nOffset > nSize && nOffset - nSize > DEFERRED_THRESHOLD )
		{
			TransferFiles.QueueDeferred( this );
			
			if ( m_nDeferred >= DEFERRED_MAX ) DeferredWrite();
			
			DefWrite* pWrite = &m_pDeferred[ m_nDeferred++ ];
			
			pWrite->m_nOffset	= nOffset;
			pWrite->m_nLength	= (DWORD)nBuffer;
			pWrite->m_pBuffer	= new BYTE[ (DWORD)nBuffer ];
			CopyMemory( pWrite->m_pBuffer, pBuffer, (DWORD)nBuffer );
			*pnWritten = nBuffer;
			
			theApp.Message( MSG_TEMP, _T("Deferred write of %I64i bytes at %I64i"), nBuffer, nOffset );
			
			return TRUE;
		}
	}
	
	DWORD nOffsetLow	= (DWORD)( nOffset & 0x00000000FFFFFFFF );
	DWORD nOffsetHigh	= (DWORD)( ( nOffset & 0xFFFFFFFF00000000 ) >> 32 );
	SetFilePointer( m_hFile, nOffsetLow, (PLONG)&nOffsetHigh, FILE_BEGIN );
	
	return WriteFile( m_hFile, pBuffer, (DWORD)nBuffer, (LPDWORD)pnWritten, NULL );
}

//////////////////////////////////////////////////////////////////////
// CTransferFile deferred writes

void CTransferFile::DeferredWrite(BOOL bOffline)
{
	if ( m_nDeferred == 0 ) return;
	if ( m_hFile == INVALID_HANDLE_VALUE ) return;
	if ( ! m_bWrite ) return;
	
	DefWrite* pWrite = m_pDeferred;
	
	for ( int nDeferred = 0 ; nDeferred < m_nDeferred ; nDeferred++, pWrite++ )
	{
		theApp.Message( MSG_TEMP, _T("Committing deferred write of %lu bytes at %I64i"),
			pWrite->m_nLength, pWrite->m_nOffset );
		
		DWORD nOffsetLow	= (DWORD)( pWrite->m_nOffset & 0x00000000FFFFFFFF );
		DWORD nOffsetHigh	= (DWORD)( ( pWrite->m_nOffset & 0xFFFFFFFF00000000 ) >> 32 );
		SetFilePointer( m_hFile, nOffsetLow, (PLONG)&nOffsetHigh, FILE_BEGIN );
		
		DWORD nWritten = 0;
		WriteFile( m_hFile, pWrite->m_pBuffer, pWrite->m_nLength, &nWritten, NULL );
		
		delete [] pWrite->m_pBuffer;
	}
	
	m_nDeferred = 0;
	theApp.Message( MSG_TEMP, _T("Commit finished") );
}
