//
// Library.cpp
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
#include "Library.h"
#include "LibraryMaps.h"
#include "LibraryFolders.h"
#include "LibraryDictionary.h"
#include "LibraryBuilder.h"
#include "LibraryHistory.h"
#include "HashDatabase.h"
#include "SharedFolder.h"
#include "SharedFile.h"
#include "AlbumFolder.h"

#include "QuerySearch.h"
#include "Application.h"

#include "XML.h"
#include "Schema.h"
#include "SchemaCache.h"

#include "SHA.h"
#include "ED2K.h"
#include "TigerTree.h"

IMPLEMENT_DYNAMIC(CLibrary, CComObject)

BEGIN_INTERFACE_MAP(CLibrary, CComObject)
	INTERFACE_PART(CLibrary, IID_ILibrary, Library)
END_INTERFACE_MAP()

CLibrary Library;


//////////////////////////////////////////////////////////////////////
// CLibrary construction

CLibrary::CLibrary()
{
	EnableDispatch( IID_ILibrary );
	
	m_nUpdateSaved	= 0;
	m_nScanCount	= 0;
	
	m_hThread		= NULL;
	m_bThread		= TRUE;
	m_nScanCookie	= 1;
	m_nUpdateCookie	= 0;
	m_nUpdateSaved	= 0;
	m_nFileSwitch	= 0;
	m_nInhibit		= 0;
	
	m_pfnGFAEW		= NULL;
	m_pfnGFAEA		= NULL;
	
	if ( m_hKernel = LoadLibrary( _T("kernel32") ) )
	{
		(FARPROC&)m_pfnGFAEW = GetProcAddress( m_hKernel, "GetFileAttributesExW" );
		(FARPROC&)m_pfnGFAEA = GetProcAddress( m_hKernel, "GetFileAttributesExA" );
	}
}

CLibrary::~CLibrary()
{
	if ( m_hKernel != NULL ) FreeLibrary( m_hKernel );
}

//////////////////////////////////////////////////////////////////////
// CLibrary locking

BOOL CLibrary::Lock(DWORD nTimeout)
{
	return m_pSection.Lock( nTimeout );
}

void CLibrary::Unlock(BOOL bSetUpdated)
{
	if ( bSetUpdated ) m_nUpdateCookie = GetTickCount();
	m_pSection.Unlock();
}

void CLibrary::Inhibit(BOOL bInhibit)
{
	if ( bInhibit )
		InterlockedIncrement( (PLONG)&m_nInhibit );
	else
		InterlockedDecrement( (PLONG)&m_nInhibit );
}

//////////////////////////////////////////////////////////////////////
// CLibrary file and folder operations

CLibraryFile* CLibrary::LookupFile(DWORD nIndex, BOOL bLockOnSuccess, BOOL bSharedOnly, BOOL bAvailableOnly)
{
	return LibraryMaps.LookupFile( nIndex, bLockOnSuccess, bSharedOnly, bAvailableOnly );
}

CAlbumFolder* CLibrary::GetAlbumRoot()
{
	return LibraryFolders.GetAlbumRoot();
}

void CLibrary::AddFile(CLibraryFile* pFile)
{
	LibraryMaps.OnFileAdd( pFile );
	
	if ( pFile->m_bSHA1 )
	{
		LibraryDictionary.Add( pFile );
	}
	
	if ( pFile->IsAvailable() )
	{
		if ( pFile->m_bSHA1 || pFile->m_bTiger || pFile->m_bMD5 || pFile->m_bED2K )
		{
			LibraryHistory.Submit( pFile );
			GetAlbumRoot()->OrganiseFile( pFile );
		}
		
		if ( ! pFile->m_bSHA1 || ! pFile->m_bTiger || ! pFile->m_bMD5 || ! pFile->m_bED2K )
		{
			LibraryBuilder.Add( pFile );
		}
	}
}

void CLibrary::RemoveFile(CLibraryFile* pFile)
{
	LibraryMaps.OnFileRemove( pFile );
	
	if ( pFile->m_nIndex )
	{
		LibraryBuilder.Remove( pFile );
		LibraryDictionary.Remove( pFile );
	}
}

