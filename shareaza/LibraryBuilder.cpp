//
// LibraryBuilder.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2006.
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
#include "Security.h"

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
	m_nIndex		= 0;
	m_tActive		= 0;

	m_nReaded		= 0;
	m_nElapsed		= 0;
	QueryPerformanceFrequency( &m_nFreq );
	QueryPerformanceCounter( &m_nLastCall );
}

CLibraryBuilder::~CLibraryBuilder()
{
	StopThread();
	Clear();

	delete m_pPlugins;
	delete m_pInternals;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilder add and remove

void CLibraryBuilder::Add(CLibraryFile* pFile)
{
	CSingleLock pLock( &m_pSection, TRUE );

	POSITION pos = m_pFiles.Find( pFile->m_nIndex );
	if ( pos == NULL ) m_pFiles.AddHead( pFile->m_nIndex );

	if ( ! m_bThread ) StartThread();
}

void CLibraryBuilder::Remove(CLibraryFile* pFile)
{
	m_pSection.Lock();

	if ( POSITION pos = m_pFiles.Find( pFile->m_nIndex ) )
	{
		m_pFiles.RemoveAt( pos );

		if ( pos = m_pPriority.Find( pFile->GetPath() ) )
		{
			m_pPriority.RemoveAt( pos );
		}
	}

	m_pSection.Unlock();
}

INT_PTR CLibraryBuilder::GetRemaining()
{
	m_pSection.Lock();
	INT_PTR nCount = m_pFiles.GetCount();
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

void CLibraryBuilder::UpdateStatus(CString& strFileName, int* pRemaining )
{
	m_pSection.Lock();
		
	if ( pRemaining != NULL )
	{
		*pRemaining = static_cast< int >( m_pFiles.GetCount() );
		if ( m_bThread ) *pRemaining ++;
	}

	strFileName = m_sPath;
	if ( ! m_bThread ) 
		strFileName.Empty();

	m_pSection.Unlock();
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
	BOOL bWorkToDo = !m_pFiles.IsEmpty();
	m_pSection.Unlock();

	if ( ! bWorkToDo ) return FALSE;

	m_pInternals->LoadSettings();

	m_bThread	= TRUE;
	m_tActive	= 0;

	m_hThread = BeginThread( "LibraryBuilder", ThreadStart, this, m_bPriority ?
		THREAD_PRIORITY_NORMAL : THREAD_PRIORITY_BELOW_NORMAL );

	return TRUE;
}

void CLibraryBuilder::StopThread()
{
	if ( m_hThread == NULL ) return;

	m_bThread = FALSE;

	CloseThread( &m_hThread );

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
/*
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
*/

//////////////////////////////////////////////////////////////////////
// CLibraryBuilder rehash current file

void CLibraryBuilder::ReHashCurrentFile()
{
	m_pSection.Lock();
	if ( m_pFiles.Find( 0ul ) == NULL ) m_pFiles.AddTail( 0ul );
	m_pFiles.AddTail( m_nIndex );
	m_pSection.Unlock();
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilder read file with speed limits

BOOL CLibraryBuilder::ReadFileWithPriority(
	HANDLE hFile,
	LPVOID lpBuffer,
	DWORD nNumberOfBytesToRead,
	LPDWORD lpNumberOfBytesRead,
	BOOL bPriority /* = TRUE */ )
{
	BOOL ret = ::ReadFile ( hFile, lpBuffer, nNumberOfBytesToRead,
		lpNumberOfBytesRead, NULL );

	CQuickLock oLock( m_pDelaySection );

	LARGE_INTEGER count;
	QueryPerformanceCounter( &count );
	m_nElapsed += ((( count.QuadPart - m_nLastCall.QuadPart ) * 1000000 ) /
		m_nFreq.QuadPart);
	m_nLastCall.QuadPart = count.QuadPart;

	if ( ret ) m_nReaded += (QWORD) *lpNumberOfBytesRead;

	// Check speed every...
	if ( m_nElapsed > 0 && m_nReaded > 0 )
	{
		// Calculation of compensation delay
		QWORD nSpeed = ( m_nReaded * 1000000 ) / m_nElapsed; // B/s
		QWORD nMaxSpeed = 1024 * 1024 *
			( bPriority ? Settings.Library.HighPriorityHashing : Settings.Library.LowPriorityHashing);
		if ( nSpeed > nMaxSpeed )
		{
			QWORD nDelay = ( ( nSpeed * m_nElapsed ) / nMaxSpeed ) - m_nElapsed;
			if ( nDelay > 500000 )
				// Dont do overcompensation
				nDelay = 500000;
			if ( nDelay > 1000 )
				// Compensation
				Sleep( (DWORD) ( nDelay / 1000 ) );
		}
	}
	if ( m_nElapsed > 10000000 ) // 10 s
	{
		// Reset statistics
		m_nElapsed = 0;
		m_nReaded = 0;
	}

	return ret;
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

			m_nIndex = m_pFiles.RemoveHead();

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

				if ( LPCTSTR pszExt = _tcsrchr( m_sPath, '.' ) )
				{
					if ( IsIn( Settings.Library.PrivateTypes, pszExt + 1 ) )
					{
						continue;
					}
				}
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

		if ( hFile != INVALID_HANDLE_VALUE )
		{
			theApp.Message( MSG_DEBUG, _T("Hashing: %s"), (LPCTSTR)m_sPath );

			Hashes::Sha1Hash oSHA1;

			if ( HashFile( hFile, bPriority, oSHA1 ) )
			{
				SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
				m_tActive = GetTickCount();

				if ( m_pPlugins->ExtractMetadata( m_sPath, hFile ) )
				{
					// Plugin got it
				}
				else if ( m_pInternals->ExtractMetadata( m_sPath, hFile, oSHA1 ) )
				{
					// Internal got it
				}
			}
			else
				ReHashCurrentFile();

			CloseHandle( hFile );
		}
		else
			ReHashCurrentFile();

		m_nIndex	= 0;
		m_sPath.Empty();
	}
	
	Settings.Live.NewFile = FALSE;
	m_pPlugins->Cleanup();

	m_tActive	= 0;
	m_bThread	= FALSE;

	theApp.Message( MSG_DEBUG, _T("CLibraryBuilder shutting down.") );
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilder file hashing (threaded)

#define MAX_HASH_BUFFER_SIZE	20480ul	// 20 Kb

BOOL CLibraryBuilder::HashFile(HANDLE hFile, BOOL bPriority, Hashes::Sha1Hash& oOutSHA1)
{
	char pBuffer [MAX_HASH_BUFFER_SIZE];

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

	DWORD nBlock;
	for ( QWORD nLength = nFileSize ; nLength > 0 ; nLength -= nBlock )
	{
		nBlock	= min( nLength, MAX_HASH_BUFFER_SIZE );

		if ( ! m_bThread ) return FALSE;

		if ( ! ReadFileWithPriority( hFile, pBuffer, nBlock, &nBlock, m_bPriority || bPriority ) ) return FALSE;

		if ( ! nBlock ) return FALSE;

		pSHA1.Add( pBuffer, nBlock );
		pMD5.Add( pBuffer, nBlock );
		pTiger.AddToFile( pBuffer, nBlock );
		pED2K.AddToFile( pBuffer, nBlock );
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
		
		pSHA1.GetHash( pFile->m_oSHA1 );
		oOutSHA1 = pFile->m_oSHA1;
		pMD5.GetHash( pFile->m_oMD5 );
		pTiger.GetRoot( pFile->m_oTiger );
		pED2K.GetRoot( pFile->m_oED2K );
		
		LibraryMaps.CullDeletedFiles( pFile );
		Library.AddFile( pFile );
		
		// child pornography check
		bool bHit = false;
		if ( AdultFilter.IsChildPornography( pFile->GetSearchName() ) )
			bHit = true;
		else
		{
			if ( AdultFilter.IsChildPornography( pFile->GetMetadataWords() ) )
				bHit = true;
		}
		if ( bHit )
			pFile->m_bVerify = pFile->m_bShared = TS_FALSE;

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

	if ( ! ReadFile( hFile, &pInfo, sizeof(pInfo), &nRead, NULL ) ) return FALSE;
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

	if ( ! ReadFile( hFile, &pHeader, sizeof(pHeader), &nRead, NULL ) ) return FALSE;
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
