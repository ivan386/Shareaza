//
// Connection.h
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

#if !defined(AFX_CONNECTION_H__6312EF26_B2C8_431F_93EF_243EA5E1A3DF__INCLUDED_)
#define AFX_CONNECTION_H__6312EF26_B2C8_431F_93EF_243EA5E1A3DF__INCLUDED_

#pragma once

class CBuffer;
class CConnection;

typedef struct
{
	DWORD*		pLimit;
	BOOL		bUnscaled;
	DWORD		nTotal;
	DWORD		tLast;
	DWORD		nMeasure;
	DWORD		pHistory[64];
	DWORD		pTimes[64];
	DWORD		nPosition;
	DWORD		tLastAdd;
	DWORD		tLastSlot;
} TCPBandwidthMeter;


class CConnection  
{
// Construction
public:
	CConnection();
	virtual ~CConnection();

// Attributes
public:
	SOCKADDR_IN	m_pHost;
	CString		m_sAddress;
	BOOL		m_bInitiated;
	BOOL		m_bConnected;
	DWORD		m_tConnected;
public:
	SOCKET		m_hSocket;
	CBuffer*	m_pInput;
	CBuffer*	m_pOutput;
	CString		m_sUserAgent;
	CString		m_sLastHeader;
public:
	TCPBandwidthMeter	m_mInput;
	TCPBandwidthMeter	m_mOutput;
	int					m_nQueuedRun;

// Operations
public:
	virtual BOOL	ConnectTo(SOCKADDR_IN* pHost);
	virtual BOOL	ConnectTo(IN_ADDR* pAddress, WORD nPort);
	virtual void	AcceptFrom(SOCKET hSocket, SOCKADDR_IN* pHost);
	virtual void	AttachTo(CConnection* pConnection);
	virtual void	Close();
public:
	BOOL	DoRun();
	void	QueueRun();
	void	Measure();
	BOOL	ReadHeaders();
	BOOL	SendMyAddress();
	BOOL	IsAgentBlocked();
	
protected:
	virtual BOOL	OnRun();
	virtual BOOL	OnConnected();
	virtual BOOL	OnRead();
	virtual BOOL	OnWrite();
	virtual void	OnDropped(BOOL bError);
	virtual BOOL	OnHeaderLine(CString& strHeader, CString& strValue);
	virtual BOOL	OnHeadersComplete();
	
// Statics
public:
	static CString	URLEncode(LPCTSTR pszInput);
	static CString	URLDecode(LPCTSTR pszInput);
	static BOOL		StartsWith(LPCTSTR pszInput, LPCTSTR pszText);

};

#endif // !defined(AFX_CONNECTION_H__6312EF26_B2C8_431F_93EF_243EA5E1A3DF__INCLUDED_)
