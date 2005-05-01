//
// CtrlLibraryTileView.h
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

#if !defined(AFX_CTRLLIBRARYTILEVIEW_H__C409D195_CBAA_45F3_9757_AF8F0D67174C__INCLUDED_)
#define AFX_CTRLLIBRARYTILEVIEW_H__C409D195_CBAA_45F3_9757_AF8F0D67174C__INCLUDED_

#pragma once

#include "CtrlLibraryView.h"

class CLibraryTileItem;
class CAlbumFolder;


class CLibraryTileView : public CLibraryView
{
// Construction
public:
	CLibraryTileView();
	virtual ~CLibraryTileView();

// Attributes
protected:
	CSize				m_szBlock;
	int					m_nColumns;
	int					m_nRows;
protected:
	CLibraryTileItem**	m_pList;
	int					m_nCount;
	int					m_nBuffer;
	int					m_nScroll;
protected:
	int					m_nSelected;
	CLibraryTileItem*	m_pFocus;
	CLibraryTileItem*	m_pFirst;
	CPtrList			m_pSelTile;
	BOOL				m_bDrag;
	CPoint				m_ptDrag;

// Operations
public:
	virtual BOOL	CheckAvailable(CLibraryTreeItem* pSel);
	virtual void	Update();
	virtual BOOL	Select(DWORD nObject);
protected:
	void			Clear();
	int				GetTileIndex(CLibraryTileItem* pTile) const;
	BOOL			Select(CLibraryTileItem* pTile, TRISTATE bSelect = TS_TRUE);
	BOOL			DeselectAll(CLibraryTileItem* pTile = NULL);
	BOOL			SelectTo(CLibraryTileItem* pTile);
	void			SelectTo(int nDelta);
	void			Highlight(CLibraryTileItem* pTile);
protected:
	static int			SortList(LPCVOID pA, LPCVOID pB);
	void				UpdateScroll();
	void				ScrollBy(int nDelta);
	void				ScrollTo(int nDelta);
	CLibraryTileItem*	HitTest(const CPoint& point) const;
	BOOL				GetItemRect(CLibraryTileItem* pTile, CRect* pRect);
	void				StartDragging(CPoint& ptMouse);
	CImageList*			CreateDragImage(const CPoint& ptMouse);

// Overrides
public:
	//{{AFX_VIRTUAL(CLibraryTileView)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CLibraryTileView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnUpdateLibraryAlbumOpen(CCmdUI* pCmdUI);
	afx_msg void OnLibraryAlbumOpen();
	afx_msg void OnUpdateLibraryAlbumDelete(CCmdUI* pCmdUI);
	afx_msg void OnLibraryAlbumDelete();
	afx_msg void OnUpdateLibraryAlbumProperties(CCmdUI* pCmdUI);
	afx_msg void OnLibraryAlbumProperties();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};


class CLibraryTileItem
{
// Construction
public:
	CLibraryTileItem(CAlbumFolder* pFolder);
	virtual ~CLibraryTileItem();

// Attributes
public:
	CAlbumFolder*	m_pFolder;
	DWORD			m_nCookie;
	BOOL			m_bSelected;
	CString			m_sTitle;
	CString			m_sSubtitle1;
	CString			m_sSubtitle2;
	int				m_nIcon32;
	int				m_nIcon48;
	BOOL			m_bCollection;

// Operations
public:
	BOOL	Update();
	void	Paint(CDC* pDC, const CRect& rcBlock, CDC* pMemDC);
protected:
	void	DrawText(CDC* pDC, const CRect* prcClip, int nX, int nY, LPCTSTR pszText, CRect* prcUnion = NULL);

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_CTRLLIBRARYTILEVIEW_H__C409D195_CBAA_45F3_9757_AF8F0D67174C__INCLUDED_)
