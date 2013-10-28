//
// CtrlLibraryTreeView.h
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

#include "CtrlTipFolder.h"
#include "CtrlTipAlbum.h"
#include "ShareazaDataSource.h"

class CLibraryTreeItem;
class CLibraryFolder;
class CAlbumFolder;

class CLibraryTreeView : public CWnd
{
// Construction
public:
	CLibraryTreeView();
	virtual ~CLibraryTreeView();

	DECLARE_DYNAMIC(CLibraryTreeView)

// Attributes
private:
	CLibraryTreeItem*	m_pRoot;
	size_t				m_nTotal;
	int					m_nVisible;
	int					m_nScroll;
	int					m_nSelected;
	CLibraryTreeItem*	m_pSelFirst;
	CLibraryTreeItem*	m_pSelLast;
	CLibraryTreeItem*	m_pFocus;
	void*				m_pFocusedObject[ 2 ];	// Last focused object in both views
	BOOL				m_bDrag;
	CPoint				m_ptDrag;
	CLibraryTreeItem*	m_pDropItem;
	DWORD				m_nCleanCookie;
	BOOL				m_bVirtual;
	CFolderTipCtrl		m_wndFolderTip;
	CAlbumTipCtrl		m_wndAlbumTip;

// Operations
public:
	BOOL				Expand(CLibraryTreeItem* pItem, TRISTATE bExpand = TRI_TRUE, BOOL bInvalidate = TRUE);
	BOOL				Select(CLibraryTreeItem* pItem, TRISTATE bSelect = TRI_TRUE, BOOL bInvalidate = TRUE);
	BOOL				SelectAll(CLibraryTreeItem* pParent = NULL, BOOL bInvalidate = TRUE);
	BOOL				DeselectAll(CLibraryTreeItem* pExcept = NULL, CLibraryTreeItem* pParent = NULL, BOOL bInvalidate = TRUE);
	BOOL				Highlight(CLibraryTreeItem* pItem);
	int					GetSelectedCount() const;
	CLibraryTreeItem*	GetFirstSelected() const;
	CLibraryTreeItem*	GetLastSelected() const;
	CLibraryTreeItem*	HitTest(const POINT& point, RECT* pRect = NULL) const;
	BOOL				GetRect(CLibraryTreeItem* pItem, RECT* pRect) const;
	// Search tree for physical folder or virtual album (recursive)
	CLibraryTreeItem*	GetFolderItem(LPCVOID pSearch, CLibraryTreeItem* pParent = NULL) const;
	// Get default download folder for physical view or favorites album for virtual view
	CLibraryTreeItem*	GetDefaultFolderItem() const;
	void				SetVirtual(BOOL bVirtual);
	BOOL				Update(DWORD nSelectCookie);
	// Forcefully select and highlight folder, then update all views
	BOOL				SelectFolder(LPCVOID pSearch);
	// Forcefully select and highlight tree item, then update all views
	BOOL				SelectFolderItem(CLibraryTreeItem* pItem);

protected:
	void				Clear();
	void				UpdateScroll();
	void				ScrollBy(int nDelta);
	void				ScrollTo(int nPosition);
	void				Paint(CDC& dc, CRect& rcClient, CPoint& pt, CLibraryTreeItem* pItem);
	CLibraryTreeItem*	HitTest(CRect& rcClient, CPoint& pt, CLibraryTreeItem* pItem, const POINT& point, RECT* pRect) const;
	BOOL				GetRect(CPoint& pt, CLibraryTreeItem* pItem, CLibraryTreeItem* pFind, RECT* pRect) const;
	BOOL				CleanItems(CLibraryTreeItem* pItem, DWORD nCookie, BOOL bVisible);
	BOOL				CollapseRecursive(CLibraryTreeItem* pItem);
	void				NotifySelection();
	virtual HBITMAP		CreateDragImage(const CPoint& ptMouse, CPoint& ptMiddle);
	void				StartDragging(CPoint& ptMouse);
	void				PostUpdate();
	BOOL				UpdatePhysical(DWORD nSelectCookie);
	BOOL				UpdateVirtual(DWORD nSelectCookie);
	BOOL				Update(CLibraryFolder* pFolder, CLibraryTreeItem* pItem, CLibraryTreeItem* pParent, BOOL bVisible, BOOL bShared, DWORD nCleanCookie, DWORD nSelectCookie, BOOL bRecurse);
	BOOL				Update(CAlbumFolder* pFolder, CLibraryTreeItem* pItem, CLibraryTreeItem* pParent, BOOL bVisible, DWORD nCleanCookie, DWORD nSelectCookie);

// Overrides
public:
	virtual BOOL Create(CWnd* pParentWnd);
	virtual BOOL PreTranslateMessage(MSG* pMsg);


// Implementation
protected:
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
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnUpdateLibraryParent(CCmdUI* pCmdUI);
	afx_msg void OnLibraryParent();
	afx_msg void OnUpdateLibraryExplore(CCmdUI* pCmdUI);
	afx_msg void OnLibraryExplore();
	afx_msg void OnUpdateLibraryScan(CCmdUI* pCmdUI);
	afx_msg void OnLibraryScan();
	afx_msg void OnUpdateLibraryShared(CCmdUI* pCmdUI);
	afx_msg void OnLibraryShared();
	afx_msg void OnLibraryAdd();
	afx_msg void OnUpdateLibraryRemove(CCmdUI* pCmdUI);
	afx_msg void OnLibraryRemove();
	afx_msg void OnUpdateLibraryFolderProperties(CCmdUI* pCmdUI);
	afx_msg void OnLibraryFolderProperties();
	afx_msg void OnUpdateLibraryFolderNew(CCmdUI* pCmdUI);
	afx_msg void OnLibraryFolderNew();
	afx_msg void OnUpdateLibraryFolderDelete(CCmdUI* pCmdUI);
	afx_msg void OnLibraryFolderDelete();
	afx_msg void OnUpdateLibraryFolderMetadata(CCmdUI* pCmdUI);
	afx_msg void OnLibraryFolderMetadata();
	afx_msg void OnUpdateLibraryFolderEnqueue(CCmdUI* pCmdUI);
	afx_msg void OnLibraryFolderEnqueue();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnUpdateLibraryFolderFileProperties(CCmdUI* pCmdUI);
	afx_msg void OnLibraryFolderFileProperties();
	afx_msg void OnUpdateLibraryRebuild(CCmdUI* pCmdUI);
	afx_msg void OnLibraryRebuild();
	afx_msg void OnUpdateLibraryExportCollection(CCmdUI *pCmdUI);
	afx_msg void OnLibraryExportCollection();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnUpdateLibraryCreateTorrent(CCmdUI* pCmdUI);
	afx_msg void OnLibraryCreateTorrent();

