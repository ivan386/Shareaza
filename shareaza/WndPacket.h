//
// WndPacket.h
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

#if !defined(AFX_WNDPACKET_H__83132A9A_A3A0_4850_A3D9_DCF825D2F273__INCLUDED_)
#define AFX_WNDPACKET_H__83132A9A_A3A0_4850_A3D9_DCF825D2F273__INCLUDED_

#pragma once

#include "WndPanel.h"

class CNeighbour;
class CPacket;
class CCoolMenu;


class CPacketWnd : public CPanelWnd
{
// Construction
public:
	CPacketWnd(CChildWnd* pOwner = NULL);
	virtual ~CPacketWnd();

	DECLARE_SERIAL(CPacketWnd)

// Attributes
public:
	CChildWnd*		m_pOwner;
	DWORD			m_nInputFilter;
	DWORD			m_nOutputFilter;
	BOOL			m_bTypeG1[16];
	BOOL			m_bTypeG2[64];
	BOOL			m_bPaused;
protected:
	CListCtrl			m_wndList;
	CLiveListSizer		m_pSizer;
	CFont				m_pFont;
	CCoolMenu*			m_pCoolMenu;
	CPtrList			m_pQueue;
	CCriticalSection	m_pSection;

	static LPCSTR m_pszG2[];
		
// Operations
public:
	void		Process(const CNeighbour* pNeighbour, const IN_ADDR* pUDP, BOOL bOutgoing, const CPacket* pPacket);
protected:
	void		AddNeighbour(CMenu* pMenus, int nGroup, UINT nID, DWORD nTarget, LPCTSTR pszText);

// Overrides
public:
	//{{AFX_VIRTUAL(CPacketWnd)
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CPacketWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCustomDrawList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnUpdateSystemClear(CCmdUI* pCmdUI);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	afx_msg void OnUpdateBlocker(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#define IDC_PACKETS		100

#endif // !defined(AFX_WNDPACKET_H__83132A9A_A3A0_4850_A3D9_DCF825D2F273__INCLUDED_)