void CLibrary::OnFileDelete(CLibraryFile* pFile)
{
	LibraryFolders.OnFileDelete( pFile );
	LibraryHistory.OnFileDelete( pFile );
	LibraryHashDB.DeleteAll( pFile->m_nIndex );
}

//////////////////////////////////////////////////////////////////////
// CLibrary search

CPtrList* CLibrary::Search(CQuerySearch* pSearch, int nMaximum, BOOL bLocal)
{
	if ( ! m_pSection.Lock( 50 ) ) return NULL;
	
	CPtrList* pHits = LibraryMaps.Search( pSearch, nMaximum, bLocal );
	
	if ( pHits == NULL && pSearch != NULL )
	{
		pHits = LibraryDictionary.Search( pSearch, nMaximum, bLocal );
	}
	
	if ( pHits == NULL ) m_pSection.Unlock();
	
	return pHits;
}

//////////////////////////////////////////////////////////////////////
// CLibrary clear

void CLibrary::Clear()
{
	StopThread();
	
	CSingleLock pLock( &m_pSection, TRUE );
	
	LibraryHistory.Clear();
	LibraryDictionary.Clear();
	LibraryFolders.Clear();
	LibraryMaps.Clear();
	
	m_nUpdateCookie++;
}

//////////////////////////////////////////////////////////////////////
// CLibrary load from disk

BOOL CLibrary::Load()
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	GetAlbumRoot();
	
	FILETIME pFileTime1 = { 0, 0 }, pFileTime2 = { 0, 0 };
	CFile pFile1, pFile2;
	BOOL bFile1, bFile2;
	CString strFile;
	
	strFile = Settings.General.UserPath + _T("\\Data\\Library");
	
	bFile1 = pFile1.Open( strFile + _T("1.dat"), CFile::modeRead );
	bFile2 = pFile2.Open( strFile + _T("2.dat"), CFile::modeRead );
	
	if ( bFile1 || bFile2 )
	{
		if ( bFile1 ) bFile1 = pFile1.Read( &pFileTime1, sizeof(FILETIME) ) == sizeof(FILETIME);
		if ( bFile2 ) bFile2 = pFile2.Read( &pFileTime2, sizeof(FILETIME) ) == sizeof(FILETIME);
	}
	else
	{
		bFile1 = pFile1.Open( strFile + _T(".dat"), CFile::modeRead );
		pFileTime1.dwHighDateTime++;
	}
	
	if ( bFile1 || bFile2 )
	{
		CFile* pNewest	= ( CompareFileTime( &pFileTime1, &pFileTime2 ) >= 0 )
						? &pFile1 : &pFile2;
		
		try
		{
			CArchive ar( pNewest, CArchive::load, 40960 );
			Serialize( ar );
			ar.Close();
		}
		catch ( CException* pException )
		{
			pException->Delete();
			Clear();
			
			if ( pNewest == &pFile1 && bFile2 )
				pNewest = &pFile2;
			else if ( pNewest == &pFile2 && bFile1 )
				pNewest = &pFile1;
			else
				pNewest = NULL;
			
			if ( pNewest != NULL )
			{
				try
				{
					CArchive ar( pNewest, CArchive::load, 40960 );
					Serialize( ar );
					ar.Close();
				}
				catch ( CException* pException )
				{
					pException->Delete();
				}
			}
		}
		
		pNewest->Close();
	}
	else
	{
		CreateDirectory( Settings.Downloads.CompletePath, NULL );
		LibraryFolders.AddFolder( Settings.Downloads.CompletePath );

		CreateDirectory( Settings.Downloads.CollectionPath, NULL );
		LibraryFolders.AddFolder( Settings.Downloads.CollectionPath );

		//CreateDirectory( Settings.Downloads.TorrentPath, NULL );
		//LibraryFolders.AddFolder( Settings.Downloads.TorrentPath, FALSE );
	}
	
	LibraryFolders.CreateAlbumTree();
	LibraryHashDB.Create();
	StartThread();

	LibraryBuilder.BoostPriority( Settings.Library.HighPriorityHash );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLibrary save to disk

