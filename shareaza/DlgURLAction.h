//
// DlgURLAction.h
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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

class CShareazaURL;


class CURLActionDlg : public CSkinDialog
{
	DECLARE_DYNAMIC(CURLActionDlg)

public:
	CURLActionDlg(CShareazaURL* pURL);
	virtual ~CURLActionDlg();

	enum { IDD = IDD_URL_ACTION };

protected:
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
	CShareazaURL* m_pURL;

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void PostNcDestroy();

	afx_msg void OnUrlDownload();
	afx_msg void OnUrlSearch();

	DECLARE_MESSAGE_MAP()
};
