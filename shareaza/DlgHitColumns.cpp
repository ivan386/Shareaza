//
// DlgHitColumns.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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
#include "Schema.h"
#include "DlgHitColumns.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CSchemaColumnsDlg, CSkinDialog)
	//{{AFX_MSG_MAP(CSchemaColumnsDlg)
	ON_CBN_SELCHANGE(IDC_SCHEMAS, OnSelChangeSchemas)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSchemaColumnsDlg dialog

CSchemaColumnsDlg::CSchemaColumnsDlg(CWnd* pParent ) : CSkinDialog(CSchemaColumnsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSchemaColumnsDlg)
	//}}AFX_DATA_INIT
}

void CSchemaColumnsDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSchemaColumnsDlg)
	DDX_Control(pDX, IDC_COLUMNS, m_wndColumns);
	DDX_Control(pDX, IDC_SCHEMAS, m_wndSchemas);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CSchemaColumnsDlg message handlers

BOOL CSchemaColumnsDlg::OnInitDialog() 
{
	CSkinDialog::OnInitDialog();
	
	SkinMe( _T("CSchemaColumnsDlg"), IDR_SEARCHFRAME );

	m_wndColumns.InsertColumn( 0, _T("Member"), LVCFMT_LEFT, 128, -1 );
	m_wndColumns.InsertColumn( 1, _T("Type"), LVCFMT_LEFT, 0, 0 );
	ListView_SetExtendedListViewStyle( m_wndColumns.GetSafeHwnd(), LVS_EX_CHECKBOXES );

	m_wndSchemas.Load( m_pSchema ? m_pSchema->m_sURI : _T("") );
	m_wndSchemas.m_sNoSchemaText = _T("No Schema Columns");

	OnSelChangeSchemas();

	for ( int nMember = 0 ; nMember < m_wndColumns.GetItemCount() ; nMember++ )
	{
		BOOL bChecked = m_pColumns.Find( (LPVOID)m_wndColumns.GetItemData( nMember ) ) != NULL;
		m_wndColumns.SetItemState( nMember, INDEXTOSTATEIMAGEMASK( bChecked + 1 ), LVIS_STATEIMAGEMASK );
	}

	return TRUE;
}

void CSchemaColumnsDlg::OnSelChangeSchemas() 
{
	CSchema* pSchema = m_wndSchemas.GetSelected();

	m_wndColumns.DeleteAllItems();
	if ( ! pSchema ) return;
	
	CString strMembers = theApp.GetProfileString( _T("Interface"),
		_T("SchemaColumns.") + pSchema->m_sSingular, _T("(EMPTY)") );
	
	if ( strMembers == _T("(EMPTY)") ) strMembers = pSchema->m_sDefaultColumns;
	
	for ( POSITION pos = pSchema->GetMemberIterator() ; pos ; )
	{
		CSchemaMember* pMember = pSchema->GetNextMember( pos );

		LV_ITEM pItem;
		ZeroMemory( &pItem, sizeof( pItem ) );

		pItem.mask		= LVIF_TEXT|LVIF_PARAM;
		pItem.iItem		= m_wndColumns.GetItemCount();
		pItem.lParam	= (LPARAM)pMember;
		pItem.pszText	= (LPTSTR)(LPCTSTR)pMember->m_sTitle;
		pItem.iItem		= m_wndColumns.InsertItem( &pItem );
		pItem.mask		= LVIF_TEXT;
		pItem.iSubItem	= 1;
		pItem.pszText	= (LPTSTR)(LPCTSTR)pMember->m_sType;
		m_wndColumns.SetItem( &pItem );

		if ( strMembers.Find( _T("|") + pMember->m_sName + _T("|") ) >= 0 )
		{
			m_wndColumns.SetItemState( pItem.iItem, INDEXTOSTATEIMAGEMASK( 2 ),
				LVIS_STATEIMAGEMASK );
		}
	}
}

