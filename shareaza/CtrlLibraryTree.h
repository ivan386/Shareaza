//
// CtrlLibraryTree.h
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

#if !defined(AFX_CTRLLIBRARYTREE_H__F092090F_D43A_4F3D_A584_8AE441A95945__INCLUDED_)
#define AFX_CTRLLIBRARYTREE_H__F092090F_D43A_4F3D_A584_8AE441A95945__INCLUDED_

#pragma once

class CLibraryTreeItem;
class CCoolTipCtrl;
class CLibraryFolder;
class CAlbumFolder;


class CLibraryTreeCtrl : public CWnd
{
// Construction
public:
	CLibraryTreeCtrl();
	virtual ~CLibraryTreeCtrl();

	DECLARE_DYNAMIC(CLibraryTreeCtrl)

// Attributes
protected:
	CLibraryTreeItem*	m_pRoot;
	int					m_nTotal;
	int					m_nVisible;
	int					m_nScroll;
protected:
	int					m_nSelected;
	CLibraryTreeItem*	m_pSelFirst;
	CLibraryTreeItem*	m_pSelLast;
	CLibraryTreeItem*	m_pFocus;
protected:
	BOOL				m_bDrag;
	CPoint				m_ptDrag;
	CLibraryTreeItem*	m_pDropItem;
protected:
	DWORD				m_nCleanCookie;
	CCoolTipCtrl*		m_pTip;

// Operations
public:
	void				SetToolTip(CCoolTipCtrl* pTip);
	void				Clear();
	BOOL				Expand(CLibraryTreeItem* pItem, TRISTATE bExpand = TS_TRUE, BOOL bInvalidate = TRUE);
	BOOL				Select(CLibraryTreeItem* pItem, TRISTATE bSelect = TS_TRUE, BOOL bInvalidate = TRUE);
	BOOL				SelectAll(CLibraryTreeItem* pParent = NULL, BOOL bInvalidate = TRUE);
	BOOL				DeselectAll(CLibraryTreeItem* pExcept = NULL, CLibraryTreeItem* pParent = NULL, BOOL bInvalidate = TRUE);
	BOOL				Highlight(CLibraryTreeItem* pItem);
	int					GetSelectedCount() const;
	CLibraryTreeItem*	GetFirstSelected() const;
	CLibraryTreeItem*	GetLastSelected() const;
	CLibraryTreeItem*	HitTest(const POINT& point, RECT* pRect = NULL) const;
	BOOL				GetRect(CLibraryTreeItem* pItem, RECT* pRect);
	CLibraryTreeItem*	GetFolderItem(LPVOID pSearch, CLibraryTreeItem* pParent = NULL);
protected:
	void				UpdateScroll();
	void				ScrollBy(int nDelta);
	void				ScrollTo(int nPosition);
	void				Paint(CDC& dc, CRect& rcClient, CPoint& pt, CLibraryTreeItem* pItem);
	CLibraryTreeItem*	HitTest(CRect& rcClient, CPoint& pt, CLibraryTreeItem* pItem, const POINT& point, RECT* pRect) const;
	BOOL				GetRect(CPoint& pt, CLibraryTreeItem* pItem, CLibraryTreeItem* pFind, RECT* pRect);
	BOOL				CleanItems(CLibraryTreeItem* pItem, DWORD nCookie, BOOL bVisible);
	BOOL				CollapseRecursive(CLibraryTreeItem* pItem);
	void				NotifySelection();
	void				StartDragging(CPoint& ptMouse);
	CImageList*			CreateDragImage(const CPoint& ptMouse);

// Overrides
public:
	//{{AFX_VIRTUAL(CLibraryTreeCtrl)
	public:
	virtual BOOL Create(CWnd* pParentWnd);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CLibraryTreeCtrl)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};


class CLibraryTreeItem
{
// Construction
public:
	CLibraryTreeItem(CLibraryTreeItem* pParent = NULL);
	virtual ~CLibraryTreeItem();

// Attributes
public:
	CLibraryTreeItem*	m_pParent;
	CLibraryTreeItem**	m_pList;
	int					m_nCount;
	int					m_nBuffer;
	CLibraryTreeItem*	m_pSelPrev;
	CLibraryTreeItem*	m_pSelNext;
	DWORD				m_nCleanCookie;
public:
	BOOL				m_bExpanded;
	BOOL				m_bSelected;
	BOOL				m_bContract1;
	BOOL				m_bContract2;
public:
	CLibraryFolder*		m_pPhysical;
	CAlbumFolder*		m_pVirtual;
	DWORD				m_nCookie;
	CString				m_sText;
	BOOL				m_bBold;
	BOOL				m_bShared;
	BOOL				m_bCollection;
	int					m_nIcon16;

// Operations
public:
	CLibraryTreeItem*	Add(LPCTSTR pszName);
	void				Delete();
	void				Delete(CLibraryTreeItem* pItem);
	void				Delete(int nItem);
	void				Clear();
	BOOL				IsVisible() const;
	int					GetChildCount() const;
	void				Paint(CDC& dc, CRect& rc, BOOL bTarget, COLORREF crBack = CLR_NONE) const;
	int					GetFileList(CLibraryList* pList, BOOL bRecursive = FALSE) const;

};

//{{AFX_INSERT_LOCATION}}

#define IDC_LIBRARY_TREE	131
#define LTN_SELCHANGED		101

#endif // !defined(AFX_CTRLLIBRARYTREE_H__F092090F_D43A_4F3D_A584_8AE441A95945__INCLUDED_)
