//
// G2Neighbour.h
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

#if !defined(AFX_G2NEIGHBOUR_H__F3C423B0_60F0_4721_81A3_1109E59CD425__INCLUDED_)
#define AFX_G2NEIGHBOUR_H__F3C423B0_60F0_4721_81A3_1109E59CD425__INCLUDED_

#pragma once

#include "Neighbour.h"

class CG2Packet;
class CHubHorizonGroup;


class CG2Neighbour : public CNeighbour  
{
// Construction
public:
	CG2Neighbour(CNeighbour* pBase);
	virtual ~CG2Neighbour();

// Attributes
public:
	DWORD				m_nLeafCount;
	DWORD				m_nLeafLimit;
	BOOL				m_bCachedKeys;
	CRouteCache*		m_pGUIDCache;
	CHubHorizonGroup*	m_pHubGroup;
protected:
	LONG				m_tAdjust;
	DWORD				m_tLastPingIn;
	DWORD				m_tLastPingOut;
	DWORD				m_tWaitLNI;
	DWORD				m_tLastKHL;
	DWORD				m_tLastHAW;
protected:
	CPtrList			m_pOutbound;

// Operations
public:
	virtual BOOL	Send(CPacket* pPacket, BOOL bRelease = TRUE, BOOL bBuffered = FALSE);
	virtual BOOL	SendQuery(CQuerySearch* pSearch, CPacket* pPacket, BOOL bLocal);
protected:
	virtual BOOL	OnRead();
	virtual BOOL	OnWrite();
	virtual BOOL	OnRun();
protected:
	void	SendStartups();
	BOOL	ProcessPackets();
	BOOL	OnPacket(CG2Packet* pPacket);
	BOOL	OnPing(CG2Packet* pPacket);
	void	SendLNI();
	BOOL	OnLNI(CG2Packet* pPacket);
	void	SendKHL();
	BOOL	OnKHL(CG2Packet* pPacket);
	void	SendHAW();
	BOOL	OnHAW(CG2Packet* pPacket);
	BOOL	OnQuery(CG2Packet* pPacket);
	BOOL	OnQueryAck(CG2Packet* pPacket);
	BOOL	OnQueryKeyReq(CG2Packet* pPacket);
	BOOL	OnQueryKeyAns(CG2Packet* pPacket);
	BOOL	OnPush(CG2Packet* pPacket);
	BOOL	OnProfileChallenge(CG2Packet* pPacket);
	BOOL	OnProfileDelivery(CG2Packet* pPacket);

};

#endif // !defined(AFX_G2NEIGHBOUR_H__F3C423B0_60F0_4721_81A3_1109E59CD425__INCLUDED_)
