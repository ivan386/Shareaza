//
// BTTrackerRequest.h
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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

#include "Packet.h"
#include "ThreadImpl.h"

class CBENode;
class CBTTrackerPacket;
class CBTTrackerRequest;
class CDownload;
class CHttpRequest;


#pragma warning(push)
#pragma warning(disable:4200)
#pragma pack(push,1)

// BitTorrent UDP tracker connecting packet

#define bt_connection_magic	0x41727101980

typedef struct
{
	QWORD		connection_id;	// Must be initialized to 0x41727101980 in network byte order. This will identify the protocol.
	DWORD		action;			// Action. in this case, 0 for connecting. See actions.
	DWORD		transaction_id;	// Randomized by client.
} bt_udp_connecting_request_t;

typedef struct
{
	DWORD		action;			// Describes the type of packet, in this case it should be 0, for connect. If 3 (for error) see errors.
	DWORD		transaction_id;	// Must match the transaction_id sent from the client.
	QWORD		connection_id;	// A connection id, this is used when further information is exchanged with the tracker, to identify you. This connection id can be reused for multiple requests, but if it's cached for too long, it will not be valid anymore.
} bt_udp_connecting_response_t;

typedef struct
{
	DWORD		action;			// The action, in this case 3, for error. See actions.
	DWORD		transaction_id;	// Must match the transaction_id sent from the client.
	BYTE		error_string[];	// The rest of the packet is a string describing the error.
} bt_udp_error_response_t;

// BitTorrent UDP tracker announcing packet

typedef struct
{
	QWORD		connection_id;	// The connection id acquired from establishing the connection.
	DWORD		action;			// Action. in this case, 1 for announce. See actions.
	DWORD		transaction_id;	// Randomized by client.
	BYTE		info_hash[20];	// The info-hash of the torrent you want announce yourself in.
	BYTE		peer_id[20];	// Your peer id.
	QWORD		downloaded;		// The number of byte you've downloaded in this session.
	QWORD		left;			// The number of bytes you have left to download until you're finished.
	QWORD		uploaded;		// The number of bytes you have uploaded in this session.
	DWORD		event;			// The event, one of: update = 0; completed = 1; started = 2; stopped = 3.
	DWORD		ip;				// Your ip address. Set to 0 if you want the tracker to use the sender of this UDP packet.
	DWORD		key;			// A unique key that is randomized by the client.
	DWORD		num_want;		// The maximum number of peers you want in the reply. Use -1 for default.
	WORD		port;			// The port you're listening on.
	WORD		extensions;		// See extensions
} bt_udp_announcing_request_t;

typedef struct
{
	DWORD		ip;				// The ip of a peer in the swarm.
	WORD		port;			// The peer's listen port.
} bt_peer_t;

typedef struct
{
	DWORD		action;			// The action this is a reply to. Should in this case be 1 for announce. If 3 (for error) see errors. See actions.
	DWORD		transaction_id;	// Must match the transaction_id sent in the announce request.
	DWORD		interval;		// The number of seconds you should wait until re-announcing yourself.
	DWORD		leechers;		// The number of peers in the swarm that has not finished downloading.
	DWORD		seeders;		// The number of peers in the swarm that has finished downloading and are seeding.
	bt_peer_t	peers[];		// The rest of the packet is a list of peers.
} bt_udp_announcing_response_t;

// BitTorrent UDP tracker scraping packet

typedef struct
{
	BYTE		info_hash[20];	// The info hash that is to be scraped.
} bt_hash_t;

typedef struct
{
	QWORD		connection_id;	// The connection id retrieved from the establishing of the connection.
	DWORD		action;			// The action, in this case, 2 for scrape. See actions.
	DWORD		transaction_id;	// Randomized by client.
	bt_hash_t	info_hashes[];	// The rest of the packet is a list of info-hashes to scrape (limited by the MTU).
} bt_udp_scraping_request_t;

typedef struct
{
	DWORD		seeders;		// The current number of connected seeds.
	DWORD		downloaded;		// The number of times this torrent has been downloaded.
	DWORD		leechers;		// The current number of connected leechers.
} bt_scrape_t;

typedef struct
{
	DWORD		action;			// The action, should in this case be 2 for scrape. If 3 (for error) see errors.
	DWORD		transaction_id;	// Must match the sent transaction id.
	bt_scrape_t	scrapes[];		// The rest of the packet contains the following structures once for each info-hash you asked in the scrape request.
} bt_udp_scraping_response_t;

