//
// CtrlCoolBar.cpp
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
#include "CtrlCoolBar.h"
#include "CoolInterface.h"
#include "Skin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CCoolBarCtrl, CControlBar)

BEGIN_MESSAGE_MAP(CCoolBarCtrl, CControlBar)
	//{{AFX_MSG_MAP(CCoolBarCtrl)
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_WM_HSCROLL()
	ON_WM_CTLCOLOR()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#define DEFAULT_HEIGHT		30
#define GRIPPER_WIDTH		4
#define SEPARATOR_WIDTH		7
#define MARGIN_WIDTH		5

#define BUTTON_WIDTH		9
#define IMAGE_WIDTH			16
#define IMAGEBUTTON_WIDTH	25	// ( BUTTON_WIDTH + IMAGE_WIDTH )
#define TEXT_GAP			4
#define CONTROL_HEIGHT		19


/////////////////////////////////////////////////////////////////////////////
// CCoolBar construction

CCoolBarCtrl::CCoolBarCtrl()
{
	m_bStretch		= FALSE;
	m_nHeight		= DEFAULT_HEIGHT;
	m_bGripper		= FALSE;
	m_bBold			= FALSE;
	m_bDragForward	= FALSE;
	m_pSyncObject	= NULL;
	m_bBuffered		= FALSE;
	m_bMenuGray		= FALSE;
	
	m_pDown		= NULL;
	m_pHot		= NULL;
	m_bTimer	= FALSE;
	m_crBack	= 0;
	m_bRecalc	= FALSE;
}

CCoolBarCtrl::~CCoolBarCtrl()
{
	Clear();
}

/////////////////////////////////////////////////////////////////////////////
// CCoolBar system operations

BOOL CCoolBarCtrl::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
	CRect rc;
	dwStyle |= WS_CHILD;
	return CWnd::Create( NULL, NULL, dwStyle, rc, pParentWnd, nID, NULL );
}

void CCoolBarCtrl::SetSize(int nHeight, BOOL bStretch)
{
	m_bStretch	= bStretch;
	m_nHeight	= nHeight < 1 ? DEFAULT_HEIGHT : nHeight;

	SetWindowPos( NULL, 0, 0, 0, 0,
		SWP_DRAWFRAME|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER );
}

void CCoolBarCtrl::SetGripper(BOOL bGripper)
{
	m_bGripper = bGripper;
}

void CCoolBarCtrl::SetBold(BOOL bBold)
{
	m_bBold = bBold;
	OnUpdated();
}

void CCoolBarCtrl::SetDragForward(BOOL bForward)
{
	m_bDragForward = bForward;
}

void CCoolBarCtrl::SetWatermark(HBITMAP hBitmap, BOOL bDetach)
{
	if ( m_bmImage.m_hObject )
	{
		if ( bDetach )
		{
			m_bmImage.Detach();
		}
		else
		{
			m_bmImage.DeleteObject();
		}
	}

	if ( hBitmap ) m_bmImage.Attach( hBitmap );
}

void CCoolBarCtrl::SetSyncObject(CSyncObject* pSyncObject)
{
	m_pSyncObject = pSyncObject;
}

/////////////////////////////////////////////////////////////////////////////
// CCoolBar item operations

CCoolBarItem* CCoolBarCtrl::Add(UINT nID, LPCTSTR pszText, int nPosition)
{
	CCoolBarItem* pItem = new CCoolBarItem( this, nID );
	
	if ( nPosition == -1 )
	{
		m_pItems.AddTail( pItem );
	}
	else
	{
		POSITION pos = m_pItems.FindIndex( nPosition );
		if ( pos ) m_pItems.InsertBefore( pos, pItem ); else m_pItems.AddTail( pItem );
	}
	
	pItem->m_nImage = CoolInterface.ImageForID( nID );
	
	if ( pszText ) pItem->SetText( pszText );
	
	return pItem;
}

CCoolBarItem* CCoolBarCtrl::Add(UINT nCtrlID, int nWidth, int nHeight)
{
	CCoolBarItem* pItem = new CCoolBarItem( this, nCtrlID );
	m_pItems.AddTail( pItem );
	
	pItem->m_nCtrlID		= nCtrlID;
	pItem->m_nWidth			= nWidth;
	pItem->m_nCtrlHeight	= nHeight ? nHeight : CONTROL_HEIGHT;
	
	return pItem;
}

CCoolBarItem* CCoolBarCtrl::GetIndex(int nIndex) const
{
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		CCoolBarItem* pItem = (CCoolBarItem*)m_pItems.GetNext( pos );
		if ( ! nIndex-- ) return pItem;
	}
	
	return NULL;
}

