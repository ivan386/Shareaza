//
// DlgConnectTo.h
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

#if !defined(AFX_DLGCONNECTTO_H__08D02B14_977A_4C9C_AEAE_8FBBE0E868B8__INCLUDED_)
#define AFX_DLGCONNECTTO_H__08D02B14_977A_4C9C_AEAE_8FBBE0E868B8__INCLUDED_

#pragma once

#include "DlgSkinDialog.h"


class CConnectToDlg : public CSkinDialog
{
// Construction
public:
	CConnectToDlg(CWnd* pParent = NULL, BOOL bBrowseHost = FALSE);

// Dialog Data
public:
	//{{AFX_DATA(CConnectToDlg)
	enum { IDD = IDD_CONNECT_TO };
	CButton	m_wndAdvanced;
	CComboBox	m_wndProtocol;
	CButton	m_wndUltrapeer;
	CButton	m_wndPrompt;
	CEdit	m_wndPort;
	CComboBox	m_wndHost;
	CString	m_sHost;
	BOOL	m_bNoUltraPeer;
	int		m_nPort;
	int		m_nProtocol;
	//}}AFX_DATA

	CImageList	m_pImages;
	BOOL		m_bBrowseHost;

	void		LoadItem(int nItem);

// Overrides
public:
	//{{AFX_VIRTUAL(CConnectToDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CConnectToDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelChangeConnectHost();
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnCloseUpConnectProtocol();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_DLGCONNECTTO_H__08D02B14_977A_4C9C_AEAE_8FBBE0E868B8__INCLUDED_)
