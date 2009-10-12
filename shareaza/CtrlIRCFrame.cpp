//
// CtrlIRCFrame.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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

	CRect rectDefault;
	SetOwner( GetParent() );

	m_wndTab.Create( WS_CHILD | WS_VISIBLE | TCS_FLATBUTTONS | TCS_HOTTRACK | TCS_OWNERDRAWFIXED, 
		rectDefault, this, IDC_CHAT_TABS );

	FillChanList();
	m_wndView.Create( WS_CHILD|WS_VISIBLE, rectDefault, this, IDC_CHAT_TEXT );

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

	return 0;
}

// if strUserCount = 0 it will increase the current number
// if strUserCount = -1 it will decrease current number
// otherwise, it sets the number
void CIRCFrame::FillCountChanList(const CString& strUserCount, const CString& strChannelName)
{
	CString strCurrentChannel, strList, strUsers, strDisplay;
	BOOL bFound = FALSE;
	int nCount = _tstoi( strUserCount ), nList, nIndex, nCountWnd;
	CListCtrl* pChannelList = (CListCtrl*)&(m_wndPanel.m_boxChans.m_wndChanList);
	nIndex = m_pChanList.GetIndexOfName( strChannelName );
	if ( nIndex == -1 )
		return;

	strDisplay = m_pChanList.GetDisplayOfIndex( nIndex );
	for ( nList = 0 ; nList < pChannelList->GetItemCount() ; nList++ )
	{
		strList = pChannelList->GetItemText( nList, 0 );
		if ( strDisplay.CompareNoCase( strList ) == 0 )
		{
			bFound = TRUE;
			break;
		}
	}
	if ( bFound ) 
		strList = pChannelList->GetItemText( nList, 1 );
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
		nList = pChannelList->InsertItem( pChannelList->GetItemCount() , strDisplay );
	pChannelList->SetItemText( nList, 1, strCount );
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
	m_fntEdit.DeleteObject();
	m_pContent.m_fntNormal.DeleteObject();

	TEXTMETRIC txtMetric;
	int nHeight = 10;

	m_fntEdit.CreateFont( -13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, theApp.m_nFontQuality,
		DEFAULT_PITCH|FF_DONTCARE, Settings.Fonts.DefaultFont );

	// Find optimal font size values for the starting point
	// Code adjusted for the majority of fonts for different languages
	CDC* pDC = GetDC();
	
	LOGFONT lf = {};
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfQuality = theApp.m_nFontQuality;
	lstrcpy( lf.lfFaceName, Settings.IRC.ScreenFont );

	CFont pFont;
	pFont.CreateFontIndirect( &lf );
	pDC->SelectObject( pFont );
	pDC->GetTextMetrics( &txtMetric );

	// 19, 16 and 13 are Tahoma font measurements
	if ( txtMetric.tmInternalLeading )
	{
		if ( ( txtMetric.tmPitchAndFamily & TMPF_VECTOR ) &&
			 ( txtMetric.tmPitchAndFamily & TMPF_TRUETYPE ) || txtMetric.tmCharSet < 2 )
		{
			int nMainChar = txtMetric.tmAscent - txtMetric.tmInternalLeading;
			float nPercentage = (float)nMainChar / (float)txtMetric.tmAscent;
			if ( nPercentage < 0.45 )
			{
				nMainChar = txtMetric.tmInternalLeading;
				nPercentage = 1 - nPercentage;
			}
			if ( nPercentage < 0.55 )
				nHeight =  (int)( 10.0f * 19.0f / txtMetric.tmAscent + 0.5 );
			else if ( nPercentage > 0.85 )
				nHeight =  (int)( 10.0f * 19.0f / txtMetric.tmHeight + 0.44 );
			else if ( nPercentage > 0.69 )
				nHeight =  (int)( 10.0f * 13.0f / (float)nMainChar + 0.1 );
			else
				nHeight =  (int)( 10.0f * 13.0f / ( 16.0f / txtMetric.tmAscent * nMainChar ) + 0.5 );
		}
		else nHeight = 10;
	}
	else
	{
		if ( txtMetric.tmPitchAndFamily & TMPF_FIXED_PITCH )
			nHeight =  (int)( 10.0 * 16.0 / txtMetric.tmAscent + 0.5 );
		else
			nHeight = 11;
	}
	nHeight = -MulDiv( nHeight, GetDeviceCaps( pDC->m_hDC, LOGPIXELSY ), 72 );

	ReleaseDC( pDC );

	m_pContent.m_fntNormal.CreateFont( nHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, theApp.m_nFontQuality,
		DEFAULT_PITCH|FF_DONTCARE, Settings.IRC.ScreenFont );
	m_wndEdit.SetFont( &m_fntEdit, TRUE );
	m_wndTab.SetFont( &theApp.m_gdiFont, TRUE );
}

