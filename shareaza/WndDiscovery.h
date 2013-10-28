//
// WndDiscovery.h
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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

class CDiscoveryService;


class CDiscoveryWnd : public CPanelWnd
{
	DECLARE_SERIAL(CDiscoveryWnd)

public:
	CDiscoveryWnd();

protected:
	CLiveListCtrl	m_wndList;
	CImageList		m_gdiImageList;
	CLiveListSizer	m_pSizer;
	BOOL			m_bShowGnutella;
	BOOL			m_bShowWebCache;
	BOOL			m_bShowServerMet;
	BOOL			m_bShowBlocked;

	void			Update();
	CDiscoveryService* GetItem(int nItem);
	virtual void	OnSkinChange();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDblClkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSortList(NMHDR* pNotifyStruct, LRESULT *pResult);
	afx_msg void OnUpdateDiscoveryQuery(CCmdUI* pCmdUI);
	afx_msg void OnDiscoveryQuery();
	afx_msg void OnUpdateDiscoveryRemove(CCmdUI* pCmdUI);
	afx_msg void OnDiscoveryRemove();
	afx_msg void OnDiscoveryAdd();
	afx_msg void OnDiscoveryEdit();
	afx_msg void OnUpdateDiscoveryEdit(CCmdUI* pCmdUI);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnUpdateDiscoveryGnutella(CCmdUI* pCmdUI);
	afx_msg void OnDiscoveryGnutella();
	afx_msg void OnUpdateDiscoveryWebcache(CCmdUI* pCmdUI);
	afx_msg void OnDiscoveryWebcache();
	afx_msg void OnUpdateDiscoveryServerMet(CCmdUI* pCmdUI);
	afx_msg void OnDiscoveryServerMet();
	afx_msg void OnUpdateDiscoveryBlocked(CCmdUI* pCmdUI);
	afx_msg void OnDiscoveryBlocked();
	afx_msg void OnUpdateDiscoveryAdvertise(CCmdUI* pCmdUI);
	afx_msg void OnDiscoveryAdvertise();
	afx_msg void OnUpdateDiscoveryBrowse(CCmdUI* pCmdUI);
	afx_msg void OnDiscoveryBrowse();
	afx_msg void OnCustomDrawList(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
};

#define IDC_SERVICES	100
