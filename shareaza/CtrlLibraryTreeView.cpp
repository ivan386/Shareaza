//
// CtrlLibraryTreeView.cpp
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
#include "LibraryFolders.h"
#include "SharedFile.h"
#include "SharedFolder.h"
#include "AlbumFolder.h"
#include "FileExecutor.h"
#include "CtrlLibraryTreeView.h"
#include "CtrlLibraryFrame.h"
#include "CtrlCoolBar.h"
#include "Schema.h"
#include "Skin.h"

#include "DlgFolderScan.h"
#include "DlgFolderProperties.h"
#include "DlgFilePropertiesSheet.h"
#include "DlgFileCopy.h"
#include "DlgCollectionExport.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CLibraryTreeView, CLibraryTreeCtrl)

BEGIN_MESSAGE_MAP(CLibraryTreeView, CLibraryTreeCtrl)
	//{{AFX_MSG_MAP(CLibraryTreeView)
	ON_WM_CONTEXTMENU()
	ON_WM_LBUTTONDBLCLK()
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_PARENT, OnUpdateLibraryParent)
	ON_COMMAND(ID_LIBRARY_PARENT, OnLibraryParent)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_EXPLORE, OnUpdateLibraryExplore)
	ON_COMMAND(ID_LIBRARY_EXPLORE, OnLibraryExplore)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_SCAN, OnUpdateLibraryScan)
	ON_COMMAND(ID_LIBRARY_SCAN, OnLibraryScan)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_SHARED_FOLDER, OnUpdateLibraryShared)
	ON_COMMAND(ID_LIBRARY_SHARED_FOLDER, OnLibraryShared)
	ON_COMMAND(ID_LIBRARY_ADD, OnLibraryAdd)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_REMOVE, OnUpdateLibraryRemove)
	ON_COMMAND(ID_LIBRARY_REMOVE, OnLibraryRemove)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_FOLDER_PROPERTIES, OnUpdateLibraryFolderProperties)
	ON_COMMAND(ID_LIBRARY_FOLDER_PROPERTIES, OnLibraryFolderProperties)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_FOLDER_NEW, OnUpdateLibraryFolderNew)
	ON_COMMAND(ID_LIBRARY_FOLDER_NEW, OnLibraryFolderNew)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_FOLDER_DELETE, OnUpdateLibraryFolderDelete)
	ON_COMMAND(ID_LIBRARY_FOLDER_DELETE, OnLibraryFolderDelete)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_FOLDER_METADATA, OnUpdateLibraryFolderMetadata)
	ON_COMMAND(ID_LIBRARY_FOLDER_METADATA, OnLibraryFolderMetadata)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_FOLDER_ENQUEUE, OnUpdateLibraryFolderEnqueue)
	ON_COMMAND(ID_LIBRARY_FOLDER_ENQUEUE, OnLibraryFolderEnqueue)
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_FOLDER_FILE_PROPERTIES, OnUpdateLibraryFolderFileProperties)
	ON_COMMAND(ID_LIBRARY_FOLDER_FILE_PROPERTIES, OnLibraryFolderFileProperties)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_REBUILD, OnUpdateLibraryRebuild)
	ON_COMMAND(ID_LIBRARY_REBUILD, OnLibraryRebuild)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_EXPORT_COLLECTION, OnUpdateLibraryExportCollection)
	ON_COMMAND(ID_LIBRARY_EXPORT_COLLECTION, OnLibraryExportCollection)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView construction

CLibraryTreeView::CLibraryTreeView()
{
	m_bVirtual = -1;
}

