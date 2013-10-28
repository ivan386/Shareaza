//
// WindowManager.h
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

class CMainWnd;
class CChildWnd;


class CWindowManager : public CWnd
{
	DECLARE_DYNCREATE(CWindowManager)

public:
	CWindowManager();
	virtual ~CWindowManager();

	void		SetOwner(CMainWnd* pParent);
	CChildWnd*	GetActive() const;
	BOOL		IsEmpty() const { return m_pWindows.IsEmpty(); }
	POSITION	GetIterator() const;
	CChildWnd*	GetNext(POSITION& pos) const;
	BOOL		Check(CChildWnd* pChild) const;
	CChildWnd*	Find(CRuntimeClass* pClass, CChildWnd* pAfter = NULL, CChildWnd* pExcept = NULL);
	CChildWnd*	Open(CRuntimeClass* pClass, BOOL bToggle = FALSE, BOOL bFocus = TRUE);
	CChildWnd*	FindFromPoint(const CPoint& point) const;
	void		Close();
	void		AutoResize();
	void		Cascade(BOOL bActiveOnly = FALSE);
	void		SetGUIMode(int nMode, BOOL bSaveState = TRUE);
	void		LoadWindowStates();
	void		SaveWindowStates() const;
	BOOL		LoadSearchWindows();
	BOOL		SaveSearchWindows() const;
	BOOL		LoadBrowseHostWindows();
	BOOL		SaveBrowseHostWindows() const;
	void		OpenNewSearchWindow();
	void		PostSkinChange();
	void		PostSkinRemove();

protected:
	CMainWnd*	m_pParent;
	CRect		m_rcSize;
	BOOL		m_bIgnoreActivate;
	CList< CChildWnd* >	m_pWindows;

	void		Add(CChildWnd* pChild);
	void		Remove(CChildWnd* pChild);
	void		ActivateGrouped(CChildWnd* pExcept);

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();

	DECLARE_MESSAGE_MAP()

	friend class CChildWnd;
	friend class CPluginWnd;
};