	DECLARE_MESSAGE_MAP()
	DECLARE_DROP()
};


class CLibraryTreeItem : public CObject
{
public:
	CLibraryTreeItem(CLibraryTreeItem* pParent = NULL, const CString& name = CString());

protected:
	CLibraryTreeItem* const m_pParent;
	typedef boost::ptr_list< CLibraryTreeItem > Container;
	Container m_oList;

public:
	typedef Container::iterator iterator;
	typedef Container::const_iterator const_iterator;
	typedef Container::reverse_iterator reverse_iterator;
	typedef Container::const_reverse_iterator const_reverse_iterator;

	CLibraryTreeItem*       parent()       { return m_pParent; }
	const CLibraryTreeItem* parent() const { return m_pParent; }

	iterator               begin()        { return m_oList.begin(); }
	const_iterator         begin()  const { return m_oList.begin(); }
	iterator               end()          { return m_oList.end(); }
	const_iterator         end()    const { return m_oList.end(); }
	reverse_iterator       rbegin()       { return m_oList.rbegin(); }
	const_reverse_iterator rbegin() const { return m_oList.rbegin(); }
	reverse_iterator       rend()         { return m_oList.rend(); }
	const_reverse_iterator rend()   const { return m_oList.rend(); }

	size_t size() const { return m_oList.size(); }
	size_t treeSize() const
	{
		size_t result = size();
		for ( const_iterator i = begin(); i != end(); ++i )
		{
			result += i->treeSize();
		}
		return result;
	}
	bool empty() const { return m_oList.empty(); }
	void clear() { m_oList.clear(); }
	CLibraryTreeItem* addItem(const CString& name);
	iterator erase(iterator item) { return m_oList.erase( item ); }

public:
	CLibraryTreeItem*	m_pSelPrev;
	CLibraryTreeItem*	m_pSelNext;
	DWORD				m_nCleanCookie;
	BOOL				m_bExpanded;
	BOOL				m_bSelected;
	BOOL				m_bContract1;
	BOOL				m_bContract2;
	CLibraryFolder*		m_pPhysical;
	CAlbumFolder*		m_pVirtual;
	DWORD				m_nCookie;
	CString				m_sText;
	BOOL				m_bBold;
	BOOL				m_bShared;
	BOOL				m_bCollection;
	int					m_nIcon16;

	BOOL				IsVisible() const;
	void				Paint(CDC& dc, CRect& rc, BOOL bTarget, COLORREF crBack = CLR_NONE) const;
	int					GetFileList(CLibraryList* pList, BOOL bRecursive = FALSE) const;
};

#define IDC_LIBRARY_TREE	131
#define LTN_SELCHANGED		101
