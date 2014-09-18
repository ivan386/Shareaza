//
// CtrlMediaList.cpp
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
#include "AlbumFolder.h"
#include "Buffer.h"
#include "CoolInterface.h"
#include "CtrlMediaList.h"
#include "DlgCollectionExport.h"
#include "DlgFilePropertiesSheet.h"
#include "Library.h"
#include "LiveList.h"
#include "SchemaCache.h"
#include "SharedFile.h"
#include "ShellIcons.h"
#include "Skin.h"
#include "WndMedia.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CMediaListCtrl, CListCtrl)

BEGIN_MESSAGE_MAP(CMediaListCtrl, CListCtrl)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_NOTIFY_REFLECT(LVN_KEYDOWN, OnKeyDown)
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, OnBeginDrag)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDoubleClick)
	ON_COMMAND(ID_MEDIA_ADD, OnMediaAdd)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_RATE, OnUpdateMediaRate)
	ON_COMMAND(ID_MEDIA_RATE, OnMediaRate)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_PROPERTIES, OnUpdateMediaProperties)
	ON_COMMAND(ID_MEDIA_PROPERTIES, OnMediaProperties)
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
	ON_UPDATE_COMMAND_UI(ID_MEDIA_REPEAT, OnUpdateMediaRepeat)
	ON_COMMAND(ID_MEDIA_REPEAT, OnMediaRepeat)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_RANDOM, OnUpdateMediaRandom)
	ON_COMMAND(ID_MEDIA_RANDOM, OnMediaRandom)
	ON_COMMAND(ID_MEDIA_ADD_FOLDER, OnMediaAddFolder)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_EXPORT_COLLECTION, OnUpdateMediaCollection)
	ON_COMMAND(ID_MEDIA_EXPORT_COLLECTION, OnMediaCollection)
END_MESSAGE_MAP()

#define STATE_CURRENT	(1<<12)
#define STATE_PLAYED	(1<<13)

/////////////////////////////////////////////////////////////////////////////
// CMediaListCtrl construction

CMediaListCtrl::CMediaListCtrl()
	: m_pDragImage		( NULL )
	, m_bCreateDragImage( FALSE )
	, m_tLastUpdate		( 0 )
	, m_nSelectedCount	( 0 )
{
}

CMediaListCtrl::~CMediaListCtrl()
{
}

UINT CMediaListCtrl::GetSelectedCount()
{
	DWORD tNow = GetTickCount();
	if ( tNow > m_tLastUpdate + 250 || tNow < m_tLastUpdate )
	{
		m_tLastUpdate = tNow;
		m_nSelectedCount = CListCtrl::GetSelectedCount();
	}
	return m_nSelectedCount;
}

/////////////////////////////////////////////////////////////////////////////
// CMediaListCtrl operations

BOOL CMediaListCtrl::Create(CWnd* pParentWnd, UINT nID) 
{
	CRect rect;

	return CListCtrl::Create(
		WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | WS_VSCROLL | LVS_ICON | LVS_REPORT | LVS_SHAREIMAGELISTS | LVS_NOCOLUMNHEADER,
		rect, pParentWnd, nID );
}

BOOL CMediaListCtrl::Play(LPCTSTR pszFile)
{
	Clear();
	
	Enqueue( pszFile );
	
	if ( GetItemCount() == 0 ) return FALSE;
	
	PlayNext();
	
	return TRUE;
}

