//
// CtrlLibraryTreeView.h
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

#if !defined(AFX_CTRLLIBRARYTREEVIEW_H__C1190F39_DB7E_4407_A958_1D3E90EF7318__INCLUDED_)
#define AFX_CTRLLIBRARYTREEVIEW_H__C1190F39_DB7E_4407_A958_1D3E90EF7318__INCLUDED_

#pragma once

#include "CtrlLibraryTree.h"
#include "CtrlTipFolder.h"
#include "CtrlTipAlbum.h"


class CLibraryTreeView : public CLibraryTreeCtrl
{
// Construction
public:
	CLibraryTreeView();
	virtual ~CLibraryTreeView();

	DECLARE_DYNAMIC(CLibraryTreeView)

// Attributes
public:
	BOOL			m_bVirtual;
	CFolderTipCtrl	m_wndFolderTip;
	CAlbumTipCtrl	m_wndAlbumTip;

// Operations
public:
	void	SetVirtual(BOOL bVirtual);
	void	Update(DWORD nSelectCookie);
	BOOL	SelectFolder(LPVOID pSearch);
	BOOL	DropShowTarget(CLibraryList* pList, const CPoint& point);
	BOOL	DropObjects(CLibraryList* pList, BOOL bCopy, CSingleLock& oLock);
protected:
	void	PostUpdate();
	void	UpdatePhysical(DWORD nSelectCookie);
	void	UpdateVirtual(DWORD nSelectCookie);
	BOOL	Update(CLibraryFolder* pFolder, CLibraryTreeItem* pItem, CLibraryTreeItem* pParent, BOOL bVisible, BOOL bShared, DWORD nCleanCookie, DWORD nSelectCookie, BOOL bRecurse);
	BOOL	Update(CAlbumFolder* pFolder, CLibraryTreeItem* pItem, CLibraryTreeItem* pParent, BOOL bVisible, DWORD nCleanCookie, DWORD nSelectCookie);

// Overrides
public:
	//{{AFX_VIRTUAL(CLibraryTreeView)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CLibraryTreeView)
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
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
	afx_msg void OnUpdateLibraryFolderFileProperties(CCmdUI* pCmdUI);
	afx_msg void OnLibraryFolderFileProperties();
	afx_msg void OnUpdateLibraryRebuild(CCmdUI* pCmdUI);
	afx_msg void OnLibraryRebuild();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnUpdateLibraryExportCollection(CCmdUI *pCmdUI);
	afx_msg void OnLibraryExportCollection();
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_CTRLLIBRARYTREEVIEW_H__C1190F39_DB7E_4407_A958_1D3E90EF7318__INCLUDED_)
