//
// Library.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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

#include "QuerySearch.h"
#include "Application.h"

#include "XML.h"
#include "Schema.h"
#include "SchemaCache.h"

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
	m_nUpdateCookie				( 0ul )
,	m_nForcedUpdateCookie		( 0ul )
,	m_nScanCount				( 0ul )
,	m_nScanCookie				( 1ul )
,	m_nScanTime					( 0ul )
,	m_nUpdateSaved				( 0ul )
,	m_nFileSwitch				( 0 )
{
	EnableDispatch( IID_ILibrary );
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

		if ( ! pFile->IsHashed() )
		{
			LibraryBuilder.Add( pFile ); // hash the file and add it again
		}
		else if ( pFile->IsNewFile() ) // the new file was hashed
		{
			pFile->m_bNewFile = FALSE;

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

		if ( validAndEqual( pFile->m_oMD5, pExisting->m_oMD5 ) )
			nCount++;
	}

	if ( nCount >= 5 ) // if more than 4 the same files, it's suspicious
	{
		if ( Settings.Live.LastDuplicateHash == pFile->m_oMD5.toString() && !bForce )
		{
			// we already warned about the same file
			return;
		}
		Settings.Live.LastDuplicateHash = pFile->m_oMD5.toString();
		if ( !theApp.m_bLive ) return;

		// warn the user
		CExistingFileDlg dlg( pFile, NULL, true );
		Settings.Live.MaliciousWarning = TRUE;

		m_pSection.Unlock();
		if ( dlg.DoModal() != IDOK )
		{
			Settings.Live.LastDuplicateHash.Empty();
			dlg.m_nAction = 3;
		}

		if ( dlg.m_nAction == 0 )
		{
			CMainWnd* pMainWnd = (CMainWnd*)AfxGetMainWnd();
			if ( pMainWnd )
			{
				CString strHash = L"urn:md5:" + Settings.Live.LastDuplicateHash;
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

void CLibrary::CheckDuplicates(LPCTSTR pszMD5Hash)
{
	Hashes::Md5Hash oMD5;
	oMD5.fromString( pszMD5Hash );

	if ( oMD5 )
	{
		CSingleLock oLock( &m_pSection );
		if ( !oLock.Lock( 50 ) ) return;
		CLibraryFile* pFile = LibraryMaps.LookupFileByMD5( oMD5, FALSE, TRUE );
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
	LibraryBuilder.StopThread();

	CloseThread();

	CSingleLock pLock( &m_pSection, TRUE );

	LibraryHistory.Clear();
	LibraryDictionary.Clear();
	LibraryFolders.Clear();
	LibraryMaps.Clear();
}

//////////////////////////////////////////////////////////////////////
// CLibrary load from disk

BOOL CLibrary::SafeReadTime(CFile& pFile, FILETIME* pFileTime) throw()
{
	__try
	{
		return pFile.Read( pFileTime, sizeof(FILETIME) ) == sizeof(FILETIME);
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
	}
	return FALSE;
}

BOOL CLibrary::SafeSerialize(CArchive& ar) throw()
{
	CFile* fp = ar.GetFile();

	__try
	{
		Serialize( ar );

		ar.Close();
		fp->Close();

		return TRUE;
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
	}
	__try
	{
		ar.Close();
		fp->Close();

		Clear();
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
	}
	return FALSE;
}

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
			bFile1 = SafeReadTime( pFile1, &pFileTime1 );
		}
		if ( bFile2 )
		{
			bFile2 = SafeReadTime( pFile2, &pFileTime2 );
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

		CArchive ar( pNewest, CArchive::load, 40960 );
		if ( ! SafeSerialize( ar ) )
		{
			if ( pNewest == &pFile1 && bFile2 )
				pNewest = &pFile2;
			else if ( pNewest == &pFile2 && bFile1 )
				pNewest = &pFile1;
			else
				pNewest = NULL;

			if ( pNewest != NULL )
			{
				CArchive ar( pNewest, CArchive::load, 40960 );
				SafeSerialize( ar );
			}
		}
	}
	else
	{
		CreateDirectory( Settings.Downloads.CompletePath );
		LibraryFolders.AddFolder( Settings.Downloads.CompletePath );

		CreateDirectory( Settings.Downloads.CollectionPath );
		LibraryFolders.AddFolder( Settings.Downloads.CollectionPath );

		CreateDirectory( Settings.Downloads.TorrentPath );
		LibraryFolders.AddFolder( Settings.Downloads.TorrentPath );
	}

	LibraryFolders.CreateAlbumTree();
	LibraryHashDB.Create();
	LibraryDictionary.BuildHashTable();
	LibraryBuilder.BoostPriority( Settings.Library.HighPriorityHash );

	Update();
	m_nUpdateSaved = GetTickCount();

	BeginThread( "Library" );

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
// CLibrary thread run

void CLibrary::OnRun()
{
	while ( IsThreadEnabled() )
	{
		ThreadScan();
		Doze( 1000 );
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

	BOOL bForcedScan = FALSE, bPeriodicScan = FALSE;
	DWORD tNow = GetTickCount();

	if ( !Settings.Library.WatchFolders )
	{
		bForcedScan = ( m_nForcedUpdateCookie == 0 );
	}
	else
	{
		bPeriodicScan = ( m_nScanTime < tNow - Settings.Library.WatchFoldersTimeout * 1000 );
	}

	BOOL bChanged = LibraryFolders.ThreadScan( bPeriodicScan || bForcedScan );

	if ( bPeriodicScan || bForcedScan || bChanged )
	{
		m_nScanTime = GetTickCount();

		if ( bForcedScan )
			m_nForcedUpdateCookie = m_nUpdateCookie;

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
		// uTorrent part files
		if ( _tcsnicmp( pszFilenameOnly, _T("~uTorrentPartFile_"), 18 ) == 0 ) return TRUE;

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
	return CApplication::GetApp( ppApplication );
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
