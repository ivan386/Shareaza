//
// CtrlMediaFrame.cpp
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
#include "ImageFile.h"
#include "Plugins.h"
#include "Library.h"
#include "SharedFile.h"
#include "Registry.h"

#include "Skin.h"
#include "CtrlMediaFrame.h"
#include "DlgSettingsManager.h"
#include "DlgMediaVis.h"
#include "CoolInterface.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifndef WM_APPCOMMAND

#define WM_APPCOMMAND					0x319
#define APPCOMMAND_VOLUME_MUTE			8
#define APPCOMMAND_VOLUME_DOWN			9
#define APPCOMMAND_VOLUME_UP			10
#define APPCOMMAND_MEDIA_NEXTTRACK		11
#define APPCOMMAND_MEDIA_PREVIOUSTRACK	12
#define APPCOMMAND_MEDIA_STOP			13
#define APPCOMMAND_MEDIA_PLAY_PAUSE		14
#define FAPPCOMMAND_MASK				0x8000
#define GET_APPCOMMAND_LPARAM(lParam) ((short)(HIWORD(lParam) & ~FAPPCOMMAND_MASK))

#endif

#define VOLUME_KEY_MULTIPLIER 5

IMPLEMENT_DYNAMIC(CMediaFrame, CWnd)

BEGIN_MESSAGE_MAP(CMediaFrame, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_CONTEXTMENU()
	ON_WM_HSCROLL()
	ON_WM_CLOSE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_SYSCOMMAND()
	ON_WM_LBUTTONDBLCLK()
	ON_UPDATE_COMMAND_UI(ID_MEDIA_CLOSE, OnUpdateMediaClose)
	ON_COMMAND(ID_MEDIA_CLOSE, OnMediaClose)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_PLAY, OnUpdateMediaPlay)
	ON_COMMAND(ID_MEDIA_PLAY, OnMediaPlay)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_PAUSE, OnUpdateMediaPause)
	ON_COMMAND(ID_MEDIA_PAUSE, OnMediaPause)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_STOP, OnUpdateMediaStop)
	ON_COMMAND(ID_MEDIA_STOP, OnMediaStop)
	ON_COMMAND(ID_MEDIA_ZOOM, OnMediaZoom)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_SIZE_FILL, OnUpdateMediaSizeFill)
	ON_COMMAND(ID_MEDIA_SIZE_FILL, OnMediaSizeFill)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_SIZE_DISTORT, OnUpdateMediaSizeDistort)
	ON_COMMAND(ID_MEDIA_SIZE_DISTORT, OnMediaSizeDistort)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_SIZE_ONE, OnUpdateMediaSizeOne)
	ON_COMMAND(ID_MEDIA_SIZE_ONE, OnMediaSizeOne)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_SIZE_TWO, OnUpdateMediaSizeTwo)
	ON_COMMAND(ID_MEDIA_SIZE_TWO, OnMediaSizeTwo)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_SIZE_THREE, OnUpdateMediaSizeThree)
	ON_COMMAND(ID_MEDIA_SIZE_THREE, OnMediaSizeThree)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_ASPECT_DEFAULT, OnUpdateMediaAspectDefault)
	ON_COMMAND(ID_MEDIA_ASPECT_DEFAULT, OnMediaAspectDefault)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_ASPECT_4_3, OnUpdateMediaAspect43)
	ON_COMMAND(ID_MEDIA_ASPECT_4_3, OnMediaAspect43)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_ASPECT_16_9, OnUpdateMediaAspect169)
	ON_COMMAND(ID_MEDIA_ASPECT_16_9, OnMediaAspect169)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_FULLSCREEN, OnUpdateMediaFullScreen)
	ON_COMMAND(ID_MEDIA_FULLSCREEN, OnMediaFullScreen)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_PLAYLIST, OnUpdateMediaPlaylist)
	ON_COMMAND(ID_MEDIA_PLAYLIST, OnMediaPlaylist)
	ON_COMMAND(ID_MEDIA_SETTINGS, OnMediaSettings)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_SETTINGS, OnUpdateMediaSettings)
	ON_COMMAND(ID_MEDIA_VIS, OnMediaVis)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_VIS, OnUpdateMediaVis)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_STATUS, OnUpdateMediaStatus)
	ON_COMMAND(ID_MEDIA_STATUS, OnMediaStatus)
	ON_UPDATE_COMMAND_UI(ID_MEDIA_MUTE, OnUpdateMediaMute)
	ON_COMMAND(ID_MEDIA_MUTE, OnMediaMute)
	ON_NOTIFY(MLN_NEWCURRENT, IDC_MEDIA_PLAYLIST, OnNewCurrent)
	ON_MESSAGE(WM_APPCOMMAND, OnMediaKey)
END_MESSAGE_MAP()

#define SIZE_INTERNAL	1982
#define SIZE_BARSLIDE	1983
#define TOOLBAR_HEIGHT	28
#define TOOLBAR_STICK	3000
#define TOOLBAR_ANIMATE	1000
#define HEADER_HEIGHT	16
#define STATUS_HEIGHT	18
#define SPLIT_SIZE		6
#define META_DELAY		10000
#define TIME_FACTOR		1000000
#define ONE_SECOND		10000000

CMediaFrame* CMediaFrame::g_pMediaFrame = NULL;


/////////////////////////////////////////////////////////////////////////////
// CMediaFrame construction

CMediaFrame::CMediaFrame()
{
	if ( g_pMediaFrame == NULL ) g_pMediaFrame = this;

	m_nState			= smsNull;
	m_bMute				= FALSE;
	m_bThumbPlay		= FALSE;
	m_bRepeat			= FALSE;
	m_bLastMedia		= FALSE;
	m_bLastNotPlayed	= FALSE;
	m_bStopFlag			= FALSE;
	m_bEnqueue			= FALSE;
	m_tLastPlay			= 0;
	m_tMetadata			= 0;
	m_bNoLogo			= VARIANT_FALSE;
	m_bFullScreen		= FALSE;
	m_bListWasVisible   = Settings.MediaPlayer.ListVisible;
	m_bListVisible		= Settings.MediaPlayer.ListVisible;
	m_nListSize			= Settings.MediaPlayer.ListSize;
	m_bStatusVisible	= Settings.MediaPlayer.StatusVisible;
	m_rcVideo.SetRectEmpty();
	m_rcStatus.SetRectEmpty();
	m_bScreenSaverEnabled = TRUE;
	m_nVidAC = m_nVidDC	= 0;
	m_nPowerSchemeId	= 0;
	m_nScreenSaverTime	= 0;
	ZeroMemory( &m_CurrentGP, sizeof(GLOBAL_POWER_POLICY) );
	ZeroMemory( &m_CurrentPP, sizeof(POWER_POLICY) );

	UpdateNowPlaying(TRUE);
}

CMediaFrame::~CMediaFrame()
{
	UpdateNowPlaying(TRUE);

	if ( g_pMediaFrame == this ) g_pMediaFrame = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CMediaFrame system message handlers

BOOL CMediaFrame::Create(CWnd* pParentWnd)
{
	CRect rect;
	return CWnd::Create( NULL, _T("CMediaFrame"), WS_CHILD | WS_VISIBLE,
		rect, pParentWnd, 0, NULL );
}

int CMediaFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	CRect rectDefault;
	SetOwner( GetParent() );

	m_wndList.Create( this, IDC_MEDIA_PLAYLIST );

	if ( ! m_wndListBar.Create( this, WS_CHILD|CBRS_NOALIGN, AFX_IDW_TOOLBAR ) ) return -1;
	m_wndListBar.SetBarStyle( m_wndListBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_BORDER_TOP );
	m_wndListBar.SetOwner( GetOwner() );

	if ( ! m_wndToolBar.Create( this, WS_CHILD|WS_VISIBLE|CBRS_NOALIGN, AFX_IDW_TOOLBAR ) ) return -1;
	m_wndToolBar.SetBarStyle( m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_BORDER_TOP );
	m_wndToolBar.SetOwner( GetOwner() );

	m_wndPosition.Create( WS_CHILD|WS_TABSTOP|TBS_HORZ|TBS_NOTICKS|TBS_TOP,
		rectDefault, &m_wndToolBar, IDC_MEDIA_POSITION );
	m_wndPosition.SetRange( 0, 0 );
	m_wndPosition.SetPageSize( 0 );

	m_wndSpeed.Create( WS_CHILD|WS_TABSTOP|TBS_HORZ|TBS_NOTICKS|TBS_TOP,
		rectDefault, &m_wndToolBar, IDC_MEDIA_SPEED );
	m_wndSpeed.SetRange( 0, 200 );
	m_wndSpeed.SetTic( 0 );
	m_wndSpeed.SetTic( 100 );
	m_wndSpeed.SetTic( 200 );

	m_wndVolume.Create( WS_CHILD|WS_TABSTOP|TBS_HORZ|TBS_NOTICKS|TBS_TOP,
		rectDefault, &m_wndToolBar, IDC_MEDIA_VOLUME );
	m_wndVolume.SetRange( 0, 100 );
	m_wndVolume.SetTic( 0 );
	m_wndVolume.SetTic( 100 );

	if ( Settings.General.LanguageRTL )
	{
		m_wndPosition.ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );
		m_wndSpeed.ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );
		m_wndVolume.ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );
	}

	CBitmap bmIcons;
	bmIcons.LoadBitmap( IDB_MEDIA_STATES );
	m_pIcons.Create( 16, 16, ILC_COLOR32|ILC_MASK, 3, 0 ) ||
	m_pIcons.Create( 16, 16, ILC_COLOR24|ILC_MASK, 3, 0 ) ||
	m_pIcons.Create( 16, 16, ILC_COLOR16|ILC_MASK, 3, 0 );
	m_pIcons.Add( &bmIcons, RGB( 0, 255, 0 ) );

	m_wndList.LoadTextList(
		Settings.General.UserPath + _T("\\Data\\Default.m3u") );

	UpdateState();

	SetTimer( 1, 200, NULL );

	return 0;
}

