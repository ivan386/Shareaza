//
// CtrlSharedFolder.cpp
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
#include "LibraryFolders.h"
#include "SharedFolder.h"
#include "ShellIcons.h"
#include "CtrlSharedFolder.h"
#include "CoolInterface.h"
#include "Skin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CLibraryFolderCtrl, CTreeCtrl)
	//{{AFX_MSG_MAP(CLibraryFolderCtrl)
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_CREATE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDEDW, OnItemExpanded)
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDEDA, OnItemExpanded)
	ON_NOTIFY_REFLECT(TVN_SELCHANGEDW, OnSelChanged)
	ON_NOTIFY_REFLECT(TVN_SELCHANGEDA, OnSelChanged)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	ON_WM_RBUTTONDOWN()
	ON_WM_NCPAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLibraryFolderCtrl construction

CLibraryFolderCtrl::CLibraryFolderCtrl()
{
	m_bFirstClick	= TRUE;
	m_bMultiSelect	= TRUE;
	m_bSaveExpand	= FALSE;
}

CLibraryFolderCtrl::~CLibraryFolderCtrl()
{
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryFolderCtrl operations

BOOL CLibraryFolderCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	dwStyle |= WS_CHILD|/*TVS_PRIVATEIMAGELISTS|*/TVS_HASLINES|TVS_LINESATROOT;
	dwStyle |= TVS_HASBUTTONS|TVS_SHOWSELALWAYS;
	return CTreeCtrl::Create( dwStyle, rect, pParentWnd, nID );
}

void CLibraryFolderCtrl::SetMultiSelect(BOOL bMultiSelect)
{
	m_bMultiSelect = bMultiSelect;
}

void CLibraryFolderCtrl::SetSaveExpand(BOOL bSaveExpand)
{
	m_bSaveExpand = bSaveExpand;
}

void CLibraryFolderCtrl::Update(DWORD nUpdateCookie)
{
	CList< CLibraryFolder* > pAlready;

	for ( HTREEITEM hItem = GetChildItem( m_hRoot ) ; hItem ; )
	{
		HTREEITEM hNext = GetNextSiblingItem( hItem );

		CLibraryFolder* pFolder = (CLibraryFolder*)GetItemData( hItem );

		if ( LibraryFolders.CheckFolder( pFolder ) )
		{
			Update( pFolder, hItem, NULL, nUpdateCookie, FALSE );
			pAlready.AddTail( pFolder );
		}
		else
		{
			DeleteItem( hItem );
		}

		hItem = hNext;
	}

	for ( POSITION pos = LibraryFolders.GetFolderIterator() ; pos ; )
	{
		CLibraryFolder* pFolder = LibraryFolders.GetNextFolder( pos );

		if ( pAlready.Find( pFolder ) == NULL )
		{
			Update( pFolder, NULL, m_hRoot, nUpdateCookie, FALSE );
		}
	}
}

