//
// LocalSearch.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "LocalSearch.h"
#include "QueryHit.h"

#include "Library.h"
#include "LibraryFolders.h"
#include "SharedFile.h"
#include "SharedFolder.h"
#include "AlbumFolder.h"

#include "QuerySearch.h"
#include "GProfile.h"
#include "Network.h"
#include "Neighbours.h"
#include "Neighbour.h"
#include "Datagrams.h"
#include "G1Packet.h"
#include "G2Packet.h"
#include "Buffer.h"
#include "ZLib.h"
#include "GGEP.h"
#include "BTClients.h"

#include "Transfers.h"
#include "Downloads.h"
#include "Download.h"
#include "Uploads.h"
#include "UploadQueue.h"
#include "UploadQueues.h"
#include "ImageServices.h"

#include "XML.h"
#include "Schema.h"
#include "SchemaCache.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// Limit query answer packet size since Gnutella 1/2 drops packets
// large than Settings.Gnutella.MaximumPacket
#define MAX_QUERY_PACKET_SIZE 16384 // (bytes)

//////////////////////////////////////////////////////////////////////
// CLocalSearch construction

CLocalSearch::CLocalSearch(CQuerySearch* pSearch, CNeighbour* pNeighbour, BOOL bWrapped) :
	m_pSearch		( pSearch ),
	m_pNeighbour	( pNeighbour ),
	m_pEndpoint		( NULL ),
	m_pBuffer		( NULL ),
	m_nProtocol		( bWrapped ? PROTOCOL_G1 : pNeighbour->m_nProtocol ),
	m_bWrapped		( bWrapped ),
	m_pPacket		( NULL )
{

}

CLocalSearch::CLocalSearch(CQuerySearch* pSearch, SOCKADDR_IN* pEndpoint) :
	m_pSearch		( pSearch ),
	m_pNeighbour	( NULL ),
	m_pEndpoint		( pEndpoint ),
	m_pBuffer		( NULL ),
	m_nProtocol		( PROTOCOL_G2 ),
	m_bWrapped		( FALSE ),
	m_pPacket		( NULL )
{
}

CLocalSearch::CLocalSearch(CQuerySearch* pSearch, CBuffer* pBuffer, PROTOCOLID nProtocol) :
	m_pSearch		( pSearch ),
	m_pNeighbour	( NULL ),
	m_pEndpoint		( NULL ),
	m_pBuffer		( pBuffer ),
	m_nProtocol		( nProtocol ),
	m_bWrapped		( FALSE ),
	m_pPacket		( NULL )
{
}

CLocalSearch::~CLocalSearch()
{
	GetXMLString();
}

template<>
bool CLocalSearch::IsValidForHit< CDownload >(const CDownload* pDownload) const
{
	return
		// If download shareable and ...
		pDownload->IsShared() &&
		// download active and ...
		( pDownload->IsTorrent() || pDownload->IsStarted() ) &&
		// download matches
		m_pSearch->Match( pDownload->GetSearchName(), pDownload->m_nSize, NULL,
			NULL, pDownload->m_oSHA1, pDownload->m_oTiger, pDownload->m_oED2K,
			pDownload->m_oBTH, pDownload->m_oMD5 );
}

template<>
bool CLocalSearch::IsValidForHit< CLibraryFile >(const CLibraryFile* pFile) const
{
	switch ( m_nProtocol )
	{
	case PROTOCOL_G1:
		return IsValidForHitG1( pFile );

	case PROTOCOL_G2:
		return IsValidForHitG2( pFile );

	default:
		return false;
	}
}

bool CLocalSearch::IsValidForHitG1(CLibraryFile const * const pFile) const
{
	return Settings.Gnutella1.EnableToday &&
		// Browse request, or real file
		( ! m_pSearch || pFile->IsAvailable() );
}

bool CLocalSearch::IsValidForHitG2(CLibraryFile const * const pFile) const
{
	return Settings.Gnutella2.EnableToday &&
		// Browse request, or comments request, or real file
		( ! m_pSearch || m_pSearch->m_bWantCOM || pFile->IsAvailable() );
}

//////////////////////////////////////////////////////////////////////
// CLocalSearch execute

INT_PTR CLocalSearch::Execute(INT_PTR nMaximum)
{
	if ( nMaximum < 0 )
		nMaximum = Settings.Gnutella.MaxHits;

	if ( m_pSearch )
	{
		m_oGUID = m_pSearch->m_oGUID;
	}
	else
	{
		Network.CreateID( m_oGUID );
	}

	INT_PTR nHits = ExecutePartialFiles( nMaximum );

	if ( ! nMaximum || nHits < nMaximum )
	{
		nHits += ExecuteSharedFiles( nMaximum ? nMaximum - nHits : 0 );
	}

	ASSERT( ! nMaximum || nHits <= nMaximum );

	return nHits;
}

//////////////////////////////////////////////////////////////////////
// CLocalSearch execute partial files