CLibraryTreeView::~CLibraryTreeView()
{
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView root update

void CLibraryTreeView::SetVirtual(BOOL bVirtual)
{
	if ( bVirtual == m_bVirtual ) return;
	
	m_bVirtual = bVirtual;
	SetToolTip( m_bVirtual ? (CCoolTipCtrl*)&m_wndAlbumTip : (CCoolTipCtrl*)&m_wndFolderTip );

	Clear();
}

void CLibraryTreeView::Update(DWORD nSelectCookie)
{
	if ( m_bVirtual )
	{
		UpdateVirtual( nSelectCookie );
	}
	else
	{
		UpdatePhysical( nSelectCookie );
	}
}

void CLibraryTreeView::PostUpdate()
{
	GetOwner()->PostMessage( WM_COMMAND, ID_LIBRARY_REFRESH );
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView update physical

void CLibraryTreeView::UpdatePhysical(DWORD nSelectCookie)
{
	DWORD nCleanCookie = m_nCleanCookie++;
	BOOL bChanged = FALSE;

	for ( POSITION pos = LibraryFolders.GetFolderIterator() ; pos ; )
	{
		CLibraryFolder* pFolder = LibraryFolders.GetNextFolder( pos );

		CLibraryTreeItem** pChild = m_pRoot->m_pList;
		
		for ( int nChild = m_pRoot->m_nCount ; nChild ; nChild--, pChild++ )
		{
			CLibraryFolder* pOld = (*pChild)->m_pPhysical;

			if ( pOld == pFolder )
			{
				bChanged |= Update( pFolder, *pChild, m_pRoot, TRUE, TRUE,
					nCleanCookie, nSelectCookie, FALSE );
				break;
			}
		}

		if ( nChild == 0 )
		{
			bChanged |= Update( pFolder, NULL, m_pRoot, TRUE, TRUE,
				nCleanCookie, nSelectCookie, FALSE );
		}
	}

	bChanged |= CleanItems( m_pRoot, nCleanCookie, TRUE );

	if ( bChanged )
	{
		UpdateScroll();
		Invalidate();
		NotifySelection();
	}
}

BOOL CLibraryTreeView::Update(CLibraryFolder* pFolder, CLibraryTreeItem* pItem, CLibraryTreeItem* pParent, BOOL bVisible, BOOL bShared, DWORD nCleanCookie, DWORD nSelectCookie, BOOL bRecurse)
{
	BOOL bChanged = FALSE;

	if ( pFolder->m_bShared == TS_TRUE ) bShared = TRUE;
	else if ( pFolder->m_bShared == TS_FALSE ) bShared = FALSE;

	if ( pItem == NULL )
	{
		pItem = pParent->Add( pFolder->m_sName );
		if ( bVisible ) m_nTotal++;
		
		pItem->m_bExpanded	= pFolder->m_bExpanded;
		pItem->m_pPhysical	= pFolder;
		pItem->m_sText		= pFolder->m_sName;
		pItem->m_bShared	= bShared;
		pItem->m_bBold		= ( pFolder->m_sPath.CompareNoCase( Settings.Downloads.CompletePath ) == 0 );
		
		if ( pFolder->m_pParent == NULL )
		{
			if ( pFolder->m_sPath.Find( _T(":\\") ) == 1 || pFolder->m_sPath.GetLength() == 2 )
			{
				CString strDrive;
				strDrive.Format( _T(" (%c:)"), pFolder->m_sPath[0] );
				pItem->m_sText += strDrive;
			}
			else
			{
				pItem->m_sText += _T(" (Net)");
			}
		}
		
		bChanged = bVisible;
	}
	else
	{
		if ( pFolder->m_pParent == NULL )
		{
			BOOL bBold = ( pFolder->m_sPath.CompareNoCase( Settings.Downloads.CompletePath ) == 0 );
			
			if ( bBold != pItem->m_bBold )
			{
				pItem->m_bBold = bBold;
				bChanged |= bVisible;
			}
		}
		
		if ( pItem->m_bShared != bShared )
		{
			pItem->m_bShared = bShared;
			bChanged |= bVisible;
		}
	}
	
	pItem->m_nCleanCookie = nCleanCookie;
	
	bVisible = bVisible && pItem->m_bExpanded;
	
	if ( nSelectCookie )
	{
		if ( bRecurse || pItem->m_bSelected )
		{
			CLibraryFile* pFile;
			CString strTemp;
			
			for ( POSITION pos = pFolder->m_pFiles.GetStartPosition() ; pos ; )
			{
				pFolder->m_pFiles.GetNextAssoc( pos, strTemp, (CObject*&)pFile );
				pFile->m_nSelectCookie = nSelectCookie;
			}
			
			pFolder->m_nSelectCookie = nSelectCookie;
			bRecurse |= ( ! pItem->m_bExpanded );
		}
	}
	
	nCleanCookie = m_nCleanCookie++;
	
	for ( POSITION pos = pFolder->GetFolderIterator() ; pos ; )
	{
		CLibraryFolder* pSub = pFolder->GetNextFolder( pos );
		
		CLibraryTreeItem** pChild = pItem->m_pList;
		
		for ( int nChild = pItem->m_nCount ; nChild ; nChild--, pChild++ )
		{
			CLibraryFolder* pOld = (*pChild)->m_pPhysical;

			if ( pOld == pSub )
			{
				bChanged |= Update( pSub, *pChild, pItem, bVisible, bShared,
					nCleanCookie, nSelectCookie, bRecurse );
				break;
			}
		}
		
		if ( nChild == 0 )
		{
			bChanged |= Update( pSub, NULL, pItem, bVisible, bShared,
				nCleanCookie, nSelectCookie, bRecurse );
		}
	}
	
	bChanged |= CleanItems( pItem, nCleanCookie, bVisible );
	
	pItem->m_nCookie = pFolder->m_nUpdateCookie;
	
	return bChanged;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView update virtual

void CLibraryTreeView::UpdateVirtual(DWORD nSelectCookie)
{
	BOOL bChanged = Update( Library.GetAlbumRoot(), m_pRoot, NULL, TRUE, 0, nSelectCookie );
	
	if ( bChanged )
	{
		UpdateScroll();
		Invalidate();
		NotifySelection();
	}
}

BOOL CLibraryTreeView::Update(CAlbumFolder* pFolder, CLibraryTreeItem* pItem, CLibraryTreeItem* pParent, BOOL bVisible, DWORD nCleanCookie, DWORD nSelectCookie)
{
	BOOL bChanged = FALSE;
	
	if ( pItem != NULL && pParent != NULL && pItem->m_sText != pFolder->m_sName )
	{
		// CleanCookie is not updated so it will be dropped later
		pItem = NULL;
	}
	
	if ( pItem == NULL )
	{
		pItem = pParent->Add( pFolder->m_sName );
		if ( bVisible ) m_nTotal++;
		
		pItem->m_bExpanded	= pFolder->m_bExpanded;
		pItem->m_pVirtual	= pFolder;
		pItem->m_sText		= pFolder->m_sName;
		pItem->m_nIcon16	= pFolder->m_pSchema ? pFolder->m_pSchema->m_nIcon16 : -1;
		pItem->m_bBold		= pItem->m_bCollection = pFolder->m_oCollSHA1.IsValid();
		
		bChanged = bVisible;
	}
	else
	{
		if ( pFolder->m_pSchema != NULL && pItem->m_nIcon16 != pFolder->m_pSchema->m_nIcon16 )
		{
			pItem->m_nIcon16 = pFolder->m_pSchema->m_nIcon16;
			bChanged = bVisible;
		}
		
		if ( pItem->m_bCollection != pFolder->m_oCollSHA1.IsValid() )
		{
			pItem->m_bBold = pItem->m_bCollection = pFolder->m_oCollSHA1.IsValid();
			bChanged = bVisible;
		}
	}
	
	pItem->m_nCleanCookie = nCleanCookie;
	
	bVisible = bVisible && pItem->m_bExpanded;
	
	if ( nSelectCookie && pItem->m_bSelected )
	{
		for ( POSITION pos = pFolder->m_pFiles.GetHeadPosition() ; pos ; )
		{
			CLibraryFile* pFile = (CLibraryFile*)pFolder->m_pFiles.GetNext( pos );
			pFile->m_nSelectCookie = nSelectCookie;
		}
		
		pFolder->m_nSelectCookie = nSelectCookie;
	}
	
	nCleanCookie = m_nCleanCookie++;
	
	for ( POSITION pos = pFolder->GetFolderIterator() ; pos ; )
	{
		CAlbumFolder* pSub = pFolder->GetNextFolder( pos );
		
		CLibraryTreeItem** pChild = pItem->m_pList;
		
		for ( int nChild = pItem->m_nCount ; nChild ; nChild--, pChild++ )
		{
			CAlbumFolder* pOld = (*pChild)->m_pVirtual;
			
			if ( pOld == pSub )
			{
				bChanged |= Update( pSub, *pChild, pItem, bVisible,
					nCleanCookie, nSelectCookie );
				break;
			}
		}
		
		if ( nChild == 0 )
		{
			bChanged |= Update( pSub, NULL, pItem, bVisible,
				nCleanCookie, nSelectCookie );
		}
	}
	
	bChanged |= CleanItems( pItem, nCleanCookie, bVisible );
	
	pItem->m_nCookie = pFolder->m_nUpdateCookie;
	
	return bChanged;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView folder selection

BOOL CLibraryTreeView::SelectFolder(LPVOID pSearch)
{
	CLibraryTreeItem* pItem = GetFolderItem( pSearch );
	if ( pItem == NULL ) return FALSE;

	if ( m_nSelected == 1 && pItem->m_bSelected )
	{
		Highlight( pItem );
		return TRUE;
	}

	DeselectAll( pItem, NULL, FALSE );
	Select( pItem, TS_TRUE, FALSE );
	Highlight( pItem );
	NotifySelection();
	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView drag drop

BOOL CLibraryTreeView::DropShowTarget(CLibraryList* pList, const CPoint& point)
{
	CPoint ptLocal( point );
	ScreenToClient( &ptLocal );

	CRect rcClient, rcItem;
	GetClientRect( &rcClient );

	CLibraryTreeItem* pHit = HitTest( ptLocal, &rcItem );

	if ( pHit && ! rcItem.PtInRect( ptLocal ) ) pHit = NULL;

	if ( m_pDropItem != pHit )
	{
		m_pDropItem = pHit;
		CImageList::DragShowNolock( FALSE );
		RedrawWindow();
		CImageList::DragShowNolock( TRUE );
	}

	if ( rcClient.PtInRect( ptLocal ) )
	{
		if ( ptLocal.y < rcClient.top + 8 )
		{
			CImageList::DragShowNolock( FALSE );
			ScrollBy( -16 );
			UpdateWindow();
			CImageList::DragShowNolock( TRUE );
		}
		else if ( ptLocal.y >= rcClient.bottom - 8 )
		{
			CImageList::DragShowNolock( FALSE );
			ScrollBy( 16 );
			UpdateWindow();
			CImageList::DragShowNolock( TRUE );
		}
	}

	return ( m_pDropItem != NULL );
}

BOOL CLibraryTreeView::DropObjects(CLibraryList* pList, BOOL bCopy)
{
	if ( m_pDropItem == NULL ) return FALSE;
	
	CWaitCursor pCursor;
	
	if ( pList == NULL )
	{
	}
	else if ( m_pDropItem->m_pPhysical != NULL )
	{
		CFileCopyDlg dlg( NULL, ! bCopy );
		
		for ( POSITION pos = pList->GetHeadPosition() ; pos ; )
		{
			DWORD nObject = pList->GetNext( pos );

			if ( CLibraryFile* pFile = Library.LookupFile( nObject ) )
			{
				if ( pFile->m_pFolder != m_pDropItem->m_pPhysical )
					dlg.AddFile( pFile );
			}
		}

		dlg.m_sTarget = m_pDropItem->m_pPhysical->m_sPath;
		m_pDropItem = NULL;

		Library.Unlock();
		dlg.DoModal();
		Library.Lock();
	}
	else if ( m_pDropItem->m_pVirtual != NULL )
	{
		for ( POSITION pos = pList->GetHeadPosition() ; pos ; )
		{
			DWORD nObject = pList->GetNext( pos );

			if ( CLibraryFile* pFile = Library.LookupFile( nObject ) )
			{
				m_pDropItem->m_pVirtual->AddFile( pFile );

				for ( CLibraryTreeItem* pItem = m_pSelFirst ; pItem && ! bCopy ;
						pItem = pItem->m_pSelNext )
				{
					if ( pItem->m_pVirtual != NULL && pItem != m_pDropItem )
					{
						if ( LibraryFolders.CheckAlbum( pItem->m_pVirtual ) )
						{
							pItem->m_pVirtual->RemoveFile( pFile );
						}
					}
				}
			}
			else if ( LibraryFolders.CheckAlbum( (CAlbumFolder*)nObject ) )
			{
				CAlbumFolder* pFolder = (CAlbumFolder*)nObject;

				if ( pFolder != m_pDropItem->m_pVirtual &&
					 pFolder->m_pParent != m_pDropItem->m_pVirtual &&
					 pFolder->CheckFolder( m_pDropItem->m_pVirtual, TRUE ) == FALSE )
				{
					pFolder->m_pParent->OnFolderDelete( pFolder );
					pFolder->m_pParent = m_pDropItem->m_pVirtual;
					pFolder->m_pParent->m_pFolders.AddTail( pFolder );
					pFolder->m_pParent->m_nUpdateCookie++;
					Library.m_nUpdateCookie++;
				}
			}
		}
	}
	
	m_pDropItem = NULL;
	Invalidate();
	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView message handlers

int CLibraryTreeView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CLibraryTreeCtrl::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	m_wndFolderTip.Create( this );
	m_wndAlbumTip.Create( this );

	return 0;
}

BOOL CLibraryTreeView::PreTranslateMessage(MSG* pMsg) 
{
	if ( m_bVirtual )
	{
		if ( pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_DELETE )
		{
			OnLibraryFolderDelete();
			return TRUE;
		}
		else if ( pMsg->message == WM_SYSKEYDOWN && pMsg->wParam == VK_RETURN )
		{
			OnLibraryFolderProperties();
			return TRUE;
		}
	}
	else
	{
		if ( pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN )
		{
			OnLibraryExplore();
			return TRUE;
		}
	}

	return CLibraryTreeCtrl::PreTranslateMessage( pMsg );
}

void CLibraryTreeView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	if ( m_bVirtual )
	{
		Skin.TrackPopupMenu( _T("CLibraryTree.Virtual"), point, ID_LIBRARY_FOLDER_PROPERTIES );
	}
	else
	{
		Skin.TrackPopupMenu( _T("CLibraryTree.Physical"), point, ID_LIBRARY_EXPLORE );
	}
}

void CLibraryTreeView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	CLibraryTreeCtrl::OnLButtonDblClk( nFlags, point );
	
	if ( m_pFocus == NULL && ! m_bVirtual )
	{
		SelectAll();
		NotifySelection();
	}
}

void CLibraryTreeView::OnUpdateLibraryParent(CCmdUI* pCmdUI) 
{
	CLibraryFrame* pFrame = (CLibraryFrame*)GetParent();
	ASSERT_KINDOF(CLibraryFrame, pFrame);

	CCoolBarCtrl* pBar = &pFrame->m_wndViewTop;
	CCoolBarItem* pItem = pBar->GetID( ID_LIBRARY_PARENT );

	BOOL bAvailable = ( m_nSelected == 1 );

	if ( pItem == pCmdUI ) pItem->Show( bAvailable );
	pCmdUI->Enable( bAvailable );
}

void CLibraryTreeView::OnLibraryParent() 
{
	CLibraryTreeItem* pNew = NULL;

	if ( m_nSelected == 1 && m_pSelFirst->m_pParent != m_pRoot )
	{
		pNew = m_pSelFirst->m_pParent;
	}

	DeselectAll( pNew );

	if ( pNew != NULL )
	{
		Select( pNew );
		Highlight( pNew );
	}

	Invalidate();
	NotifySelection();
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView physical command handlers

void CLibraryTreeView::OnUpdateLibraryExplore(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( ! m_bVirtual && m_nSelected == 1 );
}

void CLibraryTreeView::OnLibraryExplore() 
{
	if ( m_bVirtual || m_nSelected != 1 || m_pSelFirst == NULL ) return;
	
	CSingleLock pLock( &Library.m_pSection, TRUE );
	if ( ! LibraryFolders.CheckFolder( m_pSelFirst->m_pPhysical, TRUE ) ) return;
	CString strPath = m_pSelFirst->m_pPhysical->m_sPath;
	pLock.Unlock();
	
	CFileExecutor::Execute( strPath, TRUE );
}

void CLibraryTreeView::OnUpdateLibraryScan(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( ! m_bVirtual && m_nSelected > 0 );
}

void CLibraryTreeView::OnLibraryScan() 
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	for ( CLibraryTreeItem* pItem = m_pSelFirst ; pItem ; pItem = pItem->m_pSelNext )
	{
		if ( LibraryFolders.CheckFolder( pItem->m_pPhysical, TRUE ) ) pItem->m_pPhysical->Scan();
	}
}

void CLibraryTreeView::OnUpdateLibraryShared(CCmdUI* pCmdUI) 
{
	CSingleLock pLock( &Library.m_pSection );
	if ( ! pLock.Lock( 50 ) ) return;

	TRISTATE bShared = TS_UNKNOWN;

	for ( CLibraryTreeItem* pItem = m_pSelFirst ; pItem ; pItem = pItem->m_pSelNext )
	{
		if ( LibraryFolders.CheckFolder( pItem->m_pPhysical, TRUE ) )
		{
			if ( bShared == TS_UNKNOWN )
			{
				bShared = pItem->m_pPhysical->IsShared() ? TS_TRUE : TS_FALSE;
			}
			else if ( ( bShared == TS_TRUE ) != pItem->m_pPhysical->IsShared() )
			{
				pCmdUI->Enable( FALSE );
				return;
			}
		}
	}

	pCmdUI->Enable( m_nSelected > 0 );
	pCmdUI->SetCheck( bShared == TS_TRUE );
}

void CLibraryTreeView::OnLibraryShared() 
{
	Library.Lock();

	for ( CLibraryTreeItem* pItem = m_pSelFirst ; pItem ; pItem = pItem->m_pSelNext )
	{
		if ( LibraryFolders.CheckFolder( pItem->m_pPhysical, TRUE ) )
		{
			BOOL bShared = pItem->m_pPhysical->IsShared();
			pItem->m_pPhysical->m_bShared = TS_UNKNOWN;
			
			if ( bShared )
				pItem->m_pPhysical->m_bShared = pItem->m_pPhysical->IsShared() ? TS_FALSE : TS_UNKNOWN;
			else
				pItem->m_pPhysical->m_bShared = pItem->m_pPhysical->IsShared() ? TS_UNKNOWN : TS_TRUE;
			pItem->m_pPhysical->m_nUpdateCookie++;
		}
	}

	Library.Unlock( TRUE );
	PostUpdate();
}

void CLibraryTreeView::OnUpdateLibraryRemove(CCmdUI* pCmdUI) 
{
	CSingleLock pLock( &Library.m_pSection );
	if ( ! pLock.Lock( 50 ) ) return;

	for ( CLibraryTreeItem* pItem = m_pSelFirst ; pItem ; pItem = pItem->m_pSelNext )
	{
		if ( LibraryFolders.CheckFolder( pItem->m_pPhysical, TRUE ) )
		{
			if ( pItem->m_pPhysical->m_pParent == NULL )
			{
				pCmdUI->Enable( TRUE );
				return;
			}
		}
	}

	pCmdUI->Enable( FALSE );
}

void CLibraryTreeView::OnLibraryRemove() 
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	for ( CLibraryTreeItem* pItem = m_pSelFirst ; pItem ; pItem = pItem->m_pSelNext )
	{
		if ( LibraryFolders.CheckFolder( pItem->m_pPhysical, TRUE ) )
		{
			if ( pItem->m_pPhysical->m_pParent == NULL )
			{
				LibraryFolders.RemoveFolder( pItem->m_pPhysical );
			}
		}
	}

	Library.Save();
	PostUpdate();
}

void CLibraryTreeView::OnLibraryAdd() 
{
	TCHAR szPath[MAX_PATH];
	LPITEMIDLIST pPath;
	LPMALLOC pMalloc;
	BROWSEINFO pBI;
		
	ZeroMemory( &pBI, sizeof(pBI) );
	pBI.hwndOwner		= AfxGetMainWnd()->GetSafeHwnd();
	pBI.pszDisplayName	= szPath;
	pBI.lpszTitle		= _T("Select folder to share:");
	pBI.ulFlags			= BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	
	pPath = SHBrowseForFolder( &pBI );

	if ( pPath == NULL ) return;

	SHGetPathFromIDList( pPath, szPath );
	SHGetMalloc( &pMalloc );
	pMalloc->Free( pPath );

	CFolderScanDlg dlgScan;

	LibraryFolders.AddFolder( szPath );

	dlgScan.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryTreeView virtual command handlers

void CLibraryTreeView::OnUpdateLibraryFolderEnqueue(CCmdUI* pCmdUI) 
{
	if ( ! Library.Lock( 50 ) ) return;

	for ( CLibraryTreeItem* pItem = m_pSelFirst ; pItem ; pItem = pItem->m_pSelNext )
	{
		if ( LibraryFolders.CheckAlbum( pItem->m_pVirtual ) && pItem->m_pVirtual->GetFileCount() > 0 )
		{
			pCmdUI->Enable( TRUE );
			Library.Unlock();
			return;
		}
	}

	pCmdUI->Enable( FALSE );
	Library.Unlock();
}

void CLibraryTreeView::OnLibraryFolderEnqueue() 
{
	CStringList pList;

	if ( ! Library.Lock( 50 ) ) return;

	for ( CLibraryTreeItem* pItem = m_pSelFirst ; pItem ; pItem = pItem->m_pSelNext )
	{
		if ( LibraryFolders.CheckAlbum( pItem->m_pVirtual ) )
		{
			for ( POSITION pos = pItem->m_pVirtual->GetFileIterator() ; pos ; )
			{
				CLibraryFile* pFile = pItem->m_pVirtual->GetNextFile( pos );
				pList.AddTail( pFile->GetPath() );
			}
		}
	}

	Library.Unlock();

	for ( POSITION pos = pList.GetHeadPosition() ; pos ; )
	{
		CString strPath = pList.GetNext( pos );
		CFileExecutor::Enqueue( strPath );
	}
}

void CLibraryTreeView::OnUpdateLibraryFolderMetadata(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_nSelected > 0 && m_pSelFirst->m_pVirtual != NULL );
}

void CLibraryTreeView::OnLibraryFolderMetadata() 
{
	Library.Lock();
	
	for ( CLibraryTreeItem* pItem = m_pSelFirst ; pItem ; pItem = pItem->m_pSelNext )
	{
		CAlbumFolder* pFolder = pItem->m_pVirtual;
		if ( LibraryFolders.CheckAlbum( pFolder ) ) pFolder->MetaToFiles( TRUE );
	}

	Library.Unlock( TRUE );
}

void CLibraryTreeView::OnUpdateLibraryFolderDelete(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_nSelected > 0 && m_pSelFirst->m_pVirtual != NULL );
}

void CLibraryTreeView::OnLibraryFolderDelete() 
{
	if ( m_pSelFirst == NULL || m_pSelFirst->m_pVirtual == NULL ) return;
	
	CString strFormat, strMessage;
	Skin.LoadString( strFormat, IDS_LIBRARY_FOLDER_DELETE );
	strMessage.Format( strFormat, m_nSelected );

	if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_OKCANCEL ) != IDOK ) return;

	Library.Lock();

	for ( CLibraryTreeItem* pItem = m_pSelFirst ; pItem ; pItem = pItem->m_pSelNext )
	{
		CAlbumFolder* pFolder = pItem->m_pVirtual;
		if ( LibraryFolders.CheckAlbum( pFolder ) ) pFolder->Delete();
	}

	Library.Unlock();

	NotifySelection();
}