CCoolBarItem* CCoolBarCtrl::GetID(UINT nID) const
{
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		CCoolBarItem* pItem = (CCoolBarItem*)m_pItems.GetNext( pos );
		if ( pItem->m_nID == nID ) return pItem;
	}
	
	return NULL;
}

int CCoolBarCtrl::GetIndexForID(UINT nID) const
{
	int nIndex = 0;
	
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; nIndex++ )
	{
		CCoolBarItem* pItem = (CCoolBarItem*)m_pItems.GetNext( pos );
		if ( pItem->m_nID == nID ) return nIndex;
	}
	
	return -1;
}

int CCoolBarCtrl::GetCount() const
{
	return m_pItems.GetCount();
}

BOOL CCoolBarCtrl::LoadToolBar(UINT nIDToolBar)
{
	CToolBar pToolBar;
	
	if ( ! pToolBar.Create( this ) || ! pToolBar.LoadToolBar( nIDToolBar ) ) return FALSE;
	
	for ( int i = 0 ; i < pToolBar.GetCount(); i++ )
	{
		UINT nID, nStyle;
		int nImage;
		
		pToolBar.GetButtonInfo( i, nID, nStyle, nImage );
		
		Add( nID );
	}
	
	return TRUE;
}

void CCoolBarCtrl::Clear()
{
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		delete (CCoolBarItem*)m_pItems.GetNext( pos );
	}
	
	m_pItems.RemoveAll();
}

void CCoolBarCtrl::Copy(CCoolBarCtrl* pOther)
{
	Clear();
	
	for ( POSITION pos = pOther->m_pItems.GetHeadPosition() ; pos ; )
	{
		CCoolBarItem* pItem = (CCoolBarItem*)pOther->m_pItems.GetNext( pos );
		m_pItems.AddTail( new CCoolBarItem( this, pItem ) );
	}
}

UINT CCoolBarCtrl::ThrowMenu(UINT nID, CMenu* pMenu, CWnd* pParent, BOOL bCommand, BOOL bRight)
{
	if ( pMenu == NULL ) return 0;
	if ( pParent == NULL ) pParent = AfxGetMainWnd();
	if ( pParent == NULL ) pParent = this;
	
	m_pDown = GetID( nID );
	if ( m_pDown == NULL ) return 0;
	
	m_bMenuGray = TRUE;
	Invalidate();
	UpdateWindow();
	
	CRect rcButton;
	GetItemRect( m_pDown, &rcButton );
	ClientToScreen( &rcButton );
	rcButton.DeflateRect( 1, 2 );
	
	TPMPARAMS tpm;
	tpm.cbSize = sizeof(tpm);
	tpm.rcExclude = rcButton;
	
	DWORD nFlags = TPM_LEFTBUTTON|TPM_VERTICAL;
	
	if ( bCommand ) nFlags |= TPM_RETURNCMD;
	
#if 1
	CoolMenu.RegisterEdge( rcButton.left, rcButton.bottom, rcButton.Width() );
	bRight = FALSE;
#endif
	
	nFlags |= ( bRight ? TPM_RIGHTALIGN : TPM_LEFTALIGN );
	
	UINT nCmd = TrackPopupMenuEx( pMenu->GetSafeHmenu(), nFlags,
		bRight ? rcButton.right : rcButton.left, rcButton.bottom,
		pParent->GetSafeHwnd(), &tpm );
	
	m_bMenuGray = FALSE;
	m_pDown = NULL;
	Invalidate();
	
	return nCmd;
}

void CCoolBarCtrl::OnUpdated()
{
	if ( ! m_bStretch )
	{
		CSize czLast = m_czLast;
		
		if ( CalcFixedLayout( FALSE, TRUE ) != czLast )
		{
			CMDIFrameWnd* pOwner = (CMDIFrameWnd*)GetOwner();

			if ( pOwner && pOwner->IsKindOf( RUNTIME_CLASS(CMDIFrameWnd) ) )
			{
				if ( pOwner->IsIconic() )
					m_bRecalc = TRUE;
				else
					pOwner->RecalcLayout();
			}
		}
	}
	
	Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
// CCoolBar message handlers

int CCoolBarCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CControlBar::OnCreate( lpCreateStruct ) == -1 ) return -1;
	m_dwStyle |= CBRS_BORDER_3D;
	return 0;
}

CSize CCoolBarCtrl::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	if ( m_bStretch || bStretch )
	{
		CSize size( 32000, m_nHeight );

		if ( CWnd* pParent = AfxGetMainWnd() )
		{
			CRect rc;
			pParent->GetWindowRect( &rc );
			if ( rc.Width() > 32 ) size.cx = rc.Width() + 2;
		}

		m_czLast = size;

		return size;
	}
	else
	{
		CSize size( MARGIN_WIDTH * 2 + 5, m_nHeight );
		if ( m_bGripper ) size.cx += GRIPPER_WIDTH;

		for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
		{
			CCoolBarItem* pItem = (CCoolBarItem*)m_pItems.GetNext( pos );
			if ( pItem->m_bVisible ) size.cx += pItem->m_nWidth;
		}

		m_czLast = size;

		return size;
	}
}

