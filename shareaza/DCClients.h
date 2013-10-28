//
// DCClients.h
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

class CConnection;
class CDCClient;
class CDCNeighbour;


class CDCClients
{
public:
	CDCClients();
	~CDCClients();

	// Add client
	void		Add(CDCClient* pClient);
	// Remove client
	void		Remove(CDCClient* pClient);
	// Remove all clients
	void		Clear();
	// Get client count
	int			GetCount() const;
	// Maintain not-connected (queued) clients
	void		OnRun();
	// Find client by GUID
	CDCClient*	GetClient(const CString& sNick) const;
	// Find hub by user nick
	CDCNeighbour* GetHub(const CString& sNick) const;
	// Initiate connection to hub
	BOOL 		Connect(const IN_ADDR* pHubAddress, WORD nHubPort, const CString& sRemoteNick, BOOL& bSuccess);
	// Initiate connection to client
	BOOL		ConnectTo(const IN_ADDR* pAddress, WORD nPort, CDCNeighbour* pHub, const CString& sRemoteNick);
	// Accept incoming TCP connection
	BOOL		OnAccept(CConnection* pConnection);
	// Merge same connections into one
	BOOL		Merge(CDCClient* pClient);
	// Calculate key
	static std::string MakeKey(const std::string& aLock);
	// Create DC++ compatible nick
	static CString CreateNick(LPCTSTR szNick = NULL);
	// Create GUID from nick
	static void CreateGUID(const CString& sNick, Hashes::Guid& oGUID);

private:
	CList< CDCClient* >	m_pList;
	mutable CMutexEx	m_pSection;	// Object guard

	static std::string KeySubst(const BYTE* aKey, size_t len, size_t n);
	static BOOL IsExtra(BYTE b);
};

extern CDCClients DCClients;

// Shareaza Client-Client capabilities
#define DC_CLIENT_SUPPORTS "$Supports MiniSlots XmlBZList ADCGet TTHL TTHF ZLIG|"

// Shareaza Client-Hub capabilities
#define DC_HUB_SUPPORTS "$Supports NoHello NoGetINFO UserIP2 TTHSearch ZPipe0|"