void CLibraryTreeView::OnUpdateLibraryFolderNew(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_nSelected == 0 || ( m_nSelected == 1 && m_pSelFirst->m_pVirtual != NULL ) );
}

void CLibraryTreeView::OnLibraryFolderNew() 
{
	if ( m_pSelFirst != NULL && m_pSelFirst->m_pVirtual == NULL ) return;

	Library.Lock();

	CAlbumFolder* pFolder = Library.GetAlbumRoot();

	if ( m_pSelFirst ) pFolder = m_pSelFirst->m_pVirtual;
		
	pFolder = pFolder->AddFolder( NULL, _T("New Folder") );

	if ( m_pSelFirst ) Expand( m_pSelFirst, TS_TRUE, FALSE );

	NotifySelection();

	if ( CLibraryTreeItem* pItem = GetFolderItem( pFolder ) )
	{
		Select( pItem, TS_TRUE, FALSE );
		DeselectAll( pItem, NULL, FALSE );
	}

	Library.Unlock( TRUE );

	Invalidate();

	if ( pFolder ) PostMessage( WM_COMMAND, ID_LIBRARY_FOLDER_PROPERTIES );
}

void CLibraryTreeView::OnUpdateLibraryRebuild(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_nSelected > 0 );
}

void CLibraryTreeView::OnLibraryRebuild() 
{
	if ( ! Library.Lock( 50 ) ) return;
	
	CLibraryList pList;
	
	for ( CLibraryTreeItem* pItem = m_pSelFirst ; pItem ; pItem = pItem->m_pSelNext )
	{
		pItem->GetFileList( &pList, TRUE );
	}
	
	for ( POSITION pos = pList.GetIterator() ; pos ; )
	{
		if ( CLibraryFile* pFile = pList.GetNextFile( pos ) )
		{
			pFile->Rebuild();
		}
	}
	
	Library.Unlock( TRUE );
}

