//
// CtrlIRCFrame.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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
#include "ShareazaURL.h"
#include "Network.h"
#include "Skin.h"
#include "RichElement.h"
#include "RichFragment.h"
#include "CtrlIRCFrame.h"
#include "DlgSettingsManager.h"
#include "CoolInterface.h"
#include "Buffer.h"
#include "WndMain.h"
#include "DlgIrcInput.h"
#include "GProfile.h"
#include "Plugins.h"
#include "Emoticons.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CIRCFrame, CWnd)

BEGIN_MESSAGE_MAP(CIRCFrame, CWnd)
	ON_COMMAND(ID_IRC_CONNECT, OnIrcConnect)
	ON_UPDATE_COMMAND_UI(ID_IRC_CONNECT, OnUpdateIrcConnect)
	ON_COMMAND(ID_IRC_DISCONNECT, OnIrcDisconnect)
	ON_UPDATE_COMMAND_UI(ID_IRC_DISCONNECT, OnUpdateIrcDisconnect)
	ON_COMMAND(ID_IRC_CLOSETAB, OnIrcCloseTab)
	ON_UPDATE_COMMAND_UI(ID_IRC_CLOSETAB, OnUpdateIrcCloseTab)

	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_DRAWITEM()
	ON_WM_TIMER()
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOMMAND()

	ON_COMMAND(ID_IRC_SETTINGS, OnIrcShowSettings)
	ON_UPDATE_COMMAND_UI(ID_IRC_SETTINGS, OnUpdateIrcUserCmd)
	ON_COMMAND(ID_IRC_QUERY, UserListDblClick)
	ON_UPDATE_COMMAND_UI(ID_IRC_QUERY, OnUpdateIrcUserCmd)
 	ON_COMMAND(ID_IRC_TIME, OnIrcUserCmdTime)
	ON_UPDATE_COMMAND_UI(ID_IRC_TIME, OnUpdateIrcUserCmd)
	ON_COMMAND(ID_IRC_VERSION, OnIrcUserCmdVersion)
	ON_UPDATE_COMMAND_UI(ID_IRC_VERSION, OnUpdateIrcUserCmd)
	ON_COMMAND(ID_IRC_BROWSE, OnIrcUserCmdBrowse)
	ON_UPDATE_COMMAND_UI(ID_IRC_BROWSE, OnUpdateIrcUserCmd)
	ON_COMMAND(ID_IRC_WHOIS, OnIrcUserCmdWhois)
	ON_UPDATE_COMMAND_UI(ID_IRC_WHOIS, OnUpdateIrcUserCmd)
	ON_COMMAND(ID_IRC_IGNORE, OnIrcUserCmdIgnore)
	ON_UPDATE_COMMAND_UI(ID_IRC_IGNORE, OnUpdateIrcUserCmd)
	ON_COMMAND(ID_IRC_UNIGNORE, OnIrcUserCmdUnignore)
	ON_UPDATE_COMMAND_UI(ID_IRC_UNIGNORE, OnUpdateIrcUserCmd)

	ON_COMMAND(ID_IRC_OP, OnIrcUserCmdOp)
	ON_UPDATE_COMMAND_UI(ID_IRC_OP, OnUpdateIrcUserCmd)
	ON_COMMAND(ID_IRC_DEOP, OnIrcUserCmdDeop)
	ON_UPDATE_COMMAND_UI(ID_IRC_DEOP, OnUpdateIrcUserCmd)
	ON_COMMAND(ID_IRC_VOICE, OnIrcUserCmdVoice)
	ON_UPDATE_COMMAND_UI(ID_IRC_VOICE, OnUpdateIrcUserCmd)
	ON_COMMAND(ID_IRC_DEVOICE, OnIrcUserCmdDevoice)
	ON_UPDATE_COMMAND_UI(ID_IRC_DEVOICE, OnUpdateIrcUserCmd)

	ON_COMMAND(ID_IRC_KICK, OnIrcUserCmdKick)
	ON_UPDATE_COMMAND_UI(ID_IRC_KICK, OnUpdateIrcUserCmd)
	ON_COMMAND(ID_IRC_KICKWHY, OnIrcUserCmdKickWhy)
	ON_UPDATE_COMMAND_UI(ID_IRC_KICKWHY, OnUpdateIrcUserCmd)
	ON_COMMAND(ID_IRC_BAN, OnIrcUserCmdBan)
	ON_UPDATE_COMMAND_UI(ID_IRC_BAN, OnUpdateIrcUserCmd)
	ON_COMMAND(ID_IRC_UNBAN, OnIrcUserCmdUnban)
	ON_UPDATE_COMMAND_UI(ID_IRC_UNBAN, OnUpdateIrcUserCmd)
	ON_COMMAND(ID_IRC_BANKICK, OnIrcUserCmdBanKick)
	ON_UPDATE_COMMAND_UI(ID_IRC_BANKICK, OnUpdateIrcUserCmd)
	ON_COMMAND(ID_IRC_BANKICKWHY, OnIrcUserCmdBanKickWhy)
	ON_UPDATE_COMMAND_UI(ID_IRC_BANKICKWHY, OnUpdateIrcUserCmd)

	ON_NOTIFY(RVN_CLICK, IDC_CHAT_TEXT, OnRichClk)
	ON_NOTIFY(RVN_DBLCLICK, IDC_CHAT_TEXT, OnRichDblClk)
	ON_NOTIFY(RVN_SETCURSOR, IDC_CHAT_TEXT, OnRichCursorMove)
	ON_NOTIFY(TCN_SELCHANGE, IDC_CHAT_TABS, OnClickTab)
END_MESSAGE_MAP()

#define SIZE_INTERNAL	1982
#define SIZE_BARSLIDE	1983
#define NEWLINE_FORMAT	_T("2")
#define DEST_PORT		6667
#define HISTORY_SIZE	20

CIRCFrame* CIRCFrame::g_pIrcFrame = NULL;

/////////////////////////////////////////////////////////////////////////////
// CIRCFrame construction

CIRCFrame::CIRCFrame()
	: m_bConnected( FALSE )
	, m_nSelectedTab( 0 )
	, m_nMsgsInSec( 0 )
	, m_nTimerVal( 0 )
	, m_nSelectedTabType( 0 )
	, m_nRSelectedTab( 0 )
	, m_bFloodProtectionRunning( FALSE )
	, m_nFloodLimit( 0 )
	, m_nFloodingDelay( 4000 )
	, m_nUpdateFrequency( 40 )
	, m_nUpdateChanListFreq( 100000 )
	, m_nBufferCount( 0 )
	, m_nHeaderIcon( 0 )
	, m_hBuffer( NULL )
	, m_ptCursor( 0, 0 )
	, m_nListWidth( 170 )
	, m_nSocket( INVALID_SOCKET )
	, m_nLocalTextLimit( 300 )
	, m_nLocalLinesLimit( 14 )
{
	if ( g_pIrcFrame == NULL ) g_pIrcFrame = this;

	for ( int nChannel = 0; nChannel < MAX_CHANNELS; ++nChannel )
		m_nCurrentPosLineBuffer[ nChannel ] = -1;
}

CIRCFrame::~CIRCFrame()
{
	if ( g_pIrcFrame == this )
		g_pIrcFrame = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CIRCFrame system message handlers

BOOL CIRCFrame::Create(CWnd* pParentWnd)
{
	CRect rect;
	return CWnd::Create( NULL, _T("CIRCFrame"), WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,
		rect, pParentWnd, IDC_IRC_FRAME );
}

int CIRCFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
		return -1;

	if ( ! m_wndPanel.Create( this ) )
		return -1;

	m_gdiImageList.Create( 16, 16, ILC_COLOR32|ILC_MASK, 1, 1 ) ||
	m_gdiImageList.Create( 16, 16, ILC_COLOR24|ILC_MASK, 1, 1 ) ||
	m_gdiImageList.Create( 16, 16, ILC_COLOR16|ILC_MASK, 1, 1 );
	AddIcon( IDI_TICK, m_gdiImageList );

	CRect rectDefault;
	SetOwner( GetParent() );

	m_wndTab.Create( WS_CHILD | WS_VISIBLE | TCS_HOTTRACK,
		rectDefault, this, IDC_CHAT_TABS );
	m_wndTab.SetImageList( &m_gdiImageList );

	FillChanList();
	m_wndView.Create( WS_CHILD | WS_VISIBLE, rectDefault, this, IDC_CHAT_TEXT );

	m_pContent.m_crBackground = Settings.IRC.Colors[ ID_COLOR_CHATBACK ];
	m_wndView.SetDocument( &m_pContent );
	m_wndView.SetSelectable( TRUE );
	m_wndView.SetFollowBottom( TRUE );

	if ( ! m_wndMainBar.Create( this, WS_CHILD | WS_VISIBLE | CBRS_NOALIGN, AFX_IDW_TOOLBAR ) )
		return -1;
	m_wndMainBar.SetBarStyle( m_wndMainBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_BORDER_TOP );
	m_wndMainBar.SetOwner( GetOwner() );

	if ( Settings.General.LanguageRTL )
		m_wndMainBar.ModifyStyleEx( 0, WS_EX_LAYOUTRTL, 0 );

	m_wndEdit.Create( WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, rectDefault, this, IDC_CHAT_EDIT );
	m_wndEdit.SetLimitText( m_nLocalTextLimit );
	m_wndEdit.ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );
	m_wndTab.ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );
	m_wndView.ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );

	SetFonts();

	// Create "Status" tab
	LoadString( m_sStatus, IDS_TIP_STATUS );
	m_sStatus.Remove( ':' );
	AddTab( m_sStatus, ID_KIND_CLIENT );

	// Welcome message
	StatusMessage( LoadString( IDS_IRC_PANEL_TITLE ), ID_COLOR_TEXT );
	StatusMessage( LoadString( IDS_IRC_PANEL_SUBTITLE ), ID_COLOR_TEXT );

	// Reload saved channels
	OnIrcChanCmdOpen();

	return 0;
}

void CIRCFrame::ClearCountChanList()
{
	CListCtrl& wndChanList = m_wndPanel.m_boxChans.m_wndChanList;
	for ( int i = 0 ; i < wndChanList.GetItemCount() ; ++i )
	{
		wndChanList.SetItemText( i, 1, _T("") );
	}
}

// if strUserCount = 0 it will increase the current number
// if strUserCount = -1 it will decrease current number
// otherwise, it sets the number
void CIRCFrame::FillCountChanList(const CString& strUserCount, const CString& strChannelName)
{
	CString strList, strDisplay;
	BOOL bFound = FALSE;
	int nCount = _tstoi( strUserCount ), nList, nIndex, nCountWnd;
	CListCtrl& wndChanList = m_wndPanel.m_boxChans.m_wndChanList;
	nIndex = m_pChanList.GetIndexOfName( strChannelName );
	if ( nIndex == -1 )
		return;

	strDisplay = m_pChanList.GetDisplayOfIndex( nIndex );
	for ( nList = 0 ; nList < wndChanList.GetItemCount() ; nList++ )
	{
		strList = wndChanList.GetItemText( nList, 0 );
		if ( strDisplay.CompareNoCase( strList ) == 0 )
		{
			bFound = TRUE;
			break;
		}
	}
	if ( bFound )
		strList = wndChanList.GetItemText( nList, 1 );
	else
		strList = strUserCount;

	nCountWnd = _tstoi( strList );
	if ( strUserCount == _T("0")  )
		nCountWnd++;
	else if ( strUserCount == _T("-1") )
		nCountWnd--;
	else
		nCountWnd = nCount;

	CString strCount;
	strCount.Format( _T("%d"), nCountWnd );
	if ( ! bFound )
		nList = wndChanList.InsertItem( wndChanList.GetItemCount() , strDisplay );
	wndChanList.SetItemText( nList, 1, strCount );
}

