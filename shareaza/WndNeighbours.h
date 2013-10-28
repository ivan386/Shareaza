//
// WndNeighbours.h
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

#include "WndPanel.h"
#include "CtrlTipList.h"

class CNeighbour;


class CNeighboursWnd : public CPanelWnd
{
	DECLARE_SERIAL(CNeighboursWnd)

public:
	CNeighboursWnd();

protected:
	CCoolBarCtrl		m_wndToolBar;
	CTipListCtrl		m_wndList;
	CNeighbourTipCtrl	m_wndTip;
	CImageList			m_gdiImageList;
	CLiveListSizer		m_pSizer;
	DWORD				m_tLastUpdate;

	void		 Update();
	CNeighbour*	 GetItem(int nItem);
	UINT		 GetSelectedCount() const;
	void		 OpenPacketWnd(BOOL bIncoming, BOOL bOutgoing);
	void		 DrawEmptyMessage(CDC* pDC);
	virtual void OnSkinChange();

	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSortList(NMHDR* pNotifyStruct, LRESULT *pResult);
	afx_msg void OnCustomDrawList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnUpdateNeighboursDisconnect(CCmdUI* pCmdUI);
	afx_msg void OnNeighboursDisconnect();
	afx_msg void OnUpdateNeighboursViewAll(CCmdUI* pCmdUI);
	afx_msg void OnNeighboursViewAll();
	afx_msg void OnUpdateNeighboursViewIncoming(CCmdUI* pCmdUI);
	afx_msg void OnNeighboursViewIncoming();
	afx_msg void OnUpdateNeighboursViewOutgoing(CCmdUI* pCmdUI);
	afx_msg void OnNeighboursViewOutgoing();
	afx_msg void OnDestroy();
	afx_msg void OnUpdateNeighboursChat(CCmdUI* pCmdUI);
	afx_msg void OnNeighboursChat();
	afx_msg void OnUpdateSecurityBan(CCmdUI* pCmdUI);
	afx_msg void OnSecurityBan();
	afx_msg void OnUpdateBrowseLaunch(CCmdUI* pCmdUI);
	afx_msg void OnBrowseLaunch();
	afx_msg void OnUpdateNeighboursCopy(CCmdUI* pCmdUI);
	afx_msg void OnNeighboursCopy();
	afx_msg void OnNeighboursSettings();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);

	DECLARE_MESSAGE_MAP()
};

#define IDC_NEIGHBOURS	100
