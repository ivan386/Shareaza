//
// UploadTransferED2K.h
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

#if !defined(AFX_UPLOADTRANSFERED2K_H__04FAF448_0A7F_4566_97D2_38845BF71F20__INCLUDED_)
#define AFX_UPLOADTRANSFERED2K_H__04FAF448_0A7F_4566_97D2_38845BF71F20__INCLUDED_

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
	CEDClient*		m_pClient;
	DWORD			m_tRequest;
	int				m_nRanking;
	DWORD			m_tRanking;
private:
    FF::SimpleFragmentQueue m_oRequested;
    FF::SimpleFragmentQueue m_oServed;

// Operations
public:
	BOOL			Request(MD4* pMD4);
	virtual void	Close(BOOL bMessage = FALSE);
	virtual BOOL	OnRun();
	virtual BOOL	OnConnected();
	virtual void	OnDropped(BOOL bError);
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
	BOOL	SendRanking();
	void	AddRequest(QWORD nOffset, QWORD nLength);
	BOOL	ServeRequests();
	BOOL	OpenFile();
	BOOL	StartNextRequest();
	BOOL	DispatchNextChunk();
	BOOL	CheckFinishedRequest();

};

#endif // !defined(AFX_UPLOADTRANSFERED2K_H__04FAF448_0A7F_4566_97D2_38845BF71F20__INCLUDED_)
