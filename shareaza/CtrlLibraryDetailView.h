//
// CtrlLibraryDetailView.h
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

#if !defined(AFX_CTRLLIBRARYDETAILVIEW_H__996200C6_D0C9_4508_BBD7_5DF796ECE954__INCLUDED_)
#define AFX_CTRLLIBRARYDETAILVIEW_H__996200C6_D0C9_4508_BBD7_5DF796ECE954__INCLUDED_

#pragma once

#include "CtrlLibraryFileView.h"

class CSchema;
class CLibraryFile;


class CLibraryDetailView : public CLibraryFileView
{
// Construction
public:
	CLibraryDetailView(UINT nCommandID = ID_LIBRARY_VIEW_DETAIL);
	virtual ~CLibraryDetailView();

	DECLARE_DYNCREATE(CLibraryDetailView)

// Operations
public:
	virtual void			Update();
	virtual BOOL			Select(DWORD nObject);
	virtual void			CacheSelection();
	virtual DWORD			HitTestIndex(const CPoint& point) const;
public:
	void	SetViewSchema(CSchema* pSchema, CPtrList* pColumns, BOOL bSave, BOOL bUpdate);
protected:
	void	CacheItem(int nItem);
	void	SortItems(int nColumn = -1);

// Attributes
protected:
	UINT		m_nStyle;
protected:
	CSchema*	m_pSchema;
	CPtrList	m_pColumns;
	CCoolMenu*	m_pCoolMenu;
	BOOL		m_bCreateDragImage;

	struct LDVITEM
	{
		DWORD			nIndex;
		DWORD			nCookie;
		DWORD			nState;
		int				nIcon;
		CStringArray*	pText;
	};

// List
protected:
	LDVITEM*	m_pList;
	DWORD		m_nList;
	DWORD		m_nBuffer;
	DWORD		m_nListCookie;
	int			m_nSortColumn;
	BOOL		m_bSortFlip;
	
	static int ListCompare(LPCVOID pA, LPCVOID pB);
	static CLibraryDetailView* m_pThis;
	
// Overrides
public:
	//{{AFX_VIRTUAL(CLibraryDetailView)
	public:
	virtual BOOL Create(CWnd* pParentWnd);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CLibraryDetailView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnUpdateLibraryRename(CCmdUI* pCmdUI);
	afx_msg void OnLibraryRename();
	afx_msg void OnLibraryColumns();
	afx_msg void OnUpdateLibraryColumns(CCmdUI* pCmdUI);
	//}}AFX_MSG
	
	afx_msg void OnCacheHint(NMLVCACHEHINT* pNotify, LRESULT* pResult);
	afx_msg void OnGetDispInfoW(NMLVDISPINFO* pNotify, LRESULT* pResult);
	afx_msg void OnGetDispInfoA(NMLVDISPINFO* pNotify, LRESULT* pResult);
	afx_msg void OnColumnClick(NM_LISTVIEW* pNotify, LRESULT* pResult);
	afx_msg void OnBeginLabelEdit(LV_DISPINFO* pNotify, LRESULT* pResult);
	afx_msg void OnEndLabelEditW(LV_DISPINFO* pNotify, LRESULT* pResult);
	afx_msg void OnEndLabelEditA(LV_DISPINFO* pNotify, LRESULT* pResult);
	afx_msg void OnBeginDrag(NM_LISTVIEW* pNotify, LRESULT* pResult);
	afx_msg void OnItemChanged(NM_LISTVIEW* pNotify, LRESULT* pResult);
	afx_msg void OnItemRangeChanged(NMLVODSTATECHANGE* pNotify, LRESULT* pResult);
	afx_msg void OnFindItemW(NMLVFINDITEM* pNotify, LRESULT* pResult);
	afx_msg void OnFindItemA(NMLVFINDITEM* pNotify, LRESULT* pResult);
	afx_msg void OnCustomDraw(NMLVCUSTOMDRAW* pNotify, LRESULT* pResult);
	afx_msg void OnDblClk(NMHDR* pNotify, LRESULT* pResult);
	afx_msg void OnUpdateBlocker(CCmdUI* pCmdUI);

	DECLARE_MESSAGE_MAP()

};

class CLibraryListView : public CLibraryDetailView
{
public:
	CLibraryListView() : CLibraryDetailView( ID_LIBRARY_VIEW_LIST ) {}
	DECLARE_DYNCREATE(CLibraryListView);
};

class CLibraryIconView : public CLibraryDetailView
{
public:
	CLibraryIconView() : CLibraryDetailView( ID_LIBRARY_VIEW_ICON ) {}
	DECLARE_DYNCREATE(CLibraryIconView);
};

//{{AFX_INSERT_LOCATION}}

#define LDVI_SELECTED	0x01
#define LDVI_PRIVATE	0x02
#define LDVI_UNSCANNED	0x04
#define LDVI_UNSAFE		0x08

#endif // !defined(AFX_CTRLLIBRARYDETAILVIEW_H__996200C6_D0C9_4508_BBD7_5DF796ECE954__INCLUDED_)
