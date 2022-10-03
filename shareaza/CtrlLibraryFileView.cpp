//
// CtrlLibraryFileView.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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
#include "LibraryBuilder.h"
#include "LibraryFolders.h"
#include "SharedFolder.h"
#include "SharedFile.h"
#include "AlbumFolder.h"
#include "FileExecutor.h"
#include "CoolInterface.h"
#include "Skin.h"
#include "CtrlLibraryFrame.h"
#include "CtrlLibraryFileView.h"
#include "CtrlLibraryTip.h"
#include "Shell.h"
#include "DlgFilePropertiesSheet.h"
#include "DlgFileCopy.h"
#include "DlgURLCopy.h"
#include "DlgURLExport.h"
#include "DlgDeleteFile.h"
#include "DlgDecodeMetadata.h"
#include "RelatedSearch.h"
#include "Security.h"
#include "Schema.h"
#include "XML.h"
#include "Transfers.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CLibraryFileView, CLibraryView)

BEGIN_MESSAGE_MAP(CLibraryFileView, CLibraryView)
	ON_WM_CONTEXTMENU()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_LAUNCH, OnUpdateLibraryLaunch)
	ON_COMMAND(ID_LIBRARY_LAUNCH, OnLibraryLaunch)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_ENQUEUE, OnUpdateLibraryEnqueue)
	ON_COMMAND(ID_LIBRARY_ENQUEUE, OnLibraryEnqueue)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_URL, OnUpdateLibraryURL)
	ON_COMMAND(ID_LIBRARY_URL, OnLibraryURL)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_MOVE, OnUpdateLibraryMove)
	ON_COMMAND(ID_LIBRARY_MOVE, OnLibraryMove)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_COPY, OnUpdateLibraryCopy)
	ON_COMMAND(ID_LIBRARY_COPY, OnLibraryCopy)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_DELETE, OnUpdateLibraryDelete)
	ON_COMMAND(ID_LIBRARY_DELETE, OnLibraryDelete)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_REFRESH_METADATA, OnUpdateLibraryRefreshMetadata)
	ON_COMMAND(ID_LIBRARY_REFRESH_METADATA, OnLibraryRefreshMetadata)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_SHARED_FILE, OnUpdateLibraryShared)
	ON_COMMAND(ID_LIBRARY_SHARED_FILE, OnLibraryShared)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_PROPERTIES, OnUpdateLibraryProperties)
	ON_COMMAND(ID_LIBRARY_PROPERTIES, OnLibraryProperties)
	ON_UPDATE_COMMAND_UI(ID_WEBSERVICES_MUSICBRAINZ, OnUpdateMusicBrainzLookup)
	ON_COMMAND(ID_WEBSERVICES_MUSICBRAINZ, OnMusicBrainzLookup)
	ON_UPDATE_COMMAND_UI(ID_MUSICBRAINZ_MATCHES, OnUpdateMusicBrainzMatches)
	ON_COMMAND(ID_MUSICBRAINZ_MATCHES, OnMusicBrainzMatches)
	ON_UPDATE_COMMAND_UI(ID_MUSICBRAINZ_ALBUMS, OnUpdateMusicBrainzAlbums)
	ON_COMMAND(ID_MUSICBRAINZ_ALBUMS, OnMusicBrainzAlbums)
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_UNLINK, OnUpdateLibraryUnlink)
	ON_COMMAND(ID_LIBRARY_UNLINK, OnLibraryUnlink)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_FOR_THIS, OnUpdateSearchForThis)
	ON_COMMAND(ID_SEARCH_FOR_THIS, OnSearchForThis)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_FOR_SIMILAR, OnUpdateSearchForSimilar)
	ON_COMMAND(ID_SEARCH_FOR_SIMILAR, OnSearchForSimilar)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_FOR_ARTIST, OnUpdateSearchForArtist)
	ON_COMMAND(ID_SEARCH_FOR_ARTIST, OnSearchForArtist)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_FOR_ALBUM, OnUpdateSearchForAlbum)
	ON_COMMAND(ID_SEARCH_FOR_ALBUM, OnSearchForAlbum)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_FOR_SERIES, OnUpdateSearchForSeries)
	ON_COMMAND(ID_SEARCH_FOR_SERIES, OnSearchForSeries)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_CREATETORRENT, OnUpdateLibraryCreateTorrent)
	ON_COMMAND(ID_LIBRARY_CREATETORRENT, OnLibraryCreateTorrent)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_REBUILD_ANSI, OnUpdateLibraryRebuildAnsi)
	ON_COMMAND(ID_LIBRARY_REBUILD_ANSI, OnLibraryRebuildAnsi)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_FILE_REBUILD, OnUpdateLibraryRebuild)
	ON_COMMAND(ID_LIBRARY_FILE_REBUILD, OnLibraryRebuild)
	ON_MESSAGE(WM_METADATA, OnServiceDone)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLibraryFileView construction

