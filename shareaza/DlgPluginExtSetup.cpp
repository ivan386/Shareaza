//
// DlgPluginExtSetup.cpp
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

#include "stdafx.h"
#include "Shareaza.h"
#include "DlgPluginExtSetup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CPluginExtSetupDlg, CDialog)

BEGIN_MESSAGE_MAP(CPluginExtSetupDlg, CDialog)
	//{{AFX_MSG_MAP(CPluginExtSetupDlg)
	ON_NOTIFY(LVN_ITEMCHANGING, IDC_ASSOCIATIONS, OnChangingAssociations)
	ON_BN_CLICKED(IDOK, OnOK)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CPluginExtSetupDlg::CPluginExtSetupDlg(CWnd* pParent, LPCTSTR pszExt)
	: CDialog(CPluginExtSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPluginExtSetupDlg)
	m_sExtensions	= CString( pszExt );
	m_pParent		= (CListCtrl*)pParent;
	//}}AFX_DATA_INIT
}

CPluginExtSetupDlg::~CPluginExtSetupDlg()
{
}

void CPluginExtSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPluginExtSetupDlg)
	DDX_Control(pDX, IDC_ASSOCIATIONS, m_wndList);
	//}}AFX_DATA_MAP
}

BOOL CPluginExtSetupDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect rc;

	m_wndList.GetClientRect( &rc );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL ) + 1;

	m_wndList.InsertColumn( 0, _T("Extension"), LVCFMT_LEFT, rc.right, 0 );
	m_wndList.SendMessage( LVM_SETEXTENDEDLISTVIEWSTYLE,
		LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES|LVS_EX_LABELTIP, 
		LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES|LVS_EX_LABELTIP );

	CStringArray oTokens;
	Split( m_sExtensions, _T('|'), oTokens );

	m_bRunning = FALSE;

	INT_PTR nTotal = oTokens.GetCount();
	INT_PTR nChecked = 0;

	for ( INT_PTR nToken = 0 ; nToken < nTotal ; nToken++ )
	{
		CString strToken = oTokens.GetAt( nToken );
		if ( strToken.IsEmpty() ) continue; // shouldn't happen but anyway

		BOOL bChecked = ( strToken.Left( 1 ) != _T("-") );
		int nItem = m_wndList.InsertItem( LVIF_TEXT, m_wndList.GetItemCount(),
			! bChecked ? strToken.Mid( 1 ) : strToken, 0, 0, 0, 0 );
		
		if ( bChecked )
		{
			m_wndList.SetItemState( nItem, 2 << 12, LVIS_STATEIMAGEMASK );
			nChecked++;
		}
	}
	if ( nChecked == nTotal ) m_bParentState = TRI_TRUE;
	else if ( nChecked == 0 ) m_bParentState = TRI_FALSE;
	else m_bParentState = TRI_UNKNOWN;

	m_bRunning = TRUE;

	UpdateData( FALSE );
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CPluginExtSetupDlg message handlers

void CPluginExtSetupDlg::OnChangingAssociations(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;

	if ( ( pNMLV->uOldState & LVIS_STATEIMAGEMASK ) == 0 &&
		 ( pNMLV->uNewState & LVIS_STATEIMAGEMASK ) != 0 )
	{
		if ( m_bRunning ) *pResult = 1;
	}
}

void CPluginExtSetupDlg::OnOK()
{
	CString strCurrExt, strExt;
	int nTotal = m_wndList.GetItemCount();
	int nChecked = 0;
	TRISTATE bCurrState = m_bParentState;

	for ( int nItem = 0 ; nItem < nTotal ; nItem++ )
	{
		TRISTATE bEnabled = static_cast< TRISTATE >(
			m_wndList.GetItemState( nItem, LVIS_STATEIMAGEMASK ) >> 12 );

		if ( bEnabled == TRI_TRUE )
		{
			nChecked++;
			strExt = m_wndList.GetItemText( nItem, 0 );
		}
		else
			strExt = _T("-") + m_wndList.GetItemText( nItem, 0 );

		// invert the order since the extension map becomes inversed
		strCurrExt.Insert( 0, _T("|") );
		strCurrExt.Insert( 0, strExt );
	}
	if ( strCurrExt.GetLength() )
		strCurrExt.Insert( 0, _T("|") );

	if ( nChecked == nTotal ) bCurrState = TRI_TRUE;
	else if ( nChecked == 0 ) bCurrState = TRI_FALSE;
	else bCurrState = TRI_UNKNOWN;

	if ( strCurrExt != m_sExtensions )
	{
		int nItem = m_pParent->GetNextItem( -1, LVNI_SELECTED );
		m_pParent->SetItemText( nItem, 2, strCurrExt );
		if ( bCurrState != m_bParentState )
		{
			if ( bCurrState != TRI_UNKNOWN ) // 0 state removes checkbox, we don't need that
				m_pParent->SetItemState( nItem, bCurrState << 12, LVIS_STATEIMAGEMASK );
			else
				m_pParent->SetItemState( nItem, 2 << 12, LVIS_STATEIMAGEMASK );
		}
		m_bParentState = bCurrState;
		m_sExtensions = strCurrExt;
	}

	CDialog::OnOK();
}