// BitTorrent UDP tracker actions
enum { BTA_TRACKER_CONNECT, BTA_TRACKER_ANNOUNCE, BTA_TRACKER_SCRAPE, BTA_TRACKER_ERROR };

// BitTorrent UDP tracker events
typedef enum { BTE_TRACKER_UPDATE, BTE_TRACKER_COMPLETED, BTE_TRACKER_STARTED, BTE_TRACKER_STOPPED, BTE_TRACKER_SCRAPE } BTTrackerEvent;


#pragma pack(pop)		// 1
#pragma warning(pop)	// C4200

//
// BitTorrent tracker source and source list
//

class CBTTrackerSource
{
public:
	CBTTrackerSource()
		: m_pPeerID	()
		, m_pAddress()
	{
	}

	CBTTrackerSource(const Hashes::BtGuid& pPeerID, const SOCKADDR_IN& pAddress)
		: m_pPeerID	( pPeerID )
		, m_pAddress( pAddress )
	{
	}

	CBTTrackerSource(const CBTTrackerSource& source)
		: m_pPeerID	( source.m_pPeerID )
		, m_pAddress( source.m_pAddress )
	{
	}

	CBTTrackerSource& operator=(const CBTTrackerSource& source)
	{
		m_pPeerID = source.m_pPeerID;
		m_pAddress = source.m_pAddress;
		return *this;
	}

	Hashes::BtGuid	m_pPeerID;
	SOCKADDR_IN		m_pAddress;
};

typedef CList< CBTTrackerSource > CBTTrackerSourceList;

//
// BitTorrent tracker packet
//

class CBTTrackerPacket : public CPacket
{
protected:
	CBTTrackerPacket();
	virtual ~CBTTrackerPacket();

public:
	DWORD	m_nAction;
	DWORD	m_nTransactionID;
	QWORD	m_nConnectionID;

	virtual void		Reset();
	virtual	void		ToBuffer(CBuffer* pBuffer, bool bTCP = true);
	static	CBTTrackerPacket*	ReadBuffer(CBuffer* pBuffer);
	virtual CString		GetType() const;
	virtual CString		ToHex()   const;
	virtual CString		ToASCII() const;

// Packet Pool
protected:
	class CBTTrackerPacketPool : public CPacketPool
	{
	public:
		virtual ~CBTTrackerPacketPool() { Clear(); }
	protected:
		virtual void NewPoolImpl(int nSize, CPacket*& pPool, int& nPitch);
		virtual void FreePoolImpl(CPacket* pPool);
	};

	static CBTTrackerPacketPool POOL;

// Allocation
public:
	static CBTTrackerPacket* New(DWORD nAction, DWORD nTransactionID, QWORD nConnectionID, const BYTE* pBuffer = NULL, DWORD nLength = 0);
	static CBTTrackerPacket* New(const BYTE* pBuffer, DWORD nLength);

	inline virtual void Delete()
	{
		POOL.Delete( this );
	}

	// Packet handler
	virtual BOOL OnPacket(const SOCKADDR_IN* pHost);

	friend class CBTTrackerPacket::CBTTrackerPacketPool;

private:
	CBTTrackerPacket(const CBTTrackerPacket&);
	CBTTrackerPacket& operator=(const CBTTrackerPacket&);
};

inline void CBTTrackerPacket::CBTTrackerPacketPool::NewPoolImpl(int nSize, CPacket*& pPool, int& nPitch)
{
	nPitch	= sizeof( CBTTrackerPacket );
	pPool	= new CBTTrackerPacket[ nSize ];
}

inline void CBTTrackerPacket::CBTTrackerPacketPool::FreePoolImpl(CPacket* pPacket)
{
	delete [] (CBTTrackerPacket*)pPacket;
}


//
// BitTorrent tracker request event notification interface
//

class CTrackerEvent
{
public:
	virtual void OnTrackerEvent(bool bSuccess, LPCTSTR pszReason, LPCTSTR pszTip, CBTTrackerRequest* pEvent) = 0;
};

//
// BitTorrent tracker request
//

class CBTTrackerRequest : public CThreadImpl
{
public:
	ULONG	AddRef();
	ULONG	Release();
	
	inline BTTrackerEvent GetEvent() const { return m_nEvent; }		// Tracker event (update, announce, etc.)
	inline DWORD GetComplete() const { return m_nComplete; }		// Seeders
	inline DWORD GetDownloaded() const { return m_nDownloaded; }	// Downloaded
	inline DWORD GetIncomplete() const { return m_nIncomplete; }	// Leeches

