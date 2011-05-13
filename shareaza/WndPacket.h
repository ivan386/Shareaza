//
// WndPacket.h
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

#include "WndPanel.h"
#include "BTPacket.h"
#include "G1Packet.h"
#include "G2Packet.h"
#include "DCPacket.h"
#include "EDPacket.h"

class CLiveItem;
class CNeighbour;
class CPacket;
class CCoolMenu;

const int		nTypeG1Size = G1_PACKTYPE_MAX;
const int		nTypeG2Size = 22;

class CPacketWnd : public CPanelWnd
{
	DECLARE_SERIAL(CPacketWnd)

public:
	CPacketWnd(CChildWnd* pOwner = NULL);
	virtual ~CPacketWnd();

	void SmartDump(const CPacket* pPacket, const SOCKADDR_IN* pAddress, BOOL bUDP, BOOL bOutgoing, DWORD_PTR nNeighbourUnique = 0);
	virtual void OnSkinChange();

	CChildWnd*			m_pOwner;
	DWORD_PTR			m_nInputFilter;
	DWORD_PTR			m_nOutputFilter;
	BOOL				m_bPaused;

protected:
	BOOL				m_bTCP;
	BOOL				m_bUDP;
	BOOL				m_bTypeG1[nTypeG1Size];
	BOOL				m_bTypeG2[nTypeG2Size];
	BOOL				m_bTypeED;
	BOOL				m_bTypeBT;
	BOOL				m_bTypeDC;
	CListCtrl			m_wndList;
	CImageList			m_gdiImageList;
	CLiveListSizer		m_pSizer;
	CFont				m_pFont;
	CCoolMenu*			m_pCoolMenu;
	CList< CLiveItem* >	m_pQueue;
	CCriticalSection	m_pSection;
	static G2_PACKET	m_nG2[nTypeG2Size];

	void AddNeighbour(CMenu* pMenus, int nGroup, UINT nID, DWORD_PTR nTarget, LPCTSTR pszText);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCustomDrawList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnUpdateSystemClear(CCmdUI* pCmdUI);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnUpdateBlocker(CCmdUI* pCmdUI);

	DECLARE_MESSAGE_MAP()
};

#define IDC_PACKETS		100
