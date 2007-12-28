//
// PageTorrentTrackers.h
//
// Copyright (c) Shareaza Development Team, 2002-2006.
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

#if !defined(AFX_PAGETORRENTTRACKERS_H__3FE33E_A574_484A_88EB_4AD8K2BE64__INCLUDED_)
#define AFX_PAGETORRENTTRACKERS_H__3FE33E_A574_484A_88EB_4AD8K2BE64__INCLUDED_

#pragma once

#include "DlgTorrentInfoPage.h"
#include "HttpRequest.h"

class CTorrentTrackersPage : public CTorrentInfoPage
{
// Construction
public:
	CTorrentTrackersPage();
	virtual ~CTorrentTrackersPage();

	DECLARE_DYNCREATE(CTorrentTrackersPage)

// Dialog Data
public:
	//{{AFX_DATA(CTorrentTrackersPage)
	enum { IDD = IDD_TORRENT_TRACKERS };
	CString			m_sName;
	CString			m_sTracker;
	CString			m_sEscapedPeerID;

	CButton			m_wndRefresh;
	CEdit			m_wndComplete;
	CEdit			m_wndIncomplete;
	CComboBox		m_wndTrackerMode;

	CListCtrl		m_wndTrackers;
	//}}AFX_DATA

// Attributes
public:
	CHttpRequest	m_pRequest;
	HANDLE			m_hThread;
	int				m_nComplete;
	int				m_nIncomplete;

// Thread
protected:
	static UINT		ThreadStart(LPVOID pParam);
	void			OnRun();
	BOOL			OnTree(CBENode* pNode);

// URL escaper
protected:
	CString			Escape(const CString& str);

// Overrides
public:
	//{{AFX_VIRTUAL(CTorrentTrackersPage)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CTorrentTrackersPage)
	virtual BOOL	OnInitDialog();
	afx_msg void	OnTorrentRefresh();
	afx_msg void	OnTimer(UINT_PTR nIDEvent);
	afx_msg void	OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_PAGETORRENTTRACKERS_H__3FE33E_A574_484A_88EB_4AD8K2BE64__INCLUDED_)
