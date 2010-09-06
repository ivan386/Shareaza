//
// UploadTransferDC.h 
//
// Copyright (c) Shareaza Development Team, 2010.
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

	virtual void	Close(UINT nError = 0);
	virtual DWORD	GetMeasuredSpeed();
	virtual BOOL	OnRun();
	virtual BOOL	OnWrite();

	// Got $ADCGET command
	BOOL			OnUpload(const std::string& strType, const std::string& strFilename, QWORD nOffset, QWORD nLength, const std::string& strOptions);

protected:
	CDCClient*		m_pClient;		// Upload owner

	BOOL			RequestFileList(BOOL bFile, BOOL bZip, const std::string& strFilename, QWORD nOffset, QWORD nLength);
	BOOL			RequestTigerTree(const std::string& strFilename, QWORD nOffset, QWORD nLength, CLibraryFile* pFile);
	BOOL			RequestFile(const std::string& strFilename, QWORD nOffset, QWORD nLength, CLibraryFile* pFile);
	BOOL			SendFile(const std::string& strFilename);
	void			LibraryToFileList(const CString& strRoot, CBuffer& pXML);
	void			FolderToFileList(CLibraryFolder* pFolder, CBuffer& pXML);
	void			FileToFileList(CLibraryFile* pFile, CBuffer& pXML);
};
