//
// WndSearch.h
//
// Copyright (c) Shareaza Development Team, 2002-2004.
// This file is part of SHAREAZA (www.shareaza.com)
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

#include "WndBaseMatch.h"
#include "CtrlSearchPanel.h"
#include "CtrlSearchDetailPanel.h"

class CManagedSearch;


class CSearchWnd : public CBaseMatchWnd
{
public:
	CSearchWnd(CQuerySearch* pSearch = NULL);
	virtual ~CSearchWnd();

	DECLARE_DYNCREATE(CSearchWnd)
	friend class CRemote;

// Attributes
protected:
	CSearchPanel		m_wndPanel;
	BOOL				m_bPanel;
	BOOL				m_bSetFocus;
	CSearchDetailPanel	m_wndDetails;
	BOOL				m_bDetails;
	int					m_nDetails;
public:
	CPtrList			m_pSearches;
	DWORD				m_tSearch;
	DWORD				m_nCacheHits;
	DWORD				m_nCacheHubs;
	DWORD				m_nCacheLeaves;
	CString				m_sCaption;
	
// Operations
public:
	void			Serialize(CArchive& ar);
	CManagedSearch*	GetLastManager();
	CQuerySearch*	GetLastSearch();
	void			ExecuteSearch();
protected:
	BOOL			DoSizeDetails();
public:	
	virtual void	OnSkinChange();
	virtual BOOL	OnQueryHits(CQueryHit* pHits);
	virtual void	UpdateMessages(BOOL bActive = TRUE);

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnSelChangeMatches();
	afx_msg void OnUpdateSearchSearch(CCmdUI* pCmdUI);
	afx_msg void OnSearchSearch();
	afx_msg void OnSearchClear();
	afx_msg void OnUpdateSearchStop(CCmdUI* pCmdUI);
	afx_msg void OnSearchStop();
	afx_msg void OnUpdateSearchPanel(CCmdUI* pCmdUI);
	afx_msg void OnSearchPanel();
	afx_msg void OnUpdateSearchClear(CCmdUI* pCmdUI);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnUpdateSearchDetails(CCmdUI* pCmdUI);
	afx_msg void OnSearchDetails();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
};
