//
// DlgFilterSearch.h
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

#if !defined(AFX_DLGFILTERSEARCH_H__779A07CC_0E13_4720_8337_ED05A9790295__INCLUDED_)
#define AFX_DLGFILTERSEARCH_H__779A07CC_0E13_4720_8337_ED05A9790295__INCLUDED_

#pragma once

#include "DlgSkinDialog.h"

class CMatchList;
class CResultFilters;

class CFilterSearchDlg : public CSkinDialog
{
// Construction
public:
	CFilterSearchDlg(CWnd* pParent = NULL, CMatchList* pList = NULL);

// Dialog Data
public:
	//{{AFX_DATA(CFilterSearchDlg)
	enum { IDD = IDD_FILTER_SEARCH };
	CSpinButtonCtrl	m_wndSources;
	CString	m_sFilter;
	BOOL	m_bHideBusy;
	BOOL	m_bHideLocal;
	BOOL	m_bHidePush;
	BOOL	m_bHideReject;
	BOOL	m_bHideUnstable;
	BOOL	m_bHideBogus;
	BOOL	m_bHideDRM;
	BOOL	m_bHideAdult;
	BOOL	m_bHideSuspicious;
	int		m_nSources;
	CString	m_sMaxSize;
	CString	m_sMinSize;
	BOOL    m_bDefault;
	//}}AFX_DATA

	CMatchList*	m_pMatches;
	CResultFilters * m_pResultFilters;

// Overrides
protected:
	//{{AFX_VIRTUAL(CFilterSearchDlg)
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CFilterSearchDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedSaveFilter();
	afx_msg void OnBnClickedDeleteFilter();
	afx_msg void OnBnClickedSetDefaultFilter();
	afx_msg void OnCbnSelChangeFilters();
	afx_msg void OnEnKillFocusMinMaxSize();

private:
	void UpdateFields();
	void UpdateList();
	CComboBox m_Filters;
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_DLGFILTERSEARCH_H__779A07CC_0E13_4720_8337_ED05A9790295__INCLUDED_)
