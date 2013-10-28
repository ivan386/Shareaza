//
// WndChat.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2011.
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
#include "ChatWindows.h"
#include "CoolInterface.h"
#include "EDClients.h"
#include "EDPacket.h"
#include "Emoticons.h"
#include "GProfile.h"
#include "Plugins.h"
#include "Security.h"
#include "WndChat.h"
#include "WndMain.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CChatWnd, CPanelWnd)

BEGIN_MESSAGE_MAP(CChatWnd, CPanelWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_CONTEXTMENU()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_WM_SETFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_UPDATE_COMMAND_UI(ID_CHAT_BOLD, &CChatWnd::OnUpdateChatBold)
	ON_COMMAND(ID_CHAT_BOLD, &CChatWnd::OnChatBold)
	ON_UPDATE_COMMAND_UI(ID_CHAT_ITALIC, &CChatWnd::OnUpdateChatItalic)
	ON_COMMAND(ID_CHAT_ITALIC, &CChatWnd::OnChatItalic)
	ON_UPDATE_COMMAND_UI(ID_CHAT_UNDERLINE, &CChatWnd::OnUpdateChatUnderline)
	ON_COMMAND(ID_CHAT_UNDERLINE, &CChatWnd::OnChatUnderline)
	ON_COMMAND(ID_CHAT_COLOUR, &CChatWnd::OnChatColour)
	ON_COMMAND(ID_CHAT_CLEAR, &CChatWnd::OnChatClear)
	ON_COMMAND(ID_CHAT_EMOTICONS, &CChatWnd::OnChatEmoticons)
	ON_NOTIFY(RVN_CLICK, IDC_CHAT_TEXT, &CChatWnd::OnClickView)
	ON_UPDATE_COMMAND_UI(ID_CHAT_TIMESTAMP, &CChatWnd::OnUpdateChatTimestamp)
	ON_COMMAND(ID_CHAT_TIMESTAMP, &CChatWnd::OnChatTimestamp)
	ON_MESSAGE(WM_CHAT_MESSAGE, &CChatWnd::OnChatMessage)
	ON_MESSAGE(WM_CHAT_ADD_USER, &CChatWnd::OnChatAddUser)
	ON_MESSAGE(WM_CHAT_DELETE_USER, &CChatWnd::OnChatDeleteUser)
	ON_COMMAND_RANGE(1, 200, &CChatWnd::OnEmoticons)
END_MESSAGE_MAP()

#define NEWLINE_FORMAT	_T("2")
#define EDIT_HISTORY	256
#define EDIT_HEIGHT		32
#define TOOLBAR_HEIGHT	30
#define SPLIT_SIZE		6

/////////////////////////////////////////////////////////////////////////////
// CChatWnd construction

CChatWnd::CChatWnd()
	: m_nHistory	( 0 )
	, m_nUsersSize	( Settings.Community.UserPanelSize )
{
}

CChatWnd::~CChatWnd()
{
}

BOOL CChatWnd::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( pMsg->wParam == VK_RETURN )
		{
			CString sCurrent;
			m_wndEdit.GetWindowText( sCurrent );

			for ( ; sCurrent.GetLength(); sCurrent = sCurrent.Mid( 1 ) )
			{
				CString strLine = sCurrent.SpanExcluding( _T("\r\n") );
				if ( strLine.GetLength() )
				{
					if ( ! OnLocalText( strLine ) )
						// Leave rest of string in edit box
						break;
					sCurrent = sCurrent.Mid( strLine.GetLength() );
				}
			}

			m_wndEdit.SetWindowText( sCurrent );
			return TRUE;
		}
		else if ( pMsg->wParam == VK_ESCAPE )
		{
			m_wndEdit.SetWindowText( _T("") );
			m_nHistory = static_cast< int >( m_pHistory.GetSize() );
			return TRUE;
		}
		else if ( pMsg->wParam == VK_UP )
		{
			MoveHistory( -1 );
			return TRUE;
		}
		else if ( pMsg->wParam == VK_DOWN )
		{
			MoveHistory( 1 );
			return TRUE;
		}
		else if ( pMsg->wParam == VK_PRIOR )
		{
			m_wndView.PostMessage( WM_VSCROLL, MAKELONG( SB_PAGEUP, 0 ), NULL );
			return TRUE;
		}
		else if ( pMsg->wParam == VK_NEXT )
		{
			m_wndView.PostMessage( WM_VSCROLL, MAKELONG( SB_PAGEDOWN, 0 ), NULL );
			return TRUE;
		}
		else if ( pMsg->wParam == VK_HOME && ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) )
		{
			m_wndView.PostMessage( WM_VSCROLL, MAKELONG( SB_TOP, 0 ), NULL );
			return TRUE;
		}
		else if ( pMsg->wParam == VK_END && ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) )
		{
			m_wndView.PostMessage( WM_VSCROLL, MAKELONG( SB_BOTTOM, 0 ), NULL );
			return TRUE;
		}
		else if ( pMsg->wParam == 'B' && ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) )
		{
			OnChatBold();
			return TRUE;
		}
		else if ( pMsg->wParam == 'I' && ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) )
		{
			OnChatItalic();
			return TRUE;
		}
		else if ( pMsg->wParam == 'U' && ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) )
		{
			OnChatUnderline();
			return TRUE;
		}
		else if ( pMsg->wParam == 'K' && ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) )
		{
			OnChatColour();
			return TRUE;
		}
	}

	return CPanelWnd::PreTranslateMessage( pMsg );
}

