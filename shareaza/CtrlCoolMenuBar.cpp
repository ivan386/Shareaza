//
// CtrlCoolMenuBar.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2013.
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
#include "CoolInterface.h"
#include "CoolMenu.h"
#include "CtrlCoolMenuBar.h"
#include "WndMain.h"
#include "WndChild.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CCoolMenuBarCtrl, CCoolBarCtrl)
	ON_WM_LBUTTONDOWN()
	ON_WM_TIMER()
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_WM_INITMENUPOPUP()
	ON_WM_MENUSELECT()
	ON_WM_ENTERIDLE()
	ON_WM_ENTERMENULOOP()
	ON_WM_EXITMENULOOP()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCoolMenuBarCtrl construction

CCoolMenuBarCtrl::CCoolMenuBarCtrl()
{
	m_bMenuGray	= TRUE;
	m_bGripper	= TRUE;

	m_bStretch	= theApp.GetProfileInt( _T(""), _T("MenuStretch"), TRUE );
	if ( theApp.GetProfileInt( _T(""), _T("MenuHalfHeight"), TRUE ) ) m_nHeight = 28;

	m_hMenu		= NULL;
}

CCoolMenuBarCtrl::~CCoolMenuBarCtrl()
{
}

/////////////////////////////////////////////////////////////////////////////
// CCoolMenuBarCtrl operations

void CCoolMenuBarCtrl::SetMenu(HMENU hMenu)
{
	m_hMenu = hMenu;

	Clear();

	if ( ! m_hMenu ) return;

	CMenu pMenu;
	pMenu.Attach( m_hMenu );

	for ( UINT nItem = 0 ; nItem < (UINT)pMenu.GetMenuItemCount() ; nItem++ )
	{
		CString strMenu;
		pMenu.GetMenuString( nItem, strMenu, MF_BYPOSITION );

		int nAmp = strMenu.Find( '&' );
		if ( nAmp >= 0 ) strMenu = strMenu.Left( nAmp ) + strMenu.Mid( nAmp + 1 );

		CCoolBarItem* pItem = new CCoolBarItem( this, nItem + 1 );
		pItem->SetText( _T(" ") + strMenu + _T(" ") );
		m_pItems.AddTail( pItem );
	}

	pMenu.Detach();
}

void CCoolMenuBarCtrl::OpenMenuBar()
{
	if ( m_pDown == NULL )
	{
		if ( ( m_pSelect = GetIndex( 0 ) ) != NULL ) PostMessage( WM_TIMER, 5 );
	}
}

