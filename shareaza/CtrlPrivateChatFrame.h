//
// CtrlPrivateChatFrame.h"
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

#if !defined(AFX_CTRLPRIVATECHATFRAME_H__63B46831_EC54_47CF_A276_A2F281A2EBBF__INCLUDED_)
#define AFX_CTRLPRIVATECHATFRAME_H__63B46831_EC54_47CF_A276_A2F281A2EBBF__INCLUDED_

#pragma once

#include "CtrlChatFrame.h"
#include "GProfile.h"

class CChatSession;


class CPrivateChatFrame : public CChatFrame
{
// Construction
public:
	CPrivateChatFrame();
	virtual ~CPrivateChatFrame();

	DECLARE_DYNAMIC(CPrivateChatFrame)

// Attributes
public:
	CString		m_sNick;

// Operations
public:
	void	Initiate(GGUID* pGUID, SOCKADDR_IN* pHost, BOOL bMustPush);
	BOOL	Accept(CChatSession* pSession);
public:
	virtual void	OnLocalMessage(BOOL bAction, LPCTSTR pszText);
	virtual void	OnLocalCommand(LPCTSTR pszCommand, LPCTSTR pszArgs);
	virtual void	OnProfileReceived();
	virtual void	OnRemoteMessage(BOOL bAction, LPCTSTR pszText);

// Overrides
public:
	//{{AFX_VIRTUAL(CPrivateChatFrame)
	//}}AFX_VIRTUAL

	virtual void OnSkinChange();

// Implementation
protected:
	//{{AFX_MSG(CPrivateChatFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnUpdateChatBrowse(CCmdUI* pCmdUI);
	afx_msg void OnChatBrowse();
	afx_msg void OnUpdateChatPriority(CCmdUI* pCmdUI);
	afx_msg void OnChatPriority();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_CTRLPRIVATECHATFRAME_H__63B46831_EC54_47CF_A276_A2F281A2EBBF__INCLUDED_)
