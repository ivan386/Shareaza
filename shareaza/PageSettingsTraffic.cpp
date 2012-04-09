//
// PageSettingsTraffic.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_PROPERTIES, &CAdvancedSettingsPage::OnItemChangedProperties)
	ON_EN_CHANGE(IDC_VALUE, &CAdvancedSettingsPage::OnChangeValue)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_PROPERTIES, &CAdvancedSettingsPage::OnColumnClickProperties)
	ON_BN_CLICKED(IDC_DEFAULT_VALUE, &CAdvancedSettingsPage::OnBnClickedDefaultValue)
	ON_BN_CLICKED(IDC_BOOL, &CAdvancedSettingsPage::OnChangeValue)
	ON_CBN_SELCHANGE(IDC_FONT, &CAdvancedSettingsPage::OnChangeValue)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdvancedSettingsPage property page

CAdvancedSettingsPage::CAdvancedSettingsPage()
	: CSettingsPage(CAdvancedSettingsPage::IDD)
	,  m_bUpdating( false )
{
}

void CAdvancedSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VALUE_SPIN, m_wndValueSpin);
	DDX_Control(pDX, IDC_VALUE, m_wndValue);
	DDX_Control(pDX, IDC_PROPERTIES, m_wndList);
	DDX_Control(pDX, IDC_FONT, m_wndFonts);
	DDX_Control(pDX, IDC_DEFAULT_VALUE, m_wndDefaultBtn);
	DDX_Control(pDX, IDC_BOOL, m_wndBool);
}

/////////////////////////////////////////////////////////////////////////////
// CAdvancedSettingsPage message handlers

BOOL CAdvancedSettingsPage::OnInitDialog() 
{
	CSettingsPage::OnInitDialog();

	CRect rc;
	m_wndList.GetClientRect( &rc );
	rc.right -= GetSystemMetrics( SM_CXVSCROLL ) + 1;

	m_wndList.InsertColumn( 0, _T("Setting"), LVCFMT_LEFT, rc.right - 120, 0 );
	m_wndList.InsertColumn( 1, _T("Value"), LVCFMT_LEFT, 120, 1 );
	m_wndList.SetExtendedStyle( m_wndList.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP );

	Skin.Translate( _T("CAdvancedSettingsList"), m_wndList.GetHeaderCtrl() );

	AddSettings();

	CLiveList::Sort( &m_wndList, 0 );
	CLiveList::Sort( &m_wndList, 0 );

	UpdateInputArea();

	m_wndList.EnsureVisible( Settings.General.LastSettingsIndex, FALSE );
	m_wndList.EnsureVisible( Settings.General.LastSettingsIndex + m_wndList.GetCountPerPage() - 1, FALSE );

	return TRUE;
}

