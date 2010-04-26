//
// PageSettingsTraffic.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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
	ON_WM_DESTROY()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_PROPERTIES, OnItemChangedProperties)
	ON_EN_CHANGE(IDC_VALUE, OnChangeValue)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_PROPERTIES, OnColumnClickProperties)
	ON_BN_CLICKED(IDC_DEFAULT_VALUE, OnBnClickedDefaultValue)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdvancedSettingsPage property page

CAdvancedSettingsPage::CAdvancedSettingsPage() :
	CSettingsPage(CAdvancedSettingsPage::IDD)
{
}

CAdvancedSettingsPage::~CAdvancedSettingsPage()
{
}

void CAdvancedSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VALUE_SPIN, m_wndValueSpin);
	DDX_Control(pDX, IDC_VALUE, m_wndValue);
	DDX_Control(pDX, IDC_PROPERTIES, m_wndList);
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

	UpdateInputArea();

	m_wndList.EnsureVisible( Settings.General.LastSettingsIndex, FALSE );
	m_wndList.EnsureVisible( Settings.General.LastSettingsIndex +
		m_wndList.GetCountPerPage() - 1, FALSE );

	return TRUE;
}

void CAdvancedSettingsPage::AddSettings()
{
	m_wndList.DeleteAllItems();

	for ( POSITION pos = Settings.GetHeadPosition() ; pos ; )
	{
		CSettings::Item* pItem = Settings.GetNext( pos );
		if ( ! pItem->m_bHidden &&
			( pItem->m_pBool || ( pItem->m_pDword && pItem->m_nScale ) ) )
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

			UpdateListItem( pList.iItem );
		}
	}
}
void CAdvancedSettingsPage::CommitAll()
{
	for ( int nItem = 0 ; nItem < m_wndList.GetItemCount() ; nItem++ )
	{
		EditItem* pItem = (EditItem*)m_wndList.GetItemData( nItem );
		pItem->Commit();
	}
}

void CAdvancedSettingsPage::UpdateAll()
{
	for ( int nItem = 0 ; nItem < m_wndList.GetItemCount() ; nItem++ )
	{
		EditItem* pItem = (EditItem*)m_wndList.GetItemData( nItem );
		pItem->Update();
		UpdateListItem( nItem );
	}

	UpdateInputArea();
}

void CAdvancedSettingsPage::UpdateListItem(int nItem)
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
	if ( pItem->IsDefault() )
	{
		if ( pItem->m_sName.Right( 1 ) == L"*" )
		{
			pItem->m_sName.TrimRight( L"*" );
			m_wndList.SetItemText( nItem, 0, pItem->m_sName );
		}
	}
	else
	{
		if ( pItem->m_sName.Right( 1 ) != L"*" )
		{
			pItem->m_sName += L"*";
			m_wndList.SetItemText( nItem, 0, pItem->m_sName );
		}
	}
}

void CAdvancedSettingsPage::OnItemChangedProperties(NMHDR* /*pNMHDR*/, LRESULT* pResult) 
{
//	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	UpdateInputArea();

	*pResult = 0;
}

void CAdvancedSettingsPage::UpdateInputArea()
{
	int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );
	
	if ( nItem >= 0 )
	{
		EditItem* pItem = (EditItem*)m_wndList.GetItemData( nItem );
		CString strValue;
		
		pItem->m_pItem->SetRange( m_wndValueSpin );
		if ( pItem->m_pItem->m_pDword )
		{
			strValue.Format( _T("%lu"), pItem->m_nValue / pItem->m_pItem->m_nScale );
		}
		else
		{
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

			UpdateListItem( nItem );

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
	CommitAll();

	UpdateAll();
	
	CSettingsPage::OnOK();
}

void CAdvancedSettingsPage::OnDestroy() 
{
	Settings.General.LastSettingsIndex = m_wndList.GetTopIndex();

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

		UpdateListItem( nItem );
		UpdateInputArea();
	}
}

bool CAdvancedSettingsPage::IsModified() const
{
	for ( int nItem = 0 ; nItem < m_wndList.GetItemCount() ; nItem++ )
	{
		EditItem* pItem = (EditItem*)m_wndList.GetItemData( nItem );
		if ( pItem->IsModified() )
			return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////
// CSettingEdit construction

CAdvancedSettingsPage::EditItem::EditItem(CSettings::Item* pItem) :
	m_pItem( pItem ),
	m_nValue( pItem->m_pDword ? *pItem->m_pDword : 0 ),
	m_bValue( pItem->m_pBool ? *pItem->m_pBool : false ),
	m_nOriginalValue( pItem->m_pDword ? *pItem->m_pDword : 0 ),
	m_bOriginalValue( pItem->m_pBool ? *pItem->m_pBool : false ),
	m_sName(  ( ! *pItem->m_szSection ||				// Settings.Name -> General.Name
		! lstrcmpi( pItem->m_szSection, L"Settings" ) )	// .Name -> General.Name
		? L"General" : pItem->m_szSection )
{
	m_sName += L".";
	m_sName += pItem->m_szName;
	if ( !IsDefault() )
		m_sName += L"*";
}

void CAdvancedSettingsPage::EditItem::Update()
{
	if ( m_pItem->m_pDword )
		m_nOriginalValue = m_nValue = *m_pItem->m_pDword;
	else
		m_bOriginalValue = m_bValue = *m_pItem->m_pBool;
}

void CAdvancedSettingsPage::EditItem::Commit()
{
	if ( m_pItem->m_pDword )
	{
		if ( m_nValue != m_nOriginalValue )
			*m_pItem->m_pDword = m_nOriginalValue = m_nValue;
	}
	else
	{
		if ( m_bValue != m_bOriginalValue )
			*m_pItem->m_pBool= m_bOriginalValue = m_bValue;
	}
}

bool CAdvancedSettingsPage::EditItem::IsModified() const
{
	if ( m_pItem->m_pDword )
		return ( m_nValue != m_nOriginalValue );
	else
		return ( m_bValue != m_bOriginalValue );
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
