//
// DlgDonkeyServers.h
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

#if !defined(AFX_DLGDONKEYSERVERS_H__F0BCB926_4529_469D_B132_000122735D9F__INCLUDED_)
#define AFX_DLGDONKEYSERVERS_H__F0BCB926_4529_469D_B132_000122735D9F__INCLUDED_

#pragma once

#include "ThreadImpl.h"
#include "DlgSkinDialog.h"


class CDonkeyServersDlg :
	public CSkinDialog,
	public CThreadImpl
{
// Construction
public:
	CDonkeyServersDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDonkeyServersDlg();

// Dialog Data
public:
	//{{AFX_DATA(CDonkeyServersDlg)
	enum { IDD = IDD_DONKEY_SERVERS };
	CEdit	m_wndURL;
	CButton	m_wndOK;
	CProgressCtrl	m_wndProgress;
	CString	m_sURL;
	//}}AFX_DATA

// Attributes
public:
	HINTERNET	m_hInternet;

// Operations
protected:
	void			OnRun();

// Overrides
public:
	//{{AFX_VIRTUAL(CDonkeyServersDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CDonkeyServersDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnChangeURL();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_DLGDONKEYSERVERS_H__F0BCB926_4529_469D_B132_000122735D9F__INCLUDED_)
