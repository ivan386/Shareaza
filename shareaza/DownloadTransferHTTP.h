//
// DownloadTransferHTTP.h
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

#if !defined(AFX_DOWNLOADTRANSFERHTTP_H__EE18C980_54B9_40EF_A55B_42FC2AAEA3B0__INCLUDED_)
#define AFX_DOWNLOADTRANSFERHTTP_H__EE18C980_54B9_40EF_A55B_42FC2AAEA3B0__INCLUDED_

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
	DWORD			m_tRequest;
	DWORD			m_tContent;
	BOOL			m_bBadResponse;
	BOOL			m_bBusyFault;
	BOOL			m_bRangeFault;
	BOOL			m_bKeepAlive;
	BOOL			m_bHashMatch;
	CString			m_sTigerTree;
	BOOL			m_bTigerFetch;
	BOOL			m_bTigerIgnore;
	CString			m_sMetadata;
	BOOL			m_bMetaFetch;
	BOOL			m_bMetaIgnore;
	BOOL			m_bGotRange;
	BOOL			m_bGotRanges;
	BOOL			m_bQueueFlag;
	QWORD			m_nContentLength;
	CString			m_sContentType;
	DWORD			m_nRetryDelay;

// Operations
public:
	virtual BOOL	Initiate();
	BOOL			AcceptPush(CConnection* pConnection);
	virtual void	Close(TRISTATE bKeepSource);
	virtual void	Boost();
	virtual DWORD	GetAverageSpeed();
	virtual BOOL	SubtractRequested(CFileFragment** ppFragments);
	virtual BOOL	OnRun();
protected:
	BOOL			StartNextFragment();
	BOOL			SendRequest();
	BOOL			ReadResponseLine();
	BOOL			ReadContent();
	BOOL			ReadTiger();
	BOOL			ReadMetadata();
	BOOL			ReadFlush();
protected:
	virtual BOOL	OnConnected();
	virtual BOOL	OnRead();
	virtual void	OnDropped(BOOL bError);
	virtual BOOL	OnHeaderLine(CString& strHeader, CString& strValue);
	virtual BOOL	OnHeadersComplete();

};

#endif // !defined(AFX_DOWNLOADTRANSFERHTTP_H__EE18C980_54B9_40EF_A55B_42FC2AAEA3B0__INCLUDED_)
