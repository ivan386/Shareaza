//
// CtrlIRCFrame.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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
// Author: peer_l_@hotmail.com
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
{
	if ( g_pIrcFrame == NULL ) g_pIrcFrame = this;
	m_nBufferCount			 = 0;
	m_nListWidth			= 170;
	m_nFloodingDelay		= 4000;
	m_nFloodLimit			= 0;
	m_nUpdateFrequency		= 40;
	m_nUpdateChanListFreq	= 100000;
	m_bConnected			= FALSE;
	m_nLocalTextLimit		= 300;
	m_nLocalLinesLimit		= 14;
	m_pszLineJoiner			= _T("\x200D");
	m_pTray					= ((CMainWnd*)AfxGetMainWnd())->m_pTray;
}

CIRCFrame::~CIRCFrame()
{
	if ( g_pIrcFrame == this ) g_pIrcFrame = NULL;
}

BOOL CIRCNewMessage::operator =(const CIRCNewMessage &rhs)
{
	m_sTargetName	= rhs.m_sTargetName;
	nColorID		= rhs.nColorID;
	m_pMessages.Copy( rhs.m_pMessages );
	return TRUE;
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
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;
	if ( ! m_wndPanel.Create( this ) ) return -1;
	CRect rectDefault;
	SetOwner( GetParent() );

	m_wndTab.Create( WS_CHILD | WS_VISIBLE | TCS_FLATBUTTONS | TCS_HOTTRACK | TCS_OWNERDRAWFIXED, 
		rectDefault, this, IDC_CHAT_TABS );

	FillChanList();
	m_wndView.Create( WS_CHILD|WS_VISIBLE, rectDefault, this, IDC_CHAT_TEXT );
	m_wndView.m_szSign = m_pszLineJoiner + '  ';

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
void CIRCFrame::FillCountChanList(CString strUserCount, CString strChannelName)
{
	CString strCurrentChannel, strList, strUsers, strDisplay;
	BOOL bFound = FALSE;
	int nCount = _tstoi( (LPCTSTR)strUserCount ), nList, nIndex, nCountWnd;
	CListCtrl* pChannelList = (CListCtrl*)&(m_wndPanel.m_boxChans.m_wndChanList);
	nIndex = m_pChanList.GetIndexOfName( strChannelName );
	if ( nIndex == -1 ) return;
	strDisplay = m_pChanList.GetDisplayOfIndex( nIndex );
	for ( nList = 0 ; nList < pChannelList->GetItemCount() ; nList++ )
	{
		strList = pChannelList->GetItemText( nList, 0 );
		if ( strDisplay.CompareNoCase( strList ) == 0 ) { bFound = TRUE; break; }
	}
	if ( bFound ) 
		strList = pChannelList->GetItemText( nList, 1 );
	else
		strList = strUserCount;
	nCountWnd = _tstoi( (LPCTSTR)strList );
	if ( strUserCount == "0"  ) nCountWnd++;
	else if ( strUserCount == "-1" ) nCountWnd--;
	else nCountWnd = nCount;
	strUserCount.Format( _T("%d"), nCountWnd );
	if ( !bFound ) 
		nList = pChannelList->InsertItem( pChannelList->GetItemCount() , strDisplay );
	pChannelList->SetItemText( nList, 1, strUserCount.GetBuffer() );
}
 
void CIRCFrame::FillChanList()
{
	m_pChanList.RemoveAll();
	m_pChanList.AddChannel( _T("^Support"), _T("#Shareaza") );
 	m_pChanList.AddChannel( _T("^Admins"), _T("#Shareaza-Admin") );
	m_pChanList.AddChannel( _T("^English"), _T("#Shareaza-Chat") );
	m_pChanList.AddChannel( _T("^Developers"), _T("#Shareaza-dev") );
 	m_pChanList.AddChannel( _T("Afrikaans"), _T("#Shareaza-Afrikaans") );
	m_pChanList.AddChannel( _T("Arabic"), _T("#Shareaza-Arabic") );
 	m_pChanList.AddChannel( _T("Chinese"), _T("#Shareaza-Chinese") );
	m_pChanList.AddChannel( _T("Croatian"), _T("#Shareaza-Croatian") );
 	m_pChanList.AddChannel( _T("Czech"), _T("#Shareaza-Czech") );
	m_pChanList.AddChannel( _T("Dutch"), _T("#Shareaza-Dutch") );
 	m_pChanList.AddChannel( _T("Finish"), _T("#Shareaza-Finish") );
	m_pChanList.AddChannel( _T("French"), _T("#Shareaza-French") );
 	m_pChanList.AddChannel( _T("German"), _T("#Shareaza-German") );
	m_pChanList.AddChannel( _T("Greek"), _T("#Shareaza-Greek") );
 	m_pChanList.AddChannel( _T("Hebrew"), _T("#Shareaza-Hebrew") );
	m_pChanList.AddChannel( _T("Hungarian"), _T("#Shareaza-Hungarian") );
 	m_pChanList.AddChannel( _T("Italian"), _T("#Shareaza-Italian") );
 	m_pChanList.AddChannel( _T("Japanese"), _T("#Shareaza-Japanese") );
	m_pChanList.AddChannel( _T("Lithuanian"), _T("#Shareaza-Lithuanian") );
 	m_pChanList.AddChannel( _T("Norwegian"), _T("#Shareaza-Norwegian") );
	m_pChanList.AddChannel( _T("Polish"), _T("#Shareaza-Polish") );
 	m_pChanList.AddChannel( _T("Portuguese"), _T("#Shareaza-Portuguese") );
	m_pChanList.AddChannel( _T("Russian"), _T("#Shareaza-Russian") );
 	m_pChanList.AddChannel( _T("Spain"), _T("#Shareaza-Spain") );
 	m_pChanList.AddChannel( _T("Swedish"), _T("#Shareaza-Swedish") );
}
 
void CIRCFrame::SetFonts()
{
	m_fntEdit.DeleteObject();
	m_pContent.m_fntNormal.DeleteObject();

	TEXTMETRIC txtMetric;
	int nHeight = 10;

	m_fntEdit.CreateFont( -13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, theApp.m_sDefaultFont );

	// Find optimal font size values for the starting point
	// Code adjusted for the majority of fonts for different languages
	CDC* pDC = GetDC();
	
	LOGFONT lf;
	memset( &lf, 0, sizeof(LOGFONT) );
	lf.lfCharSet = DEFAULT_CHARSET;
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
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, Settings.IRC.ScreenFont );
	m_wndEdit.SetFont( &m_fntEdit, TRUE );
	m_wndTab.SetFont( &theApp.m_gdiFont, TRUE );
}

