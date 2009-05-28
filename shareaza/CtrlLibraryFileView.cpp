//
// CtrlLibraryFileView.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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
#include "DlgBitziDownload.h"
#include "DlgURLCopy.h"
#include "DlgURLExport.h"
#include "DlgDeleteFile.h"
#include "DlgDecodeMetadata.h"
#include "RelatedSearch.h"
#include "Security.h"
#include "Schema.h"
#include "XML.h"
#include "ShareMonkeyData.h"
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
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
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
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_BITZI_WEB, OnUpdateLibraryBitziWeb)
	ON_COMMAND(ID_LIBRARY_BITZI_WEB, OnLibraryBitziWeb)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_BITZI_DOWNLOAD, OnUpdateLibraryBitziDownload)
	ON_COMMAND(ID_LIBRARY_BITZI_DOWNLOAD, OnLibraryBitziDownload)
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
	ON_UPDATE_COMMAND_UI(ID_WEBSERVICES_SHAREMONKEY, OnUpdateShareMonkeyLookup)
	ON_COMMAND(ID_WEBSERVICES_SHAREMONKEY, OnShareMonkeyLookup)
	ON_UPDATE_COMMAND_UI(ID_SHAREMONKEY_DOWNLOAD, OnUpdateShareMonkeyDownload)
	ON_COMMAND(ID_SHAREMONKEY_DOWNLOAD, OnShareMonkeyDownload)
	ON_UPDATE_COMMAND_UI(ID_SHAREMONKEY_SAVE, OnUpdateShareMonkeySave)
	ON_COMMAND(ID_SHAREMONKEY_SAVE, OnShareMonkeySave)
	ON_UPDATE_COMMAND_UI(ID_SHAREMONKEY_SAVE_OPTION, OnUpdateShareMonkeySaveOption)
	ON_COMMAND(ID_SHAREMONKEY_SAVE_OPTION, OnShareMonkeySaveOption)
	ON_UPDATE_COMMAND_UI(ID_SHAREMONKEY_PREVIOUS, OnUpdateShareMonkeyPrevious)
	ON_COMMAND(ID_SHAREMONKEY_PREVIOUS, OnShareMonkeyPrevious)
	ON_UPDATE_COMMAND_UI(ID_SHAREMONKEY_NEXT, OnUpdateShareMonkeyNext)
	ON_COMMAND(ID_SHAREMONKEY_NEXT, OnShareMonkeyNext)
	ON_UPDATE_COMMAND_UI(ID_SHAREMONKEY_PRICES, OnUpdateShareMonkeyPrices)
	ON_COMMAND(ID_SHAREMONKEY_PRICES, OnShareMonkeyPrices)
	ON_UPDATE_COMMAND_UI(ID_SHAREMONKEY_COMPARE, OnUpdateShareMonkeyCompare)
	ON_COMMAND(ID_SHAREMONKEY_COMPARE, OnShareMonkeyCompare)
	ON_UPDATE_COMMAND_UI(ID_SHAREMONKEY_BUY, OnUpdateShareMonkeyBuy)
	ON_COMMAND(ID_SHAREMONKEY_BUY, OnShareMonkeyBuy)
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

void CLibraryFileView::StartSelectedFileLoop()
{
	m_posSel = m_pSelection.GetHeadPosition();
}

CLibraryFile* CLibraryFileView::GetNextSelectedFile()
{
	while ( m_posSel )
	{
		CLibraryFile* pFile = Library.LookupFile( m_pSelection.GetNext( m_posSel ) );
		if ( pFile != NULL && pFile->IsAvailable() ) return pFile;
	}

	return NULL;
}

CLibraryFile* CLibraryFileView::GetSelectedFile()
{
	if ( m_pSelection.GetCount() == 0 ) return NULL;
	CLibraryFile* pFile = Library.LookupFile( m_pSelection.GetHead() );
	if ( pFile != NULL && pFile->IsAvailable() ) return pFile;
	return NULL;
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
	CStringList oFiles;
	{
		CQuickLock pLock( Library.m_pSection );
		StartSelectedFileLoop();
		for ( CLibraryFile* pFile = GetNextSelectedFile(); pFile; pFile = GetNextSelectedFile() )
		{
			oFiles.AddTail( pFile->GetPath() );
		}
	}

	CString strName( m_pszToolBar );
	strName += Settings.Library.ShowVirtual ? _T(".Virtual") : _T(".Physical");
	Skin.TrackPopupMenu( strName, point, ID_LIBRARY_LAUNCH, 0, oFiles );
}