void CCoolBarCtrl::PrepareRect(CRect* pRect) const
{
	CRect rcClient;
	GetClientRect( &rcClient );
	CalcInsideRect( rcClient, FALSE );

	rcClient.left -= m_cyTopBorder;
	rcClient.top -= m_cxLeftBorder;
	rcClient.right += m_cyBottomBorder;
	rcClient.bottom += m_cxRightBorder;

	pRect->SetRect( rcClient.left + MARGIN_WIDTH, rcClient.top + 1, rcClient.right - MARGIN_WIDTH, rcClient.bottom - 1 );
	if ( m_bGripper ) pRect->left += GRIPPER_WIDTH;
}

CCoolBarItem* CCoolBarCtrl::HitTest(const CPoint& point, CRect* pItemRect, BOOL bSeparators) const
{
	if ( m_pItems.IsEmpty() ) return NULL;

	BOOL bRight = FALSE;
	CRect rcClient, rcItem;

	PrepareRect( &rcClient );
	rcItem.CopyRect( &rcClient );
	
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		CCoolBarItem* pItem = (CCoolBarItem*)m_pItems.GetNext( pos );
		if ( ! pItem->m_bVisible ) continue;
		
		if ( pItem->m_nID == ID_RIGHTALIGN && ! bRight )
		{
			int nRight = 0;
			bRight = TRUE;
			
			for ( POSITION pos2 = pos ; pos2 ; )
			{
				CCoolBarItem* pRight = (CCoolBarItem*)m_pItems.GetNext( pos2 );
				if ( pRight->m_bVisible ) nRight += pRight->m_nWidth;
			}
			
			if ( rcClient.right - rcItem.left >= nRight )
			{
				rcItem.left = rcClient.right - nRight;
			}
		}
		else
		{
			rcItem.right = rcItem.left + pItem->m_nWidth;
			
			if ( rcItem.PtInRect( point ) )
			{
				if ( pItemRect ) *pItemRect = rcItem;
				return ( pItem->m_nID != ID_SEPARATOR || bSeparators ) ? pItem : NULL;
			}
			
			rcItem.OffsetRect( rcItem.Width(), 0 );
		}
	}

	return NULL;
}

BOOL CCoolBarCtrl::GetItemRect(CCoolBarItem* pFind, CRect* pRect) const
{
	if ( m_pItems.IsEmpty() ) return FALSE;
	
	BOOL bRight = FALSE;
	CRect rcClient, rcItem;
	
	PrepareRect( &rcClient );
	rcItem.CopyRect( &rcClient );
	
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		CCoolBarItem* pItem = (CCoolBarItem*)m_pItems.GetNext( pos );
		if ( ! pItem->m_bVisible ) continue;

		if ( pItem->m_nID == ID_RIGHTALIGN && ! bRight )
		{
			int nRight = 0;
			bRight = TRUE;
			
			for ( POSITION pos2 = pos ; pos2 ; )
			{
				CCoolBarItem* pRight = (CCoolBarItem*)m_pItems.GetNext( pos2 );
				if ( pRight->m_bVisible ) nRight += pRight->m_nWidth;
			}
			
			if ( rcClient.right - rcItem.left >= nRight )
			{
				rcItem.left = rcClient.right - nRight;
			}
		}
		else
		{
			rcItem.right = rcItem.left + pItem->m_nWidth;
			
			if ( pItem == pFind )
			{
				*pRect = rcItem;
				return TRUE;
			}
			
			rcItem.OffsetRect( rcItem.Width(), 0 );
		}
	}

	return FALSE;
}

int CCoolBarCtrl::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	CRect rcItem;
	CCoolBarItem* pItem = HitTest( point, &rcItem );

	if ( pItem == NULL ) return -1;
	if ( ! pTI ) return 1;

	pTI->uFlags		= 0;
	pTI->hwnd		= GetSafeHwnd();
	pTI->uId		= (UINT)pItem->m_nID;
	pTI->rect		= rcItem;
	pTI->lpszText	= LPSTR_TEXTCALLBACK;

	if ( pItem->m_sTip.GetLength() )
	{
		pTI->lpszText = _tcsdup( pItem->m_sTip );
	}
	else
	{
		CString strTip;

		if ( LoadString( strTip, pTI->uId ) )
		{
			if ( LPCTSTR pszBreak = _tcschr( strTip, '\n' ) )
			{
				pTI->lpszText = _tcsdup( pszBreak + 1 );
			}
			else
			{
				strTip = strTip.SpanExcluding( _T(".") );
				pTI->lpszText = _tcsdup( strTip );
			}
		}
	}

	return pTI->uId;
}