void CLibraryTreeView::OnUpdateLibraryFolderProperties(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_nSelected == 1 && m_pSelFirst->m_pVirtual != NULL );
}

void CLibraryTreeView::OnLibraryFolderProperties() 
{
	if ( m_pSelFirst == NULL || m_pSelFirst->m_pVirtual == NULL ) return;

	CAlbumFolder* pFolder = m_pSelFirst->m_pVirtual;

	CFolderPropertiesDlg dlg( NULL, pFolder );

	if ( dlg.DoModal() == IDOK )
	{
		NotifySelection();

		if ( CLibraryTreeItem* pItem = GetFolderItem( pFolder ) )
		{
			Select( pItem, TS_TRUE, FALSE );
			DeselectAll( pItem, NULL, FALSE );
			Invalidate();
			NotifySelection();
		}
	}
}

void CLibraryTreeView::OnUpdateLibraryFolderFileProperties(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_nSelected > 0 );
}

void CLibraryTreeView::OnLibraryFolderFileProperties() 
{
	CSingleLock pLock( &Library.m_pSection, TRUE );
	CLibraryList pList;
	
	for ( CLibraryTreeItem* pItem = m_pSelFirst ; pItem ; pItem = pItem->m_pSelNext )
	{
		pItem->GetFileList( &pList, TRUE );
	}
	
	pLock.Unlock();
	
	CFilePropertiesSheet dlg;
	dlg.Add( &pList );
	dlg.DoModal();
}

void CLibraryTreeView::OnUpdateLibraryExportCollection(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( m_bVirtual && m_nSelected == 1 );
}

void CLibraryTreeView::OnLibraryExportCollection()
{
	if ( m_pSelFirst == NULL || m_pSelFirst->m_pSelNext != NULL ) return;
	if ( m_pSelFirst->m_pVirtual == NULL ) return;
	if ( m_pSelFirst->m_pVirtual->m_pXML == NULL ) return;
	
	CCollectionExportDlg dlg( m_pSelFirst->m_pVirtual );
	dlg.DoModal();
}
