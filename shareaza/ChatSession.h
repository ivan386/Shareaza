//
// ChatSession.h
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

#if !defined(AFX_CHATSESSION_H__F75BAA22_513A_4569_9BFB_90A053662CDB__INCLUDED_)
#define AFX_CHATSESSION_H__F75BAA22_513A_4569_9BFB_90A053662CDB__INCLUDED_

#pragma once

#include "GUID.h"
#include "Connection.h"

class CG2Packet;
class CGProfile;
class CChatFrame;
class CPrivateChatFrame;


class CChatSession : public CConnection
{
// Construction
public:
	CChatSession(CChatFrame* pFrame = NULL);
	virtual ~CChatSession();
	
// Attributes
public:
	BOOL		m_bGUID;
	CGUID		m_pGUID;
public:
	int			m_nState;
	BOOL		m_bG2;
	BOOL		m_bOld;
	BOOL		m_bMustPush;
	DWORD		m_tPushed;
	CString		m_sUserAgent;
	CString		m_sUserNick;
	CGProfile*	m_pProfile;
public:
	CPrivateChatFrame*	m_pWndPrivate;
	CWnd*				m_pWndPublic;
	
// Operations
public:
	void			Setup(CGUID* pGUID, SOCKADDR_IN* pHost, BOOL bMustPush);
	BOOL			Connect();
	TRISTATE		GetConnectedState() const;
	virtual void	AttachTo(CConnection* pConnection);
	BOOL			SendPush(BOOL bAutomatic);
	BOOL			OnPush(CGUID* pGUID, CConnection* pConnection);
	virtual void	Close();
public:
	void		Print(LPCTSTR pszString);
	void		Send(CG2Packet* pPacket, BOOL bRelease = TRUE);
	BOOL		SendPrivateMessage(BOOL bAction, LPCTSTR pszText);
	void		StatusMessage(int nFlags, UINT nID, ...);
	void		OnOpenWindow();
	void		OnCloseWindow();
protected:
	virtual BOOL	OnRun();
	virtual BOOL	OnConnected();
	virtual BOOL	OnRead();
	virtual void	OnDropped(BOOL bError);
	virtual BOOL	OnHeaderLine(CString& strHeader, CString& strValue);
	virtual BOOL	OnHeadersComplete();
protected:
	BOOL	ReadHandshake();
	BOOL	ReadText();
	BOOL	ReadPackets();
	void	PostOpenWindow();
protected:
	BOOL	OnEstablished();
	BOOL	OnText(const CString& str);
	BOOL	OnPacket(CG2Packet* pPacket);
	BOOL	OnProfileChallenge(CG2Packet* pPacket);
	BOOL	OnProfileDelivery(CG2Packet* pPacket);
	BOOL	OnChatRequest(CG2Packet* pPacket);
	BOOL	OnChatAnswer(CG2Packet* pPacket);
	BOOL	OnChatMessage(CG2Packet* pPacket);
	
};

enum
{
	cssNull, cssConnecting, cssRequest1, cssHeaders1, cssRequest2, cssHeaders2,
	cssRequest3, cssHeaders3, cssHandshake, cssActive
};

#endif // !defined(AFX_CHATSESSION_H__F75BAA22_513A_4569_9BFB_90A053662CDB__INCLUDED_)
