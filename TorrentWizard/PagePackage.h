//
// PagePackage.h
//
// Copyright © Shareaza Development Team, 2002-2009.
// This file is part of Shareaza Torrent Wizard (shareaza.sourceforge.net).
//
// Shareaza Torrent Wizard is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Torrent Wizard is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Shareaza; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#if !defined(AFX_PAGEPACKAGE_H__FEF966D1_AA75_4842_9452_56F3DDD41DCB__INCLUDED_)
#define AFX_PAGEPACKAGE_H__FEF966D1_AA75_4842_9452_56F3DDD41DCB__INCLUDED_

#pragma once

#include "WizardSheet.h"


class CPackagePage : public CWizardPage
{
// Construction
public:
	CPackagePage();
	virtual ~CPackagePage();
	
	DECLARE_DYNCREATE(CPackagePage)
	
// Dialog Data
public:
	//{{AFX_DATA(CPackagePage)
	enum { IDD = IDD_PACKAGE_PAGE };
	CButton	m_wndRemove;
	CListCtrl	m_wndList;
	//}}AFX_DATA

	HIMAGELIST	m_hImageList;

// Operations
protected:
	void	AddFile(LPCTSTR pszFile);
	void	AddFolder(LPCTSTR pszPath, int nRecursive);
	
// Overrides
public:
	//{{AFX_VIRTUAL(CPackagePage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
	virtual void OnReset();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CPackagePage)
	virtual BOOL OnInitDialog();
	afx_msg void OnItemChangedFileList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAddFolder();
	afx_msg void OnAddFile();
	afx_msg void OnRemoveFile();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_PAGEPACKAGE_H__FEF966D1_AA75_4842_9452_56F3DDD41DCB__INCLUDED_)
