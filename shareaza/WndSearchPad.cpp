//
// WndSearchPad.cpp
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
#include "CoolInterface.h"
#include "QuerySearch.h"
#include "Network.h"
#include "Schema.h"
#include "Skin.h"
#include "XML.h"

#include "WndSearchPad.h"
#include "WndSearch.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CSearchPadWnd, CPanelWnd)

BEGIN_MESSAGE_MAP(CSearchPadWnd, CPanelWnd)
	//{{AFX_MSG_MAP(CSearchPadWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_CBN_SELCHANGE(IDC_SEARCH_SCHEMAS, OnSelChangeSchemas)
	ON_BN_CLICKED(IDC_SEARCH_CREATE, OnSearchCreate)
	ON_WM_MDIACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSearchPadWnd construction

CSearchPadWnd::CSearchPadWnd() : CPanelWnd( TRUE, FALSE )
{
	Create( IDR_SEARCHPADFRAME );
}

CSearchPadWnd::~CSearchPadWnd()
{
}

/////////////////////////////////////////////////////////////////////////////
// CSearchPadWnd system message handlers

int CSearchPadWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CPanelWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	CRect rc( 0, 0, 0, 0 );

	if ( ! m_wndText.Create( WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL|WS_TABSTOP|WS_GROUP, rc,
		this, IDC_SEARCH_TEXT ) ) return -1;

	m_wndText.SetFont( &theApp.m_gdiFont );
	m_wndText.ModifyStyleEx( 0, WS_EX_CLIENTEDGE );

	if ( ! m_wndSchema.Create( WS_VISIBLE|WS_TABSTOP, rc, this, IDC_SEARCH_SCHEMAS ) )
		return -1;
	
	m_wndSchema.SetDroppedWidth( 200 );
	LoadString( m_wndSchema.m_sNoSchemaText, IDS_SEARCH_PANEL_AFT );
	m_wndSchema.Load( Settings.Search.LastSchemaURI );

	if ( ! m_wndData.Create( WS_CHILD|WS_VISIBLE|WS_BORDER|WS_TABSTOP, rc, this,
		IDC_SEARCH_DATA ) ) return -1;

	CString strCaption;
	LoadString( strCaption, IDS_SEARCH_PANEL_START );
	m_wndSearch.Create( rc, this, IDC_SEARCH_CREATE );
	m_wndSearch.SetWindowText( strCaption );
	m_wndSearch.SetIcon( CoolInterface.ExtractIcon( ID_SEARCH_SEARCH ) );
	m_wndSearch.SetHandCursor( TRUE );

	m_pFont.CreateFont( -20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, _T("Tahoma") );

	LoadState();

	OnSelChangeSchemas();
	
	return 0;
}

void CSearchPadWnd::OnDestroy() 
{
	SaveState();
	CPanelWnd::OnDestroy();
}

void CSearchPadWnd::OnSkinChange()
{
	CString strCaption;
	LoadString( strCaption, IDS_SEARCH_PANEL_START );
	m_wndSearch.SetWindowText( strCaption );
	LoadString( m_wndSchema.m_sNoSchemaText, IDS_SEARCH_PANEL_AFT );

	CPanelWnd::OnSkinChange();
}

/////////////////////////////////////////////////////////////////////////////
// CSearchPadWnd display message handlers

void CSearchPadWnd::OnSize(UINT nType, int cx, int cy) 
{
	CPanelWnd::OnSize( nType, cx, cy );
	
	CRect rcClient( 0, 0, cx, cy );
	CRect rcItem;
	
	if ( rcClient.Width() > 640 )
		rcClient.DeflateRect( rcClient.Width() / 2 - 320, 0 );
	if ( rcClient.Height() > 480 )
		rcClient.DeflateRect( 0, rcClient.Height() / 2 - 240 );

	rcClient.DeflateRect( 72, 64 );
	rcClient.top += 8;
	
	rcItem.SetRect( rcClient.left, rcClient.top, rcClient.right - 104, rcClient.top + 19 );
	m_wndText.MoveWindow( &rcItem );

	rcItem.SetRect( rcClient.right - 92, rcClient.top - 2, rcClient.right, rcClient.top + 22 );
	m_wndSearch.MoveWindow( &rcItem );
	
	rcClient.top += 32;

	rcItem.SetRect( rcClient.right - 104 - 128, rcClient.top, rcClient.right - 104, rcClient.top + 256 );
	rcItem.left = max( rcItem.left, rcClient.left );
	m_wndSchema.MoveWindow( &rcItem );

	rcClient.top += 40;

	rcItem.CopyRect( &rcClient );
	m_wndData.MoveWindow( &rcItem );
}

