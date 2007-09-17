//
// SharedFolder.cpp
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
#include "SharedFolder.h"
#include "SharedFile.h"
#include "Library.h"
#include "Application.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CLibraryFolder, CComObject)

BEGIN_INTERFACE_MAP(CLibraryFolder, CComObject)
	INTERFACE_PART(CLibraryFolder, IID_ILibraryFolder, LibraryFolder)
	INTERFACE_PART(CLibraryFolder, IID_ILibraryFolders, LibraryFolders)
	INTERFACE_PART(CLibraryFolder, IID_ILibraryFiles, LibraryFiles)
END_INTERFACE_MAP()

//////////////////////////////////////////////////////////////////////
// CLibraryFolder construction

CLibraryFolder::CLibraryFolder(CLibraryFolder* pParent, LPCTSTR pszPath) :
	m_nScanCookie( 0 ),
	m_nUpdateCookie( 0 ),
	m_nSelectCookie( 0 ),
	m_pParent( pParent ),
	m_sPath( pszPath ),
	m_bShared( pParent ? TS_UNKNOWN : TS_TRUE ),
	m_bExpanded( pParent ? FALSE : TRUE ),
	m_nFiles( 0 ),
	m_nVolume( 0 ),
	m_hMonitor( INVALID_HANDLE_VALUE ),
	m_bForceScan( TRUE ),
	m_bOffline( FALSE )
{
	EnableDispatch( IID_ILibraryFolder );
	EnableDispatch( IID_ILibraryFolders );
	EnableDispatch( IID_ILibraryFiles );

	PathToName();
}

CLibraryFolder::~CLibraryFolder()
{
	if ( m_hMonitor != INVALID_HANDLE_VALUE ) FindCloseChangeNotification( m_hMonitor );

	Clear();
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder folder list

POSITION CLibraryFolder::GetFolderIterator() const
{
	return m_pFolders.GetStartPosition();
}

CLibraryFolder* CLibraryFolder::GetNextFolder(POSITION& pos) const
{
	CLibraryFolder* pOutput = NULL;
	CString strName;
	m_pFolders.GetNextAssoc( pos, strName, pOutput );
	return pOutput;
}

CLibraryFolder* CLibraryFolder::GetFolderByName(LPCTSTR pszName) const
{
	CLibraryFolder* pOutput = NULL;
	CString strName( pszName );
	ToLower( strName );
	return ( m_pFolders.Lookup( strName, pOutput ) ) ? pOutput : NULL;
}

CLibraryFolder* CLibraryFolder::GetFolderByPath(LPCTSTR pszPath) const
{
	if ( m_sPath.CompareNoCase( pszPath ) == 0 ) return (CLibraryFolder*)this;

	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		CLibraryFolder* pFolder = GetNextFolder( pos )->GetFolderByPath( pszPath );
		if ( pFolder ) return pFolder;
	}

	return NULL;
}

BOOL CLibraryFolder::CheckFolder(CLibraryFolder* pFolder, BOOL bRecursive) const
{
	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		CLibraryFolder* pCheck = GetNextFolder( pos );
		if ( pCheck == pFolder ) return TRUE;
		if ( bRecursive && pCheck->CheckFolder( pFolder, TRUE ) ) return TRUE;
	}

	return FALSE;
}

