//
// QuerySearch.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "QuerySearch.h"
#include "Network.h"
#include "Datagrams.h"
#include "G1Packet.h"
#include "G2Packet.h"
#include "EDPacket.h"

#include "Schema.h"
#include "SchemaCache.h"
#include "QueryHashTable.h"
#include "GGEP.h"
#include "XML.h"
#include "SHA.h"
#include "MD5.h"
#include "ED2K.h"
#include "TigerTree.h"

#include "WndSearch.h"

#include "Download.h"
#include "Downloads.h"
#include "Transfers.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CQuerySearch construction

CQuerySearch::CQuerySearch(BOOL bGUID)
: m_oWords()
{
	if ( bGUID ) Network.CreateID( m_oGUID );
	
	m_pSchema	= NULL;
	m_pXML		= NULL;
	m_nMinSize	= 0x0000000000000000;
	m_nMaxSize	= 0xFFFFFFFFFFFFFFFF;
	
	m_bWantURL	= TRUE;
	m_bWantDN	= TRUE;
	m_bWantXML	= TRUE;
	m_bWantCOM	= TRUE;
	m_bWantPFS	= TRUE;
	m_bAndG1	= Settings.Gnutella1.EnableToday;
	
	m_bUDP		= FALSE;
	m_nKey		= 0;
	m_bFirewall	= FALSE;
	
//	m_nWords	= 0;
//	m_pWordPtr	= NULL;
//	m_pWordLen	= NULL;
}

CQuerySearch::CQuerySearch(const CQuerySearch* pOrigin)
: m_oGUID( pOrigin->m_oGUID ),
  m_sSearch( pOrigin->m_sSearch ),
  m_sKeywords( pOrigin->m_sKeywords ),
  m_sPosKeywords( pOrigin->m_sPosKeywords ),
  m_sG2Keywords( pOrigin->m_sG2Keywords ),
  m_pSchema( pOrigin->m_pSchema ),
  m_pXML( pOrigin->m_pXML ? pOrigin->m_pXML->Clone() : NULL ),
  m_nMinSize( pOrigin->m_nMinSize ),
  m_nMaxSize( pOrigin->m_nMaxSize ),

  m_oSimilarED2K(),         //! \todo verify this

  m_bWantURL( pOrigin->m_bWantURL ),
  m_bWantDN( pOrigin->m_bWantDN ),
  m_bWantXML( pOrigin->m_bWantXML ),
  m_bWantCOM( pOrigin->m_bWantCOM ),
  m_bWantPFS( pOrigin->m_bWantPFS ),
  m_bAndG1( pOrigin->m_bAndG1 ),

  m_bUDP( pOrigin->m_bUDP ),
  m_pEndpoint( pOrigin->m_pEndpoint ),
  m_nKey( pOrigin->m_nKey ),
  m_oURNs( pOrigin->m_oURNs ),
  m_oKeywordHashList( pOrigin->m_oKeywordHashList )
  //m_oWords()                //! \todo comment this - we copy the search string but not the word list
{
	m_oSHA1		= pOrigin->m_oSHA1;
	m_oTiger	= pOrigin->m_oTiger;
	m_oED2K		= pOrigin->m_oED2K;
	m_oBTH		= pOrigin->m_oBTH;
	m_oMD5		= pOrigin->m_oMD5;
}

CQuerySearch::~CQuerySearch()
{
	if ( m_pXML ) delete m_pXML;
}

auto_ptr< CQuerySearch > CQuerySearch::clone() const
{
	return auto_ptr< CQuerySearch >( new CQuerySearch( this ) );
}

//////////////////////////////////////////////////////////////////////
// CQuerySearch to G1 packet

CG1Packet* CQuerySearch::ToG1Packet(DWORD nTTL)
{
	CG1Packet* pPacket = CG1Packet::New( G1_PACKET_QUERY,
		( nTTL ? min( nTTL, Settings.Gnutella1.SearchTTL ) : Settings.Gnutella1.SearchTTL ),
		m_oGUID );
	
	WORD nFlags = G1_QF_TAG | G1_QF_BIN_HASH | G1_QF_DYNAMIC;
	if ( ! Network.IsListening() ) nFlags |= G1_QF_FIREWALLED;
	if ( m_bWantXML ) nFlags |= G1_QF_XML;
	pPacket->WriteShortLE( nFlags );
	
	CString strExtra;

	if ( !m_sPosKeywords.IsEmpty() )
	{
		if ( Settings.Gnutella1.QuerySearchUTF8 ) //Support UTF-8 Query
		{
			pPacket->WriteStringUTF8( m_sPosKeywords );
		}
		else
		{
			pPacket->WriteString( m_sPosKeywords );
		}
	}
	else if ( m_pSchema != NULL && m_pXML != NULL )
	{
		strExtra = m_pSchema->GetIndexedWords( m_pXML->GetFirstElement() );
		MakeKeywords( strExtra, false );
		pPacket->WriteString( strExtra );
		strExtra.Empty();
	}
	else
	{
		pPacket->WriteByte( 0 );
	}
	
	// Some Gnutella Node does not like forwarding Query containing URN
	/* if ( m_oSHA1 )
	{
		strExtra = m_oSHA1.toUrn();
	}
	else if ( m_oTiger )
	{
		strExtra = m_oTiger.toUrn();
	}
	else if ( m_oED2K )
	{
		strExtra = m_oED2K.toUrn();
	}
	else if ( m_oBTH )
	{
		strExtra = m_oBTH.toUrn();
	}
	else if ( m_oMD5 )
	{
		strExtra = m_oMD5.toUrn();
	}
	else
	{ 
		strExtra = _T("urn:");
	}*/
	
	if ( m_pXML )
	{
		//if ( strExtra.GetLength() ) strExtra += '\x1C';
		pPacket->WriteString( m_pXML->ToString( TRUE ) );
	}
	
	pPacket->WriteByte( 0 );
		
	return pPacket;
}

//////////////////////////////////////////////////////////////////////
// CQuerySearch to G2 packet

