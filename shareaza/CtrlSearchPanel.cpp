//
// CtrlSearchPanel.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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
#include "Schema.h"
#include "SchemaCache.h"
#include "ManagedSearch.h"
#include "QuerySearch.h"
#include "CtrlSearchPanel.h"
#include "CoolInterface.h"
#include "ShellIcons.h"
#include "XML.h"
#include "Skin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CSearchPanel, CTaskPanel)
BEGIN_MESSAGE_MAP(CSearchPanel, CTaskPanel)
	//{{AFX_MSG_MAP(CSearchPanel)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CSearchInputBox, CTaskBox)
BEGIN_MESSAGE_MAP(CSearchInputBox, CTaskBox)
	//{{AFX_MSG_MAP(CSearchInputBox)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_CBN_SELCHANGE(IDC_SCHEMAS, OnSelChangeSchemas)
	ON_CBN_CLOSEUP(IDC_SCHEMAS, OnCloseUpSchemas)
	ON_COMMAND(IDC_SEARCH_START, OnSearchStart)
	ON_COMMAND(IDC_SEARCH_STOP, OnSearchStop)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CSearchAdvancedBox, CTaskBox)
BEGIN_MESSAGE_MAP(CSearchAdvancedBox, CTaskBox)
	//{{AFX_MSG_MAP(CSearchAdvancedBox)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CSearchSchemaBox, CTaskBox)
BEGIN_MESSAGE_MAP(CSearchSchemaBox, CTaskBox)
	//{{AFX_MSG_MAP(CSearchSchemaBox)
	ON_WM_CREATE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CSearchResultsBox, CTaskBox)
BEGIN_MESSAGE_MAP(CSearchResultsBox, CTaskBox)
	//{{AFX_MSG_MAP(CSearchResultsBox)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#define BOX_MARGIN	10


/////////////////////////////////////////////////////////////////////////////
// CSearchPanel construction

CSearchPanel::CSearchPanel()
{
	m_bSendSearch	= FALSE;
	m_bAdvanced		= FALSE;
}

CSearchPanel::~CSearchPanel()
{
}

/////////////////////////////////////////////////////////////////////////////
// CSearchPanel message handlers

BOOL CSearchPanel::Create(CWnd* pParentWnd)
{
	CRect rect;
	return CTaskPanel::Create( WS_VISIBLE, rect, pParentWnd, IDC_SEARCH_PANEL );
}

int CSearchPanel::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CTaskPanel::OnCreate( lpCreateStruct ) == -1 )return -1;
	
	m_bAdvanced = ( Settings.General.GUIMode != GUI_BASIC ) &&  ( Settings.Search.AdvancedPanel );
	
	m_boxSearch.Create( this, 136, _T("Search"), IDR_SEARCHFRAME );
	m_boxAdvanced.Create( this, 100, _T("Advanced"), IDR_SEARCHFRAME );
	m_boxSchema.Create( this, 0, _T("Schema"), IDR_SEARCHFRAME );
	m_boxResults.Create( this, 80, _T("Results"), IDR_HOSTCACHEFRAME );
	
	AddBox( &m_boxSearch );
	if ( m_bAdvanced ) 
	{
		AddBox( &m_boxAdvanced );
		m_boxAdvanced.Expand( FALSE );
	}
	AddBox( &m_boxSchema );
	if ( m_bAdvanced ) AddBox( &m_boxResults );
	
	SetStretchBox( &m_boxSchema );
	
	OnSkinChange();
	
	return 0;
}

