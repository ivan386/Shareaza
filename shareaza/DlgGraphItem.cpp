//
// DlgGraphItem.cpp
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
#include "GraphItem.h"
#include "DlgGraphItem.h"
#include "Skin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CGraphItemDlg, CSkinDialog)
	//{{AFX_MSG_MAP(CGraphItemDlg)
	ON_CBN_SELCHANGE(IDC_GRAPH_SOURCE, OnSelChangeGraphSource)
	ON_BN_CLICKED(IDC_GRAPH_COLOUR, OnGraphColour)
	ON_WM_PAINT()
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CGraphItemDlg dialog

CGraphItemDlg::CGraphItemDlg(CWnd* pParent, CGraphItem* pItem) : CSkinDialog(CGraphItemDlg::IDD, pParent)
, m_nMultiplier(1.0f)
{
	//{{AFX_DATA_INIT(CGraphItemDlg)
	//}}AFX_DATA_INIT
	m_pItem = pItem;
}

void CGraphItemDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGraphItemDlg)
	DDX_Control(pDX, IDOK, m_wndOK);
	DDX_Control(pDX, IDC_GRAPH_UNITS, m_wndUnits);
	DDX_Control(pDX, IDC_GRAPH_SOURCE, m_wndSource);
	DDX_Control(pDX, IDC_GRAPH_REMOVE, m_wndRemove);
	DDX_Float(pDX, IDC_GRAPH_PARAM, m_nMultiplier);
	DDX_Control(pDX, IDC_GRAPH_COLOUR_BOX, m_wndColourBox);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CGraphItemDlg message handlers

BOOL CGraphItemDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CGraphItemDlg"), IDR_TRAFFICFRAME );

	m_gdiImageList.Create( 16, 16, ILC_COLOR32|ILC_MASK, 1, 1 );
	m_gdiImageList.Add( theApp.LoadIcon( IDR_TRAFFICFRAME ) );

	for ( int nItem = 1 ; CGraphItem::m_pItemDesc[ nItem ].m_nCode ; nItem++ )
	{
		const GRAPHITEM* pItem = &CGraphItem::m_pItemDesc[ nItem ];
		CString strItem;

		::Skin.LoadString( strItem, pItem->m_nStringID );
		int nIndex = m_wndSource.AddString( strItem );
		m_wndSource.SetItemData( nIndex, (LPARAM)pItem );

		if ( pItem->m_nCode == m_pItem->m_nCode ) m_wndSource.SetCurSel( nIndex );
	}

	m_crColour = m_pItem->m_nColour;
	m_nMultiplier = m_pItem->m_nMultiplier;

	OnSelChangeGraphSource();

	return TRUE;
}

void CGraphItemDlg::OnMeasureItem(int /*nIDCtl*/, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	lpMeasureItemStruct->itemWidth	= 1024;
	lpMeasureItemStruct->itemHeight	= 18;
}

void CGraphItemDlg::OnDrawItem(int /*nIDCtl*/, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if ( lpDrawItemStruct->itemID == (UINT)-1 ) return;
	if ( ( lpDrawItemStruct->itemAction & ODA_SELECT ) == 0 &&
		 ( lpDrawItemStruct->itemAction & ODA_DRAWENTIRE ) == 0 ) return;

	CRect rcItem( &lpDrawItemStruct->rcItem );
	CDC dc;

	dc.Attach( lpDrawItemStruct->hDC );

	dc.FillSolidRect( &rcItem,
		GetSysColor( ( lpDrawItemStruct->itemState & ODS_SELECTED )
		? COLOR_HIGHLIGHT : COLOR_WINDOW ) );

	dc.SetTextColor( GetSysColor( ( lpDrawItemStruct->itemState & ODS_SELECTED )
		? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT ) );

	dc.SetBkMode( TRANSPARENT );

	CPoint pt( rcItem.left + 1, rcItem.top + 1 );

	ImageList_Draw( m_gdiImageList.GetSafeHandle(),
		0, dc.GetSafeHdc(), pt.x, pt.y,
		( lpDrawItemStruct->itemState & ODS_SELECTED ) ? ILD_SELECTED : ILD_NORMAL );

	rcItem.left += 20; rcItem.right -= 2;

	CString strText;
	m_wndSource.GetLBText( lpDrawItemStruct->itemID, strText );

	CFont* pOldFont = (CFont*)dc.SelectObject( &theApp.m_gdiFont );
	dc.DrawText( strText, &rcItem, DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_NOPREFIX );
	dc.SelectObject( pOldFont );

	dc.Detach();
}

