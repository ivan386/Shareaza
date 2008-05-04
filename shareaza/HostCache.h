//
// HostCache.h
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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

class CNeighbour;
class CG1Packet;
class CVendor;


class CHostCacheHost
{
public:
	CHostCacheHost(PROTOCOLID nProtocol = PROTOCOL_NULL);

	// Attributes: Host Information
	PROTOCOLID	m_nProtocol;		// Host protocol (PROTOCOL_*)
	IN_ADDR		m_pAddress;			// Host IP address
	WORD		m_nPort;			// Host TCP port number
	WORD		m_nUDPPort;			// Host UDP port number
	CVendor*	m_pVendor;			// Vendor handler from VendorCache
	BOOL		m_bPriority;		// Host cannot be removed on failure
	DWORD		m_nUserCount;		// G2 leaf count / ED2K user count
	DWORD		m_nUserLimit;		// G2 leaf limit / ED2K user limit
	DWORD		m_nFileLimit;		// ED2K-server file limit
	CString		m_sName;			// Host name
	CString		m_sDescription;		// Host description
	DWORD		m_nTCPFlags;		// ED2K TCP flags (ED2K_SERVER_TCP_*)
	DWORD		m_nUDPFlags;		// ED2K UDP flags (ED2K_SERVER_UDP_*)
	BOOL		m_bCheckedLocally;	// Host was successfully accessed via TCP or UDP
	CString		m_sCountry;			// Country code

	// Attributes: Contact Times
	DWORD		m_tAdded;			// Time when host was constructed (in ticks)
	DWORD		m_tRetryAfter;		// G2 retry time according G2_PACKET_RETRY_AFTER packet (in seconds)
	DWORD		m_tConnect;			// TCP connect time (in seconds)
	DWORD		m_tQuery;			// G2 / ED2K query time (in seconds)
	DWORD		m_tAck;				// Time when we sent something requires acknowledgment (0 - not required)
	DWORD		m_tStats;			// ED2K stats UDP request
	DWORD		m_tFailure;			// Last failure time
	DWORD		m_nFailures;		// Failures counter
	DWORD		m_nDailyUptime;		// Daily uptime

	// Attributes: Query Keys
	DWORD		m_tKeyTime;			// G2 time when query key was received
	DWORD		m_nKeyValue;		// G2 query key
	DWORD		m_nKeyHost;			// G2 query key host

	// Attributes: DHT
	BOOL			m_bDHT;			// Host is DHT capable
	Hashes::BtGuid	m_oBtGUID;		// Host GUID (160 bit)
	CArray< BYTE >	m_Token;		// Host access token

	// Attributes: Kademlia
	Hashes::Guid	m_oGUID;		// Host GUID (128 bit)
	BYTE			m_nKADVersion;	// Kademlia version

	CNeighbour*	ConnectTo(BOOL bAutomatic = FALSE);
	CString		ToString() const;
	bool		IsExpired(DWORD tNow) const;		// Is this host expired?
	bool		IsThrottled(DWORD tNow) const;		// Is host temporary throttled down?
	BOOL		CanConnect(DWORD tNow = 0) const;	// Can we connect to this host now?
	BOOL		CanQuote(DWORD tNow = 0) const;		// Is this a recently seen host?
	BOOL		CanQuery(DWORD tNow = 0) const;		// Can we UDP query this host? (G2/ed2k)
	void		SetKey(DWORD nKey, const IN_ADDR* pHost = NULL);

	inline DWORD Seen() const throw()
	{
		return m_tSeen;
	}

protected:
	DWORD		m_tSeen;

	// Return: true - if tSeen cnaged, false - otherwise.
	bool		Update(WORD nPort, DWORD tSeen = 0, LPCTSTR pszVendor = NULL, DWORD nUptime = 0);
	void		Serialize(CArchive& ar, int nVersion);

	inline bool	IsValid() const throw()
	{
		return m_nProtocol != PROTOCOL_NULL &&
			m_pAddress.s_addr != INADDR_ANY && m_pAddress.s_addr != INADDR_NONE &&
			m_nPort != 0 &&
			m_tSeen != 0;
	}

