//
// DlgDeleteFile.h
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

#pragma once

#include "DlgSkinDialog.h"
#include "afxwin.h"

class CDownload;
class CLibraryFile;


class CDeleteFileDlg : public CSkinDialog
{
// Construction
public:
	CDeleteFileDlg(CWnd* pParent = NULL);
	virtual ~CDeleteFileDlg();

	DECLARE_DYNAMIC(CDeleteFileDlg)
	enum { IDD = IDD_DELETE_FILE };

// Members
public:
	BOOL m_bAll;
	CStatic m_wndName;
	CString m_sComments;
	CString m_sName;
	BOOL m_nRateValue;
	CButton m_wndOK;
	CEdit m_wndComments;
	CStatic m_wndPrompt;
	CButton m_wndAll;

// Operations
public:
	void	Apply(CLibraryFile* pFile);
	void	Create(CDownload* pDownload, BOOL bShare);

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
    virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDeleteAll();
public:
	afx_msg void OnBnClickedRateValue();
};
