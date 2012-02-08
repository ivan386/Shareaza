//
// Network.h
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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

class CBuffer;
class CConnection;
class CFirewall;
class CG2Packet;
class CLocalSearch;
class CNeighbour;
class CPacket;
class CQueryHit;
class CQueryKeys;
class CQuerySearch;
class CRouteCache;
class CUPnPFinder;


enum // It is used from CNetwork::IsFirewalled
{
	CHECK_BOTH, CHECK_TCP, CHECK_UDP, CHECK_IP
};

class CNetwork : public CComObject, public CThreadImpl
{
	DECLARE_DYNCREATE(CNetwork)

public:
	CNetwork();
	~CNetwork();

// Attributes
public:
	CAutoPtr< CRouteCache >	NodeRoute;
	CAutoPtr< CRouteCache >	QueryRoute;
	CAutoPtr< CQueryKeys >	QueryKeys;
	CAutoPtr< CUPnPFinder >	UPnPFinder;			// Control Point UPnP
	CAutoPtr< CFirewall >	Firewall;			// Windows Firewall

	CMutexEx		m_pSection;
	SOCKADDR_IN		m_pHost;					// Structure (Windows Sockets) which holds address of the local machine
	BOOL			m_bAutoConnect;
	volatile bool	m_bConnected;				// Network has finished initializing and is connected
	DWORD			m_tStartedConnecting;		// The time Shareaza started trying to connect
	DWORD			m_tLastConnect;				// The last time a neighbour connection attempt was made
	DWORD			m_tLastED2KServerHop;		// The last time the ed2k server was changed
	TRISTATE		m_bUPnPPortsForwarded;		// UPnP values are assigned when the discovery is complete

protected:
	CStringA		m_sHostName;
	mutable CCriticalSection	m_pHASection;
	CList< ULONG >	m_pHostAddresses;
	DWORD			m_nSequence;
	CComPtr< IUPnPNAT >			m_pNat;			// NAT UPnP
	CComPtr< INATEventManager >	m_pNatManager;	// NAT Manager interface
	IN_ADDR			m_nUPnPExternalAddress;		// UPnP current external address
	DWORD			m_tUPnPMap;					// Time of last UPnP port mapping

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

	CResolveMap					m_pLookups;
	mutable CCriticalSection	m_pLookupsSection;

	class CJob
	{
	public:
		enum JobType { Null, Hit, Search };

		CJob(JobType nType = Null, void* pData = NULL, int nStage = 0)
			: m_nType( nType )
			, m_pData( pData )
			, m_nStage( nStage )
		{
		}

		CJob(const CJob& oJob)
			: m_nType( oJob.m_nType )
			, m_pData( oJob.m_pData )
			, m_nStage( oJob.m_nStage )
		{
		}

		CJob& operator=(const CJob& oJob)
		{
			m_nType = oJob.m_nType;
			m_pData = oJob.m_pData;
			m_nStage = oJob.m_nStage;
			return *this;
		}

		void Next()
		{
			++ m_nStage;
		}

		JobType GetType() const
		{
			return m_nType;
		}

		void* GetData() const
		{
			return m_pData;
		}

		int GetStage() const
		{
			return m_nStage;
		}

	protected:
		JobType	m_nType;
		void*	m_pData;
		int		m_nStage;
	};
	CList< CJob > m_oJobs;

	// Process asynchronous jobs (hits, searches, etc.)
	void		RunJobs();
	void		ClearJobs();

	// Handle and destroy query searches
	bool		ProcessQuerySearch(CNetwork::CJob& oJob);

	// Handle and destroy query hits
	bool		ProcessQueryHits(CNetwork::CJob& oJob);

	// Get asynchronously resolved host
	ResolveStruct* GetResolve(HANDLE hAsync);

	// Clear asynchronous resolver queue
	void		ClearResolve();

	// Restore WinINet connection to Internet
	bool		InternetConnect();

