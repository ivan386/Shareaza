//
// CtrlChatFrame.cpp
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
#include "CtrlChatFrame.h"
#include "RichElement.h"
#include "EDClients.h"
#include "EDPacket.h"
#include "ChatWindows.h"
#include "ChatCore.h"
#include "ChatSession.h"
#include "CoolInterface.h"
#include "Emoticons.h"
#include "WndChat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CChatFrame, CWnd)

BEGIN_MESSAGE_MAP(CChatFrame, CWnd)
	//{{AFX_MSG_MAP(CChatFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_UPDATE_COMMAND_UI(ID_CHAT_BOLD, OnUpdateChatBold)
	ON_COMMAND(ID_CHAT_BOLD, OnChatBold)
	ON_UPDATE_COMMAND_UI(ID_CHAT_ITALIC, OnUpdateChatItalic)
	ON_COMMAND(ID_CHAT_ITALIC, OnChatItalic)
	ON_UPDATE_COMMAND_UI(ID_CHAT_UNDERLINE, OnUpdateChatUnderline)
	ON_COMMAND(ID_CHAT_UNDERLINE, OnChatUnderline)
	ON_COMMAND(ID_CHAT_COLOUR, OnChatColour)
	ON_UPDATE_COMMAND_UI(ID_CHAT_CONNECT, OnUpdateChatConnect)
	ON_COMMAND(ID_CHAT_CONNECT, OnChatConnect)
	ON_UPDATE_COMMAND_UI(ID_CHAT_DISCONNECT, OnUpdateChatDisconnect)
	ON_COMMAND(ID_CHAT_DISCONNECT, OnChatDisconnect)
	ON_COMMAND(ID_CHAT_CLEAR, OnChatClear)
	ON_COMMAND(ID_CHAT_EMOTICONS, OnChatEmoticons)
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_WM_TIMER()
	ON_WM_SETFOCUS()
	ON_NOTIFY(RVN_CLICK, IDC_CHAT_TEXT, OnClickView)
	ON_UPDATE_COMMAND_UI(ID_CHAT_TIMESTAMP, OnUpdateChatTimestamp)
	ON_COMMAND(ID_CHAT_TIMESTAMP, OnChatTimestamp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#define EDIT_HISTORY	256
#define NEWLINE_FORMAT	_T("2")


/////////////////////////////////////////////////////////////////////////////
// CChatFrame construction

CChatFrame::CChatFrame()
{
	m_pSession		= NULL;
	m_pIconMenu		= NULL;
	m_pChildWnd		= NULL;
	m_pDesktopWnd	= NULL;
}

CChatFrame::~CChatFrame()
{
	ASSERT( m_pSession == NULL );
}

/////////////////////////////////////////////////////////////////////////////
// CChatFrame system message handlers

int CChatFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	CRect rc;
	GetClientRect( &rc );
	
	m_wndView.Create( WS_CHILD|WS_VISIBLE, rc, this, IDC_CHAT_TEXT );
	m_wndView.SetDocument( &m_pContent );
	m_wndView.SetSelectable( TRUE );
	m_wndView.SetFollowBottom( TRUE );
	
	m_pContent.m_szMargin = CSize( 8, 4 );
	
	m_wndToolBar.Create( this, WS_CHILD|WS_VISIBLE );
	m_wndToolBar.SetBarStyle( m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM );
	
	m_wndEdit.Create( WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_AUTOVSCROLL, rc, this, IDC_CHAT_EDIT );
	m_wndEdit.SetFont( &theApp.m_gdiFont );
	
	m_nHistory = 0;
	
	SetDesktopMode( FALSE );
	
	ChatWindows.Add( this );
	
	return 0;
}

void CChatFrame::OnDestroy() 
{
	CSingleLock pLock( &ChatCore.m_pSection, TRUE );
	
	if ( m_pSession != NULL )
	{
		m_pSession->OnCloseWindow();
		m_pSession = NULL;
	}
	
	ChatWindows.Remove( this );
	
	pLock.Unlock();
	
	CWnd::OnDestroy();
}

void CChatFrame::OnSkinChange()
{
}

/////////////////////////////////////////////////////////////////////////////
// CChatFrame desktop mode