INT_PTR CLibraryFolder::GetFolderCount() const
{
	return m_pFolders.GetCount();
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder file list

POSITION CLibraryFolder::GetFileIterator() const
{
	return m_pFiles.GetStartPosition();
}

CLibraryFile* CLibraryFolder::GetNextFile(POSITION& pos) const
{
	CLibraryFile* pOutput = NULL;
	CString strName;
	m_pFiles.GetNextAssoc( pos, strName, pOutput );
	return pOutput;
}

CLibraryFile* CLibraryFolder::GetFile(LPCTSTR pszName) const
{
	CLibraryFile* pOutput = NULL;
	CString strName( pszName );
	ToLower( strName );
	return ( m_pFiles.Lookup( strName, pOutput ) ) ? pOutput : NULL;
}

INT_PTR CLibraryFolder::GetFileCount() const
{
	return m_pFiles.GetCount();
}

int CLibraryFolder::GetFileList(CLibraryList* pList, BOOL bRecursive) const
{
	int nCount = 0;
	
	for ( POSITION pos = GetFileIterator() ; pos ; )
	{
		pList->CheckAndAdd( GetNextFile( pos ) );
		nCount++;
	}
	
	if ( bRecursive )
	{
		for ( POSITION pos = GetFolderIterator() ; pos ; )
		{
			GetNextFolder( pos )->GetFileList( pList, bRecursive );
		}
	}
	
	return nCount;
}

int CLibraryFolder::GetSharedCount() const
{
	int nCount = 0;
	
	for ( POSITION pos = GetFileIterator() ; pos ; )
	{
		CLibraryFile* pFile = GetNextFile( pos );
		if ( pFile->IsShared() ) nCount++;
	}
	
	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		nCount += GetNextFolder( pos )->GetSharedCount();
	}
	
	return nCount;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder clear

void CLibraryFolder::Clear()
{
	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		delete GetNextFolder( pos );
	}
	
	for ( POSITION pos = GetFileIterator() ; pos ; )
	{
		delete GetNextFile( pos );
	}
	
	m_pFolders.RemoveAll();
	m_pFiles.RemoveAll();
	
	m_nFiles	= 0;
	m_nVolume	= 0;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder serialize

void CLibraryFolder::Serialize(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		ar << m_sPath;
		ar << m_bShared;
		ar << m_bExpanded;

		ar.WriteCount( GetFolderCount() );

		for ( POSITION pos = GetFolderIterator() ; pos ; )
		{
			GetNextFolder( pos )->Serialize( ar, nVersion );
		}
		
		ar.WriteCount( GetFileCount() );

		for ( POSITION pos = GetFileIterator() ; pos ; )
		{
			GetNextFile( pos )->Serialize( ar, nVersion );
		}
	}
	else
	{
		Clear();

		ar >> m_sPath;

		if ( nVersion >= 5 )
		{
			ar >> m_bShared;
		}
		else
		{
			BYTE bShared;
			ar >> bShared;
			m_bShared = bShared ? TS_UNKNOWN : TS_FALSE;
		}
		
		if ( nVersion >= 3 ) ar >> m_bExpanded;

		PathToName();

		DWORD_PTR nCount = ar.ReadCount();

		m_pFolders.InitHashTable( GetBestHashTableSize( nCount ) );

		for ( ; nCount > 0 ; nCount-- )
		{
			CLibraryFolder* pFolder = new CLibraryFolder( this );
			if ( pFolder == NULL )
			{
				theApp.Message( MSG_ERROR, _T("Memory allocation error in CLibraryFolder::Serialize") );
				break;
			}
			pFolder->Serialize( ar, nVersion );

			m_pFolders.SetAt( pFolder->m_sNameLC, pFolder );

			m_nFiles	+= pFolder->m_nFiles;
			m_nVolume	+= pFolder->m_nVolume;
		}
	
		nCount = ar.ReadCount();
		m_pFiles.InitHashTable( GetBestHashTableSize( nCount ) );

		for ( ; nCount > 0 ; nCount-- )
		{
			CLibraryFile* pFile = new CLibraryFile( this );
			if ( pFile == NULL )
			{
				theApp.Message( MSG_ERROR, _T("Memory allocation error in CLibraryFolder::Serialize") );
				break;
			}
			pFile->Serialize( ar, nVersion );

			m_pFiles.SetAt( pFile->GetNameLC(), pFile );

			m_nFiles	++;
			m_nVolume	+= pFile->m_nSize;
		}
	}
}