void CIRCFrame::OnDestroy() 
{
	OnIrcChanCmdSave();

	SendString( _T("QUIT") );

	KillTimer( 9 );
	KillTimer( 7 );

	m_pChanList.RemoveAll();

	closesocket( m_nSocket );
	m_nSocket = INVALID_SOCKET;

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
	rc.top    += IRCHEADER_HEIGHT;
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
	if ( Settings.IRC.Updated )
	{
		Settings.IRC.Updated = FALSE;
		OnSettings();
	}
	dc.SetTextColor( Skin.m_crBannerText );
	dc.SetBkMode( TRANSPARENT );

	CRect rcComponent;

	rcComponent.right = rcClient.right;
	rcComponent.left = rcClient.left + PANEL_WIDTH;
	rcComponent.top = rcClient.top;
	rcComponent.bottom = rcComponent.top + IRCHEADER_HEIGHT;
	PaintHeader( rcComponent, dc );
	rcComponent.DeflateRect( 14, 0 );
	CoolInterface.DrawEx( &dc, m_nHeaderIcon,
		CPoint( rcComponent.left + 4, rcComponent.top + 4 ), CSize( 48, 48 ),
		CLR_NONE, CLR_NONE, ILD_NORMAL, LVSIL_BIG );
	rcComponent.DeflateRect( 44, 0 );
	rcComponent.DeflateRect( 10, 12 );

	CString pszTitle;
	LoadString( pszTitle, IDS_IRC_PANEL_TITLE );
	CString pszSubtitle;
	LoadString( pszSubtitle, IDS_IRC_PANEL_SUBTITLE );
	dc.SelectObject( &CoolInterface.m_fntCaption );
	DrawText( &dc, rcComponent.left, rcComponent.top, pszTitle );
	rcComponent.DeflateRect( 0, 14 );
	dc.SelectObject( &CoolInterface.m_fntNormal );
	DrawText( &dc, rcComponent.left, rcComponent.top, pszSubtitle );

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
	m_sDestinationIP = Settings.IRC.ServerName;
	m_sDestinationPort = Settings.IRC.ServerPort;
	WORD nPort = (WORD)_tstoi( (LPCTSTR)m_sDestinationPort );
	m_sWsaBuffer.Empty();

	struct sockaddr_in dest_addr = {};   // will hold the destination addr
    dest_addr.sin_family = AF_INET;      // host byte order
	dest_addr.sin_port	= (u_short)ntohs( nPort ); // Copy the port number into the m_pHost structure
	m_nSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ; // do some error checking!
	if ( m_nSocket == INVALID_SOCKET )
	{
		OnStatusMessage( _T("Error: Cannot open a socket."), ID_COLOR_NOTICE );
	 	return;
	}
	
	struct hostent* host = gethostbyname( (LPCSTR)CT2A( m_sDestinationIP ) );
	if ( host == NULL ) return;

	memcpy( &( dest_addr.sin_addr.s_addr ), host->h_addr, sizeof(int) );
    memset( &( dest_addr.sin_zero ), '\0', 8 );  // zero the rest of the struct

	int RetVal = WSAConnect(
		m_nSocket, 					// Our socket
		(struct sockaddr *)&dest_addr,		// The remote IP address and port number
		sizeof(struct sockaddr),		// How many bytes the function can read
		NULL, NULL, NULL, NULL );	// No advanced features

	if ( RetVal == -1 ) return;
	m_sWsaBuffer.Empty();
	LoadString( m_sStatus, IDS_TIP_STATUS );
	m_sStatus.Remove( ':' );
	AddTab( m_sStatus, ID_KIND_CLIENT );
	m_nBufferCount = 1;
	if  ( RetVal == -1 )
	{
		int nError = WSAGetLastError();
		int nTargetWindow = IsTabExist( m_sStatus );
		if ( nTargetWindow == -1 ) return;
		if ( nError == WSAETIMEDOUT )
		{
			OnStatusMessage( _T("QUIT: Connection reset by peer."), ID_COLOR_NOTICE );
			m_pIrcBuffer[ nTargetWindow ].Add( char(ID_COLOR_NOTICE) + _T("QUIT: Connection reset by peer.") );
		}
		else if ( nError == WSAENOTCONN )
		{
			OnStatusMessage( _T("QUIT: Connection dropped."), ID_COLOR_NOTICE );
			m_pIrcBuffer[ nTargetWindow ].Add( char(ID_COLOR_NOTICE) + _T("QUIT: Connection dropped.") );
		}
		else 
		{
			OnStatusMessage( _T("QUIT: Connection closed."), ID_COLOR_NOTICE );
			m_pIrcBuffer[ nTargetWindow ].Add( char(ID_COLOR_NOTICE) + _T("QUIT: Connection closed.") );
		}
	 	return;
	}
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
 	CString strCommand = "NICK " + m_sNickname;
	SendString( strCommand );
	strCommand =  _T("USER ");
	strCommand += Settings.IRC.UserName + " ";
	strCommand += "razaUserHost ";
	strCommand += "razaUserServ :";
	strCommand += Settings.IRC.RealName;
	SendString( strCommand );

	WSAEventSelect( m_nSocket, m_pWakeup, FD_READ );
	m_nTimerVal = 0;
	OnStatusMessage( _T("Activating Connection..."), ID_COLOR_NOTICE );
	m_nMsgsInSec = 0;
	m_bConnected = TRUE;
	m_bFloodProtectionRunning = FALSE;

	SetTimer( 9, m_nUpdateFrequency, NULL );
	SetTimer( 7, m_nUpdateChanListFreq, NULL );
	m_pLastLineBuffer->RemoveAll();
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
	OnLocalText( "/whois " + RemoveModeOfNick( strText ) );
}

void CIRCFrame::OnIrcUserCmdOp() 
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	OnLocalText( "/mode " + GetTabText() + " +o " + RemoveModeOfNick( strText ) );
}

void CIRCFrame::OnIrcUserCmdDeop() 
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	OnLocalText( "/mode " + GetTabText() + " -o " + RemoveModeOfNick( strText ) );
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
			m_pChanList.AddChannel( strItem.Mid( 1 ), strItem, TRUE );
			m_wndPanel.m_boxChans.m_wndChanList.InsertItem( m_wndPanel.m_boxChans.m_wndChanList.GetItemCount(),
				strItem.Mid( 1 ) );
		}
	}
}

void CIRCFrame::OnIrcChanCmdSave()
{
	CString strFile, strPath, strItem;
	CFile pFile;
	strFile.Empty();
	int n_mpChanListIndex;
	CListCtrl* pChannelList = (CListCtrl*)&(m_wndPanel.m_boxChans.m_wndChanList);
	strPath = Settings.General.UserPath + _T("\\Data\\ChatChanlist.dat");
	for ( int nIndex = 0 ; nIndex < pChannelList->GetItemCount() ; nIndex++ )
	{
		n_mpChanListIndex = m_pChanList.GetIndexOfDisplay( pChannelList->GetItemText( nIndex, 0 ) );
		if ( n_mpChanListIndex != -1 && m_pChanList.GetType( m_pChanList.GetDisplayOfIndex( n_mpChanListIndex ) )  )
		{
			strItem = m_pChanList.GetNameOfIndex( n_mpChanListIndex );
			strFile += strItem + _T("\r\n");
		}
	}
	if ( ! pFile.Open( strPath, CFile::modeWrite|CFile::modeCreate ) ) return;

	CT2A pszFile( (LPCTSTR)strFile );

	pFile.Write( (LPCSTR)pszFile, (DWORD)strlen( pszFile ) );
	pFile.Close();
}

void CIRCFrame::OnIrcUserCmdVoice() 
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	OnLocalText( "/mode " + GetTabText() + " +v " + RemoveModeOfNick( strText ) );
}

void CIRCFrame::OnIrcShowSettings()
{
	CSettingsManagerDlg::Run( _T("CIRCSettingsPage") );
}

void CIRCFrame::OnIrcUserCmdBan() 
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	OnLocalText( "/mode " + GetTabText() + " +b " + RemoveModeOfNick( strText ) );
}

void CIRCFrame::OnIrcUserCmdUnban() 
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	OnLocalText( "/mode " + GetTabText() + " -b " + RemoveModeOfNick( strText ) );
}

void CIRCFrame::OnIrcUserCmdKick() 
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	OnLocalText( "/kick " + GetTabText() + " " + RemoveModeOfNick( strText ) );
}

void CIRCFrame::OnIrcUserCmdKickWhy() 
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	CIrcInputDlg dlg( this, 1, TRUE );	// 1 = select the second caption
	if ( dlg.DoModal() != IDOK ) return;
	OnLocalText( "/kick " + GetTabText() + " " + RemoveModeOfNick( strText ) + " " + dlg.m_sAnswer );
}

void CIRCFrame::OnIrcUserCmdBanKick() 
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	OnLocalText( "/mode " + GetTabText() + " +b " + RemoveModeOfNick( strText ) );
	OnLocalText( "/kick " + GetTabText() + " " + RemoveModeOfNick( strText ) );
}