void CLibraryFolderCtrl::Update(CLibraryFolder* pFolder, HTREEITEM hFolder, HTREEITEM hParent, DWORD nUpdateCookie, BOOL bRecurse)
{
	if ( ! hFolder )
	{
		DWORD dwStyle = INDEXTOOVERLAYMASK( pFolder->IsShared() ? 0 : SHI_O_LOCKED );

		if ( pFolder->m_sPath.CompareNoCase( Settings.Downloads.CompletePath ) == 0 ) dwStyle |= TVIS_BOLD;

		if ( m_bMultiSelect && GetFirstSelectedItem() == NULL ) dwStyle |= TVIS_SELECTED;
		if ( pFolder->m_bExpanded ) dwStyle |= TVIS_EXPANDED;

		CString strName = pFolder->m_sName;

		if ( pFolder->m_pParent == NULL )
		{
			CString strDrive;
			if ( pFolder->m_sPath.Find( _T(":\\") ) == 1 || pFolder->m_sPath.GetLength() == 2 )
				strDrive.Format( _T(" (%c:)"), pFolder->m_sPath.GetAt( 0 ) );
			else
				strDrive = _T(" (Net)");

			strName += strDrive;
			dwStyle |= TVIS_EXPANDED;
		}

		hFolder = InsertItem( TVIF_PARAM|TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_STATE,
			strName, SHI_FOLDER_CLOSED, SHI_FOLDER_CLOSED, dwStyle,
			TVIS_EXPANDED|TVIS_SELECTED|TVIS_OVERLAYMASK, (LPARAM)pFolder, hParent, TVI_SORT );
	}
	else
	{
		DWORD dwMask = TVIS_OVERLAYMASK | TVIS_BOLD;
		DWORD dwStyle = INDEXTOOVERLAYMASK( pFolder->IsShared() ? 0 : SHI_O_LOCKED );

		if ( pFolder->m_sPath.CompareNoCase( Settings.Downloads.CompletePath ) == 0 ) dwStyle |= TVIS_BOLD;

		DWORD dwExisting = GetItemState( hFolder, dwMask ) & dwMask;

		if ( dwExisting != dwStyle ) SetItemState( hFolder, dwStyle, dwMask );
	}

	if ( nUpdateCookie )
	{
		if ( bRecurse || ( GetItemState( hFolder, TVIS_SELECTED ) & TVIS_SELECTED ) )
		{
			pFolder->m_nSelectCookie = nUpdateCookie;
			bRecurse |= ( GetItemState( hFolder, TVIS_EXPANDED ) & TVIS_EXPANDED ) == 0;
		}
	}

	CList< CLibraryFolder* > pAlready;

	for ( HTREEITEM hItem = GetChildItem( hFolder ) ; hItem ; )
	{
		HTREEITEM hNext = GetNextSiblingItem( hItem );

		CLibraryFolder* pChild = (CLibraryFolder*)GetItemData( hItem );

		if ( pFolder->CheckFolder( pChild ) )
		{
			Update( pChild, hItem, NULL, nUpdateCookie, bRecurse );
			pAlready.AddTail( pChild );
		}
		else
		{
			DeleteItem( hItem );
		}

		hItem = hNext;
	}

	for ( POSITION pos = pFolder->GetFolderIterator() ; pos ; )
	{
		CLibraryFolder* pChild = pFolder->GetNextFolder( pos );

		if ( pAlready.Find( pChild ) == NULL )
		{
			Update( pChild, NULL, hFolder, nUpdateCookie, bRecurse );
		}
	}

	int nOldImage1, nOldImage2;
	GetItemImage( hFolder, nOldImage1, nOldImage2 );

	int nImage = ItemHasChildren( hFolder ) && ( GetItemState( hFolder, TVIS_EXPANDED ) & TVIS_EXPANDED );
	nImage = nImage ? SHI_FOLDER_OPEN : SHI_FOLDER_CLOSED;
	if ( nOldImage1 != nImage ) SetItemImage( hFolder, nImage, nImage );
}

void CLibraryFolderCtrl::SetSelectedCookie(DWORD nUpdateCookie, HTREEITEM hParent, BOOL bSelect)
{
	if ( ! hParent ) hParent = m_hRoot;

	for ( HTREEITEM hItem = GetChildItem( hParent ) ; hItem ; )
	{
		BOOL bRecurse = bSelect;

		if ( bSelect || ( GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED ) )
		{
			CLibraryFolder* pFolder = (CLibraryFolder*)GetItemData( hItem );
			pFolder->m_nSelectCookie = nUpdateCookie;
			if ( ( GetItemState( hItem, TVIS_EXPANDED ) & TVIS_EXPANDED ) == 0 ) bRecurse = TRUE;
		}

		SetSelectedCookie( nUpdateCookie, hItem, bRecurse );

		hItem = GetNextSiblingItem( hItem );
	}
}

POSITION CLibraryFolderCtrl::GetSelectedFolderIterator() const
{
	return (POSITION)GetFirstSelectedItem();
}