void CIRCFrame::FillChanList()
{
	m_pChanList.RemoveAll();
	m_pChanList.AddChannel( _T("^Support"),		_T("#Shareaza") );
 	m_pChanList.AddChannel( _T("^Admins"),		_T("#Shareaza-Admin") );
	m_pChanList.AddChannel( _T("^English"),		_T("#Shareaza-Chat") );
	m_pChanList.AddChannel( _T("^Developers"),	_T("#Shareaza-dev") );
 	m_pChanList.AddChannel( _T("Afrikaans"),	_T("#Shareaza-Afrikaans") );
	m_pChanList.AddChannel( _T("Arabic"),		_T("#Shareaza-Arabic") );
 	m_pChanList.AddChannel( _T("Chinese"),		_T("#Shareaza-Chinese") );
	m_pChanList.AddChannel( _T("Croatian"),		_T("#Shareaza-Croatian") );
 	m_pChanList.AddChannel( _T("Czech"),		_T("#Shareaza-Czech") );
	m_pChanList.AddChannel( _T("Dutch"),		_T("#Shareaza-Dutch") );
 	m_pChanList.AddChannel( _T("Finish"),		_T("#Shareaza-Finish") );
	m_pChanList.AddChannel( _T("French"),		_T("#Shareaza-French") );
 	m_pChanList.AddChannel( _T("German"),		_T("#Shareaza-German") );
	m_pChanList.AddChannel( _T("Greek"),		_T("#Shareaza-Greek") );
 	m_pChanList.AddChannel( _T("Hebrew"),		_T("#Shareaza-Hebrew") );
	m_pChanList.AddChannel( _T("Hungarian"),	_T("#Shareaza-Hungarian") );
 	m_pChanList.AddChannel( _T("Italian"),		_T("#Shareaza-Italian") );
 	m_pChanList.AddChannel( _T("Japanese"),		_T("#Shareaza-Japanese") );
	m_pChanList.AddChannel( _T("Lithuanian"),	_T("#Shareaza-Lithuanian") );
 	m_pChanList.AddChannel( _T("Norwegian"),	_T("#Shareaza-Norwegian") );
	m_pChanList.AddChannel( _T("Polish"),		_T("#Shareaza-Polish") );
 	m_pChanList.AddChannel( _T("Portuguese"),	_T("#Shareaza-Portuguese") );
	m_pChanList.AddChannel( _T("Russian"),		_T("#Shareaza-Russian") );
 	m_pChanList.AddChannel( _T("Spanish"),		_T("#Shareaza-Spanish") );
 	m_pChanList.AddChannel( _T("Swedish"),		_T("#Shareaza-Swedish") );
}

void CIRCFrame::SetFonts()
{
	// Main window font
	LOGFONT lfDefault = {};
	CoolInterface.m_fntRichDefault.GetLogFont( &lfDefault );
	_tcsncpy( lfDefault.lfFaceName, Settings.IRC.ScreenFont, LF_FACESIZE );
	m_pContent.CreateFonts( &lfDefault );

	// Edit box font
	m_fntEdit.DeleteObject();
	m_fntEdit.CreateFont( -13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, theApp.m_nFontQuality,
		DEFAULT_PITCH|FF_DONTCARE, Settings.Fonts.DefaultFont );
	m_wndEdit.SetFont( &m_fntEdit, TRUE );

	// Tab font
	m_wndTab.SetFont( &theApp.m_gdiFont, TRUE );
}

void CIRCFrame::OnDestroy()
{
	OnIrcChanCmdSave();

	OnIrcDisconnect();

	CWnd::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CIRCFrame presentation message handlers

void CIRCFrame::OnSkinChange()
{
	m_wndPanel.OnSkinChange();

	Skin.CreateToolBar( _T("CIRCFrame"), &m_wndMainBar );
	if ( m_bmWatermark.m_hObject != NULL ) m_bmWatermark.DeleteObject();
	if ( HBITMAP hMark = Skin.GetWatermark( _T("CIRCHeaderPanel") ) )
		m_bmWatermark.Attach( hMark );
	else if ( Skin.m_crBannerBack == RGB( 122, 161, 230 ) )
		m_bmWatermark.LoadBitmap( IDB_BANNER_MARK );

	m_nHeaderIcon = CoolInterface.ExtractIconID( IDR_IRCFRAME, FALSE, LVSIL_BIG );

	m_pContent.m_crBackground = Settings.IRC.Colors[ ID_COLOR_CHATBACK ];

	SetFonts();

	m_wndView.SetDocument( &m_pContent );

	TabClick();
}

void CIRCFrame::OnUpdateCmdUI()
{
	m_wndMainBar.OnUpdateCmdUI( (CFrameWnd*)GetOwner(), TRUE );
}

void CIRCFrame::OnSize(UINT nType, int cx, int cy)
{
	if ( nType != SIZE_INTERNAL && nType != SIZE_BARSLIDE )
		CWnd::OnSize( nType, cx, cy );

	CRect rc;
	GetClientRect( &rc );

	if ( rc.Width() < 32 || rc.Height() < 32 ) return;

	m_wndPanel.SetWindowPos( NULL, 0, 0, PANEL_WIDTH, rc.Height(), SWP_NOZORDER );

	rc.bottom -= TOOLBAR_HEIGHT;
	rc.left   += PANEL_WIDTH;
	m_wndMainBar.SetWindowPos( NULL, rc.left, rc.bottom, rc.Width(),
		TOOLBAR_HEIGHT, SWP_NOZORDER|SWP_SHOWWINDOW );
	m_wndTab.SetWindowPos( NULL, rc.left, rc.top,
		rc.Width(), TABBAR_HEIGHT, SWP_NOZORDER|SWP_SHOWWINDOW );
	rc.top += TABBAR_HEIGHT;
	m_wndEdit.SetWindowPos( NULL, rc.left, rc.bottom - EDITBOX_HEIGHT,
		rc.Width(), EDITBOX_HEIGHT, SWP_NOZORDER|SWP_SHOWWINDOW );
	rc.bottom -= EDITBOX_HEIGHT;
	m_wndView.SetWindowPos( NULL, rc.left, rc.top,
		rc.Width(), rc.Height() - SEPERATOR_HEIGHT - SMALLHEADER_HEIGHT, SWP_NOZORDER|SWP_SHOWWINDOW );
	if ( nType != SIZE_BARSLIDE ) Invalidate();
	if ( m_wndTab.GetItemCount() > 0 ) ReloadViewText();

}

void CIRCFrame::OnPaint()
{
	CPaintDC dc( this );
	CRect rcClient;
	GetClientRect( &rcClient );

	dc.SetTextColor( Skin.m_crBannerText );
	dc.SetBkMode( TRANSPARENT );

	CRect rcComponent;
	rcComponent.right = rcClient.right;
	rcComponent.left = rcClient.left + PANEL_WIDTH;
	rcComponent.top = rcClient.bottom - TOOLBAR_HEIGHT - EDITBOX_HEIGHT -
		SMALLHEADER_HEIGHT;
	rcComponent.bottom = rcComponent.top + SMALLHEADER_HEIGHT;
	PaintHeader( rcComponent, dc );
	rcComponent.DeflateRect( 10, 4 );
	dc.SelectObject( &CoolInterface.m_fntCaption );
	CString str;
	LoadString( str, IDS_IRC_INPUT_CAPTION );
	DrawText( &dc, rcComponent.left, rcComponent.top, str );

	rcComponent.right = rcClient.right;
	rcComponent.left = rcClient.left + PANEL_WIDTH;
	rcComponent.top = rcClient.bottom - TOOLBAR_HEIGHT - EDITBOX_HEIGHT -
		SMALLHEADER_HEIGHT - SEPERATOR_HEIGHT;
	rcComponent.bottom = rcComponent.top + SEPERATOR_HEIGHT;
	dc.FillSolidRect( rcComponent.left, rcComponent.top, 1,
		rcComponent.Height(), GetSysColor( COLOR_BTNFACE ) );
	dc.FillSolidRect( rcComponent.left + 1, rcComponent.top, 1,
		rcComponent.Height(), GetSysColor( COLOR_3DHIGHLIGHT ) );
	dc.FillSolidRect( rcComponent.right - 1, rcComponent.top, 1,
		rcComponent.Height(), GetSysColor( COLOR_3DSHADOW ) );
	dc.FillSolidRect( rcComponent.left + 2, rcComponent.top,
		rcComponent.Width() - 3, rcComponent.Height(), GetSysColor( COLOR_BTNFACE ) );
	dc.ExcludeClipRect( &rcComponent );
}

void CIRCFrame::PaintHeader(CRect rcHeader, CDC &dc)
{
	CRect rcClient;
	GetWindowRect( &rcClient );
	ScreenToClient( &rcClient );
	if ( rcClient.IsRectEmpty() ) return;
	if ( rcHeader.IsRectEmpty() ) return;
	if ( m_bmBuffer.m_hObject != NULL )
	{
		m_dcBuffer.SelectObject( m_hBuffer );
		m_dcBuffer.DeleteDC();
		m_bmBuffer.DeleteObject();
	}
	m_bmBuffer.CreateCompatibleBitmap( &dc, rcClient.Width(), rcClient.Height() );
	m_dcBuffer.CreateCompatibleDC( &dc );
	m_hBuffer = (HBITMAP)m_dcBuffer.SelectObject( &m_bmBuffer )->m_hObject;
	if ( ! CoolInterface.DrawWatermark( &m_dcBuffer, &rcClient, &m_bmWatermark, 0, 0 ) )
	{
		m_dcBuffer.FillSolidRect( &rcHeader, Skin.m_crBannerBack );
	}
	dc.BitBlt( rcHeader.left, rcHeader.top, rcHeader.Width(),
		rcHeader.Height(), &m_dcBuffer, 0, 0, SRCCOPY );
}

void CIRCFrame::DrawText(CDC* pDC, int nX, int nY, LPCTSTR pszText)
{
	CSize sz = pDC->GetTextExtent( pszText );

	CRect rc( nX - 2, nY - 2, nX + sz.cx + 2, nY + sz.cy + 2 );

	DWORD nOptions = ETO_CLIPPED | ( Settings.General.LanguageRTL ? ETO_RTLREADING : 0 );
	pDC->ExtTextOut( nX, nY, nOptions, &rc, pszText, static_cast< UINT >( _tcslen( pszText ) ), NULL );
}

/////////////////////////////////////////////////////////////////////////////
// CIRCFrame interaction message handlers

void CIRCFrame::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if ( pWnd->m_hWnd == m_wndView.m_hWnd )
	{
		CString strText = GetTextFromRichPoint();
		if ( strText.IsEmpty() ) return;
		int nIndex = IsUserInList( strText );
		if ( nIndex != -1 )
		{
			SetSelectedUser( nIndex );
			Skin.TrackPopupMenu( _T("CIRCUserList"), point );
		}
	}
	if ( pWnd->m_hWnd == m_wndTab.m_hWnd )
	{
		CRect rcTab;
		for ( int nTab = 0 ; nTab < m_wndTab.GetItemCount() ; nTab++ )
		{
			m_wndTab.GetItemRect( nTab, rcTab );
			m_wndTab.ClientToScreen( rcTab );
			if ( rcTab.PtInRect( point ) )
			{
				m_wndTab.SetCurSel( nTab );
				Skin.TrackPopupMenu( _T("CIRCTabRClick"), point );
				return;
			}
		}
	}
}

void CIRCFrame::OnIrcConnect()
{
	if ( m_bConnected )
		// Already Connected
		return;

	m_sWsaBuffer.Empty();

	m_sDestinationIP = Settings.IRC.ServerName;
	m_sDestinationPort = Settings.IRC.ServerPort;

	CString strMessage;
	strMessage.Format( LoadString( IDS_CHAT_CONNECTING_TO ), (LPCTSTR)m_sDestinationIP );
	StatusMessage( strMessage, ID_COLOR_TEXT );

	struct hostent* host = gethostbyname( (LPCSTR)CT2A( m_sDestinationIP ) );
	if ( host == NULL )
	{
		// Unknown host
		strMessage.Format( LoadString( IDS_CHAT_CANT_CONNECT ), (LPCTSTR)m_sDestinationIP );
		StatusMessage( strMessage );
		return;
	}

	SOCKADDR_IN dest_addr = {};
    dest_addr.sin_family = AF_INET;
	dest_addr.sin_port	= (u_short)ntohs( (WORD)_tstoi( (LPCTSTR)m_sDestinationPort ) );
	dest_addr.sin_addr.s_addr = *(ULONG*)host->h_addr;

	m_nSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

	int RetVal = WSAConnect(
		m_nSocket, 					// Our socket
		(SOCKADDR*)&dest_addr,		// The remote IP address and port number
		sizeof(SOCKADDR_IN),		// How many bytes the function can read
		NULL, NULL, NULL, NULL );	// No advanced features
	if ( RetVal == SOCKET_ERROR )
	{
		strMessage.Format( LoadString( IDS_CHAT_CANT_CONNECT ), (LPCTSTR)m_sDestinationIP );
		StatusMessage( strMessage );
		CNetwork::CloseSocket( m_nSocket, false );
		return;
	}

	StatusMessage( LoadString( IDS_CHAT_CONNECTED ), ID_COLOR_TEXT );

	m_sNickname	= Settings.IRC.Nick;
	if ( m_sNickname.IsEmpty() )
	{
		CString strNick = MyProfile.GetNick();
		if ( strNick.IsEmpty() )
		{
			m_sNickname.Format( L"razaIrc%09u", GetRandomNum( 0ui32, _UI32_MAX ) );
		}
		else
		{
			Settings.IRC.Nick = m_sNickname = strNick;
		}
	}

	SendString( _T("NICK ") + m_sNickname );

	SendString( _T("USER ") + Settings.IRC.UserName + _T(" ") +
		_T("razaUserHost ") + _T("razaUserServ :") + Settings.IRC.RealName );

	m_pWakeup.ResetEvent();
	WSAEventSelect( m_nSocket, m_pWakeup, FD_READ | FD_CLOSE );
	m_nTimerVal = 0;
	m_nMsgsInSec = 0;
	m_bConnected = TRUE;
	m_bFloodProtectionRunning = FALSE;

	SetTimer( 9, m_nUpdateFrequency, NULL );
	SetTimer( 7, m_nUpdateChanListFreq, NULL );
}

