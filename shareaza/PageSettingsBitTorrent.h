//
// PageSettingsBitTorrent.h
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

#if !defined(AFX_PAGESETTINGSBITTORRENT_H__7A9C2316_0CF6_4251_9BD2_C56B9DA5AD89__INCLUDED_)
#define AFX_PAGESETTINGSBITTORRENT_H__7A9C2316_0CF6_4251_9BD2_C56B9DA5AD89__INCLUDED_

#pragma once

#include "WndSettingsPage.h"
#include "CtrlIconButton.h"


class CBitTorrentSettingsPage : public CSettingsPage
{
// Construction
public:
	CBitTorrentSettingsPage();
	virtual ~CBitTorrentSettingsPage();

	DECLARE_DYNCREATE(CBitTorrentSettingsPage)

// Dialog Data
public:
	//{{AFX_DATA(CBitTorrentSettingsPage)
	enum { IDD = IDD_SETTINGS_BITTORRENT };
	BOOL	m_bTorrentInterface;
	BOOL	m_bEndGame;
	CSpinButtonCtrl	m_wndLinksSpin;
	int		m_nLinks;
	CSpinButtonCtrl	m_wndDownloadsSpin;
	int		m_nDownloads;
	CIconButtonCtrl	m_wndTorrentPath;
	CString	m_sTorrentPath;
	CString	m_sTracker;
	CIconButtonCtrl	m_wndMakerPath;
	CString	m_sMakerPath;
	//}}AFX_DATA

// Overrides
public:
	//{{AFX_VIRTUAL(CBitTorrentSettingsPage)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CLibrarySettingsPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnTorrentsBrowse();
	afx_msg void OnMakerBrowse();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnDeltaposTorrentlinksSpin2(NMHDR *pNMHDR, LRESULT *pResult);
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_PAGESETTINGSBITTORRENT_H__7A9C2316_0CF6_4251_9BD2_C56B9DA5AD89__INCLUDED_)