void CSearchPanel::OnSkinChange()
{
	CString strCaption;
	
	LoadString( strCaption, IDS_SEARCH_PANEL_INPUT_CAPTION );
	m_boxSearch.SetCaption( strCaption );
	LoadString( strCaption, IDS_SEARCH_PANEL_RESULTS_CAPTION );
	m_boxResults.SetCaption( strCaption );
	
	SetWatermark( Skin.GetWatermark( _T("CSearchPanel") ) );
	SetFooter( Skin.GetWatermark( _T("CSearchPanel.Footer") ), TRUE );
	
	m_boxSearch.SetWatermark( Skin.GetWatermark( _T("CSearchInputBox") ) );
	m_boxSearch.SetCaptionmark( Skin.GetWatermark( _T("CSearchInputBox.Caption") ) );
	m_boxSearch.OnSkinChange();

	m_boxAdvanced.SetWatermark( Skin.GetWatermark( _T("CSearchAdvancedBox") ) );
	m_boxAdvanced.SetCaptionmark( Skin.GetWatermark( _T("CSearchAdvancedBox.Caption") ) );
	m_boxAdvanced.OnSkinChange();
	
	m_boxSchema.SetWatermark( Skin.GetWatermark( _T("CSearchSchemaBox") ) );
	m_boxSchema.SetCaptionmark( Skin.GetWatermark( _T("CSearchSchemaBox.Caption") ) );
	
	m_boxResults.SetWatermark( Skin.GetWatermark( _T("CSearchResultsBox") ) );
	m_boxResults.SetCaptionmark( Skin.GetWatermark( _T("CSearchResultsBox.Caption") ) );
	
	Invalidate();
}

void CSearchPanel::SetSearchFocus()
{
	m_boxSearch.m_wndSearch.SetFocus();
}

void CSearchPanel::ShowSearch(CManagedSearch* pSearch)
{
	if ( pSearch == NULL )
	{
		OnSchemaChange();
		return;
	}
	
	m_boxSearch.m_wndSearch.SetWindowText( pSearch->m_pSearch->m_sSearch );
	m_boxSearch.m_wndSchemas.Select( pSearch->m_pSearch->m_pSchema );

	if ( m_bAdvanced )
	{
		if ( pSearch->m_bAllowG2 && ! pSearch->m_bAllowG1 && ! pSearch->m_bAllowED2K )
			m_boxAdvanced.m_wndNetworks.SetNetwork( PROTOCOL_G2 );
		else if ( ! pSearch->m_bAllowG2 && pSearch->m_bAllowG1 && ! pSearch->m_bAllowED2K )
			m_boxAdvanced.m_wndNetworks.SetNetwork( PROTOCOL_G1 );
		else if ( ! pSearch->m_bAllowG2 && ! pSearch->m_bAllowG1 && pSearch->m_bAllowED2K )
			m_boxAdvanced.m_wndNetworks.SetNetwork( PROTOCOL_ED2K );
		else
			m_boxAdvanced.m_wndNetworks.SetNetwork( PROTOCOL_NULL );


		CString strSize;
		if ( pSearch->m_pSearch->m_nMinSize > 0 )
			strSize = Settings.SmartVolume( pSearch->m_pSearch->m_nMinSize, FALSE );
		else
			strSize.Empty();
		if ( m_boxAdvanced.m_wndSizeMin.m_hWnd != NULL ) m_boxAdvanced.m_wndSizeMin.SetWindowText( strSize );


		if ( pSearch->m_pSearch->m_nMaxSize < SIZE_UNKNOWN )
			strSize = Settings.SmartVolume( pSearch->m_pSearch->m_nMaxSize, FALSE );
		else
			strSize.Empty();
		if ( m_boxAdvanced.m_wndSizeMax.m_hWnd != NULL ) m_boxAdvanced.m_wndSizeMax.SetWindowText( strSize );
	}
	
	OnSchemaChange();
	
	if ( pSearch->m_pSearch->m_pXML != NULL )
	{
		m_boxSchema.m_wndSchema.UpdateData( pSearch->m_pSearch->m_pXML->GetFirstElement(), FALSE );
	}
}

void CSearchPanel::ShowStatus(BOOL bStarted, BOOL bSearching, DWORD nFiles, DWORD nHits, DWORD nHubs, DWORD nLeaves)
{
	CString strCaption;

	if ( bStarted )
	{
		//LoadString( strCaption,  bSearching? IDS_SEARCH_PANEL_SEARCHING : IDS_SEARCH_PANEL_MORE );
		if ( bSearching )
		{
			LoadString( strCaption, IDS_SEARCH_PANEL_SEARCHING );
			m_boxSearch.m_wndStart.EnableWindow( FALSE );
		}
		else
		{
			LoadString( strCaption, IDS_SEARCH_PANEL_MORE );
			m_boxSearch.m_wndStart.EnableWindow( TRUE );
		}
	}
	else
	{
		LoadString( strCaption, IDS_SEARCH_PANEL_START ); 
		m_boxSearch.m_wndStart.EnableWindow( TRUE );
	}
	m_boxSearch.m_wndStart.SetText( strCaption );
	
	LoadString( strCaption, bStarted ? IDS_SEARCH_PANEL_STOP : IDS_SEARCH_PANEL_CLEAR );
	m_boxSearch.m_wndStop.SetText( strCaption );
	
	m_boxResults.Update( bStarted, nFiles, nHits, nHubs, nLeaves );
}

