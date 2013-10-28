//
// KeywordTestDlg.h
//
// Copyright (c) Shareaza Development Team, 2011.
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

class CKeywordTestDlg : public CDialog
{
public:
	CKeywordTestDlg(CWnd* pParent = NULL);	// standard constructor
	enum { IDD = IDD_KEYWORDTEST_DIALOG };

protected:
	CString m_sInput;
	CString m_sSplitted;
	CListBox m_pResults;
	BOOL m_bExp;
	HICON m_hIcon;

	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnEnChangeInput();
	afx_msg void OnBnClickedExp();

	DECLARE_MESSAGE_MAP()
};
