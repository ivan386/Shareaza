//
// DCNeighbour.h
//
// Copyright (c) Shareaza Development Team, 20010.
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

#include "Neighbour.h"


class CDCNeighbour : public CNeighbour
{
public:
	CDCNeighbour();
	virtual ~CDCNeighbour();

	virtual BOOL	ConnectTo(const IN_ADDR* pAddress, WORD nPort, BOOL bAutomatic);
	virtual BOOL	Send(CPacket* pPacket, BOOL bRelease = TRUE, BOOL bBuffered = FALSE);

	// Send $ConnectToMe command
	BOOL			ConnectToMe(const CString& sNick);

	CString			m_sNick;		// User nick on this hub
	BOOL			m_bExtended;	// Using extended protocol
	CStringList		m_oFeatures;	// Remote client supported features

protected:
	virtual BOOL	OnConnected();
	virtual void	OnDropped();
	virtual BOOL	OnRead();

	// Read single command from input buffer
	BOOL			ReadCommand(std::string& strLine);
	// Got DC++ command
	BOOL			OnCommand(const std::string& strCommand, const std::string& strParams);
	// Got $Lock command
	BOOL			OnLock(const std::string& strLock);
	// Got $Hello command
	BOOL			OnHello();
	// Got $Supports command
	BOOL			OnSupport();
	// Got chat message
	BOOL			OnChat(const std::string& strMessage);
	// Got search request
	BOOL			OnSearch(const IN_ADDR* pAddress, WORD nPort, std::string& strSearch);
};
