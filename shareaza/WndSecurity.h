//
// WndSecurity.h
//
// Copyright (c) Shareaza Development Team, 2002-2011.
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

class CSecureRule;


class CSecurityWnd : public CPanelWnd
{
	DECLARE_SERIAL(CSecurityWnd)

public:
	CSecurityWnd();
	virtual ~CSecurityWnd();

protected:
	CCoolBarCtrl	m_wndToolBar;
	CListCtrl		m_wndList;
	CImageList		m_gdiImageList;
	CLiveListSizer	m_pSizer;
	DWORD			tLastUpdate;

	void			Update(int nColumn = -1, BOOL bSort = TRUE);
	CSecureRule*	GetItem(int nItem);
	virtual void	OnSkinChange();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnCustomDrawList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblClkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSortList(NMHDR* pNotifyStruct, LRESULT *pResult);
	afx_msg void OnUpdateSecurityEdit(CCmdUI* pCmdUI);
	afx_msg void OnSecurityEdit();
	afx_msg void OnUpdateSecurityReset(CCmdUI* pCmdUI);
	afx_msg void OnSecurityReset();
	afx_msg void OnUpdateSecurityRemove(CCmdUI* pCmdUI);
	afx_msg void OnSecurityRemove();
	afx_msg void OnSecurityAdd();
	afx_msg void OnUpdateSecurityPolicyAccept(CCmdUI* pCmdUI);
	afx_msg void OnSecurityPolicyAccept();
	afx_msg void OnUpdateSecurityPolicyDeny(CCmdUI* pCmdUI);
	afx_msg void OnSecurityPolicyDeny();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnUpdateSecurityMoveUp(CCmdUI* pCmdUI);
	afx_msg void OnSecurityMoveUp();
	afx_msg void OnUpdateSecurityMoveDown(CCmdUI* pCmdUI);
	afx_msg void OnSecurityMoveDown();
	afx_msg void OnUpdateSecurityExport(CCmdUI* pCmdUI);
	afx_msg void OnSecurityExport();
	afx_msg void OnSecurityImport();

	DECLARE_MESSAGE_MAP()
};

#define IDC_RULES		100