void CChatWnd::MoveHistory(int nDelta)
{
	CString sCurrent;
	if ( m_nHistory == m_pHistory.GetSize() )
	{
		m_wndEdit.GetWindowText( sCurrent );
	}

	m_nHistory += nDelta;
	m_nHistory = max( 0, min( (int)m_pHistory.GetSize(), m_nHistory ) );

	if ( m_nHistory == m_pHistory.GetSize() )
	{
		m_wndEdit.SetWindowText( sCurrent );
	}
	else
	{
		m_wndEdit.SetWindowText( m_pHistory.GetAt( m_nHistory ) );
	}

	int nLen = m_wndEdit.GetWindowTextLength();
	m_wndEdit.SetSel( nLen, nLen );
}

BOOL CChatWnd::IsInRange(LPCTSTR pszToken)
{
	CString strRange, strToken;
	int nStart, nEnd;

	m_wndEdit.GetSel( nStart, nEnd );
	if ( nStart != nEnd ) return FALSE;

	m_wndEdit.GetWindowText( strRange );
	if ( nStart <= 0 ) return FALSE;
	if ( nStart < strRange.GetLength() ) strRange = strRange.Left( nStart );

	ToLower( strRange );
	strRange.MakeReverse();
	strToken.Format( _T("]%s["), pszToken );
	nStart = strRange.Find( strToken );
	strToken.Format( _T("]%s/["), pszToken );
	nEnd = strRange.Find( strToken );

	if ( nStart < 0 ) return FALSE;
	if ( nEnd < 0 ) return TRUE;

	return ( nEnd > nStart );
}

void CChatWnd::InsertText(LPCTSTR pszToken)
{
	int nStart, nEnd;
	m_wndEdit.GetSel( nStart, nEnd );

	if ( nStart == nEnd )
	{
		m_wndEdit.ReplaceSel( pszToken );
	}
	else
	{
		CString strIn, strOut;
		m_wndEdit.GetWindowText( strIn );
		if ( nEnd < nStart ) m_wndEdit.GetSel( nEnd, nStart );
		strOut.Format( _T("%s%s[/%c]"), pszToken,
			(LPCTSTR)strIn.Mid( nStart, nEnd - nStart ), pszToken[1] );
		m_wndEdit.ReplaceSel( strOut );
	}

	m_wndEdit.SetFocus();
}

/////////////////////////////////////////////////////////////////////////////
// CChatWnd text view controller

void CChatWnd::AddTimestamp()
{
	if ( Settings.Community.Timestamp )
	{
		CTime tNow = CTime::GetCurrentTime();
		CString str;
		str.Format( _T("[%.2i:%.2i:%.2i] "), tNow.GetHour(), tNow.GetMinute(), tNow.GetSecond() );
		m_pContent.Add( retText, str, NULL, retfColour )->m_cColour = CoolInterface.m_crChatNull;
	}
}