	friend class CHostCacheList;
};

typedef CHostCacheHost* CHostCacheHostPtr;

template<>
struct std::less< IN_ADDR > : public std::binary_function< IN_ADDR, IN_ADDR, bool>
{
	inline bool operator()(const IN_ADDR& _Left, const IN_ADDR& _Right) const throw()
	{
		return ( ntohl( _Left.s_addr ) < ntohl( _Right.s_addr ) );
	}
};

typedef std::map< IN_ADDR, CHostCacheHostPtr > CHostCacheMap;
typedef std::pair< IN_ADDR, CHostCacheHostPtr > CHostCacheMapPair;

template<>
struct std::less< CHostCacheHostPtr > : public std::binary_function< CHostCacheHostPtr, CHostCacheHostPtr, bool>
{
	inline bool operator()(const CHostCacheHostPtr& _Left, const CHostCacheHostPtr& _Right) const throw()
	{
		return ( _Left->Seen() > _Right->Seen() );
	}
};

typedef std::multiset< CHostCacheHostPtr > CHostCacheIndex;
typedef std::pair < CHostCacheIndex::iterator, CHostCacheIndex::iterator > CHostCacheTimeItPair;
typedef CHostCacheIndex::const_iterator CHostCacheIterator;
typedef CHostCacheIndex::const_reverse_iterator CHostCacheRIterator;

struct good_host : public std::binary_function< CHostCacheMapPair, BOOL, bool>
{
	inline bool operator()(const CHostCacheMapPair& _Pair, const BOOL& _bLocally) const throw()
	{
		return ( _Pair.second->m_nFailures == 0 &&
			( _Pair.second->m_bCheckedLocally || _bLocally ) );
	}
};


class CHostCacheList
{
public:
	CHostCacheList(PROTOCOLID nProtocol);
	virtual ~CHostCacheList();

	PROTOCOLID			m_nProtocol;
	DWORD				m_nCookie;
	mutable CMutex		m_pSection;

	CHostCacheHostPtr	Add(IN_ADDR* pAddress, WORD nPort, DWORD tSeen = 0, LPCTSTR pszVendor = NULL, DWORD nUptime = 0);
	// Add host in form "IP:Port SeenTime"
	BOOL				Add(LPCTSTR pszHost, DWORD tSeen = 0, LPCTSTR pszVendor = NULL, DWORD nUptime = 0);
	void				Update(CHostCacheHostPtr pHost, WORD nPort = 0, DWORD tSeen = 0, LPCTSTR pszVendor = NULL, DWORD nUptime = 0);
	bool				Remove(CHostCacheHostPtr pHost);
	bool				Remove(const IN_ADDR* pAddress);
	void				OnFailure(const IN_ADDR* pAddress, WORD nPort, bool bRemove=true);
	void				OnSuccess(const IN_ADDR* pAddress, WORD nPort, bool bUpdate=true);
	void				PruneByQueryAck();			// For G2
	void				PruneOldHosts();
	void				Clear();
	void				Serialize(CArchive& ar, int nVersion);

	inline CHostCacheIterator Begin() const throw()
	{
		return m_HostsTime.begin();
	}

	inline CHostCacheIterator End() const throw()
	{
		return m_HostsTime.end();
	}

	inline CHostCacheRIterator RBegin() const throw()
	{
		return m_HostsTime.rbegin();
	}

	inline CHostCacheRIterator REnd() const throw()
	{
		return m_HostsTime.rend();
	}

	inline const CHostCacheHostPtr GetNewest() const throw()
	{
		return IsEmpty() ? NULL : *Begin();
	}

	inline const CHostCacheHostPtr GetOldest() const throw()
	{
		return IsEmpty() ? NULL : *( End()-- );
	}

	inline bool IsEmpty() const throw()
	{
		return m_HostsTime.empty();
	}

	inline DWORD GetCount() const throw()
	{
		return (DWORD)m_Hosts.size();
	}

	inline CHostCacheHostPtr Find(const IN_ADDR* pAddress) const throw()
	{
		CQuickLock oLock( m_pSection );
		CHostCacheMap::const_iterator i = m_Hosts.find( *pAddress );
		return ( i != m_Hosts.end() ) ? (*i).second : NULL;
	}

