//
// LibraryBuilder.cpp
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
#include "Settings.h"
#include "SharedFile.h"
#include "Library.h"
#include "LibraryBuilder.h"
#include "LibraryBuilderInternals.h"
#include "LibraryBuilderPlugins.h"
#include "HashDatabase.h"

#include "XML.h"
#include "Packet.h"
#include "Schema.h"
#include "SchemaCache.h"
#include "ID3.h"

#include "SHA.h"
#include "TigerTree.h"
#include "MD5.h"
#include "ED2K.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CLibraryBuilder LibraryBuilder;


//////////////////////////////////////////////////////////////////////
// CLibraryBuilder construction

CLibraryBuilder::CLibraryBuilder()
{
	m_pInternals	= new CLibraryBuilderInternals( this );
	m_pPlugins		= new CLibraryBuilderPlugins( this );
	
	m_hThread		= NULL;
	m_bThread		= FALSE;
	m_bPriority		= FALSE;
	m_nHashSleep	= 100;
	m_nIndex		= 0;
	m_tActive		= 0;
	m_pBuffer		= NULL;
}

CLibraryBuilder::~CLibraryBuilder()
{
	StopThread();
	Clear();
	
	delete m_pPlugins;
	delete m_pInternals;
	if ( m_pBuffer ) delete [] m_pBuffer;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilder add and remove

void CLibraryBuilder::Add(CLibraryFile* pFile)
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	POSITION pos = m_pFiles.Find( (LPVOID)pFile->m_nIndex );
	if ( pos == NULL ) m_pFiles.AddHead( (LPVOID)pFile->m_nIndex );
	
	if ( ! m_bThread ) StartThread();
}

void CLibraryBuilder::Remove(CLibraryFile* pFile)
{
	m_pSection.Lock();
	
	if ( POSITION pos = m_pFiles.Find( (LPVOID)pFile->m_nIndex ) )
	{
		m_pFiles.RemoveAt( pos );
		
		if ( pos = m_pPriority.Find( pFile->GetPath() ) )
		{
			m_pPriority.RemoveAt( pos );
		}
	}
	
	m_pSection.Unlock();
}

int CLibraryBuilder::GetRemaining()
{
	m_pSection.Lock();
	int nCount = m_pFiles.GetCount();
	if ( m_bThread ) nCount ++;
	m_pSection.Unlock();
	return nCount;
}

CString CLibraryBuilder::GetCurrentFile()
{
	m_pSection.Lock();
	CString str = m_sPath;
	if ( ! m_bThread ) str.Empty();
	m_pSection.Unlock();
	return str;
}

void CLibraryBuilder::RequestPriority(LPCTSTR pszPath)
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	POSITION pos = m_pPriority.Find( pszPath );
	if ( pos == NULL ) m_pPriority.AddTail( pszPath );
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilder clear