int CMediaListCtrl::Enqueue(LPCTSTR pszFile)
{
	LPCTSTR szExt = PathFindExtension( pszFile );
	if ( _tcsicmp( szExt, _T(".m3u") ) == 0 || _tcsicmp( szExt, _T(".pls") ) == 0 )
	{
		return LoadTextList( pszFile );
	}

	Add( pszFile );

	return 1;
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
				nCount += Enqueue( strPath );
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

int CMediaListCtrl::LoadTextList(LPCTSTR pszFile)
{
	int nCount = 0;

	CString strPath = pszFile;
	strPath = strPath.Left( strPath.ReverseFind( '\\' ) + 1 );

	CFile pFile;
	if ( pFile.Open( pszFile, CFile::modeRead ) )
	{
		CBuffer pBuffer;
		pBuffer.EnsureBuffer( (DWORD)pFile.GetLength() );
		pBuffer.m_nLength = (DWORD)pFile.Read( pBuffer.m_pBuffer, (DWORD)pFile.GetLength() );

		CString strItem;
		while ( pBuffer.ReadLine( strItem ) )
		{
			strItem.Trim();

			if ( strItem.GetLength() && strItem.GetAt( 0 ) != '#' )
			{
				if ( strItem.Find( '\\' ) != 0 && strItem.Find( ':' ) != 1 )
					// Relative path
					strItem = strPath + strItem;

				if ( GetFileAttributes( strItem ) != INVALID_FILE_ATTRIBUTES )
					nCount += Enqueue( strItem );
			}
		}
	}

	return nCount;
}

BOOL CMediaListCtrl::SaveTextList(LPCTSTR pszFile)
{
	CString strPath = pszFile;
	strPath = strPath.Left( strPath.ReverseFind( '\\' ) + 1 );

	CString strFile;
	for ( int nItem = 0 ; nItem < GetItemCount() ; nItem++ )
	{
		CString strItem = GetItemText( nItem, 1 );

		if ( _tcsnicmp( strPath, strItem, strPath.GetLength() ) == 0 )
			// Relative path
			strItem = strItem.Mid( strPath.GetLength() );

		strFile += strItem + _T("\r\n");
	}

	CFile pFile;
	if ( ! pFile.Open( pszFile, CFile::modeWrite | CFile::modeCreate ) )
		return FALSE;

	CT2CA strFileA( (LPCTSTR)strFile );

	pFile.Write( strFileA, static_cast< UINT >( strlen( strFileA ) ) );

	return TRUE;
}

int CMediaListCtrl::Add(LPCTSTR pszPath, int nItem)
{
	LPCTSTR pszFile = _tcsrchr( pszPath, '\\' );
	pszFile = pszFile ? pszFile + 1 : pszPath;
	
	DWORD nFile = 0;
	{
		CQuickLock oLock( Library.m_pSection );
		if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( pszPath ) )
		{
			nFile = pFile->m_nIndex;
		}
	}
	
	CString strTemp = (LPTSTR)pszFile;
	int nDotPos = strTemp.ReverseFind( '.' );
	if ( nDotPos != -1 ) strTemp = strTemp.Left( nDotPos );
	LPTSTR pszFileTmp = strTemp.GetBuffer( strTemp.GetLength() );
	LV_ITEM pItem = {};
	pItem.mask		= LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
	pItem.iItem		= nItem >= 0 ? nItem : GetItemCount();
	pItem.iImage	= ShellIcons.Get( pszFile, 16 );
	pItem.lParam	= nFile;
	pItem.pszText	= pszFileTmp;
	pItem.iItem		= InsertItem( &pItem );
	pItem.mask		= LVIF_TEXT;
	pItem.iSubItem	= 1;
	pItem.pszText	= (LPTSTR)pszPath;
	SetItem( &pItem );
	strTemp.ReleaseBuffer();
	return pItem.iItem;
}

void CMediaListCtrl::Remove(int nItem)
{
	if ( nItem < 0 || nItem >= GetItemCount() ) return;
	
	BOOL bActive = IsCurrent( nItem );
	DeleteItem( nItem );
	
	if ( bActive ) SetCurrent( nItem );
}

int CMediaListCtrl::GetCount() const
{
	return GetItemCount();
}

void CMediaListCtrl::Clear()
{
	DeleteAllItems();

	SetCurrent( -1 );
}

