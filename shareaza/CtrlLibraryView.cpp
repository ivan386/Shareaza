//
// CtrlLibraryView.cpp
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
#include "AlbumFolder.h"
#include "Schema.h"
#include "ShellIcons.h"
#include "Skin.h"
#include "CtrlLibraryFrame.h"
#include "CtrlLibraryView.h"
#include "CtrlLibraryTree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CLibraryView, CWnd)

BEGIN_MESSAGE_MAP(CLibraryView, CWnd)
	//{{AFX_MSG_MAP(CLibraryView)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLibraryView construction

CLibraryView::CLibraryView()
{
	m_nCommandID	= ID_LIBRARY_VIEW;
	m_pszToolBar	= NULL;
	m_bAvailable	= FALSE;
}

CLibraryView::~CLibraryView()
{
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryView operations

BOOL CLibraryView::Create(CWnd* pParentWnd) 
{
	CRect rect;
	SelClear( FALSE );
	return CWnd::Create( NULL, NULL, WS_CHILD, rect, pParentWnd, IDC_LIBRARY_VIEW, NULL );
}

BOOL CLibraryView::CheckAvailable(CLibraryTreeItem* pSel)
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
		for ( ; pItem->m_pParent ;
			pItem = pItem->m_pParent )
		{
			if ( str.GetLength() ) str = '\\' + str;
			str = pItem->m_sText + str;
		}

		LoadString( strFormat, IDS_LIBHEAD_EXPLORE_FOLDER );
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
}

void CLibraryView::Update()
{
}

BOOL CLibraryView::Select(DWORD nObject)
{
	return FALSE;
}

void CLibraryView::CacheSelection()
{
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryView helper operations

void CLibraryView::PostUpdate()
{
	GetOwner()->PostMessage( WM_COMMAND, ID_LIBRARY_REFRESH );
}

CLibraryFrame* CLibraryView::GetFrame() const
{
	CLibraryFrame* pFrame = (CLibraryFrame*)GetOwner();
	ASSERT_KINDOF(CLibraryFrame, pFrame );
	return pFrame;
}

CLibraryTipCtrl* CLibraryView::GetToolTip() const
{
	return &GetFrame()->m_wndViewTip;
}

DWORD CLibraryView::GetFolderCookie() const
{
	return GetFrame()->m_nFolderCookie;
}

CLibraryTreeItem* CLibraryView::GetFolderSelection() const
{
	return GetFrame()->m_pFolderSelection;
}

CAlbumFolder* CLibraryView::GetSelectedAlbum(CLibraryTreeItem* pSel) const
{
	if ( pSel == NULL && m_hWnd != NULL ) pSel = GetFolderSelection();
	if ( pSel == NULL ) return NULL;
	if ( pSel->m_pSelNext != NULL ) return NULL;
	return pSel->m_pVirtual;
}

void CLibraryView::DragObjects(CImageList* pImage, const CPoint& ptMouse)
{
	CLibraryFrame* pFrame	= (CLibraryFrame*)GetOwner();
	CLibraryList* pList		= new CLibraryList( GetSelectedCount() );
	
	for ( POSITION pos = m_pSelection.GetHeadPosition() ; pos ; )
		pList->AddTail( m_pSelection.GetNext( pos ) );

	pFrame->DragObjects( pList, pImage, ptMouse );
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryView selection operations

BOOL CLibraryView::SelAdd(DWORD nObject, BOOL bNotify)
{
	if ( m_pSelection.Find( nObject ) ) return FALSE;
	m_pSelection.AddTail( nObject );

	if ( bNotify )
	{
		CLibraryFrame* pFrame = (CLibraryFrame*)GetOwner();
		pFrame->OnViewSelection();
	}

	return TRUE;
}

BOOL CLibraryView::SelRemove(DWORD nObject, BOOL bNotify)
{
	POSITION pos = m_pSelection.Find( nObject );
	if ( pos == NULL ) return FALSE;
	m_pSelection.RemoveAt( pos );

	if ( bNotify )
	{
		CLibraryFrame* pFrame = (CLibraryFrame*)GetOwner();
		pFrame->OnViewSelection();
	}

	return TRUE;
}

BOOL CLibraryView::SelClear(BOOL bNotify)
{
	if ( m_pSelection.GetCount() == 0 ) return FALSE;
	m_pSelection.RemoveAll();

	if ( bNotify )
	{
		CLibraryFrame* pFrame = (CLibraryFrame*)GetOwner();
		pFrame->OnViewSelection();
	}

	return TRUE;
}

int CLibraryView::GetSelectedCount() const
{
	return m_pSelection.GetCount();
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryView message handlers

