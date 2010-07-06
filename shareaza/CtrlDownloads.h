//
// CtrlDownloads.h
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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

#include "CtrlDownloadTip.h"

class CDownload;
class CDownloadSource;


class CDownloadsCtrl : public CWnd
{
// Construction
public:
	CDownloadsCtrl();

	DECLARE_DYNAMIC(CDownloadsCtrl)

// Operations
public:
	BOOL		Create(CWnd* pParentWnd, UINT nID);
	BOOL		Update();
	BOOL		Update(int nGroupCookie);
	BOOL		DropShowTarget(CList< CDownload* >* pSel, const CPoint& ptScreen);
	BOOL		DropObjects(CList< CDownload* >* pSel, const CPoint& ptScreen);
	int			GetExpandableColumnX() const;
	void		OnSkinChange();
protected:
	void		InsertColumn(int nColumn, LPCTSTR pszCaption, int nFormat, int nWidth);
	void		SaveColumnState();
	BOOL		LoadColumnState();
	void		SelectTo(int nIndex);
	void		BubbleSortDownloads(int nColumn);
    void		DeselectAll(CDownload* pExcept1 = NULL, CDownloadSource* pExcept2 = NULL);
	void		SelectAll(CDownload* pExcept1 = NULL, CDownloadSource* pExcept2 = NULL);
	int			GetSelectedCount();
	BOOL		HitTest(const CPoint& point, CDownload** ppDownload, CDownloadSource** ppSource, int* pnIndex, RECT* prcItem);
	BOOL		GetAt(int nSelect, CDownload** ppDownload, CDownloadSource** ppSource);
	BOOL		GetRect(CDownload* pSelect, RECT* prcItem);
	void		MoveSelected(int nDelta);
	void		PaintDownload(CDC& dc, const CRect& rcRow, CDownload* pDownload, BOOL bFocus, BOOL bDrop);
	void		PaintSource(CDC& dc, const CRect& rcRow, CDownload* pDownload, CDownloadSource* pSource, BOOL bFocus);
	void		OnBeginDrag(CPoint ptAction);
	CImageList*	CreateDragImage(CList< CDownload* >* pSel, const CPoint& ptMouse);
public:
	static bool	IsFiltered(const CDownload* pDownload);
	static BOOL	IsExpandable(CDownload* pDownload);

	friend class CDownloadsWnd;

// Attributes
protected:
	CHeaderCtrl			m_wndHeader;
	CDownloadTipCtrl	m_wndTip;
	CImageList			m_pProtocols;
	int					m_nGroupCookie;
	int					m_nFocus;
	BOOL				m_bCreateDragImage;
	CDownload*			m_pDragDrop;
	BOOL				m_bDrag;
	CPoint				m_ptDrag;
	CDownload*			m_pDeselect1;
	CDownloadSource*	m_pDeselect2;
	BOOL*				m_pbSortAscending;
	int					m_nScrollWheelLines;
	BOOL				m_bShowSearching;
	DWORD				m_tSwitchTimer;

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnChangeHeader(NMHDR* pNotifyStruct, LRESULT* pResult);
	afx_msg void OnSortPanelItems(NMHDR* pNotifyStruct, LRESULT* pResult);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnEnterKey();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg UINT OnGetDlgCode();
};

#define DLF_ACTIVE		0x01
#define DLF_QUEUED		0x02
#define DLF_SOURCES		0x04
#define DLF_PAUSED		0x08

#define DLF_ALL			(DLF_ACTIVE|DLF_QUEUED|DLF_SOURCES|DLF_PAUSED)
