//
// PageSettingsRemote.h
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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


class CRemoteSettingsPage : public CSettingsPage
{
	DECLARE_DYNAMIC(CRemoteSettingsPage)

public:
	CRemoteSettingsPage();
	virtual ~CRemoteSettingsPage();

	enum { IDD = IDD_SETTINGS_REMOTE };

protected:
	BOOL		m_bEnable;
	CStatic		m_wndURL;
	CEdit		m_wndUsername;
	CString		m_sUsername;
	CEdit		m_wndPassword;
	CString		m_sPassword;
	BOOL		m_bOldEnable;
	CString		m_sOldUsername;
	CString		m_sOldPassword;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnSkinChange();

	afx_msg void OnNewPassword();
	afx_msg void OnBnClickedRemoteEnable();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};
