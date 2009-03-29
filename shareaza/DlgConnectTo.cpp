//
// DlgConnectTo.cpp
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
#include "DlgConnectTo.h"
#include "CoolInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef struct {
	CString		sHost;
	int			nPort;
	PROTOCOLID	nProtocol;
} CONNECT_HOST_DATA;

const LPCTSTR CONNECT_SECTION = _T("ConnectTo");

BEGIN_MESSAGE_MAP(CConnectToDlg, CSkinDialog)
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_CONNECT_HOST, OnCbnSelchangeConnectHost)
	ON_CBN_SELCHANGE(IDC_CONNECT_PROTOCOL, OnCbnSelchangeConnectProtocol)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CConnectToDlg dialog

CConnectToDlg::CConnectToDlg(CWnd* pParent, BOOL bBrowseHost) :
	CSkinDialog		( CConnectToDlg::IDD, pParent )
,	m_bNoUltraPeer	( FALSE )
,	m_nPort			( GNUTELLA_DEFAULT_PORT )
,	m_nProtocol		( 1 )							// G2 Protocol
,	m_bBrowseHost	( bBrowseHost )
{
}

void CConnectToDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
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
}

/////////////////////////////////////////////////////////////////////////////
// CConnectToDlg message handlers

BOOL CConnectToDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CConnectToDlg"), m_bBrowseHost ? ID_NETWORK_BROWSE_TO : ID_NETWORK_CONNECT_TO );

	SelectCaption( this, m_bBrowseHost );
	SelectCaption( &m_wndPrompt, m_bBrowseHost );

	// Load default images
	CBitmap bmImages;
	bmImages.LoadBitmap( IDB_PROTOCOLS );
	if ( Settings.General.LanguageRTL )
		bmImages.m_hObject = CreateMirroredBitmap( (HBITMAP)bmImages.m_hObject );

	m_pImages.Create( 16, 16, ILC_COLOR32|ILC_MASK, 7, 1 ) ||
	m_pImages.Create( 16, 16, ILC_COLOR24|ILC_MASK, 7, 1 ) ||
	m_pImages.Create( 16, 16, ILC_COLOR16|ILC_MASK, 7, 1 );
	m_pImages.Add( &bmImages, RGB( 0, 255, 0 ) );

	// Replace with the skin images (if fails old images remain)
	for ( int nImage = 1 ; nImage < 4 ; nImage++ )
	{
		HICON hIcon = CoolInterface.ExtractIcon( (UINT)protocolCmdMap[ nImage ].commandID, FALSE );
		if ( hIcon )
		{
			m_pImages.Replace( nImage, hIcon );
			DestroyIcon( hIcon );
		}
	}

	m_wndAdvanced.ShowWindow( m_bBrowseHost ? SW_HIDE : SW_SHOW );
	m_wndProtocol.ShowWindow( m_bBrowseHost ? SW_HIDE : SW_SHOW );
	m_wndUltrapeer.ShowWindow( m_bBrowseHost ? SW_HIDE : SW_SHOW );
	m_wndUltrapeer.EnableWindow( FALSE );

	int nItem, nCount = theApp.GetProfileInt( CONNECT_SECTION, _T("Count"), 0 );

	for ( nItem = 0 ; nItem < nCount ; nItem++ )
	{
		CONNECT_HOST_DATA* pData = new CONNECT_HOST_DATA;
		if ( pData )
		{
			CString strItem;
			strItem.Format( _T("%.3i.Host"), nItem + 1 );
			pData->sHost = theApp.GetProfileString( CONNECT_SECTION, strItem, _T("") );
			pData->sHost.Trim( _T(" \t\r\n:\"") );
			ToLower( pData->sHost );

			strItem.Format( _T("%.3Ii.Port"), nItem + 1 );
			pData->nPort = theApp.GetProfileInt( CONNECT_SECTION, strItem, GNUTELLA_DEFAULT_PORT );

			strItem.Format( _T("%.3Ii.Protocol"), nItem + 1 );
			pData->nProtocol = (PROTOCOLID)theApp.GetProfileInt( CONNECT_SECTION, strItem, PROTOCOL_G2 );

			// Validation
			if ( pData->sHost.GetLength() &&
				pData->nPort >= 0 &&
				pData->nPort <= 65535 &&
				( pData->nProtocol == PROTOCOL_G1 ||
				  pData->nProtocol == PROTOCOL_G2 ||
				  pData->nProtocol == PROTOCOL_ED2K ) &&
				m_wndHost.FindStringExact( -1, pData->sHost ) == CB_ERR )
			{
				int nIndex = m_wndHost.AddString( pData->sHost );
				ASSERT( nIndex != CB_ERR );
				VERIFY( m_wndHost.SetItemDataPtr( nIndex, pData ) != CB_ERR );
			}
			else
			{
				delete pData;
			}
		}
	}
	nCount = m_wndHost.GetCount();
	nItem = theApp.GetProfileInt( CONNECT_SECTION, _T("Last.Index"), 0 );
	if ( nItem >= nCount ) nItem = 0;
	LoadItem( nItem );

	m_wndPort.SendMessage(EM_SETLIMITTEXT, 5);

	UpdateData( FALSE );

	return TRUE;
}

