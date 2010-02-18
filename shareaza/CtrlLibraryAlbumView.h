//
// CtrlLibraryAlbumView.h
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

#if !defined(AFX_CTRLLIBRARYALBUMVIEW_H__19FB3DBB_8526_4E9D_A647_75DF07BAE7E2__INCLUDED_)
#define AFX_CTRLLIBRARYALBUMVIEW_H__19FB3DBB_8526_4E9D_A647_75DF07BAE7E2__INCLUDED_

#pragma once

#include "CtrlLibraryFileView.h"

class CLibraryFile;
class CLibraryAlbumTrack;


class CLibraryAlbumView : public CLibraryFileView
{
// Construction
public:
	CLibraryAlbumView();

	DECLARE_DYNCREATE(CLibraryAlbumView)

// Attributes
protected:
	CSize					m_szTrack;
	int						m_nRows;
	LPCTSTR					m_pStyle;
	static LPCTSTR			m_pStaticStyle;
protected:
	CLibraryAlbumTrack**	m_pList;
	int						m_nCount;
	int						m_nBuffer;
	int						m_nScroll;
	CImageList				m_pStars;
protected:
	CList< CLibraryAlbumTrack* > m_pSelTrack;
	int						m_nSelected;
	CLibraryAlbumTrack*		m_pFocus;
	CLibraryAlbumTrack*		m_pFirst;
	CLibraryAlbumTrack*		m_pRating;
	BOOL					m_bDrag;
	CPoint					m_ptDrag;
public:
	static COLORREF			m_crRows[2];

// Operations
public:
	virtual void		Update();
	virtual BOOL		Select(DWORD nObject);
	virtual void		SelectAll();
	virtual DWORD_PTR 	HitTestIndex(const CPoint& point) const;
	virtual HBITMAP		CreateDragImage(const CPoint& ptMouse, CPoint& ptMiddle);
protected:
	void				Clear();
	BOOL				Select(CLibraryAlbumTrack* pItem, TRISTATE bSelect = TRI_TRUE);
	BOOL				DeselectAll(CLibraryAlbumTrack* pItem = NULL);
	BOOL				SelectTo(CLibraryAlbumTrack* pItem);
	void				SelectTo(int nDelta);
protected:
	void				UpdateScroll();
	void				ScrollBy(int nDelta);
	void				ScrollTo(int nDelta);
	CLibraryAlbumTrack*	HitTest(const CPoint& point, CRect* pRect = NULL) const;
	int					GetTrackIndex(CLibraryAlbumTrack* pTrack) const;
	BOOL				GetItemRect(CLibraryAlbumTrack* pTrack, CRect* pRect);

	static int			SortList(LPCVOID pA, LPCVOID pB);

// Overrides
public:
	//{{AFX_VIRTUAL(CLibraryAlbumView)
	protected:
	virtual BOOL Create(CWnd* pParentWnd);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CLibraryAlbumView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg UINT OnGetDlgCode();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	friend class CLibraryAlbumTrack;
};

class CLibraryAlbumTrack
{
// Construction
public:
	CLibraryAlbumTrack(CLibraryFile* pFile);
	virtual ~CLibraryAlbumTrack();

// Attributes
public:
	DWORD	m_nIndex;
	DWORD		m_nCookie;
	BOOL		m_bShared;
	BOOL		m_bSelected;
public:
	int		m_nShell;
	int		m_nTrack;
	CString	m_sTrack;
	CString	m_sTitle;
	CString	m_sArtist;
	CString	m_sAlbum;
	int		m_nLength;
	CString	m_sLength;
	int		m_nBitrate;
	CString	m_sBitrate;
	int		m_nRating;
	BOOL	m_bComments;
	int		m_nSetRating;

// Operations
public:
	BOOL	Update(CLibraryFile* pFile);
	void	Paint(CLibraryAlbumView* pView, CDC* pDC, const CRect& rcBlock, int nCount);
	BOOL	HitTestRating(const CRect& rcBlock, const CPoint& point);
	BOOL	LockRating();
	static void	PaintText(CDC* pDC, const CRect& rcTrack, int nFrom, int nTo, const CString* pstr, BOOL bCenter = FALSE);
	static void	PaintText(CDC* pDC, const CRect& rcTrack, int nFrom, int nTo, int nID, BOOL bCenter = FALSE);
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_CTRLLIBRARYALBUMVIEW_H__19FB3DBB_8526_4E9D_A647_75DF07BAE7E2__INCLUDED_)