INT_PTR CLocalSearch::ExecutePartialFiles(INT_PTR nMaximum)
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 250 ) )
		return 0;

	if ( ! m_pSearch || ! m_pSearch->m_bWantPFS || m_nProtocol != PROTOCOL_G2 )
		// Browse request, or no partials requested, or non Gnutella 2 request
		return 0;

	INT_PTR nHits = 0;
	CList< const CDownload* > oFilesInPacket;

	for ( POSITION pos = Downloads.GetIterator() ;
		pos && ( ! nMaximum || ( nHits + oFilesInPacket.GetCount() < nMaximum ) ); )
	{
		const CDownload* pDownload = Downloads.GetNext( pos );

		if ( IsValidForHit( pDownload ) )
		{
			oFilesInPacket.AddTail( pDownload );

			if ( ( Settings.Gnutella.HitsPerPacket &&
				(DWORD)oFilesInPacket.GetCount() >= Settings.Gnutella.HitsPerPacket ) ||
				( m_pPacket && m_pPacket->m_nLength >= MAX_QUERY_PACKET_SIZE ) )
			{
				// Packet full, send it
				nHits += SendHits( oFilesInPacket );

				oFilesInPacket.RemoveAll();
			}
		}
	}

	// Send rest of files
	nHits += SendHits( oFilesInPacket );

	return nHits;
}

//////////////////////////////////////////////////////////////////////
// CLocalSearch execute shared files

INT_PTR CLocalSearch::ExecuteSharedFiles(INT_PTR nMaximum)
{
	CSingleLock oLock( &Library.m_pSection );
	if ( ! oLock.Lock( 1000 ) )
		return 0;

	CFileList* pFiles = Library.Search(
		m_pSearch, static_cast< int >( nMaximum ), FALSE,
		// Ghost files only for G2
		m_nProtocol != PROTOCOL_G2 );

	if ( pFiles == NULL )
		// No files found
		return 0;

	INT_PTR nHits = 0;
	CFileList oFilesInPacket;

	for ( POSITION pos = pFiles->GetHeadPosition() ;
		pos && ( ! nMaximum || ( nHits + oFilesInPacket.GetCount() < nMaximum ) ); )
	{
		const CLibraryFile* pFile = pFiles->GetNext( pos );

		// Select valid files
		if ( IsValidForHit( pFile ) )
		{
			oFilesInPacket.AddTail( pFile );

			if ( ( Settings.Gnutella.HitsPerPacket &&
				(DWORD)oFilesInPacket.GetCount() >= Settings.Gnutella.HitsPerPacket ) ||
				( m_pPacket && m_pPacket->m_nLength >= MAX_QUERY_PACKET_SIZE ) )
			{
				// Packet full, send it
				nHits += SendHits( oFilesInPacket );

				oFilesInPacket.RemoveAll();
			}
		}
	}

	// Send rest of files
	nHits += SendHits( oFilesInPacket );

	delete pFiles;

	return nHits;
}

template< typename T >
INT_PTR CLocalSearch::SendHits(const CList< const T * >& oFiles)
{
	INT_PTR nHits = oFiles.GetCount();
	if ( nHits )
	{
		CreatePacket( (int)nHits );

		int nHitIndex = 0;
		for ( POSITION pos = oFiles.GetHeadPosition(); pos; )
		{
			AddHit( oFiles.GetNext( pos ), nHitIndex++ );
		}

		WriteTrailer();

		DispatchPacket();
	}
	return nHits;
}

//////////////////////////////////////////////////////////////////////
// CLocalSearch add file hit

template<>
void CLocalSearch::AddHit< CLibraryFile >(const CLibraryFile* pFile, int nIndex)
{
	ASSERT( m_pPacket != NULL );

	switch ( m_nProtocol )
	{
	case PROTOCOL_G1:
		AddHitG1( pFile, nIndex );
		break;

	case PROTOCOL_G2:
		AddHitG2( pFile, nIndex );
		break;

	default:
		;
	}
}

void CLocalSearch::AddHitG1(CLibraryFile const * const pFile, int nIndex)
{
	m_pPacket->WriteLongLE( pFile->m_nIndex );
	m_pPacket->WriteLongLE( (DWORD)min( pFile->GetSize(), 0xFFFFFFFF ) );
	if ( Settings.Gnutella1.QueryHitUTF8 ) //Support UTF-8 Query
	{
		m_pPacket->WriteStringUTF8( pFile->m_sName );
	}
	else
	{
		m_pPacket->WriteString( pFile->m_sName );
	}

	if ( pFile->m_oSHA1 )
	{
		CString strHash = pFile->m_oSHA1.toUrn();
		m_pPacket->WriteString( strHash );

		/*
		CGGEPBlock pBlock;

		CGGEPItem* pItem = pBlock.Add( GGEP_HEADER_HASH );
		pItem->WriteByte( 1 );
		pItem->Write( &pFile->m_pSHA1, 20 );

		pBlock.Write( m_pPacket );
		m_pPacket->WriteByte( 0 );
		*/
	}
	else if ( pFile->m_oTiger )
	{
		CString strHash = pFile->m_oTiger.toUrn();
		m_pPacket->WriteString( strHash );
	}
	else if ( pFile->m_oED2K )
	{
		CString strHash = pFile->m_oED2K.toUrn();
		m_pPacket->WriteString( strHash );
	}
	else if ( pFile->m_oBTH )
	{
		CString strHash = pFile->m_oBTH.toUrn();
		m_pPacket->WriteString( strHash );
	}
	else if ( pFile->m_oMD5 )
	{
		CString strHash = pFile->m_oMD5.toUrn();
		m_pPacket->WriteString( strHash );
	}
	else
	{
		m_pPacket->WriteByte( 0 );
	}

	if ( pFile->m_pSchema != NULL && pFile->m_pMetadata != NULL && ( ! m_pSearch || m_pSearch->m_bWantXML ) )
	{
		AddMetadata( pFile->m_pSchema, pFile->m_pMetadata, nIndex );
	}
}

