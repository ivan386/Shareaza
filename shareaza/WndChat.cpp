//
// WndChat.cpp
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
#include "Settings.h"
#include "WndChat.h"
#include "CtrlChatFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CChatWnd, CChildWnd)

BEGIN_MESSAGE_MAP(CChatWnd, CChildWnd)
	//{{AFX_MSG_MAP(CChatWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CChatWnd construction

CChatWnd::CChatWnd(CChatFrame* pFrame)
{
	m_pFrame = pFrame;
	ASSERT_VALID(m_pFrame);
	Create( IDR_CHATFRAME, FALSE );
}

CChatWnd::~CChatWnd()
{
}

/////////////////////////////////////////////////////////////////////////////
// CChatWnd message handlers

int CChatWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CChildWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	ASSERT_VALID( m_pFrame );

	m_pFrame->SetParent( this );

	LoadState( _T("CChatWnd"), FALSE );

	CRect rc;
	GetClientRect( &rc );
	m_pFrame->MoveWindow( &rc, TRUE );

	SetAlert();

	return 0;
}

void CChatWnd::OnDestroy()
{
	SaveState( _T("CChatWnd") );

	if ( m_pFrame != NULL )
	{
		m_pFrame->DestroyWindow();
		delete m_pFrame;
	}

	CChildWnd::OnDestroy();
}

void CChatWnd::OnSkinChange()
{
	CChildWnd::OnSkinChange();
	if ( m_pFrame != NULL ) m_pFrame->OnSkinChange();
}

BOOL CChatWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if ( m_pFrame != NULL )
	{
		if ( m_pFrame->OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) ) return TRUE;
	}

	return CChildWnd::OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );
}

void CChatWnd::OnSize(UINT nType, int cx, int cy)
{
	CChildWnd::OnSize( nType, cx, cy );

	if ( m_pFrame != NULL )
	{
		m_pFrame->SetWindowPos( NULL, 0, 0, cx, cy, SWP_NOZORDER|SWP_SHOWWINDOW );
	}
}

void CChatWnd::OnTimer(UINT_PTR nIDEvent)
{
	if ( nIDEvent == 1 && m_pFrame != NULL && IsActive( TRUE ) )
	{
		m_pFrame->SetFocus();
	}
	else if ( nIDEvent == 2 && m_pFrame != NULL )
	{
		CString str;
		m_pFrame->GetWindowText( str );
		if ( str.Find( _T("Chat : ") ) == 0 )
		{
			CString strTranslation;
			LoadString( strTranslation, IDR_CHATFRAME );
			str = strTranslation + str.Mid( 4 );
		}
		SetWindowText( str );
	}

	CChildWnd::OnTimer( nIDEvent );
}