	inline bool Check(const CHostCacheHostPtr pHost) const throw()
	{
		CQuickLock oLock( m_pSection );
		return std::find( m_HostsTime.begin(), m_HostsTime.end(), pHost )
			!= m_HostsTime.end();
	}

	inline DWORD CountHosts(const BOOL bCountUncheckedLocally = FALSE) const throw()
	{
		CQuickLock oLock( m_pSection );
		return (DWORD)(size_t) std::count_if( m_Hosts.begin(), m_Hosts.end(),
			std::bind2nd( good_host(), bCountUncheckedLocally ) );
	}

	inline const CHostCacheHostPtr GetForDHTQuery() const throw()
	{
		for ( CHostCacheIterator it = m_HostsTime.begin();
			it != m_HostsTime.end(); ++it )
		{
			CHostCacheHost* pHost = (*it);
			if ( pHost->CanQuery() && pHost->m_bDHT && pHost->m_oBtGUID )
				return pHost;
		}
		return NULL;
	}

	inline const CHostCacheHostPtr GetOldestForQuery() const throw()
	{
		for ( CHostCacheRIterator it = m_HostsTime.rbegin();
			it != m_HostsTime.rend(); ++it )
		{
			CHostCacheHost* pHost = (*it);
			if ( pHost->CanQuery() )
				return pHost;
		}
		return NULL;
	}

protected:
	CHostCacheMap				m_Hosts;		// Hosts map (sorted by IP)
	CHostCacheIndex				m_HostsTime;	// Host index (sorted from newer to older)

	CHostCacheHostPtr	AddInternal(const IN_ADDR* pAddress, WORD nPort, DWORD tSeen, LPCTSTR pszVendor, DWORD nUptime);
	void				PruneHosts();
};


class CHostCache
{
public:
	CHostCache();

	CHostCacheList	Gnutella1;
	CHostCacheList	Gnutella2;
	CHostCacheList	eDonkey;
	CHostCacheList	G1DNA;
	CHostCacheList	BitTorrent;
	CHostCacheList	Kademlia;

	BOOL				Load();
	BOOL				Save();
	int					Import(LPCTSTR pszFile);
	int					ImportMET(CFile* pFile);
	int					ImportNodes(CFile* pFile);
	bool				CheckMinimumED2KServers();
	CHostCacheHostPtr	Find(const IN_ADDR* pAddress) const;
	BOOL				Check(const CHostCacheHostPtr pHost) const;
	void				Remove(CHostCacheHostPtr pHost);
	void				OnFailure(const IN_ADDR* pAddress, WORD nPort, 
							  PROTOCOLID nProtocol=PROTOCOL_NULL, bool bRemove=true);
	void				OnSuccess(const IN_ADDR* pAddress, WORD nPort, 
							  PROTOCOLID nProtocol=PROTOCOL_NULL, bool bUpdate=true);
	void				PruneOldHosts();

	inline bool EnoughED2KServers() const throw()
	{
		return ( eDonkey.CountHosts( TRUE ) >= 8 );
	}

	inline CHostCacheList* ForProtocol(PROTOCOLID nProtocol) throw()
	{
		switch ( nProtocol )
		{
		case PROTOCOL_G1:
			return &Gnutella1;
		case PROTOCOL_G2:
			return &Gnutella2;
		case PROTOCOL_ED2K:
			return &eDonkey;
		case PROTOCOL_BT:
			return &BitTorrent;
		case PROTOCOL_KAD:
			return &Kademlia;
		case PROTOCOL_NULL:
		case PROTOCOL_ANY:
		case PROTOCOL_HTTP:
		case PROTOCOL_FTP:
		default:
			ASSERT(FALSE);
			return NULL;
		}
	}

protected:
	CList< CHostCacheList* >	m_pList;
	mutable CCriticalSection	m_pSection;
	DWORD						m_tLastPruneTime;

	void				Serialize(CArchive& ar);
	void				Clear();
	void				DoED2KServersImport();
	int					LoadDefaultED2KServers();
};

extern CHostCache HostCache;
