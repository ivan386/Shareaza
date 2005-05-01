//
// CtrlWndTabBar.h
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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

#if !defined(AFX_CTRLWNDTABBAR_H__879DC787_89B3_41D4_A23E_0D690E04D000__INCLUDED_)
#define AFX_CTRLWNDTABBAR_H__879DC787_89B3_41D4_A23E_0D690E04D000__INCLUDED_

#pragma once

class CChildWnd;


class CWndTabBar : public CControlBar
{
// Construction
public:
	CWndTabBar();
	virtual ~CWndTabBar();

	friend class CWindowManager;
// Item Class
public:
	class TabItem
	{
	// Construction
	public:
		TabItem(CChildWnd* pWnd, DWORD nCookie, LPCTSTR pszCaption);
		virtual ~TabItem();

	// Attributes
	public:
		HWND			m_hWnd;
		CRuntimeClass*	m_pClass;
		int				m_nImage;
	public:
		CString			m_sCaption;
		BOOL			m_bVisible;
		BOOL			m_bAlert;
		DWORD			m_nCookie;

	// Operations
	public:
		void	Paint(CWndTabBar* pBar, CDC* pDC, CRect* pRect, BOOL bSelected, BOOL bHot, BOOL bTransparent);

	};

// Attributes
protected:
	CPtrList		m_pItems;
	TabItem*		m_pSelected;
	TabItem*		m_pHot;
	DWORD			m_nCookie;
	BOOL			m_bTimer;
	BOOL			m_bMenuGray;
protected:
	CImageList		m_pImages;
	CMapPtrToWord	m_pIcons;
	int				m_nCloseImage;
	CMenu			m_mnuChild;
protected:
	int				m_nMaximumWidth;
	UINT			m_nMessage;
	CString			m_sMessage;
	CRect			m_rcMessage;
	CBitmap			m_bmImage;

// Operations
public:
	void			SetMaximumWidth(int nWidth);
	void			SetMessage(UINT nMessageID);
	void			SetMessage(LPCTSTR pszText);
	void			SetWatermark(HBITMAP hBitmap);
protected:
	TabItem*		HitTest(const CPoint& point, CRect* pItemRect = NULL) const;
	int				ImageIndexForWindow(CWnd* pChild);

// Overrides
public:
	//{{AFX_VIRTUAL(CWndTabBar)
	virtual BOOL Create(CWnd* pParentWnd, DWORD dwStyle = WS_CHILD|WS_VISIBLE|CBRS_BOTTOM, UINT nID = AFX_IDW_STATUS_BAR);
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual int OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual void DoPaint(CDC* pDC);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CWndTabBar)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	friend class TabItem;
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_CTRLWNDTABBAR_H__879DC787_89B3_41D4_A23E_0D690E04D000__INCLUDED_)
