//
// CtrlBrowseFrame.cpp
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
#include "MatchObjects.h"
#include "QueryHit.h"
#include "Schema.h"
#include "SchemaCache.h"
#include "G2Packet.h"
#include "CtrlBrowseFrame.h"
#include "CtrlMatch.h"
#include "DlgHitColumns.h"
#include "Skin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CBrowseFrameCtrl, CWnd)

BEGIN_MESSAGE_MAP(CBrowseFrameCtrl, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_UPDATE_COMMAND_UI(ID_SEARCH_DETAILS, OnUpdateSearchDetails)
	ON_COMMAND(ID_SEARCH_DETAILS, OnSearchDetails)
	ON_NOTIFY(BTN_SELCHANGED, IDC_BROWSE_TREE, OnTreeSelection)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_TREE_PHYSICAL, OnUpdateLibraryTreePhysical)
	ON_COMMAND(ID_LIBRARY_TREE_PHYSICAL, OnLibraryTreePhysical)
	ON_UPDATE_COMMAND_UI(ID_LIBRARY_TREE_VIRTUAL, OnUpdateLibraryTreeVirtual)
	ON_COMMAND(ID_LIBRARY_TREE_VIRTUAL, OnLibraryTreeVirtual)
END_MESSAGE_MAP()

#define SIZE_INTERNAL	1982
#define SPLIT_SIZE		6
#define BAR_HEIGHT		28


/////////////////////////////////////////////////////////////////////////////
// CBrowseFrameCtrl construction

CBrowseFrameCtrl::CBrowseFrameCtrl()
{
	m_bTreeVisible	= FALSE;
	m_nTreeSize		= Settings.Search.BrowseTreeSize;
	m_bPanelEnable	= FALSE;
	m_bPanelVisible	= Settings.Search.DetailPanelVisible;
	m_nPanelSize	= Settings.Search.DetailPanelSize;
	m_pTree[0]		= NULL;
	m_pTree[1]		= NULL;
	m_nTree			= 1;
}

CBrowseFrameCtrl::~CBrowseFrameCtrl()
{
	if ( m_pTree[0] != NULL ) m_pTree[0]->Release();
	if ( m_pTree[1] != NULL ) m_pTree[1]->Release();
}

/////////////////////////////////////////////////////////////////////////////
// CBrowseFrameCtrl message handlers

BOOL CBrowseFrameCtrl::Create(CWnd* pParentWnd, CMatchCtrl* pMatch)
{
	CRect rect( 0, 0, 0, 0 );
	m_wndList = pMatch;
	m_wndList->SetBrowseMode();
	return CWnd::Create(	NULL, _T("CBrowseFrameCtrl"), WS_CHILD|WS_VISIBLE,
							rect, pParentWnd, IDC_BROWSE_FRAME, NULL );
}

int CBrowseFrameCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	if ( ! m_wndTreeTop.Create( this, WS_CHILD|WS_VISIBLE|CBRS_NOALIGN, AFX_IDW_TOOLBAR ) ) return -1;
	m_wndTreeTop.SetBarStyle( m_wndTreeTop.GetBarStyle() | CBRS_TOOLTIPS|CBRS_BORDER_BOTTOM );
	m_wndTreeTop.SetOwner( GetOwner() );
	
	m_wndTree.Create( this );
	m_wndDetails.Create( this );
	m_wndList->SetParent( this );
	m_wndList->SetOwner( GetParent() );
	
	return 0;
}

void CBrowseFrameCtrl::OnDestroy() 
{
	CWnd::OnDestroy();
}

void CBrowseFrameCtrl::OnSkinChange()
{
	Skin.CreateToolBar( _T("CBrowseTree.Top"), &m_wndTreeTop );
}

void CBrowseFrameCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize( nType, cx, cy );
	
	CRect rc;
	GetClientRect( &rc );
	
	if ( rc.Height() < 32 ) return;
	
	if ( rc.Width() < m_nTreeSize + SPLIT_SIZE )
	{
		m_nTreeSize = max( 0, rc.Width() - SPLIT_SIZE );
	}
	
	HDWP hDWP = BeginDeferWindowPos( 4 );
	
	if ( m_bTreeVisible )
	{
		DeferWindowPos( hDWP, m_wndTreeTop, NULL, rc.left, rc.top, m_nTreeSize,
			BAR_HEIGHT, SWP_SHOWWINDOW );
		DeferWindowPos( hDWP, m_wndTree, NULL, rc.left, rc.top + BAR_HEIGHT, m_nTreeSize,
			rc.Height() - BAR_HEIGHT, SWP_SHOWWINDOW );
		rc.left += m_nTreeSize + SPLIT_SIZE;
	}
	else
	{
		m_wndTreeTop.ShowWindow( SW_HIDE );
		m_wndTree.ShowWindow( SW_HIDE );
	}
	
	rc.top ++;
	
	if ( rc.Height() < m_nPanelSize + SPLIT_SIZE )
	{
		m_nPanelSize = max( 0, rc.Height() - SPLIT_SIZE );
	}
	
	if ( m_bPanelEnable && m_bPanelVisible )
	{
		DeferWindowPos( hDWP, m_wndDetails, NULL, rc.left, rc.bottom - m_nPanelSize, rc.Width(),
			m_nPanelSize, SWP_NOZORDER|SWP_SHOWWINDOW );
		rc.bottom -= m_nPanelSize + SPLIT_SIZE;
	}
	else
	{
		m_wndDetails.ShowWindow( SW_HIDE );
	}
	
	DeferWindowPos( hDWP, m_wndList->GetSafeHwnd(), NULL, rc.left, rc.top,
		rc.Width(), rc.Height(), SWP_NOZORDER|SWP_SHOWWINDOW );
	
	EndDeferWindowPos( hDWP );
}

void CBrowseFrameCtrl::OnPaint() 
{
	CPaintDC dc( this );
	CRect rcClient;
	
	GetClientRect( &rcClient );
	
	CRect rcBar(	rcClient.left + m_nTreeSize,
					rcClient.top,
					rcClient.left + m_nTreeSize + SPLIT_SIZE,
					rcClient.bottom );
	
	if ( m_wndTree.IsWindowVisible() )
	{
		dc.FillSolidRect( rcBar.left, rcBar.top, 1, rcBar.Height(), GetSysColor( COLOR_BTNFACE ) );
		dc.FillSolidRect( rcBar.left + 1, rcBar.top, 1, rcBar.Height(), GetSysColor( COLOR_3DHIGHLIGHT ) );
		dc.FillSolidRect( rcBar.right - 1, rcBar.top, 1, rcBar.Height(), GetSysColor( COLOR_3DSHADOW ) );
		dc.FillSolidRect( rcBar.left + 2, rcBar.top, rcBar.Width() - 3, rcBar.Height(), GetSysColor( COLOR_BTNFACE ) );
		dc.ExcludeClipRect( &rcBar );
		dc.FillSolidRect( rcBar.left, rcBar.top, rcClient.right - rcBar.left, 1, GetSysColor( COLOR_3DHIGHLIGHT ) );
	}
	else
	{
		dc.FillSolidRect( rcClient.left, rcClient.top, rcClient.Width(), 1, GetSysColor( COLOR_3DHIGHLIGHT ) );
	}
	
	rcBar.SetRect(	rcClient.left,
					rcClient.bottom - m_nPanelSize - SPLIT_SIZE,
					rcClient.right,
					rcClient.bottom - m_nPanelSize );
	
	if ( m_wndDetails.IsWindowVisible() )
	{
		if ( m_wndTree.IsWindowVisible() ) rcBar.left += m_nTreeSize + SPLIT_SIZE;
		
		dc.FillSolidRect( rcBar.left, rcBar.top, rcBar.Width(), 1, GetSysColor( COLOR_BTNFACE ) );
		dc.FillSolidRect( rcBar.left, rcBar.top + 1, rcBar.Width(), 1, GetSysColor( COLOR_3DHIGHLIGHT ) );
		dc.FillSolidRect( rcBar.left, rcBar.bottom - 1, rcBar.Width(), 1, GetSysColor( COLOR_3DSHADOW ) );
		dc.FillSolidRect( rcBar.left, rcBar.top + 2, rcBar.Width(), rcBar.Height() - 3,
			GetSysColor( COLOR_BTNFACE ) );
		dc.ExcludeClipRect( &rcBar );
	}
}

