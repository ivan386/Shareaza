//
// ChatWindows.h
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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

class CChatFrame;
class CPrivateChatFrame;


class CChatWindows
{
// Construction
public:
	CChatWindows();
	virtual ~CChatWindows();

// Attributes
private:
	CList< CChatFrame* > m_pList;

// Operations
public:
	// Start new or reopen existing chat session
	CPrivateChatFrame*	OpenPrivate(const Hashes::Guid& oGUID, SOCKADDR_IN* pHost, BOOL bMustPush = FALSE, PROTOCOLID nProtocol = PROTOCOL_NULL, SOCKADDR_IN* pServer = NULL );

	// Start new or reopen existing chat session (nPort and nServerPort must be in host byte order)
	CPrivateChatFrame*	OpenPrivate(const Hashes::Guid& oGUID, IN_ADDR* pAddress, WORD nPort = 6346, BOOL bMustPush = FALSE, PROTOCOLID nProtocol = PROTOCOL_NULL, IN_ADDR* pServerAddress = NULL, WORD nServerPort = 0 );

	void				Add(CChatFrame* pFrame);
	void				Remove(CChatFrame* pFrame);
	CPrivateChatFrame*	FindPrivate(const Hashes::Guid& oGUID);
	CPrivateChatFrame*	FindPrivate(IN_ADDR* pAddress);
	CPrivateChatFrame*  FindED2KFrame(SOCKADDR_IN* pAddress);
	CPrivateChatFrame*  FindED2KFrame(DWORD nClientID, SOCKADDR_IN* pServerAddress);
private:
	POSITION			GetIterator() const;
	CChatFrame*			GetNext(POSITION& pos) const;
};

extern CChatWindows ChatWindows;
