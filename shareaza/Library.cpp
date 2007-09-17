//
// Library.cpp
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
#include "DlgExistingFile.h"
#include "WndMain.h"
#include "Downloads.h"
#include "Download.h"

#include "QuerySearch.h"
#include "Application.h"

#include "XML.h"
#include "Schema.h"
#include "SchemaCache.h"

#include "SHA.h"
#include "ED2K.h"
#include "TigerTree.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CLibrary, CComObject)

BEGIN_INTERFACE_MAP(CLibrary, CComObject)
	INTERFACE_PART(CLibrary, IID_ILibrary, Library)
END_INTERFACE_MAP()

CLibrary Library;


//////////////////////////////////////////////////////////////////////
// CLibrary construction

CLibrary::CLibrary() :
	m_nUpdateCookie				( 0 ),
	m_nForcedUpdateCookie		( 0 ),
	m_nScanCount				( 0 ),
	m_nScanCookie				( 1 ),
	m_nScanTime					( 0 ),
	m_nUpdateSaved				( 0 ),
	m_nFileSwitch				( 0 ),
	m_hThread					( NULL ),
	m_bThread					( TRUE ),
	m_pfnGetFileAttributesExW	( NULL ),
	m_pfnGetFileAttributesExA	( NULL )
{
	EnableDispatch( IID_ILibrary );

	// theApp.m_hKernel cannot be used because this code is executed before CShareazaApp::InitResources()
	HINSTANCE hKernel;

	// It is not necessary to call LoadLibrary on Kernel32.dll, because it is already loaded into every process address space.
	if ( ( hKernel = GetModuleHandle( _T("kernel32.dll") ) ) != NULL )
	{
		(FARPROC&)m_pfnGetFileAttributesExW = GetProcAddress( hKernel, "GetFileAttributesExW" );
		(FARPROC&)m_pfnGetFileAttributesExA = GetProcAddress( hKernel, "GetFileAttributesExA" );
	}
}

CLibrary::~CLibrary()
{
}

//////////////////////////////////////////////////////////////////////
// CLibrary file and folder operations

CLibraryFile* CLibrary::LookupFile(DWORD_PTR nIndex, BOOL bSharedOnly, BOOL bAvailableOnly) const
{
	return LibraryMaps.LookupFile( nIndex, bSharedOnly, bAvailableOnly );
}

CAlbumFolder* CLibrary::GetAlbumRoot()
{
	return LibraryFolders.GetAlbumRoot();
}

