//
// DlgNewSearch.h
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

#pragma once

#include "DlgSkinDialog.h"
#include "CtrlSchema.h"
#include "CtrlSchemaCombo.h"

class CQuerySearch;


class CNewSearchDlg : public CSkinDialog
{
public:
	CNewSearchDlg(CWnd* pParent = NULL, CQuerySearch* pSearch = NULL,
		BOOL bLocal = FALSE, BOOL bAgain = FALSE);

	enum { IDD = IDD_NEW_SEARCH };

	CQuerySearchPtr GetSearch() const
	{
		return m_pSearch;
	}

protected:
	CButton			m_wndCancel;
	CButton			m_wndOK;
	CSchemaCombo	m_wndSchemas;
	CComboBox		m_wndSearch;
	CSchemaCtrl		m_wndSchema;
	BOOL			m_bLocal;
	BOOL			m_bAgain;
	CQuerySearchPtr m_pSearch;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnSelChangeSchemas();
	afx_msg void OnCloseUpSchemas();
	afx_msg void OnChangeSearch();

	DECLARE_MESSAGE_MAP()
};

#define IDC_METADATA		100
