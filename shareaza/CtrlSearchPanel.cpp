//
// CtrlSearchPanel.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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
#include "Schema.h"
#include "SchemaCache.h"
#include "ManagedSearch.h"
#include "QuerySearch.h"
#include "CtrlSearchPanel.h"
#include "CoolInterface.h"
#include "ShellIcons.h"
#include "XML.h"
#include "Skin.h"
#include "WndSearch.h"

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
	ON_COMMAND(IDC_SEARCH_PREFIX, OnSearchPrefix)
	ON_COMMAND(IDC_SEARCH_PREFIX_SHA1, OnSearchPrefixSHA1)
	ON_COMMAND(IDC_SEARCH_PREFIX_TIGER, OnSearchPrefixTiger)
	ON_COMMAND(IDC_SEARCH_PREFIX_SHA1_TIGER, OnSearchPrefixSHA1Tiger)
	ON_COMMAND(IDC_SEARCH_PREFIX_ED2K, OnSearchPrefixED2K)
	ON_COMMAND(IDC_SEARCH_PREFIX_BTH, OnSearchPrefixBTH)
	ON_COMMAND(IDC_SEARCH_PREFIX_MD5, OnSearchPrefixMD5)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CSearchAdvancedBox, CTaskBox)
BEGIN_MESSAGE_MAP(CSearchAdvancedBox, CTaskBox)
	//{{AFX_MSG_MAP(CSearchAdvancedBox)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_SEARCH_GNUTELLA2, OnG2Clicked)
	ON_BN_CLICKED(IDC_SEARCH_GNUTELLA1, OnG1Clicked)
	ON_BN_CLICKED(IDC_SEARCH_EDONKEY, OnED2KClicked)
	ON_BN_CLICKED(IDC_SEARCH_DC, OnDCClicked)
	ON_MESSAGE(WM_CTLCOLORSTATIC, OnCtlColorStatic)
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

#define BOX_MARGIN	6

/////////////////////////////////////////////////////////////////////////////
// CSearchPanel construction

