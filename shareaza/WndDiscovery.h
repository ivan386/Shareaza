//
// WndDiscovery.h
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

#if !defined(AFX_WNDDISCOVERY_H__F98780CE_5C8F_4933_A252_022FA84F706A__INCLUDED_)
#define AFX_WNDDISCOVERY_H__F98780CE_5C8F_4933_A252_022FA84F706A__INCLUDED_

#pragma once

#include "WndPanel.h"

class CDiscoveryService;


class CDiscoveryWnd : public CPanelWnd
{
// Construction
public:
	CDiscoveryWnd();
	virtual ~CDiscoveryWnd();

	DECLARE_SERIAL(CDiscoveryWnd)

// Attributes
public:
	CListCtrl		m_wndList;
	CImageList		m_gdiImageList;
	CLiveListSizer	m_pSizer;
	BOOL			m_bShowGnutella;
	BOOL			m_bShowWebCache;
	BOOL			m_bShowServerMet;

// Operations
public:
	void				Update();
	CDiscoveryService*	GetItem(int nItem);

// Overrides
public:
	//{{AFX_VIRTUAL(CDiscoveryWnd)
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CDiscoveryWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDblClkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSortList(NMHDR* pNotifyStruct, LRESULT *pResult);
	afx_msg void OnUpdateDiscoveryQuery(CCmdUI* pCmdUI);
	afx_msg void OnDiscoveryQuery();
	afx_msg void OnUpdateDiscoveryRemove(CCmdUI* pCmdUI);
	afx_msg void OnDiscoveryRemove();
	afx_msg void OnDiscoveryAdd();
	afx_msg void OnDiscoveryEdit();
	afx_msg void OnUpdateDiscoveryEdit(CCmdUI* pCmdUI);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnUpdateDiscoveryGnutella(CCmdUI* pCmdUI);
	afx_msg void OnDiscoveryGnutella();
	afx_msg void OnUpdateDiscoveryWebcache(CCmdUI* pCmdUI);
	afx_msg void OnDiscoveryWebcache();
	afx_msg void OnUpdateDiscoveryAdvertise(CCmdUI* pCmdUI);
	afx_msg void OnDiscoveryAdvertise();
	afx_msg void OnUpdateDiscoveryBrowse(CCmdUI* pCmdUI);
	afx_msg void OnDiscoveryBrowse();
	afx_msg void OnUpdateDiscoveryServerMet(CCmdUI* pCmdUI);
	afx_msg void OnDiscoveryServerMet();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#define IDC_SERVICES	100

#endif // !defined(AFX_WNDDISCOVERY_H__F98780CE_5C8F_4933_A252_022FA84F706A__INCLUDED_)
