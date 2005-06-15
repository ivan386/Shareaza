//
// DlgConnectTo.cpp
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
#include "DlgConnectTo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CConnectToDlg, CSkinDialog)
	//{{AFX_MSG_MAP(CConnectToDlg)
	ON_CBN_CLOSEUP(IDC_CONNECT_HOST, OnSelChangeConnectHost)
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_CBN_CLOSEUP(IDC_CONNECT_PROTOCOL, OnCloseUpConnectProtocol)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CConnectToDlg dialog

CConnectToDlg::CConnectToDlg(CWnd* pParent, BOOL bBrowseHost) : CSkinDialog(CConnectToDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConnectToDlg)
	m_sHost = _T("");
	m_bNoUltraPeer = FALSE;
	m_nPort = 0;
	m_nProtocol = -1;
	//}}AFX_DATA_INIT
	m_bBrowseHost = bBrowseHost;
}

void CConnectToDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConnectToDlg)
	DDX_Control(pDX, IDC_CONNECT_ADVANCED, m_wndAdvanced);
	DDX_Control(pDX, IDC_CONNECT_PROTOCOL, m_wndProtocol);
	DDX_Control(pDX, IDC_CONNECT_ULTRAPEER, m_wndUltrapeer);
	DDX_Control(pDX, IDC_CONNECT_PROMPT, m_wndPrompt);
	DDX_Control(pDX, IDC_CONNECT_PORT, m_wndPort);
	DDX_Control(pDX, IDC_CONNECT_HOST, m_wndHost);
	DDX_CBString(pDX, IDC_CONNECT_HOST, m_sHost);
	DDX_Check(pDX, IDC_CONNECT_ULTRAPEER, m_bNoUltraPeer);
	DDX_Text(pDX, IDC_CONNECT_PORT, m_nPort);
	DDX_CBIndex(pDX, IDC_CONNECT_PROTOCOL, m_nProtocol);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CConnectToDlg message handlers

BOOL CConnectToDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CConnectToDlg"), m_bBrowseHost ? ID_NETWORK_BROWSE_TO : ID_NETWORK_CONNECT_TO );

	SelectCaption( this, m_bBrowseHost );
	SelectCaption( &m_wndPrompt, m_bBrowseHost );

	CBitmap bmImages;
	bmImages.LoadBitmap( IDB_PROTOCOLS );
	if ( theApp.m_bRTL ) 
		bmImages.m_hObject = CreateMirroredBitmap( (HBITMAP)bmImages.m_hObject );
	m_pImages.Create( 16, 16, ILC_COLOR16|ILC_MASK, 7, 1 );
	m_pImages.Add( &bmImages, RGB( 0, 255, 0 ) );

	m_wndAdvanced.ShowWindow( m_bBrowseHost ? SW_HIDE : SW_SHOW );
	m_wndProtocol.ShowWindow( m_bBrowseHost ? SW_HIDE : SW_SHOW );
	m_wndUltrapeer.ShowWindow( m_bBrowseHost ? SW_HIDE : SW_SHOW );
	m_wndUltrapeer.EnableWindow( FALSE );

	int nItem, nCount = theApp.GetProfileInt( _T("ConnectTo"), _T("Count"), 0 );

	for ( nItem = 0 ; nItem < nCount ; nItem++ )
	{
		CString strItem, strHost;
		strItem.Format( _T("%.3i.Host"), nItem + 1 );
		strHost = theApp.GetProfileString( _T("ConnectTo"), strItem, _T("") );
		if ( strHost.GetLength() )
			m_wndHost.SetItemData( m_wndHost.AddString( strHost ), nItem + 1 );
	}

	m_nPort		= Settings.Connection.InPort;
	m_nProtocol	= 1;

	UpdateData( FALSE );

	if ( nItem = theApp.GetProfileInt( _T("ConnectTo"), _T("Last.Index"), 0 ) )
	{
		LoadItem( nItem );
	}

	return TRUE;
}

void CConnectToDlg::LoadItem(int nItem)
{
	CString strItem, strHost;
	strItem.Format( _T("%.3i.Host"), nItem );
	m_sHost = theApp.GetProfileString( _T("ConnectTo"), strItem, _T("") );
	strItem.Format( _T("%.3i.Port"), nItem );
	m_nPort = theApp.GetProfileInt( _T("ConnectTo"), strItem, GNUTELLA_DEFAULT_PORT );
	strItem.Format( _T("%.3i.Protocol"), nItem );
	m_nProtocol = theApp.GetProfileInt( _T("ConnectTo"), strItem, PROTOCOL_G2 );
	UpdateData( FALSE );
}

