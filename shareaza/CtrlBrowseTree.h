//
// CtrlBrowseTree.h
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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

#include "Schema.h"

class CBrowseTreeItem;
class CG2Packet;
class CXMLElement;


class CBrowseTreeCtrl : public CWnd
{
// Construction
public:
	CBrowseTreeCtrl();
	virtual ~CBrowseTreeCtrl();

	DECLARE_DYNAMIC(CBrowseTreeCtrl)

// Attributes
protected:
	CCriticalSection	m_csRoot;
	CBrowseTreeItem*	m_pRoot;
	int					m_nTotal;
	int					m_nVisible;
	int					m_nScroll;
protected:
	int					m_nSelected;
	CBrowseTreeItem*	m_pSelFirst;
	CBrowseTreeItem*	m_pSelLast;
	CBrowseTreeItem*	m_pFocus;
protected:
	DWORD				m_nCleanCookie;

// Operations
public:
	virtual BOOL		Create(CWnd* pParentWnd);
	void				Clear(BOOL bGUI = TRUE);
	BOOL				Expand(CBrowseTreeItem* pItem, TRISTATE bExpand = TRI_TRUE, BOOL bInvalidate = TRUE);
	BOOL				Select(CBrowseTreeItem* pItem, TRISTATE bSelect = TRI_TRUE, BOOL bInvalidate = TRUE);
	BOOL				DeselectAll(CBrowseTreeItem* pExcept = NULL, CBrowseTreeItem* pParent = NULL, BOOL bInvalidate = TRUE);
	BOOL				Highlight(CBrowseTreeItem* pItem);
	int					GetSelectedCount() const;
	CBrowseTreeItem*	GetFirstSelected() const;
	CBrowseTreeItem*	GetLastSelected() const;
	CBrowseTreeItem*	HitTest(const POINT& point, RECT* pRect = NULL) const;
	BOOL				GetRect(CBrowseTreeItem* pItem, RECT* pRect);
	void				OnTreePacket(CG2Packet* pPacket);
protected:
	void				UpdateScroll();
	void				ScrollBy(int nDelta);
	void				ScrollTo(int nPosition);
	void				Paint(CDC& dc, CRect& rcClient, CPoint& pt, CBrowseTreeItem* pItem);
	CBrowseTreeItem*	HitTest(CRect& rcClient, CPoint& pt, CBrowseTreeItem* pItem, const POINT& point, RECT* pRect) const;
	BOOL				GetRect(CPoint& pt, CBrowseTreeItem* pItem, CBrowseTreeItem* pFind, RECT* pRect);
	BOOL				CleanItems(CBrowseTreeItem* pItem, DWORD nCookie, BOOL bVisible);
	BOOL				CollapseRecursive(CBrowseTreeItem* pItem);
	void				NotifySelection();
	void				OnTreePacket(CG2Packet* pPacket, DWORD nFinish, CBrowseTreeItem* pItem);

// Inlines
public:
	inline CSyncObject* SyncRoot()
	{
		return &m_csRoot;
	}

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
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
	afx_msg LRESULT OnUpdate(WPARAM, LPARAM);

};


class CBrowseTreeItem : public CObject
{
// Construction
public:
	CBrowseTreeItem(CBrowseTreeItem* pParent = NULL);
	virtual ~CBrowseTreeItem();

// Attributes
public:
	CBrowseTreeItem*	m_pParent;
	CBrowseTreeItem**	m_pList;
	int					m_nCount;
	int					m_nBuffer;
	CBrowseTreeItem*	m_pSelPrev;
	CBrowseTreeItem*	m_pSelNext;
	DWORD				m_nCleanCookie;
public:
	BOOL				m_bExpanded;
	BOOL				m_bSelected;
	BOOL				m_bContract1;
	BOOL				m_bContract2;
public:
	DWORD				m_nCookie;
	CString				m_sText;
	BOOL				m_bBold;
	int					m_nIcon16;
public:
	CSchemaPtr			m_pSchema;
	DWORD*				m_pFiles;
	DWORD				m_nFiles;

// Operations
public:
	CBrowseTreeItem*	Add(LPCTSTR pszName);
	CBrowseTreeItem*	Add(CBrowseTreeItem* pNewItem);
	void				Delete();
	void				Delete(CBrowseTreeItem* pItem);
	void				Delete(int nItem);
	void				Clear();
	BOOL				IsVisible() const;
	int					GetChildCount() const;
	void				Paint(CDC& dc, CRect& rc, BOOL bTarget, COLORREF crBack = CLR_NONE) const;
	void				AddXML(const CXMLElement* pXML);

};

#define IDC_BROWSE_TREE 125
#define BTN_SELCHANGED	101
