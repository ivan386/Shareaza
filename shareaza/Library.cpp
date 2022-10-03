//
// Library.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2017.
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

#include "QueryHit.h"
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

CLibrary::CLibrary()
	:	m_nUpdateCookie	( 0 )
	,	m_nForcedUpdate	( FALSE )
	,	m_nScanCount	( 0 )
	,	m_nScanCookie	( 1 )
	,	m_nScanTime		( 0 )
	,	m_nSaveCookie	( 0 )
	,	m_nSaveTime		( 0 )
	,	m_nFileSwitch	( 0 )
{
	EnableDispatch( IID_ILibrary );
}

CLibrary::~CLibrary()
{
}

//////////////////////////////////////////////////////////////////////
// CLibrary file and folder operations

CLibraryFile* CLibrary::LookupFile(DWORD nIndex, BOOL bSharedOnly, BOOL bAvailableOnly) const
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

	if ( pFile->HasHash() )
	{
		LibraryDictionary.AddFile( pFile );
	}

	if ( pFile->IsAvailable() )
	{
		if ( pFile->IsHashed() )
		{
			LibraryHistory.Submit( pFile );

			GetAlbumRoot()->OrganiseFile( pFile );

			if ( pFile->IsNewFile() ) // the new file was hashed
			{
				pFile->m_bNewFile = FALSE;

				// TODO: May be some security and anti-virus checks?
			}
		}
		else
		{
			LibraryBuilder.Add( pFile ); // hash the file and add it again
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

	LibraryBuilder.Remove( pFile );

	if ( pFile->m_nIndex )
	{
		LibraryDictionary.RemoveFile( pFile );
	}
}

bool CLibrary::OnQueryHits(const CQueryHit* pHits)
{
	CSingleLock oLock( &m_pSection );
	if ( ! oLock.Lock( 250 ) )
		return false;

	for ( const CQueryHit* pHit = pHits ; pHit; pHit = pHit->m_pNext )
	{
		if ( ! pHit->m_sURL.IsEmpty() )
		{
			if ( CLibraryFile* pFile = LibraryMaps.LookupFileByHash( pHit ) )
			{
				pFile->AddAlternateSources( pHit->m_sURL );
			}
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// CLibrary search

CFileList* CLibrary::Search(const CQuerySearch* pSearch, int nMaximum, bool bLocal, bool bAvailableOnly)
{
	ASSUME_LOCK( m_pSection );

	if ( pSearch == NULL )
	{
		// Host browsing
		ASSERT( ! bLocal );
		return LibraryMaps.Browse( nMaximum );
	}
	else if ( pSearch->m_bWhatsNew )
	{
		// "Whats New" search
		ASSERT( ! bLocal );
		return LibraryMaps.WhatsNew( pSearch, nMaximum );
	}
	else
	{
		// Hash or exactly filename+size search
		if ( CFileList* pHits = LibraryMaps.LookupFilesByHash( pSearch, ! bLocal, bAvailableOnly, nMaximum ) )
		{
			return pHits;
		}

		// Regular keywords search
		return LibraryDictionary.Search( pSearch, nMaximum, bLocal, bAvailableOnly );
	}
}

//////////////////////////////////////////////////////////////////////
// CLibrary clear

void CLibrary::Clear()
{
	CSingleLock pLock( &m_pSection, TRUE );

	LibraryHistory.Clear();
	LibraryDictionary.Clear();
	LibraryFolders.Clear();
	LibraryMaps.Clear();
}

void CLibrary::StopThread()
{
	Exit();
	Wakeup();
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

	LibraryFolders.CreateAlbumTree();

	return FALSE;
}

BOOL CLibrary::Load()
{
	CSingleLock pLock( &m_pSection, TRUE );

#ifdef _DEBUG
	__int64 nStart = GetMicroCount();
#endif

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

		CArchive ar1( pNewest, CArchive::load, 262144 );	// 256 KB buffer
		if ( ! SafeSerialize( ar1 ) )
		{
			if ( pNewest == &pFile1 && bFile2 )
				pNewest = &pFile2;
			else if ( pNewest == &pFile2 && bFile1 )
				pNewest = &pFile1;
			else
				pNewest = NULL;

			if ( pNewest != NULL )
			{
				CArchive ar2( pNewest, CArchive::load, 262144 );	// 256 KB buffer
				SafeSerialize( ar2 );
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
	LibraryFolders.Maintain();

	LibraryHashDB.Create();
	LibraryBuilder.BoostPriority( Settings.Library.HighPriorityHash );

#ifdef _DEBUG
	__int64 nEnd = GetMicroCount();
	theApp.Message( MSG_DEBUG, _T("Library load time : %I64i ms. Files: %d, Keywords: %d, Names: %d, Paths: %d\n"),
		( nEnd - nStart ) / 1000,
		LibraryMaps.GetFileCount(),
		LibraryDictionary.GetWordCount(),
		LibraryMaps.GetNameCount(),
		LibraryMaps.GetPathCount() );
#endif

	Update();

	m_nSaveCookie = m_nUpdateCookie;
	m_nSaveTime = GetTickCount();

	BeginThread( "Library" );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLibrary save to disk

BOOL CLibrary::Save()
{
	CSingleLock pLock( &m_pSection, TRUE );

	//CAutoPtr< CXMLElement > pXML( LibraryFolders.CreateXML( _T("/"), TRUE, xmlDC ) );
	//CString strXML( pXML->ToString( TRUE, TRUE, TRUE, TRI_TRUE ) );
	//CString strXMLFile;
	//strXMLFile.Format( _T("%s\\Data\\files.xml"), (LPCTSTR)Settings.General.UserPath );
	//CStdioFile pXMLFile;
	//pXMLFile.Open( strXMLFile, CFile::modeWrite|CFile::modeCreate );
	//pXMLFile.WriteString( strXML );
	//pXMLFile.Close();

	CString strFile;
	strFile.Format( _T("%s\\Data\\Library%i.dat"),
		(LPCTSTR)Settings.General.UserPath, m_nFileSwitch + 1 );

	m_nFileSwitch = ( m_nFileSwitch == 0 ) ? 1 : 0;
	m_nSaveTime = GetTickCount();

	CFile pFile;
	if ( ! pFile.Open( strFile, CFile::modeWrite|CFile::modeCreate ) )
	{
		theApp.Message( MSG_ERROR, _T("Library save error to: %s"), (LPCTSTR)strFile );
		return FALSE;
	}

	try
	{
		FILETIME pFileTime = {};
		pFile.Write( &pFileTime, sizeof( FILETIME ) );
		pFile.Flush();

		CArchive ar( &pFile, CArchive::store, 262144 );	// 256 KB buffer
		try
		{
			Serialize( ar );
			ar.Close();
		}
		catch ( CException* pException )
		{
			ar.Abort();
			pFile.Abort();
			pException->Delete();
			theApp.Message( MSG_ERROR, _T("Library save error to: %s"), (LPCTSTR)strFile );
			return FALSE;
		}

		pFile.Flush();
		GetSystemTimeAsFileTime( &pFileTime );
		pFile.Seek( 0, 0 );
		pFile.Write( &pFileTime, sizeof( FILETIME ) );
		pFile.Close();
	}
	catch ( CException* pException )
	{
		pFile.Abort();
		pException->Delete();
		theApp.Message( MSG_ERROR, _T("Library save error to: %s"), (LPCTSTR)strFile );
		return FALSE;
	}

	m_nSaveCookie = m_nUpdateCookie;
	theApp.Message( MSG_DEBUG, _T("Library successfully saved to: %s"), (LPCTSTR)strFile );
	return TRUE;
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
		Doze( 1000 );

		if ( ! theApp.m_bLive )
		{
			Sleep( 0 );
			continue;
		}

		ThreadScan();
	}
}

//////////////////////////////////////////////////////////////////////
// CLibrary threaded scan

BOOL CLibrary::ThreadScan()
{
	// Do not start scanning until app is loaded
	if ( ! theApp.m_bLive || theApp.m_bClosing ) return FALSE;

	CSingleLock pLock( &m_pSection );
	if ( ! pLock.Lock( 100 ) )
	{
		Wakeup();	// skip default delay
		return FALSE;
	}

	// Scan was requested by Library.Update( true ) call
	BOOL bForcedScan = InterlockedCompareExchange( &m_nForcedUpdate, FALSE, TRUE );

	// If folders not watched then scan them at periodic basis
	// (default is one time per 5 seconds)
	const DWORD nNow = GetTickCount();
	BOOL bPeriodicScan = ! Settings.Library.WatchFolders &&
		( nNow < m_nScanTime || nNow - m_nScanTime > Settings.Library.WatchFoldersTimeout * 1000 );

	BOOL bChanged = LibraryFolders.ThreadScan( bPeriodicScan || bForcedScan );

	if ( bPeriodicScan || bForcedScan || bChanged )
	{
		m_nScanTime =  nNow;

		// Mark library as changed
		if ( bChanged )
			Update();
	}

	m_nScanCount++;

	// Save library changes but not frequently
	// (one time per 30 seconds)
	if ( m_nUpdateCookie != m_nSaveCookie && ( nNow < m_nSaveTime || nNow - m_nSaveTime > 30000 ) )
	{
		LibraryFolders.ClearGhosts( FALSE );
		Save();
	}

	return bChanged;
}

BOOL CLibrary::IsBadFile(LPCTSTR pszFilenameOnly, LPCTSTR pszPathOnly, DWORD dwFileAttributes)
{
	// Ignore error files (probably access denied)
	if ( dwFileAttributes == INVALID_FILE_ATTRIBUTES ) return TRUE;

	// Ignore files or folders begins from dot
	if ( pszFilenameOnly && pszFilenameOnly[ 0 ] == _T('.') ) return TRUE;

	// Ignore hidden or system files or folders
	if ( dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM) ) return TRUE;

	// Ignore metadata file or folder
	if ( pszFilenameOnly && (
		_tcsicmp( pszFilenameOnly, _T("Metadata") ) == 0 ) ) return TRUE;

	if ( ! ( dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
	{
		// Ignore encrypted files
		if ( ( dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED ) ) return TRUE;

		if ( pszFilenameOnly && (
		// Ignore our thumbnail database
			_tcsicmp( pszFilenameOnly, _T("SThumbs.dat") ) == 0 ||
		// Ignore windows thumbnail database
			_tcsicmp( pszFilenameOnly, _T("Thumbs.db") ) == 0 ||
		// Ignore video tag-file
			_tcsicmp( pszFilenameOnly, _T("dxva_sig.txt") ) == 0 ||
		// Ignore uTorrent part files
			_tcsnicmp( pszFilenameOnly, _T("~uTorrentPartFile_"), 18 ) == 0 ||
		// Ignore Ares Galaxy partials
			_tcsnicmp( pszFilenameOnly, _T("___ARESTRA___"), 13 ) == 0 ||
		// Ignore Opera password storage
			_tcsicmp( pszFilenameOnly, _T("wand.dat") ) == 0 ||
		// Ignore Thunderbird, Sunbird, and Firefox < v1.5 password storage
			_tcsicmp( pszFilenameOnly, _T("signons.txt") ) == 0 ||
		// Ignore Firefox >= v1.5 < v.3.0 password storage
			_tcsicmp( pszFilenameOnly, _T("signons2.txt") ) == 0 ||
		// Ignore Firefox >= v.3.0 < v.3.1 password storage
			_tcsicmp( pszFilenameOnly, _T("signons3.txt") ) == 0 ||
		// Ignore Firefox >= v.3.1 password storage
			_tcsicmp( pszFilenameOnly, _T("signons.sqlite") ) == 0 ||
		// Ignore Firefox key storage
			_tcsicmp( pszFilenameOnly, _T("key3.db") ) == 0 ) ) return TRUE;

		if ( pszFilenameOnly )
		{
			if ( LPCTSTR pszExt = _tcsrchr( pszFilenameOnly, _T('.') ) )
			{
				pszExt++;

				// Ignore private type files
				if ( IsIn( Settings.Library.PrivateTypes, pszExt ) )
					return TRUE;

				if ( pszPathOnly && (
				// Ignore .dat files in Kazaa folder
					_tcsistr( pszPathOnly, _T("kazaa") ) && _tcsicmp( pszExt, _T("dat") ) == 0 ) ) return TRUE;
			}
		}
	}

	if ( pszPathOnly && (
	// Ignore Firefox profile
		_tcsistr( pszPathOnly, _T("\\mozilla\\firefox\\profiles") ) ||
	// Ignore Windows Mail folder
		_tcsistr( pszPathOnly, _T("\\microsoft\\windows mail") ) ||
	// Ignore Internet Explorer folder
		_tcsistr( pszPathOnly, _T("\\Temporary Internet Files") ) ) ) return TRUE;

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CLibrary library download queue

IMPLEMENT_DISPATCH_DISPATCH(CLibrary, Library)

STDMETHODIMP_(ULONG) CLibrary::XLibrary::AddRef()
{
	METHOD_PROLOGUE( CLibrary, Library )
	pThis->m_pSection.Lock();
	return pThis->ComAddRef( this );
}

STDMETHODIMP_(ULONG) CLibrary::XLibrary::Release()
{
	METHOD_PROLOGUE( CLibrary, Library )
	pThis->m_pSection.Unlock();
	return pThis->ComRelease( this );
}

STDMETHODIMP CLibrary::XLibrary::QueryInterface(REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE( CLibrary, Library )
	HRESULT hr = pThis->ComQueryInterface( this, iid, ppvObj );
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
	return *ppLibrary ? S_OK : E_NOINTERFACE;
}

STDMETHODIMP CLibrary::XLibrary::get_Folders(ILibraryFolders FAR* FAR* ppFolders)
{
	METHOD_PROLOGUE( CLibrary, Library )
	if ( ppFolders == NULL ) return E_INVALIDARG;
	*ppFolders = (ILibraryFolders*)pThis->GetInterface( IID_ILibraryFolders, TRUE );
	return *ppFolders ? S_OK : E_NOINTERFACE;
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
	*ppFiles = (ILibraryFiles*)LibraryMaps.GetInterface( IID_ILibraryFiles, TRUE );
	return *ppFiles ? S_OK : E_NOINTERFACE;
}

STDMETHODIMP CLibrary::XLibrary::FindByName(BSTR sName, ILibraryFile FAR* FAR* ppFile)
{
	METHOD_PROLOGUE( CLibrary, Library )
	CLibraryFile* pFile = LibraryMaps.LookupFileByName( CString( sName ), SIZE_UNKNOWN, FALSE, FALSE );
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
