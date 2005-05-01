//
// DlgURLExport.h
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

#if !defined(AFX_DLGURLEXPORT_H__1B69A614_F171_4EE4_925E_FEDBBAF13A0D__INCLUDED_)
#define AFX_DLGURLEXPORT_H__1B69A614_F171_4EE4_925E_FEDBBAF13A0D__INCLUDED_

#pragma once

#include "DlgSkinDialog.h"

class CLibraryFile;


class CURLExportDlg : public CSkinDialog
{
// Construction
public:
	CURLExportDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
public:
	//{{AFX_DATA(CURLExportDlg)
	enum { IDD = IDD_URL_EXPORT };
	CButton	m_wndSave;
	CButton	m_wndCopy;
	CProgressCtrl	m_wndProgress;
	CComboBox	m_wndToken;
	CComboBox	m_wndPreset;
	CEdit	m_wndFormat;
	CStatic	m_wndMessage;
	CString	m_sFormat;
	//}}AFX_DATA

	CPtrList	m_pFiles;
	void		AddFile(CLibraryFile* pFile);

// Overrides
public:
	//{{AFX_VIRTUAL(CURLExportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CURLExportDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnCloseUpUrlToken();
	afx_msg void OnCloseUpUrlPreset();
	afx_msg void OnSave();
	afx_msg void OnCopy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_DLGURLEXPORT_H__1B69A614_F171_4EE4_925E_FEDBBAF13A0D__INCLUDED_)
