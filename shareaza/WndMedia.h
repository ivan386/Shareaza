//
// WndMedia.h
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

#if !defined(AFX_WNDMEDIA_H__33CD05D9_37E1_4925_A4C0_934318FE224E__INCLUDED_)
#define AFX_WNDMEDIA_H__33CD05D9_37E1_4925_A4C0_934318FE224E__INCLUDED_

#pragma once

#include "WndPanel.h"
#include "CtrlMediaFrame.h"


class CMediaWnd : public CPanelWnd
{
// Construction
public:
	CMediaWnd();
	virtual ~CMediaWnd();

	DECLARE_SERIAL(CMediaWnd)

// Attributes
protected:
	CMediaFrame	m_wndFrame;

// Operations
public:
	virtual void OnSkinChange();
	virtual BOOL OnDropFiles(CStringList& pFiles, const CPoint& ptScreen, BOOL bDrop);
public:
	virtual BOOL PlayFile(LPCTSTR pszFile);
	virtual BOOL EnqueueFile(LPCTSTR pszFile);
	virtual BOOL IsPlaying();
	virtual void OnFileDelete(LPCTSTR pszFile);

// Overrides
public:
	//{{AFX_VIRTUAL(CMediaWnd)
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CMediaWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	//}}AFX_MSG

	afx_msg LONG OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
	afx_msg LONG OnMediaKey(WPARAM wParam, LPARAM lParam);
	afx_msg LONG OnDevModeChange(WPARAM wParam, LPARAM lParam);
	afx_msg LONG OnDisplayChange(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_WNDMEDIA_H__33CD05D9_37E1_4925_A4C0_934318FE224E__INCLUDED_)

