//
// DlgTorrentAdd.h
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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

#include <boost/function.hpp>
#include "DlgSkinDialog.h"
//#include "BTInfo.h"
#include "LtHookTorrent.h"


class CTorrentAddDlg :
	public CSkinDialog,
	public CThreadImpl,
    public CWinDataExchangeEx<AddTorrentDialog>
{
	DECLARE_DYNAMIC(CTorrentAddDlg)

public:
	CTorrentAddDlg(wstring& sTempFolder, wstring& sCompleteFolder, bool& bUseTemp, bool& bStartPaused, bool& bManaged, LtHook::bit::allocations& nAllocationType);

	enum { IDD = IDD_TORRENT_ADD };

protected:
	CButton			m_wndOK;
	CButton			m_wndCancel;
	CString			m_sTempFolder;
	CString         m_sCompleteFolder;
	BOOL			m_bUseTemp;
	BOOL            m_bStartPaused;
	BOOL            m_bManaged
	LtHook::bit::allocations m_nAllocationType
	CIconButtonCtrl	m_wndCompletePath;
	CIconButtonCtrl	m_wndTempPath;
	CString			m_sMessage;
	int				m_nScaled;
	int				m_nOldScaled;

	void		OnRun();

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnOK();
	afx_msg void OnCompleteBrowse();
	afx_msg void OnTemporaryBrowse();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()
};
