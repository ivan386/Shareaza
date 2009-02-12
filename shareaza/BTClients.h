//
// BTClients.h
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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
class CBTClient;


class CBTClients : private boost::noncopyable
{
// Construction
public:
	CBTClients();
	virtual ~CBTClients();

// Attributes
protected:
	CList< CBTClient* >	m_pList;
	CMutex				m_pListSection;	// m_pList guard

// Operations
public:
	void		Clear();
	BOOL		OnAccept(CConnection* pConnection);

protected:
	void		Add(CBTClient* pClient);
	void		Remove(CBTClient* pClient);

	friend class CBTClient;
};

extern CBTClients BTClients;

#define BT_PROTOCOL_HEADER			"\023BitTorrent protocol"
#define BT_PROTOCOL_HEADER_LEN		20