void CIRCFrame::OnIrcUserCmdBanKickWhy() 
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	CIrcInputDlg dlg( this, 1, FALSE );	// 1 = select the second caption
	if ( dlg.DoModal() != IDOK ) return;
	OnLocalText( "/mode " + GetTabText() + " +b " + RemoveModeOfNick( strText ) );
	OnLocalText( "/kick " + GetTabText() + " " + RemoveModeOfNick( strText ) + " " + dlg.m_sAnswer );
}

void CIRCFrame::OnIrcUserCmdDevoice() 
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	OnLocalText( "/mode " + GetTabText() + " -v " + RemoveModeOfNick( strText ) );
}

void CIRCFrame::OnIrcUserCmdIgnore() 
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	OnLocalText( "/SILENCE +" + RemoveModeOfNick( strText ) );
}

void CIRCFrame::OnIrcUserCmdUnignore() 
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	OnLocalText( "/SILENCE -" + RemoveModeOfNick( strText ) );
}

void CIRCFrame::OnIrcUserCmdVersion() 
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	OnLocalText( "/PRIVMSG " + RemoveModeOfNick( strText ) + _T(" :\x01VERSION\x01") );
}

void CIRCFrame::OnIrcUserCmdTime() 
{
	CString strText = GetSelectedUser();
	if ( strText.IsEmpty() ) return;
	OnLocalText( "/PRIVMSG " + RemoveModeOfNick( strText ) + _T(" :\x01TIME\x01") );
}

