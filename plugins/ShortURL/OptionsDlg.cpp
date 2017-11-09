//
// OptionsDlg.cpp : Implementation of COptionsDlg
//
// Copyright (c) Nikolay Raspopov, 2014.
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

#include "stdafx.h"
#include "Plugin.h"
#include "OptionsDlg.h"

// COptionsDlg

COptionsDlg::COptionsDlg(CPlugin* pOwner)
	: m_pOwner	( pOwner )
{
}

COptionsDlg::~COptionsDlg()
{
}

void COptionsDlg::LoadList( CString sURLs )
{
	HWND hWnd = GetDlgItem( IDC_URL_LIST ).m_hWnd;

	ListView_DeleteAllItems( hWnd );

	LVITEM item = { LVIF_TEXT };
	while ( sURLs.GetLength() )
	{
		CString sURL = sURLs.SpanExcluding( _T( "|" ) );
		sURLs = sURLs.Mid( sURL.GetLength() + 1 );
		sURL.Trim();
		sURL.Replace( _T( "%7C" ), _T( "|" ) );

		if ( sURL.GetLength() )
		{
			item.pszText = const_cast< LPTSTR >( (LPCTSTR)sURL );
			ListView_InsertItem( hWnd, &item );
			++ item.iItem;
		}
	}

	UpdateInterface();
}

CString COptionsDlg::SaveList() const
{
	CString sURLs;
	HWND hWnd = GetDlgItem( IDC_URL_LIST ).m_hWnd;

	int nCount = ListView_GetItemCount( hWnd );
	for ( int i = 0; i < nCount; i++ )
	{
		CString sURL;
		ListView_GetItemText( hWnd, i, 0, sURL.GetBuffer( MAX_PATH ), MAX_PATH );
		sURL.ReleaseBuffer();
		sURL.Trim();
		sURL.Replace( _T("|"), _T("%7C") );

		if ( sURLs.GetLength() )
			sURLs += _T('|');
		sURLs += sURL;
	}

	return sURLs;
}

int COptionsDlg::GetSelectedURL() const
{
	HWND hWnd = GetDlgItem( IDC_URL_LIST ).m_hWnd;

	int nCount = ListView_GetItemCount( hWnd );
	for ( int i = 0; i < nCount; i++ )
	{
		if ( ListView_GetItemState( hWnd, i, LVIS_SELECTED ) == LVIS_SELECTED )
		{
			return i;
		}
	}

	return -1;
}

void COptionsDlg::UpdateInterface()
{
	HWND hWnd = GetDlgItem( IDC_URL_LIST ).m_hWnd;
	const int nCount = ListView_GetItemCount( hWnd );
	const int nSelected = GetSelectedURL();

	GetDlgItem( IDC_TEST ).EnableWindow( nSelected >= 0 );
	GetDlgItem( IDC_UP ).EnableWindow( nSelected > 0 );
	GetDlgItem( IDC_DOWN ).EnableWindow( nSelected >= 0 && nSelected < nCount - 1 );
	GetDlgItem( IDC_NEW ).EnableWindow( nCount < 100 );
	GetDlgItem( IDC_DELETE ).EnableWindow( nSelected >= 0 );
}

void COptionsDlg::Edit()
{
	HWND hWnd = GetDlgItem( IDC_URL_LIST ).m_hWnd;
	const int nSelected = GetSelectedURL();
	if ( nSelected >= 0 )
	{
		::SetFocus( hWnd );
		ListView_EditLabel( hWnd, nSelected );
	}
}

