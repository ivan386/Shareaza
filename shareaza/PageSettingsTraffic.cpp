//
// PageSettingsTraffic.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "LiveList.h"
#include "PageSettingsTraffic.h"
#include "Skin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CAdvancedSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CAdvancedSettingsPage, CSettingsPage)
	//{{AFX_MSG_MAP(CAdvancedSettingsPage)
	ON_WM_DESTROY()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_PROPERTIES, OnItemChangedProperties)
	ON_EN_CHANGE(IDC_VALUE, OnChangeValue)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_PROPERTIES, OnColumnClickProperties)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_DEFAULT_VALUE, OnBnClickedDefaultValue)
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CAdvancedSettingsPage property page

CAdvancedSettingsPage::CAdvancedSettingsPage() : CSettingsPage(CAdvancedSettingsPage::IDD)
{
	//{{AFX_DATA_INIT(CAdvancedSettingsPage)
	//}}AFX_DATA_INIT
}

CAdvancedSettingsPage::~CAdvancedSettingsPage()
{
}

void CAdvancedSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAdvancedSettingsPage)
	DDX_Control(pDX, IDC_VALUE_SPIN, m_wndValueSpin);
	DDX_Control(pDX, IDC_VALUE, m_wndValue);
	DDX_Control(pDX, IDC_PROPERTIES, m_wndList);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CAdvancedSettingsPage message handlers

BOOL CAdvancedSettingsPage::OnInitDialog() 
{
	CSettingsPage::OnInitDialog();

	CRect rc;
	m_wndList.GetClientRect( &rc );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL ) + 1;

	m_wndList.InsertColumn( 0, _T("Setting"), LVCFMT_LEFT, rc.right - 80, 0 );
	m_wndList.InsertColumn( 1, _T("Value"), LVCFMT_LEFT, 80, 1 );

	m_wndList.SendMessage( LVM_SETEXTENDEDLISTVIEWSTYLE,
		LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT );

	Skin.Translate( _T("CAdvancedSettingsList"), m_wndList.GetHeaderCtrl() );

	AddSettings();

	CLiveList::Sort( &m_wndList, 0 );
	CLiveList::Sort( &m_wndList, 0 );

	UpdateAll();

	return TRUE;
}

void CAdvancedSettingsPage::AddSettings()
{
	for ( POSITION pos = Settings.GetHeadPosition() ; pos ; )
	{
		CSettings::Item* pItem = Settings.GetNext( pos );
		if ( pItem->m_pBool || ( pItem->m_pDword && pItem->m_nScale ) )
		{
			EditItem* pEdit = new EditItem( pItem );
			ASSERT( pEdit != NULL );
			if ( pEdit == NULL ) return;

			LV_ITEM pList = {};
			pList.mask		= LVIF_PARAM|LVIF_TEXT|LVIF_IMAGE;
			pList.iItem		= m_wndList.GetItemCount();
			pList.lParam	= (LPARAM)pEdit;
			pList.iImage	= 0;
			pList.pszText	= (LPTSTR)(LPCTSTR)pEdit->m_sName;
			pList.iItem		= m_wndList.InsertItem( &pList );

			UpdateItem( pList.iItem );
		}
	}
}

void CAdvancedSettingsPage::UpdateItem(int nItem)
{
	EditItem* pItem = (EditItem*)m_wndList.GetItemData( nItem );
	CString strValue;
		
	if ( pItem->m_pItem->m_pBool )
	{
		ASSERT( pItem->m_pItem->m_nScale == 1 &&
			pItem->m_pItem->m_nMin == 0 && pItem->m_pItem->m_nMax == 1 );
		strValue = pItem->m_bValue ? _T("True") : _T("False");
	}
	else if ( pItem->m_pItem->m_pDword )
	{
		ASSERT( pItem->m_pItem->m_nScale &&
			pItem->m_pItem->m_nMin < pItem->m_pItem->m_nMax );
		strValue.Format( _T("%lu"), pItem->m_nValue / pItem->m_pItem->m_nScale );
		if ( Settings.General.LanguageRTL )
			strValue = _T("\x200E") + strValue + pItem->m_pItem->m_szSuffix;
		else
			strValue += pItem->m_pItem->m_szSuffix;
	}
	
	m_wndList.SetItemText( nItem, 1, strValue );
}

void CAdvancedSettingsPage::OnItemChangedProperties(NMHDR* /*pNMHDR*/, LRESULT* pResult) 
{
//	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	UpdateAll();

	*pResult = 0;
}