void CIRCFrame::OnDestroy() 
{
	OnIrcChanCmdSave();
	SendString( _T("/QUIT") );
	KillTimer( 9 );
	KillTimer( 7 );
	m_pChanList.RemoveAll();
	closesocket( m_nSocket );
	CWnd::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CIRCFrame presentation message handlers

void CIRCFrame::OnSkinChange()
{
	m_wndPanel.OnSkinChange();

	Skin.CreateToolBar( _T("CMainIrcBar"), &m_wndMainBar );
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
	m_wndTab.SetWindowPos( NULL, rc.left, rc.bottom - TABBAR_HEIGHT,
		rc.Width(), TABBAR_HEIGHT, SWP_NOZORDER|SWP_SHOWWINDOW );
	rc.bottom -= TABBAR_HEIGHT;
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
		TABBAR_HEIGHT - SMALLHEADER_HEIGHT;
	rcComponent.bottom = rcComponent.top + SMALLHEADER_HEIGHT;
	PaintHeader( rcComponent, dc );
	rcComponent.DeflateRect( 10, 4 );
	dc.SelectObject( &CoolInterface.m_fntCaption );
	DrawText( &dc, rcComponent.left, rcComponent.top, _T("Chat!") );

	rcComponent.right = rcClient.right;
	rcComponent.left = rcClient.left + PANEL_WIDTH;
	rcComponent.top = rcClient.bottom - TOOLBAR_HEIGHT - EDITBOX_HEIGHT - 
		TABBAR_HEIGHT - SMALLHEADER_HEIGHT - SEPERATOR_HEIGHT;
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
			m_wndPanel.m_boxUsers.m_wndUserList.SetCurSel( nIndex );
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
	struct hostent* host;
	WSADATA WsaData;
	m_sDestinationIP = Settings.IRC.ServerName;
	m_sDestinationPort = Settings.IRC.ServerPort;
	WORD nPort = (WORD)_tstoi( (LPCTSTR)m_sDestinationPort );
	m_sWsaBuffer.Empty();
	if ( WSAStartup( MAKEWORD( 1, 1 ), &WsaData ) != 0 ) return;

    struct sockaddr_in dest_addr;   // will hold the destination addr
    dest_addr.sin_family = AF_INET;          // host byte order
	dest_addr.sin_port	= (u_short)ntohs( nPort ); // Copy the port number into the m_pHost structure
	m_nSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ; // do some error checking!

	if ( m_nSocket == INVALID_SOCKET )
	{
		OnStatusMessage( _T("Error: Cannot open a socket."), ID_COLOR_NOTICE );
	 	return;
	}
	int nBytes = WideCharToMultiByte( CP_ACP, 0, m_sDestinationIP, m_sDestinationIP.GetLength(), 
		NULL, 0, NULL, NULL );
	LPSTR pBytes = new CHAR[ nBytes + 1 ];
	WideCharToMultiByte( CP_ACP, 0, m_sDestinationIP, m_sDestinationIP.GetLength(), pBytes, nBytes, NULL, NULL );
	pBytes[nBytes] = '\0';
	
	host = gethostbyname( pBytes );
	delete [] pBytes;
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
			OnStatusMessage( _T("QUIT: Connection Reset By PEER."), ID_COLOR_NOTICE );
			m_pIrcBuffer[ nTargetWindow ].Add( char(ID_COLOR_NOTICE) + _T("QUIT: Connection Reset By PEER.") );
		}
		else if ( nError == WSAENOTCONN )
		{
			OnStatusMessage( _T("QUIT: Connection Dropped."), ID_COLOR_NOTICE );
						m_pIrcBuffer[ nTargetWindow ].Add( char(ID_COLOR_NOTICE) + _T("QUIT: Connection Dropped.") );
		}
		else 
		{
			OnStatusMessage( _T("QUIT: Connection Closed."), ID_COLOR_NOTICE );
			m_pIrcBuffer[ nTargetWindow ].Add( char(ID_COLOR_NOTICE) + _T("QUIT: Connection Closed.") );
		}
	 	return;
	}
	m_sNickname	= Settings.IRC.Nick;
	if ( m_sNickname.IsEmpty() ) 
	{
		CString strNick = MyProfile.GetNick();
		if ( strNick.IsEmpty() )
		{
			CString strRandomNumber;
			m_sNickname = "razaIrc";
			srand( (unsigned)time( NULL ) );
			for ( int nDigit = 0 ; nDigit < 7 ; nDigit++ )
			{
				strRandomNumber.Format( L"%d", rand() );
				m_sNickname += strRandomNumber.GetAt( 0 );
			}
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
	int nItem = m_wndPanel.m_boxUsers.m_wndUserList.GetCurSel();
	if ( nItem < 0 ) return;
	CString strText;
	m_wndPanel.m_boxUsers.m_wndUserList.GetText( nItem, strText );
	OnLocalText( "/whois " + RemoveModeOfNick( strText ) );
}

void CIRCFrame::OnIrcUserCmdOp() 
{
	int nItem = m_wndPanel.m_boxUsers.m_wndUserList.GetCurSel();
	if ( nItem < 0 ) return;
	CString strText2;
	m_wndPanel.m_boxUsers.m_wndUserList.GetText( nItem, strText2 );
	CString strText = "/mode " + GetTabText() + " +o " + RemoveModeOfNick( strText2 );
	OnLocalText( strText );
}

void CIRCFrame::OnIrcUserCmdDeop() 
{
	int nItem = m_wndPanel.m_boxUsers.m_wndUserList.GetCurSel();
	if ( nItem < 0 ) return;
	CString strText2;
	m_wndPanel.m_boxUsers.m_wndUserList.GetText( nItem, strText2 );
	CString strText = "/mode " + GetTabText() + " -o " + RemoveModeOfNick( strText2 );
	OnLocalText( strText );
}

void CIRCFrame::OnIrcChanCmdOpen()
{
	CString strPath, strItem;
	CFile pFile;
	
	strPath = Settings.General.UserPath + _T("\\Data\\ChatChanlist.dat");
	
	if ( ! pFile.Open( strPath, CFile::modeRead ) ) return;
	
	CBuffer pBuffer;
	pBuffer.EnsureBuffer( (DWORD)pFile.GetLength() );
	pBuffer.m_nLength = (DWORD)pFile.GetLength();
	pFile.Read( pBuffer.m_pBuffer, pBuffer.m_nLength );
	pFile.Close();
	
	while ( pBuffer.ReadLine( strItem ) )
	{
		strItem.TrimLeft();
		strItem.TrimRight();

		if ( strItem.GetLength() && strItem.GetAt( 0 ) == '#' )
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

	USES_CONVERSION;
	LPCSTR pszFile = T2CA( (LPCTSTR)strFile );
	
	pFile.Write( pszFile, (DWORD)strlen( pszFile ) );
	pFile.Close();
}

void CIRCFrame::OnIrcUserCmdVoice() 
{
	int nItem = m_wndPanel.m_boxUsers.m_wndUserList.GetCurSel();
	if ( nItem < 0 ) return;
	CString strText2;
	m_wndPanel.m_boxUsers.m_wndUserList.GetText( nItem, strText2 );
	CString strText = "/mode " + GetTabText() + " +v " + RemoveModeOfNick( strText2 );
	OnLocalText( strText );
}

void CIRCFrame::OnIrcShowSettings()
{
	CSettingsManagerDlg::Run( _T("CIRCSettingsPage") );
}

void CIRCFrame::OnIrcUserCmdBan() 
{
	int nItem = m_wndPanel.m_boxUsers.m_wndUserList.GetCurSel();
	if ( nItem < 0 ) return;
	CString strText2;
	m_wndPanel.m_boxUsers.m_wndUserList.GetText( nItem, strText2 );
	CString strText = "/mode " + GetTabText() + " +b " + RemoveModeOfNick( strText2 );
	OnLocalText( strText );
}
void CIRCFrame::OnIrcUserCmdUnban() 
{
	int nItem = m_wndPanel.m_boxUsers.m_wndUserList.GetCurSel();
	if ( nItem < 0 ) return;
	CString strText2;
	m_wndPanel.m_boxUsers.m_wndUserList.GetText( nItem, strText2 );
	CString strText = "/mode " + GetTabText() + " -b " + RemoveModeOfNick( strText2 );
	OnLocalText( strText );
}

void CIRCFrame::OnIrcUserCmdKick() 
{
	int nItem = m_wndPanel.m_boxUsers.m_wndUserList.GetCurSel();
	if ( nItem < 0 ) return;
	CString strText2;
	m_wndPanel.m_boxUsers.m_wndUserList.GetText( nItem, strText2 );
	CString strText = "/kick " + GetTabText() + " " + RemoveModeOfNick( strText2 );
	OnLocalText( strText );
}

void CIRCFrame::OnIrcUserCmdKickWhy() 
{
	int nItem = m_wndPanel.m_boxUsers.m_wndUserList.GetCurSel();
	if ( nItem < 0 ) return;
	CIrcInputDlg dlg( this, 1, TRUE );	// 1 = select the second caption
	if ( dlg.DoModal() != IDOK ) return;
	CString strReason = dlg.m_sAnswer;
	if ( strReason.IsEmpty() ) return;
	CString strText2;
	m_wndPanel.m_boxUsers.m_wndUserList.GetText( nItem, strText2 );
	CString strText = "/kick " + GetTabText() + " " + RemoveModeOfNick( strText2 ) + " " + strReason;
	OnLocalText( strText );
}

void CIRCFrame::OnIrcUserCmdBanKick() 
{
	int nItem = m_wndPanel.m_boxUsers.m_wndUserList.GetCurSel();
	if ( nItem < 0 ) return;
	CString strText2;
	m_wndPanel.m_boxUsers.m_wndUserList.GetText( nItem, strText2 );
	CString strText = "/mode " + GetTabText() + " +b " + RemoveModeOfNick( strText2 );
	OnLocalText( strText );
	strText = "/kick " + GetTabText() + " " + RemoveModeOfNick( strText2 );
	OnLocalText( strText );
}

void CIRCFrame::OnIrcUserCmdBanKickWhy() 
{
	int nItem = m_wndPanel.m_boxUsers.m_wndUserList.GetCurSel();
	if ( nItem < 0 ) return;
	CIrcInputDlg dlg( this, 1, FALSE );	// 1 = select the second caption
	if ( dlg.DoModal() != IDOK ) return;
	CString strReason = dlg.m_sAnswer;
	if ( strReason.IsEmpty() ) return;
	CString strText2;
	m_wndPanel.m_boxUsers.m_wndUserList.GetText( nItem, strText2 );
	CString strText = "/mode " + GetTabText() + " +b " + RemoveModeOfNick( strText2 );
	OnLocalText( strText );
	strText = "/kick " + GetTabText() + " " + RemoveModeOfNick( strText2 ) + " " + strReason;
	OnLocalText( strText );
}

void CIRCFrame::OnIrcUserCmdDevoice() 
{
	int nItem = m_wndPanel.m_boxUsers.m_wndUserList.GetCurSel();
	if ( nItem < 0 ) return;
	CString strText2;
	m_wndPanel.m_boxUsers.m_wndUserList.GetText( nItem, strText2 );
	CString strText = "/mode " + GetTabText() + " -v " + RemoveModeOfNick( strText2 );
	OnLocalText( strText );
}

void CIRCFrame::OnIrcUserCmdIgnore() 
{
	int nItem = m_wndPanel.m_boxUsers.m_wndUserList.GetCurSel();
	if ( nItem < 0 ) return;
	CString strText2;
	m_wndPanel.m_boxUsers.m_wndUserList.GetText( nItem, strText2 );
	CString strText = "/SILENCE +" + RemoveModeOfNick( strText2 );
	OnLocalText( strText );
}

void CIRCFrame::OnIrcUserCmdUnignore() 
{
	int nItem = m_wndPanel.m_boxUsers.m_wndUserList.GetCurSel();
	if ( nItem < 0 ) return;
	CString strText2;
	m_wndPanel.m_boxUsers.m_wndUserList.GetText( nItem, strText2 );
	CString strText = "/SILENCE -" + RemoveModeOfNick( strText2 );
	OnLocalText( strText );
}

void CIRCFrame::OnIrcUserCmdVersion() 
{
	int nItem = m_wndPanel.m_boxUsers.m_wndUserList.GetCurSel();
	if ( nItem < 0 ) return;
	CString strText2;
	m_wndPanel.m_boxUsers.m_wndUserList.GetText( nItem, strText2 );
	CString strText = "/PRIVMSG " + RemoveModeOfNick( strText2 );
	strText = strText + " " + char(0x1);
	strText = strText + _T("VERSION");
	strText = strText + char(0x1) + " ";
	OnLocalText( strText );
}

void CIRCFrame::OnIrcUserCmdTime() 
{
	int nItem = m_wndPanel.m_boxUsers.m_wndUserList.GetCurSel();
	if ( nItem < 0 ) return;
	CString strText2;
	m_wndPanel.m_boxUsers.m_wndUserList.GetText( nItem, strText2 );
	CString strText = "/PRIVMSG " + RemoveModeOfNick( strText2 );
	strText = strText + " " + char( 0x1 );
	strText = strText + _T("TIME");
	strText = strText + char(0x1) + " ";
	if ( nItem >= 0 ) OnLocalText( strText );
}

void CIRCFrame::OnIrcCloseTab() 
{
	int nTab = m_wndTab.GetCurSel(), nOldTab( nTab );

	m_wndPanel.m_boxUsers.m_wndUserList.ResetContent();
	m_pContent.Clear();

	CString strChannelName = GetTabText( nTab );
	FillCountChanList( "-1", strChannelName );
	if ( strChannelName == m_sStatus ) return;
	m_wndTab.DeleteItem( nTab );
	m_pIrcBuffer[ nTab ].RemoveAll();
	m_pIrcUsersBuffer[ nTab ].RemoveAll();

	for ( ; nTab < m_nBufferCount - 1 ; nTab++ )
		m_pIrcBuffer[ nTab ].Append( m_pIrcBuffer[ nTab + 1 ] );
	if ( strChannelName.Left( 1 ) == "#" ) SendString( "PART " + strChannelName );

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
	m_wndPanel.m_boxUsers.m_wndUserList.ResetContent();
	m_wndPanel.m_boxChans.m_wndChanList.DeleteAllItems();
	OnStatusMessage( _T("Disconnected."), ID_COLOR_NOTICE );
	SendString( _T("/QUIT") );
	closesocket( m_nSocket );
	KillTimer( 9 );
	KillTimer( 7 );
	m_wndTab.DeleteAllItems();
	for ( int nChannel = 0 ; nChannel < MAX_CHANNELS ; nChannel++ )
	{
		m_pIrcBuffer[ nChannel ].RemoveAll();
		m_pIrcUsersBuffer[ nChannel ].RemoveAll();
		m_nBufferCount = 0;
	}
	m_bConnected = FALSE;
}

void CIRCFrame::OnUpdateIrcDisconnect(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_bConnected );
	pCmdUI->SetCheck( FALSE );
}

CString CIRCFrame::GetTabText(int nTabIndex)
{
	const int BUFFER_SIZE = 100;
	if ( nTabIndex == -1 ) nTabIndex = m_wndTab.GetCurSel();
	TCHAR* pszBuffer = new TCHAR[ BUFFER_SIZE ];
	TCITEM item;
	item.mask = TCIF_IMAGE | TCIF_PARAM | TCIF_TEXT;
	item.pszText = pszBuffer;
	item.cchTextMax = BUFFER_SIZE/sizeof(TCHAR);
	m_wndTab.GetItem( nTabIndex, &item );
	CString strTmp = pszBuffer;
	delete[] pszBuffer;
	return strTmp;
}

void CIRCFrame::OnLocalText(LPCTSTR pszText)
{
	CString strMessage = pszText, strText, strTabTitle, strBufferMsg, strStatusMsg;
	strTabTitle = GetTabText();
	int nOldIndex = 0;
	BOOL bMeMsg = FALSE;
	int nIndex = strMessage.Find( _T("\r\n"), nOldIndex );
	while ( nIndex != -2 )
	{
		if ( nIndex == -1 ) 
			strText = strMessage.Mid( nOldIndex );
		else
			strText = strMessage.Mid( nOldIndex, nIndex - nOldIndex );
		strText = strText.SpanExcluding( _T("\r\n") );
		if ( strText.GetLength() > 2 ) 
		{
			if ( strText.Left( 3 ) == "/j " ) 
				strText = "/join" + strText.Mid( 2 );
		}
		if ( strText.GetLength() > 3 ) 
		{
 			if ( strText.Left( 4 ) == "/bs " ) 
				strText = "/botserv" + strText.Mid( 3 );
			if ( strText.Left( 4 ) == "/ns " ) 
				strText = "/nickserv" + strText.Mid( 3 );
 			else if ( strText.Left( 4 ) == "/cs " ) 
				strText = "/chanserv" + strText.Mid( 3 );
			else if ( strText.Left( 4 ) == "/me " ) 
				bMeMsg = TRUE;
		}
		if ( !bMeMsg )
		{
			strStatusMsg = "<" + m_sNickname + "> " + strText;
		}
		else
		{
			strText = strText.Mid( 4 );
			strStatusMsg = "* " + m_sNickname + " " + strText;
		}

		CString strLine;
		if ( Settings.IRC.Timestamp )
		{
			CTime pNow = CTime::GetCurrentTime();
			strLine.Format( _T("[%.2i:%.2i:%.2i] %s"),
				pNow.GetHour(), pNow.GetMinute(), pNow.GetSecond(), strStatusMsg );
		}
		else strLine = strStatusMsg;

		strBufferMsg = CHAR( !bMeMsg ? ID_COLOR_TEXT : ID_COLOR_ME ) + strLine;
		if ( !( strTabTitle == m_sStatus || strText.Left( 1 ) == "/") || bMeMsg )
		{
			OnStatusMessage( strLine, !bMeMsg ? ID_COLOR_TEXT : ID_COLOR_ME );
			m_pIrcBuffer[ m_wndTab.GetCurSel() ].Add( strBufferMsg );
			if ( bMeMsg ) 
			{
				strText = _T( "ACTION ") + strText;
				strText = _T( " \x01" ) + strText + _T("\x01");
			}
			else
				strText = " :" + strText;
			strText = "PRIVMSG " + strTabTitle + strText;
		}
		if ( strText.Left( 1 ) == "/" ) strText = strText.Mid( 1 );
		SendString( strText );
		if ( nIndex == -1 ) break;
		nOldIndex = nIndex + 2 ; 	
		nIndex = strMessage.Find( _T("\r\n"), nOldIndex );
	}
}

 
void CIRCFrame::OnTimer(UINT_PTR nIDEvent)
{
	int nIndex;
	if ( nIDEvent == 7 )
	{
		CListCtrl* pChannelList = (CListCtrl*)&(m_wndPanel.m_boxChans.m_wndChanList);
		for ( int nList = 0 ; nList < pChannelList->GetItemCount() ; nList++ )
			if ( !pChannelList->GetItemText( nList, 1 ).IsEmpty() )
				pChannelList->SetItemText( nList, 1, _T("0") );
		OnLocalText( _T("/list *shareaza*") );
		return;
	}
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
				if ( m_wndTab.GetCurSel() == nTargetWindow ) 
					OnStatusMessage( _T("Running flood protection..."), ID_COLOR_SERVERMSG );
				m_pIrcBuffer[ nTargetWindow ].Add( char(ID_COLOR_SERVERMSG ) + _T("Running flood protection...") );
			}
			m_nMsgsInSec = 0;
		}
		CString strMessage;
		CSingleLock pLock( &m_pSection );
		if ( WaitForSingleObject( m_pWakeup, 0 ) == WAIT_OBJECT_0 )
		{
			int nRetVal = SOCKET_ERROR;
			char* pszData = new char[102];
			nRetVal = recv( m_nSocket, (char*)pszData, 100, 0 );
			if ( nRetVal > 0 ) pszData[ nRetVal + 1 ] = '\0';
			CString strTmp = pszData;
			strTmp = m_sWsaBuffer + strTmp;
				strTmp = TrimString( strTmp );
			delete[] pszData;
			m_sWsaBuffer.Empty();

			// ToDo: convert UTF-8 strings to CP_ACP here
			if ( nRetVal == 0 )
			{
				OnStatusMessage( strTmp, ID_COLOR_NOTICE );
				OnIrcDisconnect();
				return;
			}
			if ( nRetVal == -1 )
				{
					int nError = WSAGetLastError();
					KillTimer( 9 );
					if ( nError == WSAETIMEDOUT )
					OnStatusMessage( _T("QUIT: Connection Reset By PEER." ), ID_COLOR_NOTICE );
					else if ( nError == WSAENOTCONN )
						OnStatusMessage( _T("QUIT: Connection Dropped."), ID_COLOR_NOTICE );
					else 
						OnStatusMessage( _T("QUIT: Server is busy, please try again in a minute."), ID_COLOR_NOTICE );
					OnIrcDisconnect();
					return;
				}
			if ( nRetVal != -1 && !strTmp.IsEmpty() )
			{	
				m_nMsgsInSec++;
				nIndex = strTmp.Find( _T("\x000D\x000A") );
				if ( nIndex == -1 ) 
					m_sWsaBuffer = strTmp;
				else
				{
					while ( nIndex != -1 && nRetVal != -1 && !strTmp.IsEmpty() )
					{
						strMessage = strTmp.Left( nIndex + 1 );
						strMessage = TrimString( strMessage );
						if ( strMessage.Find( ' ' ) != -1 && !strMessage.IsEmpty() )
							OnNewMessage( strMessage );
						strTmp = strTmp.Mid( nIndex + 1 );
						nIndex = strTmp.Find( _T("\x000D\x000A") );
					}
					if ( !strTmp.IsEmpty() ) 
						m_sWsaBuffer = m_sWsaBuffer + strTmp;
					else
						m_sWsaBuffer.Empty();
				}
			}
			strTmp.ReleaseBuffer();
		}
		pLock.Unlock();
	} 
	CWnd::OnTimer( nIDEvent );
}