CLibraryFileView::CLibraryFileView()
: m_bRequestingService( FALSE )
, m_bServiceFailed( FALSE )
, m_nCurrentPage( 0 )
{
	m_pszToolBar = L"CLibraryFileView";
}

CLibraryFileView::~CLibraryFileView()
{
	for ( POSITION pos = m_pServiceDataPages.GetHeadPosition() ; pos ; )
	{
		delete m_pServiceDataPages.GetNext( pos );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryFileView selected item interface

BOOL CLibraryFileView::CheckAvailable(CLibraryTreeItem* pSel)
{
	m_bAvailable = FALSE;
	if ( pSel == NULL ) return m_bAvailable;
	m_bAvailable = TRUE;

	if ( pSel->m_pSelNext == NULL && pSel->m_pVirtual != NULL )
	{
		m_bAvailable = ( pSel->m_pVirtual->GetFileCount() > 0 );
	}

	return m_bAvailable;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryFileView message handlers

int CLibraryFileView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CLibraryView::OnCreate( lpCreateStruct ) == -1 ) return -1;
	m_bEditing = FALSE;
	return 0;
}

BOOL CLibraryFileView::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN && ! m_bEditing )
	{
		switch ( pMsg->wParam )
		{
		case VK_RETURN:
			OnLibraryLaunch();
			return TRUE;
		case VK_DELETE:
			OnLibraryDelete();
			return TRUE;
		}
	}
	else if ( pMsg->message == WM_SYSKEYDOWN && pMsg->wParam == VK_RETURN )
	{
		OnLibraryProperties();
		return TRUE;
	}

	return CLibraryView::PreTranslateMessage( pMsg );
}

void CLibraryFileView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	GetToolTip()->Hide();

	CStringList oFiles;
	{
		CQuickLock pLock( Library.m_pSection );
		POSITION posSel = StartSelectedFileLoop();
		while ( CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
		{
			oFiles.AddTail( pFile->GetPath() );
		}
	}

	// If no files were selected then try folder itself
	if ( oFiles.GetCount() == 0 )
		if ( CLibraryTreeItem* pRoot = GetFolderSelection() )
			if ( pRoot->m_pPhysical )
				oFiles.AddTail( pRoot->m_pPhysical->m_sPath );

	Skin.TrackPopupMenu( CString( m_pszToolBar ) + ( Settings.Library.ShowVirtual ? _T(".Virtual") : _T(".Physical") ), point, ID_LIBRARY_LAUNCH, oFiles );
}

void CLibraryFileView::OnMouseMove(UINT nFlags, CPoint point)
{
	CLibraryView::OnMouseMove( nFlags, point );

	if ( DWORD nFile = (DWORD)HitTestIndex( point ) )
	{
		GetToolTip()->Show( nFile );
	}
	else
	{
		GetToolTip()->Hide();
	}
}

void CLibraryFileView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CLibraryView::OnKeyDown( nChar, nRepCnt, nFlags );

	CheckDynamicBar();
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryFileView command handlers

void CLibraryFileView::OnUpdateLibraryLaunch(CCmdUI* pCmdUI)
{
	if ( m_bGhostFolder )
		pCmdUI->Enable( FALSE );
	else
		pCmdUI->Enable( GetSelectedCount() > 0 );
}

