//
// CtrlFontCombo.cpp
//
// Copyright © Shareaza Development Team, 2002-2009.
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
#include "CtrlFontCombo.h"
#include "Settings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define SPACING			10
#define SYMBOL_WIDTH	15

/////////////////////////////////////////////////////////////////////////////
// CFontCombo construction

CFontCombo::CFontCombo()
{
	m_nFontHeight = 16;
	m_pImages.Create( IDB_FONT_SYMBOLS, SYMBOL_WIDTH, 2, RGB(255,255,255) );
}

CFontCombo::~CFontCombo()
{
}

IMPLEMENT_DYNAMIC(CFontCombo, CComboBox)

BEGIN_MESSAGE_MAP(CFontCombo, CComboBox)
	//{{AFX_MSG_MAP(CFontCombo)
	ON_WM_CREATE()
	ON_MESSAGE(OCM_DRAWITEM, OnOcmDrawItem)
	ON_CONTROL_REFLECT(CBN_DROPDOWN, OnDropdown)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFontCombo creation and initialization

BOOL CFontCombo::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	dwStyle |= WS_CHILD|WS_VSCROLL|CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS|CBS_SORT;

	return CComboBox::Create( dwStyle, rect, pParentWnd, nID );
}

int CFontCombo::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CComboBox::OnCreate(lpCreateStruct) == -1 ) return -1;
	Initialize();

	return 0;
}

void CFontCombo::PreSubclassWindow()
{
	CComboBox::PreSubclassWindow();
	ModifyStyle( 0, CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS|CBS_SORT );
	Initialize();
}

void CFontCombo::Initialize()
{
	CClientDC dc(this);

	ResetContent();
	DeleteAllFonts();
	EnumFontFamiliesEx( dc.m_hDC, NULL, (FONTENUMPROC)EnumFontProc, (LPARAM)this, 0 );

	SetCurSel( 0 );
}

