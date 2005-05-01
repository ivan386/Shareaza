//
// LocalSearch.h
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

#if !defined(AFX_LOCALSEARCH_H__5588AE7C_89B4_4B75_95C5_1519DD6F6D9E__INCLUDED_)
#define AFX_LOCALSEARCH_H__5588AE7C_89B4_4B75_95C5_1519DD6F6D9E__INCLUDED_

#pragma once

class CQuerySearch;
class CNeighbour;
class CLibraryFile;
class CAlbumFolder;
class CLibraryFolder;
class CSchema;
class CXMLElement;
class CBuffer;
class CDownload;
class CPacket;
class CG2Packet;


class CLocalSearch
{
// Construction
public:
	CLocalSearch(CQuerySearch* pSearch, CNeighbour* pNeighbour, BOOL bWrapped = FALSE);
	CLocalSearch(CQuerySearch* pSearch, SOCKADDR_IN* pEndpoint);
	CLocalSearch(CQuerySearch* pSearch, CBuffer* pBuffer, PROTOCOLID nProtocol = PROTOCOL_G1);
	virtual ~CLocalSearch();

// Attributes
protected:
	CQuerySearch*	m_pSearch;
	CNeighbour*		m_pNeighbour;
	SOCKADDR_IN*	m_pEndpoint;
	CBuffer*		m_pBuffer;
	DWORD			m_nTTL;
	GGUID			m_pGUID;
	PROTOCOLID		m_nProtocol;
	BOOL			m_bWrapped;
protected:
	CPacket*		m_pPacket;
	CMapPtrToPtr	m_pSchemas;

// Operations
public:
	int			Execute(int nMaximum = -1);
	void		WriteVirtualTree();
protected:
	int			ExecuteSharedFiles(int nMaximum);
	BOOL		AddHit(CLibraryFile* pFile, int nIndex);
	BOOL		AddHitG1(CLibraryFile* pFile, int nIndex);
	BOOL		AddHitG2(CLibraryFile* pFile, int nIndex);
	int			ExecutePartialFiles(int nMaximum);
	void		AddHit(CDownload* pDownload, int nIndex);
protected:
	void		CreatePacket(int nCount);
	void		CreatePacketG1(int nCount);
	void		CreatePacketG2();
	void		AddMetadata(CSchema* pSchema, CXMLElement* pXML, int nIndex);
	CString		GetXMLString();
	void		WriteTrailer();
	void		WriteTrailerG1();
	void		WriteTrailerG2();
	void		DispatchPacket();
	void		DestroyPacket();
	CG2Packet*	AlbumToPacket(CAlbumFolder* pFolder);
	CG2Packet*	FoldersToPacket();
	CG2Packet*	FolderToPacket(CLibraryFolder* pFolder);

};

#endif // !defined(AFX_LOCALSEARCH_H__5588AE7C_89B4_4B75_95C5_1519DD6F6D9E__INCLUDED_)
