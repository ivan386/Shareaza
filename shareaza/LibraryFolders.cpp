//
// LibraryFolders.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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

#include "LibraryFolders.h"

#include "AlbumFolder.h"
#include "Application.h"
#include "CollectionFile.h"
#include "DlgHelp.h"
#include "GProfile.h"
#include "Settings.h"
#include "Library.h"
#include "LibraryMaps.h"
#include "Schema.h"
#include "SchemaCache.h"
#include "SharedFile.h"
#include "SharedFolder.h"
#include "ShellIcons.h"
#include "Skin.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CLibraryFolders, CComObject)

BEGIN_INTERFACE_MAP(CLibraryFolders, CComObject)
	INTERFACE_PART(CLibraryFolders, IID_ILibraryFolders, LibraryFolders)
END_INTERFACE_MAP()

CLibraryFolders LibraryFolders;


//////////////////////////////////////////////////////////////////////
// CLibraryFolders construction

CLibraryFolders::CLibraryFolders() :
	m_pAlbumRoot( NULL )
{
	EnableDispatch( IID_ILibraryFolders );
}

CLibraryFolders::~CLibraryFolders()
{
	delete m_pAlbumRoot;
}

CXMLElement* CLibraryFolders::CreateXML(LPCTSTR szRoot, BOOL bSharedOnly, XmlType nType) const
{
	CXMLElement* pRoot;

	switch ( nType )
	{
	case xmlDC:
		pRoot = new CXMLElement( NULL, _T("FileListing") );
		if ( pRoot )
		{
			pRoot->AddAttribute( _T("Version"), 1 );
			pRoot->AddAttribute( _T("Base"), szRoot );
			pRoot->AddAttribute( _T("Generator"), Settings.SmartAgent() );
			Hashes::Guid oGUID( MyProfile.oGUID );
			pRoot->AddAttribute( _T("CID"), oGUID.toString< Hashes::base32Encoding >() );
		}
		break;

	default:
		pRoot = new CXMLElement( NULL, _T("folders") );
		if ( pRoot )
		{
			pRoot->AddAttribute( _T("xmlns"), CSchema::uriFolder );
		}
	}

	if ( ! pRoot )
		// Out of memory
		return NULL;

	CSingleLock oLock( &Library.m_pSection, TRUE );

	if ( _tcsicmp( szRoot, _T("/") ) == 0 )
	{
		// All folders
		for ( POSITION pos = LibraryFolders.GetFolderIterator() ; pos ; )
		{
			LibraryFolders.GetNextFolder( pos )->CreateXML( pRoot, bSharedOnly, nType );
		}
	}
	else if ( const CLibraryFolder* pFolder = LibraryFolders.GetFolderByName( szRoot ) )
	{
		// Specified folder
		pFolder->CreateXML( pRoot, bSharedOnly, nType );
	}

	return pRoot;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders physical folder enumeration

POSITION CLibraryFolders::GetFolderIterator() const
{
	ASSUME_LOCK( Library.m_pSection );

	return m_pFolders.GetHeadPosition();
}

CLibraryFolder* CLibraryFolders::GetNextFolder(POSITION& pos) const
{
	ASSUME_LOCK( Library.m_pSection );

	return m_pFolders.GetNext( pos );
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders physical folder search

CLibraryFolder* CLibraryFolders::GetFolder(const CString& strPath) const
{
	ASSUME_LOCK( Library.m_pSection );

	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		CLibraryFolder* pFolder = GetNextFolder( pos )->GetFolderByPath( strPath );
		if ( pFolder != NULL )
			return pFolder;
	}

	return NULL;
}

BOOL CLibraryFolders::CheckFolder(CLibraryFolder* pFolder, BOOL bRecursive) const
{
	ASSUME_LOCK( Library.m_pSection );

	if ( m_pFolders.Find( pFolder ) != NULL )
		return TRUE;

	if ( ! bRecursive )
		return FALSE;

	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		if ( GetNextFolder( pos )->CheckFolder( pFolder, TRUE ) )
			return TRUE;
	}

	return FALSE;
}

