//
// CtrlLibraryTileView.h
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

#include "CtrlLibraryView.h"


class CLibraryTileItem
{
public:
	CLibraryTileItem(CAlbumFolder* pFolder)
		: m_pAlbum		( pFolder )
		, m_nCookie		( ~0ul )
		, m_nIcon32		( -1 )
		, m_nIcon48		( -1 )
		, m_bSelected	( false )
		, m_bCollection	( false )
	{
		Update();
	}

	bool			m_bSelected;

	bool	Update();
	void	Paint(CDC* pDC, const CRect& rcBlock, CDC* pMemDC);
	CAlbumFolder*	GetAlbum() const	{ return LibraryFolders.CheckAlbum( m_pAlbum ) ? m_pAlbum : NULL; }
	const CString&	GetTitle() const	{ return m_sTitle; }

protected:
	CAlbumFolder*	m_pAlbum;
	DWORD			m_nCookie;
	CString			m_sTitle;
	CString			m_sSubtitle1;
	CString			m_sSubtitle2;
	int				m_nIcon32;
	int				m_nIcon48;
	bool			m_bCollection;

	void	DrawText(CDC* pDC, const CRect* prcClip, int nX, int nY, const CString& strText, CRect* prcUnion = NULL);
};


class CLibraryTileView : public CLibraryView
{
	DECLARE_DYNAMIC(CLibraryTileView)

public:
	CLibraryTileView();

protected:
	typedef boost::ptr_list< CLibraryTileItem > Container;
	typedef Container::iterator iterator;
	typedef Container::const_iterator const_iterator;
	typedef Container::reverse_iterator reverse_iterator;
	typedef Container::const_reverse_iterator const_reverse_iterator;

	iterator               begin()        { return m_oList.begin(); }
	const_iterator         begin()  const { return m_oList.begin(); }
	iterator               end()          { return m_oList.end(); }
	const_iterator         end()    const { return m_oList.end(); }
	reverse_iterator       rbegin()       { return m_oList.rbegin(); }
	const_reverse_iterator rbegin() const { return m_oList.rbegin(); }
	reverse_iterator       rend()         { return m_oList.rend(); }
	const_reverse_iterator rend()   const { return m_oList.rend(); }

	size_t size() const { return m_oList.size(); }
	bool empty() const { return m_oList.empty(); }
	iterator erase(iterator item) { return m_oList.erase( item ); }

	CSize					m_szBlock;
	int						m_nColumns;
	int						m_nRows;
	Container				m_oList;
	int						m_nScroll;
	int						m_nSelected;
	iterator				m_pFocus;
	iterator				m_pFirst;
	std::list< iterator >	m_oSelTile;
	BOOL					m_bDrag;
	CPoint					m_ptDrag;

	virtual BOOL		Create(CWnd* pParentWnd);
	virtual BOOL		CheckAvailable(CLibraryTreeItem* pSel);
	virtual void		Update();
	virtual BOOL		Select(DWORD nObject);
	virtual void		SelectAll();
	virtual CLibraryListItem DropHitTest(const CPoint& point) const;
	virtual HBITMAP		CreateDragImage(const CPoint& ptMouse, CPoint& ptMiddle);

	void				clear();
//	int					GetTileIndex(CLibraryTileItem* pTile) const;
	BOOL				Select(iterator pTile, TRISTATE bSelect = TRI_TRUE);
	BOOL				DeselectAll(iterator pTile);
	BOOL				DeselectAll();
	BOOL				SelectTo(iterator pTile);
	void				SelectTo(int nDelta);
	void				Highlight(iterator pTile);

	struct SortList : public std::binary_function<CLibraryTileItem, CLibraryTileItem, bool >
	{
		bool operator()(const CLibraryTileItem& lhs, const CLibraryTileItem& rhs) const
		{
			return _tcsicoll( lhs.GetTitle(), rhs.GetTitle() ) < 0;
		}
	};

	void				UpdateScroll();
	void				ScrollBy(int nDelta);
	void				ScrollTo(int nDelta);
	const_iterator		HitTest(const CPoint& point) const;
	iterator			HitTest(const CPoint& point);
	virtual DWORD_PTR	HitTestIndex(const CPoint& point) const;
	bool				GetItemRect(iterator pTile, CRect* pRect);

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
	afx_msg UINT OnGetDlgCode();

	DECLARE_MESSAGE_MAP()
};
