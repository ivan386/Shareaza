//
// PageSettingsBitTorrent.h
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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

#include "WndSettingsPage.h"
#include "CtrlIconButton.h"


class CBitTorrentSettingsPage : public CSettingsPage
{
	DECLARE_DYNCREATE(CBitTorrentSettingsPage)

public:
	CBitTorrentSettingsPage();

	enum { IDD = IDD_SETTINGS_BITTORRENT };

	virtual void OnOK();
	virtual BOOL OnSetActive();

protected:
	BOOL			m_bEnableToday;
	BOOL			m_bEnableAlways;
	BOOL			m_bEnableDHT;
	BOOL			m_bEndGame;
	CSpinButtonCtrl	m_wndLinksSpin;
	int				m_nLinks;
	CSpinButtonCtrl	m_wndDownloadsSpin;
	int				m_nDownloads;
	BOOL			m_bAutoClear;
	CEdit			m_wndClearPercentage;
	CSpinButtonCtrl	m_wndClearPercentageSpin;
	int				m_nClearPercentage;
	BOOL			m_bPrefBTSources;
	CIconButtonCtrl	m_wndTorrentPath;
	CString			m_sTracker;
	CIconButtonCtrl	m_wndMakerPath;
	CString			m_sMakerPath;
	CComboBoxPath	m_wndTorrentFolder;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	afx_msg void OnTorrentsAutoClear();
	afx_msg void OnTorrentsBrowse();
	afx_msg void OnMakerBrowse();

	DECLARE_MESSAGE_MAP()
};
