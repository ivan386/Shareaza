//
// DlgFolderScan.h
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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

#if !defined(AFX_DLGFOLDERSCAN_H__2FB9F048_44EC_4306_8023_46244F674629__INCLUDED_)
#define AFX_DLGFOLDERSCAN_H__2FB9F048_44EC_4306_8023_46244F674629__INCLUDED_

#pragma once

#include "DlgSkinDialog.h"


class CFolderScanDlg : public CSkinDialog
{
// Construction
public:
	CFolderScanDlg(CWnd* pParent = NULL);
	virtual ~CFolderScanDlg();

// Dialog Data
public:
	//{{AFX_DATA(CFolderScanDlg)
	enum { IDD = IDD_FOLDER_SCAN };
	CStatic	m_wndVolume;
	CStatic	m_wndFiles;
	CStatic	m_wndFile;
	//}}AFX_DATA
	
// Operations
public:
	static void Update(LPCTSTR pszName, DWORD nVolume, BOOL bLock);
	void	InstanceUpdate(LPCTSTR pszName, DWORD nVolume);

// Data
protected:
	static	CFolderScanDlg*	m_pDialog;
	DWORD	m_nCookie;
	DWORD	m_nFiles;
	DWORD	m_nVolume;
	DWORD	m_tLastUpdate;
	BOOL	m_bActive;
	
// Overrides
public:
	//{{AFX_VIRTUAL(CFolderScanDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CFolderScanDlg)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_DLGFOLDERSCAN_H__2FB9F048_44EC_4306_8023_46244F674629__INCLUDED_)