void CCoolBarCtrl::DoPaint(CDC* pDC)
{
	ASSERT_VALID( this );
	ASSERT_VALID( pDC );
	
	CRect rc;
	GetClientRect( &rc );
	
	if ( m_bBuffered || m_bmImage.m_hObject != NULL )
	{
		CDC* pBuffer = CoolInterface.GetBuffer( *pDC, rc.Size() );
		
		if ( CoolInterface.DrawWatermark( pBuffer, &rc, &m_bmImage ) )
		{
			CalcInsideRect( rc, FALSE );
			rc.left -= m_cyTopBorder;
			rc.top -= m_cxLeftBorder;
			rc.right += m_cyBottomBorder;
			rc.bottom += m_cxRightBorder;
		}
		else
		{
			DrawBorders( pBuffer, rc );
		}
		
		DoPaint( pBuffer, rc, TRUE );
		
		GetClientRect( &rc );
		pDC->BitBlt( 0, 0, rc.Width(), rc.Height(), pBuffer, 0, 0, SRCCOPY );
		pBuffer->SelectClipRgn( NULL );
	}
	else
	{
		DrawBorders( pDC, rc );
		DoPaint( pDC, rc, FALSE );
		pDC->FillSolidRect( &rc, CoolInterface.m_crMidtone );
	}
}

void CCoolBarCtrl::DoPaint(CDC* pDC, CRect& rcClient, BOOL bTransparent)
{
	CRect rcItem( rcClient.left + MARGIN_WIDTH, rcClient.top + 1, rcClient.right - MARGIN_WIDTH, rcClient.bottom - 1 );
	CRect rcCopy;
	
	if ( m_bGripper )
	{
		if ( bTransparent )
		{
			for ( int nY = rcClient.top + 4 ; nY < rcClient.bottom - 4 ; nY += 2 )
			{
				pDC->Draw3dRect( rcClient.left + 3, nY, GRIPPER_WIDTH, 1,
					CoolInterface.m_crDisabled, CoolInterface.m_crDisabled );
			}
		}
		else
		{
			for ( int nY = rcClient.top + 4 ; nY < rcClient.bottom - 4 ; nY += 2 )
			{
				pDC->Draw3dRect( rcClient.left + 3, nY, GRIPPER_WIDTH, 2,
					CoolInterface.m_crDisabled, CoolInterface.m_crMidtone );
			}
			
			pDC->ExcludeClipRect( rcClient.left + 3, rcClient.top + 4, rcClient.left + GRIPPER_WIDTH + 2, rcClient.bottom - 4 );
		}
		
		rcItem.left += GRIPPER_WIDTH;
	}
	
	if ( m_pItems.GetCount() == 0 ) return;
	
	CFont* pOldFont = (CFont*)pDC->SelectObject( m_bBold ? &CoolInterface.m_fntBold : &CoolInterface.m_fntNormal );
	BOOL bRight = FALSE;
	
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		CCoolBarItem* pItem = (CCoolBarItem*)m_pItems.GetNext( pos );
		
		if ( pItem->m_nID == ID_RIGHTALIGN && ! bRight )
		{
			int nRight = 0;
			bRight = TRUE;
			
			for ( POSITION pos2 = pos ; pos2 ; )
			{
				CCoolBarItem* pRight = (CCoolBarItem*)m_pItems.GetNext( pos2 );
				if ( pRight->m_bVisible ) nRight += pRight->m_nWidth;
			}
			
			if ( rcClient.right - rcItem.left >= nRight + MARGIN_WIDTH )
			{
				rcItem.left = rcClient.right - nRight - MARGIN_WIDTH;
			}
		}
		else if ( pItem->m_bVisible )
		{
			
			rcItem.right = rcItem.left + pItem->m_nWidth;
			rcCopy.CopyRect( &rcItem );
			
			CWnd* pCtrl = ( pItem->m_nCtrlID ) ? GetDlgItem( pItem->m_nCtrlID ) : NULL;
			
			pItem->Paint( pDC, rcCopy, m_pDown == pItem,
				m_pHot == pItem || ( pCtrl && pCtrl == GetFocus() ),
				m_bMenuGray, bTransparent );
			
			if ( ! bTransparent ) pDC->ExcludeClipRect( &rcItem );
			if ( pCtrl ) SmartMove( pCtrl, &rcCopy );
			
			rcItem.OffsetRect( rcItem.Width(), 0 );
		}
		else if ( pItem->m_nCtrlID )
		{
			CWnd* pCtrl = GetDlgItem( pItem->m_nCtrlID );
			if ( pCtrl && pCtrl->IsWindowVisible() ) pCtrl->ShowWindow( SW_HIDE );
		}
	}
	
	pDC->SelectObject( pOldFont );
}

