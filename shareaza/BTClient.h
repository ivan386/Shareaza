//
// BTClient.h
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

#pragma once

#include "Transfer.h"

class CBTPacket;
class CDownload;
class CDownloadTransferBT;
class CUploadTransferBT;


class CBTClient : public CTransfer  
{
// Construction
public:
	CBTClient();
	virtual ~CBTClient();

// Attributes
public:
	SHA1					m_pGUID;
	BOOL					m_bShareaza;
	BOOL					m_bExchange;
public:
	CUploadTransferBT*		m_pUpload;
	CDownload*				m_pDownload;
	CDownloadTransferBT*	m_pDownloadTransfer;
protected:
	BOOL					m_bShake;
	BOOL					m_bOnline;
	BOOL					m_bClosing;
	
// Operations
public:
	virtual BOOL	Connect(CDownloadTransferBT* pDownloadTransfer);
	virtual void	AttachTo(CConnection* pConnection);
	virtual void	Close();
public:
	void			Send(CBTPacket* pPacket, BOOL bRelease = TRUE);
protected:
	virtual BOOL	OnRun();
	virtual BOOL	OnConnected();
	virtual void	OnDropped(BOOL bError);
	virtual BOOL	OnWrite();
	virtual BOOL	OnRead();
protected:
	void			SendHandshake(BOOL bPart1, BOOL bPart2);
	BOOL			OnHandshake1();
	BOOL			OnHandshake2();
	BOOL			OnOnline();
	BOOL			OnPacket(CBTPacket* pPacket);
	void			SendBeHandshake();
	BOOL			OnBeHandshake(CBTPacket* pPacket);
	BOOL			OnSourceRequest(CBTPacket* pPacket);
	void			DetermineUserAgent();

public:
	inline BOOL IsOnline() const { return m_bOnline; }

};