void CLocalSearch::AddHitG2(CLibraryFile const * const pFile, int /*nIndex*/)
{
	// Pass 1: Calculate child group size
	// Pass 2: Write the child packet
	CG2Packet* pPacket = static_cast< CG2Packet* >( m_pPacket );
	DWORD nGroup = 0;
	bool bCalculate = false;
	do
	{
		bCalculate = ! bCalculate;

		if ( ! bCalculate )
		{
			pPacket->WritePacket( G2_PACKET_HIT_DESCRIPTOR, nGroup, TRUE );
		}

		if ( pFile->m_oTiger && pFile->m_oSHA1 )
		{
			const char prefix[] = "bp";
			if ( bCalculate )
				nGroup += G2_PACKET_LEN( G2_PACKET_URN, sizeof( prefix ) + Hashes::Sha1Hash::byteCount + Hashes::TigerHash::byteCount );
			else
			{
				pPacket->WritePacket( G2_PACKET_URN, sizeof( prefix ) + Hashes::Sha1Hash::byteCount + Hashes::TigerHash::byteCount );
				pPacket->WriteString( prefix );
				pPacket->Write( pFile->m_oSHA1 );
				pPacket->Write( pFile->m_oTiger );
			}
		}
		else if ( pFile->m_oTiger )
		{
			const char prefix[] = "ttr";
			if ( bCalculate )
				nGroup += G2_PACKET_LEN( G2_PACKET_URN, sizeof( prefix ) + Hashes::TigerHash::byteCount );
			else
			{
				pPacket->WritePacket( G2_PACKET_URN, sizeof( prefix ) + Hashes::TigerHash::byteCount );
				pPacket->WriteString( prefix );
				pPacket->Write( pFile->m_oTiger );
			}
		}
		else if ( pFile->m_oSHA1 )
		{
			const char prefix[] = "sha1";
			if ( bCalculate )
				nGroup += G2_PACKET_LEN( G2_PACKET_URN, sizeof( prefix ) + Hashes::Sha1Hash::byteCount );
			else
			{
				pPacket->WritePacket( G2_PACKET_URN, sizeof( prefix ) + Hashes::Sha1Hash::byteCount );
				pPacket->WriteString( prefix );
				pPacket->Write( pFile->m_oSHA1 );
			}
		}

		if ( pFile->m_oED2K )
		{
			const char prefix[] = "ed2k";
			if ( bCalculate )
				nGroup += G2_PACKET_LEN( G2_PACKET_URN, sizeof( prefix ) + Hashes::Ed2kHash::byteCount );
			else
			{
				pPacket->WritePacket( G2_PACKET_URN, sizeof( prefix ) + Hashes::Ed2kHash::byteCount );
				pPacket->WriteString( prefix );
				pPacket->Write( pFile->m_oED2K );
			}
		}

		if ( pFile->m_oBTH )
		{
			const char prefix[] = "btih";
			if ( bCalculate )
				nGroup += G2_PACKET_LEN( G2_PACKET_URN, sizeof( prefix ) + Hashes::BtHash::byteCount );
			else
			{
				pPacket->WritePacket( G2_PACKET_URN, sizeof( prefix ) + Hashes::BtHash::byteCount );
				pPacket->WriteString( prefix );
				pPacket->Write( pFile->m_oBTH );
			}
		}

		if ( pFile->m_oMD5 )
		{
			const char prefix[] = "md5";
			if ( bCalculate )
				nGroup += G2_PACKET_LEN( G2_PACKET_URN, sizeof( prefix ) + Hashes::Md5Hash::byteCount );
			else
			{
				pPacket->WritePacket( G2_PACKET_URN, sizeof( prefix ) + Hashes::Md5Hash::byteCount );
				pPacket->WriteString( prefix );
				pPacket->Write( pFile->m_oMD5 );
			}
		}

		if ( ! m_pSearch || m_pSearch->m_bWantDN )
		{
			if ( pFile->GetSize() <= 0xFFFFFFFF )
			{
				if ( bCalculate )
					nGroup += G2_PACKET_LEN( G2_PACKET_DESCRIPTIVE_NAME, sizeof( DWORD ) + pPacket->GetStringLen( pFile->m_sName ) );
				else
				{
					pPacket->WritePacket( G2_PACKET_DESCRIPTIVE_NAME, sizeof( DWORD ) + pPacket->GetStringLen( pFile->m_sName ) );
					pPacket->WriteLongBE( (DWORD)pFile->GetSize() );
					pPacket->WriteString( pFile->m_sName, FALSE );
				}
			}
			else
			{
				if ( bCalculate )
					nGroup += G2_PACKET_LEN( G2_PACKET_SIZE, sizeof( QWORD ) ) +
						G2_PACKET_LEN( G2_PACKET_DESCRIPTIVE_NAME, pPacket->GetStringLen( pFile->m_sName ) );
				else
				{
					pPacket->WritePacket( G2_PACKET_SIZE, sizeof( QWORD ) );
					pPacket->WriteInt64( pFile->GetSize() );
					pPacket->WritePacket( G2_PACKET_DESCRIPTIVE_NAME, pPacket->GetStringLen( pFile->m_sName ) );
					pPacket->WriteString( pFile->m_sName, FALSE );
				}
			}

			if ( LPCTSTR pszType = _tcsrchr( pFile->m_sName, '.' ) )
			{
				if ( _tcsicmp( pszType, _T(".co") ) == 0 ||
					 _tcsicmp( pszType, _T(".collection") ) == 0 ||
					 _tcsicmp( pszType, _T(".emulecollection") ) == 0 )
				{
					if ( ! pFile->m_bBogus )
					{
						if ( bCalculate )
							nGroup += G2_PACKET_LEN( G2_PACKET_COLLECTION, 0 );
						else
							pPacket->WritePacket( G2_PACKET_COLLECTION, 0 );
					}
				}
			}
		}

		if ( ! m_pSearch || m_pSearch->m_bWantURL )
		{
			if ( bCalculate )
				nGroup += G2_PACKET_LEN( G2_PACKET_URL, 0 );
			else
				pPacket->WritePacket( G2_PACKET_URL, 0 );

			if ( INT_PTR nCount = pFile->m_pSources.GetCount() )
			{
				if ( bCalculate )
					nGroup += G2_PACKET_LEN( G2_PACKET_CACHED_SOURCES, sizeof( WORD ) );
				else
				{
					pPacket->WritePacket( G2_PACKET_CACHED_SOURCES, sizeof( WORD ) );
					pPacket->WriteShortBE( (WORD)nCount );
				}
			}

			if ( Settings.Uploads.SharePreviews &&
				( pFile->m_bCachedPreview || ( Settings.Uploads.DynamicPreviews &&
				CImageServices::IsFileViewable( (LPCTSTR)pFile->m_sName ) ) ) )
			{
				if ( bCalculate )
					nGroup += G2_PACKET_LEN( G2_PACKET_PREVIEW_URL, 0 );
				else
					pPacket->WritePacket( G2_PACKET_PREVIEW_URL, 0 );
			}
		}

		if ( pFile->m_pMetadata != NULL && ( ! m_pSearch || m_pSearch->m_bWantXML ) )
		{
			CString strMetadata = pFile->m_pMetadata->ToString();
			if ( bCalculate )
				nGroup += G2_PACKET_LEN( G2_PACKET_METADATA, pPacket->GetStringLen( strMetadata ) );
			else
			{
				pPacket->WritePacket( G2_PACKET_METADATA, pPacket->GetStringLen( strMetadata ) );
				pPacket->WriteString( strMetadata, FALSE );
			}
		}

		{
			CQuickLock pQueueLock( UploadQueues.m_pSection );
			CUploadQueue* pQueue = UploadQueues.SelectQueue( PROTOCOL_HTTP, pFile );
			if ( bCalculate )
				nGroup += G2_PACKET_LEN( G2_PACKET_GROUP_ID, sizeof( BYTE ) );
			else
			{
				pPacket->WritePacket( G2_PACKET_GROUP_ID, sizeof( BYTE ) );
				pPacket->WriteByte( BYTE( pQueue ? pQueue->m_nIndex + 1 : 0 ) );
			}
		}

		if ( ! m_pSearch || m_pSearch->m_bWantCOM )
		{
			if ( pFile->IsRated() )
			{
				CString strComment;
				if ( pFile->m_nRating > 0 )
					strComment.Format( _T("<comment rating=\"%i\">"), pFile->m_nRating - 1 );
				else
					strComment = _T("<comment>");
				CXMLNode::ValueToString( pFile->m_sComments, strComment );
				if ( strComment.GetLength() > 2048 ) strComment = strComment.Left( 2048 );
				strComment += _T("</comment>");
				strComment.Replace( _T("\r\n"), _T("{n}") );
				if ( bCalculate )
					nGroup += G2_PACKET_LEN( G2_PACKET_COMMENT, pPacket->GetStringLen( strComment ) );
				else
				{
					pPacket->WritePacket( G2_PACKET_COMMENT, pPacket->GetStringLen( strComment ) );
					pPacket->WriteString( strComment, FALSE );
				}
			}

			if ( pFile->m_bBogus )
			{
				if ( bCalculate )
					nGroup += G2_PACKET_LEN( G2_PACKET_BOGUS, 0 );
				else
					pPacket->WritePacket( G2_PACKET_BOGUS, 0 );
			}
		}


		if ( ! m_pSearch )
		{
			if ( bCalculate )
				nGroup += G2_PACKET_LEN( G2_PACKET_OBJECT_ID, sizeof( DWORD ) );
			else
			{
				pPacket->WritePacket( G2_PACKET_OBJECT_ID, sizeof( DWORD ) );
				pPacket->WriteLongBE( pFile->m_nIndex );
			}
		}
	}
	while( bCalculate );
}