CG2Packet* CQuerySearch::ToG2Packet(SOCKADDR_IN* pUDP, DWORD nKey)
{
	CG2Packet* pPacket = CG2Packet::New( G2_PACKET_QUERY, TRUE );
	
	if ( pUDP )
	{
		pPacket->WritePacket( G2_PACKET_UDP, nKey ? 10 : 6 );
		pPacket->WriteLongLE( pUDP->sin_addr.S_un.S_addr );
		pPacket->WriteShortBE( htons( pUDP->sin_port ) );
		if ( nKey ) pPacket->WriteLongBE( nKey );
	}
	else if ( nKey )
	{
		pPacket->WritePacket( G2_PACKET_QKY, 4 );
		pPacket->WriteLongBE( nKey );
	}

	if ( m_oTiger && m_oSHA1 )
	{
        pPacket->WritePacket( G2_PACKET_URN, Hashes::Sha1Hash::byteCount + Hashes::TigerHash::byteCount + 3 );
		pPacket->WriteString( "bp" );
		pPacket->Write( m_oSHA1 );
        pPacket->Write( m_oTiger );
	}
	else if ( m_oSHA1 )
	{
		pPacket->WritePacket( G2_PACKET_URN, Hashes::Sha1Hash::byteCount + 5 );
		pPacket->WriteString( "sha1" );
		pPacket->Write( m_oSHA1 );
	}
	else if ( m_oTiger )
	{
		pPacket->WritePacket( G2_PACKET_URN, Hashes::TigerHash::byteCount + 4 );
		pPacket->WriteString( "ttr" );
		pPacket->Write( m_oTiger );
	}
	// If the target source has only ed2k hash (w/o SHA1) it will allow to find such files
	if ( m_oED2K )
	{
        pPacket->WritePacket( G2_PACKET_URN, Hashes::Ed2kHash::byteCount + 5 );
		pPacket->WriteString( "ed2k" );
		pPacket->Write( m_oED2K );
	}
	
	if ( m_oBTH )
	{
		pPacket->WritePacket( G2_PACKET_URN, Hashes::BtHash::byteCount + 5 );
		pPacket->WriteString( "btih" );
		pPacket->Write( m_oBTH );
	}

	if ( m_oMD5 )
	{
		pPacket->WritePacket( G2_PACKET_URN, Hashes::Md5Hash::byteCount + 4 );
		pPacket->WriteString( "md5" );
		pPacket->Write( m_oMD5 );
	}
	
	if ( !m_sG2Keywords.IsEmpty() )
	{
		short bValue = (short)( 2 * rand() / ( RAND_MAX + 1.0 ) );
		if ( bValue )
		{
			pPacket->WritePacket( G2_PACKET_DESCRIPTIVE_NAME, pPacket->GetStringLen( m_sG2Keywords ) );
			pPacket->WriteString( m_sG2Keywords, FALSE );
		}
		else if ( !m_sSearch.IsEmpty() )
		{
			pPacket->WritePacket( G2_PACKET_DESCRIPTIVE_NAME, pPacket->GetStringLen( m_sSearch ) );
			pPacket->WriteString( m_sSearch, FALSE );
		}
	}
	
	if ( m_pXML != NULL )
	{
		CString strXML;
		if ( CXMLElement* pBody = m_pXML->GetFirstElement() )
			strXML = pBody->ToString();
		pPacket->WritePacket( G2_PACKET_METADATA, pPacket->GetStringLen( strXML ) );
		pPacket->WriteString( strXML, FALSE );
	}
	
	if ( m_nMinSize != 0 || m_nMaxSize != SIZE_UNKNOWN )
	{
		if ( m_nMinSize < 0xFFFFFFFF && ( m_nMaxSize < 0xFFFFFFFF || m_nMaxSize == SIZE_UNKNOWN ) )
		{
			pPacket->WritePacket( G2_PACKET_SIZE_RESTRICTION, 8 );
			pPacket->WriteLongBE( (DWORD)m_nMinSize );
			pPacket->WriteLongBE( m_nMaxSize == SIZE_UNKNOWN ? 0xFFFFFFFF : (DWORD)m_nMaxSize );
		}
		else
		{
			pPacket->WritePacket( G2_PACKET_SIZE_RESTRICTION, 16 );
			pPacket->WriteInt64( m_nMinSize );
			pPacket->WriteInt64( m_nMaxSize );
		}
	}
	
	if ( ! m_bWantURL || ! m_bWantDN || ! m_bWantXML || ! m_bWantCOM || ! m_bWantPFS )
	{
		pPacket->WritePacket( G2_PACKET_INTEREST,
			( m_bWantURL ? 4 : 0 ) + ( m_bWantDN ? 3 : 0 ) + ( m_bWantXML ? 3 : 0 ) +
			( m_bWantCOM ? 4 : 0 ) + ( m_bWantPFS ? 4 : 0 ) );
		
		if ( m_bWantURL ) pPacket->WriteString( "URL" );
		if ( m_bWantDN ) pPacket->WriteString( "DN" );
		if ( m_bWantXML ) pPacket->WriteString( "MD" );
		if ( m_bWantCOM ) pPacket->WriteString( "COM" );
		if ( m_bWantPFS ) pPacket->WriteString( "PFS" );
	}
	
	//if ( m_bAndG1 ) pPacket->WritePacket( G2_PACKET_G1, 0 );
	
	pPacket->WriteByte( 0 );
	pPacket->Write( m_oGUID );
	
	return pPacket;
}

//////////////////////////////////////////////////////////////////////
// CQuerySearch to ED2K packet

CEDPacket* CQuerySearch::ToEDPacket(BOOL bUDP, DWORD nServerFlags)
{
	BOOL bUTF8, bGetS2;

	CEDPacket* pPacket = NULL;
	
	CString strWords = m_pSchema->GetIndexedWords( m_pXML->GetFirstElement() );

	if ( bUDP )
	{
		bUTF8 = nServerFlags & ED2K_SERVER_UDP_UNICODE;
		bGetS2 = nServerFlags & ED2K_SERVER_UDP_GETSOURCES2;
	}
	else
	{
		bUTF8 = nServerFlags & ED2K_SERVER_TCP_UNICODE;
		bGetS2 = nServerFlags & ED2K_SERVER_TCP_GETSOURCES2;
	}
	
	if ( m_oED2K )
	{
		if ( m_bWantDN && Settings.eDonkey.MagnetSearch )
		{			
			// We need the size- do a search by magnet (hash)
			pPacket = CEDPacket::New( bUDP ? ED2K_C2SG_SEARCHREQUEST : ED2K_C2S_SEARCHREQUEST );
			pPacket->WriteByte( 1 );
			pPacket->WriteEDString( _T("magnet:?xt=ed2k:") + m_oED2K.toString(), bUTF8 );
		}
		else
		{			
			// Don't need the size- use GETSOURCES

			// For newer servers, send the file size if it's valid (and not over 4GB)
			if ( ( bGetS2 ) && ( m_nMinSize == m_nMaxSize ) && ( m_nMaxSize < 0xFFFFFFFF ) )
			{
				// theApp.Message( MSG_DEBUG, ( _T("Creating multi-hash capable GetSources2 for: ") + m_oED2K.toString() ) );

				// Newer server, send size as well as hash
				pPacket = CEDPacket::New( bUDP ? ED2K_C2SG_GETSOURCES2 : ED2K_C2S_GETSOURCES );
				// Add the hash/size this packet is for
				pPacket->Write( m_oED2K );
				pPacket->WriteLongLE( (DWORD)m_nMaxSize );
				// Add any other hashes that need to be searched for.
				WriteHashesToEDPacket( pPacket, bUDP );

			}
			else
			{
				// Old style GetSources, with no size attached
				pPacket = CEDPacket::New( bUDP ? ED2K_C2SG_GETSOURCES : ED2K_C2S_GETSOURCES );
				pPacket->Write( m_oED2K );
			}
		}
	}
	else if ( m_oBTH )
	{
		// BitTorrent searches prohibited unless they are GETSOURCES above
	}
	else if ( !m_sKeywords.IsEmpty() && !m_sSearch.IsEmpty() || strWords.GetLength() > 0 )
	{
		pPacket = CEDPacket::New( bUDP ? ED2K_C2SG_SEARCHREQUEST : ED2K_C2S_SEARCHREQUEST );
		
		if ( m_nMinSize > 0 || m_nMaxSize < 0xFFFFFFFF )
		{
			// Add size limits to search (if available)
			pPacket->WriteByte( 0 );		// Boolean AND (min/max) / (name/type)
			pPacket->WriteByte( 0 );
			
			pPacket->WriteByte( 0 );		// Boolean AND (Min/Max)
			pPacket->WriteByte( 0 );
			
			// Size limit (min)
			pPacket->WriteByte( 3 );		
			pPacket->WriteLongLE( (DWORD)m_nMinSize );
			pPacket->WriteByte( 1 );
			pPacket->WriteShortLE( 1 );
			pPacket->WriteByte( ED2K_FT_FILESIZE );
			
			// Size limit (max)
			pPacket->WriteByte( 3 );		
			pPacket->WriteLongLE( (DWORD)min( m_nMaxSize, 0xFFFFFFFF ) );
			pPacket->WriteByte( 2 );
			pPacket->WriteShortLE( 1 );
			pPacket->WriteByte( ED2K_FT_FILESIZE );
		}

		if ( ( m_pSchema == NULL ) || ( ! m_pSchema->m_sDonkeyType.GetLength() ) )
		{	
			// ed2k search without file type
			// Name / Key Words
			pPacket->WriteByte( 1 );		
			// Check if this is a "search for similar files"
			if ( ( m_oSimilarED2K ) && ( ! bUDP ) && ( nServerFlags & ED2K_SERVER_TCP_RELATEDSEARCH ) )
			{
				// This is a search for similar files
				pPacket->WriteEDString( _T( "related::" ) + m_oSimilarED2K.toString(), bUTF8 );
			}
			else
			{
				// Regular search
				pPacket->WriteEDString( !m_sSearch.IsEmpty() ? m_sSearch : strWords, bUTF8 );
			}
		}
		else
		{	
			// ed2k search including file type
			pPacket->WriteByte( 0 );		// Boolean AND (name/type)
			pPacket->WriteByte( 0 );

			// Name / Key Words
			pPacket->WriteByte( 1 );		
			pPacket->WriteEDString( !m_sSearch.IsEmpty() ? m_sSearch : strWords, bUTF8 );

			// Metadata (file type)
			pPacket->WriteByte( 2 );		
			pPacket->WriteEDString( m_pSchema->m_sDonkeyType, bUTF8 );
			pPacket->WriteShortLE( 1 );
			pPacket->WriteByte( ED2K_FT_FILETYPE );
		}
	}
	
	return pPacket;
}