void CLibrary::AddFile(CLibraryFile* pFile)
{
	if ( ! pFile->m_oBTH )
	{
		// Get BTIH of recently downloaded file
		CDownload* pDownload = Downloads.FindByPath( pFile->GetPath() );
		if ( pDownload && pDownload->IsSingleFileTorrent() )
		{
			pFile->m_oBTH = pDownload->m_oBTH;
		}
	}

	LibraryMaps.OnFileAdd( pFile );

	if ( pFile->m_oSHA1 )
	{
		LibraryDictionary.Add( pFile );
	}

	if ( pFile->IsAvailable() )
	{
        if ( pFile->m_oSHA1 || pFile->m_oTiger || pFile->m_oMD5 || pFile->m_oED2K || pFile->m_oBTH )
		{
			LibraryHistory.Submit( pFile );
			GetAlbumRoot()->OrganiseFile( pFile );
		}

        if ( !pFile->m_oSHA1 || !pFile->m_oTiger || !pFile->m_oMD5 || !pFile->m_oED2K ) // BTH ignored
		{
			LibraryBuilder.Add( pFile ); // hash the file and add it again
			Settings.Live.NewFile = TRUE;
			return;
		}
		else if ( Settings.Live.NewFile ) // the new file was hashed
		{
			CheckDuplicates( pFile ); // check for duplicates
		}
	}
	else
	{
		GetAlbumRoot()->OrganiseFile( pFile );
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

void CLibrary::OnFileDelete(CLibraryFile* pFile, BOOL bDeleteGhost)
{
	ASSERT( pFile != NULL );
	
	LibraryFolders.OnFileDelete( pFile, bDeleteGhost );
	LibraryHistory.OnFileDelete( pFile );
	LibraryHashDB.DeleteAll( pFile->m_nIndex );
}

void CLibrary::CheckDuplicates(CLibraryFile* pFile, bool bForce)
{
	long nCount = 0;

	// malicious software are usually small, we won't search duplicates
	if ( pFile->m_nSize / 1024 > Settings.Library.MaxMaliciousFileSize ) return;

	int nDot = pFile->m_sName.ReverseFind( '.' );

	if ( nDot == -1 ) return;
	if ( _tcsistr( _T("|exe|com|zip|rar|ace|7z|cab|lzh|tar|tgz|bz2|wmv|"), 
		pFile->m_sName.Mid( nDot + 1 ) ) == NULL ) return;

	for ( POSITION pos = LibraryMaps.GetFileIterator() ; pos ; )
	{
		CLibraryFile* pExisting = LibraryMaps.GetNextFile( pos );
		
		if ( validAndEqual( pFile->m_oED2K, pExisting->m_oED2K ) )
			nCount++;
	}

	if ( nCount >= 5 ) // if more than 4 the same files, it's suspicious
	{
		if ( Settings.Live.LastDuplicateHash == pFile->m_oED2K.toString() && !bForce )
		{
			// we already warned about the same file
			Settings.Live.NewFile = FALSE;
			return;
		}
		Settings.Live.LastDuplicateHash = pFile->m_oED2K.toString();
		if ( !theApp.m_bLive ) return;

		// warn the user
		CExistingFileDlg dlg( pFile, NULL, true );
		Settings.Live.MaliciousWarning = TRUE;

		m_pSection.Unlock();
		if ( dlg.DoModal() != IDOK )
		{
			Settings.Live.NewFile = FALSE;
			Settings.Live.LastDuplicateHash.Empty();
			dlg.m_nAction = 3;
		}

		if ( dlg.m_nAction == 0 )
		{
			CMainWnd* pMainWnd = (CMainWnd*)AfxGetMainWnd();
			if ( pMainWnd )
			{
				CString strHash = L"urn:ed2k:" + Settings.Live.LastDuplicateHash;
				int nLen = strHash.GetLength() + 1;
				LPTSTR pszHash = new TCHAR[ nLen ];

				CopyMemory( pszHash, strHash.GetBuffer(), sizeof(TCHAR) * nLen );
				pMainWnd->PostMessage( WM_LIBRARYSEARCH, (WPARAM)pszHash );
			}
		}
		Settings.Live.MaliciousWarning = FALSE;
	}
	else Settings.Live.LastDuplicateHash.Empty();
}

void CLibrary::CheckDuplicates(LPCTSTR pszED2KHash)
{
	Hashes::Ed2kHash oED2K;
	oED2K.fromString( pszED2KHash );

	if ( oED2K )
	{
		CSingleLock oLock( &m_pSection );
		if ( !oLock.Lock( 50 ) ) return;
		CLibraryFile* pFile = LibraryMaps.LookupFileByED2K( oED2K, FALSE, TRUE );
		if ( pFile )
		{
			CheckDuplicates( pFile, true );
		}
		else
		{
			Settings.Live.LastDuplicateHash.Empty();
			Settings.Live.MaliciousWarning = FALSE;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CLibrary search

CList< CLibraryFile* >* CLibrary::Search(CQuerySearch* pSearch, int nMaximum, BOOL bLocal, BOOL bAvailableOnly)
{
	CSingleLock oLock( &m_pSection );

	if ( !oLock.Lock( 50 ) ) return NULL;

	CList< CLibraryFile* >* pHits = LibraryMaps.Search( pSearch, nMaximum, bLocal, bAvailableOnly );

	if ( pHits == NULL && pSearch != NULL )
	{
		pHits = LibraryDictionary.Search( pSearch, nMaximum, bLocal, bAvailableOnly );
	}

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
		if ( bFile1 )
		{
			try
			{
				bFile1 = pFile1.Read( &pFileTime1, sizeof(FILETIME) ) == sizeof(FILETIME);
			}
			catch ( CException* pException )
			{
				pException->Delete();
				bFile1 = FALSE;
			}
		}
		if ( bFile2 )
		{
			try
			{
				bFile2 = pFile2.Read( &pFileTime2, sizeof(FILETIME) ) == sizeof(FILETIME);
			}
			catch ( CException* pException )
			{
				pException->Delete();
				bFile2 = FALSE;
			}
		}
	}
	else
	{
		bFile1 = pFile1.Open( strFile + _T(".dat"), CFile::modeRead );
		pFileTime1.dwHighDateTime++;
	}

	if ( bFile1 || bFile2 )
	{
		CFile* pNewest	=  ( bFile1 && bFile2 ) ?
			( ( CompareFileTime( &pFileTime1, &pFileTime2 ) >= 0 ) ? &pFile1 : &pFile2 ) :
			( bFile1 ? &pFile1 : &pFile2 );

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
	LibraryDictionary.BuildHashTable();
	LibraryBuilder.BoostPriority( Settings.Library.HighPriorityHash );

	Update();
	m_nUpdateSaved = GetTickCount();

	StartThread();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLibrary save to disk

BOOL CLibrary::Save()
{
	CSingleLock pLock( &m_pSection, TRUE );

	FILETIME pFileTime = { 0, 0 };
	SYSTEMTIME pSystemTime;
	CString strFile;
	CFile pFile;

	strFile.Format( _T("%s\\Data\\Library%i.dat"),
		(LPCTSTR)Settings.General.UserPath, m_nFileSwitch + 1 );

	m_nFileSwitch = ( m_nFileSwitch == 0 ) ? 1 : 0;

	if ( ! pFile.Open( strFile, CFile::modeWrite|CFile::modeCreate ) ) return FALSE;

	try
	{
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
		
		theApp.Message( MSG_DEBUG, _T("Library successfully saved to: %s"), strFile );
		return TRUE;
	}
	catch ( CException* pException )
	{
		pException->Delete();

		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// CLibrary serialize

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

	LibraryDictionary.Serialize( ar, nVersion );
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
		m_hThread = BeginThread( "Library", ThreadStart, this, THREAD_PRIORITY_BELOW_NORMAL );
	}
}

void CLibrary::StopThread()
{
	LibraryBuilder.StopThread();

	m_bThread = FALSE;
	Wakeup();

	CloseThread( &m_hThread );
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
		ThreadScan();
		WaitForSingleObject( m_pWakeup, 1000 );
	}
}

//////////////////////////////////////////////////////////////////////
// CLibrary threaded scan

BOOL CLibrary::ThreadScan()
{
	// Do not start scanning until app is loaded
	if ( ! theApp.m_bLive ) return FALSE;

	CSingleLock pLock( &m_pSection );
	if ( ! pLock.Lock( 100 ) )
	{
		Wakeup();	// skip default delay
		return FALSE;
	}

	BOOL bChanged = FALSE;
	DWORD tNow = GetTickCount();
	BOOL bPeriodicScan = ( m_nScanTime < tNow - Settings.Library.WatchFoldersTimeout * 1000 );
	BOOL bForcedScan = ( m_nForcedUpdateCookie < m_nUpdateCookie ) &&
		( m_nUpdateCookie > tNow - Settings.Library.WatchFoldersTimeout * 1000 );
	if ( bForcedScan )
		m_nForcedUpdateCookie = m_nUpdateCookie;

	if ( bPeriodicScan || bForcedScan )
	{
		bChanged = LibraryFolders.ThreadScan( &m_bThread, FALSE );
		m_nScanTime = GetTickCount();

		if ( bChanged )
			Update();
	}

	m_nScanCount++;

	if ( m_nUpdateCookie > m_nUpdateSaved && tNow - m_nUpdateSaved > 30000 )
	{
		if ( Save() )
			m_nUpdateSaved = GetTickCount();

		if ( bChanged )
		{
			LibraryDictionary.BuildHashTable();
		}
	}

	return bChanged;
}

BOOL CLibrary::IsBadFile(LPCTSTR pszFilenameOnly, LPCTSTR pszPathOnly, DWORD dwFileAttributes)
{
	ASSERT( pszFilenameOnly );

	// Ignore files or folders begins from dot
	if ( pszFilenameOnly[0] == _T('.') ) return TRUE;
	// Ignore hidden or system files or folders
	if ( dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM) ) return TRUE;
	// Ignore metadata file or folder
	if ( _tcsicmp( pszFilenameOnly, _T("Metadata") ) == 0 ) return TRUE;

	if ( ! ( dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
	{
		// Ignore encrypted files
		if ( dwFileAttributes & (FILE_ATTRIBUTE_ENCRYPTED) ) return TRUE;
		// Ignore our thumbnail database
		if ( _tcsicmp( pszFilenameOnly, _T("SThumbs.dat") ) == 0 ) return TRUE;
		// Ignore windows thumbnail database
		if ( _tcsicmp( pszFilenameOnly, _T("Thumbs.db") ) == 0 ) return TRUE;
		// Ignore video tag-file
		if ( _tcsicmp( pszFilenameOnly, _T("dxva_sig.txt") ) == 0 ) return TRUE;
		// Ignore WinMX partial files
		if ( _tcsnicmp( pszFilenameOnly, _T("__INCOMPLETE___"), 15 ) == 0 ) return TRUE;

		LPCTSTR pszExt = _tcsrchr( pszFilenameOnly, _T('.') );
		if ( pszExt++ )
		{
			// Ignore private type files
			if ( IsIn( Settings.Library.PrivateTypes, pszExt ) )
				return TRUE;
			// Ignore .dat files in Kazaa folder
			if ( pszPathOnly && _tcsistr( pszPathOnly, _T("kazaa") ) != NULL &&
				_tcsicmp( pszExt, _T("dat") ) == 0 )
				return TRUE;
		}
	}

	return FALSE;
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