void CSearchPanel::OnSchemaChange()
{
	CSchema* pSchema = m_boxSearch.m_wndSchemas.GetSelected();
	
	m_boxSchema.m_wndSchema.SetSchema( pSchema, TRUE );
	m_boxSchema.SetSize( pSchema != NULL ? 1 : 0 );
	
	if ( pSchema != NULL )
	{
		m_boxSchema.SetIcon( ShellIcons.ExtractIcon( pSchema->m_nIcon16, 16 ), TRUE );
		CString strTitle = pSchema->m_sTitle;
		int nPos = strTitle.Find( ':' );
		if ( nPos > 0 ) strTitle = strTitle.Mid( nPos + 1 );
		m_boxSchema.SetCaption( strTitle );
	}
}

CManagedSearch* CSearchPanel::GetSearch()
{
	CManagedSearch* pSearch = new CManagedSearch();
	
	m_boxSearch.m_wndSearch.GetWindowText( pSearch->m_pSearch->m_sSearch );
	
	if ( CSchema* pSchema = m_boxSearch.m_wndSchemas.GetSelected() )
	{
		pSearch->m_pSearch->m_pSchema	= pSchema;
		pSearch->m_pSearch->m_pXML		= pSchema->Instantiate();
		
		m_boxSchema.m_wndSchema.UpdateData(
			pSearch->m_pSearch->m_pXML->AddElement( pSchema->m_sSingular ), TRUE );
		
		Settings.Search.LastSchemaURI = pSchema->m_sURI;
	}
	else
	{
		Settings.Search.LastSchemaURI.Empty();
	}
	if ( m_bAdvanced )
	{
		if ( m_boxAdvanced.m_wndNetworks.m_hWnd != NULL )
		{
			switch ( m_boxAdvanced.m_wndNetworks.GetNetwork() )
			{
			case PROTOCOL_NULL:
				pSearch->m_bAllowG2		= TRUE;
				pSearch->m_bAllowG1		= TRUE;
				pSearch->m_bAllowED2K	= TRUE;
				break;
			case PROTOCOL_G2:
				pSearch->m_bAllowG2		= TRUE;
				pSearch->m_bAllowG1		= FALSE;
				pSearch->m_bAllowED2K	= FALSE;
				break;
			case PROTOCOL_G1:
				pSearch->m_bAllowG2		= FALSE;
				pSearch->m_bAllowG1		= TRUE;
				pSearch->m_bAllowED2K	= FALSE;
				break;
			case PROTOCOL_ED2K:
				pSearch->m_bAllowG2		= FALSE;
				pSearch->m_bAllowG1		= FALSE;
				pSearch->m_bAllowED2K	= TRUE;
				break;
			}
		}

		if ( m_boxAdvanced.m_wndSizeMin.m_hWnd != NULL )
		{
			CString strWindowValue;

			m_boxAdvanced.m_wndSizeMin.GetWindowText( strWindowValue );
			if ( strWindowValue.IsEmpty() || ( _tcsicmp( strWindowValue, _T("any") ) == 0 ) )
				pSearch->m_pSearch->m_nMinSize = 0;
			else
				pSearch->m_pSearch->m_nMinSize = Settings.ParseVolume( strWindowValue, FALSE );


			m_boxAdvanced.m_wndSizeMax.GetWindowText( strWindowValue );
			if ( strWindowValue.IsEmpty() || ( _tcsicmp( strWindowValue, _T("any") ) == 0 )  || ( _tcsicmp( strWindowValue, _T("max") ) == 0 ) )
				pSearch->m_pSearch->m_nMaxSize = SIZE_UNKNOWN;
			else
				pSearch->m_pSearch->m_nMaxSize = Settings.ParseVolume( strWindowValue, FALSE );

			// Check it wasn't invalid
			if ( pSearch->m_pSearch->m_nMinSize > pSearch->m_pSearch->m_nMaxSize )
				pSearch->m_pSearch->m_nMaxSize = SIZE_UNKNOWN;
		}
	}
	
	pSearch->m_pSearch->BuildWordList();
	
	if ( ! pSearch->m_pSearch->CheckValid() )
	{
		delete pSearch;
		return NULL;
	}
	
	return pSearch;
}