CLibraryFolder* CLibraryFolders::GetFolderByName(LPCTSTR pszName) const
{
	ASSUME_LOCK( Library.m_pSection );

	LPCTSTR szNextName = _tcschr( pszName, _T( '\\' ) );
	if ( szNextName )
	{
		CString strName( pszName, (int)( szNextName - pszName ) );

		for ( POSITION pos = GetFolderIterator(); pos; )
		{
			CLibraryFolder* pFolder = GetNextFolder( pos );

			if ( _tcsicmp( pFolder->m_sName, strName ) == 0 )
			{
				return pFolder->GetFolderByName( szNextName + 1 );
			}
		}
	}
	else
	{
		for ( POSITION pos = GetFolderIterator(); pos; )
		{
			CLibraryFolder* pFolder = GetNextFolder( pos );

			if ( _tcsicmp( pFolder->m_sName, pszName ) == 0 )
			{
				return pFolder;
			}
		}
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders add a root physical folder

CLibraryFolder* CLibraryFolders::AddFolder(LPCTSTR pszPath)
{
	CQuickLock oLock( Library.m_pSection );

	CString strPath = pszPath;

	if ( strPath.GetLength() == 3 && strPath.GetAt( 2 ) == '\\' )
		strPath = strPath.Left( 2 );

	if ( IsFolderShared( strPath ) )
		return NULL;

	if ( IsSubFolderShared( strPath ) )
		return NULL;

	CLibraryFolder* pFolder = new CLibraryFolder( NULL, strPath );
	if ( ! pFolder )
		return NULL;

	BOOL bAdded = FALSE;

	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		POSITION posAdd = pos;

		if ( _tcsicmp( GetNextFolder( pos )->m_sName, pFolder->m_sName ) >= 0 )
		{
			m_pFolders.InsertBefore( posAdd, pFolder );
			bAdded = TRUE;
			break;
		}
	}

	if ( ! bAdded ) m_pFolders.AddTail( pFolder );

	pFolder->Maintain( TRUE );

	Library.Update( true );

	Maintain();

	return pFolder;
}

CLibraryFolder* CLibraryFolders::AddFolder(LPCTSTR pszPath, BOOL bShared)
{
	CLibraryFolder* pFolder = AddFolder( pszPath );

	if( pFolder )
		pFolder->SetShared( bShared ? TRI_TRUE : TRI_FALSE );

	return pFolder;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders add a shared folder to a List Control

bool CLibraryFolders::AddSharedFolder(CListCtrl& oList)
{
	// Store the last path selected
	static CString strLastPath;
	if ( strLastPath.IsEmpty() )
		strLastPath = oList.GetItemText( 0, 0 );

	// Let user select a path to share
	CString strPath( BrowseForFolder( _T("Select folder to share:"), strLastPath ) );
	strPath.Trim();
	strPath.TrimRight( _T("\\") );
	const int nLength = strPath.GetLength();
	if ( ! nLength )
		return false;

	strLastPath = strPath;

	// Check if path is valid
	if ( ! IsShareable( strPath ) )
	{
		CHelpDlg::Show( _T( "ShareHelp.BadShare" ) );
		return false;
	}

	// Check if path is already shared
	bool bForceAdd = false;
	for ( int nItem = 0 ; nItem < oList.GetItemCount() ; ++nItem )
	{
		bool bSubFolder = false;
		const CString strOld = oList.GetItemText( nItem, 0 );
		const int nOldLength = strOld.GetLength();

		if ( nLength == nOldLength && strPath.CompareNoCase( strOld ) == 0 )
		{
			// Matches exactly
		}
		else if ( nLength > nOldLength )
		{
			if ( strPath.GetAt( nOldLength ) != _T('\\') ||
				 strPath.Left( nOldLength ).CompareNoCase( strOld ) != 0 )
				continue;
		}
		else if ( nLength < nOldLength )
		{
			bSubFolder = true;
			if ( strOld.GetAt( nLength ) != _T('\\') ||
				 strOld.Left( nLength ).CompareNoCase( strPath ) != 0 )
				continue;
		}
		else
		{
			continue;
		}

		if ( bSubFolder )
		{
			CString strMessage;
			strMessage.Format( LoadString( IDS_LIBRARY_SUBFOLDER_IN_LIBRARY ), (LPCTSTR)strPath );

			if ( bForceAdd || AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) == IDYES )
			{
				// Don't bother asking again- remove all sub-folders
				bForceAdd = true;

				// Remove the sub-folder
				oList.DeleteItem( nItem );
				--nItem;
			}
			else
			{
				return false;
			}
		}
		else
		{
			CString strMessage;
			strMessage.Format( LoadString( IDS_WIZARD_SHARE_ALREADY ), (LPCTSTR)strOld );
			AfxMessageBox( strMessage, MB_ICONINFORMATION );
			return false;
		}
	}

	//Add path to shared list
	oList.InsertItem( LVIF_TEXT|LVIF_IMAGE, oList.GetItemCount(), strPath, 0,
		0, SHI_FOLDER_OPEN, 0 );

	// Return success
	return true;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders remove a root physical folder

BOOL CLibraryFolders::RemoveFolder(CLibraryFolder* pFolder)
{
	CWaitCursor pCursor;
	CQuickLock pLock( Library.m_pSection );

	POSITION pos = m_pFolders.Find( pFolder );
	if ( pos == NULL ) return FALSE;

	pFolder->Maintain( FALSE );

	pFolder->OnDelete( Settings.Library.CreateGhosts ? TRI_TRUE : TRI_FALSE );
	m_pFolders.RemoveAt( pos );

	Library.Update( true );

	Maintain();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders check if a physical folder is part of the library

CLibraryFolder* CLibraryFolders::IsFolderShared(const CString& strPath) const
{
	CQuickLock oLock( Library.m_pSection );

	const int nLength = strPath.GetLength();

	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		CLibraryFolder* pFolder = GetNextFolder( pos );

		const int nOldLength = pFolder->m_sPath.GetLength();
		if ( nLength > nOldLength )
		{
			if ( strPath.GetAt( nOldLength ) == _T('\\') &&
				 strPath.Left( nOldLength ).CompareNoCase( pFolder->m_sPath ) == 0 )
				return pFolder;
		}
		else
		{
			if ( nLength == nOldLength && strPath.CompareNoCase( pFolder->m_sPath ) == 0 )
				return pFolder;
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders check if a subfolder of a physical folder is part of the library

CLibraryFolder* CLibraryFolders::IsSubFolderShared(const CString& strPath) const
{
	CQuickLock oLock( Library.m_pSection );

	const int nLength = strPath.GetLength();

	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		CLibraryFolder* pFolder = GetNextFolder( pos );

		const int nOldLength = pFolder->m_sPath.GetLength();
		if ( nLength < nOldLength )
		{
			if ( pFolder->m_sPath.GetAt( nLength ) == _T('\\') &&
				 pFolder->m_sPath.Left( nLength ).CompareNoCase( strPath ) == 0 )
				return pFolder;
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders check if folder is not a system directory, incomplete folder etc...

bool CLibraryFolders::IsShareable(const CString& strPath)
{
	if ( strPath.IsEmpty() )
		return false;

	// Get system paths (to compare)

	const CString sWindows = theApp.GetWindowsFolder();
	if ( _tcsnicmp( sWindows, strPath, strPath.GetLength() ) == 0 )
		return false;

	const CString sProgramFiles64 = theApp.GetProgramFilesFolder64();
	if ( _tcsnicmp( sProgramFiles64, strPath, strPath.GetLength() ) == 0 )
		return false;

	const CString sProgramFiles = theApp.GetProgramFilesFolder();
	if ( _tcsnicmp( sProgramFiles, strPath, strPath.GetLength() ) == 0 )
		return false;

	// Get various Shareaza paths (to compare)

	if ( _tcsnicmp( strPath, Settings.General.Path, Settings.General.Path.GetLength() ) == 0 )
		return false;
	if ( _tcsnicmp( Settings.General.Path, strPath, strPath.GetLength() ) == 0 )
		return false;

	if ( _tcsnicmp( Settings.Downloads.IncompletePath, strPath, strPath.GetLength() ) == 0 )
		return false;

	if ( _tcsicmp( strPath, Settings.General.UserPath ) == 0 )
		return false;
	if ( _tcsicmp( strPath, Settings.General.UserPath + _T("\\Data") ) == 0 )
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders virtual album list access

CAlbumFolder* CLibraryFolders::GetAlbumRoot() const
{
	ASSUME_LOCK( Library.m_pSection );

	return m_pAlbumRoot;
}

BOOL CLibraryFolders::CheckAlbum(CAlbumFolder* pFolder) const
{
	ASSUME_LOCK( Library.m_pSection );

	return m_pAlbumRoot && m_pAlbumRoot->CheckFolder( pFolder, TRUE );
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders virtual album target search

CAlbumFolder* CLibraryFolders::GetAlbumTarget(LPCTSTR pszSchemaURI, LPCTSTR pszMember, LPCTSTR pszValue) const
{
	ASSUME_LOCK( Library.m_pSection );

	if ( m_pAlbumRoot == NULL ) return NULL;

	CSchemaPtr pSchema = SchemaCache.Get( pszSchemaURI );
	if ( pSchema == NULL ) return NULL;

	CSchemaMemberPtr pMember = pSchema->GetMember( pszMember );

	if ( pMember == NULL )
	{
		if ( pSchema->GetMemberCount() == 0 ) return NULL;
		POSITION pos = pSchema->GetMemberIterator();
		pMember = pSchema->GetNextMember( pos );
	}

	if ( pszValue != NULL )
	{
		CString strValue( pszValue );
		CXMLNode::UniformString( strValue );
		return m_pAlbumRoot->GetTarget( pMember, strValue );
	}
	else
	{
		return m_pAlbumRoot->GetTarget( pMember, NULL );
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders virtual album collection search

CAlbumFolder* CLibraryFolders::GetCollection(const Hashes::Sha1Hash& oSHA1)
{
	ASSUME_LOCK( Library.m_pSection );

	return m_pAlbumRoot ? m_pAlbumRoot->FindCollection( oSHA1 ) : NULL;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders mount a collection

BOOL CLibraryFolders::MountCollection(const Hashes::Sha1Hash& oSHA1, CCollectionFile* pCollection)
{
	CSingleLock pLock( &Library.m_pSection );
	BOOL bSuccess = FALSE;

	if ( ! pLock.Lock( 500 ) ) return FALSE;

	if ( pCollection->GetThisURI().GetLength() )
	{
		bSuccess |= m_pAlbumRoot->MountCollection( oSHA1, pCollection );
	}

/*
	if ( pCollection->GetParentURI().GetLength() )
	{
		if ( CAlbumFolder* pFolder = GetAlbumTarget( pCollection->GetParentURI(), NULL, NULL ) )
		{
			bSuccess |= pFolder->MountCollection( oSHA1, pCollection, TRUE );
		}
	}
*/
	return bSuccess;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders virtual album default tree

CAlbumFolder* CLibraryFolders::CreateAlbumTree()
{
	ASSUME_LOCK( Library.m_pSection );

	if ( m_pAlbumRoot == NULL )
		m_pAlbumRoot = new CAlbumFolder( NULL, CSchema::uriLibrary );

	DWORD nCount = m_pAlbumRoot->GetFolderCount();

	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriAllFiles ) == NULL )
	{
		/*CAlbumFolder* pAllFiles		=*/ m_pAlbumRoot->AddFolder( CSchema::uriAllFiles );
	}

	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriApplicationRoot ) == NULL )
	{
		CAlbumFolder* pAppRoot		= m_pAlbumRoot->AddFolder( CSchema::uriApplicationRoot );
		/*CAlbumFolder* pAppAll		=*/ pAppRoot->AddFolder( CSchema::uriApplicationAll );
	}

	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriBookRoot ) == NULL )
	{
		CAlbumFolder* pBookRoot		= m_pAlbumRoot->AddFolder( CSchema::uriBookRoot );
		/*CAlbumFolder* pBookAll		=*/ pBookRoot->AddFolder( CSchema::uriBookAll );
	}

	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriImageRoot ) == NULL )
	{
		CAlbumFolder* pImageRoot	= m_pAlbumRoot->AddFolder( CSchema::uriImageRoot );
		/*CAlbumFolder* pImageAll		=*/ pImageRoot->AddFolder( CSchema::uriImageAll );
	}

	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriMusicRoot ) == NULL )
	{
		CAlbumFolder* pMusicRoot	= m_pAlbumRoot->AddFolder( CSchema::uriMusicRoot );
		/*CAlbumFolder* pMusicAll		=*/ pMusicRoot->AddFolder( CSchema::uriMusicAll );
		/*CAlbumFolder* pMusicAlbum	=*/ pMusicRoot->AddFolder( CSchema::uriMusicAlbumCollection );
		/*CAlbumFolder* pMusicArtist	=*/ pMusicRoot->AddFolder( CSchema::uriMusicArtistCollection );
		/*CAlbumFolder* pMusicGenre	=*/ pMusicRoot->AddFolder( CSchema::uriMusicGenreCollection );
	}

	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriVideoRoot ) == NULL )
	{
		CAlbumFolder* pVideoRoot	= m_pAlbumRoot->AddFolder( CSchema::uriVideoRoot );
		/*CAlbumFolder* pVideoAll		=*/ pVideoRoot->AddFolder( CSchema::uriVideoAll );
		/*CAlbumFolder* pVideoSeries	=*/ pVideoRoot->AddFolder( CSchema::uriVideoSeriesCollection );
		/*CAlbumFolder* pVideoFilm	=*/ pVideoRoot->AddFolder( CSchema::uriVideoFilmCollection );
		/*CAlbumFolder* pVideoMusic	=*/ pVideoRoot->AddFolder( CSchema::uriVideoMusicCollection );
	}

	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriFavouritesFolder ) == NULL )
	{
		/*CAlbumFolder* pFavourites	=*/ m_pAlbumRoot->AddFolder( CSchema::uriFavouritesFolder );
	}

	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriCollectionsFolder ) == NULL )
	{
		/*CAlbumFolder* pCollections	=*/ m_pAlbumRoot->AddFolder( CSchema::uriCollectionsFolder );
	}

	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriDocumentRoot ) == NULL )
	{
		CAlbumFolder* pDocumentRoot		= m_pAlbumRoot->AddFolder( CSchema::uriDocumentRoot );
		/*CAlbumFolder* pDocumentAll		=*/ pDocumentRoot->AddFolder( CSchema::uriDocumentAll );
	}

	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriGhostFolder ) == NULL )
	{
		/*CAlbumFolder* pGhostFolder	=*/ m_pAlbumRoot->AddFolder( CSchema::uriGhostFolder );
	}

	if ( m_pAlbumRoot->GetFolderCount() != nCount )
	{
		for ( POSITION pos = LibraryMaps.GetFileIterator() ; pos ; )
		{
			CLibraryFile* pFile = LibraryMaps.GetNextFile( pos );
			if ( pFile->IsAvailable() ) m_pAlbumRoot->OrganiseFile( pFile );
		}
	}

	return m_pAlbumRoot;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders file delete handler

