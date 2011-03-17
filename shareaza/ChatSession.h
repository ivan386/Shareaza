//
// ChatSession.h
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

#include "Connection.h"

class CEDPacket;
class CEDClient;
class CG2Packet;
class CGProfile;
class CChatWnd;
class CPrivateChatWnd;


class CChatSession : public CConnection
{
public:
	CChatSession(PROTOCOLID nProtocol = PROTOCOL_ANY, CPrivateChatWnd* pFrame = NULL);
	virtual ~CChatSession();

	Hashes::Guid	m_oGUID;
	int				m_nState;
	BOOL			m_bMustPush;
	CString			m_sUserNick;
	BOOL			m_bUnicode;		// ED2K Client in UTF-8 format
	DWORD			m_nClientID;	// ED2K Client ID (if appropriate)
	SOCKADDR_IN		m_pServer;		// ED2K server (if appropriate)

	virtual void	AttachTo(CConnection* pConnection);
	virtual void	Close(UINT nError = 0);
	virtual void	OnDropped();

	BOOL			Connect();
	TRISTATE		GetConnectedState() const;
	void			OnED2KMessage(CEDPacket* pPacket);
	BOOL			SendPush(BOOL bAutomatic);
	BOOL			OnPush(const Hashes::Guid& oGUID, CConnection* pConnection);
	BOOL			SendPrivateMessage(bool bAction, const CString& strText);
	void			StatusMessage(int nFlags, UINT nID, ...);
	void			OnOpenWindow();
	void			OnCloseWindow();

protected:
	TRISTATE			m_bOld;			// Chat version: TRI_TRUE = CHAT/0.1
	DWORD				m_tPushed;
	CGProfile*			m_pProfile;
	CPrivateChatWnd*	m_pWndPrivate;

	virtual BOOL	OnRun();
	virtual BOOL	OnConnected();
	virtual BOOL	OnRead();
	virtual BOOL	OnHeaderLine(CString& strHeader, CString& strValue);
	virtual BOOL	OnHeadersComplete();

	BOOL	ReadHandshake();
	BOOL	ReadPacketsED2K();
	BOOL	SendPacketsED2K();
	BOOL	ReadText();
	BOOL	ReadPackets();
	void	PostOpenWindow();
	void	Send(CG2Packet* pPacket);
	BOOL	SendChatMessage(CEDPacket* pPacket);
	void	MakeActive();

	BOOL	OnChatMessage(CEDPacket* pPacket);
	BOOL	OnCaptchaRequest(CEDPacket* pPacket);
	BOOL	OnCaptchaResult(CEDPacket* pPacket);

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
	cssRequest3, cssHeaders3, cssHandshake, cssActive, cssAway
};