void CIRCFrame::OnUpdateIrcConnect(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( !m_bConnected );
	pCmdUI->SetCheck( m_bConnected );
}

void CIRCFrame::OnUpdateIrcUserCmd(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( TRUE );
	pCmdUI->SetCheck( FALSE );
}

void CIRCFrame::OnIrcUserCmdWhois()
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	OnLocalText( _T("/whois ") + RemoveModeOfNick( strText ) );
}

void CIRCFrame::OnIrcUserCmdOp()
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	OnLocalText( _T("/mode ") + GetTabText() + _T(" +o ") + RemoveModeOfNick( strText ) );
}

void CIRCFrame::OnIrcUserCmdDeop()
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	OnLocalText( _T("/mode ") + GetTabText() + _T(" -o ") + RemoveModeOfNick( strText ) );
}

void CIRCFrame::OnIrcChanCmdOpen()
{
	CString strPath = Settings.General.UserPath + _T("\\Data\\ChatChanlist.dat");

	CFile pFile;
	if ( ! pFile.Open( strPath, CFile::modeRead ) ) return;

	CBuffer pBuffer;
	pBuffer.EnsureBuffer( (DWORD)pFile.GetLength() );
	pBuffer.m_nLength = (DWORD)pFile.GetLength();
	pFile.Read( pBuffer.m_pBuffer, pBuffer.m_nLength );
	pFile.Close();

	CString strItem;
	while ( pBuffer.ReadLine( strItem ) )
	{
		strItem.TrimLeft();
		strItem.TrimRight();

		if ( strItem.GetLength() && strItem.GetAt( 0 ) == _T('#') )
		{
			CListCtrl& wndChanList = m_wndPanel.m_boxChans.m_wndChanList;
			m_pChanList.AddChannel( strItem.Mid( 1 ), strItem, TRUE );
			wndChanList.InsertItem( wndChanList.GetItemCount(), strItem.Mid( 1 ) );
		}
	}
}

void CIRCFrame::OnIrcChanCmdSave()
{
	CFile pFile;
	if ( ! pFile.Open( Settings.General.UserPath + _T("\\Data\\ChatChanlist.dat"),
		CFile::modeWrite | CFile::modeCreate ) )
		return;

	CListCtrl& wndChanList = m_wndPanel.m_boxChans.m_wndChanList;
	for ( int nIndex = 0 ; nIndex < wndChanList.GetItemCount() ; nIndex++ )
	{
		int n_mpChanListIndex = m_pChanList.GetIndexOfDisplay( wndChanList.GetItemText( nIndex, 0 ) );
		if ( n_mpChanListIndex != -1 && m_pChanList.GetType( m_pChanList.GetDisplayOfIndex( n_mpChanListIndex ) )  )
		{
			CT2A pszFile( m_pChanList.GetNameOfIndex( n_mpChanListIndex ) + _T("\r\n") );
			pFile.Write( (LPCSTR)pszFile, (DWORD)strlen( pszFile ) );
		}
	}
}

void CIRCFrame::OnIrcUserCmdVoice()
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	OnLocalText( _T("/mode ") + GetTabText() + _T(" +v ") + RemoveModeOfNick( strText ) );
}

void CIRCFrame::OnIrcShowSettings()
{
	CSettingsManagerDlg::Run( _T("CIRCSettingsPage") );
}

void CIRCFrame::OnIrcUserCmdBan()
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	OnLocalText( _T("/mode ") + GetTabText() + _T(" +b ") + RemoveModeOfNick( strText ) );
}

void CIRCFrame::OnIrcUserCmdUnban()
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	OnLocalText( _T("/mode ") + GetTabText() + _T(" -b ") + RemoveModeOfNick( strText ) );
}

void CIRCFrame::OnIrcUserCmdKick()
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	OnLocalText( _T("/kick ") + GetTabText() + _T(" ") + RemoveModeOfNick( strText ) );
}

void CIRCFrame::OnIrcUserCmdKickWhy()
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	CIrcInputDlg dlg( this, 1, TRUE );	// 1 = select the second caption
	if ( dlg.DoModal() != IDOK ) return;
	OnLocalText( _T("/kick ") + GetTabText() + _T(" ") + RemoveModeOfNick( strText ) + _T(" ") + dlg.m_sAnswer );
}

void CIRCFrame::OnIrcUserCmdBanKick()
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	OnLocalText( _T("/mode ") + GetTabText() + _T(" +b ") + RemoveModeOfNick( strText ) );
	OnLocalText( _T("/kick ") + GetTabText() + _T(" ") + RemoveModeOfNick( strText ) );
}

void CIRCFrame::OnIrcUserCmdBanKickWhy()
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	CIrcInputDlg dlg( this, 1, FALSE );	// 1 = select the second caption
	if ( dlg.DoModal() != IDOK ) return;
	OnLocalText( _T("/mode ") + GetTabText() + _T(" +b ") + RemoveModeOfNick( strText ) );
	OnLocalText( _T("/kick ") + GetTabText() + _T(" ") + RemoveModeOfNick( strText ) + _T(" ") + dlg.m_sAnswer );
}

void CIRCFrame::OnIrcUserCmdDevoice()
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	OnLocalText( _T("/mode ") + GetTabText() + _T(" -v ") + RemoveModeOfNick( strText ) );
}

void CIRCFrame::OnIrcUserCmdIgnore()
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	OnLocalText( _T("/SILENCE +") + RemoveModeOfNick( strText ) );
}

void CIRCFrame::OnIrcUserCmdUnignore()
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	OnLocalText( _T("/SILENCE -") + RemoveModeOfNick( strText ) );
}

void CIRCFrame::OnIrcUserCmdVersion()
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	OnLocalText( _T("/PRIVMSG ") + RemoveModeOfNick( strText ) + _T(" :\x01VERSION\x01") );
}

void CIRCFrame::OnIrcUserCmdBrowse()
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	OnLocalText( _T("/PRIVMSG ") + RemoveModeOfNick( strText ) + _T(" :\x01USERINFO\x01") );
}

void CIRCFrame::OnIrcUserCmdTime()
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	OnLocalText( _T("/PRIVMSG ") + RemoveModeOfNick( strText ) + _T(" :\x01TIME\x01") );
}

void CIRCFrame::OnIrcCloseTab()
{
	int nTab = m_wndTab.GetCurSel(), nOldTab( nTab );

	CString strChannelName = GetTabText( nTab );
	FillCountChanList( _T("-1"), strChannelName );
	if ( strChannelName == m_sStatus )
		return;

	ClearUserList();

	m_pContent.Clear();
	m_wndTab.DeleteItem( nTab );
	m_pIrcBuffer[ nTab ].RemoveAll();
	m_pIrcUsersBuffer[ nTab ].RemoveAll();

	for ( ; nTab < m_nBufferCount - 1 ; nTab++ )
	{
		m_pIrcBuffer[ nTab ].Append( m_pIrcBuffer[ nTab + 1 ] );
		m_pIrcUsersBuffer[ nTab ].Append( m_pIrcUsersBuffer[ nTab + 1 ] );
	}
	if ( strChannelName.Left( 1 ) == _T("#") )
		SendString( _T("PART ") + strChannelName );

	m_nBufferCount--;

	if ( nOldTab <= 1 )
		m_wndTab.SetCurSel( 0 );
	else
		m_wndTab.SetCurSel( nOldTab - 1 );

	TabClick();
}

void CIRCFrame::OnUpdateIrcCloseTab(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( TRUE );
	pCmdUI->SetCheck( FALSE );
}

void CIRCFrame::OnIrcDisconnect()
{
	ClearCountChanList();
	ClearUserList();

	StatusMessage( LoadString( IDS_CHAT_CLOSED ), ID_COLOR_TEXT );

	SendString( _T("QUIT") );

	CNetwork::CloseSocket( m_nSocket, false );

	KillTimer( 9 );
	KillTimer( 7 );

	// Delete all tabs except "Status"
	for ( int nChannel = 1 ; nChannel < MAX_CHANNELS ; nChannel++ )
	{
		m_wndTab.DeleteItem( nChannel );
		m_pIrcBuffer[ nChannel ].RemoveAll();
		m_nCurrentPosLineBuffer[ nChannel ] = -1;
		m_pIrcUsersBuffer[ nChannel ].RemoveAll();
		m_pLastLineBuffer[ nChannel ].RemoveAll();
	}
	m_nBufferCount = 1;

	m_bConnected = FALSE;
}

void CIRCFrame::OnUpdateIrcDisconnect(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_bConnected );
	pCmdUI->SetCheck( FALSE );
}

/* Return the title of a tab.
 * If no argument is supplied, the currently selected tab is returned
 */
CString CIRCFrame::GetTabText(int nTabIndex) const
{
	if ( nTabIndex == -1 )
		nTabIndex = m_wndTab.GetCurSel();

	CString sBuffer;
	TCITEM item = { TCIF_IMAGE | TCIF_PARAM | TCIF_TEXT };
	item.pszText = sBuffer.GetBuffer( 1024 );
	item.cchTextMax = 1023;
	BOOL ret = m_wndTab.GetItem( nTabIndex, &item );
	sBuffer.ReleaseBuffer();

	return ret ? sBuffer : CString();
}

/* When locally created text needs to be sent to the server or displayed on the client.
 */
void CIRCFrame::OnLocalText(LPCTSTR pszText)
{
	if ( ! m_bConnected )
	{
		StatusMessage( LoadString( IDS_CHAT_NOT_CONNECTED_1 ) );
		OnIrcConnect();
		return;
	}

	CString strMessage = pszText;	// the line as typed, shared between Send and Disp.
	CString strSend; //text		// the line sent to the server
	CString strDisp; //line		// the line displayed on the client

	const CString strTabTitle = GetTabText(); // () -> ( -1 ) -> currently selected tab
	CString strBufferMsg;

	BOOL isActionMsg = FALSE;

	// -1 if no CRLF, otherwise the position of CRLF
	strMessage.TrimRight( _T("\r\n") );

	// Expand short commands to full-length commands
	if ( strMessage.GetLength() > 2 )
	{
		if ( strMessage.Left( 3 ).CompareNoCase( _T("/j ") ) == 0 )
			strMessage = _T("/JOIN") + strMessage.Mid( 2 );
	}
	if ( strMessage.GetLength() > 3 )
	{
		if ( strMessage.Left( 4 ).CompareNoCase( _T("/bs ") ) == 0 )
			strMessage = _T("/BOTSERV") + strMessage.Mid( 3 );
		else if ( strMessage.Left( 4 ).CompareNoCase( _T("/ns ") ) == 0 )
			strMessage = _T("/NICKSERV") + strMessage.Mid( 3 );
		else if ( strMessage.Left( 4 ).CompareNoCase( _T("/cs ") ) == 0 )
			strMessage = _T("/CHANSERV") + strMessage.Mid( 3 );
		else if ( strMessage.Left( 4 ).CompareNoCase( _T("/me ") ) == 0 )
		{
			isActionMsg = TRUE;
			// Remove the '/me ' from the text
			strMessage = strMessage.Mid( 4 );
		}
	}

//----- START local display line

	// Add timestamp if enabled
	if ( Settings.IRC.Timestamp )
	{
		CTime tNow = CTime::GetCurrentTime();
		strDisp.Format( _T("[%.2i:%.2i:%.2i] "),
			tNow.GetHour(), tNow.GetMinute(), tNow.GetSecond() );
	}

	// Formatting
	if ( isActionMsg )
	{
		strDisp = strDisp + _T("* ") + m_sNickname + _T(" ") + strMessage;
	}
	else if( strMessage.Left( 1 ).Compare( _T("/") ) != 0 )
	{
		strDisp = strDisp + _T("<") + m_sNickname + _T("> ") + strMessage;
	}
	//else It's a command, the server will send feedback

	// Prepend the color code to the message
	strBufferMsg = CHAR( isActionMsg ? ID_COLOR_ME : ID_COLOR_TEXT ) + strDisp;

//------- END local display line

	// If it's a command, remove the '/'
	if ( strMessage.Left( 1 ).Compare( _T("/") ) == 0 )
	{
		strSend = strMessage.Mid( 1 );
	}
	else
	{
		// Not a command but in the status window?
		if ( strTabTitle.Compare( m_sStatus ) == 0 )
		{
			StatusMessage( _T("You are currently not in a channel.") );
		}
		else
		{
			// Display the line locally, in the appropriate color
			OnStatusMessage( strDisp, isActionMsg ? ID_COLOR_ME : ID_COLOR_TEXT );

			m_pIrcBuffer[ m_wndTab.GetCurSel() ].Add( strBufferMsg );

			if ( isActionMsg )
			{
				strSend = _T("\x01");
				strSend += _T("ACTION ");
				strSend += strMessage;
				strSend += _T("\x01");
			}
			else strSend = strMessage;

			strSend = _T("PRIVMSG ") + strTabTitle + _T(" :") + strSend;

			// Notify chat plugins about new local message
			Plugins.OnChatMessage( strTabTitle, TRUE, m_sNickname, strTabTitle, pszText );
		}
	}

	// Send 'strSend' to the server
	SendString( strSend );
}


