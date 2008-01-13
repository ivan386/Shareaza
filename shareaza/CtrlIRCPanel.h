//
// CtrlIRCPanel.h
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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
// Author: peer_l_@hotmail.com
//

#if !defined(AFX_CTRLIRCPANEL_H__INCLUDED_)
#define AFX_CTRLIRCPANEL_H__INCLUDED_

#pragma once

#include "CtrlRichTaskBox.h"
#include "CtrlCoolBar.h"
#include "CtrlIconButton.h"

static const int PANEL_WIDTH	    = 200;
static const int BOXUSERS_HEIGHT	= 240;
static const int BOXCHANS_MINHEIGHT = 90;
static const int BOX_VOFFSET		= 9;
static const int BOX_HOFFSET		= 9;
static const int BUTTON_HEIGHT	    = 24;
static const int PANELOFFSET_HEIGHT = 130;

class CIRCUsersBox : public CRichTaskBox
{

// Construction
public:
	CIRCUsersBox();
	virtual ~CIRCUsersBox();
	DECLARE_DYNAMIC(CIRCUsersBox)

// Attributes
public:
	CListBox			m_wndUserList;
	HCURSOR				m_hHand;
	CBitmap				m_bmWatermark;
	CDC					m_dcBuffer;
	CBitmap				m_bmBuffer;
	HBITMAP				m_hBuffer;
	CString				m_sCaption;
// Operations
public:
	void	Setup();
	void	UpdateCaptionCount();
// Implementation
protected:
	//{{AFX_MSG(CIRCUsersBox)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnUsersDoubleClick();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CIRCChannelsBox : public CRichTaskBox
{
// Construction
public:
	CIRCChannelsBox();
	virtual ~CIRCChannelsBox();
	DECLARE_DYNAMIC(CIRCChannelsBox)

// Attributes
public:
	CListCtrl			m_wndChanList;
	CIconButtonCtrl		m_wndAddChannel;
	CIconButtonCtrl		m_wndRemoveChannel;
	HCURSOR				m_hHand;
	CBitmap				m_bmWatermark;
	CDC					m_dcBuffer;
	CBitmap				m_bmBuffer;
	HBITMAP				m_hBuffer;
	CString				m_sPassedChannel;

// Operations
public:
	void	Setup();

// Implementation
protected:
	//{{AFX_MSG(CIRCChannelsBox)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnChansDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAddChannel();
	afx_msg void OnRemoveChannel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

typedef struct
{
	NMHDR			hdr;
} IRC_PANELEVENT;

class CIRCPanel : public CTaskPanel
{
// Construction
public:
	CIRCPanel();
	virtual ~CIRCPanel();
	DECLARE_DYNAMIC(CIRCPanel)

// Attributes
public:
	CIRCUsersBox	m_boxUsers;
	CIRCChannelsBox	m_boxChans;
private:
	CFont			m_pFont;

// Operations
public:
	void	Setup();

// Overrides
public:
	//{{AFX_VIRTUAL(CIRCPanel)
	virtual BOOL Create(CWnd* pParentWnd);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CIRCPanel)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#define IDC_IRC_PANEL	112
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
#endif // !defined(AFX_CTRLIRCPANEL_H__INCLUDED_)
