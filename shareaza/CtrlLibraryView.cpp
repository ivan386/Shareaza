//
// CtrlLibraryView.cpp
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
#include "Settings.h"
#include "Library.h"
#include "SharedFile.h"
#include "SharedFolder.h"
#include "AlbumFolder.h"
#include "Schema.h"
#include "ShellIcons.h"
#include "Skin.h"
#include "CtrlLibraryFrame.h"
#include "CtrlLibraryView.h"
#include "Schema.h"
#include "SchemaCache.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CLibraryView, CWnd)

BEGIN_MESSAGE_MAP(CLibraryView, CWnd)
	ON_WM_MOUSEWHEEL()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLibraryView construction

CLibraryView::CLibraryView()
	: m_nCommandID	( ID_LIBRARY_VIEW )
	, m_pszToolBar	( NULL )
	, m_bAvailable	( FALSE )
	, m_bGhostFolder( FALSE )
	, m_pSelection	( new CLibraryList() )
{
}

CLibraryView::~CLibraryView()
{
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryView operations

BOOL CLibraryView::Create(CWnd* pParentWnd)
{
	CRect rect( 0, 0, 0, 0 );
	SelClear( FALSE );
	return CWnd::CreateEx( WS_EX_CONTROLPARENT, NULL, NULL, WS_CHILD|WS_TABSTOP, rect, pParentWnd,
		IDC_LIBRARY_VIEW, NULL );
}

BOOL CLibraryView::CheckAvailable(CLibraryTreeItem* /*pSel*/)
{
	return ( m_bAvailable = FALSE );
}

void CLibraryView::GetHeaderContent(int& nImage, CString& str)
{
	CString strFormat;
	int nCount = 0;

	for ( CLibraryTreeItem* pItem = GetFolderSelection() ; pItem ;
		  pItem = pItem->m_pSelNext ) nCount++;

	if ( nCount == 1 )
	{
        CLibraryTreeItem* pItem = GetFolderSelection();
		for ( ; pItem->parent() ;
			pItem = pItem->parent() )
		{
			if ( str.GetLength() ) str = '\\' + str;
			str = pItem->m_sText + str;
		}

		LoadString( strFormat, IDS_LIBHEAD_EXPLORE_FOLDER );
		if ( Settings.General.LanguageRTL ) 
			str =  _T("\x202A") + str + _T(" \x200E") + strFormat;
		else
			str = strFormat + str;

		nImage	= SHI_FOLDER_OPEN;
		pItem	= GetFolderSelection();

		if ( pItem->m_pVirtual && pItem->m_pVirtual->m_pSchema )
			nImage = pItem->m_pVirtual->m_pSchema->m_nIcon16;

	}
	else if ( nCount > 1 )
	{
		LoadString( strFormat, IDS_LIBHEAD_EXPLORE_MANY );
		str.Format( strFormat, nCount );
		nImage = SHI_FOLDER_OPEN;
	}
	else if ( CSchemaPtr pSchema = SchemaCache.Get( CSchema::uriLibrary ) )
	{
		nImage = pSchema->m_nIcon16;
		LoadString( str, IDS_LIBHEAD_EXPLORE_FOLDER );
		LPCTSTR psz = _tcschr( pSchema->m_sTitle, ':' );
		if ( Settings.General.LanguageRTL )
		{
			CString strCaption( psz ? psz + 1 : pSchema->m_sTitle );
			str = _T("\x202A") + strCaption + _T(" \x200E") + str;
		}
		else
			str += psz ? psz + 1 : pSchema->m_sTitle;
	}
	else
	{
		nImage = SHI_COMPUTER;
		LoadString( str, IDS_LIBHEAD_HOME );
	}
}

void CLibraryView::Update()
{
}

BOOL CLibraryView::Select(DWORD /*nObject*/)
{
	return FALSE;
}

void CLibraryView::CacheSelection()
{
}

CLibraryListItem CLibraryView::DropHitTest(const CPoint& /*point*/) const
{
	return CLibraryListItem();
}

CLibraryListItem CLibraryView::GetFolder() const
{
	CLibraryListItem oHit;

	CLibraryTreeItem* pRoot = GetFolderSelection();
	if ( pRoot )
	{
		if ( pRoot->m_pPhysical )
		{
			oHit = pRoot->m_pPhysical;
		}
		else if ( pRoot->m_pVirtual )
		{
			oHit = pRoot->m_pVirtual;
		}
	}
	else
	{
		oHit = Library.GetAlbumRoot();
	}
	return oHit;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryView helper operations

void CLibraryView::PostUpdate()
{
	GetOwner()->PostMessage( WM_COMMAND, ID_LIBRARY_REFRESH );
	InvalidateRect( NULL );
}

CLibraryFrame* CLibraryView::GetFrame() const
{
	CLibraryFrame* pFrame = (CLibraryFrame*)GetOwner();
	ASSERT_KINDOF(CLibraryFrame, pFrame );
	return pFrame;
}

CLibraryTipCtrl* CLibraryView::GetToolTip() const
{
	return GetFrame()->GetToolTip();
}

DWORD CLibraryView::GetFolderCookie() const
{
	return GetFrame()->GetFolderCookie();
}

CLibraryTreeItem* CLibraryView::GetFolderSelection() const
{
	return GetFrame()->GetFolderSelection();
}

CAlbumFolder* CLibraryView::GetSelectedAlbum(CLibraryTreeItem* pSel) const
{
	if ( pSel == NULL && m_hWnd != NULL ) pSel = GetFolderSelection();
	if ( pSel == NULL ) return NULL;
	if ( pSel->m_pSelNext != NULL ) return NULL;
	return pSel->m_pVirtual;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryView selection operations

BOOL CLibraryView::SelAdd(CLibraryListItem oObject, BOOL bNotify)
{
	if ( m_pSelection->Find( oObject ) ) return FALSE;
	m_pSelection->AddTail( oObject );

	if ( bNotify )
	{
		CLibraryFrame* pFrame = (CLibraryFrame*)GetOwner();
		pFrame->OnViewSelection();
	}

	return TRUE;
}

BOOL CLibraryView::SelRemove(CLibraryListItem oObject, BOOL bNotify)
{
	POSITION pos = m_pSelection->Find( oObject );
	if ( pos == NULL ) return FALSE;
	m_pSelection->RemoveAt( pos );

	if ( bNotify )
	{
		CLibraryFrame* pFrame = (CLibraryFrame*)GetOwner();
		pFrame->OnViewSelection();
	}

	return TRUE;
}

BOOL CLibraryView::SelClear(BOOL bNotify)
{
	if ( m_pSelection->IsEmpty() ) return FALSE;
	m_pSelection->RemoveAll();

	if ( bNotify )
	{
		CLibraryFrame* pFrame = (CLibraryFrame*)GetOwner();
		pFrame->OnViewSelection();
	}

	return TRUE;
}

INT_PTR CLibraryView::GetSelectedCount() const
{
	return m_pSelection->GetCount();
}

POSITION CLibraryView::StartSelectedFileLoop() const
{
	return m_pSelection->GetHeadPosition();
}

CLibraryFile* CLibraryView::GetNextSelectedFile(POSITION& posSel, BOOL bSharedOnly, BOOL bAvailableOnly) const
{
	while ( posSel )
	{
		DWORD nIndex = m_pSelection->GetNext( posSel );
		if ( CLibraryFile* pFile = Library.LookupFile( nIndex, bSharedOnly, bAvailableOnly ) )
			return pFile;
	}

	return NULL;
}

CLibraryFile* CLibraryView::GetSelectedFile()
{
	if ( m_pSelection->GetCount() == 0 ) return NULL;
	return Library.LookupFile( m_pSelection->GetHead() );
}

int CLibraryView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	ENABLE_DROP()

	OnSkinChange();

	return 0;
}

void CLibraryView::OnDestroy() 
{
	DISABLE_DROP()

	CWnd::OnDestroy();
}

void CLibraryView::OnLButtonDown(UINT nFlags, CPoint point)
{
	GetToolTip()->Hide();

	CWnd::OnLButtonDown( nFlags, point );
}

void CLibraryView::OnRButtonDown(UINT nFlags, CPoint point)
{
	GetToolTip()->Hide();

	CWnd::OnRButtonDown( nFlags, point );
}

void CLibraryView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	GetToolTip()->Hide();

	BOOL bShift = ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0x8000;
	BOOL bControl = ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) == 0x8000;

	switch ( nChar )
	{
	case 'A':
	case 'a':
		if ( bControl && ! bShift ) SelectAll();
		return;
	}

	CWnd::OnKeyDown( nChar, nRepCnt, nFlags );
}

