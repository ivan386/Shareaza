//
// Handshakes.h
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

#if !defined(AFX_HANDSHAKES_H__2314A7BE_5C51_4F8E_A4C6_6059A7621AE0__INCLUDED_)
#define AFX_HANDSHAKES_H__2314A7BE_5C51_4F8E_A4C6_6059A7621AE0__INCLUDED_

#pragma once

class CHandshake;


class CHandshakes  
{
// Construction
public:
	CHandshakes();
	virtual ~CHandshakes();
	
// Attributes
public:
	DWORD		m_nStableCount;
	DWORD		m_tStableTime;
protected:
	CCriticalSection	m_pSection;
	CEvent				m_pWakeup;
	SOCKET				m_hSocket;
	HANDLE				m_hThread;
	CPtrList			m_pList;
	
// Operations
public:
	BOOL		Listen();
	void		Disconnect();
	BOOL		PushTo(IN_ADDR* pAddress, WORD nPort, DWORD nIndex = 0);
	BOOL		IsConnectedTo(IN_ADDR* pAddress);
protected:
	void		Substitute(CHandshake* pOld, CHandshake* pNew);
	void		Remove(CHandshake* pHandshake);
protected:
	static UINT	ThreadStart(LPVOID pParam);
	void		OnRun();
	void		RunHandshakes();
	BOOL		AcceptConnection();
	void		CreateHandshake(SOCKET hSocket, SOCKADDR_IN* pHost);
	void		RunStableUpdate();
	static int	CALLBACK AcceptCheck(IN LPWSABUF lpCallerId, IN LPWSABUF lpCallerData, IN OUT LPQOS lpSQOS, IN OUT LPQOS lpGQOS, IN LPWSABUF lpCalleeId, OUT LPWSABUF lpCalleeData, OUT GROUP FAR * g, IN DWORD dwCallbackData);
	
// Inlines
public:
	inline POSITION GetIterator() const
	{
		return m_pList.GetHeadPosition();
	}

	inline CHandshake* GetNext(POSITION& pos) const
	{
		return (CHandshake*)m_pList.GetNext( pos );
	}

	inline BOOL IsListening() const
	{
		return m_hSocket != INVALID_SOCKET;
	}
	
	friend class CHandshake;
};

extern CHandshakes Handshakes;

#endif // !defined(AFX_HANDSHAKES_H__2314A7BE_5C51_4F8E_A4C6_6059A7621AE0__INCLUDED_)
