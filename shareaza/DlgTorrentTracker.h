//
// DlgTorrentTracker.h
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

#if !defined(AFX_DLGTORRENTTRACKER_H__543DC4E4_C8F6_42AC_99D5_56F58C12ECB3__INCLUDED_)
#define AFX_DLGTORRENTTRACKER_H__543DC4E4_C8F6_42AC_99D5_56F58C12ECB3__INCLUDED_

#pragma once

#include "BTInfo.h"
#include "DlgSkinDialog.h"
#include "HttpRequest.h"


class CTorrentTrackerDlg : public CSkinDialog
{
// Construction
public:
	CTorrentTrackerDlg(CBTInfo* pInfo, CWnd* pParent = NULL);

// Dialog Data
public:
	//{{AFX_DATA(CTorrentTrackerDlg)
	enum { IDD = IDD_TORRENT_TRACKER };
	CComboBox	m_wndView;
	CButton	m_wndRefresh;
	CListCtrl	m_wndFiles;
	CEdit	m_wndComplete;
	CEdit	m_wndIncomplete;
	CString	m_sName;
	CString	m_sTracker;
	//}}AFX_DATA
	
// Attributes
public:
	CBTInfo			m_pInfo;
	CHttpRequest	m_pRequest;
	HANDLE			m_hThread;
	int				m_nComplete;
	int				m_nIncomplete;
	
// Thread
protected:
	static UINT	ThreadStart(LPVOID pParam);
	void		OnRun();
	BOOL		OnTree(CBENode* pNode);
	
// Overrides
public:
	//{{AFX_VIRTUAL(CTorrentTrackerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CTorrentTrackerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangeTorrentView();
	afx_msg void OnTorrentRefresh();
	afx_msg void OnTimer(UINT nIDEvent);
	virtual void OnOK();
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_DLGTORRENTTRACKER_H__543DC4E4_C8F6_42AC_99D5_56F58C12ECB3__INCLUDED_)