void CCoolBarCtrl::SmartMove(CWnd* pCtrl, CRect* pRect)
{
	CRect rc;
	pCtrl->GetWindowRect( &rc );
	ScreenToClient( &rc );
	
	if ( rc != *pRect )
	{
		pCtrl->SetWindowPos( NULL, pRect->left, pRect->top, pRect->Width(),
			pRect->Height(), SWP_NOZORDER|SWP_SHOWWINDOW );
	}
	else if ( ! pCtrl->IsWindowVisible() )
	{
		pCtrl->ShowWindow( SW_SHOW );
	}
}

HBRUSH CCoolBarCtrl::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CControlBar::OnCtlColor( pDC, pWnd, nCtlColor );
	
	if ( nCtlColor == CTLCOLOR_STATIC )
	{
		pDC->SetBkColor( CoolInterface.m_crMidtone );
		
		if ( m_crBack != CoolInterface.m_crMidtone )
		{
			if ( m_brBack.m_hObject ) m_brBack.DeleteObject();
			m_brBack.CreateSolidBrush( m_crBack = CoolInterface.m_crMidtone );
		}

		hbr = m_brBack;
	}
	
	return hbr;
}

void CCoolBarCtrl::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	UINT nIndex		= 0;
	BOOL bDirty		= FALSE;
	BOOL bLocked	= FALSE;
	
	if ( m_pSyncObject != NULL )
	{
		bLocked = m_pSyncObject->Lock( 200 );
	}
	
	for ( POSITION pos = m_pItems.GetHeadPosition() ; pos ; )
	{
		CCoolBarItem* pItem = (CCoolBarItem*)m_pItems.GetNext( pos );
		
		if ( pItem->m_nID == ID_SEPARATOR ) continue;
		if ( pItem->m_nCtrlID ) continue;
		
		pItem->m_pOther		= this;
		pItem->m_nIndex		= nIndex++;
		pItem->m_nIndexMax	= m_pItems.GetCount();
		pItem->m_bDirty		= FALSE;
		BOOL bEnabled		= pItem->m_bEnabled;
		
		if ( ! CWnd::OnCmdMsg( pItem->m_nID, CN_UPDATE_COMMAND_UI, pItem, NULL ) )
		{
			pItem->DoUpdate( pTarget, bDisableIfNoHndler );
		}
		
		pItem->m_bDirty |= ( pItem->m_bEnabled != bEnabled );
		bDirty |= pItem->m_bDirty; 
	}
	
	if ( bLocked ) m_pSyncObject->Unlock();
	
	if ( bDirty ) OnUpdated();
}

void CCoolBarCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	CCoolBarItem* pItem = HitTest( point );
	
	if ( m_pDown && m_pDown != pItem ) pItem = NULL;
	
	if ( pItem != m_pHot )
	{
		m_pHot = pItem;
		Invalidate();
	}

	if ( ! m_bTimer )
	{
		SetTimer( 1, 100, NULL );
		m_bTimer = TRUE;
	}

	CControlBar::OnMouseMove( nFlags, point );
}

void CCoolBarCtrl::OnTimer(UINT nIDEvent) 
{
	if ( m_bRecalc )
	{
		CMDIFrameWnd* pOwner = (CMDIFrameWnd*)GetOwner();

		if ( pOwner != NULL && pOwner->IsKindOf( RUNTIME_CLASS(CMDIFrameWnd) ) )
		{
			if ( ! pOwner->IsIconic() )
			{
				pOwner->RecalcLayout();
				m_bRecalc = FALSE;
			}
		}
	}

	if ( nIDEvent == 1 && ( ! m_bMenuGray || m_pDown == NULL ) )
	{
		CRect rcWindow;
		CPoint point;

		GetCursorPos( &point );
		ScreenToClient( &point );
		GetClientRect( &rcWindow );

		if ( rcWindow.PtInRect( point ) && GetTopLevelParent()->IsWindowEnabled() )
		{
			CCoolBarItem* pItem = HitTest( point );

			if ( m_pDown && m_pDown != pItem ) pItem = NULL;

			if ( pItem != m_pHot )
			{
				m_pHot = pItem;
				Invalidate();
			}
		}
		else
		{
			KillTimer( nIDEvent );
			m_bTimer = FALSE;

			if ( m_pHot )
			{
				m_pHot = NULL;
				Invalidate();
			}
		}
	}

	CControlBar::OnTimer( nIDEvent );
}

void CCoolBarCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CWnd* pFocus = GetFocus();
	
	if ( pFocus && pFocus->GetParent() == this ) SetFocus();
	
	CCoolBarItem* pItem = HitTest( point );
	
	if ( pItem && pItem->m_bEnabled )
	{
		m_pDown = m_pHot = pItem;
		SetCapture();
		Invalidate();
		return;
	}
	
	if ( m_bDragForward )
		GetParent()->SendMessage( WM_LBUTTONDOWN, nFlags, MAKELONG( point.x, point.y ) );
	else
		CControlBar::OnLButtonDown( nFlags, point );
}

void CCoolBarCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CCoolBarItem* pItem = HitTest( point );
	
	if ( m_pDown != NULL )
	{
		BOOL bOn = ( m_pDown == pItem );
		
		m_pDown	= NULL;
		m_pHot	= pItem;
		
		ReleaseCapture();
		Invalidate();
		
		if ( bOn ) GetOwner()->PostMessage( WM_COMMAND, pItem->m_nID );
		
		return;
	}
	
	if ( m_bDragForward )
		GetParent()->SendMessage( WM_LBUTTONUP, nFlags, MAKELONG( point.x, point.y ) );
	else
		CControlBar::OnRButtonUp( nFlags, point );
}

void CCoolBarCtrl::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	CCoolBarCtrl::OnLButtonDown( nFlags, point );
}

void CCoolBarCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	if ( m_pDown != NULL || m_pHot != NULL )
	{
		m_pDown = m_pHot = NULL;
		
		ReleaseCapture();
		Invalidate();
	}
	
	CControlBar::OnRButtonDown( nFlags, point );
}

/////////////////////////////////////////////////////////////////////////////
// CCoolBar message forwarding

BOOL CCoolBarCtrl::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if ( HIWORD( wParam ) == EN_SETFOCUS ||
		 HIWORD( wParam ) == EN_KILLFOCUS ) Invalidate();

	GetParent()->SendMessage( WM_COMMAND, wParam, lParam );

	return CControlBar::OnCommand( wParam, lParam );
}

BOOL CCoolBarCtrl::PreTranslateMessage(MSG* pMsg) 
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( pMsg->wParam == VK_RETURN )
		{
			GetOwner()->PostMessage( WM_COMMAND, MAKELONG( GetDlgCtrlID(), BN_CLICKED ), (LPARAM)GetSafeHwnd() );
		}
		else if ( pMsg->wParam == VK_ESCAPE )
		{
			GetOwner()->PostMessage( WM_COMMAND, MAKELONG( GetDlgCtrlID(), BN_DBLCLK ), (LPARAM)GetSafeHwnd() );
		}
	}

	return CControlBar::PreTranslateMessage( pMsg );
}

void CCoolBarCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	GetParent()->SendMessage( WM_HSCROLL, MAKELONG( nSBCode, nPos ), (LPARAM)pScrollBar->GetSafeHwnd() );
}


/////////////////////////////////////////////////////////////////////////////
// CCoolBarItem construction

CCoolBarItem::CCoolBarItem(CCoolBarCtrl* pBar, UINT nID, int nImage)
{
	m_pBar		= pBar;
	m_nID		= nID;
	m_nImage	= nImage;
	m_bVisible	= TRUE;
	m_bEnabled	= TRUE;
	m_bChecked	= FALSE;
	m_crText	= 0xFFFFFFFF;
	
	switch ( nID )
	{
	case ID_SEPARATOR:
		m_nWidth = SEPARATOR_WIDTH;
		break;
	case ID_RIGHTALIGN:
		m_nWidth = 0;
		break;
	default:
		m_nWidth = IMAGEBUTTON_WIDTH;
		break;
	}

	m_nCtrlID		= 0;
	m_nCtrlHeight	= CONTROL_HEIGHT;
}

CCoolBarItem::CCoolBarItem(CCoolBarCtrl* pBar, CCoolBarItem* pCopy)
{
	m_pBar		= pBar;
	m_nID		= pCopy->m_nID;
	m_nImage	= pCopy->m_nImage;
	m_sTip		= pCopy->m_sTip;
	m_bVisible	= pCopy->m_bVisible;
	m_bEnabled	= pCopy->m_bEnabled;
	m_bChecked	= pCopy->m_bChecked;
	m_crText	= pCopy->m_crText;
	m_nWidth	= pCopy->m_nWidth;

	m_nCtrlID		= pCopy->m_nCtrlID;
	m_nCtrlHeight	= pCopy->m_nCtrlHeight;

	/* if ( m_nImage < 0 ) */ m_nImage = CoolInterface.ImageForID( m_nID );
	SetText( pCopy->m_sText );
}

CCoolBarItem::~CCoolBarItem()
{
}

/////////////////////////////////////////////////////////////////////////////
// CCoolBarItem CCmdUI handlers

void CCoolBarItem::Show(BOOL bOn)
{
	if ( this == NULL ) return;
	if ( m_bVisible == bOn ) return;
	m_bVisible	= bOn;
	m_bDirty	= TRUE;
}