BOOL CBrowseFrameCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CRect rcClient, rc;
	CPoint point;
	
	GetCursorPos( &point );
	GetClientRect( &rcClient );
	ClientToScreen( &rcClient );
	
	rc.SetRect(	rcClient.left + m_nTreeSize,
				rcClient.top,
				rcClient.left + m_nTreeSize + SPLIT_SIZE,
				rcClient.bottom );
	
	if ( m_wndTree.IsWindowVisible() && rc.PtInRect( point ) )
	{
		SetCursor( AfxGetApp()->LoadStandardCursor( IDC_SIZEWE ) );
		return TRUE;
	}
	
	rc.SetRect(	rcClient.left,
				rcClient.bottom - m_nPanelSize - SPLIT_SIZE,
				rcClient.right,
				rcClient.bottom - m_nPanelSize );
	
	if ( m_wndTree.IsWindowVisible() ) rc.left += m_nTreeSize + SPLIT_SIZE;
	
	if ( m_wndDetails.IsWindowVisible() && rc.PtInRect( point ) )
	{
		SetCursor( AfxGetApp()->LoadStandardCursor( IDC_SIZENS ) );
		return TRUE;
	}
	
	return CWnd::OnSetCursor( pWnd, nHitTest, message );
}

void CBrowseFrameCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CRect rcClient, rc;
	GetClientRect( &rcClient );
	
	rc.SetRect(	rcClient.left + m_nTreeSize,
				rcClient.top,
				rcClient.left + m_nTreeSize + SPLIT_SIZE,
				rcClient.bottom );
	
	if ( m_wndTree.IsWindowVisible() && rc.PtInRect( point ) )
	{
		DoSizeTree();
		return;
	}
	
	rc.SetRect(	rcClient.left,
				rcClient.bottom - m_nPanelSize - SPLIT_SIZE,
				rcClient.right,
				rcClient.bottom - m_nPanelSize );
	
	if ( m_wndTree.IsWindowVisible() ) rc.left += m_nTreeSize + SPLIT_SIZE;
	
	if ( m_wndDetails.IsWindowVisible() && rc.PtInRect( point ) )
	{
		DoSizePanel();
		return;
	}
	
	CWnd::OnLButtonDown( nFlags, point );
}

BOOL CBrowseFrameCtrl::DoSizeTree()
{
	MSG* pMsg = &AfxGetThreadState()->m_msgCur;
	CRect rcClient;
	CPoint point;
	
	GetClientRect( &rcClient );
	ClientToScreen( &rcClient );
	ClipCursor( &rcClient );
	SetCapture();
	
	GetClientRect( &rcClient );
	
	int nOffset = 0xFFFF;
	
	while ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 )
	{
		while ( ::PeekMessage( pMsg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE ) );
		
		if ( ! AfxGetThread()->PumpMessage() )
		{
			AfxPostQuitMessage( 0 );
			break;
		}
		
		GetCursorPos( &point );
		ScreenToClient( &point );
		
		int nSplit = point.x - rcClient.left;
		
		if ( nOffset == 0xFFFF ) nOffset = m_nTreeSize - nSplit;
		nSplit += nOffset;
		
		nSplit = max( nSplit, 0 );
		nSplit = min( nSplit, rcClient.right - SPLIT_SIZE );
		
		if ( nSplit < 8 )
			nSplit = 0;
		if ( nSplit > rcClient.right - SPLIT_SIZE - 8 )
			nSplit = rcClient.right - SPLIT_SIZE;
		
		if ( nSplit != m_nTreeSize )
		{
			m_nTreeSize = nSplit;
			Settings.Search.BrowseTreeSize = (DWORD)m_nTreeSize;
			OnSize( SIZE_INTERNAL, 0, 0 );
			Invalidate();
		}
	}
	
	ReleaseCapture();
	ClipCursor( NULL );
	
	return TRUE;
}