BOOL CQuerySearch::WriteHashesToEDPacket(CEDPacket* pPacket, BOOL bUDP)
{
	ASSERT ( pPacket != NULL );
	ASSERT ( pPacket->m_nType == bUDP ? ED2K_C2SG_GETSOURCES2 : ED2K_C2S_GETSOURCES );

	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return FALSE;

	int nFiles = 1; // There's one hash in the packet to begin with
	DWORD tNow = GetTickCount();

	// Run through all active downloads
	for ( POSITION pos = Downloads.GetIterator() ; pos ; )
	{
		CDownload* pDownload = Downloads.GetNext( pos );
		
		// Basic check
		if ( pDownload->m_oED2K &&					// Must have an ed2k hash
			 pDownload->IsTrying() &&				// Must be active
			 pDownload->m_nSize < 0xFFFFFFFF &&		// Must have a valid size
			 pDownload->IsCompleted() == FALSE &&	// Must not be complete
			 pDownload->NeedHashset() == FALSE &&	// Must have hashset
			 validAndUnequal( pDownload->m_oED2K, m_oED2K ) )// Must not be already added to packet
		{
			// If a download is allowed to ask for more sources
			DWORD tNextQuery = bUDP ? pDownload->m_tLastED2KGlobal + Settings.eDonkey.QueryFileThrottle : pDownload->m_tLastED2KLocal + Settings.eDonkey.QueryFileThrottle;
			if ( tNow > tNextQuery )
			{
				// If we want more sources for this file
				int nSources = pDownload->GetSourceCount( FALSE, TRUE );
				if ( nSources < ( Settings.Downloads.SourcesWanted / 4 ) )
				{
					BOOL bFewSources = nSources < Settings.Downloads.MinSources;
					BOOL bDataStarve = ( tNow > pDownload->m_tReceived ? tNow - pDownload->m_tReceived : 0 ) > Settings.Downloads.StarveTimeout * 1000;

					if ( ( bFewSources ) || ( bDataStarve ) || ( nFiles < 10 ) )
					{
						// Add the hash/size for this download
						pPacket->Write( pDownload->m_oED2K );
						pPacket->WriteLongLE( (DWORD)pDownload->m_nSize );
						if ( bUDP )
							pDownload->m_tLastED2KGlobal = tNow; 
						else
							pDownload->m_tLastED2KLocal = tNow; 
						nFiles ++;
						if ( nFiles >= ED2K_MAXFILESINPACKET ) return TRUE;
					}
				}
			}
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CQuerySearch from packet root

CQuerySearch* CQuerySearch::FromPacket(CPacket* pPacket, SOCKADDR_IN* pEndpoint)
{
	CQuerySearch* pSearch = new CQuerySearch( FALSE );
	
	if ( pPacket->m_nProtocol == PROTOCOL_G1 )
	{
		if ( pSearch->ReadG1Packet( (CG1Packet*)pPacket ) ) return pSearch;
	}
	else if ( pPacket->m_nProtocol == PROTOCOL_G2 )
	{
		if ( ((CG2Packet*)pPacket)->IsType( G2_PACKET_QUERY_WRAP ) )
		{
			//if ( pSearch->ReadG1Packet( (CG1Packet*)pPacket ) ) return pSearch;
			theApp.Message( MSG_DEBUG, _T("CQuerySearch::FromPacket dropping obsolete wrapped packet") );
		}
		else
		{
			if ( pSearch->ReadG2Packet( (CG2Packet*)pPacket, pEndpoint ) ) return pSearch;
		}
	}
	
	delete pSearch;
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CQuerySearch from G1 packet

BOOL CQuerySearch::ReadG1Packet(CPacket* pPacket)
{
	Hashes::Sha1Hash	oSHA1;
	Hashes::TigerHash	oTiger;
	Hashes::Ed2kHash	oED2K;
	Hashes::BtHash		oBTH;
	Hashes::Md5Hash		oMD5;

	m_bWantCOM = m_bWantPFS = FALSE;
	
	if ( pPacket->m_nProtocol == PROTOCOL_G2 )
	{
		GNUTELLAPACKET pG1;
		if ( ! ((CG2Packet*)pPacket)->SeekToWrapped() ) return NULL;
		pPacket->Read( &pG1, sizeof(pG1) );
		m_oGUID = pG1.m_oGUID;
	}
	else
	{
		m_oGUID = ((CG1Packet*)pPacket)->m_oGUID;
	}
	
	if ( pPacket->GetRemaining() < 4 ) return FALSE;
	
	WORD nFlags = pPacket->ReadShortLE();
	
	if ( nFlags & G1_QF_TAG )
	{
		m_bFirewall	= 0 != ( nFlags & G1_QF_FIREWALLED );
		m_bWantXML	= 0 != ( nFlags & G1_QF_XML );
	}
	
	m_sKeywords = m_sSearch = Settings.Gnutella1.QuerySearchUTF8 ?
		pPacket->ReadStringUTF8() : pPacket->ReadStringASCII();

	if ( pPacket->GetRemaining() > 1 )
	{
		const char* pszData = (const char*)pPacket->m_pBuffer + pPacket->m_nPosition;
		for ( int nDataLength = pPacket->GetRemaining() - 1; nDataLength > 0; )
		{
			const char* pszSep = (const char*)memchr( pszData, G1_PACKET_HIT_SEP, nDataLength );
			int nLength = ( pszSep && *pszSep == G1_PACKET_HIT_SEP ) ?
				(int)( pszSep - pszData ) : nDataLength;
			if ( nLength >= 4 && memcmp( pszData, "urn:", 4 ) == 0 )
			{
				CString strURN( pszData, nLength );
				if ( nLength == 4 );					// Got empty urn
				else if ( oSHA1.fromUrn(  strURN ) );	// Got SHA1
				else if ( oTiger.fromUrn( strURN ) );	// Got Tiger
				else if ( oED2K.fromUrn(  strURN ) );	// Got ED2K
				else if ( oBTH.fromUrn(   strURN ) );	// Got BTH
				else if ( oMD5.fromUrn(   strURN ) );	// Got MD5
				else
					theApp.Message( MSG_DEBUG, _T("Got Gnutella query packet with unknown URN: \"%s\""), strURN );
			}
			else if ( nLength >= 4 && *pszData == '<' )
			{
				CString strXML( pszData, nLength );
				ASSERT( m_pXML == NULL );
				m_pXML = CXMLElement::FromString( strXML, FALSE );
				if ( m_pXML == NULL )
				{
					theApp.Message( MSG_DEBUG, _T("Got Gnutella query packet with bad XML: \"%s\""), strXML );
				}
				else
				{
					ASSERT( m_pSchema == NULL );
					m_pSchema = SchemaCache.Get(
						m_pXML->GetAttributeValue( CXMLAttribute::schemaName, NULL ) );
					if ( m_pSchema == NULL )
					{
						// Schemas do not match by URN, get first element to compare
						// with names map of schemas (which are singulars)
						bool bFound = false;
						if ( CXMLElement* pElement = m_pXML->GetFirstElement() )
						{
							m_pSchema = SchemaCache.Guess( pElement->GetName() );
							if ( m_pSchema ) bFound = true;
						}
						else // has no plural envelope
						{
							m_pSchema = SchemaCache.Guess( m_pXML->GetName() );
							if ( m_pSchema ) bFound = true;
						}
						
						if ( !bFound )
							theApp.Message( MSG_DEBUG, _T("Got Gnutella query packet with unknown XML schema: \"%s\""), strXML );
					}
				}
			}
			else if ( *pszData == GGEP_MAGIC )
			{
				if ( Settings.Gnutella1.EnableGGEP )
				{
					CGGEPBlock pGGEP;
					pGGEP.ReadFromBuffer( (LPVOID)pszData, nLength );

					// TODO: Add multiple GGEP items handling

					if ( CGGEPItem* pItem = pGGEP.Find( _T("H"), 21 ) )
					{
						if ( pItem->m_pBuffer[0] == GGEP_H_SHA1 ||
							pItem->m_pBuffer[0] == GGEP_H_BITPRINT )
						{
							m_oSHA1 = reinterpret_cast< Hashes::Sha1Hash::RawStorage& >(
								pItem->m_pBuffer[ 1 ] );
						}
						if ( pItem->m_pBuffer[0] == GGEP_H_BITPRINT &&
							pItem->m_nLength >= 24 + 20 + 1 )
						{
							m_oTiger = reinterpret_cast< Hashes::TigerHash::RawStorage& >(
								pItem->m_pBuffer[ 21 ] );
						}

						// TODO: Add GGEP_H_MD5, GGEP_H_UUID, GGEP_H_MD4 handling

					}
					else if ( CGGEPItem* pItem = pGGEP.Find( _T("u") ) )
					{
						CString strURN( pItem->ToString() );
						if (      oSHA1.fromUrn(  strURN ) );	// Got SHA1
						else if ( oTiger.fromUrn( strURN ) );	// Got Tiger
						else if ( oED2K.fromUrn(  strURN ) );	// Got ED2K
						else if ( oBTH.fromUrn(   strURN ) );	// Got BTH
						else if ( oMD5.fromUrn(   strURN ) );	// Got MD5
						else
							theApp.Message( MSG_DEBUG, _T("Got Gnutella query packet with unknown GGEP URN: \"%s\""), strURN );
					}
				}
				else
					theApp.Message( MSG_DEBUG, _T("Got GGEP Gnutella query packet, but GGEP disabled") );
			}
			else
				theApp.Message( MSG_DEBUG, _T("Got Gnutella query packet with unknown part: \"%hs\""), CString( pszData, nLength ) );

			pszData += nLength + 1;
			nDataLength -= nLength;
		}

		if ( oSHA1  && ! m_oSHA1 )  m_oSHA1  = oSHA1;
		if ( oTiger && ! m_oTiger ) m_oTiger = oTiger;
		if ( oED2K  && ! m_oED2K )  m_oED2K  = oED2K;
		if ( oBTH   && ! m_oBTH )   m_oBTH   = oBTH;
		if ( oMD5   && ! m_oMD5 )   m_oMD5   = oMD5;

		// "Read all"
		pPacket->m_nPosition = pPacket->m_nLength;
	}

	m_bAndG1 = TRUE;

	return CheckValid( false );
}

//////////////////////////////////////////////////////////////////////
// CQuerySearch from G2 packet

BOOL CQuerySearch::ReadG2Packet(CG2Packet* pPacket, SOCKADDR_IN* pEndpoint)
{
	if ( ! pPacket->m_bCompound ) return FALSE;
	
	G2_PACKET nType;
	DWORD nLength;

	m_bAndG1 = FALSE;
	
	while ( pPacket->ReadPacket( nType, nLength ) )
	{
		DWORD nOffset = pPacket->m_nPosition + nLength;
		
		if ( nType == G2_PACKET_QKY && nLength >= 4 )
		{
			if ( m_pEndpoint.sin_addr.S_un.S_addr == 0 && pEndpoint != NULL )
				m_pEndpoint = *pEndpoint;
			m_bUDP = ! Network.IsFirewalledAddress( &m_pEndpoint.sin_addr );
			
			m_nKey = pPacket->ReadLongBE();
			DWORD* pZero = (DWORD*)( pPacket->m_pBuffer + pPacket->m_nPosition - 4 );
			*pZero = 0;
		}
		else if ( nType == G2_PACKET_UDP && nLength >= 6 )
		{
			m_pEndpoint.sin_addr.S_un.S_addr = pPacket->ReadLongLE();
			m_pEndpoint.sin_port = htons( pPacket->ReadShortBE() );
			
			if ( m_pEndpoint.sin_addr.S_un.S_addr == 0 && pEndpoint != NULL )
				m_pEndpoint = *pEndpoint;
			m_bUDP = ! Network.IsFirewalledAddress( &m_pEndpoint.sin_addr );
			if ( m_bUDP ) m_pEndpoint.sin_family = PF_INET;
			
			if ( nLength >= 10 )
			{
				m_nKey = pPacket->ReadLongBE();
				DWORD* pZero = (DWORD*)( pPacket->m_pBuffer + pPacket->m_nPosition - 4 );
				*pZero = 0;
			}
		}
		else if ( nType == G2_PACKET_INTEREST )
		{
			m_bWantURL = m_bWantDN = m_bWantXML = m_bWantCOM = m_bWantPFS = FALSE;
			
			while ( nLength > 0 )
			{
				CString str = pPacket->ReadString( nLength );
				nLength -= str.GetLength() + 1;
				
				if ( str == _T("URL") )			m_bWantURL = TRUE;
				else if ( str == _T("DN") )		m_bWantDN = TRUE;
				else if ( str == _T("SZ") )		m_bWantDN = TRUE;	// Hack
				else if ( str == _T("MD") )		m_bWantXML = TRUE;
				else if ( str == _T("COM") )	m_bWantCOM = TRUE;
				else if ( str == _T("PFS") )	m_bWantPFS = TRUE;
			}
		}
		else if ( nType == G2_PACKET_URN )
		{
			CString strURN = pPacket->ReadString( nLength );
			if ( strURN.GetLength() + 1 >= (int)nLength ) return FALSE;
			nLength -= strURN.GetLength() + 1;
			
			if ( nLength >= 20 && strURN == _T("sha1") )
			{
				pPacket->Read( m_oSHA1 );
			}
			else if ( nLength >= 44 && ( strURN == _T("bp") || strURN == _T("bitprint") ) )
			{
				pPacket->Read( m_oSHA1 );
				pPacket->Read( m_oTiger );
			}
			else if ( nLength >= 24 && ( strURN == _T("ttr") || strURN == _T("tree:tiger/") ) )
			{
				pPacket->Read( m_oTiger );
			}
			else if ( nLength >= 16 && strURN == _T("ed2k") )
			{
				pPacket->Read( m_oED2K );
			}
			else if ( nLength >= 20 && strURN == _T("btih") )
			{
				pPacket->Read( m_oBTH );
			}
			else if ( nLength >= 16 && strURN == _T("md5") )
			{
				pPacket->Read( m_oMD5 );
			}
		}
		else if ( nType == G2_PACKET_DESCRIPTIVE_NAME )
		{
			m_sSearch = pPacket->ReadString( nLength );
			m_sKeywords = m_sSearch;
			// Commenting out below 2 lines. they are called from CheckValid at the end of this function thus
			// no need to call it here.
			//ToLower( m_sKeywords );
			//MakeKeywords( m_sKeywords, false );
		}
		else if ( nType == G2_PACKET_METADATA )
		{
			CString strXML = pPacket->ReadString( nLength );
			
			m_pXML->Delete();
			m_pXML = CXMLElement::FromString( strXML );
			m_pSchema = NULL;
			
			if ( m_pXML != NULL )
			{
				if ( CXMLAttribute *pURI = m_pXML->GetAttribute( CXMLAttribute::schemaName ) )
				{
					m_pSchema = SchemaCache.Get( pURI->GetValue() );
				}
				else if ( m_pSchema = SchemaCache.Guess( m_pXML->GetName() ) )
				{
					CXMLElement* pRoot = m_pSchema->Instantiate( TRUE );
					pRoot->AddElement( m_pXML );
					m_pXML = pRoot;
				}
			}
		}
		else if ( nType == G2_PACKET_SIZE_RESTRICTION )
		{
			if ( nLength == 8 )
			{
				m_nMinSize = pPacket->ReadLongBE();
				m_nMaxSize = pPacket->ReadLongBE();
				if ( m_nMaxSize == 0xFFFFFFFF ) m_nMaxSize = SIZE_UNKNOWN;
			}
			else if ( nLength == 16 )
			{
				m_nMinSize = pPacket->ReadInt64();
				m_nMaxSize = pPacket->ReadInt64();
			}
		}
		else if ( nType == G2_PACKET_G1 )
		{
			m_bAndG1 = TRUE;
		}
		
		pPacket->m_nPosition = nOffset;
	}
	
	if ( pPacket->GetRemaining() < 16 ) return FALSE;
	
	pPacket->Read( m_oGUID );
	
	return CheckValid( true );
}

//////////////////////////////////////////////////////////////////////
// CQuerySearch validity check

BOOL CQuerySearch::CheckValid(bool bExpression)
{
	bool bHashOk = false;

	// Searches by hash are ok
	if ( m_oSHA1 || m_oTiger || m_oED2K || m_oBTH || m_oMD5 )
	{
		if ( m_oURNs.empty() )
		{
			if ( m_oSHA1 )
			{
				CString strurn = m_oSHA1.toUrn();
				m_oURNs.push_back( CQueryHashTable::HashWord( strurn, 0, strurn.GetLength(), 32 ) );
			}

			if ( m_oTiger )
			{
				CString strurn = m_oTiger.toUrn();
				m_oURNs.push_back( CQueryHashTable::HashWord( strurn, 0, strurn.GetLength(), 32 ) );
			}

			if ( m_oED2K )
			{
				CString strurn = m_oED2K.toUrn();
				m_oURNs.push_back( CQueryHashTable::HashWord( strurn, 0, strurn.GetLength(), 32 ) );
			}

			if ( m_oBTH )
			{
				CString strurn = m_oBTH.toUrn();
				m_oURNs.push_back( CQueryHashTable::HashWord( strurn, 0, strurn.GetLength(), 32 ) );
			}

			if ( m_oMD5 )
			{
				CString strurn = m_oMD5.toUrn();
				m_oURNs.push_back( CQueryHashTable::HashWord( strurn, 0, strurn.GetLength(), 32 ) );
			}
		}

		bHashOk = true;
	}

	BuildWordList( bExpression );

	if ( m_oKeywordHashList.size() )
	{
		return TRUE;
	}
	else if ( !m_oWords.empty() )
	{
		// Setting up Common keyword list
		static const LPCTSTR common[] =
		{
			L"mp3", L"ogg", L"ac3",
			L"jpg", L"gif", L"png", L"bmp",
			L"mpg", L"avi", L"mkv", L"wmv", L"mov", L"ogm", L"mpa", L"mpv", L"m2v", L"mp2",
			L"exe", L"zip", L"rar", L"iso", L"bin", L"cue", L"img", L"lzh", L"bz2", L"rpm", L"deb",
			L"dvd", L"mpeg", L"divx", L"xvid", L"mp4",
			L"xxx", L"sex", L"fuck",
			L"torrent"
		};
		static const size_t commonWords = sizeof common / sizeof common[ 0 ];

		bool bExtendChar;	// flag used for extended char
		TCHAR szChar;
		int nLength;
		DWORD nValidWords = 0;
		DWORD nCommonWords = 0;
		size_t nValidCharacters = 0;

		// Check we aren't just searching for broad terms - set counters, etc
		for ( const_iterator pWord = begin(); pWord != end(); pWord++ )
		{
			nValidCharacters = 0;
			szChar = *(pWord->first);
			nLength = int(pWord->second);

			bExtendChar = false;	//  clear the flag used for extended char
			// NOTE: because of how oWord act, each keywords in oWords gets sorted in ascending order with HEX code of char,
			//		thus Extended chars are always located at end of oWords. Which means it is not necessary to Clear the flag
			//		inside the loop.

			if ( !IsCharacter( szChar ) ) // check if the char is valid
			{
				// do nothing here
				continue;
			} //after the char inspection
			else if ( nLength > 3 )	// any char string longer than 3 byte are counted.
			{
				nValidCharacters = nLength;
			}
			else if ( 0x00 <= szChar && 0x7f >= szChar ) // check if the char is 1 byte length in UTF8 (non-char will not reach here)
			{
				nValidCharacters = nLength;
			}
			else if ( 0x80 <= szChar && 0x7ff >= szChar)  // check if the char is 2 byte length in UTF8 (non-char will not reach here)
			{
				nValidCharacters = nLength * 2;
			}
			else if ( 0x3041 <= szChar && 0x30fe >= szChar )	// these region is for Japanese Hiragana/Katakana chars(3Bytes).
			{													// because of number of chars exist in that region, they
																// are counted as 2byte chars to make only 2 or longer chars
																// are accepted on Query.
				nValidCharacters = nLength * 2;
				bExtendChar = true;	// set Extended char flag
			}
			else if ( 0x800 <= szChar && 0xffff >= szChar)  // check if the char is 3 byte length in UTF8 (non-char will not reach here)
			{
				nValidCharacters = nLength * 3;
				bExtendChar = true;	// set Extended char flag
			}
			else if ( nLength > 2 )
			{
				// char inspection
				bool bWord =false;
				bool bDigit =false;
				bool bMix =false;
				IsType(&szChar, 0, nLength, bWord, bDigit, bMix);
				if ( bWord || bMix )
					nValidCharacters = nLength;
			}

			if ( nValidCharacters > 2 ) // if char is longer than 3byte in utf8 (Gnutella standard)
			{
				if ( std::find_if( common, common + commonWords, FindStr( *pWord ) ) != common + commonWords )
					// if the keyword is matched to one of the common keyword set in common[] array.
				{
					// Common term. Don't count it as valid keywords, instead count it as common keywords
					nCommonWords++;
					DWORD nHash = CQueryHashTable::HashWord( pWord->first, 0, pWord->second, 32 );
					m_oKeywordHashList.push_back( nHash );
				}
				else
				{
					// check if it is valid search term.
					// NOTE: code below will filter and narrowing down more. it has to be in one of the condition
					//			1. It is 4byte or longer in UTF8 string(Japanese Hiragana/Katakana are both 3 byte char too 
					//				however they are counted as 2byte char)
					//			2. Query has Schema with it(File type specified)
					//			3. the string contains extended char(3byte length char used in Asia region )
					//if ( nValidCharacters > 3 || m_pSchema != NULL || bExtendChar ) nValidWords++;

					nValidWords++;	// count any 3char or longer as valid keywords
					DWORD nHash = CQueryHashTable::HashWord( pWord->first, 0, pWord->second, 32 );
					m_oKeywordHashList.push_back( nHash );
				}
			}
		}

		if ( m_pSchema != NULL ) // if schema has been selected
		{
			nValidWords += ( nCommonWords > 1 ) ? 1 : 0; // make it accept query, if there are 2 or more different common words.
		}
		else // no schema
		{
			nValidWords += ( nCommonWords > 2 ) ? 1 : 0; // make it accept query, if there are 3 or more different common words.
		}

		if ( nValidWords ) return TRUE;
	}
	
	if ( bHashOk )
	{
		return TRUE;
	}

	m_oKeywordHashList.clear();
	m_oWords.clear();

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CQuerySearch matching

BOOL CQuerySearch::Match(LPCTSTR pszFilename, QWORD nSize, LPCTSTR pszSchemaURI, CXMLElement* pXML, const Hashes::Sha1Hash& oSHA1, const Hashes::TigerHash& oTiger, const Hashes::Ed2kHash& oED2K, const Hashes::BtHash& oBTH, const Hashes::Md5Hash& oMD5)
{
	if ( nSize == SIZE_UNKNOWN || nSize < m_nMinSize || nSize > m_nMaxSize ) return FALSE;
	
	if ( m_oSHA1 )
	{
		return validAndEqual( m_oSHA1, oSHA1 );
	}
	else if ( m_oTiger )
	{
		return validAndEqual( oTiger, m_oTiger );
	}
	else if ( m_oED2K )
	{
		return validAndEqual( oED2K, m_oED2K );
	}
	else if ( m_oBTH )
	{
		return validAndEqual( oBTH, m_oBTH );
	}
	else if ( m_oMD5 )
	{
		return validAndEqual( oMD5, m_oMD5 );
	}
	
	if ( pszSchemaURI && *pszSchemaURI && pXML )
	{
		TRISTATE bResult = MatchMetadata( pszSchemaURI, pXML );
		if ( bResult != TS_UNKNOWN && !Settings.Search.SchemaTypes )
			return ( bResult == TS_TRUE );

		if ( m_sKeywords.GetLength() > 0 )
		{
			bool bReject = false;
			if ( MatchMetadataShallow( pszSchemaURI, pXML, &bReject ) )
			{
				// If searching in Local library return true
				if ( !m_oSHA1 && !m_oTiger && !m_oED2K && !m_oBTH && !m_oMD5 && !m_oSimilarED2K )
					return TRUE;

				// Otherwise, only return WordMatch when negative terms are used
				// to filter out filenames from the search window
				BOOL bNegative = FALSE;
				if ( m_sKeywords.GetLength() > 1 )
				{
					int nMinusPos = -1;
					while ( !bNegative )
					{
						nMinusPos = m_sKeywords.Find( '-', nMinusPos + 1 );
						if ( nMinusPos != -1 )
						{
							bNegative = ( IsCharacter( m_sKeywords.GetAt( nMinusPos + 1 ) ) != 0 );
							if ( nMinusPos > 0 )
								bNegative &= ( IsCharacter( m_sKeywords.GetAt( nMinusPos - 1 ) ) == 0 );
						}
						else break;
					}
				}
				return bNegative ? WordMatch( pszFilename, m_sKeywords ) : TRUE;
			}
			else if ( bReject )
				return FALSE;
		}
	}
	// If it's a search for similar files, the text doesn't have to match
	return m_oSimilarED2K || m_sKeywords.GetLength() && WordMatch( pszFilename, m_sKeywords );
}

TRISTATE CQuerySearch::MatchMetadata(LPCTSTR pszSchemaURI, CXMLElement* pXML)
{
	if ( ! m_pSchema || ! m_pXML ) return TS_UNKNOWN;
	if ( ! pszSchemaURI || ! *pszSchemaURI || ! pXML ) return TS_UNKNOWN;
	if ( ! m_pSchema->CheckURI( pszSchemaURI ) ) return TS_FALSE;
	
	CXMLElement* pRoot = m_pXML->GetFirstElement();
	int nCount = 0;
	
	for ( POSITION pos = m_pSchema->GetMemberIterator() ; pos ; )
	{
		CSchemaMember* pMember = m_pSchema->GetNextMember( pos );
		
		CString strSearch = pMember->GetValueFrom( pRoot );
		CString strTarget = pMember->GetValueFrom( pXML );
		
		if ( strSearch.GetLength() )
		{
			if ( strTarget.GetLength() )
			{
				if ( pMember->m_bNumeric )
				{
					if ( ! NumberMatch( strTarget, strSearch ) ) return TS_FALSE;
				}
				else
				{
					if ( ! WordMatch( strTarget, strSearch ) ) return TS_FALSE;
				}
				
				nCount++;
			}
			else
			{
				return TS_FALSE;
			}
		}
	}
	
	return ( nCount > 0 ) ? TS_TRUE : TS_UNKNOWN;
}

BOOL CQuerySearch::MatchMetadataShallow(LPCTSTR pszSchemaURI, CXMLElement* pXML, bool* bReject)
{
	if ( ! pXML || m_sSearch.IsEmpty() ) return FALSE;
	
	if ( CSchema* pSchema = SchemaCache.Get( pszSchemaURI ) )
	{
		for ( POSITION pos = pSchema->GetMemberIterator() ; pos ; )
		{
			CSchemaMember* pMember = pSchema->GetNextMember( pos );
			
			if ( pMember->m_bSearched )
			{
				CString strTarget = pMember->GetValueFrom( pXML, _T(""), FALSE );
				if ( WordMatch( strTarget, m_sKeywords, bReject ) ) 
					return TRUE;
				else if ( bReject && *bReject )
					return FALSE;
			}
		}
	}
	else
	{
		for ( POSITION pos = pXML->GetAttributeIterator() ; pos ; )
		{
			CXMLAttribute* pAttribute = pXML->GetNextAttribute( pos );

			CString strTarget = pAttribute->GetValue();

			if ( WordMatch( strTarget, m_sKeywords, bReject ) ) 
				return TRUE;
			else if ( bReject && *bReject )
				return FALSE;
		}
	}
	
	return FALSE;
}

BOOL CQuerySearch::WordMatch(LPCTSTR pszString, LPCTSTR pszFind, bool* bReject)
{
	LPCTSTR pszWord	= pszFind;
	LPCTSTR pszPtr	= pszFind;
	BOOL bQuote		= FALSE;
	BOOL bNegate	= FALSE;
	BOOL bSpace		= TRUE;
	int nCount		= 0;
	
	for ( ; *pszPtr ; pszPtr++ )
	{
		if ( ( bQuote && *pszPtr == '\"' ) || ( ! bQuote && ( *pszPtr <= ' ' || *pszPtr == '\t' || *pszPtr == '-' || *pszPtr == '\"' ) ) )
		{
			if ( pszWord < pszPtr )
			{
				if ( bNegate )
				{
					if ( _tcsnistr( pszString, pszWord, pszPtr - pszWord ) ) 
					{
						if ( bReject )
							*bReject = true;
						return FALSE;
					}
				}
				else
				{
					if ( ! _tcsnistr( pszString, pszWord, pszPtr - pszWord ) ) return FALSE;
				}
				
				nCount++;
			}
			
			pszWord	= pszPtr + 1;
			
			if ( *pszPtr == '\"' )
			{
				bQuote = ! bQuote;
				bSpace = TRUE;
			}
			else if ( *pszPtr == '-' && pszPtr[1] != ' ' && bSpace && ! bQuote )
			{
				bNegate = TRUE;
				bSpace = FALSE;
			}
			else
			{
				bSpace = ( *pszPtr == ' ' );
			}
			
			if ( bNegate && ! bQuote && *pszPtr != '-' ) bNegate = FALSE;
		}
		else
		{
			bSpace = FALSE;
		}
	}
	
	if ( pszWord < pszPtr )
	{
		if ( bNegate )
		{
			if ( _tcsnistr( pszString, pszWord, pszPtr - pszWord ) ) 
			{
				if ( bReject )
					*bReject = true;
				return FALSE;
			}
		}
		else
		{
			if ( ! _tcsnistr( pszString, pszWord, pszPtr - pszWord ) ) return FALSE;
		}
		
		nCount++;
	}
	
	return nCount > 0;
}

BOOL CQuerySearch::NumberMatch(const CString& strValue, const CString& strRange)
{
	double nValue, nMinimum, nMaximum = 0;

	if ( _stscanf( strValue, _T("%lf"), &nValue ) != 1 ) return FALSE;

	int nPos = strRange.Find( '-' );

	if ( nPos < 0 )
	{
		return _stscanf( strRange, _T("%lf"), &nMinimum ) == 1 && nValue == nMinimum;
	}
	else if ( nPos == 0 )
	{
		return _stscanf( (LPCTSTR)strRange + 1, _T("%lf"), &nMaximum ) && nValue <= nMaximum;
	}
	else if ( nPos == strRange.GetLength() - 1 )
	{
		return _stscanf( strRange, _T("%lf"), &nMinimum ) && nValue >= nMinimum;
	}
	else
	{
		if ( _stscanf( strRange.Left( nPos ), _T("%lf"), &nMinimum ) != 1 ||
			 _stscanf( strRange.Mid( nPos + 1 ), _T("%lf"), &nMaximum ) != 1 ) return FALSE;
		return nValue >= nMinimum && nValue <= nMaximum;
	}
}

//////////////////////////////////////////////////////////////////////
// CQuerySearch word list builder

void CQuerySearch::BuildWordList(bool bExpression, bool /* bLocal */ )
{
	// clear word tables.
	m_oWords.clear();
	m_oNegWords.clear();

	m_sSearch.Trim();

	ToLower( m_sSearch );

	if ( m_sKeywords.IsEmpty() )
		m_sKeywords = m_sSearch;

	BOOL bHash = ( m_oSHA1 || m_oTiger || m_oED2K || m_oBTH || m_oMD5 ) && m_sSearch.IsEmpty();
	
	if ( 0 == _tcsncmp( m_sSearch, _T("magnet:?"), 8 ) )
	{
		int nURN = m_sSearch.Find( _T("urn:") );
		
		if ( nURN > 0 )
		{
			m_sSearch = m_sSearch.Mid( nURN );
			bHash = TRUE;
		}
	}

	if ( bHash || 0 == _tcsncmp( m_sSearch, _T("urn:"), 4 ) )
	{
		if ( ! m_oSHA1 )
		{
			if ( m_oSHA1.fromUrn( m_sSearch ) )
			{
				bHash = TRUE;
			}
		}
		
		if ( ! m_oTiger )
		{
			if ( m_oTiger.fromUrn( m_sSearch ) )
			{
				bHash = TRUE;
			}
		}

		if ( ! m_oED2K )
		{
			if ( m_oED2K.fromUrn( m_sSearch ) )
			{
				bHash = TRUE;
			}
		}

		if ( ! m_oBTH )
		{
			if ( m_oBTH.fromUrn( m_sSearch ) )
			{
				bHash = TRUE;
			}
		}

		if ( ! m_oMD5 )
		{
			if ( m_oMD5.fromUrn( m_sSearch ) )
			{
				bHash = TRUE;
			}
		}

		if ( bHash ) 
		{
			int nFirstSpace = m_sSearch.Find( _T(" ") );
			if ( nFirstSpace != -1 )
				m_sSearch = m_sSearch.Mid( nFirstSpace + 1 );
			else
				m_sSearch.Empty();
		}
	}

	if ( !bHash )
	{
		MakeKeywords( m_sKeywords, bExpression );
	}

	if ( m_pXML )
	{
		if ( CXMLElement* pXML = m_pXML->GetFirstElement() )
		{
			if ( m_pSchema != NULL )
			{
				for ( POSITION pos = m_pSchema->GetMemberIterator() ; pos ; )
				{
					CSchemaMember* pMember = m_pSchema->GetNextMember( pos );

					if ( pMember->m_bIndexed )
					{
						// quick hack for bitrate problem.
						if ( pMember->m_sName.CompareNoCase( _T("bitrate") ) == 0 )
						{
							// do nothing.
						}
						else if ( CXMLAttribute* pAttribute = pXML->GetAttribute( pMember->m_sName ) )
						{
							ToLower( pAttribute->m_sValue );
							CString strKeywords = pAttribute->m_sValue;
							MakeKeywords( strKeywords, bExpression );
							if ( strKeywords.GetLength() )
								m_sKeywords += L" " + strKeywords;
						}
					}
					else
					{
						if ( CXMLAttribute* pAttribute = pXML->GetAttribute( pMember->m_sName ) )
						{
							ToLower( pAttribute->m_sValue );
							//MakeKeywords( pAttribute->m_sValue, bExpression );
						}
					}
				}
			}
			else
			{
				for ( POSITION pos = pXML->GetAttributeIterator() ; pos ; )
				{
					CXMLAttribute* pAttribute = pXML->GetNextAttribute( pos );
					ToLower( pAttribute->m_sValue );
					CString strKeywords = pAttribute->m_sValue;
					MakeKeywords( strKeywords, bExpression );
					if ( strKeywords.GetLength() )
						m_sKeywords += L" " + strKeywords;
				}
			}
		}
	}

	m_sKeywords.TrimLeft();

	// build word pos/neg tables (m_oWords/m_oNegWords)
	BuildWordTable( m_sKeywords );
	
	// clear QueryStrings.
	m_sPosKeywords.Empty();
	m_sG2Keywords.Empty();

	// create string with positive keywords.
	for ( const_iterator pWord = begin(); pWord != end(); pWord++ )
	{
		m_sPosKeywords.AppendFormat( _T("%s "), LPCTSTR( CString( pWord->first, int(pWord->second) ) ) );
	}

	m_sG2Keywords = m_sPosKeywords;	// copy Positive keywords string to G2 keywords string.
	m_sPosKeywords.TrimRight();		// trim off extra space char at the end of string.

	// append negative keywords to G2 keywords string.
	for ( const_iterator pWord = beginNeg(); pWord != endNeg(); pWord++ )
	{
		m_sG2Keywords.AppendFormat( _T("-%s "), LPCTSTR( CString( pWord->first, int(pWord->second) ) ) );
	}
	m_sG2Keywords.TrimRight();		// trim off extra space char at the end of string.
}

// Function is used to split a phrase in asian languages to separate keywords
// to ease keyword matching, allowing user to type as in the natural language.
// Spacebar key is not a convenient way to separate keywords with IME, and user
// may not know how application is keywording their files.
//
// The function splits katakana, hiragana and CJK phrases out of the input string.
// ToDo: "minus" words and quoted phrases for asian languages may not work correctly in all cases.
void CQuerySearch::MakeKeywords(CString& strPhrase, bool bExpression)
{
	if ( strPhrase.IsEmpty() ) return;

	CString str( L" " );
	LPCTSTR pszPtr = strPhrase;
	ScriptType boundary[ 2 ] = { sNone, sNone };
    int nPos = 0;
	int nPrevWord = 0;
	BOOL bNegative = FALSE;

	for ( ; *pszPtr ; nPos++, pszPtr++ )
	{
		// boundary[ 0 ] -- previous character;
		// boundary[ 1 ] -- current character;
		boundary[ 0 ] = boundary[ 1 ];
		boundary[ 1 ] = sNone;

		if ( IsKanji( *pszPtr ) )
			boundary[ 1 ] = (ScriptType)( boundary[ 1 ] | sKanji);
		if ( IsKatakana( *pszPtr ) )
			boundary[ 1 ] = (ScriptType)( boundary[ 1 ] | sKatakana);
		if ( IsHiragana( *pszPtr ) )
			boundary[ 1 ] = (ScriptType)( boundary[ 1 ] | sHiragana);
		if ( IsCharacter( *pszPtr ) )
			boundary[ 1 ] = (ScriptType)( boundary[ 1 ] | sRegular);
		// for now, disable Numeric Detection in order not to split string like "shareaza2" to "shareaza 2"
		//if ( _istdigit( *pszPtr ) )
		//	boundary[ 1 ] = (ScriptType)( boundary[ 1 ] | sNumeric);

		if ( ( boundary[ 1 ] & (sHiragana | sKatakana) ) == (sHiragana | sKatakana) && ( boundary[ 0 ] & (sHiragana | sKatakana) ) )
		{
			boundary[ 1 ] = boundary[ 0 ];
		}

		bool bCharacter = ( boundary[ 1 ] & sRegular )||
			bExpression && ( *pszPtr == '-' || *pszPtr == '"' );
		if ( *pszPtr == '-' ) bNegative = TRUE;
		else if ( *pszPtr == ' ' ) bNegative = FALSE;

		int nDistance = !bCharacter ? 1 : 0;
		
		if ( !bCharacter || boundary[ 0 ] != boundary[ 1 ] && nPos  )
		{
			if ( nPos > nPrevWord )
			{
				ASSERT( str.GetLength() );
				TCHAR sz = TCHAR( str.Right( 2 ).GetAt( 0 ) );
				if ( boundary[ 0 ] && _tcschr( L" -\"", sz ) != NULL &&
					!_istdigit( TCHAR( str.Right( nPos < 3 ? 1 : 3 ).GetAt( 0 ) ) ) )
				{
					// Join two phrases if the previous was a sigle characters word.
					// idea of joining single characters breaks GDF compatibility completely,
					// but because Shareaza 2.2 and above are not really following GDF about
					// word length limit for ASIAN chars, merging is necessary to be done.
				}
				else if ( str.Right( 1 ) != ' ' && bCharacter )
				{
					if ( ( str.Right( 1 ) != '-' || str.Right( 1 ) != '"' || *pszPtr == '"' ) && 
						( !bNegative || !( boundary[ 0 ] & ( sHiragana | sKatakana | sKanji ) ) ) )
						str.Append( L" " );
				}
				ASSERT( strPhrase.GetLength() > nPos - 1 );
				if ( _tcschr( L"-", strPhrase.GetAt( nPos - 1 ) ) != NULL && nPos > 1 )
				{
					ASSERT( strPhrase.GetLength() > nPos - 2 );
					if ( *pszPtr != ' ' && strPhrase.GetAt( nPos - 2 ) != ' ' )
					{
						nPrevWord += nDistance + 1;
						continue;
					}
					else
					{
						str += strPhrase.Mid( nPrevWord, nPos - nDistance - nPrevWord );
					}
				}
				else
				{
					str += strPhrase.Mid( nPrevWord, nPos - nPrevWord );
					if ( boundary[ 1 ] == sNone && !bCharacter || *pszPtr == ' ' || !bExpression ||
						 ( boundary[ 0 ] & ( sHiragana | sKatakana | sKanji ) && !bNegative ) )
						str.Append( L" " );
				}
			}
			nPrevWord = nPos + nDistance;
		}
	}

	ASSERT( !str.IsEmpty() );
	TCHAR sz = TCHAR( str.Right( 2 ).GetAt( 0 ) );
	if ( boundary[ 0 ] && _tcschr( L" -\"", sz ) != NULL &&
		 boundary[ 1 ] )
	{
		// Join two phrases if the previous was a sigle characters word.
		// idea of joining single characters breaks GDF compatibility completely,
		// but because Shareaza 2.2 and above are not really following GDF about
		// word length limit for ASIAN chars, merging is necessary to be done.
	}
	else if ( str.Right( 1 ) != ' ' && boundary[ 1 ] )
	{
		if ( ( str.Right( 1 ) != '-' || str.Right( 1 ) != '"' ) && !bNegative )
			str.Append( L" " );
	}
	str += strPhrase.Mid( nPrevWord, nPos - nPrevWord );

	strPhrase = str.TrimLeft().TrimRight( L" -" );
	return;
}

// Function makes a set of keywords separated by space
// using a sliding window algorithm to match asian words
void CQuerySearch::SlideKeywords(CString& strPhrase)
{
	if ( strPhrase.GetLength() < 3 ) return;

	CString strTemp;
	LPCTSTR pszPhrase = strPhrase.GetBuffer();
	TCHAR* pszToken = new TCHAR[ 3 ];

	while ( _tcslen( pszPhrase ) )
	{
		_tcsncpy( pszToken, pszPhrase, 2 );
		pszToken[ 2 ] = 0;
		if ( IsKanji( pszToken[ 0 ] ) || 
			 IsKatakana( pszToken[ 0 ] ) || 
			 IsHiragana( pszToken[ 0 ] ) )
		{
			if ( pszToken[ 1 ] != ' ' && _tcslen( pszPhrase ) > 1 )
			{
				strTemp.Append( (LPCTSTR)pszToken );
				strTemp.Append( L" " );
			}
		}
		else
		{
			strTemp += *pszToken;
		}
		pszPhrase++;
	}
	delete [] pszToken;
	strPhrase = strTemp.TrimRight( L" " );
}

void CQuerySearch::BuildWordTable(LPCTSTR pszString)
{
	if ( ! *pszString ) return;

	LPCTSTR pszWord	= pszString;
	LPCTSTR pszPtr	= pszString;
	BOOL bQuote		= FALSE;
	BOOL bNegate	= FALSE;
	BOOL bSpace		= TRUE;
	
	for ( ; *pszPtr ; pszPtr++ )
	{
		if ( IsCharacter( *pszPtr ) )
		{
			bSpace = FALSE;
		}
		else
		{
			if ( pszWord < pszPtr )
			{
				if ( bNegate )
				{
					m_oNegWords.insert( std::make_pair( pszWord, pszPtr - pszWord ) );
				}
				else
				{
					bool bWord = false, bDigit = false, bMix = false;
					IsType( pszWord, 0, pszPtr - pszWord, bWord, bDigit, bMix );
					if ( ( bWord || bMix ) || ( bDigit && pszPtr - pszWord > 3 ) )
						m_oWords.insert( std::make_pair( pszWord, pszPtr - pszWord ) );
				}
			}
			
			pszWord = pszPtr + 1;
			
			if ( *pszPtr == '\"' )
			{
				bQuote = ! bQuote;
				bSpace = TRUE;
			}
			else if ( *pszPtr == '-' && pszPtr[1] != ' ' && bSpace && ! bQuote )
			{
				bNegate = TRUE;
				bSpace = FALSE;
			}
			else
			{
				bSpace = ( *pszPtr == ' ' );
			}
			
			if ( bNegate && ! bQuote && *pszPtr != '-' ) bNegate = FALSE;
		}
	}

	if ( pszWord < pszPtr )
	{
		if ( bNegate )
		{
			m_oNegWords.insert( std::make_pair( pszWord, pszPtr - pszWord ) );
		}
		else
		{
			bool bWord = false, bDigit = false, bMix = false;
			IsType( pszWord, 0, pszPtr - pszWord, bWord, bDigit, bMix );
			if ( ( bWord || bMix ) || ( bDigit && pszPtr - pszWord > 3 ) )
				m_oWords.insert( std::make_pair( pszWord, pszPtr - pszWord ) );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CQuerySearch serialization

void CQuerySearch::Serialize(CArchive& ar)
{
	int nVersion = 7;
	CString strURI;
	
	if ( ar.IsStoring() )
	{
		ar << nVersion;
		
		ar.Write( &m_oGUID[ 0 ], Hashes::Guid::byteCount );
		
		ar << m_sSearch;
		
		SerializeOut( ar, m_oSHA1 );
		SerializeOut( ar, m_oTiger );
		SerializeOut( ar, m_oED2K );
		SerializeOut( ar, m_oBTH );
		SerializeOut( ar, m_oMD5 );
		
		if ( m_pSchema != NULL && m_pXML != NULL )
		{
			ar << m_pSchema->m_sURI;
			m_pXML->Serialize( ar );
		}
		else
		{
			ar << strURI;
		}

		ar << m_bWantURL;
		ar << m_bWantDN;
		ar << m_bWantXML;
		ar << m_bWantCOM;
		ar << m_bWantPFS;
	}
	else
	{
		ar >> nVersion;
		if ( nVersion < 4 ) AfxThrowUserException();
		
		ReadArchive( ar, &m_oGUID[ 0 ], Hashes::Guid::byteCount );
		
		ar >> m_sSearch;
		
		SerializeIn( ar, m_oSHA1, nVersion );
		SerializeIn( ar, m_oTiger, nVersion );
		SerializeIn( ar, m_oED2K, nVersion );
		SerializeIn( ar, m_oBTH, nVersion );

		if ( nVersion >= 7 )
		{
			SerializeIn( ar, m_oMD5, nVersion );
		}

		ar >> strURI;
		
		if ( strURI.GetLength() )
		{
			m_pSchema = SchemaCache.Get( strURI );
			m_pXML = new CXMLElement();
			m_pXML->Serialize( ar );
		}
		
		if ( nVersion >= 5 )
		{
			ar >> m_bWantURL;
			ar >> m_bWantDN;
			ar >> m_bWantXML;
			ar >> m_bWantCOM;
			ar >> m_bWantPFS;
		}
		
		BuildWordList();
	}
}

//////////////////////////////////////////////////////////////////////
// CQuerySearch open window

CSearchWnd* CQuerySearch::OpenWindow(auto_ptr< CQuerySearch > pSearch)
{
	if ( !pSearch.get() ) 
		return NULL;

	pSearch->BuildWordList( false );
	if ( pSearch->CheckValid( false ) )
	{
		return new CSearchWnd( pSearch );
	}
	else
	{
		return NULL;
	}
}

void CQuerySearch::PrepareCheck()
{
	m_oWords.clear();
	m_oURNs.clear();
	m_oKeywordHashList.clear();
	m_sKeywords.Empty();
}

void CQuerySearch::PrepareCheck(CQuerySearch* pQuerySearch)
{
	pQuerySearch->PrepareCheck();
}
