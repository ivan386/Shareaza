//
// Neighbour.h
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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

#if !defined(AFX_NEIGHBOUR_H__7B1C7637_2718_4D5F_B39D_28894CC0669D__INCLUDED_)
#define AFX_NEIGHBOUR_H__7B1C7637_2718_4D5F_B39D_28894CC0669D__INCLUDED_

#pragma once

#include "Connection.h"

class CBuffer;
class CPacket;
class CVendor;
class CGProfile;
class CQuerySearch;
class CQueryHashTable;

typedef enum NeighbourStateEnum
{
	nrsNull, nrsConnecting,
	nrsHandshake1, nrsHandshake2, nrsHandshake3, nrsRejected,
	nrsClosing, nrsConnected
} NrsState;

typedef enum NeighbourNodeEnum
{
	ntNode, ntHub, ntLeaf
} NrsNode;

#define PONG_NEEDED_BUFFER	32


class CNeighbour : public CConnection
{
// Construction
public:
	CNeighbour(PROTOCOLID nProtocol);
	CNeighbour(PROTOCOLID nProtocol, CNeighbour* pBase);
	virtual ~CNeighbour();
	
// Attributes : State
public:
	DWORD		m_nRunCookie;
	DWORD		m_zStart;
	DWORD		m_nUnique;
	PROTOCOLID	m_nProtocol;
	NrsState	m_nState;
	CVendor*	m_pVendor;
	BOOL		m_bGUID;
	GGUID		m_pGUID;
	CGProfile*	m_pProfile;
	GGUID*		m_pMoreResultsGUID;		//Last search GUID- used to get more results
// Attributes : Capabilities
public:
	BOOL		m_bAutomatic;
	BOOL		m_bShake06;
	BOOL		m_bShareaza;
	NrsNode		m_nNodeType;
	BOOL		m_bQueryRouting;
	BOOL		m_bPongCaching;
	BOOL		m_bVendorMsg;
	BOOL		m_bGGEP;
	DWORD		m_tLastQuery;
// Attributes: Statistics
public:
	DWORD		m_nInputCount;
	DWORD		m_nOutputCount;
	DWORD		m_nDropCount;
	DWORD		m_nLostCount;
	DWORD		m_nOutbound;
	DWORD		m_nFileCount;
	DWORD		m_nFileVolume;
// Attributes : Query Hash Tables
public:
	CQueryHashTable*	m_pQueryTableRemote;
	CQueryHashTable*	m_pQueryTableLocal;
// Attributes : Internals
protected:
	DWORD		m_tLastPacket;
	CBuffer*	m_pZInput;
	CBuffer*	m_pZOutput;
	DWORD		m_nZInput;
	DWORD		m_nZOutput;
	LPVOID		m_pZSInput;
	LPVOID		m_pZSOutput;
	BOOL		m_bZFlush;
	DWORD		m_tZOutput;
protected:
	DWORD		m_zEnd;
	
// Operations
public:
	virtual BOOL	Send(CPacket* pPacket, BOOL bRelease = TRUE, BOOL bBuffered = FALSE);
	virtual void	Close(UINT nError = IDS_CONNECTION_CLOSED);
	void			DelayClose(UINT nError = 0);
	virtual BOOL	SendQuery(CQuerySearch* pSearch, CPacket* pPacket, BOOL bLocal);
protected:
	virtual BOOL	OnRun();
	virtual void	OnDropped(BOOL bError);
	virtual BOOL	OnRead();
	virtual BOOL	OnWrite();
	virtual BOOL	OnCommonHit(CPacket* pPacket);
	virtual BOOL	OnCommonQueryHash(CPacket* pPacket);
public:
	void	GetCompression(float* pnInRate, float* pnOutRate);
	
};

#endif // !defined(AFX_NEIGHBOUR_H__7B1C7637_2718_4D5F_B39D_28894CC0669D__INCLUDED_)