void CLibrary::Save()
{
	CSingleLock pLock( &m_pSection, TRUE );
	
	FILETIME pFileTime = { 0, 0 };
	SYSTEMTIME pSystemTime;
	CString strFile;
	CFile pFile;
	
	strFile.Format( _T("%s\\Data\\Library%i.dat"),
		(LPCTSTR)Settings.General.UserPath, m_nFileSwitch + 1 );
	
	m_nFileSwitch = ( m_nFileSwitch == 0 ) ? 1 : 0;
	
	if ( ! pFile.Open( strFile, CFile::modeWrite|CFile::modeCreate ) ) return;
	
	pFile.Write( &pFileTime, sizeof(FILETIME) );
	
	CArchive ar( &pFile, CArchive::store, 40960 );
	Serialize( ar );
	ar.Close();
	pFile.Flush();
	
	GetSystemTime( &pSystemTime );
	SystemTimeToFileTime( &pSystemTime, &pFileTime );
	pFile.Seek( 0, 0 );
	pFile.Write( &pFileTime, sizeof(FILETIME) );
	pFile.Close();
}

//////////////////////////////////////////////////////////////////////
// CLibrary serialize

#define LIBRARY_SER_VERSION	23

void CLibrary::Serialize(CArchive& ar)
{
	int nVersion = LIBRARY_SER_VERSION;
	
	if ( ar.IsStoring() )
	{
		ar << nVersion;
	}
	else
	{
		Clear();
		ar >> nVersion;
		if ( nVersion < 1 || nVersion > LIBRARY_SER_VERSION ) AfxThrowUserException();
	}
	
	LibraryMaps.Serialize1( ar, nVersion );
	LibraryFolders.Serialize( ar, nVersion );
	LibraryHistory.Serialize( ar, nVersion );
	LibraryMaps.Serialize2( ar, nVersion );
}

//////////////////////////////////////////////////////////////////////
// CLibrary thread control

void CLibrary::StartThread()
{
	if ( m_hThread == NULL )
	{
		m_bThread = TRUE;
		CWinThread* pThread = AfxBeginThread( ThreadStart, this, THREAD_PRIORITY_BELOW_NORMAL );
		m_hThread = pThread->m_hThread;
	}
	
	LibraryBuilder.StartThread();
}