//////////////////////////////////////////////////////////////////////
// CLocalSearch add download hit

template<>
void CLocalSearch::AddHit< CDownload >(const CDownload* pDownload, int /*nIndex*/)
{
	// Pass 1: Calculate child group size
	// Pass 2: Write the child packet
	CG2Packet* pPacket = static_cast< CG2Packet* >( m_pPacket );
	DWORD nGroup = 0;
	bool bCalculate = false;
	do
	{
		bCalculate = ! bCalculate;

		if ( ! bCalculate )
		{
			pPacket->WritePacket( G2_PACKET_HIT_DESCRIPTOR, nGroup, TRUE );
		}

		if ( pDownload->m_oTiger && pDownload->m_oSHA1 )
		{
			const char prefix[] = "bp";
			if ( bCalculate )
				nGroup += G2_PACKET_LEN( G2_PACKET_URN, sizeof( prefix ) + Hashes::Sha1Hash::byteCount + Hashes::TigerHash::byteCount );
			else
			{
				pPacket->WritePacket( G2_PACKET_URN, sizeof( prefix ) + Hashes::Sha1Hash::byteCount + Hashes::TigerHash::byteCount );
				pPacket->WriteString( prefix );
				pPacket->Write( pDownload->m_oSHA1 );
				pPacket->Write( pDownload->m_oTiger );
			}
		}
		else if ( pDownload->m_oTiger )
		{
			const char prefix[] = "ttr";
			if ( bCalculate )
				nGroup += G2_PACKET_LEN( G2_PACKET_URN, sizeof( prefix ) + Hashes::TigerHash::byteCount );
			else
			{
				pPacket->WritePacket( G2_PACKET_URN, sizeof( prefix ) + Hashes::TigerHash::byteCount );
				pPacket->WriteString( prefix );
				pPacket->Write( pDownload->m_oTiger );
			}
		}
		else if ( pDownload->m_oSHA1 )
		{
			const char prefix[] = "sha1";
			if ( bCalculate )
				nGroup += G2_PACKET_LEN( G2_PACKET_URN, sizeof( prefix ) + Hashes::Sha1Hash::byteCount );
			else
			{
				pPacket->WritePacket( G2_PACKET_URN, sizeof( prefix ) + Hashes::Sha1Hash::byteCount );
				pPacket->WriteString( prefix );
				pPacket->Write( pDownload->m_oSHA1 );
			}
		}

		if ( pDownload->m_oED2K )
		{
			const char prefix[] = "ed2k";
			if ( bCalculate )
				nGroup += G2_PACKET_LEN( G2_PACKET_URN, sizeof( prefix ) + Hashes::Ed2kHash::byteCount );
			else
			{
				pPacket->WritePacket( G2_PACKET_URN, sizeof( prefix ) + Hashes::Ed2kHash::byteCount );
				pPacket->WriteString( prefix );
				pPacket->Write( pDownload->m_oED2K );
			}
		}

		if ( pDownload->m_oBTH )
		{
			const char prefix[] = "btih";
			if ( bCalculate )
				nGroup += G2_PACKET_LEN( G2_PACKET_URN, sizeof( prefix ) + Hashes::BtHash::byteCount );
			else
			{
				pPacket->WritePacket( G2_PACKET_URN, sizeof( prefix ) + Hashes::BtHash::byteCount );
				pPacket->WriteString( prefix );
				pPacket->Write( pDownload->m_oBTH );
			}
		}

		if ( pDownload->m_oMD5 )
		{
			const char prefix[] = "md5";
			if ( bCalculate )
				nGroup += G2_PACKET_LEN( G2_PACKET_URN, sizeof( prefix ) + Hashes::Md5Hash::byteCount );
			else
			{
				pPacket->WritePacket( G2_PACKET_URN, sizeof( prefix ) + Hashes::Md5Hash::byteCount );
				pPacket->WriteString( prefix );
				pPacket->Write( pDownload->m_oMD5 );
			}
		}

		if ( m_pSearch->m_bWantDN )
		{
			if ( pDownload->m_nSize <= 0xFFFFFFFF )
			{
				if ( bCalculate )
					nGroup += G2_PACKET_LEN( G2_PACKET_DESCRIPTIVE_NAME, sizeof( DWORD ) + pPacket->GetStringLen( pDownload->m_sName ) );
				else
				{
					pPacket->WritePacket( G2_PACKET_DESCRIPTIVE_NAME, sizeof( DWORD ) + pPacket->GetStringLen( pDownload->m_sName ) );
					pPacket->WriteLongBE( (DWORD)pDownload->m_nSize );
					pPacket->WriteString( pDownload->m_sName, FALSE );
				}
			}
			else
			{
				if ( bCalculate )
					nGroup += G2_PACKET_LEN( G2_PACKET_SIZE, sizeof( QWORD ) ) +
						G2_PACKET_LEN( G2_PACKET_DESCRIPTIVE_NAME, pPacket->GetStringLen( pDownload->m_sName ) );
				else
				{
					pPacket->WritePacket( G2_PACKET_SIZE, sizeof( QWORD ) );
					pPacket->WriteInt64( pDownload->m_nSize );
					pPacket->WritePacket( G2_PACKET_DESCRIPTIVE_NAME, pPacket->GetStringLen( pDownload->m_sName ) );
					pPacket->WriteString( pDownload->m_sName, FALSE );
				}
			}
		}

		if ( m_pSearch->m_bWantURL )
		{
			if ( bCalculate )
				nGroup += G2_PACKET_LEN( G2_PACKET_URL, 0 );
			else
				pPacket->WritePacket( G2_PACKET_URL, 0 );
		}

		QWORD nComplete = pDownload->GetVolumeComplete();
		if ( nComplete <= 0xFFFFFFFF )
		{
			if ( bCalculate )
				nGroup += G2_PACKET_LEN( G2_PACKET_PARTIAL, sizeof( DWORD ) );
			else
			{
				pPacket->WritePacket( G2_PACKET_PARTIAL, sizeof( DWORD ) );
				pPacket->WriteLongBE( (DWORD)nComplete );
			}
		}
		else
		{
			if ( bCalculate )
				nGroup += G2_PACKET_LEN( G2_PACKET_PARTIAL, sizeof( QWORD ) );
			else
			{
				pPacket->WritePacket( G2_PACKET_PARTIAL, sizeof( QWORD ) );
				pPacket->WriteInt64( nComplete );
			}
		}
	}
	while( bCalculate );
}