BOOL CBrowseFrameCtrl::DoSizePanel()
{
	MSG* pMsg = &AfxGetThreadState()->m_msgCur;
	CRect rcClient;
	CPoint point;
	
	GetClientRect( &rcClient );
	if ( m_bTreeVisible ) rcClient.left += m_nTreeSize + SPLIT_SIZE;
	ClientToScreen( &rcClient );
	ClipCursor( &rcClient );
	SetCapture();
	
	ScreenToClient( &rcClient );
	
	int nOffset = 0xFFFF;
	
	while ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 )
	{
		while ( ::PeekMessage( pMsg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE ) );
		
		if ( ! AfxGetThread()->PumpMessage() )
		{
			AfxPostQuitMessage( 0 );
			break;
		}
		
		GetCursorPos( &point );
		ScreenToClient( &point );
		
		int nSplit = rcClient.bottom - point.y;
		
		if ( nOffset == 0xFFFF ) nOffset = m_nPanelSize - nSplit;
		nSplit += nOffset;
		
		if ( nSplit < 8 )
			nSplit = 0;
		if ( nSplit > rcClient.Height() - SPLIT_SIZE - 8 )
			nSplit = rcClient.Height() - SPLIT_SIZE;
		
		if ( nSplit != m_nPanelSize )
		{
			m_nPanelSize = nSplit;
			Settings.Search.DetailPanelSize = nSplit;
			OnSize( SIZE_INTERNAL, 0, 0 );
			Invalidate();
		}
	}
	
	ReleaseCapture();
	ClipCursor( NULL );
	
	return TRUE;
}

void CBrowseFrameCtrl::OnPhysicalTree(CG2Packet* pPacket)
{
	m_pTree[0] = pPacket;
	pPacket->AddRef();
	if ( m_nTree == 0 ) m_wndTree.OnTreePacket( pPacket );
	
	if ( ! m_bTreeVisible )
	{
		m_bTreeVisible = TRUE;
		PostMessage( WM_SIZE, SIZE_INTERNAL, 0 );
	}
}

void CBrowseFrameCtrl::OnVirtualTree(CG2Packet* pPacket)
{
	m_pTree[1] = pPacket;
	pPacket->AddRef();
	if ( m_nTree == 1 ) m_wndTree.OnTreePacket( pPacket );
	
	if ( ! m_bTreeVisible )
	{
		m_bTreeVisible = TRUE;
		PostMessage( WM_SIZE, SIZE_INTERNAL, 0 );
	}
}

