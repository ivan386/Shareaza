//
// G1Neighbour.h
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

#if !defined(AFX_G1NEIGHBOUR_H__BF099C28_0FD5_4A9A_B36E_6490DA6FB62F__INCLUDED_)
#define AFX_G1NEIGHBOUR_H__BF099C28_0FD5_4A9A_B36E_6490DA6FB62F__INCLUDED_

#pragma once

#include "Neighbour.h"

class CG1Packet;
class CG1PacketBuffer;
class CPongItem;


class CG1Neighbour : public CNeighbour  
{
// Construction
public:
	CG1Neighbour(CNeighbour* pBase);
	virtual ~CG1Neighbour();
	
// Attributes
protected:
	DWORD		m_tLastInPing;
	DWORD		m_tLastOutPing;
	DWORD		m_tClusterHost;
	DWORD		m_tClusterSent;
protected:
	BYTE		m_nPongNeeded[PONG_NEEDED_BUFFER];
	GGUID		m_pLastPingID;
	BYTE		m_nLastPingHops;
	BYTE		m_nHopsFlow;
protected:
	CG1PacketBuffer*	m_pOutbound;

// Operations
public:
	virtual BOOL	Send(CPacket* pPacket, BOOL bRelease = TRUE, BOOL bBuffered = FALSE);
	BOOL			SendPing(DWORD dwNow = 0, GGUID* pGUID = NULL);
	void			OnNewPong(CPongItem* pPong);
	virtual BOOL	SendQuery(CQuerySearch* pSearch, CPacket* pPacket, BOOL bLocal);
	void			SendG2Push(GGUID* pGUID, CPacket* pPacket);
protected:
	virtual BOOL	OnRead();
	virtual BOOL	OnWrite();
	virtual BOOL	OnRun();
protected:
	BOOL	ProcessPackets();
	BOOL	OnPacket(CG1Packet* pPacket);
	BOOL	OnPing(CG1Packet* pPacket);
	BOOL	OnPong(CG1Packet* pPacket);
	BOOL	OnBye(CG1Packet* pPacket);
	BOOL	OnVendor(CG1Packet* pPacket);
	BOOL	OnPush(CG1Packet* pPacket);
	BOOL	OnQuery(CG1Packet* pPacket);
	BOOL	OnHit(CG1Packet* pPacket);
	void	SendClusterAdvisor();
	BOOL	OnClusterAdvisor(CG1Packet* pPacket);

};

#endif // !defined(AFX_G1NEIGHBOUR_H__BF099C28_0FD5_4A9A_B36E_6490DA6FB62F__INCLUDED_)