int CMediaListCtrl::GetCurrent() const
{
	for ( int nItem = GetItemCount() - 1 ; nItem >= 0 ; nItem-- )
	{
		if ( IsCurrent( nItem ) )
			return nItem;
	}

	return -1;
}

BOOL CMediaListCtrl::IsCurrent(int nItem) const
{
	return ( nItem >= 0 && nItem < GetItemCount() ) && ( GetItemState( nItem, STATE_CURRENT ) == STATE_CURRENT );
}

void CMediaListCtrl::SetCurrent(int nCurrent)
{
	for ( int nItem = GetItemCount() - 1 ; nItem >= 0 ; nItem-- )
	{
		if ( IsCurrent( nItem ) )
		{
			if ( nItem != nCurrent ) SetItemState( nItem, 0, STATE_CURRENT );
		}
		else
		{
			if ( nItem == nCurrent ) SetItemState( nItem, STATE_CURRENT, STATE_CURRENT );
		}
	}

	if ( CMediaFrame* pFrame = static_cast< CMediaFrame* >( GetParent() ) )
	{
		pFrame->PlayCurrent();
	}
}

void CMediaListCtrl::SetPlayed(int nItem)
{
	if ( nItem >= 0 && nItem < GetItemCount() )
		SetItemState( nItem, STATE_PLAYED, STATE_PLAYED );
}

void CMediaListCtrl::ClearPlayed()
{
	for ( int nItem = GetItemCount() - 1 ; nItem >= 0 ; nItem-- )
		if ( IsPlayed( nItem ) )
			SetItemState( nItem, 0, STATE_PLAYED );
}

BOOL CMediaListCtrl::IsPlayed(int nItem) const
{
	return ( nItem >= 0 && nItem < GetItemCount() ) && ( GetItemState( nItem, STATE_PLAYED ) == STATE_PLAYED );
}

int CMediaListCtrl::GetUnplayedCount() const
{
	int nCount = 0;
	for ( int nItem = GetItemCount() - 1 ; nItem >= 0 ; nItem-- )
	{
		if ( ! IsPlayed( nItem ) )
			nCount++;
	}
	return nCount;
}

void CMediaListCtrl::PlayNext()
{
	int nTotal = GetCount();
	if ( nTotal > 0 )
	{
		if ( Settings.MediaPlayer.Random )
		{
			int nCount = GetUnplayedCount();
			if ( nCount == 0 )
			{
				if ( Settings.MediaPlayer.Repeat )
				{
					// Play again except current file
					ClearPlayed();
					if ( nTotal > 1 )
					{
						SetPlayed( GetCurrent () );
						nCount = nTotal - 1;
					}
					else
						nCount = nTotal;
				}
			}
			if ( nCount > 0 )
			{
				nCount = GetRandomNum( 0, nCount - 1 );
				for ( int nItem = nTotal - 1 ; nItem >= 0 ; nItem-- )
				{
					if ( ! IsPlayed( nItem ) )
					{
						if ( nCount-- == 0 )
						{
							SetCurrent( nItem );
							return;
						}
					}
				}
			}
		}
		else
		{
			int nItem = GetCurrent();
			if ( ++nItem >= nTotal )
			{
				if ( Settings.MediaPlayer.Repeat )
				{
					ClearPlayed();
					SetCurrent( 0 );
					return;
				}
			}
			else
			{
				SetCurrent( nItem );
				return;
			}
		}
	}

	Reset();
}

void CMediaListCtrl::Reset()
{
	ClearPlayed();

	SetCurrent( -1 );
}

CString CMediaListCtrl::GetPath(int nItem) const
{
	if ( nItem < 0 || nItem >= GetItemCount() )
		return CString();

	return GetItemText( nItem, 1 );
}

/////////////////////////////////////////////////////////////////////////////
// CMediaListCtrl message handlers

int CMediaListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CListCtrl::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	ShellIcons.AttachTo( this, 16 );
	InsertColumn( 0, _T("Name"), LVCFMT_LEFT, 100, -1 );
	InsertColumn( 1, _T("Path"), LVCFMT_LEFT, 0, 0 );

	SetExtendedStyle( GetExtendedStyle() | LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT );
	
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

void CMediaListCtrl::OnCustomDraw(NMHDR* pNotify, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pDraw = (NMLVCUSTOMDRAW*)pNotify;
	int nItem = (int)pDraw->nmcd.dwItemSpec;

	if ( pDraw->nmcd.dwDrawStage == CDDS_PREPAINT )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( pDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT )
	{
		if ( IsCurrent( nItem ) )
		{
			pDraw->clrText = CoolInterface.m_crMediaPanelActiveText;
			pDraw->clrTextBk = CoolInterface.m_crMediaPanelActive;
		}
		else if ( IsPlayed( nItem ) )
		{
			pDraw->clrText = CoolInterface.m_crMediaPanelActiveText;
			pDraw->clrTextBk = CoolInterface.m_crMediaStatus;
		}
		else
		{
			pDraw->clrText = CoolInterface.m_crMediaPanelText;
			pDraw->clrTextBk = CoolInterface.m_crMediaPanel;
		}

		if ( m_bCreateDragImage ) pDraw->clrTextBk = DRAG_COLOR_KEY;

		*pResult = CDRF_DODEFAULT;
	}
	else
	{
		*pResult = 0;
	}
}

void CMediaListCtrl::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* pResult) 
{
	OnMediaSelect();
	*pResult = 0;
}

void CMediaListCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point) 
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
		DWORD nFile = ( nHit >= 0 ) ? (DWORD)GetItemData( nHit ) : 0;

		if ( nFile > 0 && ! Library.LookupFile( nFile ) ) nFile = 0;

		if ( nFile > 0 )
		{
			m_wndTip.Show( nFile );
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

	CArray< CString > pPaths;
	CArray< UINT > pStates;

	for (;;)
	{
		int nItem = GetNextItem( -1, LVNI_SELECTED );
		if ( nItem < 0 ) break;

		pPaths.Add( GetItemText( nItem, 1 ) );
		pStates.Add( GetItemState( nItem, 0xFFFFFFFF ) );

		DeleteItem( nItem );

		if ( m_nDragDrop > nItem ) m_nDragDrop --;
	}

	for ( int nItem = 0 ; nItem < pPaths.GetSize() ; nItem++ )
	{
		int nIndex = Add( pPaths.GetAt( nItem ), m_nDragDrop++ );
		SetItemState( nIndex, pStates.GetAt( nItem ), 0xFFFFFFFF );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMediaListCtrl command handlers

BOOL CMediaListCtrl::AreSelectedFilesInLibrary()
{
	if ( GetSelectedCount() )
	{
		// If at least one selected file is in the library then enable
		for ( int nItem = -1 ; ( nItem = GetNextItem( nItem, LVIS_SELECTED ) ) >= 0 ; )
		{
			CString sPath = GetPath( nItem );

			if ( LibraryMaps.LookupFileByPath( sPath ) )
				return TRUE;
		}
	}
	return FALSE;
}

void CMediaListCtrl::ShowFilePropertiesDlg( int nPage )
{
	CFilePropertiesSheet dlg;

	for ( int nItem = -1 ; ( nItem = GetNextItem( nItem, LVIS_SELECTED ) ) >= 0 ; )
	{
		CString sPath = GetPath( nItem );

		CQuickLock oLock( Library.m_pSection );
		if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( sPath ) )
			dlg.Add( pFile );
	}

	dlg.DoModal( nPage );
}

void CMediaListCtrl::OnUpdateMediaRate(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( AreSelectedFilesInLibrary() );
}

void CMediaListCtrl::OnMediaRate() 
{
	ShowFilePropertiesDlg( 2 );
}

void CMediaListCtrl::OnUpdateMediaProperties(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( AreSelectedFilesInLibrary() );
}

void CMediaListCtrl::OnMediaProperties() 
{
	ShowFilePropertiesDlg();
}

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
	
	CFileDialog dlg( TRUE, NULL, NULL,
		OFN_HIDEREADONLY|OFN_ALLOWMULTISELECT|OFN_ENABLESIZING,
		SchemaCache.GetFilter( CSchema::uriVideoAll ) +
		SchemaCache.GetFilter( CSchema::uriMusicAll ) +
		SchemaCache.GetFilter( CSchema::uriAllFiles ) +
		_T("|"), this );

	const int nLimit = 81920;

	auto_array< TCHAR > szFiles( new TCHAR[ nLimit ] );
	ZeroMemory( szFiles.get(), nLimit * sizeof( TCHAR ) );
	dlg.m_ofn.lpstrFile	= szFiles.get();
	dlg.m_ofn.nMaxFile	= nLimit;

	if ( dlg.DoModal() != IDOK ) return;
	
	CString strFolder( szFiles.get() );
	LPCTSTR pszFile = szFiles.get() + strFolder.GetLength() + 1;

	if ( *pszFile )
	{
		for ( strFolder += '\\' ; *pszFile ; )
		{
			Enqueue( strFolder + pszFile );
			pszFile += _tcslen( pszFile ) + 1;
		}
	}
	else
	{
		Enqueue( strFolder );
	}
}

