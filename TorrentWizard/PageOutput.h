//
// PageOutput.h
//
// Copyright (c) Shareaza Development Team, 2007.
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

class COutputPage : public CWizardPage
{
// Construction
public:
	COutputPage();

	DECLARE_DYNCREATE(COutputPage)

// Dialog Data
public:
	enum { IDD = IDD_OUTPUT_PAGE };
	CEdit		m_wndName;
	CComboBox	m_wndFolders;
	CString		m_sFolder;
	CString		m_sName;
	BOOL		m_bAutoPieces;
	CComboBox	m_wndPieceSize;
	int			m_nPieceIndex;
	BOOL		m_bSHA1;
	BOOL		m_bED2K;
	BOOL		m_bMD5;

// Overrides
protected:
	virtual void OnReset();
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnClearFolders();
	afx_msg void OnBrowseFolder();
	afx_msg void OnClickedAutoPieceSize();
	afx_msg void OnCloseupPieceSize();

	DECLARE_MESSAGE_MAP()
};