void CLibraryFolder::PathToName()
{
	m_sName = m_sPath;
	int nPos = m_sName.ReverseFind( '\\' );
	if ( nPos >= 0 && nPos < m_sName.GetLength() - 1 ) m_sName = m_sName.Mid( nPos + 1 );
	m_sNameLC = m_sName;
	ToLower( m_sNameLC );
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder threaded scan

BOOL CLibraryFolder::ThreadScan(volatile BOOL* pbContinue, DWORD nScanCookie)
{
	CSingleLock pLock( &Library.m_pSection );
	CString strMetaData;
	LPCTSTR pszMetaData = NULL;
	WIN32_FIND_DATA pFind;
	HANDLE hSearch;
	
	if ( m_pParent == NULL )
		theApp.Message( MSG_DEBUG, _T("Library scanning: %s"), (LPCTSTR)m_sPath );
	
	if ( m_sPath.CompareNoCase( Settings.Downloads.IncompletePath ) == 0 ) return FALSE;
	
	strMetaData = m_sPath + _T("\\Metadata");
	
	hSearch = FindFirstFile( strMetaData + _T("\\*.*"), &pFind );
	
	if ( hSearch != INVALID_HANDLE_VALUE )
	{
		FindClose( hSearch );
		strMetaData += '\\';
		pszMetaData = strMetaData;
	}
	
	hSearch = FindFirstFile( m_sPath + _T("\\*.*"), &pFind );
	
	pLock.Lock();
	m_nScanCookie	= nScanCookie;
	nScanCookie		= ++Library.m_nScanCookie;
	pLock.Unlock();
	
	BOOL bChanged = FALSE;
	
	if ( hSearch != INVALID_HANDLE_VALUE )
	{
		do
		{
			if ( CLibrary::IsBadFile( pFind.cFileName, m_sPath,
				pFind.dwFileAttributes )  ) continue;

			if ( pFind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				CLibraryFolder* pFolder = GetFolderByName( pFind.cFileName );
				if ( pFolder != NULL )
				{
					m_nFiles	-= pFolder->m_nFiles;
					m_nVolume	-= pFolder->m_nVolume;
					
					if ( pFolder->m_sName != pFind.cFileName )
					{
						CString strNameLC( pFolder->m_sName );
						ToLower( strNameLC );

						pLock.Lock();
						m_pFolders.RemoveKey( strNameLC );
						pFolder->OnDelete();
						pFolder = new CLibraryFolder( this, m_sPath + _T("\\") + pFind.cFileName );
						m_pFolders.SetAt( pFolder->m_sNameLC, pFolder );
						bChanged = TRUE;
						m_nUpdateCookie++;
						pLock.Unlock();
					}
				}
				else
				{
					pFolder = new CLibraryFolder( this, m_sPath + _T("\\") + pFind.cFileName );
					pLock.Lock();
					m_pFolders.SetAt( pFolder->m_sNameLC, pFolder );
					bChanged = TRUE;
					m_nUpdateCookie++;
					pLock.Unlock();
				}
				
				if ( pFolder->ThreadScan( pbContinue, nScanCookie ) )
					bChanged = TRUE;
				
				m_nFiles	+= pFolder->m_nFiles;
				m_nVolume	+= pFolder->m_nVolume;
			}
			else
			{
				CLibraryFile* pFile = GetFile( pFind.cFileName );
				if ( pFile != NULL )
				{
					m_nVolume -= pFile->m_nSize;
					
					if ( pFile->m_sName != pFind.cFileName )
					{
						pLock.Lock();
						Library.RemoveFile( pFile );
						pFile->m_sName = pFind.cFileName;
						Library.AddFile( pFile );
						bChanged = TRUE;
						pLock.Unlock();
					}
				}
				else
				{
					pFile = new CLibraryFile( this, pFind.cFileName );
					pLock.Lock();
					m_pFiles.SetAt( pFile->GetNameLC(), pFile );
					m_nFiles++;
					bChanged = TRUE;
					pLock.Unlock();
				}

				QWORD nLongSize = (QWORD)pFind.nFileSizeLow |
					( (QWORD)pFind.nFileSizeHigh << 32 );

				if ( pFile->ThreadScan( pLock, nScanCookie, nLongSize,
					&pFind.ftLastWriteTime, pszMetaData ) )
					bChanged = TRUE;

				m_nVolume += pFile->m_nSize;
			}
		}
		while ( *pbContinue && FindNextFile( hSearch, &pFind ) );
		
		FindClose( hSearch );
	}
	
	if ( ! *pbContinue ) return FALSE;
	
	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		CLibraryFolder* pFolder = GetNextFolder( pos );
		
		if ( pFolder->m_nScanCookie != nScanCookie )
		{
			CString strNameLC( pFolder->m_sName );
			ToLower( strNameLC );
			
			m_nFiles	-= pFolder->m_nFiles;
			m_nVolume	-= pFolder->m_nVolume;
			
			if ( ! pLock.IsLocked() ) pLock.Lock();
			
			m_pFolders.RemoveKey( strNameLC );
			pFolder->OnDelete();
			
			bChanged = TRUE;
			m_nUpdateCookie++;
		}
	}
	
	for ( POSITION pos = GetFileIterator() ; pos ; )
	{
		CLibraryFile* pFile = GetNextFile( pos );
		
		if ( pFile->m_nScanCookie != nScanCookie )
		{
			CString strNameLC( pFile->m_sName );
			ToLower( strNameLC );
			
			m_nFiles	--;
			m_nVolume	-= pFile->m_nSize;
			
			if ( ! pLock.IsLocked() ) pLock.Lock();
			
			pFile->OnDelete();
			m_pFiles.RemoveKey( strNameLC );
			
			bChanged = TRUE;
		}
	}
	
	if ( m_pParent == NULL )
		theApp.Message( MSG_DEBUG, _T("Finished scanning (%i)"), bChanged );
	
	return bChanged;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder monitor

BOOL CLibraryFolder::IsChanged()
{
	BOOL bChanged = FALSE;

	if ( m_bForceScan )
	{
		// Change status forced
		bChanged = TRUE;
		m_bForceScan = FALSE;
	}

	// Monitor changes
	if ( m_hMonitor != INVALID_HANDLE_VALUE )
	{
		switch ( WaitForSingleObject( m_hMonitor, 0 ) )
		{
		case WAIT_TIMEOUT:
			// No changes
			break;

		case WAIT_OBJECT_0:
			// Changes detected
			bChanged = TRUE;

			// Reset monitor
			if ( FindNextChangeNotification( m_hMonitor ) )
				break;

		default:
			// Errors
			FindCloseChangeNotification( m_hMonitor );
			m_hMonitor = INVALID_HANDLE_VALUE;
		}
	}

	if ( m_hMonitor == INVALID_HANDLE_VALUE && Settings.Library.WatchFolders )
	{
		// Enable monitor
		m_hMonitor = FindFirstChangeNotification( m_sPath, TRUE,
			FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME|
			FILE_NOTIFY_CHANGE_LAST_WRITE );
	}
	else if ( m_hMonitor != INVALID_HANDLE_VALUE && ! Settings.Library.WatchFolders )
	{
		// Disable monitor
		FindCloseChangeNotification( m_hMonitor );
		m_hMonitor = INVALID_HANDLE_VALUE;
	}

	return bChanged;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder scan

void CLibraryFolder::Scan()
{
    CLibraryFolder* pFolder = this;
	for ( ; pFolder->m_pParent ; pFolder = pFolder->m_pParent );
	if ( pFolder ) pFolder->m_bForceScan = TRUE;
	Library.Wakeup();
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder shared status managing

BOOL CLibraryFolder::IsShared() const
{
	if ( m_bOffline )
		return FALSE;

	for ( const CLibraryFolder* pFolder = this ; pFolder ; pFolder = pFolder->m_pParent )
	{
		if ( pFolder->m_bShared )
		{
			if ( pFolder->m_bShared == TS_TRUE )
				return TRUE;
			if ( pFolder->m_bShared == TS_FALSE )
				return FALSE;
		}
	}

	return TRUE;
}

void CLibraryFolder::GetShared(BOOL& bShared) const
{
	if ( m_bOffline )
		bShared = FALSE;
	else if ( m_bShared == TS_TRUE )
		bShared = TRUE;
	else if ( m_bShared == TS_FALSE )
		bShared = FALSE;
}

void CLibraryFolder::SetShared(TRISTATE bShared)
{
	m_bShared = bShared;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder offline status managing

BOOL CLibraryFolder::IsOffline() const
{
	return m_bOffline;
}

BOOL CLibraryFolder::SetOffline()
{
	if ( ! m_bOffline )
	{
		m_bOffline = TRUE;
		m_nUpdateCookie++;

		for ( POSITION pos = GetFolderIterator() ; pos ; )
		{
			GetNextFolder( pos )->SetOffline();
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CLibraryFolder::SetOnline()
{
	if ( m_bOffline )
	{
		m_bOffline = FALSE;
		m_nUpdateCookie++;

		for ( POSITION pos = GetFolderIterator() ; pos ; )
		{
			GetNextFolder( pos )->SetOnline();
		}

		return TRUE;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder callbacks

void CLibraryFolder::OnDelete(TRISTATE bCreateGhost)
{
	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		GetNextFolder( pos )->OnDelete();
	}
	
	for ( POSITION pos = GetFileIterator() ; pos ; )
	{
		GetNextFile( pos )->OnDelete( FALSE, bCreateGhost );
	}
	
	m_pFolders.RemoveAll();
	m_pFiles.RemoveAll();
	
	delete this;
}

void CLibraryFolder::OnFileRename(CLibraryFile* pFile)
{
	for ( POSITION pos = m_pFiles.GetStartPosition() ; pos ; )
	{
		CLibraryFile* pOld = NULL;
		CString strName;

		m_pFiles.GetNextAssoc( pos, strName, pOld );

		if ( pFile == pOld )
		{
			m_pFiles.RemoveKey( strName );
			m_pFiles.SetAt( pFile->GetNameLC(), pFile );
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder ILibraryFolder

IMPLEMENT_DISPATCH(CLibraryFolder, LibraryFolder)

STDMETHODIMP CLibraryFolder::XLibraryFolder::get_Application(IApplication FAR* FAR* ppApplication)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolder )
	*ppApplication = Application.GetApp();
	return S_OK;
}

STDMETHODIMP CLibraryFolder::XLibraryFolder::get_Library(ILibrary FAR* FAR* ppLibrary)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolder )
	*ppLibrary = (ILibrary*)Library.GetInterface( IID_ILibrary, TRUE );
	return S_OK;
}

STDMETHODIMP CLibraryFolder::XLibraryFolder::get_Parent(ILibraryFolder FAR* FAR* ppFolder)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolder )
	if ( pThis->m_pParent )
		*ppFolder = (ILibraryFolder*)pThis->m_pParent->GetInterface( IID_ILibraryFolder, TRUE );
	else
		*ppFolder = NULL;
	return S_OK;
}

STDMETHODIMP CLibraryFolder::XLibraryFolder::get_Path(BSTR FAR* psPath)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolder )
	pThis->m_sPath.SetSysString( psPath );
	return S_OK;
}

STDMETHODIMP CLibraryFolder::XLibraryFolder::get_Name(BSTR FAR* psPath)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolder )
	pThis->m_sName.SetSysString( psPath );
	return S_OK;
}

STDMETHODIMP CLibraryFolder::XLibraryFolder::get_Shared(STRISTATE FAR* pnValue)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolder )
	*pnValue = (STRISTATE)pThis->m_bShared;
	return S_OK;
}

