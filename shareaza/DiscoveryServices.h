//
// DiscoveryServices.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

class CDiscoveryService;


class CDiscoveryServices
{
// Construction
public:
	CDiscoveryServices();
	virtual ~CDiscoveryServices();
	
	enum { wcmHosts, wcmCaches, wcmUpdate, wcmSubmit, wcmServerMet };

// Attributes
protected:
	CList< CDiscoveryService* > m_pList;
	HANDLE				m_hThread;
	HINTERNET			m_hInternet;
	HINTERNET			m_hRequest;
	CDiscoveryService*	m_pWebCache;
	int					m_nWebCache;
	CDiscoveryService*	m_pSubmit;
	PROTOCOLID			m_nLastQueryProtocol;		// Protocol that was queried most recently
	DWORD				m_tUpdated;					// Time a webcache was last updated
	PROTOCOLID			m_nLastUpdateProtocol;		// Protocol that had a service update most recently
	BOOL				m_bFirstTime;
	DWORD				m_tExecute;					// Time the Execute() function was last run
	DWORD				m_tQueried;					// Time a webcache/MET was last queried
	DWORD				m_tMetQueried;				// Time a MET was last queried

// Operations
public:
	POSITION			GetIterator() const;
	CDiscoveryService*	GetNext(POSITION& pos) const;
	BOOL				Check(CDiscoveryService* pService, int nType = -1) const;
	CDiscoveryService*	Add(CDiscoveryService* pService);
	CDiscoveryService*	Add(LPCTSTR pszAddress, int nType, PROTOCOLID nProtocol = PROTOCOL_NULL);
	BOOL				CheckMinimumServices();
//	DWORD				MetQueried() const;
	DWORD				LastExecute() const;
	CDiscoveryService*	GetByAddress(LPCTSTR pszAddress) const;
	CDiscoveryService*	GetByAddress( IN_ADDR* pAddress, WORD nPort, int nSubType );
	BOOL				Load();
	BOOL				Save();
	BOOL				Update();
	BOOL				Execute(BOOL bDiscovery, PROTOCOLID nProtocol, USHORT nForceDiscovery);
	void				Stop();
	void				OnGnutellaAdded(IN_ADDR* pAddress, int nCount);
	void				OnGnutellaFailed(IN_ADDR* pAddress);

protected:
	void				Remove(CDiscoveryService* pService, BOOL bCheck = TRUE);
	DWORD				GetCount(int nType = 0, PROTOCOLID nProtocol = PROTOCOL_NULL) const;
	BOOL				CheckWebCacheValid(LPCTSTR pszAddress);
	void				Clear();
	int					ExecuteBootstraps( int nCount, BOOL bUDP = FALSE, PROTOCOLID nProtocol = PROTOCOL_NULL );
	void				Serialize(CArchive& ar);
	BOOL				RequestRandomService(PROTOCOLID nProtocol);	
	CDiscoveryService*  GetRandomService(PROTOCOLID nProtocol);
	CDiscoveryService*	GetRandomWebCache(PROTOCOLID nProtocol, BOOL bWorkingOnly, CDiscoveryService* pExclude = NULL, BOOL bForUpdate = FALSE);
	BOOL				RequestWebCache(CDiscoveryService* pService, int nMode, PROTOCOLID nProtocol);
	void				StopWebRequest();
	static UINT			ThreadStart(LPVOID pParam);
	void				OnRun();
	BOOL				RunWebCacheGet(BOOL bCache);
	BOOL				RunWebCacheUpdate();
	BOOL				RunServerMet();
	BOOL				SendWebCacheRequest(CString strURL, CString& strOutput);
	BOOL				EnoughServices() const;
	void				AddDefaults();

	friend class CDiscoveryService;
};


class CDiscoveryService
{
// Construction
public:
	CDiscoveryService(int nType = 0, LPCTSTR pszAddress = NULL);
	virtual ~CDiscoveryService();

// Attributes
public:
	int			m_nType;
	CString		m_sAddress;
	BOOL		m_bGnutella2;			// Webcache supports Gnutella 2
	BOOL		m_bGnutella1;			// Webcache supports Gnutella
	DWORD		m_tCreated;
	DWORD		m_tAccessed;
	DWORD		m_nAccesses;
	DWORD		m_tUpdated;
	DWORD		m_nUpdates;
	DWORD		m_nHosts;
	DWORD		m_nFailures;
	DWORD		m_nAccessPeriod;
	DWORD		m_nUpdatePeriod;
	int			m_nSubType; // 0 = old BootStrap, 1 = Gnutella TCP, 2 = Gnutella2 TCP, 3 = Gnutella UDPHC, 4 = Gnutella2 UDPKHL
	IN_ADDR		m_pAddress;
	WORD		m_nPort;

	enum
	{
		dsNull, dsGnutella, dsWebCache, dsServerMet, dsBlocked
	};
	
// Operations
public:
	void		Remove(BOOL bCheck = TRUE);
	BOOL		Execute(int nMode = 0);
	void		OnSuccess();
	void		OnFailure();

protected:
	void		OnAccess();
	void		OnHostAdd(int nCount = 1);
	void		Serialize(CArchive& ar, int nVersion);
	BOOL		ResolveGnutella();
	
	friend class CDiscoveryServices;	
};

extern CDiscoveryServices DiscoveryServices;
