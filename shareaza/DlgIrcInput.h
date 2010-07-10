//
// DlgIrcInput.h
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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

#pragma once

#include "DlgSkinDialog.h"
// CIrcInputDlg dialog

class CIrcInputDlg : public CSkinDialog
{
public:
	CIrcInputDlg(CWnd* pParent = NULL, int m_nCaptionIndex = 0, BOOL m_bKickOnly = FALSE);
	virtual ~CIrcInputDlg();

// Dialog Data
	enum { IDD = IDD_IRC_INPUTBOX };
	int			m_nCaptionIndex;
	BOOL		m_bKickOnly;
	CButton		m_wndPrompt;
	CEdit		m_wndAnswer;

// Overrides
protected:
	//{{AFX_VIRTUAL(CIrcInputDlg)
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CIrcInputDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG

public:
	void OnOK();
	CString	m_sAnswer;
};
