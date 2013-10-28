//
// Datagrams.h
//
// Copyright (c) Shareaza Development Team, 2002-2011.
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

#pragma pack(push,1)

typedef struct
{
	CHAR	szTag[3];
	BYTE	nFlags;
	WORD	nSequence;
	BYTE	nPart;
	BYTE	nCount;
} SGP_HEADER;

#pragma pack(pop)

#define SGP_TAG_1		"SGP"
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
class CDatagramIn;
class CDatagramOut;
class CPacket;


class CDatagrams
{
public:
	CDatagrams();
	~CDatagrams();

	DWORD				m_nInBandwidth;
	DWORD				m_nInFrags;
	DWORD				m_nInPackets;

	DWORD				m_nOutBandwidth;
	DWORD				m_nOutFrags;
	DWORD				m_nOutPackets;

	BOOL	Listen();
	void	Disconnect();

	// True if the socket is valid, false if its closed
	inline BOOL IsValid() const
	{
		return ( m_hSocket != INVALID_SOCKET );
	}

	// Avoid using this function directly, use !Network.IsFirewalled(CHECK_UDP) instead
	inline BOOL IsStable() const
	{
		return IsValid() && m_bStable;
	}

	inline void SetStable(BOOL bStable = TRUE)
	{
		m_bStable = bStable;
	}

	BOOL	Send(const IN_ADDR* pAddress, WORD nPort, CPacket* pPacket, BOOL bRelease = TRUE, LPVOID pToken = NULL, BOOL bAck = TRUE);
	BOOL	Send(const SOCKADDR_IN* pHost, CPacket* pPacket, BOOL bRelease = TRUE, LPVOID pToken = NULL, BOOL bAck = TRUE);
	void	PurgeToken(LPVOID pToken);
	void	OnRun();

protected:
	SOCKET			m_hSocket;
	WORD			m_nSequence;
	BOOL			m_bStable;
	DWORD			m_tLastWrite;

	CBuffer*		m_pBufferBuffer;	// Output buffers
	DWORD			m_nBufferBuffer;	// Number of output buffers (Settings.Gnutella2.UdpBuffers)
	CBuffer*		m_pBufferFree;		// List of free output buffers
	DWORD			m_nBufferFree;		// Number of items in list of free output buffers

	CDatagramIn*	m_pInputBuffer;
	DWORD			m_nInputBuffer;
	CDatagramIn*	m_pInputFree;
	CDatagramIn*	m_pInputFirst;
	CDatagramIn*	m_pInputLast;
	CDatagramIn*	m_pInputHash[32];

	CDatagramOut*	m_pOutputBuffer;
	DWORD			m_nOutputBuffer;
	CDatagramOut*	m_pOutputFree;
	CDatagramOut*	m_pOutputFirst;
	CDatagramOut*	m_pOutputLast;
	CDatagramOut*	m_pOutputHash[32];

	UDPBandwidthMeter	m_mInput;
	UDPBandwidthMeter	m_mOutput;

	// Buffer for current incoming UDP packet. It's global since CDatagrams
	// process one packet at once only. Maximal UDP size 64KB.
	BYTE			m_pReadBuffer[ 65536 ];

	void	Measure();
	BOOL	TryWrite();
	void	ManageOutput();
	void	Remove(CDatagramOut* pDG);

	BOOL	TryRead();
	BOOL	OnDatagram(const SOCKADDR_IN* pHost, const BYTE* pBuffer, DWORD nLength);
	BOOL	OnReceiveSGP(const SOCKADDR_IN* pHost, const SGP_HEADER* pHeader, DWORD nLength);
	BOOL	OnAcknowledgeSGP(const SOCKADDR_IN* pHost, const SGP_HEADER* pHeader, DWORD nLength);
	void	ManagePartials();
	void	Remove(CDatagramIn* pDG, BOOL bReclaimOnly = FALSE);
};

extern CDatagrams Datagrams;
