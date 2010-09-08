//
// DCClients.h
//
// Copyright (c) Shareaza Development Team, 2010.
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


class CDCClients
{
public:
	CDCClients();
	~CDCClients();

	mutable CMutex		m_pSection;	// Object guard

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

	// Initiate connection to hub
	BOOL 		Connect(const IN_ADDR& pHubAddress, WORD nHubPort, const CString& sNick, BOOL& bSuccess);

	// Accept incoming TCP connection
	BOOL		OnAccept(CConnection* pConnection);

	// Merge same connections into one
	BOOL		Merge(CDCClient* pClient);

	// Calculate key
	std::string	MakeKey(const std::string& aLock) const;

	// Create DC++ compatible nick
	CString		GetDefaultNick() const;

private:
	CList< CDCClient* >	m_pList;

	std::string	KeySubst(const BYTE* aKey, size_t len, size_t n) const;
	BOOL		IsExtra(BYTE b) const;
};

extern CDCClients DCClients;