void CMediaListCtrl::OnMediaAddFolder() 
{
	if ( ! AfxGetMainWnd()->IsWindowEnabled() ) return;

	CString strPath( BrowseForFolder( LoadString( ID_MEDIA_ADD_FOLDER ) ) );
	if ( strPath.IsEmpty() )
		return;

	RecursiveEnqueue( strPath );
}

void CMediaListCtrl::OnUpdateMediaRemove(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetSelectedCount() );
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
	CFileDialog dlg( TRUE, NULL, NULL, OFN_HIDEREADONLY|OFN_ENABLESIZING,
		SchemaCache.GetFilter( CSchema::uriVideoAll ) +
		SchemaCache.GetFilter( CSchema::uriMusicAll ) +
		SchemaCache.GetFilter( CSchema::uriAllFiles ) +
		_T("|"), this );

	if ( dlg.DoModal() != IDOK ) return;

	Play( dlg.GetPathName() );
}

void CMediaListCtrl::OnUpdateMediaSave(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetItemCount() > 0 );
}

void CMediaListCtrl::OnMediaSave() 
{
	CFileDialog dlg( FALSE, _T("m3u"), NULL, OFN_HIDEREADONLY|OFN_ENABLESIZING,
		_T("Media Playlists|*.m3u|") +
		SchemaCache.GetFilter( CSchema::uriAllFiles ) +
		_T("|"), this );

	if ( dlg.DoModal() != IDOK ) return;

	SaveTextList( dlg.GetPathName() );
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
	PlayNext();
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

void CMediaListCtrl::OnUpdateMediaCollection(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( GetItemCount() > 0 && GetItemCount() <= 200 );
}

void CMediaListCtrl::OnMediaCollection()
{
	// The album title name is a collection folder name
	// We leave it empty to have the collection mounted under collections folder

	CAutoPtr< CAlbumFolder > pCollection( new CAlbumFolder( NULL, NULL, _T(""), TRUE ) );
	if ( pCollection )
	{
		for ( int nItem = GetItemCount() - 1 ; nItem >= 0 ; nItem-- )
		{
			CString strPath = GetPath( nItem );

			CQuickLock oLock( Library.m_pSection );
			if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( strPath, FALSE, TRUE ) )
			{
				pCollection->AddFile( pFile );
			}
		}

		CCollectionExportDlg dlg( pCollection );
		dlg.DoModal();
	}
}

void CMediaListCtrl::OnSkinChange()
{
	SetBkColor( CoolInterface.m_crMediaPanel );
}
