//
// CtrlChatFrame.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

#if !defined(AFX_CTRLCHATFRAME_H__2244C71A_1F23_4D0E_B360_5E551CBAC37D__INCLUDED_)
#define AFX_CTRLCHATFRAME_H__2244C71A_1F23_4D0E_B360_5E551CBAC37D__INCLUDED_

#pragma once

#include "RichViewCtrl.h"
#include "RichDocument.h"
#include "CtrlCoolBar.h"

class CChatSession;
class CChatWnd;


class CChatFrame : public CWnd
{
// Construction
public:
	CChatFrame();
	virtual ~CChatFrame();

	DECLARE_DYNAMIC(CChatFrame)

// Attributes
public:
	CChatSession*	m_pSession;
protected:
	CRichDocument	m_pContent;
	CRichViewCtrl	m_wndView;
	CCoolBarCtrl	m_wndToolBar;
	CEdit			m_wndEdit;
	CMenu*			m_pIconMenu;
protected:
	CArray< CString >	m_pHistory;
	int				m_nHistory;
	CString			m_sCurrent;
protected:
	CChatWnd*		m_pChildWnd;
	CWnd*			m_pDesktopWnd;

// Operations
public:
	void			SetDesktopMode(BOOL bDesktop);
protected:
	void			SetAlert(BOOL bAlert = TRUE);
	void			MoveHistory(int nDelta);
	BOOL			IsInRange(LPCTSTR pszToken);
	void			InsertText(LPCTSTR pszToken);
	void			AddText(LPCTSTR pszText);
	void			AddText(BOOL bSelf, BOOL bAction, LPCTSTR pszNick, LPCTSTR pszBody);
public:
	virtual void	OnSkinChange();
	virtual void	OnStatusMessage(int nFlags, LPCTSTR pszText);
	virtual void	OnLocalText(LPCTSTR pszText);
	virtual void	OnLocalMessage(BOOL bAction, LPCTSTR pszText);
	virtual void	OnLocalCommand(LPCTSTR pszText, LPCTSTR pszCommand);

// Overrides
public:
	//{{AFX_VIRTUAL(CChatFrame)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CChatFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnUpdateChatBold(CCmdUI* pCmdUI);
	afx_msg void OnChatBold();
	afx_msg void OnUpdateChatItalic(CCmdUI* pCmdUI);
	afx_msg void OnChatItalic();
	afx_msg void OnUpdateChatUnderline(CCmdUI* pCmdUI);
	afx_msg void OnChatUnderline();
	afx_msg void OnChatColour();
	afx_msg void OnUpdateChatConnect(CCmdUI* pCmdUI);
	afx_msg void OnChatConnect();
	afx_msg void OnUpdateChatDisconnect(CCmdUI* pCmdUI);
	afx_msg void OnChatDisconnect();
	afx_msg void OnChatClear();
	afx_msg void OnChatEmoticons();
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnClickView(NMHDR* pNotify, LRESULT *pResult);
	afx_msg void OnUpdateChatTimestamp(CCmdUI* pCmdUI);
	afx_msg void OnChatTimestamp();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#define IDC_CHAT_TEXT	100
#define IDC_CHAT_EDIT	101

#endif // !defined(AFX_CTRLCHATFRAME_H__2244C71A_1F23_4D0E_B360_5E551CBAC37D__INCLUDED_)