void CConnectToDlg::OnSelChangeConnectHost()
{
	int nSel = m_wndHost.GetCurSel();
	if ( nSel < 0 ) return;
	LoadItem( m_wndHost.GetItemData( nSel ) );
}

void CConnectToDlg::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	lpMeasureItemStruct->itemWidth	= 256;
	lpMeasureItemStruct->itemHeight	= 18;
}

void CConnectToDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if ( lpDrawItemStruct->itemID == (UINT)-1 ) return;
	if ( ( lpDrawItemStruct->itemAction & ODA_SELECT ) == 0 &&
		 ( lpDrawItemStruct->itemAction & ODA_DRAWENTIRE ) == 0 ) return;

	CRect rcItem( &lpDrawItemStruct->rcItem );
	CPoint pt( rcItem.left + 1, rcItem.top + 1 );
	CString str;
	CDC dc;

	dc.Attach( lpDrawItemStruct->hDC );
	if ( theApp.m_bRTL ) SetLayout( dc.m_hDC, LAYOUT_RTL );

	CFont* pOldFont = (CFont*)dc.SelectObject( &theApp.m_gdiFont );
	dc.SetTextColor( GetSysColor( ( lpDrawItemStruct->itemState & ODS_SELECTED )
		? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT ) );

	dc.FillSolidRect( &rcItem,
		GetSysColor( ( lpDrawItemStruct->itemState & ODS_SELECTED )
		? COLOR_HIGHLIGHT : COLOR_WINDOW ) );
	dc.SetBkMode( TRANSPARENT );

	int nImage = (int)lpDrawItemStruct->itemID;

	if ( theApp.m_bRTL ) 
		nImage = m_pImages.GetImageCount() - nImage - 2;
	else
		nImage += 1;

	m_pImages.Draw( &dc, nImage, pt,
		( lpDrawItemStruct->itemState & ODS_SELECTED ) ? ILD_SELECTED : ILD_NORMAL );

	m_wndProtocol.GetLBText( lpDrawItemStruct->itemID, str );

	rcItem.left += 22; rcItem.right -= 2;
	dc.DrawText( str, &rcItem, DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_NOPREFIX );

	dc.SelectObject( pOldFont );
	dc.Detach();
}

void CConnectToDlg::OnCloseUpConnectProtocol()
{
	int nPort;

	switch ( m_wndProtocol.GetCurSel() + 1 )
	{
	case PROTOCOL_G1:
		nPort = GNUTELLA_DEFAULT_PORT;
		m_wndUltrapeer.EnableWindow( TRUE );
		break;
	case PROTOCOL_G2:
		nPort = GNUTELLA_DEFAULT_PORT;
		m_wndUltrapeer.EnableWindow( FALSE );
		break;
	case PROTOCOL_ED2K:
		nPort = ED2K_DEFAULT_PORT;
		m_wndUltrapeer.EnableWindow( FALSE );
		break;
	default:
		return;
	}

	CString str;
	str.Format( _T("%lu"), nPort );
	m_wndPort.SetWindowText( str );
	m_wndHost.SetFocus();
}

void CConnectToDlg::OnOK()
{
	UpdateData( TRUE );

	int nColon = m_sHost.Find( ':' );

	if ( nColon > 0 )
	{
		CString strPort = m_sHost.Mid( nColon + 1 );
		_stscanf( strPort, _T("%lu"), &m_nPort );
		m_sHost = m_sHost.Left( nColon );

		m_wndHost.SetWindowText( m_sHost );
		m_wndPort.SetWindowText( strPort );
	}

	int nItem = m_wndHost.FindString( -1, m_sHost );

	if ( nItem < 0 )
	{
		nItem = theApp.GetProfileInt( _T("ConnectTo"), _T("Count"), 0 ) + 1;
		theApp.WriteProfileInt( _T("ConnectTo"), _T("Count"), nItem );
		theApp.WriteProfileInt( _T("ConnectTo"), _T("Last.Index"), nItem );
	}
	else
	{
		theApp.WriteProfileInt( _T("ConnectTo"), _T("Last.Index"), ++nItem );
	}

	CString strItem;
	strItem.Format( _T("%.3i.Host"), nItem );
	theApp.WriteProfileString( _T("ConnectTo"), strItem, m_sHost );
	strItem.Format( _T("%.3i.Port"), nItem );
	theApp.WriteProfileInt( _T("ConnectTo"), strItem, m_nPort );
	strItem.Format( _T("%.3i.Protocol"), nItem );
	theApp.WriteProfileInt( _T("ConnectTo"), strItem, m_nProtocol );

	CSkinDialog::OnOK();
}
