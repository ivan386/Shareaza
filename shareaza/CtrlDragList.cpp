//
// CtrlDragList.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "CtrlDragList.h"
#include "LiveList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CDragListCtrl, CListCtrl)

BEGIN_MESSAGE_MAP(CDragListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CDragListCtrl)
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, OnBeginDrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDragListCtrl construction

CDragListCtrl::CDragListCtrl()
{
	m_pDragImage		= NULL;
	m_bCreateDragImage	= FALSE;
}

CDragListCtrl::~CDragListCtrl()
{
}

/////////////////////////////////////////////////////////////////////////////
// CDragListCtrl message handlers

void CDragListCtrl::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	*pResult = 0;

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

void CDragListCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	if ( m_pDragImage != NULL )
	{
		int nHit = HitTest( point );

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

	CListCtrl::OnMouseMove( nFlags, point );
}

void CDragListCtrl::OnLButtonUp(UINT nFlags, CPoint point)
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
	//else
	//	m_nDragDrop = GetItemCount();

	OnDragDrop( m_nDragDrop );
}

void CDragListCtrl::OnDragDrop(int nDrop)
{
	NM_LISTVIEW pNotify;
	pNotify.hdr.hwndFrom	= GetSafeHwnd();
	pNotify.hdr.idFrom		= GetDlgCtrlID();
	pNotify.hdr.code		= LVN_DRAGDROP;
	pNotify.iItem			= nDrop;
	GetOwner()->SendMessage( WM_NOTIFY, pNotify.hdr.idFrom, (LPARAM)&pNotify );
}
