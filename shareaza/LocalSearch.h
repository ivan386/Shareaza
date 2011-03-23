//
// LocalSearch.h
//
// Copyright (c) Shareaza Development Team, 2002-2011.
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
class CG1Packet;
class CG2Packet;
class CDCPacket;


class CLocalSearch
{
// Construction
public:
	CLocalSearch(CQuerySearch* pSearch, const CNeighbour* pNeighbour);
	CLocalSearch(CQuerySearch* pSearch, PROTOCOLID nProtocol);
	CLocalSearch(CBuffer* pBuffer, PROTOCOLID nProtocol);

	SOCKADDR_IN		m_pEndpoint;	// Endpoint or neighbour address
	BOOL			m_bUDP;			// Send packets via UDP or TCP

	// Search library and downloads (-1 - use default limit, 0 - no limit)
	bool		Execute(INT_PTR nMaximum = -1, bool bPartial = true, bool bShared = true);
	const CQuerySearch* GetSearch() const { return m_pSearch; }

protected:
	CQuerySearchPtr	m_pSearch;
	CBuffer*		m_pBuffer;		// Save packets to this buffer
	Hashes::Guid	m_oGUID;
	PROTOCOLID		m_nProtocol;
	typedef CMap< CSchemaPtr, CSchemaPtr, CXMLElement*, CXMLElement* > CSchemaMap;

	bool		ExecuteSharedFiles(INT_PTR nMaximum, INT_PTR& nHits);
	bool		ExecutePartialFiles(INT_PTR nMaximum, INT_PTR& nHits);
	template< typename T > void SendHits(const CList< T * >& oFiles);
	template< typename T > void AddHit(CPacket* pPacket, CSchemaMap& pSchemas, T * pHit, int nIndex);
	void		AddHitG1(CG1Packet* pPacket, CSchemaMap& pSchemas, CLibraryFile* pFile, int nIndex);
	void		AddHitG2(CG2Packet* pPacket, CSchemaMap& pSchemas, CLibraryFile* pFile, int nIndex);
	void		AddHitDC(CDCPacket* pPacket, CSchemaMap& pSchemas, CLibraryFile* pFile, int nIndex);
	void		AddHitG1(CG1Packet* pPacket, CSchemaMap& pSchemas, CDownload* pDownload, int nIndex);
	void		AddHitG2(CG2Packet* pPacket, CSchemaMap& pSchemas, CDownload* pDownload, int nIndex);
	void		AddHitDC(CDCPacket* pPacket, CSchemaMap& pSchemas, CDownload* pDownload, int nIndex);
	template< typename T > bool IsValidForHit(const T * pHit) const;

	CPacket*	CreatePacket();
	CG1Packet*	CreatePacketG1();
	CG2Packet*	CreatePacketG2();
	CDCPacket*	CreatePacketDC();
	void		WriteTrailer(CPacket* pPacket, CSchemaMap& pSchemas, BYTE nHits);
	void		WriteTrailerG1(CG1Packet* pPacket, CSchemaMap& pSchemas, BYTE nHits);
	void		WriteTrailerG2(CG2Packet* pPacket, CSchemaMap& pSchemas, BYTE nHits);
	void		WriteTrailerDC(CDCPacket* pPacket, CSchemaMap& pSchemas, BYTE nHits);
	void		DispatchPacket(CPacket* pPacket);
	CG2Packet*	AlbumToPacket(CAlbumFolder* pFolder);
	CG2Packet*	FoldersToPacket();
	CG2Packet*	FolderToPacket(CLibraryFolder* pFolder);

private:
	// Limit query answer packet size since Gnutella 1/2 drops packets
	// large than Settings.Gnutella.MaximumPacket
	static const DWORD	MAX_QUERY_PACKET_SIZE		= 16384; // (bytes)
};