void CMediaFrame::OnDestroy()
{
	m_wndList.SaveTextList(
		Settings.General.UserPath + _T("\\Data\\Default.m3u") );

	Settings.MediaPlayer.ListVisible	= m_bListVisible != FALSE;
	Settings.MediaPlayer.ListSize		= m_nListSize;
	Settings.MediaPlayer.StatusVisible	= m_bStatusVisible != FALSE;

	KillTimer( 2 );
	KillTimer( 1 );

	Cleanup();

	if ( ! m_bScreenSaverEnabled ) EnableScreenSaver();

	CWnd::OnDestroy();
}

BOOL CMediaFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if ( m_wndList.m_hWnd )
	{
		if ( m_wndList.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) ) return TRUE;
	}
	if ( m_wndListBar.m_hWnd )
	{
		if ( m_wndListBar.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) ) return TRUE;
	}
	if ( m_wndToolBar.m_hWnd )
	{
		if ( m_wndToolBar.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) ) return TRUE;
	}

	return CWnd::OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );
}

BOOL CMediaFrame::PreTranslateMessage(MSG* pMsg)
{
	switch ( pMsg->message )
	{
	case WM_KEYDOWN:
		switch( pMsg->wParam )
		{
		case VK_ESCAPE:
			if ( m_bFullScreen )
			{
				SetFullScreen( FALSE );
				return TRUE;
			}
			break;

		case VK_UP:
			OffsetVolume( VOLUME_KEY_MULTIPLIER );
			return TRUE;

		case VK_DOWN:
			OffsetVolume( - VOLUME_KEY_MULTIPLIER );
			return TRUE;

		case VK_LEFT:
			if ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
				m_wndList.PostMessage( WM_COMMAND, ID_MEDIA_PREVIOUS );
			else if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
				OffsetPosition( -2 );
			return TRUE;

		case VK_RIGHT:
			if ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
				m_wndList.PostMessage( WM_COMMAND, ID_MEDIA_NEXT );
			else if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
				OffsetPosition( 2 );
			return TRUE;

		case VK_SPACE:
			PostMessage( WM_COMMAND, m_nState == smsPlaying ? ID_MEDIA_PAUSE : ID_MEDIA_PLAY );
			return TRUE;
		}
		break;

	case WM_SYSKEYDOWN:
		if ( pMsg->wParam == VK_RETURN )
		{
			SetFullScreen( ! m_bFullScreen );
			return TRUE;
		}
		break;
	}

	return CWnd::PreTranslateMessage( pMsg );
}

/////////////////////////////////////////////////////////////////////////////
// CMediaFrame presentation message handlers

void CMediaFrame::OnSkinChange()
{
	Skin.CreateToolBar( _T("CMediaFrame"), &m_wndToolBar );
	Skin.CreateToolBar( _T("CMediaList"), &m_wndListBar );

	if ( CCoolBarItem* pItem = m_wndToolBar.GetID( IDC_MEDIA_POSITION ) ) pItem->Enable( FALSE );
	if ( CCoolBarItem* pItem = m_wndToolBar.GetID( IDC_MEDIA_SPEED ) ) pItem->Enable( FALSE );
	if ( CCoolBarItem* pItem = m_wndToolBar.GetID( IDC_MEDIA_VOLUME ) ) pItem->Enable( FALSE );

	HICON hIcon = CoolInterface.ExtractIcon( (UINT)ID_MEDIA_STATE_STOP, FALSE );
	if ( hIcon )
	{
		m_pIcons.Replace( 0, hIcon );
		DestroyIcon( hIcon );
	}
	hIcon = CoolInterface.ExtractIcon( (UINT)ID_MEDIA_STATE_PAUSE, FALSE );
	if ( hIcon )
	{
		m_pIcons.Replace( 1, hIcon );
		DestroyIcon( hIcon );
	}
	hIcon = CoolInterface.ExtractIcon( (UINT)ID_MEDIA_STATE_PLAY, FALSE );
	if ( hIcon )
	{
		m_pIcons.Replace( 2, hIcon );
		DestroyIcon( hIcon );
	}

	m_wndList.OnSkinChange();
}

void CMediaFrame::OnUpdateCmdUI()
{
	m_wndToolBar.OnUpdateCmdUI( (CFrameWnd*)GetOwner(), TRUE );
	m_wndListBar.OnUpdateCmdUI( (CFrameWnd*)GetOwner(), TRUE );
}

void CMediaFrame::SetFullScreen(BOOL bFullScreen)
{
	if ( bFullScreen == m_bFullScreen ) return;

	ShowWindow( SW_HIDE );
	m_tBarTime = GetTickCount();

	m_bFullScreen = bFullScreen;
	if ( m_bFullScreen )
	{
		ModifyStyle( WS_CHILD, 0 );
		SetParent( NULL );

		HMONITOR hMonitor = MonitorFromWindow( AfxGetMainWnd()->GetSafeHwnd(),
			MONITOR_DEFAULTTOPRIMARY );

		MONITORINFO oMonitor = {0};
		oMonitor.cbSize = sizeof( MONITORINFO );
		GetMonitorInfo( hMonitor, &oMonitor );

		SetWindowPos( &wndTopMost, oMonitor.rcMonitor.left, oMonitor.rcMonitor.top,
			oMonitor.rcMonitor.right - oMonitor.rcMonitor.left,
			oMonitor.rcMonitor.bottom - oMonitor.rcMonitor.top, SWP_FRAMECHANGED|SWP_SHOWWINDOW );

		m_bListWasVisible 	= m_bListVisible;
		m_bListVisible 		= FALSE;
		OnSize( SIZE_INTERNAL, 0, 0 );

		SetTimer( 2, 30, NULL );
	}
	else
	{
		CWnd* pOwner = GetOwner();
		CRect rc;

		ModifyStyle( 0, WS_CHILD );
		SetParent( pOwner );

		pOwner->GetClientRect( &rc );
		SetWindowPos( NULL, 0, 0, rc.right, rc.bottom,
			SWP_FRAMECHANGED|SWP_SHOWWINDOW );
		m_bListVisible = m_bListWasVisible;
		OnSize( SIZE_INTERNAL, 0, 0 );
		KillTimer( 2 );
	}
}

void CMediaFrame::OnSize(UINT nType, int cx, int cy)
{
	HRESULT hr;

	if ( nType != SIZE_INTERNAL && nType != SIZE_BARSLIDE )
		CWnd::OnSize( nType, cx, cy );

	CRect rc;
	GetClientRect( &rc );

	if ( rc.Width() < 32 || rc.Height() < 32 ) return;

	if ( rc.Width() < m_nListSize + SPLIT_SIZE )
	{
		m_nListSize = max( 0, rc.Width() - SPLIT_SIZE );
	}

	if ( m_bListVisible || ! m_bFullScreen )
	{
		rc.bottom -= TOOLBAR_HEIGHT;
		m_wndToolBar.SetWindowPos( NULL, rc.left, rc.bottom, rc.Width(),
			TOOLBAR_HEIGHT, SWP_NOZORDER|SWP_SHOWWINDOW );

		if ( m_bListVisible )
		{
			rc.right -= m_nListSize;
			m_wndList.SetWindowPos( NULL, rc.right, rc.top + HEADER_HEIGHT, m_nListSize,
				rc.bottom - TOOLBAR_HEIGHT - HEADER_HEIGHT, SWP_NOZORDER|SWP_SHOWWINDOW );
			m_wndListBar.SetWindowPos( NULL, rc.right, rc.bottom - TOOLBAR_HEIGHT,
				m_nListSize, TOOLBAR_HEIGHT, SWP_NOZORDER|SWP_SHOWWINDOW );
			rc.right -= SPLIT_SIZE;
		}
		else if ( m_wndList.IsWindowVisible() )
		{
			m_wndList.ShowWindow( SW_HIDE );
			m_wndListBar.ShowWindow( SW_HIDE );
		}
	}
	else
	{
		if ( m_wndList.IsWindowVisible() )
		{
			m_wndList.ShowWindow( SW_HIDE );
			m_wndListBar.ShowWindow( SW_HIDE );
		}

		DWORD tElapse = GetTickCount() - m_tBarTime;
		int nBar = TOOLBAR_HEIGHT;

		if ( tElapse < TOOLBAR_STICK )
		{
			nBar = TOOLBAR_HEIGHT;
		}
		else if ( tElapse > TOOLBAR_STICK + TOOLBAR_ANIMATE )
		{
			nBar = 0;
			SetCursor( NULL );
			KillTimer( 2 );
		}
		else
		{
			tElapse -= TOOLBAR_STICK;
			nBar = TOOLBAR_HEIGHT - ( tElapse * TOOLBAR_HEIGHT / TOOLBAR_ANIMATE );
		}

		m_wndToolBar.SetWindowPos( NULL, rc.left, rc.bottom - nBar, rc.Width(),
			TOOLBAR_HEIGHT, SWP_NOZORDER|SWP_SHOWWINDOW );
	}

	if ( m_bStatusVisible )
	{
		if ( m_bFullScreen )
		{
			m_rcStatus.SetRect( rc.left, rc.top, rc.right, rc.top + STATUS_HEIGHT );
			rc.top += STATUS_HEIGHT;
		}
		else
		{
			m_rcStatus.SetRect( rc.left, rc.bottom - STATUS_HEIGHT, rc.right, rc.bottom );
			rc.bottom -= STATUS_HEIGHT;
		}
	}

	m_rcVideo = rc;

	if ( m_pPlayer && nType != SIZE_BARSLIDE )
	{
		hr = m_pPlayer->Reposition( rc.left, rc.top, rc.Width(), rc.Height() );
		if ( FAILED( hr ) )
		{
			Cleanup( TRUE );
			return;
		}
	}

	if ( nType != SIZE_BARSLIDE ) Invalidate();
}

