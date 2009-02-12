//
// ChatCore.h
//
// Copyright © Shareaza Development Team, 2002-2009.
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

#include "ThreadImpl.h"

class CConnection;
class CChatSession;


class CChatCore :
	public CThreadImpl
{
// Construction
public:
	CChatCore();
	virtual ~CChatCore();

// Attributes
public:
	CMutex		m_pSection;
protected:
	CList< CChatSession* > m_pSessions;

// Operations
public:
	POSITION		GetIterator() const;
	CChatSession*	GetNext(POSITION& pos) const;
	INT_PTR			GetCount() const { return m_pSessions.GetCount(); }
	BOOL			Check(CChatSession* pSession) const;
	void			Close();
	void			OnAccept(CConnection* pConnection, PROTOCOLID nProtocol = PROTOCOL_NULL);
	BOOL			OnPush(const Hashes::Guid& oGUID, CConnection* pConnection);
	void			OnED2KMessage(CEDClient* pClient, CEDPacket* pPacket);
	CChatSession*	FindSession(CEDClient* pClient);
	void			StopThread();
protected:
	void			Add(CChatSession* pSession);
	void			Remove(CChatSession* pSession);
	void			StartThread();
protected:
	void			OnRun();

	friend class CChatSession;
};

extern CChatCore ChatCore;
