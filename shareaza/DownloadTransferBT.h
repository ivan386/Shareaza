//
// DownloadTransferBT.h
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

class CBTClient;
class CBTPacket;


class CDownloadTransferBT : public CDownloadTransfer
{
// Construction
public:
	CDownloadTransferBT(CDownloadSource* pSource, CBTClient* pClient);
	virtual ~CDownloadTransferBT();

// Attributes
public:
	CBTClient*		m_pClient;
	BOOL			m_bChoked;
	BOOL			m_bInterested;
public:
	BOOL			m_bAvailable;
	DWORD			m_tRunThrottle;

// Operations
public:
	BOOL	OnBitfield(CBTPacket* pPacket);
	BOOL	OnHave(CBTPacket* pPacket);
	BOOL	OnChoked(CBTPacket* pPacket);
	BOOL	OnUnchoked(CBTPacket* pPacket);
	BOOL	OnPiece(CBTPacket* pPacket);
	BOOL	OnSourceResponse(CBTPacket* pPacket);
	void	SendFinishedBlock(DWORD nBlock);
protected:
	void	ShowInterest();

// Overides
public:
	virtual BOOL	Initiate();
	virtual void	Close(TRISTATE bKeepSource, DWORD nRetryAfter = 0);
	virtual void	Boost();
	virtual DWORD	GetMeasuredSpeed();
	virtual CString	GetStateText(BOOL bLong);
	virtual BOOL	OnRun();
	virtual BOOL	OnConnected();
	virtual BOOL	SubtractRequested(Fragments::List& ppFragments) const;
	virtual bool	UnrequestRange(QWORD nOffset, QWORD nLength);
protected:
	virtual bool	SendFragmentRequests();
};
