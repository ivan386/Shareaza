//
// SaveFilterAsDlg.cpp
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
//
// Author : roo_koo_too@yahoo.com
//
#include "stdafx.h"
#include "Shareaza.h"
#include "DlgSaveFilterAs.h"

// CSaveFilterAsDlg dialog

CSaveFilterAsDlg::CSaveFilterAsDlg(CWnd* pParent /*=NULL*/)
	: CSkinDialog(CSaveFilterAsDlg::IDD, pParent)
	, m_sName(_T(""))
{
}

CSaveFilterAsDlg::~CSaveFilterAsDlg()
{
}

void CSaveFilterAsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_NAME, m_sName);
}


BEGIN_MESSAGE_MAP(CSaveFilterAsDlg, CSkinDialog)
	ON_EN_CHANGE(IDC_NAME, OnEnChangeName)
END_MESSAGE_MAP()


// CSaveFilterAsDlg message handlers

BOOL CSaveFilterAsDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CSaveFilterAsDlg"), IDR_SEARCHFRAME );

	return TRUE;
}

void CSaveFilterAsDlg::OnEnChangeName()
{
	UpdateData(TRUE);

	GetDlgItem(IDOK)->EnableWindow(!m_sName.IsEmpty());
}
