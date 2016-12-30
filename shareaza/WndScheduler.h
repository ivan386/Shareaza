//
// WndScheduler.h
//
// Copyright (c) Shareaza Development Team, 2002-2016.
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


class CSchedulerWnd : public CPanelWnd
{
	DECLARE_SERIAL(CSchedulerWnd)

public:
	CSchedulerWnd();

	void			Update(int nColumn = -1, BOOL bSort = TRUE);
	CString			GetItem(int nItem);

	virtual void	OnSkinChange();
	virtual BOOL	PreTranslateMessage(MSG* pMsg);

protected:
	CComPtr< ITaskScheduler > m_pScheduler;
	CCoolBarCtrl	m_wndToolBar;
	CListCtrl		m_wndList;
	CImageList		m_gdiImageList;
	CLiveListSizer	m_pSizer;
	DWORD			m_tLastUpdate;

	BOOL IsTaskActive(LPCWSTR szTaskName) const;

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnCustomDrawList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblClkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSortList(NMHDR* pNotifyStruct, LRESULT *pResult);
	afx_msg void OnUpdateSchedulerAdd(CCmdUI* pCmdUI);
	afx_msg void OnSchedulerAdd();
	afx_msg void OnUpdateSchedulerActivate(CCmdUI* pCmdUI);
	afx_msg void OnSchedulerActivate();
	afx_msg void OnUpdateSchedulerDeactivate(CCmdUI* pCmdUI);
	afx_msg void OnSchedulerDeactivate();
	afx_msg void OnUpdateSchedulerEdit(CCmdUI* pCmdUI);
	afx_msg void OnSchedulerEdit();
	afx_msg void OnUpdateSchedulerRemove(CCmdUI* pCmdUI);
	afx_msg void OnSchedulerRemove();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnUpdateSchedulerRemoveAll(CCmdUI* pCmdUI);
	afx_msg void OnSchedulerRemoveAll();

	DECLARE_MESSAGE_MAP()
};

#define IDC_SCHEDULE		100