BOOL CCoolMenuBarCtrl::OpenMenuChar(UINT nChar)
{
	CMenu pMenu;
	pMenu.Attach( m_hMenu );

	for ( UINT nItem = 0 ; nItem < (UINT)pMenu.GetMenuItemCount() ; nItem++ )
	{
		CString strMenu;
		pMenu.GetMenuString( nItem, strMenu, MF_BYPOSITION );

		LPCTSTR pszChar = _tcschr( strMenu, '&' );
		if ( ! pszChar++ ) continue;

		if ( toupper( *pszChar ) == toupper( nChar ) )
		{
			pMenu.Detach();
			if ( ( m_pSelect = GetIndex( nItem ) ) != NULL ) PostMessage( WM_TIMER, 5 );
			return TRUE;
		}
	}

	pMenu.Detach();
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CCoolMenuBarCtrl message handlers

void CCoolMenuBarCtrl::ShowMenu()
{
	if ( m_pHot == NULL || ! IsMenu( m_hMenu ) ) return;

	CMenu* pMenu = CMenu::FromHandle( m_hMenu )->GetSubMenu( m_pHot->m_nID - 1 );

	if ( pMenu == NULL )
	{
		m_pHot = m_pDown = NULL;
		Invalidate();
		return;
	}

	UINT nFirstID = pMenu->GetMenuItemID( 0 );

	if ( nFirstID == ID_WINDOW_CASCADE ||
		 nFirstID == ID_WINDOW_NAVBAR )
	{
		UpdateWindowMenu( pMenu );
	}

	m_pDown = m_pHot;
	Invalidate();

	KillTimer( 1 );

	TPMPARAMS tpm;
	CRect rc;

	GetItemRect( m_pDown, &rc );
	ClientToScreen( &rc );
	rc.DeflateRect( 1, 2 );

	tpm.cbSize = sizeof(tpm);
	tpm.rcExclude = rc;

	m_pMenuBar = this;
	m_hMsgHook = SetWindowsHookEx( WH_MSGFILTER, MenuFilter, NULL, GetCurrentThreadId() );

	CoolMenu.RegisterEdge( Settings.General.LanguageRTL ? rc.right : rc.left, rc.bottom, rc.Width() );
	
	UINT nCmd = TrackPopupMenuEx( pMenu->GetSafeHmenu(),
		TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_VERTICAL|TPM_RETURNCMD,
		Settings.General.LanguageRTL ? rc.right : rc.left, rc.bottom, GetSafeHwnd(), &tpm );

	UnhookWindowsHookEx( m_hMsgHook );

	m_hMsgHook = NULL;
	m_pMenuBar = NULL;

	m_pDown = NULL;
	OnTimer( 1 );

	if ( m_pHot != NULL )
	{
		SetTimer( 1, 100, NULL );
		m_bTimer = TRUE;
	}

	Invalidate();
	UpdateWindow();

	if ( nCmd ) GetOwner()->PostMessage( WM_COMMAND, nCmd );
}

void CCoolMenuBarCtrl::UpdateWindowMenu(CMenu* pMenu)
{
	for ( UINT nItem = 0 ; nItem < (UINT)pMenu->GetMenuItemCount() ; nItem++ )
	{
		UINT nID = pMenu->GetMenuItemID( nItem );

		if ( nID >= AFX_IDM_FIRST_MDICHILD )
		{
			for ( UINT nRemove = nItem ; nRemove < (UINT)pMenu->GetMenuItemCount() ; )
				pMenu->RemoveMenu( nItem, MF_BYPOSITION );
			pMenu->RemoveMenu( nItem - 1, MF_BYPOSITION );
			break;
		}
	}

	CMainWnd* pFrame = theApp.SafeMainWnd();
	if ( ! pFrame->IsKindOf( RUNTIME_CLASS(CMDIFrameWnd) ) ) return;

    CWnd* pClient = pFrame->GetWindow( GW_CHILD );
	for ( ; pClient ; pClient = pClient->GetNextWindow() )
	{
		TCHAR szClass[64];
		GetClassName( pClient->GetSafeHwnd(), szClass, 64 );
		if ( _tcsicmp( szClass, _T("MDIClient") ) == 0 ) break;
	}

	if ( pClient == NULL ) return;

	CMDIChildWnd* pActive = pFrame->m_pWindows.GetActive();
	BOOL bSeparator = TRUE;

	for ( UINT nIndex = 1, nID = AFX_IDM_FIRST_MDICHILD ; nIndex <= 10 ; nIndex++, nID++ )
	{
		CChildWnd* pChildWnd = (CChildWnd*)pClient->GetDlgItem( nID );
		if ( ! pChildWnd ) break;

		if ( pChildWnd->m_bTabMode )
		{
			nIndex--;
			continue;
		}

		if ( bSeparator )
		{
			pMenu->AppendMenu( MF_SEPARATOR, ID_SEPARATOR );
			bSeparator = FALSE;
		}

		CString strMenu, strWindow;
		pChildWnd->GetWindowText( strWindow );

		strMenu.Format( _T("&%u %s"), nIndex, (LPCTSTR)strWindow );

		pMenu->AppendMenu( MF_STRING | ( pChildWnd == pActive ? MF_CHECKED : 0 ),
			nID, strMenu );
	}
}

void CCoolMenuBarCtrl::ShiftMenu(int nOffset)
{
	INT_PTR nIndex = 0;

	if ( m_pDown )
	{
		nIndex = m_pDown->m_nID - 1 + nOffset;
		if ( nIndex < 0 ) nIndex = GetCount() - 1;
		if ( nIndex >= GetCount() ) nIndex = 0;
	}

	if ( Settings.WINE.MenuFix )
		PostMessage( WM_CANCELMODE, 0, 0 );
	else
		SendMessage( WM_CANCELMODE, 0, 0 );
	m_pSelect = GetIndex( static_cast< int >( nIndex ) );
	m_pHot = m_pDown = NULL;
	PostMessage( WM_TIMER, 5 );
}

void CCoolMenuBarCtrl::OnTimer(UINT_PTR nIDEvent)
{
	if ( theApp.m_bClosing )
		return;

	switch ( nIDEvent )
	{
	case 5:
		m_pHot = m_pSelect;
	case 4:
		ShowMenu();
		break;
	default:
		CCoolBarCtrl::OnTimer( nIDEvent );
		break;
	}
}

void CCoolMenuBarCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	if ( theApp.m_bClosing )
		return;

	CCoolBarItem* pHit = HitTest( point );

	if ( pHit && pHit == m_pHot )
	{
		ShowMenu();
		return;
	}

	CCoolBarCtrl::OnLButtonDown( nFlags, point );
}

/////////////////////////////////////////////////////////////////////////////
// CCoolMenuBarCtrl menu message forwarding

void CCoolMenuBarCtrl::OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL /*bDisableIfNoHndler*/)
{
}

