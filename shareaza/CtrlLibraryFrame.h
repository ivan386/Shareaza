//
// CtrlLibraryFrame.h
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

#if !defined(AFX_CTRLLIBRARYFRAME_H__C39BE20E_4746_4FA8_B96D_B94ADC1F4ABA__INCLUDED_)
#define AFX_CTRLLIBRARYFRAME_H__C39BE20E_4746_4FA8_B96D_B94ADC1F4ABA__INCLUDED_

#pragma once

#include "CtrlCoolBar.h"
#include "CtrlLibraryTreeView.h"
#include "CtrlLibraryHeaderBar.h"
#include "CtrlLibraryHeaderPanel.h"
#include "CtrlLibraryTip.h"
#include "CtrlSchemaCombo.h"

class CLibraryView;
class CLibraryPanel;


class CLibraryFrame : public CWnd
{
// Construction
public:
	CLibraryFrame();
	virtual ~CLibraryFrame();

	DECLARE_DYNAMIC(CLibraryFrame)

// Operations
public:
	void				OnSkinChange();
	BOOL				Update(BOOL bForce = TRUE, BOOL bBestView = TRUE);
	BOOL				Display(CLibraryFolder* pFolder);
	BOOL				Display(CAlbumFolder* pFolder);
	BOOL				Display(CLibraryFile* pFile);
	BOOL				Select(DWORD nObject);
	CLibraryTreeItem*	GetFolderSelection() const;
	CLibraryList*		GetViewSelection() const;

// Data
public:
	CLibraryTreeView	m_wndTree;
	CLibraryHeaderPanel	m_wndHeader;
	CCoolBarCtrl		m_wndTreeTop;
	CCoolBarCtrl		m_wndTreeBottom;
	CSchemaCombo		m_wndTreeTypes;
	CLibraryHeaderBar	m_wndViewTop;
	CCoolBarCtrl		m_wndViewBottom;
	CLibraryTipCtrl		m_wndViewTip;
	CEdit				m_wndSearch;
	CList< CLibraryView* >	m_pViews;
	CLibraryView*		m_pView;
	CList< CLibraryPanel* >	m_pPanels;
	CLibraryPanel*		m_pPanel;
	int					m_nTreeSize;
	int					m_nPanelSize;
	BOOL				m_bPanelShow;
	int					m_nHeaderSize;
	int					m_nTreeTypesHeight;
	BOOL				m_bUpdating;
	DWORD				m_nLibraryCookie;
	DWORD				m_nFolderCookie;
	CLibraryTreeItem*	m_pFolderSelection;
	CLibraryList*		m_pViewSelection;
	BOOL				m_bViewSelection;
	CLibraryList		m_pViewEmpty;

// Implementation
protected:
	BOOL		DoSizeTree();
	BOOL		DoSizePanel();
	void		UpdatePanel(BOOL bForce);
	void		SetView(CLibraryView* pView, BOOL bUpdate = TRUE, BOOL bUser = TRUE);
	void		SetPanel(CLibraryPanel* pPanel);
	void		RunLocalSearch(auto_ptr< CQuerySearch > pSearch);

// Overrides
public:
	//{{AFX_VIRTUAL(CLibraryFrame)
	virtual BOOL Create(CWnd* pParentWnd);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

// Implementation
protected:
    DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLibraryRefresh();
	afx_msg void OnUpdateLibraryTreePhysical(CCmdUI* pCmdUI);
	afx_msg void OnLibraryTreePhysical();
	afx_msg void OnUpdateLibraryTreeVirtual(CCmdUI* pCmdUI);
	afx_msg void OnLibraryTreeVirtual();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnUpdateLibraryPanel(CCmdUI* pCmdUI);
	afx_msg void OnLibraryPanel();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnLibrarySearch();
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnLibrarySearchQuick();
	afx_msg void OnTreeSelection(NMHDR* pNotify, LRESULT* pResult);
	afx_msg void OnViewSelection();
	afx_msg void OnFilterTypes();
	afx_msg void OnToolbarReturn();
	afx_msg void OnToolbarEscape();
	afx_msg void OnSetFocus(CWnd* pOldWnd);

	friend class CLibraryHeaderBar;
	friend class CLibraryHeaderPanel;
	friend class CLibraryView;
};

#endif // !defined(AFX_CTRLLIBRARYFRAME_H__C39BE20E_4746_4FA8_B96D_B94ADC1F4ABA__INCLUDED_)