void CMediaFrame::OnPaint()
{
	CPaintDC dc( this );

	if ( m_bmLogo.m_hObject == NULL)
	{
		m_bmLogo.m_hObject = CImageFile::LoadBitmapFromResource( IDR_LARGE_LOGO, RT_JPEG );
//		if ( m_pPlayer && m_bmLogo.m_hObject )
//			m_pPlayer->SetLogoBitmap( m_bmLogo );
	}

	if ( m_pFontDefault.m_hObject == NULL )
	{
		LOGFONT pFont = { 80, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH|FF_DONTCARE, _T("Tahoma") };

		m_pFontDefault.CreatePointFontIndirect( &pFont );

		pFont.lfHeight = 80;
		m_pFontValue.CreatePointFontIndirect( &pFont );
		pFont.lfWeight = FW_BLACK;
		m_pFontKey.CreatePointFontIndirect( &pFont );
	}

	CFont* pOldFont = (CFont*)dc.SelectObject( &m_pFontDefault );

	CRect rcClient;
	GetClientRect( &rcClient );

	if ( m_bListVisible )
	{
		CRect rcBar(	rcClient.right - m_nListSize - SPLIT_SIZE,
						rcClient.top,
						rcClient.right - m_nListSize,
						rcClient.bottom - TOOLBAR_HEIGHT );

		dc.FillSolidRect( rcBar.left, rcBar.top, 1, rcBar.Height(), CoolInterface.m_crResizebarEdge );
		dc.FillSolidRect( rcBar.left + 1, rcBar.top, 1, rcBar.Height(), CoolInterface.m_crResizebarHighlight );
		dc.FillSolidRect( rcBar.right - 1, rcBar.top, 1, rcBar.Height(), CoolInterface.m_crResizebarShadow );
		dc.FillSolidRect( rcBar.left + 2, rcBar.top, rcBar.Width() - 3, rcBar.Height(), CoolInterface.m_crResizebarFace );
		dc.ExcludeClipRect( &rcBar );

		rcBar.SetRect( rcBar.right, rcClient.top,
			rcClient.right, rcClient.top + HEADER_HEIGHT );

		if ( dc.RectVisible( &rcBar ) )
			PaintListHeader( dc, rcBar );
	}

	if ( m_bStatusVisible )
	{
		CRect rcStatus( &m_rcStatus );
		if ( dc.RectVisible( &rcStatus ) ) PaintStatus( dc, rcStatus );
	}

	if ( dc.RectVisible( &m_rcVideo ) && ! m_bNoLogo )
	{
		PaintSplash( dc, m_rcVideo );
	}

	dc.SelectObject( pOldFont );
}

void CMediaFrame::PaintSplash(CDC& dc, CRect& /*rcBar*/)
{
	if ( m_bmLogo.m_hObject == NULL )
	{
		dc.FillSolidRect( &m_rcVideo, CoolInterface.m_crMediaWindow );
		return;
	}

	BITMAP pInfo;
	m_bmLogo.GetBitmap( &pInfo );

	CPoint pt = m_rcVideo.CenterPoint();
	pt.x -= pInfo.bmWidth / 2;
	pt.y -= ( pInfo.bmHeight + 32 ) / 2;

	CDC dcMem;
	dcMem.CreateCompatibleDC( &dc );
	CBitmap* pOldBmp = (CBitmap*)dcMem.SelectObject( &m_bmLogo );
	dc.BitBlt( pt.x, pt.y, pInfo.bmWidth, pInfo.bmHeight, &dcMem,
		0, 0, SRCCOPY );
	dc.ExcludeClipRect( pt.x, pt.y, pt.x + pInfo.bmWidth, pt.y + pInfo.bmHeight );
	dcMem.SelectObject( pOldBmp );

	CRect rcText( m_rcVideo.left, pt.y + pInfo.bmHeight, m_rcVideo.right, pt.y + pInfo.bmHeight + 32 );

	CString strText;
	LoadString( strText, IDS_MEDIA_TITLE );

	pt.x = ( m_rcVideo.left + m_rcVideo.right ) / 2 - dc.GetTextExtent( strText ).cx / 2;
	pt.y = rcText.top + 8;

	dc.SetBkColor( CoolInterface.m_crMediaWindow );
	dc.SetTextColor( CoolInterface.m_crMediaWindowText );
	dc.ExtTextOut( pt.x, pt.y, ETO_OPAQUE, &m_rcVideo, strText, NULL );
	dc.ExcludeClipRect( &rcText );

	dc.FillSolidRect( &m_rcVideo, CoolInterface.m_crMediaWindow );
}

void CMediaFrame::PaintListHeader(CDC& dc, CRect& rcBar)
{
	CString strText;
	CPoint pt = rcBar.CenterPoint();
	LoadString( strText, IDS_MEDIA_PLAYLIST );
	CSize szText = dc.GetTextExtent( strText );
	pt.x -= szText.cx / 2; pt.y -= szText.cy / 2;
	dc.SetBkMode( OPAQUE );
	dc.SetBkColor( CoolInterface.m_crMediaPanelCaption );
	dc.SetTextColor( CoolInterface.m_crMediaPanelCaptionText );
	dc.ExtTextOut( pt.x, pt.y, ETO_OPAQUE|ETO_CLIPPED, &rcBar, strText, NULL );
}

void CMediaFrame::PaintStatus(CDC& dc, CRect& rcBar)
{
	COLORREF crBack = CoolInterface.m_crMediaStatus;
	COLORREF crText = CoolInterface.m_crMediaStatusText;

	dc.SelectObject( &m_pFontValue );
	DWORD dwOptions = Settings.General.LanguageRTL ? ETO_RTLREADING : 0;

	int nY = ( rcBar.top + rcBar.bottom ) / 2 - dc.GetTextExtent( _T("Cy") ).cy / 2;
	CRect rcPart( &rcBar );
	CString str;
	CSize sz;

	int nState = 0;
	if ( m_nState >= smsPlaying ) nState = 2;
	else if ( m_nState >= smsPaused ) nState = 1;
	ImageList_DrawEx( m_pIcons, nState, dc, rcBar.left + 2,
		( rcBar.top + rcBar.bottom ) / 2 - 8, 16, 16,
		crBack, CLR_NONE, ILD_NORMAL );
	dc.ExcludeClipRect( rcBar.left + 2, ( rcBar.top + rcBar.bottom ) / 2 - 8,
		rcBar.left + 18, ( rcBar.top + rcBar.bottom ) / 2 + 8 );

	dc.SetBkMode( OPAQUE );
	dc.SetBkColor( crBack );
	dc.SetTextColor( crText );

	if ( CMetaItem* pItem = m_pMetadata.GetFirst() )
	{
		dc.SelectObject( &m_pFontKey );
		str				= Settings.General.LanguageRTL ? ':' + pItem->m_sKey : pItem->m_sKey + ':';
		sz				= dc.GetTextExtent( str );
		rcPart.left		= rcBar.left + 20;
		rcPart.right	= rcPart.left + sz.cx + 8;
		dc.ExtTextOut( rcPart.left + 4, nY, ETO_CLIPPED|ETO_OPAQUE|dwOptions, &rcPart, str, NULL );
		dc.ExcludeClipRect( &rcPart );

		dc.SelectObject( &m_pFontValue );
		sz				= dc.GetTextExtent( pItem->m_sValue );
		rcPart.left		= rcPart.right;
		rcPart.right	= rcPart.left + sz.cx + 8;
		dc.ExtTextOut( rcPart.left + 4, nY, ETO_CLIPPED|ETO_OPAQUE|dwOptions, &rcPart, pItem->m_sValue, NULL );
		dc.ExcludeClipRect( &rcPart );
	}
	else
	{
		if ( m_nState >= smsOpen )
		{
			int nSlash = m_sFile.ReverseFind( '\\' );
			str = nSlash >= 0 ? m_sFile.Mid( nSlash + 1 ) : m_sFile;
		}
		else
		{
			LoadString( str, IDS_MEDIA_EMPTY );
		}

		sz				= dc.GetTextExtent( str );
		rcPart.left		= rcBar.left + 20;
		rcPart.right	= rcPart.left + sz.cx + 8;
		dc.ExtTextOut( rcPart.left + 4, nY, ETO_CLIPPED|ETO_OPAQUE|dwOptions, &rcPart, str, NULL );
		dc.ExcludeClipRect( &rcPart );
	}

	if ( m_nState >= smsOpen )
	{
		CString strFormat;
		LoadString( strFormat, IDS_GENERAL_OF );
		strFormat = _T("%.2i:%.2i ") + strFormat + _T(" %.2i:%.2i");
		if ( Settings.General.LanguageRTL ) strFormat = _T("\x200F") + strFormat;

		str.Format( strFormat,
			(int)( ( m_nPosition / ONE_SECOND ) / 60 ),
			(int)( ( m_nPosition / ONE_SECOND ) % 60 ),
			(int)( ( m_nLength / ONE_SECOND ) / 60 ),
			(int)( ( m_nLength / ONE_SECOND ) % 60 ) );

		sz				= dc.GetTextExtent( str );
		rcPart.right	= rcBar.right;
		rcPart.left		= rcPart.right - sz.cx - 8;

		dc.ExtTextOut( rcPart.left + 4, nY, ETO_CLIPPED|ETO_OPAQUE|dwOptions, &rcPart, str, NULL );
		dc.ExcludeClipRect( &rcPart );
	}

	dc.FillSolidRect( &rcBar, crBack );
	dc.SelectObject( &m_pFontDefault );
}