void CSearchPanel::ExecuteSearch()
{
	m_bSendSearch = TRUE;
	GetParent()->SendMessage( WM_COMMAND, ID_SEARCH_SEARCH );
	m_bSendSearch = FALSE;
}

BOOL CSearchPanel::PreTranslateMessage(MSG* pMsg) 
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( pMsg->wParam == VK_RETURN )
		{
			ExecuteSearch();
			return TRUE;
		}
		else if ( pMsg->wParam == VK_TAB )
		{
			BOOL bShift = GetAsyncKeyState( VK_SHIFT ) & 0x8000;
			CWnd* pWnd = GetFocus();
			
			if ( pWnd == &m_boxSearch.m_wndSearch )
			{
				if ( bShift )
					m_boxSchema.m_wndSchema.SetFocus();
				else
					m_boxSearch.m_wndSchemas.SetFocus();
				return TRUE;
			}
			else if ( pWnd == &m_boxSearch.m_wndSchemas )
			{
				if ( bShift )
					m_boxSearch.m_wndSearch.SetFocus();
				else
					m_boxSchema.m_wndSchema.SetFocus();
				return TRUE;
			}
			else
			{
				m_boxSearch.m_wndSearch.SetFocus();
			}
		}
	}
	
	return CTaskPanel::PreTranslateMessage( pMsg );
}

void CSearchPanel::Enable()
{
	m_boxSearch.m_wndSearch.EnableWindow( TRUE );
	m_boxSearch.m_wndSchemas.EnableWindow( TRUE );

	m_boxAdvanced.m_wndNetworks.EnableWindow( TRUE );
	m_boxAdvanced.m_wndSizeMin.EnableWindow( TRUE );
	m_boxAdvanced.m_wndSizeMax.EnableWindow( TRUE );

	m_boxSchema.m_wndSchema.Enable();
}

void CSearchPanel::Disable()
{
	m_boxSearch.m_wndSearch.EnableWindow( FALSE );
	m_boxSearch.m_wndSchemas.EnableWindow( FALSE );

	m_boxAdvanced.m_wndNetworks.EnableWindow( FALSE );
	m_boxAdvanced.m_wndSizeMin.EnableWindow( FALSE );
	m_boxAdvanced.m_wndSizeMax.EnableWindow( FALSE );

	m_boxSchema.m_wndSchema.Disable();
}


/////////////////////////////////////////////////////////////////////////////
// CSearchInputBox construction

CSearchInputBox::CSearchInputBox()
{
}

CSearchInputBox::~CSearchInputBox()
{
}

/////////////////////////////////////////////////////////////////////////////
// CSearchInputBox message handlers

int CSearchInputBox::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CTaskBox::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	CRect rc( 0, 0, 0, 0 );
	CString strCaption;
	
	if ( ! m_wndSearch.Create( ES_AUTOHSCROLL|WS_TABSTOP|WS_GROUP, rc,
		this, IDC_SEARCH ) ) return -1;
	
	m_wndSearch.SetFont( &theApp.m_gdiFont );
	m_wndSearch.ModifyStyleEx( 0, WS_EX_CLIENTEDGE );
	
	if ( ! m_wndSchemas.Create( WS_TABSTOP, rc, this, IDC_SCHEMAS ) ) return -1;
	
	LoadString( m_wndSchemas.m_sNoSchemaText, IDS_SEARCH_PANEL_AFT );
	m_wndSchemas.Load( Settings.Search.LastSchemaURI );
	m_wndSchemas.SendMessage( CB_SETDROPPEDWIDTH, 200 );
	
	LoadString( strCaption, IDS_SEARCH_PANEL_START );
	m_wndStart.Create( rc, this, IDC_SEARCH_START );
	m_wndStart.SetWindowText( strCaption );
	m_wndStart.SetIcon( CoolInterface.ExtractIcon( ID_SEARCH_SEARCH ) );
	m_wndStart.SetHandCursor( TRUE );

	LoadString( strCaption, IDS_SEARCH_PANEL_STOP );
	m_wndStop.Create( rc, this, IDC_SEARCH_STOP );
	m_wndStop.SetWindowText( strCaption );
	m_wndStop.SetIcon( CoolInterface.ExtractIcon( ID_SEARCH_STOP ) );
	m_wndStop.SetHandCursor( TRUE );
	
	SetPrimary( TRUE );
	
	return 0;
}