void CIRCFrame::OnTimer(UINT_PTR nIDEvent)
{
	// Refresh channel list
	if ( nIDEvent == 7 )
	{
		CListCtrl& wndChanList = m_wndPanel.m_boxChans.m_wndChanList;
		for ( int nList = 0 ; nList < wndChanList.GetItemCount() ; nList++ )
			if ( ! wndChanList.GetItemText( nList, 1 ).IsEmpty() )
				wndChanList.SetItemText( nList, 1, _T("0") );
		OnLocalText( _T("/list #shareaza*") );
		return;
	}
	// Stop flood protection
	if ( nIDEvent == 8 )
	{
		m_bFloodProtectionRunning = FALSE;
		int nTargetWindow = IsTabExist( m_sStatus ) ;
		if ( m_wndTab.GetCurSel() == nTargetWindow )
			StatusMessage( _T("Stopped flood protection."), ID_COLOR_SERVERMSG );
		m_pIrcBuffer[ nTargetWindow ].Add( char(ID_COLOR_SERVERMSG) + CString( _T("Stopped flood protection.") ) );
		KillTimer( 8 );
		m_nTimerVal = 0;
		return;
	}
	// Flood-check trigger + ???
	if ( nIDEvent == 9 )
	{
		m_nTimerVal++;
		if ( m_nTimerVal > 3000 / m_nUpdateFrequency && !m_bFloodProtectionRunning )
		{
			m_nTimerVal = 0;
			m_nFloodLimit = _tstoi( (LPCTSTR)Settings.IRC.FloodLimit ) * 3;

			if ( m_nMsgsInSec > m_nFloodLimit )
			{
				SetTimer( 8, m_nFloodingDelay, NULL );
				m_bFloodProtectionRunning = TRUE;
				int nTargetWindow = IsTabExist( m_sStatus );
				if ( m_wndTab.GetCurSel() != nTargetWindow )
					m_pIrcBuffer[ m_wndTab.GetCurSel() ].Add( char(ID_COLOR_SERVERMSG ) + CString( _T("Starting flood protection...") ) );
				StatusMessage( _T("Starting flood protection..."), ID_COLOR_SERVERMSG );
			}
			m_nMsgsInSec = 0;
		}

		if ( WaitForSingleObject( m_pWakeup, 0 ) == WAIT_OBJECT_0 )
		{
			auto_array< char > pszData( new char[ 4096 ] );
			pszData[ 0 ] = '\0';
			int nRetVal = CNetwork::Recv( m_nSocket, pszData.get(), 4095 );
			if ( nRetVal > 0 )
				pszData[ nRetVal ] = '\0';
			CStringA strTmp = m_sWsaBuffer + pszData.get();
			m_sWsaBuffer.Empty();

			switch ( nRetVal )
			{
			case 0:	// Connection has been gracefully closed
				StatusMessage( TrimString( UTF8Decode( strTmp ) ), ID_COLOR_SERVERMSG );
				OnIrcDisconnect();
				return;

			case SOCKET_ERROR:
				KillTimer( 9 );
				StatusMessage( LoadString( IDS_CHAT_DROPPED ) );
				if ( ! strTmp.IsEmpty() )
					StatusMessage( TrimString( UTF8Decode( strTmp ) ), ID_COLOR_SERVERMSG );
				OnIrcDisconnect();
				return;
			}

			if ( ! strTmp.IsEmpty() )
			{
				m_nMsgsInSec++;

				// If it's not a complete line, add it to the buffer until we get the rest
				int nIndex = strTmp.Find( "\r\n" );
				if ( nIndex == -1 )
				{
					m_sWsaBuffer = strTmp;
				}
				else
				{
					while ( nIndex != -1 && ! strTmp.IsEmpty() )
					{
						CStringA strMessage = strTmp.Left( nIndex );
						strMessage.TrimLeft();

						if ( ! strMessage.IsEmpty() )
							OnNewMessage( TrimString( UTF8Decode( strMessage ) ) );

						strTmp = strTmp.Mid( nIndex + 2 );
						nIndex = strTmp.Find( "\r\n" );
					}

					if ( ! strTmp.IsEmpty() )
						m_sWsaBuffer = strTmp;
				}
			}
		}
	}
	CWnd::OnTimer( nIDEvent );
}

void CIRCFrame::SendString(const CString& strMessage)
{
	CStringA strEncoded = UTF8Encode( strMessage + _T("\r\n") );
	CNetwork::Send( m_nSocket, (LPCSTR)strEncoded, strEncoded.GetLength() );
}

void CIRCFrame::StatusMessage(LPCTSTR pszText, int nFlags)
{
	if ( ! *pszText )
		return;

	if ( Settings.IRC.Timestamp )
	{
		CString strMessage;
		CTime tNow = CTime::GetCurrentTime();
		strMessage.Format( _T("[%.2i:%.2i:%.2i] %s"),
			tNow.GetHour(), tNow.GetMinute(), tNow.GetSecond(), pszText );
		OnStatusMessage( strMessage, nFlags );
		m_pIrcBuffer[ 0 ].Add( char( nFlags ) + strMessage );
	}
	else
	{
		OnStatusMessage( pszText, nFlags );
		m_pIrcBuffer[ 0 ].Add( char( nFlags ) + CString( pszText ) );
	}
}

void CIRCFrame::OnStatusMessage(LPCTSTR pszText, int nFlags)
{
	CString strMessage = pszText;
	COLORREF cRGB = Settings.IRC.Colors[ nFlags ];

	// While there are color codes, remove them.
	int nIndex = strMessage.Find( _T("\x03") ); // Find a color code indicator (Ctrl+K)
	while ( nIndex != -1 )
	{
		int nSize = 1;
		// If character (nIndex+1) is a digit nSize = 2
		if ( ( (int)( strMessage.GetAt( nIndex + 1 ) ) - 48 ) > -1 &&
			 ( (int)( strMessage.GetAt( nIndex + 1 ) ) - 48 < 10 ) )
		{
			nSize++;
			// If character (nIndex+2) is a digit = 3
			if ( ( (int)( strMessage.GetAt( nIndex + 2 ) ) - 48 ) > -1 &&
				 ( (int)( strMessage.GetAt( nIndex + 2 ) ) - 48 < 10 ) ) nSize++;

			if ( strMessage.GetAt( nIndex + 3 ) == ',' ||
				 strMessage.GetAt( nIndex + 2 ) == ',' )
			{
				nSize++;
				if ( ( (int)( strMessage.GetAt( nIndex + 4 ) ) - 48 ) > -1 &&
					 ( (int)( strMessage.GetAt( nIndex + 4 ) ) - 48 < 10 ) ) nSize++;

				if ( ( (int)( strMessage.GetAt( nIndex + 5 ) ) - 48 ) > -1 &&
					 ( (int)( strMessage.GetAt( nIndex + 5 ) ) - 48 < 10 ) ) nSize++;
			}
		}
		strMessage = strMessage.Mid( 0, nIndex ) + strMessage.Mid( nIndex + nSize + 1 );
		nIndex = strMessage.Find( _T("\x03") );
	}

	// Remove invalid characters (0x01 - 0x1F)
	for ( nIndex = 1 ; nIndex < 32 ; nIndex++ )
		strMessage.Remove( char(nIndex) );

	//Emoticons.FormatText( &m_pContent, strMessage, FALSE, cRGB );
	m_pContent.Add( retText, strMessage, NULL, retfColour )->m_cColour = cRGB;
	m_pContent.Add( retNewline, NEWLINE_FORMAT );
	m_wndView.InvalidateIfModified();
}

void CIRCFrame::ReloadViewText()
{
    LoadBufferForWindow( m_wndTab.GetCurSel() );
}

BOOL CIRCFrame::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_NOTIFY )
	{
		if ( pMsg->wParam == IDC_IRC_DBLCLKCHANNELS )
		{
			ChanListDblClick();
			return TRUE;
		}
		if ( pMsg->wParam == IDC_IRC_DBLCLKUSERS )
		{
			UserListDblClick();
			return TRUE;
		}
		if ( pMsg->wParam == IDC_IRC_MENUUSERS )
		{
			Skin.TrackPopupMenu( _T("CIRCUserList"), pMsg->pt );
			return TRUE;
		}

	}
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( pMsg->wParam == VK_UP )
		{
			int nTab = m_wndTab.GetCurSel();
			if ( m_nCurrentPosLineBuffer[ nTab ] + 1 > m_pLastLineBuffer[ nTab ].GetCount() - 1 ) return TRUE;
			if ( m_nCurrentPosLineBuffer[ nTab ] < HISTORY_SIZE ) m_nCurrentPosLineBuffer[ nTab ]++;
			m_wndEdit.SetWindowText( m_pLastLineBuffer[ nTab ].GetAt( m_nCurrentPosLineBuffer[ nTab ] ) );
			return TRUE;
		}
		else if ( pMsg->wParam == VK_DOWN )
		{
			int nTab = m_wndTab.GetCurSel();
			if ( m_nCurrentPosLineBuffer[ nTab ] - 1 > m_pLastLineBuffer[ nTab ].GetCount() - 1 ) return TRUE;
			if ( m_nCurrentPosLineBuffer[ nTab ] > -1 ) m_nCurrentPosLineBuffer[ nTab ]--;
			CString strBufferText;
			if ( m_nCurrentPosLineBuffer[ nTab ] != -1 )
				strBufferText = m_pLastLineBuffer[ m_wndTab.GetCurSel() ]
					.GetAt( m_nCurrentPosLineBuffer[ m_wndTab.GetCurSel() ] );
			m_wndEdit.SetWindowText( strBufferText );
			return TRUE;
		}
		else if ( pMsg->wParam == VK_RETURN )
		{
			if ( m_wndEdit.GetLineCount() > m_nLocalLinesLimit )
			{
				AfxMessageBox( _T("You have exceeded the maximum number of lines allowed!") );
				return TRUE;
			}
			m_wndEdit.GetWindowText( m_sCurrent );
			if ( m_sCurrent.IsEmpty() ) return TRUE;
			if ( m_sCurrent.GetLength() > m_nLocalTextLimit )
			{
				AfxMessageBox( _T("You have exceeded the maximum length of text allowed!") );
				return TRUE;
			}
			int nTab = m_wndTab.GetCurSel();
			if ( nTab == -1 ) return TRUE;
			m_pLastLineBuffer[ nTab ].InsertAt( 0, m_sCurrent );
			if ( m_pLastLineBuffer[ nTab ].GetCount() > HISTORY_SIZE )
				m_pLastLineBuffer[ nTab ].RemoveAt( HISTORY_SIZE );
			m_nCurrentPosLineBuffer[ nTab ] = -1;
			OnLocalText( m_sCurrent );

			m_sCurrent.Empty();
			m_wndEdit.SetWindowText( m_sCurrent );
			return TRUE;
		}
	}
	if ( pMsg->message == WM_REMOVECHANNEL )
	{
		if ( pMsg->wParam == IDC_IRC_CHANNELS )
		{
			CListCtrl& wndChanList = m_wndPanel.m_boxChans.m_wndChanList;
			int nItem = wndChanList.GetNextItem( -1, LVNI_SELECTED );
			if ( nItem == -1 ) return TRUE;
			CString strItem = wndChanList.GetItemText( nItem, 0 );
			if ( m_pChanList.GetType( strItem  ) )
				wndChanList.DeleteItem( nItem );
			OnIrcChanCmdSave();
			return TRUE;
		}
	}
	if ( pMsg->message == WM_ADDCHANNEL )
	{
		if ( pMsg->wParam == IDC_IRC_CHANNELS )
		{
			CString strChan;
			strChan = m_wndPanel.m_boxChans.m_sPassedChannel;
			m_wndPanel.m_boxChans.m_sPassedChannel.Empty();
			m_pChanList.AddChannel( strChan, _T("#") + strChan, TRUE );
			OnIrcChanCmdSave();
			return TRUE;
		}
	}

	return CWnd::PreTranslateMessage( pMsg );
}

