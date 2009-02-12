//
// DownloadTransferED2K.h
//
// Copyright © Shareaza Development Team, 2002-2009.
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

#if !defined(AFX_DOWNLOADTRANSFERED2K_H__C03CF64E_8944_4C5C_BB14_C61C3F37E19E__INCLUDED_)
#define AFX_DOWNLOADTRANSFERED2K_H__C03CF64E_8944_4C5C_BB14_C61C3F37E19E__INCLUDED_

#pragma once

#include "DownloadTransfer.h"

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
	BOOL			m_bHashset;
	DWORD			m_tRequest;				//When request was last sent
	DWORD			m_tSources;				//When source request was last sent
	DWORD			m_tRanking;				//When queue ranking was last received
	BYTE*			m_pAvailable;
	Fragments::Queue m_oRequested;
	BOOL			m_bUDP;
protected:
	LPVOID			m_pInflatePtr;
	CBuffer*		m_pInflateBuffer;
	QWORD			m_nInflateOffset;
	QWORD			m_nInflateLength;
	QWORD			m_nInflateRead;
	QWORD			m_nInflateWritten;

// Operations
public:
	virtual BOOL	Initiate();
	virtual void	Close(TRISTATE bKeepSource, DWORD nRetryAfter = 0);
	virtual void	Boost();
	virtual DWORD	GetMeasuredSpeed();
	virtual BOOL	SubtractRequested(Fragments::List& ppFragments);
	virtual BOOL	OnRun();
	virtual BOOL	OnConnected();
	virtual void	OnDropped();
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
protected:
	void	Send(CEDPacket* pPacket, BOOL bRelease = TRUE);
	BOOL	SendPrimaryRequest();
	BOOL	SendSecondaryRequest();
	BOOL	SendFragmentRequests();
	void	ClearRequests();
	BOOL	SelectFragment(const Fragments::List& oPossible, QWORD& nOffset, QWORD& nLength);
	BOOL	RunQueued(DWORD tNow);

public:		// 64bit Large file support
	BOOL	OnSendingPart64(CEDPacket* pPacket);
	BOOL	OnCompressedPart64(CEDPacket* pPacket);
};

#endif // !defined(AFX_DOWNLOADTRANSFERED2K_H__C03CF64E_8944_4C5C_BB14_C61C3F37E19E__INCLUDED_)