void CSearchInputBox::OnSkinChange()
{
	CString strCaption;
	
	LoadString( strCaption, IDS_SEARCH_PANEL_START );
	m_wndStart.SetWindowText( strCaption );
	m_wndStart.SetIcon( CoolInterface.ExtractIcon( ID_SEARCH_SEARCH ) );

	LoadString( strCaption, IDS_SEARCH_PANEL_STOP );
	m_wndStop.SetWindowText( strCaption );
	m_wndStop.SetIcon( CoolInterface.ExtractIcon( ID_SEARCH_STOP ) );
}

void CSearchInputBox::OnSize(UINT nType, int cx, int cy) 
{
	CTaskBox::OnSize( nType, cx, cy );
	
	HDWP hDWP = BeginDeferWindowPos( 4 );

	DeferWindowPos( hDWP, m_wndSearch, NULL, BOX_MARGIN, 27, cx - BOX_MARGIN * 2, 19, SWP_SHOWWINDOW|SWP_NOZORDER );
	DeferWindowPos( hDWP, m_wndSchemas, NULL, BOX_MARGIN, 67, cx - BOX_MARGIN * 2, 256, SWP_SHOWWINDOW|SWP_NOZORDER );
	
	DeferWindowPos( hDWP, m_wndStart, NULL, BOX_MARGIN, 102, 94, 24, SWP_SHOWWINDOW|SWP_NOZORDER );
	DeferWindowPos( hDWP, m_wndStop, NULL, cx - BOX_MARGIN - 52, 102, 52, 24, SWP_SHOWWINDOW|SWP_NOZORDER );

	
	EndDeferWindowPos( hDWP );
}

void CSearchInputBox::OnPaint() 
{
	CPaintDC dc( this );
	CRect rc, rct;
	CString str;
	
	UINT nFlags = ETO_CLIPPED;
	CDC* pDC = &dc;
	
	GetClientRect( &rc );
	
	if ( m_bmWatermark.m_hObject != NULL )
	{
		pDC = CoolInterface.GetBuffer( dc, rc.Size() );
		CoolInterface.DrawWatermark( pDC, &rc, &m_bmWatermark );
		pDC->SetBkMode( TRANSPARENT );
	}
	else
	{
		pDC->SetBkMode( OPAQUE );
		pDC->SetBkColor( CoolInterface.m_crTaskBoxClient );
		nFlags |= ETO_OPAQUE;
	}
	
	CFont* pOldFont = (CFont*)pDC->SelectObject( &CoolInterface.m_fntNormal );
	
	pDC->SetTextColor( 0 );
	
	LoadString( str, IDS_SEARCH_PANEL_INPUT_1 );
	rct.SetRect( BOX_MARGIN + 1, BOX_MARGIN, rc.right - BOX_MARGIN, BOX_MARGIN + 16 );
	pDC->ExtTextOut( rct.left, rct.top, nFlags, &rct, str, NULL );
	pDC->ExcludeClipRect( &rct );

	LoadString( str, IDS_SEARCH_PANEL_INPUT_2 );
	rct.OffsetRect( 0, 50 - rct.top );
	pDC->ExtTextOut( rct.left, rct.top, nFlags, &rct, str, NULL );
	pDC->ExcludeClipRect( &rct );
	
	pDC->SelectObject( pOldFont );
	
	if ( pDC != &dc )
	{
		dc.BitBlt( 0, 0, rc.Width(), rc.Height(), pDC, 0, 0, SRCCOPY );
		pDC->SelectClipRgn( NULL );
	}
	else
	{
		pDC->FillSolidRect( &rc, CoolInterface.m_crTaskBoxClient );
	}
}

