//
// CtrlMediaList.cpp
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
#include "SharedFile.h"
#include "ShellIcons.h"
#include "LiveList.h"
#include "Skin.h"
#include "Buffer.h"
#include "CtrlMediaList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CMediaListCtrl, CListCtrl)

BEGIN_MESSAGE_MAP(CMediaListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CMediaListCtrl)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDoubleClick)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_MEDIA_ADD, OnMediaAdd)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_REMOVE, OnUpdateMediaRemove)
	ON_COMMAND(ID_MEDIA_REMOVE, OnMediaRemove)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_CLEAR, OnUpdateMediaClear)
	ON_COMMAND(ID_MEDIA_CLEAR, OnMediaClear)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_SELECT, OnUpdateMediaSelect)
	ON_COMMAND(ID_MEDIA_SELECT, OnMediaSelect)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_SAVE, OnUpdateMediaSave)
	ON_COMMAND(ID_MEDIA_SAVE, OnMediaSave)
	ON_COMMAND(ID_MEDIA_OPEN, OnMediaOpen)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_PREVIOUS, OnUpdateMediaPrevious)
	ON_COMMAND(ID_MEDIA_PREVIOUS, OnMediaPrevious)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_NEXT, OnUpdateMediaNext)
	ON_COMMAND(ID_MEDIA_NEXT, OnMediaNext)
	ON_NOTIFY_REFLECT(LVN_KEYDOWN, OnKeyDown)
	ON_WM_SIZE()
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, OnBeginDrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_UPDATE_COMMAND_UI(ID_MEDIA_REPEAT, OnUpdateMediaRepeat)
	ON_COMMAND(ID_MEDIA_REPEAT, OnMediaRepeat)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_RANDOM, OnUpdateMediaRandom)
	ON_COMMAND(ID_MEDIA_RANDOM, OnMediaRandom)
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_MEDIA_ADD_FOLDER, OnMediaAddFolder)
	//}}AFX_MSG_MAP
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
END_MESSAGE_MAP()

#define STATE_CURRENT	(1<<12)
#define STATE_PLAYED	(1<<13)


/////////////////////////////////////////////////////////////////////////////
// CMediaListCtrl construction

CMediaListCtrl::CMediaListCtrl()
{
	m_pDragImage		= NULL;
	m_bCreateDragImage	= FALSE;
}

CMediaListCtrl::~CMediaListCtrl()
{
}

/////////////////////////////////////////////////////////////////////////////
// CMediaListCtrl operations

BOOL CMediaListCtrl::Create(CWnd* pParentWnd, UINT nID) 
{
	CRect rect;

	return CListCtrl::Create(
		WS_CHILD|WS_VSCROLL|LVS_ICON|LVS_REPORT|LVS_SHAREIMAGELISTS|LVS_NOCOLUMNHEADER,
		rect, pParentWnd, nID );
}

BOOL CMediaListCtrl::Open(LPCTSTR pszFile)
{
	Clear();
	
	Enqueue( pszFile, FALSE );
	
	if ( GetItemCount() == 0 ) return FALSE;
	
	GetNext();
	
	return TRUE;
}

BOOL CMediaListCtrl::Enqueue(LPCTSTR pszFile, BOOL bStart)
{
	if ( _tcsistr( pszFile, _T(".m3u") ) != NULL ||
		 _tcsistr( pszFile, _T(".pls") ) != NULL )
	{
		LoadTextList( pszFile );
	}
	else
	{
		Add( pszFile );
	}
	
	if ( bStart && GetItemCount() == 1 ) GetNext();
	
	return TRUE;
}

int CMediaListCtrl::RecursiveEnqueue(LPCTSTR pszPath)
{
	WIN32_FIND_DATA pFind;
	CString strPath;
	HANDLE hSearch;
	int nCount = 0;
	
	strPath.Format( _T("%s\\*.*"), pszPath );
	
	hSearch = FindFirstFile( strPath, &pFind );
	
	if ( hSearch != INVALID_HANDLE_VALUE )
	{
		do
		{
			if ( pFind.cFileName[0] == '.' ) continue;
			if ( _tcsicmp( pFind.cFileName, _T("Metadata") ) == 0 ) continue;
			
			strPath.Format( _T("%s\\%s"), pszPath, pFind.cFileName );
			
			if ( pFind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				nCount += RecursiveEnqueue( strPath );
			}
			else
			{
				nCount += Enqueue( strPath, FALSE );
			}
		}
		while ( FindNextFile( hSearch, &pFind ) );
		
		FindClose( hSearch );
	}
	
	return nCount;
}

