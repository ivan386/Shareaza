//
// PagePackage.h
//
// Copyright (c) Shareaza Development Team, 2007-2011.
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

#pragma once

#include "WizardSheet.h"


class CPackagePage : public CWizardPage
{
public:
	CPackagePage();
	
	DECLARE_DYNCREATE(CPackagePage)
	
	enum { IDD = IDD_PACKAGE_PAGE };

	CButton		m_wndRemove;
	CListCtrl	m_wndList;
	HIMAGELIST	m_hImageList;

	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
	virtual void OnReset();

protected:
	void	AddFile(LPCTSTR pszFile);
	void	AddFolder(LPCTSTR pszPath, int nRecursive);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	afx_msg void OnItemChangedFileList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAddFolder();
	afx_msg void OnAddFile();
	afx_msg void OnRemoveFile();

	DECLARE_MESSAGE_MAP()
};
