//
// CtrlMediaFrame.cpp
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
#include "Settings.h"
#include "ImageServices.h"
#include "Plugins.h"
#include "Library.h"
#include "SharedFile.h"

#include "Skin.h"
#include "ShellIcons.h"
#include "CtrlMediaFrame.h"
#include "DlgSettingsManager.h"
#include "DlgMediaVis.h"

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
	ON_MESSAGE(0x0319, OnMediaKey)
END_MESSAGE_MAP()

#define SIZE_INTERNAL	1982
#define SIZE_BARSLIDE	1983
#define TOOLBAR_HEIGHT	28
#define TOOLBAR_STICK	4000
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
	
	m_pPlayer		= NULL;
	m_nState		= smsNull;
	m_bMute			= FALSE;
	m_bThumbPlay	= FALSE;
	m_bAutoPlay		= TRUE;
	m_tLastPlay		= 0;
	m_tMetadata		= 0;
	
	m_bFullScreen		= FALSE;
	m_bListVisible		= Settings.MediaPlayer.ListVisible;
	m_nListSize			= Settings.MediaPlayer.ListSize;
	m_bStatusVisible	= Settings.MediaPlayer.StatusVisible;
}

CMediaFrame::~CMediaFrame()
{
	if ( g_pMediaFrame == this ) g_pMediaFrame = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CMediaFrame system message handlers

BOOL CMediaFrame::Create(CWnd* pParentWnd) 
{
	CRect rect;
	return CWnd::Create( NULL, _T("CMediaFrame"), WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,
		rect, pParentWnd, IDC_MEDIA_FRAME, NULL );
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
	
	CBitmap bmIcons;
	bmIcons.LoadBitmap( IDB_MEDIA_STATES );
	m_pIcons.Create( 16, 16, ILC_COLOR16|ILC_MASK, 3, 0 );
	m_pIcons.Add( &bmIcons, RGB( 0, 255, 0 ) );
	
	UpdateState();
	
	SetTimer( 1, 200, NULL );
	
	return 0;
}

void CMediaFrame::OnDestroy() 
{
	Settings.MediaPlayer.ListVisible	= m_bListVisible;
	Settings.MediaPlayer.ListSize		= m_nListSize;
	Settings.MediaPlayer.StatusVisible	= m_bStatusVisible;
	
	KillTimer( 2 );
	KillTimer( 1 );
	
	Cleanup();
	
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

void CMediaFrame::OnSysCommand(UINT nID, LPARAM lParam) 
{
	switch ( nID & 0xFFF0 )
	{
	case SC_SCREENSAVE:
	case SC_MONITORPOWER:
		if ( m_nState == smsPlaying ) return;
		break;
	}
	
	CWnd::OnSysCommand( nID, lParam );
}

BOOL CMediaFrame::PreTranslateMessage(MSG* pMsg) 
{
	if ( pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE )
	{
		if ( m_bFullScreen )
		{
			SetFullScreen( FALSE );
			return TRUE;
		}
	}
	else if ( pMsg->message == WM_SYSKEYDOWN && pMsg->wParam == VK_RETURN )
	{
		SetFullScreen( ! m_bFullScreen );
		return TRUE;
	}
	else if ( pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_LEFT )
	{
		if ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
		{
			m_wndList.PostMessage( WM_COMMAND, ID_MEDIA_PREVIOUS );
			return TRUE;
		}
	}
	else if ( pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RIGHT )
	{
		if ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
		{
			m_wndList.PostMessage( WM_COMMAND, ID_MEDIA_NEXT );
			return TRUE;
		}
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
	
	if ( m_bFullScreen = bFullScreen )
	{
		ModifyStyle( WS_CHILD, 0 );
		SetParent( NULL );
		SetWindowPos( &wndTopMost, 0, 0, GetSystemMetrics( SM_CXSCREEN ),
			GetSystemMetrics( SM_CYSCREEN ), SWP_FRAMECHANGED|SWP_SHOWWINDOW );
		SetTimer( 2, 50, NULL );
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
		KillTimer( 2 );
	}
}

void CMediaFrame::OnSize(UINT nType, int cx, int cy) 
{
	if ( nType != SIZE_INTERNAL && nType != SIZE_BARSLIDE ) CWnd::OnSize( nType, cx, cy );
	
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
	
	if ( m_pPlayer != NULL && nType != SIZE_BARSLIDE )
	{
		m_pPlayer->Reposition( &rc );
	}
	
	if ( nType != SIZE_BARSLIDE ) Invalidate();
}

void CMediaFrame::OnPaint() 
{
	CPaintDC dc( this );
	
	if ( m_bmLogo.m_hObject == NULL)
	{
		if ( CImageServices::LoadBitmap( &m_bmLogo, IDR_LARGE_LOGO, RT_JPEG ) )
		{
			if ( m_pPlayer ) m_pPlayer->SetLogoBitmap( (HBITMAP)m_bmLogo.m_hObject );
		}
	}
	
	if ( m_pFontDefault.m_hObject == NULL )
	{
		LOGFONT pFont = { 80, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH|FF_DONTCARE, _T("Verdana") };
		
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
						rcClient.bottom );
		
		dc.FillSolidRect( rcBar.left, rcBar.top, 1, rcBar.Height(), GetSysColor( COLOR_BTNFACE ) );
		dc.FillSolidRect( rcBar.left + 1, rcBar.top, 1, rcBar.Height(), GetSysColor( COLOR_3DHIGHLIGHT ) );
		dc.FillSolidRect( rcBar.right - 1, rcBar.top, 1, rcBar.Height(), GetSysColor( COLOR_3DSHADOW ) );
		dc.FillSolidRect( rcBar.left + 2, rcBar.top, rcBar.Width() - 3, rcBar.Height(), GetSysColor( COLOR_BTNFACE ) );
		dc.ExcludeClipRect( &rcBar );
		
		rcBar.SetRect( rcBar.right, rcClient.top,
			rcClient.right, rcClient.top + HEADER_HEIGHT );
		
		if ( dc.RectVisible( &rcBar ) ) PaintListHeader( dc, rcBar );
	}
	
	if ( m_bStatusVisible )
	{
		CRect rcStatus( &m_rcStatus );
		if ( dc.RectVisible( &rcStatus ) ) PaintStatus( dc, rcStatus );
	}
	
	if ( m_pPlayer == NULL )
	{
		if ( dc.RectVisible( &m_rcVideo ) ) PaintSplash( dc, m_rcVideo );
	}
	
	dc.SelectObject( pOldFont );
}

void CMediaFrame::PaintSplash(CDC& dc, CRect& rcBar)
{
	if ( m_bmLogo.m_hObject == NULL )
	{
		dc.FillSolidRect( &m_rcVideo, 0 );
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
	
	dc.SetBkColor( 0 );
	dc.SetTextColor( RGB( 200, 200, 255 ) );
	dc.ExtTextOut( pt.x, pt.y, ETO_OPAQUE, &m_rcVideo, strText, NULL );
	dc.ExcludeClipRect( &rcText );
	
	dc.FillSolidRect( &m_rcVideo, 0 );
}

void CMediaFrame::PaintListHeader(CDC& dc, CRect& rcBar)
{
	CString strText;
	CPoint pt = rcBar.CenterPoint();
	LoadString( strText, IDS_MEDIA_PLAYLIST );
	CSize szText = dc.GetTextExtent( strText );
	pt.x -= szText.cx / 2; pt.y -= szText.cy / 2;
	dc.SetBkMode( OPAQUE );
	dc.SetBkColor( RGB( 0, 0, 0x80 ) );
	dc.SetTextColor( RGB( 0xFF, 0xFF, 0 ) );
	dc.ExtTextOut( pt.x, pt.y, ETO_OPAQUE|ETO_CLIPPED, &rcBar, strText, NULL );
}

void CMediaFrame::PaintStatus(CDC& dc, CRect& rcBar)
{
	COLORREF crBack = RGB( 0x00, 0x00, 0x60 );
	COLORREF crText = RGB( 0xF0, 0xF0, 0xFF );
	
	dc.SelectObject( &m_pFontValue );
	
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
		sz				= dc.GetTextExtent( pItem->m_sKey + ':' );
		rcPart.left		= rcBar.left + 20;
		rcPart.right	= rcPart.left + sz.cx + 8;
		dc.ExtTextOut( rcPart.left + 4, nY, ETO_CLIPPED|ETO_OPAQUE, &rcPart, pItem->m_sKey + ':', NULL );
		dc.ExcludeClipRect( &rcPart );
		
		dc.SelectObject( &m_pFontValue );
		sz				= dc.GetTextExtent( pItem->m_sValue );
		rcPart.left		= rcPart.right;
		rcPart.right	= rcPart.left + sz.cx + 8;
		dc.ExtTextOut( rcPart.left + 4, nY, ETO_CLIPPED|ETO_OPAQUE, &rcPart, pItem->m_sValue, NULL );
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
		dc.ExtTextOut( rcPart.left + 4, nY, ETO_CLIPPED|ETO_OPAQUE, &rcPart, str, NULL );
		dc.ExcludeClipRect( &rcPart );
	}

	if ( m_nState >= smsOpen )
	{
		str.Format( _T("%.2i:%.2i of %.2i:%.2i"),
			(int)( ( m_nPosition / ONE_SECOND ) / 60 ),
			(int)( ( m_nPosition / ONE_SECOND ) % 60 ),
			(int)( ( m_nLength / ONE_SECOND ) / 60 ),
			(int)( ( m_nLength / ONE_SECOND ) % 60 ) );
		
		sz				= dc.GetTextExtent( str );
		rcPart.right	= rcBar.right;
		rcPart.left		= rcPart.right - sz.cx - 8;
		
		dc.ExtTextOut( rcPart.left + 4, nY, ETO_CLIPPED|ETO_OPAQUE, &rcPart, str, NULL );
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
	
	if ( m_nState >= smsOpen )
	{
		str.Format( _T("%.2i:%.2i of %.2i:%.2i"),
			(int)( ( m_nPosition / ONE_SECOND ) / 60 ),
			(int)( ( m_nPosition / ONE_SECOND ) % 60 ),
			(int)( ( m_nLength / ONE_SECOND ) / 60 ),
			(int)( ( m_nLength / ONE_SECOND ) % 60 ) );
		
		sz				= dc.GetTextExtent( str );
		rcPart.right	= rcStatus.right;
		rcPart.left		= rcPart.right - sz.cx - 2;
		rcStatus.right	= rcPart.left;
		
		dc.DrawText( str, &rcPart, DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_RIGHT );
	}
	
	if ( CMetaItem* pItem = m_pMetadata.GetFirst() )
	{
		sz				= dc.GetTextExtent( pItem->m_sKey + ':' );
		rcPart.left		= rcStatus.left;
		rcPart.right	= rcPart.left + sz.cx + 2;
		rcStatus.left	= rcPart.right;

		dc.DrawText( pItem->m_sKey + ':', &rcPart, DT_SINGLELINE|DT_VCENTER|DT_LEFT|DT_NOPREFIX );
		dc.DrawText( pItem->m_sValue, &rcStatus, DT_SINGLELINE|DT_VCENTER|DT_LEFT|DT_NOPREFIX|DT_END_ELLIPSIS );
	}
	else
	{
		if ( m_nState >= smsOpen )
		{
			int nSlash = m_sFile.ReverseFind( '\\' );
			str = nSlash >= 0 ? m_sFile.Mid( nSlash + 1 ) : m_sFile;
		}
		
		dc.DrawText( str, &rcStatus, DT_SINGLELINE|DT_VCENTER|DT_LEFT|DT_NOPREFIX|DT_END_ELLIPSIS );
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMediaFrame interaction message handlers

void CMediaFrame::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	Skin.TrackPopupMenu( _T("CMediaFrame"), point,
		m_nState == smsPlaying ? ID_MEDIA_PAUSE : ID_MEDIA_PLAY );
}

void CMediaFrame::OnTimer(UINT nIDEvent) 
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
		
		rc.SetRect(	rcClient.right - m_nListSize - SPLIT_SIZE,
					rcClient.top,
					rcClient.right - m_nListSize,
					rcClient.bottom );
		
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

LONG CMediaFrame::OnMediaKey(WPARAM wParam, LPARAM lParam)
{
	if ( wParam != 1 ) return 0;
	
	switch ( lParam )
	{
	case 0xB0000:
		GetOwner()->PostMessage( WM_COMMAND, ID_MEDIA_NEXT );
		return 1;
	case 0xC0000:
		GetOwner()->PostMessage( WM_COMMAND, ID_MEDIA_PREVIOUS );
		return 1;
	case 0xD0000:
		GetOwner()->PostMessage( WM_COMMAND, ID_MEDIA_STOP );
		return 1;
	case 0xE0000:
		GetOwner()->PostMessage( WM_COMMAND, m_nState == smsPlaying ? ID_MEDIA_PAUSE : ID_MEDIA_PLAY );
		return 1;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CMediaFrame thumb bars

void CMediaFrame::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if ( pScrollBar == (CScrollBar*)&m_wndVolume )
	{
		double nVolume = (double)m_wndVolume.GetPos() / 100.0f;
		
		if ( nVolume != Settings.MediaPlayer.Volume )
		{
			Settings.MediaPlayer.Volume = nVolume;
			if ( m_pPlayer != NULL ) m_pPlayer->SetVolume( m_bMute ? 0 : Settings.MediaPlayer.Volume );
		}
	}
	
	if ( m_pPlayer == NULL ) return;
	
	MediaState nState = smsNull;
	if ( FAILED( m_pPlayer->GetState( &nState ) ) ) return;
	if ( nState < smsOpen ) return;
	
	if ( pScrollBar == (CScrollBar*)&m_wndPosition )
	{
		LONGLONG nLength = 0;
		if ( FAILED( m_pPlayer->GetLength( &nLength ) ) ) return;
		nLength /= TIME_FACTOR;
		
		LONGLONG nPosition = 0;
		if ( FAILED( m_pPlayer->GetPosition( &nPosition ) ) ) return;
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
			if ( TRUE )
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
			nPosition = (int)nPos;
			break;
		}
		
		if ( nState == smsOpen ) nPosition = 0;
		if ( nPosition < 0 ) nPosition = 0;
		if ( nPosition > nLength ) nPosition = nLength;
		
		if ( nState == smsPlaying )
		{
			m_pPlayer->Pause();
			m_bThumbPlay = TRUE;
		}
		
		m_pPlayer->SetPosition( nPosition * TIME_FACTOR );
		m_wndPosition.SetPos( (int)nPosition );
		
		if ( m_bThumbPlay && nSBCode == TB_ENDTRACK )
		{
			m_pPlayer->Play();
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

		m_pPlayer->GetSpeed( &nOldSpeed );
		if ( nNewSpeed != nOldSpeed ) m_pPlayer->SetSpeed( nNewSpeed );
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
	if ( m_nState < smsOpen )
	{
		PostMessage( WM_COMMAND, ID_MEDIA_OPEN );
	}
	else
	{
		if ( m_pPlayer != NULL ) m_pPlayer->Play();
		UpdateState();
	}
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
	if ( m_pPlayer ) m_pPlayer->Pause();
	UpdateState();
}

void CMediaFrame::OnUpdateMediaStop(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_nState > smsOpen );
}

void CMediaFrame::OnMediaStop() 
{
	// if ( m_pPlayer ) m_pPlayer->Stop();
	m_bAutoPlay = FALSE;
	m_wndList.Reset();
	m_bAutoPlay = TRUE;
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
	m_bMute = ! m_bMute;
	if ( m_pPlayer != NULL ) m_pPlayer->SetVolume( m_bMute ? 0 : Settings.MediaPlayer.Volume );
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
	m_bAutoPlay = FALSE;
	BOOL bResult = m_wndList.Enqueue( pszFile, TRUE );
	m_bAutoPlay = TRUE;
	return bResult;
}

BOOL CMediaFrame::IsPlaying()
{
	return m_pPlayer != NULL && m_nState == smsPlaying;
}

void CMediaFrame::OnFileDelete(LPCTSTR pszFile)
{
	if ( m_sFile.CompareNoCase( pszFile ) == 0 )
	{
		if ( m_pPlayer ) m_pPlayer->Close();
	}
}

float CMediaFrame::GetPosition()
{
	if ( m_pPlayer != NULL && m_nState >= smsOpen && m_nLength > 0 )
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
	if ( m_pPlayer != NULL && m_nState >= smsPaused && m_nLength > 0 )
	{
		m_nPosition = (LONGLONG)( nPosition * (float)m_nLength );
		m_pPlayer->SetPosition( m_nPosition );
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
	Settings.MediaPlayer.Volume = (double)nVolume;
	if ( m_pPlayer != NULL ) m_pPlayer->SetVolume( m_bMute ? 0 : Settings.MediaPlayer.Volume );
	OnTimer( 1 );
	return ( m_pPlayer != NULL );
}

/////////////////////////////////////////////////////////////////////////////
// CMediaFrame private media operations

BOOL CMediaFrame::Prepare()
{
	m_bThumbPlay = FALSE;

	if ( m_pPlayer != NULL ) return TRUE;
	if ( GetSafeHwnd() == NULL ) return FALSE;
	
	CWaitCursor pCursor;
	CLSID pCLSID;
	
	if ( Plugins.LookupCLSID( _T("MediaPlayer"), _T("Default"), pCLSID ) )
	{
		HINSTANCE hRes = AfxGetResourceHandle();
		CoCreateInstance( pCLSID, NULL, CLSCTX_INPROC_SERVER|CLSCTX_LOCAL_SERVER,
			IID_IMediaPlayer, (void**)&m_pPlayer );
		AfxSetResourceHandle( hRes );
	}
	
	if ( m_pPlayer == NULL )
	{
		pCursor.Restore();
		CString strMessage;
		Skin.LoadString( strMessage, IDS_MEDIA_PLUGIN_CREATE );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		return FALSE;
	}
	
	m_pPlayer->Create( GetSafeHwnd() );
	m_pPlayer->SetZoom( Settings.MediaPlayer.Zoom );
	m_pPlayer->SetAspect( Settings.MediaPlayer.Aspect );
	m_pPlayer->SetVolume( m_bMute ? 0 : Settings.MediaPlayer.Volume );
	
	if ( m_bmLogo.m_hObject ) m_pPlayer->SetLogoBitmap( (HBITMAP)m_bmLogo.m_hObject );
	
	if ( TRUE )
	{
		HINSTANCE hRes = AfxGetResourceHandle();
		PrepareVis();
		AfxSetResourceHandle( hRes );
	}
	
	OnSize( SIZE_INTERNAL, 0, 0 );
	UpdateState();
	
	return TRUE;
}

BOOL CMediaFrame::PrepareVis()
{
	if ( m_pPlayer == NULL ) return FALSE;
	
	IAudioVisPlugin* pPlugin = NULL;
	CLSID pCLSID;
	
	if ( GUIDX::Decode( Settings.MediaPlayer.VisCLSID, &pCLSID ) &&
		 Plugins.LookupEnable( pCLSID, TRUE ) )
	{
		HRESULT hr = CoCreateInstance( pCLSID, NULL,
			CLSCTX_INPROC_SERVER|CLSCTX_LOCAL_SERVER, IID_IAudioVisPlugin,
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
					BSTR bsPath = Settings.MediaPlayer.VisPath.AllocSysString();
					hr = pWrap->Load( bsPath, 0 );
					SysFreeString( bsPath );
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
	
	m_pPlayer->SetPluginSize( Settings.MediaPlayer.VisSize );
	m_pPlayer->SetPlugin( pPlugin );
	
	if ( pPlugin != NULL ) pPlugin->Release();
	
	return TRUE;
}

BOOL CMediaFrame::OpenFile(LPCTSTR pszFile)
{
	if ( ! Prepare() ) return FALSE;
	
	if ( m_sFile == pszFile )
	{
		m_pPlayer->Stop();
		UpdateState();
		return TRUE;
	}
	
	CWaitCursor pCursor;
	m_sFile.Empty();
	m_pMetadata.Clear();
	
	HINSTANCE hRes = AfxGetResourceHandle();
	
	BSTR bsFile = CString( pszFile ).AllocSysString();
	HRESULT hr = m_pPlayer->Open( bsFile );
	SysFreeString( bsFile );

	AfxSetResourceHandle( hRes );
	
	UpdateState();
	pCursor.Restore();
	m_tMetadata = GetTickCount();
	
	if ( FAILED(hr) )
	{
		LPCTSTR pszBase = _tcsrchr( pszFile, '\\' );
		pszBase = pszBase ? pszBase + 1 : pszFile;
		CString strMessage, strFormat;
		LoadString( strFormat, IDS_MEDIA_LOAD_FAIL );
		strMessage.Format( strFormat, pszBase );
		m_pMetadata.Add( _T("Error"), strMessage );
		LoadString( strMessage, IDS_MEDIA_LOAD_FAIL_HELP );
		m_pMetadata.Add( _T("Error"), strMessage );
		return FALSE;
	}
	
	m_sFile = pszFile;
	
	if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( m_sFile, TRUE ) )
	{
		m_pMetadata.Add( _T("Filename"), pFile->m_sName );
		m_pMetadata.Setup( pFile->m_pSchema, FALSE );
		m_pMetadata.Combine( pFile->m_pMetadata );
		m_pMetadata.Clean( 1024 );
		Library.Unlock();
		
		CMetaItem* pWidth	= m_pMetadata.Find( _T("Width") );
		CMetaItem* pHeight	= m_pMetadata.Find( _T("Height") );
		
		if ( pWidth != NULL && pHeight != NULL )
		{
			pWidth->m_sKey = _T("Dimensions");
			pWidth->m_sValue += 'x' + pHeight->m_sValue;
			m_pMetadata.Remove( _T("Height") );
		}
	}
	
	if ( hr != S_OK )
	{
		CString strMessage;
		LoadString( strMessage, IDS_MEDIA_PARTIAL_RENDER );
		m_pMetadata.Add( _T("Warning"), strMessage );
	}
	
	return TRUE;
}

void CMediaFrame::Cleanup()
{
	m_sFile.Empty();
	m_pMetadata.Clear();
	
	if ( m_pPlayer != NULL )
	{
		HINSTANCE hRes = AfxGetResourceHandle();
		m_pPlayer->Close();
		m_pPlayer->Destroy();
		m_pPlayer->Release();
		m_pPlayer = NULL;
		AfxSetResourceHandle( hRes );
	}
	
	UpdateState();
	Invalidate();
}

void CMediaFrame::ZoomTo(MediaZoom nZoom)
{
	if ( Settings.MediaPlayer.Zoom == nZoom ) return;
	Settings.MediaPlayer.Zoom = nZoom;
	if ( m_pPlayer == NULL ) return;
	m_pPlayer->SetZoom( Settings.MediaPlayer.Zoom );
}

void CMediaFrame::AspectTo(double nAspect)
{
	if ( Settings.MediaPlayer.Aspect == nAspect ) return;
	Settings.MediaPlayer.Aspect = nAspect;
	if ( m_pPlayer == NULL ) return;
	m_pPlayer->SetAspect( Settings.MediaPlayer.Aspect );
}

void CMediaFrame::UpdateState()
{
	m_nState = smsNull;
	
	if ( m_pPlayer ) m_pPlayer->GetState( &m_nState );
	
	if ( m_nState >= smsOpen )
	{
		m_nLength = 0;
		m_pPlayer->GetLength( &m_nLength );
		int nLength = (int)( m_nLength / TIME_FACTOR );
		
		m_nPosition = 0;
		m_pPlayer->GetPosition( &m_nPosition );
		int nPosition = (int)( m_nPosition / TIME_FACTOR );
		
		m_wndPosition.EnableWindow( TRUE );
		m_wndPosition.SetRangeMax( (int)nLength );
		m_wndPosition.SetPos( (int)nPosition );
		
		double nSpeed = 1.0f;
		m_pPlayer->GetSpeed( &nSpeed );
		m_wndSpeed.SetPos( (int)( nSpeed * 100 ) );
		m_wndSpeed.EnableWindow( TRUE );
		
		if ( ! m_bMute )
		{
			Settings.MediaPlayer.Volume = 1.0f;
			m_pPlayer->GetVolume( &Settings.MediaPlayer.Volume );
		}
		
		if ( m_nState == smsPlaying && nPosition >= nLength )
		{
			m_wndList.GetNext();
		}
	}
	else
	{
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
}

void CMediaFrame::OnNewCurrent(NMHDR* pNotify, LRESULT* pResult)
{
	int nCurrent = m_wndList.GetCurrent();
	m_wndList.UpdateWindow();

	if ( nCurrent >= 0 )
	{
		if ( OpenFile( m_wndList.GetPath( nCurrent ) ) )
		{
			if ( m_bAutoPlay )
			{
				m_pPlayer->Play();
				UpdateState();
			}
		}
		else if ( m_bAutoPlay )
		{
			m_bAutoPlay = FALSE;
			m_wndList.Reset();
			m_bAutoPlay = TRUE;
		}
	}
	else if ( m_wndList.GetItemCount() > 0 )
	{
		m_wndList.Reset( FALSE );
		nCurrent = m_wndList.GetNext( FALSE );

		m_bAutoPlay = m_bAutoPlay && Settings.MediaPlayer.Repeat;

		if ( nCurrent >= 0 )
			m_wndList.SetCurrent( nCurrent );
		else
			Cleanup();

		m_bAutoPlay = TRUE;
	}
	else
	{
		Cleanup();
	}

	*pResult = 0;
}