void CIRCFrame::ChanListDblClick()
{
	CListCtrl& wndChanList = m_wndPanel.m_boxChans.m_wndChanList;
	int nItem = wndChanList.GetNextItem( -1, LVNI_SELECTED );
	if ( nItem >= 0 )
	{
		CString strDisplay = wndChanList.GetItemText( nItem, 0 );
		int nIndex = m_pChanList.GetIndexOfDisplay( strDisplay );
		if ( nIndex >= 0 )
		{
			CString strChannelName = m_pChanList.GetNameOfIndex( nIndex );
			int nTab = IsTabExist( strChannelName );
			if ( nTab < 0 )
			{
				// Join it
				if ( m_bConnected )
					SendString( _T("JOIN ") + strChannelName );
				else
				{
					StatusMessage( LoadString( IDS_CHAT_NOT_CONNECTED_1 ) );
					OnIrcConnect();
				}
			}
			else
			{
				// Show it
				if ( nTab != m_wndTab.GetCurSel() )
				{
					m_wndTab.SetCurSel( nTab );
					TabClick();
				}
			}
		}
	}
}

BOOL CIRCFrame::OnNewMessage(const CString& strMessage)
{
	CIRCNewMessage oNewMessage;
	ParseString( strMessage, oNewMessage );

	for ( int nMessage = 0 ; nMessage < oNewMessage.m_pMessages.GetCount() ; nMessage++ )
	{
		int nColorID = oNewMessage.m_pMessages[ nMessage ].nColorID;
		int nTargetWindow = IsTabExist( oNewMessage.m_pMessages[ nMessage ].sTargetName );
		if ( nTargetWindow != -1 &&
			! oNewMessage.m_pMessages[ nMessage ].sMessage.IsEmpty() )
		{
			CString strLine;
			if ( Settings.IRC.Timestamp )
			{
				CTime tNow = CTime::GetCurrentTime();
				strLine.Format( _T("[%.2i:%.2i:%.2i] "),
					tNow.GetHour(), tNow.GetMinute(), tNow.GetSecond() );
			}
			strLine += oNewMessage.m_pMessages[ nMessage ].sMessage;
			if ( m_wndTab.GetCurSel() == nTargetWindow )
				OnStatusMessage( strLine, nColorID );
			m_pIrcBuffer[ nTargetWindow ].Add( char( nColorID ) + strLine );
		}
	}

	return TRUE;
}

void CIRCFrame::HighlightTab(int nTab, BOOL bHighlight)
{
	TCITEM it = {};
	it.mask = TCIF_IMAGE;
	it.iImage = bHighlight ? 0 : -1;
	m_wndTab.SetItem( nTab, &it );
	// m_wndTab.SetTabColor( nTab, bHighlight ? Settings.IRC.Colors[ ID_COLOR_NEWMSG ] : RGB(0, 0, 0) );
}