void CAdvancedSettingsPage::UpdateAll()
{
	int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );
	
	if ( nItem >= 0 )
	{
		EditItem* pItem = (EditItem*)m_wndList.GetItemData( nItem );
		CString strValue;
		
		if ( pItem->m_pItem->m_pDword )
		{
			m_wndValueSpin.SendMessage( UDM_SETRANGE32, pItem->m_pItem->m_nMin, pItem->m_pItem->m_nMax );
			strValue.Format( _T("%lu"), pItem->m_nValue / pItem->m_pItem->m_nScale );
		}
		else
		{
			m_wndValueSpin.SendMessage( UDM_SETRANGE32, 0, 1 );
			strValue = pItem->m_bValue ? _T("1") : _T("0");
		}
		m_wndValue.SetWindowText( strValue );
		m_wndValue.EnableWindow( TRUE );
		m_wndValueSpin.EnableWindow( TRUE );
		GetDlgItem( IDC_DEFAULT_VALUE )->EnableWindow( ! pItem->IsDefault() );
	}
	else
	{
		m_wndValue.SetWindowText( _T("") );
		m_wndValue.EnableWindow( FALSE );
		m_wndValueSpin.EnableWindow( FALSE );
		GetDlgItem( IDC_DEFAULT_VALUE )->EnableWindow( FALSE );
	}	
}

void CAdvancedSettingsPage::OnChangeValue() 
{
	if ( m_wndList.m_hWnd == NULL ) return;
	
	int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );
	
	if ( nItem >= 0 )
	{
		EditItem* pItem = (EditItem*)m_wndList.GetItemData( nItem );
		CString strValue;
		
		m_wndValue.GetWindowText( strValue );
		
		DWORD nValue = 0;
		if ( _stscanf( strValue, _T("%lu"), &nValue ) == 1 )
		{
			if ( pItem->m_pItem->m_pDword )
				pItem->m_nValue = max( pItem->m_pItem->m_nMin,
					min( pItem->m_pItem->m_nMax, nValue ) ) * pItem->m_pItem->m_nScale;
			else
				pItem->m_bValue = ( nValue == 1 );

			UpdateItem( nItem );

			GetDlgItem( IDC_DEFAULT_VALUE )->EnableWindow( ! pItem->IsDefault() );
		}
	}
}

void CAdvancedSettingsPage::OnColumnClickProperties(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	CLiveList::Sort( &m_wndList, pNMListView->iSubItem );
	*pResult = 0;
}

void CAdvancedSettingsPage::OnOK() 
{
	for ( int nItem = 0 ; nItem < m_wndList.GetItemCount() ; nItem++ )
	{
		EditItem* pItem = (EditItem*)m_wndList.GetItemData( nItem );
		pItem->Commit();
	}
	
	CSettingsPage::OnOK();
}

void CAdvancedSettingsPage::OnDestroy() 
{
	for ( int nItem = 0 ; nItem < m_wndList.GetItemCount() ; nItem++ )
	{
		delete (EditItem*)m_wndList.GetItemData( nItem );
	}
	
	CSettingsPage::OnDestroy();
}

void CAdvancedSettingsPage::OnBnClickedDefaultValue()
{
	int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );
	if ( nItem >= 0 )
	{
		EditItem* pItem = (EditItem*)m_wndList.GetItemData( nItem );
		pItem->Default();

		UpdateItem( nItem );
		UpdateAll();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSettingEdit construction

CAdvancedSettingsPage::EditItem::EditItem(CSettings::Item* pItem) :
	m_pItem( pItem ),
	m_nValue( pItem->m_pDword ? *pItem->m_pDword : 0 ),
	m_bValue( pItem->m_pBool ? *pItem->m_pBool : false ),
	m_sName(  ( ! *pItem->m_szSection ||					// Settings.Name -> General.Name
		! lstrcmp( pItem->m_szSection, _T("Settings") ) )	// .Name -> General.Name
		? _T("General") : pItem->m_szSection )
{
	m_sName += _T(".");
	m_sName += pItem->m_szName;
}

void CAdvancedSettingsPage::EditItem::Commit()
{
	if ( m_pItem->m_pDword )
		*m_pItem->m_pDword = m_nValue;
	else
		*m_pItem->m_pBool= m_bValue;
}

bool CAdvancedSettingsPage::EditItem::IsDefault() const
{
	if ( m_pItem->m_pDword )
		return ( m_pItem->m_DwordDefault == m_nValue );
	else
		return ( m_pItem->m_BoolDefault == m_bValue );
}

void CAdvancedSettingsPage::EditItem::Default()
{
	if ( m_pItem->m_pDword )
		m_nValue = m_pItem->m_DwordDefault;
	else
		m_bValue = m_pItem->m_BoolDefault;
}
