//
// DiscoveryServices.h
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

#if !defined(AFX_DISCOVERYSERVICES_H__2AD74990_0834_4FE8_AE38_1DD609232BC7__INCLUDED_)
#define AFX_DISCOVERYSERVICES_H__2AD74990_0834_4FE8_AE38_1DD609232BC7__INCLUDED_

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
	CPtrList			m_pList;
	HANDLE				m_hThread;
	HINTERNET			m_hInternet;
	HINTERNET			m_hRequest;
	CDiscoveryService*	m_pWebCache;
	int					m_nWebCache;
	CDiscoveryService*	m_pSubmit;
	DWORD				m_tQueried;					// Time a webcache was last queried
	PROTOCOLID			m_nLastQueryProtocol;		// Protocol that was queried most recently
	DWORD				m_tUpdated;					// Time a webcache was last updated
	PROTOCOLID			m_nLastUpdateProtocol;		// Protocol that had a service update most recently
	DWORD				m_tExecute;					// Time the Execute() function was last run
	BOOL				m_bFirstTime;

// Operations
public:
	POSITION			GetIterator() const;
	CDiscoveryService*	GetNext(POSITION& pos) const;
	BOOL				Check(CDiscoveryService* pService, int nType = -1) const;
	int					GetCount(int nType = 0, PROTOCOLID nProtocol = PROTOCOL_NULL) const;
	CDiscoveryService*	Add(LPCTSTR pszAddress, int nType, PROTOCOLID nProtocol = PROTOCOL_NULL);
	CDiscoveryService*	Add(CDiscoveryService* pService);
	void				Remove(CDiscoveryService* pService, BOOL bCheck = TRUE);
	BOOL				CheckWebCacheValid(LPCTSTR pszAddress);
	CDiscoveryService*	GetByAddress(LPCTSTR pszAddress) const;
	void				Clear();
	BOOL				CheckMinimumServices();
public:
	BOOL				Load();
	BOOL				Save();
	BOOL				Update();
	BOOL				Execute(BOOL bSecondary = FALSE);
	BOOL				ExecuteDonkey();
	void				Stop();
	void				OnGnutellaAdded(IN_ADDR* pAddress, int nCount);
	void				OnGnutellaFailed(IN_ADDR* pAddress);
protected:
	void				Serialize(CArchive& ar);
	BOOL				EnoughServices() const;
	void				AddDefaults();
	int					ExecuteBootstraps(int nCount);
	BOOL				RequestRandomService(PROTOCOLID nProtocol);	
	CDiscoveryService*  GetRandomService(PROTOCOLID nProtocol);
	CDiscoveryService*	GetRandomWebCache(PROTOCOLID nProtocol, BOOL bWorkingOnly, CDiscoveryService* pExclude = NULL, BOOL bForUpdate = FALSE);
	BOOL				RequestWebCache(CDiscoveryService* pService, int nMode, PROTOCOLID nProtocol);
	void				StopWebRequest();
protected:
	static UINT			ThreadStart(LPVOID pParam);
	void				OnRun();
	BOOL				RunWebCacheGet(BOOL bCache);
	BOOL				RunWebCacheUpdate();
	BOOL				RunServerMet();
	BOOL				SendWebCacheRequest(CString strURL, CString& strOutput);

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
public:
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

	enum
	{
		dsNull, dsGnutella, dsWebCache, dsServerMet, dsBlocked
	};
	
// Operations
public:
	void		Remove(BOOL bCheck = TRUE);
	BOOL		Execute(int nMode = 0);
	void		OnAccess();
	void		OnHostAdd(int nCount = 1);
	void		OnSuccess();
	void		OnFailure();
protected:
	void		Serialize(CArchive& ar, int nVersion);
	BOOL		ResolveGnutella();
	
	friend class CDiscoveryServices;
	
};

extern CDiscoveryServices DiscoveryServices;


#endif // !defined(AFX_DISCOVERYSERVICES_H__2AD74990_0834_4FE8_AE38_1DD609232BC7__INCLUDED_)