void CSearchInputBox::OnSelChangeSchemas()
{
	CSearchPanel* pPanel = (CSearchPanel*)GetPanel();
	pPanel->OnSchemaChange();
}

void CSearchInputBox::OnCloseUpSchemas()
{
}

void CSearchInputBox::OnSearchStart()
{
	CSearchPanel* pPanel = (CSearchPanel*)GetPanel();
	pPanel->ExecuteSearch();
}

void CSearchInputBox::OnSearchStop()
{
	CString strCaption, strTest;
	
	LoadString( strTest, IDS_SEARCH_PANEL_CLEAR );
	m_wndStop.GetWindowText( strCaption );
	
	CWnd* pTarget = GetPanel()->GetParent();
	
	if ( strCaption == strTest )
		pTarget->PostMessage( WM_COMMAND, ID_SEARCH_CLEAR );
	else
		pTarget->PostMessage( WM_COMMAND, ID_SEARCH_STOP );
}


/////////////////////////////////////////////////////////////////////////////
// CSearchAdvancedBox construction

CSearchAdvancedBox::CSearchAdvancedBox()
{
}

CSearchAdvancedBox::~CSearchAdvancedBox()
{
}

/////////////////////////////////////////////////////////////////////////////
// CSearchAdvancedBox message handlers

int CSearchAdvancedBox::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CTaskBox::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	CRect rc( 0, 0, 0, 0 );
	CString strCaption;

	if ( ! m_wndNetworks.Create( WS_TABSTOP, this, IDC_SEARCH_NETWORKS ) ) return -1;

	if ( ! m_wndSizeMin.Create( WS_TABSTOP, rc, this, IDC_SEARCH_SIZEMIN ) ) return -1;
	m_wndSizeMin.SetFont( &theApp.m_gdiFont );
	m_wndSizeMin.ModifyStyleEx( 0, WS_EX_CLIENTEDGE );

	if ( ! m_wndSizeMax.Create( WS_TABSTOP, rc, this, IDC_SEARCH_SIZEMAX ) ) return -1;
	m_wndSizeMax.SetFont( &theApp.m_gdiFont );
	m_wndSizeMax.ModifyStyleEx( 0, WS_EX_CLIENTEDGE );
	
	return 0;
}

void CSearchAdvancedBox::OnSkinChange()
{
	if ( m_wndNetworks.m_hWnd != NULL ) m_wndNetworks.OnSkinChange();
}

void CSearchAdvancedBox::OnSize(UINT nType, int cx, int cy) 
{
	CTaskBox::OnSize( nType, cx, cy );
	
	HDWP hDWP = BeginDeferWindowPos( 3 );

	if ( m_wndNetworks.m_hWnd != NULL )
		DeferWindowPos( hDWP, m_wndNetworks, NULL, BOX_MARGIN, 27, cx - BOX_MARGIN * 2, 256, SWP_SHOWWINDOW|SWP_NOZORDER );
		
	if ( m_wndSizeMin.m_hWnd != NULL )
	{
		DeferWindowPos( hDWP, m_wndSizeMin, NULL, BOX_MARGIN, 71, ( cx - BOX_MARGIN * 4 ) /2, 19, SWP_SHOWWINDOW|SWP_NOZORDER );
		DeferWindowPos( hDWP, m_wndSizeMax, NULL, ( cx / 2 ) + BOX_MARGIN, 71, ( cx - BOX_MARGIN * 4 ) /2, 19, SWP_SHOWWINDOW|SWP_NOZORDER );
	}
	
	EndDeferWindowPos( hDWP );
}