void CAdvancedSettingsPage::AddSettings()
{
	m_wndList.DeleteAllItems();

	for ( POSITION pos = Settings.GetHeadPosition() ; pos ; )
	{
		CSettings::Item* pItem = Settings.GetNext( pos );
		if ( ! pItem->m_bHidden &&
			( pItem->m_pBool ||
			( pItem->m_pDword && pItem->m_nScale ) ||
			( pItem->m_pString && pItem->m_nType != CSettings::setNull ) ) )
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
	else if ( pItem->m_pItem->m_pString )
	{
		strValue = pItem->m_sValue;
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
	*pResult = 0;

	UpdateInputArea();
}

void CAdvancedSettingsPage::UpdateInputArea()
{
	if ( m_bUpdating ) return;
	m_bUpdating = true;

	int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );

	m_wndValue.ShowWindow( SW_HIDE );
	m_wndValue.EnableWindow( FALSE );

	m_wndBool.ShowWindow( SW_HIDE );
	m_wndBool.EnableWindow( FALSE );

	m_wndValueSpin.ShowWindow( SW_HIDE );
	m_wndValueSpin.EnableWindow( FALSE );

	m_wndFonts.ShowWindow( SW_HIDE );
	m_wndFonts.EnableWindow( TRUE );

	m_wndDefaultBtn.EnableWindow( FALSE );
	
	if ( nItem >= 0 )
	{
		const EditItem* pItem = (EditItem*)m_wndList.GetItemData( nItem );
		
		if ( pItem->m_pItem->m_pDword )
		{
			CString strValue;
			strValue.Format( _T("%lu"), pItem->m_nValue / pItem->m_pItem->m_nScale );
			m_wndValue.ShowWindow( SW_SHOW );
			m_wndValue.EnableWindow( TRUE );
			m_wndValueSpin.ShowWindow( SW_SHOW );
			m_wndValueSpin.EnableWindow( TRUE );
			pItem->m_pItem->SetRange( m_wndValueSpin );
			m_wndValue.SetWindowText( strValue );
		}
		else if ( pItem->m_pItem->m_pBool )
		{
			m_wndBool.SetCheck( pItem->m_bValue ? BST_CHECKED : BST_UNCHECKED );
			m_wndBool.ShowWindow( SW_SHOW );
			m_wndBool.EnableWindow( TRUE );
		}
		else if ( pItem->m_pItem->m_pString && pItem->m_pItem->m_nType == CSettings::setString )
		{
			m_wndValue.SetWindowText( pItem->m_sValue );
			m_wndValue.ShowWindow( SW_SHOW );
			m_wndValue.EnableWindow( TRUE );
			m_wndValueSpin.ShowWindow( SW_SHOW );
		}
		else if ( pItem->m_pItem->m_pString && pItem->m_pItem->m_nType == CSettings::setFont )
		{
			m_wndFonts.SelectFont( pItem->m_sValue );
			m_wndFonts.ShowWindow( SW_SHOW );
			m_wndFonts.EnableWindow( TRUE );
		}

		if ( ! pItem->IsDefault() )
			m_wndDefaultBtn.EnableWindow( TRUE );
	}

	m_bUpdating = false;
}

void CAdvancedSettingsPage::OnChangeValue() 
{
	if ( m_wndList.m_hWnd == NULL || m_bUpdating ) return;
	
	int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );
	
	if ( nItem >= 0 )
	{
		EditItem* pItem = (EditItem*)m_wndList.GetItemData( nItem );

		if ( pItem->m_pItem->m_pDword )
		{
			CString strValue;
			m_wndValue.GetWindowText( strValue );
			DWORD nValue;
			if ( _stscanf( strValue, _T("%lu"), &nValue ) != 1 )
				return;
			pItem->m_nValue = max( pItem->m_pItem->m_nMin, min( pItem->m_pItem->m_nMax, nValue ) ) * pItem->m_pItem->m_nScale;
		}
		else if ( pItem->m_pItem->m_pBool )
		{
			pItem->m_bValue = ( m_wndBool.GetCheck() == BST_CHECKED );
		}
		else if ( pItem->m_pItem->m_pString && pItem->m_pItem->m_nType == CSettings::setString )
		{
			m_wndValue.GetWindowText( pItem->m_sValue );
		}
		else if ( pItem->m_pItem->m_pString && pItem->m_pItem->m_nType == CSettings::setFont )
		{
			pItem->m_sValue = m_wndFonts.GetSelectedFont();
		}
		else
			return;

		UpdateListItem( nItem );

		m_wndDefaultBtn.EnableWindow( ! pItem->IsDefault() );
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

	m_wndFonts.Invalidate();

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
		const EditItem* pItem = (EditItem*)m_wndList.GetItemData( nItem );
		if ( pItem->IsModified() )
			return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////
// CSettingEdit construction

CAdvancedSettingsPage::EditItem::EditItem(const CSettings::Item* pItem) :
	m_pItem( const_cast<CSettings::Item*>( pItem ) ),
	m_nValue( pItem->m_pDword ? *pItem->m_pDword : 0 ),
	m_bValue( pItem->m_pBool ? *pItem->m_pBool : false ),
	m_sValue( pItem->m_pString ? *pItem->m_pString : CString() ),
	m_nOriginalValue( pItem->m_pDword ? *pItem->m_pDword : 0 ),
	m_bOriginalValue( pItem->m_pBool ? *pItem->m_pBool : false ),
	m_sOriginalValue( pItem->m_pString ? *pItem->m_pString : CString() ),
	m_sName(  ( ! *pItem->m_szSection ||				// Settings.Name -> General.Name
		! lstrcmpi( pItem->m_szSection, L"Settings" ) )	// .Name -> General.Name
		? L"General" : pItem->m_szSection )
{
	m_sName += L".";
	m_sName += pItem->m_szName;
	if ( ! IsDefault() )
		m_sName += L"*";
}

void CAdvancedSettingsPage::EditItem::Update()
{
	if ( m_pItem->m_pDword )
		m_nOriginalValue = m_nValue = *m_pItem->m_pDword;
	else if ( m_pItem->m_pBool )
		m_bOriginalValue = m_bValue = *m_pItem->m_pBool;
	else if ( m_pItem->m_pString )
		m_sOriginalValue = m_sValue = *m_pItem->m_pString;
}

void CAdvancedSettingsPage::EditItem::Commit()
{
	if ( m_pItem->m_pDword )
	{
		if ( m_nValue != m_nOriginalValue )
			*m_pItem->m_pDword = m_nOriginalValue = m_nValue;
	}
	else if ( m_pItem->m_pBool )
	{
		if ( m_bValue != m_bOriginalValue )
			*m_pItem->m_pBool= m_bOriginalValue = m_bValue;
	}
	else if ( m_pItem->m_pString )
	{
		if ( m_sValue != m_sOriginalValue )
			*m_pItem->m_pString= m_sOriginalValue = m_sValue;
	}
}

bool CAdvancedSettingsPage::EditItem::IsModified() const
{
	if ( m_pItem->m_pDword )
		return ( m_nValue != m_nOriginalValue );
	else if ( m_pItem->m_pBool )
		return ( m_bValue != m_bOriginalValue );
	else if ( m_pItem->m_pString )
		return ( m_sValue != m_sOriginalValue );
	else
		return false;
}

bool CAdvancedSettingsPage::EditItem::IsDefault() const
{
	if ( m_pItem->m_pDword )
		return ( m_pItem->m_DwordDefault == m_nValue );
	else if ( m_pItem->m_pBool )
		return ( m_pItem->m_BoolDefault == m_bValue );
	else if ( m_pItem->m_pString )
		return ( m_pItem->m_StringDefault == NULL || m_sValue == m_pItem->m_StringDefault );
	else
		return true;
}

void CAdvancedSettingsPage::EditItem::Default()
{
	if ( m_pItem->m_pDword )
		m_nValue = m_pItem->m_DwordDefault;
	else if ( m_pItem->m_pBool )
		m_bValue = m_pItem->m_BoolDefault;
	else if ( m_pItem->m_pString )
		m_sValue = m_pItem->m_StringDefault;
}