//////////////////////////////////////////////////////////////////////
// CLocalSearch create packet

void CLocalSearch::CreatePacket(int nCount)
{
	ASSERT( m_pPacket == NULL );

	if ( m_nProtocol == PROTOCOL_G1 )
		CreatePacketG1( nCount );
	else
		CreatePacketG2();

	if ( m_pSchemas.GetCount() ) GetXMLString();
}

void CLocalSearch::CreatePacketG1(int nCount)
{
	m_pPacket = CG1Packet::New( G1_PACKET_HIT,
		( m_pSearch ? m_pSearch->m_nTTL : Settings.Gnutella1.SearchTTL ), m_oGUID );

	m_pPacket->WriteByte( BYTE( nCount ) );
	m_pPacket->WriteShortLE( htons( Network.m_pHost.sin_port ) );
	m_pPacket->WriteLongLE( Network.m_pHost.sin_addr.S_un.S_addr );

	if ( Uploads.m_bStable )
	{
		m_pPacket->WriteLongLE( Uploads.m_nBestSpeed * 8 / 1024 );
	}
	else
	{
		m_pPacket->WriteLongLE( Settings.Connection.OutSpeed );
	}
}

void CLocalSearch::CreatePacketG2()
{
	CG2Packet* pPacket = CG2Packet::New( G2_PACKET_HIT, TRUE );
	m_pPacket = pPacket;

	pPacket->WritePacket( G2_PACKET_NODE_GUID, 16 );
	pPacket->Write( Hashes::Guid( MyProfile.oGUID ) );

	//if ( Network.IsListening() )
	{
		pPacket->WritePacket( G2_PACKET_NODE_ADDRESS, 6 );
		pPacket->WriteLongLE( Network.m_pHost.sin_addr.S_un.S_addr );
		pPacket->WriteShortBE( htons( Network.m_pHost.sin_port ) );
	}

	pPacket->WritePacket( G2_PACKET_VENDOR, 4 );
	pPacket->WriteString( VENDOR_CODE, FALSE );

	if ( Network.IsFirewalled() )
	{
		pPacket->WritePacket( G2_PACKET_PEER_FIREWALLED, 0 );
	}

	{
		CSingleLock pNetLock( &Network.m_pSection );

		if ( pNetLock.Lock( 50 ) )
		{
			for ( POSITION pos = Neighbours.GetIterator() ; pos ; )
			{
				CNeighbour* pNeighbour = Neighbours.GetNext( pos );

				if ( pNeighbour->m_nNodeType != ntLeaf &&
					 pNeighbour->m_nProtocol == PROTOCOL_G2 )
				{
					pPacket->WritePacket( G2_PACKET_NEIGHBOUR_HUB, 6 );
					pPacket->WriteLongLE( pNeighbour->m_pHost.sin_addr.S_un.S_addr );
					pPacket->WriteShortBE( htons( pNeighbour->m_pHost.sin_port ) );
				}
			}
		}
	}

	if ( ! Uploads.m_bStable ) pPacket->WritePacket( G2_PACKET_PEER_UNSTABLE, 0 );

	CSingleLock pQueueLock( &UploadQueues.m_pSection );
	int nQueue = 1;

	if ( pQueueLock.Lock() )
	{
		for ( POSITION pos = UploadQueues.GetIterator() ; pos ; nQueue++ )
		{
			CUploadQueue* pQueue = UploadQueues.GetNext( pos );
			pPacket->WritePacket( G2_PACKET_HIT_GROUP, ( 4 + 7 ) + 2, TRUE );
			pPacket->WritePacket( G2_PACKET_PEER_STATUS, 7 );
			pPacket->WriteShortBE( WORD( pQueue->GetQueuedCount() + pQueue->GetTransferCount() ) );
			pPacket->WriteByte( BYTE( pQueue->GetTransferCount( TRUE ) ) );
			pPacket->WriteLongBE( pQueue->GetPredictedBandwidth() * 8 / 1024 );
			pPacket->WriteByte( 0 );
			pPacket->WriteByte( BYTE( nQueue ) );
		}

		pQueueLock.Unlock();
	}

	CString strNick = MyProfile.GetNick();
	if ( strNick.GetLength() > 32 ) strNick = strNick.Left( 32 );

	if ( strNick.GetLength() )
	{
		int nNick = pPacket->GetStringLen( strNick );
		pPacket->WritePacket( G2_PACKET_PROFILE, nNick + 6, TRUE );
		pPacket->WritePacket( G2_PACKET_NICK, nNick );
		pPacket->WriteString( strNick, FALSE );
	}

	if ( Settings.Community.ServeProfile ) pPacket->WritePacket( G2_PACKET_BROWSE_PROFILE, 0 );
	if ( Settings.Community.ServeFiles ) pPacket->WritePacket( G2_PACKET_BROWSE_HOST, 0 );
	if ( Settings.Community.ChatEnable ) pPacket->WritePacket( G2_PACKET_PEER_CHAT, 0 );
}

