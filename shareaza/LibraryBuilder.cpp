//
// LibraryBuilder.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

CLibraryBuilder::CLibraryBuilder() :
	m_hThread( NULL ),
	m_bThread( FALSE ),
	m_bPriority( FALSE ),
	m_nReaded( 0 ),
	m_nElapsed( 0 )
{
	QueryPerformanceFrequency( &m_nFreq );
	QueryPerformanceCounter( &m_nLastCall );
}

CLibraryBuilder::~CLibraryBuilder()
{
	StopThread();
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilder add and remove

void CLibraryBuilder::Add(CLibraryFile* pFile)
{
	CQuickLock pLock( m_pSection );

	if ( std::find( m_pFiles.begin(), m_pFiles.end(), pFile->m_nIndex ) == m_pFiles.end() )
		m_pFiles.push_back( pFile->m_nIndex );

	if ( ! m_bThread ) StartThread();
}

int CLibraryBuilder::GetRemaining()
{
	CQuickLock oLock( m_pSection );

	return (int)m_pFiles.size();
}

CString CLibraryBuilder::GetCurrent()
{
	CQuickLock oLock( m_pSection );

	return m_sPath;
}

void CLibraryBuilder::RequestPriority(LPCTSTR pszPath)
{
	CQuickLock oLock( m_pPrioritySection );

	m_pPriority.remove( pszPath );
	m_pPriority.push_front( pszPath );
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilder remove file from processing queue

void CLibraryBuilder::Remove(CLibraryFile* pFile)
{
	{
		CQuickLock oLock( m_pSection );

		m_pFiles.remove( pFile->m_nIndex );
	}
	{
		CQuickLock oLock( m_pPrioritySection );

		m_pPriority.remove( pFile->GetPath() );
	}
}

void CLibraryBuilder::Remove(LPCTSTR szPath)
{
	ASSERT( szPath );

	CLibraryFile* pFile = LibraryMaps.LookupFileByPath( szPath );
	if ( pFile )
	{
		CQuickLock oLock( m_pSection );
		m_pFiles.remove( pFile->m_nIndex );
	}
	{
		CQuickLock oLock( m_pPrioritySection );
		m_pPriority.remove( szPath );
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilder rotate queue

void CLibraryBuilder::Skip()
{
	// Put first file to end
	DWORD index;
	{
		CQuickLock oLock( m_pSection );
		index = *m_pFiles.begin();
		if ( m_pFiles.size() > 1 )
		{
			m_pFiles.pop_front();
			m_pFiles.push_back( index );
		}
	}

	// Remove it from priority list
	CLibraryFile* pFile = LibraryMaps.LookupFile( index );
	if ( pFile )
	{
		CQuickLock oLock( m_pPrioritySection );
		m_pPriority.remove( pFile->GetPath() );
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilder clear

void CLibraryBuilder::Clear()
{
	{
		CQuickLock oLock( m_pSection );
		m_pFiles.clear();
		m_sPath.Empty();
	}
	{
		CQuickLock oLock( m_pPrioritySection );
		m_pPriority.clear();
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilder thread control

BOOL CLibraryBuilder::StartThread()
{
	if ( m_hThread || m_bThread ) return TRUE;

	CQuickLock oLock( m_pSection );

	if ( m_pFiles.empty() ) return FALSE;

	m_bThread	= TRUE;

	m_hThread = BeginThread( "LibraryBuilder", ThreadStart, this, m_bPriority ?
		THREAD_PRIORITY_BELOW_NORMAL : THREAD_PRIORITY_IDLE );

	return TRUE;
}

void CLibraryBuilder::StopThread()
{
	if ( m_hThread == NULL || ! m_bThread ) return;

	m_bThread = FALSE;

	CloseThread( &m_hThread );
}

BOOL CLibraryBuilder::IsAlive() const
{
	return m_bThread;
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
			THREAD_PRIORITY_BELOW_NORMAL : THREAD_PRIORITY_IDLE );
	}
}

BOOL CLibraryBuilder::GetBoostPriority()
{
	return m_bPriority;
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
		QWORD nSpeed = ( m_nReaded * 1000000 ) / m_nElapsed;	// B/s
		QWORD nMaxSpeed = 1024 * 1024 * ( bPriority ? Settings.Library.HighPriorityHashing :
			Settings.Library.LowPriorityHashing);				// B/s
		if ( nMaxSpeed > 0 && nSpeed > nMaxSpeed )
		{
			QWORD nDelay = ( ( nSpeed * m_nElapsed ) / nMaxSpeed ) - m_nElapsed;
			if ( nDelay > 1000000 )
				// Dont do overcompensation, max 1 sec
				nDelay = 1000000;
			if ( nDelay > 1000 )
			{
				// Compensation
				Sleep( (DWORD) ( nDelay / 1000 ) );
			}
		}
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilder thread run (threaded)

UINT CLibraryBuilder::ThreadStart(LPVOID pParam)
{
	((CLibraryBuilder*)pParam)->OnRun();
	return 0;
}

void CLibraryBuilder::OnRun()
{
	CLibraryBuilderInternals Internals;
	CLibraryBuilderPlugins Plugins;
	
	Internals.LoadSettings();

	while ( m_bThread )
	{
		Sleep( 100 );

		DWORD nIndex = 0;
		CString sPath;

		CSingleLock oLock0( &Library.m_pSection );
		if ( oLock0.Lock( 100 ) )
		{
			CSingleLock oLock1( &m_pSection );
			if ( oLock1.Lock( 100 ) )
			{
				if ( m_pFiles.empty() )
					// No files left
					break;

				std::list< DWORD >::iterator i = m_pFiles.begin();

				CSingleLock oLock2( &m_pPrioritySection );
				if ( oLock2.Lock( 100 ) )
				{
					if ( ! m_pPriority.empty() )
					{
						CString sFoo( *m_pPriority.begin() );
						m_pPriority.pop_front();
						CLibraryFile* pFile = LibraryMaps.LookupFileByPath( sFoo );
						if ( pFile )
						{
							i = std::find( m_pFiles.begin(), m_pFiles.end(), pFile->m_nIndex );
						}
					}
					oLock2.Unlock();
				}

				CLibraryFile* pFile = LibraryMaps.LookupFile( (*i) );
				if ( pFile )
				{
					sPath = pFile->GetPath();

					// Remove private type
					if ( int nExt = sPath.ReverseFind( _T('.') ) + 1 )
					{
						if ( IsIn( Settings.Library.PrivateTypes, (LPCTSTR)sPath + nExt ) )
						{
							Remove( sPath );
							continue;
						}
					}

					// Ready to hash
					nIndex = (*i);
					m_sPath = sPath;
				}
				oLock1.Unlock();
			}
			oLock0.Unlock();
		}
		if ( nIndex )
		{
			HANDLE hFile = CreateFile( sPath, GENERIC_READ,
				FILE_SHARE_READ | ( theApp.m_bNT ? FILE_SHARE_DELETE : 0 ), NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
			VERIFY_FILE_ACCESS( hFile, sPath )
			if ( hFile != INVALID_HANDLE_VALUE )
			{
				theApp.Message( MSG_DEBUG, _T("Hashing: %s"), (LPCTSTR)sPath );

				Hashes::Sha1Hash oSHA1;
				Hashes::Md5Hash oMD5;
				// ToDo: We need MD5 hash of the audio file without tags...
				if ( HashFile( sPath, hFile, oSHA1, oMD5, nIndex ) )
				{
					SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
					Internals.ExtractMetadata( nIndex, sPath, hFile, oSHA1, oMD5 );

					SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
					Plugins.ExtractMetadata( nIndex, sPath, hFile );

					// Done
					Remove( sPath );
				}
				else
					Skip();

				CloseHandle( hFile );
			}
			else
			{
				DWORD err = GetLastError();
				if ( err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND )
					// Fatal error
					Remove( sPath );
				else
					Skip();
			}
		}
		m_sPath.Empty();
	}

	Settings.Live.NewFile = FALSE;
	m_bThread	= FALSE;
	m_hThread	= NULL;

	theApp.Message( MSG_DEBUG, _T("CLibraryBuilder shutting down.") );
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilder file hashing (threaded)

#define MAX_HASH_BUFFER_SIZE	262144ul	// 256 Kb

BOOL CLibraryBuilder::HashFile(LPCTSTR szPath, HANDLE hFile, Hashes::Sha1Hash& oOutSHA1, Hashes::Md5Hash& oOutMD5, DWORD nIndex)
{
	char pBuffer [MAX_HASH_BUFFER_SIZE];

	DWORD nSizeHigh	= 0;
	DWORD nSizeLow	= GetFileSize( hFile, &nSizeHigh );
	QWORD nFileSize	= (QWORD)nSizeLow | ( (QWORD)nSizeHigh << 32 );
	QWORD nFileBase	= 0;

	BOOL bVirtual = FALSE;

	if ( Settings.Library.VirtualFiles )
		bVirtual = DetectVirtualFile( szPath, hFile, nFileBase, nFileSize );

	nSizeLow	= (DWORD)( nFileBase & 0xFFFFFFFF );
	nSizeHigh	= (DWORD)( nFileBase >> 32 );
	SetFilePointer( hFile, nSizeLow, (PLONG)&nSizeHigh, FILE_BEGIN );

	CTigerTree pTiger;
	CED2K pED2K;
	CSHA pSHA1;
	CMD5 pMD5;

	pTiger.BeginFile( Settings.Library.TigerHeight, nFileSize );
	pED2K.BeginFile( nFileSize );

	// Reset statistics
	m_nElapsed = 0;
	m_nReaded = 0;

	DWORD nBlock;
	for ( QWORD nLength = nFileSize ; nLength > 0 ; nLength -= nBlock )
	{
		nBlock	= min( nLength, MAX_HASH_BUFFER_SIZE );

		if ( ! m_bThread ) return FALSE;

		if ( ! ReadFileWithPriority( hFile, pBuffer, nBlock, &nBlock, m_bPriority ) ) return FALSE;

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
		CQuickLock oLibraryLock( Library.m_pSection );
		CLibraryFile* pFile = Library.LookupFile( nIndex );
		if ( pFile == NULL ) return FALSE;

		Library.RemoveFile( pFile );

		pFile->m_bBogus			= FALSE;
		pFile->m_nVirtualBase	= bVirtual ? nFileBase : 0;
		pFile->m_nVirtualSize	= bVirtual ? nFileSize : 0;
		
		pSHA1.GetHash( pFile->m_oSHA1 );
		oOutSHA1 = pFile->m_oSHA1;
		pMD5.GetHash( pFile->m_oMD5 );
		oOutMD5 = pFile->m_oMD5;
		pTiger.GetRoot( pFile->m_oTiger );
		pED2K.GetRoot( pFile->m_oED2K );
		
		LibraryMaps.CullDeletedFiles( pFile );
		Library.AddFile( pFile );

		// child pornography check
		if ( Settings.Search.AdultFilter &&
			( AdultFilter.IsChildPornography( pFile->GetSearchName() ) ||
			  AdultFilter.IsChildPornography( pFile->GetMetadataWords() ) ) )
		{
			pFile->m_bVerify = pFile->m_bShared = TS_FALSE;
		}

		theApp.Message( MSG_DEBUG, _T("Hashing completed: %s"), szPath );

		Library.Update();
	}

	LibraryHashDB.StoreTiger( nIndex, &pTiger );
	LibraryHashDB.StoreED2K( nIndex, &pED2K );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilder metadata submission (threaded)

int CLibraryBuilder::SubmitMetadata(DWORD nIndex, LPCTSTR pszSchemaURI, CXMLElement*& pXML)
{
	int nAttributeCount = 0;
	CSchema* pSchema = SchemaCache.Get( pszSchemaURI );

	if ( pSchema == NULL )
	{
		delete pXML;
		return nAttributeCount;
	}

	CXMLElement* pBase = pSchema->Instantiate( TRUE );
	pBase->AddElement( pXML );

	if ( ! pSchema->Validate( pBase, TRUE ) )
	{
		delete pBase;
		return nAttributeCount;
	}

	pXML->Detach();
	delete pBase;

	nAttributeCount = pXML->GetAttributeCount();

	CQuickLock oLibraryLock( Library.m_pSection );
	if ( CLibraryFile* pFile = Library.LookupFile( nIndex ) )
	{
		if ( pFile->m_pMetadata )
			// Merge new with old metadata
			pXML->Merge( pFile->m_pMetadata );
		else
			pFile->m_bMetadataAuto	= TRUE;

		Library.RemoveFile( pFile );

		// Delete old one
		delete pFile->m_pMetadata;

		// Set new matadata
		pFile->m_pSchema		= pSchema;
		pFile->m_pMetadata		= pXML;
		pFile->ModifyMetadata();

		Library.AddFile( pFile );
		Library.Update();

		return nAttributeCount;
	}

	delete pXML;

	return nAttributeCount;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilder bogus/corrupted state submission (threaded)

BOOL CLibraryBuilder::SubmitCorrupted(DWORD nIndex)
{
	CQuickLock oLibraryLock( Library.m_pSection );
	if ( CLibraryFile* pFile = Library.LookupFile( nIndex ) )
	{
		pFile->m_bBogus = TRUE;
		return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilder virtual file detection (threaded)

BOOL CLibraryBuilder::DetectVirtualFile(LPCTSTR szPath, HANDLE hFile, QWORD& nOffset, QWORD& nLength)
{
	BOOL bVirtual = FALSE;

	if ( _tcsistr( szPath, _T(".mp3") ) != NULL )
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

	DWORD nTagSize = swapEndianess( pHeader.nSize );
	ID3_DESYNC_SIZE( nTagSize );

	if ( pHeader.nFlags & ID3V2_FOOTERPRESENT ) nTagSize += 10;
	nTagSize += sizeof(pHeader);

	if ( nLength <= nTagSize ) return FALSE;

	nOffset += nTagSize;
	nLength -= nTagSize;

	return TRUE;
}