void CIRCFrame::ActivateMessageByID(CIRCNewMessage& oNewMessage, int nMessageType)
{
	switch ( nMessageType )
	{
		case NULL:
		{
			int nTab = IsTabExist( m_sStatus );
			if ( nTab != -1 && nTab != m_wndTab.GetCurSel() )
				HighlightTab( nTab );
			oNewMessage.Add( GetStringAfterParsedItem( 0 ), m_sStatus, ID_COLOR_SERVERMSG );
			return;
		}
		case ID_MESSAGE_SERVER_MSG:
		{
			int nTab = IsTabExist( m_sStatus );
			if ( nTab != -1 && nTab != m_wndTab.GetCurSel() )
				HighlightTab( nTab );
			oNewMessage.Add( GetStringAfterParsedItem( 3 ), m_sStatus, ID_COLOR_SERVERMSG );
			return;
		}
		case ID_MESSAGE_SERVER_ERROR:
		{
			int nTab = IsTabExist( m_sStatus );
			if ( nTab != -1 && nTab != m_wndTab.GetCurSel() )
				HighlightTab( nTab );
			oNewMessage.Add( GetStringAfterParsedItem( 3 ), m_sStatus, ID_COLOR_SERVERERROR );
			return;
		}
		case ID_MESSAGE_IGNORE:
		{
			return;
		}
		case ID_MESSAGE_SERVER_CONNECTED:
		{
			m_sNickname = m_pWords.GetAt( 2 );
			oNewMessage.Add( _T("Connection Established!"), m_sStatus, ID_COLOR_NOTICE );
			OnLocalText( _T("/list #shareaza*") );
			return;
		}
		case ID_MESSAGE_SERVER_PING:
		{
			SendString( _T("PONG ") + GetStringAfterParsedItem( 1 ) );
#ifdef _DEBUG
			oNewMessage.Add ( _T("Ping? Pong!"), m_sStatus, ID_COLOR_SERVERMSG );
#endif
			return;
		}
		case ID_MESSAGE_USER_MESSAGE:
		{
			int nTab = AddTab( m_pWords.GetAt( 0 ), ID_KIND_PRIVATEMSG );
			if ( nTab == -1 )
				nTab = IsTabExist( m_pWords.GetAt( 0 ) );
			if ( nTab != m_wndTab.GetCurSel() )
				HighlightTab( nTab );
			CString strSender = _T("<") + m_pWords.GetAt( 0 ) + _T("> ");
			CString strText = GetStringAfterParsedItem( 7 );

			if ( CMainWnd* pWnd = theApp.CShareazaApp::SafeMainWnd() )
			{
				if ( ! pWnd->IsForegroundWindow() )
					pWnd->ShowTrayPopup( strText, m_pWords.GetAt( 0 ), NIIF_USER );
			}

			oNewMessage.Add( strSender + GetStringAfterParsedItem( 7 ), m_pWords.GetAt( 0 ), ID_COLOR_TEXT );

			// Notify chat plugins about new remote message
			Plugins.OnChatMessage( GetTabText( nTab ), FALSE, m_pWords.GetAt( 0 ), m_sNickname, strText );
			return;
		}
		case ID_MESSAGE_USER_AWAY:
		{
			CString strSender;
			strSender.Format( LoadString( IDS_CHAT_PRIVATE_AWAY ), (LPCTSTR)m_pWords.GetAt( 3 ), (LPCTSTR)GetStringAfterParsedItem( 4 ) );
			strSender = _T("* ") + strSender;
			oNewMessage.Add( strSender, GetTabText( m_wndTab.GetCurSel() ), ID_COLOR_SERVERMSG );
			return;
		}
		case ID_MESSAGE_USER_KICK:
		{
			if ( m_pWords.GetAt( 7 ) != m_sNickname )
			{
				int nTab = IsTabExist( m_pWords.GetAt( 6 ) );
				if ( nTab!= -1 )
				{
					int nListUser = FindInList( m_pWords.GetAt( 7 ) );
					if ( nTab == m_wndTab.GetCurSel() && nListUser != -1 )
						DeleteUser( nListUser );
					nListUser = FindInList( m_pWords.GetAt( 7 ), 2, nTab );
					if ( nListUser != -1 )
						m_pIrcUsersBuffer[ nTab ].RemoveAt( nListUser );
					SortUserList();
					FillCountChanList( _T("-1"), m_pWords.GetAt( 6 ) );
				}
				CString strSender = _T("* ") + m_pWords.GetAt( 7 ) + _T(" was kicked by ") + m_pWords.GetAt( 0 ) + _T(" (") + GetStringAfterParsedItem( 8 ) + _T(")");
				oNewMessage.Add( strSender, GetTabText( m_wndTab.GetCurSel() ), ID_COLOR_SERVERMSG );
			}
			else
			{
				int nTab = IsTabExist( m_pWords.GetAt( 6 ) );
				if ( nTab != -1 )
				{
					m_wndTab.SetCurSel( nTab );
					OnIrcCloseTab();
				}
				m_wndTab.SetCurSel( 0 );
				TabClick();
				CString strSender = _T("* You were kicked by ") + m_pWords.GetAt( 0 ) + _T("from channel ") + m_pWords.GetAt( 6 ) + _T(" (") + GetStringAfterParsedItem( 8 ) + _T(")");
				oNewMessage.Add( strSender, m_sStatus, ID_COLOR_SERVERMSG );
			}
			return;
		}
		case ID_MESSAGE_STOPAWAY:
		{
			oNewMessage.Add( _T("* You are no longer set as away"), GetTabText( m_wndTab.GetCurSel() ), ID_COLOR_SERVERMSG );
			return;
		}
		case ID_MESSAGE_SETAWAY:
		{
			oNewMessage.Add( _T("* You are now set as away"), GetTabText( m_wndTab.GetCurSel() ), ID_COLOR_SERVERMSG );
			return;
		}
		case ID_MESSAGE_USER_ME:
		{
			int nTab = AddTab( m_pWords.GetAt( 0 ), ID_KIND_PRIVATEMSG );
			if ( nTab != m_wndTab.GetCurSel() )
				HighlightTab( nTab );
			CString strSender = _T("* ") + m_pWords.GetAt( 0 ) + _T(" ");
			oNewMessage.Add( strSender + GetStringAfterParsedItem( 8 ), m_pWords.GetAt( 0 ), ID_COLOR_ME );
			return;
		}
		case ID_MESSAGE_USER_INVITE:
		{
			CString strSender = _T("* You have just been invited to channel ") + m_pWords.GetAt( 8 ) + _T(" by ") + m_pWords.GetAt( 6 );
			oNewMessage.Add( strSender, GetTabText( m_wndTab.GetCurSel() ), ID_COLOR_SERVERMSG );
			return;
		}
		case ID_MESSAGE_CHANNEL_MESSAGE:
		{
			int m_nTab = IsTabExist( m_pWords.GetAt( 6 ) );
			if ( m_nTab == -1 ) return;
			if ( m_nTab != m_wndTab.GetCurSel() )
				HighlightTab( m_nTab );
			CString strSender = _T("<") + m_pWords.GetAt( 0 ) + _T("> ");
			CString strText = GetStringAfterParsedItem( 7 );
			oNewMessage.Add( strSender + strText, m_pWords.GetAt( 6 ), ID_COLOR_TEXT );

			// Notify chat plugins about new remote message
			Plugins.OnChatMessage( GetTabText( m_nTab ), FALSE, m_pWords.GetAt( 0 ), m_sNickname, strText );
			return;
		}
		case ID_MESSAGE_CHANNEL_ME:
		{
			int m_nTab = IsTabExist( m_pWords.GetAt( 6 ) );
			if ( m_nTab == -1 ) return;
			if ( m_nTab != m_wndTab.GetCurSel() )
				HighlightTab( m_nTab );
			CString strSender = _T("* ") + m_pWords.GetAt( 0 ) + _T(" ");
			oNewMessage.Add( strSender + GetStringAfterParsedItem( 8 ), m_pWords.GetAt( 6 ), ID_COLOR_ME );
			return;
		}

		case ID_MESSAGE_CHANNEL_NOTICE:
		{
			oNewMessage.Add( _T("-") + m_pWords.GetAt( 0 ) + _T(" - ") + GetStringAfterParsedItem ( 8 ), m_pWords.GetAt( 6 ), ID_COLOR_NOTICE );
			return;
		}
		case ID_MESSAGE_CLIENT_INVITE:
		{
			CString strSender = _T("* You have just invited ") + m_pWords.GetAt( 3 ) + _T(" to channel ") + m_pWords.GetAt( 4 );
			oNewMessage.Add( strSender, GetTabText( m_wndTab.GetCurSel() ), ID_COLOR_SERVERMSG );
			return;
		}
		case ID_MESSAGE_CLIENT_WHOWAS:
		{
			CString strSender = m_pWords.GetAt( 2 ) + _T(" was ") + m_pWords.GetAt( 5 ) + m_pWords.GetAt( 6 ) + GetStringAfterParsedItem( 7 );
			oNewMessage.Add( strSender, m_sStatus, ID_COLOR_SERVERMSG );
			return;
		}
		case ID_MESSAGE_CLIENT_WHOIS:
		{
			CString strSender = GetStringAfterParsedItem( 3 );
			oNewMessage.Add( strSender, m_sStatus, ID_COLOR_SERVERMSG );
			return;
		}
		case ID_MESSAGE_CLIENT_NOTICE:
		{
			int nTab = m_wndTab.GetCurSel();
			CString strChannelName;

			TCITEM item = { TCIF_IMAGE | TCIF_PARAM | TCIF_TEXT };
			item.pszText = strChannelName.GetBuffer( 1024 );
			item.cchTextMax = 1023;
			m_wndTab.GetItem( nTab, &item );
			strChannelName.ReleaseBuffer();

			oNewMessage.Add( _T("-") + m_pWords.GetAt( 0 ) + _T("- ") + GetStringAfterParsedItem ( 7 ), strChannelName, ID_COLOR_NOTICE );
			return;
		}
		case ID_MESSAGE_USER_CTCPTIME:
		{
			CTime time = CTime::GetCurrentTime();
			TIME_ZONE_INFORMATION tzi = {};
			int nTZBias;
			if ( GetTimeZoneInformation( &tzi ) == TIME_ZONE_ID_DAYLIGHT )
				nTZBias = tzi.Bias + tzi.DaylightBias;
			else
				nTZBias = tzi.Bias;
			CString strReply;
			strReply.Format( _T("/NOTICE %s :\x01TIME %s %+.2d%.2d\x01"),
				(LPCTSTR)m_pWords.GetAt( 0 ),
				(LPCTSTR)time.Format( _T("%Y-%m-%d %H:%M:%S") ),
				- nTZBias / 60, nTZBias % 60 );
			OnLocalText( strReply );
			oNewMessage.Add( _T("* ") + m_pWords.GetAt( 0 ) + _T(" just TIMEed you."), m_sStatus, ID_COLOR_SERVERMSG );
			return;
		}
		case ID_MESSAGE_USER_CTCPVERSION:
		{
			CString strReply;
			strReply.Format( _T("/NOTICE %s :\x01VERSION %s:%s:Microsoft Windows\x01"),
				(LPCTSTR)m_pWords.GetAt( 0 ), _T(CLIENT_NAME),
				(LPCTSTR)theApp.m_sVersionLong );
			OnLocalText( strReply );
			oNewMessage.Add( _T("* ") + m_pWords.GetAt( 0 ) + _T(" just VERSIONed you."), m_sStatus, ID_COLOR_SERVERMSG );
			return;
		}
		case ID_MESSAGE_USER_CTCPBROWSE:
		{
			if ( Settings.Community.ServeFiles )
			{
				CString strReply;
				strReply.Format( _T("/NOTICE %s :\x01USERINFO :You can browse me by double-clicking on shareaza:browse:%s:%u\x01"),
					(LPCTSTR)m_pWords.GetAt( 0 ), (LPCTSTR)CString( inet_ntoa( Network.m_pHost.sin_addr ) ), htons( Network.m_pHost.sin_port ) );
				OnLocalText( strReply );
				oNewMessage.Add( _T("* ") + m_pWords.GetAt( 0 ) + _T(" just USERINFOed you."), m_sStatus, ID_COLOR_SERVERMSG );
			}
			return;
		}
		case ID_MESSAGE_SERVER_NOTICE:
		{
			oNewMessage.Add( GetStringAfterParsedItem ( FindParsedItem( _T(":"), 2 ) ) + _T(" (") + m_pWords.GetAt( 2 ) + _T(")"), m_sStatus, ID_COLOR_NOTICE );
			return;
		}
		case ID_MESSAGE_CHANNEL_LIST:
		{
			FillCountChanList( m_pWords.GetAt( 4 ), m_pWords.GetAt( 3 ) );
			return;
		}
		case ID_MESSAGE_CHANNEL_LISTEND:
		{
			CListCtrl* pChannelList = (CListCtrl*)&(m_wndPanel.m_boxChans.m_wndChanList);
			for ( int nList = 0 ; nList < pChannelList->GetItemCount() ; nList++ )
				if ( pChannelList->GetItemText( nList, 1 ) == _T("0") )
					pChannelList->DeleteItem( nList );
			return;
		}
		case ID_MESSAGE_CLIENT_JOIN_ENDNAMES:
		{
			m_wndPanel.m_boxUsers.UpdateCaptionCount();
			return;
		}
		case ID_MESSAGE_CLIENT_JOIN_USERLIST:
		{
			CString strChannelName = m_pWords.GetAt( 4 ), strTemp;
			int nMode, nWord, nModeColumn, nTab = m_wndTab.GetCurSel();
			for ( nWord = 6 ; nWord < m_pWords.GetCount() - 1 ; nWord++ )
			{
				strTemp = m_pWords.GetAt( nWord );
				nModeColumn = 0;
				nMode = 48;
				if ( strTemp.Left( 1 ) == _T("+") )
					nModeColumn = 1;
				else if ( strTemp.Left( 1 ) == _T("@") )
					nModeColumn = 2;
				else if ( strTemp.Left( 1 ) == _T("%") )
					nModeColumn = 4;
				nMode += nModeColumn;
				if ( nMode != 48 ) strTemp = strTemp.Mid( 1 );
				AddUser( m_pWords.GetAt( nWord ) );
				strTemp = char(nMode) + strTemp;
				//Add user to userlist
				m_pIrcUsersBuffer[ nTab ].Add( strTemp );
			}
			SortUserList();
			oNewMessage.Add( _T("* You have just joined the channel ") + strChannelName, strChannelName, ID_COLOR_CHANNELACTION );
			return;
		}
		case ID_MESSAGE_CHANNEL_TOPICSETBY:
		{
			CString strChannelName = m_pWords.GetAt( 3 );
			oNewMessage.Add( _T("* Topic set by ") + m_pWords.GetAt( 4 ), strChannelName, ID_COLOR_TOPIC );
			return;
		}
		case ID_MESSAGE_CHANNEL_TOPICSHOW:
		{
			CString strChannelName = m_pWords.GetAt( 3 );
			m_pContent.Clear();
			oNewMessage.Add( _T("* Topic is: ") + GetStringAfterParsedItem ( 4 ), strChannelName, ID_COLOR_TOPIC );
			return;
		}
		case ID_MESSAGE_CHANNEL_PART:
		{
			CString strNick( m_pWords.GetAt( 0 ) ), strChannelName( m_pWords.GetAt( 6 ) );
			int nTab = 	IsTabExist( strChannelName );
			if ( nTab != -1 )
			{
				int nListUser = FindInList( strNick );
				if ( nTab == m_wndTab.GetCurSel() && nListUser != -1 )
					DeleteUser( nListUser );
				nListUser = FindInList( strNick, 2, nTab );
				if ( nListUser != -1 )
					m_pIrcUsersBuffer[ nTab ].RemoveAt( nListUser );
				SortUserList();
				FillCountChanList( _T("-1"), strChannelName );
				oNewMessage.Add( _T("* ") + strNick + _T(" has parted ") + strChannelName, strChannelName, ID_COLOR_CHANNELACTION );
			}
			return;
		}
		case ID_MESSAGE_CHANNEL_PART_FORCED:
		{
			// Get the channel we're being force-parted from
			CString channelName = m_pWords.GetAt( 6 );
			// The status window is not a channel
			if ( channelName == m_sStatus ) { return; }
			// Get the tab number of the channel
			int nTab = IsTabExist( channelName );
			// If we're not in that channel, we can't actually leave it, so bail out
			if( nTab < 0 ) { return; }

			ClearUserList();
			m_pContent.Clear();

			// ???
			FillCountChanList( _T("-1"), channelName );
			// Remove the tab
			m_wndTab.DeleteItem( nTab );
			// Delete the contents of the tab
			m_pIrcBuffer[ nTab ].RemoveAll();
			m_pIrcUsersBuffer[ nTab ].RemoveAll();

			int oldTab = nTab;
			//m_pIrcBuffer This should be an expandable array.  Seriously.
			for ( ; nTab < m_nBufferCount - 1 ; nTab++ )
				m_pIrcBuffer[ nTab ].Append( m_pIrcBuffer[ nTab + 1 ] );

			m_nBufferCount--;

			// Set the tab to the left as the active tab. If there isn't one, set tab 0 as active
			if ( m_nSelectedTab != oldTab ) {
				if ( oldTab <= 1 )
					m_wndTab.SetCurSel( 0 );
				else
					m_wndTab.SetCurSel( oldTab - 1 );

				TabClick();
			}

			return;
		}
		case ID_MESSAGE_CHANNEL_QUIT:
		{
			CString strNick = m_pWords.GetAt( 0 );
			CString strUserMsg = GetStringAfterParsedItem ( 6 ) ;
			for ( int nTab = 1 ; nTab < m_nBufferCount ; nTab++ )
			{
				int nListUser = FindInList( strNick, 0, nTab );
				if ( nTab == m_wndTab.GetCurSel() && nListUser != -1 )
					DeleteUser( nListUser );

				CString strTabName = GetTabText( nTab );
				nListUser = FindInList( strNick, 2, nTab );
				if ( nListUser != -1 )
				{
					m_pIrcUsersBuffer[ nTab ].RemoveAt( nListUser );
					FillCountChanList( _T("-1"), strTabName );

					oNewMessage.Add( _T("* ") + strNick + _T(" has quit: (") + strUserMsg + _T(")"), strTabName, ID_COLOR_SERVERMSG );
				}
				else if ( strNick == strTabName )
				{
					oNewMessage.Add( _T("* ") + strNick + _T(" has quit: (") + strUserMsg + _T(")"), strTabName, ID_COLOR_SERVERMSG );
				}
			}
			SortUserList();
			return;
		}
		case ID_MESSAGE_CLIENT_JOIN:
		{
			CString strChannelName( m_pWords.GetAt( 7 ) );
			int nTab = 	AddTab( strChannelName, ID_KIND_CHANNEL );
			if (nTab==-1) return;
			m_wndTab.SetCurSel( nTab );
			ClearUserList();
			m_pIrcUsersBuffer[ nTab ].RemoveAll();
			m_pContent.Clear();
			FillCountChanList( _T("0"), strChannelName );
			return;
		}
		case ID_MESSAGE_CHANNEL_JOIN:
		{
			CString strNick( m_pWords.GetAt( 0 ) ), strChannelName( m_pWords.GetAt( 7 ) );
			int nTab = 	IsTabExist( strChannelName );
			if ( nTab == -1 ) return;
			m_pIrcUsersBuffer[ nTab ].Add( char( 48 ) + strNick );
			if ( nTab == m_wndTab.GetCurSel() )
				AddUser( strNick );
			FillCountChanList( _T("0"), strChannelName );
			SortUserList();
			oNewMessage.Add( _T("* ") + strNick + _T(" has joined ") + strChannelName, strChannelName, ID_COLOR_CHANNELACTION );
			return;
		}
		case ID_MESSAGE_SERVER_DISCONNECT:
		{
			OnStatusMessage( GetStringAfterParsedItem( FindParsedItem( _T(":"), 2 ) ), ID_COLOR_SERVERMSG );
			OnIrcDisconnect();
			return;
		}
		case ID_MESSAGE_CHANNEL_SETMODE:
		{
			CString strMode = m_pWords.GetAt( 7 );
			BOOL bSign = ( strMode.Left( 1 ) == _T("+") );
			int nCurNick = 8;
			for ( int nChar = 1 ; nChar < strMode.GetLength() ; nChar++ )
			{
				if ( m_pWords.GetCount() - 1 < nCurNick ) break;

				if ( strMode.Mid( nChar, 1 ) == _T("+") ||
					 strMode.Mid( nChar, 1 ) == _T("-") )
				{
					bSign = ( strMode.Mid( nChar, 1 ) == _T("+") );
					nCurNick--;
				}
				else if ( strMode.Mid( nChar, 1 ) == _T("o") ||
						  strMode.Mid( nChar, 1 ) == _T("v") ||
						  strMode.Mid( nChar, 1 ) == _T("h") )
				{
					int nSign = 1;

					if ( strMode.Mid( nChar, 1 ) == _T("o") )
						nSign = 2;
					else if ( strMode.Mid( nChar, 1 ) == _T("h") )
						nSign = 4;

					int nInUserList = FindInList( m_pWords.GetAt( nCurNick ) );
					if ( nInUserList <= -1 ) return;
					int nTab = IsTabExist( m_pWords.GetAt( 6 ) );
					int nInBufferList = FindInList( m_pWords.GetAt( nCurNick ), 2, nTab );
					if ( nInBufferList <= -1 ) return;

					CString strCurItem = m_pIrcUsersBuffer[nTab].GetAt( nInBufferList );
					int nMode = int(strCurItem.GetAt( 0 ) - 48);
					strCurItem = strCurItem.Mid( 1 );
					CString strCurUser( strCurItem );

					nMode = nMode + nSign * ( bSign ? 1 : -1 );
					if ( nMode == 2 || nMode == 3 || nMode == 6 || nMode == 7 )
						strCurUser = _T("@") + strCurUser;
					else if ( nMode == 1 )
						strCurUser = _T("+") + strCurUser;
					else if ( nMode == 4 || nMode == 5 )
						strCurUser = _T("%") + strCurUser;

					DeleteUser( nInUserList );
					AddUser( strCurUser );
					m_pIrcUsersBuffer[ nTab ].RemoveAt( nInBufferList );
					nMode += 48;
					m_pIrcUsersBuffer[ nTab ].Add( char(nMode) + strCurItem );
					nCurNick++;
				}
			}
			CString strMessage = m_pWords.GetAt( 0 );
			strMessage = strMessage + _T(" sets mode: ") + GetStringAfterParsedItem( 6 );
			SortUserList();
			oNewMessage.Add( strMessage, m_pWords.GetAt( 6 ), ID_COLOR_CHANNELACTION );
			return;
		}
		case ID_MESSAGE_NICK:
		{
			CString strNick = m_pWords.GetAt( 0 );
			CString strChannelName = GetTabText();
			CString strCurUser;
			int nListUser, nTab;
			for ( nTab = 0 ; nTab < m_wndTab.GetItemCount() ; nTab++ )
			{
				nListUser = FindInList( strNick, 2, nTab );
				if ( nListUser != -1 )
				{
					CString strCurItem = m_pIrcUsersBuffer[ nTab ].GetAt( nListUser );
					int nMode = int(strCurItem.GetAt( 0 ) - 48);
					strCurItem = strCurItem.Mid( 1 );
					strCurUser = m_pWords.GetAt( 7 );
					if ( nMode == 2 || nMode == 3 || nMode == 6 || nMode == 7 )
						strCurUser = _T("@") + m_pWords.GetAt( 7 );
					else if ( nMode == 1 )
						strCurUser = _T("+") + m_pWords.GetAt( 7 );
					else if ( nMode == 4 || nMode == 5 )
						strCurUser = _T("%") + m_pWords.GetAt( 7 );
					nMode += 48;
					m_pIrcUsersBuffer[ nTab ].SetAt( nListUser, char(nMode) + m_pWords.GetAt( 7 ) );
					break;
				}
			}

			if ( strNick.CompareNoCase( m_sNickname ) == 0 )
				m_sNickname = m_pWords.GetAt( 7 );
			nListUser = FindInList( strNick );
			if ( nListUser != -1 )
			{
				DeleteUser( nListUser );
				AddUser( strCurUser );
				oNewMessage.Add( _T("* ") + strNick + _T(" is now known as ") + m_pWords.GetAt( 7 ), strChannelName, ID_COLOR_CHANNELACTION );
			}
			return;
		}
	}
}