//////////////////////////////////////////////////////////////////////
// CLocalSearch meta data

void CLocalSearch::AddMetadata(CSchema* pSchema, CXMLElement* pXML, int nIndex)
{
	ASSERT( pSchema != NULL );
	ASSERT( pXML != NULL );
	ASSERT( pXML->GetParent() == NULL );

	CXMLElement* pGroup;

	if ( ! m_pSchemas.Lookup( pSchema, pGroup ) )
	{
		pGroup = pSchema->Instantiate();
		m_pSchemas.SetAt( pSchema, pGroup );
	}

	CString strIndex;
	strIndex.Format( _T("%lu"), nIndex );

	pXML->AddAttribute( _T("index"), strIndex );
	pGroup->AddElement( pXML );
}

//////////////////////////////////////////////////////////////////////
// CLocalSearch XML to string

CString CLocalSearch::GetXMLString(BOOL bNewlines)
{
	CString strXML;

	for ( POSITION pos1 = m_pSchemas.GetStartPosition() ; pos1 ; )
	{
		CXMLElement* pGroup;
		CSchema* pSchema;

		m_pSchemas.GetNextAssoc( pos1, pSchema, pGroup );

		strXML += pGroup->ToString( TRUE, bNewlines );

		for ( POSITION pos2 = pGroup->GetElementIterator() ; pos2 ; )
		{
			CXMLElement* pChild = pGroup->GetNextElement( pos2 );
			pChild->DeleteAttribute( _T("index") );
			pChild->Detach();
		}

		delete pGroup;
	}

	m_pSchemas.RemoveAll();

	return strXML;
}