void CChatFrame::SetDesktopMode(BOOL bDesktop)
{
	if ( bDesktop && m_pDesktopWnd != NULL ) return;
	if ( ! bDesktop && m_pChildWnd != NULL ) return;
	
	ShowWindow( SW_HIDE );
	SetParent( NULL );
	
	if ( m_pDesktopWnd != NULL )
	{
		// m_pDesktopWnd->m_pFrame = NULL;
		m_pDesktopWnd->DestroyWindow();
		m_pDesktopWnd = NULL;
	}
	
	if ( m_pChildWnd != NULL )
	{
		m_pChildWnd->m_pFrame = NULL;
		m_pChildWnd->DestroyWindow();
		m_pChildWnd = NULL;
	}
	
	if ( bDesktop )
	{
		// TODO: m_pDesktopWnd = new CChatDesktopWnd( this );
	}
	else
	{
		m_pChildWnd = new CChatWnd( this );
	}
}

void CChatFrame::SetAlert(BOOL /*bAlert*/)
{
	PostMessage( WM_TIMER, 1 );
}

/////////////////////////////////////////////////////////////////////////////
// CChatFrame text input controller

BOOL CChatFrame::PreTranslateMessage(MSG* pMsg) 
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( ! m_pSession ) return TRUE;

		if ( pMsg->wParam == VK_RETURN )
		{
			m_wndEdit.GetWindowText( m_sCurrent );
			if ( m_sCurrent.IsEmpty() ) return TRUE;
			
			m_sCurrent = m_sCurrent.SpanExcluding( _T("\r\n") );
			OnLocalText( m_sCurrent );
			
			m_pHistory.Add( m_sCurrent );
			if ( m_pHistory.GetSize() > EDIT_HISTORY ) m_pHistory.RemoveAt( 0 );
			m_nHistory = static_cast< int >( m_pHistory.GetSize() );
			
			m_sCurrent.Empty();
			m_wndEdit.SetWindowText( m_sCurrent );
			return TRUE;
		}
		else if ( pMsg->wParam == VK_ESCAPE )
		{
			m_wndEdit.SetWindowText( _T("") );
			m_nHistory = static_cast< int >( m_pHistory.GetSize() );
			m_sCurrent.Empty();
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
	
	return CWnd::PreTranslateMessage( pMsg );
}

void CChatFrame::MoveHistory(int nDelta)
{
	if ( m_nHistory == m_pHistory.GetSize() )
	{
		m_wndEdit.GetWindowText( m_sCurrent );
	}
	
	m_nHistory += nDelta;
	m_nHistory = (int)max( 0, min( m_pHistory.GetSize(), m_nHistory ) );
	
	if ( m_nHistory == m_pHistory.GetSize() )
	{
		m_wndEdit.SetWindowText( m_sCurrent );
	}
	else
	{
		m_wndEdit.SetWindowText( m_pHistory.GetAt( m_nHistory ) );
	}
	
	int nLen = m_wndEdit.GetWindowTextLength();
	m_wndEdit.SetSel( nLen, nLen );
}

BOOL CChatFrame::IsInRange(LPCTSTR pszToken)
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

void CChatFrame::InsertText(LPCTSTR pszToken)
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
// CChatFrame text view controller

void CChatFrame::AddText(LPCTSTR pszText)
{
	m_pContent.Add( retText, pszText );
	m_pContent.Add( retNewline, NEWLINE_FORMAT );
	m_wndView.InvalidateIfModified();
}

void CChatFrame::AddText(BOOL bSelf, BOOL bAction, LPCTSTR pszNick, LPCTSTR pszBody)
{
	CString str;
	
	if ( Settings.Community.Timestamp )
	{
		CTime tNow = CTime::GetCurrentTime();

		str.Format( _T("[%.2i:%.2i] "),
			tNow.GetHour(), tNow.GetMinute() );
		m_pContent.Add( retText, str, NULL, retfColour )->m_cColour = CoolInterface.m_crChatNull;
	}
	
	str.Format( bAction ? _T("* %s ") : _T("%s: "), pszNick );
	m_pContent.Add( retText, str, NULL, retfBold | retfColour )->m_cColour
		= ( bSelf ? CoolInterface.m_crChatOut : CoolInterface.m_crChatIn );
	
	Emoticons.FormatText( &m_pContent, pszBody );
	
	m_pContent.Add( retNewline, NEWLINE_FORMAT );
	m_wndView.InvalidateIfModified();
}

