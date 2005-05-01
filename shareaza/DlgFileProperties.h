//
// DlgFileProperties.h
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

#if !defined(AFX_DLGFILEPROPERTIES_H__AAD90EC0_F939_459D_90EF_F416A1E72D08__INCLUDED_)
#define AFX_DLGFILEPROPERTIES_H__AAD90EC0_F939_459D_90EF_F416A1E72D08__INCLUDED_

#pragma once

#include "DlgSkinDialog.h"
#include "CtrlSchema.h"
#include "CtrlSchemaCombo.h"


class CFilePropertiesDlg : public CSkinDialog
{
// Construction
public:
	CFilePropertiesDlg(CWnd* pParent = NULL, DWORD nIndex = 0);

	DECLARE_DYNAMIC(CFilePropertiesDlg)

// Dialog Data
public:
	//{{AFX_DATA(CFilePropertiesDlg)
	enum { IDD = IDD_FILE_PROPERTIES };
	CStatic	m_wndHash;
	CStatic	m_wndIcon;
	CButton	m_wndCancel;
	CButton	m_wndOK;
	CSchemaCombo	m_wndSchemas;
	CString	m_sName;
	CString	m_sSize;
	CString	m_sType;
	CString	m_sPath;
	CString	m_sIndex;
	CString	m_sSHA1;
	CString	m_sTiger;
	//}}AFX_DATA

// Attributes
protected:
	CSchemaCtrl	m_wndSchema;
	DWORD		m_nIndex;
	BOOL		m_bHexHash;
	int			m_nWidth;

// Operations
public:
	void	Update();

// Overrides
public:
	//{{AFX_VIRTUAL(CFilePropertiesDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CFilePropertiesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnSelChangeSchemas();
	virtual void OnOK();
	afx_msg void OnDestroy();
	afx_msg void OnCloseUpSchemas();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#define IDC_METADATA		100

#endif // !defined(AFX_DLGFILEPROPERTIES_H__AAD90EC0_F939_459D_90EF_F416A1E72D08__INCLUDED_)