//////////////////////////////////////////////////////////////////////
// CLocalSearch core trailer

void CLocalSearch::WriteTrailer()
{
	ASSERT( m_pPacket != NULL );

	if ( m_nProtocol == PROTOCOL_G1 )
		WriteTrailerG1();
	else
		WriteTrailerG2();
}

void CLocalSearch::WriteTrailerG1()
{
	CG1Packet* pPacket = static_cast< CG1Packet* >( m_pPacket );

	pPacket->WriteString( _T( VENDOR_CODE ), FALSE );

	BYTE nFlags[2] = { 0, 0 };

	nFlags[0] |= G1_QHD_BUSY|G1_QHD_STABLE|G1_QHD_SPEED;
	nFlags[1] |= G1_QHD_PUSH;

	if ( Network.IsFirewalled() ) nFlags[0] |= G1_QHD_PUSH;
	if ( Uploads.m_bStable ) nFlags[1] |= G1_QHD_STABLE;
	if ( Uploads.m_bStable ) nFlags[1] |= G1_QHD_SPEED;
	if ( ! UploadQueues.IsTransferAvailable() ) nFlags[1] |= G1_QHD_BUSY;

	if ( Settings.Community.ServeFiles && Settings.Gnutella1.EnableGGEP )
	{
		nFlags[0] |= G1_QHD_GGEP;
		nFlags[1] |= G1_QHD_GGEP;
	}

	CString strXML		= GetXMLString( FALSE );
	DWORD nCompressed	= 0;
	auto_array< BYTE > pCompressed;

	pPacket->WriteByte( strXML.IsEmpty() ? 2 : 4 );
	pPacket->WriteByte( nFlags[0] );
	pPacket->WriteByte( nFlags[1] );

	LPSTR pszXML = NULL;
	int nXML = 0;

	if ( strXML.GetLength() > 0 )
	{
		nXML = WideCharToMultiByte( CP_ACP, 0, strXML, -1, NULL, 0, NULL, NULL );
		pszXML = new CHAR[ nXML ];
		WideCharToMultiByte( CP_ACP, 0, strXML, -1, pszXML, nXML, NULL, NULL );
		if ( nXML > 0 ) nXML --;

		pCompressed = CZLib::Compress( pszXML, nXML, &nCompressed );

		// 9 = "{deflate}", 11 = "{plaintext}"
		if ( nCompressed + 9 < (DWORD)nXML + 11 && pCompressed.get() != NULL )
		{
			pPacket->WriteShortLE( (WORD)( nCompressed + 9 + 1 ) );
		}
		else
		{
			pPacket->WriteShortLE( WORD( nXML + 11 + 1 ) );
			pCompressed.reset();
		}
	}

	pPacket->WriteByte( Settings.Community.ChatEnable ? 1 : 0 );

	if ( Settings.Community.ServeFiles && Settings.Gnutella1.EnableGGEP )
	{
		pPacket->WriteByte( GGEP_MAGIC );
		pPacket->WriteByte( GGEP_HDR_LAST | 2 );
		pPacket->WriteByte( 'B' );
		pPacket->WriteByte( 'H' );
		pPacket->WriteByte( GGEP_LEN_LAST );
	}

	if ( pCompressed.get() != NULL )
	{
		pPacket->Write( "{deflate}", 9 );
		pPacket->Write( pCompressed.get(), nCompressed );
		pPacket->WriteByte( 0 );
	}
	else if ( pszXML != NULL )
	{
		pPacket->Write( "{plaintext}", 11 );
		pPacket->Write( pszXML, nXML );
	}

	if ( pszXML != NULL ) delete [] pszXML;

	pPacket->Write( Hashes::Guid( MyProfile.oGUID ) );

#ifdef _DEBUG
	// Test created hit
	if ( CQueryHit* pDebugHit = CQueryHit::FromG1Packet( pPacket ) )
	{
		pDebugHit->Delete();
		m_pPacket->m_nPosition = 0;
	}
	else
		theApp.Message( MSG_ERROR | MSG_FACILITY_SEARCH, _T("[G1] Shareaza produced search packet above but cannot parse it back!") );
#endif // _DEBUG
}

void CLocalSearch::WriteTrailerG2()
{
	CG2Packet* pPacket = static_cast< CG2Packet* >( m_pPacket );

	pPacket->WriteByte( 0 );	// End of packet
	pPacket->WriteByte( 0 );	// nHops
	pPacket->Write( m_oGUID );	// SearchID

#ifdef _DEBUG
	// Test created hit
	if ( CQueryHit* pDebugHit = CQueryHit::FromG2Packet( pPacket ) )
	{
		pDebugHit->Delete();
		m_pPacket->m_nPosition = 0;
	}
	else
		theApp.Message( MSG_ERROR | MSG_FACILITY_SEARCH, _T("[G2] Shareaza produced search packet above but cannot parse it back!") );
#endif // _DEBUG
}