void CChatWnd::AddLogin(LPCTSTR pszText)
{
	AddTimestamp();

	m_pContent.Add( retText, LoadString( IDS_CHAT_PROFILE_ACCEPTED ), NULL, retfColour )->m_cColour = CoolInterface.m_crChatNull ;
	m_pContent.Add( retLink, pszText, _T("raza:command:ID_CHAT_BROWSE") );
	m_pContent.Add( retNewline, NEWLINE_FORMAT );
	m_wndView.InvalidateIfModified();
}

void CChatWnd::AddBitmap(HBITMAP hBitmap)
{
	m_pContent.Add( hBitmap );
	m_pContent.Add( retNewline, NEWLINE_FORMAT );
	m_wndView.InvalidateIfModified();
}

void CChatWnd::AddText(bool bAction, bool bOutgoing, LPCTSTR pszNick, LPCTSTR pszBody)
{
	AddTimestamp();

	CString str;
	str.Format( bAction ? _T("* %s ") : _T("<%s> "), pszNick );
	m_pContent.Add( retText, str, NULL, retfBold | retfColour )->m_cColour
		= ( bOutgoing ? CoolInterface.m_crChatIn : CoolInterface.m_crChatOut );

	Emoticons.FormatText( &m_pContent, pszBody, TRUE );

	m_pContent.Add( retNewline, NEWLINE_FORMAT );
	m_wndView.InvalidateIfModified();
}

void CChatWnd::Open()
{
	if ( IsIconic() ) ShowWindow( SW_SHOWNORMAL );
	BringWindowToTop();
	SetForegroundWindow();
}

void CChatWnd::OnSkinChange()
{
	CPanelWnd::OnSkinChange();

	Skin.CreateToolBar( CString( GetRuntimeClass()->m_lpszClassName ), &m_wndToolBar );
}

/////////////////////////////////////////////////////////////////////////////
// CChatWnd event handlers

void CChatWnd::OnStatusMessage(int nFlags, const CString& sText)
{
	if ( sText.IsEmpty() )
		return;

	AddTimestamp();

	m_pContent.Add( retText, sText, NULL, retfColour )->m_cColour
		= ( nFlags == 1 ) ? CoolInterface.m_crChatOut : CoolInterface.m_crChatNull;
	m_pContent.Add( retNewline, NEWLINE_FORMAT );
	m_wndView.InvalidateIfModified();
}

BOOL CChatWnd::OnLocalText(const CString& sText)
{
	// Save history
	m_pHistory.Add( sText );
	if ( m_pHistory.GetSize() > EDIT_HISTORY ) m_pHistory.RemoveAt( 0 );
	m_nHistory = static_cast< int >( m_pHistory.GetSize() );

	if ( sText.GetAt( 0 ) == _T('/') )
	{
		CString strCommand = sText.SpanExcluding( _T(" \t") ).Trim();
		if ( strCommand.CompareNoCase( _T("/me") ) == 0 )
		{
			// Action text
			return OnLocalMessage( true, sText.Mid( 4 ) );
		}
		else if ( OnLocalCommand( strCommand, sText.Mid( strCommand.GetLength() + 1 ).Trim() ) )
		{
			// Handled command 
			return TRUE;
		}
	}
	else if ( sText.GetAt( 0 ) == _T('*') &&
		( sText.GetAt( 1 ) == _T(' ') || sText.GetAt( 1 ) == _T('\t') ) )
	{
		// Action text
		return OnLocalMessage( true, sText.Mid( 2 ) );
	}

	// Regular text
	return OnLocalMessage( false, sText );
}

BOOL CChatWnd::OnLocalCommand(const CString& sCommand, const CString& /*sArgs*/)
{
	if ( sCommand.CompareNoCase( _T("/clear") ) == 0 )
	{
		PostMessage( WM_COMMAND, ID_CHAT_CLEAR );
	}
	else if ( sCommand.CompareNoCase( _T("/close") ) == 0 ||
			  sCommand.CompareNoCase( _T("/exit")  ) == 0 ||
			  sCommand.CompareNoCase( _T("/quit")  ) == 0 )
	{
		PostMessage( WM_CLOSE );
	}
	else
	{
		// Not a command
		return FALSE;
	}
	return TRUE;
}

