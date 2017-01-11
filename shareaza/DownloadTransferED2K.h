//
// DownloadTransferED2K.h
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

#include "DownloadTransfer.h"
#include "ZLibWarp.h"

class CEDClient;
class CEDPacket;

class CDownloadTransferED2K : public CDownloadTransfer  
{
// Construction
public:
	CDownloadTransferED2K(CDownloadSource* pSource);
	virtual ~CDownloadTransferED2K();
	
// Attributes
public:
	CEDClient*		m_pClient;
	bool			m_bHashset;
	DWORD			m_tRanking;				//When queue ranking was last received
	bool			m_bUDP;
protected:
	z_streamp		m_pInflatePtr;
	CBuffer*		m_pInflateBuffer;
	QWORD			m_nInflateOffset;
	QWORD			m_nInflateLength;
	QWORD			m_nInflateRead;
	QWORD			m_nInflateWritten;

// Operations
public:
	BOOL	OnRunEx(DWORD tNow);
	BOOL	OnFileReqAnswer(CEDPacket* pPacket);
	BOOL	OnFileNotFound(CEDPacket* pPacket);
	BOOL	OnFileStatus(CEDPacket* pPacket);
	BOOL	OnHashsetAnswer(CEDPacket* pPacket);
	BOOL	OnQueueRank(CEDPacket* pPacket);
	BOOL	OnRankingInfo(CEDPacket* pPacket);
	BOOL	OnFileComment(CEDPacket* pPacket);
	BOOL	OnStartUpload(CEDPacket* pPacket);
	BOOL	OnFinishUpload(CEDPacket* pPacket);
	BOOL	OnSendingPart(CEDPacket* pPacket);
	BOOL	OnCompressedPart(CEDPacket* pPacket);
	void	SetQueueRank(int nRank);
	BOOL	OnSendingPart64(CEDPacket* pPacket);
	BOOL	OnCompressedPart64(CEDPacket* pPacket);
protected:
	void	Send(CEDPacket* pPacket, BOOL bRelease = TRUE);
	BOOL	SendPrimaryRequest();
	BOOL	SendSecondaryRequest();
	void	ClearRequests();
	BOOL	RunQueued(DWORD tNow);

// Overides
public:
	virtual BOOL	Initiate();
	virtual void	Close(TRISTATE bKeepSource, DWORD nRetryAfter = 0);
	virtual void	Boost();
	virtual DWORD	GetMeasuredSpeed();
	virtual BOOL	SubtractRequested(Fragments::List& ppFragments) const;
	virtual BOOL	OnRun();
	virtual BOOL	OnConnected();
	virtual void	OnDropped();
protected:
	virtual bool	SendFragmentRequests();
};