void CCoolBarItem::Enable(BOOL bOn)
{
	if ( this == NULL ) return;
	m_bEnableChanged = TRUE;
	m_bEnabled = bOn;
}

void CCoolBarItem::SetCheck(int nCheck)
{
	if ( this == NULL ) return;
	if ( m_bChecked == ( nCheck == 1 ) ) return;
	m_bChecked	= ( nCheck == 1 );
	m_bDirty	= TRUE;
}

void CCoolBarItem::SetText(LPCTSTR lpszText)
{
	if ( this == NULL ) return;
	if ( m_sText == lpszText ) return;

	m_sText		= lpszText;
	m_bDirty	= TRUE;

	CDC dc;
	dc.Attach( ::GetDC( 0 ) );

	CFont* pOld = (CFont*)dc.SelectObject( m_pBar->m_bBold ? &CoolInterface.m_fntBold : &CoolInterface.m_fntNormal );
	m_nWidth = BUTTON_WIDTH + dc.GetTextExtent( m_sText ).cx;
	if ( m_nImage >= 0 ) m_nWidth += IMAGE_WIDTH + TEXT_GAP;
	dc.SelectObject( pOld );

	::ReleaseDC( 0, dc.Detach() );
}

void CCoolBarItem::SetTip(LPCTSTR pszTip)
{
	if ( this == NULL ) return;
	if ( m_sTip == pszTip ) return;
	m_sTip		= pszTip;
	m_bDirty	= TRUE;
}

void CCoolBarItem::SetTextColour(COLORREF crText)
{
	if ( this == NULL ) return;
	if ( m_crText == crText ) return;
	m_crText	= crText;
	m_bDirty	= TRUE;
}

void CCoolBarItem::SetImage(UINT nCommandID)
{
	if ( this == NULL ) return;
	int nImage = CoolInterface.ImageForID( nCommandID );
	if ( nImage == m_nImage ) return;
	m_nImage = nImage;
	m_bDirty = TRUE;
}

CCoolBarItem* CCoolBarItem::FromCmdUI(CCmdUI* pUI)
{
	if ( pUI->m_pOther == NULL ) return NULL;
	if ( pUI->m_pOther->IsKindOf( RUNTIME_CLASS(CCoolBarCtrl) ) == FALSE ) return NULL;
	return (CCoolBarItem*)pUI;
}

/////////////////////////////////////////////////////////////////////////////
// CCoolBarItem painting

