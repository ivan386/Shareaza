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

#pragma once

#include "ThreadImpl.h"
#include "PagePropertyAdv.h"
#include "HttpRequest.h"

class CTorrentTrackersPage :
	public CPropertyPageAdv,
	public CThreadImpl
{
public:
	CTorrentTrackersPage();
	virtual ~CTorrentTrackersPage();

	DECLARE_DYNCREATE(CTorrentTrackersPage)

	enum { IDD = IDD_TORRENT_TRACKERS };

protected:
	CDownload*		m_pDownload;
	CString			m_sName;
	CString			m_sTracker;
	CString			m_sEscapedPeerID;
	CButton			m_wndRefresh;
	CEdit			m_wndComplete;
	CEdit			m_wndIncomplete;
	CComboBox		m_wndTrackerMode;
	CListCtrl		m_wndTrackers;
	CHttpRequest	m_pRequest;
	int				m_nComplete;
	int				m_nIncomplete;

	void			OnRun();
	BOOL			OnTree(CBENode* pNode);
	CString			Escape(const CString& str);

	virtual void	DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL	OnInitDialog();
	virtual void	OnOK();
	afx_msg void	OnTorrentRefresh();
	afx_msg void	OnTimer(UINT_PTR nIDEvent);
	afx_msg void	OnDestroy();

	DECLARE_MESSAGE_MAP()
};