void CLibraryFileView::OnMouseMove(UINT nFlags, CPoint point)
{
	CLibraryView::OnMouseMove( nFlags, point );

	if ( DWORD_PTR nFile = HitTestIndex( point ) )
	{
		GetToolTip()->Show( (void*)nFile );
	}
	else
	{
		GetToolTip()->Hide();
	}
}

void CLibraryFileView::OnLButtonDown(UINT nFlags, CPoint point)
{
	GetToolTip()->Hide();

	CWnd::OnLButtonDown( nFlags, point );
}

void CLibraryFileView::OnRButtonDown(UINT nFlags, CPoint point)
{
	GetToolTip()->Hide();

	CWnd::OnRButtonDown( nFlags, point );
}

void CLibraryFileView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	GetToolTip()->Hide();
	CWnd::OnKeyDown( nChar, nRepCnt, nFlags );
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
	CMap< CString, const CString&, bool, bool > oFileList;

	{
		CQuickLock pLock( Library.m_pSection );
		StartSelectedFileLoop();
		for ( CLibraryFile* pFile = GetNextSelectedFile(); pFile; pFile = GetNextSelectedFile() )
		{
			CString strPath = pFile->GetPath();
			oFileList.SetAt( strPath, ( pFile->m_bVerify == TRI_FALSE ) &&
				( ! Settings.Search.AdultFilter || ! AdultFilter.IsChildPornography( strPath ) ) );
		}
	}

	for ( POSITION pos = oFileList.GetStartPosition(); pos; )
	{
		CString strPath;
		bool bSecurity;
		oFileList.GetNextAssoc( pos, strPath, bSecurity );

		if ( bSecurity )
		{
			CString strFormat, strMessage;

			LoadString( strFormat, IDS_LIBRARY_VERIFY_FAIL );
			strMessage.Format( strFormat, (LPCTSTR)strPath );
			INT_PTR nResponse = AfxMessageBox( strMessage, MB_ICONEXCLAMATION|MB_YESNOCANCEL|MB_DEFBUTTON2 );
			if ( nResponse == IDCANCEL )
				break;
			if ( nResponse == IDNO )
				continue;

			LoadString( strMessage, IDS_LIBRARY_VERIFY_FIX );
			nResponse = AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNOCANCEL|MB_DEFBUTTON2 );
			if ( nResponse == IDCANCEL )
				break;
			if ( nResponse == IDYES )
			{
				CQuickLock pLock( Library.m_pSection );
				CLibraryFile* pFile = LibraryMaps.LookupFileByPath( strPath );
				if ( pFile )
				{
					pFile->m_bVerify = TRI_UNKNOWN;
					Library.Update();
				}
			}
		}
		if ( ! CFileExecutor::Execute( strPath, FALSE ) )
			break;
	}
}

void CLibraryFileView::OnUpdateLibraryEnqueue(CCmdUI* pCmdUI)
{
	if ( m_bGhostFolder )
	{
		pCmdUI->Enable( FALSE );
		return;
	}
	CSingleLock pLock( &Library.m_pSection );

	if ( GetSelectedCount() > 0 && pLock.Lock( 100 ) )
	{
		StartSelectedFileLoop();

		for ( CLibraryFile* pFile = GetNextSelectedFile(); pFile; pFile = GetNextSelectedFile() )
		{
			if ( LPCTSTR pszType = _tcsrchr( pFile->m_sName, '.' ) )
			{
				if ( IsIn( Settings.MediaPlayer.FileTypes, pszType + 1 ) )
				{
					pCmdUI->Enable( TRUE );
					return;
				}
			}
		}
	}

	pCmdUI->Enable( FALSE );
}

void CLibraryFileView::OnLibraryEnqueue()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	StartSelectedFileLoop();

	for ( CLibraryFile* pFile = GetNextSelectedFile(); pFile; pFile = GetNextSelectedFile() )
	{
		CString strPath = pFile->GetPath();
		pLock.Unlock();
		CFileExecutor::Enqueue( strPath );
		pLock.Lock();
	}
}

