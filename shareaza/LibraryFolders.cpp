//
// LibraryFolders.cpp
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
#include "Library.h"
#include "LibraryMaps.h"
#include "LibraryFolders.h"
#include "SharedFile.h"
#include "SharedFolder.h"
#include "AlbumFolder.h"
#include "Application.h"
#include "CollectionFile.h"

#include "XML.h"
#include "Schema.h"
#include "SchemaCache.h"

IMPLEMENT_DYNAMIC(CLibraryFolders, CComObject)

BEGIN_INTERFACE_MAP(CLibraryFolders, CComObject)
	INTERFACE_PART(CLibraryFolders, IID_ILibraryFolders, LibraryFolders)
END_INTERFACE_MAP()

CLibraryFolders LibraryFolders;


//////////////////////////////////////////////////////////////////////
// CLibraryFolders construction

CLibraryFolders::CLibraryFolders()
{
	EnableDispatch( IID_ILibraryFolders );
	
	m_pAlbumRoot	= NULL;
	m_bRemoveMask	= FALSE;
}

CLibraryFolders::~CLibraryFolders()
{
	if ( m_pAlbumRoot != NULL ) delete m_pAlbumRoot;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders physical folder enumeration

POSITION CLibraryFolders::GetFolderIterator() const
{
	return m_pFolders.GetHeadPosition();
}

CLibraryFolder* CLibraryFolders::GetNextFolder(POSITION& pos) const
{
	return (CLibraryFolder*)m_pFolders.GetNext( pos );
}

int CLibraryFolders::GetFolderCount() const
{
	return m_pFolders.GetCount();
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders physical folder search

CLibraryFolder* CLibraryFolders::GetFolder(LPCTSTR pszPath) const
{
	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		CLibraryFolder* pFolder = GetNextFolder( pos )->GetFolderByPath( pszPath );
		if ( pFolder != NULL ) return pFolder;
	}
	
	return NULL;
}

BOOL CLibraryFolders::CheckFolder(CLibraryFolder* pFolder, BOOL bRecursive) const
{
	if ( m_pFolders.Find( pFolder ) != NULL ) return TRUE;
	if ( ! bRecursive ) return FALSE;
	
	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		if ( GetNextFolder( pos )->CheckFolder( pFolder, TRUE ) ) return TRUE;
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders add a root physical folder

CLibraryFolder* CLibraryFolders::AddFolder(LPCTSTR pszPath)
{
	CString strPath = pszPath;
	
	if ( strPath.GetLength() == 3 && strPath.GetAt( 2 ) == '\\' )
		strPath = strPath.Left( 2 );
	
	if ( IsFolderShared( strPath ) ) return NULL;
	
	Library.Lock();
	
	CLibraryFolder* pFolder = new CLibraryFolder( NULL, strPath );
	BOOL bAdded = FALSE;
	
	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		POSITION posAdd = pos;
		
		if ( GetNextFolder( pos )->m_sName.CompareNoCase( pFolder->m_sName ) >= 0 )
		{
			m_pFolders.InsertBefore( posAdd, pFolder );
			bAdded = TRUE;
			break;
		}
	}
	
	if ( ! bAdded ) m_pFolders.AddTail( pFolder );
	
	Library.Unlock( TRUE );
	Library.StartThread();
	
	return pFolder;
}

CLibraryFolder* CLibraryFolders::AddFolder(LPCTSTR pszPath, BOOL bShared)
{
	CLibraryFolder* pFolder = AddFolder( pszPath );

	if( pFolder )
	{
		if( bShared )
			pFolder->m_bShared = TS_TRUE;
		else
			pFolder->m_bShared = TS_FALSE;
	}

	return pFolder;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders remove a root physical folder

BOOL CLibraryFolders::RemoveFolder(CLibraryFolder* pFolder)
{
	CSingleLock pLock( &Library.m_pSection, TRUE );
	CWaitCursor pCursor;
	
	if ( m_bRemoveMask ) return FALSE;
	
	POSITION pos = m_pFolders.Find( pFolder );
	if ( pos == NULL ) return FALSE;
	
	pFolder->OnDelete();
	m_pFolders.RemoveAt( pos );
	
	Library.Lock();
	Library.Unlock( TRUE );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders check if a physical folder is shared

CLibraryFolder* CLibraryFolders::IsFolderShared(LPCTSTR pszPath)
{
	CString strPathLC( pszPath );
	strPathLC.MakeLower();
	
	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		CLibraryFolder* pFolder = GetNextFolder( pos );
		
		CString strOldLC( pFolder->m_sPath );
		strOldLC.MakeLower();
		
		if ( strPathLC.GetLength() >= strOldLC.GetLength() )
		{
			if ( strPathLC.Left( strOldLC.GetLength() ) == strOldLC ) return pFolder;
		}
		else if ( strPathLC.GetLength() < strOldLC.GetLength() )
		{
			if ( strOldLC.Left( strPathLC.GetLength() ) == strPathLC ) return pFolder;
		}
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders virtual album list access

CAlbumFolder* CLibraryFolders::GetAlbumRoot()
{
	if ( m_pAlbumRoot == NULL )
	{
		m_pAlbumRoot = new CAlbumFolder( NULL, CSchema::uriLibrary );
	}
	
	return m_pAlbumRoot;
}

BOOL CLibraryFolders::CheckAlbum(CAlbumFolder* pFolder) const
{
	if ( m_pAlbumRoot == NULL ) return FALSE;
	return m_pAlbumRoot->CheckFolder( pFolder, TRUE );
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders virtual album target search

CAlbumFolder* CLibraryFolders::GetAlbumTarget(LPCTSTR pszSchemaURI, LPCTSTR pszMember, LPCTSTR pszValue) const
{
	if ( m_pAlbumRoot == NULL ) return NULL;
	
	CSchema* pSchema = SchemaCache.Get( pszSchemaURI );
	if ( pSchema == NULL ) return NULL;
	
	CSchemaMember* pMember = pSchema->GetMember( pszMember );
	
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

CAlbumFolder* CLibraryFolders::GetCollection(SHA1* pSHA1)
{
	return GetAlbumRoot()->FindCollection( pSHA1 );
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders mount a collection

BOOL CLibraryFolders::MountCollection(SHA1* pSHA1, CCollectionFile* pCollection)
{
	CSingleLock pLock( &Library.m_pSection );
	BOOL bSuccess = FALSE;
	
	if ( ! pLock.Lock( 500 ) ) return FALSE;
	
	if ( pCollection->GetThisURI().GetLength() )
	{
		bSuccess |= GetAlbumRoot()->MountCollection( pSHA1, pCollection );
	}
	
	if ( pCollection->GetParentURI().GetLength() )
	{
		if ( CAlbumFolder* pFolder = GetAlbumTarget( pCollection->GetParentURI(), NULL, NULL ) )
		{
			bSuccess |= pFolder->MountCollection( pSHA1, pCollection, TRUE );
		}
	}
	
	return bSuccess;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders virtual album default tree

void CLibraryFolders::CreateAlbumTree()
{
	int nCount = m_pAlbumRoot->GetFolderCount();
	
	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriAllFiles ) == NULL )
	{
		CAlbumFolder* pAllFiles		= m_pAlbumRoot->AddFolder( CSchema::uriAllFiles );
	}
	
	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriApplicationRoot ) == NULL )
	{
		CAlbumFolder* pAppRoot		= m_pAlbumRoot->AddFolder( CSchema::uriApplicationRoot );
		CAlbumFolder* pAppAll		= pAppRoot->AddFolder( CSchema::uriApplicationAll );
	}
	
	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriBookRoot ) == NULL )
	{
		CAlbumFolder* pBookRoot		= m_pAlbumRoot->AddFolder( CSchema::uriBookRoot );
		CAlbumFolder* pBookAll		= pBookRoot->AddFolder( CSchema::uriBookAll );
	}
	
	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriImageRoot ) == NULL )
	{
		CAlbumFolder* pImageRoot	= m_pAlbumRoot->AddFolder( CSchema::uriImageRoot );
		CAlbumFolder* pImageAll		= pImageRoot->AddFolder( CSchema::uriImageAll );
	}
	
	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriMusicRoot ) == NULL )
	{
		CAlbumFolder* pMusicRoot	= m_pAlbumRoot->AddFolder( CSchema::uriMusicRoot );
		CAlbumFolder* pMusicAll		= pMusicRoot->AddFolder( CSchema::uriMusicAll );
		CAlbumFolder* pMusicAlbum	= pMusicRoot->AddFolder( CSchema::uriMusicAlbumCollection );
		CAlbumFolder* pMusicArtist	= pMusicRoot->AddFolder( CSchema::uriMusicArtistCollection );
		CAlbumFolder* pMusicGenre	= pMusicRoot->AddFolder( CSchema::uriMusicGenreCollection );
	}
	
	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriVideoRoot ) == NULL )
	{
		CAlbumFolder* pVideoRoot	= m_pAlbumRoot->AddFolder( CSchema::uriVideoRoot );
		CAlbumFolder* pVideoAll		= pVideoRoot->AddFolder( CSchema::uriVideoAll );
		CAlbumFolder* pVideoSeries	= pVideoRoot->AddFolder( CSchema::uriVideoSeriesCollection );
		CAlbumFolder* pVideoFilm	= pVideoRoot->AddFolder( CSchema::uriVideoFilmCollection );
		CAlbumFolder* pVideoMusic	= pVideoRoot->AddFolder( CSchema::uriVideoMusicCollection );
	}
	
	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriFavouritesFolder ) == NULL )
	{
		CAlbumFolder* pFavourites	= m_pAlbumRoot->AddFolder( CSchema::uriFavouritesFolder );
	}
	
	if ( m_pAlbumRoot->GetFolderByURI( CSchema::uriCollectionsFolder ) == NULL )
	{
		CAlbumFolder* pCollections	= m_pAlbumRoot->AddFolder( CSchema::uriCollectionsFolder );
	}
	
	if ( m_pAlbumRoot->GetFolderCount() != nCount )
	{
		for ( POSITION pos = LibraryMaps.GetFileIterator() ; pos ; )
		{
			CLibraryFile* pFile = LibraryMaps.GetNextFile( pos );
			if ( pFile->IsAvailable() ) m_pAlbumRoot->OrganiseFile( pFile );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders file delete handler

void CLibraryFolders::OnFileDelete(CLibraryFile* pFile)
{
	if ( m_pAlbumRoot != NULL ) m_pAlbumRoot->OnFileDelete( pFile );
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders clear

void CLibraryFolders::Clear()
{
	for ( POSITION pos = GetFolderIterator() ; pos ; )
	{
		delete GetNextFolder( pos );
	}
	
	m_pFolders.RemoveAll();
	
	if ( m_pAlbumRoot != NULL ) delete m_pAlbumRoot;
	m_pAlbumRoot = NULL;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders thread scan

BOOL CLibraryFolders::ThreadScan(BOOL* pbContinue, BOOL bForce)
{
	BOOL bChanged = FALSE;
	
	Library.Lock();
	m_bRemoveMask = TRUE;
	Library.Unlock();
	
	for ( POSITION pos = GetFolderIterator() ; pos && *pbContinue ; )
	{
		CLibraryFolder* pFolder = GetNextFolder( pos );
		
		if ( GetFileAttributes( pFolder->m_sPath ) != 0xFFFFFFFF )
		{
			if ( bForce || pFolder->CheckMonitor() )
			{
				if ( pFolder->ThreadScan() ) bChanged = TRUE;
			}
			
			pFolder->SetMonitor();
		}
	}
	
	Library.Lock();
	m_bRemoveMask = FALSE;
	Library.Unlock();
	
	return bChanged;
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders serialize

void CLibraryFolders::Serialize(CArchive& ar, int nVersion)
{
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
		for ( int nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
		{
			CLibraryFolder* pFolder = new CLibraryFolder( NULL );
			pFolder->Serialize( ar, nVersion );
			m_pFolders.AddTail( pFolder );
		}
	}
	
	if ( nVersion >= 6 ) GetAlbumRoot()->Serialize( ar, nVersion );
}

//////////////////////////////////////////////////////////////////////
// CLibraryFolders ILibraryFolders

IMPLEMENT_DISPATCH(CLibraryFolders, LibraryFolders)

STDMETHODIMP CLibraryFolders::XLibraryFolders::get_Application(IApplication FAR* FAR* ppApplication)
{
	METHOD_PROLOGUE( CLibraryFolders, LibraryFolders )
	*ppApplication = Application.GetApp();
	return S_OK;
}

STDMETHODIMP CLibraryFolders::XLibraryFolders::get_Library(ILibrary FAR* FAR* ppLibrary)
{
	METHOD_PROLOGUE( CLibraryFolders, LibraryFolders )
	*ppLibrary = (ILibrary*)Library.GetInterface( IID_ILibrary, TRUE );
	return S_OK;
}

STDMETHODIMP CLibraryFolders::XLibraryFolders::get__NewEnum(IUnknown FAR* FAR* ppEnum)
{
	METHOD_PROLOGUE( CLibraryFolders, LibraryFolders )
	return E_NOTIMPL;
}

STDMETHODIMP CLibraryFolders::XLibraryFolders::get_Item(VARIANT vIndex, ILibraryFolder FAR* FAR* ppFolder)
{
	METHOD_PROLOGUE( CLibraryFolders, LibraryFolders )

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
	*pnCount = pThis->GetFolderCount();
	return S_OK;
}
