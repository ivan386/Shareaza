//
// WndChat.h
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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


enum MessageType
{
	cmtNull,
	cmtMessage,	// Chat message
	cmtAction,	// Chat action
	cmtStatus,	// Regular status message (gray)
	cmtError,	// Error message (red)
	cmtInfo,	// Informational message (black)
	cmtProfile,	// Profile received
	cmtCaption	// Set chat window caption
};

enum UserType
{
	cutMe,
	cutUser,
	cutOperator
};


class CChatMessage
{
public:
	inline CChatMessage(MessageType bType = cmtNull, const CString& sFrom = CString(), const CString& sMessage = CString(), HBITMAP hBitmap = NULL)
		: m_bType( bType ), m_sFrom( sFrom ), m_sMessage( sMessage ), m_hBitmap( hBitmap ) {}

	inline CChatMessage(const CChatMessage& pMsg)
		: m_bType( pMsg.m_bType ), m_sFrom( pMsg.m_sFrom ), m_sMessage( pMsg.m_sMessage ), m_hBitmap( pMsg.m_hBitmap ) {}

	inline CChatMessage& operator=(const CChatMessage& pMsg)
		{ m_bType = pMsg.m_bType; m_sFrom = pMsg.m_sFrom; m_sMessage = pMsg.m_sMessage; m_hBitmap = pMsg.m_hBitmap; return *this; }

	MessageType	m_bType;
	CString		m_sFrom;
	CString		m_sMessage;
	HBITMAP		m_hBitmap;
};


class CChatUser
{
public:
	inline CChatUser(UserType bType = cutUser, const CString& sNick = CString(), const CString& sDescription = CString())
		: m_bType( bType ), m_sNick( sNick ), m_sDescription( sDescription ) {}

	inline CChatUser(const CChatUser& pUser)
		: m_bType( pUser.m_bType ), m_sNick( pUser.m_sNick ), m_sDescription( pUser.m_sDescription ), m_sUserAgent( pUser.m_sUserAgent ) {}

	inline CChatUser& operator=(const CChatUser& pUser)
		{ m_bType = pUser.m_bType; m_sNick = pUser.m_sNick; m_sDescription = pUser.m_sDescription; m_sUserAgent = pUser.m_sUserAgent; return *this;}

	typedef CMap< CString, const CString&, CChatUser*, CChatUser* > Map;
	typedef CList< CChatUser > List;

	UserType	m_bType;
	CString		m_sNick;
	CString		m_sDescription;
	CString		m_sUserAgent;
};


class CChatWnd : public CPanelWnd
{
	DECLARE_DYNAMIC(CChatWnd)

public:
	CChatWnd();
	virtual ~CChatWnd();

	void Open();

private:
	CImageList			m_gdiImageList;
	CCoolBarCtrl		m_wndToolBar;
	CRichDocument		m_pContent;
	CRichViewCtrl		m_wndView;
	CEdit				m_wndEdit;
	CListCtrl			m_wndUsers;		// Chat users list
	int					m_nUsersSize;	// Width of chat users panel (pixels)
	CArray< CString >	m_pHistory;
	int					m_nHistory;
	CString				m_sCaption;

	void MoveHistory(int nDelta);
	BOOL IsInRange(LPCTSTR pszToken);
	void InsertText(LPCTSTR pszToken);
	void AddTimestamp();
	BOOL OnLocalText(const CString& sText);

protected:
	virtual CString GetChatID() const = 0;
	virtual CString GetCaption() const = 0;

	void AddLogin(LPCTSTR pszText);
	void AddBitmap(HBITMAP hBitmap);
	void AddText(bool bAction, bool bOutgoing, LPCTSTR pszNick, LPCTSTR pszBody);
	void OnMessage(bool bAction, const CString& sChatID, bool bOutgoing, const CString& sFrom, const CString& sTo, const CString& sText);
	BOOL DoSizeView();
	void DeleteAllUsers();

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
	afx_msg void OnPaint();
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
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnChatMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnChatAddUser(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnChatDeleteUser(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};

#define IDC_CHAT_TEXT	100
#define IDC_CHAT_EDIT	101
#define IDC_CHAT_USERS	102

#define WM_CHAT_MESSAGE				(WM_APP+70)	// (WPARAM: unused, LPARAM: CChatMessage* pMsg)
#define WM_CHAT_ADD_USER			(WM_APP+71)	// (WPARAM: unused, LPARAM: CChatUser* pAddUser)
#define WM_CHAT_DELETE_USER			(WM_APP+72)	// (WPARAM: unused, LPARAM: CString* pDeleteUser). pDeleteUser == NULL - delete all users.
