//
// SharedFolder.cpp
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

CLibraryFolder::CLibraryFolder(CLibraryFolder* pParent, LPCTSTR pszPath)
{
	EnableDispatch( IID_ILibraryFolder );
	EnableDispatch( IID_ILibraryFolders );
	EnableDispatch( IID_ILibraryFiles );
	
	m_pParent	= pParent;
	
	m_nFiles	= 0;
	m_nVolume	= 0;
	m_bShared	= pParent ? TS_UNKNOWN : TS_TRUE;
	m_bExpanded	= pParent ? FALSE : TRUE;
	
	m_nScanCookie	= 0;
	m_nUpdateCookie	= 0;
	m_nSelectCookie	= 0;
	
	m_hMonitor		= INVALID_HANDLE_VALUE;
	m_bMonitor		= FALSE;

	if ( pszPath )
	{
		m_sPath = pszPath;
		PathToName();
	}
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
	m_pFolders.GetNextAssoc( pos, strName, (CObject*&)pOutput );
	return pOutput;
}

CLibraryFolder* CLibraryFolder::GetFolderByName(LPCTSTR pszName) const
{
	CLibraryFolder* pOutput = NULL;
	CString strName( pszName );
	CharLower( strName.GetBuffer() );
	strName.ReleaseBuffer();
	return ( m_pFolders.Lookup( strName, (CObject*&)pOutput ) ) ? pOutput : NULL;
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

int CLibraryFolder::GetFolderCount() const
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
	m_pFiles.GetNextAssoc( pos, strName, (CObject*&)pOutput );
	return pOutput;
}

CLibraryFile* CLibraryFolder::GetFile(LPCTSTR pszName) const
{
	CLibraryFile* pOutput = NULL;
	CString strName( pszName );
	CharLower( strName.GetBuffer() );
	strName.ReleaseBuffer();
	return ( m_pFiles.Lookup( strName, (CObject*&)pOutput ) ) ? pOutput : NULL;
}

int CLibraryFolder::GetFileCount() const
{
	return m_pFiles.GetCount();
}

