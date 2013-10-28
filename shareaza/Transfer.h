//
// Transfer.h
//
// Copyright (c) Shareaza Development Team, 2002-2011.
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

#include "Connection.h"

class CBuffer;


class CTransfer abstract : public CConnection
{
public:
	CTransfer(PROTOCOLID nProtocol = PROTOCOL_ANY);
	virtual ~CTransfer();

	SOCKADDR_IN			m_pServer;			// Reference server (ED2K, DC++)
	CString				m_sRemoteNick;		// Remote user nick
	DWORD				m_nRunCookie;
	CList< CString >	m_pSourcesSent;
	CArray< CString >	m_pHeaderName;
	CArray< CString >	m_pHeaderValue;
	int					m_nState;			// Common state code
	DWORD				m_nBandwidth;		// Bandwidth allocated
	QWORD				m_nOffset;			// Fragment offset
	QWORD				m_nLength;			// Fragment length
	QWORD				m_nPosition;		// Fragment position
	DWORD				m_tRequest;			// The time a request was sent

	virtual BOOL	ConnectTo(const IN_ADDR* pAddress, WORD nPort);
	virtual void	AttachTo(CConnection* pConnection);
	virtual void	Close(UINT nError = 0);

protected:
	void			ClearHeaders();
	virtual BOOL	OnHeaderLine(CString& strHeader, CString& strValue);
};