BOOL CALLBACK CFontCombo::EnumFontProc(LPENUMLOGFONTEX lplf, NEWTEXTMETRICEX* lpntm,
									   DWORD dwFontType, LPVOID lpData)
{
	CFontCombo *pThis = reinterpret_cast<CFontCombo*>(lpData);

	if ( lpntm->ntmTm.tmCharSet != OEM_CHARSET && lpntm->ntmTm.tmCharSet != SYMBOL_CHARSET &&
		 dwFontType != DEVICE_FONTTYPE && _tcsicmp( lplf->elfLogFont.lfFaceName, _T("Small Fonts") ) != 0 )
	{
		int nFamily = lplf->elfLogFont.lfPitchAndFamily ? lplf->elfLogFont.lfPitchAndFamily >> 4 : 6;
		if ( nFamily < 4 ) // don't use unknown, decorative and script fonts
		{
			// Filter out vertical fonts starting with @
			if ( lplf->elfLogFont.lfFaceName[ 0 ] != '@' && pThis->AddFont( lplf->elfLogFont.lfFaceName ) )
			{
				int nIndex = pThis->AddString( lplf->elfLogFont.lfFaceName );
				if ( nIndex == -1 ) return FALSE;
				if ( pThis->SetItemData( nIndex, dwFontType ) == 0 )
					return FALSE;
			}
		}
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CFontCombo message handlers

void CFontCombo::OnDestroy()
{
	DeleteAllFonts();
	CComboBox::OnDestroy();
}

void CFontCombo::OnDropdown()
{
	int nNumEntries = GetCount();
	int nWidth = 0;
	CString str;

	CClientDC dc( this );
	int nSave = dc.SaveDC();
	dc.SelectObject( GetFont() );

	int nScrollWidth = GetSystemMetrics( SM_CXVSCROLL );
	for ( int nEntry = 0 ; nEntry < nNumEntries ; nEntry++ )
	{
		GetLBText( nEntry, str );
		int nLength = dc.GetTextExtent( str ).cx + nScrollWidth + SYMBOL_WIDTH;
		nWidth = max( nWidth, nLength );
	}

	nWidth += SYMBOL_WIDTH;
	// Add margin space to the calculations
	nWidth += dc.GetTextExtent( _T("0") ).cx;

	dc.RestoreDC( nSave );
	SetDroppedWidth( nWidth );
}

LRESULT CFontCombo::OnOcmDrawItem(WPARAM /*wParam*/, LPARAM lParam)
{
	DrawItem( (LPDRAWITEMSTRUCT)lParam );

	return 1;
}

void CFontCombo::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if ( lpDrawItemStruct->itemID == (UINT)-1 ) return;
	if ( ( lpDrawItemStruct->itemAction & ODA_SELECT ) == 0 &&
		 ( lpDrawItemStruct->itemAction & ODA_DRAWENTIRE ) == 0 ) return;

	if ( lpDrawItemStruct->CtlType != ODT_COMBOBOX ) return;

	CDC* pDC = CDC::FromHandle( lpDrawItemStruct->hDC );
	CRect rcItem( &lpDrawItemStruct->rcItem );
	CPoint pt( rcItem.left + 1, rcItem.top + 1 );

	int nOldDC = pDC->SaveDC();

	if ( Settings.General.LanguageRTL )
		SetLayout( pDC->m_hDC, LAYOUT_RTL );

	CString strCurrentFont;
	GetLBText( lpDrawItemStruct->itemID, strCurrentFont );

	CFont* pFont = (CFont*)pDC->SelectObject( strCurrentFont == m_sSelectedFont ?
		&theApp.m_gdiFontBold : &theApp.m_gdiFont );
	pDC->SetTextColor( GetSysColor( ( lpDrawItemStruct->itemState & ODS_SELECTED ) ?
		COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT ) );

	if ( IsWindowEnabled() )
	{
		if ( lpDrawItemStruct->itemState & ODS_SELECTED )
		{
			pDC->FillSolidRect( &rcItem, GetSysColor( COLOR_HIGHLIGHT ) );
		}
		else
			pDC->FillSolidRect( &rcItem, GetSysColor( COLOR_WINDOW ) );
	}
	else
		pDC->FillSolidRect( &rcItem, GetBkColor( lpDrawItemStruct->hDC ) );

	pDC->SetBkMode( TRANSPARENT );

	DWORD dwData = GetItemData( lpDrawItemStruct->itemID );
	if ( dwData & TRUETYPE_FONTTYPE )
		m_pImages.Draw( pDC, 0, CPoint( rcItem.left + 5, rcItem.top + 4 ),
		( lpDrawItemStruct->itemState & ODS_SELECTED ) ? ILD_SELECTED : ILD_NORMAL );
	else
		m_pImages.Draw( pDC, 1, CPoint( rcItem.left + 5, rcItem.top + 4 ),
		( lpDrawItemStruct->itemState & ODS_SELECTED ) ? ILD_SELECTED : ILD_NORMAL );

	rcItem.left += SYMBOL_WIDTH;
	int nOffsetX = SPACING;

	CFont* pFontValid;
	if ( m_pFonts.Lookup( strCurrentFont, (void*&)pFontValid ) == NULL ) return;

	CSize sz = pDC->GetTextExtent( strCurrentFont );
	int nPosY = ( rcItem.Height() - sz.cy ) / 2;
	pDC->TextOut( rcItem.left + nOffsetX, rcItem.top + nPosY, strCurrentFont );

	pDC->SelectObject( pFont );
	pDC->RestoreDC( nOldDC );
}

/////////////////////////////////////////////////////////////////////////////
// CFontCombo implementation

BOOL CFontCombo::AddFont(const CString& strFontName)
{
	CFont* pFont = NULL;
	// Sometimes font with the same name exists on the system; check it.
	if ( m_pFonts.Lookup( strFontName, (void*&)pFont ) == NULL )
	{
		pFont = new CFont;

		if ( pFont->CreateFont( m_nFontHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH,
			strFontName ) )
		{
			m_pFonts.SetAt( strFontName, pFont );
		}
	}
	else return FALSE;

	return TRUE;
}

void CFontCombo::SetFontHeight(int nNewHeight, BOOL bReinitialize)
{
	if ( nNewHeight == m_nFontHeight ) return;
	m_nFontHeight = nNewHeight;
	if ( bReinitialize ) Initialize();
}

int CFontCombo::GetFontHeight()
{
	return m_nFontHeight;
}

void CFontCombo::DeleteAllFonts()
{
	POSITION pos;
	CString str;
	for ( pos = m_pFonts.GetStartPosition() ; pos ; )
	{
		CFont* pFont = NULL;
		m_pFonts.GetNextAssoc( pos, str, (void*&)pFont );
		if ( pFont != NULL ) delete pFont;
	}
	m_pFonts.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
// CFontCombo custom dialog data exchange

void PASCAL DDX_FontCombo(CDataExchange* pDX, int nIDC, CString& strFontName)
{
	HWND hWndCtrl = pDX->PrepareCtrl( nIDC );
	_ASSERTE( hWndCtrl != NULL );

	CFontCombo* pCombo = static_cast<CFontCombo*>(CWnd::FromHandle( hWndCtrl ));
	// data from control

	if ( pDX->m_bSaveAndValidate )
	{
		int nIndex = pCombo->GetCurSel();
		if ( nIndex != CB_ERR )
		{
			pCombo->GetLBText( nIndex, strFontName );
			pCombo->m_sSelectedFont = strFontName;
		}
		else
			strFontName = theApp.m_sDefaultFont;
	}
	else //data to control
	{
		int nIndex = pCombo->FindString( -1, strFontName );
		if ( nIndex != CB_ERR )
		{
			pCombo->SetCurSel( nIndex );
			if ( pCombo->m_sSelectedFont.IsEmpty() )
			{
				pCombo->m_sSelectedFont = strFontName;
			}
		}
		else
		{
			nIndex = pCombo->FindString( -1, theApp.m_sDefaultFont );
			if ( nIndex != CB_ERR )
			{
				pCombo->SetCurSel( nIndex );
				if ( pCombo->m_sSelectedFont.IsEmpty() )
				{
					pCombo->m_sSelectedFont = theApp.m_sDefaultFont;
				}
			}
			else
				pCombo->SetCurSel( 0 );
		}
	}
}