STDMETHODIMP CLibraryFolder::XLibraryFolder::put_Shared(STRISTATE nValue)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolder )
	pThis->m_bShared = nValue;
	return S_OK;
}

STDMETHODIMP CLibraryFolder::XLibraryFolder::get_EffectiveShared(VARIANT_BOOL FAR* pbValue)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolder )
	*pbValue = pThis->IsShared() ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CLibraryFolder::XLibraryFolder::get_Folders(ILibraryFolders FAR* FAR* ppFolders)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolder )
	*ppFolders = (ILibraryFolders*)pThis->GetInterface( IID_ILibraryFolders, TRUE );
	return S_OK;
}

STDMETHODIMP CLibraryFolder::XLibraryFolder::get_Files(ILibraryFiles FAR* FAR* ppFiles)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolder )
	*ppFiles = (ILibraryFiles*)pThis->GetInterface( IID_ILibraryFiles, TRUE );
	return S_OK;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder ILibraryFolders

IMPLEMENT_DISPATCH(CLibraryFolder, LibraryFolders)

STDMETHODIMP CLibraryFolder::XLibraryFolders::get_Application(IApplication FAR* FAR* ppApplication)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolders )
	*ppApplication = Application.GetApp();
	return S_OK;
}

STDMETHODIMP CLibraryFolder::XLibraryFolders::get_Library(ILibrary FAR* FAR* ppLibrary)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolders )
	*ppLibrary = (ILibrary*)Library.GetInterface( IID_ILibrary, TRUE );
	return S_OK;
}

