//
// CtrlBrowseFrame.h
//
// Copyright (c) Shareaza Development Team, 2002-2006.
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
#include "CtrlBrowseTree.h"
#include "CtrlSearchDetailPanel.h"

class CMatchCtrl;
class CG2Packet;
class CQueryHit;
class CBrowseTreeItem;


class CBrowseFrameCtrl : public CWnd
{
// Construction
public:
	CBrowseFrameCtrl();
	virtual ~CBrowseFrameCtrl();

	DECLARE_DYNAMIC(CBrowseFrameCtrl)

// Attributes
protected:
	CCoolBarCtrl		m_wndTreeTop;
	CBrowseTreeCtrl		m_wndTree;
	CSearchDetailPanel	m_wndDetails;
	CMatchCtrl*			m_wndList;
	BOOL				m_bTreeVisible;
	int					m_nTreeSize;
	BOOL				m_bPanelEnable;
	BOOL				m_bPanelVisible;
	int					m_nPanelSize;
	CG2Packet*			m_pTree[2];
	int					m_nTree;

// Operations
public:
	void			Serialize(CArchive& ar, int nVersion /* BROWSER_SER_VERSION */);
	virtual BOOL	Create(CWnd* pParentWnd, CMatchCtrl* pMatch);
	void			OnSkinChange();
	void			OnPhysicalTree(CG2Packet* pPacket);
	void			OnVirtualTree(CG2Packet* pPacket);
	void			OnSelChangeMatches();
protected:
	BOOL			DoSizeTree();
	BOOL			DoSizePanel();
	void			SelectTree(CBrowseTreeItem* pItem, CQueryHit* pHit);

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnUpdateSearchDetails(CCmdUI* pCmdUI);
	afx_msg void OnSearchDetails();
	afx_msg void OnUpdateLibraryTreePhysical(CCmdUI *pCmdUI);
	afx_msg void OnLibraryTreePhysical();
	afx_msg void OnUpdateLibraryTreeVirtual(CCmdUI *pCmdUI);
	afx_msg void OnLibraryTreeVirtual();
	afx_msg void OnTreeSelection(NMHDR* pNotify, LRESULT* pResult);

};

#define IDC_BROWSE_FRAME	110
