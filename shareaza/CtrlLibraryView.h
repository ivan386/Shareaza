//
// CtrlLibraryView.h
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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


class CLibraryView : public CWnd
{
// Construction
public:
	CLibraryView();
	virtual ~CLibraryView();

	DECLARE_DYNAMIC(CLibraryView)

// Attributes
public:
	UINT				m_nCommandID;
	LPCTSTR				m_pszToolBar;
	BOOL				m_bAvailable;
	BOOL				m_bGhostFolder;
	CLibraryList		m_pSelection;
	CLibraryListItem	m_oDropItem;

// Operations
public:
	virtual BOOL				CheckAvailable(CLibraryTreeItem* pSel);
	virtual void				GetHeaderContent(int& nImage, CString& strTitle);
	virtual void				Update();
	virtual BOOL				Select(DWORD nObject);
	virtual void				CacheSelection();
	virtual CLibraryListItem	DropHitTest(const CPoint& point);
	virtual CLibraryListItem	GetFolder() const;
	virtual void				StartDragging(const CPoint& ptMouse);
	virtual HBITMAP				CreateDragImage(const CPoint& ptMouse, CPoint& ptMiddle);
	virtual void				OnSkinChange() {}

protected:
	void				PostUpdate();
	CLibraryFrame*		GetFrame() const;
	CLibraryTipCtrl*	GetToolTip() const;
	DWORD				GetFolderCookie() const;
	CLibraryTreeItem*	GetFolderSelection() const;
	CAlbumFolder*		GetSelectedAlbum(CLibraryTreeItem* pSel = NULL) const;

protected:
	BOOL	SelAdd(CLibraryListItem oObject, BOOL bNotify = TRUE);
	BOOL	SelRemove(CLibraryListItem oObject, BOOL bNotify = TRUE);
	BOOL	SelClear(BOOL bNotify = TRUE);
	INT_PTR	GetSelectedCount() const;

// Overrides
public:
	//{{AFX_VIRTUAL(CLibraryView)
	public:
	virtual BOOL Create(CWnd* pParentWnd);
	//}}AFX_VIRTUAL

// Implementation
protected:
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()

	DECLARE_DROP()
};

#define IDC_LIBRARY_VIEW	132