	bool		PreRun();
	void		OnRun();
	void		PostRun();

	// Create TCP and UDP port mappings
	void MapPorts();
	// Remove TCP and UDP port mappings
	void DeletePorts();
	// Create port mapping
	static BOOL MapPort(IStaticPortMappingCollection* pCollection, LPCWSTR szLocalIP, long nPort, LPCWSTR szProtocol, LPCWSTR szDescription);

// Operations
public:
	// Initialize network: Windows Sockets, Windows Firewall, UPnP NAT.
	BOOL		Init();
	// Shutdown network
	void		Clear();
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

	BOOL		Connect(BOOL bAutoConnect = FALSE);
	void		Disconnect();
	BOOL		ConnectTo(LPCTSTR pszAddress, int nPort = 0, PROTOCOLID nProtocol = PROTOCOL_NULL, BOOL bNoUltraPeer = FALSE);
	BOOL		AcquireLocalAddress(SOCKET hSocket);
	BOOL		AcquireLocalAddress(LPCTSTR pszHeader);
	BOOL		AcquireLocalAddress(const IN_ADDR& pAddress);
	static BOOL	Resolve(LPCTSTR pszHost, int nPort, SOCKADDR_IN* pHost, BOOL bNames = TRUE);
	BOOL		AsyncResolve(LPCTSTR pszAddress, WORD nPort, PROTOCOLID nProtocol, BYTE nCommand);
	// Pending network name resolves queue size
	UINT		GetResolveCount() const;
	BOOL		IsReserved(const IN_ADDR* pAddress) const;
	WORD		RandomPort() const;
	void		CreateID(Hashes::Guid& oID);
	BOOL		IsFirewalledAddress(const IN_ADDR* pAddress, BOOL bIncludeSelf = FALSE) const;
	WORD		GetPort() const;

	BOOL		GetNodeRoute(const Hashes::Guid& oGUID, CNeighbour** ppNeighbour, SOCKADDR_IN* pEndpoint);
	BOOL		RoutePacket(CG2Packet* pPacket);
	BOOL		SendPush(const Hashes::Guid& oGUID, DWORD nIndex = 0);
	BOOL		RouteHits(CQueryHit* pHits, CPacket* pPacket);
	void		OnWinsock(WPARAM wParam, LPARAM lParam);

	// Handle push for downloads, chats and browsers
	BOOL		OnPush(const Hashes::Guid& oGUID, CConnection* pConnection);

	// Add query search to queue
	void		OnQuerySearch(CLocalSearch* pSearch);

	// Add query hit to queue
	void		OnQueryHits(CQueryHit* pHits);

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
	// Safe way to call WSACleanup
	static void Cleanup();

	// Update TCP/UDP port mappings using UPnP
	void UpdatePortMapping()
	{
		m_tUPnPMap = 0;
	}
	// Got new external IP address. Called by UPnP-services
	void OnNewExternalIPAddress(const IN_ADDR& pAddress);
	// UPnP success (called by UPnP-services)
	void OnMapSuccess();
	// UPnP error (called by UPnP-services)
	void OnMapFailed();

	// INATNumberOfEntriesCallback interface
	BEGIN_INTERFACE_PART(NATNumberOfEntriesCallback, INATNumberOfEntriesCallback)
		STDMETHOD(NewNumberOfEntries)(/* [in] */ long lNewNumberOfEntries);
	END_INTERFACE_PART(NATNumberOfEntriesCallback)

	// INATExternalIPAddressCallback interface
	BEGIN_INTERFACE_PART(NATExternalIPAddressCallback, INATExternalIPAddressCallback)
		STDMETHOD(NewExternalIPAddress)(/* [in] */ BSTR bstrNewExternalIPAddress);
	END_INTERFACE_PART(NATExternalIPAddressCallback)

	DECLARE_INTERFACE_MAP()

	friend class CHandshakes;
	friend class CNeighbours;
};

extern CNetwork Network;