CSearchPanel::CSearchPanel()
{
	m_bSendSearch	= FALSE;
	m_bAdvanced		= FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CSearchPanel message handlers

BOOL CSearchPanel::Create(CWnd* pParentWnd)
{
	CRect rect( 0, 0, 0, 0 );
	return CTaskPanel::Create( _T("CSearchPanel"), WS_VISIBLE, rect, pParentWnd, 0 );
}

int CSearchPanel::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if ( CTaskPanel::OnCreate( lpCreateStruct ) == -1 )return -1;
	
	m_bAdvanced = ( Settings.General.GUIMode != GUI_BASIC ) &&  ( Settings.Search.AdvancedPanel );
	
	m_boxSearch.Create( this, 136, _T("Search"), IDR_SEARCHFRAME );
	m_boxAdvanced.Create( this, 110, _T("Advanced"), IDR_SEARCHFRAME );
	m_boxSchema.Create( this, 0, _T("Schema"), IDR_SEARCHFRAME );
	m_boxResults.Create( this, 80, _T("Results"), IDR_HOSTCACHEFRAME );
	
	// Basic search box
	AddBox( &m_boxSearch );

	// Advanced search options
	if ( m_bAdvanced ) 
	{
		AddBox( &m_boxAdvanced );
		// If the resolution is low, minimise the advanced box by default
		if ( GetSystemMetrics( SM_CYSCREEN ) < 1024 ) m_boxAdvanced.Expand( FALSE );
	}

	// Metadata
	AddBox( &m_boxSchema );

	// Results summary
	if ( m_bAdvanced ) AddBox( &m_boxResults );
	
	// The metadata box varies in height to fill available space
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
	LoadString( strCaption, IDS_SEARCH_PANEL_ADVANCED );
	m_boxAdvanced.SetCaption( strCaption );
	
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

void CSearchPanel::ShowSearch(const CManagedSearch* pManaged)
{
	if ( ! pManaged )
	{
		OnSchemaChange();
		return;
	}

	CQuerySearchPtr pSearch = pManaged->GetSearch();
	if ( ! pSearch )
	{
		OnSchemaChange();
		return;
	}

	CString strURN;

	// The search is based on the priority from the lowest to highest

	if ( pSearch->m_oMD5 )
	{
		strURN = pSearch->m_oMD5.toUrn();
	}
	if ( pSearch->m_oBTH )
	{
		strURN = pSearch->m_oBTH.toUrn();
	}
	if ( pSearch->m_oTiger )
	{
		strURN = pSearch->m_oTiger.toUrn();
	}
	if ( pSearch->m_oED2K )
	{
		strURN = pSearch->m_oED2K.toUrn();
	}
	if ( pSearch->m_oSHA1 )
	{
		strURN = pSearch->m_oSHA1.toUrn();
	}

	if ( ! strURN.IsEmpty() )
	{
		m_boxSearch.m_wndSearch.SetWindowText(
			pSearch->m_sSearch.IsEmpty() ? strURN :
			( strURN + _T(" ") + pSearch->m_sSearch ) );
	} else
		m_boxSearch.m_wndSearch.SetWindowText( pSearch->m_sSearch );

	m_boxSearch.m_wndSchemas.Select( pSearch->m_pSchema );

	if ( m_bAdvanced )
	{
		m_boxAdvanced.m_wndCheckBoxG2.SetCheck( pManaged->m_bAllowG2 ? BST_CHECKED : BST_UNCHECKED);
		m_boxAdvanced.m_wndCheckBoxG1.SetCheck( pManaged->m_bAllowG1 ? BST_CHECKED : BST_UNCHECKED );
		m_boxAdvanced.m_wndCheckBoxED2K.SetCheck( pManaged->m_bAllowED2K ? BST_CHECKED : BST_UNCHECKED );
		m_boxAdvanced.m_wndCheckBoxDC.SetCheck( pManaged->m_bAllowDC ? BST_CHECKED : BST_UNCHECKED );

		CString strSize;
		if ( pSearch->m_nMinSize > 0 && pSearch->m_nMinSize < SIZE_UNKNOWN )
			strSize = Settings.SmartVolume(pSearch->m_nMinSize, Bytes, true );
		else
			strSize.Empty();
		if ( m_boxAdvanced.m_wndSizeMin.m_hWnd != NULL ) m_boxAdvanced.m_wndSizeMin.SetWindowText( strSize );


		if ( pSearch->m_nMaxSize > 0 && pSearch->m_nMaxSize < SIZE_UNKNOWN )
			strSize = Settings.SmartVolume( pSearch->m_nMaxSize, Bytes, true );
		else
			strSize.Empty();
		if ( m_boxAdvanced.m_wndSizeMax.m_hWnd != NULL ) m_boxAdvanced.m_wndSizeMax.SetWindowText( strSize );
	}
	
	OnSchemaChange();
	
	if ( pSearch->m_pXML != NULL )
	{
		m_boxSchema.m_wndSchema.UpdateData( pSearch->m_pXML->GetFirstElement(), FALSE );
	}
}

void CSearchPanel::ShowStatus(BOOL bStarted, BOOL bSearching, DWORD nFiles, DWORD nHits, DWORD nHubs, DWORD nLeaves)
{
	CString strCaption;

	if ( bStarted )
	{
		if ( bSearching )
		{
			LoadString( strCaption, IDS_SEARCH_PANEL_SEARCHING );
			m_boxSearch.m_wndStart.EnableWindow( FALSE );
			m_boxSearch.m_wndPrefix.EnableWindow( FALSE );
		}
		else
		{
			LoadString( strCaption, IDS_SEARCH_PANEL_MORE );
			m_boxSearch.m_wndStart.EnableWindow( TRUE );
			m_boxSearch.m_wndPrefix.EnableWindow( TRUE );
		}
	}
	else
	{
		LoadString( strCaption, IDS_SEARCH_PANEL_START ); 
		m_boxSearch.m_wndStart.EnableWindow( TRUE );
		m_boxSearch.m_wndPrefix.EnableWindow( TRUE );
	}
	m_boxSearch.m_wndStart.SetText( strCaption );
	
	LoadString( strCaption, bStarted ? IDS_SEARCH_PANEL_STOP : IDS_SEARCH_PANEL_CLEAR );
	m_boxSearch.m_wndStop.SetText( strCaption );
	
	m_boxResults.Update( bStarted, nFiles, nHits, nHubs, nLeaves );
}

void CSearchPanel::OnSchemaChange()
{
	CSchemaPtr pSchema = m_boxSearch.m_wndSchemas.GetSelected();
	
	m_boxSchema.m_wndSchema.SetSchema( pSchema, TRUE );
	m_boxSchema.SetSize( pSchema != NULL ? 1 : 0 );
	
	if ( pSchema != NULL )
	{
		HICON hIcon = ShellIcons.ExtractIcon( pSchema->m_nIcon16, 16 );
		m_boxSchema.SetIcon( hIcon );
		CString strTitle = pSchema->m_sTitle;
		int nPos = strTitle.Find( ':' );
		if ( nPos > 0 ) strTitle = strTitle.Mid( nPos + 1 );
		m_boxSchema.SetCaption( strTitle );
	}

	CBaseMatchWnd* pMainSearchFrame = static_cast< CBaseMatchWnd* >(GetParent());
	if ( pMainSearchFrame )
	{
		CList< CSchemaMember* > pColumns;

		if ( pSchema )
		{
			CString strMembers = pSchema->m_sDefaultColumns;
			for ( POSITION pos = pSchema->GetMemberIterator() ; pos ; )
			{
				CSchemaMember* pMember = pSchema->GetNextMember( pos );

				if ( strMembers.Find( _T("|") + pMember->m_sName + _T("|") ) >= 0 )
					pColumns.AddTail( pMember );
			}
		}

		pMainSearchFrame->m_wndList.SelectSchema( pSchema, &pColumns );
	}
}

CSearchPtr CSearchPanel::GetSearch()
{
	CSearchPtr pManaged( new CManagedSearch() );
	CQuerySearchPtr pSearch = pManaged->GetSearch();

	CString sSearch;
	m_boxSearch.m_wndSearch.GetWindowText( sSearch );

	pSearch->m_oSHA1.fromUrn( sSearch ) ||
		pSearch->m_oSHA1.fromString( sSearch );
	pSearch->m_oTiger.fromUrn( sSearch ) ||
		pSearch->m_oTiger.fromString( sSearch );
	pSearch->m_oED2K.fromUrn( sSearch ) ||
		pSearch->m_oED2K.fromString( sSearch );
	pSearch->m_oMD5.fromUrn( sSearch ) ||
		pSearch->m_oMD5.fromString( sSearch );
	pSearch->m_oBTH.fromUrn( sSearch ) ||
		pSearch->m_oBTH.fromString( sSearch ) ||
		pSearch->m_oBTH.fromUrn< Hashes::base16Encoding >( sSearch ) ||
		pSearch->m_oBTH.fromString< Hashes::base16Encoding >( sSearch );
	if ( pSearch->m_oSHA1  ||
		 pSearch->m_oTiger ||
		 pSearch->m_oED2K  ||
		 pSearch->m_oBTH   ||
		 pSearch->m_oMD5   )
	{
		// Hash search
	}
	else
	{
		// Keyword search
		pSearch->m_sSearch = sSearch;
	}
	if ( CSchemaPtr pSchema = m_boxSearch.m_wndSchemas.GetSelected() )
	{
		pSearch->m_pSchema	= pSchema;
		pSearch->m_pXML		= pSchema->Instantiate();

		m_boxSchema.m_wndSchema.UpdateData(
			pSearch->m_pXML->AddElement( pSchema->m_sSingular ), TRUE );

		Settings.Search.LastSchemaURI = pSchema->GetURI();
	}
	else
	{
		Settings.Search.LastSchemaURI.Empty();
	}
	if ( m_bAdvanced )
	{
		pManaged->m_bAllowG2		= m_boxAdvanced.m_wndCheckBoxG2.GetCheck();
		pManaged->m_bAllowG1		= m_boxAdvanced.m_wndCheckBoxG1.GetCheck();
		pManaged->m_bAllowED2K		= m_boxAdvanced.m_wndCheckBoxED2K.GetCheck();
		pManaged->m_bAllowDC		= m_boxAdvanced.m_wndCheckBoxDC.GetCheck();

		if ( ! pManaged->m_bAllowG2 &&
			 ! pManaged->m_bAllowG1 &&
			 ! pManaged->m_bAllowED2K &&
			 ! pManaged->m_bAllowDC )
		{
			m_boxAdvanced.m_wndCheckBoxG2.SetCheck( BST_CHECKED );
			m_boxAdvanced.m_wndCheckBoxG1.SetCheck( BST_CHECKED );
			m_boxAdvanced.m_wndCheckBoxED2K.SetCheck( BST_CHECKED );
			m_boxAdvanced.m_wndCheckBoxDC.SetCheck( BST_CHECKED );
			pManaged->m_bAllowG2	=	TRUE;
			pManaged->m_bAllowG1	=	TRUE;
			pManaged->m_bAllowED2K	=	TRUE;
			pManaged->m_bAllowDC	=	TRUE;
		}

		if ( m_boxAdvanced.m_wndSizeMin.m_hWnd != NULL )
		{
			CString strWindowValue;

			m_boxAdvanced.m_wndSizeMin.GetWindowText( strWindowValue );
			if ( strWindowValue.IsEmpty() || ( _tcsicmp( strWindowValue, _T("any") ) == 0 ) )
				pSearch->m_nMinSize = 0;
			else
			{
				if ( !_tcsstr( strWindowValue, _T("B") ) && !_tcsstr( strWindowValue, _T("b") ) )
					strWindowValue += _T("B");
				pSearch->m_nMinSize = Settings.ParseVolume( strWindowValue );
			}


			m_boxAdvanced.m_wndSizeMax.GetWindowText( strWindowValue );
			if ( strWindowValue.IsEmpty() || ( _tcsicmp( strWindowValue, _T("any") ) == 0 )  || ( _tcsicmp( strWindowValue, _T("max") ) == 0 ) )
				pSearch->m_nMaxSize = SIZE_UNKNOWN;
			else
			{
				if ( !_tcsstr( strWindowValue, _T("B") ) && !_tcsstr( strWindowValue, _T("b") ) )
					strWindowValue += _T("B");
				pSearch->m_nMaxSize = Settings.ParseVolume( strWindowValue );
			}

			// Check it wasn't invalid
			if ( pSearch->m_nMinSize >pSearch->m_nMaxSize )
				pSearch->m_nMaxSize = SIZE_UNKNOWN;
		}
	}

	pSearch->PrepareCheck();

	return pManaged;
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
	}
	
	return CTaskPanel::PreTranslateMessage( pMsg );
}

