//
// CDownloadTransferFTP.h
//
// Copyright (c) Nikolay Raspopov, 2002-2004.
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

#include "DownloadTransfer.h"
#include "DownloadTransferFTPLIST.h"
#include "DownloadTransferFTPDATA.h"

class CDownloadTransferFTP : public CDownloadTransfer
{
// Construction
public:
	CDownloadTransferFTP(CDownloadSource* pSource);
	virtual ~CDownloadTransferFTP ();

// Attributes
protected:
	enum FTP_STATES {
		ftpConnecting,		// 220
		ftpUSER,			// 331 USER
		ftpPASS,			// 230 PASS

		ftpTYPE1,			// 200 TYPE A
		ftpPASV1,			// 227 PASV
		ftpLIST1,			// 125 LIST path 226

		ftpTYPE2,			// 200 TYPE I
		ftpPASV2,			// 227 PASV
		ftpREST2,			// 350 REST offset
		ftpRETR2,			// 125 RETR path 226

		ftpCompleted
	};
	FTP_STATES		m_FtpState;
	DWORD			m_nRequests;
	DWORD			m_tRequest;
	DWORD			m_tContent;
	BOOL			m_bBusyFault;
	BOOL			m_bQueueFlag;
	CString			m_sContentType;
	DWORD			m_nRetryDelay;

	CDownloadTransferFTPLIST*	m_pListTransfer;
	BOOL						m_bListTransferValid;
	CString						m_sListData;
	CDownloadTransferFTPDATA*	m_pDataTransfer;
	BOOL						m_bDataTransferValid;

// Operations
public:
	virtual BOOL	Initiate();
	virtual void	Close(TRISTATE bKeepSource);
	virtual void	Boost();
	virtual DWORD	GetAverageSpeed();
	virtual BOOL	SubtractRequested(CFileFragmentList& Fragments);
	virtual BOOL	OnRun();
	virtual BOOL	OnRead();

protected:
	BOOL			StartNextFragment();
	BOOL			SendRequest();

protected:
	virtual BOOL	OnConnected();
	virtual void	OnDropped(BOOL bError);
	virtual BOOL	OnHeaderLine(CString& strHeader, CString& strValue);
};
