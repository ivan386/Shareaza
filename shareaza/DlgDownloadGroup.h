//
// DlgDownloadGroup.h
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

#if !defined(AFX_DLGDOWNLOADGROUP_H__DFB60CF9_6E83_4C66_9A1A_EE60C40391BE__INCLUDED_)
#define AFX_DLGDOWNLOADGROUP_H__DFB60CF9_6E83_4C66_9A1A_EE60C40391BE__INCLUDED_

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
	//{{AFX_DATA(CDownloadGroupDlg)
	enum { IDD = IDD_DOWNLOAD_GROUP };
	CIconButtonCtrl	m_wndBrowse;
	CListCtrl	m_wndImages;
	CEdit	m_wndFolder;
	CButton	m_wndFilterAdd;
	CButton	m_wndFilterRemove;
	CComboBox	m_wndFilterList;
	CString	m_sName;
	CString	m_sFolder;
	//}}AFX_DATA

	CDownloadGroup*	m_pGroup;

// Overrides
public:
	//{{AFX_VIRTUAL(CDownloadGroupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CDownloadGroupDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnFilterAdd();
	afx_msg void OnFilterRemove();
	afx_msg void OnEditChangeFilterList();
	afx_msg void OnSelChangeFilterList();
	virtual void OnOK();
	afx_msg void OnBrowse();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_DLGDOWNLOADGROUP_H__DFB60CF9_6E83_4C66_9A1A_EE60C40391BE__INCLUDED_)