int CIRCFrame::ParseMessageID()
{
	int incomingWordCount = (int)m_pWords.GetCount();
	int nMessageType = NULL;
	if ( incomingWordCount > 1 )
	{
		int nServerErrNum = _tstoi( (LPCTSTR)m_pWords.GetAt( 1 ) );
		if ( nServerErrNum == 0 )
		{
			if ( m_pWords.GetAt( 0 ) == "PING" )
				nMessageType = ID_MESSAGE_SERVER_PING;
			else if ( m_pWords.GetAt( 0 ) == "ERROR" )
				nMessageType = ID_MESSAGE_SERVER_DISCONNECT;
			if ( m_pWords.GetAt( 1 ) == "NOTICE" )
				nMessageType = ID_MESSAGE_SERVER_NOTICE;
		}
		else
		{
			if ( nServerErrNum == 322 )
				nMessageType = ID_MESSAGE_CHANNEL_LIST;
			else if ( nServerErrNum == 323 )
				nMessageType = ID_MESSAGE_CHANNEL_LISTEND;
			else if ( nServerErrNum == 332 )
				nMessageType = ID_MESSAGE_CHANNEL_TOPICSHOW;
			else if ( nServerErrNum == 333 )
				nMessageType = ID_MESSAGE_CHANNEL_TOPICSETBY;
 			else if ( nServerErrNum == 353 )
				nMessageType = ID_MESSAGE_CLIENT_JOIN_USERLIST;
			else if ( nServerErrNum == 366 )
				nMessageType = ID_MESSAGE_CLIENT_JOIN_ENDNAMES;
			else if ( nServerErrNum == 376 )
				nMessageType = ID_MESSAGE_SERVER_CONNECTED;
			else if ( nServerErrNum > 0 && nServerErrNum < 5 )
				nMessageType = ID_MESSAGE_SERVER_MSG;
			else if ( nServerErrNum > 250 && nServerErrNum < 270 )
				nMessageType = ID_MESSAGE_SERVER_MSG;
			else if ( nServerErrNum == 372 || nServerErrNum == 375 )
				nMessageType = ID_MESSAGE_SERVER_MSG;
			else if ( nServerErrNum > 400 )
				nMessageType = ID_MESSAGE_SERVER_ERROR;
			else if ( nServerErrNum == 5 || nServerErrNum == 351 ||
				nServerErrNum == 369 || nServerErrNum == 342 ||
				nServerErrNum == 331 || nServerErrNum == 321 ||
				nServerErrNum == 317 || nServerErrNum == 318 ||
				( nServerErrNum > 299 && nServerErrNum < 304 ) )
				nMessageType = ID_MESSAGE_IGNORE;
			else if ( nServerErrNum == 301 )
				nMessageType = ID_MESSAGE_USER_AWAY;
			else if ( nServerErrNum == 305 )
				nMessageType = ID_MESSAGE_STOPAWAY;
			else if ( nServerErrNum == 306 )
				nMessageType = ID_MESSAGE_SETAWAY;
			else if ( nServerErrNum == 341 )
				nMessageType = ID_MESSAGE_CLIENT_INVITE;
			else if ( nServerErrNum == 314 )
				nMessageType = ID_MESSAGE_CLIENT_WHOWAS;
			else if ( nServerErrNum == 311 || nServerErrNum == 313 || nServerErrNum == 378 ||
					  nServerErrNum == 312 || nServerErrNum == 319 )
				nMessageType = ID_MESSAGE_CLIENT_WHOIS;
		}
	}
	if ( incomingWordCount > 6 )
	{
		const CString strMessage1( m_pWords.GetAt( 5 ) );
		const CString strMessage2( m_pWords.GetAt( 0 ) );

		if ( strMessage1 == "PART" && strMessage2 != m_sNickname )
			nMessageType = ID_MESSAGE_CHANNEL_PART;
		if ( strMessage1 == "PART" && strMessage2 == m_sNickname )
			nMessageType = ID_MESSAGE_CHANNEL_PART_FORCED;
		else if ( strMessage1 == "QUIT" && strMessage2 != m_sNickname )
			nMessageType = ID_MESSAGE_CHANNEL_QUIT;
		else if ( strMessage1 == "JOIN" )
		{
			if ( strMessage2 == m_sNickname )
				nMessageType = ID_MESSAGE_CLIENT_JOIN;
				else
				nMessageType = ID_MESSAGE_CHANNEL_JOIN;
		}
		else if ( strMessage1 == "MODE" )
			nMessageType = ID_MESSAGE_CHANNEL_SETMODE;
		else if ( strMessage1 == "NICK" )
			nMessageType = ID_MESSAGE_NICK;
		else if ( strMessage1 == "KICK" )
			nMessageType = ID_MESSAGE_USER_KICK;
	}
	if ( incomingWordCount > 8 )
	{
		const CString command( m_pWords.GetAt( 5 ) );
		const CString origin( m_pWords.GetAt( 0 ) );

		if ( command == "INVITE" )
			nMessageType = ID_MESSAGE_USER_INVITE;
		else if ( command == "NOTICE" && origin != m_sDestinationIP )
		{
			if ( m_pWords.GetAt( 6 ) == m_sNickname )
				nMessageType = ID_MESSAGE_CLIENT_NOTICE;
			else
				nMessageType = ID_MESSAGE_CHANNEL_NOTICE;
		}
		else if ( command == "PRIVMSG" )
		{
			CString str = m_pWords.GetAt( 8 );
			TCHAR pszFirst = str.GetAt( 0 );
			str = str.Mid( 1, str.GetLength() - 2 ).MakeLower();
			// 0x01 indicates a CTCP message, which includes '/me'
			if ( pszFirst == _T('\x01') )
			{
				if( m_pWords.GetAt( 6 ).CompareNoCase( m_sNickname ) == 0 )
				{
					if ( str == _T("version") )
						nMessageType = ID_MESSAGE_USER_CTCPVERSION;
					else if ( str == _T("userinfo") )
						nMessageType = ID_MESSAGE_USER_CTCPBROWSE;
					else if ( str == "time" )
						nMessageType = ID_MESSAGE_USER_CTCPTIME;
					else if ( str == "action" || str == "actio" )
						nMessageType = ID_MESSAGE_USER_ME;
				}
				else if ( str == "action" || str == "actio" )
				{
					nMessageType = ID_MESSAGE_CHANNEL_ME;
				}
			}
			else
			{
				if ( m_pWords.GetAt( 6 ) == m_sNickname )
				{
					nMessageType = ID_MESSAGE_USER_MESSAGE;
				}
				else
				{
					nMessageType = ID_MESSAGE_CHANNEL_MESSAGE;
				}
			}
		}
	}

	return nMessageType;
}

void CIRCFrame::ParseString(const CString& strMessage, CIRCNewMessage& oNewMessage)
{
	m_pWords.RemoveAll(); //Class-wide variable

	CStringArray incomingWords;

 	CString str, strNick;
	int nPos, nOldPos = -1;

	// Tokens in user ID, for e.g. nick!ident@domain.com
	int nFirstToken, nSecondToken, nThirdToken;
	if ( strMessage.GetAt( 0 ) == _T(':') ) nOldPos = 0;
	nPos = strMessage.Find( ' ' );
	nThirdToken = strMessage.ReverseFind( _T(':') );
	if ( nThirdToken == 0 ) nThirdToken = strMessage.GetLength() - 1;
	while ( nPos != -2 )
	{
		if ( nPos == -1 )
			str = strMessage.Mid( nOldPos + 1 );
		else
			str = strMessage.Mid( nOldPos + 1, nPos - nOldPos - 1 );
		nFirstToken = str.Find( _T('!'), 1 );
		nSecondToken = str.Find( _T('@'), 1 );
		if ( str.GetAt( 0 ) == _T(':') && nOldPos <= nThirdToken )
		{
			incomingWords.Add( _T(":") );
			str = str.Mid( 1 );
		}
		if ( nFirstToken != -1 && nSecondToken != -1 && nFirstToken < nSecondToken &&
			( nSecondToken < nThirdToken && nThirdToken != -1 || nThirdToken == -1 )
			  && nOldPos <= nThirdToken )
		{
			strNick = str.Mid( 0, nFirstToken );
			incomingWords.Add( strNick );
			incomingWords.Add( _T("!") );
			strNick = str.Mid( nFirstToken + 1, nSecondToken - nFirstToken - 1 );
			incomingWords.Add( strNick );
			incomingWords.Add( _T("@") );
			strNick = str.Mid( nSecondToken + 1 );
			incomingWords.Add( strNick );
		}
		else
			incomingWords.Add( str );
		if ( nPos == -1 ) break;
		nOldPos = nPos;
		nPos = strMessage.Find( _T(' '), nPos + 1 );
	}

	for( int index = 0; index < incomingWords.GetCount(); index++ ) {
		m_pWords.Add( incomingWords.GetAt( index ) );
	}

	int nMessageID = ParseMessageID();
	ActivateMessageByID( oNewMessage, nMessageID );

	// Drop messages if flood protection is running
	// Seems rather indiscriminate, plus useless since the message is already active
	if ( m_bFloodProtectionRunning && nMessageID != 0 )
	{
		if ( nMessageID > 203 )
		{
			oNewMessage.m_pMessages.RemoveAll();
		}
	}
}

int CIRCFrame::FindParsedItem(LPCTSTR szMessage, int nFirst)
{
	for ( int nItem = nFirst ; nItem < m_pWords.GetCount() - 1 ; nItem++ )
	{
		if ( m_pWords.GetAt( nItem ).Compare( szMessage ) == 0 ) return nItem;
	}
	return -1;
}

CString CIRCFrame::GetStringAfterParsedItem(int nItem) const
{
	CString strMessage;
	for ( int nWord = nItem + 1 ; nWord < m_pWords.GetCount() ; nWord++ )
		strMessage = strMessage + _T(" ") + m_pWords.GetAt( nWord );
	strMessage.Trim();
	return strMessage;
}