void CChatWnd::OnMessage(bool bAction, const CString& sChatID, bool bOutgoing, const CString& sFrom, const CString& sTo, const CString& sText)
{
	// Check incoming message spam filter (if enabled)
	if ( ! bOutgoing && MessageFilter.IsFiltered( sText ) )
		return;

	// Notify chat plugins about new message
	Plugins.OnChatMessage( sChatID, bOutgoing, sFrom, sTo, sText );

	// Adult filter (if enabled)
	CString sCensoredText( sText );
	if ( Settings.Community.ChatCensor )
	{
		AdultFilter.Censor( sCensoredText );
	}
	AddText( bAction, bOutgoing, sFrom, sCensoredText );

	SetAlert();

	if ( CMainWnd* pWnd = theApp.CShareazaApp::SafeMainWnd() )
	{
		if ( GetForegroundWindow() != pWnd )
		{
			// pWnd->ShowTrayPopup( sText, sFrom, NIIF_NONE, 30 );

			FLASHWINFO pFWX =
			{
				sizeof( pFWX ),
				pWnd->GetSafeHwnd(),
				FLASHW_ALL | FLASHW_TIMERNOFG,
				1000
			};
			::FlashWindowEx( &pFWX );
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CChatWnd message handlers

int CChatWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	m_gdiImageList.Create( 16, 16, ILC_COLOR32|ILC_MASK, 2, 0 ) ||
	m_gdiImageList.Create( 16, 16, ILC_COLOR24|ILC_MASK, 2, 0 ) ||
	m_gdiImageList.Create( 16, 16, ILC_COLOR16|ILC_MASK, 2, 0 );
	AddIcon( IDI_USER_ME, m_gdiImageList );
	AddIcon( IDI_USER, m_gdiImageList );

	CRect rc( 0, 0, 0, 0 );

	if ( ! m_wndView.Create( WS_CHILD | WS_VISIBLE| WS_CLIPSIBLINGS | WS_VSCROLL, rc, this, IDC_CHAT_TEXT ) ) return -1;
	m_wndView.SetDocument( &m_pContent );
	m_wndView.SetSelectable( TRUE );
	m_wndView.SetFollowBottom( TRUE );

	if ( ! m_wndUsers.Create( WS_CHILD | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE | WS_VSCROLL | LVS_SINGLESEL | LVS_REPORT | LVS_NOCOLUMNHEADER | LVS_SORTASCENDING | LVS_NOLABELWRAP | WS_CLIPSIBLINGS, rc, this, IDC_CHAT_USERS ) )  return -1;
	m_wndUsers.SetExtendedStyle( m_wndUsers.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER );
	m_wndUsers.SetFont( &theApp.m_gdiFont );
	m_wndUsers.InsertColumn( 0, _T("Name") );
	m_wndUsers.SetImageList( &m_gdiImageList, LVSIL_SMALL );

	if ( ! m_wndToolBar.Create( this, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | CBRS_NOALIGN, AFX_IDW_TOOLBAR ) ) return -1;
	m_wndToolBar.SetBarStyle( m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM );

	if ( ! m_wndEdit.Create( WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_GROUP | WS_CLIPSIBLINGS | ES_MULTILINE | ES_AUTOVSCROLL, rc, this, IDC_CHAT_EDIT ) ) return -1;
	m_wndEdit.SetFont( &theApp.m_gdiFont );

	m_pContent.m_szMargin = CSize( 8, 4 );

	ChatWindows.Add( this );

	LoadState( _T("CChatWnd"), TRUE );

	return 0;
}

void CChatWnd::OnDestroy()
{
	DeleteAllUsers();

	Settings.Community.UserPanelSize = m_nUsersSize;

	SaveState( _T("CChatWnd") );

	ChatWindows.Remove( this );

	CPanelWnd::OnDestroy();
}

void CChatWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	Skin.TrackPopupMenu( CString( GetRuntimeClass()->m_lpszClassName ), point );
}

BOOL CChatWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	return CPanelWnd::OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );
}

