//
// EDClients.h
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

#if !defined(AFX_EDCLIENTS_H__CAA5D657_A66D_4F1E_97E7_64279D0B821D__INCLUDED_)
#define AFX_EDCLIENTS_H__CAA5D657_A66D_4F1E_97E7_64279D0B821D__INCLUDED_

#pragma once

class CConnection;
class CEDClient;
class CEDPacket;


class CEDClients  
{
// Construction
public:
	CEDClients();
	virtual ~CEDClients();
	
// Attributes
protected:
	CEDClient*		m_pFirst;
	CEDClient*		m_pLast;
	int				m_nCount;
	DWORD			m_tLastRun;
	DWORD			m_tLastServerStats;
	in_addr			m_pLastServer;
	DWORD			m_nLastServerKey;
	BOOL			m_bAllServersDone;
	
// Operations
protected:
	void			Add(CEDClient* pClient);
	void			Remove(CEDClient* pClient);
public:
	void			Clear();
	BOOL			PushTo(DWORD nClientID, WORD nClientPort);
	CEDClient*		Connect(DWORD nClientID, WORD nClientPort, IN_ADDR* pServerAddress, WORD nServerPort, GGUID* pGUID = NULL);
	CEDClient*		GetByIP(IN_ADDR* pAddress);
	CEDClient*		GetByID(DWORD nClientID, IN_ADDR* pServer = NULL, GGUID* pGUID = NULL);
	CEDClient*		GetByGUID(GGUID* pHash);
	BOOL			Merge(CEDClient* pClient);
	BOOL			IsFull(CEDClient* pCheckThis = NULL);
public:
	void			OnRun();
	BOOL			OnAccept(CConnection* pConnection);
	BOOL			OnUDP(SOCKADDR_IN* pHost, CEDPacket* pPacket);
private:
	void			OnServerStatus(SOCKADDR_IN* pHost, CEDPacket* pPacket);
	void			RequestServerStatus(IN_ADDR* pHost, WORD nPort);
	void			RunGlobalStatsRequests(DWORD tNow);

public:
	
	inline CEDClient* GetFirst() const
	{
		return m_pFirst;
	}
	
	friend class CEDClient;
};


extern CEDClients EDClients;


#endif // !defined(AFX_EDCLIENTS_H__CAA5D657_A66D_4F1E_97E7_64279D0B821D__INCLUDED_)