void CSearchAdvancedBox::OnPaint() 
{
	CPaintDC dc( this );
	CRect rc, rct;
	CString strControlTitle;
	
	UINT nFlags = ETO_CLIPPED;
	CDC* pDC = &dc;
	
	GetClientRect( &rc );
	
	if ( m_bmWatermark.m_hObject != NULL )
	{
		pDC = CoolInterface.GetBuffer( dc, rc.Size() );
		CoolInterface.DrawWatermark( pDC, &rc, &m_bmWatermark );
		pDC->SetBkMode( TRANSPARENT );
	}
	else
	{
		pDC->SetBkMode( OPAQUE );
		pDC->SetBkColor( CoolInterface.m_crTaskBoxClient );
		nFlags |= ETO_OPAQUE;
	}
	
	CFont* pOldFont = (CFont*)pDC->SelectObject( &CoolInterface.m_fntNormal );
	
	pDC->SetTextColor( 0 );
	
	LoadString( strControlTitle, IDS_SEARCH_PANEL_INPUT_3 );
	rct.SetRect( BOX_MARGIN + 1, BOX_MARGIN, rc.right - BOX_MARGIN, BOX_MARGIN + 16 );
	pDC->ExtTextOut( rct.left, rct.top, nFlags, &rct, strControlTitle, NULL );
	pDC->ExcludeClipRect( &rct );

	LoadString( strControlTitle, IDS_SEARCH_PANEL_INPUT_4 );
	rct.OffsetRect( 0, 54 - rct.top );
	pDC->ExtTextOut( rct.left, rct.top, nFlags, &rct, strControlTitle, NULL );
	pDC->ExcludeClipRect( &rct );

	LoadString( strControlTitle, IDS_SEARCH_PANEL_INPUT_5 );
	rct.OffsetRect( ( rc.Width() / 2 ) - ( BOX_MARGIN * 2 ) , 18 );
	pDC->ExtTextOut( rct.left, rct.top, nFlags, &rct, strControlTitle, NULL );
	pDC->ExcludeClipRect( &rct );
	
	pDC->SelectObject( pOldFont );
	
	if ( pDC != &dc )
	{
		dc.BitBlt( 0, 0, rc.Width(), rc.Height(), pDC, 0, 0, SRCCOPY );
		pDC->SelectClipRgn( NULL );
	}
	else
	{
		pDC->FillSolidRect( &rc, CoolInterface.m_crTaskBoxClient );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSearchSchemaBox construction

CSearchSchemaBox::CSearchSchemaBox()
{
}

CSearchSchemaBox::~CSearchSchemaBox()
{
}

/////////////////////////////////////////////////////////////////////////////
// CSearchSchemaBox message handlers

int CSearchSchemaBox::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CTaskBox::OnCreate( lpCreateStruct ) == -1 ) return -1;
	
	CRect rc;
	if ( ! m_wndSchema.Create( WS_VISIBLE, rc, this, IDC_SCHEMAS ) ) return -1;

	m_wndSchema.m_nCaptionWidth	= 0;
	m_wndSchema.m_nItemHeight	= 42;
	m_wndSchema.m_bShowBorder	= FALSE;
	
	return 0;
}

void CSearchSchemaBox::OnSize(UINT nType, int cx, int cy) 
{
	CTaskBox::OnSize( nType, cx, cy );
	m_wndSchema.SetWindowPos( NULL, 0, 1, cx, cy - 1, SWP_NOZORDER );
}

BOOL CSearchSchemaBox::PreTranslateMessage(MSG* pMsg) 
{
	if ( pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB )
	{
		if ( m_wndSchema.OnTab() ) return TRUE;
	}

	return CTaskBox::PreTranslateMessage( pMsg );
}

/////////////////////////////////////////////////////////////////////////////
// CSearchResultsBox

CSearchResultsBox::CSearchResultsBox()
{
	Expand( theApp.GetProfileInt( _T("Settings"), _T("SearchPanelResults"), TRUE ) );

	m_bActive	= FALSE;
	m_nFiles	= 0;
	m_nHits		= 0;
	m_nHubs		= 0;
	m_nLeaves	= 0;
}

CSearchResultsBox::~CSearchResultsBox()
{
}

/////////////////////////////////////////////////////////////////////////////
// CSearchResultsBox message handlers