void CLibraryView::StartDragging(const CPoint& ptMouse)
{
	CQuickLock oLock( Library.m_pSection );

	CPoint ptMiddle( 0, 0 );
	HBITMAP pImage = CreateDragImage( ptMouse, ptMiddle );
	if ( ! pImage )
		return;

	// Get GUID of parent folder
	Hashes::Guid oGUID;
	CLibraryListItem oHit = GetFolder();
	if ( oHit.Type == CLibraryListItem::AlbumFolder )
	{
		oGUID = ((CAlbumFolder*)oHit)->m_oGUID;
	}
	CShareazaDataSource::DoDragDrop( m_pSelection, pImage, oGUID, ptMiddle );
}

HBITMAP CLibraryView::CreateDragImage(const CPoint& /*ptMouse*/, CPoint& /*ptMiddle*/)
{
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryView drag drop

IMPLEMENT_DROP(CLibraryView,CWnd)

BOOL CLibraryView::OnDrop(IDataObject* pDataObj, DWORD grfKeyState, POINT ptScreen, DWORD* pdwEffect, BOOL bDrop)
{
	CQuickLock oLock( Library.m_pSection );

	if ( ! pDataObj )
	{
		m_oDropItem.Type = CLibraryListItem::Empty;
		return TRUE;
	}

	CPoint pt( ptScreen );
	ScreenToClient( &pt );

	CLibraryListItem oHit = DropHitTest( pt );

	if ( bDrop )
	{
		m_oDropItem.Type = CLibraryListItem::Empty;
	}
	else if ( m_oDropItem != oHit )
	{
		m_oDropItem = oHit;
	}

	if ( oHit.Type == CLibraryListItem::Empty )
	{
		oHit = GetFolder();
	}

	switch ( oHit.Type )
	{
		case CLibraryListItem::LibraryFolder:
			return CShareazaDataSource::DropToFolder( pDataObj, grfKeyState,
				pdwEffect, bDrop, ((CLibraryFolder*)oHit)->m_sPath );

		case CLibraryListItem::AlbumFolder:
			return CShareazaDataSource::DropToAlbum( pDataObj, grfKeyState,
				pdwEffect, bDrop, ((CAlbumFolder*)oHit) );

		case CLibraryListItem::Empty:
		case CLibraryListItem::LibraryFile:
			break;
	}

	return FALSE;
}

BOOL CLibraryView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	// Scroll window under cursor
	if ( CWnd* pWnd = WindowFromPoint( pt ) )
	{
		if ( pWnd != this )
		{
			if ( pWnd == FindWindowEx(
				GetParent()->GetSafeHwnd(), NULL, NULL, _T("CPanelCtrl") ) ||
				pWnd == FindWindowEx(
				GetParent()->GetSafeHwnd(), NULL, NULL, _T("CLibraryTreeView") ) )
			{
				pWnd->PostMessage( WM_MOUSEWHEEL, MAKEWPARAM( nFlags, zDelta ), MAKELPARAM( pt.x, pt.y ) );
				return TRUE;
			}
		}
	}

	return CWnd::OnMouseWheel( nFlags, zDelta, pt );
}