BOOL CMediaFrame::PaintStatusMicro(CDC& dc, CRect& rcBar)
{
	if ( m_nState <= smsOpen ) return FALSE;

	CRect rcStatus( &rcBar );
	CRect rcPart( &rcBar );
	CString str;
	CSize sz;
	CSize size = rcBar.Size();
	CDC* pMemDC = CoolInterface.GetBuffer( dc, size );

	DWORD dwOptions = Settings.General.LanguageRTL ? DT_RTLREADING : 0;
	if ( m_nState >= smsOpen )
	{
		CString strFormat;
		LoadString( strFormat, IDS_GENERAL_OF );
		strFormat = _T("%.2i:%.2i ") + strFormat + _T(" %.2i:%.2i");
		if ( Settings.General.LanguageRTL ) strFormat = _T("\x200F") + strFormat;

		str.Format( strFormat,
			(int)( ( m_nPosition / ONE_SECOND ) / 60 ),
			(int)( ( m_nPosition / ONE_SECOND ) % 60 ),
			(int)( ( m_nLength / ONE_SECOND ) / 60 ),
			(int)( ( m_nLength / ONE_SECOND ) % 60 ) );

		sz				= pMemDC->GetTextExtent( str );
		rcPart.right	= rcStatus.right;
		rcPart.left		= rcPart.right - sz.cx - 2;
		rcStatus.right	= rcPart.left;

		pMemDC->DrawText( str, &rcPart, DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_RIGHT );
	}

	if ( CMetaItem* pItem = m_pMetadata.GetFirst() )
	{
		str				= Settings.General.LanguageRTL ? ':' + pItem->m_sKey : pItem->m_sKey + ':';
		sz				= pMemDC->GetTextExtent( str );
		rcPart.left		= rcStatus.left;
		rcPart.right	= rcPart.left + sz.cx + 2;
		rcStatus.left	= rcPart.right;

		pMemDC->DrawText( str, &rcPart, DT_SINGLELINE|DT_VCENTER|DT_LEFT|DT_NOPREFIX|dwOptions );
		pMemDC->DrawText( pItem->m_sValue, &rcStatus, DT_SINGLELINE|DT_VCENTER|DT_LEFT|DT_NOPREFIX|DT_END_ELLIPSIS|dwOptions );
	}
	else
	{
		if ( m_nState >= smsOpen )
		{
			int nSlash = m_sFile.ReverseFind( '\\' );
			str = nSlash >= 0 ? m_sFile.Mid( nSlash + 1 ) : m_sFile;
		}

		pMemDC->DrawText( str, &rcStatus, DT_SINGLELINE|DT_VCENTER|DT_LEFT|DT_NOPREFIX|DT_END_ELLIPSIS|dwOptions );
	}

	if ( Settings.General.LanguageRTL )
		dc.StretchBlt( rcBar.Width() + rcBar.left, rcBar.top, -rcBar.Width(), rcBar.Height(),
			pMemDC, rcBar.left, rcBar.top, rcBar.Width(), rcBar.Height(), SRCCOPY );
	else
		dc.BitBlt( rcBar.left, rcBar.top, rcBar.Width(), rcBar.Height(),
			pMemDC, rcBar.left, rcBar.top, SRCCOPY );

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMediaFrame interaction message handlers

void CMediaFrame::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	Skin.TrackPopupMenu( _T("CMediaFrame"), point,
		m_nState == smsPlaying ? ID_MEDIA_PAUSE : ID_MEDIA_PLAY );
}

void CMediaFrame::OnTimer(UINT_PTR nIDEvent)
{
	if ( nIDEvent == 1 )
	{
		DWORD tNow = GetTickCount();

		UpdateState();

		if ( m_bFullScreen && ! m_bListVisible )
		{
			CPoint ptCursor;
			GetCursorPos( &ptCursor );

			if ( ptCursor != m_ptCursor )
			{
				m_tBarTime = tNow;
				m_ptCursor = ptCursor;
				SetTimer( 2, 50, NULL );
			}
		}

		if ( tNow - m_tMetadata > META_DELAY )
		{
			m_tMetadata = tNow;
			m_pMetadata.Shuffle();
		}
	}
	else if ( nIDEvent == 2 )
	{
		OnSize( SIZE_BARSLIDE, 0, 0 );
	}
}

void CMediaFrame::OnClose()
{
	SetFullScreen( FALSE );
}

BOOL CMediaFrame::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if ( m_bListVisible )
	{
		CRect rcClient, rc;
		CPoint point;

		GetCursorPos( &point );
		GetClientRect( &rcClient );
		ClientToScreen( &rcClient );

		rc.SetRect(	Settings.General.LanguageRTL ? rcClient.left + m_nListSize :
					rcClient.right - m_nListSize - SPLIT_SIZE,
					rcClient.top,
					Settings.General.LanguageRTL ? rcClient.left + m_nListSize + SPLIT_SIZE :
					rcClient.right - m_nListSize,
					rcClient.bottom - TOOLBAR_HEIGHT );

		if ( rc.PtInRect( point ) )
		{
			SetCursor( AfxGetApp()->LoadStandardCursor( IDC_SIZEWE ) );
			return TRUE;
		}
	}
	else if ( m_bFullScreen )
	{
		DWORD tElapse = GetTickCount() - m_tBarTime;

		if ( tElapse > TOOLBAR_STICK + TOOLBAR_ANIMATE )
		{
			SetCursor( NULL );
			return TRUE;
		}
	}

	return CWnd::OnSetCursor( pWnd, nHitTest, message );
}

void CMediaFrame::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rcClient;
	GetClientRect( &rcClient );
	if ( theApp.m_bMenuWasVisible )
	{
		theApp.m_bMenuWasVisible = FALSE ;
		return;
	}
	if ( m_bListVisible )
	{
		CRect rcBar(	rcClient.right - m_nListSize - SPLIT_SIZE,
						rcClient.top,
						rcClient.right - m_nListSize,
						rcClient.bottom );

		if ( rcBar.PtInRect( point ) )
		{
			DoSizeList();
			return;
		}
	}

	if (	( m_bFullScreen && point.y <= STATUS_HEIGHT ) ||
			( ! m_bFullScreen && point.y >= rcClient.bottom - STATUS_HEIGHT - TOOLBAR_HEIGHT ) )
	{
		OnMediaStatus();
		return;
	}

	CRect rcSenseLess(	rcClient.right - m_nListSize - SPLIT_SIZE - 30,
						rcClient.top,
						rcClient.right - m_nListSize - SPLIT_SIZE,
						rcClient.bottom );

	if ( rcSenseLess.PtInRect( point ) )
		return;

	if ( m_nState == smsPlaying )
		OnMediaPause();
	else if ( m_nState == smsPaused )
		OnMediaPlay();

	CWnd::OnLButtonDown( nFlags, point );
}

void CMediaFrame::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	OnMediaFullScreen();

	if ( m_nState == smsPlaying )
		OnMediaPause();
	else if ( m_nState == smsPaused )
		OnMediaPlay();

	CWnd::OnLButtonDblClk( nFlags, point );
}

BOOL CMediaFrame::DoSizeList()
{
	MSG* pMsg = &AfxGetThreadState()->m_msgCur;
	CRect rcClient;
	CPoint point;

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

		GetCursorPos( &point );
		ScreenToClient( &point );

		int nSplit = rcClient.right - point.x;

		if ( nOffset == 0xFFFF ) nOffset = m_nListSize - nSplit;
		nSplit += nOffset;

		nSplit = max( nSplit, 0 );
		nSplit = min( nSplit, rcClient.right - SPLIT_SIZE );

		if ( nSplit < 8 )
			nSplit = 0;
		if ( nSplit > rcClient.right - SPLIT_SIZE - 8 )
			nSplit = rcClient.right - SPLIT_SIZE;

		if ( nSplit != m_nListSize )
		{
			m_nListSize = nSplit;
			OnSize( SIZE_INTERNAL, 0, 0 );
			Invalidate();
		}
	}

	ReleaseCapture();
	ClipCursor( NULL );

	return TRUE;
}

