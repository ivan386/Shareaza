//
// DlgDownloadGroup.h
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

#pragma once

#include "DlgSkinDialog.h"
#include "CtrlIconButton.h"
#include "CtrlSchemaCombo.h"

class CDownloadGroup;


class CDownloadGroupDlg : public CSkinDialog
{
public:
	CDownloadGroupDlg(CDownloadGroup* pGroup, CWnd* pParent = NULL);

	enum { IDD = IDD_DOWNLOAD_GROUP };

protected:
	CIconButtonCtrl	m_wndBrowse;
	CIconButtonCtrl	m_wndCancel;
	CSchemaCombo	m_wndSchemas;
	CEdit			m_wndFolder;
	CButton			m_wndFilterAdd;
	CButton			m_wndFilterRemove;
	CComboBox		m_wndFilterList;
	CString			m_sName;
	CString			m_sFolder;
	BOOL			m_bTorrent;
	CDownloadGroup*	m_pGroup;
	CString			m_sOldSchemaURI;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	afx_msg void OnFilterAdd();
	afx_msg void OnFilterRemove();
	afx_msg void OnEditChangeFilterList();
	afx_msg void OnSelChangeFilterList();
	afx_msg void OnBrowse();
	afx_msg void OnCbnCloseupSchemas();
	afx_msg void OnBnClickedDownloadDefault();

	DECLARE_MESSAGE_MAP()
};
