//
// DCClient.h
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

#include "DCStream.h"
#include "UploadTransfer.h"

class CLibraryFolder;
class CLibraryFile;


class CDCClient : public CDCStream< CUploadTransfer >
{
public:
	CDCClient();
	virtual ~CDCClient();

	virtual BOOL	ConnectTo(const IN_ADDR* pAddress, WORD nPort);

protected:
	virtual BOOL	OnConnected();
	virtual void	OnDropped();
	virtual BOOL	OnWrite();

	virtual BOOL	OnCommand(const std::string& strCommand, const std::string& strParams);
	virtual BOOL	OnKey();
	virtual BOOL	OnChat(const std::string& strMessage);

	BOOL			OnGet(const std::string& strType, const std::string& strFilename, QWORD nOffset, QWORD nLength, const std::string& strOptions);

	BOOL			RequestFileList(const std::string& strFilename, QWORD nOffset, QWORD nLength);
	BOOL			RequestTigerTree(const std::string& strFilename, QWORD nOffset, QWORD nLength, CLibraryFile* pFile);
	BOOL			RequestFile(const std::string& strFilename, QWORD nOffset, QWORD nLength, CLibraryFile* pFile);
	BOOL			SendFile(const std::string& strFilename);

	void			LibraryToFileList(CBuffer& pXML);
	void			FolderToFileList(CLibraryFolder* pFolder, CBuffer& pXML);
	void			FileToFileList(CLibraryFile* pFile, CBuffer& pXML);
};