void CLibrary::StopThread()
{
	LibraryBuilder.StopThread();
	
	if ( m_hThread != NULL )
	{
		m_bThread = FALSE;
		m_pWakeup.SetEvent();
		
		for ( int nAttempt = 10 ; nAttempt > 0 ; nAttempt-- )
		{
			DWORD nCode;
			if ( ! GetExitCodeThread( m_hThread, &nCode ) ) break;
			if ( nCode != STILL_ACTIVE ) break;
			Sleep( 200 );
		}
		
		if ( nAttempt == 0 )
		{
			TerminateThread( m_hThread, 0 );
			theApp.Message( MSG_DEBUG, _T("WARNING: Terminating CLibrary thread.") );
			Sleep( 100 );
		}
		
		m_hThread = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// CLibrary thread run

UINT CLibrary::ThreadStart(LPVOID pParam)
{
	CLibrary* pLibrary = (CLibrary*)pParam;
	pLibrary->OnRun();
	return 0;
}

void CLibrary::OnRun()
{
	while ( m_bThread )
	{
		if ( m_nInhibit == 0 ) ThreadScan();
		WaitForSingleObject( m_pWakeup, 500 );
	}
}

//////////////////////////////////////////////////////////////////////
// CLibrary threaded scan

BOOL CLibrary::ThreadScan()
{
	BOOL bChanged = LibraryFolders.ThreadScan( &m_bThread, FALSE );
	
	CSingleLock pLock( &m_pSection, TRUE );
	
	m_nScanCount++;
	if ( bChanged ) m_nUpdateCookie = GetTickCount();
	
	if ( m_nUpdateCookie - m_nUpdateSaved > 5000 )
	{
		Save();
		m_nUpdateSaved = m_nUpdateCookie = GetTickCount();
	}
	
	LibraryDictionary.BuildHashTable();
	StartThread();
	
	return bChanged;
}

//////////////////////////////////////////////////////////////////////
// CLibrary library download queue

IMPLEMENT_DISPATCH_DISPATCH(CLibrary, Library)

STDMETHODIMP_(ULONG) CLibrary::XLibrary::AddRef()
{
	METHOD_PROLOGUE( CLibrary, Library )
	pThis->m_pSection.Lock();
	return pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) CLibrary::XLibrary::Release()
{
	METHOD_PROLOGUE( CLibrary, Library )
	pThis->m_pSection.Unlock();
	return pThis->ExternalRelease();
}

STDMETHODIMP CLibrary::XLibrary::QueryInterface(REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE( CLibrary, Library )
	HRESULT hr = pThis->ExternalQueryInterface( &iid, ppvObj );
	if ( SUCCEEDED(hr) ) pThis->m_pSection.Lock();
	return hr;
}

STDMETHODIMP CLibrary::XLibrary::get_Application(IApplication FAR* FAR* ppApplication)
{
	METHOD_PROLOGUE( CLibrary, Library )
	if ( ppApplication == NULL ) return E_INVALIDARG;
	*ppApplication = Application.GetApp();
	return S_OK;
}

STDMETHODIMP CLibrary::XLibrary::get_Library(ILibrary FAR* FAR* ppLibrary)
{
	METHOD_PROLOGUE( CLibrary, Library )
	if ( ppLibrary == NULL ) return E_INVALIDARG;
	*ppLibrary = (ILibrary*)pThis->GetInterface( IID_ILibrary, TRUE );
	return S_OK;
}

STDMETHODIMP CLibrary::XLibrary::get_Folders(ILibraryFolders FAR* FAR* ppFolders)
{
	METHOD_PROLOGUE( CLibrary, Library )
	if ( ppFolders == NULL ) return E_INVALIDARG;
	*ppFolders = (ILibraryFolders*)pThis->GetInterface( IID_ILibraryFolders, TRUE );
	return S_OK;
}

STDMETHODIMP CLibrary::XLibrary::get_Albums(IUnknown FAR* FAR* ppAlbums)
{
	METHOD_PROLOGUE( CLibrary, Library )
	if ( ppAlbums == NULL ) return E_INVALIDARG;
	return E_NOTIMPL;
}

STDMETHODIMP CLibrary::XLibrary::get_Files(ILibraryFiles FAR* FAR* ppFiles)
{
	METHOD_PROLOGUE( CLibrary, Library )
	if ( ppFiles == NULL ) return E_INVALIDARG;
	*ppFiles = (ILibraryFiles*)pThis->GetInterface( IID_ILibraryFiles, TRUE );
	return S_OK;
}

STDMETHODIMP CLibrary::XLibrary::FindByName(BSTR sName, ILibraryFile FAR* FAR* ppFile)
{
	METHOD_PROLOGUE( CLibrary, Library )
	CLibraryFile* pFile = LibraryMaps.LookupFileByName( CString( sName ) );
	*ppFile = pFile ? (ILibraryFile*)pFile->GetInterface( IID_ILibraryFile, TRUE ) : NULL;
	return pFile ? S_OK : S_FALSE;
}

STDMETHODIMP CLibrary::XLibrary::FindByPath(BSTR sPath, ILibraryFile FAR* FAR* ppFile)
{
	METHOD_PROLOGUE( CLibrary, Library )
	CLibraryFile* pFile = LibraryMaps.LookupFileByPath( CString( sPath ) );
	*ppFile = pFile ? (ILibraryFile*)pFile->GetInterface( IID_ILibraryFile, TRUE ) : NULL;
	return pFile ? S_OK : S_FALSE;
}

STDMETHODIMP CLibrary::XLibrary::FindByURN(BSTR sURN, ILibraryFile FAR* FAR* ppFile)
{
	METHOD_PROLOGUE( CLibrary, Library )
	CLibraryFile* pFile = LibraryMaps.LookupFileByURN( CString( sURN ) );
	*ppFile = pFile ? (ILibraryFile*)pFile->GetInterface( IID_ILibraryFile, TRUE ) : NULL;
	return pFile ? S_OK : S_FALSE;
}

STDMETHODIMP CLibrary::XLibrary::FindByIndex(LONG nIndex, ILibraryFile FAR* FAR* ppFile)
{
	METHOD_PROLOGUE( CLibrary, Library )
	CLibraryFile* pFile = pThis->LookupFile( (DWORD)nIndex );
	*ppFile = pFile ? (ILibraryFile*)pFile->GetInterface( IID_ILibraryFile, TRUE ) : NULL;
	return pFile ? S_OK : S_FALSE;
}
