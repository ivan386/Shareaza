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
	m_bTerminate	= FALSE;
	m_bPriority		= FALSE;
//	m_nHashSleep	= 100;
	m_nIndex		= 0;
	m_tActive		= 0;
//	m_pBuffer		= NULL;
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
	
	POSITION pos = m_pFiles.Find( (LPVOID)pFile->m_nIndex );
	if ( pos == NULL ) 
	{
		BOOL bNotFound = TRUE;
		if ( m_bThread && !m_bTerminate )
		{
			DWORD nEntry = 0;
			if ( m_nStackTop ) do
			{
			}
			while ( ( bNotFound = m_qStack[ nEntry ].m_nIndex != pFile->m_nIndex ) && ( ++nEntry < m_nStackTop ) );
			if ( bNotFound ) bNotFound = m_qStackN.m_nIndex != pFile->m_nIndex;
		}
		if ( bNotFound ) m_pFiles.AddHead( (LPVOID)pFile->m_nIndex );
	}
	if ( ! m_bThread ) StartThread();
}

void CLibraryBuilder::Remove(CLibraryFile* pFile)
{
	m_pSection.Lock();
	
	if ( POSITION pos = m_pFiles.Find( (LPVOID)pFile->m_nIndex ) )
	{
		m_pFiles.RemoveAt( pos );
		SignalListChanged();
/*		if ( pos = m_pPriority.Find( pFile->GetPath() ) )
		{
			m_pPriority.RemoveAt( pos );
		}*/
	}
	
	m_pSection.Unlock();
}