LRESULT COptionsDlg::OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	CAxDialogImpl< COptionsDlg >::OnInitDialog( uMsg, wParam, lParam, bHandled );

	GetDlgItem( IDC_TEST ).SendMessage( BM_SETIMAGE, IMAGE_ICON,
		(LPARAM)LoadImage( _AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE( IDI_TEST ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR ) );
	GetDlgItem( IDC_UP ).SendMessage( BM_SETIMAGE, IMAGE_ICON,
		(LPARAM)LoadImage( _AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE( IDI_UP ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR ) );
	GetDlgItem( IDC_DOWN ).SendMessage( BM_SETIMAGE, IMAGE_ICON,
		(LPARAM)LoadImage( _AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE( IDI_DOWN ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR ) );
	GetDlgItem( IDC_NEW ).SendMessage( BM_SETIMAGE, IMAGE_ICON,
		(LPARAM)LoadImage( _AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE( IDI_NEW ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR ) );
	GetDlgItem( IDC_DELETE ).SendMessage( BM_SETIMAGE, IMAGE_ICON,
		(LPARAM)LoadImage( _AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE( IDI_DELETE ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR ) );

	HWND hWnd = GetDlgItem( IDC_URL_LIST ).m_hWnd;

	ListView_SetExtendedListViewStyle( hWnd, ListView_GetExtendedListViewStyle( hWnd ) |
		LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_ONECLICKACTIVATE );

	RECT rc;
	::GetClientRect( hWnd, &rc );
	LVCOLUMN col = { LVCF_FMT | LVCF_WIDTH, LVCFMT_LEFT, rc.right - rc.left - GetSystemMetrics( SM_CXVSCROLL ) - 2 };
	ListView_InsertColumn( hWnd, 0, &col );

	LoadList( GetURLs() );

	bHandled = TRUE;
	return 1;  // Let the system set the focus
}

LRESULT COptionsDlg::OnClickedOK( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled )
{
	SaveURLs( SaveList() );

	EndDialog( wID );
	bHandled = TRUE;
	return 0;
}

LRESULT COptionsDlg::OnClickedCancel( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled )
{
	EndDialog( wID );
	bHandled = TRUE;
	return 0;
}

LRESULT COptionsDlg::OnClickedNew( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled )
{
	HWND hWnd = GetDlgItem( IDC_URL_LIST ).m_hWnd;

	int nSelected = GetSelectedURL();
	if ( nSelected < 0 )
		nSelected = 0;

	ListView_SetItemState( hWnd, nSelected, 0, LVIS_SELECTED );

	LVITEM item = { LVIF_TEXT | LVIF_STATE, nSelected, 0, LVIS_SELECTED, LVIS_SELECTED };
	CString sHint = LoadString( IDS_HINT );
	item.pszText = const_cast< LPTSTR >( (LPCTSTR)sHint );
	ListView_InsertItem( hWnd, &item );

	UpdateInterface();

	Edit();

	bHandled = TRUE;
	return 0;
}

LRESULT COptionsDlg::OnClickedDelete( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled )
{
	HWND hWnd = GetDlgItem( IDC_URL_LIST ).m_hWnd;

	const int nCount = ListView_GetItemCount( hWnd );
	const int nSelected = GetSelectedURL();
	if ( nSelected >= 0 )
	{
		ListView_DeleteItem( hWnd, nSelected );
		if ( nCount > 1 )
			ListView_SetItemState( hWnd, nSelected, LVIS_SELECTED, LVIS_SELECTED );

		UpdateInterface();
	}

	bHandled = TRUE;
	return 0;
}

LRESULT COptionsDlg::OnClickedUp( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled )
{
	HWND hWnd = GetDlgItem( IDC_URL_LIST ).m_hWnd;

	const int nSelected = GetSelectedURL();
	if ( nSelected > 0 )
	{
		CString sFirst;
		ListView_GetItemText( hWnd, nSelected - 1, 0, sFirst.GetBuffer( MAX_PATH ), MAX_PATH );
		sFirst.ReleaseBuffer();

		CString sSecond;
		ListView_GetItemText( hWnd, nSelected, 0, sSecond.GetBuffer( MAX_PATH ), MAX_PATH );
		sSecond.ReleaseBuffer();

		ListView_SetItemText( hWnd, nSelected - 1, 0, const_cast< LPTSTR >( (LPCTSTR)sSecond ) );
		ListView_SetItemText( hWnd, nSelected, 0, const_cast< LPTSTR >( (LPCTSTR)sFirst ) );

		ListView_SetItemState( hWnd, nSelected - 1, LVIS_SELECTED, LVIS_SELECTED );
		ListView_SetItemState( hWnd, nSelected, 0, LVIS_SELECTED );

		UpdateInterface();
	}

	bHandled = TRUE;
	return 0;
}

LRESULT COptionsDlg::OnClickedDown( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled )
{
	HWND hWnd = GetDlgItem( IDC_URL_LIST ).m_hWnd;

	const int nCount = ListView_GetItemCount( hWnd );
	const int nSelected = GetSelectedURL();
	if ( nSelected >= 0 && nSelected < nCount - 1 )
	{
		CString sFirst;
		ListView_GetItemText( hWnd, nSelected + 1, 0, sFirst.GetBuffer( MAX_PATH ), MAX_PATH );
		sFirst.ReleaseBuffer();

		CString sSecond;
		ListView_GetItemText( hWnd, nSelected, 0, sSecond.GetBuffer( MAX_PATH ), MAX_PATH );
		sSecond.ReleaseBuffer();

		ListView_SetItemText( hWnd, nSelected + 1, 0, const_cast< LPTSTR >( (LPCTSTR)sSecond ) );
		ListView_SetItemText( hWnd, nSelected, 0, const_cast< LPTSTR >( (LPCTSTR)sFirst ) );

		ListView_SetItemState( hWnd, nSelected + 1, LVIS_SELECTED, LVIS_SELECTED );
		ListView_SetItemState( hWnd, nSelected, 0, LVIS_SELECTED );

		UpdateInterface();
	}

	bHandled = TRUE;
	return 0;
}

LRESULT COptionsDlg::OnItemChanged( int /*idCtrl*/, LPNMHDR /*pNMHDR*/, BOOL& bHandled )
{
	UpdateInterface();

	bHandled = TRUE;
	return 0;
}

LRESULT COptionsDlg::OnItemEdited( int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& bHandled )
{
	HWND hWnd = GetDlgItem( IDC_URL_LIST ).m_hWnd;
	const NMLVDISPINFO* pdi = (const NMLVDISPINFO*)pNMHDR;

	if ( pdi->item.pszText )
	{
		ListView_SetItemText( hWnd, pdi->item.iItem, 0, pdi->item.pszText );
		ListView_SetItemState( hWnd, pdi->item.iItem, LVIS_SELECTED, LVIS_SELECTED );

		UpdateInterface();
	}

	bHandled = TRUE;
	return 0;
}

LRESULT COptionsDlg::OnKeyDown( int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& bHandled )
{
	const bool bControl = ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) != 0;
	LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN)pNMHDR;

	if ( bControl )
	{
		if ( pnkd->wVKey == VK_DOWN )
			OnClickedDown( 0, 0, 0, bHandled );
		else if ( pnkd->wVKey == VK_UP )
			OnClickedUp( 0, 0, 0, bHandled );
	}
	else
	{
		if ( pnkd->wVKey == VK_INSERT )
			OnClickedNew( 0, 0, 0, bHandled );
		else if ( pnkd->wVKey == VK_DELETE )
			OnClickedDelete( 0, 0, 0, bHandled );
		else if ( pnkd->wVKey == VK_F2 )
		{
			Edit();
			bHandled = TRUE;
		}
	}

	return 0;
}

LRESULT COptionsDlg::OnDblClick( int /*idCtrl*/, LPNMHDR /*pNMHDR*/, BOOL& bHandled )
{
	Edit();

	bHandled = TRUE;
	return 0;
}

LRESULT COptionsDlg::OnClickedDefaults( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled )
{
	LoadList( LoadString( IDS_URL ) );

	bHandled = TRUE;
	return 0;
}

LRESULT COptionsDlg::OnClickedTest( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled )
{
	HWND hWnd = GetDlgItem( IDC_URL_LIST ).m_hWnd;

	const int nSelected = GetSelectedURL();
	if ( nSelected >= 0 )
	{
		CString sURL;
		ListView_GetItemText( hWnd, nSelected, 0, sURL.GetBuffer( MAX_PATH ), MAX_PATH );
		sURL.ReleaseBuffer();

		CComPtr< IProgressDialog > pProgress;
		pProgress.CoCreateInstance( CLSID_ProgressDialog );
		if ( pProgress )
		{
			pProgress->SetTitle( LoadString( IDS_PROJNAME ) );
			pProgress->SetLine( 1, LoadString( IDS_PROGRESS ), FALSE, NULL );
			pProgress->SetLine( 2, sURL.Left( sURL.ReverseFind( _T('/') ) ), FALSE, NULL );
			pProgress->StartProgressDialog( NULL, NULL, PROGDLG_NOTIME | PROGDLG_NOCANCEL | PROGDLG_MARQUEEPROGRESS, NULL );
		}

		CStringA sShortURL = m_pOwner->RequestURL( sURL + URLEncode( LoadString( IDS_TEST ) ) );

		if ( pProgress )
			pProgress->StopProgressDialog();

		MessageBox( sShortURL.IsEmpty() ? LoadString( IDS_FAILED ) : ( LoadString( IDS_URL_REPORT ) + _T(" ") + CA2T( sShortURL ) ),
			LoadString( IDS_PROJNAME ), MB_OK | ( sShortURL.IsEmpty() ? MB_ICONERROR : MB_ICONINFORMATION ) );
	}

	bHandled = TRUE;
	return 0;
}
