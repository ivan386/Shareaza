//
// CtrlIRCPanel.h
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

#include "CtrlTaskPanel.h"
#include "CtrlCoolBar.h"
#include "CtrlIconButton.h"


class CIRCUsersBox : public CTaskBox
{
public:
	CIRCUsersBox();
	virtual ~CIRCUsersBox();
	DECLARE_DYNAMIC(CIRCUsersBox)

	CListBox			m_wndUserList;
	CBitmap				m_bmWatermark;
	CDC					m_dcBuffer;
	CBitmap				m_bmBuffer;
	HBITMAP				m_hBuffer;

	void	OnSkinChange();
	void	UpdateCaptionCount();

protected:
	int HitTest(const CPoint& pt) const;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnUsersDoubleClick();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg int OnCompareItem(int nIDCtl, LPCOMPAREITEMSTRUCT lpCompareItemStruct);

	DECLARE_MESSAGE_MAP()
};


class CIRCChannelsBox : public CTaskBox
{
public:
	CIRCChannelsBox();
	virtual ~CIRCChannelsBox();
	DECLARE_DYNAMIC(CIRCChannelsBox)

	CListCtrl			m_wndChanList;
	CIconButtonCtrl		m_wndAddChannel;
	CIconButtonCtrl		m_wndRemoveChannel;
	HCURSOR				m_hHand;
	CBitmap				m_bmWatermark;
	CDC					m_dcBuffer;
	HBITMAP				m_hBuffer;
	CString				m_sPassedChannel;

	void	OnSkinChange();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnChansDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAddChannel();
	afx_msg void OnRemoveChannel();

	DECLARE_MESSAGE_MAP()
};


typedef struct
{
	NMHDR			hdr;
} IRC_PANELEVENT;

class CIRCPanel : public CTaskPanel
{
public:
	CIRCPanel();
	virtual ~CIRCPanel();
	DECLARE_DYNAMIC(CIRCPanel)

	CIRCUsersBox	m_boxUsers;
	CIRCChannelsBox	m_boxChans;

	void	OnSkinChange();
	virtual BOOL Create(CWnd* pParentWnd);

protected:
	CFont			m_pFont;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()
};

#define	ID_COLOR_LISTTEXT				6
#define IDC_IRC_USERS					121
#define IDC_IRC_CHANNELS				122
#define IDC_IRC_DBLCLKCHANNELS			200
#define IDC_IRC_DBLCLKUSERS				201
#define	IDC_IRC_MENUUSERS				202
#define IDC_IRC_ADDCHANNEL				203
#define IDC_IRC_REMOVECHANNEL			204

#define	WM_REMOVECHANNEL				20933
#define	WM_ADDCHANNEL					20934