void CSearchPadWnd::OnPaint() 
{
	CRect rcClient, rcItem;
	CPaintDC dc( this );
	CString str;
	
	GetClientRect( &rcClient );

	if ( rcClient.Width() > 640 )
		rcClient.DeflateRect( rcClient.Width() / 2 - 320, 0 );
	if ( rcClient.Height() > 480 )
		rcClient.DeflateRect( 0, rcClient.Height() / 2 - 240 );
	
	rcClient.DeflateRect( 16, 16 );

	// draw image

	rcClient.DeflateRect( 56, 0 );

	CFont* pOldFont = (CFont*)dc.SelectObject( &m_pFont );
	dc.SetBkMode( OPAQUE );
	dc.SetBkColor( CoolInterface.m_crWindow );
	dc.SetTextColor( Skin.m_crPanelBack );

	LoadString( str, IDS_SEARCH_PAD_HEADER );

	rcItem.SetRect( rcClient.left, rcClient.top, rcClient.right, rcClient.top + 26 );
	dc.ExtTextOut( rcItem.left + 2, rcItem.top + 2, ETO_CLIPPED|ETO_OPAQUE,
		&rcItem, str, NULL );
	dc.ExcludeClipRect( &rcItem );
	
	rcClient.DeflateRect( 0, 26 );
	
	LoadString( str, IDS_SEARCH_PAD_WORDS );

	rcItem.SetRect( rcClient.left, rcClient.top, rcClient.right, rcClient.top + 16 );
	dc.SetTextColor( 0 );
	dc.SelectObject( &CoolInterface.m_fntBold );
	dc.ExtTextOut( rcItem.left + 2, rcItem.top + 2, ETO_CLIPPED|ETO_OPAQUE,
		&rcItem, str, NULL );
	dc.ExcludeClipRect( &rcItem );

	rcClient.DeflateRect( 0, 22 );
	rcClient.top += 8;
	rcClient.top += 32;

	rcItem.SetRect( rcClient.left, rcClient.top,
		rcClient.right - 104 - 128 - 4, rcClient.top + 22 );
	
	if ( rcItem.right > rcItem.left )
	{
		LoadString( str, IDS_SEARCH_PAD_TYPE );
		CSize sz = dc.GetTextExtent( str );
		dc.ExtTextOut( rcItem.right - sz.cx, ( rcItem.top + rcItem.bottom ) / 2 - sz.cy / 2,
			ETO_CLIPPED|ETO_OPAQUE, &rcItem, str, NULL );
		dc.ExcludeClipRect( &rcItem );
	}

	dc.SelectObject( pOldFont );
	GetClientRect( &rcClient );
	dc.FillSolidRect( &rcClient, CoolInterface.m_crWindow );
}

/////////////////////////////////////////////////////////////////////////////
// CSearchPadWnd search interface

CQuerySearch* CSearchPadWnd::GetSearch()
{
	CQuerySearch* pSearch = new CQuerySearch();
	
	m_wndText.GetWindowText( pSearch->m_sSearch );
	pSearch->m_sSearch.TrimLeft();
	pSearch->m_sSearch.TrimRight();
	
	if ( CSchema* pSchema = m_wndSchema.GetSelected() )
	{
		pSearch->m_pSchema	= pSchema;
		pSearch->m_pXML		= pSchema->Instantiate();
		
		m_wndData.UpdateData(
			pSearch->m_pXML->AddElement( pSchema->m_sSingular ), TRUE );
		
		Settings.Search.LastSchemaURI = pSchema->m_sURI;
	}
	else
	{
		Settings.Search.LastSchemaURI.Empty();
	}
	
	if ( ! pSearch->CheckValid() )
	{
		delete pSearch;
		return NULL;
	}
	
	return pSearch;
}

void CSearchPadWnd::ClearSearch()
{
	m_wndText.SetWindowText( _T("") );

	CSchema* pSchema = m_wndSchema.GetSelected();
	m_wndData.SetSchema( pSchema, TRUE );
}

/////////////////////////////////////////////////////////////////////////////
// CSearchPadWnd control message handlers

void CSearchPadWnd::OnSelChangeSchemas()
{
	CSchema* pSchema = m_wndSchema.GetSelected();
	m_wndData.SetSchema( pSchema, TRUE );
	m_wndData.ShowWindow( pSchema != NULL ? SW_SHOW : SW_HIDE );
}

void CSearchPadWnd::OnSearchCreate()
{
	if ( ! Network.IsWellConnected() ) Network.Connect( TRUE );
	
	CQuerySearch* pSearch = GetSearch();
	if ( NULL == pSearch ) return;
	
	ClearSearch();
	
	new CSearchWnd( pSearch );
	
	PostMessage( WM_CLOSE );
}

void CSearchPadWnd::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd) 
{
	CPanelWnd::OnMDIActivate( bActivate, pActivateWnd, pDeactivateWnd );
	if ( bActivate ) m_wndText.SetFocus();
}

BOOL CSearchPadWnd::PreTranslateMessage(MSG* pMsg) 
{
	if ( pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN )
	{
		PostMessage( WM_COMMAND, IDC_SEARCH_CREATE );
		return TRUE;
	}
	else if ( pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB )
	{
		if ( m_wndData.OnTab() ) return TRUE;
	}
	
	return CPanelWnd::PreTranslateMessage( pMsg );
}