void CLibraryFileView::OnLibraryLaunch()
{
	GetToolTip()->Hide();

	CStringList oList;

	{
		CSingleLock oLock( &Library.m_pSection );
		if ( !oLock.Lock( 250 ) ) return;

		POSITION posSel = StartSelectedFileLoop();
		while ( CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
		{
			oList.AddTail( pFile->GetPath() );
		}
	}

	CFileExecutor::Execute( oList );
}

void CLibraryFileView::OnUpdateLibraryEnqueue(CCmdUI* pCmdUI)
{
	if ( m_bGhostFolder )
		pCmdUI->Enable( FALSE );
	else
		pCmdUI->Enable( GetSelectedCount() > 0 );
}

void CLibraryFileView::OnLibraryEnqueue()
{
	CStringList pList;

	{
		CSingleLock oLock( &Library.m_pSection );
		if ( !oLock.Lock( 250 ) ) return;

		POSITION posSel = StartSelectedFileLoop();
		while ( CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
		{
			pList.AddTail( pFile->GetPath() );
		}
	}

	CFileExecutor::Enqueue( pList );
}

void CLibraryFileView::OnUpdateLibraryURL(CCmdUI* pCmdUI)
{
	const bool bShift = ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) != 0;

	pCmdUI->Enable( GetSelectedCount() > 0 );
	pCmdUI->SetText( LoadString( ( GetSelectedCount() == 1 && ! bShift ) ? IDS_LIBRARY_COPYURI : IDS_LIBRARY_EXPORTURIS ) );
}

void CLibraryFileView::OnLibraryURL()
{
	const bool bShift = ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) != 0;

	CSingleLock pLock( &Library.m_pSection, TRUE );

	if ( GetSelectedCount() == 1 && ! bShift )
	{
		const CLibraryFile* pFile = GetSelectedFile();
		if ( ! pFile ) return;

		CURLCopyDlg dlg;

		dlg.Add( pFile );

		pLock.Unlock();

		dlg.DoModal();
	}
	else if ( GetSelectedCount() > 0 )
	{
		CURLExportDlg dlg;

		POSITION posSel = StartSelectedFileLoop();

		while ( const CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
		{
			dlg.Add( pFile );
		}

		pLock.Unlock();

		dlg.DoModal();
	}
}

void CLibraryFileView::OnUpdateLibraryMove(CCmdUI* pCmdUI)
{
	if ( m_bGhostFolder )
		pCmdUI->Enable( FALSE );
	else
		pCmdUI->Enable( GetSelectedCount() > 0 );
}

void CLibraryFileView::OnLibraryMove()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );
	CFileCopyDlg dlg( NULL, TRUE );

	POSITION posSel = StartSelectedFileLoop();

	while ( CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
	{
		dlg.AddFile( pFile );
	}

	pLock.Unlock();

	dlg.DoModal();
}

void CLibraryFileView::OnUpdateLibraryCopy(CCmdUI* pCmdUI)
{
	if ( m_bGhostFolder )
		pCmdUI->Enable( FALSE );
	else
		pCmdUI->Enable( GetSelectedCount() > 0 );
}

void CLibraryFileView::OnLibraryCopy()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );
	CFileCopyDlg dlg( NULL, FALSE );

	POSITION posSel = StartSelectedFileLoop();

	while ( CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
	{
		dlg.AddFile( pFile );
	}

	pLock.Unlock();

	dlg.DoModal();
}

void CLibraryFileView::OnUpdateLibraryDelete(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( GetSelectedCount() > 0 );
}

void CLibraryFileView::OnLibraryDelete()
{
	CStringList pList;
	{
		CSingleLock pLibraryLock( &Library.m_pSection, TRUE );

		POSITION posSel = StartSelectedFileLoop();
		while ( CLibraryFile* pFile = GetNextSelectedFile( posSel, FALSE, ! m_bGhostFolder ) )
		{
			if ( m_bGhostFolder )
				pFile->Delete( TRUE );
			else
				pList.AddTail( pFile->GetPath() );
		}
	}

	DeleteFiles( pList );

	Library.Update( true );
}