STDMETHODIMP CLibraryFolder::XLibraryFolders::get__NewEnum(IUnknown FAR* FAR* /*ppEnum*/)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolders )
	return E_NOTIMPL;
}

STDMETHODIMP CLibraryFolder::XLibraryFolders::get_Item(VARIANT vIndex, ILibraryFolder FAR* FAR* ppFolder)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolders )

	CLibraryFolder* pFolder = NULL;
	*ppFolder = NULL;
	
	if ( vIndex.vt == VT_BSTR )
	{
		CString strName( vIndex.bstrVal );
		if ( strName.Find( '\\' ) >= 0 )
			pFolder = pThis->GetFolderByPath( strName );
		else
			pFolder = pThis->GetFolderByName( strName );
	}
	else
	{
		VARIANT va;
		VariantInit( &va );

		if ( FAILED( VariantChangeType( &va, (VARIANT FAR*)&vIndex, 0, VT_I4 ) ) )
			return E_INVALIDARG;
		if ( va.lVal < 0 || va.lVal >= pThis->GetFolderCount() )
			return E_INVALIDARG;
		
		for ( POSITION pos = pThis->GetFolderIterator() ; pos ; )
		{
			pFolder = pThis->GetNextFolder( pos );
			if ( va.lVal-- == 0 ) break;
			pFolder = NULL;
		}
	}
	
	*ppFolder = pFolder ? (ILibraryFolder*)pFolder->GetInterface( IID_ILibraryFolder, TRUE ) : NULL;
	
	return S_OK;
}

