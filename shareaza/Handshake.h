//
// Handshake.h
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

#if !defined(AFX_HANDSHAKE_H__FC762B48_46E6_4BB1_8B25_BC783DA966A4__INCLUDED_)
#define AFX_HANDSHAKE_H__FC762B48_46E6_4BB1_8B25_BC783DA966A4__INCLUDED_

#pragma once

#include "Connection.h"

class CEDPacket;


class CHandshake : public CConnection
{
// Construction
public:
	CHandshake();
	CHandshake(SOCKET hSocket, SOCKADDR_IN* pHost);
	CHandshake(CHandshake* pCopy);
	virtual ~CHandshake();
	
// Attributes
public:
	BOOL			m_bPushing;
	DWORD			m_nIndex;
	
// Operations
public:
	virtual BOOL	Push(IN_ADDR* pAddress, WORD nPort, DWORD nIndex);
protected:
	virtual BOOL	OnRun();
	virtual BOOL	OnConnected();
	virtual void	OnDropped(BOOL bError);
	virtual BOOL	OnRead();
protected:
	BOOL	OnAcceptGive();
	BOOL	OnAcceptPush();
	BOOL	OnPush(GGUID* pGUID);
	
};

#endif // !defined(AFX_HANDSHAKE_H__FC762B48_46E6_4BB1_8B25_BC783DA966A4__INCLUDED_)