void CMediaListCtrl::Remove(LPCTSTR pszFile)
{
	for ( int nItem = GetItemCount() - 1 ; nItem >= 0 ; nItem-- )
	{
		if ( GetItemText( nItem, 1 ).CompareNoCase( pszFile ) == 0 )
		{
			Remove( nItem );
		}
	}
}

BOOL CMediaListCtrl::LoadTextList(LPCTSTR pszFile)
{
	CString strPath, strItem;
	CFile pFile;
	
	strPath = pszFile;
	strPath = strPath.Left( strPath.ReverseFind( '\\' ) + 1 );
	
	if ( ! pFile.Open( pszFile, CFile::modeRead ) ) return FALSE;
	
	CBuffer pBuffer;
	pBuffer.EnsureBuffer( (DWORD)pFile.GetLength() );
	pBuffer.m_nLength = (DWORD)pFile.GetLength();
	pFile.Read( pBuffer.m_pBuffer, pBuffer.m_nLength );
	pFile.Close();
	
	while ( pBuffer.ReadLine( strItem ) )
	{
		strItem.TrimLeft();
		strItem.TrimRight();

		if ( strItem.GetLength() && strItem.GetAt( 0 ) != '#' )
		{
			if ( strItem.Find( '\\' ) != 0 &&
				 strItem.Find( ':' ) != 1 )
				strItem = strPath + strItem;

			if ( GetFileAttributes( strItem ) != 0xFFFFFFFF )
				Enqueue( strItem, FALSE );
		}
	}
	
	return TRUE;
}

int CMediaListCtrl::Add(LPCTSTR pszPath, int nItem)
{
	LPCTSTR pszFile = _tcsrchr( pszPath, '\\' );
	pszFile = pszFile ? pszFile + 1 : pszPath;
	
	DWORD nFile = 0;
	if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( pszPath, TRUE ) )
	{
		nFile = pFile->m_nIndex;
		Library.Unlock();
	}
	
	LV_ITEM pItem;
	ZeroMemory( &pItem, sizeof(pItem) );
	pItem.mask		= LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
	pItem.iItem		= nItem >= 0 ? nItem : GetItemCount();
	pItem.iImage	= ShellIcons.Get( pszFile, 16 );
	pItem.lParam	= nFile;
	pItem.pszText	= (LPTSTR)pszFile;
	pItem.iItem		= InsertItem( &pItem );
	pItem.mask		= LVIF_TEXT;
	pItem.iSubItem	= 1;
	pItem.pszText	= (LPTSTR)pszPath;
	SetItem( &pItem );
	
	return pItem.iItem;
}

void CMediaListCtrl::Remove(int nItem)
{
	if ( nItem < 0 || nItem >= GetItemCount() ) return;
	
	BOOL bActive = GetItemState( nItem, STATE_CURRENT );
	DeleteItem( nItem );
	
	if ( bActive ) SetCurrent( nItem );
}

int CMediaListCtrl::GetCount()
{
	return GetItemCount();
}

void CMediaListCtrl::Clear()
{
	DeleteAllItems();
	SetCurrent( -1 );
}

int CMediaListCtrl::GetCurrent()
{
	for ( int nItem = GetItemCount() - 1 ; nItem >= 0 ; nItem-- )
	{
		if ( GetItemState( nItem, STATE_CURRENT ) ) return nItem;
	}

	return -1;
}

void CMediaListCtrl::SetCurrent(int nCurrent)
{
	for ( int nItem = GetItemCount() - 1 ; nItem >= 0 ; nItem-- )
	{
		if ( GetItemState( nItem, STATE_CURRENT ) )
		{
			if ( nItem != nCurrent ) SetItemState( nItem, 0, STATE_CURRENT );
		}
		else
		{
			if ( nItem == nCurrent ) SetItemState( nItem, STATE_CURRENT, STATE_CURRENT );
		}
	}

	NMHDR pNM = { GetSafeHwnd(), GetDlgCtrlID(), MLN_NEWCURRENT };
	GetOwner()->SendMessage( WM_NOTIFY, pNM.idFrom, (LPARAM)&pNM );
}

