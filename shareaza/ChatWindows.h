//
// ChatWindows.h
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

#if !defined(AFX_CHATWINDOWS_H__F756916C_1CDF_460A_B1F2_8EC53E72B6C2__INCLUDED_)
#define AFX_CHATWINDOWS_H__F756916C_1CDF_460A_B1F2_8EC53E72B6C2__INCLUDED_

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
protected:
	CPtrList	m_pList;

// Operations
public:
	POSITION	GetIterator() const;
	CChatFrame*	GetNext(POSITION& pos) const;
	int			GetCount() const;
	void		Close();
public:
	CPrivateChatFrame*	FindPrivate(GGUID* pGUID);
	CPrivateChatFrame*	FindPrivate(IN_ADDR* pAddress);
	CPrivateChatFrame*	OpenPrivate(GGUID* pGUID, SOCKADDR_IN* pHost, BOOL bMustPush = FALSE);
	CPrivateChatFrame*	OpenPrivate(GGUID* pGUID, IN_ADDR* pAddress, WORD nPort = 6346, BOOL bMustPush = FALSE);
protected:
	void	Add(CChatFrame* pFrame);
	void	Remove(CChatFrame* pFrame);
	
	friend class CChatFrame;
};

extern CChatWindows ChatWindows;

#endif // !defined(AFX_CHATWINDOWS_H__F756916C_1CDF_460A_B1F2_8EC53E72B6C2__INCLUDED_)