LRESULT CMediaFrame::OnMediaKey(WPARAM wParam, LPARAM lParam)
{
	if ( wParam != 1 && !IsTopParentActive() ) return 0;
	if ( mixerGetNumDevs() < 1 ) return 0;

	switch ( GET_APPCOMMAND_LPARAM( lParam ) )
	{
	case APPCOMMAND_MEDIA_NEXTTRACK:
		m_wndList.PostMessage( WM_COMMAND, ID_MEDIA_NEXT );
		return 1;

	case APPCOMMAND_MEDIA_PREVIOUSTRACK:
		m_wndList.PostMessage( WM_COMMAND, ID_MEDIA_PREVIOUS );
		return 1;

	case APPCOMMAND_MEDIA_STOP:
		OnMediaStop();
		return 1;

	case APPCOMMAND_VOLUME_MUTE:
	{
		MMRESULT result;
		HMIXER hMixer;
		// obtain a handle to the mixer device
		result = mixerOpen( &hMixer, MIXER_OBJECTF_MIXER, 0, 0, 0 );
		if ( result != MMSYSERR_NOERROR ) return 0;

		// get the speaker line of the mixer device
		MIXERLINE ml = {0};
		ml.cbStruct = sizeof(MIXERLINE);
		ml.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
		result = mixerGetLineInfo( reinterpret_cast<HMIXEROBJ>(hMixer), &ml,
			MIXER_GETLINEINFOF_COMPONENTTYPE );
		if ( result != MMSYSERR_NOERROR ) return 0;

		// get the mute control of the speaker line
		MIXERLINECONTROLS mlc = {0};
		MIXERCONTROL mc = {0};
		mlc.cbStruct = sizeof(MIXERLINECONTROLS);
		mlc.dwLineID = ml.dwLineID;
		mlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
		mlc.cControls = 1;
		mlc.pamxctrl = &mc;
		mlc.cbmxctrl = sizeof(MIXERCONTROL);
		result = mixerGetLineControls( reinterpret_cast<HMIXEROBJ>(hMixer), &mlc,
			MIXER_GETLINECONTROLSF_ONEBYTYPE );
		if ( result != MMSYSERR_NOERROR ) return 0;

		// set 1 channel if it controls mute state for all channels
		if ( MIXERCONTROL_CONTROLF_UNIFORM & mc.fdwControl )
			ml.cChannels = 1;

		// get the current mute values for all channels
		MIXERCONTROLDETAILS mcd = {0};
		MIXERCONTROLDETAILS_BOOLEAN* pmcd_b = new MIXERCONTROLDETAILS_BOOLEAN[ ml.cChannels ];
		mcd.cbStruct = sizeof(mcd);
		mcd.cChannels = ml.cChannels;
		mcd.cMultipleItems = mc.cMultipleItems;
		mcd.dwControlID = mc.dwControlID;
		mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN) * ml.cChannels;
		mcd.paDetails = pmcd_b;
		result = mixerGetControlDetails( reinterpret_cast<HMIXEROBJ>(hMixer), &mcd,
			MIXER_GETCONTROLDETAILSF_VALUE );

		if ( result == MMSYSERR_NOERROR )
		{
			// change mute values for all channels
			LONG lNewValue = LONG( pmcd_b->fValue == 0 );
			while ( ml.cChannels-- )
				pmcd_b[ ml.cChannels ].fValue = lNewValue;

			// set the mute status
			result = mixerSetControlDetails( reinterpret_cast<HMIXEROBJ>(hMixer), &mcd,
				MIXER_SETCONTROLDETAILSF_VALUE );
		}
		delete [] pmcd_b;

		// now mute Shareaza player control ( probably, not needed )
		PostMessage( WM_COMMAND, ID_MEDIA_MUTE );
		return 1;
	}

	case APPCOMMAND_VOLUME_DOWN:
		OffsetVolume( - VOLUME_KEY_MULTIPLIER );
		return 1;

	case APPCOMMAND_VOLUME_UP:
		OffsetVolume( VOLUME_KEY_MULTIPLIER );
		return 1;

	case APPCOMMAND_MEDIA_PLAY_PAUSE:
		PostMessage( WM_COMMAND, m_nState == smsPlaying ? ID_MEDIA_PAUSE : ID_MEDIA_PLAY );
		return 1;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CMediaFrame thumb bars

void CMediaFrame::OnHScroll(UINT nSBCode, UINT /* nPos */, CScrollBar* pScrollBar)
{
	HRESULT hr;

	if ( pScrollBar == (CScrollBar*)&m_wndVolume )
	{
		double nVolume = (double)m_wndVolume.GetPos() / 100.0f;

		if ( nVolume != Settings.MediaPlayer.Volume )
		{
			Settings.MediaPlayer.Volume = nVolume;
			if ( m_pPlayer )
			{
				hr = m_pPlayer->SetVolume( m_bMute ? 0 : Settings.MediaPlayer.Volume );
				if ( FAILED( hr ) )
				{
					Cleanup( TRUE );
					return;
				}
			}
		}
	}

	if ( m_pPlayer == NULL )
		return;

	MediaState nState = smsNull;
	hr = m_pPlayer->GetState( &nState );
	if ( FAILED( hr ) )
	{
		Cleanup( TRUE );
		return;
	}
	if ( nState < smsOpen )
		return;

	if ( pScrollBar == (CScrollBar*)&m_wndPosition )
	{
		LONGLONG nLength = 0;
		hr = m_pPlayer->GetLength( &nLength );
		if ( FAILED( hr ) )
		{
			Cleanup( TRUE );
			return;
		}
		nLength /= TIME_FACTOR;

		LONGLONG nPosition = 0;
		hr = m_pPlayer->GetPosition( &nPosition );
		if ( FAILED( hr ) )
		{
			Cleanup( TRUE );
			return;
		}
		nPosition /= TIME_FACTOR;

		switch ( nSBCode )
		{
		case TB_TOP:
			nPosition = 0;
			break;
		case TB_BOTTOM:
			nPosition = nLength;
			break;
		case TB_LINEUP:
			nPosition -= 5;
			break;
		case TB_LINEDOWN:
			nPosition += 5;
			break;
		case TB_PAGEUP:
		case TB_PAGEDOWN:
			{
				CRect rc1, rc2;
				CPoint pt;

				GetCursorPos( &pt );
				pScrollBar->GetWindowRect( &rc1 );
				((CSliderCtrl*)pScrollBar)->GetChannelRect( &rc2 );

				if ( rc1.PtInRect( pt ) )
				{
					rc2.OffsetRect( rc1.left, rc1.top );
					if ( pt.x <= rc2.left )
						nPosition = 0;
					else if ( pt.x >= rc2.right )
						nPosition = nLength;
					else
						nPosition = (LONGLONG)( (double)( pt.x - rc2.left ) / (double)rc2.Width() * (double)nLength );
				}
			}
			break;
		case TB_THUMBPOSITION:
		case TB_THUMBTRACK:
			nPosition = m_wndPosition.GetPos();
			break;
		}

		if ( nState == smsOpen )
			nPosition = 0;
		if ( nPosition < 0 )
			nPosition = 0;
		if ( nPosition > nLength )
			nPosition = nLength;

		if ( nState == smsPlaying )
		{
			hr = m_pPlayer->Pause();
			if ( FAILED( hr ) )
			{
				Cleanup( TRUE );
				return;
			}
			m_bThumbPlay = TRUE;
		}

		hr = m_pPlayer->SetPosition( nPosition * TIME_FACTOR );
		if ( FAILED( hr ) )
		{
			Cleanup( TRUE );
			return;
		}
		m_wndPosition.SetPos( (int)nPosition );

		if ( m_bThumbPlay && nSBCode == TB_ENDTRACK )
		{
			hr = m_pPlayer->Play();
			if ( FAILED( hr ) )
			{
				Cleanup( TRUE );
				return;
			}
			m_bThumbPlay = FALSE;
		}

		UpdateState();
		UpdateWindow();
	}
	else if ( pScrollBar == (CScrollBar*)&m_wndSpeed )
	{
		double nNewSpeed = (double)m_wndSpeed.GetPos() / 100.0f;
		double nOldSpeed;

		if ( nSBCode == TB_TOP || nSBCode == TB_BOTTOM )
		{
			nNewSpeed = 1.0f;
			m_wndSpeed.SetPos( 100 );
		}

		hr = m_pPlayer->GetSpeed( &nOldSpeed );
		if ( FAILED( hr ) )
		{
			Cleanup( TRUE );
			return;
		}
		if ( nNewSpeed != nOldSpeed )
		{
			m_pPlayer->SetSpeed( nNewSpeed );
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMediaFrame command handlers

void CMediaFrame::OnUpdateMediaClose(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_wndList.GetItemCount() > 0 );
}

void CMediaFrame::OnMediaClose()
{
	Cleanup();
	m_wndList.Clear();
}

void CMediaFrame::OnUpdateMediaPlay(CCmdUI* pCmdUI)
{
	MediaState nState = m_nState;
	if ( m_bThumbPlay && nState == smsPaused ) nState = smsPlaying;
	pCmdUI->Enable( m_nState < smsPlaying );
	if ( CCoolBarItem* pItem = CCoolBarItem::FromCmdUI( pCmdUI ) )
		pItem->Show( nState != smsPlaying );
}

void CMediaFrame::OnMediaPlay()
{
	HRESULT hr;

	if ( m_nState < smsOpen )
	{
		if ( m_wndList.GetCount() == 0 )
			PostMessage( WM_COMMAND, ID_MEDIA_OPEN );
		else
			m_wndList.GetNext();
	}
	else
	{
		if ( m_pPlayer )
		{
			hr = m_pPlayer->Play();
			if ( FAILED( hr ) )
			{
				Cleanup( TRUE );
				return;
			}
		}
		if ( ! UpdateState() )
			return;
	}

	UpdateNowPlaying();
}

void CMediaFrame::OnUpdateMediaPause(CCmdUI* pCmdUI)
{
	MediaState nState = m_nState;
	if ( m_bThumbPlay && nState == smsPaused ) nState = smsPlaying;
	pCmdUI->Enable( nState == smsPlaying );
	if ( CCoolBarItem* pItem = CCoolBarItem::FromCmdUI( pCmdUI ) )
		pItem->Show( nState == smsPlaying );
}

void CMediaFrame::OnMediaPause()
{
	HRESULT hr;

	if ( m_pPlayer )
	{
		hr = m_pPlayer->Pause();
		if ( FAILED( hr ) )
		{
			Cleanup( TRUE );
			return;
		}
	}

	if ( ! UpdateState() )
		return;

	if ( ! m_bScreenSaverEnabled )
		EnableScreenSaver();

	UpdateNowPlaying(TRUE);
}

void CMediaFrame::OnUpdateMediaStop(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( m_nState > smsOpen );
}

void CMediaFrame::OnMediaStop()
{
	HRESULT hr;

	if ( m_pPlayer )
	{
		hr = m_pPlayer->Stop();
		if ( FAILED( hr ) )
		{
			Cleanup( TRUE );
			return;
		}
	}
	m_bStopFlag = TRUE;
	m_wndList.Reset();
	EnableScreenSaver();

	UpdateNowPlaying(TRUE);
}

void CMediaFrame::OnUpdateMediaFullScreen(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_bFullScreen );
}

void CMediaFrame::OnMediaFullScreen()
{
	SetFullScreen( ! m_bFullScreen );
}

void CMediaFrame::OnMediaZoom()
{
	CMenu* pMenu = Skin.GetMenu( _T("CMediaFrame.Zoom") );
	m_wndToolBar.ThrowMenu( ID_MEDIA_ZOOM, pMenu );
}

void CMediaFrame::OnUpdateMediaSizeFill(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.MediaPlayer.Zoom == smzFill );
}

void CMediaFrame::OnMediaSizeFill()
{
	ZoomTo( smzFill );
}

void CMediaFrame::OnUpdateMediaSizeDistort(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.MediaPlayer.Zoom == smzDistort );
}

