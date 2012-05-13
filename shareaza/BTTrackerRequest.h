//
// BTTrackerRequest.h
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

#include "Packet.h"
#include "HttpRequest.h"

class CBENode;
class CBTTrackerPacket;
class CBTTrackerRequest;
class CDownload;


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
	DWORD		event;			// The event, one of: none = 0; completed = 1; started = 2; stopped = 3.
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
enum { BTE_TRACKER_UPDATE, BTE_TRACKER_COMPLETED, BTE_TRACKER_STARTED, BTE_TRACKER_STOPPED, BTE_TRACKER_SCRAPE };


#pragma pack(pop)		// 1
#pragma warning(pop)	// C4200

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
	virtual	void		ToBuffer(CBuffer* pBuffer, bool bTCP = true) const;
	static	CBTTrackerPacket*	ReadBuffer(CBuffer* pBuffer);
	virtual void		SmartDump(const SOCKADDR_IN* pAddress, BOOL bUDP, BOOL bOutgoing, DWORD_PTR nNeighbourUnique = 0) const;
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

class CBTTrackerRequest
{
public:
	CBTTrackerRequest(CDownload* pDownload, DWORD nEvent, DWORD nNumWant, CTrackerEvent* pOnTrackerEvent);

	ULONG	AddRef();
	ULONG	Release();

	DWORD						m_nSeeders;			// Scrape
	DWORD						m_nDownloaded;		// Scrape
	DWORD						m_nLeechers;		// Scrape

    static CString Escape(const Hashes::BtHash& oBTH);
    static CString Escape(const Hashes::BtGuid& oGUID);

	inline void Cancel()
	{
		m_pOnTrackerEvent = NULL; // Disable notification

		m_pCancel.SetEvent();

		if ( m_pRequest )
		{
			m_pRequest->Cancel();
		}
	}

	inline bool IsCanceled() const
	{
		return ( WaitForSingleObject( m_pCancel, 0 ) != WAIT_TIMEOUT );
	}

	BOOL OnConnect(CBTTrackerPacket* pPacket);
	BOOL OnAnnounce(CBTTrackerPacket* pPacket);
	BOOL OnScrape(CBTTrackerPacket* pPacket);
	BOOL OnError(CBTTrackerPacket* pPacket);

protected:
	volatile LONG				m_dwRef;			// Reference counter
	bool						m_bHTTP;			// HTTP - TRUE, UDP - FALSE.
	CString						m_sURL;				// Tracker URL
	SOCKADDR_IN					m_pHost;			// Resolved tracker address (UDP)
	CDownload*					m_pDownload;		// Handle of owner download
	CString						m_sName;			// Name of download
	CAutoPtr< CHttpRequest >	m_pRequest;			// HTTP request object
	DWORD						m_nEvent;			// Tracker event (update, announce, etc.)
	DWORD						m_nNumWant;			// Number of peers wanted
	QWORD						m_nConnectionID;	// UDP tracker connection ID
	DWORD						m_nTransactionID;	// UDP tracker transaction ID
	CEvent						m_pCancel;			// Cancel flag
	CTrackerEvent*				m_pOnTrackerEvent;	// Callback

	virtual ~CBTTrackerRequest();

	void ProcessHTTP();
	void ProcessUDP();
	void Process(const CBENode* pRoot);
	static UINT	ThreadStart(LPVOID pParam);
	void OnRun();
	void OnTrackerEvent(bool bSuccess, LPCTSTR pszReason, LPCTSTR pszTip = NULL);
};

template<>
inline void CAutoPtr< CBTTrackerRequest >::Free() throw()
{
	if ( m_p )
	{
		m_p->Release();
		m_p = NULL;
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

	DWORD Add(CBTTrackerRequest* pRequest);
	void Remove(DWORD nTransactionID);
	CBTTrackerRequest* Lookup(DWORD nTransactionID) const;
	BOOL Check(DWORD nTransactionID) const;

protected:
	typedef CMap< DWORD, DWORD, CBTTrackerRequest*, CBTTrackerRequest* > CBTTrackerRequestMap;

	mutable CCriticalSection	m_pSection;
	CBTTrackerRequestMap		m_pTrackerRequests;	// Tracker ID to tracker pointer map
};

extern CBTTrackerRequests TrackerRequests;