void CLibraryFileView::OnUpdateLibraryCreateTorrent(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( ! m_bGhostFolder && Settings.BitTorrent.TorrentCreatorPath.GetLength() && GetSelectedCount() == 1 );
}

void CLibraryFileView::OnLibraryCreateTorrent()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	if ( CLibraryFile* pFile = GetSelectedFile() )
	{
		CString sPath = pFile->GetPath();
		pLock.Unlock();

		if ( sPath.GetLength() > 0 )
		{
			CString sCommandLine = _T(" -sourcefile \"") + sPath +
				_T("\" -destination \"") + Settings.Downloads.TorrentPath +
				_T("\" -tracker \"" + Settings.BitTorrent.DefaultTracker +
				_T("\"") );

			ShellExecute( GetSafeHwnd(), _T("open"),
				Settings.BitTorrent.TorrentCreatorPath, sCommandLine,
				Settings.Downloads.TorrentPath,
				SW_SHOWNORMAL );
		}

	}
}

void CLibraryFileView::OnUpdateLibraryRebuildAnsi(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( ! m_bGhostFolder && GetSelectedCount() > 0 );
}

void CLibraryFileView::OnLibraryRebuildAnsi()
{
	CDecodeMetadataDlg dlg;

	CSingleLock pLock( &Library.m_pSection, TRUE );

	POSITION posSel = StartSelectedFileLoop();

	while ( CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
	{
		dlg.AddFile( pFile );
	}

	pLock.Unlock();

	if ( dlg.m_pFiles.GetCount() ) dlg.DoModal();
}


void CLibraryFileView::OnUpdateLibraryRebuild(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( ! m_bGhostFolder && GetSelectedCount() > 0 );
}

void CLibraryFileView::OnLibraryRebuild()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	POSITION posSel = StartSelectedFileLoop();

	while ( CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
	{
		pFile->Rebuild();
	}

	Library.Update( true );
}

void CLibraryFileView::OnUpdateLibraryRefreshMetadata(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( ! m_bGhostFolder && GetSelectedCount() > 0 );
}

void CLibraryFileView::OnLibraryRefreshMetadata()
{
	CProgressDialog dlgProgress( LoadString( ID_LIBRARY_REFRESH_METADATA ) + _T("...") );

	CQuickLock pLock( Library.m_pSection );

	DWORD nCompleted = 0, nTotal = (DWORD)GetSelectedCount();

	POSITION posSel = StartSelectedFileLoop();
	while ( CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
	{
		CString sPath = pFile->GetPath();

		dlgProgress.Progress( sPath, nCompleted++, nTotal );

		LibraryBuilder.RefreshMetadata( sPath );
	}
}

void CLibraryFileView::OnUpdateLibraryProperties(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( GetSelectedCount() > 0 );
}

void CLibraryFileView::OnLibraryProperties()
{
//	CStringList oFiles;

	CSingleLock pLock( &Library.m_pSection, TRUE );
	CFilePropertiesSheet dlg;

	POSITION posSel = StartSelectedFileLoop();
	while ( CLibraryFile* pFile = GetNextSelectedFile( posSel, FALSE, FALSE ) )
	{
		dlg.Add( pFile );
//		oFiles.AddTail( pFile->GetPath() );
	}
	pLock.Unlock();

/*	HRESULT hr;
	CComPtr< IDataObject > pDataObject;
	// Convert path string list to PIDL list
	{
		auto_array< PIDLIST_ABSOLUTE > pShellFileAbs( new PIDLIST_ABSOLUTE [ oFiles.GetCount() ] );
		for ( int i = 0; i < oFiles.GetCount(); ++i )
		  pShellFileAbs[ i ] = ILCreateFromPath( oFiles.GetHead() );

		PIDLIST_ABSOLUTE pShellParent = ILCloneFull( pShellFileAbs[ 0 ] );
		ILRemoveLastID( pShellParent );

		auto_array< LPCITEMIDLIST > pShellFiles( new LPCITEMIDLIST [ oFiles.GetCount() ] );
		POSITION pos = oFiles.GetHeadPosition();
		for ( int i = 0; i < oFiles.GetCount(); ++i )
			pShellFiles[ i ] = ILFindChild( pShellParent, pShellFileAbs[ i ] );

		hr = CIDLData_CreateFromIDArray( pShellParent, oFiles.GetCount(),
			pShellFiles.get(), &pDataObject );

		ILFree( pShellParent );

		for ( int i = 0; i < oFiles.GetCount(); ++i )
			ILFree( (LPITEMIDLIST)pShellFileAbs[ i ] );
	}
	if ( SUCCEEDED( hr ) )
		hr = SHMultiFileProperties( pDataObject, 0 );*/

	dlg.DoModal();
}

void CLibraryFileView::OnUpdateLibraryShared(CCmdUI* pCmdUI)
{
	CSingleLock pLock( &Library.m_pSection );
	TRISTATE bShared = TRI_UNKNOWN;

	if ( GetSelectedCount() > 0 && pLock.Lock( 100 ) )
	{
		POSITION posSel = StartSelectedFileLoop();
		while ( CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
		{
			if ( bShared == TRI_UNKNOWN )
			{
				bShared = pFile->IsShared() ? TRI_TRUE : TRI_FALSE;
			}
			else if ( ( bShared == TRI_TRUE ) != pFile->IsShared() )
			{
				pCmdUI->Enable( FALSE );
				return;
			}
		}
	}
	pCmdUI->Enable( GetSelectedCount() > 0 );
	pCmdUI->SetCheck( bShared == TRI_TRUE );
}

void CLibraryFileView::OnLibraryShared()
{
	CQuickLock oLock( Library.m_pSection );

	POSITION posSel = StartSelectedFileLoop();
	while ( CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
	{
		pFile->SetShared( ! pFile->IsShared() );
	}

	Library.Update();
}

void CLibraryFileView::OnUpdateLibraryUnlink(CCmdUI* pCmdUI)
{
	if ( m_bGhostFolder )
	{
		pCmdUI->Enable( FALSE );
		return;
	}
	CLibraryTreeItem* pItem = GetFolderSelection();
	pCmdUI->Enable( GetSelectedCount() > 0 && pItem && pItem->m_pVirtual && pItem->m_pSelNext == NULL );
}

void CLibraryFileView::OnLibraryUnlink()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	CLibraryTreeItem* pItem = GetFolderSelection();

	if ( pItem == NULL || pItem->m_pVirtual == NULL || pItem->m_pSelNext != NULL ) return;

	CAlbumFolder* pFolder = pItem->m_pVirtual;
	if ( ! LibraryFolders.CheckAlbum( pFolder ) ) return;

	POSITION posSel = StartSelectedFileLoop();
	while ( CLibraryFile* pFile = GetNextSelectedFile( posSel ) )
	{
		pFolder->RemoveFile( pFile );
	}
}

void CLibraryFileView::OnUpdateSearchForThis(CCmdUI* pCmdUI)
{
	CSingleLock pLock( &Library.m_pSection, TRUE );
	CRelatedSearch pSearch( GetSelectedFile() );
	pCmdUI->Enable( pSearch.CanSearchForThis() );
}

void CLibraryFileView::OnSearchForThis()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );
	CRelatedSearch pSearch( GetSelectedFile() );
	pLock.Unlock();
	pSearch.RunSearchForThis();
}

void CLibraryFileView::OnUpdateSearchForSimilar(CCmdUI* pCmdUI)
{
	CSingleLock pLock( &Library.m_pSection, TRUE );
	CRelatedSearch pSearch( GetSelectedFile() );
	pCmdUI->Enable( pSearch.CanSearchForSimilar() );
}

void CLibraryFileView::OnSearchForSimilar()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );
	CRelatedSearch pSearch( GetSelectedFile() );
	pLock.Unlock();
	pSearch.RunSearchForSimilar();
}

