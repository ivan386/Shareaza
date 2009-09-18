//
// DownloadTransfer.h
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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

#include "Transfer.h"
#include "FileFragments.hpp"

class CDownload;
class CDownloadSource;


class CDownloadTransfer : public CTransfer
{
public:
	CDownloadTransfer(CDownloadSource* pSource, PROTOCOLID nProtocol);
	virtual ~CDownloadTransfer();

	CDownloadTransfer*	m_pDlPrev;
	CDownloadTransfer*	m_pDlNext;

	int					m_nState;
	DWORD				m_nQueuePos;
	DWORD				m_nQueueLen;
	CString				m_sQueueName;
	DWORD				m_nBandwidth;		// Bandwidth allocated

	QWORD				m_nOffset;			// Fragment offset
	QWORD				m_nLength;			// Fragment length
	QWORD				m_nPosition;		// Fragment position
	QWORD				m_nDownloaded;

	BOOL				m_bWantBackwards;
	BOOL				m_bRecvBackwards;	// Got "Content-Encoding: backwards"

	virtual BOOL	Initiate() = 0;
	virtual void	Close(TRISTATE bKeepSource, DWORD nRetryAfter = 0);
	virtual void	Boost();
	virtual DWORD	GetAverageSpeed();
	virtual DWORD	GetMeasuredSpeed();
	virtual BOOL	SubtractRequested(Fragments::List& ppFragments) = 0;
	virtual BOOL	UnrequestRange(QWORD /*nOffset*/, QWORD /*nLength*/) { return FALSE; }
	virtual CString	GetStateText(BOOL bLong);
	virtual BOOL	OnRun();
	void			SetState(int nState);

	// Get owner download
	CDownload*		 GetDownload() const;

	// Get associated source
	CDownloadSource* GetSource() const;

protected:
	CDownload*					m_pDownload;
	CDownloadSource*			m_pSource;
	CTimeAverage< DWORD, 2000 >	m_AverageSpeed;

	void			ChunkifyRequest(QWORD* pnOffset, QWORD* pnLength, QWORD nChunk, BOOL bVerifyLock);
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