void CCoolBarItem::Paint(CDC* pDC, CRect& rc, BOOL bDown, BOOL bHot, BOOL bMenuGray, BOOL bTransparent)
{
	COLORREF crBackground;

	if ( m_nID == ID_SEPARATOR )
	{
		if ( ! bTransparent ) pDC->FillSolidRect( rc.left, rc.top, 3, rc.Height(), CoolInterface.m_crMidtone );
		pDC->Draw3dRect( rc.left + 3, rc.top, 1, rc.Height(), CoolInterface.m_crDisabled, CoolInterface.m_crDisabled );
		if ( ! bTransparent ) pDC->FillSolidRect( rc.left + 4, rc.top, 3, rc.Height(), CoolInterface.m_crMidtone );
		return;
	}

	if ( m_nCtrlID )
	{
		for ( int nShrink = rc.Height() - m_nCtrlHeight ; nShrink > 0 ; nShrink -= 2 )
		{
			if ( ! bTransparent ) pDC->Draw3dRect( &rc, CoolInterface.m_crMidtone, CoolInterface.m_crMidtone );
			rc.DeflateRect( 0, 1 );
		}
		rc.DeflateRect( 1, 0 );
	}
	else
	{
		if ( ! bTransparent ) pDC->Draw3dRect( &rc, CoolInterface.m_crMidtone, CoolInterface.m_crMidtone );
		rc.DeflateRect( 1, 1 );
	}

//	if ( ( m_bEnabled || m_nCtrlID ) && ( bHot || bDown || m_bChecked ) )
	if ( m_bEnabled && ( bHot || bDown || m_bChecked ) )
	{
		if ( bMenuGray && bDown )
		{
			pDC->Draw3dRect( &rc, CoolInterface.m_crDisabled, CoolInterface.m_crDisabled );
		}
		else
		{
			pDC->Draw3dRect( &rc, CoolInterface.m_crBorder, CoolInterface.m_crBorder );
		}

		rc.DeflateRect( 1, 1 );

		if ( bMenuGray && bDown )
		{
			crBackground = CoolInterface.m_crBackNormal;
		}
		else if ( m_bChecked )
		{
			crBackground = bHot ? CoolInterface.m_crBackCheckSel : CoolInterface.m_crBackCheck;
		}
		else
		{
			crBackground = bDown && bHot ? CoolInterface.m_crBackCheckSel : CoolInterface.m_crBackSel;
		}
	}
	else
	{
		if ( bTransparent )
		{
			crBackground = CLR_NONE;
		}
		else
		{
			crBackground = CoolInterface.m_crMidtone;
			pDC->Draw3dRect( &rc, crBackground, crBackground );
		}

		rc.DeflateRect( 1, 1 );
	}

	if ( m_nCtrlID )
	{
		if ( m_nCtrlHeight == CONTROL_HEIGHT )
		{
			pDC->Draw3dRect( &rc, CoolInterface.m_crWindow, CoolInterface.m_crWindow );
			rc.DeflateRect( 1, 1 );
		}
		return;
	}

	if ( crBackground == CLR_NONE )
	{
		pDC->SetBkMode( TRANSPARENT );
	}
	else
	{
		pDC->SetBkMode( OPAQUE );
		pDC->SetBkColor( crBackground );
	}

	if ( m_sText.GetLength() )
	{
		if ( m_crText != 0xFFFFFFFF )
			pDC->SetTextColor( m_crText );
		else if ( ! m_bEnabled )
			pDC->SetTextColor( CoolInterface.m_crDisabled );
		else if ( ( bHot || bDown || m_bChecked ) && ( ! bMenuGray || ! bDown ) )
			pDC->SetTextColor( CoolInterface.m_crCmdTextSel );
		else
			pDC->SetTextColor( CoolInterface.m_crCmdText );

		rc.left += ( m_nImage >= 0 ) ? 20 : 1;
		int nY = ( rc.top + rc.bottom ) / 2 - pDC->GetTextExtent( m_sText ).cy / 2 - 1;
		
		if ( crBackground == CLR_NONE ) 
			pDC->ExtTextOut( rc.left + 2, nY, ETO_CLIPPED, &rc, m_sText, NULL );
		else
			pDC->ExtTextOut( rc.left + 2, nY, ETO_CLIPPED|ETO_OPAQUE, &rc, m_sText, NULL );

		rc.right = rc.left;
		rc.left -= ( m_nImage >= 0 ) ? 20 : 1;
	}

	if ( m_nImage >= 0 )
	{
		CPoint ptImage( rc.left + 3, ( rc.top + rc.bottom ) / 2 - 8 );

		if ( ! m_bEnabled )
		{
			ImageList_DrawEx( CoolInterface.m_pImages.GetSafeHandle(), m_nImage, pDC->GetSafeHdc(),
				ptImage.x, ptImage.y, 0, 0, crBackground, CoolInterface.m_crShadow, ILD_BLEND50 );
			pDC->ExcludeClipRect( ptImage.x, ptImage.y, ptImage.x + 16, ptImage.y + 16 );
		}
		else if ( m_bChecked )
		{
			ImageList_DrawEx( CoolInterface.m_pImages.GetSafeHandle(), m_nImage, pDC->GetSafeHdc(),
				ptImage.x, ptImage.y, 0, 0, crBackground, CLR_NONE, ILD_NORMAL );
			pDC->ExcludeClipRect( ptImage.x, ptImage.y, ptImage.x + 16, ptImage.y + 16 );
		}
		else if ( ( bHot && ! bDown ) || ( bDown && ! bHot ) )
		{
			ptImage.Offset( 1, 1 );
			pDC->SetTextColor( CoolInterface.m_crShadow );
			ImageList_DrawEx( CoolInterface.m_pImages.GetSafeHandle(), m_nImage, pDC->GetSafeHdc(),
				ptImage.x, ptImage.y, 0, 0, crBackground, CLR_NONE, ILD_MASK );

			ptImage.Offset( -2, -2 );

			if ( crBackground != CLR_NONE )
			{
				pDC->FillSolidRect( ptImage.x, ptImage.y, 18, 2, crBackground );
				pDC->FillSolidRect( ptImage.x, ptImage.y + 2, 2, 16, crBackground );
			}

			ImageList_DrawEx( CoolInterface.m_pImages.GetSafeHandle(), m_nImage, pDC->GetSafeHdc(),
				ptImage.x, ptImage.y, 0, 0, CLR_NONE, CLR_NONE, ILD_NORMAL );

			pDC->ExcludeClipRect( ptImage.x, ptImage.y, ptImage.x + 18, ptImage.y + 18 );
		}
		else
		{
			ImageList_DrawEx( CoolInterface.m_pImages.GetSafeHandle(), m_nImage, pDC->GetSafeHdc(),
				ptImage.x, ptImage.y, 0, 0, crBackground, CoolInterface.m_crBackNormal,
				bDown ? ILD_NORMAL : ILD_BLEND25 );
			pDC->ExcludeClipRect( ptImage.x, ptImage.y, ptImage.x + 16, ptImage.y + 16 );
		}
	}
	
	if ( crBackground != CLR_NONE ) pDC->FillSolidRect( &rc, crBackground );
}