/////////////////////////////////////////////////////////////////////////////
// CChatFrame event handlers

void CChatFrame::OnStatusMessage(int nFlags, LPCTSTR pszText)
{
	m_pContent.Add( retText, pszText, NULL, retfColour )->m_cColour
		= nFlags == 1 ? CoolInterface.m_crChatOut : CoolInterface.m_crChatNull;
	m_pContent.Add( retNewline, NEWLINE_FORMAT );
	m_wndView.InvalidateIfModified();
}

void CChatFrame::OnLocalText(LPCTSTR pszText)
{
	if ( pszText == NULL || *pszText == 0 || ! m_pSession ) return;
	
	if ( *pszText == '/' )
	{
		CString strCommand = CString( pszText ).SpanExcluding( _T(" \t") );
		ToLower( strCommand );
		
		if ( strCommand == _T("/me") )
		{
			OnLocalMessage( TRUE, pszText + 4 );
		}
		else
		{
			OnLocalCommand( strCommand, pszText + strCommand.GetLength() + 1 );
		}
	}
	else
	{
		OnLocalMessage( FALSE, pszText );
	}
}

void CChatFrame::OnLocalMessage(BOOL /*bAction*/, LPCTSTR /*pszText*/)
{
}

void CChatFrame::OnLocalCommand(LPCTSTR pszCommand, LPCTSTR /*pszArgs*/)
{
	if ( _tcsicmp( pszCommand, _T("/clear") ) == 0 )
	{
		PostMessage( WM_COMMAND, ID_CHAT_CLEAR );
	}
	else if ( _tcsicmp( pszCommand, _T("/connect") ) == 0 )
	{
		PostMessage( WM_COMMAND, ID_CHAT_CONNECT );
	}
	else if ( _tcsicmp( pszCommand, _T("/disconnect") ) == 0 )
	{
		PostMessage( WM_COMMAND, ID_CHAT_DISCONNECT );
	}
	else if ( _tcsicmp( pszCommand, _T("/close") ) == 0 )
	{
		GetParent()->PostMessage( WM_CLOSE );
	}
	else if ( _tcsicmp( pszCommand, _T("/exit") ) == 0 )
	{
		GetParent()->PostMessage( WM_CLOSE );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CChatFrame command handlers

void CChatFrame::OnUpdateChatBold(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( IsInRange( _T("b") ) );
}

void CChatFrame::OnChatBold() 
{
	if ( ! m_pSession ) return;

	if ( IsInRange( _T("b") ) )
		InsertText( _T("[/b]") );
	else
		InsertText( _T("[b]") );
}

void CChatFrame::OnUpdateChatItalic(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( IsInRange( _T("i") ) );
}

void CChatFrame::OnChatItalic() 
{
	if ( ! m_pSession ) return;

	if ( IsInRange( _T("i") ) )
		InsertText( _T("[/i]") );
	else
		InsertText( _T("[i]") );
}

void CChatFrame::OnUpdateChatUnderline(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( IsInRange( _T("u") ) );
}

void CChatFrame::OnChatUnderline() 
{
	if ( ! m_pSession ) return;

	if ( IsInRange( _T("u") ) )
		InsertText( _T("[/u]") );
	else
		InsertText( _T("[u]") );
}

void CChatFrame::OnChatColour() 
{
	if ( ! m_pSession ) return;

	CColorDialog dlg( 0, CC_ANYCOLOR | CC_FULLOPEN );
	if ( dlg.DoModal() != IDOK ) return;
	
	COLORREF cr = dlg.GetColor();
	CString str;
	
	str.Format( _T("[c:#%.2x%.2x%.2x]"), GetRValue( cr ), GetGValue( cr ), GetBValue( cr ) );
	InsertText( str );
}

void CChatFrame::OnChatEmoticons() 
{
	if ( ! m_pSession ) return;

	m_pIconMenu = Emoticons.CreateMenu();
	
	UINT nID = m_wndToolBar.ThrowMenu( ID_CHAT_EMOTICONS, m_pIconMenu, this, TRUE );
	
	delete m_pIconMenu;
	m_pIconMenu = NULL;
	
	if ( nID == 0 ) return;
	
	LPCTSTR pszToken = Emoticons.GetText( nID - 1 );
	if ( pszToken != NULL ) InsertText( pszToken );
}

void CChatFrame::OnChatClear() 
{
	m_pContent.Clear();
	m_wndView.InvalidateIfModified();
}

void CChatFrame::OnUpdateChatTimestamp(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( Settings.Community.Timestamp );
}

void CChatFrame::OnChatTimestamp() 
{
	Settings.Community.Timestamp = ! Settings.Community.Timestamp;
}

void CChatFrame::OnUpdateChatConnect(CCmdUI* pCmdUI) 
{
	BOOL bState = ( ( m_pSession != NULL ) && 
					( m_pSession->GetConnectedState() == TRI_FALSE )  &&
					( m_pSession->m_nProtocol != PROTOCOL_ED2K ) );
	if ( CCoolBarItem* pItem = CCoolBarItem::FromCmdUI( pCmdUI ) ) pItem->Show( bState );
	pCmdUI->Enable( bState );
}

void CChatFrame::OnChatConnect() 
{
	if ( m_pSession != NULL && m_pSession->GetConnectedState() == TRI_FALSE )
	{
		CWnd* pParent = GetParent();
		if ( pParent->IsIconic() ) pParent->ShowWindow( SW_SHOWNORMAL );
		pParent->BringWindowToTop();
		pParent->SetForegroundWindow();
		m_pSession->Connect();
	}
}

void CChatFrame::OnUpdateChatDisconnect(CCmdUI* pCmdUI) 
{
	BOOL bState = ( m_pSession != NULL ) && 
				  ( m_pSession->GetConnectedState() != TRI_FALSE ) &&
				  ( m_pSession->m_nProtocol != PROTOCOL_ED2K );
	if ( CCoolBarItem* pItem = CCoolBarItem::FromCmdUI( pCmdUI ) ) pItem->Show( bState );
	pCmdUI->Enable( bState );
}

void CChatFrame::OnChatDisconnect() 
{
	if ( m_pSession != NULL ) m_pSession->Close();
}

/////////////////////////////////////////////////////////////////////////////
// CChatFrame message handlers

void CChatFrame::OnTimer(UINT_PTR nIDEvent) 
{
	if ( nIDEvent == 1 )
	{
		if ( m_pChildWnd != NULL ) m_pChildWnd->SetAlert();
		// if ( m_pDesktopWnd != NULL ) m_pDesktopWnd->SetAlert();
	}
	else if ( nIDEvent == 4 )
	{
		if ( GetForegroundWindow() != GetTopLevelParent() )
		{
			CWnd* pParentWnd = GetTopLevelParent();

			if ( theApp.m_hUser32 != 0 )
			{
				BOOL (WINAPI *pfnFlashWindowEx)(PFLASHWINFO pfwi);
				
				(FARPROC&)pfnFlashWindowEx = GetProcAddress( theApp.m_hUser32, "FlashWindowEx" );
				if ( pfnFlashWindowEx )
				{
					FLASHWINFO pFWX;
					
					pFWX.cbSize		= sizeof(pFWX);
					pFWX.dwFlags	= FLASHW_ALL | FLASHW_TIMERNOFG;
					pFWX.uCount		= 3;
					pFWX.dwTimeout	= 0;
					pFWX.hwnd		= pParentWnd->GetSafeHwnd();
					
					(*pfnFlashWindowEx)( &pFWX );
				}
			}
		}
	}
}

void CChatFrame::OnSetFocus(CWnd* pOldWnd) 
{
	CWnd::OnSetFocus( pOldWnd );
	m_wndEdit.SetFocus();
}

void CChatFrame::OnMeasureItem(int /*nIDCtl*/, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	lpMeasureItemStruct->itemWidth	= 20;
	lpMeasureItemStruct->itemHeight	= 22;
}

void CChatFrame::OnDrawItem(int /*nIDCtl*/, LPDRAWITEMSTRUCT lpDrawItemStruct) 
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

void CChatFrame::OnClickView(NMHDR* pNotify, LRESULT* /*pResult*/)
{
	if ( CRichElement* pElement = ((RVN_ELEMENTEVENT*) pNotify)->pElement )
	{
		theApp.InternalURI( pElement->m_sLink );
	}
}