BOOL CLibraryFolders::OnFileDelete(CLibraryFile* pFile, BOOL bDeleteGhost)
{
	ASSUME_LOCK( Library.m_pSection );

	if ( m_pAlbumRoot )
		m_pAlbumRoot->OnFileDelete( pFile, bDeleteGhost );

	for ( POSITION pos = GetFolderIterator() ; pos ; )
		if ( GetNextFolder( pos )->OnFileDelete( pFile ) )
			return TRUE;

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders clear

void CLibraryFolders::Clear()
{
	ASSUME_LOCK( Library.m_pSection );

	delete m_pAlbumRoot;
	m_pAlbumRoot = NULL;

	for ( POSITION pos = GetFolderIterator() ; pos ; )
		delete GetNextFolder( pos );
	m_pFolders.RemoveAll();
}

void CLibraryFolders::ClearGhosts(BOOL bAll)
{
	ASSUME_LOCK( Library.m_pSection );

	if ( m_pAlbumRoot )
	{
		if ( CAlbumFolder* pGhosts = m_pAlbumRoot->GetFolderByURI( CSchema::uriGhostFolder ) )
		{
			const DWORD nLimit = bAll ? 0 : Settings.Library.GhostLimit;

			if ( pGhosts->GetFileCount() > nLimit )
			{
				std::list< CLibraryFile* > pList;

				for ( POSITION pos = pGhosts->GetFileIterator(); pos; )
				{
					CLibraryFile* pFile = pGhosts->GetNextFile( pos );
					ASSERT( !pFile->IsAvailable() );
					pList.push_back( pFile );
				}

				if ( ! bAll )
				{
					pList.sort( Earlier() );
				}

				while ( pList.size() > nLimit )
				{
					CLibraryFile* pFile = pList.front();
					pList.pop_front();
#ifdef _DEBUG
					CString strDate, strTime;
					SYSTEMTIME pTime;
					FileTimeToSystemTime( &pFile->m_pTime, &pTime );
					SystemTimeToTzSpecificLocalTime( NULL, &pTime, &pTime );
					GetDateFormat( LOCALE_USER_DEFAULT, DATE_LONGDATE, &pTime, NULL, strDate.GetBuffer( 64 ), 64 );
					GetTimeFormat( LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT, &pTime, NULL, strTime.GetBuffer( 64 ), 64 );
					strDate.ReleaseBuffer();
					strTime.ReleaseBuffer();
					TRACE( "Removed extra ghost file \"%s\" %s %s\n", (LPCSTR)CT2A( (LPCTSTR)pFile->m_sName ), (LPCSTR)CT2A( (LPCTSTR)strDate ), (LPCSTR)CT2A( (LPCTSTR)strTime ) );
#endif
					pFile->Delete( TRUE );
				}
			}
		}
	}
}

DWORD CLibraryFolders::GetGhostCount() const
{
	CQuickLock oLock( Library.m_pSection );

	if ( m_pAlbumRoot )
	{
		if ( CAlbumFolder* pGhosts = m_pAlbumRoot->GetFolderByURI( CSchema::uriGhostFolder ) )
		{
			return pGhosts->GetFileCount();
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders thread scan

BOOL CLibraryFolders::ThreadScan(const BOOL bForce)
{
	ASSUME_LOCK( Library.m_pSection );

	BOOL bChanged = FALSE;

	for ( POSITION pos = GetFolderIterator() ; pos && Library.IsThreadEnabled() ; )
	{
		CLibraryFolder* pFolder = GetNextFolder( pos );

		if ( GetFileAttributes( CString( _T("\\\\?\\") ) + pFolder->m_sPath ) != INVALID_FILE_ATTRIBUTES )
		{
			if ( pFolder->SetOnline() ) bChanged = TRUE;

			if ( pFolder->IsChanged() || bForce )
			{
				if ( pFolder->ThreadScan() ) bChanged = TRUE;
			}
		}
		else
			if ( pFolder->SetOffline() ) bChanged = TRUE;
	}

	return bChanged;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders serialize

void CLibraryFolders::Serialize(CArchive& ar, int nVersion)
{
	ASSUME_LOCK( Library.m_pSection );

	if ( m_pAlbumRoot == NULL )
		m_pAlbumRoot = new CAlbumFolder( NULL, CSchema::uriLibrary );

	if ( ar.IsStoring() )
	{
		ar.WriteCount( GetFolderCount() );

		for ( POSITION pos = GetFolderIterator() ; pos ; )
		{
			GetNextFolder( pos )->Serialize( ar, nVersion );
		}
	}
	else
	{
		for ( DWORD_PTR nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
		{
			CLibraryFolder* pFolder = new CLibraryFolder( NULL );
			pFolder->Serialize( ar, nVersion );
			m_pFolders.AddTail( pFolder );
		}
	}

	if ( nVersion >= 6 )
		m_pAlbumRoot->Serialize( ar, nVersion );
}

void CLibraryFolders::Maintain()
{
	CQuickLock oLock( Library.m_pSection );

	CComPtr< IShellLibrary > pIShellLib;
	if ( theApp.m_bIs7OrNewer && Settings.Library.UseWindowsLibrary )
		pIShellLib.CoCreateInstance( CLSID_ShellLibrary );

	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		CLibraryFolder* pFolder = GetNextFolder( pos );

		pFolder->Maintain( TRUE );

		if ( pIShellLib && theApp.m_pfnSHCreateItemFromParsingName )
		{
			CComPtr< IShellItem > psiFolder;
			theApp.m_pfnSHCreateItemFromParsingName( (LPCWSTR)CT2W( pFolder->m_sPath ), NULL, IID_PPV_ARGS( &psiFolder ) );
			if ( psiFolder )
				pIShellLib->AddFolder( psiFolder );
		}
	}

	if ( pIShellLib )
	{
		pIShellLib->SetIcon( (LPCWSTR)CT2W( Skin.GetImagePath( IDR_LIBRARYFRAME ) ) );

		CComPtr< IShellItem > psiLibrary;
		pIShellLib->SaveInKnownFolder( FOLDERID_UsersLibraries, CLIENT_NAME_T, LSF_OVERRIDEEXISTING, &psiLibrary );
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders ILibraryFolders

IMPLEMENT_DISPATCH(CLibraryFolders, LibraryFolders)

STDMETHODIMP CLibraryFolders::XLibraryFolders::get_Application(IApplication FAR* FAR* ppApplication)
{
	METHOD_PROLOGUE( CLibraryFolders, LibraryFolders )
	return CApplication::GetApp( ppApplication );
}

STDMETHODIMP CLibraryFolders::XLibraryFolders::get_Library(ILibrary FAR* FAR* ppLibrary)
{
	METHOD_PROLOGUE( CLibraryFolders, LibraryFolders )
	*ppLibrary = (ILibrary*)Library.GetInterface( IID_ILibrary, TRUE );
	return S_OK;
}

STDMETHODIMP CLibraryFolders::XLibraryFolders::get__NewEnum(IUnknown FAR* FAR* /*ppEnum*/)
{
	METHOD_PROLOGUE( CLibraryFolders, LibraryFolders )
	return E_NOTIMPL;
}

STDMETHODIMP CLibraryFolders::XLibraryFolders::get_Item(VARIANT vIndex, ILibraryFolder FAR* FAR* ppFolder)
{
	METHOD_PROLOGUE( CLibraryFolders, LibraryFolders )

	CQuickLock oLock( Library.m_pSection );

	CLibraryFolder* pFolder = NULL;
	*ppFolder = NULL;

	if ( vIndex.vt == VT_BSTR )
	{
		CString strName( vIndex.bstrVal );
		pFolder = pThis->GetFolder( strName );
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

STDMETHODIMP CLibraryFolders::XLibraryFolders::get_Count(LONG FAR* pnCount)
{
	METHOD_PROLOGUE( CLibraryFolders, LibraryFolders )

	CQuickLock oLock( Library.m_pSection );

	*pnCount = static_cast< LONG >( pThis->GetFolderCount() );

	return S_OK;
}
