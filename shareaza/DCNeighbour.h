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

protected:
	BOOL			m_bExtended;	// Using extended protocol
	CStringList		m_oFeatures;	// Supported features
	CString			m_sNick;		// Registered nick

	virtual BOOL	OnRead();
	virtual BOOL	OnWrite();
	virtual BOOL	OnRun();
	virtual BOOL	OnConnected();
	virtual void	OnDropped();

	virtual BOOL	OnCommand(const std::string& strCommand, const std::string& strParams);

	// Read single command from input stream
	BOOL			ReadCommand(std::string& strLine);
	// Answer on $Lock, send $Key and $Support
	BOOL			SendKey(const std::string& strLock);
	std::string		MakeKey(const std::string& aLock);
	std::string		KeySubst(const BYTE* aKey, size_t len, size_t n);
	BOOL			IsExtra(BYTE b) const;
	// Answer on $Hello
	BOOL			SendVersion();
};
