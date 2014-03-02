//
// DlgSelect.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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
#include "DlgSelect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CSelectDialog dialog

IMPLEMENT_DYNAMIC(CSelectDialog, CSkinDialog)

CSelectDialog::CSelectDialog(CWnd* pParent /*=NULL*/)
	: CSkinDialog(CSelectDialog::IDD, pParent),
	m_nData( 0 )
{
}

void CSelectDialog::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FILE_LIST, m_ListCtrl);
}

BEGIN_MESSAGE_MAP(CSelectDialog, CSkinDialog)
	ON_CBN_DROPDOWN(IDC_FILE_LIST, &CSelectDialog::OnCbnDropdownList)
END_MESSAGE_MAP()

// CSelectDialog message handlers

BOOL CSelectDialog::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CSelectDialog"), IDR_MAINFRAME );

	m_ListCtrl.SetExtendedUI();

	int select = 0;
	for ( POSITION pos = m_List.GetHeadPosition(); pos; )
	{
		const CItem it = m_List.GetNext( pos );
		const int index = m_ListCtrl.AddString( it.m_sItem );
		m_ListCtrl.SetItemData( index, it.m_nData );
		if ( it.m_nData == m_nData )
			select = index;
	}

	m_ListCtrl.SetCurSel( select );

	return FALSE;
}

void CSelectDialog::OnOK()
{
	m_nData = m_ListCtrl.GetItemData( m_ListCtrl.GetCurSel() );

	CSkinDialog::OnOK();
}

void CSelectDialog::OnCancel()
{
	m_nData = m_ListCtrl.GetItemData( m_ListCtrl.GetCurSel() );

	CSkinDialog::OnCancel();
}

void CSelectDialog::OnCbnDropdownList()
{
	RecalcDropWidth( &m_ListCtrl );
}