void CIRCFrame::SendString(CString strMessage)
{
	strMessage = strMessage.Trim();
	strMessage = strMessage + _T("\x000D\x000A");
	// ToDo: convert to UTF-8 depending on the settings
	int nBytes = WideCharToMultiByte( CP_UTF8, 0, strMessage, strMessage.GetLength(), NULL, 0, NULL, NULL );
	LPSTR pBytes = new CHAR[ nBytes + 1 ];
	WideCharToMultiByte( CP_UTF8, 0, strMessage, strMessage.GetLength(), pBytes, nBytes, NULL, NULL );
	pBytes[nBytes] = '\0';
 	int nLength, nBytesSent;
	nLength = int( strlen( pBytes ) );
	nBytesSent = int( send( m_nSocket, pBytes, nLength, 0 ) );
	delete [] pBytes;
}

void CIRCFrame::OnStatusMessage(LPCTSTR pszText, int nFlags)
{
	CString strMessage = pszText;
	COLORREF cRGB = Settings.IRC.Colors[ nFlags ];
	int nIndex = strMessage.Find( _T("\x003") );
	int nSize;
	while ( nIndex != -1 )
	{
		nSize = 1;
		if ( ( (int)( strMessage.GetAt( nIndex + 1 ) ) - 48 ) > -1 && 
			 ( (int)( strMessage.GetAt( nIndex + 1 ) ) - 48 < 10 ) )
		{
			nSize++;
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
		nIndex = strMessage.Find( _T("\x003") );
	}
	for ( nIndex = 1 ; nIndex < 32 ; nIndex++ )
		strMessage.Remove( char(nIndex) );

	CRect rectView;
	m_wndView.GetWindowRect( &rectView );
	int nViewSize = int( rectView.Width() / 14 * 1.84 );
	int nCurrentChar, nOldChar = 0, nCurrentLength, nCheckLength;
	CString strMsgTemp, strCurrentWord;
	CStringArray pWordDivide;
	BOOL bStartedSplit = FALSE;
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
			nCheckLength = nCurrentLength + 
						   int( strCurrentWord.GetLength() * 0.3 );
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
				MessageBox( _T("You excedded the Max number of lines allowed!") );
				return TRUE;
			}
    		m_wndEdit.GetWindowText( m_sCurrent );
			if ( m_sCurrent.IsEmpty() ) return TRUE;
			if ( m_sCurrent.GetLength() > m_nLocalTextLimit ) 
			{
				MessageBox( _T("You excedded the Max legnth of text allowed!") );
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
	int nIndex;
	if ( nItem >= 0 )
	{
		CString strJoinChannel = m_wndPanel.m_boxChans.m_wndChanList.GetItemText( nItem, 0 );
		nIndex = m_pChanList.GetIndexOfDisplay( strJoinChannel );
		if ( nIndex == -1 ) return;
		strJoinChannel = _T("JOIN ") + m_pChanList.GetNameOfIndex( nIndex );
		SendString( strJoinChannel );
	}
}
	
BOOL CIRCFrame::OnNewMessage(CString strMessage)
{
	int nTargetWindow;
	CString strTargetName;
	CIRCNewMessage oNewMessage;

	oNewMessage.nColorID = 0;
	oNewMessage.m_sTargetName.Empty();
	ParseString( strMessage, &oNewMessage );
	nTargetWindow = IsTabExist( oNewMessage.m_sTargetName ) ; 
	for ( int nMessage = 0 ; nMessage <= oNewMessage.m_pMessages.GetCount() - 1 ; nMessage++ )
	{
		if ( nTargetWindow != -1 ) 
		{
			CString strText = oNewMessage.m_pMessages.GetAt( nMessage );
			CString strLine;
			if ( Settings.IRC.Timestamp )
			{
				CTime pNow = CTime::GetCurrentTime();
				strLine.Format( _T("[%.2i:%.2i:%.2i] %s"),
					pNow.GetHour(), pNow.GetMinute(), pNow.GetSecond(), strText );
			}
			else strLine = strText;

			if ( m_wndTab.GetCurSel() == nTargetWindow )
				OnStatusMessage( strLine, oNewMessage.nColorID );
			m_pIrcBuffer[ nTargetWindow ].Add( char( oNewMessage.nColorID ) + strLine );
		}
	}
	oNewMessage.m_pMessages.RemoveAll();
	return TRUE;
}


void CIRCFrame::ActivateMessageByID(CString strMessage, CIRCNewMessage* oNewMessage, int nMessageType)
{
	switch ( nMessageType )
	{
		case NULL:
		{
			int nTab = IsTabExist( m_sStatus );
			if ( nTab != -1 && nTab != m_wndTab.GetCurSel() )
				m_wndTab.SetTabColor( nTab, Settings.IRC.Colors[ ID_COLOR_NEWMSG ] );
			oNewMessage->m_pMessages.Add ( GetStringAfterParsedItem( 0 ) );
			oNewMessage->m_sTargetName	= m_sStatus;
			oNewMessage->nColorID		= ID_COLOR_SERVERMSG;
			return;
		}
		case ID_MESSAGE_SERVER_MSG:
		{
			int nTab = IsTabExist( m_sStatus );
			if ( nTab != -1 && nTab != m_wndTab.GetCurSel() )
				m_wndTab.SetTabColor( nTab, Settings.IRC.Colors[ ID_COLOR_NEWMSG ] );
			oNewMessage->m_pMessages.Add( GetStringAfterParsedItem( 3 ) );
			oNewMessage->m_sTargetName	= m_sStatus;
			oNewMessage->nColorID		= ID_COLOR_SERVERMSG;
			return;
		}
		case ID_MESSAGE_SERVER_ERROR:
		{
			int nTab = IsTabExist( m_sStatus );
			if ( nTab != -1 && nTab != m_wndTab.GetCurSel() )
				m_wndTab.SetTabColor( nTab, Settings.IRC.Colors[ ID_COLOR_NEWMSG ] );
			oNewMessage->m_pMessages.Add( GetStringAfterParsedItem( 3 ) );
			oNewMessage->m_sTargetName	= m_sStatus;
			oNewMessage->nColorID		= ID_COLOR_SERVERERROR;
			return;
		}
		case ID_MESSAGE_IGNORE:
		{
// 			oNewMessage->m_sTargetName = m_sStatus;
			return;
		}
		case ID_MESSAGE_SERVER_CONNECTED:
		{
			m_sNickname = m_pWords.GetAt( 2 );
			oNewMessage->m_pMessages.Add( _T("Connection Established!") );
			oNewMessage->m_sTargetName	= m_sStatus;
			oNewMessage->nColorID		= ID_COLOR_NOTICE;
			OnIrcChanCmdOpen();
			OnLocalText( _T("/list *shareaza*") );
			return;
		}
		case ID_MESSAGE_SERVER_PING:
		{
			SendString(	_T("PONG ") + GetStringAfterParsedItem( 1 ) );
			oNewMessage->m_pMessages.Add ( _T("Ping? Pong!" ) );
			oNewMessage->m_sTargetName	= m_sStatus;
			oNewMessage->nColorID		= ID_COLOR_SERVERMSG;
			return;
		}
		case ID_MESSAGE_USER_MESSAGE:
		{
			int nTab = AddTab( m_pWords.GetAt( 0 ), ID_KIND_PRIVATEMSG );
			if ( nTab != m_wndTab.GetCurSel() )
				m_wndTab.SetTabColor( nTab, Settings.IRC.Colors[ ID_COLOR_NEWMSG ] );
			oNewMessage->m_sTargetName	= m_pWords.GetAt( 0 );
			oNewMessage->nColorID		= ID_COLOR_TEXT;
			CString strSender = "<" + oNewMessage->m_sTargetName + "> ";
			CString strText = GetStringAfterParsedItem( 7 );
			ShowTrayPopup( strText, oNewMessage->m_sTargetName, NIIF_NONE, 30 );
			oNewMessage->m_pMessages.Add( strSender + GetStringAfterParsedItem( 7 ) );
			return;
		}
		case ID_MESSAGE_USER_AWAY:
		{
			oNewMessage->m_sTargetName	= GetTabText( m_wndTab.GetCurSel() );
			oNewMessage->nColorID		= ID_COLOR_SERVERMSG;
			CString strSender = "* " + m_pWords.GetAt( 3 ) + " is away: ";
			oNewMessage->m_pMessages.Add( strSender + GetStringAfterParsedItem( 4 ) );
			return;
		}
		case ID_MESSAGE_USER_KICK:
		{
			if ( m_pWords.GetAt( 7 ) != m_sNickname )
			{
				oNewMessage->m_sTargetName = GetTabText( m_wndTab.GetCurSel() );
				int nTab = IsTabExist( m_pWords.GetAt( 6 ) );
				if ( nTab!= -1 ) 
				{
					int nListUser = FindInList( m_pWords.GetAt( 7 ) );
					if ( nTab == m_wndTab.GetCurSel() && nListUser != -1 )
							m_wndPanel.m_boxUsers.m_wndUserList.DeleteString( nListUser );
					nListUser = FindInList( m_pWords.GetAt( 7 ), 2, nTab );
					if ( nListUser != -1 )
						m_pIrcUsersBuffer[ nTab ].RemoveAt( nListUser );
					SortUserList();
					FillCountChanList( "-1", m_pWords.GetAt( 6 ) );
				}
				CString strSender = "* " + m_pWords.GetAt( 7 ) + " was kicked by " + 
					m_pWords.GetAt( 0 ) + " (" + GetStringAfterParsedItem( 8 ) + ")";
				oNewMessage->nColorID = ID_COLOR_SERVERMSG;
				oNewMessage->m_pMessages.Add( strSender );
			}
			else
			{
				oNewMessage->m_sTargetName = m_sStatus;
				int nTab = IsTabExist( m_pWords.GetAt( 6 ) );
				if ( nTab != -1 ) 
				{
					m_wndTab.SetCurSel( nTab );
					OnIrcCloseTab();
				}
				m_wndTab.SetCurSel( 0 );
				TabClick();
				CString strSender = "* You were kicked by " + m_pWords.GetAt( 0 ) + 
					"of channel " + m_pWords.GetAt( 6 ) + " (" + GetStringAfterParsedItem( 8 ) + ")";
				oNewMessage->nColorID = ID_COLOR_SERVERMSG;
				oNewMessage->m_pMessages.Add( strSender );
			}
			return;
		}
		case ID_MESSAGE_STOPAWAY:
		{
			oNewMessage->m_sTargetName	= GetTabText( m_wndTab.GetCurSel() );
			oNewMessage->nColorID		= ID_COLOR_SERVERMSG;
			CString strSender = "* You are no longer set as away: ";
			oNewMessage->m_pMessages.Add( strSender );
			return;
		}
		case ID_MESSAGE_SETAWAY:
		{
			oNewMessage->m_sTargetName	= GetTabText( m_wndTab.GetCurSel() );
			oNewMessage->nColorID		= ID_COLOR_SERVERMSG;
			CString strSender = "* You are now set as away: ";
			oNewMessage->m_pMessages.Add( strSender );
			return;
		}
		case ID_MESSAGE_USER_ME:
		{
			int nTab = AddTab( m_pWords.GetAt( 0 ), ID_KIND_PRIVATEMSG );
			if ( nTab != m_wndTab.GetCurSel() )
				m_wndTab.SetTabColor( nTab, Settings.IRC.Colors[ ID_COLOR_NEWMSG ] );
			oNewMessage->m_sTargetName	= m_pWords.GetAt( 0 );
			oNewMessage->nColorID		= ID_COLOR_ME;
			CString strSender = "* " + oNewMessage->m_sTargetName + " ";
			oNewMessage->m_pMessages.Add( strSender + GetStringAfterParsedItem( 8 ) );
			return;
		}
		case ID_MESSAGE_USER_INVITE:
		{
			oNewMessage->m_sTargetName	= GetTabText( m_wndTab.GetCurSel() );
			oNewMessage->nColorID		= ID_COLOR_SERVERMSG;
			CString strSender = "* You have just been invited to channel " + 
				m_pWords.GetAt( 8 ) + " by " + m_pWords.GetAt( 6 );
			oNewMessage->m_pMessages.Add( strSender );
			return;
		}
		case ID_MESSAGE_CHANNEL_MESSAGE:
		{
			int m_nTab = IsTabExist( m_pWords.GetAt( 6 ) );
			if ( m_nTab == -1 ) return;
			if ( m_nTab != m_wndTab.GetCurSel() )
				m_wndTab.SetTabColor( m_nTab, Settings.IRC.Colors[ ID_COLOR_NEWMSG ] );
			oNewMessage->m_sTargetName	= m_pWords.GetAt( 6 );
			oNewMessage->nColorID		= ID_COLOR_TEXT;
			CString strSender = "<" + m_pWords.GetAt( 0 ) + "> ";
			oNewMessage->m_pMessages.Add( strSender + GetStringAfterParsedItem( 7 ) );
			return;
		}
		case ID_MESSAGE_CHANNEL_ME:
		{
			int m_nTab = IsTabExist( m_pWords.GetAt( 6 ) );
			if ( m_nTab == -1 ) return;
			if ( m_nTab != m_wndTab.GetCurSel() )
				m_wndTab.SetTabColor( m_nTab, Settings.IRC.Colors[ ID_COLOR_NEWMSG ] );
			oNewMessage->m_sTargetName	= m_pWords.GetAt( 6 );
			oNewMessage->nColorID		= ID_COLOR_ME;
			CString strSender = "* " + m_pWords.GetAt( 0 ) + " ";
			oNewMessage->m_pMessages.Add( strSender + GetStringAfterParsedItem( 8 ) );
			return;
		}

		case ID_MESSAGE_CHANNEL_NOTICE:
		{
			oNewMessage->m_pMessages.Add( "-" + m_pWords.GetAt( 0 ) + " - " + GetStringAfterParsedItem ( 8 ) );
			oNewMessage->m_sTargetName	= m_pWords.GetAt( 6 );
			oNewMessage->nColorID		= ID_COLOR_NOTICE;
			return;
		}
		case ID_MESSAGE_CLIENT_INVITE:
		{
			oNewMessage->m_sTargetName	= GetTabText( m_wndTab.GetCurSel() );
			oNewMessage->nColorID		= ID_COLOR_SERVERMSG;
			CString strSender = "* You have just invited " + m_pWords.GetAt( 3 ) + 
				" to channel " + m_pWords.GetAt( 4 );
			oNewMessage->m_pMessages.Add( strSender );
			return;
		}
		case ID_MESSAGE_CLIENT_WHOWAS:
		{
			oNewMessage->m_sTargetName	= m_sStatus;
			oNewMessage->nColorID		= ID_COLOR_SERVERMSG;
			CString strSender = m_pWords.GetAt( 2 ) + " was " + m_pWords.GetAt( 5 ) + 
				m_pWords.GetAt( 6 ) + GetStringAfterParsedItem( 7 );
			oNewMessage->m_pMessages.Add( strSender );
			return;
		}
		case ID_MESSAGE_CLIENT_WHOIS:
		{
			oNewMessage->m_sTargetName	= m_sStatus;
			oNewMessage->nColorID		= ID_COLOR_SERVERMSG;
			CString strSender			= GetStringAfterParsedItem( 3 );
			oNewMessage->m_pMessages.Add( strSender );
			return;
		}
		case ID_MESSAGE_CLIENT_NOTICE:
		{
			int nTab = m_wndTab.GetCurSel();
			TCHAR pszBuffer[ 20 ];
			CString strChannelName;

			TCITEM item;
			item.mask = TCIF_IMAGE | TCIF_PARAM | TCIF_TEXT;
			item.pszText = pszBuffer;
			item.cchTextMax = sizeof(pszBuffer) / sizeof(TCHAR);
		    m_wndTab.GetItem( nTab, &item );

			strChannelName = pszBuffer;
			oNewMessage->m_pMessages.Add( "-" + m_pWords.GetAt( 0 ) + "- " + GetStringAfterParsedItem ( 8 ) );
			oNewMessage->m_sTargetName	= strChannelName;
			oNewMessage->nColorID		= ID_COLOR_NOTICE;
			return;
		}
		case ID_MESSAGE_USER_CTCPTIME:
		{
			SYSTEMTIME pTime;
			GetSystemTime( &pTime ) ; // gets current time
			CString strDay;

			if ( pTime.wDayOfWeek == 0 ) LoadString( strDay, IDS_DAY_SUNDAY );
			else if ( pTime.wDayOfWeek == 1 ) LoadString( strDay, IDS_DAY_MONDAY );
			else if ( pTime.wDayOfWeek == 2 ) LoadString( strDay, IDS_DAY_TUESDAY );
			else if ( pTime.wDayOfWeek == 3 ) LoadString( strDay, IDS_DAY_WEDNESDAY );
			else if ( pTime.wDayOfWeek == 4 ) LoadString( strDay, IDS_DAY_THURSDAY );
			else if ( pTime.wDayOfWeek == 5 ) LoadString( strDay, IDS_DAY_FRIDAY );
			else if ( pTime.wDayOfWeek == 6 ) LoadString( strDay, IDS_DAY_SATURDAY );

			OnLocalText( "/NOTICE " + m_pWords.GetAt( 0 ) + ' ' + char( 0x1 ) + 
				"TIME Today is " + strDay + " for more info contact me." + char( 0x1 ) );

			oNewMessage->m_pMessages.Add( "* " + m_pWords.GetAt( 0 ) + " just TIMEed you." );
			oNewMessage->m_sTargetName	= m_sStatus;
			oNewMessage->nColorID		= ID_COLOR_SERVERMSG;
			return;
		}
		case ID_MESSAGE_USER_CTCPVERSION:
		{
			OnLocalText( "/NOTICE " + m_pWords.GetAt( 0 ) + ' ' + char( 0x1 ) + 
				"VERSION Shareaza " + theApp.m_sVersion + char( 0x1 ) );
			oNewMessage->m_pMessages.Add( "* " + m_pWords.GetAt( 0 ) + " just VERSIONed you." );
			oNewMessage->m_sTargetName	= m_sStatus;
			oNewMessage->nColorID		= ID_COLOR_SERVERMSG;
			return;
		}
		case ID_MESSAGE_SERVER_NOTICE:
		{
			oNewMessage->m_pMessages.Add( GetStringAfterParsedItem ( FindParsedItem( ":", 2 ) ) + 
				" (" + m_pWords.GetAt( 2 ) + ")" );
			oNewMessage->m_sTargetName	= m_sStatus;
			oNewMessage->nColorID		= ID_COLOR_NOTICE;
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
				if ( strTemp.Left( 1 ) == "+" ) 
					nModeColumn = 1;
				else if ( strTemp.Left( 1 ) == "@" ) 
					nModeColumn = 2;
				else if ( strTemp.Left( 1 ) == "%" ) 
					nModeColumn = 4;
				nMode += nModeColumn;
				if ( nMode != 48 ) strTemp = strTemp.Mid( 1 );
				m_wndPanel.m_boxUsers.m_wndUserList.InsertString( nWord - 6, m_pWords.GetAt( nWord ) );
				strTemp = char(nMode) + strTemp;
				m_pIrcUsersBuffer[ nTab ].Add( strTemp );
			}
			SortUserList();
			oNewMessage->m_pMessages.Add( "* You have just joined the channel " + strChannelName );
			oNewMessage->m_sTargetName	= strChannelName;
			oNewMessage->nColorID		= ID_COLOR_CHANNELACTION;
			return;
		}
		case ID_MESSAGE_CHANNEL_TOPICSETBY:
		{
			CString strChannelName = m_pWords.GetAt( 3 );
			oNewMessage->m_pMessages.Add( "* Topic set by " + m_pWords.GetAt( 4 ) );
			oNewMessage->m_sTargetName	= strChannelName;
			oNewMessage->nColorID		= ID_COLOR_TOPIC;
			return;
		}
		case ID_MESSAGE_CHANNEL_TOPICSHOW:
		{
			CString strChannelName = m_pWords.GetAt( 3 );
			m_pContent.Clear();
			oNewMessage->m_pMessages.Add( "* Topic is: " + GetStringAfterParsedItem ( 4 ) );
			oNewMessage->m_sTargetName	= strChannelName;
			oNewMessage->nColorID		= ID_COLOR_TOPIC;
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
						m_wndPanel.m_boxUsers.m_wndUserList.DeleteString( nListUser );
				nListUser = FindInList( strNick, 2, nTab );
				if ( nListUser != -1 )
					m_pIrcUsersBuffer[ nTab ].RemoveAt( nListUser );
				SortUserList();
				FillCountChanList( "-1", strChannelName );
				oNewMessage->m_pMessages.Add( "* " + strNick + " has parted " + strChannelName );
				oNewMessage->m_sTargetName	= strChannelName;
				oNewMessage->nColorID		= ID_COLOR_CHANNELACTION;
			}
			return;
		}
		case ID_MESSAGE_CHANNEL_QUIT:
		{
			CString strNick( m_pWords.GetAt( 0 ) ), strChannelName;
			CString strUserMsg = GetStringAfterParsedItem ( 6 ) ;
			int nTab = m_wndTab.GetCurSel();
			int nListUser = FindInList( strNick );
			if ( nTab == m_wndTab.GetCurSel() && nListUser != -1 )
					m_wndPanel.m_boxUsers.m_wndUserList.DeleteString( nListUser );

			TCHAR pszBuffer[ 20 ];
			TCITEM item;
			item.mask = TCIF_IMAGE | TCIF_PARAM | TCIF_TEXT;
			item.pszText = pszBuffer;
			item.cchTextMax = sizeof(pszBuffer) / sizeof(TCHAR);
		    m_wndTab.GetItem( nTab, &item );
			strChannelName = pszBuffer;

			SortUserList();
			FillCountChanList( "-1", strChannelName );
			oNewMessage->m_pMessages.Add( "* " + strNick + " has Quit: ( " + strUserMsg + " ) " );
			oNewMessage->m_sTargetName	= strChannelName;
			oNewMessage->nColorID		= ID_COLOR_SERVERMSG;
			for ( nTab = 1 ; nTab < m_nBufferCount ; nTab++ )
			{
				nListUser = FindInList( strNick, 2, nTab );
				if ( nListUser != -1 )
				{
					m_pIrcUsersBuffer[ nTab ].RemoveAt( nListUser );
					if ( m_wndTab.GetCurSel() != nTab ) 
						m_pIrcBuffer[ nTab ].Add( char(ID_COLOR_SERVERMSG) + oNewMessage->m_pMessages.GetAt( 0 ) );
				}
			}
			return;
		}
		case ID_MESSAGE_CLIENT_JOIN:
		{
			CString strChannelName( m_pWords.GetAt( 7 ) );
			int nTab = 	AddTab( strChannelName, ID_KIND_CHANNEL );
			m_wndTab.SetCurSel( nTab );
			m_wndPanel.m_boxUsers.m_wndUserList.ResetContent();
			m_pIrcUsersBuffer[ nTab ].RemoveAll();
			m_pContent.Clear();
			FillCountChanList( "0", strChannelName );
			oNewMessage->m_sTargetName	= strChannelName;
			oNewMessage->nColorID		= ID_COLOR_CHANNELACTION;
			return;
		}
		case ID_MESSAGE_CHANNEL_JOIN:
		{
			CString strNick( m_pWords.GetAt( 0 ) ), strChannelName( m_pWords.GetAt( 7 ) );
			int nTab = 	IsTabExist( strChannelName );
			if ( nTab == -1 ) return;
			m_pIrcUsersBuffer[ nTab ].Add( char( 48 ) + strNick );
			if ( nTab == m_wndTab.GetCurSel() ) 
				m_wndPanel.m_boxUsers.m_wndUserList.InsertString( m_wndPanel.m_boxUsers.m_wndUserList.GetCount(), 
					strNick );
			FillCountChanList( "0", strChannelName );
			SortUserList();
			oNewMessage->m_pMessages.Add( "* " + strNick + " has joined " + strChannelName );
			oNewMessage->m_sTargetName	= strChannelName;
			oNewMessage->nColorID		= ID_COLOR_CHANNELACTION;
			return;
		}
		case ID_MESSAGE_SERVER_DISCONNECT:
		{		
			oNewMessage->m_pMessages.Add( GetStringAfterParsedItem( 0 ) );
			oNewMessage->m_sTargetName	= m_sStatus;
			oNewMessage->nColorID		= ID_COLOR_SERVERMSG;
			OnIrcDisconnect();
			return;
		}
		case ID_MESSAGE_CHANNEL_SETMODE:
		{
			CString strMode = m_pWords.GetAt( 7 );
			BOOL bSign = ( strMode.Left( 1 ) == "+" );
			int nCurNick = 8;
			for ( int nChar = 1 ; nChar < strMode.GetLength() ; nChar++ )
			{
				if ( m_pWords.GetCount() - 1 < nCurNick ) break;

				if ( strMode.Mid( nChar, 1 ) == "+" || 
					 strMode.Mid( nChar, 1 ) == "-" ) 
				{
					bSign = ( strMode.Mid( nChar, 1 ) == "+" );
					nCurNick--;
				}
				else if ( strMode.Mid( nChar, 1 ) == "o" || 
						  strMode.Mid( nChar, 1 ) == "v" ||
						  strMode.Mid( nChar, 1 ) == "h" )
				{
					int nSign = 1;

					if ( strMode.Mid( nChar, 1 ) == "o" ) 
						nSign = 2;
					else if ( strMode.Mid( nChar, 1 ) == "h" ) 
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
						strCurUser = "@" + strCurUser;
					else if ( nMode == 1 ) 
						strCurUser = "+" + strCurUser;
					else if ( nMode == 4 || nMode == 5 ) 
						strCurUser = "%" + strCurUser;

					m_wndPanel.m_boxUsers.m_wndUserList.DeleteString( nInUserList );
					m_wndPanel.m_boxUsers.m_wndUserList.InsertString( nInUserList - 1, strCurUser );
					m_pIrcUsersBuffer[ nTab ].RemoveAt( nInBufferList );
					nMode += 48;
					m_pIrcUsersBuffer[ nTab ].Add( char(nMode) + strCurItem );
					nCurNick++;
				}
			}
			CString strMessage = m_pWords.GetAt( 0 );
			strMessage = strMessage + " sets mode: " + GetStringAfterParsedItem( 6 );
			SortUserList();
			oNewMessage->m_pMessages.Add( strMessage );
			oNewMessage->m_sTargetName	= m_pWords.GetAt( 6 );
			oNewMessage->nColorID		= ID_COLOR_CHANNELACTION;
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
						strCurUser = "@" + m_pWords.GetAt( 7 );
					else if ( nMode == 1 ) 
						strCurUser = "+" + m_pWords.GetAt( 7 );
					else if ( nMode == 4 || nMode == 5 ) 
						strCurUser = "%" + m_pWords.GetAt( 7 );
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
				m_wndPanel.m_boxUsers.m_wndUserList.DeleteString( nListUser );
				m_wndPanel.m_boxUsers.m_wndUserList.InsertString( nListUser, strCurUser );
				oNewMessage->m_pMessages.Add( "* " + strNick + " is now known as " + m_pWords.GetAt( 7 ) );
				oNewMessage->m_sTargetName	= strChannelName;
				oNewMessage->nColorID		= ID_COLOR_CHANNELACTION;
			}
			return;
		}
	}
}

