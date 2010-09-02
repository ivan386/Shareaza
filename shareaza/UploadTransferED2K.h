//
// UploadTransferED2K.h
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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

#include "UploadTransfer.h"
#include "FileFragments.hpp"

class CEDClient;
class CEDPacket;

class CUploadTransferED2K : public CUploadTransfer
{
// Construction
public:
	CUploadTransferED2K(CEDClient* pClient);
	virtual ~CUploadTransferED2K();
	
// Attributes
public:
	CEDClient*		m_pClient;					// The remote client.
	int				m_nRanking;					// The last queue position the remote client was sent.
	DWORD			m_tRankingSent;				// The time a queue ranking packet was last sent.
	DWORD			m_tRankingCheck;			// The time the queue position was last checked.
	DWORD			m_tLastRun;					// The time the transfer was last run
private:
	Fragments::Queue m_oRequested;
	Fragments::Queue m_oServed;

// Operations
public:
    BOOL			Request(const Hashes::Ed2kHash& oED2K);
	virtual void	Close(UINT nError = 0);
	virtual BOOL	OnRun();
	virtual BOOL	OnConnected();
	virtual void	OnDropped();
	virtual void	OnQueueKick();
	virtual DWORD	GetMeasuredSpeed();
public:
	BOOL	OnRunEx(DWORD tNow);
	BOOL	OnQueueRelease(CEDPacket* pPacket);
	BOOL	OnRequestParts(CEDPacket* pPacket);
	BOOL	OnReask();
protected:
	void	Cleanup(BOOL bDequeue = TRUE);
	void	Send(CEDPacket* pPacket, BOOL bRelease = TRUE);
	BOOL	CheckRanking();		// Check the client's Q rank. Start upload or send notification if required.
	void	AddRequest(QWORD nOffset, QWORD nLength);
	BOOL	ServeRequests();
	BOOL	StartNextRequest();
	BOOL	DispatchNextChunk();
	BOOL	CheckFinishedRequest();

public:		// 64bit Large file support
	BOOL	OnRequestParts64(CEDPacket* pPacket);
};
