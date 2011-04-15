//
// CtrlLibraryThumbView.h
//
// Copyright (c) Shareaza Development Team, 2002-2011.
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

#include "CtrlLibraryFileView.h"

class CLibraryThumbItem;
class CLibraryFile;


class CLibraryThumbView :
	public CLibraryFileView,
	public CThreadImpl
{
	DECLARE_DYNCREATE(CLibraryThumbView)

public:
	CLibraryThumbView();

protected:
	DWORD				m_nInvalidate;
	int					m_nColumns;
	int					m_nRows;
	CLibraryThumbItem**	m_pList;
	int					m_nCount;
	int					m_nBuffer;
	int					m_nScroll;
	int					m_nSelected;
	CLibraryThumbItem*	m_pFocus;
	CLibraryThumbItem*	m_pFirst;
	CList< CLibraryThumbItem* > m_pSelThumb;
	BOOL				m_bDrag;
	CPoint				m_ptDrag;

	virtual BOOL		Create(CWnd* pParentWnd);
	virtual void		Update();
	virtual BOOL		Select(DWORD nObject);
	virtual void		SelectAll();
	virtual DWORD_PTR	HitTestIndex(const CPoint& point) const;
	virtual HBITMAP		CreateDragImage(const CPoint& ptMouse, CPoint& ptMiddle);

	void				Clear();
	int					GetThumbIndex(CLibraryThumbItem* pThumb) const;
	BOOL				Select(CLibraryThumbItem* pThumb, TRISTATE bSelect = TRI_TRUE);
	BOOL				DeselectAll(CLibraryThumbItem* pThumb = NULL);
	BOOL				SelectTo(CLibraryThumbItem* pThumb);
	void				SelectTo(int nDelta);
	void				UpdateScroll();
	void				ScrollBy(int nDelta);
	void				ScrollTo(int nDelta);
	CLibraryThumbItem*	HitTest(const CPoint& point) const;
	BOOL				GetItemRect(CLibraryThumbItem* pThumb, CRect* pRect);
	void				StartThread();
	void				StopThread();
	void				OnRun();

	static int			SortList(LPCVOID pA, LPCVOID pB);

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
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg UINT OnGetDlgCode();

	DECLARE_MESSAGE_MAP()
};


class CLibraryThumbItem
{
public:
	CLibraryThumbItem(CLibraryFile* pFile);
	virtual ~CLibraryThumbItem();

	DWORD	m_nIndex;
	DWORD	m_nCookie;
	CString	m_sText;
	BOOL	m_bShared;
	BOOL	m_bSelected;
	int		m_nThumb;
	CBitmap	m_bmThumb;
	int		m_nShell;

	enum { thumbWaiting, thumbValid, thumbError };

public:
	BOOL	Update(CLibraryFile* pFile);
	void	Paint(CDC* pDC, const CRect& rcBlock);
};
