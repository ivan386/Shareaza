//
// CtrlSharedFolder.h
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

#if !defined(AFX_CTRLSHAREDFOLDER_H__15350972_9EEB_47FD_896F_5644AA6546B8__INCLUDED_)
#define AFX_CTRLSHAREDFOLDER_H__15350972_9EEB_47FD_896F_5644AA6546B8__INCLUDED_

#pragma once

class CLibraryFolder;


class CLibraryFolderCtrl : public CTreeCtrl
{
// Construction
public:
	CLibraryFolderCtrl();
	virtual ~CLibraryFolderCtrl();

// Attributes
protected:
	HTREEITEM	m_hRoot;
	HTREEITEM	m_hFirstSelected;
	BOOL		m_bFirstClick;
	BOOL		m_bMultiSelect;
	BOOL		m_bSaveExpand;

// Operations
public:
	void		SetMultiSelect(BOOL bMultiSelect);
	void		SetSaveExpand(BOOL bSaveExpand);
	void		Update(DWORD nUpdateCookie = 0);
	void		SetSelectedCookie(DWORD nUpdateCookie, HTREEITEM hParent = NULL, BOOL bSelect = FALSE);
	POSITION		GetSelectedFolderIterator() const;
	CLibraryFolder*	GetNextSelectedFolder(POSITION& pos) const;
	BOOL		ClearSelection(HTREEITEM hExcept = NULL, HTREEITEM hItem = NULL, BOOL bSelect = FALSE);
	BOOL		SelectAll(HTREEITEM hExcept = NULL);
	BOOL		SelectFolder(CLibraryFolder* pFolder, HTREEITEM hItem = NULL);
protected:
	void		Update(CLibraryFolder* pFolder, HTREEITEM hFolder, HTREEITEM hParent, DWORD nUpdateCookie, BOOL bRecurse);
	BOOL		SelectItems(HTREEITEM hItemFrom, HTREEITEM hItemTo);
	HTREEITEM	GetFirstSelectedItem() const;
	HTREEITEM	GetNextSelectedItem(HTREEITEM hItem) const;
	void		NotifySelectionChanged();

// Overrides
public:
	//{{AFX_VIRTUAL(CLibraryFolderCtrl)
	public:
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CLibraryFolderCtrl)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnItemExpanded(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnNcPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_CTRLSHAREDFOLDER_H__15350972_9EEB_47FD_896F_5644AA6546B8__INCLUDED_)
