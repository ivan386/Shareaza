//
// CtrlNetworkCombo.cpp
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
#include "CtrlNetworkCombo.h"

IMPLEMENT_DYNAMIC(CNetworkCombo, CComboBox)

BEGIN_MESSAGE_MAP(CNetworkCombo, CComboBox)
	ON_WM_CREATE()
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////////
// CNetworkCombo construction

CNetworkCombo::CNetworkCombo()
{
}

CNetworkCombo::~CNetworkCombo()
{
}

//////////////////////////////////////////////////////////////////////////////
// CNetworkCombo operations

BOOL CNetworkCombo::Create(DWORD dwStyle, CWnd* pParentWnd, UINT nID)
{
	CRect rect( 0, 0, 0, 0 );
	dwStyle |= WS_CHILD|WS_VSCROLL|CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS;
	return CComboBox::Create( dwStyle, rect, pParentWnd, nID );
}

int CNetworkCombo::GetNetwork()
{
	int nSel = GetCurSel();
	return nSel >= 0 ? GetItemData( nSel ) : PROTOCOL_NULL;
}

void CNetworkCombo::SetNetwork(int nProtocol)
{
	for ( int nItem = 0 ; nItem < GetCount() ; nItem ++ )
	{
		if ( GetItemData( nItem ) == nProtocol )
		{
			SetCurSel( nItem );
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
// CNetworkCombo message handlers

int CNetworkCombo::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CComboBox::OnCreate( lpCreateStruct ) == -1 ) return -1;

	CBitmap bmProtocols;
	bmProtocols.LoadBitmap( IDB_PROTOCOLS );
	if ( theApp.m_bRTL )
		bmProtocols.m_hObject = CreateMirroredBitmap( (HBITMAP)bmProtocols.m_hObject );

	if ( ! m_gdiImageList.Create( 16, 16, ILC_COLOR32|ILC_MASK, 6, 1 ) )
		m_gdiImageList.Create( 16, 16, ILC_COLOR16|ILC_MASK, 6, 1 );

	m_gdiImageList.Add( AfxGetApp()->LoadIcon( IDR_MAINFRAME ) );
	m_gdiImageList.Add( &bmProtocols, RGB( 0, 255, 0 ) );

	CString str;
	LoadString( str, IDS_SEARCH_ALL_NETWORKS );

	SetItemData( AddString( str ), PROTOCOL_NULL );
	SetItemData( AddString( _T("Gnutella2") ), PROTOCOL_G2 );
	SetItemData( AddString( _T("Gnutella1") ), PROTOCOL_G1 );
	SetItemData( AddString( _T("eDonkey2000") ), PROTOCOL_ED2K );
	SetCurSel( 0 );

	return 0;
}

void CNetworkCombo::OnSkinChange()
{
	CString str;
	LoadString( str, IDS_SEARCH_ALL_NETWORKS );
	BOOL bSel = GetCurSel() == 0;
	DeleteString( 0 );
	InsertString( 0, str );
	SetItemData( 0, PROTOCOL_NULL );
	if ( bSel ) SetCurSel( 0 );
}

void CNetworkCombo::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	lpMeasureItemStruct->itemWidth	= 1024;
	lpMeasureItemStruct->itemHeight	= 18;
}

void CNetworkCombo::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if ( lpDrawItemStruct->itemID == (UINT)-1 ) return;
	if ( ( lpDrawItemStruct->itemAction & ODA_SELECT ) == 0 &&
		 ( lpDrawItemStruct->itemAction & ODA_DRAWENTIRE ) == 0 ) return;

	CRect rcItem( &lpDrawItemStruct->rcItem );
	CPoint pt( rcItem.left + 1, rcItem.top + 1 );
	CDC dc;

	dc.Attach( lpDrawItemStruct->hDC );
	if ( theApp.m_bRTL ) SetLayout( dc.m_hDC, LAYOUT_RTL );

	CFont* pOldFont = (CFont*)dc.SelectObject( lpDrawItemStruct->itemData == 0 ?
		&theApp.m_gdiFontBold : &theApp.m_gdiFont );

	dc.SetTextColor( GetSysColor( ( lpDrawItemStruct->itemState & ODS_SELECTED )
		? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT ) );

	/*dc.FillSolidRect( &rcItem, GetSysColor( ( lpDrawItemStruct->itemState & ODS_SELECTED )
			? COLOR_HIGHLIGHT : COLOR_WINDOW ) );*/
	if ( IsWindowEnabled() )
	{
		if ( lpDrawItemStruct->itemState & ODS_SELECTED )
			dc.FillSolidRect( &rcItem, GetSysColor( COLOR_HIGHLIGHT ) );
		else
			dc.FillSolidRect( &rcItem, GetSysColor( COLOR_WINDOW));
	}
	else
		dc.FillSolidRect( &rcItem, GetBkColor(lpDrawItemStruct->hDC) );

	dc.SetBkMode( TRANSPARENT );


	int nImage = (int)lpDrawItemStruct->itemData;
	if ( nImage ) nImage ++;

	if ( theApp.m_bRTL && nImage ) nImage = m_gdiImageList.GetImageCount() - nImage;
	m_gdiImageList.Draw( &dc, nImage, pt,
		( lpDrawItemStruct->itemState & ODS_SELECTED ) ? ILD_SELECTED : ILD_NORMAL );

	rcItem.left += 20; rcItem.right -= 2;

	CString str;
	GetLBText( lpDrawItemStruct->itemID, str );
	dc.DrawText( str, &rcItem, DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_NOPREFIX );

	dc.SelectObject( pOldFont );

	dc.Detach();
}
