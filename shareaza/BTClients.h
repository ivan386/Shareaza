//
// BTClients.h
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

#if !defined(AFX_BTCLIENTS_H__A52C25F0_7D86_40A2_BBF6_DB452DED7A1E__INCLUDED_)
#define AFX_BTCLIENTS_H__A52C25F0_7D86_40A2_BBF6_DB452DED7A1E__INCLUDED_

#pragma once

class CConnection;
class CBTClient;
class CBTTrackerRequest;


class CBTClients  
{
// Construction
public:
	CBTClients();
	virtual ~CBTClients();
	
// Attributes
protected:
	CPtrList	m_pList;
	//SHA1		m_pGUID;
protected:
	CCriticalSection	m_pSection;
	CEvent				m_pShutdown;
	BOOL				m_bShutdown;
	CPtrList			m_pRequests;
	
// Operations
public:
	void		Clear();
	//SHA1*		GetGUID();
	BOOL		OnAccept(CConnection* pConnection);
	void		ShutdownRequests();
protected:
	void		Add(CBTClient* pClient);
	void		Remove(CBTClient* pClient);
	void		Add(CBTTrackerRequest* pRequest);
	void		Remove(CBTTrackerRequest* pRequest);
	
// List Access
public:
	inline POSITION GetIterator() const
	{
		return m_pList.GetHeadPosition();
	}
	
	inline CBTClient* GetNext(POSITION& pos) const
	{
		return (CBTClient*)m_pList.GetNext( pos );
	}
	
	inline int GetCount() const
	{
		return m_pList.GetCount();
	}
	
	friend class CBTClient;
	friend class CBTTrackerRequest;

};

extern CBTClients BTClients;

#define BT_PROTOCOL_HEADER			"\023BitTorrent protocol"
#define BT_PROTOCOL_HEADER_LEN		20

#endif // !defined(AFX_BTCLIENTS_H__A52C25F0_7D86_40A2_BBF6_DB452DED7A1E__INCLUDED_)
