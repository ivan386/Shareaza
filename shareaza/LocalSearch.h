//
// LocalSearch.h
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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

#include "QuerySearch.h"

class CNeighbour;
class CLibraryFile;
class CAlbumFolder;
class CLibraryFolder;
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
	~CLocalSearch();

// Attributes
protected:
	CQuerySearchPtr	m_pSearch;
	CNeighbour*		m_pNeighbour;
	SOCKADDR_IN*	m_pEndpoint;
	CBuffer*		m_pBuffer;
	Hashes::Guid	m_oGUID;
	PROTOCOLID		m_nProtocol;
	BOOL			m_bWrapped;
protected:
	CPacket*		m_pPacket;
	CMap< CSchemaPtr, CSchemaPtr, CXMLElement*, CXMLElement* > m_pSchemas;

// Operations
public:
	// Search library and downloads (-1 - use default limit, 0 - no limit)
	bool		Execute(int nMaximum = -1, bool bPartial = true, bool bShared = true);
	void		WriteVirtualTree();
	const CQuerySearch* GetSearch() const{ return m_pSearch; }

protected:
	bool		ExecuteSharedFiles(int nMaximum, int& nHits);
	bool		ExecutePartialFiles(int nMaximum, int& nHits);
	template< typename T > void SendHits(const CList< const T * >& oFiles);
	template< typename T > void AddHit(const T * pHit, int nIndex);
	void		AddHitG1(CLibraryFile const * const pFile, int nIndex);
	void		AddHitG2(CLibraryFile const * const pFile, int nIndex);
	template< typename T > bool IsValidForHit(const T * pHit) const;
	inline bool	IsValidForHitG1(CLibraryFile const * const pFile) const;
	inline bool	IsValidForHitG2(CLibraryFile const * const pFile) const;
protected:
	void		CreatePacket();
	void		CreatePacketG1();
	void		CreatePacketG2();
	void		AddMetadata(CSchemaPtr pSchema, CXMLElement* pXML, int nIndex);
	CString		GetXMLString(BOOL bNewlines = TRUE);
	void		WriteTrailer(int nHits);
	void		WriteTrailerG1(int nHits);
	void		WriteTrailerG2();
	void		DispatchPacket();
	void		DestroyPacket();
	CG2Packet*	AlbumToPacket(CAlbumFolder* pFolder);
	CG2Packet*	FoldersToPacket();
	CG2Packet*	FolderToPacket(CLibraryFolder* pFolder);

};