void CChatWnd::OnSize(UINT nType, int cx, int cy)
{
	if ( nType != 1982 ) CPanelWnd::OnSize( nType, cx, cy );

	CRect rc;
	GetClientRect( &rc );

	if ( rc.Width() < m_nUsersSize + SPLIT_SIZE )
	{
		m_nUsersSize = max( 0, rc.Width() - SPLIT_SIZE );
	}

	HDWP hDWP = BeginDeferWindowPos( 4 );

	DeferWindowPos( hDWP, m_wndView, NULL, rc.left, rc.top,
		rc.Width() - m_nUsersSize - SPLIT_SIZE, rc.Height() - TOOLBAR_HEIGHT - EDIT_HEIGHT, SWP_NOZORDER );

	DeferWindowPos( hDWP, m_wndUsers, NULL, rc.left + rc.Width() - m_nUsersSize, rc.top,
		m_nUsersSize, rc.Height() - TOOLBAR_HEIGHT - EDIT_HEIGHT, SWP_NOZORDER );

	DeferWindowPos( hDWP, m_wndToolBar, NULL,
		rc.left, rc.bottom - TOOLBAR_HEIGHT - EDIT_HEIGHT,
		rc.Width(), TOOLBAR_HEIGHT, SWP_NOZORDER );

	DeferWindowPos( hDWP, m_wndEdit, NULL, rc.left, rc.bottom - EDIT_HEIGHT,
		rc.Width(), EDIT_HEIGHT, SWP_NOZORDER );

	EndDeferWindowPos( hDWP );

	m_wndUsers.SetColumnWidth( 0, m_nUsersSize - GetSystemMetrics( SM_CXVSCROLL ) );
}

void CChatWnd::OnPaint()
{
	CPaintDC dc( this );

	CRect rcClient;
	GetClientRect( &rcClient );

	CRect rc( rcClient.right - m_nUsersSize - SPLIT_SIZE,
			  rcClient.top,
			  rcClient.right - m_nUsersSize,
			  rcClient.bottom - TOOLBAR_HEIGHT - EDIT_HEIGHT );

	dc.FillSolidRect( rc.left, rc.top, 1, rc.Height(), CoolInterface.m_crResizebarEdge );
	dc.FillSolidRect( rc.left + 1, rc.top, 1, rc.Height(), CoolInterface.m_crResizebarHighlight );
	dc.FillSolidRect( rc.right - 1, rc.top, 1, rc.Height(), CoolInterface.m_crResizebarShadow );
	dc.FillSolidRect( rc.left + 2, rc.top, rc.Width() - 3, rc.Height(),	CoolInterface.m_crResizebarFace );
}

void CChatWnd::OnUpdateChatBold(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( IsInRange( _T("b") ) );
}

void CChatWnd::OnChatBold()
{
	if ( IsInRange( _T("b") ) )
		InsertText( _T("[/b]") );
	else
		InsertText( _T("[b]") );
}

void CChatWnd::OnUpdateChatItalic(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( IsInRange( _T("i") ) );
}

void CChatWnd::OnChatItalic()
{
	if ( IsInRange( _T("i") ) )
		InsertText( _T("[/i]") );
	else
		InsertText( _T("[i]") );
}

void CChatWnd::OnUpdateChatUnderline(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( IsInRange( _T("u") ) );
}

void CChatWnd::OnChatUnderline()
{
	if ( IsInRange( _T("u") ) )
		InsertText( _T("[/u]") );
	else
		InsertText( _T("[u]") );
}

void CChatWnd::OnChatColour()
{
	CColorDialog dlg( 0, CC_ANYCOLOR | CC_FULLOPEN );
	if ( dlg.DoModal() != IDOK ) return;

	COLORREF cr = dlg.GetColor();
	CString str;

	str.Format( _T("[c:#%.2x%.2x%.2x]"), GetRValue( cr ), GetGValue( cr ), GetBValue( cr ) );
	InsertText( str );
}