void CCoolMenuBarCtrl::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	GetOwner()->SendMessage( WM_INITMENUPOPUP, (WPARAM)pPopupMenu->GetSafeHmenu(),
		MAKELONG( nIndex, bSysMenu ) );
}

void CCoolMenuBarCtrl::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	GetOwner()->SendMessage( WM_MEASUREITEM, (WPARAM)nIDCtl, (LPARAM)lpMeasureItemStruct );
}

void CCoolMenuBarCtrl::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	GetOwner()->SendMessage( WM_DRAWITEM, (WPARAM)nIDCtl, (LPARAM)lpDrawItemStruct );
}

void CCoolMenuBarCtrl::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu)
{
	GetOwner()->SendMessage( WM_MENUSELECT, MAKELONG( nItemID, nFlags ), (LPARAM)hSysMenu );
}

void CCoolMenuBarCtrl::OnEnterMenuLoop(BOOL bIsTrackPopupMenu)
{
	GetOwner()->SendMessage( WM_ENTERMENULOOP, (WPARAM)bIsTrackPopupMenu );
}

void CCoolMenuBarCtrl::OnExitMenuLoop(BOOL bIsTrackPopupMenu)
{
	GetOwner()->SendMessage( WM_EXITMENULOOP, (WPARAM)bIsTrackPopupMenu );
}

void CCoolMenuBarCtrl::OnEnterIdle(UINT nWhy, CWnd* pWho)
{
	GetOwner()->SendMessage( WM_ENTERIDLE, (WPARAM)nWhy, (LPARAM)pWho->GetSafeHwnd() );
}

/////////////////////////////////////////////////////////////////////////////
// CCoolMenuBarCtrl menu message hooking

CCoolMenuBarCtrl* CCoolMenuBarCtrl::m_pMenuBar = NULL;
HHOOK CCoolMenuBarCtrl::m_hMsgHook = NULL;

LRESULT CCoolMenuBarCtrl::MenuFilter(int nCode, WPARAM wParam, LPARAM lParam)
{
	MSG* pMsg = (MSG*)lParam;

	if ( m_pMenuBar && nCode == MSGF_MENU )
	{
		if ( m_pMenuBar->OnMenuMessage( pMsg ) ) return TRUE;
	}

	return CallNextHookEx( m_hMsgHook, nCode, wParam, lParam );
}

BOOL CCoolMenuBarCtrl::OnMenuMessage(MSG* pMsg)
{
	if ( theApp.m_bClosing )
		return FALSE;

	switch ( pMsg->message )
	{
	case WM_MOUSEMOVE:
		{
			//CPoint pt( LOWORD( pMsg->lParam ), HIWORD( pMsg->lParam ) );
			CPoint pt( pMsg->lParam );
			ScreenToClient( &pt );

			if ( m_pMouse == pt ) return TRUE;
			m_pMouse = pt;

			CCoolBarItem* pHit = HitTest( pt );

			if ( pHit && pHit != m_pDown )
			{
				if ( Settings.WINE.MenuFix )
					PostMessage( WM_CANCELMODE, 0, 0 );
				else
					SendMessage( WM_CANCELMODE, 0, 0 );
				m_pHot	= pHit;
				m_pDown	= NULL;
				PostMessage( WM_TIMER, 4 );
				return TRUE;
			}
		}
		break;
	case WM_LBUTTONDOWN:
		{
			//CPoint pt( LOWORD( pMsg->lParam ), HIWORD( pMsg->lParam ) );
			CPoint pt( pMsg->lParam );

			CWnd* pWnd = CWnd::WindowFromPoint( pt );

			if ( pWnd )
			{
				TCHAR szClass[2];
				GetClassName( pWnd->GetSafeHwnd(), szClass, 2 );
				if ( szClass[0] == '#' ) return FALSE;
			}

			ScreenToClient( &pt );

			CCoolBarItem* pHit = HitTest( pt );

			if ( pHit == NULL )
			{
				m_pHot = m_pDown = NULL;
				if ( Settings.WINE.MenuFix )
					PostMessage( WM_CANCELMODE, 0, 0 );
				else
					SendMessage( WM_CANCELMODE, 0, 0 );
				return TRUE;
			}
			else if ( pHit == m_pDown )
			{
				m_pDown = NULL;
				PostMessage( WM_CANCELMODE, 0, 0 );
				return TRUE;
			}
		}
		break;
	case WM_KEYDOWN:
		switch ( pMsg->wParam )
		{
		case VK_LEFT:
			ShiftMenu( -1 );
			return TRUE;
		case VK_RIGHT:
			ShiftMenu( 1 );
			return TRUE;
		case VK_ESCAPE:
			PostMessage( WM_CANCELMODE, 0, 0 );
			m_pHot = m_pDown = NULL;
			return TRUE;
		}
		break;
	default:
		break;
	}

	return FALSE;
}