int CLibraryBuilder::GetRemaining()
{
	m_pSection.Lock();
	int nCount = m_pFiles.GetCount();
	if ( m_bThread ) nCount += m_nStackTop + 1;
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
/*	CSingleLock pLock( &m_pSection, TRUE );
	
	POSITION pos = m_pPriority.Find( pszPath );
	if ( pos == NULL ) m_pPriority.AddTail( pszPath );*/
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilder clear

void CLibraryBuilder::Clear()
{
	m_pSection.Lock();
	m_pFiles.RemoveAll();
//	m_pPriority.RemoveAll();
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
	RequestTerminateBuilder();
	
	for ( int nAttempt = 100 ; nAttempt > 0 ; nAttempt-- )
	{
		DWORD nCode;
		if ( ! GetExitCodeThread( m_hThread, &nCode ) ) break;
		if ( nCode != STILL_ACTIVE ) break;
		Sleep( 30 );
	}
	
	if ( nAttempt == 0 )
	{
		TerminateThread( m_hThread, 0 );
		theApp.Message( MSG_DEBUG, _T("WARNING: Terminating CLibraryBuilder thread.") );
		Sleep( 100 );
	}
	
	m_bThread	= FALSE;
	m_bTerminate = FALSE;
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
	DWORD nEntry;
	if ( m_pBuffer == NULL )
	{
		Init();
		m_pBuffer = (BYTE*)malloc( ( ( m_nStackSize - 1 ) * m_nMaxBlock + 1 ) * LIBRARY_BUILDER_BLOCK_SIZE );
		if ( m_pBuffer == NULL )
		{
			m_nMaxBlock = m_nStackSize = 1;
			m_pBuffer = (BYTE*)malloc( LIBRARY_BUILDER_BLOCK_SIZE );
			ASSERT( m_pBuffer != NULL );
		}
		nEntry = 0;
		if ( m_nStackSize > 1 ) do
		{
			m_qStack[ nEntry ].m_pBufferIndex = m_qStack[ nEntry ].m_pBuffer =
				m_pBuffer + ( nEntry * m_nMaxBlock + 1 ) * LIBRARY_BUILDER_BLOCK_SIZE;
		}
		while ( ++nEntry < m_nStackSize - 1 );
		do
		{
			m_qStack[ nEntry ].m_pBufferIndex = m_qStack[ nEntry ].m_pBuffer = NULL;
		}
		while ( ++nEntry < MAX_PARALLEL );
    	m_qStackN.m_pBufferIndex = m_qStackN.m_pBuffer = NULL;
		m_qStackO.m_pBufferIndex = m_qStackO.m_pBuffer = NULL;
		m_nStackTop = 0;
	}
	while ( m_bThread && !m_bTerminate )
	{
		if ( m_pSection.Lock() )
		{
			m_nIndex	= 0;
			m_tActive	= 0;
			m_qStackN.m_sPath.Empty();
			
			if ( m_bTerminate )
			{
				m_pSection.Unlock();
				break;
			}

			if ( m_bSuspend )
			{
				m_bSuspended = TRUE;
				m_pSection.Unlock();
				while ( m_bSuspend )
				{
					if ( m_bTerminate ) break;
					Sleep ( 20 );
				}
				if ( m_bTerminate ) break;
				SignalListChanged();
				m_bSuspended = FALSE;
				continue;
			}

			if ( m_bListChanged )
			{
				m_pSection.Unlock();
				nEntry = 0;
				while ( nEntry < m_nStackTop )
				{
					if ( Library.LookupFile( m_qStackN.m_nIndex, TRUE ) == NULL )
					{
						CloseHandle( m_qStack[ nEntry ].hFile );
						BYTE* pBuffer = m_qStack[ nEntry ].m_pBuffer;
						DWORD nCopyEntry = nEntry;
						if ( nCopyEntry < m_nStackSize - 1 ) do
						{
							m_qStack[ nCopyEntry ] = m_qStack[ nCopyEntry + 1 ];
						}
						while ( ++nCopyEntry < m_nStackSize - 1 );
						if ( nEntry < m_nStackSize - 1 )		// the last entry never holds a valid pointer
						{
							m_qStack[ m_nStackSize - 2 ].m_pBuffer = pBuffer;
							m_qStack[ m_nStackSize - 2 ].m_pBufferIndex = pBuffer;
						}
						--m_nStackTop;
					}
					else
					{
						++nEntry;
					}
				}
				m_bListChanged = false;
				continue;
			}

			if ( ( m_pFiles.IsEmpty() ) && ( m_nStackTop == 0 ) )
			{
				m_pSection.Unlock();
				break;
			}
			
			if ( ( !m_pFiles.IsEmpty() ) && ( m_nStackTop < m_nStackSize ) )
			{
				m_qStackN.m_nIndex = (DWORD)m_pFiles.RemoveHead();
				m_pSection.Unlock();
				if ( m_qStackN.m_nIndex == 0 )
				{
					m_qStackN = m_qStack[ --m_nStackTop ];
					m_sPath = m_qStackN.m_sPath;
					HashFile(true);
					continue;
				}
				if ( CLibraryFile* pFile = Library.LookupFile( m_qStackN.m_nIndex, TRUE ) )
				{
					m_qStackN.m_sPath = pFile->GetPath();
					Library.Unlock();
					m_qStackN.hFile = CreateFile( m_qStackN.m_sPath, GENERIC_READ,
						FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN, NULL );
					if ( m_qStackN.hFile == INVALID_HANDLE_VALUE )
					{
						m_pSection.Lock();
						if ( m_pFiles.Find( NULL ) == NULL ) m_pFiles.AddTail( (LPVOID)0 );
						m_pFiles.AddTail( (LPVOID)m_nIndex );
						m_pSection.Unlock();
						continue;
					}
					theApp.Message( MSG_DEBUG, _T("Hashing: %s"), (LPCTSTR)m_qStackN.m_sPath);
					m_qStackN.nSizeHigh = 0;
					m_qStackN.nSizeLow = GetFileSize( m_qStackN.hFile, &m_qStackN.nSizeHigh );
					m_qStackN.nFileSize = (QWORD)m_qStackN.nSizeLow | ( (QWORD)m_qStackN.nSizeHigh << 32 );
					m_qStackN.nFileBase = 0;
					m_qStackN.bVirtual = Settings.Library.VirtualFiles ? DetectVirtualFile( m_qStackN.hFile, m_qStackN.nFileBase, m_qStackN.nFileSize ) : false;
					m_qStackN.nSizeLow = (DWORD)( m_qStackN.nFileBase & 0xFFFFFFFF );
					m_qStackN.nSizeHigh = (DWORD)( m_qStackN.nFileBase >> 32 );
					SetFilePointer( m_qStackN.hFile, m_qStackN.nSizeLow, (PLONG)&m_qStackN.nSizeHigh, FILE_BEGIN );
					m_qStackN.nStoredBytes = 0;
					m_qStackN.nCount = m_qStackN.nFileSize;
					m_qStackN.pTiger = new CTigerTree;
					m_qStackN.pTiger->BeginFile( Settings.Library.TigerHeight, m_qStackN.nFileSize );
					m_qStackN.pED2K = new CED2K;
					m_qStackN.pED2K->BeginFile( m_qStackN.nFileSize );
					m_qStackN.pMD5 = new CMD5;
					m_qStackN.pSHA1 = new CSHA1;
					if ( m_qStackN.nFileSize == 0 )
					{
						m_qStackO = m_qStackN;	// ToDo: do something witty to account for the TigerTree bug
						m_bRetired = TRUE;
					}
					else
					{
						m_sPath = m_qStackN.m_sPath;
						HashFile( FALSE );
					}
				}
				else
				{
					m_nIndex = 0;
					continue;
				}
			}
			else
			{
				m_pSection.Unlock();
				m_qStackN = m_qStack[ --m_nStackTop ];
				theApp.Message( MSG_DEBUG, _T("Continue Hashing: %s"), (LPCTSTR)m_qStackN.m_sPath);
				m_sPath = m_qStackN.m_sPath;
				HashFile( TRUE );
			}
			if ( m_bRetired )
			{
				m_qStackO.pSHA1->Finish();
				m_qStackO.pMD5->Finish();
				m_qStackO.pTiger->FinishFile();
				m_qStackO.pED2K->FinishFile();
				CLibraryFile* pFile = Library.LookupFile( m_qStackO.m_nIndex, TRUE );
				if ( pFile != NULL )
				{
					Library.RemoveFile( pFile );
					pFile->m_bBogus			= FALSE;
					pFile->m_nVirtualBase	= m_qStackO.bVirtual ? m_qStackO.nFileBase : 0;
					pFile->m_nVirtualSize	= m_qStackO.bVirtual ? m_qStackO.nFileSize : 0;
					pFile->m_oSHA1 = *m_qStackO.pSHA1;
					pFile->m_oMD5 = *m_qStackO.pMD5;
					pFile->m_oTiger = *m_qStackO.pTiger;
					pFile->m_oED2K = *m_qStackO.pED2K;
					LibraryMaps.CullDeletedFiles( pFile );
					Library.AddFile( pFile );
					Library.Unlock( TRUE );
					LibraryHashDB.StoreTiger( m_nIndex, m_qStackO.pTiger );
					LibraryHashDB.StoreED2K( m_nIndex, m_qStackO.pED2K );
					theApp.Message( MSG_DEBUG, _T("Finished Hashing: %s"), (LPCTSTR)m_qStackO.m_sPath);
					SetFilePointer( m_qStackO.hFile, 0, NULL, FILE_BEGIN );
					m_tActive = GetTickCount();
					if ( m_pPlugins->ExtractMetadata( m_qStackO.m_sPath, m_qStackO.hFile ) )
					{
						// Plugin got it
					}
					else if ( m_pInternals->ExtractMetadata( m_qStackO.m_sPath, m_qStackO.hFile, *m_qStackO.pSHA1 ) )
					{
						// Internal got it
					}
				}
				theApp.Message( MSG_DEBUG, _T("Finished Hashing: %s"), (LPCTSTR)m_qStackO.m_sPath);
				CloseHandle( m_qStackO.hFile );
				delete m_qStackO.pSHA1;
				delete m_qStackO.pMD5;
				delete m_qStackO.pTiger;
				delete m_qStackO.pED2K;
				m_bRetired = FALSE;
			}
		}
		else Sleep ( 100 );
	}

	for ( nEntry = 0; nEntry < m_nStackTop; ++nEntry ) CloseHandle( m_qStack[ nEntry ].hFile );
	free ( m_pBuffer );
	m_pBuffer = NULL;
	m_pPlugins->Cleanup();
	m_nIndex	= 0;
	m_tActive	= 0;
	m_bThread	= FALSE;
	m_sPath.Empty();
	theApp.Message( MSG_DEBUG, _T("CLibraryBuilder shutting down.") );
}

void CLibraryBuilder::HashFile(BOOL bForceHash)
{
	DWORD nBytesToRead = LIBRARY_BUILDER_BLOCK_SIZE;
	m_bRetired = FALSE;
	DWORD nTime=GetTickCount(), nTime2, nTime3, nEntry, nInsertEntry; 
	if ( bForceHash )
	{
		while ( m_qStackN.nStoredBytes >= LIBRARY_BUILDER_BLOCK_SIZE )
		{
			(this->*pDoHash[ m_nStackTop ])();
			m_qStackN.nStoredBytes -= LIBRARY_BUILDER_BLOCK_SIZE;
			m_qStackN.m_pBufferIndex += LIBRARY_BUILDER_BLOCK_SIZE;
			if ( m_bTerminate || m_bSuspend || m_bListChanged )
			{
				m_qStack[ m_nStackTop++ ] = m_qStackN;
				return;
			}
		}
		if ( m_qStackN.nStoredBytes == m_qStackN.nCount )
		{
			if ( m_qStackN.nCount != 0 )
			{
				m_qStackN.pSHA1->Add( m_qStackN.m_pBufferIndex, (DWORD)m_qStackN.nCount );
				m_qStackN.pED2K->AddToFile( m_qStackN.m_pBufferIndex, (DWORD)m_qStackN.nCount );
				m_qStackN.pMD5->Add( m_qStackN.m_pBufferIndex, (DWORD)m_qStackN.nCount );
				m_qStackN.nCount = 0;
			}
			m_qStackO = m_qStackN;
			m_bRetired = TRUE;
			return;
		}
	}
	m_qStackN.m_pBufferIndex = m_pBuffer;
	if ( ( m_nStackSize == 1 ) || ( ( bForceHash ) && ( m_nStackTop == 0 ) ) )// special handling to speed up bufferless hashing
	{
		while ( m_qStackN.nCount >= LIBRARY_BUILDER_BLOCK_SIZE )
		{
			ReadFile( m_qStackN.hFile, m_qStackN.m_pBufferIndex, LIBRARY_BUILDER_BLOCK_SIZE, &nBytesToRead, NULL );
			m_qStackN.pTiger->AddToFile( m_qStackN.m_pBufferIndex, LIBRARY_BUILDER_BLOCK_SIZE );
			(this->*pDoHash[0])();
			if ( m_bTerminate || m_bSuspend || m_bListChanged)
			{
				m_qStack[m_nStackTop++] = m_qStackN;
				return;
			}
			if ( ! m_bPriority ) 
			{
				nTime2 = nTime + ( 1000 * LIBRARY_BUILDER_BLOCK_SIZE ) / ( 1024*1024*Settings.Library.LowPriorityHashing);
				if ( nTime2 <= ( nTime3 = GetTickCount() ) ) nTime2 = 1; else nTime2 = nTime2 - nTime3;
				Sleep ( nTime2 );
				nTime=GetTickCount();
			}
		}
	}
	else if ( m_nStackTop == m_nStackSize - 1 )					// parallel hashing
	{
		while ( ( m_qStack[m_nStackTop-1].nStoredBytes >= LIBRARY_BUILDER_BLOCK_SIZE )
			&& ( m_qStackN.nCount >= LIBRARY_BUILDER_BLOCK_SIZE ) )
		{
			ReadFile( m_qStackN.hFile, m_qStackN.m_pBufferIndex, LIBRARY_BUILDER_BLOCK_SIZE, &nBytesToRead, NULL );
			m_qStackN.pTiger->AddToFile( m_qStackN.m_pBufferIndex, LIBRARY_BUILDER_BLOCK_SIZE );
			(this->*pDoHash[m_nStackTop])();
			if ( m_bTerminate || m_bSuspend || m_bListChanged)
			{
				m_qStack[m_nStackTop++] = m_qStackN;
				return;
			}
			if ( ! m_bPriority ) 
			{
				nTime2 = nTime + ( 1000 * LIBRARY_BUILDER_BLOCK_SIZE ) / ( 1024*1024*Settings.Library.LowPriorityHashing);
				if ( nTime2 <= ( nTime3 = GetTickCount() ) ) nTime2 = 1; else nTime2 = nTime2 - nTime3;
				Sleep ( nTime2 );
				nTime=GetTickCount();
			}
		}
	}
	if ( m_qStackN.nCount < LIBRARY_BUILDER_BLOCK_SIZE )	// find out why we stopped
	{
		if ( m_qStackN.nCount != 0 )
		{
			ReadFile( m_qStackN.hFile, m_qStackN.m_pBufferIndex, (DWORD)m_qStackN.nCount, &nBytesToRead, NULL );
			m_qStackN.pTiger->AddToFile( m_qStackN.m_pBufferIndex, (DWORD)m_qStackN.nCount );
			m_qStackN.pSHA1->Add( m_qStackN.m_pBufferIndex, (DWORD)m_qStackN.nCount );
			m_qStackN.pED2K->AddToFile( m_qStackN.m_pBufferIndex, (DWORD)m_qStackN.nCount );
			m_qStackN.pMD5->Add( m_qStackN.m_pBufferIndex, (DWORD)m_qStackN.nCount );
			if ( ! m_bPriority ) 
			{
				nTime2 = nTime + ( 1000 * nBytesToRead ) / ( 1024*1024*Settings.Library.LowPriorityHashing);
				if ( nTime2 <= ( nTime3 = GetTickCount() ) ) nTime2 = 1; else nTime2 = nTime2 - nTime3;
				Sleep ( nTime2 );
				nTime=GetTickCount();
			}
		}
		m_qStackO = m_qStackN;
		m_bRetired = TRUE;
		return;
	}
	if ( ( m_nStackTop > 0 ) && ( ( m_qStack[m_nStackTop-1].nCount == 0 ) || 
		( ( m_qStack[m_nStackTop-1].nStoredBytes > 0 ) && ( m_qStack[m_nStackTop-1].nCount < LIBRARY_BUILDER_BLOCK_SIZE ) ) ) )
	{
		m_qStackO = m_qStack[--m_nStackTop];
		if ( m_qStackO.nCount < LIBRARY_BUILDER_BLOCK_SIZE )
		{
			m_qStackO.pSHA1->Add( m_qStackO.m_pBufferIndex, (DWORD)m_qStackO.nCount );
			m_qStackO.pED2K->AddToFile( m_qStackO.m_pBufferIndex, (DWORD)m_qStackO.nCount );
			m_qStackO.pMD5->Add( m_qStackO.m_pBufferIndex, (DWORD)m_qStackO.nCount );
		}
		m_bRetired = TRUE;
	}
	DWORD nBlockCount = 0;												// fill the buffer
	m_qStackN.m_pBuffer = m_qStack[m_nStackSize-2].m_pBuffer;  // we do know this isn't used
	m_qStackN.m_pBufferIndex = m_qStackN.m_pBuffer;
	m_qStack[m_nStackSize-2].m_pBuffer = NULL;
	while ( ( nBlockCount < m_nMaxBlock ) && ( m_qStackN.nCount >= ( ( nBlockCount + 1 ) * LIBRARY_BUILDER_BLOCK_SIZE ) ) )
	{
		ReadFile( m_qStackN.hFile, m_qStackN.m_pBufferIndex, LIBRARY_BUILDER_BLOCK_SIZE, &nBytesToRead, NULL );
		m_qStackN.pTiger->AddToFile( m_qStackN.m_pBufferIndex, LIBRARY_BUILDER_BLOCK_SIZE );
		++nBlockCount;
		m_qStackN.m_pBufferIndex += LIBRARY_BUILDER_BLOCK_SIZE;
		if ( m_bTerminate || m_bSuspend || m_bListChanged)
		{
			m_qStackN.nStoredBytes = m_qStackN.m_pBufferIndex - m_qStackN.m_pBuffer;
			nInsertEntry = 0;
			if ( m_nStackTop )
			{
				while ( m_qStackN.nStoredBytes < m_qStack[ nInsertEntry ].nStoredBytes && ++nInsertEntry < m_nStackTop );
			}
			nEntry = MAX_PARALLEL;
			while ( --nEntry > nInsertEntry ) m_qStack[ nEntry ] = m_qStack[ nEntry - 1 ];
			++m_nStackTop;
			m_qStack[ nEntry ] = m_qStackN;
			return;
		}
		if ( ! m_bPriority ) 
		{
			nTime2 = nTime + ( 1000 * LIBRARY_BUILDER_BLOCK_SIZE ) / ( 1024*1024*Settings.Library.LowPriorityHashing);
			if ( nTime2 <= ( nTime3 = GetTickCount() ) ) nTime2 = 1; else nTime2 = nTime2 - nTime3;
			Sleep ( nTime2 );
			nTime=GetTickCount();
		}
	}
	if ( ( nBlockCount < m_nMaxBlock ) && ( ( nBlockCount * LIBRARY_BUILDER_BLOCK_SIZE ) < m_qStackN.nCount) )
	{
		ReadFile( m_qStackN.hFile, m_qStackN.m_pBufferIndex,
			(DWORD)m_qStackN.nCount - ( nBlockCount * LIBRARY_BUILDER_BLOCK_SIZE ), &nBytesToRead, NULL );
		m_qStackN.pTiger->AddToFile( m_qStackN.m_pBufferIndex, nBytesToRead );
		if ( ! m_bPriority ) 
		{
			nTime2 = nTime + ( 1000 * nBytesToRead ) / ( 1024*1024*Settings.Library.LowPriorityHashing);
			if ( nTime2 <= ( nTime3 = GetTickCount() ) ) nTime2 = 1; else nTime2 = nTime2 - nTime3;
			Sleep ( nTime2 );
			nTime=GetTickCount();
		}
	}
	m_qStackN.m_pBufferIndex = m_qStackN.m_pBuffer;
	m_qStackN.nStoredBytes =
		( nBlockCount == m_nMaxBlock ) ? nBlockCount * LIBRARY_BUILDER_BLOCK_SIZE : (DWORD)m_qStackN.nCount;
	nInsertEntry = 0;
	if ( m_nStackTop )
	{
		while ( m_qStackN.nStoredBytes < m_qStack[ nInsertEntry ].nStoredBytes && ++nInsertEntry < m_nStackTop );
	}
	nEntry = MAX_PARALLEL;
	while ( --nEntry > nInsertEntry ) m_qStack[ nEntry ] = m_qStack[ nEntry - 1 ];
	++m_nStackTop;
	m_qStack[ nEntry ] = m_qStackN;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilder metadata submission (threaded)

BOOL CLibraryBuilder::SubmitMetadata(LPCTSTR pszSchemaURI, CXMLElement* pXML)
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
	
	if ( CLibraryFile* pFile = Library.LookupFile( m_nIndex, TRUE ) )
	{
		if ( pFile->m_pMetadata == NULL )
		{
			Library.RemoveFile( pFile );
			
			pFile->m_pSchema		= pSchema;
			pFile->m_pMetadata		= pXML;
			pFile->m_bMetadataAuto	= TRUE;
			
			Library.AddFile( pFile );
			Library.Unlock( TRUE );
			
			return TRUE;
		}
		
		Library.Unlock();
	}
	
	delete pXML;
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilder bogus/corrupted state submission (threaded)

BOOL CLibraryBuilder::SubmitCorrupted()
{
	if ( CLibraryFile* pFile = Library.LookupFile( m_nIndex, TRUE ) )
	{
		pFile->m_bBogus = TRUE;
		Library.Unlock( TRUE );
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
	
	DWORD nTagSize = _byteswap_ulong( pHeader.nSize );
	ID3_DESYNC_SIZE( nTagSize );
	
	if ( pHeader.nFlags & ID3V2_FOOTERPRESENT ) nTagSize += 10;
	nTagSize += sizeof(pHeader);
	
	if ( nLength <= nTagSize ) return FALSE;
	
	nOffset += nTagSize;
	nLength -= nTagSize;
	
	return TRUE;
}

DWORD CLibraryBuilder::m_nStackSize = MAX_PARALLEL;
DWORD CLibraryBuilder::m_nMaxBlock = 10;
void CLibraryBuilder::DoHash1()
{
	CMD5::pAdd1(m_qStackN.pMD5, m_qStackN.m_pBufferIndex);
	CSHA1::pAdd1(m_qStackN.pSHA1, m_qStackN.m_pBufferIndex);
	m_qStackN.pED2K->Add1( m_qStackN.m_pBufferIndex );
	m_qStackN.nCount -= LIBRARY_BUILDER_BLOCK_SIZE;
}
void CLibraryBuilder::DoHash2()
{
	CMD5::pAdd2(m_qStackN.pMD5, m_qStackN.m_pBufferIndex,
		m_qStack[0].pMD5, m_qStack[0].m_pBufferIndex);
	CSHA1::pAdd2(m_qStackN.pSHA1, m_qStackN.m_pBufferIndex,
		m_qStack[0].pSHA1, m_qStack[0].m_pBufferIndex);
	m_qStackN.pED2K->Add2( m_qStackN.m_pBufferIndex,
		m_qStack[0].pED2K, m_qStack[0].m_pBufferIndex);
	m_qStackN.nCount -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[0].nCount -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[0].nStoredBytes -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[0].m_pBufferIndex += LIBRARY_BUILDER_BLOCK_SIZE;
}
void CLibraryBuilder::DoHash3()
{
	CMD5::pAdd3(m_qStackN.pMD5, m_qStackN.m_pBufferIndex,
		m_qStack[0].pMD5, m_qStack[0].m_pBufferIndex,
		m_qStack[1].pMD5, m_qStack[1].m_pBufferIndex);
	CSHA1::pAdd3(m_qStackN.pSHA1, m_qStackN.m_pBufferIndex,
		m_qStack[0].pSHA1, m_qStack[0].m_pBufferIndex,
		m_qStack[1].pSHA1, m_qStack[1].m_pBufferIndex);
	m_qStackN.pED2K->Add3( m_qStackN.m_pBufferIndex,
		m_qStack[0].pED2K, m_qStack[0].m_pBufferIndex,
		m_qStack[1].pED2K, m_qStack[1].m_pBufferIndex);
	m_qStackN.nCount -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[0].nCount -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[1].nCount -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[0].nStoredBytes -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[1].nStoredBytes -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[0].m_pBufferIndex += LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[1].m_pBufferIndex += LIBRARY_BUILDER_BLOCK_SIZE;
}
void CLibraryBuilder::DoHash4()
{
	CMD5::pAdd4(m_qStackN.pMD5, m_qStackN.m_pBufferIndex,
		m_qStack[0].pMD5, m_qStack[0].m_pBufferIndex,
		m_qStack[1].pMD5, m_qStack[1].m_pBufferIndex,
		m_qStack[2].pMD5, m_qStack[2].m_pBufferIndex);
	CSHA1::pAdd4(m_qStackN.pSHA1, m_qStackN.m_pBufferIndex,
		m_qStack[0].pSHA1, m_qStack[0].m_pBufferIndex,
		m_qStack[1].pSHA1, m_qStack[1].m_pBufferIndex,
		m_qStack[2].pSHA1, m_qStack[2].m_pBufferIndex);
	m_qStackN.pED2K->Add4( m_qStackN.m_pBufferIndex,
		m_qStack[0].pED2K, m_qStack[0].m_pBufferIndex,
		m_qStack[1].pED2K, m_qStack[1].m_pBufferIndex,
		m_qStack[2].pED2K, m_qStack[2].m_pBufferIndex);
	m_qStackN.nCount -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[0].nCount -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[1].nCount -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[2].nCount -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[0].nStoredBytes -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[1].nStoredBytes -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[2].nStoredBytes -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[0].m_pBufferIndex += LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[1].m_pBufferIndex += LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[2].m_pBufferIndex += LIBRARY_BUILDER_BLOCK_SIZE;
}
void CLibraryBuilder::DoHash5()
{
	CMD5::pAdd5(m_qStackN.pMD5, m_qStackN.m_pBufferIndex,
		m_qStack[0].pMD5, m_qStack[0].m_pBufferIndex,
		m_qStack[1].pMD5, m_qStack[1].m_pBufferIndex,
		m_qStack[2].pMD5, m_qStack[2].m_pBufferIndex,
		m_qStack[3].pMD5, m_qStack[3].m_pBufferIndex);
	CSHA1::pAdd5(m_qStackN.pSHA1, m_qStackN.m_pBufferIndex,
		m_qStack[0].pSHA1, m_qStack[0].m_pBufferIndex,
		m_qStack[1].pSHA1, m_qStack[1].m_pBufferIndex,
		m_qStack[2].pSHA1, m_qStack[2].m_pBufferIndex,
		m_qStack[3].pSHA1, m_qStack[3].m_pBufferIndex);
	m_qStackN.pED2K->Add5( m_qStackN.m_pBufferIndex,
		m_qStack[0].pED2K, m_qStack[0].m_pBufferIndex,
		m_qStack[1].pED2K, m_qStack[1].m_pBufferIndex,
		m_qStack[2].pED2K, m_qStack[2].m_pBufferIndex,
		m_qStack[3].pED2K, m_qStack[3].m_pBufferIndex);
	m_qStackN.nCount -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[0].nCount -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[1].nCount -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[2].nCount -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[3].nCount -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[0].nStoredBytes -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[1].nStoredBytes -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[2].nStoredBytes -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[3].nStoredBytes -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[0].m_pBufferIndex += LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[1].m_pBufferIndex += LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[2].m_pBufferIndex += LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[3].m_pBufferIndex += LIBRARY_BUILDER_BLOCK_SIZE;
}
void CLibraryBuilder::DoHash6()
{
	CMD5::pAdd6(m_qStackN.pMD5, m_qStackN.m_pBufferIndex,
		m_qStack[0].pMD5, m_qStack[0].m_pBufferIndex,
		m_qStack[1].pMD5, m_qStack[1].m_pBufferIndex,
		m_qStack[2].pMD5, m_qStack[2].m_pBufferIndex,
		m_qStack[3].pMD5, m_qStack[3].m_pBufferIndex,
		m_qStack[4].pMD5, m_qStack[4].m_pBufferIndex);
	CSHA1::pAdd6(m_qStackN.pSHA1, m_qStackN.m_pBufferIndex,
		m_qStack[0].pSHA1, m_qStack[0].m_pBufferIndex,
		m_qStack[1].pSHA1, m_qStack[1].m_pBufferIndex,
		m_qStack[2].pSHA1, m_qStack[2].m_pBufferIndex,
		m_qStack[3].pSHA1, m_qStack[3].m_pBufferIndex,
		m_qStack[4].pSHA1, m_qStack[4].m_pBufferIndex);
	m_qStackN.pED2K->Add6( m_qStackN.m_pBufferIndex,
		m_qStack[0].pED2K, m_qStack[0].m_pBufferIndex,
		m_qStack[1].pED2K, m_qStack[1].m_pBufferIndex,
		m_qStack[2].pED2K, m_qStack[2].m_pBufferIndex,
		m_qStack[3].pED2K, m_qStack[3].m_pBufferIndex,
		m_qStack[4].pED2K, m_qStack[4].m_pBufferIndex);
	m_qStackN.nCount -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[0].nCount -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[1].nCount -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[2].nCount -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[3].nCount -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[4].nCount -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[0].nStoredBytes -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[1].nStoredBytes -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[2].nStoredBytes -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[3].nStoredBytes -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[4].nStoredBytes -= LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[0].m_pBufferIndex += LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[1].m_pBufferIndex += LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[2].m_pBufferIndex += LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[3].m_pBufferIndex += LIBRARY_BUILDER_BLOCK_SIZE;
	m_qStack[4].m_pBufferIndex += LIBRARY_BUILDER_BLOCK_SIZE;
}

const CLibraryBuilder::tpDoHash CLibraryBuilder::pDoHash[MAX_PARALLEL] = {
	&CLibraryBuilder::DoHash1, &CLibraryBuilder::DoHash2, &CLibraryBuilder::DoHash3,
	&CLibraryBuilder::DoHash4, &CLibraryBuilder::DoHash5, &CLibraryBuilder::DoHash6 };
void CLibraryBuilder::Init()
{
	m_nStackSize = Settings.Library.Parallel;
	if ( m_nStackSize > MAX_PARALLEL )
	{
		m_nStackSize = MAX_PARALLEL;
	}
	else if ( m_nStackSize == 0 )
	{
		m_nStackSize = 1;
		if ( SupportsMMX() ) m_nStackSize = 4;
		if ( SupportsSSE2() ) m_nStackSize = 6;
	}
	if ( Settings.Library.BufferSize == 0 )
	{
		MEMORYSTATUS stat;
		GlobalMemoryStatus (&stat);
		m_nMaxBlock = ( m_nStackSize == 1 ) ? 1 : ( stat.dwTotalPhys / ( LIBRARY_BUILDER_BLOCK_SIZE * ( m_nStackSize - 1 ) * 4 ) );
	}
	else m_nMaxBlock = ( m_nStackSize == 1 ) ? 1 : ( ( Settings.Library.BufferSize*1024*1024 ) / ( LIBRARY_BUILDER_BLOCK_SIZE * ( m_nStackSize -1 ) ) );
	if ( m_nMaxBlock == 0 ) m_nMaxBlock = 1;
}