void CLibraryFileView::OnUpdateSearchForArtist(CCmdUI* pCmdUI)
{
	CSingleLock pLock( &Library.m_pSection, TRUE );
	CRelatedSearch pSearch( GetSelectedFile() );
	pCmdUI->Enable( pSearch.CanSearchForArtist() );
}

void CLibraryFileView::OnSearchForArtist()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );
	CRelatedSearch pSearch( GetSelectedFile() );
	pLock.Unlock();
	pSearch.RunSearchForArtist();
}

void CLibraryFileView::OnUpdateSearchForAlbum(CCmdUI* pCmdUI)
{
	CSingleLock pLock( &Library.m_pSection, TRUE );
	CRelatedSearch pSearch( GetSelectedFile() );
	pCmdUI->Enable( pSearch.CanSearchForAlbum() );
}

void CLibraryFileView::OnSearchForAlbum()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );
	CRelatedSearch pSearch( GetSelectedFile() );
	pLock.Unlock();
	pSearch.RunSearchForAlbum();
}

void CLibraryFileView::OnUpdateSearchForSeries(CCmdUI* pCmdUI)
{
	CSingleLock pLock( &Library.m_pSection, TRUE );
	CRelatedSearch pSearch( GetSelectedFile() );
	pCmdUI->Enable( pSearch.CanSearchForSeries() );
}

