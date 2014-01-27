//
// CtrlLibraryView.h
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

#include "ShareazaDataSource.h"

class CAlbumFolder;
class CLibraryFrame;
class CLibraryTreeItem;
class CLibraryTipCtrl;

// Abstract base class for CLibraryTileView and CLibraryFileView

class CLibraryView : public CWnd
{
	DECLARE_DYNAMIC(CLibraryView)

public:
	CLibraryView();
	virtual ~CLibraryView();

	UINT				m_nCommandID;
	LPCTSTR				m_pszToolBar;
	BOOL				m_bAvailable;
	BOOL				m_bGhostFolder;
	CLibraryListItem	m_oDropItem;

	inline CLibraryList* GetSelection() const
	{
		return m_pSelection;
	}

	virtual BOOL				Create(CWnd* pParentWnd);
	virtual BOOL				CheckAvailable(CLibraryTreeItem* pSel);
	virtual void				GetHeaderContent(int& nImage, CString& strTitle);
	virtual void				Update();
	virtual BOOL				Select(DWORD nObject);
	virtual void				SelectAll() = 0;
	virtual void				CacheSelection();
	virtual CLibraryListItem	DropHitTest(const CPoint& point) const;
	virtual CLibraryListItem	GetFolder() const;
	virtual void				StartDragging(const CPoint& ptMouse);
	virtual HBITMAP				CreateDragImage(const CPoint& ptMouse, CPoint& ptMiddle);
	virtual void				OnSkinChange() {}
	virtual DWORD_PTR			HitTestIndex(const CPoint& point) const = 0;

	CLibraryFrame*		GetFrame() const;
	CLibraryTipCtrl*	GetToolTip() const;

private:
	CLibraryListPtr		m_pSelection;

protected:
	void				PostUpdate();
	DWORD				GetFolderCookie() const;
	CLibraryTreeItem*	GetFolderSelection() const;
	CAlbumFolder*		GetSelectedAlbum(CLibraryTreeItem* pSel = NULL) const;
	BOOL				SelAdd(CLibraryListItem oObject, BOOL bNotify = TRUE);
	BOOL				SelRemove(CLibraryListItem oObject, BOOL bNotify = TRUE);
	BOOL				SelClear(BOOL bNotify = TRUE);
	INT_PTR				GetSelectedCount() const;
	POSITION			StartSelectedFileLoop() const;
	CLibraryFile*		GetNextSelectedFile(POSITION& posSel, BOOL bSharedOnly = FALSE, BOOL bAvailableOnly = TRUE) const;
	CLibraryFile*		GetSelectedFile();

	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	DECLARE_MESSAGE_MAP()

	DECLARE_DROP()
};

#define IDC_LIBRARY_VIEW	132