void CConnectToDlg::LoadItem(int nItem)
{
	ASSERT( nItem != CB_ERR);

	if ( m_wndHost.GetCurSel() != nItem )
	{
		m_wndHost.SetCurSel( nItem );
	}
	CONNECT_HOST_DATA* pData = static_cast< CONNECT_HOST_DATA* >( m_wndHost.GetItemDataPtr( nItem ) );
	ASSERT( pData != NULL );
	if ( reinterpret_cast< INT_PTR >( pData ) != -1 )
	{
		m_sHost		= pData->sHost;
		m_nPort		= pData->nPort;
		m_nProtocol	= pData->nProtocol - 1;
	}
	m_wndUltrapeer.EnableWindow( ( m_nProtocol + 1 ) == PROTOCOL_G1 );
	UpdateData( FALSE );
}

void CConnectToDlg::OnCbnSelchangeConnectHost()
{
	if ( ! UpdateData () ) return;

	int nItem = m_wndHost.GetCurSel();
	ASSERT( nItem != CB_ERR );
	LoadItem( nItem );
}

void CConnectToDlg::OnCbnSelchangeConnectProtocol()
{
	if ( ! UpdateData () ) return;

	m_wndUltrapeer.EnableWindow( ( m_nProtocol + 1 ) == PROTOCOL_G1 );
}

void CConnectToDlg::OnMeasureItem(int /*nIDCtl*/, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	lpMeasureItemStruct->itemWidth	= 256;
	lpMeasureItemStruct->itemHeight	= 18;
}