void CChatWnd::OnChatEmoticons()
{
	CAutoPtr< CMenu > pIconMenu( Emoticons.CreateMenu() );
	if ( UINT nID = m_wndToolBar.ThrowMenu( ID_CHAT_EMOTICONS, pIconMenu, this, TRUE ) )
	{
		if ( LPCTSTR pszToken = Emoticons.GetText( nID - 1 ) )
			InsertText( CString( _T(" ") ) + pszToken + _T(" ") );
	}
}

void CChatWnd::OnChatClear()
{
	m_pContent.Clear();
	m_wndView.InvalidateIfModified();
}

void CChatWnd::OnUpdateChatTimestamp(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.Community.Timestamp );
}

void CChatWnd::OnChatTimestamp()
{
	Settings.Community.Timestamp = ! Settings.Community.Timestamp;
}

/////////////////////////////////////////////////////////////////////////////
// CChatWnd message handlers

void CChatWnd::OnSetFocus(CWnd* pOldWnd)
{
	CPanelWnd::OnSetFocus( pOldWnd );

	m_wndEdit.SetFocus();
}

void CChatWnd::OnMeasureItem(int /*nIDCtl*/, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	lpMeasureItemStruct->itemWidth	= 20;
	lpMeasureItemStruct->itemHeight	= 22;
}

void CChatWnd::OnDrawItem(int /*nIDCtl*/, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC* pDC = CDC::FromHandle( lpDrawItemStruct->hDC );
	CRect rc( &lpDrawItemStruct->rcItem );

	Emoticons.Draw( pDC, lpDrawItemStruct->itemID - 1, rc.left + 8, rc.top + 3 );

	if ( lpDrawItemStruct->itemState & ODS_SELECTED )
	{
		pDC->Draw3dRect( &rc, CoolInterface.m_crHighlight, CoolInterface.m_crHighlight );
		rc.DeflateRect( 1, 1 );
		pDC->Draw3dRect( &rc, CoolInterface.m_crHighlight, CoolInterface.m_crHighlight );
	}
	else
	{
		pDC->Draw3dRect( &rc, CoolInterface.m_crWindow, CoolInterface.m_crWindow );
		rc.DeflateRect( 1, 1 );
		pDC->Draw3dRect( &rc, CoolInterface.m_crWindow, CoolInterface.m_crWindow );
	}
}

void CChatWnd::OnClickView(NMHDR* pNotify, LRESULT* /*pResult*/)
{
	if ( CRichElement* pElement = ((RVN_ELEMENTEVENT*) pNotify)->pElement )
	{
		theApp.InternalURI( pElement->m_sLink );
	}
}

void CChatWnd::OnEmoticons(UINT /*nID*/)
{
	// Used to enable emoticons menu items
}

BOOL CChatWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint point;
	GetCursorPos( &point );

	CRect rcClient;
	GetClientRect( &rcClient );
	ClientToScreen( &rcClient );

	CRect rc( rcClient.right - m_nUsersSize - SPLIT_SIZE,
		rcClient.top,
		rcClient.right - m_nUsersSize,
		rcClient.bottom - TOOLBAR_HEIGHT - EDIT_HEIGHT );

	if ( rc.PtInRect( point ) )
	{
		SetCursor( AfxGetApp()->LoadStandardCursor( IDC_SIZEWE ) );
		return TRUE;
	}

	return CPanelWnd::OnSetCursor( pWnd, nHitTest, message );
}

void CChatWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rcClient;
	GetClientRect( &rcClient );

	CRect rc( rcClient.right - m_nUsersSize - SPLIT_SIZE,
		rcClient.top,
		rcClient.right - m_nUsersSize,
		rcClient.bottom - TOOLBAR_HEIGHT - EDIT_HEIGHT );

	if ( rc.PtInRect( point ) )
	{
		DoSizeView();
		return;
	}

	CPanelWnd::OnLButtonDown( nFlags, point );
}