int CLibraryFolder::GetFileList(CLibraryList* pList, BOOL bRecursive) const
{
	int nCount = 0;
	
	for ( POSITION pos = GetFileIterator() ; pos ; )
	{
		pList->CheckAndAdd( GetNextFile( pos )->m_nIndex );
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

		for ( int nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
		{
			CLibraryFolder* pFolder = new CLibraryFolder( this );
			pFolder->Serialize( ar, nVersion );

			m_pFolders.SetAt( pFolder->m_sNameLC, pFolder );

			m_nFiles	+= pFolder->m_nFiles;
			m_nVolume	+= pFolder->m_nVolume;
		}

		for ( int nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
		{
			CLibraryFile* pFile = new CLibraryFile( this );
			pFile->Serialize( ar, nVersion );

			m_pFiles.SetAt( pFile->GetNameLC(), pFile );

			m_nFiles	++;
			m_nVolume	+= pFile->m_nSize / 1024;
		}
	}
}

void CLibraryFolder::PathToName()
{
	m_sName = m_sPath;
	int nPos = m_sName.ReverseFind( '\\' );
	if ( nPos >= 0 && nPos < m_sName.GetLength() - 1 ) m_sName = m_sName.Mid( nPos + 1 );
	m_sNameLC = m_sName;
	CharLower( m_sNameLC.GetBuffer() );
	m_sNameLC.ReleaseBuffer();
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder threaded scan

BOOL CLibraryFolder::ThreadScan(DWORD nScanCookie)
{
	CSingleLock pLock( &Library.m_pSection );
	CString strPath, strMetaData;
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
	
	BOOL bKazaaFolder = _tcsistr( m_sPath, _T("kazaa") ) != NULL;
	BOOL bChanged = FALSE;
	
	if ( hSearch != INVALID_HANDLE_VALUE )
	{
		do
		{
			strPath.Format( _T("%s\\%s"), (LPCTSTR)m_sPath, pFind.cFileName );
			
			if ( pFind.cFileName[0] == '.' ) continue;
			if ( pFind.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM) ) continue;
			if ( _tcsicmp( pFind.cFileName, _T("Metadata") ) == 0 ) continue;
			if ( _tcsicmp( pFind.cFileName, _T("SThumbs.dat") ) == 0 ) continue;
			
			if ( pFind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				CLibraryFolder* pFolder = GetFolderByName( pFind.cFileName );
				
				if ( pFolder != NULL )
				{
					m_nFiles	-= pFolder->m_nFiles;
					m_nVolume	-= pFolder->m_nVolume;
					
					if ( pFolder->m_sName != pFind.cFileName )
					{
						pFolder->m_sPath = strPath;
						pFolder->PathToName();
						pFolder->m_nUpdateCookie++;
						bChanged = TRUE;
					}
				}
				else
				{
					pLock.Lock();
					pFolder = new CLibraryFolder( this, strPath );
					m_pFolders.SetAt( pFolder->m_sNameLC, pFolder );
					bChanged = TRUE;
					m_nUpdateCookie++;
					pLock.Unlock();
				}
				
				bChanged |= pFolder->ThreadScan( nScanCookie );
				
				m_nFiles	+= pFolder->m_nFiles;
				m_nVolume	+= pFolder->m_nVolume;
			}
			else
			{
				CLibraryFile* pFile = GetFile( pFind.cFileName );
				
				if ( pFile != NULL )
				{
					m_nVolume -= pFile->m_nSize / 1024;
					
					if ( pFile->m_sName != pFind.cFileName )
					{
						pFile->m_sName = pFind.cFileName;
						bChanged = TRUE;
					}
				}
				else if ( bKazaaFolder && _tcsistr( pFind.cFileName, _T(".dat") ) != NULL )
				{
					// Ignore .dat files in Kazaa folder
					continue;
				}
				else if ( _tcsnicmp( pFind.cFileName, _T("__INCOMPLETE___"), 15 ) == 0 )
				{
					// Ignore WinMX partial files
					continue;
				}
				else
				{
					pLock.Lock();
					pFile = new CLibraryFile( this, pFind.cFileName );
					m_pFiles.SetAt( pFile->GetNameLC(), pFile );
					m_nFiles++;
					bChanged = TRUE;
					pLock.Unlock();
				}
				
				QWORD nLongSize = (QWORD)pFind.nFileSizeLow | ( (QWORD)pFind.nFileSizeHigh << 32 );
				
				bChanged |= pFile->ThreadScan(	pLock, nScanCookie, nLongSize,
												&pFind.ftLastWriteTime, pszMetaData );
				
				m_nVolume += pFile->m_nSize / 1024;
			}
		}
		while ( Library.m_bThread && FindNextFile( hSearch, &pFind ) );
		
		FindClose( hSearch );
	}
	
	if ( ! Library.m_bThread ) return FALSE;
	
	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		CLibraryFolder* pFolder = GetNextFolder( pos );
		
		if ( pFolder->m_nScanCookie != nScanCookie )
		{
			CString strNameLC( pFolder->m_sName );
			CharLower( strNameLC.GetBuffer() );
			strNameLC.ReleaseBuffer();
			
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
			CharLower( strNameLC.GetBuffer() );
			strNameLC.ReleaseBuffer();
			
			m_nFiles	--;
			m_nVolume	-= pFile->m_nSize / 1024;
			
			if ( ! pLock.IsLocked() ) pLock.Lock();
			
			pFile->OnDelete();
			m_pFiles.RemoveKey( strNameLC );
			
			bChanged = TRUE;
		}
	}
	
	if ( m_pParent == NULL )
		theApp.Message( MSG_DEBUG, _T("Finnished scanning (%i)"), bChanged );
	
	return bChanged;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder monitor

BOOL CLibraryFolder::SetMonitor()
{
	if ( ! m_bMonitor )
	{
		m_bMonitor = TRUE;

		if ( m_hMonitor != INVALID_HANDLE_VALUE ) FindCloseChangeNotification( m_hMonitor );
		m_hMonitor = INVALID_HANDLE_VALUE;

		if ( ! Settings.Library.WatchFolders ) return FALSE;

		m_hMonitor = FindFirstChangeNotification( m_sPath, TRUE,
			FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_DIR_NAME|
			FILE_NOTIFY_CHANGE_SIZE|FILE_NOTIFY_CHANGE_LAST_WRITE );
	}

	return ( m_hMonitor != INVALID_HANDLE_VALUE );
}

BOOL CLibraryFolder::CheckMonitor()
{
	if ( ! m_bMonitor ) return TRUE;

	if ( m_hMonitor == INVALID_HANDLE_VALUE ) return FALSE;

	if ( WaitForSingleObject( m_hMonitor, 0 ) != WAIT_OBJECT_0 ) return FALSE;
	
	if ( ! FindNextChangeNotification( m_hMonitor ) )
	{
		FindCloseChangeNotification( m_hMonitor );
		m_hMonitor = INVALID_HANDLE_VALUE;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder scan

void CLibraryFolder::Scan()
{
    CLibraryFolder* pFolder = this;
	for ( ; pFolder->m_pParent ; pFolder = pFolder->m_pParent );
	if ( pFolder ) pFolder->m_bMonitor = FALSE;
	Library.m_pWakeup.SetEvent();
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder shared check

BOOL CLibraryFolder::IsShared()
{
	for ( CLibraryFolder* pFolder = this ; pFolder ; pFolder = pFolder->m_pParent )
	{
		if ( pFolder->m_bShared )
		{
			if ( pFolder->m_bShared == TS_TRUE ) return TRUE;
			if ( pFolder->m_bShared == TS_FALSE ) return FALSE;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolder callbacks

void CLibraryFolder::OnDelete()
{
	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		GetNextFolder( pos )->OnDelete();
	}
	
	for ( POSITION pos = GetFileIterator() ; pos ; )
	{
		GetNextFile( pos )->OnDelete();
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

		m_pFiles.GetNextAssoc( pos, strName, (CObject*&)pOld );

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

STDMETHODIMP CLibraryFolder::XLibraryFolders::get__NewEnum(IUnknown FAR* FAR* ppEnum)
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
	*pnCount = pThis->GetFolderCount();
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

STDMETHODIMP CLibraryFolder::XLibraryFiles::get__NewEnum(IUnknown FAR* FAR* ppEnum)
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
	*pnCount = pThis->GetFileCount();
	return S_OK;
}

