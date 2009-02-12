//
// CtrlDownloadTabBar.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

class CDownloadGroup;


class CDownloadTabBar : public CControlBar
{
// Construction
public:
	CDownloadTabBar();
	virtual ~CDownloadTabBar();

// Item Class
public:
	class TabItem
	{
	// Construction
	public:
		TabItem(CDownloadGroup* pGroup, int nCookie);
		virtual ~TabItem();

	// Attributes
	public:
		CDownloadGroup*	m_pGroup;
		CString			m_sCaption;
		int				m_nImage;
		CString			m_sName;
		int				m_nCount;
		BOOL			m_bSelected;
		BOOL			m_bTabTest;
		CBitmap			m_bmTabmark;

	// Operations
	public:
		BOOL	Update(int nCookie);
		BOOL	Select(BOOL bSelect);
		void	Paint(CDownloadTabBar* pBar, CDC* pDC, CRect* pRect, BOOL bHot, BOOL bTransparent);
		void	SetTabmark(HBITMAP hBitmap);
	};

// Attributes
protected:
	CList< TabItem* > m_pItems;
	TabItem*		m_pHot;
	BOOL			m_bTimer;
	BOOL			m_bMenuGray;
	int				m_nCookie;
protected:
	int				m_nMaximumWidth;
	UINT			m_nMessage;
	CString			m_sMessage;
	CBitmap			m_bmImage;

// Operations
public:
	void			SetWatermark(HBITMAP hBitmap);
	void			OnSkinChange();
	void			Update(int nCookie);
	BOOL			DropShowTarget(CList< CDownload* >* pList, const CPoint& ptScreen);
	BOOL			DropObjects(CList< CDownload* >* pList, const CPoint& ptScreen);
protected:
	void			UpdateGroups(int nCookie);
	void			UpdateStates(int nCookie);
	TabItem*		HitTest(const CPoint& point, CRect* pItemRect = NULL) const;
	BOOL			Select(TabItem* pHit);
	int				GetSelectedCount(BOOL bDownloads = FALSE);
	TabItem*		GetSelectedItem();
	CDownloadGroup*	GetSelectedGroup();
	void			GetSelectedDownloads(CList< CDownload* >* pDownloads);
	void			NotifySelection();

// Overrides
public:
	virtual BOOL	Create(CWnd* pParentWnd, DWORD dwStyle = WS_CHILD|WS_VISIBLE|CBRS_BOTTOM, UINT nID = AFX_IDW_STATUS_BAR);
	virtual CSize	CalcFixedLayout(BOOL bStretch, BOOL bHorz);
//	virtual INT_PTR	OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	virtual void	DoPaint(CDC* pDC);
	virtual void	OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL /*bDisableIfNoHndler*/) {};

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnDownloadGroupNew();
	afx_msg void OnUpdateDownloadGroupRemove(CCmdUI* pCmdUI);
	afx_msg void OnDownloadGroupRemove();
	afx_msg void OnUpdateDownloadGroupProperties(CCmdUI* pCmdUI);
	afx_msg void OnDownloadGroupProperties();
	afx_msg void OnUpdateDownloadGroupResume(CCmdUI* pCmdUI);
	afx_msg void OnDownloadGroupResume();
	afx_msg void OnUpdateDownloadGroupPause(CCmdUI* pCmdUI);
	afx_msg void OnDownloadGroupPause();
	afx_msg void OnUpdateDownloadGroupClear(CCmdUI* pCmdUI);
	afx_msg void OnDownloadGroupClear();

	friend class TabItem;
};