BOOL CChatWnd::DoSizeView()
{
	MSG* pMsg = &AfxGetThreadState()->m_msgCur;

	CRect rcClient;
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

		CPoint point;
		GetCursorPos( &point );
		ScreenToClient( &point );
		
		int nSplit = rcClient.right - point.x;

		if ( nOffset == 0xFFFF ) nOffset = m_nUsersSize - nSplit;
		nSplit += nOffset;

		nSplit = max( nSplit, 0 );
		nSplit = min( nSplit, (int)rcClient.right - SPLIT_SIZE );

		if ( nSplit < 8 )
			nSplit = 0;
		if ( nSplit > rcClient.right - SPLIT_SIZE - 8 )
			nSplit = rcClient.right - SPLIT_SIZE;

		if ( nSplit != m_nUsersSize )
		{
			m_nUsersSize = nSplit;
			OnSize( 1982, 0, 0 );
			Invalidate();
		}
	}

	ReleaseCapture();
	ClipCursor( NULL );

	return TRUE;
}

LRESULT CChatWnd::OnChatMessage(WPARAM /*wParam*/, LPARAM lParam)
{
	CAutoPtr< CChatMessage > pMsg( (CChatMessage*)lParam );

	if ( pMsg->m_hBitmap )
	{
		CChatWnd::AddBitmap( pMsg->m_hBitmap );
	}

	switch ( pMsg->m_bType )
	{
	case cmtProfile:
		Open();
		SetAlert();
		CChatWnd::AddLogin( pMsg->m_sFrom );
		break;

	case cmtError:
		SetAlert();
	case cmtStatus:
	case cmtInfo:
		CChatWnd::OnStatusMessage( (int)pMsg->m_bType - (int)cmtStatus, pMsg->m_sMessage );
		break;

	case cmtMessage:
		CChatWnd::OnMessage( false, GetChatID(), false, pMsg->m_sFrom, MyProfile.GetNick(), pMsg->m_sMessage );
		break;

	case cmtAction:
		CChatWnd::OnMessage( false, GetChatID(), false, pMsg->m_sFrom, MyProfile.GetNick(), pMsg->m_sMessage );
		break;

	case cmtCaption:
		m_sCaption = _T(" : ") + pMsg->m_sMessage;
		break;

	default:
		;
	}

	SetWindowText( GetCaption() + m_sCaption );

	return 0;
}

LRESULT CChatWnd::OnChatAddUser(WPARAM /*wParam*/, LPARAM lParam)
{
	CAutoPtr< CChatUser > pUser( (CChatUser*)lParam );

	LVFINDINFO lvfi = { LVFI_STRING, pUser->m_sNick };
	int index = -1;
	for (;;)
	{
		index = m_wndUsers.FindItem( &lvfi, index );
		if ( index == -1 )
			break;

		CChatUser* pCurrent = (CChatUser*)m_wndUsers.GetItemData( index );
		if ( pCurrent->m_bType == pUser->m_bType )
		{
			// Update existing user
			pCurrent->m_sDescription = pUser->m_sDescription;

			// TODO: Other user properties

			return 0;
		}
	}

	// New User
	int i = m_wndUsers.InsertItem( 0, pUser->m_sNick, pUser->m_bType );
	m_wndUsers.SetItemData( i, (DWORD_PTR)pUser.Detach() );

	return 0;
}

LRESULT CChatWnd::OnChatDeleteUser(WPARAM /*wParam*/, LPARAM lParam)
{
	CAutoPtr< CString > psNick( (CString*)lParam );

	if ( ! psNick )
	{
		DeleteAllUsers();
		return 0;
	}

	LVFINDINFO lvfi = { LVFI_STRING, *psNick };
	int index = -1;
	for (;;)
	{
		index = m_wndUsers.FindItem( &lvfi, index );
		if ( index == -1 )
			break;

		CChatUser* pCurrent = (CChatUser*)m_wndUsers.GetItemData( index );
		if ( pCurrent->m_bType != cutMe ) // Except me
		{
			delete pCurrent;
			m_wndUsers.DeleteItem( index );
		}
	}

	return 0;
}

void CChatWnd::DeleteAllUsers()
{
	int nCount = m_wndUsers.GetItemCount();
	for ( int i = 0; i < nCount; ++i )
	{
		delete (CChatUser*)m_wndUsers.GetItemData( i );
	}
	m_wndUsers.DeleteAllItems();
}