STDMETHODIMP CLibraryFolder::XLibraryFolders::get_Count(LONG FAR* pnCount)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFolders )
	*pnCount = static_cast< LONG >( pThis->GetFolderCount() );
	return S_OK;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder ILibraryFiles

IMPLEMENT_DISPATCH(CLibraryFolder, LibraryFiles)

STDMETHODIMP CLibraryFolder::XLibraryFiles::get_Application(IApplication FAR* FAR* ppApplication)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFiles )
	*ppApplication = Application.GetApp();
	return S_OK;
}

STDMETHODIMP CLibraryFolder::XLibraryFiles::get_Library(ILibrary FAR* FAR* ppLibrary)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFiles )
	*ppLibrary = (ILibrary*)Library.GetInterface( IID_ILibrary, TRUE );
	return S_OK;
}

STDMETHODIMP CLibraryFolder::XLibraryFiles::get__NewEnum(IUnknown FAR* FAR* /*ppEnum*/)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFiles )
	return E_NOTIMPL;
}

STDMETHODIMP CLibraryFolder::XLibraryFiles::get_Item(VARIANT vIndex, ILibraryFile FAR* FAR* ppFile)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFiles )

	CLibraryFile* pFile = NULL;
	*ppFile = NULL;
	
	if ( vIndex.vt == VT_BSTR )
	{
		CString strName( vIndex.bstrVal );
		pFile = pThis->GetFile( strName );
	}
	else
	{
		VARIANT va;
		VariantInit( &va );

		if ( FAILED( VariantChangeType( &va, (VARIANT FAR*)&vIndex, 0, VT_I4 ) ) )
			return E_INVALIDARG;
		if ( va.lVal < 0 || va.lVal >= pThis->GetFileCount() )
			return E_INVALIDARG;
		
		for ( POSITION pos = pThis->GetFileIterator() ; pos ; )
		{
			pFile = pThis->GetNextFile( pos );
			if ( va.lVal-- == 0 ) break;
			pFile = NULL;
		}
	}
	
	*ppFile = pFile ? (ILibraryFile*)pFile->GetInterface( IID_ILibraryFile, TRUE ) : NULL;
	
	return S_OK;
}

STDMETHODIMP CLibraryFolder::XLibraryFiles::get_Count(LONG FAR* pnCount)
{
	METHOD_PROLOGUE( CLibraryFolder, LibraryFiles )
	*pnCount = static_cast< LONG >( pThis->GetFileCount() );
	return S_OK;
}

