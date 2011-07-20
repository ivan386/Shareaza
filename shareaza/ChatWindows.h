//
// ChatWindows.h
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

class CChatWnd;
class CPrivateChatWnd;


class CChatWindows
{
public:
	CChatWindows();
	virtual ~CChatWindows();

	// Start new or reopen existing chat session
	CPrivateChatWnd*	OpenPrivate(const Hashes::Guid& oGUID, const SOCKADDR_IN* pHost, BOOL bMustPush = FALSE, PROTOCOLID nProtocol = PROTOCOL_NULL, SOCKADDR_IN* pServer = NULL);

	// Start new or reopen existing chat session (nPort and nServerPort must be in host byte order)
	CPrivateChatWnd*	OpenPrivate(const Hashes::Guid& oGUID, const IN_ADDR* pAddress, WORD nPort = protocolPorts[ PROTOCOL_G2 ], BOOL bMustPush = FALSE, PROTOCOLID nProtocol = PROTOCOL_NULL, IN_ADDR* pServerAddress = NULL, WORD nServerPort = 0);

	void				Add(CChatWnd* pFrame);
	void				Remove(CChatWnd* pFrame);
	CPrivateChatWnd*	FindPrivate(const Hashes::Guid& oGUID, bool bLive) const;
	CPrivateChatWnd*	FindPrivate(const SOCKADDR_IN* pAddress) const;
	CPrivateChatWnd*	FindED2KFrame(const SOCKADDR_IN* pAddress) const;
	CPrivateChatWnd*	FindED2KFrame(DWORD nClientID, const SOCKADDR_IN* pServerAddress) const;

private:
	CList< CChatWnd* > m_pList;

	POSITION			GetIterator() const;
	CChatWnd*			GetNext(POSITION& pos) const;

	CPrivateChatWnd*	OpenPrivateGnutella(const Hashes::Guid& oGUID, const SOCKADDR_IN* pHost, BOOL bMustPush, PROTOCOLID nProtocol);
	CPrivateChatWnd*	OpenPrivateED2K(const Hashes::Guid& oGUID, const SOCKADDR_IN* pHost, BOOL bMustPush, SOCKADDR_IN* pServer);
};

extern CChatWindows ChatWindows;
