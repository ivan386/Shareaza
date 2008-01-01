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

#if !defined(DLGDOWNLOADGROUP_H)
#define DLGDOWNLOADGROUP_H

#pragma once

#include "DlgSkinDialog.h"
#include "CtrlIconButton.h"

class CDownloadGroup;


class CDownloadGroupDlg : public CSkinDialog
{
// Construction
public:
	CDownloadGroupDlg(CDownloadGroup* pGroup, CWnd* pParent = NULL);

// Dialog Data
public:
	enum { IDD = IDD_DOWNLOAD_GROUP };
	CIconButtonCtrl	m_wndBrowse;
	CListCtrl		m_wndImages;
	CEdit			m_wndFolder;
	CButton			m_wndFilterAdd;
	CButton			m_wndFilterRemove;
	CComboBox		m_wndFilterList;
	CString			m_sName;
	CString			m_sFolder;

	CDownloadGroup*	m_pGroup;

protected:
	bool			m_bInitializing;

// Overrides
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnFilterAdd();
	afx_msg void OnFilterRemove();
	afx_msg void OnEditChangeFilterList();
	afx_msg void OnSelChangeFilterList();
	virtual void OnOK();
	afx_msg void OnBrowse();
	afx_msg void OnLvnItemchangingIconList(NMHDR *pNMHDR, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()
};

#endif // !defined(DLGDOWNLOADGROUP_H)