//////////////////////////////////////////////////////////////////////
// CLocalSearch dispatch packet

void CLocalSearch::DispatchPacket()
{
	ASSERT( m_pPacket != NULL );

	if ( m_pNeighbour != NULL )
	{
		if ( m_bWrapped )
		{
			// ****Debug
			theApp.Message( MSG_INFO | MSG_FACILITY_SEARCH, _T("CLocalSearch::DispatchPacket() Wrapped query hit created") );
			// ****

			CG2Packet* pG2 = CG2Packet::New( G2_PACKET_HIT_WRAP, (CG1Packet*)m_pPacket );
			m_pPacket->Release();
			m_pPacket = pG2;
		}

		m_pNeighbour->Send( m_pPacket, FALSE, TRUE );
	}

	if ( m_pEndpoint != NULL )
	{
		Datagrams.Send( m_pEndpoint, static_cast< CG2Packet* >( m_pPacket ), FALSE );
	}

	if ( m_pBuffer != NULL )
	{
		m_pPacket->ToBuffer( m_pBuffer );
	}

	DestroyPacket();
}

void CLocalSearch::DestroyPacket()
{
	if ( m_pPacket != NULL )
	{
		m_pPacket->Release();
		m_pPacket = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// CLocalSearch physical and virtual folder tree

void CLocalSearch::WriteVirtualTree()
{
	CSingleLock oLock( &Library.m_pSection );
	if ( oLock.Lock( 1000 ) )
	{
		m_pPacket = AlbumToPacket( Library.GetAlbumRoot() );
		oLock.Unlock();
		if ( m_pPacket != NULL ) DispatchPacket();
	}

	if ( oLock.Lock( 1000 ) )
	{
		m_pPacket = FoldersToPacket();
		oLock.Unlock();
		if ( m_pPacket != NULL ) DispatchPacket();
	}
}

CG2Packet* CLocalSearch::AlbumToPacket(CAlbumFolder* pFolder)
{
	if ( pFolder == NULL ) return NULL;

	if ( pFolder->m_pSchema != NULL && pFolder->m_pSchema->m_bPrivate ) return NULL;
	if ( pFolder->GetSharedCount() == 0 ) return NULL;

	CG2Packet* pPacket = CG2Packet::New( G2_PACKET_VIRTUAL_FOLDER, TRUE );

	if ( pFolder->m_pSchema != NULL )
	{
		CXMLElement* pXML = pFolder->m_pSchema->Instantiate( TRUE );

		if ( pFolder->m_pXML != NULL )
		{
			pXML->AddElement( pFolder->m_pXML->Clone() );
		}
		else
		{
			CXMLElement* pBody = pXML->AddElement( pFolder->m_pSchema->m_sSingular );
			pBody->AddAttribute( pFolder->m_pSchema->GetFirstMemberName(), pFolder->m_sName );
		}

		CString strXML = pXML->ToString();
		delete pXML;

		pPacket->WritePacket( G2_PACKET_METADATA, pPacket->GetStringLen( strXML ) );
		pPacket->WriteString( strXML, FALSE );
	}

	for ( POSITION pos = pFolder->GetFolderIterator() ; pos ; )
	{
		if ( CG2Packet* pChild = AlbumToPacket( pFolder->GetNextFolder( pos ) ) )
		{
			pPacket->WritePacket( pChild );
			pChild->Release();
		}
	}

	pPacket->WritePacket( G2_PACKET_FILES, static_cast< DWORD >( pFolder->GetFileCount() * 4 ) );

	for ( POSITION pos = pFolder->GetFileIterator() ; pos ; )
	{
		CLibraryFile* pFile = pFolder->GetNextFile( pos );
		pPacket->WriteLongBE( pFile->m_nIndex );
	}

	return pPacket;
}

CG2Packet* CLocalSearch::FoldersToPacket()
{
	CG2Packet* pPacket = CG2Packet::New( G2_PACKET_PHYSICAL_FOLDER, TRUE );

	for ( POSITION pos = LibraryFolders.GetFolderIterator() ; pos ; )
	{
		if ( CG2Packet* pChild = FolderToPacket( LibraryFolders.GetNextFolder( pos ) ) )
		{
			pPacket->WritePacket( pChild );
			pChild->Release();
		}
	}

	return pPacket;
}

CG2Packet* CLocalSearch::FolderToPacket(CLibraryFolder* pFolder)
{
	if ( pFolder == NULL ) return NULL;

	if ( pFolder->GetSharedCount() == 0 ) return NULL;

	CG2Packet* pPacket = CG2Packet::New( G2_PACKET_PHYSICAL_FOLDER, TRUE );

	pPacket->WritePacket( G2_PACKET_DESCRIPTIVE_NAME, pPacket->GetStringLen( pFolder->m_sName ) );
	pPacket->WriteString( pFolder->m_sName, FALSE );

	for ( POSITION pos = pFolder->GetFolderIterator() ; pos ; )
	{
		if ( CG2Packet* pChild = FolderToPacket( pFolder->GetNextFolder( pos ) ) )
		{
			pPacket->WritePacket( pChild );
			pChild->Release();
		}
	}

	pPacket->WritePacket( G2_PACKET_FILES, static_cast< DWORD >( pFolder->GetFileCount() * 4 ) );

	for ( POSITION pos = pFolder->GetFileIterator() ; pos ; )
	{
		CLibraryFile* pFile = pFolder->GetNextFile( pos );
		pPacket->WriteLongBE( pFile->m_nIndex );
	}

	return pPacket;
}
