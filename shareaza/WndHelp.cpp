//
// WndHelp.cpp
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
#include "WndHelp.h"
#include "Skin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL(CHelpWnd, CPanelWnd, 1)

BEGIN_MESSAGE_MAP(CHelpWnd, CPanelWnd)
	//{{AFX_MSG_MAP(CHelpWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#define PANEL_WIDTH		200
#define TOOLBAR_HEIGHT	28


/////////////////////////////////////////////////////////////////////////////
// CHelpWnd	construction

CHelpWnd::CHelpWnd() : CPanelWnd( TRUE, FALSE )
{
	Create( IDR_HELPFRAME );
}

CHelpWnd::~CHelpWnd()
{
}

/////////////////////////////////////////////////////////////////////////////
// CHelpWnd message handlers

int CHelpWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	CRect rect( 0, 0, 0, 0 );
	
	if ( ! m_wndToolBar.Create( this, WS_CHILD|WS_VISIBLE|CBRS_NOALIGN, AFX_IDW_TOOLBAR ) ) return -1;
	m_wndToolBar.SetBarStyle( m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_BORDER_TOP );
	m_wndPanel.Create( WS_CHILD|WS_VISIBLE, rect, this, 100 );
	m_wndView.Create( WS_CHILD|WS_VISIBLE, rect, this, 100 );
	
	LoadState();
	
	return 0;
}

void CHelpWnd::OnDestroy() 
{
	SaveState();
	CPanelWnd::OnDestroy();
}

void CHelpWnd::OnSkinChange()
{
	CPanelWnd::OnSkinChange();
	Skin.CreateToolBar( _T("CHelpWnd"), &m_wndToolBar );
	// m_wndPanel.OnSkinChange();
}

void CHelpWnd::OnSize(UINT nType, int cx, int cy) 
{
	if ( nType != 1982 ) CPanelWnd::OnSize( nType, cx, cy );
	
	CRect rc;
	GetClientRect( &rc );
	
	HDWP hDWP = BeginDeferWindowPos( 3 );
	
	DeferWindowPos( hDWP, m_wndToolBar, HWND_TOP, PANEL_WIDTH, rc.bottom - TOOLBAR_HEIGHT, rc.right - PANEL_WIDTH, TOOLBAR_HEIGHT, 0 );
	DeferWindowPos( hDWP, m_wndPanel, NULL, 0, 0, PANEL_WIDTH, rc.bottom, SWP_NOZORDER );
	DeferWindowPos( hDWP, m_wndView, NULL, PANEL_WIDTH, 0, rc.right - PANEL_WIDTH, rc.bottom - TOOLBAR_HEIGHT, SWP_NOZORDER );
	
	EndDeferWindowPos( hDWP );
}

BOOL CHelpWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	if ( m_wndToolBar.m_hWnd != NULL )
	{
		if ( m_wndToolBar.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) ) return TRUE;
	}
	
	return CPanelWnd::OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );
}