void CLibraryFileView::OnUpdateLibraryURL(CCmdUI* pCmdUI)
{
	CString strMessage;
	if ( m_bGhostFolder )
		pCmdUI->Enable( FALSE );
	else
		pCmdUI->Enable( GetSelectedCount() > 0 );
	GetSelectedCount() > 1 ? LoadString( strMessage, IDS_LIBRARY_EXPORTURIS ) : LoadString( strMessage, IDS_LIBRARY_COPYURI );
	pCmdUI->SetText( strMessage );
}

void CLibraryFileView::OnLibraryURL()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	if ( GetSelectedCount() == 1 )
	{
		CLibraryFile* pFile = GetSelectedFile();
		if ( ! pFile ) return;

		CURLCopyDlg dlg;

		dlg.Add( pFile );

		pLock.Unlock();

		dlg.DoModal();
	}
	else
	{
		CURLExportDlg dlg;

		StartSelectedFileLoop();

		for ( CLibraryFile* pFile = GetNextSelectedFile(); pFile; pFile = GetNextSelectedFile() )
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

	StartSelectedFileLoop();

	for ( CLibraryFile* pFile = GetNextSelectedFile(); pFile; pFile = GetNextSelectedFile() )
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

	StartSelectedFileLoop();

	for ( CLibraryFile* pFile = GetNextSelectedFile(); pFile; pFile = GetNextSelectedFile() )
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
	CSingleLock pTransfersLock( &Transfers.m_pSection, TRUE ); // Can clear uploads and downloads
	CSingleLock pLibraryLock( &Library.m_pSection, TRUE );
	CLibraryList pList;

	StartSelectedFileLoop();

	while ( m_posSel )
	{
		if ( CLibraryFile* pFile = Library.LookupFile( m_pSelection.GetNext( m_posSel ),
				FALSE, ! m_bGhostFolder ) )
			pList.AddTail( pFile );
	}

	while ( !pList.IsEmpty() )
	{
		CLibraryFile* pFile = Library.LookupFile( pList.GetHead(), FALSE, ! m_bGhostFolder );
		if ( pFile == NULL )
		{
			pList.RemoveHead(); // Remove item from list to avoid endless loop
			continue;
		}

		if ( m_bGhostFolder )
		{
			for ( INT_PTR nProcess = pList.GetCount() ; nProcess > 0 && pList.GetCount() > 0 ; nProcess-- )
			{
				if ( ( pFile = Library.LookupFile( pList.RemoveHead() ) ) != NULL )
				{
					pFile->Delete( TRUE );
				}
			}
		}
		else
		{
			CDeleteFileDlg dlg( this );
			dlg.m_sName	= pFile->m_sName;
			dlg.m_sComments = pFile->m_sComments;
			dlg.m_nRateValue = pFile->m_nRating;
			dlg.m_bAll	= pList.GetCount() > 1;

			pLibraryLock.Unlock();
			pTransfersLock.Unlock();

			if ( dlg.DoModal() != IDOK ) break;

			pTransfersLock.Lock();
			pLibraryLock.Lock();

			for ( INT_PTR nProcess = dlg.m_bAll ? pList.GetCount() : 1 ; nProcess > 0 && pList.GetCount() > 0 ; nProcess-- )
			{
				if ( ( pFile = Library.LookupFile( pList.RemoveHead(), FALSE, TRUE ) ) != NULL )
				{
					dlg.Apply( pFile );
					pFile->Delete();
				}
			}
		}

		Library.Update( true );
	}
}

void CLibraryFileView::OnUpdateLibraryBitziWeb(CCmdUI* pCmdUI)
{
	if ( m_bGhostFolder )
		pCmdUI->Enable( FALSE );
	else
		pCmdUI->Enable( GetSelectedCount() == 1 && Settings.WebServices.BitziWebSubmit.GetLength() );
}

void CLibraryFileView::OnLibraryBitziWeb()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	if ( CLibraryFile* pFile = GetSelectedFile() )
	{
		DWORD nIndex = pFile->m_nIndex;
		pLock.Unlock();
		CFileExecutor::ShowBitziTicket( nIndex );
	}
}