	// Tracker request interval (in seconds)
	inline DWORD GetInterval() const { return min( max( m_nInterval, 60ul * 2ul ), 60ul * 60ul ); }

	// Retrieve peers
	inline POSITION GetSources() const { return m_pSources.GetHeadPosition(); }
	inline const CBTTrackerSource& GetNextSource(POSITION& rPosition) const { return m_pSources.GetNext( rPosition ); }

    static CString Escape(const Hashes::BtHash& oBTH);
    static CString Escape(const Hashes::BtGuid& oGUID);

	void Cancel();

	BOOL OnConnect(CBTTrackerPacket* pPacket);
	BOOL OnAnnounce(CBTTrackerPacket* pPacket);
	BOOL OnScrape(CBTTrackerPacket* pPacket);
	BOOL OnError(CBTTrackerPacket* pPacket);

protected:
	volatile LONG				m_dwRef;			// Reference counter
	bool						m_bHTTP;			// HTTP - TRUE, UDP - FALSE.
	CString						m_sURL;				// Tracker full URL
	SOCKADDR_IN					m_pHost;			// Resolved tracker address (UDP)
	Hashes::BtHash				m_oBTH;				// BitTorrent Info Hash (Base32)
	Hashes::BtGuid				m_pPeerID;			// Shareaza Peer ID
	QWORD						m_nTorrentUploaded;
	QWORD						m_nTorrentDownloaded;
	QWORD						m_nTorrentLeft;
	CDownload*					m_pDownload;		// Handle of owner download
	CString						m_sName;			// Name of download
	CString						m_sAddress;			// Tracker original URL
	CAutoPtr< CHttpRequest >	m_pRequest;			// HTTP request object
	BTTrackerEvent				m_nEvent;			// Tracker event (update, announce, etc.)
	DWORD						m_nNumWant;			// Number of peers wanted
	QWORD						m_nConnectionID;	// UDP tracker connection ID
	DWORD						m_nTransactionID;	// UDP tracker transaction ID
	CTrackerEvent*				m_pOnTrackerEvent;	// Callback
	DWORD						m_nComplete;		// Seeders
	DWORD						m_nDownloaded;		// Downloaded
	DWORD						m_nIncomplete;		// Leeches
	DWORD						m_nInterval;		// Tracker request interval (in seconds)
	CBTTrackerSourceList		m_pSources;			// Peers

	CBTTrackerRequest(CDownload* pDownload, BTTrackerEvent nEvent, DWORD nNumWant, CTrackerEvent* pOnTrackerEvent);
	virtual ~CBTTrackerRequest();

	void ProcessHTTP();
	void ProcessUDP();
	void Process(const CBENode* pRoot);
	void OnRun();
	void OnTrackerEvent(bool bSuccess, LPCTSTR pszReason, LPCTSTR pszTip = NULL);

	friend class CBTTrackerRequests;

private:
	CBTTrackerRequest(const CBTTrackerRequest&);
	CBTTrackerRequest& operator=(const CBTTrackerRequest&);
};

template<>
inline void CAutoPtr< CBTTrackerRequest >::Free() throw()
{
	if ( m_p )
	{
		CBTTrackerRequest* p = m_p;
		m_p = NULL;
		p->Release();
	}
}

//
// BitTorrent Tracker request manager
//

class CBTTrackerRequests
{
public:
	CBTTrackerRequests();
	~CBTTrackerRequests();

	// Create tracker request. Return: transaction ID (0 if error).
	DWORD Request(CDownload* pDownload, BTTrackerEvent nEvent, DWORD nNumWant = 0, CTrackerEvent* pOnTrackerEvent = NULL);

	CBTTrackerRequest* Lookup(DWORD nTransactionID) const;

	BOOL Check(DWORD nTransactionID) const;

	// Cancel tracker request
	void Cancel(DWORD nTransactionID);

	// Cancel all requests
	void Clear();

protected:
	typedef CMap< DWORD, DWORD, CBTTrackerRequest*, CBTTrackerRequest* > CBTTrackerRequestMap;

	mutable CCriticalSection	m_pSection;
	CBTTrackerRequestMap		m_pTrackerRequests;	// Tracker ID to tracker pointer map

	void Remove(DWORD nTransactionID);
	CBTTrackerRequest* GetFirst() const;

	friend class CBTTrackerRequest;
};

extern CBTTrackerRequests TrackerRequests;
