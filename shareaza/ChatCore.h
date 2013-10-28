//
// ChatCore.h
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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
#include "ChatSession.h"

class CEDClient;
class CPacket;


class CChatCore : public CThreadImpl
{
public:
	CChatCore();
	virtual ~CChatCore();

	CMutexEx		m_pSection;

	POSITION		GetIterator() const;
	CChatSession*	GetNext(POSITION& pos) const;
	INT_PTR			GetCount() const { return m_pSessions.GetCount(); }
	BOOL			Check(CChatSession* pSession) const;
	void			Close();
	BOOL			OnAccept(CConnection* pConnection);
	BOOL			OnPush(const Hashes::Guid& oGUID, CConnection* pConnection);

	template< typename T > void OnMessage(const T* pClient, CPacket* pPacket = NULL)
	{
		if ( ! Settings.Community.ChatEnable ||
			 ! Settings.Community.ChatAllNetworks )
			 // Chat disabled
			 return;

		CSingleLock pLock( &m_pSection );
		if ( pLock.Lock( 250 ) )
		{
			if ( CChatSession* pSession = FindSession< T >( pClient, TRUE ) )
			{
				pSession->OnMessage( pPacket );
			}
		}
	}

	template< typename T > void OnDropped(const T* pClient)
	{
		CSingleLock pLock( &m_pSection );
		if ( pLock.Lock( 250 ) )
		{
			if ( CChatSession* pSession = FindSession< T >( pClient, FALSE ) )
			{
				pSession->OnDropped();
			}
		}
	}

	template< typename T > void OnAddUser(const T* pClient, CChatUser* pUser)
	{
		CSingleLock pLock( &m_pSection );
		if ( pLock.Lock( 250 ) )
		{
			if ( CChatSession* pSession = FindSession< T >( pClient, FALSE ) )
			{
				pSession->AddUser( pUser );
				return;
			}
		}
		delete pUser;
	}

	template< typename T > void OnDeleteUser(const T* pClient, CString* pUser)
	{
		CSingleLock pLock( &m_pSection );
		if ( pLock.Lock( 250 ) )
		{
			if ( CChatSession* pSession = FindSession< T >( pClient, FALSE ) )
			{
				pSession->DeleteUser( pUser );
				return;
			}
		}
		delete pUser;
	}

protected:
	CList< CChatSession* > m_pSessions;

	template< typename T > CChatSession* FindSession(const T* pClient, BOOL bCreate);

	void			Add(CChatSession* pSession);
	void			Remove(CChatSession* pSession);

	void			StartThread();
	void			OnRun();

	friend class CChatSession;
};

extern CChatCore ChatCore;
