//
// DownloadTransferHTTP.h
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


class CDownloadTransferHTTP : public CDownloadTransfer
{
// Construction
public:
	CDownloadTransferHTTP(CDownloadSource* pSource);
	virtual ~CDownloadTransferHTTP();

// Attributes
protected:
	DWORD			m_nRequests;
	DWORD			m_tContent;
	BOOL			m_bBadResponse;
	BOOL			m_bBusyFault;
	BOOL			m_bRangeFault;
	BOOL			m_bKeepAlive;
	CString			m_sTigerTree;		// X-TigerTree-Path
	BOOL			m_bTigerFetch;
	BOOL			m_bTigerIgnore;
	CString			m_sMetadata;		// X-Metadata-Path
	BOOL			m_bMetaFetch;
	BOOL			m_bGotRange;
	BOOL			m_bGotRanges;
	BOOL			m_bQueueFlag;
	QWORD			m_nContentLength;
	CString			m_sContentType;
	DWORD			m_nRetryDelay;      // Delay for queuing
	DWORD			m_nRetryAfter;      // Got "Retry-After: x" seconds
	BOOL			m_bRedirect;
	CString			m_sRedirectionURL;
	BOOL			m_bGzip;			// Got "Content-Encoding: gzip" or x-gzip
	BOOL			m_bCompress;		// Got "Content-Encoding: compress" or x-compress
	BOOL			m_bDeflate;			// Got "Content-Encoding: deflate"
	BOOL			m_bChunked;			// Got "Transfer-Encoding: chunked"
	enum ChunkState
	{
		Header,							// Reading chunk header "Length<CR><LF>"
		Body,							// Reading chunk body
		BodyEnd,						// Waiting for chunk body ending "<CR><LF>"
		Footer							// Bypassing data after last chunk
	};
	ChunkState		m_ChunkState;
	QWORD			m_nChunkLength;

// Operations
public:
	virtual BOOL	Initiate();
	virtual void	AttachTo(CConnection* pConnection);
	virtual void	Close(TRISTATE bKeepSource, DWORD nRetryAfter = 0);
	virtual void	Boost();
	virtual DWORD	GetAverageSpeed();
	virtual BOOL	SubtractRequested(Fragments::List& ppFragments) const;
	virtual BOOL	OnRun();

protected:
	BOOL			StartNextFragment();
	BOOL			SendRequest();
	BOOL			ReadResponseLine();
	BOOL			ReadContent();
	BOOL			ReadTiger(bool bDropped = false);
	BOOL			ReadMetadata();
	BOOL			ReadFlush();

	virtual BOOL	OnConnected();
	virtual BOOL	OnRead();
	virtual void	OnDropped();
	virtual BOOL	OnHeaderLine(CString& strHeader, CString& strValue);
	virtual BOOL	OnHeadersComplete();
};
