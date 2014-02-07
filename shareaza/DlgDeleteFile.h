//
// DlgDeleteFile.h
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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
	int m_nRateValue;

private:
	CString m_sOriginalComments;
	int m_nOriginalRating;
	CButton m_wndOK;
	CEdit m_wndComments;
	CButton m_wndAll;
	CComboBox m_wndOptions;
	int m_nOption;
	CComboBox m_wndRating;
	BOOL m_bCreateGhost;
	CStatic m_wndPrompt;

// Operations
public:
	void	Apply(CLibraryFile* pFile);
	void	Apply(CShareazaFile* pSrc, BOOL bShare);

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDeleteAll();
	afx_msg void OnCbnChangeOptions();
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnCbnChangeGhostRating();
	afx_msg void OnChangeComments();
	afx_msg void OnClickedCreateGhost();
	afx_msg void OnCbnDropdownOptions();

	DECLARE_MESSAGE_MAP()
};