CString CIRCFrame::TrimString(CString strMessage) const
{
	// If "\x000A" exists as the first character, remove it from the string.
	if ( strMessage.GetAt( 0 ) == _T('\x000A') )
		strMessage = strMessage.Mid( 1 );

	// Go through each character in the message.
	// If the character is \x05BC, stop adding characters
	int nPos = strMessage.Find( _T('\x05BC') );
	return ( nPos == -1 ) ? strMessage : strMessage.Left( nPos );
}

int CIRCFrame::IsTabExist(const CString& strTabName) const
{
	for ( int nTab = 0 ; nTab < m_wndTab.GetItemCount() ; nTab++ )
	{
		if ( GetTabText( nTab ).CompareNoCase( strTabName ) == 0 )
			return nTab;
	}
	return -1;
}

void CIRCFrame::LoadBufferForWindow(int nTab)
{
	if ( nTab == -1 )
		return;
	m_pContent.Clear();
	CString str;
	int nFlag;
	for ( int nLine = 1 ; nLine < m_pIrcBuffer[ nTab ].GetCount() ; nLine++ )
	{
		str = m_pIrcBuffer[ nTab ].GetAt( nLine );
		nFlag = int( CHAR( str.GetAt( 0 ) ) );
		str = str.Mid( 1 );
		OnStatusMessage( str, nFlag );
		m_wndView.InvalidateIfModified();
	}
}

void CIRCFrame::OnClickTab(NMHDR* /* pNMHDR */, LRESULT* pResult)
{
	*pResult = 0;
	TabClick();
}

void CIRCFrame::SortUserList()
{
	// User list sorted automatically

	// Update caption
	m_wndPanel.m_boxUsers.UpdateCaptionCount();
}

void CIRCFrame::TabClick()
{
	m_pContent.Clear();

	m_wndView.InvalidateIfModified();

	ClearUserList();

	CString str;
	int nTab = m_wndTab.GetCurSel(), nMode;
	HighlightTab( nTab, FALSE );
	for ( int nUser = 0 ; nUser < m_pIrcUsersBuffer[ nTab ].GetCount() ; nUser++ )
	{
		str = m_pIrcUsersBuffer[ nTab ].GetAt( nUser );
		nMode = int( str.GetAt( 0 ) ) - 48;
		if ( nMode == 0 ) str = str.Mid( 1 );
		if ( nMode == 2 || nMode == 3 || nMode == 6 || nMode == 7 ) str = _T("@") + str.Mid( 1 );
		if ( nMode == 1 ) str = _T("+") + str.Mid( 1 );
		if ( nMode == 4 || nMode == 5 ) str = _T("%") + str.Mid( 1 );
		AddUser( str );
	}
	LoadBufferForWindow( nTab );

	SortUserList();

	RedrawWindow();
}

int CIRCFrame::AddTab(const CString& strTabName, int nKindOfTab)
{
	if ( m_wndTab.GetItemCount() + 1 == MAX_CHANNELS )
	{
		StatusMessage( _T("You have exceeded the maximum number of opened channels.") );
		return -1;
	}

	int m_nTab = IsTabExist( strTabName );
	if ( m_nTab != -1 )
		return -1;

	int nNewTab = m_wndTab.InsertItem( TCIF_TEXT | ( Settings.General.LanguageRTL ? TCIF_RTLREADING : 0 ),
		m_nBufferCount, strTabName, NULL, NULL );
	m_pIrcBuffer[ m_nBufferCount ].RemoveAll();
	m_pIrcUsersBuffer[ m_nBufferCount ].RemoveAll();
	m_pIrcBuffer[ m_nBufferCount ].Add( CString( (char)nKindOfTab ) );
	m_nCurrentPosLineBuffer[ m_nBufferCount ] = -1;
	m_nBufferCount++;
	HighlightTab( nNewTab, FALSE );
	return nNewTab;
}

// Events

void CIRCFrame::OnRichCursorMove(NMHDR* /* pNMHDR */, LRESULT* pResult)
{
	AFX_NOTIFY* pNotify = (AFX_NOTIFY*)pResult;

	CString strText = GetTextFromRichPoint();
	if ( strText.IsEmpty() )
		return;

	CShareazaURL oURL;
	if ( IsUserInList( strText ) != -1 ||				// User name
		strText.GetAt( 0 ) == _T('#') ||				// IRC channel
		( strText.GetLength() > 8 &&					// Common web address
		_tcsncicmp( strText, _T("www."), 4 ) == 0 ) ||
		oURL.Parse( strText, FALSE ) )					// URL
	{
		pNotify->pResult = (LRESULT*)theApp.LoadCursor( IDC_HAND );
	}
	else
	{
		pNotify->pResult = NULL;
	}
}

void CIRCFrame::OnRichDblClk(NMHDR* /* pNMHDR */, LRESULT* pResult)
{
	*pResult = 0;

	CString strText = GetTextFromRichPoint();
	if ( strText.IsEmpty() )
		return;

	CShareazaURL oURL;
	if ( IsUserInList( strText ) != -1 )				// User name
	{
		UserListDblClick();
	}
	else if ( strText.GetAt( 0 ) == _T('#') )			// IRC channel
	{
		SendString( _T("JOIN ") + strText );
	}
	else if ( strText.GetLength() > 8 &&				// Common web address
		_tcsncicmp( strText, _T("www."), 4 ) == 0 )
	{
		ShellExecute( GetSafeHwnd(), _T("open"), _T("http://") + strText, NULL, NULL, SW_SHOWNORMAL );
	}
	else if ( oURL.Parse( strText, FALSE ) )			// URL
	{
		if ( oURL.m_nAction == CShareazaURL::uriDownload && ! oURL.HasHash() )
			ShellExecute( GetSafeHwnd(), _T("open"), oURL.m_sURL, NULL, NULL, SW_SHOWNORMAL );
		else
			PostMainWndMessage( WM_URL, (WPARAM)new CShareazaURL( oURL ) );
	}
}

void CIRCFrame::OnRichClk(NMHDR* /* pNMHDR */, LRESULT* pResult)
{
	CString strText = GetTextFromRichPoint();
	if ( strText.IsEmpty() ) return;
	int nIndex = IsUserInList( strText );
	if ( nIndex != -1 )
	{
		SetSelectedUser( nIndex );
		m_wndPanel.m_boxUsers.m_wndUserList.PostMessage( WM_LBUTTONUP );
	}
	*pResult = 0;
}

// Operations

int CIRCFrame::FindInList(CString strName, int nList, int nTab)
{
	CListCtrl& wndChanList = m_wndPanel.m_boxChans.m_wndChanList;
	strName = RemoveModeOfNick( strName );

	INT_PTR nListCount = 0;
	if ( nList == 0 ) nListCount = GetUserCount();
	if ( nList == 1 ) nListCount = wndChanList.GetItemCount();
	if ( nList == 2 ) nListCount = m_pIrcUsersBuffer[ nTab ].GetCount();

	for ( int nItem = 0 ; nItem < nListCount ; nItem++ )
	{
		CString strNick;
		if ( nList == 0 ) strNick = GetUser( nItem );
		if ( nList == 1 ) strNick = wndChanList.GetItemText( nItem, 1 );
		if ( nList == 2 ) strNick = m_pIrcUsersBuffer[ nTab ].GetAt( nItem ).Mid( 1 );

		if ( RemoveModeOfNick( strNick ).CompareNoCase( strName ) == 0 )
			return nItem;
	}

	return -1;
}

CString CIRCFrame::GetTextFromRichPoint() const
{
	CPoint point;
	GetCursorPos( &point );
	return m_wndView.GetWordFromPoint( point, _T(" \r\n\t <>[](){}`\'\"!,;*") );
}

void CIRCFrame::UserListDblClick()
{
	CString strQueryUser = GetSelectedUser();
	if ( strQueryUser.IsEmpty() ) return;
	strQueryUser = RemoveModeOfNick( strQueryUser );
	int nTab = IsTabExist( strQueryUser );
	if ( nTab == -1 )
	{
		nTab = AddTab( strQueryUser, ID_KIND_PRIVATEMSG );
		if ( nTab == -1 ) return;
	}
	m_wndTab.SetCurSel( nTab ) ;
	TabClick();
}

CString CIRCFrame::RemoveModeOfNick(CString strNick) const
{
	return strNick.TrimLeft( _T("@%+~&") );
}

int CIRCFrame::IsUserInList(CString strUser) const
{
	strUser = RemoveModeOfNick( strUser );

	int nCount = GetUserCount() - 1;
	for ( int nUser = 0 ; nUser <= nCount ; nUser++ )
	{
		CString strNick = RemoveModeOfNick( GetUser( nUser ) );
		if ( strNick.CompareNoCase( strUser ) == 0 )
			return nUser;
	}
	return -1;
}


CIRCChannelList::CIRCChannelList()
	: m_nCountUserDefined( 0 )
	, m_nCount( 0 )
{
}

void CIRCChannelList::AddChannel(LPCTSTR strDisplayName, LPCTSTR strName, BOOL bUserDefined)
{
	m_bUserDefined.Add( bUserDefined );
	m_sChannelDisplayName.Add( strDisplayName );
	m_sChannelName.Add( strName );
	m_nCount++;
	if ( bUserDefined ) m_nCountUserDefined++;
}

int CIRCChannelList::GetCount(int nType) const
{
	if ( nType == -1 ) return m_nCount;
	if ( nType == 0  ) return m_nCount - m_nCountUserDefined;
	if ( nType == 1  ) return m_nCountUserDefined;
	return -1;
}
void CIRCChannelList::RemoveAll(int nType)
{
	if ( nType == -1 )
	{
		m_nCount = 0;
		m_nCountUserDefined = 0;
		m_bUserDefined.RemoveAll();
		m_sChannelDisplayName.RemoveAll();
		m_sChannelName.RemoveAll();
	}
	else
	{
		for ( int nChannel = 0 ; nChannel < m_bUserDefined.GetCount() ; nChannel++ )
		{
			if ( m_bUserDefined.GetAt( nChannel ) && nType == 1
				 || !m_bUserDefined.GetAt( nChannel ) && nType == 0 )
			{
					m_nCount--;
					if ( m_bUserDefined.GetAt( nChannel ) ) m_nCountUserDefined--;
					m_bUserDefined.RemoveAt( nChannel );
					m_sChannelDisplayName.RemoveAt( nChannel );
					m_sChannelName.RemoveAt( nChannel );
					nChannel--;
			}
		}
	}
}

BOOL CIRCChannelList::GetType(const CString& strDisplayName) const
{
	int nIndex = GetIndexOfDisplay( strDisplayName );
	if ( nIndex == -1 ) return FALSE;
	return m_bUserDefined.GetAt( nIndex );
}

void CIRCChannelList::RemoveChannel(const CString& strDisplayName)
{
	int nIndex = GetIndexOfDisplay( strDisplayName );
	if ( nIndex == -1 ) return;
	m_nCount--;
	if ( m_bUserDefined.GetAt( nIndex ) ) m_nCountUserDefined--;
	m_bUserDefined.RemoveAt( nIndex );
	m_sChannelDisplayName.RemoveAt( nIndex );
	m_sChannelName.RemoveAt( nIndex );
}

int CIRCChannelList::GetIndexOfDisplay(const CString& strDisplayName) const
{
	int nChannel, nCount = GetCount();
	BOOL bFoundChannel = FALSE;
	for ( nChannel = 0 ; nChannel < nCount ; nChannel++ )
	{
		if ( strDisplayName.CompareNoCase( GetDisplayOfIndex( nChannel ) ) == 0 )
		{
			bFoundChannel = TRUE;
			break;
		}
	}
	if ( !bFoundChannel ) return -1;
	return nChannel;
}

int CIRCChannelList::GetIndexOfName(const CString& strName) const
{
	int nChannel;
	int ChannelCount = GetCount();
	BOOL bFoundChannel = FALSE;
	for ( nChannel = 0 ; nChannel < ChannelCount ; nChannel++ )
	{
		if ( strName.CompareNoCase( GetNameOfIndex( nChannel ) ) == 0 )
		{
			bFoundChannel = TRUE;
			break;
		}
	}
	if ( !bFoundChannel ) return -1;
	return nChannel;
}

CString CIRCChannelList::GetDisplayOfIndex(int nIndex) const
{
	return m_sChannelDisplayName.GetAt( nIndex );
}

CString CIRCChannelList::GetNameOfIndex(int nIndex) const
{
	return m_sChannelName.GetAt( nIndex );
}