int CIRCFrame::ParseMessageID()
{
	INT_PTR nWordCount = m_pWords.GetCount();
	int nMessageType = NULL;
	if ( nWordCount > 1 )
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
	if ( nWordCount > 6 )
	{
		const CString strMessage1( m_pWords.GetAt( 5 ) );
		const CString strMessage2( m_pWords.GetAt( 0 ) );

		if ( strMessage1 == "PART" && strMessage2 != m_sNickname )
			nMessageType = ID_MESSAGE_CHANNEL_PART;
		if ( strMessage1 == "PART" && strMessage2 == m_sNickname )
			nMessageType = ID_MESSAGE_IGNORE;
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
	if ( nWordCount > 8 )
	{
		const CString strMessage1( m_pWords.GetAt( 5 ) );
		const CString strMessage2( m_pWords.GetAt( 0 ) );

		if ( strMessage1 == "INVITE" ) 
			nMessageType = ID_MESSAGE_USER_INVITE;
		else if ( strMessage1 == "NOTICE" && strMessage2 != m_sDestinationIP )
		{
			if ( m_pWords.GetAt( 6 ) == m_sNickname ) 
				nMessageType = ID_MESSAGE_CLIENT_NOTICE;
			else
				nMessageType = ID_MESSAGE_CHANNEL_NOTICE;
		}
		else if ( strMessage1 == "PRIVMSG" )
		{
			char pszFirst = (char)m_pWords.GetAt( 8 ).GetAt( 0 );
			CString str = m_pWords.GetAt( 8 ).Right( 1 );
			char pszEnd = (char)str.GetAt( 0 );
			str = m_pWords.GetAt( 8 );
			str = str.Mid( 1, str.GetLength() - 2 ).MakeLower();
			if ( pszFirst == char( 0x1 ) )
			{
				if ( m_pWords.GetAt( 6 ) == m_sNickname )
				{
					if ( pszFirst == pszEnd )
					{
						if ( str == "version" ) 
							nMessageType = ID_MESSAGE_USER_CTCPVERSION;
						else if ( str == "time" ) 
							nMessageType = ID_MESSAGE_USER_CTCPTIME;
					}
					else if ( str == "actio" ) 
						nMessageType = ID_MESSAGE_USER_ME;
				}
				else if ( str == "actio" ) 
					nMessageType = ID_MESSAGE_CHANNEL_ME;
				}
				else
				{
				if ( m_pWords.GetAt( 6 ) == m_sNickname )
				{
					if ( pszFirst != char( 0x1 ) ) nMessageType = ID_MESSAGE_USER_MESSAGE;
		}
		else
				{
					if ( pszFirst != char( 0x1 ) ) nMessageType = ID_MESSAGE_CHANNEL_MESSAGE;
				}
			}
		}
	}

	return nMessageType;
}

void CIRCFrame::ParseString(CString strMessage, CIRCNewMessage* oNewMessage)
{
 	CString str, strNick;
	m_pWords.RemoveAll();
	int nPos, nOldPos = -1;

	// Tokens in user ID, for e.g. nick!nick@domain.com
	int nFirstToken, nSecondToken, nThirdToken;
	if ( strMessage.Left( 1 ) == ":" ) nOldPos = 0;
	nPos = strMessage.Find( ' ' );
	nThirdToken = strMessage.ReverseFind( ':' );
	if ( nThirdToken == 0 ) nThirdToken = strMessage.GetLength() - 1;
	while ( nPos != -2 )
	{
		if ( nPos == -1 ) 
			str = strMessage.Mid( nOldPos + 1 );
		else 
			str = strMessage.Mid( nOldPos + 1, nPos - nOldPos - 1 );
		nFirstToken = str.Find( '!', 1 );
		nSecondToken = str.Find( '@', 1 );
		if ( str.Mid( 0, 1 ) == ":" && nOldPos <= nThirdToken )
		{
			m_pWords.Add( ":" );
			str = str.Mid( 1 );
		}
		if ( nFirstToken != -1 && nSecondToken != -1 && nFirstToken < nSecondToken && 
			( nSecondToken < nThirdToken && nThirdToken != -1 || nThirdToken == -1 ) 
			  && nOldPos <= nThirdToken )
		{	
			strNick = str.Mid( 0, nFirstToken );
			m_pWords.Add( strNick );
			m_pWords.Add( "!" );
			strNick = str.Mid( nFirstToken + 1, nSecondToken - nFirstToken - 1 );
			m_pWords.Add( strNick );
			m_pWords.Add( "@" );
			strNick = str.Mid( nSecondToken + 1 );
			m_pWords.Add( strNick );
		}
		else
			m_pWords.Add( str );
		if ( nPos == -1 ) break;
		nOldPos = nPos;
		nPos = strMessage.Find( ' ', nPos + 1 );
	}
	int nMessageID = ParseMessageID();
	ActivateMessageByID( strMessage, oNewMessage, nMessageID );
	if ( m_bFloodProtectionRunning && nMessageID != 0 )
	{
		if ( nMessageID > 203 )
		{
			oNewMessage->nColorID = 0;
			oNewMessage->m_sTargetName.Empty();
			oNewMessage->m_pMessages.RemoveAll();
		}
	}
	return;
}

int CIRCFrame::FindParsedItem(CString strMessage, int nFirst)
{
	for ( int nItem = nFirst ; nItem < m_pWords.GetCount() - 1 ; nItem++ )
	{
		if ( strMessage == m_pWords.GetAt( nItem ) ) return nItem;
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
	nIndex = strMessage.Find( _T("\x000A") );
	if ( nIndex == 0 ) strMessage = strMessage.Mid( 1 );
	for( nIndex = 0 ; nIndex < strMessage.GetLength() - 1 ; nIndex++ )
	{
		if ( strMessage.Mid( nIndex, 1 ) == _T("\x05BC") )
			return strTmp;
		strTmp = strTmp + strMessage.Mid( nIndex, 1 );
	}
	return strTmp;
}

int CIRCFrame::IsTabExist(CString strTabName)
{
 for ( int nTab = 0 ; nTab < m_wndTab.GetItemCount() ; nTab++ )
	 if ( GetTabText( nTab ).CompareNoCase( strTabName ) == 0 ) return nTab;
  return -1;
}

void CIRCFrame::LoadBufferForWindow(int nTab)
{
	if ( nTab == -1 ) return;
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
	int nCmpResult, nBiggest, nUser1 , nUser2;
	CString strUser1, strUser2;
	for ( nUser1 = 0 ; nUser1 < m_wndPanel.m_boxUsers.m_wndUserList.GetCount() ; nUser1++ )
	{
		nBiggest = nUser1;
		for ( nUser2 = nUser1 + 1 ; nUser2 < m_wndPanel.m_boxUsers.m_wndUserList.GetCount() ; nUser2++ )
		{
			m_wndPanel.m_boxUsers.m_wndUserList.GetText( nBiggest, strUser1 );
			m_wndPanel.m_boxUsers.m_wndUserList.GetText( nUser2, strUser2 );
			nCmpResult = CompareUsers( strUser1, strUser2 );
			if ( nCmpResult > 0 ) nBiggest = nUser2;
		}
		if ( nBiggest != nUser1 )
		{
			m_wndPanel.m_boxUsers.m_wndUserList.GetText( nBiggest, strUser1 );
			m_wndPanel.m_boxUsers.m_wndUserList.GetText( nUser1, strUser2 );
			m_wndPanel.m_boxUsers.m_wndUserList.DeleteString( nUser1 );
			m_wndPanel.m_boxUsers.m_wndUserList.InsertString( nUser1, strUser1.GetBuffer() );
			m_wndPanel.m_boxUsers.m_wndUserList.DeleteString( nBiggest );
			m_wndPanel.m_boxUsers.m_wndUserList.InsertString( nBiggest, strUser2.GetBuffer() );
		}
	}
	m_wndPanel.m_boxUsers.UpdateCaptionCount();
}
int CIRCFrame::CompareUsers(CString str1, CString str2)
{
	int nModeColumn1 = 0, nModeColumn2 = 0;
	CString strUser1 = str1, strUser2 = str2;

	switch ( (char)strUser1.GetAt( 0 ) )
	{
		case '+': nModeColumn1 = 1; break;
		case '%': nModeColumn1 = 2; break;
		case '@': nModeColumn1 = 3;
	}
	switch ( (char)strUser2.GetAt( 0 ) )
	{
		case '+': nModeColumn2 = 1; break;
		case '%': nModeColumn2 = 2; break;
		case '@': nModeColumn2 = 3;
	}
	if ( nModeColumn1 == nModeColumn2 )
	{
		if ( nModeColumn1 != 0 ) strUser1 = strUser1.Mid( 1 );
		if ( nModeColumn2 != 0 ) strUser2 = strUser2.Mid( 1 );
		return _tcsicmp( strUser1.GetBuffer(), strUser2.GetBuffer() );
	}
	return ( nModeColumn2 - nModeColumn1 );
}

void CIRCFrame::TabClick()
{
	m_pContent.Clear();
	m_wndView.InvalidateIfModified();
    m_wndPanel.m_boxUsers.m_wndUserList.ResetContent();
	CString str;
	int nTab = m_wndTab.GetCurSel(), nMode;
	m_wndTab.SetTabColor( nTab, RGB(0, 0, 0) );
	for ( int nUser = 0 ; nUser < m_pIrcUsersBuffer[ nTab ].GetCount() ; nUser++ )
	{
		str = m_pIrcUsersBuffer[ nTab ].GetAt( nUser );
		nMode = int( str.GetAt( 0 ) ) - 48;
		if ( nMode == 0 ) str = str.Mid( 1 );
		if ( nMode == 2 || nMode == 3 || nMode == 6 || nMode == 7 ) str = "@" + str.Mid( 1 );
		if ( nMode == 1 ) str = "+" + str.Mid( 1 );
		if ( nMode == 4 || nMode == 5 ) str = "%" + str.Mid( 1 );
		m_wndPanel.m_boxUsers.m_wndUserList.AddString( str );
	}
	LoadBufferForWindow( nTab );
	SortUserList();
	this->RedrawWindow();
}

int CIRCFrame::AddTab(CString strTabName, int nKindOfTab)
{
	if ( m_wndTab.GetItemCount() + 1 == MAX_CHANNELS )
	{
		OnLocalText( _T("You have exceeded the maximum number of opened channels") );
		return -1;
	}
	int m_nTab = IsTabExist( strTabName );
	if ( m_nTab != -1 ) return -1;
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
	if ( nIndex != -1 || strText.Left( 1 ) == "#" ||
		strText.Left( 7 ) == "http://" || strText.Left( 4 ) == "www." ||
		strText.Left( 7 ) == "magnet:" || strText.Left( 4 ) == "ed2k:" ||
		strText.Mid( 1, 7 ) == "http://" || strText.Mid( 1, 4 ) == "www." )
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
	else if ( strText.Left( 1 ) == "#" ) 
	{
		CString strJoinChan = _T("JOIN ") + strText;
		SendString( strJoinChan );
	}
	else if ( strText.Left( 7 ) == "http://" || strText.Left( 4 ) == "www." ||
		strText.Mid( 1, 7 ) == "http://" || strText.Mid( 1, 4 ) == "www." )	
	{
		if ( strText.Mid( 1, 7 ) == "http://" || strText.Mid( 1, 4 ) == "www." )
			strText = strText.Mid( 1 );
		if ( strText.Right( 1 ) == ")" ) strText = strText.Mid( 0, strText.GetLength() - 1 );
		theApp.InternalURI(	strText );
	}
	else if ( strText.Left( 7 ) == "magnet:" || strText.Left( 4 ) == "ed2k:" ||
		strText.Mid( 1, 7 ) == "magnet:" || strText.Mid( 1, 4 ) == "ed2k:" )	
	{
		if ( strText.Mid( 1, 7 ) == "magnet:" || strText.Mid( 1, 4 ) == "ed2k:" )
			strText = strText.Mid( 1 );
		if ( strText.Right( 1 ) == ")" ) strText = strText.Mid( 0, strText.GetLength() - 1 );
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
		m_wndPanel.m_boxUsers.m_wndUserList.SetCurSel( nIndex );
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
	CString strNick;
	INT_PTR nListCount = 0;
	int nReturn = -1;
	if ( nList == 0 ) nListCount = m_wndPanel.m_boxUsers.m_wndUserList.GetCount();
	if ( nList == 1 ) nListCount = m_wndPanel.m_boxChans.m_wndChanList.GetItemCount();
	if ( nList == 2 ) nListCount = m_pIrcUsersBuffer[ nTab ].GetCount();
	for ( int nItem = 0 ; nItem < nListCount ; nItem++ )
	{
		if ( nList == 0 ) m_wndPanel.m_boxUsers.m_wndUserList.GetText( nItem, strNick );
		if ( nList == 1 ) strNick = m_wndPanel.m_boxChans.m_wndChanList.GetItemText( nItem, 1 );
		if ( nList == 2 ) strNick = m_pIrcUsersBuffer[ nTab ].GetAt( nItem ).Mid( 1 );
		if ( RemoveModeOfNick( strNick ) == strName ) 
			nReturn = nItem;
	}
	return nReturn;
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
			if ( _tcscmp( strTemp.Left( 1 ), m_pszLineJoiner ) != 0 ) break;
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
			if ( _tcscmp( strText.Left( 1 ), m_pszLineJoiner ) != 0 ) break;
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
	int nItem = m_wndPanel.m_boxUsers.m_wndUserList.GetCurSel();

	if ( nItem < 0 ) return;

	CString strQueryUser;
	m_wndPanel.m_boxUsers.m_wndUserList.GetText( nItem, strQueryUser );
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
	if ( strNick.Mid( 0, 1 ) == "@" || strNick.Mid( 0, 1 ) == "%" || strNick.Mid( 0, 1 ) == "+" )
		strNick = strNick.Mid( 1 );
	return strNick;
}

int CIRCFrame::IsUserInList(CString strUser)
{
	CString strNick;
	for ( int nUser = 0 ; nUser <= m_wndPanel.m_boxUsers.m_wndUserList.GetCount() - 1 ; nUser++ )
	{
		m_wndPanel.m_boxUsers.m_wndUserList.GetText( nUser, strNick );
		strNick = RemoveModeOfNick( strNick );
		if ( strNick.MakeLower().Trim() == strUser.MakeLower().Trim() ) return nUser;
	}
	return -1;
}

BOOL CIRCFrame::ShowTrayPopup(LPCTSTR szText, LPCTSTR szTitle, DWORD dwIcon, UINT uTimeout)
{
	BOOL bMinimized = ( (CMainWnd*)AfxGetMainWnd() )->m_bTrayHide;
	if ( ! bMinimized ) return FALSE;

	OSVERSIONINFO pVersion;
	pVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx( &pVersion );

	if ( pVersion.dwMajorVersion > 4 )
	{
		// Verify input parameters
		// The balloon tooltip text can be up to 255 chars long.
		CString strText( szText );

		if ( strText.GetLength() > 255 )
		{
			if ( strText.GetAt( 256 ) == ' ' )
				strText = strText.Left( 255 );
			else
			{
				if ( strText.GetAt( 255 ) == ' ' )
					strText = strText.Left( 254 ) + _T("\x2026");
				else
				{
					strText = strText.Left( 254 );
					int nLastWord = strText.ReverseFind( ' ' );
					strText = strText.Left( nLastWord ) + _T("\x2026");
				}
			}
		}
		// The balloon title text can be up to 63 chars long.
		CString strTitle;
		if ( szTitle )
		{
			strTitle.SetString( szTitle, 63 );
			strTitle.ReleaseBuffer( 63 );
		}

		// dwBalloonIcon must be valid.
		if ( NIIF_NONE		!= dwIcon && NIIF_INFO	!= dwIcon &&
			NIIF_WARNING	!= dwIcon && NIIF_ERROR	!= dwIcon ) return FALSE;

		// The timeout must be between 10 and 30 seconds.
		if ( uTimeout < 10 || uTimeout > 30 ) return FALSE;

		m_pTray.uFlags = NIF_INFO;
		_tcsncpy( m_pTray.szInfo, strText.GetBuffer(), 256 );
		if ( szTitle )
			_tcsncpy( m_pTray.szInfoTitle, strTitle.GetBuffer(), 64 );
		else
			m_pTray.szInfoTitle[0] = _T('\0');
		m_pTray.dwInfoFlags = dwIcon;
		m_pTray.uTimeout = uTimeout * 1000;   // convert time to ms

		BOOL bSuccess = Shell_NotifyIcon( NIM_MODIFY, &m_pTray );

		m_pTray.szInfo[0] = _T('\0');

		return bSuccess;
	}
	else return FALSE; // We do not support Win9x/Me/NT
}

BEGIN_MESSAGE_MAP(CIRCTabCtrl, CTabCtrl)
	ON_WM_CREATE()
//	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

CIRCTabCtrl::CIRCTabCtrl() : m_hTheme( NULL )
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
	if ( theApp.m_pfnIsThemeActive && theApp.m_pfnIsThemeActive() &&
		 m_hTheme != NULL && theApp.m_pfnDrawThemeBackground )
	{
		hr = theApp.m_pfnDrawThemeBackground( m_hTheme, dc, nPartID, nStateID, prcBox, NULL );
	}
	return hr;
}

void CIRCTabCtrl::DrawXPTabItem(HDC dc, int nItem, const RECT& rcItem, UINT flags)
{
	if ( m_hTheme == NULL || theApp.m_pfnIsThemeActive == NULL || !theApp.m_pfnIsThemeActive() ||
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

		TC_ITEM tci;
		TCHAR pszBuffer[ 128 + 4 ];
		tci.mask = TCIF_TEXT|TCIF_IMAGE;
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

	BITMAPINFO bmiOut;
	BITMAPINFOHEADER& bmihOut = bmiOut.bmiHeader;
	ZeroMemory( &bmiOut, sizeof( BITMAPINFO ) );
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
	TC_ITEM item;
	TCHAR pszBuffer[ 128 + 4 ];
	item.mask = TCIF_TEXT | TCIF_IMAGE;
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
	TC_ITEM tci;
	tci.mask = TCIF_PARAM;
	tci.lParam = cRGB;
	SetItem( nItem, &tci );
	RedrawWindow();
}

COLORREF CIRCTabCtrl::GetTabColor(int nItem)
{
	TC_ITEM tci;
	tci.mask = TCIF_PARAM;
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
	PAINTSTRUCT ps;
	CDC* pDC;
	if ( message == WM_PAINT )
	{
		if ( ( pDC = BeginPaint( &ps ) ) != NULL ) 
			DrawTabControl( pDC );
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
	TCHITTESTINFO hti;
	hti.flags = 0;
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

void CIRCChannelList::Initialize()
{
	m_nCount = 0;
	m_nCountUserDefined = 0;
}
void CIRCChannelList::AddChannel(CString strDisplayName, CString strName, BOOL bUserDefined)
{
	m_bUserDefined.Add( bUserDefined );
	m_sChannelDisplayName.Add( strDisplayName );
	m_sChannelName.Add( strName );
	m_nCount++;
	if ( bUserDefined ) m_nCountUserDefined++;
}

int CIRCChannelList::GetCount(int nType)
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

BOOL CIRCChannelList::GetType(CString strDisplayName)
{
	int nIndex = GetIndexOfDisplay( strDisplayName );
	if ( nIndex == -1 ) return FALSE;
	return m_bUserDefined.GetAt( nIndex );
}

void CIRCChannelList::RemoveChannel(CString strDisplayName)
{
	int nIndex = GetIndexOfDisplay( strDisplayName );
	if ( nIndex == -1 ) return;
	m_nCount--;
	if ( m_bUserDefined.GetAt( nIndex ) ) m_nCountUserDefined--;
	m_bUserDefined.RemoveAt( nIndex );
	m_sChannelDisplayName.RemoveAt( nIndex );
	m_sChannelName.RemoveAt( nIndex );
}

int CIRCChannelList::GetIndexOfDisplay(CString strDisplayName)
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

int CIRCChannelList::GetIndexOfName(CString strName)
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
