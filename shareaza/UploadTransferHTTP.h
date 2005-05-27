//
// UploadTransferHTTP.h
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

#if !defined(AFX_UPLOADTRANSFERHTTP_H__FFC5B664_6827_41EC_87E2_033318A36E0A__INCLUDED_)
#define AFX_UPLOADTRANSFERHTTP_H__FFC5B664_6827_41EC_87E2_033318A36E0A__INCLUDED_

#pragma once

#include "UploadTransfer.h"

class CLibraryFile;
class CDownload;
class CTigerTree;
class CXMLElement;
class CED2K;


class CUploadTransferHTTP : public CUploadTransfer
{
// Construction
public:
	CUploadTransferHTTP();
	virtual ~CUploadTransferHTTP();

// Attributes
protected:
	CString		m_sRequest;
	DWORD		m_tRequest;
	BOOL		m_bHead;
	BOOL		m_bConnectHdr;
	BOOL		m_bKeepAlive;
	BOOL		m_bHostBrowse;
	BOOL		m_bDeflate;
	BOOL		m_bBackwards;
	BOOL		m_bRange;
	BOOL		m_bQueueMe;
	BOOL		m_bNotShareaza;
	int			m_nGnutella;
	int			m_nReaskMultiplier; //Last re-ask time multiplier used
protected:
	BOOL		m_bTigerTree;
	BOOL		m_bMetadata;
	CString		m_sLocations;
	CString		m_sRanges;

// Operations
public:
	virtual void	AttachTo(CConnection* pConnection);
protected:
	BOOL	ReadRequest();
	BOOL	RequestSharedFile(CLibraryFile* pFile, CSingleLock& oLibraryLock);
	BOOL	RequestPartialFile(CDownload* pFile);
	BOOL	RequestTigerTreeRaw(CTigerTree* pTigerTree, BOOL bDelete);
	BOOL	RequestTigerTreeDIME(CTigerTree* pTigerTree, int nDepth, CED2K* pHashset, BOOL bDelete);
	BOOL	RequestMetadata(CXMLElement* pMetadata);
	BOOL	RequestPreview(CLibraryFile* pFile, CSingleLock& oLibraryLock);
	BOOL	RequestHostBrowse();
protected:
	BOOL	IsNetworkDisabled();
	BOOL	QueueRequest();
	BOOL	OpenFileSendHeaders();
	void	SendDefaultHeaders();
	void	SendFileHeaders();
	void	OnCompleted();
	void	SendResponse(UINT nResourceID, BOOL bFileHeaders = FALSE);
	void	GetNeighbourList(CString& strOutput);
protected:
	virtual BOOL	OnRun();
	virtual void	OnDropped(BOOL bError);
	virtual BOOL	OnRead();
	virtual BOOL	OnWrite();
	virtual BOOL	OnHeaderLine(CString& strHeader, CString& strValue);
	virtual BOOL	OnHeadersComplete();

public:
	inline BOOL IsBackwards() const { return m_bBackwards; }
};

#endif // !defined(AFX_UPLOADTRANSFERHTTP_H__FFC5B664_6827_41EC_87E2_033318A36E0A__INCLUDED_)