void CGraphItemDlg::OnSelChangeGraphSource()
{
	int nItem = m_wndSource.GetCurSel();
	if ( nItem < 0 ) return;

	GRAPHITEM* pItem = (GRAPHITEM*)m_wndSource.GetItemData( nItem );
	if ( ! pItem ) return;

	switch ( pItem->m_nUnits )
	{
	case 0:
		m_wndUnits.SetWindowText( _T("Items") );
		break;
	case 1:
		m_wndUnits.SetWindowText( Settings.General.RatesInBytes ? _T("Bytes per Second") : _T("Bits per Second") );
		break;
	case 2:
		m_wndUnits.SetWindowText( _T("Volume (B/KB/MB/GB/TB") );
		break;
	case 3:
		m_wndUnits.SetWindowText( _T("Percentage (%)") );
		break;
	default:
		m_wndUnits.SetWindowText( _T("") );
		break;
	}

	UpdateData( FALSE );

	m_wndOK.EnableWindow( TRUE );
}

void CGraphItemDlg::OnGraphColour()
{
	CColorDialog dlg( m_crColour, CC_ANYCOLOR|CC_SOLIDCOLOR, this );

	if ( dlg.DoModal() == IDOK )
	{
		m_crColour = dlg.GetColor();
		Invalidate();
	}
}

void CGraphItemDlg::OnPaint()
{
	CPaintDC dc( this );
	CRect rc;

	m_wndColourBox.GetWindowRect( &rc );
	ScreenToClient( &rc );

	dc.Draw3dRect( &rc, 0, 0 );
	rc.DeflateRect( 1, 1 );
	dc.FillSolidRect( &rc, 0 );
	dc.Draw3dRect( rc.left, ( rc.top + rc.bottom ) / 2 - 1, rc.Width(), 2, m_crColour, m_crColour );
}

void CGraphItemDlg::OnOK()
{
	int nItem = m_wndSource.GetCurSel();
	if ( nItem < 0 ) return;

	GRAPHITEM* pItem = (GRAPHITEM*)m_wndSource.GetItemData( nItem );
	if ( ! pItem ) return;

	UpdateData( TRUE );
	m_pItem->SetCode( pItem->m_nCode );
	m_pItem->m_nColour = m_crColour;
	m_pItem->m_nMultiplier = m_nMultiplier;

	CSkinDialog::OnOK();
}

/////////////////////////////////////////////////////////////////////////////
// CGraphItemDlg custom dialog data exchange

void PASCAL DDX_Float(CDataExchange* pDX, int nIDC, float& nValue)
{
	HWND hWndCtrl = pDX->PrepareCtrl( nIDC );
	_ASSERTE( hWndCtrl != NULL );

	CWnd* pWnd = CWnd::FromHandle( hWndCtrl );	
	// data from control

	if ( pDX->m_bSaveAndValidate )
	{
		CString str;
		pWnd->GetWindowText( str );
		nValue = 1.0f;
		if ( str.IsEmpty() )
			return;

		float nNumber;
		if ( _stscanf( str, L"%f", &nNumber ) == 1 )
			nValue = nNumber;
	}
	else //data to control
	{
		CString str;
		if ( nValue <= 0 )
			nValue = 1.0;

		str.Format( L"%f", nValue );
		pWnd->SetWindowText( (LPCTSTR)str );
	}
}
