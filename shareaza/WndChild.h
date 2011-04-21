//
// WndChild.h
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

#include "CtrlCoolBar.h"
#include "LiveListSizer.h"

class CBuffer;
class CConnection;
class CLibraryFile;
class CMainWnd;
class CQueryHit;
class CQuerySearch;
class CSkinWindow;
class CWindowManager;


class CChildWnd : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CChildWnd)

public:
	CChildWnd();

	BOOL			m_bTabMode;
	BOOL			m_bGroupMode;
	CChildWnd*		m_pGroupParent;
	float			m_nGroupSize;
	BOOL			m_bPanelMode;
	BOOL			m_bAlert;

	void			GetWindowText(CString& rString);
	void			SetWindowText(LPCTSTR lpszString);
	CMainWnd*		GetMainWnd();
	CWindowManager*	GetManager();
	BOOL			IsActive(BOOL bFocused = FALSE);
	BOOL			IsPartiallyVisible();
	BOOL			TestPoint(const CPoint& ptScreen);
	BOOL			LoadState(LPCTSTR pszName = NULL, BOOL bDefaultMaximise = TRUE);
	BOOL			SaveState(LPCTSTR pszName = NULL);
	BOOL			SetAlert(BOOL bAlert = TRUE);
	void			SizeListAndBar(CWnd* pList, CWnd* pBar);
	void			RemoveSkin();

	// Notify window about skin change
	virtual void	OnSkinChange();
	// Notify window about arrived query search
	virtual void	OnQuerySearch(const CQuerySearch* /*pSearch*/) {}
	// Notify window about arrived query hits
	virtual BOOL	OnQueryHits(const CQueryHit* /*pHits*/) { return FALSE; }
	// Notify window about security rules changed
	virtual void	SanityCheck() {}
	// Notify window about new push connection available
	virtual BOOL	OnPush(const Hashes::Guid& /*pClientID*/, CConnection* /*pConnection*/) { return FALSE; }
	// Notify window about new library file (return TRUE to cancel event route)
	virtual BOOL	OnNewFile(CLibraryFile* /*pFile*/) { return FALSE; }

	virtual HRESULT	GetGenericView(IGenericView** ppView);
	virtual BOOL	DestroyWindow();

private:
	CString			m_sCaption;
	CMainWnd*		m_pMainWndCache;
	static CChildWnd* m_pCmdMsg;

protected:
	UINT			m_nResID;
	CSkinWindow*	m_pSkin;

	virtual BOOL	Create(UINT nID, BOOL bVisible = TRUE);
	virtual BOOL	OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL	PreTranslateMessage(MSG* pMsg);

	afx_msg int		OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void	OnDestroy();
	afx_msg BOOL	OnEraseBkgnd(CDC* pDC);
	afx_msg void	OnSize(UINT nType, int cx, int cy);
	afx_msg void	OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void	OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	afx_msg void	OnNcRButtonUp(UINT nHitTest, CPoint point);
	afx_msg void	OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg LRESULT	OnNcHitTest(CPoint point);
	afx_msg void	OnNcPaint();
	afx_msg BOOL	OnNcActivate(BOOL bActive);
	afx_msg void	OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void	OnNcLButtonUp(UINT nHitTest, CPoint point);
	afx_msg void	OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void	OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg BOOL	OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg LRESULT	OnSetText(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};