void CMediaFrame::OnMediaSizeDistort()
{
	ZoomTo( smzDistort );
}

void CMediaFrame::OnUpdateMediaSizeOne(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.MediaPlayer.Zoom == 1 );
}

void CMediaFrame::OnMediaSizeOne()
{
	ZoomTo( (MediaZoom)1 );
}

void CMediaFrame::OnUpdateMediaSizeTwo(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.MediaPlayer.Zoom == 2 );
}

void CMediaFrame::OnMediaSizeTwo()
{
	ZoomTo( (MediaZoom)2 );
}

void CMediaFrame::OnUpdateMediaSizeThree(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( Settings.MediaPlayer.Zoom == 3 );
}

void CMediaFrame::OnMediaSizeThree()
{
	ZoomTo( (MediaZoom)3 );
}

void CMediaFrame::OnUpdateMediaAspectDefault(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( Settings.MediaPlayer.Zoom != smzDistort );
	pCmdUI->SetCheck( Settings.MediaPlayer.Aspect == smaDefault );
}

void CMediaFrame::OnMediaAspectDefault()
{
	AspectTo( smaDefault );
}

void CMediaFrame::OnUpdateMediaAspect43(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( Settings.MediaPlayer.Zoom != smzDistort );
	pCmdUI->SetCheck( fabs( Settings.MediaPlayer.Aspect - 4.0f/3.0f ) < 0.1f );
}

void CMediaFrame::OnMediaAspect43()
{
	AspectTo( (4.0f/3.0f) );
}

void CMediaFrame::OnUpdateMediaAspect169(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( Settings.MediaPlayer.Zoom != smzDistort );
	pCmdUI->SetCheck( fabs( Settings.MediaPlayer.Aspect - 16.0f/9.0f ) < 0.1f );
}

void CMediaFrame::OnMediaAspect169()
{
	AspectTo( (16.0f/9.0f) );
}

void CMediaFrame::OnUpdateMediaPlaylist(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_bListVisible );
}

void CMediaFrame::OnMediaPlaylist()
{
	m_bListVisible = ! m_bListVisible;
	m_tBarTime = GetTickCount();
	OnSize( SIZE_INTERNAL, 0, 0 );
}

void CMediaFrame::OnUpdateMediaStatus(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_bStatusVisible );
}

void CMediaFrame::OnMediaStatus()
{
	m_bStatusVisible = ! m_bStatusVisible;
	OnSize( SIZE_INTERNAL, 0, 0 );
}

void CMediaFrame::OnUpdateMediaVis(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( ! m_bFullScreen );
}

void CMediaFrame::OnMediaVis()
{
	CMediaVisDlg dlg( this );
	if ( dlg.DoModal() == IDOK ) PrepareVis();
}

void CMediaFrame::OnUpdateMediaSettings(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( ! m_bFullScreen );
}

void CMediaFrame::OnMediaSettings()
{
	CSettingsManagerDlg::Run( _T("CMediaSettingsPage") );
}

void CMediaFrame::OnUpdateMediaMute(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck( m_bMute );
}

