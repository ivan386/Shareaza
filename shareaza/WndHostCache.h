//
// WndHostCache.h
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

#if !defined(WNDHOSTCACHE_H)
#define WNDHOSTCACHE_H

#pragma once

#include "WndPanel.h"
#include "LiveList.h"

class CHostCacheHost;


class CHostCacheWnd : public CPanelWnd
{
public:
	CHostCacheWnd();
	virtual ~CHostCacheWnd();

	DECLARE_SERIAL(CHostCacheWnd)

// Attributes
public:
	PROTOCOLID		m_nMode;
	BOOL			m_bAllowUpdates;
protected:
	CCoolBarCtrl	m_wndToolBar;
	CLiveListCtrl	m_wndList;
	CLiveListSizer	m_pSizer;
	CImageList		m_gdiImageList;
	DWORD			m_nCookie;
	DWORD			tLastUpdate;

// Operations
public:
	void			Update(BOOL bForce = FALSE);
	CHostCacheHost*	GetItem(int nItem);
	virtual void	OnSkinChange();
	
// Overrides
protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

// Implementation
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCustomDrawList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblClkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSortList(NMHDR* pNotifyStruct, LRESULT *pResult);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnUpdateHostCacheConnect(CCmdUI* pCmdUI);
	afx_msg void OnHostCacheConnect();
	afx_msg void OnUpdateHostCacheDisconnect(CCmdUI* pCmdUI);
	afx_msg void OnHostCacheDisconnect();
	afx_msg void OnUpdateHostCacheRemove(CCmdUI* pCmdUI);
	afx_msg void OnHostCacheRemove();
	afx_msg void OnDestroy();
	afx_msg void OnUpdateHostcacheG2Horizon(CCmdUI* pCmdUI);
	afx_msg void OnHostcacheG2Horizon();
	afx_msg void OnUpdateHostcacheG2Cache(CCmdUI* pCmdUI);
	afx_msg void OnHostcacheG2Cache();
	afx_msg void OnUpdateHostcacheG1Cache(CCmdUI* pCmdUI);
	afx_msg void OnHostcacheG1Cache();
	afx_msg void OnUpdateHostcacheEd2kCache(CCmdUI* pCmdUI);
	afx_msg void OnHostcacheEd2kCache();
	afx_msg void OnUpdateHostcacheBTCache(CCmdUI* pCmdUI);
	afx_msg void OnHostcacheBTCache();
	afx_msg void OnUpdateHostcacheKADCache(CCmdUI* pCmdUI);
	afx_msg void OnHostcacheKADCache();
	afx_msg void OnHostcacheImport();
	afx_msg void OnHostcacheEd2kDownload();
	afx_msg void OnUpdateHostcachePriority(CCmdUI* pCmdUI);
	afx_msg void OnHostcachePriority();
	afx_msg void OnUpdateNeighboursCopy(CCmdUI *pCmdUI);
	afx_msg void OnNeighboursCopy();

	DECLARE_MESSAGE_MAP()

protected:
	virtual void RecalcLayout(BOOL bNotify = TRUE);
};

#define IDC_HOSTS		100

#endif // !defined(WNDHOSTCACHE_H)
