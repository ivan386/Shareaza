//
// CtrlLibraryFrame.h
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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

#include "CtrlCoolBar.h"
#include "CtrlLibraryTreeView.h"
#include "CtrlLibraryHeaderBar.h"
#include "CtrlLibraryHeaderPanel.h"
#include "CtrlLibraryTip.h"
#include "CtrlLibraryMetaPanel.h"
#include "CtrlLibraryHistoryPanel.h"
#include "CtrlSchemaCombo.h"
#include "AutocompleteEdit.h"

class CLibraryView;
class CPanelCtrl;
class CMetaList;

class CLibraryFrame : public CWnd
{
public:
	CLibraryFrame();
	virtual ~CLibraryFrame();

	DECLARE_DYNAMIC(CLibraryFrame)

	typedef CList< CLibraryView* > CLibraryViewList;

	virtual BOOL	Create(CWnd* pParentWnd);
	virtual BOOL	OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	void			OnViewSelection();
	void			SetView(CLibraryView* pView, BOOL bUpdate = TRUE, BOOL bUser = TRUE);
	void			OnSkinChange();
	BOOL			Update(BOOL bForce = TRUE, BOOL bBestView = TRUE);
	BOOL			Display(const CLibraryFolder* pFolder);
	BOOL			Display(const CAlbumFolder* pFolder);
	BOOL			Display(const CLibraryFile* pFile);
	BOOL			Select(DWORD nObject);
	BOOL			SetDynamicBar(LPCTSTR pszName);
 	void			HideDynamicBar();
	void			SetPanelData(CMetaList* pPanel);	// Sets new metadata to display which can be saved later
	CMetaList*		GetPanelData();

	inline LPCTSTR GetDynamicBarName() const
	{
		return (LPCTSTR)m_sDynamicBarName;
	}

	inline CLibraryTreeItem* GetFolderSelection() const
	{
		return m_wndTree.GetFirstSelected();
	}

	inline CLibraryList* GetViewSelection() const
	{
		return m_pViewSelection;
	}

	inline CCoolBarCtrl* GetViewTop()
	{
		return &m_wndViewTop;
	}

	inline CLibraryTipCtrl* GetToolTip()
	{
		return &m_wndViewTip;
	}

	inline void SetSearchText(LPCTSTR pszSearch)
	{
		m_wndSearch.SetWindowText( pszSearch );
	}

	inline CLibraryViewList* GetViews()
	{
		return &m_pViews;
	}

	inline DWORD GetFolderCookie() const
	{
		return m_nFolderCookie;
	}

	inline void Switch()
	{
		if ( Settings.Library.ShowVirtual )
			OnLibraryTreePhysical();
		else
			OnLibraryTreeVirtual();
	}

protected:
	CLibraryTreeView	m_wndTree;
	CLibraryHeaderPanel	m_wndHeader;
	CCoolBarCtrl		m_wndTreeTop;
	CCoolBarCtrl		m_wndTreeBottom;
	CSchemaCombo		m_wndTreeTypes;
	CLibraryTipCtrl		m_wndViewTip;
	CLibraryHeaderBar	m_wndViewTop;
	CCoolBarCtrl		m_wndViewBottom;
	CCoolBarCtrl		m_wndBottomDynamic;
	CAutocompleteEdit	m_wndSearch;
	CLibraryViewList	m_pViews;
	CLibraryView*		m_pView;
	CLibraryMetaPanel	m_pMetaPanel;
	CLibraryHistoryPanel	m_pHistoryPanel;
	CPanelCtrl*			m_pPanel;
	int					m_nTreeSize;
	int					m_nPanelSize;
	BOOL				m_bPanelShow;
	int					m_nHeaderSize;
	int					m_nTreeTypesHeight;
	BOOL				m_bUpdating;
	DWORD				m_nLibraryCookie;
	DWORD				m_nFolderCookie;
	CLibraryListPtr		m_pViewSelection;
	BOOL				m_bViewSelection;
	CLibraryListPtr		m_pViewEmpty;

	CString				m_sDynamicBarName;
	BOOL				m_bShowDynamicBar;
	BOOL				m_bDynamicBarHidden;

	BOOL		HasView() const;
	BOOL		HasPanel() const;
	BOOL		DoSizeTree();
	BOOL		DoSizePanel();
	void		UpdatePanel(BOOL bForce);
	void		SetPanel(CPanelCtrl* pPanel);
	void		RunLocalSearch(CQuerySearch* pSearch);

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
	afx_msg void OnUpdateShowWebServices(CCmdUI* pCmdUI);
	afx_msg void OnShowWebServices();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnLibrarySearch();
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnLibrarySearchQuick();
	afx_msg void OnTreeSelection(NMHDR* pNotify, LRESULT* pResult);
	afx_msg void OnFilterTypes();
	afx_msg void OnToolbarReturn();
	afx_msg void OnToolbarEscape();
	afx_msg void OnSetFocus(CWnd* pOldWnd);

	DECLARE_MESSAGE_MAP()
};
