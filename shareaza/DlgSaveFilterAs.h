//
// SaveFilterAsDlg.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
//
// Author : roo_koo_too@yahoo.com
//
#pragma once

#include "DlgSkinDialog.h"
// CSaveFilterAsDlg dialog

class CSaveFilterAsDlg : public CSkinDialog
{
public:
	CSaveFilterAsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSaveFilterAsDlg();

// Dialog Data
	enum { IDD = IDD_FILTER_SAVE_AS };

// Implementation
protected:
	//{{AFX_MSG(CSaveFilterAsDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeName();
	void OnOK();
	// The current filter name
	CString m_sName;
};
