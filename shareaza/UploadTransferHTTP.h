//
// UploadTransferHTTP.h
//
// Copyright (c) Shareaza Development Team, 2002-2017.
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

#include "UploadTransfer.h"

class CLibraryFile;
class CDownload;
class CTigerTree;
class CXMLElement;
class CED2K;


class CUploadTransferHTTP : public CUploadTransfer
{
public:
	CUploadTransferHTTP();
	virtual ~CUploadTransferHTTP();

	inline BOOL		IsBackwards() const { return m_bBackwards; }

	virtual void	AttachTo(CConnection* pConnection);

protected:
	CString		m_sRequest;
	BOOL		m_bHead;
	BOOL		m_bConnectHdr;
	BOOL		m_bKeepAlive;
	// "Accept" header:
	// 0 - unknown;
	// 1 - Gnutella 1 (application/x-gnutella-packets);
	// 2 - Gnutella 2 (application/x-gnutella2 or application/x-shareaza).
	int			m_nAccept;
	BOOL		m_bDeflate;
	BOOL		m_bBackwards;
	BOOL		m_bRange;
	BOOL		m_bQueueMe;
	BOOL		m_bNotShareaza;
	// Gnutella functionality:
	// 0 - Pure HTTP
	// 1 - Pure G1
	// 2 - Pure G2
	// 3 - Mixed G1/G2
	int			m_nGnutella;
	int			m_nReaskMultiplier; //Last re-ask time multiplier used
	BOOL		m_bTigerTree;		// Is TigerTree hashset present?
	BOOL		m_bHashset;			// Is eDonkey2000 hashset present?
	BOOL		m_bMetadata;
	CString		m_sLocations;
	CString		m_sRanges;

	BOOL	ReadRequest();
	BOOL	RequestSharedFile(CLibraryFile* pFile, CSingleLock& oLibraryLock);
	BOOL	RequestPartialFile(CDownload* pFile);
	BOOL	RequestTigerTreeRaw(const CTigerTree* pTigerTree, BOOL bDelete);
	BOOL	RequestTigerTreeDIME(const CTigerTree* pTigerTree, int nDepth, const CED2K* pHashset, BOOL bDelete);
	BOOL	RequestMetadata(CXMLElement* pMetadata);
	BOOL	RequestPreview(CLibraryFile* pFile, CSingleLock& oLibraryLock);
	BOOL	RequestHostBrowse();

	BOOL	IsNetworkDisabled();
	BOOL	QueueRequest();
	BOOL	OpenFileSendHeaders();
	void	SendDefaultHeaders();
	void	SendFileHeaders();
	void	OnCompleted();
	void	SendResponse(UINT nResourceID, BOOL bFileHeaders = FALSE);

	virtual BOOL	OnRun();
	virtual void	OnDropped();
	virtual BOOL	OnRead();
	virtual BOOL	OnWrite();
	virtual BOOL	OnHeaderLine(CString& strHeader, CString& strValue);
	virtual BOOL	OnHeadersComplete();
};