void CIRCFrame::OnIrcCloseTab() 
{
	int nTab = m_wndTab.GetCurSel(), nOldTab( nTab );

	ClearUserList();
	m_pContent.Clear();

	CString strChannelName = GetTabText( nTab );
	FillCountChanList( "-1", strChannelName );
	if ( strChannelName == m_sStatus )
		return;

	m_wndTab.DeleteItem( nTab );
	m_pIrcBuffer[ nTab ].RemoveAll();
	m_pIrcUsersBuffer[ nTab ].RemoveAll();

	for ( ; nTab < m_nBufferCount - 1 ; nTab++ )
	{
		m_pIrcBuffer[ nTab ].Append( m_pIrcBuffer[ nTab + 1 ] );
		m_pIrcUsersBuffer[ nTab ].Append( m_pIrcUsersBuffer[ nTab + 1 ] );
	}
	if ( strChannelName.Left( 1 ) == "#" )
		SendString( "PART " + strChannelName );

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
	ClearUserList();
	m_wndPanel.m_boxChans.m_wndChanList.DeleteAllItems();

	OnStatusMessage( _T("Disconnected."), ID_COLOR_NOTICE );

	SendString( _T("QUIT") );

	closesocket( m_nSocket );
	m_nSocket = INVALID_SOCKET;

	KillTimer( 9 );
	KillTimer( 7 );

	m_wndTab.DeleteAllItems();

	for ( int nChannel = 0 ; nChannel < MAX_CHANNELS ; nChannel++ )
	{
		m_pIrcBuffer[ nChannel ].RemoveAll();
		m_nCurrentPosLineBuffer[ nChannel ] = -1;
		m_pIrcUsersBuffer[ nChannel ].RemoveAll();
		m_pLastLineBuffer[ nChannel ].RemoveAll();
	}
	m_nBufferCount = 0;

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
			strMessage = "/JOIN" + strMessage.Mid( 2 );
	}
	if ( strMessage.GetLength() > 3 ) 
	{
		if ( strMessage.Left( 4 ).CompareNoCase( _T("/bs ") ) == 0 )
			strMessage = "/BOTSERV" + strMessage.Mid( 3 );
		else if ( strMessage.Left( 4 ).CompareNoCase( _T("/ns ") ) == 0 ) 
			strMessage = "/NICKSERV" + strMessage.Mid( 3 );
		else if ( strMessage.Left( 4 ).CompareNoCase( _T("/cs ") ) == 0 ) 
			strMessage = "/CHANSERV" + strMessage.Mid( 3 );
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
		CTime pNow = CTime::GetCurrentTime();
		strDisp.Format( _T("[%.2i:%.2i:%.2i] "),
			pNow.GetHour(), pNow.GetMinute(), pNow.GetSecond() );
	}
		
	// Formatting
	if ( isActionMsg )
	{
		strDisp = strDisp + "* " + m_sNickname + " " + strMessage;
	}
	else if( strMessage.Left( 1 ).Compare( _T("/") ) != 0 ) 
	{
		strDisp = strDisp + "<" + m_sNickname + "> " + strMessage;
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
	else {
		// Not a command but in the status window?
		if ( strTabTitle.Compare( m_sStatus ) == 0 )
		{
			//TODO:
			//OnNewMessage( _T(":localhost 400 " + m_sNickname + " You are not currently in a channel") );
			OnNewMessage( _T(":localhost 400 You are currently not in a channel") );
			return;
		}
		else
		{
			// Display the line locally, in the appropriate color
			OnStatusMessage( strDisp, isActionMsg ? ID_COLOR_ME : ID_COLOR_TEXT );
			
			m_pIrcBuffer[ m_wndTab.GetCurSel() ].Add( strBufferMsg );
			
			if ( isActionMsg ) 
			{
				strSend = "\x01";
				strSend += _T("ACTION ");
				strSend += strMessage;
				strSend += "\x01";
			}
			else strSend = strMessage;
		
			strSend = "PRIVMSG " + strTabTitle + " :" + strSend;
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
		CListCtrl* pChannelList = (CListCtrl*)&(m_wndPanel.m_boxChans.m_wndChanList);
		for ( int nList = 0 ; nList < pChannelList->GetItemCount() ; nList++ )
			if ( !pChannelList->GetItemText( nList, 1 ).IsEmpty() )
				pChannelList->SetItemText( nList, 1, _T("0") );
		OnLocalText( _T("/list #shareaza*") );
		return;
	}
	// Stop flood protection
	if ( nIDEvent == 8 )
	{
		m_bFloodProtectionRunning = FALSE;
		int nTargetWindow = IsTabExist( m_sStatus ) ; 
		if ( m_wndTab.GetCurSel() == nTargetWindow ) 
			OnStatusMessage( _T("Stopped flood protection."), ID_COLOR_SERVERMSG );
		m_pIrcBuffer[ nTargetWindow ].Add( char(ID_COLOR_SERVERMSG) + _T("Stopped flood protection.") );
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
					m_pIrcBuffer[ m_wndTab.GetCurSel() ].Add( char(ID_COLOR_SERVERMSG ) + _T("Starting flood protection...") );
				OnStatusMessage( _T("Starting flood protection..."), ID_COLOR_SERVERMSG );
			}
			m_nMsgsInSec = 0;
		}

		if ( WaitForSingleObject( m_pWakeup, 0 ) == WAIT_OBJECT_0 )
		{
			auto_array< char > pszData( new char[ 4096 ] );
			int nRetVal = recv( m_nSocket, pszData.get(), 4094, 0 );
			if ( nRetVal > 0 )
				pszData[ nRetVal + 1 ] = '\0';
			CString strTmp = TrimString( m_sWsaBuffer + UTF8Decode( pszData.get() ) );
			m_sWsaBuffer.Empty();

			switch ( nRetVal )
			{
			case 0:
				OnStatusMessage( strTmp, ID_COLOR_NOTICE );
				OnIrcDisconnect();
				return;
			
			case -1:
				// Error
				KillTimer( 9 );
				if ( WSAGetLastError() == WSAETIMEDOUT )
					OnStatusMessage( _T("QUIT: Connection reset by peer." ), ID_COLOR_NOTICE );
				else if ( WSAGetLastError() == WSAENOTCONN )
					OnStatusMessage( _T("QUIT: Connection dropped."), ID_COLOR_NOTICE );
				else 
					OnStatusMessage( _T("QUIT: Server is busy, please try again in a minute."), ID_COLOR_NOTICE );
				OnIrcDisconnect();
				return;
			}

			if ( ! strTmp.IsEmpty() )
			{
				m_nMsgsInSec++;

				// If it's not a complete line, add it to the buffer until we get the rest
				int nIndex = strTmp.Find( _T("\r\n") );
				if ( nIndex == -1 )
				{
					m_sWsaBuffer = strTmp;
				}
				else
				{
					while ( nIndex != -1 && ! strTmp.IsEmpty() )
					{
						CString strMessage = strTmp.Left( nIndex );
						strMessage.TrimLeft();

						if ( ! strMessage.IsEmpty() )
							OnNewMessage( strMessage );

						strTmp = strTmp.Mid( nIndex + 2 );
						nIndex = strTmp.Find( _T("\r\n") );
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
	send( m_nSocket, (LPCSTR)strEncoded, strEncoded.GetLength(), 0 );
}

/* Displays messages on the client
 */
void CIRCFrame::OnStatusMessage(LPCTSTR pszText, int nFlags)
{
	CString strMessage = pszText; //The 
	COLORREF cRGB = Settings.IRC.Colors[ nFlags ];
	int nIndex = strMessage.Find( _T("\x03") ); //Find a color code indicator (Ctrl+K)
	int nSize;
	// While there are color codes, remove them.
	while ( nIndex != -1 )
	{
		nSize = 1;
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

	// Fit the text into the window -- The UI libraries handle this automatically?
	/*
	CRect rectView;
	m_wndView.GetWindowRect( &rectView );
	int nViewSize = int( rectView.Width() / 14 * 1.84 );
	int nCurrentChar, nOldChar = 0, nCurrentLength, nCheckLength;
	CString strMsgTemp, strCurrentWord;
	CStringArray pWordDivide;
	BOOL bStartedSplit = FALSE;

	// Divide the line into individual words
	nCurrentChar = strMessage.Find( ' ', nOldChar );
	strMessage.Trim();
	while ( nCurrentChar != -1 )
	{
		pWordDivide.Add( strMessage.Mid( nOldChar, nCurrentChar - nOldChar ) );
		nOldChar = nCurrentChar + 1;
		nCurrentChar = strMessage.Find( ' ', nOldChar );
	}
	pWordDivide.Add( strMessage.Mid( nOldChar ) );

	nCurrentLength = 0;
	strMsgTemp.Empty();

	for ( int nWord = 0 ; nWord < pWordDivide.GetCount() ; nWord++ )
	{
		strCurrentWord = pWordDivide.GetAt( nWord ).SpanExcluding( _T("\r\n") );
		nCurrentLength += strCurrentWord.GetLength();
		if ( nCurrentLength > nViewSize )
		{
			nCurrentLength = strMsgTemp.GetLength();
			nCheckLength = nCurrentLength + int( strCurrentWord.GetLength() * 0.3 );
			nOldChar = 0;
			nCurrentChar = 0;
			if ( strCurrentWord.GetLength() > nViewSize && bStartedSplit == TRUE )
			{
				bStartedSplit = FALSE;
				while ( nViewSize < strCurrentWord.GetLength() - nOldChar )
				{
					strMsgTemp += strCurrentWord.Mid( nOldChar, nViewSize );
					nOldChar += nViewSize;
					strMsgTemp = m_pszLineJoiner + strMsgTemp;
					m_pContent.Add( retText, strMsgTemp.GetBuffer(), NULL, retfColour )->m_cColour = cRGB;
					m_pContent.Add( retNewline, NEWLINE_FORMAT );
					strMsgTemp.Empty();
				}
				strMsgTemp = m_pszLineJoiner + strCurrentWord.Mid( nOldChar );
				nCurrentLength = strMsgTemp.GetLength();
			}
			else if  ( nCheckLength <= nViewSize )
			{
				strMsgTemp += strCurrentWord.Left( nViewSize - nCurrentLength );
				m_pContent.Add( retText, strMsgTemp.GetBuffer(), NULL, retfColour )->m_cColour = cRGB;
				m_pContent.Add( retNewline, NEWLINE_FORMAT );
				strMsgTemp.Empty();
				pWordDivide.SetAt( nWord, m_pszLineJoiner + strCurrentWord.Mid( nViewSize - nCurrentLength ) );
				nWord--;
				nCurrentLength = 0;
				bStartedSplit = TRUE;
				// Add chars up to 30% and the rest to the next line.
			}
			else if ( nCheckLength > nViewSize ) // Move to the next line.
			{
				m_pContent.Add( retText, strMsgTemp.GetBuffer(), NULL, retfColour )->m_cColour = cRGB;
				m_pContent.Add( retNewline, NEWLINE_FORMAT );
				nWord--;
				nCurrentLength = 0;
				strMsgTemp.Empty();
				bStartedSplit = TRUE;
			}
		}
		else
		{
			strMsgTemp += pWordDivide.GetAt( nWord ) + " ";
		}
	}
	// Display here what left.
	pWordDivide.RemoveAll();
	strMsgTemp.Trim();

	if ( !strMsgTemp.IsEmpty() )
	{
		m_pContent.Add( retText, strMsgTemp.GetBuffer(), NULL, retfColour )->m_cColour = cRGB ;
		m_pContent.Add( retNewline, NEWLINE_FORMAT );
	}
	*/
	
	
	m_pContent.Add( retText, strMessage.GetBuffer(), NULL, retfColour )->m_cColour = cRGB;
	m_pContent.Add( retNewline, NEWLINE_FORMAT );
	/*
	m_pIrcBuffer[ IsTabExist( m_sStatus ) ].Add( char(ID_COLOR_SERVERMSG) + strMessage );
	*/
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
		if ( pMsg->wParam == VK_UP && m_bConnected )
		{
			int nTab = m_wndTab.GetCurSel();
			if ( m_nCurrentPosLineBuffer[ nTab ] + 1 > m_pLastLineBuffer[ nTab ].GetCount() - 1 ) return TRUE;
			if ( m_nCurrentPosLineBuffer[ nTab ] < 10 ) m_nCurrentPosLineBuffer[ nTab ]++;
			m_wndEdit.SetWindowText( m_pLastLineBuffer[ nTab ].GetAt( m_nCurrentPosLineBuffer[ nTab ] ) );
			return TRUE;
		}
		if ( pMsg->wParam == VK_DOWN && m_bConnected )
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
		if ( pMsg->wParam == VK_RETURN )
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
			m_pLastLineBuffer[ nTab ].Add( m_sCurrent );
			if ( m_pLastLineBuffer[ nTab ].GetCount() > 10 )
				m_pLastLineBuffer[ nTab ].RemoveAt( 0 );
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
			int nItem = m_wndPanel.m_boxChans.m_wndChanList.GetNextItem( -1, LVNI_SELECTED );
			if ( nItem == -1 ) return TRUE;
			CString strItem = m_wndPanel.m_boxChans.m_wndChanList.GetItemText( nItem, 0 );
			if ( m_pChanList.GetType( strItem  ) )
				m_wndPanel.m_boxChans.m_wndChanList.DeleteItem( nItem );
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
			m_pChanList.AddChannel( strChan, "#" + strChan, TRUE );
			OnIrcChanCmdSave();
			return TRUE;
		}
	}

	return CWnd::PreTranslateMessage( pMsg );
}

void CIRCFrame::ChanListDblClick()
{
	int nItem = m_wndPanel.m_boxChans.m_wndChanList.GetNextItem( -1, LVNI_SELECTED );
	if ( nItem >= 0 )
	{
		CString strDisplay = m_wndPanel.m_boxChans.m_wndChanList.GetItemText( nItem, 0 );
		int nIndex = m_pChanList.GetIndexOfDisplay( strDisplay );
		if ( nIndex >= 0 )
		{
			CString strChannelName = m_pChanList.GetNameOfIndex( nIndex );
			int nTab = IsTabExist( strChannelName );
			if ( nTab < 0 )
			{
				// Join it
				SendString( _T("JOIN ") + strChannelName );
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
		if ( nTargetWindow != -1 ) 
		{
			CString strLine;
			if ( Settings.IRC.Timestamp )
			{
				CTime pNow = CTime::GetCurrentTime();
				strLine.Format( _T("[%.2i:%.2i:%.2i] "),
					pNow.GetHour(), pNow.GetMinute(), pNow.GetSecond() );
			}
			strLine += oNewMessage.m_pMessages[ nMessage ].sMessage;

			if ( m_wndTab.GetCurSel() == nTargetWindow )
				OnStatusMessage( strLine, nColorID );

			m_pIrcBuffer[ nTargetWindow ].Add( char( nColorID ) + strLine );
		}
	}

	return TRUE;
}


void CIRCFrame::ActivateMessageByID(CIRCNewMessage& oNewMessage, int nMessageType)
{
	switch ( nMessageType )
	{
		case NULL:
		{
			int nTab = IsTabExist( m_sStatus );
			if ( nTab != -1 && nTab != m_wndTab.GetCurSel() )
				m_wndTab.SetTabColor( nTab, Settings.IRC.Colors[ ID_COLOR_NEWMSG ] );
			oNewMessage.Add( GetStringAfterParsedItem( 0 ), m_sStatus, ID_COLOR_SERVERMSG );
			return;
		}
		case ID_MESSAGE_SERVER_MSG:
		{
			int nTab = IsTabExist( m_sStatus );
			if ( nTab != -1 && nTab != m_wndTab.GetCurSel() )
				m_wndTab.SetTabColor( nTab, Settings.IRC.Colors[ ID_COLOR_NEWMSG ] );
			oNewMessage.Add( GetStringAfterParsedItem( 3 ), m_sStatus, ID_COLOR_SERVERMSG );
			return;
		}
		case ID_MESSAGE_SERVER_ERROR:
		{
			int nTab = IsTabExist( m_sStatus );
			if ( nTab != -1 && nTab != m_wndTab.GetCurSel() )
				m_wndTab.SetTabColor( nTab, Settings.IRC.Colors[ ID_COLOR_NEWMSG ] );
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
			OnIrcChanCmdOpen();
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
				m_wndTab.SetTabColor( nTab, Settings.IRC.Colors[ ID_COLOR_NEWMSG ] );
			CString strSender = _T("<") + m_pWords.GetAt( 0 ) + _T("> ");
			CString strText = GetStringAfterParsedItem( 7 );

			if ( CMainWnd* pWnd = theApp.CShareazaApp::SafeMainWnd() )
			{
				if ( ! pWnd->IsForegroundWindow() )
					pWnd->ShowTrayPopup( strText, m_pWords.GetAt( 0 ), NIIF_NONE, 30 );
			}

			oNewMessage.Add( strSender + GetStringAfterParsedItem( 7 ), m_pWords.GetAt( 0 ), ID_COLOR_TEXT );
			return;
		}
		case ID_MESSAGE_USER_AWAY:
		{
			CString strSender = _T("* ") + m_pWords.GetAt( 3 ) + _T(" is away: ") + GetStringAfterParsedItem( 4 );
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
				m_wndTab.SetTabColor( nTab, Settings.IRC.Colors[ ID_COLOR_NEWMSG ] );
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
				m_wndTab.SetTabColor( m_nTab, Settings.IRC.Colors[ ID_COLOR_NEWMSG ] );
			CString strSender = _T("<") + m_pWords.GetAt( 0 ) + _T("> ");
			oNewMessage.Add( strSender + GetStringAfterParsedItem( 7 ), m_pWords.GetAt( 6 ), ID_COLOR_TEXT );
			return;
		}
		case ID_MESSAGE_CHANNEL_ME:
		{
			int m_nTab = IsTabExist( m_pWords.GetAt( 6 ) );
			if ( m_nTab == -1 ) return;
			if ( m_nTab != m_wndTab.GetCurSel() )
				m_wndTab.SetTabColor( m_nTab, Settings.IRC.Colors[ ID_COLOR_NEWMSG ] );
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
				m_pWords.GetAt( 0 ),
				time.Format( _T("%Y-%m-%d %H:%M:%S") ),
				- nTZBias / 60, nTZBias % 60 );
			OnLocalText( strReply );
			oNewMessage.Add( _T("* ") + m_pWords.GetAt( 0 ) + _T(" just TIMEed you."), m_sStatus, ID_COLOR_SERVERMSG );
			return;
		}
		case ID_MESSAGE_USER_CTCPVERSION:
		{
			CString strReply;
			strReply.Format( _T("/NOTICE %s :\x01VERSION %s:%s:Microsoft Windows %u.%u\x01"),
				m_pWords.GetAt( 0 ), _T(CLIENT_NAME), theApp.m_sVersionLong, theApp.m_nWindowsVersion, theApp.m_nWindowsVersionMinor );
			OnLocalText( strReply );
			oNewMessage.Add( _T("* ") + m_pWords.GetAt( 0 ) + _T(" just VERSIONed you."), m_sStatus, ID_COLOR_SERVERMSG );
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
			CString strChannelName = m_pWords.GetAt( 4 ), strTemp, nModeStr;
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
			FillCountChanList( "-1", channelName );
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
			CString strTmp;
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
	int incomingWordCount = m_pWords.GetCount();
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
			else if ( nServerErrNum == 5 || nServerErrNum == 318 || nServerErrNum == 351 || 
				nServerErrNum == 369 || nServerErrNum == 342 || 
				nServerErrNum == 331 || nServerErrNum == 321 || 
				nServerErrNum == 317 || nServerErrNum == 318 || 
					  nServerErrNum > 299  && nServerErrNum < 304 ) 
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
			char pszFirst = str.GetAt( 0 );
			str = str.Mid( 1, str.GetLength() - 2 ).MakeLower();
			// 0x01 indicates a CTCP message, which includes '/me'
			if ( pszFirst == char('\x01') )
			{
				if( m_pWords.GetAt( 6 ).CompareNoCase( m_sNickname ) == 0 )
				{
					if ( str.Compare(_T("version")) == 0 )
						nMessageType = ID_MESSAGE_USER_CTCPVERSION;
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
	int nIndex;
	CString strTmp;

	// If "\x000A" exists as the first character, remove it from the string.
	if ( strMessage.Find( _T("\x000A") ) == 0 )
		strMessage = strMessage.Mid( 1 );	
	
	// Go through each character in the message.
	// If the character is \x05BC, stop adding characters
	for( nIndex = 0 ; nIndex < strMessage.GetLength() - 1 ; nIndex++ )
	{
		if ( strMessage.Mid( nIndex, 1 ) == _T("\x05BC") )
			return strTmp;
		strTmp = strTmp + strMessage.Mid( nIndex, 1 );
	}
	return strTmp;
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
	m_wndTab.SetTabColor( nTab, RGB(0, 0, 0) );
	for ( int nUser = 0 ; nUser < m_pIrcUsersBuffer[ nTab ].GetCount() ; nUser++ )
	{
		str = m_pIrcUsersBuffer[ nTab ].GetAt( nUser );
		nMode = int( str.GetAt( 0 ) ) - 48;
		if ( nMode == 0 ) str = str.Mid( 1 );
		if ( nMode == 2 || nMode == 3 || nMode == 6 || nMode == 7 ) str = "@" + str.Mid( 1 );
		if ( nMode == 1 ) str = _T("+") + str.Mid( 1 );
		if ( nMode == 4 || nMode == 5 ) str = _T("%") + str.Mid( 1 );
		AddUser( str );
	}
	LoadBufferForWindow( nTab );
	SortUserList();
	this->RedrawWindow();
}

int CIRCFrame::AddTab(CString strTabName, int nKindOfTab)
{
	if ( m_wndTab.GetItemCount() + 1 == MAX_CHANNELS )
	{
		OnStatusMessage( _T("You have exceeded the maximum number of opened channels"), ID_COLOR_NOTICE );
		return -1;
	}

	int m_nTab = IsTabExist( strTabName );
	if ( m_nTab != -1 )
		return -1;

	int nNewTab = m_wndTab.InsertItem( TCIF_TEXT | ( Settings.General.LanguageRTL ? TCIF_RTLREADING : 0 ), 
		m_nBufferCount, strTabName, NULL, NULL );
	m_pIrcBuffer[ m_nBufferCount ].RemoveAll();
	m_pIrcUsersBuffer[ m_nBufferCount ].RemoveAll();
	m_pIrcBuffer[ m_nBufferCount ].Add( char( nKindOfTab ) );
	m_nCurrentPosLineBuffer[ m_nBufferCount ] = -1;
	m_nBufferCount++;
	m_wndTab.SetTabColor( nNewTab, RGB(0, 0, 0) );
	return nNewTab;
}

// Events

void CIRCFrame::OnRichCursorMove(NMHDR* /* pNMHDR */, LRESULT* pResult)
{
	AFX_NOTIFY* pNotify = (AFX_NOTIFY*)pResult;
	CString strText = GetTextFromRichPoint();
	if ( strText.IsEmpty() ) return;
	int nIndex = IsUserInList( strText );
	if ( nIndex != -1 ||
		strText.Left( 1 )   == _T("#") ||
		strText.Left( 7 )   == _T("http://") ||
		strText.Left( 4 )   == _T("www.") ||
		strText.Left( 7 )   == _T("magnet:") ||
		strText.Left( 4 )   == _T("ed2k:") ||
		strText.Mid( 1, 7 ) == _T("http://") ||
		strText.Mid( 1, 4 ) == _T("www.") )
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
	CString strText = GetTextFromRichPoint();
	if ( strText.IsEmpty() ) return;
	int nIndex = IsUserInList( strText );
	if ( nIndex != -1 ) 
	{
		UserListDblClick();
	}
	else if ( strText.Left( 1 ) == _T("#") ) 
	{
		CString strJoinChan = _T("JOIN ") + strText;
		SendString( strJoinChan );
	}
	else if ( strText.Left( 7 )   == _T("http://") ||
			  strText.Left( 4 )   == _T("www.") ||
			  strText.Mid( 1, 7 ) == _T("http://") ||
			  strText.Mid( 1, 4 ) == _T("www.") )	
	{
		if ( strText.Mid( 1, 7 ) == _T("http://") ||
			 strText.Mid( 1, 4 ) == _T("www.") )
			strText = strText.Mid( 1 );
		if ( strText.Right( 1 ) == _T(")") )
			strText = strText.Mid( 0, strText.GetLength() - 1 );
		theApp.InternalURI(	strText );
	}
	else if ( strText.Left( 7 )   == _T("magnet:") ||
			  strText.Left( 4 )   == _T("ed2k:") ||
			  strText.Mid( 1, 7 ) == _T("magnet:") ||
			  strText.Mid( 1, 4 ) == _T("ed2k:") )
	{
		if ( strText.Mid( 1, 7 ) == _T("magnet:") ||
			 strText.Mid( 1, 4 ) == _T("ed2k:") )
			strText = strText.Mid( 1 );
		if ( strText.Right( 1 ) == _T(")") )
			strText = strText.Mid( 0, strText.GetLength() - 1 );
		theApp.InternalURI(	strText );
	}
	*pResult = 0;
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

void CIRCFrame::OnSettings()
{
	m_pContent.m_crBackground = Settings.IRC.Colors[ ID_COLOR_CHATBACK ];
	SetFonts();
	m_wndView.SetDocument( &m_pContent );
	TabClick();
}

// Operations

int CIRCFrame::FindInList(CString strName, int nList, int nTab)
{
	strName = RemoveModeOfNick( strName );

	INT_PTR nListCount = 0;
	if ( nList == 0 ) nListCount = GetUserCount();
	if ( nList == 1 ) nListCount = m_wndPanel.m_boxChans.m_wndChanList.GetItemCount();
	if ( nList == 2 ) nListCount = m_pIrcUsersBuffer[ nTab ].GetCount();

	for ( int nItem = 0 ; nItem < nListCount ; nItem++ )
	{
		CString strNick;
		if ( nList == 0 ) strNick = GetUser( nItem );
		if ( nList == 1 ) strNick = m_wndPanel.m_boxChans.m_wndChanList.GetItemText( nItem, 1 );
		if ( nList == 2 ) strNick = m_pIrcUsersBuffer[ nTab ].GetAt( nItem ).Mid( 1 );

		if ( RemoveModeOfNick( strNick ).CompareNoCase( strName ) == 0 ) 
			return nItem;
	}

	return -1;
}

CString CIRCFrame::GetTextFromRichPoint()
{
	CRect rc, rc2;
	CPoint point;
	GetCursorPos( &point );
	m_wndView.GetWindowRect ( &rc2 );
	ScreenToClient( &rc2 );
	rc.left = point.x ;
	rc.top = point.y ;
	ScreenToClient( &rc );
	point.x = rc.left - rc2.left;
	point.y = rc.top - rc2.top;
	RICHPOSITION rp = m_wndView.PointToPosition( point );

	if ( rp.nFragment != -1 )
	{
		CRichFragment* pFragment = (CRichFragment*)m_wndView.m_pFragments.GetAt( rp.nFragment );
		CString strText = pFragment->m_pElement->m_sText;
		if ( rp.nOffset == 0 ) rp.nOffset++;
		CString strTemp = strText.Mid( 0, rp.nOffset );
		if ( strText.IsEmpty() ) return "";
		if ( strText.Mid( rp.nOffset, 1 ) == " " ) return "";
		int nLength  = strText.GetLength();
		int nStart   = strTemp.ReverseFind( ' ' );
		int nEnd     = strText.Find( ' ', rp.nOffset );
		INT_PTR nFragCnt = m_wndView.m_pFragments.GetCount();
		strTemp = strTemp.Mid( nStart == -1 ? 0 : nStart + 1 );
		if ( nEnd == -1 ) 
			strTemp += strText.Mid( rp.nOffset );
		else
			strTemp += strText.Mid( rp.nOffset, nEnd - rp.nOffset );
		int nFrag = rp.nFragment - 1;
		while ( nStart == -1 && nFrag != -1 ) 
		{
			if ( strTemp[ 0 ] != _T('\x200D') ) break;
			pFragment = (CRichFragment*)m_wndView.m_pFragments.GetAt( nFrag );
			strText = pFragment->m_pElement->m_sText;
			if ( strText.IsEmpty() ) break;
			strTemp = strTemp.Mid( 1 );
			nStart  = strText.ReverseFind( ' ' );
			strTemp = strText.Mid( nStart == -1 ? 0 : nStart + 1 ) + strTemp;
			nFrag--;
		}
		nFrag = rp.nFragment + 1;
		while ( ( nEnd == -1 || nEnd == nLength - 1 ) && nFragCnt != nFrag ) 
		{
			pFragment = (CRichFragment*)m_wndView.m_pFragments.GetAt( nFrag );
			strText = pFragment->m_pElement->m_sText;
			if ( strText.IsEmpty() ) break;
			if ( strText[ 0 ] != _T('\x200D') ) break;
			strText = strText.Mid( 1 );
			nEnd  = strText.Find( ' ' );
			strTemp += strText.Mid( nEnd == -1 ? 0 : nEnd );
			nFrag++;
		}
		if ( strTemp.GetLength() < 1 ) return "";
		if ( ( strTemp.Left( 1 ) == "<" && strTemp.Right( 1 ) == ">" ) ||
			 ( strTemp.Left( 1 ) == "-" && strTemp.Right( 1 ) == "-" ) )
			strTemp = strTemp.Mid( 1, strTemp.GetLength() - 2 );
		return strTemp;
	}
	return "";
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

BEGIN_MESSAGE_MAP(CIRCTabCtrl, CTabCtrl)
	ON_WM_CREATE()
//	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

CIRCTabCtrl::CIRCTabCtrl()
	: m_hTheme( NULL )
{
}

CIRCTabCtrl::~CIRCTabCtrl()
{
	if ( m_hTheme && theApp.m_pfnCloseThemeData )
		theApp.m_pfnCloseThemeData( m_hTheme );
}

int CIRCTabCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CTabCtrl::OnCreate( lpCreateStruct ) == -1 )
		return -1;

	if ( theApp.m_pfnOpenThemeData )
		m_hTheme = theApp.m_pfnOpenThemeData( m_hWnd, L"Tab" );

	return 0;
}

/*
BOOL CIRCTabCtrl::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	COLORREF m_cBorder = Settings.IRC.Colors[ ID_COLOR_TABS ];
	CBrush cbr = m_cBorder;
	pDC->GetWindow()->GetWindowRect( &rect );
	pDC->GetWindow()->ScreenToClient( &rect );
	pDC->SetBkMode( OPAQUE );
	pDC->SetBkColor( m_cBorder );
	pDC->FillRect( &rect, &cbr );
	return TRUE;
}
*/

HRESULT CIRCTabCtrl::DrawThemesPart(HDC dc, int nPartID, int nStateID, LPRECT prcBox)
{
	HRESULT hr = E_FAIL;
	if ( m_hTheme != NULL &&
		 theApp.m_pfnIsThemeActive && theApp.m_pfnIsThemeActive() &&
		 theApp.m_pfnDrawThemeBackground )
	{
		hr = theApp.m_pfnDrawThemeBackground( m_hTheme, dc, nPartID, nStateID, prcBox, NULL );
	}
	return hr;
}

void CIRCTabCtrl::DrawXPTabItem(HDC dc, int nItem, const RECT& rcItem, UINT flags)
{
	if ( m_hTheme == NULL ||
		 theApp.m_pfnIsThemeActive == NULL || ! theApp.m_pfnIsThemeActive() ||
		 theApp.m_pfnDrawThemeBackground == NULL )
	{
		COLORREF m_cBack = CoolInterface.m_crBackNormal;
		COLORREF m_cBorder = CoolInterface.m_crSysBtnFace;
		COLORREF m_cShadow = CoolInterface.m_crSys3DShadow;
		CBrush cback = m_cBack;
		CBrush cbr = m_cBorder;
		CBrush cbsh = m_cShadow;
		int oldMode = SetBkMode( dc, OPAQUE );
		SetBkColor( dc, m_cBorder );
		RECT rc = rcItem;
		FillRect( dc, &rc, (HBRUSH)cback.m_hObject );
		rc.left += 2;
		rc.top += 2;
		FillRect( dc, &rc, (HBRUSH)cbsh.m_hObject );
		rc.right -= 2;
		FillRect( dc, &rc, (HBRUSH)cbr.m_hObject );
		SetTextColor( dc, GetTabColor( nItem ) );

		TC_ITEM tci = { TCIF_TEXT | TCIF_IMAGE };
		TCHAR pszBuffer[ 128 + 4 ] = {};
		tci.pszText = pszBuffer;
		tci.cchTextMax = 127;
		if ( !TabCtrl_GetItem( m_hWnd, nItem, &tci ) ) return;
		HFONT oldFont = (HFONT)SelectObject( dc, (HFONT)SendMessage( WM_GETFONT, 0, 0 ) );
		DrawText( dc, pszBuffer, (int)_tcslen( pszBuffer ), &rc, DT_SINGLELINE | DT_VCENTER | DT_CENTER );
		SelectObject( dc, oldFont );
		SetBkMode( dc, oldMode );
		return;
	}

	BOOL bBody		= flags & paintBody;
	BOOL bSel		= flags & paintSelected;
	BOOL bHot		= flags & paintHotTrack;

	int nWidth = rcItem.right - rcItem.left;
	INT nHeight = rcItem.bottom - rcItem.top;

	// Draw the background
	HDC dcMem = CreateCompatibleDC( dc );
	HBITMAP bmpMem = CreateCompatibleBitmap( dc, nWidth, nHeight );
	ASSERT( dcMem != NULL );
	ASSERT( bmpMem != NULL );

	HBITMAP oldBmp = (HBITMAP)SelectObject( dcMem, bmpMem );
	RECT rcMem;
	SetRect( &rcMem, 0, 0, nWidth, nHeight );

	if ( bSel ) rcMem.bottom += 1;

	// TABP_PANE = 9, 0, "TAB"
	// TABP_TABITEM = 1, TIS_SELECTED = 3 : TIS_HOT = 2 : TIS_NORMAL = 1, "TAB"
	if ( bBody )
		DrawThemesPart( dcMem, 9, 0, &rcMem );
	else
		DrawThemesPart( dcMem, 1, bSel ? 3 : ( bHot ? 2 : 1 ), &rcMem );

	BITMAPINFO bmiOut = {};
	BITMAPINFOHEADER& bmihOut = bmiOut.bmiHeader;
	bmihOut.biSize = sizeof( BITMAPINFOHEADER );
	bmihOut.biCompression = BI_RGB;
	bmihOut.biPlanes = 1;
	bmihOut.biBitCount = 24;
	bmihOut.biWidth = nWidth;
	bmihOut.biHeight = nHeight;

	if ( nItem >= 0 )
	{
		if ( bSel )
			rcMem.bottom -= 1;
		DrawTabItem( dcMem, nItem, rcMem, flags );
	}

	// Blit image to the screen.
	BitBlt( dc, rcItem.left, rcItem.top, nWidth, nHeight, dcMem, 0, 0, SRCCOPY );
	SelectObject( dcMem, oldBmp );

	DeleteObject( bmpMem );
	DeleteDC( dcMem );
}

void CIRCTabCtrl::DrawTabItem(HDC dc, int nItem, const RECT& rcItem, UINT flags)
{
	TCHAR pszBuffer[ 128 + 4 ] = {};

	TC_ITEM item = { TCIF_TEXT | TCIF_IMAGE };
	item.pszText = pszBuffer;
	item.cchTextMax = 127;
	TabCtrl_GetItem( m_hWnd, nItem, &item );

	BOOL bSel = flags & paintSelected;

	RECT rc = rcItem;
	rc.bottom -= bSel ? 1 : 2;
	rc.left += 6;	// Text & icon.
	rc.top += 2 + bSel ? 1 : 3;

	int oldMode = SetBkMode( dc, TRANSPARENT );
	HIMAGELIST imageList = (HIMAGELIST)TabCtrl_GetImageList( m_hWnd );
	if ( imageList && item.iImage >= 0 )
	{
		ImageList_Draw( imageList, item.iImage, dc,
			rc.left + bSel ? 2 : 0,
			rc.top + bSel ? 0 : -2, ILD_TRANSPARENT );
		rc.left += 19;
	}
	else
		OffsetRect( &rc, -2, 2 );

	int nLen = (int)_tcslen( pszBuffer );
	if ( nLen > 0 )
	{
		HFONT oldFont = (HFONT)SelectObject( dc, (HFONT)SendMessage( WM_GETFONT, 0, 0 ) );
		rc.right -= 3;
		RECT r;
		SetRect( &r, 0, 0, rc.right - rc.left, 20 );

		SetTextColor( dc, GetTabColor( nItem ) );
		DrawText( dc, pszBuffer, nLen, &r, DT_CALCRECT | DT_SINGLELINE | DT_MODIFYSTRING | DT_END_ELLIPSIS );

		DrawText( dc, pszBuffer, nLen, &rc, DT_NOPREFIX | DT_CENTER );
		SelectObject( dc, oldFont );
	}
	SetBkMode( dc, oldMode );
}

void CIRCTabCtrl::SetTabColor(int nItem, COLORREF cRGB)
{
	TC_ITEM tci = { TCIF_PARAM };
	tci.lParam = cRGB;
	SetItem( nItem, &tci );
	RedrawWindow();
}

COLORREF CIRCTabCtrl::GetTabColor(int nItem)
{
	TC_ITEM tci = { TCIF_PARAM };
	GetItem( nItem, &tci );
	return tci.lParam;
}

BOOL CIRCTabCtrl::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_THEMECHANGED )
	{
		if ( m_hTheme && theApp.m_pfnCloseThemeData )
			theApp.m_pfnCloseThemeData( m_hTheme );

		if ( theApp.m_pfnOpenThemeData )
			m_hTheme = theApp.m_pfnOpenThemeData( m_hWnd, L"Tab" );

		return TRUE;
	}

	return CTabCtrl::PreTranslateMessage(pMsg);
}

LRESULT CIRCTabCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if ( message == WM_PAINT )
	{
		PAINTSTRUCT ps = {};
		if ( CDC* pDC = BeginPaint( &ps ) ) 
		{
			DrawTabControl( pDC );
		}
		EndPaint( &ps );
		return 0;
	} 

	return CTabCtrl::WindowProc(message, wParam, lParam);
}

void CIRCTabCtrl::DrawTabControl(CDC* pDC)
{
	// Paint the tab body.
	RECT rcPage, rcItem, rcClient;
	GetClientRect( &rcClient );
	rcPage = rcClient;
	TabCtrl_AdjustRect( m_hWnd, FALSE, &rcPage );
	rcClient.top = rcPage.top - 2;

	DrawXPTabItem( pDC->m_hDC, -1, rcClient, paintBody );

	int tabCount = TabCtrl_GetItemCount( m_hWnd );
	if( tabCount == 0 ) return;

	// Now paint inactive tabs.
	TCHITTESTINFO hti = {};
	GetCursorPos( &hti.pt );
	ScreenToClient( &hti.pt );
	int nHot = TabCtrl_HitTest( m_hWnd, &hti );
	int nSel = TabCtrl_GetCurSel( m_hWnd );

	for ( int nTab = 0; nTab < tabCount; nTab++ )
	{
		if ( nTab == nSel ) continue;
		TabCtrl_GetItemRect( m_hWnd, nTab, &rcItem );
		DrawXPTabItem( pDC->m_hDC, nTab, rcItem, nTab == nHot ? paintHotTrack : paintNone );
	}

	// Now paint the active selected tab.
	TabCtrl_GetItemRect( m_hWnd, nSel, &rcItem );
	InflateRect( &rcItem, 2, 2 );
	rcItem.bottom -= 1;
	DrawXPTabItem( pDC->m_hDC, nSel, rcItem, paintSelected );
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
