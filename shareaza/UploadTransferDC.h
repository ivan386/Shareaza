//
// UploadTransferDC.h 
//
// Copyright (c) Shareaza Development Team, 2010-2012.
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

class CDCClient;
class CLibraryFolder;
class CUploadTransfer;


class CUploadTransferDC : public CUploadTransfer
{
public:
	CUploadTransferDC(CDCClient* pClient);
	virtual ~CUploadTransferDC();

	CDCClient*		m_pClient;			// Upload owner

	virtual void	Close(UINT nError = 0);
	virtual DWORD	GetMeasuredSpeed();
	virtual void	OnDropped();
	virtual BOOL	OnRun();
	virtual BOOL	OnWrite();

	// Got $ADCGET command
	BOOL			OnUpload(const std::string& strType, const std::string& strFilename, QWORD nOffset, QWORD nLength, const std::string& strOptions);

	// Check if transfer idle
	BOOL			IsIdle() const;

protected:
	DWORD			m_tRankingCheck;	// The time the queue position was last checked
	BOOL			m_bGet;				// Client uses $Get
	CBuffer			m_pXML;				// Cached library file list

	// Check the client's Q rank. Start upload or send notification if required
	BOOL			CheckRanking();
	void			Cleanup(BOOL bDequeue = TRUE);
	BOOL			RequestFileList(BOOL bFile, BOOL bZip, const std::string& strFilename, QWORD nOffset, QWORD nLength);
	BOOL			RequestTigerTree(CLibraryFile* pFile, QWORD nOffset, QWORD nLength);
	BOOL			RequestFile(CLibraryFile* pFile, QWORD nOffset, QWORD nLength);
	BOOL			SendFile();
};