CLibraryFolder* CLibraryFolderCtrl::GetNextSelectedFolder(POSITION& pos) const
{
	CLibraryFolder* pFolder = NULL;

	do
	{
		if ( pos == NULL ) return NULL;

		HTREEITEM hItem = (HTREEITEM)pos;
		pos = (POSITION)GetNextSelectedItem( hItem );

		if ( hItem == m_hRoot ) continue;

		CList< HTREEITEM > pTree;

		while ( hItem != m_hRoot )
		{
			pTree.AddHead( hItem );
			hItem = GetParentItem( hItem );
		}

		CLibraryFolder* pLastFolder = NULL;

		for ( POSITION posTree = pTree.GetHeadPosition() ; posTree ; pLastFolder = pFolder )
		{
			hItem = pTree.GetNext( posTree );
			pFolder = (CLibraryFolder*)GetItemData( hItem );

			if ( pLastFolder )
			{
				if ( pLastFolder->CheckFolder( pFolder ) ) continue;
			}
			else
			{
				if ( LibraryFolders.CheckFolder( pFolder ) ) continue;
			}

			pFolder = NULL;
			break;
		}
	}
	while ( pFolder == NULL );

	return pFolder;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryFolderCtrl message handlers

int CLibraryFolderCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CTreeCtrl::OnCreate( lpCreateStruct ) == -1 ) return -1;

	ShellIcons.AttachTo( this );

	m_hRoot				= GetRootItem();
	m_hFirstSelected	= NULL;

	return 0;
}

void CLibraryFolderCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	UINT nItemFlags = 0;
	HTREEITEM hItem = HitTest( point, &nItemFlags );

	if ( nItemFlags == TVHT_ONITEMBUTTON )
	{
		CTreeCtrl::OnLButtonDown( nFlags, point );
		return;
	}

	if ( ( nFlags & MK_CONTROL ) && m_bMultiSelect )
	{
		if ( hItem )
		{
			UINT nNewState = GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED ?
				0 : TVIS_SELECTED;

			SetItemState( hItem, nNewState,  TVIS_SELECTED );
			m_hFirstSelected = NULL;
			NotifySelectionChanged();
		}
	}
	else if ( ( nFlags & MK_SHIFT ) && m_bMultiSelect )
	{
		hItem = HitTest( point );

		if ( ! m_hFirstSelected ) m_hFirstSelected = GetFirstSelectedItem();

		if ( hItem ) SetItemState( hItem, TVIS_SELECTED, TVIS_SELECTED );

		if ( m_hFirstSelected ) SelectItems( m_hFirstSelected, hItem );

		NotifySelectionChanged();
	}
	else
	{
		BOOL bChanged = FALSE;

		BOOL bSelected = hItem && ( GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED );

		if ( ! bSelected || ( nFlags & MK_RBUTTON ) == 0 )
		{
			if ( m_bFirstClick && hItem != GetRootItem() )
			{
				Select( hItem, TVGN_CARET );
			}

			bChanged = ClearSelection( hItem );
			m_hFirstSelected = NULL;
		}

		if ( hItem )
		{
			if ( ! bSelected )
			{
				SetItemState( hItem, TVIS_SELECTED, TVIS_SELECTED );
				bChanged = TRUE;
			}
		}

		if ( bChanged ) NotifySelectionChanged();
	}

	m_bFirstClick = FALSE;

	SetFocus();
}

void CLibraryFolderCtrl::OnLButtonDblClk(UINT /*nFlags*/, CPoint point)
{
	UINT nItemFlags = 0;
	/*HTREEITEM hItem =*/ HitTest( point, &nItemFlags );

	if ( ( nItemFlags & (TVHT_ONITEMINDENT|TVHT_ONITEMRIGHT|TVHT_NOWHERE) ) && m_bMultiSelect )
	{
		SelectAll();
	}
	else
	{
		NMHDR pNM = { GetSafeHwnd(), (UINT_PTR)GetDlgCtrlID(), NM_DBLCLK };
		GetParent()->SendMessage( WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&pNM );
	}
}

