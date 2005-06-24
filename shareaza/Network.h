//
// Network.h
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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

#if !defined(AFX_NETWORK_H__544414B1_3698_4C92_B0B0_1DC56AB48074__INCLUDED_)
#define AFX_NETWORK_H__544414B1_3698_4C92_B0B0_1DC56AB48074__INCLUDED_

#pragma once

class CNeighbour;
class CBuffer;
class CPacket;
class CG2Packet;
class CRouteCache;
class CQueryKeys;
class CQuerySearch;
class CQueryHit;


class CNetwork
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
public:
	CMutex			m_pSection;
	CEvent			m_pWakeup;
	SOCKADDR_IN		m_pHost;
	BOOL			m_bEnabled;				// If the network "enabled" (Connected or trying)
	BOOL			m_bAutoConnect;
	DWORD			m_tStartedConnecting;	// The time Shareaza started trying to connect
	DWORD			m_tLastConnect;			// The last time a neighbout connection attempt was made
protected:
	HANDLE			m_hThread;
	DWORD			m_nSequence;
	CMapPtrToPtr	m_pLookups;

// Operations
public:
	BOOL		IsAvailable() const;
	BOOL		IsConnected() const;
	BOOL		IsListening() const;
	int			IsWellConnected() const;
	BOOL		IsStable() const;
	DWORD		GetStableTime() const;
	BOOL		IsConnectedTo(IN_ADDR* pAddress);
	BOOL		ReadyToTransfer(DWORD tNow) const;		// Are we ready to start downloading?
public:
	BOOL		Connect(BOOL bAutoConnect = FALSE);
	void		Disconnect();
	BOOL		ConnectTo(LPCTSTR pszAddress, int nPort = 0, PROTOCOLID nProtocol = PROTOCOL_NULL, BOOL bNoUltraPeer = FALSE);
	void		AcquireLocalAddress(LPCTSTR pszHeader);
	BOOL		Resolve(LPCTSTR pszHost, int nPort, SOCKADDR_IN* pHost, BOOL bNames = TRUE) const;
	BOOL		AsyncResolve(LPCTSTR pszAddress, WORD nPort, PROTOCOLID nProtocol, BYTE nCommand);
	WORD		RandomPort() const;
	void		CreateID(GGUID& oID);
	BOOL		IsFirewalledAddress(LPVOID pAddress, BOOL bIncludeSelf = FALSE);
public:
	BOOL		GetNodeRoute(GGUID* pGUID, CNeighbour** ppNeighbour, SOCKADDR_IN* pEndpoint);
	BOOL		RoutePacket(CG2Packet* pPacket);
	BOOL		SendPush(GGUID* pGUID, DWORD nIndex = 0);
	BOOL		RouteHits(CQueryHit* pHits, CPacket* pPacket);
	void		OnWinsock(WPARAM wParam, LPARAM lParam);
	void		OnQuerySearch(CQuerySearch* pSearch);
	void		OnQueryHits(CQueryHit* pHits);
protected:
	static UINT	ThreadStart(LPVOID pParam);
	void		OnRun();

	friend class CHandshakes;
	friend class CNeighbours;
};

extern CNetwork Network;


#endif // !defined(AFX_NETWORK_H__544414B1_3698_4C92_B0B0_1DC56AB48074__INCLUDED_)
