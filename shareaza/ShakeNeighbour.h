//
// ShakeNeighbour.h
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

#if !defined(AFX_SHAKENEIGHBOUR_H__259E22A0_EFA9_4684_B642_B98CE4CE682F__INCLUDED_)
#define AFX_SHAKENEIGHBOUR_H__259E22A0_EFA9_4684_B642_B98CE4CE682F__INCLUDED_

#pragma once

#include "Neighbour.h"


class CShakeNeighbour : public CNeighbour  
{
// Construction
public:
	CShakeNeighbour();
	virtual ~CShakeNeighbour();
	
// Attributes
protected:
	BOOL		m_bSentAddress;
	BOOL		m_bG2Send;
	BOOL		m_bG2Accept;
	BOOL		m_bDeflateSend;
	BOOL		m_bDeflateAccept;
	BOOL		m_bCanDeflate;
	TRISTATE	m_bUltraPeerSet;
	TRISTATE	m_bUltraPeerNeeded;
	TRISTATE	m_bUltraPeerLoaded;

// Operations
public:
	virtual BOOL	ConnectTo(IN_ADDR* pAddress, WORD nPost, BOOL bAutomatic = FALSE, BOOL bNoUltraPeer = FALSE);
	virtual void	AttachTo(CConnection* pConnection);
	virtual void	Close(UINT nError = IDS_CONNECTION_CLOSED ); //, BOOL bRetry04 = FALSE);
protected:
	virtual BOOL	OnConnected();
	virtual BOOL	OnRead();
	virtual void	OnDropped(BOOL bError);
	virtual BOOL	OnRun();
	virtual BOOL	OnHeaderLine(CString& strHeader, CString& strValue);
	virtual BOOL	OnHeadersComplete();
	virtual BOOL	OnHeadersCompleteG1();
	virtual BOOL	OnHeadersCompleteG2();
protected:
	void	SendMinimalHeaders();
	void	SendPublicHeaders(PROTOCOLID nProtocol = PROTOCOL_NULL);
	void	SendPrivateHeaders();
	void	SendHostHeaders(LPCTSTR pszMessage = NULL);
	BOOL	ReadResponse();
	void	OnHandshakeComplete();

};

#endif // !defined(AFX_SHAKENEIGHBOUR_H__259E22A0_EFA9_4684_B642_B98CE4CE682F__INCLUDED_)