void CLibraryFileView::OnSearchForSeries()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );
	CRelatedSearch pSearch( GetSelectedFile() );
	pLock.Unlock();
	pSearch.RunSearchForSeries();
}

/////////////////////////////////////////////////////////////////////
// Web services handling

void CLibraryFileView::ClearServicePages()
{
	for ( POSITION pos = m_pServiceDataPages.GetHeadPosition() ; pos ; )
	{
		delete m_pServiceDataPages.GetNext( pos );
	}

	m_pServiceDataPages.RemoveAll();
	m_nCurrentPage = 0;
	m_bServiceFailed = FALSE;

	GetFrame()->SetPanelData( NULL );
}

void CLibraryFileView::OnUpdateMusicBrainzLookup(CCmdUI* pCmdUI)
{
	if ( m_bGhostFolder || GetSelectedCount() != 1 || m_bRequestingService )
	{
		pCmdUI->Enable( FALSE );
		return;
	}

	CSingleLock pLock( &Library.m_pSection, TRUE );

	CLibraryFile* pFile = GetSelectedFile();
	if ( !pFile->IsSchemaURI( CSchema::uriAudio ) || pFile->m_pMetadata == NULL )
	{
		pCmdUI->Enable( FALSE );
		return;
	}

	CMetaList* pMetaList = new CMetaList();
	pMetaList->Setup( pFile->m_pSchema, FALSE );
	pMetaList->Combine( pFile->m_pMetadata );

	pCmdUI->Enable( pMetaList->IsMusicBrainz() );

	delete pMetaList;
}

void CLibraryFileView::OnMusicBrainzLookup()
{
	CLibraryFrame* pFrame = GetFrame();
	pFrame->SetDynamicBar( L"WebServices.MusicBrainz" );
}

// Called when the selection changes
void CLibraryFileView::CheckDynamicBar()
{
	bool bIsMusicBrainz = false;
	ClearServicePages();

	CLibraryFrame* pFrame = GetFrame();
	if ( _tcscmp( pFrame->GetDynamicBarName(), L"WebServices.MusicBrainz" ) == 0 )
	{
		bIsMusicBrainz = true;
	}

	if ( GetSelectedCount() != 1 )
	{
		if ( bIsMusicBrainz )
		{
			pFrame->SetDynamicBar( NULL );
			m_bRequestingService = FALSE; // TODO: abort operation
		}
		return;
	}

	CSingleLock pLock( &Library.m_pSection, TRUE );
	CLibraryFile* pFile = GetSelectedFile();

	if ( pFile == NULL || ! pFile->IsAvailable() ) // Ghost file
	{
		pFrame->SetDynamicBar( NULL );
		m_bRequestingService = FALSE;
		return;
	}

	if ( !pFile->IsSchemaURI( CSchema::uriAudio ) || pFile->m_pMetadata == NULL )
	{
		if ( bIsMusicBrainz )
		{
			pFrame->SetDynamicBar( NULL );
		}

		m_bRequestingService = FALSE; // TODO: abort operation
		return;
	}

	CMetaList* pMetaList = new CMetaList();
	pMetaList->Setup( pFile->m_pSchema, FALSE );
	pMetaList->Combine( pFile->m_pMetadata );

	if ( !pMetaList->IsMusicBrainz() && bIsMusicBrainz )
		pFrame->SetDynamicBar( NULL );
	else
		pFrame->HideDynamicBar();

	m_bRequestingService = FALSE; // TODO: abort operation
	delete pMetaList;

	pLock.Unlock();
}