int CMediaListCtrl::GetNext(BOOL bSet)
{
	int nItem = GetCurrent();

	if ( nItem >= 0 ) SetItemState( nItem, STATE_PLAYED, STATE_PLAYED );

	if ( Settings.MediaPlayer.Random )
	{
		int nCount = 0;

		for ( nItem = GetItemCount() - 1 ; nItem >= 0 ; nItem-- )
		{
			if ( GetItemState( nItem, STATE_PLAYED ) == 0 ) nCount++;
		}

		if ( nCount > 0 )
		{
			nCount = rand() % nCount;

			for ( nItem = GetItemCount() - 1 ; nItem >= 0 ; nItem-- )
			{
				if ( GetItemState( nItem, STATE_PLAYED ) == 0 )
				{
					if ( nCount-- == 0 ) break;
				}
			}
		}
	}
	else
	{
		if ( ++nItem >= GetItemCount() ) nItem = -1;
	}

	if ( bSet ) SetCurrent( nItem );

	return nItem;
}

void CMediaListCtrl::Reset(BOOL bNext)
{
	for ( int nItem = GetItemCount() - 1 ; nItem >= 0 ; nItem-- )
		SetItemState( nItem, 0, STATE_PLAYED );
	if ( bNext ) SetCurrent( -1 );
}

CString CMediaListCtrl::GetPath(int nItem)
{
	CString str;
	if ( nItem < 0 || nItem >= GetItemCount() ) return str;
	return GetItemText( nItem, 1 );
}

/////////////////////////////////////////////////////////////////////////////
// CMediaListCtrl message handlers

int CMediaListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CListCtrl::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	SetBkColor( RGB( 0, 0, 0x30 ) );
	SetImageList( ShellIcons.GetObject( 16 ), LVSIL_SMALL );
	InsertColumn( 0, _T("Name"), LVCFMT_LEFT, 100, -1 );
	InsertColumn( 1, _T("Path"), LVCFMT_LEFT, 0, 0 );

	// SendMessage( LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_LABELTIP, LVS_EX_LABELTIP );
	
	m_wndTip.Create( this, &Settings.Interface.TipMedia );

	return 0;
}

void CMediaListCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CListCtrl::OnSize( nType, cx, cy );
	
	CRect rc;
	GetWindowRect( &rc );
	
	LV_COLUMN pColumn;
	pColumn.mask	= LVCF_WIDTH;
	pColumn.cx		= rc.Width() - GetSystemMetrics( SM_CXVSCROLL ) - 8;
	SetColumn( 0, &pColumn );
}

void CMediaListCtrl::OnCustomDraw(NMLVCUSTOMDRAW* pNotify, LRESULT* pResult)
{
	if ( pNotify->nmcd.dwDrawStage == CDDS_PREPAINT )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( pNotify->nmcd.dwDrawStage == CDDS_ITEMPREPAINT )
	{
		if (	GetItemState( pNotify->nmcd.dwItemSpec, LVIS_SELECTED ) == 0 &&
				GetItemState( pNotify->nmcd.dwItemSpec, STATE_CURRENT ) != 0 )
		{
			pNotify->clrText	= RGB( 255, 255, 255 );
			pNotify->clrTextBk	= RGB( 128, 0, 0 );
		}
		else
		{
			pNotify->clrText	= RGB( 255, 255, 255 );
			pNotify->clrTextBk	= RGB( 0, 0, 0x30 );
		}

		if ( m_bCreateDragImage ) pNotify->clrTextBk = RGB( 0, 255, 0 );

		*pResult = CDRF_DODEFAULT;
	}
	else
	{
		*pResult = 0;
	}
}

void CMediaListCtrl::OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnMediaSelect();
	*pResult = 0;
}

void CMediaListCtrl::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	Skin.TrackPopupMenu( _T("CMediaList"), point, ID_MEDIA_SELECT );
}

void CMediaListCtrl::OnKeyDown(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;
	
	if ( pLVKeyDow->wVKey == VK_RETURN )
		PostMessage( WM_COMMAND, ID_MEDIA_SELECT );
	else if ( pLVKeyDow->wVKey == VK_DELETE )
		PostMessage( WM_COMMAND, ID_MEDIA_REMOVE );
	
	m_wndTip.Hide();
	*pResult = 0;
}

void CMediaListCtrl::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	*pResult = 0;

	m_wndTip.Hide();

	CPoint ptAction( pNMListView->ptAction );

	m_bCreateDragImage = TRUE;
	m_pDragImage = CLiveList::CreateDragImage( this, ptAction );
	m_bCreateDragImage = FALSE;

	if ( m_pDragImage == NULL ) return;
	m_nDragDrop = -1;

	UpdateWindow();

	CRect rcClient;
	GetClientRect( &rcClient );
	ClientToScreen( &rcClient );
	ClipCursor( &rcClient );
	SetCapture();

	SetFocus();
	UpdateWindow();

	m_pDragImage->DragEnter( this, ptAction );
}

void CMediaListCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_wndTip.Hide();
	CListCtrl::OnLButtonDown( nFlags, point );
}

void CMediaListCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{
	m_wndTip.Hide();
	CListCtrl::OnRButtonDown( nFlags, point );
}

void CMediaListCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	int nHit = HitTest( point );

	if ( m_pDragImage != NULL )
	{
		m_pDragImage->DragMove( point );

		if ( nHit != m_nDragDrop )
		{
			CImageList::DragShowNolock( FALSE );
			if ( m_nDragDrop >= 0 ) SetItemState( m_nDragDrop, 0, LVIS_DROPHILITED );
			m_nDragDrop = nHit;
			if ( m_nDragDrop >= 0 ) SetItemState( m_nDragDrop, LVIS_DROPHILITED, LVIS_DROPHILITED );
			UpdateWindow();
			CImageList::DragShowNolock( TRUE );
		}
	}
	else
	{
		DWORD nFile = nHit >= 0 ? GetItemData( nHit ) : 0;

		if ( nFile > 0 && ! Library.LookupFile( nFile ) ) nFile = 0;

		if ( nFile > 0 )
		{
			m_wndTip.Show( (LPVOID)nFile );
		}
		else
		{
			m_wndTip.Hide();
		}
	}

	CListCtrl::OnMouseMove( nFlags, point );
}

void CMediaListCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if ( m_pDragImage == NULL )
	{
		CListCtrl::OnLButtonUp( nFlags, point );
		return;
	}

	ClipCursor( NULL );
	ReleaseCapture();

	m_pDragImage->DragLeave( this );
	m_pDragImage->EndDrag();
	delete m_pDragImage;
	m_pDragImage = NULL;

	if ( m_nDragDrop >= 0 )
		SetItemState( m_nDragDrop, 0, LVIS_DROPHILITED );
	else
		m_nDragDrop = GetItemCount();

	CStringArray pPaths;
	CPtrArray pStates;

	while ( TRUE )
	{
		int nItem = GetNextItem( -1, LVNI_SELECTED );
		if ( nItem < 0 ) break;

		pPaths.Add( GetItemText( nItem, 1 ) );
		pStates.Add( (LPVOID)GetItemState( nItem, 0xFFFFFFFF ) );

		DeleteItem( nItem );

		if ( m_nDragDrop > nItem ) m_nDragDrop --;
	}

	for ( int nItem = 0 ; nItem < pPaths.GetSize() ; nItem++ )
	{
		int nIndex = Add( pPaths.GetAt( nItem ), m_nDragDrop++ );
		SetItemState( nIndex, (DWORD)pStates.GetAt( nItem ), 0xFFFFFFFF );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMediaListCtrl command handlers

void CMediaListCtrl::OnUpdateMediaSelect(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetSelectedCount() == 1 );
}

void CMediaListCtrl::OnMediaSelect() 
{
	int nItem = GetNextItem( -1, LVNI_SELECTED );

	if ( nItem >= 0 )
	{
		SetCurrent( nItem );
		SetFocus();
		Invalidate();
	}
}

void CMediaListCtrl::OnMediaAdd() 
{
	if ( ! AfxGetMainWnd()->IsWindowEnabled() ) return;
	
	CString strFilter;
	Skin.LoadString( strFilter, IDS_MEDIA_FILTER );

	CFileDialog dlg( TRUE, NULL, NULL,
		OFN_HIDEREADONLY|OFN_ALLOWMULTISELECT|OFN_ENABLESIZING,
		strFilter, this );

	TCHAR szFiles[81920];
	dlg.m_ofn.lpstrFile	= szFiles;
	dlg.m_ofn.nMaxFile	= 81920;

	if ( dlg.DoModal() != IDOK ) return;
	
	CString strFolder	= CString( szFiles );
	LPCTSTR pszFile		= szFiles + strFolder.GetLength() + 1;

	BOOL bWasEmpty = ( GetItemCount() == 0 );

	if ( *pszFile )
	{
		for ( strFolder += '\\' ; *pszFile ; )
		{
			Enqueue( strFolder + pszFile, FALSE );
			pszFile += _tcslen( pszFile ) + 1;
		}
	}
	else
	{
		Enqueue( strFolder, FALSE );
	}

	if ( GetItemCount() > 0 && bWasEmpty ) GetNext();
}

