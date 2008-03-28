//
// Datagrams.h
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

#pragma pack(1)

typedef struct
{
	CHAR	szTag[3];
	BYTE	nFlags;
	WORD	nSequence;
	BYTE	nPart;
	BYTE	nCount;
} SGP_HEADER;

#pragma pack()

// #define SGP_TAG_1	"SGP"
#define SGP_TAG_2		"GND"
#define SGP_DEFLATE		0x01
#define SGP_ACKNOWLEDGE	0x02

typedef struct
{
	DWORD		nTotal;
	DWORD		tLast;
	DWORD		nMeasure;
	DWORD		pHistory[24];
	DWORD		pTimes[24];
	DWORD		nPosition;
	DWORD		tLastAdd;
	DWORD		tLastSlot;
} UDPBandwidthMeter;

class CBuffer;
class CPacket;
class CG1Packet;
class CG2Packet;
class CDatagramIn;
class CDatagramOut;
class CBENode;


class CDatagrams
{
// Construction
public:
	CDatagrams();
	virtual ~CDatagrams();

// Attributes
protected:
	SOCKET		m_hSocket;
	WORD		m_nSequence;
	BOOL		m_bStable;
	DWORD		m_tLastWrite;
protected:
	CBuffer*	m_pBufferBuffer;
	DWORD		m_nBufferBuffer;
	CBuffer*	m_pBufferFree;
	DWORD		m_nBufferFree;
protected:
	CDatagramIn*	m_pInputBuffer;
	DWORD			m_nInputBuffer;
	CDatagramIn*	m_pInputFree;
	CDatagramIn*	m_pInputFirst;
	CDatagramIn*	m_pInputLast;
	CDatagramIn*	m_pInputHash[32];
protected:
	CDatagramOut*	m_pOutputBuffer;
	DWORD			m_nOutputBuffer;
	CDatagramOut*	m_pOutputFree;
	CDatagramOut*	m_pOutputFirst;
	CDatagramOut*	m_pOutputLast;
	CDatagramOut*	m_pOutputHash[32];
public:
	UDPBandwidthMeter	m_mInput;
	DWORD				m_nInBandwidth;
	DWORD				m_nInFrags;
	DWORD				m_nInPackets;
	UDPBandwidthMeter	m_mOutput;
	DWORD				m_nOutBandwidth;
	DWORD				m_nOutFrags;
	DWORD				m_nOutPackets;

// Operations
public:
	BOOL	Listen();
	void	Disconnect();
	inline BOOL IsStable() const	// Avoid using this function directly, use !Network.IsFirewalled(CHECK_UDP) instead
	{
		return ( m_hSocket != INVALID_SOCKET ) && m_bStable;
	}
	inline void SetStable(BOOL bStable = TRUE)
	{
		m_bStable = bStable;
	}
	BOOL	Send(IN_ADDR* pAddress, WORD nPort, CPacket* pPacket, BOOL bRelease = TRUE, LPVOID pToken = NULL, BOOL bAck = TRUE);
	BOOL	Send(SOCKADDR_IN* pHost, const CBuffer& pOutput);
	BOOL	Send(SOCKADDR_IN* pHost, CPacket* pPacket, BOOL bRelease = TRUE, LPVOID pToken = NULL, BOOL bAck = TRUE);
	void	PurgeToken(LPVOID pToken);
	void	OnRun();
protected:
	void	Measure();
	BOOL	TryWrite();
	void	ManageOutput();
	void	Remove(CDatagramOut* pDG);
protected:
	BOOL	TryRead();
	BOOL	OnDatagram(SOCKADDR_IN* pHost, BYTE* pBuffer, DWORD nLength);
	BOOL	OnReceiveSGP(SOCKADDR_IN* pHost, SGP_HEADER* pHeader, DWORD nLength);
	BOOL	OnAcknowledgeSGP(SOCKADDR_IN* pHost, SGP_HEADER* pHeader, DWORD nLength);
	void	ManagePartials();
	void	Rerequest(CDatagramIn* pDG);
	void	Remove(CDatagramIn* pDG, BOOL bReclaimOnly = FALSE);
protected:
	BOOL	OnPacket(SOCKADDR_IN* pHost, CG1Packet* pPacket);
	BOOL	OnPacket(SOCKADDR_IN* pHost, CG2Packet* pPacket);
	BOOL	OnPing(SOCKADDR_IN* pHost, CG1Packet* pPacket);
	BOOL	OnPing(SOCKADDR_IN* pHost, CG2Packet* pPacket);
	BOOL	OnPong(SOCKADDR_IN* pHost, CG1Packet* pPacket);
	BOOL	OnPong(SOCKADDR_IN* pHost, CG2Packet* pPacket);
	BOOL	OnQuery(SOCKADDR_IN* pHost, CG2Packet* pPacket);
	BOOL	OnQueryAck(SOCKADDR_IN* pHost, CG2Packet* pPacket);
	BOOL	OnCommonHit(SOCKADDR_IN* pHost, CG2Packet* pPacket);
	BOOL	OnQueryKeyRequest(SOCKADDR_IN* pHost, CG2Packet* pPacket);
	BOOL	OnQueryKeyAnswer(SOCKADDR_IN* pHost, CG2Packet* pPacket);
	BOOL	OnPush(SOCKADDR_IN* pHost, CG2Packet* pPacket);
	BOOL	OnCrawlRequest(SOCKADDR_IN* pHost, CG2Packet* pPacket);
	BOOL	OnCrawlAnswer(SOCKADDR_IN* pHost, CG2Packet* pPacket);
	BOOL	OnDiscovery(SOCKADDR_IN* pHost, CG2Packet* pPacket);
	BOOL	OnKHL(SOCKADDR_IN* pHost, CG2Packet* pPacket);
	BOOL	OnKHLA(SOCKADDR_IN* pHost, CG2Packet* pPacket);
	BOOL	OnKHLR(SOCKADDR_IN* pHost, CG2Packet* pPacket);

};

extern CDatagrams Datagrams;
