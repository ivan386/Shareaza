//
// DlgURLCopy.h
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

#if !defined(AFX_DLGURLCOPY_H__CFF49DD5_9C74_4024_B7C0_551F4B05DFB6__INCLUDED_)
#define AFX_DLGURLCOPY_H__CFF49DD5_9C74_4024_B7C0_551F4B05DFB6__INCLUDED_

#pragma once

#include "Hashes.h"
#include "DlgSkinDialog.h"


class CURLCopyDlg : public CSkinDialog
{
// Construction
public:
	CURLCopyDlg(CWnd* pParent = NULL);

	DECLARE_DYNAMIC(CURLCopyDlg)

// Dialog Data
public:
	//{{AFX_DATA(CURLCopyDlg)
	enum { IDD = IDD_URL_COPY };
	CButton	m_wndIncludeSelf;
	CStatic	m_wndMessage;
	CString	m_sHost;
	CString	m_sMagnet;
	CString	m_sED2K;
	//}}AFX_DATA

// Attributes
public:
	CString		m_sName;
	CManagedSHA1	m_oSHA1;
	CManagedTiger	m_oTiger;
	CManagedED2K	m_oED2K;
	BOOL		m_bSize;
	QWORD		m_nSize;

	static BOOL	SetClipboardText(CString& strText);

// Overrides
public:
	//{{AFX_VIRTUAL(CURLCopyDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CURLCopyDlg)
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnIncludeSelf();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_DLGURLCOPY_H__CFF49DD5_9C74_4024_B7C0_551F4B05DFB6__INCLUDED_)