void CLibraryFileView::OnUpdateLibraryCreateTorrent(CCmdUI* pCmdUI)
{
	if ( m_bGhostFolder )
		pCmdUI->Enable( FALSE );
	else
		pCmdUI->Enable( GetSelectedCount() == 1 && ( Settings.BitTorrent.DefaultTracker.GetLength() > 5 )
						&& ( Settings.BitTorrent.TorrentCreatorPath.GetLength() > 5 ) );
}

void CLibraryFileView::OnLibraryCreateTorrent()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	if ( CLibraryFile* pFile = GetSelectedFile() )
	{
		CString sCommandLine, sPath = pFile->GetPath();
		pLock.Unlock();

		if ( sPath.GetLength() > 0 )
		{
			sCommandLine = _T(" -sourcefile \"") + sPath + _T("\" -destination \"") + Settings.Downloads.TorrentPath + _T("\" -tracker \"" + Settings.BitTorrent.DefaultTracker + "\"" );

			ShellExecute( GetSafeHwnd(), _T("open"), Settings.BitTorrent.TorrentCreatorPath, sCommandLine, NULL, SW_SHOWNORMAL );

		}

	}
}

void CLibraryFileView::OnUpdateLibraryRebuildAnsi(CCmdUI* pCmdUI)
{
	if ( m_bGhostFolder )
	{
		pCmdUI->Enable( FALSE );
		return;
	}
	INT_PTR nSelected = GetSelectedCount();

	StartSelectedFileLoop();

	// Count only selected mp3's which have no custom metadata in XML format
	while ( m_posSel )
	{
		// Lookup locks library if it finds a file
		CQuickLock oLock( Library.m_pSection );

		if ( CLibraryFile* pFile = Library.LookupFile( m_pSelection.GetNext( m_posSel ), FALSE, TRUE ) )
		{
			CString strExtension = pFile->m_sName.Right(3);
			ToLower( strExtension );

			BOOL bXmlPossiblyModified = FALSE;
			if ( !pFile->m_bMetadataAuto )
			{
				WIN32_FIND_DATA fd = { 0 };
				if ( GetFileAttributesEx( pFile->GetPath(), GetFileExInfoStandard, &fd ) )
				{
					ULARGE_INTEGER nMetaDataTime;
					ULARGE_INTEGER nFileDataTime;

					nFileDataTime.HighPart = fd.ftLastWriteTime.dwHighDateTime;
					nFileDataTime.LowPart = fd.ftLastWriteTime.dwLowDateTime;
					// Convert 100 ns into seconds
					nFileDataTime.QuadPart /= 10000000;

					nMetaDataTime.HighPart = pFile->m_pMetadataTime.dwHighDateTime;
					nMetaDataTime.LowPart = pFile->m_pMetadataTime.dwLowDateTime;
					nMetaDataTime.QuadPart /= 10000000;

					// assume that XML was not modified during the first 10 sec. of creation
					if ( nMetaDataTime.HighPart == nFileDataTime.HighPart &&
						nMetaDataTime.LowPart - nFileDataTime.LowPart > 10 )
						bXmlPossiblyModified = TRUE;
				}
			}
			if ( ( strExtension != _T("mp3") && strExtension != _T("pdf") &&
				   strExtension != _T("mpc") && strExtension != _T("mpp") &&
				   strExtension != _T("mp+") && strExtension != _T("avi") )
				 || bXmlPossiblyModified )
				nSelected--;
		}
		else nSelected--;
	}
	pCmdUI->Enable( nSelected > 0 );
}

void CLibraryFileView::OnLibraryRebuildAnsi()
{
	CDecodeMetadataDlg dlg;

	CSingleLock pLock( &Library.m_pSection, TRUE );

	StartSelectedFileLoop();

	for ( CLibraryFile* pFile = GetNextSelectedFile(); pFile; pFile = GetNextSelectedFile() )
	{
		CString strExtension = pFile->m_sName.Right(3);
		ToLower( strExtension );

		if ( strExtension == _T("mp3") || strExtension == _T("pdf") ||
			 strExtension == _T("mpc") || strExtension == _T("mpp") ||
			 strExtension == _T("mp+") || strExtension == _T("avi") )
			dlg.AddFile( pFile );
	}

	pLock.Unlock();

	if ( dlg.m_pFiles.GetCount() ) dlg.DoModal();
}