void CMediaListCtrl::OnMediaAddFolder() 
{
	if ( ! AfxGetMainWnd()->IsWindowEnabled() ) return;
	
	TCHAR szPath[MAX_PATH];
	LPITEMIDLIST pPath;
	LPMALLOC pMalloc;
	BROWSEINFO pBI;
		
	ZeroMemory( &pBI, sizeof(pBI) );
	pBI.hwndOwner		= AfxGetMainWnd()->GetSafeHwnd();
	pBI.pszDisplayName	= szPath;
	pBI.lpszTitle		= _T("Select folder to add to playlist:");
	pBI.ulFlags			= BIF_RETURNONLYFSDIRS;
	
	pPath = SHBrowseForFolder( &pBI );

	if ( pPath == NULL ) return;

	SHGetPathFromIDList( pPath, szPath );
	SHGetMalloc( &pMalloc );
	pMalloc->Free( pPath );
	
	BOOL bWasEmpty = ( GetItemCount() == 0 );
	
	RecursiveEnqueue( szPath );

	if ( GetItemCount() > 0 && bWasEmpty ) GetNext();
}

void CMediaListCtrl::OnUpdateMediaRemove(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetSelectedCount() > 0 );
}

void CMediaListCtrl::OnMediaRemove() 
{
	for ( int nItem = GetItemCount() - 1 ; nItem >= 0 ; nItem-- )
	{
		if ( GetItemState( nItem, LVIS_SELECTED ) ) Remove( nItem );
	}
}

void CMediaListCtrl::OnUpdateMediaClear(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetItemCount() > 0 );
}

void CMediaListCtrl::OnMediaClear() 
{
	Clear();
}

void CMediaListCtrl::OnMediaOpen() 
{
	CString strFilter;
	Skin.LoadString( strFilter, IDS_MEDIA_FILTER );
	CFileDialog dlg( TRUE, NULL, NULL, OFN_HIDEREADONLY|OFN_ENABLESIZING, strFilter, this );

	if ( dlg.DoModal() != IDOK ) return;

	Open( dlg.GetPathName() );
}

void CMediaListCtrl::OnUpdateMediaSave(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetItemCount() > 0 );
}

void CMediaListCtrl::OnMediaSave() 
{
	CFileDialog dlg( FALSE, _T("m3u"), NULL, OFN_HIDEREADONLY|OFN_ENABLESIZING,
		_T("Media Playlists|*.m3u|All Files|*.*||"), this );

	if ( dlg.DoModal() != IDOK ) return;

	CString strFile, strPath;
	CFile pFile;

	strPath = dlg.GetPathName();
	strPath = strPath.Left( strPath.ReverseFind( '\\' ) + 1 );

	for ( int nItem = 0 ; nItem < GetItemCount() ; nItem++ )
	{
		CString strItem = GetItemText( nItem, 1 );
		
		if ( _tcsnicmp( strPath, strItem, strPath.GetLength() ) == 0 )
			strItem = strItem.Mid( strPath.GetLength() );

		strFile += strItem + _T("\r\n");
	}

	if ( ! pFile.Open( dlg.GetPathName(), CFile::modeWrite|CFile::modeCreate ) ) return;

	USES_CONVERSION;
	LPCSTR pszFile = T2CA( (LPCTSTR)strFile );
	
	pFile.Write( pszFile, strlen(pszFile) );
	pFile.Close();
}

void CMediaListCtrl::OnUpdateMediaPrevious(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetCurrent() > 0 );
}

void CMediaListCtrl::OnMediaPrevious() 
{
	int nCurrent = GetCurrent();
	if ( nCurrent > 0 ) SetCurrent( nCurrent - 1 );
}

void CMediaListCtrl::OnUpdateMediaNext(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetItemCount() > 1 );
}

void CMediaListCtrl::OnMediaNext() 
{
	GetNext();
}

void CMediaListCtrl::OnUpdateMediaRepeat(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( Settings.MediaPlayer.Repeat );
}

void CMediaListCtrl::OnMediaRepeat() 
{
	Settings.MediaPlayer.Repeat = ! Settings.MediaPlayer.Repeat;
}

void CMediaListCtrl::OnUpdateMediaRandom(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( Settings.MediaPlayer.Random );
}

void CMediaListCtrl::OnMediaRandom() 
{
	Settings.MediaPlayer.Random = ! Settings.MediaPlayer.Random;
}