void CLibraryBuilder::Clear()
{
	m_pSection.Lock();
	m_pFiles.RemoveAll();
	m_pPriority.RemoveAll();
	m_pSection.Unlock();
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilder thread control

BOOL CLibraryBuilder::StartThread()
{
	if ( m_hThread != NULL && m_bThread ) return TRUE;
	
	m_pSection.Lock();
	BOOL bWorkToDo = m_pFiles.GetCount() > 0;
	m_pSection.Unlock();
	
	if ( ! bWorkToDo ) return FALSE;
	
	m_pInternals->LoadSettings();
	
	m_bThread	= TRUE;
	m_tActive	= 0;
	
	CWinThread* pThread = AfxBeginThread( ThreadStart, this, m_bPriority ?
		THREAD_PRIORITY_NORMAL : THREAD_PRIORITY_BELOW_NORMAL );
	
	m_hThread = pThread->m_hThread;
	
	return TRUE;
}

void CLibraryBuilder::StopThread()
{
	if ( m_hThread == NULL ) return;
	
	m_bThread = FALSE;
	
    int nAttempt = 20;
	for ( ; nAttempt > 0 ; nAttempt-- )
	{
		DWORD nCode;
		if ( ! GetExitCodeThread( m_hThread, &nCode ) ) break;
		if ( nCode != STILL_ACTIVE ) break;
		Sleep( 150 );
	}
	
	if ( nAttempt == 0 )
	{
		TerminateThread( m_hThread, 0 );
		theApp.Message( MSG_DEBUG, _T("WARNING: Terminating CLibraryBuilder thread.") );
		Sleep( 100 );
	}
	
	m_hThread	= NULL;
	m_tActive	= 0;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilder priority control

void CLibraryBuilder::BoostPriority(BOOL bPriority)
{
	if ( m_bPriority == bPriority ) return;
	m_bPriority = bPriority;
	
	if ( m_bThread && m_hThread != NULL )
	{
		SetThreadPriority( m_hThread, m_bPriority ?
			THREAD_PRIORITY_NORMAL : THREAD_PRIORITY_BELOW_NORMAL );
	}
}

BOOL CLibraryBuilder::GetBoostPriority()
{
	return m_bPriority;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilder sanity check

BOOL CLibraryBuilder::SanityCheck()
{
	if ( ! m_bThread || ! m_tActive ) return TRUE;
	
	CSingleLock pLock( &m_pSection );
	
	if ( pLock.Lock( 50 ) )
	{
		if ( ! m_tActive || GetTickCount() - m_tActive < 180000 ) return TRUE;
		
		theApp.Message( MSG_ERROR, _T("CLibraryBuilder sanity check: stuck on \"%s\" (%lu)."),
			(LPCTSTR)m_sPath, ( GetTickCount() - m_tActive ) / 1000 );
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilder thread run (threaded)

UINT CLibraryBuilder::ThreadStart(LPVOID pParam)
{
	CLibraryBuilder* pBuilder = (CLibraryBuilder*)pParam;
	pBuilder->OnRun();
	return 0;
}

void CLibraryBuilder::OnRun()
{
	if ( m_pBuffer == NULL ) m_pBuffer = new BYTE[20480];
	
	while ( m_bThread )
	{
		if ( m_pSection.Lock() )
		{
			m_nIndex	= 0;
			m_tActive	= 0;
			m_sPath.Empty();
			
			if ( m_pFiles.IsEmpty() )
			{
				m_pSection.Unlock();
				break;
			}
			
			m_nIndex = (DWORD)m_pFiles.RemoveHead();
			
			m_pSection.Unlock();
		}
		
		if ( m_nIndex == 0 )
		{
			Sleep( 250 );
			continue;
		}
		
		{
			CQuickLock oLock( Library.m_pSection );
			if ( CLibraryFile* pFile = Library.LookupFile( m_nIndex ) )
			{
				m_sPath = pFile->GetPath();
			}
			else
			{
				m_nIndex = 0;
				continue;
			}
		}
		
		BOOL bPriority = FALSE;
		
		if ( m_pSection.Lock() )
		{
			if ( POSITION pos = m_pPriority.Find( m_sPath ) )
			{
				bPriority = TRUE;
				m_pPriority.RemoveAt( pos );
			}
			
			m_pSection.Unlock();
		}
		
		HANDLE hFile = CreateFile( m_sPath, GENERIC_READ,
			FILE_SHARE_READ, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN, NULL );
		
		if ( hFile == INVALID_HANDLE_VALUE )
		{
			m_pSection.Lock();
			if ( m_pFiles.Find( NULL ) == NULL ) m_pFiles.AddTail( (LPVOID)0 );
			m_pFiles.AddTail( (LPVOID)m_nIndex );
			m_pSection.Unlock();
			continue;
		}
		
		theApp.Message( MSG_DEBUG, _T("Hashing: %s"), (LPCTSTR)m_sPath );
		
		SHA1 pSHA1;
		
		if ( HashFile( hFile, bPriority, &pSHA1 ) )
		{
			SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
			m_tActive = GetTickCount();
			
			if ( m_pPlugins->ExtractMetadata( m_sPath, hFile ) )
			{
				// Plugin got it
			}
			else if ( m_pInternals->ExtractMetadata( m_sPath, hFile, &pSHA1 ) )
			{
				// Internal got it
			}
		}
		
		CloseHandle( hFile );
	}
	
	m_pPlugins->Cleanup();
	
	delete [] m_pBuffer;
	m_pBuffer = NULL;
	
	m_nIndex	= 0;
	m_tActive	= 0;
	m_bThread	= FALSE;
	m_sPath.Empty();
	
	theApp.Message( MSG_DEBUG, _T("CLibraryBuilder shutting down.") );
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilder file hashing (threaded)

BOOL CLibraryBuilder::HashFile(HANDLE hFile, BOOL bPriority, SHA1* pOutSHA1)
{
	DWORD nSizeHigh	= 0;
	DWORD nSizeLow	= GetFileSize( hFile, &nSizeHigh );
	QWORD nFileSize	= (QWORD)nSizeLow | ( (QWORD)nSizeHigh << 32 );
	QWORD nFileBase	= 0;
	
	BOOL bVirtual = FALSE;
	
	if ( Settings.Library.VirtualFiles )
		bVirtual = DetectVirtualFile( hFile, nFileBase, nFileSize );
	
	nSizeLow	= (DWORD)( nFileBase & 0xFFFFFFFF );
	nSizeHigh	= (DWORD)( nFileBase >> 32 );
	SetFilePointer( hFile, nSizeLow, (PLONG)&nSizeHigh, FILE_BEGIN );
	
	CTigerTree pTiger;
	CED2K pED2K;
	CSHA pSHA1;
	CMD5 pMD5;
	
	pTiger.BeginFile( Settings.Library.TigerHeight, nFileSize );
	pED2K.BeginFile( nFileSize );
	
	for ( QWORD nLength = nFileSize ; nLength > 0 ; )
	{
		DWORD nBlock	= (DWORD)min( nLength, QWORD(20480) );
		DWORD nTime		= GetTickCount();
		
		ReadFile( hFile, m_pBuffer, nBlock, &nBlock, NULL );
		
		pSHA1.Add( m_pBuffer, nBlock );
		pMD5.Add( m_pBuffer, nBlock );
		pTiger.AddToFile( m_pBuffer, nBlock );
		pED2K.AddToFile( m_pBuffer, nBlock );
		
		nLength -= nBlock;
		
		if ( ! m_bPriority && ! bPriority )
		{
			if ( nBlock == 20480 ) m_nHashSleep = ( GetTickCount() - nTime ) * 2;
			m_nHashSleep = max( m_nHashSleep, DWORD(20) );
			Sleep( m_nHashSleep );
		}
		
		if ( ! m_bThread ) return FALSE;
	}
	
	pSHA1.Finish();
	pMD5.Finish();
	pTiger.FinishFile();
	pED2K.FinishFile();
	
	{
		CQuickLock oLock( Library.m_pSection );
		CLibraryFile* pFile = Library.LookupFile( m_nIndex );
		if ( pFile == NULL ) return FALSE;
		
		Library.RemoveFile( pFile );
		
		pFile->m_bBogus			= FALSE;
		pFile->m_nVirtualBase	= bVirtual ? nFileBase : 0;
		pFile->m_nVirtualSize	= bVirtual ? nFileSize : 0;
		
		pFile->m_bSHA1 = TRUE;
		pSHA1.GetHash( &pFile->m_pSHA1 );
		if ( pOutSHA1 != NULL ) *pOutSHA1 = pFile->m_pSHA1;
		
		pFile->m_bMD5 = TRUE;
		pMD5.GetHash( &pFile->m_pMD5 );
		
		pFile->m_bTiger = TRUE;
		pTiger.GetRoot( &pFile->m_pTiger );
		
		pFile->m_bED2K = TRUE;
		pED2K.GetRoot( &pFile->m_pED2K );
		
		LibraryMaps.CullDeletedFiles( pFile );
		Library.AddFile( pFile );
		Library.Update();
	}
	
	LibraryHashDB.StoreTiger( m_nIndex, &pTiger );
	LibraryHashDB.StoreED2K( m_nIndex, &pED2K );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilder metadata submission (threaded)

BOOL CLibraryBuilder::SubmitMetadata(LPCTSTR pszSchemaURI, CXMLElement*& pXML)
{
	CSchema* pSchema = SchemaCache.Get( pszSchemaURI );
	
	if ( pSchema == NULL )
	{
		delete pXML;
		return FALSE;
	}
	
	CXMLElement* pBase = pSchema->Instantiate( TRUE );
	pBase->AddElement( pXML );
	
	if ( ! pSchema->Validate( pBase, TRUE ) )
	{
		delete pBase;
		return FALSE;
	}
	
	pXML->Detach();
	delete pBase;
	
	CQuickLock oLock( Library.m_pSection );
	if ( CLibraryFile* pFile = Library.LookupFile( m_nIndex ) )
	{
		if ( pFile->m_pMetadata == NULL )
		{
			Library.RemoveFile( pFile );
			
			pFile->m_pSchema		= pSchema;
			pFile->m_pMetadata		= pXML;
			pFile->m_bMetadataAuto	= TRUE;
			
			Library.AddFile( pFile );
			Library.Update();
			
			return TRUE;
		}
		
	}
	
	delete pXML;
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilder bogus/corrupted state submission (threaded)

BOOL CLibraryBuilder::SubmitCorrupted()
{
	CQuickLock oLock( Library.m_pSection );
	if ( CLibraryFile* pFile = Library.LookupFile( m_nIndex ) )
	{
		pFile->m_bBogus = TRUE;
		return TRUE;
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilder virtual file detection (threaded)

BOOL CLibraryBuilder::DetectVirtualFile(HANDLE hFile, QWORD& nOffset, QWORD& nLength)
{
	BOOL bVirtual = FALSE;
	
	if ( _tcsistr( m_sPath, _T(".mp3") ) != NULL )
	{
		bVirtual |= DetectVirtualID3v2( hFile, nOffset, nLength );
		bVirtual |= DetectVirtualID3v1( hFile, nOffset, nLength );
	}
	
	return bVirtual;
}

BOOL CLibraryBuilder::DetectVirtualID3v1(HANDLE hFile, QWORD& nOffset, QWORD& nLength)
{
	ID3V1 pInfo;
	DWORD nRead;
	
	if ( nLength <= 128 ) return FALSE;
	
	LONG nPosLow	= (LONG)( ( nOffset + nLength - 128 ) & 0xFFFFFFFF );
	LONG nPosHigh	= (LONG)( ( nOffset + nLength - 128 ) >> 32 );
	SetFilePointer( hFile, nPosLow, &nPosHigh, FILE_BEGIN );
	
	ReadFile( hFile, &pInfo, sizeof(pInfo), &nRead, NULL );
	
	if ( nRead != sizeof(pInfo) ) return FALSE;
	if ( memcmp( pInfo.szTag, ID3V1_TAG, 3 ) ) return FALSE;
	
	nLength -= 128;
	
	return TRUE;
}

BOOL CLibraryBuilder::DetectVirtualID3v2(HANDLE hFile, QWORD& nOffset, QWORD& nLength)
{
	ID3V2_HEADER pHeader;
	DWORD nRead;
	
	LONG nPosLow	= (LONG)( ( nOffset ) & 0xFFFFFFFF );
	LONG nPosHigh	= (LONG)( ( nOffset ) >> 32 );
	SetFilePointer( hFile, nPosLow, &nPosHigh, FILE_BEGIN );
	
	ReadFile( hFile, &pHeader, sizeof(pHeader), &nRead, NULL );
	if ( nRead != sizeof(pHeader) ) return FALSE;
	
	if ( strncmp( pHeader.szTag, ID3V2_TAG, 3 ) ) return FALSE;
	if ( pHeader.nMajorVersion < 2 || pHeader.nMajorVersion > 4 ) return FALSE;
	if ( pHeader.nFlags & ~ID3V2_KNOWNMASK ) return FALSE;
	if ( pHeader.nFlags & ID3V2_UNSYNCHRONISED ) return FALSE;
	
	DWORD nTagSize = SWAP_LONG( pHeader.nSize );
	ID3_DESYNC_SIZE( nTagSize );
	
	if ( pHeader.nFlags & ID3V2_FOOTERPRESENT ) nTagSize += 10;
	nTagSize += sizeof(pHeader);
	
	if ( nLength <= nTagSize ) return FALSE;
	
	nOffset += nTagSize;
	nLength -= nTagSize;
	
	return TRUE;
}
