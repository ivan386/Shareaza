//
// WndPrivateChat.h
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

#include "WndChat.h"

class CChatSession;


class CPrivateChatWnd : public CChatWnd
{
	DECLARE_DYNAMIC(CPrivateChatWnd)

public:
	CPrivateChatWnd();
	virtual ~CPrivateChatWnd();


	void	Setup(LPCTSTR szNick);
	void	Setup(const Hashes::Guid& oGUID, const SOCKADDR_IN* pHost, BOOL bMustPush, PROTOCOLID nProtocol);

	BOOL	Accept(CChatSession* pSession);
	BOOL	Find(const SOCKADDR_IN* pAddress) const;
	BOOL	Find(const Hashes::Guid& oGUID, bool bLive) const;
	BOOL	Find(const CString& sNick) const;

protected:
	CChatSession*	m_pSession;
	CString			m_sNick;

	virtual CString GetChatID() const;
	virtual CString GetCaption() const;

	virtual BOOL OnLocalMessage(bool bAction, const CString& sText);
	virtual BOOL OnLocalCommand(const CString& sCommand, const CString& sArgs);

	afx_msg void OnDestroy();
	afx_msg void OnUpdateChatBrowse(CCmdUI* pCmdUI);
	afx_msg void OnUpdateChatConnect(CCmdUI* pCmdUI);
	afx_msg void OnChatConnect();
	afx_msg void OnUpdateChatDisconnect(CCmdUI* pCmdUI);
	afx_msg void OnChatDisconnect();
	afx_msg void OnChatBrowse();
	afx_msg void OnUpdateChatPriority(CCmdUI* pCmdUI);
	afx_msg void OnChatPriority();

	DECLARE_MESSAGE_MAP()
};