void CBrowseFrameCtrl::OnTreeSelection(NMHDR* pNotify, LRESULT* pResult)
{
	CSingleLock pLock( &m_wndList->m_pMatches->m_pSection, TRUE );
	
	CMatchFile** ppFile = m_wndList->m_pMatches->m_pFiles;
	BOOL bGlobal = m_wndTree.GetSelectedCount() == 0;
	
	for ( DWORD nFiles = m_wndList->m_pMatches->m_nFiles ; nFiles ; nFiles--, ppFile++ )
	{
		CMatchFile* pFile = (*ppFile);
		
		for ( CQueryHit* pHit = pFile->m_pHits ; pHit ; pHit = pHit->m_pNext )
		{
			if ( pHit->m_bMatched = bGlobal ) continue;
			
			for (	CBrowseTreeItem* pSel = m_wndTree.GetFirstSelected() ; pSel ;
					pSel = pSel->m_pSelNext )
			{
				SelectTree( pSel, pHit );
			}
		}
	}
	
	CBrowseTreeItem* pTree = m_wndTree.GetFirstSelected();
	
	if ( pTree != NULL && pTree->m_pSelNext == NULL && pTree->m_pSchema != NULL )
	{
		CString strURI = pTree->m_pSchema->GetContainedURI( CSchema::stFile );
		
		if ( strURI.GetLength() &&
			 ( m_wndList->m_pSchema == NULL || m_wndList->m_pSchema->m_sURI != strURI ) )
		{
			if ( CSchema* pSchema = SchemaCache.Get( strURI ) )
			{
				CPtrList pColumns;
				CSchemaColumnsDlg::LoadColumns( pSchema, &pColumns );
				m_wndList->SelectSchema( pSchema, &pColumns );
			}
		}
	}
	
	m_wndList->m_pMatches->Filter();
	m_wndList->Update();
	
	GetOwner()->PostMessage( WM_COMMAND, MAKELONG( IDC_MATCHES, LBN_SELCHANGE ), 0 );
	
	*pResult = 0;
}

void CBrowseFrameCtrl::SelectTree(CBrowseTreeItem* pItem, CQueryHit* pHit)
{
	DWORD* pIndex = pItem->m_pFiles;
	
	for ( DWORD nIndex = pItem->m_nFiles ; nIndex ; nIndex--, pIndex++ )
	{
		if ( (*pIndex) == pHit->m_nIndex )
		{
			pHit->m_bMatched = TRUE;
			return;
		}
	}
	
	if ( pItem->m_bExpanded == TRUE ) return;
	
	CBrowseTreeItem** ppChild = pItem->m_pList;
	
	for ( nIndex = pItem->m_nCount ; nIndex ; nIndex--, ppChild++ )
	{
		SelectTree( *ppChild, pHit );
	}
}

void CBrowseFrameCtrl::OnSelChangeMatches()
{
	CSingleLock pLock( &m_wndList->m_pMatches->m_pSection, TRUE );
	m_wndDetails.Update( m_wndList->m_pMatches->GetSelectedFile( TRUE ) );
	pLock.Unlock();
	
	if ( m_bPanelEnable == FALSE )
	{
		m_bPanelEnable = TRUE;
		OnSize( SIZE_INTERNAL, 0, 0 );
	}
}

void CBrowseFrameCtrl::OnUpdateSearchDetails(CCmdUI* pCmdUI) 
{
	BOOL bVisible = m_bPanelEnable && m_wndList->IsWindowVisible();
	pCmdUI->Enable( bVisible );
	pCmdUI->SetCheck( bVisible && m_bPanelVisible );
}

void CBrowseFrameCtrl::OnSearchDetails() 
{
	m_bPanelVisible = Settings.Search.DetailPanelVisible = ! m_bPanelVisible;
	OnSize( SIZE_INTERNAL, 0, 0 );
}

void CBrowseFrameCtrl::OnUpdateLibraryTreePhysical(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( m_pTree[0] != NULL );
	pCmdUI->SetCheck( m_pTree[0] != NULL && m_nTree == 0 );
}

void CBrowseFrameCtrl::OnLibraryTreePhysical()
{
	if ( m_pTree[0] != NULL )
	{
		m_nTree = 0;
		m_wndTree.OnTreePacket( m_pTree[ m_nTree ] );
	}
}

void CBrowseFrameCtrl::OnUpdateLibraryTreeVirtual(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( m_pTree[1] != NULL );
	pCmdUI->SetCheck( m_pTree[1] != NULL && m_nTree == 1 );
}

void CBrowseFrameCtrl::OnLibraryTreeVirtual()
{
	if ( m_pTree[1] != NULL )
	{
		m_nTree = 1;
		m_wndTree.OnTreePacket( m_pTree[ m_nTree ] );
	}
}