void CConnectToDlg::OnDrawItem(int /*nIDCtl*/, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if ( lpDrawItemStruct->itemID == (UINT)-1 ) return;
	if ( ( lpDrawItemStruct->itemAction & ODA_SELECT ) == 0 &&
		 ( lpDrawItemStruct->itemAction & ODA_DRAWENTIRE ) == 0 ) return;

	CRect rcItem( &lpDrawItemStruct->rcItem );
	CPoint pt( rcItem.left + 1, rcItem.top + 1 );
	CString str;
	CDC dc;

	dc.Attach( lpDrawItemStruct->hDC );
	if ( Settings.General.LanguageRTL )
		SetLayout( dc.m_hDC, LAYOUT_RTL );

	CFont* pOldFont = (CFont*)dc.SelectObject( &theApp.m_gdiFont );
	dc.SetTextColor( GetSysColor( ( lpDrawItemStruct->itemState & ODS_SELECTED )
		? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT ) );

	dc.FillSolidRect( &rcItem,
		GetSysColor( ( lpDrawItemStruct->itemState & ODS_SELECTED )
		? COLOR_HIGHLIGHT : COLOR_WINDOW ) );
	dc.SetBkMode( TRANSPARENT );

	int nImage = (int)lpDrawItemStruct->itemID;

	if ( Settings.General.LanguageRTL )
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

void CConnectToDlg::OnOK()
{
	if ( ! UpdateItems() )
		return;

	SaveItems();

	CSkinDialog::OnOK();
}

BOOL CConnectToDlg::UpdateItems()
{
	if ( ! UpdateData() )
		return FALSE;

	m_sHost.Trim( _T(" \t\r\n:\"\'\\") ).MakeLower();
	int n = m_sHost.Find( _T(':') );
	if ( n != -1 )
	{
		m_nPort = _tstoi( m_sHost.Mid( n + 1 ) );
		m_sHost = m_sHost.Left( n );
	}
	if ( m_sHost.IsEmpty() || m_nPort <= 0 || m_nPort >= 65536 )
	{
		UpdateData( FALSE );
		return FALSE;
	}

	int nItem = m_wndHost.FindStringExact( -1, m_sHost );
	if ( nItem != CB_ERR )
	{
		// Edit existing item
		CONNECT_HOST_DATA* pData = static_cast< CONNECT_HOST_DATA* >( m_wndHost.GetItemDataPtr( nItem ) );
		ASSERT( pData != NULL && reinterpret_cast< INT_PTR >( pData ) != -1 );
		pData->nPort = m_nPort;
		pData->nProtocol = (PROTOCOLID)( m_nProtocol + 1 );
		if( m_wndHost.GetCurSel() != nItem ) m_wndHost.SetCurSel( nItem );
	}
	else
	{
		// Create new item
		CONNECT_HOST_DATA* pData = new CONNECT_HOST_DATA;
		ASSERT( pData != NULL );
		if ( pData )
		{
			pData->sHost = m_sHost;
			pData->nPort = m_nPort;
			pData->nProtocol = (PROTOCOLID)( m_nProtocol + 1 );
			nItem = m_wndHost.AddString( pData->sHost );
			ASSERT( nItem != CB_ERR );
			VERIFY( m_wndHost.SetItemDataPtr( nItem, pData ) != CB_ERR );
			VERIFY( m_wndHost.SetCurSel( nItem ) != CB_ERR );
		}
	}

	return TRUE;
}

void CConnectToDlg::SaveItems()
{
	int nCount = m_wndHost.GetCount();
	ASSERT( nCount != CB_ERR );
	theApp.WriteProfileInt( CONNECT_SECTION, _T("Count"), nCount );

	int nItem = m_wndHost.GetCurSel();
	ASSERT( nItem != CB_ERR );
	theApp.WriteProfileInt( CONNECT_SECTION, _T("Last.Index"), nItem );

	for( nItem = 0; nItem < nCount; nItem++ )
	{
		CONNECT_HOST_DATA* pData = static_cast< CONNECT_HOST_DATA* >( m_wndHost.GetItemDataPtr( nItem ) );
		ASSERT( pData != NULL && reinterpret_cast< INT_PTR >( pData ) != -1 );

		CString strItem;
		strItem.Format( _T("%.3i.Host"), nItem + 1 );
		theApp.WriteProfileString( CONNECT_SECTION, strItem, pData->sHost );
		strItem.Format( _T("%.3i.Port"), nItem + 1 );
		theApp.WriteProfileInt( CONNECT_SECTION, strItem, pData->nPort );
		strItem.Format( _T("%.3i.Protocol"), nItem + 1 );
		theApp.WriteProfileInt( CONNECT_SECTION, strItem, pData->nProtocol );
	}
}

void CConnectToDlg::OnDestroy()
{
	while( m_wndHost.GetCount() )
	{
		CONNECT_HOST_DATA* pData = static_cast< CONNECT_HOST_DATA* >( m_wndHost.GetItemDataPtr( 0 ) );
		ASSERT( pData != NULL && reinterpret_cast< INT_PTR >( pData ) != -1 );

		m_wndHost.DeleteString( 0 );
		delete pData;
	}

	CSkinDialog::OnDestroy();
}
