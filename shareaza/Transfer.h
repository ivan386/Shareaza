//
// Transfer.h
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

#pragma once

#include "Connection.h"

class CBuffer;


class CTransfer : public CConnection
{
// Construction
public:
	CTransfer();
	virtual ~CTransfer();

// Attributes
public:
	DWORD			m_nRunCookie;
public:
	CStringList		m_pSourcesSent;
	CStringArray	m_pHeaderName;
	CStringArray	m_pHeaderValue;

// Operations
public:
	virtual BOOL	ConnectTo(IN_ADDR* pAddress, WORD nPort);
	virtual void	AttachTo(CConnection* pConnection);
	virtual void	Close();
protected:
	void			ClearHeaders();
	virtual BOOL	OnHeaderLine(CString& strHeader, CString& strValue);

};