void CSearchPanel::Enable()
{
	m_boxSearch.m_wndSearch.EnableWindow( TRUE );
	m_boxSearch.m_wndSchemas.EnableWindow( TRUE );

	m_boxAdvanced.m_wndCheckBoxG1.EnableWindow( TRUE );
	m_boxAdvanced.m_wndCheckBoxG2.EnableWindow( TRUE );
	m_boxAdvanced.m_wndCheckBoxED2K.EnableWindow( TRUE );
	m_boxAdvanced.m_wndCheckBoxDC.EnableWindow( TRUE );
	m_boxAdvanced.m_wndSizeMin.EnableWindow( TRUE );
	m_boxAdvanced.m_wndSizeMax.EnableWindow( TRUE );

	m_boxSchema.m_wndSchema.Enable();
}

void CSearchPanel::Disable()
{
	m_boxSearch.m_wndSearch.EnableWindow( FALSE );
	m_boxSearch.m_wndSchemas.EnableWindow( FALSE );

	m_boxAdvanced.m_wndCheckBoxG2.EnableWindow( FALSE );
	m_boxAdvanced.m_wndCheckBoxG1.EnableWindow( FALSE );
	m_boxAdvanced.m_wndCheckBoxED2K.EnableWindow( FALSE );
	m_boxAdvanced.m_wndCheckBoxDC.EnableWindow( FALSE );
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
	
	if ( ! m_wndSearch.Create( ES_AUTOHSCROLL | WS_TABSTOP | WS_GROUP, rc,
		this, IDC_SEARCH, _T("Search"), _T("Search.%.2i") ) ) return -1;
	
	m_wndSearch.SetFont( &theApp.m_gdiFont );
	m_wndSearch.ModifyStyleEx( 0, WS_EX_CLIENTEDGE );
	
	if ( ! m_wndSchemas.Create( WS_TABSTOP, rc, this, IDC_SCHEMAS ) ) return -1;
	
	m_wndSchemas.SetDroppedWidth( 200 );
	LoadString( m_wndSchemas.m_sNoSchemaText, IDS_SEARCH_PANEL_AFT );
	m_wndSchemas.Load( Settings.Search.LastSchemaURI );

	m_wndStart.Create( rc, this, IDC_SEARCH_START, WS_TABSTOP | BS_DEFPUSHBUTTON );
	m_wndStart.SetHandCursor( TRUE );

	m_wndStop.Create( rc, this, IDC_SEARCH_STOP, WS_TABSTOP );
	m_wndStop.SetHandCursor( TRUE );

	m_wndPrefix.Create( rc, this, IDC_SEARCH_PREFIX );
	m_wndPrefix.SetHandCursor( TRUE );

	OnSkinChange();

	SetPrimary( TRUE );
	
	return 0;
}

