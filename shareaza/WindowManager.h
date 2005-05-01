//
// WindowManager.h
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

#if !defined(AFX_WINDOWMANAGER_H__5598C2BE_CE10_4AEC_A807_B8157C41F12C__INCLUDED_)
#define AFX_WINDOWMANAGER_H__5598C2BE_CE10_4AEC_A807_B8157C41F12C__INCLUDED_

#pragma once

#include "WndChild.h"


class CWindowManager : public CWnd
{
// Construction
public:
	CWindowManager(CMDIFrameWnd* pParent = NULL);
	virtual ~CWindowManager();

// Attributes
public:
	CMDIFrameWnd*		m_pParent;
	CPtrList			m_pWindows;
	CRect				m_rcSize;
	BOOL				m_bIgnoreActivate;
	BOOL				m_bClosing;

// Operations
public:
	void		SetOwner(CMDIFrameWnd* pParent);
	CChildWnd*	GetActive() const;
	POSITION	GetIterator() const;
	CChildWnd*	GetNext(POSITION& pos) const;
	BOOL		Check(CChildWnd* pChild) const;
	CChildWnd*	Find(CRuntimeClass* pClass, CChildWnd* pAfter = NULL, CChildWnd* pExcept = NULL);
	CChildWnd*	Open(CRuntimeClass* pClass, BOOL bToggle = FALSE, BOOL bFocus = TRUE);
	CChildWnd*	FindFromPoint(const CPoint& point) const;
public:
	void		Close();
	void		AutoResize();
	void		Cascade(BOOL bActiveOnly = FALSE);
	void		SetGUIMode(int nMode, BOOL bSaveState = TRUE);
public:
	void		LoadWindowStates();
	void		SaveWindowStates();
	BOOL		LoadSearchWindows();
	void		SaveSearchWindows();
	void		OpenNewSearchWindow();
	void		PostSkinChange();
	void		PostSkinRemove();
protected:
	void		Add(CChildWnd* pChild);
	void		Remove(CChildWnd* pChild);
	void		ActivateGrouped(CChildWnd* pExcept);
	void		CreateTabbedWindows();

// Message Map
protected:
	//{{AFX_MSG(CWindowManager)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	friend class CChildWnd;
	friend class CPluginWnd;
};

#endif // !defined(AFX_WINDOWMANAGER_H__5598C2BE_CE10_4AEC_A807_B8157C41F12C__INCLUDED_)