void CLibraryFolderCtrl::OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/)
{
}

void CLibraryFolderCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	OnLButtonDown( nFlags, point );
	OnLButtonUp( nFlags, point );
	CTreeCtrl::OnRButtonDown( nFlags, point );
}

void CLibraryFolderCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if ( ( nChar == VK_UP || nChar == VK_DOWN ) && ( GetKeyState( VK_SHIFT ) & 0x8000 ) && m_bMultiSelect )
	{
		if ( ! m_hFirstSelected )
		{
			m_hFirstSelected = GetFirstSelectedItem();
			ClearSelection( m_hFirstSelected );
		}

		HTREEITEM hItemPrevSel = GetSelectedItem();
		HTREEITEM hItemNext;

		if ( nChar == VK_UP )
			hItemNext = GetPrevVisibleItem( hItemPrevSel );
		else
			hItemNext = GetNextVisibleItem( hItemPrevSel );

		if ( hItemNext )
		{
			BOOL bReselect = ! ( GetItemState( hItemNext, TVIS_SELECTED ) & TVIS_SELECTED );

			SelectItem( hItemNext );

			if ( bReselect ) SetItemState( hItemPrevSel, TVIS_SELECTED, TVIS_SELECTED );
		}

		NotifySelectionChanged();

		return;
	}
	else if ( nChar >= VK_SPACE )
	{
		m_hFirstSelected = NULL;
		ClearSelection();
	}
	else if ( nChar == 'A' && ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) && m_bMultiSelect )
	{
		BOOL bChanged = FALSE;

		for ( HTREEITEM hItem = GetRootItem() ; hItem != NULL ; hItem = GetNextItem( hItem, TVGN_NEXTVISIBLE ) )
		{
			if ( ( GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED ) == 0 )
			{
				SetItemState( hItem, TVIS_SELECTED, TVIS_SELECTED );
				bChanged = TRUE;
			}
		}

		if ( bChanged ) NotifySelectionChanged();
		return;
	}

	CTreeCtrl::OnKeyDown( nChar, nRepCnt, nFlags );
}

BOOL CLibraryFolderCtrl::ClearSelection(HTREEITEM hExcept, HTREEITEM hItem, BOOL bSelect)
{
	BOOL bChanged = FALSE;

	if ( hItem == NULL ) hItem = GetRootItem();

	for ( ; hItem != NULL ; hItem = GetNextItem( hItem, TVGN_NEXT ) )
	{
		BOOL bIsSelected = ( GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED ) ? TRUE : FALSE;

		if ( hItem != hExcept && ( bIsSelected != bSelect ) )
		{
			SetItemState( hItem, bSelect ? TVIS_SELECTED : 0, TVIS_SELECTED );
			bChanged = TRUE;
		}

		HTREEITEM hChild = GetChildItem( hItem );
		if ( hChild != NULL ) bChanged |= ClearSelection( hExcept, hChild, bSelect );
	}

	return bChanged;
}

BOOL CLibraryFolderCtrl::SelectAll(HTREEITEM hExcept)
{
	if ( ClearSelection( hExcept, NULL, TRUE ) )
	{
		NotifySelectionChanged();
		return TRUE;
	}
	return FALSE;
}

BOOL CLibraryFolderCtrl::SelectItems(HTREEITEM hItemFrom, HTREEITEM hItemTo)
{
	HTREEITEM hItem = GetRootItem();

	while ( hItem && hItem != hItemFrom && hItem != hItemTo )
	{
		hItem = GetNextVisibleItem( hItem );
		SetItemState( hItem, 0, TVIS_SELECTED );
	}

	if ( ! hItem ) return FALSE;

	if ( hItem == hItemTo )
	{
		hItemTo		= hItemFrom;
		hItemFrom	= hItem;
	}

	BOOL bSelect = TRUE;

	while ( hItem )
	{
		SetItemState( hItem, bSelect ? TVIS_SELECTED : 0, TVIS_SELECTED );
		if ( hItem == hItemTo ) bSelect = FALSE;
		hItem = GetNextVisibleItem( hItem );
	}

	return TRUE;
}