void CSearchInputBox::OnSkinChange()
{
	CString strCaption;
	CSearchWnd* pwndSearch = static_cast< CSearchWnd* >( GetParent()->GetParent() );
	BOOL bStarted = ! pwndSearch->IsPaused();
	BOOL bSearching = ! pwndSearch->IsWaitMore();

	LoadString( strCaption, bStarted ?
		( bSearching? IDS_SEARCH_PANEL_SEARCHING : IDS_SEARCH_PANEL_MORE ) :
		IDS_SEARCH_PANEL_START );
	m_wndStart.SetWindowText( strCaption );
	m_wndStart.SetCoolIcon( ID_SEARCH_SEARCH, FALSE );

	LoadString( strCaption, bStarted ? IDS_SEARCH_PANEL_STOP : IDS_SEARCH_PANEL_CLEAR );
	m_wndStop.SetWindowText( strCaption );
	m_wndStop.SetCoolIcon( ID_SEARCH_STOP, FALSE );

	m_wndPrefix.SetIcon( IDI_HASH );
}

void CSearchInputBox::OnSize(UINT nType, int cx, int cy) 
{
	CTaskBox::OnSize( nType, cx, cy );
	
	HDWP hDWP = BeginDeferWindowPos( 4 );

	DeferWindowPos( hDWP, m_wndSearch, NULL, BOX_MARGIN, 27,
		cx - BOX_MARGIN * 2, 19,
		SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
	DeferWindowPos( hDWP, m_wndSchemas, NULL, BOX_MARGIN, 67,
		cx - BOX_MARGIN * 2, 256,
		SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
	int width = ( cx - BOX_MARGIN * 3 ) / 2;
	DeferWindowPos( hDWP, m_wndStart, NULL, BOX_MARGIN, 102, width, 24,
		SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
	DeferWindowPos( hDWP, m_wndStop, NULL, BOX_MARGIN * 2 + width, 102, width, 24,
		SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
	DeferWindowPos( hDWP, m_wndPrefix, NULL, cx - BOX_MARGIN - 8, 13, 8, 8,
		SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
	
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
		CSize size = rc.Size();
		pDC = CoolInterface.GetBuffer( dc, size );
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
	rct.SetRect( BOX_MARGIN + 1, BOX_MARGIN, rc.right - BOX_MARGIN - 8, BOX_MARGIN + 16 );
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

void CSearchInputBox::OnSearchPrefix()
{
	if ( m_wndSearch.IsWindowEnabled() )
	{
		CMenu mnuPopup;
		mnuPopup.CreatePopupMenu();
		mnuPopup.AppendMenu( MF_STRING, IDC_SEARCH_PREFIX_SHA1, _T("SHA1") );
		mnuPopup.AppendMenu( MF_STRING, IDC_SEARCH_PREFIX_TIGER, _T("Tiger") );
		mnuPopup.AppendMenu( MF_STRING, IDC_SEARCH_PREFIX_SHA1_TIGER, _T("SHA1 + Tiger") );
		mnuPopup.AppendMenu( MF_STRING, IDC_SEARCH_PREFIX_ED2K, _T("ED2K") );
		mnuPopup.AppendMenu( MF_STRING, IDC_SEARCH_PREFIX_BTH, _T("BitTorrent") );
		mnuPopup.AppendMenu( MF_STRING, IDC_SEARCH_PREFIX_MD5, _T("MD5") );
		CPoint pt;
		::GetCursorPos( &pt );
		mnuPopup.TrackPopupMenu( TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, this, NULL );
	}
}

void CSearchInputBox::OnSearchPrefixSHA1()
{
	CString sSearch;
	m_wndSearch.GetWindowText( sSearch );
	Hashes::Sha1Hash oSHA1;
	if ( oSHA1.fromUrn( sSearch ) || oSHA1.fromString( sSearch ) )
		sSearch = oSHA1.toUrn();
	else
		sSearch = _T("urn:sha1:[SHA1]");

	m_wndSearch.SetWindowText( sSearch );
	m_wndSearch.SetFocus();
	m_wndSearch.SetSel( 9, -1 );
}

void CSearchInputBox::OnSearchPrefixTiger()
{
	CString sSearch;
	m_wndSearch.GetWindowText( sSearch );
	Hashes::TigerHash oTiger;
	if ( oTiger.fromUrn( sSearch ) || oTiger.fromString( sSearch ) )
		sSearch = oTiger.toUrn();
	else
		sSearch = _T("urn:tree:tiger/:[Tiger]");

	m_wndSearch.SetWindowText( sSearch );
	m_wndSearch.SetFocus();
	m_wndSearch.SetSel( 16, -1 );
}

void CSearchInputBox::OnSearchPrefixSHA1Tiger()
{
	CString sSearch;
	m_wndSearch.GetWindowText( sSearch );
	Hashes::Sha1Hash oSHA1;
	Hashes::TigerHash oTiger;	
	oSHA1.fromUrn( sSearch ) || oSHA1.fromString( sSearch );
	oTiger.fromUrn( sSearch ) || oTiger.fromString( sSearch );	
    sSearch = _T("urn:bitprint:");
	sSearch += oSHA1 ? oSHA1.toString() : _T("[SHA1]");
	sSearch += _T(".");
	sSearch += oTiger ? oTiger.toString() : _T("[Tiger]");

	m_wndSearch.SetWindowText( sSearch );
	m_wndSearch.SetFocus();
	m_wndSearch.SetSel( 13, -1 );
}

void CSearchInputBox::OnSearchPrefixED2K()
{
	CString sSearch;
	m_wndSearch.GetWindowText( sSearch );
	Hashes::Ed2kHash oEd2k;
	if ( oEd2k.fromUrn( sSearch ) || oEd2k.fromString( sSearch ) )
		sSearch = oEd2k.toUrn();
	else
		sSearch = _T("urn:ed2khash:[ED2K]");

	m_wndSearch.SetWindowText( sSearch );
	m_wndSearch.SetFocus();
	m_wndSearch.SetSel( 13, -1 );
}

void CSearchInputBox::OnSearchPrefixBTH()
{
	CString sSearch;
	m_wndSearch.GetWindowText( sSearch );
	Hashes::BtHash oBTH;
	if ( oBTH.fromUrn( sSearch ) ||
		 oBTH.fromString( sSearch ) ||
		 oBTH.fromUrn< Hashes::base16Encoding >( sSearch ) ||
		 oBTH.fromString< Hashes::base16Encoding >( sSearch ) )
		sSearch = oBTH.toUrn();
	else
		sSearch = _T("urn:btih:[BTIH]");

	m_wndSearch.SetWindowText( sSearch );
	m_wndSearch.SetFocus();
	m_wndSearch.SetSel( 9, -1 );
}

void CSearchInputBox::OnSearchPrefixMD5()
{
	CString sSearch;
	m_wndSearch.GetWindowText( sSearch );
	Hashes::Md5Hash oMD5;
	if ( oMD5.fromUrn( sSearch ) || oMD5.fromString( sSearch ) )
		sSearch = oMD5.toUrn();
	else
		sSearch = _T("urn:md5:[MD5]");

	m_wndSearch.SetWindowText( sSearch );
	m_wndSearch.SetFocus();
	m_wndSearch.SetSel( 8, -1 );
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

	if ( ! m_wndCheckBoxG2.Create( L"G2", WS_CHILD | WS_VISIBLE | WS_TABSTOP |
		BS_CHECKBOX, rc, this, IDC_SEARCH_GNUTELLA2 ) ) return -1;
	if ( ! m_wndCheckBoxG1.Create( L"G1", WS_CHILD | WS_VISIBLE | WS_TABSTOP |
		BS_CHECKBOX, rc, this, IDC_SEARCH_GNUTELLA1 ) ) return -1;
	if ( ! m_wndCheckBoxED2K.Create( L"eD2K", WS_CHILD | WS_VISIBLE | WS_TABSTOP |
		BS_CHECKBOX, rc, this, IDC_SEARCH_EDONKEY ) ) return -1;
	if ( ! m_wndCheckBoxDC.Create( L"DC++", WS_CHILD | WS_VISIBLE | WS_TABSTOP |
		BS_CHECKBOX, rc, this, IDC_SEARCH_DC ) ) return -1;
	
	m_wndCheckBoxG2.SetFont( &theApp.m_gdiFontBold );
	m_wndCheckBoxG2.SetCheck( BST_CHECKED );
	m_wndCheckBoxG1.SetFont( &theApp.m_gdiFontBold );
	m_wndCheckBoxG1.SetCheck( BST_CHECKED );
	m_wndCheckBoxED2K.SetFont( &theApp.m_gdiFontBold );
	m_wndCheckBoxED2K.SetCheck( BST_CHECKED );
	m_wndCheckBoxDC.SetFont( &theApp.m_gdiFontBold );
	m_wndCheckBoxDC.SetCheck( BST_CHECKED );

	CoolInterface.LoadProtocolIconsTo( m_gdiProtocols );

	// Min combo
	if ( ! m_wndSizeMin.Create( WS_CHILD|WS_VISIBLE|WS_TABSTOP|CBS_AUTOHSCROLL|
		CBS_DROPDOWN, rc, this, IDC_SEARCH_SIZEMIN ) ) return -1;
	m_wndSizeMin.SetFont( &theApp.m_gdiFont );

	m_wndSizeMin.AddString( _T("") );
	m_wndSizeMin.AddString( _T("500 KB") );
	m_wndSizeMin.AddString( _T("1 MB") );
	m_wndSizeMin.AddString( _T("10 MB") );
	m_wndSizeMin.AddString( _T("50 MB") );
	m_wndSizeMin.AddString( _T("100 MB") );
	m_wndSizeMin.AddString( _T("200 MB") );
	m_wndSizeMin.AddString( _T("500 MB") );
	m_wndSizeMin.AddString( _T("1 GB") );
	m_wndSizeMin.AddString( _T("4 GB") );

	// Max combo
	if ( ! m_wndSizeMax.Create( WS_CHILD|WS_VISIBLE|WS_TABSTOP|CBS_AUTOHSCROLL|
		CBS_DROPDOWN, rc, this, IDC_SEARCH_SIZEMAX ) ) return -1;
	m_wndSizeMax.SetFont( &theApp.m_gdiFont );

	m_wndSizeMax.AddString( _T("") );
	m_wndSizeMax.AddString( _T("500 KB") );
	m_wndSizeMax.AddString( _T("1 MB") );
	m_wndSizeMax.AddString( _T("10 MB") );
	m_wndSizeMax.AddString( _T("50 MB") );
	m_wndSizeMax.AddString( _T("100 MB") );
	m_wndSizeMax.AddString( _T("200 MB") );
	m_wndSizeMax.AddString( _T("500 MB") );
	m_wndSizeMax.AddString( _T("1 GB") );
	m_wndSizeMax.AddString( _T("4 GB") );

	if ( ! m_wndSizeMinMax.Create( _T(""), WS_CHILD|WS_VISIBLE|SS_CENTER,
		rc, this ) ) return -1;
	m_wndSizeMinMax.SetFont( &theApp.m_gdiFont );

	return 0;
}

void CSearchAdvancedBox::OnSkinChange()
{
	CoolInterface.LoadProtocolIconsTo( m_gdiProtocols );

	CString strControlTitle;
	LoadString( strControlTitle, IDS_SEARCH_PANEL_INPUT_5 );
	m_wndSizeMinMax.SetWindowText( strControlTitle );
}

void CSearchAdvancedBox::OnSize(UINT nType, int cx, int cy) 
{
	CTaskBox::OnSize( nType, cx, cy );
	
	HDWP hDWP = BeginDeferWindowPos( 3 );

	if ( m_wndCheckBoxG2.m_hWnd != NULL )
		DeferWindowPos( hDWP, m_wndCheckBoxG2, NULL, BOX_MARGIN + 21, 28, 
			( cx - BOX_MARGIN * 3 ) / 2 - 20, 14,
			SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
	if ( m_wndCheckBoxG1.m_hWnd != NULL )
		DeferWindowPos( hDWP, m_wndCheckBoxG1, NULL, BOX_MARGIN + 21, 48, 
			( cx - BOX_MARGIN * 3 ) / 2 - 20, 14,
			SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
	if ( m_wndCheckBoxED2K.m_hWnd != NULL )
		DeferWindowPos( hDWP, m_wndCheckBoxED2K, NULL, ( cx / 2 ) + BOX_MARGIN / 2 + 26, 28, 
			( cx - BOX_MARGIN * 3 ) / 2 - 20, 14,
			SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
	if ( m_wndCheckBoxDC.m_hWnd != NULL )
		DeferWindowPos( hDWP, m_wndCheckBoxDC, NULL, ( cx / 2 ) + BOX_MARGIN / 2 + 26, 48, 
			( cx - BOX_MARGIN * 3 ) / 2 - 20, 14,
			SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
	if ( m_wndSizeMin.m_hWnd != NULL )
	{
		int width = ( cx - BOX_MARGIN * 6 ) / 2;
		DeferWindowPos( hDWP, m_wndSizeMin, NULL, BOX_MARGIN, 81,
			width, 219, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
		DeferWindowPos( hDWP, m_wndSizeMax, NULL, cx - BOX_MARGIN - width, 81,
			width, 219, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
		DeferWindowPos( hDWP, m_wndSizeMinMax, NULL, BOX_MARGIN + width, 81 + 2,
			BOX_MARGIN * 4, 18, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
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
		CSize size = rc.Size();
		pDC = CoolInterface.GetBuffer( dc, size );
		CoolInterface.DrawWatermark( pDC, &rc, &m_bmWatermark );
		pDC->SetBkMode( TRANSPARENT );
	}
	else
	{
		// Paints the background behind controls except checkboxes (see OnCtlColorStatic)
		pDC->SetBkMode( OPAQUE );
		pDC->SetBkColor( CoolInterface.m_crTaskBoxClient );
		nFlags |= ETO_OPAQUE;
	}
	
	CFont* pOldFont = (CFont*)pDC->SelectObject( &CoolInterface.m_fntNormal );
	
	pDC->SetTextColor( 0 );

	// Text of "Search on this Network" check boxes
	LoadString( strControlTitle, IDS_SEARCH_PANEL_INPUT_3 );
	rct.SetRect( BOX_MARGIN + 1, BOX_MARGIN, rc.right - BOX_MARGIN, BOX_MARGIN + 16 );
	pDC->ExtTextOut( rct.left, rct.top, nFlags, &rct, strControlTitle, NULL );
	pDC->ExcludeClipRect( &rct );

	// Text of "File size must be" above drop down box of MinFileSize and MaxFileSize
	LoadString( strControlTitle, IDS_SEARCH_PANEL_INPUT_4 );
	rct.OffsetRect( 0, 64 - rct.top );
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
		// Fills the background of the advanced box
		pDC->FillSolidRect( &rc, CoolInterface.m_crTaskBoxClient );
	}

	m_gdiProtocols.Draw( pDC, PROTOCOL_G2, CPoint( BOX_MARGIN, 26 ), ILD_NORMAL );
	m_gdiProtocols.Draw( pDC, PROTOCOL_G1, CPoint( BOX_MARGIN, 46 ), ILD_NORMAL );
	m_gdiProtocols.Draw( pDC, PROTOCOL_ED2K, CPoint( PANEL_WIDTH / 2 - 3, 26 ), ILD_NORMAL );
	m_gdiProtocols.Draw( pDC, PROTOCOL_DC, CPoint( PANEL_WIDTH / 2 - 3, 46 ), ILD_NORMAL );
}

LRESULT CSearchAdvancedBox::OnCtlColorStatic(WPARAM wParam, LPARAM /*lParam*/)
{
	HBRUSH hbr = NULL;
	HDC hDCStatic = (HDC)wParam;

	SetBkMode( hDCStatic, TRANSPARENT );

	if ( m_crBack != CoolInterface.m_crTaskBoxClient )
	{
		if ( m_brBack.m_hObject ) m_brBack.DeleteObject();
		m_brBack.CreateSolidBrush( m_crBack = CoolInterface.m_crTaskBoxClient );
	}
	hbr = m_brBack;

	return (LRESULT)hbr;
}

// CSearchAdvancedBox Check Boxes
void CSearchAdvancedBox::OnG2Clicked()
{
	CButton* pBox = &m_wndCheckBoxG2;
	pBox->SetCheck( pBox->GetCheck() == BST_CHECKED ? BST_UNCHECKED : BST_CHECKED );
}

void CSearchAdvancedBox::OnG1Clicked()
{
	CButton* pBox = &m_wndCheckBoxG1;
	pBox->SetCheck( pBox->GetCheck() == BST_CHECKED ? BST_UNCHECKED : BST_CHECKED );
}

void CSearchAdvancedBox::OnED2KClicked()
{
	CButton* pBox = &m_wndCheckBoxED2K;
	pBox->SetCheck( pBox->GetCheck() == BST_CHECKED ? BST_UNCHECKED : BST_CHECKED );
}

void CSearchAdvancedBox::OnDCClicked()
{
	CButton* pBox = &m_wndCheckBoxDC;
	pBox->SetCheck( pBox->GetCheck() == BST_CHECKED ? BST_UNCHECKED : BST_CHECKED );
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
	if ( ! m_wndSchema.Create( WS_VISIBLE, rc, this, 0 ) ) return -1;

	m_wndSchema.m_nCaptionWidth	= 0;
	m_wndSchema.m_nItemHeight	= 42;
	m_wndSchema.m_bShowBorder	= FALSE;
	
	return 0;
}

void CSearchSchemaBox::OnSize(UINT nType, int cx, int cy) 
{
	CTaskBox::OnSize( nType, cx, cy );
	m_wndSchema.SetWindowPos( NULL, 0, 1, cx, cy - 1, SWP_NOZORDER | SWP_NOACTIVATE );
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
		CSize size = rc.Size();
		pDC = CoolInterface.GetBuffer( dc, size );
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
	CSize cz = pDC->GetTextExtent( pszText, static_cast< int >( _tcslen( pszText ) ) );
	CRect rc( nX, nY, nX + cz.cx, nY + cz.cy );
	
	pDC->ExtTextOut( nX, nY, nFlags, &rc, pszText, static_cast< UINT >( _tcslen( pszText ) ), NULL );
	pDC->ExcludeClipRect( nX, nY, nX + cz.cx, nY + cz.cy );
}

void CSearchResultsBox::OnExpanded(BOOL bOpen)
{
	theApp.WriteProfileInt( _T("Settings"), _T("SearchPanelResults"), bOpen );
}

