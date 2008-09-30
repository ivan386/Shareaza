//
// WndSystem.h
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

#include "WndPanel.h"
#include "CtrlText.h"


class CSystemWnd : public CPanelWnd
{
public:
	CSystemWnd();

	DECLARE_SERIAL(CSystemWnd)

protected:
	CTextCtrl	m_wndText;

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSystemClear();
	afx_msg void OnSystemCopy();
	afx_msg void OnDestroy();
	afx_msg void OnUpdateSystemVerboseError(CCmdUI* pCmdUI);
	afx_msg void OnSystemVerboseError();
	afx_msg void OnUpdateSystemVerboseWarning(CCmdUI* pCmdUI);
	afx_msg void OnSystemVerboseWarning();
	afx_msg void OnUpdateSystemVerboseNotice(CCmdUI* pCmdUI);
	afx_msg void OnSystemVerboseNotice();
	afx_msg void OnUpdateSystemVerboseInfo(CCmdUI* pCmdUI);
	afx_msg void OnSystemVerboseInfo();
	afx_msg void OnUpdateSystemVerboseDebug(CCmdUI* pCmdUI);
	afx_msg void OnSystemVerboseDebug();
	afx_msg void OnUpdateSystemTimestamp(CCmdUI* pCmdUI);
	afx_msg void OnSystemTimestamp();
	afx_msg void OnSystemTest();

	DECLARE_MESSAGE_MAP()
};
