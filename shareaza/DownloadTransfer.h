//
// DownloadTransfer.h
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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

#if !defined(AFX_DOWNLOADTRANSFER_H__DB393F56_C75B_424C_85E5_FF44300E255B__INCLUDED_)
#define AFX_DOWNLOADTRANSFER_H__DB393F56_C75B_424C_85E5_FF44300E255B__INCLUDED_

#pragma once

#include "Transfer.h"
#include "FileFragments.hpp"

class CDownload;
class CDownloadSource;

class CDownloadTransfer : public CTransfer
{
// Construction
public:
	CDownloadTransfer(CDownloadSource* pSource, PROTOCOLID nProtocol);
	virtual ~CDownloadTransfer();

// Attributes
public:
	PROTOCOLID			m_nProtocol;	// Protocol of this transfer
	CDownload*			m_pDownload;
	CDownloadTransfer*	m_pDlPrev;
	CDownloadTransfer*	m_pDlNext;
	CDownloadSource*	m_pSource;
public:
	int			m_nState;
	int			m_nQueuePos;
	int			m_nQueueLen;
	CString		m_sQueueName;
	DWORD		m_nBandwidth;			// Bandwidth allocated
public:
	QWORD		m_nOffset;
	QWORD		m_nLength;
	QWORD		m_nPosition;
	QWORD		m_nDownloaded;
public:
	BOOL		m_bWantBackwards;
	BOOL		m_bRecvBackwards;		// Got "Content-Encoding: backwards"

// Operations
public:
	virtual BOOL	Initiate() = 0;
	virtual void	Close(TRISTATE bKeepSource, DWORD nRetryAfter = 0);
	virtual void	Boost();
	virtual DWORD	GetAverageSpeed();
	virtual DWORD	GetMeasuredSpeed();
	virtual BOOL	SubtractRequested(Fragments::List& ppFragments) = 0;
	virtual BOOL	UnrequestRange(QWORD /*nOffset*/, QWORD /*nLength*/) { return FALSE; }
	virtual CString	GetStateText(BOOL bLong);
	virtual BOOL	OnRun();
	void	SetState(int nState);
protected:
	void	ChunkifyRequest(QWORD* pnOffset, QWORD* pnLength, QWORD nChunk, BOOL bVerifyLock);

};

enum
{
	dtsNull, dtsConnecting, dtsRequesting, dtsHeaders, dtsDownloading,
	dtsFlushing, dtsTiger, dtsHashset, dtsMetadata, dtsBusy, dtsEnqueue, dtsQueued,
	dtsTorrent,

	dtsCountAll = -1,
	dtsCountNotQueued = -2,
	dtsCountNotConnecting = -3,
	dtsCountTorrentAndActive = -4
};


#endif // !defined(AFX_DOWNLOADTRANSFER_H__DB393F56_C75B_424C_85E5_FF44300E255B__INCLUDED_)
