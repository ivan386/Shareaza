//
// CtrlPrivateChatFrame.cpp
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
#include "ChatSession.h"
#include "RichElement.h"
#include "Transfers.h"
#include "Uploads.h"
#include "UploadTransfer.h"
#include "CtrlPrivateChatFrame.h"
#include "WndBrowseHost.h"
#include "Skin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CPrivateChatFrame, CChatFrame)

BEGIN_MESSAGE_MAP(CPrivateChatFrame, CChatFrame)
	//{{AFX_MSG_MAP(CPrivateChatFrame)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_CONTEXTMENU()
	ON_UPDATE_COMMAND_UI(ID_CHAT_BROWSE, OnUpdateChatBrowse)
	ON_COMMAND(ID_CHAT_BROWSE, OnChatBrowse)
	ON_UPDATE_COMMAND_UI(ID_CHAT_PRIORITY, OnUpdateChatPriority)
	ON_COMMAND(ID_CHAT_PRIORITY, OnChatPriority)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#define EDIT_HEIGHT		32
#define TOOLBAR_HEIGHT	30


/////////////////////////////////////////////////////////////////////////////
// CPrivateChatFrame construction

CPrivateChatFrame::CPrivateChatFrame()
{
	CRect rc( 0, 0, 0, 0 );
	Create( NULL, _T("Chat"), WS_CHILD|WS_CLIPCHILDREN, rc, AfxGetMainWnd(), 100 );
}

CPrivateChatFrame::~CPrivateChatFrame()
{
}

/////////////////////////////////////////////////////////////////////////////
// CPrivateChatFrame operations

void CPrivateChatFrame::Initiate(CGUID* pGUID, SOCKADDR_IN* pHost, BOOL bMustPush)
{
	ASSERT( m_pSession == NULL );
	
	m_pSession = new CChatSession( this );
	m_pSession->Setup( pGUID, pHost, bMustPush );
}

BOOL CPrivateChatFrame::Accept(CChatSession* pSession)
{
	if ( m_pSession != NULL )
	{
		if ( m_pSession->m_nState > cssConnecting ) return FALSE;
		m_pSession->OnCloseWindow();
	}
	
	m_pSession = pSession;
	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CPrivateChatFrame message handlers

int CPrivateChatFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CChatFrame::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	OnSkinChange();
	
	return 0;
}

void CPrivateChatFrame::OnSkinChange()
{
	CChatFrame::OnSkinChange();
	Skin.CreateToolBar( _T("CPrivateChatFrame"), &m_wndToolBar );
}

void CPrivateChatFrame::OnSize(UINT nType, int cx, int cy) 
{
	CChatFrame::OnSize( nType, cx, cy );
	
	CRect rc;
	GetClientRect( &rc );
	
	HDWP hDWP = BeginDeferWindowPos( 3 );
	
	DeferWindowPos( hDWP, m_wndView, NULL, rc.left, rc.top,
		rc.Width(), rc.Height() - TOOLBAR_HEIGHT - EDIT_HEIGHT, SWP_NOZORDER );
	
	DeferWindowPos( hDWP, m_wndToolBar, NULL,
		rc.left, rc.bottom - TOOLBAR_HEIGHT - EDIT_HEIGHT,
		rc.Width(), TOOLBAR_HEIGHT, SWP_NOZORDER );
	
	DeferWindowPos( hDWP, m_wndEdit, NULL, rc.left, rc.bottom - EDIT_HEIGHT,
		rc.Width(), EDIT_HEIGHT, SWP_NOZORDER );
	
	EndDeferWindowPos( hDWP );
}

void CPrivateChatFrame::OnPaint() 
{
	CPaintDC dc( this );
	CRect rc;
	
	GetClientRect( &rc );
}

void CPrivateChatFrame::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	Skin.TrackPopupMenu( _T("CPrivateChatFrame"), point );
}

/////////////////////////////////////////////////////////////////////////////
// CPrivateChatFrame event handlers

void CPrivateChatFrame::OnProfileReceived()
{
	CString str;
	
	LoadString( str, IDS_CHAT_PROFILE_ACCEPTED );
	m_pContent.Add( retText, str, NULL, retfColour )->m_cColour = RGB( 128, 128, 128 );
	
	m_sNick = m_pSession->m_sUserNick;
	m_pContent.Add( retLink, m_sNick, _T("raza:command:ID_CHAT_BROWSE") );
	
	SetWindowText( _T("Chat : ") + m_sNick );
	GetParent()->PostMessage( WM_TIMER, 2 );
	
	AddText( _T(".") );
	SetAlert();
}

void CPrivateChatFrame::OnRemoteMessage(BOOL bAction, LPCTSTR pszText)
{
	AddText( FALSE, bAction, m_sNick, pszText );
	SetAlert();
	PostMessage( WM_TIMER, 4 );
}

void CPrivateChatFrame::OnLocalMessage(BOOL bAction, LPCTSTR pszText)
{
	TRISTATE bConnected = m_pSession->GetConnectedState();
	
	if ( bConnected != TS_TRUE )
	{
		if ( bConnected == TS_FALSE )
		{
			m_pSession->StatusMessage( 1, IDS_CHAT_NOT_CONNECTED_1 );
			PostMessage( WM_COMMAND, ID_CHAT_CONNECT );
		}
		else
		{
			m_pSession->StatusMessage( 1, IDS_CHAT_NOT_CONNECTED_2 );
		}
		
		return;
	}
	
	AddText( TRUE, bAction, MyProfile.GetNick(), pszText );
	m_pSession->SendPrivateMessage( bAction, pszText );
}

void CPrivateChatFrame::OnLocalCommand(LPCTSTR pszCommand, LPCTSTR pszArgs)
{
	if ( _tcscmp( pszCommand, _T("/BROWSE") ) == 0 )
	{
		PostMessage( WM_COMMAND, ID_CHAT_BROWSE );
	}
	else
	{
		CChatFrame::OnLocalCommand( pszCommand, pszArgs );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPrivateChatFrame commands

void CPrivateChatFrame::OnUpdateChatBrowse(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_pSession != NULL );
}

void CPrivateChatFrame::OnChatBrowse() 
{
	if ( m_pSession != NULL )
	{
		new CBrowseHostWnd( &m_pSession->m_pHost,
			m_pSession->m_bGUID ? &m_pSession->m_pGUID : NULL );
	}
}

void CPrivateChatFrame::OnUpdateChatPriority(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_pSession != NULL && m_pSession->GetConnectedState() == TS_TRUE );
}

void CPrivateChatFrame::OnChatPriority() 
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 500 ) ) return;
	
	DWORD nAddress = m_pSession->m_pHost.sin_addr.S_un.S_addr;
	
	for ( POSITION pos = Uploads.GetIterator() ; pos ; )
	{
		CUploadTransfer* pUpload = Uploads.GetNext( pos );
		
		if (	pUpload->m_pHost.sin_addr.S_un.S_addr == nAddress &&
				pUpload->m_nState == upsQueued )
		{
			pUpload->Promote();
		}
	}
	
	m_pSession->StatusMessage( 2, IDS_CHAT_PRIORITY_GRANTED,
		(LPCTSTR)m_pSession->m_sAddress );
}
