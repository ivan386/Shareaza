//
// Network.h
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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

#include "ThreadImpl.h"

class CNeighbour;
class CBuffer;
class CPacket;
class CG2Packet;
class CRouteCache;
class CQueryKeys;
class CQuerySearch;
class CQueryHit;

enum // It is used from CNetwork::IsFirewalled
{
	CHECK_BOTH, CHECK_TCP, CHECK_UDP
};

class CNetwork :
	public CThreadImpl
{
// Construction
public:
	CNetwork();
	virtual ~CNetwork();

// Attributes
public:
	CRouteCache*	NodeRoute;
	CRouteCache*	QueryRoute;
	CQueryKeys*		QueryKeys;

	CMutexEx		m_pSection;
	SOCKADDR_IN		m_pHost;				// Structure (Windows Sockets) which holds address of the local machine
	BOOL			m_bAutoConnect;
	volatile bool	m_bConnected;			// Network has finished initialising and is connected
	DWORD			m_tStartedConnecting;	// The time Shareaza started trying to connect
	DWORD			m_tLastConnect;			// The last time a neighbour connection attempt was made
	DWORD			m_tLastED2KServerHop;	// The last time the ed2k server was changed
protected:
	CStringA		m_sHostName;
	CList< ULONG >	m_pHostAddresses;
	DWORD			m_nSequence;
	typedef struct
	{
		CString		m_sAddress;
		PROTOCOLID	m_nProtocol;
		WORD		m_nPort;
		BYTE		m_nCommand;
		union
		{
			char	m_pBuffer[ MAXGETHOSTSTRUCT ];
			HOSTENT	m_pHost;
		};
	} ResolveStruct;
	typedef CMap< HANDLE, HANDLE, ResolveStruct*, ResolveStruct* > CResolveMap;
	CResolveMap			m_pLookups;
	CCriticalSection	m_pLookupsSection;

	class CDelayedHit
	{
	public:
		CDelayedHit(CQueryHit* pHits = NULL, DWORD nStage = 0) : m_pHits( pHits ), m_nStage( nStage ) {}
		CDelayedHit(const CDelayedHit& oQHS) : m_pHits( oQHS.m_pHits ), m_nStage( oQHS.m_nStage ) {}
		CQueryHit*	m_pHits;
		DWORD		m_nStage;
	};
	CList< CDelayedHit > m_pDelayedHits;

	// Get asynchronously resolved host
	ResolveStruct* GetResolve(HANDLE hAsync);

	// Clear asynchronous resolver queue
	void		ClearResolve();

	// Restore WinINet connection to Internet
	bool		InternetConnect();

	bool		PreRun();
	void		OnRun();
	void		PostRun();

	// Handle and destroy query hits
	void		RunQueryHits();

// Operations
public:
	BOOL		IsSelfIP(const IN_ADDR& nAddress) const;
	bool		IsAvailable() const;
	bool		IsConnected() const;
	bool		IsListening() const;
	bool		IsWellConnected() const;
	bool		IsStable() const;
	BOOL		IsFirewalled(int nCheck = CHECK_UDP) const;
	DWORD		GetStableTime() const;
	BOOL		IsConnectedTo(const IN_ADDR* pAddress) const;
	BOOL		ReadyToTransfer(DWORD tNow) const;		// Are we ready to start downloading?
public:
	BOOL		Connect(BOOL bAutoConnect = FALSE);
	void		Disconnect();
	BOOL		ConnectTo(LPCTSTR pszAddress, int nPort = 0, PROTOCOLID nProtocol = PROTOCOL_NULL, BOOL bNoUltraPeer = FALSE);
	BOOL		AcquireLocalAddress(LPCTSTR pszHeader);
	BOOL		AcquireLocalAddress(const IN_ADDR& pAddress);
	BOOL		Resolve(LPCTSTR pszHost, int nPort, SOCKADDR_IN* pHost, BOOL bNames = TRUE) const;
	BOOL		AsyncResolve(LPCTSTR pszAddress, WORD nPort, PROTOCOLID nProtocol, BYTE nCommand);
	BOOL		IsReserved(const IN_ADDR* pAddress, bool bCheckLocal=true) const;
	WORD		RandomPort() const;
	void		CreateID(Hashes::Guid& oID);
	BOOL		IsFirewalledAddress(const IN_ADDR* pAddress, BOOL bIncludeSelf = FALSE) const;
public:
	BOOL		GetNodeRoute(const Hashes::Guid& oGUID, CNeighbour** ppNeighbour, SOCKADDR_IN* pEndpoint);
	BOOL		RoutePacket(CG2Packet* pPacket);
	BOOL		SendPush(const Hashes::Guid& oGUID, DWORD nIndex = 0);
	BOOL		RouteHits(CQueryHit* pHits, CPacket* pPacket);
	void		OnWinsock(WPARAM wParam, LPARAM lParam);
	void		OnQuerySearch(const CQuerySearch* pSearch);
	// Add query hit to queue
	void		OnQueryHits(CQueryHit* pHits);
public:
	void		UDPHostCache(IN_ADDR* pAddress, WORD nPort);
	void		UDPKnownHubCache(IN_ADDR* pAddress, WORD nPort);

	// Safe way to accept socket
	static SOCKET AcceptSocket(SOCKET hSocket, SOCKADDR_IN* addr, LPCONDITIONPROC lpfnCondition, DWORD_PTR dwCallbackData = 0);
	// Safe way to close socket
	static void	CloseSocket(SOCKET& hSocket, const bool bForce);
	// Safe way to send TCP data
	static int Send(SOCKET s, const char* buf, int len);
	// Safe way to send UDP data
	static int SendTo(SOCKET s, const char* buf, int len, const SOCKADDR_IN* pTo);
	// Safe way to receive TCP data
	static int Recv(SOCKET s, char* buf, int len);
	// Safe way to receive UDP data
	static int RecvFrom(SOCKET s, char* buf, int len, SOCKADDR_IN* pFrom);
	// Safe way to call InternetOpenUrl
	static HINTERNET InternetOpenUrl(HINTERNET hInternet, LPCWSTR lpszUrl, LPCWSTR lpszHeaders, DWORD dwHeadersLength, DWORD dwFlags);

	friend class CHandshakes;
	friend class CNeighbours;
};

extern CNetwork Network;