void CLibraryFileView::OnUpdateLibraryBitziDownload(CCmdUI* pCmdUI)
{
	if ( m_bGhostFolder || m_bRequestingService )
		pCmdUI->Enable( FALSE );
	else
		pCmdUI->Enable( GetSelectedCount() > 0 && Settings.WebServices.BitziXML.GetLength() );
}

void CLibraryFileView::OnLibraryBitziDownload()
{
	GetFrame()->SetDynamicBar( NULL );

	if ( ! Settings.WebServices.BitziOkay )
	{
		CString strFormat;
		Skin.LoadString( strFormat, IDS_LIBRARY_BITZI_MESSAGE );
		if ( AfxMessageBox( strFormat, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return;
		Settings.WebServices.BitziOkay = true;
		Settings.Save();
	}

	CSingleLock pLock( &Library.m_pSection, TRUE );
	CBitziDownloadDlg dlg;

	StartSelectedFileLoop();

	for ( CLibraryFile* pFile = GetNextSelectedFile(); pFile; pFile = GetNextSelectedFile() )
	{
		if ( pFile->m_oSHA1 ) dlg.AddFile( pFile->m_nIndex );
	}

	pLock.Unlock();

	dlg.DoModal();
}

void CLibraryFileView::OnUpdateLibraryRefreshMetadata(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( ! m_bGhostFolder && GetSelectedCount() > 0 );
}

void CLibraryFileView::OnLibraryRefreshMetadata()
{
	CQuickLock pLock( Library.m_pSection );

	StartSelectedFileLoop();

	for ( CLibraryFile* pFile = GetNextSelectedFile(); pFile; pFile = GetNextSelectedFile() )
	{
		LibraryBuilder.RefreshMetadata( pFile->GetPath() );
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

	StartSelectedFileLoop();
	while ( m_posSel )
	{
		if ( CLibraryFile* pFile = Library.LookupFile( m_pSelection.GetNext( m_posSel ) ) )
		{
			dlg.Add( pFile );
//			oFiles.AddTail( pFile->GetPath() );
		}
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
		StartSelectedFileLoop();
		while ( m_posSel )
		{
			if ( CLibraryFile* pFile = Library.LookupFile( m_pSelection.GetNext( m_posSel ) ) )
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
	}
	pCmdUI->Enable( GetSelectedCount() > 0 );
	pCmdUI->SetCheck( bShared == TRI_TRUE );
}

void CLibraryFileView::OnLibraryShared()
{
	CQuickLock oLock( Library.m_pSection );

	StartSelectedFileLoop();

	while ( m_posSel )
	{
		if ( CLibraryFile* pFile = Library.LookupFile( m_pSelection.GetNext( m_posSel ) ) )
		{
			// Don't share not verified files
			if ( pFile->m_bVerify != TRI_FALSE )
			{
				bool bPrivate = false;
				if ( pFile->m_pSchema != NULL &&
					pFile->m_pSchema->CheckURI( CSchema::uriBitTorrent ) &&
					pFile->m_pMetadata != NULL )
				{
					CString str = pFile->m_pMetadata->GetAttributeValue( L"privateflag", L"true" );
					bPrivate = str == L"true";
				}
				bool bFolderShared = pFile->m_pFolder ? pFile->m_pFolder->IsShared() != TRI_UNKNOWN : true;
				if ( bPrivate )
					pFile->m_bShared = TRI_FALSE;
				else if ( pFile->IsShared() )
					pFile->m_bShared = bFolderShared ? TRI_FALSE : TRI_UNKNOWN;
				else
					pFile->m_bShared = bFolderShared ? TRI_UNKNOWN : TRI_TRUE;

				pFile->m_nUpdateCookie++;
			}
		}
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

	StartSelectedFileLoop();

	for ( CLibraryFile* pFile = GetNextSelectedFile(); pFile; pFile = GetNextSelectedFile() )
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
		CMetaPanel* pPanelData = m_pServiceDataPages.GetNext( pos );
		delete pPanelData;
	}

	m_pServiceDataPages.RemoveAll();
	m_nCurrentPage = 0;
	m_bServiceFailed = FALSE;

	GetFrame()->SetPanelData( NULL );
}

void CLibraryFileView::OnUpdateShareMonkeyLookup(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( GetSelectedCount() == 1 && !m_bRequestingService );
}

void CLibraryFileView::OnShareMonkeyLookup()
{
	GetFrame()->SetDynamicBar( L"WebServices.ShareMonkey.WithSave" );
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

	if ( pFile == NULL ) // Ghost file
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

	CLibraryFile* pFile = GetSelectedFile();

	ASSERT( pFile->m_pMetadata != NULL );

	CXMLAttribute* pAttribute = pFile->m_pMetadata->GetAttribute( L"mbpuid" );
	pCmdUI->Enable( pAttribute != NULL && pAttribute->GetValue().GetLength() > 0 );
}

void CLibraryFileView::OnMusicBrainzMatches()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	CLibraryFile* pFile = GetSelectedFile();
	ASSERT( pFile->m_pMetadata != NULL );

	CXMLAttribute* pAttribute = pFile->m_pMetadata->GetAttribute( L"mbpuid" );
	CString strURL = L"http://musicbrainz.org/show/puid/?matchesonly=0&amp;puid=" + pAttribute->GetValue();

	ShellExecute( GetSafeHwnd(), _T("open"), strURL, NULL, NULL, SW_SHOWNORMAL );
}

void CLibraryFileView::OnUpdateMusicBrainzAlbums(CCmdUI* pCmdUI)
{
	CSingleLock pLock( &Library.m_pSection, TRUE );
	CLibraryFile* pFile = GetSelectedFile();

	ASSERT( pFile->m_pMetadata != NULL );

	CXMLAttribute* pAttribute = pFile->m_pMetadata->GetAttribute( L"mbartistid" );
	pCmdUI->Enable( pAttribute != NULL && pAttribute->GetValue().GetLength() > 0 );
}

void CLibraryFileView::OnMusicBrainzAlbums()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	CLibraryFile* pFile = GetSelectedFile();
	ASSERT( pFile->m_pMetadata != NULL );

	CXMLAttribute* pAttribute = pFile->m_pMetadata->GetAttribute( L"mbartistid" );
	CString strURL = L"http://musicbrainz.org/artist/" + pAttribute->GetValue();

	ShellExecute( GetSafeHwnd(), _T("open"), strURL, NULL, NULL, SW_SHOWNORMAL );
}

void CLibraryFileView::OnUpdateShareMonkeyDownload(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( !m_bRequestingService && m_pServiceDataPages.GetCount() == 0 );
}

void CLibraryFileView::OnShareMonkeyDownload()
{
	if ( ! Settings.WebServices.ShareMonkeyOkay )
	{
		CString strFormat;
		Skin.LoadString( strFormat, IDS_SHAREMONKEY_MESSAGE );
		if ( AfxMessageBox( strFormat, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return;
		Settings.WebServices.ShareMonkeyOkay = true;
		Settings.Save();
	}

	CShareMonkeyData* pPanelData = new CShareMonkeyData( m_nCurrentPage );

	CString strStatus;
	LoadString( strStatus, IDS_TIP_STATUS );
	strStatus.TrimRight( ':' );
	pPanelData->Add( strStatus, L"Please wait..." );

	if ( m_nCurrentPage == 0 )
		ClearServicePages();
	GetFrame()->SetPanelData( pPanelData );

	m_pServiceDataPages.AddTail( pPanelData );

	CSingleLock pLock( &Library.m_pSection, TRUE );
	CLibraryFile* pFile = GetSelectedFile();

	// Should be set to FALSE, and abort button created
	m_bRequestingService = pFile != NULL;
	if ( m_bRequestingService )
	{
		DWORD nIndex = pFile->m_nIndex;
		pLock.Unlock();
		pPanelData->Start( this, nIndex );
		return;
	}

	pLock.Unlock();
}

void CLibraryFileView::OnUpdateShareMonkeySave(CCmdUI* pCmdUI)
{
	BOOL bShow = TRUE;
	if ( m_bServiceFailed && m_nCurrentPage == m_pServiceDataPages.GetCount() - 1 )
		bShow = FALSE;
	pCmdUI->Enable( bShow && !m_bRequestingService && m_pServiceDataPages.GetCount() > 0 );
}

void CLibraryFileView::OnShareMonkeySave()
{
	INT_PTR nCurr = 0;
	CShareMonkeyData* pPanelData = NULL;

	for ( POSITION pos = m_pServiceDataPages.GetHeadPosition() ; pos ; nCurr++ )
	{
		if ( m_nCurrentPage == nCurr )
		{
			pPanelData = static_cast< CShareMonkeyData* >( m_pServiceDataPages.GetNext( pos ) );
			break;
		}
	}

	if ( pPanelData == NULL ) return;

	CSingleLock pLock( &Library.m_pSection, TRUE );

	CLibraryFile* pFile = GetSelectedFile();
	CSchema* pSchema = pFile->m_pSchema ? pFile->m_pSchema : pPanelData->GetSchema();

	if ( pSchema )
	{
		CXMLElement* pRoot = pSchema->Instantiate( TRUE );
		CXMLElement* pXML = NULL;

		if ( pFile->m_pMetadata )
		{
			pXML = pFile->m_pMetadata->Clone();
			pRoot->AddElement( pXML );
		}
		else
			pXML = pRoot->AddElement( pSchema->m_sSingular );

		CXMLAttribute* pTitle = new CXMLAttribute( NULL, L"title" );
		pXML->AddAttribute( pTitle );

		CXMLAttribute* pDescription = NULL;
		if ( pSchema->CheckURI( CSchema::uriApplication ) )
		{
			pDescription = new CXMLAttribute( NULL, L"fileDescription" );
			pXML->AddAttribute( pDescription );
		}
		else if ( pSchema->CheckURI( CSchema::uriArchive ) )
		{
			// No description... There should be games
		}
		else
		{
			pDescription = new CXMLAttribute( NULL, L"description" );
			pXML->AddAttribute( pDescription );
		}

		if ( pTitle )
			pTitle->SetValue( pPanelData->m_sProductName );
		if ( pDescription )
			pDescription->SetValue( pPanelData->m_sDescription );

		pFile->SetMetadata( pRoot );
		delete pRoot;
	}
}

void CLibraryFileView::OnUpdateShareMonkeySaveOption(CCmdUI* pCmdUI)
{
	OnUpdateShareMonkeySave( pCmdUI );
}

void CLibraryFileView::OnShareMonkeySaveOption()
{
	Settings.WebServices.ShareMonkeySaveThumbnail = !Settings.WebServices.ShareMonkeySaveThumbnail;
}

void CLibraryFileView::OnUpdateShareMonkeyPrevious(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( !m_bRequestingService && m_nCurrentPage > 0 );
}

void CLibraryFileView::OnShareMonkeyPrevious()
{
	INT_PTR nCurr = m_nCurrentPage--;
	CShareMonkeyData* pPanelData = NULL;

	POSITION pos = m_pServiceDataPages.GetHeadPosition();

	while ( pos && nCurr-- )
	{
		pPanelData = static_cast< CShareMonkeyData* >( m_pServiceDataPages.GetNext( pos ) );
	}

	GetFrame()->SetPanelData( pPanelData );
}

void CLibraryFileView::OnUpdateShareMonkeyNext(CCmdUI* pCmdUI)
{
	BOOL bShow = TRUE;
	if ( m_bServiceFailed && m_nCurrentPage == m_pServiceDataPages.GetCount() - 1 )
		bShow = FALSE;
	pCmdUI->Enable( bShow && !m_bRequestingService && m_pServiceDataPages.GetCount() > 0 );
}

void CLibraryFileView::OnShareMonkeyNext()
{
	INT_PTR nCurr = ++m_nCurrentPage;
	nCurr++;

	if ( m_nCurrentPage > m_pServiceDataPages.GetCount() - 1 )
		OnShareMonkeyDownload();
	else
	{
		CShareMonkeyData* pPanelData = NULL;
		POSITION pos = m_pServiceDataPages.GetHeadPosition();

		while ( pos && nCurr-- )
		{
			pPanelData = static_cast< CShareMonkeyData* >( m_pServiceDataPages.GetNext( pos ) );
		}

		GetFrame()->SetPanelData( pPanelData );
	}
}

void CLibraryFileView::OnUpdateShareMonkeyPrices(CCmdUI* pCmdUI)
{
	BOOL bShow = TRUE;
	if ( m_bServiceFailed && m_pServiceDataPages.GetCount() == 1 || m_pServiceDataPages.GetCount() == 0 )
		bShow = FALSE;
	pCmdUI->Enable( !m_bRequestingService && bShow );
}

void CLibraryFileView::OnShareMonkeyPrices()
{
	POSITION pos = m_pServiceDataPages.GetHeadPosition();
	CMetaPanel* pPanelData = NULL;

	// TODO: change m_pServiceDataPages to CMap. Now it's stupid
	for ( INT_PTR nPage = 0 ; nPage <= m_nCurrentPage ; nPage++ )
	{
		pPanelData = m_pServiceDataPages.GetNext( pos );
	}

	CShareMonkeyData* pData = static_cast< CShareMonkeyData* >( pPanelData );
	if ( pData->m_pChild == NULL )
	{
		CShareMonkeyData* pChild = new CShareMonkeyData( 0, CShareMonkeyData::stStoreMatch );
		pData->m_pChild = pChild;
		CString strStatus;
		LoadString( strStatus, IDS_TIP_STATUS );
		strStatus.TrimRight( ':' );
		pChild->Add( strStatus, L"Please wait..." );
		pChild->m_sSessionID = pData->m_sSessionID;
		pChild->m_sProductID = pData->m_sProductID;
		pChild->m_sThumbnailURL = pData->m_sThumbnailURL;

		GetFrame()->SetPanelData( pChild );
		pChild->Start( this, 0 );
	}
	else
	{
		GetFrame()->SetPanelData( pData->m_pChild );
	}
}

void CLibraryFileView::OnUpdateShareMonkeyCompare(CCmdUI* pCmdUI)
{
	BOOL bShow = TRUE;
	if ( m_bServiceFailed && m_pServiceDataPages.GetCount() == 1 || m_pServiceDataPages.GetCount() == 0 )
		bShow = FALSE;
	pCmdUI->Enable( !m_bRequestingService && bShow );
}

void CLibraryFileView::OnShareMonkeyCompare()
{
	POSITION pos = m_pServiceDataPages.GetHeadPosition();
	CMetaPanel* pPanelData = NULL;

	// TODO: change m_pServiceDataPages to CMap. Now it's stupid
	for ( INT_PTR nPage = 0 ; nPage <= m_nCurrentPage ; nPage++ )
	{
		pPanelData = m_pServiceDataPages.GetNext( pos );
	}

	CShareMonkeyData* pData = static_cast< CShareMonkeyData* >( pPanelData );
	if ( pData->m_sComparisonURL.GetLength() )
	{
		ShellExecute( GetSafeHwnd(), _T("open"), pData->m_sComparisonURL, NULL, NULL, SW_SHOWNORMAL );
	}
}

void CLibraryFileView::OnUpdateShareMonkeyBuy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( !m_bServiceFailed && !m_bRequestingService && m_pServiceDataPages.GetCount() > 0 );
}

void CLibraryFileView::OnShareMonkeyBuy()
{
	POSITION pos = m_pServiceDataPages.GetHeadPosition();
	CMetaPanel* pPanelData = NULL;

	// TODO: change m_pServiceDataPages to CMap. Now it's stupid
	for ( INT_PTR nPage = 0 ; nPage <= m_nCurrentPage ; nPage++ )
	{
		pPanelData = m_pServiceDataPages.GetNext( pos );
	}

	CShareMonkeyData* pData = static_cast< CShareMonkeyData* >( pPanelData );
	if ( pData->m_sBuyURL.GetLength() )
	{
		ShellExecute( GetSafeHwnd(), _T("open"), pData->m_sBuyURL, NULL, NULL, SW_SHOWNORMAL );
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
	CMetaPanel* pPanelData = (CMetaPanel*)wParam;

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
