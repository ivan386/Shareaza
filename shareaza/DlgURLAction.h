//
// DlgURLAction.h
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

#if !defined(AFX_DLGURLACTION_H__8BAFF36B_C812_4B29_8F29_9FBF1D89F3AE__INCLUDED_)
#define AFX_DLGURLACTION_H__8BAFF36B_C812_4B29_8F29_9FBF1D89F3AE__INCLUDED_

#pragma once

#include "DlgSkinDialog.h"

class CShareazaURL;


class CURLActionDlg : public CSkinDialog
{
// Construction
public:
	CURLActionDlg(CWnd* pParent = NULL, CShareazaURL* pURL = NULL, BOOL bMultiple = FALSE);
	virtual ~CURLActionDlg();

// Dialog Data
public:
	//{{AFX_DATA(CURLActionDlg)
	enum { IDD = IDD_URL_ACTION };
	CStatic	m_wndMessage4;
	CStatic	m_wndMessage3;
	CButton	m_wndNewWindow;
	CButton	m_wndCancel;
	CStatic	m_wndMessage2;
	CStatic	m_wndMessage1;
	CButton	m_wndSearch;
	CButton	m_wndDownload;
	CString	m_sNameTitle;
	CString	m_sNameValue;
	CString	m_sHashTitle;
	CString	m_sHashValue;
	BOOL	m_bNewWindow;
	BOOL	m_bAlwaysOpen;
	//}}AFX_DATA

// Operations
public:
	void	AddURL(CShareazaURL* pURL);
protected:
	void	Update();

// Attributes
protected:
	CPtrList	m_pURLs;
	BOOL		m_bMultiple;

// Overrides
public:
	//{{AFX_VIRTUAL(CURLActionDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CURLActionDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnUrlDownload();
	afx_msg void OnUrlSearch();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_DLGURLACTION_H__8BAFF36B_C812_4B29_8F29_9FBF1D89F3AE__INCLUDED_)
