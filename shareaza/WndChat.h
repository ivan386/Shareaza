//
// WndChat.h
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
#include "RichElement.h"
#include "RichDocument.h"
#include "RichViewCtrl.h"
#include "CtrlCoolBar.h"


class CChatWnd : public CPanelWnd
{
	DECLARE_DYNAMIC(CChatWnd)

public:
	CChatWnd();
	virtual ~CChatWnd();

	void Open();

private:
	CCoolBarCtrl		m_wndToolBar;
	CRichDocument		m_pContent;
	CRichViewCtrl		m_wndView;
	CEdit				m_wndEdit;
	CArray< CString >	m_pHistory;
	int					m_nHistory;

	void MoveHistory(int nDelta);
	BOOL IsInRange(LPCTSTR pszToken);
	void InsertText(LPCTSTR pszToken);
	void AddTimestamp();
	BOOL OnLocalText(const CString& sText);

protected:
	void AddLogin(LPCTSTR pszText);
	void AddBitmap(HBITMAP hBitmap);
	void AddText(bool bAction, bool bOutgoing, LPCTSTR pszNick, LPCTSTR pszBody);
	void OnMessage(bool bAction, const CString& sChatID, bool bOutgoing, const CString& sFrom, const CString& sTo, const CString& sText);

	virtual void OnSkinChange();

	virtual void OnStatusMessage(int nFlags, const CString& sText);
	virtual BOOL OnLocalCommand(const CString& sCommand, const CString& sArgs);
	virtual BOOL OnLocalMessage(bool bAction, const CString& sText) = 0;

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnUpdateChatBold(CCmdUI* pCmdUI);
	afx_msg void OnChatBold();
	afx_msg void OnUpdateChatItalic(CCmdUI* pCmdUI);
	afx_msg void OnChatItalic();
	afx_msg void OnUpdateChatUnderline(CCmdUI* pCmdUI);
	afx_msg void OnChatUnderline();
	afx_msg void OnChatColour();
	afx_msg void OnChatClear();
	afx_msg void OnChatEmoticons();
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnClickView(NMHDR* pNotify, LRESULT *pResult);
	afx_msg void OnUpdateChatTimestamp(CCmdUI* pCmdUI);
	afx_msg void OnChatTimestamp();
	afx_msg void OnEmoticons(UINT nID);

	DECLARE_MESSAGE_MAP()
};

#define IDC_CHAT_TEXT	100
#define IDC_CHAT_EDIT	101

#define NEWLINE_FORMAT	_T("2")
#define EDIT_HISTORY	256
#define EDIT_HEIGHT		32
#define TOOLBAR_HEIGHT	30
