//
// CtrlCoolBar.h
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

#if !defined(AFX_CTRLCOOLBAR_H__72F86023_E8A5_4C09_B562_948C421D8CCF__INCLUDED_)
#define AFX_CTRLCOOLBAR_H__72F86023_E8A5_4C09_B562_948C421D8CCF__INCLUDED_

#pragma once

class CCoolBarItem;


class CCoolBarCtrl : public CControlBar
{
// Construction
public:
	CCoolBarCtrl();
	virtual ~CCoolBarCtrl();

	DECLARE_DYNAMIC(CCoolBarCtrl)

// Attributes
protected:
	CPtrList		m_pItems;
protected:
	int				m_nHeight;
	BOOL			m_bStretch;
	BOOL			m_bGripper;
	BOOL			m_bBold;
	BOOL			m_bDragForward;
	CBitmap			m_bmImage;
	BOOL			m_bBuffered;
	BOOL			m_bMenuGray;
	CSyncObject*	m_pSyncObject;
protected:
	CCoolBarItem*	m_pDown;
	CCoolBarItem*	m_pHot;
	BOOL			m_bTimer;
	CSize			m_czLast;
	CBrush			m_brBack;
	COLORREF		m_crBack;
	BOOL			m_bRecalc;

// Operations
public:
	void			SetSize(int nHeight, BOOL bStretch);
	void			SetGripper(BOOL bGripper);
	void			SetBold(BOOL bBold);
	void			SetDragForward(BOOL bForward);
	void			SetWatermark(HBITMAP hBitmap, BOOL bDetach = FALSE);
	void			SetSyncObject(CSyncObject* pSyncObject);
public:
	CCoolBarItem*	Add(UINT nID, LPCTSTR pszText = NULL, int nPosition = -1);
	CCoolBarItem*	Add(UINT nCtrlID, int nWidth, int nHeight = 0);
	CCoolBarItem*	GetIndex(int nIndex) const;
	CCoolBarItem*	GetID(UINT nID) const;
	int				GetIndexForID(UINT nID) const;
	int				GetCount() const;
	BOOL			LoadToolBar(UINT nIDToolBar);
	void			Clear();
	void			Copy(CCoolBarCtrl* pOther);
	BOOL			GetItemRect(CCoolBarItem* pFind, CRect* pRect) const;
	UINT			ThrowMenu(UINT nID, CMenu* pMenu, CWnd* pParent = NULL, BOOL bCommand = FALSE, BOOL bRight = FALSE);
	void			OnUpdated();
protected:
	virtual void	PrepareRect(CRect* pRect) const;
	virtual void	DoPaint(CDC* pDC, CRect& rc, BOOL bTransparent);
	CCoolBarItem*	HitTest(const CPoint& point, CRect* pItemRect = NULL, BOOL bSeparators = FALSE) const;
	void			SmartMove(CWnd* pCtrl, CRect* pRect);

// Overrides
public:
	//{{AFX_VIRTUAL(CCoolBarCtrl)
	public:
	virtual BOOL Create(CWnd* pParentWnd, DWORD dwStyle = WS_CHILD|WS_VISIBLE|CBRS_TOP, UINT nID = AFX_IDW_TOOLBAR);
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual int OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	virtual void DoPaint(CDC* pDC);
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CCoolBarCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()

	friend class CCoolBarItem;
};


class CCoolBarItem : public CCmdUI
{
// Construction
public:
	CCoolBarItem(CCoolBarCtrl* pBar, UINT nID = ID_SEPARATOR, int nImage = -1);
	CCoolBarItem(CCoolBarCtrl* pBar, CCoolBarItem* pCopy);
	virtual ~CCoolBarItem();

// Attributes
public:
	BOOL		m_bVisible;
	BOOL		m_bEnabled;
	BOOL		m_bChecked;
public:
	int			m_nImage;
	CString		m_sText;
	CString		m_sTip;
	COLORREF	m_crText;
	UINT		m_nCtrlID;
	int			m_nCtrlHeight;
protected:
	CCoolBarCtrl*	m_pBar;
	BOOL			m_bDirty;
	int				m_nWidth;
	
// Operations
public:
	virtual void Show(BOOL bOn);
	virtual void Enable(BOOL bOn);
	virtual void SetCheck(int nCheck);
	virtual void SetImage(UINT nCommandID);
	virtual void SetText(LPCTSTR lpszText);
	virtual void SetTip(LPCTSTR pszTip);
	virtual void SetTextColour(COLORREF crText = 0xFFFFFFFF);
protected:
	void	Paint(CDC* pDC, CRect& rc, BOOL bDown, BOOL bHot, BOOL bMenuGray, BOOL bTransparent);
public:
	
	static CCoolBarItem* FromCmdUI(CCmdUI* pUI);
	
	friend class CCoolBarCtrl;
};

//{{AFX_INSERT_LOCATION}}

#ifndef ID_SEPARATOR
#define ID_SEPARATOR 0
#endif

#ifndef ID_RIGHTALIGN
#define ID_RIGHTALIGN -1
#endif


#endif // !defined(AFX_CTRLCOOLBAR_H__72F86023_E8A5_4C09_B562_948C421D8CCF__INCLUDED_)