BOOL CLibraryFolderCtrl::SelectFolder(CLibraryFolder* pFolder, HTREEITEM hItem)
{
	if ( hItem == NULL ) hItem = GetRootItem();

	for ( ; hItem != NULL ; hItem = GetNextItem( hItem, TVGN_NEXT ) )
	{
		if ( pFolder == (CLibraryFolder*)GetItemData( hItem ) )
		{
			SetItemState( hItem, TVIS_SELECTED, TVIS_SELECTED );
			return TRUE;
		}

		HTREEITEM hChild = GetChildItem( hItem );
		if ( hChild != NULL && SelectFolder( pFolder, hChild ) ) return TRUE;
	}

	return FALSE;
}

HTREEITEM CLibraryFolderCtrl::GetFirstSelectedItem() const
{
	for ( HTREEITEM hItem = GetRootItem() ; hItem != NULL ; hItem = GetNextItem( hItem, TVGN_NEXTVISIBLE ) )
	{
		if ( GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED ) return hItem;
	}

	return NULL;
}

HTREEITEM CLibraryFolderCtrl::GetNextSelectedItem(HTREEITEM hItem) const
{
	for ( hItem = GetNextItem( hItem, TVGN_NEXTVISIBLE ) ; hItem != NULL ; hItem = GetNextItem( hItem, TVGN_NEXTVISIBLE ) )
	{
		if ( GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED ) return hItem;
	}

	return NULL;
}

void CLibraryFolderCtrl::OnItemExpanded(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	HTREEITEM hItem	= pNMTreeView->itemNew.hItem;
	int nImage		= ItemHasChildren( hItem ) && pNMTreeView->itemNew.state & TVIS_EXPANDED;
	nImage = nImage ? SHI_FOLDER_OPEN : SHI_FOLDER_CLOSED;
	SetItemImage( hItem, nImage, nImage );

	CSingleLock oLock( &Library.m_pSection );
	if ( m_bSaveExpand && oLock.Lock( 50 ) )
	{
		CLibraryFolder* pFolder = (CLibraryFolder*)GetItemData( hItem );

		if ( LibraryFolders.CheckFolder( pFolder, TRUE ) )
		{
			pFolder->m_bExpanded = ( nImage == 1 );
		}

	}
}

void CLibraryFolderCtrl::OnSelChanged(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
//	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	NotifySelectionChanged();
	*pResult = 0;
}

void CLibraryFolderCtrl::NotifySelectionChanged()
{
	GetParent()->PostMessage( WM_TIMER, 2 );
}

void CLibraryFolderCtrl::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMTVCUSTOMDRAW* pDraw = (NMTVCUSTOMDRAW*)pNMHDR;

	if ( pDraw->nmcd.dwDrawStage == CDDS_PREPAINT )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( pDraw->nmcd.dwDrawStage == (CDDS_ITEM|CDDS_PREPAINT) )
	{
		BOOL bSelected		= GetItemState( (HTREEITEM)pDraw->nmcd.dwItemSpec, TVIS_SELECTED ) & TVIS_SELECTED ? TRUE : FALSE;
		pDraw->clrText		= ( bSelected ? CoolInterface.m_crHiText : CoolInterface.m_crText );
		pDraw->clrTextBk	= ( bSelected ? CoolInterface.m_crHighlight : CoolInterface.m_crWindow );
		*pResult = CDRF_DODEFAULT;
	}
}

void CLibraryFolderCtrl::OnNcPaint()
{
	CWnd::OnNcPaint();

	if ( GetStyle() & WS_BORDER )
	{
		CWindowDC dc( this );
		CRect rc;

		COLORREF crBorder = CoolInterface.m_crSysActiveCaption ;

		GetWindowRect( &rc );
		rc.OffsetRect( -rc.left, -rc.top );
		dc.Draw3dRect( &rc, crBorder, crBorder );
	}
}