void CSearchResultsBox::Update(BOOL bSearching, DWORD nFiles, DWORD nHits, DWORD nHubs, DWORD nLeaves)
{
	m_bActive	= bSearching;
	m_nFiles	= nFiles;
	m_nHits		= nHits;
	m_nHubs		= nHubs;
	m_nLeaves	= nLeaves;

	Invalidate();
}

void CSearchResultsBox::OnPaint() 
{
	CString strFormat, strText;
	CPaintDC dc( this );
	CRect rc;
	
	UINT nFlags = ETO_CLIPPED;
	CDC* pDC = &dc;
	
	GetClientRect( &rc );
	
	if ( m_bmWatermark.m_hObject )
	{
		pDC = CoolInterface.GetBuffer( dc, rc.Size() );
		CoolInterface.DrawWatermark( pDC, &rc, &m_bmWatermark );
		pDC->SetBkMode( TRANSPARENT );
	}
	else
	{
		pDC->SetBkMode( OPAQUE );
		pDC->SetBkColor( CoolInterface.m_crTaskBoxClient );
		nFlags |= ETO_OPAQUE;
	}

	CFont* pOldFont = (CFont*)pDC->SelectObject( &theApp.m_gdiFontBold );

	pDC->SetTextColor( 0 );

	LoadString( strText, IDS_SEARCH_PANEL_RESULTS_STATUS );
	DrawText( pDC, BOX_MARGIN, BOX_MARGIN, nFlags, strText );
	LoadString( strText, IDS_SEARCH_PANEL_RESULTS_FOUND );
	DrawText( pDC, BOX_MARGIN, BOX_MARGIN + 32, nFlags, strText );

	pDC->SelectObject( &theApp.m_gdiFont );

	if ( m_bActive )
	{
		LoadString( strFormat, IDS_SEARCH_PANEL_RESULTS_ACTIVE );
		strText.Format( strFormat, m_nHubs, m_nLeaves );
	}
	else
	{
		LoadString( strText, IDS_SEARCH_PANEL_RESULTS_INACTIVE );
	}

	DrawText( pDC, BOX_MARGIN + 8, BOX_MARGIN + 14, nFlags, strText );

	if ( m_nFiles )
	{
		LoadString( strFormat, IDS_SEARCH_PANEL_RESULTS_FORMAT );
		
		if ( strFormat.Find( '|' ) >= 0 )
		{
			if ( m_nFiles == 1 && m_nHits == 1 )
				Skin.SelectCaption( strFormat, 0 );
			else if ( m_nFiles == 1 )
				Skin.SelectCaption( strFormat, 1 );
			else
				Skin.SelectCaption( strFormat, 2 );
			
			strText.Format( strFormat,
				m_nFiles, m_nHits );
		}
		else
		{
			strText.Format( strFormat,
				m_nFiles, m_nFiles != 1 ? _T("s") : _T(""),
				m_nHits, m_nHits != 1 ? _T("s") : _T("") );
		}
	}
	else
	{
		LoadString( strText, IDS_SEARCH_PANEL_RESULTS_NONE );
	}

	DrawText( pDC, BOX_MARGIN + 8, BOX_MARGIN + 32 + 14, nFlags, strText );

	pDC->SelectObject( pOldFont );

	if ( pDC != &dc )
	{
		dc.BitBlt( 0, 0, rc.Width(), rc.Height(), pDC, 0, 0, SRCCOPY );
		pDC->SelectClipRgn( NULL );
	}
	else
	{
		pDC->FillSolidRect( &rc, CoolInterface.m_crTaskBoxClient );
	}
}

void CSearchResultsBox::DrawText(CDC* pDC, int nX, int nY, UINT nFlags, LPCTSTR pszText)
{
	CSize cz = pDC->GetTextExtent( pszText, _tcslen( pszText ) );
	CRect rc( nX, nY, nX + cz.cx, nY + cz.cy );
	
	pDC->ExtTextOut( nX, nY, nFlags, &rc, pszText, _tcslen( pszText ), NULL );
	pDC->ExcludeClipRect( nX, nY, nX + cz.cx, nY + cz.cy );
}

void CSearchResultsBox::OnExpanded(BOOL bOpen)
{
	theApp.WriteProfileInt( _T("Settings"), _T("SearchPanelResults"), bOpen );
}
