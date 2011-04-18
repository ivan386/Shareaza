//
// DCNeighbour.h
//
// Copyright (c) Shareaza Development Team, 2010-2011.
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

#include "Neighbour.h"
#include "WndChat.h"

class CChatSession;
class CDCPacket;


class CDCNeighbour : public CNeighbour
{
public:
	CDCNeighbour();
	virtual ~CDCNeighbour();

	virtual BOOL	ConnectTo(const IN_ADDR* pAddress, WORD nPort, BOOL bAutomatic);
	virtual BOOL	Send(CPacket* pPacket, BOOL bRelease = TRUE, BOOL bBuffered = FALSE);
	virtual DWORD	GetUserCount() const { return m_oUsers.GetCount(); }

	// Send $ConnectToMe command
	BOOL			ConnectToMe(const CString& sNick);

	// Find user
	CChatUser*		GetUser(const CString& sNick) const;

	// Chat window was (re)opened
	void			OnChatOpen(CChatSession* pSession);

	CString			m_sNick;		// User nick on this hub
	BOOL			m_bNickValid;	// User nick was accepted
	BOOL			m_bExtended;	// Using extended protocol
	CStringList		m_oFeatures;	// Remote client supported features

protected:
	CChatUser::Map	m_oUsers;		// Hub user list

	void			RemoveAllUsers();

	virtual BOOL	OnConnected();
	virtual void	OnDropped();
	virtual BOOL	OnRead();

	// Got DC++ command
	BOOL			OnPacket(CDCPacket* pPacket);
	// Got $Lock command
	BOOL			OnLock(LPSTR szLock);
	// Got $Hello command
	BOOL			OnHello();
	// Got search request
	BOOL			OnQuery(CDCPacket* pPacket);
};