void CMediaFrame::OnMediaMute()
{
	HRESULT hr;

	m_bMute = ! m_bMute;
	if ( m_pPlayer )
	{
		hr = m_pPlayer->SetVolume( m_bMute ? 0 : Settings.MediaPlayer.Volume );
		if ( FAILED( hr ) )
		{
			Cleanup( TRUE );
			return;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMediaFrame public media operations

BOOL CMediaFrame::PlayFile(LPCTSTR pszFile)
{
	UpdateWindow();

	DWORD tNow = GetTickCount();

	if ( tNow - m_tLastPlay < 500 )
	{
		m_tLastPlay = tNow;
		return EnqueueFile( pszFile );
	}
	else
	{
		m_tLastPlay = tNow;
		return m_wndList.Open( pszFile );
	}
}

BOOL CMediaFrame::EnqueueFile(LPCTSTR pszFile)
{
	m_bEnqueue = TRUE;
	BOOL bResult = m_wndList.Enqueue( pszFile, TRUE );
	m_bLastMedia = FALSE;
	m_bLastNotPlayed = TRUE;
	m_bEnqueue = FALSE;
	return bResult;
}

void CMediaFrame::OnFileDelete(LPCTSTR pszFile)
{
	if ( m_sFile.CompareNoCase( pszFile ) == 0 )
	{
		// Only remove from the list, the player cleans up itself
		m_wndList.Remove( pszFile );
	}
}

float CMediaFrame::GetPosition()
{
	if ( m_pPlayer && m_nState >= smsOpen && m_nLength > 0 )
	{
		return (float)m_nPosition / (float)m_nLength;
	}
	else
	{
		return 0;
	}
}

BOOL CMediaFrame::SeekTo(float nPosition)
{
	HRESULT hr;

	if ( m_pPlayer && m_nState >= smsPaused && m_nLength > 0 )
	{
		m_nPosition = (LONGLONG)( nPosition * (float)m_nLength );
		hr = m_pPlayer->SetPosition( m_nPosition );
		if ( FAILED( hr ) )
		{
			Cleanup( TRUE );
			return FALSE;
		}
		OnTimer( 1 );
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

float CMediaFrame::GetVolume()
{
	return (float)Settings.MediaPlayer.Volume;
}

BOOL CMediaFrame::SetVolume(float nVolume)
{
	HRESULT hr;

	Settings.MediaPlayer.Volume = (double)nVolume;
	if ( m_pPlayer )
	{
		hr = m_pPlayer->SetVolume( m_bMute ? 0 : Settings.MediaPlayer.Volume );
		if ( FAILED( hr ) )
		{
			Cleanup( TRUE );
			return FALSE;
		}
	}
	OnTimer( 1 );
	return ( m_pPlayer != NULL );
}

void CMediaFrame::OffsetVolume(int nVolumeOffset)
{
	HRESULT hr;

	KillTimer( 1 );
	int nVolumeTick = max( min( m_wndVolume.GetPos() + nVolumeOffset, 100 ), 0 );
	m_wndVolume.SetPos( nVolumeTick );
	Settings.MediaPlayer.Volume = (double)nVolumeTick / 100.0f;
	if ( m_pPlayer )
	{
		hr = m_pPlayer->SetVolume( Settings.MediaPlayer.Volume );
		if ( FAILED( hr ) )
		{
			Cleanup( TRUE );
			return;
		}
	}
	UpdateState();
	SetTimer( 1, 200, NULL );
}

void CMediaFrame::OffsetPosition(int nPositionOffset)
{
	HRESULT hr;

	KillTimer( 1 );
	if ( m_pPlayer )
	{
		bool bPlaying = ( m_nState == smsPlaying );
		if ( bPlaying )
		{
			hr = m_pPlayer->Pause();
			if ( FAILED( hr ) )
			{
				Cleanup( TRUE );
				return;
			}
		}
		LONGLONG nPos = 0, nLen = 0;
		hr = m_pPlayer->GetPosition( &nPos );
		if ( FAILED( hr ) )
		{
			Cleanup( TRUE );
			return;
		}
		hr = m_pPlayer->GetLength( &nLen );
		if ( FAILED( hr ) )
		{
			Cleanup( TRUE );
			return;
		}
		nPos = max( min( nPos + nPositionOffset * TIME_FACTOR, nLen ), 0 );
		hr = m_pPlayer->SetPosition( nPos );
		if ( FAILED( hr ) )
		{
			Cleanup( TRUE );
			return;
		}
		if ( bPlaying )
		{
			hr = m_pPlayer->Play();
			if ( FAILED( hr ) )
			{
				Cleanup( TRUE );
				return;
			}
		}
	}
	UpdateState();
	SetTimer( 1, 200, NULL );
}

/////////////////////////////////////////////////////////////////////////////
// CMediaFrame private media operations

BOOL CMediaFrame::Prepare()
{
	m_bThumbPlay = FALSE;

	if ( m_pPlayer )
		return TRUE;

	if ( GetSafeHwnd() == NULL )
		return FALSE;

	CWaitCursor pCursor;
	CLSID pCLSID;
	HRESULT hr = E_FAIL;

	if ( Plugins.LookupCLSID( _T("MediaPlayer"), _T("Default"), pCLSID ) )
	{
		HINSTANCE hRes = AfxGetResourceHandle();
		hr = m_pPlayer.CoCreateInstance( pCLSID );
		AfxSetResourceHandle( hRes );
	}

	if ( FAILED( hr ) )
		return FALSE;

	ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );

	hr = m_pPlayer->Create( (DOUBLE)(DWORD_PTR)GetSafeHwnd() );
	if ( FAILED( hr ) )
		return FALSE;

	if ( Settings.General.LanguageRTL ) ModifyStyleEx( 0, WS_EX_LAYOUTRTL, 0 );
	hr = m_pPlayer->SetZoom( Settings.MediaPlayer.Zoom );
	if ( FAILED( hr ) )
		return FALSE;

	hr = m_pPlayer->SetAspect( Settings.MediaPlayer.Aspect );
	if ( FAILED( hr ) )
		return FALSE;

	hr = m_pPlayer->SetVolume( m_bMute ? 0 : Settings.MediaPlayer.Volume );
	if ( FAILED( hr ) )
		return FALSE;

//	if ( m_bmLogo.m_hObject )
//		hr = m_pPlayer->SetLogoBitmap( (HBITMAP)m_bmLogo.m_hObject );

	HINSTANCE hRes = AfxGetResourceHandle();
	BOOL bSuccess = PrepareVis();
	AfxSetResourceHandle( hRes );
	if ( ! bSuccess )
		return FALSE;

	OnSize( SIZE_INTERNAL, 0, 0 );

	return UpdateState();
}

BOOL CMediaFrame::PrepareVis()
{
	HRESULT hr;

	if ( m_pPlayer == NULL ) return FALSE;

	IAudioVisPlugin* pPlugin = NULL;
	CLSID pCLSID;

	if ( Hashes::fromGuid( Settings.MediaPlayer.VisCLSID, &pCLSID ) &&
		 Plugins.LookupEnable( pCLSID ) )
	{
		hr = CoCreateInstance( pCLSID, NULL, CLSCTX_ALL, IID_IAudioVisPlugin,
			(void**)&pPlugin );

		if ( SUCCEEDED(hr) && pPlugin != NULL )
		{
			if ( Settings.MediaPlayer.VisPath.GetLength() )
			{
				IWrappedPluginControl* pWrap = NULL;

				hr = pPlugin->QueryInterface( IID_IWrappedPluginControl,
					(void**)&pWrap );

				if ( SUCCEEDED(hr) && pWrap != NULL )
				{
					hr = pWrap->Load( CComBSTR( Settings.MediaPlayer.VisPath ), 0 );
					pWrap->Release();
				}

				if ( FAILED(hr) )
				{
					pPlugin->Release();
					pPlugin = NULL;
				}
			}
		}
	}

	hr = m_pPlayer->SetPluginSize( Settings.MediaPlayer.VisSize );
	if ( FAILED( hr ) )
	{
		Cleanup( TRUE );
		return FALSE;
	}

	hr = m_pPlayer->SetPlugin( pPlugin );

	if ( pPlugin != NULL ) pPlugin->Release();

	return TRUE;
}

BOOL CMediaFrame::OpenFile(LPCTSTR pszFile)
{
	HRESULT hr;

	if ( ! Prepare() )
	{
		Cleanup();
		AfxMessageBox( IDS_MEDIA_PLUGIN_CREATE, MB_ICONEXCLAMATION );
		return FALSE;
	}

	if ( m_sFile == pszFile )
		return UpdateState();

	CWaitCursor pCursor;
	m_sFile = pszFile;
	m_pMetadata.Clear();

	HINSTANCE hRes = AfxGetResourceHandle();
	hr = PluginPlay( CComBSTR( pszFile ) );
	AfxSetResourceHandle( hRes );
	if ( FAILED( hr ) )
	{
		Cleanup( TRUE );
		return FALSE;
	}

	if ( ! UpdateState() )
		return FALSE;

	pCursor.Restore();
	m_tMetadata = GetTickCount();

	{
		CSingleLock oLock( &Library.m_pSection, TRUE );
		if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( m_sFile ) )
		{
			m_pMetadata.Add( _T("Filename"), pFile->m_sName );
			m_pMetadata.Setup( pFile->m_pSchema, FALSE );
			m_pMetadata.Combine( pFile->m_pMetadata );
			m_pMetadata.Clean( 1024 );
			oLock.Unlock();

			CMetaItem* pWidth	= m_pMetadata.Find( _T("Width") );
			CMetaItem* pHeight	= m_pMetadata.Find( _T("Height") );

			if ( pWidth != NULL && pHeight != NULL )
			{
				pWidth->m_sKey = _T("Dimensions");
				pWidth->m_sValue += 'x' + pHeight->m_sValue;
				m_pMetadata.Remove( _T("Height") );
			}
		}
	}

	if ( hr != S_OK )
	{
		CString strMessage;
		LoadString( strMessage, IDS_MEDIA_PARTIAL_RENDER );
		m_pMetadata.Add( _T("Warning"), strMessage );
	}

	m_bNoLogo = VARIANT_FALSE;
	if ( m_pPlayer )
	{
		hr = m_pPlayer->IsWindowVisible( &m_bNoLogo );
		if ( FAILED( hr ) )
		{
			Cleanup( TRUE );
			return FALSE;
		}
	}

	return TRUE;
}

HRESULT CMediaFrame::PluginPlay(BSTR bsFilePath)
{
	__try
	{
		m_pPlayer->Close();

		return m_pPlayer->Open( bsFilePath );
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		Cleanup( TRUE );
		return E_FAIL;
	}
}

void CMediaFrame::ReportError()
{
	LPCTSTR pszBase = PathFindFileName( m_sFile );
	CString strMessage;
	strMessage.Format( LoadString( IDS_MEDIA_LOAD_FAIL ), pszBase );
	m_pMetadata.Add( _T("Error"), strMessage );
	m_pMetadata.Add( _T("Error"), LoadString( IDS_MEDIA_LOAD_FAIL_HELP ) );

	theApp.Message( MSG_ERROR, _T("%s"), strMessage );

	AfxMessageBox( strMessage + _T("\r\n\r\n") +
		LoadString( IDS_MEDIA_LOAD_FAIL_HELP ), MB_ICONEXCLAMATION );
}

void CMediaFrame::Cleanup(BOOL bUnexpected)
{
	if ( m_pPlayer )
	{
		HINSTANCE hRes = AfxGetResourceHandle();
		__try
		{
			m_pPlayer->Close();
		}
		__except( EXCEPTION_EXECUTE_HANDLER  )
		{
		}
		__try
		{
			m_pPlayer->Destroy();
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
		{
			// Some buggy visualizations crash Shareaza.
			// Restore screen saver and power settings before
			EnableScreenSaver();
		}
		m_pPlayer.Release();

		AfxSetResourceHandle( hRes );
	}

	m_nState = smsNull;
	m_pMetadata.Clear();
	m_bNoLogo = VARIANT_FALSE;

	UpdateState();
	Invalidate();

	if ( bUnexpected && m_sFile.GetLength() )
	{
		ReportError();
	}

	m_sFile.Empty();
}

void CMediaFrame::ZoomTo(MediaZoom nZoom)
{
	HRESULT hr;

	if ( Settings.MediaPlayer.Zoom == nZoom ) return;
	Settings.MediaPlayer.Zoom = nZoom;
	if ( m_pPlayer )
	{
		hr = m_pPlayer->SetZoom( Settings.MediaPlayer.Zoom );
		if ( FAILED( hr ) )
		{
			Cleanup( TRUE );
			return;
		}
	}
}

void CMediaFrame::AspectTo(double nAspect)
{
	HRESULT hr;

	if ( Settings.MediaPlayer.Aspect == nAspect ) return;
	Settings.MediaPlayer.Aspect = nAspect;
	if ( m_pPlayer )
	{
		hr = m_pPlayer->SetAspect( Settings.MediaPlayer.Aspect );
		if ( FAILED( hr ) )
		{
			Cleanup( TRUE );
			return;
		}
	}
}

BOOL CMediaFrame::UpdateState()
{
	HRESULT hr;

	m_nState = smsNull;

	if ( m_pPlayer )
	{
		hr = m_pPlayer->GetState( &m_nState );
		if ( FAILED( hr ) )
		{
			Cleanup( TRUE );
			return FALSE;
		}
	}
	if ( m_pPlayer && m_nState >= smsOpen )
	{
		m_nLength = 0;
		hr = m_pPlayer->GetLength( &m_nLength );
		if ( FAILED( hr ) )
		{
			Cleanup( TRUE );
			return FALSE;
		}
		int nLength = (int)( m_nLength / TIME_FACTOR );

		m_nPosition = 0;
		hr = m_pPlayer->GetPosition( &m_nPosition );
		if ( FAILED( hr ) )
		{
			Cleanup( TRUE );
			return FALSE;
		}
		int nPosition = (int)( m_nPosition / TIME_FACTOR );

		m_wndPosition.EnableWindow( TRUE );
		m_wndPosition.SetRangeMax( (int)nLength );
		m_wndPosition.SetPos( (int)nPosition );

		double nSpeed = 1.0f;
		hr = m_pPlayer->GetSpeed( &nSpeed );
		if ( FAILED( hr ) )
		{
			Cleanup( TRUE );
			return FALSE;
		}
		m_wndSpeed.SetPos( (int)( nSpeed * 100 ) );
		m_wndSpeed.EnableWindow( TRUE );

		if ( ! m_bMute )
		{
			Settings.MediaPlayer.Volume = 1.0f;
			hr = m_pPlayer->GetVolume( &Settings.MediaPlayer.Volume );
			if ( FAILED( hr ) )
			{
				Cleanup( TRUE );
				return FALSE;
			}
		}

		if ( m_nState == smsPlaying && nPosition >= nLength && nPosition != 0 )
		{
			m_wndList.GetNext();
		}
	}
	else
	{
		if ( m_nState != smsNull && m_wndList.GetCount() == 0 ) Cleanup();
		m_wndPosition.SetPos( 0 );
		m_wndPosition.SetRange( 0, 0 );
		m_wndPosition.EnableWindow( FALSE );
		m_wndSpeed.SetPos( 100 );
		m_wndSpeed.EnableWindow( FALSE );
	}

	m_wndVolume.SetPos( (int)( Settings.MediaPlayer.Volume * 100 ) );

	if ( m_bStatusVisible )
	{
		InvalidateRect( &m_rcStatus );
	}

	return TRUE;
}

void CMediaFrame::OnNewCurrent(NMHDR* /*pNotify*/, LRESULT* pResult)
{
	HRESULT hr;

	int nCurrent = m_wndList.GetCurrent();
	m_wndList.UpdateWindow();
	m_bRepeat = Settings.MediaPlayer.Repeat;
	m_bLastMedia = ( nCurrent == m_wndList.GetItemCount() - 1 );

	if ( m_bStopFlag )
	{
		m_bStopFlag = FALSE;
		m_bLastNotPlayed = FALSE;
		Cleanup();
		if ( ! m_bScreenSaverEnabled ) EnableScreenSaver();
		*pResult = 0;
		return;
	}

	if ( nCurrent >= 0 ) // not the last in the list
	{
		BOOL bPlayIt;
		BOOL bCorrupted = FALSE;

		if ( m_bEnqueue )
			bPlayIt = FALSE;
		else
		{
			// Play when repeat is on or when whithin the playlist
			bPlayIt = m_bRepeat || ! m_bLastMedia || m_bLastNotPlayed;
			// The whole playlist was played and the list was reset
			if ( ! m_bRepeat && m_bLastNotPlayed && ! m_bLastMedia )
			{
				if ( m_nState != smsPlaying )
					bPlayIt = FALSE;
				else // New file was clicked, clear the flag
					m_bLastNotPlayed = FALSE;
			}

			if ( ! m_pPlayer || bPlayIt || m_wndList.GetItemCount() == 1 )
			{
				bPlayIt = TRUE;
				bCorrupted = ! OpenFile( m_wndList.GetPath( nCurrent ) );
			}
		}

		if ( m_pPlayer && bPlayIt && ! bCorrupted )
		{
			hr = m_pPlayer->Play();
			if ( FAILED( hr ) )
			{
				Cleanup( TRUE );
				return;
			}
			if ( m_bScreenSaverEnabled ) DisableScreenSaver();
			// check if the last was not played; flag only when we are playing the file before it
			if ( ! m_bLastNotPlayed )
				m_bLastNotPlayed = ( nCurrent == m_wndList.GetItemCount() - 2 );
			UpdateState();
		}
		else if ( bCorrupted ) // file was corrupted, move to the next file
		{
			nCurrent = m_wndList.GetNext( FALSE );
			if ( m_wndList.GetItemCount() != 1 )
				m_wndList.SetCurrent( nCurrent );
			else
				Cleanup(); //cleanup when no exception happened but the file couldn't be opened (png files)
		}
		else
		{
			// reset list and cleanup
			m_bLastNotPlayed = FALSE;
			m_wndList.Reset( TRUE );
			m_bStopFlag = FALSE;
			Cleanup();
		}
	}
	else if ( m_wndList.GetItemCount() > 0 ) // the list was reset; current file was set to -1
	{
		nCurrent = m_wndList.GetCurrent(); // get file #0

		if ( m_pPlayer )
		{
			if ( ! m_bRepeat )
				m_bStopFlag = TRUE;
			else
				nCurrent = m_wndList.GetNext( FALSE );
			if ( ! m_bEnqueue )
				m_wndList.SetCurrent( nCurrent );
		}
	}
	else
		Cleanup();

	*pResult = 0;

	UpdateNowPlaying();
}

/////////////////////////////////////////////////////////////////////////////
// Screen saver Enable / Disable functions

void CMediaFrame::DisableScreenSaver()
{
	if ( m_bScreenSaverEnabled )
	{
		GetActivePwrScheme( &m_nPowerSchemeId );				// get ID of current power scheme
		GetCurrentPowerPolicies( &m_CurrentGP, &m_CurrentPP );	// get active policies

		m_nVidAC = m_CurrentPP.user.VideoTimeoutAc;				// save current values
		m_nVidDC = m_CurrentPP.user.VideoTimeoutDc;

		m_CurrentPP.user.VideoTimeoutAc = 0;					// disallow display shutoff
		m_CurrentPP.user.VideoTimeoutDc = 0;

		// set new values
		SetActivePwrScheme( m_nPowerSchemeId, &m_CurrentGP, &m_CurrentPP );

		BOOL bParam = FALSE;
		BOOL bRetVal = SystemParametersInfo( SPI_GETSCREENSAVEACTIVE, 0, &bParam, 0 );
		if ( bRetVal && bParam )
		{
			// save current screen saver timeout value
			SystemParametersInfo( SPI_GETSCREENSAVETIMEOUT, 0, &m_nScreenSaverTime, 0 );
			// turn off screen saver
			SystemParametersInfo( SPI_SETSCREENSAVETIMEOUT, 0, NULL, 0 );
		}

		m_bScreenSaverEnabled = FALSE;
	}
}

void CMediaFrame::EnableScreenSaver()
{
	if ( ! m_bScreenSaverEnabled )
	{
		// restore previous values
		m_CurrentPP.user.VideoTimeoutAc = m_nVidAC;
		m_CurrentPP.user.VideoTimeoutDc = m_nVidDC;

		// set original values
		SetActivePwrScheme( m_nPowerSchemeId, &m_CurrentGP, &m_CurrentPP );

		// Restore screen saver timeout value if it's not zero.
		// Otherwise, if the screen saver was inactive,
		// it toggles it to active state and shutoff stops working (MS bug?)
		if ( m_nScreenSaverTime > 0 )
		{
			SystemParametersInfo( SPI_SETSCREENSAVETIMEOUT, m_nScreenSaverTime, NULL, 0 );
		}

		m_bScreenSaverEnabled = TRUE;
	}
}

void CMediaFrame::UpdateScreenSaverStatus(BOOL bWindowActive)
{
	if ( bWindowActive )
	{
		if ( m_bScreenSaverEnabled && IsPlaying() )
			DisableScreenSaver();
	}
	else
	{
		if ( ! m_bScreenSaverEnabled )
			EnableScreenSaver();
	}
}

void CMediaFrame::UpdateNowPlaying(BOOL bEmpty)
{
	if(bEmpty)
		m_sNowPlaying = _T("");
	else
	{
		m_sNowPlaying = m_sFile;

		// Strip path
		LPCTSTR pszFileName = _tcsrchr( m_sNowPlaying, '\\' );
		if ( pszFileName )
		{
			int nFileNameLen = static_cast< int >( _tcslen( pszFileName ) );
			m_sNowPlaying = m_sNowPlaying.Right( nFileNameLen - 1 );
		}

		// Strip extension
		LPCTSTR pszExt = _tcsrchr( m_sNowPlaying, '.' );
		if ( pszExt )
		{
			int nFileNameLen = static_cast< int >( pszExt - m_sNowPlaying );
			m_sNowPlaying = m_sNowPlaying.Left( nFileNameLen );
		}
	}

	CRegistry::SetString( _T("MediaPlayer"), _T("NowPlaying"), m_sNowPlaying );

	//Plugins.OnEvent(EVENT_CHANGEDSONG);	// ToDO: Maybe plug-ins can be alerted in some way
}