void CSchemaColumnsDlg::OnOK() 
{
	m_pSchema = m_wndSchemas.GetSelected();

	if ( m_pSchema )
	{
		m_pColumns.RemoveAll();

		for ( int nMember = 0 ; nMember < m_wndColumns.GetItemCount() ; nMember++ )
		{
			if ( ListView_GetCheckState( m_wndColumns.GetSafeHwnd(), nMember ) )
			{
				CSchemaMember* pMember = (CSchemaMember*)m_wndColumns.GetItemData( nMember );
				m_pColumns.AddTail( pMember );
			}
		}

		SaveColumns( m_pSchema, &m_pColumns );
	}
	
	CSkinDialog::OnOK();
}

/////////////////////////////////////////////////////////////////////////////
// CSchemaColumnsDlg load columns utility

BOOL CSchemaColumnsDlg::LoadColumns(CSchema* pSchema, CPtrList* pColumns)
{
	if ( ! pSchema || ! pColumns ) return FALSE;
	pColumns->RemoveAll();
	
	CString strMembers = theApp.GetProfileString( _T("Interface"),
		_T("SchemaColumns.") + pSchema->m_sSingular, _T("(EMPTY)") );
	
	if ( strMembers == _T("(EMPTY)") ) strMembers = pSchema->m_sDefaultColumns;
	
	for ( POSITION pos = pSchema->GetMemberIterator() ; pos ; )
	{
		CSchemaMember* pMember = pSchema->GetNextMember( pos );
		if ( strMembers.Find( _T("|") + pMember->m_sName + _T("|") ) >= 0 )
			pColumns->AddTail( pMember );
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CSchemaColumnsDlg save columns utility

BOOL CSchemaColumnsDlg::SaveColumns(CSchema* pSchema, CPtrList* pColumns)
{
	if ( ! pSchema || ! pColumns ) return FALSE;

	CString strMembers;
	
	for ( POSITION pos = pColumns->GetHeadPosition() ; pos ; )
	{
		CSchemaMember* pMember = (CSchemaMember*)pColumns->GetNext( pos );
		strMembers += '|';
		strMembers += pMember->m_sName;
		strMembers += '|';
	}
	
	theApp.WriteProfileString( _T("Interface"),
		_T("SchemaColumns.") + pSchema->m_sSingular, strMembers );
	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CSchemaColumnsDlg menu builder utility

CMenu* CSchemaColumnsDlg::BuildColumnMenu(CSchema* pSchema, CPtrList* pColumns)
{
	if ( ! pSchema ) return NULL;

	CMenu* pMenu = new CMenu();

	pMenu->CreatePopupMenu();

	UINT nID = 1000;

	for ( POSITION pos = pSchema->GetMemberIterator() ; pos ; nID++ )
	{
		CSchemaMember* pMember = pSchema->GetNextMember( pos );
		UINT nFlags = MF_STRING;
		
		if ( nID > 1000 && ( ( nID - 1000 ) % 16 ) == 0 ) nFlags |= MF_MENUBREAK;
		if ( pColumns && pColumns->Find( pMember ) != NULL ) nFlags |= MF_CHECKED;
		
		pMenu->AppendMenu( nFlags, nID, pMember->m_sTitle );
	}

	return pMenu;
}

/////////////////////////////////////////////////////////////////////////////
// CSchemaColumnsDlg column toggle utility

BOOL CSchemaColumnsDlg::ToggleColumnHelper(CSchema* pSchema, CPtrList* pSource, CPtrList* pTarget, UINT nToggleID, BOOL bSave)
{
	if ( ! pSchema ) return FALSE;

	UINT nID = 1000;

	pTarget->RemoveAll();
	pTarget->AddTail( pSource );

	for ( POSITION pos = pSchema->GetMemberIterator() ; pos ; nID++ )
	{
		CSchemaMember* pMember = pSchema->GetNextMember( pos );

		if ( nID == nToggleID )
		{
			if ( pos = pTarget->Find( pMember ) )
			{
				pTarget->RemoveAt( pos );
			}
			else
			{
				pTarget->AddTail( pMember );
			}
			
			if ( bSave ) SaveColumns( pSchema, pTarget );

			return TRUE;
		}
	}

	return FALSE;
}