void CLibraryFileView::OnUpdateMusicBrainzMatches(CCmdUI* pCmdUI)
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	if ( CLibraryFile* pFile = GetSelectedFile() )
	{
		if ( pFile->m_pMetadata )
		{
			if ( CXMLAttribute* pAttribute = pFile->m_pMetadata->GetAttribute( L"mbpuid" ) )
			{
				pCmdUI->Enable( pAttribute->GetValue().GetLength() > 0 );
				return;
			}
		}
	}
	pCmdUI->Enable( FALSE );
}

void CLibraryFileView::OnMusicBrainzMatches()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	if ( CLibraryFile* pFile = GetSelectedFile() )
	{
		if ( pFile->m_pMetadata )
		{
			if ( CXMLAttribute* pAttribute = pFile->m_pMetadata->GetAttribute( L"mbpuid" ) )
			{
				CString mbpuid = pAttribute->GetValue();
				if ( mbpuid.GetLength() )
				{
					CString strURL = L"http://musicbrainz.org/show/puid/?matchesonly=0&amp;puid=" + mbpuid;
					ShellExecute( GetSafeHwnd(), _T("open"), strURL, NULL, NULL, SW_SHOWNORMAL );
				}
			}
		}
	}
}

void CLibraryFileView::OnUpdateMusicBrainzAlbums(CCmdUI* pCmdUI)
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	if ( CLibraryFile* pFile = GetSelectedFile() )
	{
		if ( pFile->m_pMetadata )
		{
			if ( CXMLAttribute* pAttribute = pFile->m_pMetadata->GetAttribute( L"mbartistid" ) )
			{
				pCmdUI->Enable( pAttribute->GetValue().GetLength() > 0 );
				return;
			}
		}
	}
	pCmdUI->Enable( FALSE );
}

void CLibraryFileView::OnMusicBrainzAlbums()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	if ( CLibraryFile* pFile = GetSelectedFile() )
	{
		if ( pFile->m_pMetadata )
		{
			if ( CXMLAttribute* pAttribute = pFile->m_pMetadata->GetAttribute( L"mbartistid" ) )
			{
				CString mbartistid = pAttribute->GetValue();
				if ( mbartistid.GetLength() )
				{
					CString strURL = L"http://musicbrainz.org/artist/" + mbartistid;
					ShellExecute( GetSafeHwnd(), _T("open"), strURL, NULL, NULL, SW_SHOWNORMAL );
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Set pszMessage to NULL when the work is done, to remove the status item from
// the meta panel. Don't do that in the middle or the order will change.
// The status must be the first item in the meta panel.
LRESULT CLibraryFileView::OnServiceDone(WPARAM wParam, LPARAM lParam)
{
	CString strStatus;
	LoadString( strStatus, IDS_TIP_STATUS );
	strStatus.TrimRight( ':' );

	LPCTSTR pszMessage = (LPCTSTR)lParam;
	CMetaList* pPanelData = (CMetaList*)wParam;

	m_bServiceFailed = FALSE;

	if ( pPanelData == NULL )
	{
		m_nCurrentPage = 0;
		m_bRequestingService = FALSE;
		ClearServicePages();
	}
	else
	{
		if ( pszMessage == NULL )
		{
			pPanelData->Remove( strStatus );
		}
		else
		{
			CMetaItem* pItem = pPanelData->Find( strStatus );
			if ( pItem != NULL )
				pItem->m_sValue = pszMessage;
			m_bServiceFailed = TRUE;
		}
	}

	CLibraryFrame* pFrame = GetFrame();
	if ( pFrame->GetPanelData() != NULL )
		pFrame->SetPanelData( pPanelData );

	m_bRequestingService = FALSE;

	return 0;
}
