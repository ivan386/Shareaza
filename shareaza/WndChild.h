//
// WndChild.h
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

#if !defined(AFX_WNDCHILD_H__082500C0_14CD_4066_95F9_DA21AD8C7E72__INCLUDED_)
#define AFX_WNDCHILD_H__082500C0_14CD_4066_95F9_DA21AD8C7E72__INCLUDED_

#pragma once

#include "CtrlCoolBar.h"
#include "LiveListSizer.h"

class CMainWnd;
class CWindowManager;
class CQuerySearch;
class CQueryHit;
class CBuffer;
class CConnection;
class CSkinWindow;


class CChildWnd : public CMDIChildWnd
{
// Construction
public:
	CChildWnd();
	virtual ~CChildWnd();

	DECLARE_DYNCREATE(CChildWnd)

// Attributes
public:
	UINT			m_nResID;
	BOOL			m_bTabMode;
	BOOL			m_bGroupMode;
	CChildWnd*		m_pGroupParent;
	float			m_nGroupSize;
	BOOL			m_bPanelMode;
	BOOL			m_bAlert;
	CSkinWindow*	m_pSkin;
private:
	static CChildWnd*	m_pCmdMsg;

// Operations
public:
	CMainWnd*		GetMainWnd();
	CWindowManager*	GetManager();
	BOOL			IsActive(BOOL bFocused = FALSE);
	BOOL			IsPartiallyVisible();
	BOOL			TestPoint(const CPoint& ptScreen);
	void			TrackPopupMenu(LPCTSTR pszMenu, const CPoint& point, UINT nDefaultID = 0);
	BOOL			LoadState(LPCTSTR pszName = NULL, BOOL bDefaultMaximise = TRUE);
	BOOL			SaveState(LPCTSTR pszName = NULL);
	BOOL			SetAlert(BOOL bAlert = TRUE);
	void			SizeListAndBar(CWnd* pList, CWnd* pBar);
public:
	virtual void	OnSkinChange();
	virtual void	OnQuerySearch(CQuerySearch* pSearch);
	virtual BOOL	OnQueryHits(CQueryHit* pHits);
	virtual BOOL	OnPush(GGUID* pClientID, CConnection* pConnection);
	virtual HRESULT	GetGenericView(IGenericView** ppView);
	virtual BOOL	OnDropFiles(CStringList& pFiles, const CPoint& ptScreen, BOOL bDrop);

// Overrides
public:
	//{{AFX_VIRTUAL(CChildWnd)
	public:
	virtual BOOL Create(UINT nID, BOOL bVisible = TRUE);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CChildWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	afx_msg void OnNcRButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg void OnNcPaint();
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	//}}AFX_MSG
	afx_msg LONG OnSetText(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#define IDC_PANEL	101


#endif // !defined(AFX_WNDCHILD_H__082500C0_14CD_4066_95F9_DA21AD8C7E72__INCLUDED_)
