//
// QuerySearch.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CQuerySearch construction

CQuerySearch::CQuerySearch(BOOL bGUID)
{
	if ( bGUID ) Network.CreateID( &m_pGUID );
	
	m_pSchema	= NULL;
	m_pXML		= NULL;
	m_nMinSize	= 0x0000000000000000;
	m_nMaxSize	= 0xFFFFFFFFFFFFFFFF;
	
	m_bSHA1		= FALSE;
	m_bTiger	= FALSE;
	m_bED2K		= FALSE;
	m_bBTH		= FALSE;
	
	m_bWantURL	= TRUE;
	m_bWantDN	= TRUE;
	m_bWantXML	= TRUE;
	m_bWantCOM	= TRUE;
	m_bWantPFS	= TRUE;
	m_bAndG1	= Settings.Gnutella1.EnableToday;
	
	m_bUDP		= FALSE;
	m_nKey		= 0;
	m_bFirewall	= FALSE;
	
	m_nWords	= 0;
	m_pWordPtr	= NULL;
	m_pWordLen	= NULL;
}

CQuerySearch::CQuerySearch(CQuerySearch* pCopy)
{
	m_pGUID		= pCopy->m_pGUID;
	
	m_sSearch	= pCopy->m_sSearch;
	m_pSchema	= pCopy->m_pSchema;
	m_pXML		= pCopy->m_pXML ? pCopy->m_pXML->Clone() : NULL;
	m_nMinSize	= pCopy->m_nMinSize;
	m_nMaxSize	= pCopy->m_nMaxSize;
	
	m_bSHA1 = pCopy->m_bSHA1;
	if ( m_bSHA1 ) m_pSHA1 = pCopy->m_pSHA1;
	m_bTiger = pCopy->m_bTiger;
	if ( m_bTiger ) m_pTiger = pCopy->m_pTiger;
	m_bED2K = pCopy->m_bED2K;
	if ( m_bED2K ) m_pED2K = pCopy->m_pED2K;
	m_bBTH = pCopy->m_bBTH;
	if ( m_bBTH ) m_pBTH = pCopy->m_pBTH;
	
	m_bWantURL	= pCopy->m_bWantURL;
	m_bWantDN	= pCopy->m_bWantDN;
	m_bWantXML	= pCopy->m_bWantXML;
	m_bWantCOM	= pCopy->m_bWantCOM;
	m_bWantPFS	= pCopy->m_bWantPFS;
	m_bAndG1	= pCopy->m_bAndG1;
	
	m_bUDP		= pCopy->m_bUDP;
	m_nKey		= pCopy->m_nKey;
	if ( m_bUDP ) m_pEndpoint = pCopy->m_pEndpoint;
	
	m_nWords	= 0;
	m_pWordPtr	= NULL;
	m_pWordLen	= NULL;
}

CQuerySearch::~CQuerySearch()
{
	if ( m_pXML ) delete m_pXML;
	
	if ( m_pWordPtr ) delete [] m_pWordPtr;
	if ( m_pWordLen ) delete [] m_pWordLen;
}

//////////////////////////////////////////////////////////////////////
// CQuerySearch to G1 packet

CG1Packet* CQuerySearch::ToG1Packet()
{
	CG1Packet* pPacket = CG1Packet::New( G1_PACKET_QUERY,
		min( Settings.Gnutella1.SearchTTL, 4 ), &m_pGUID );
	
	WORD nFlags = G1_QF_TAG | G1_QF_BIN_HASH | G1_QF_DYNAMIC;
	if ( ! Network.IsListening() ) nFlags |= G1_QF_FIREWALLED;
	if ( m_bWantXML ) nFlags |= G1_QF_XML;
	pPacket->WriteShortLE( nFlags );
	
	CString strExtra;
	
	if ( m_sSearch.GetLength() )
	{
		pPacket->WriteString( m_sSearch );
	}
	else if ( m_pSchema != NULL && m_pXML != NULL )
	{
		strExtra = m_pSchema->GetIndexedWords( m_pXML->GetFirstElement() );
		pPacket->WriteString( strExtra );
		strExtra.Empty();
	}
	else
	{
		pPacket->WriteByte( 0 );
	}
	
	if ( m_bSHA1 )
	{
		strExtra = CSHA::HashToString( &m_pSHA1, TRUE );
	}
	else if ( m_bTiger )
	{
		strExtra = CTigerNode::HashToString( &m_pTiger, TRUE );
	}
	else if ( m_bED2K )
	{
		strExtra = CED2K::HashToString( &m_pED2K, TRUE );
	}
	else
	{
		strExtra = _T("urn:");
	}
	
	if ( m_pXML )
	{
		if ( strExtra.GetLength() ) strExtra += '\x1C';
		strExtra += m_pXML->ToString( TRUE );
	}
	
	pPacket->WriteString( strExtra );
	
	return pPacket;
}

//////////////////////////////////////////////////////////////////////
// CQuerySearch to G2 packet

CG2Packet* CQuerySearch::ToG2Packet(SOCKADDR_IN* pUDP, DWORD nKey)
{
	CG2Packet* pPacket = CG2Packet::New( G2_PACKET_QUERY, TRUE );
	
	if ( pUDP )
	{
		pPacket->WritePacket( "UDP", nKey ? 10 : 6 );
		pPacket->WriteLongLE( pUDP->sin_addr.S_un.S_addr );
		pPacket->WriteShortBE( htons( pUDP->sin_port ) );
		if ( nKey ) pPacket->WriteLongBE( nKey );
	}
	
	if ( m_bTiger && m_bSHA1 )
	{
		pPacket->WritePacket( "URN", sizeof(SHA1) + sizeof(TIGEROOT) + 3 );
		pPacket->WriteString( "bp" );
		pPacket->Write( &m_pSHA1, sizeof(SHA1) );
		pPacket->Write( &m_pTiger, sizeof(TIGEROOT) );
	}
	else if ( m_bSHA1 )
	{
		pPacket->WritePacket( "URN", sizeof(SHA1) + 5 );
		pPacket->WriteString( "sha1" );
		pPacket->Write( &m_pSHA1, sizeof(SHA1) );
	}
	else if ( m_bTiger )
	{
		pPacket->WritePacket( "URN", sizeof(TIGEROOT) + 4 );
		pPacket->WriteString( "ttr" );
		pPacket->Write( &m_pTiger, sizeof(TIGEROOT) );
	}
	else if ( m_bED2K )
	{
		pPacket->WritePacket( "URN", sizeof(MD4) + 5 );
		pPacket->WriteString( "ed2k" );
		pPacket->Write( &m_pED2K, sizeof(MD4) );
	}
	
	if ( m_bBTH )
	{
		pPacket->WritePacket( "URN", sizeof(SHA1) + 5 );
		pPacket->WriteString( "btih" );
		pPacket->Write( &m_pBTH, sizeof(SHA1) );
	}
	
	if ( m_sSearch.GetLength() )
	{
		pPacket->WritePacket( "DN", pPacket->GetStringLen( m_sSearch ) );
		pPacket->WriteString( m_sSearch, FALSE );
	}
	
	if ( m_pXML != NULL )
	{
		CString strXML;
		
		if ( true )
		{
			if ( CXMLElement* pBody = m_pXML->GetFirstElement() )
				strXML = pBody->ToString();
		}
		else
		{
			strXML = m_pXML->ToString( TRUE );
		}
		
		pPacket->WritePacket( "MD", pPacket->GetStringLen( strXML ) );
		pPacket->WriteString( strXML, FALSE );
	}
	
	if ( m_nMinSize != 0 || m_nMaxSize != SIZE_UNKNOWN )
	{
		if ( m_nMinSize < 0xFFFFFFFF && ( m_nMaxSize < 0xFFFFFFFF || m_nMaxSize == SIZE_UNKNOWN ) )
		{
			pPacket->WritePacket( "SZR", 8 );
			pPacket->WriteLongBE( (DWORD)m_nMinSize );
			pPacket->WriteLongBE( m_nMaxSize == SIZE_UNKNOWN ? 0xFFFFFFFF : (DWORD)m_nMaxSize );
		}
		else
		{
			pPacket->WritePacket( "SZR", 16 );
			pPacket->WriteInt64( m_nMinSize );
			pPacket->WriteInt64( m_nMaxSize );
		}
	}
	
	if ( ! m_bWantURL || ! m_bWantDN || ! m_bWantXML || ! m_bWantCOM || ! m_bWantPFS )
	{
		pPacket->WritePacket( "I",
			( m_bWantURL ? 4 : 0 ) + ( m_bWantDN ? 3 : 0 ) + ( m_bWantXML ? 3 : 0 ) +
			( m_bWantCOM ? 4 : 0 ) + ( m_bWantPFS ? 4 : 0 ) );
		
		if ( m_bWantURL ) pPacket->WriteString( "URL" );
		if ( m_bWantDN ) pPacket->WriteString( "DN" );
		if ( m_bWantXML ) pPacket->WriteString( "MD" );
		if ( m_bWantCOM ) pPacket->WriteString( "COM" );
		if ( m_bWantPFS ) pPacket->WriteString( "PFS" );
	}
	
	if ( m_bAndG1 ) pPacket->WritePacket( "G1", 0 );
	
	pPacket->WriteByte( 0 );
	pPacket->Write( &m_pGUID, sizeof(GGUID) );
	
	return pPacket;
}

//////////////////////////////////////////////////////////////////////
// CQuerySearch to ED2K packet

CEDPacket* CQuerySearch::ToEDPacket(BOOL bUDP, DWORD nServerFlags)
{
	CEDPacket* pPacket = NULL;
	
	CString strWords = m_pSchema->GetIndexedWords( m_pXML->GetFirstElement() );

	BOOL bUTF8 = ( ! bUDP ) && ( nServerFlags & ED2K_SERVER_TCP_UNICODE );
	
	if ( m_bED2K )
	{
		if( m_bWantDN && Settings.eDonkey.MagnetSearch )
		{			// We need the size- do a search by magnet (hash)
			pPacket = CEDPacket::New( bUDP ? ED2K_C2SG_SEARCHREQUEST : ED2K_C2S_SEARCHREQUEST );
			pPacket->WriteByte( 1 );
			pPacket->WriteEDString( _T("magnet:?xt=ed2k:") + CED2K::HashToString( &m_pED2K ), bUTF8 );
		}
		else
		{			// Don't need the size- Find more sources
			pPacket = CEDPacket::New( bUDP ? ED2K_C2SG_GETSOURCES : ED2K_C2S_GETSOURCES );
			pPacket->Write( &m_pED2K, sizeof(MD4) );
		}
	}
	else if ( m_bBTH )
	{
		// BitTorrent searches prohibited unless they are GETSOURCES above
	}
	else if ( m_sSearch.GetLength() > 0 || strWords.GetLength() > 0 )
	{
		pPacket = CEDPacket::New( bUDP ? ED2K_C2SG_SEARCHREQUEST : ED2K_C2S_SEARCHREQUEST );
		
		if ( m_nMinSize > 0 || m_nMaxSize < SIZE_UNKNOWN )
		{
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
		{	// ed2k search without file type
			// Name / Key Words
			pPacket->WriteByte( 1 );		
			pPacket->WriteEDString( m_sSearch.GetLength() ? m_sSearch : strWords, bUTF8 );
		}
		else
		{	// ed2k search including file type
			pPacket->WriteByte( 0 );		// Boolean AND (name/type)
			pPacket->WriteByte( 0 );

			// Name / Key Words
			pPacket->WriteByte( 1 );		
			pPacket->WriteEDString( m_sSearch.GetLength() ? m_sSearch : strWords, bUTF8 );

			// Metadata (file type)
			pPacket->WriteByte( 2 );		
			pPacket->WriteEDString( m_pSchema->m_sDonkeyType, bUTF8 );
			pPacket->WriteShortLE( 1 );
			pPacket->WriteByte( ED2K_FT_FILETYPE );
		}
	}
	
	return pPacket;
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
			if ( pSearch->ReadG1Packet( (CG1Packet*)pPacket ) ) return pSearch;
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
	CString strData;
	m_bWantCOM = m_bWantPFS = FALSE;
	
	if ( pPacket->m_nProtocol == PROTOCOL_G2 )
	{
		GNUTELLAPACKET pG1;
		if ( ! ((CG2Packet*)pPacket)->SeekToWrapped() ) return NULL;
		pPacket->Read( &pG1, sizeof(pG1) );
		m_pGUID = pG1.m_pGUID;
	}
	else
	{
		m_pGUID = ((CG1Packet*)pPacket)->m_pGUID;
	}
	
	if ( pPacket->GetRemaining() < 4 ) return FALSE;
	
	WORD nFlags = pPacket->ReadShortLE();
	
	if ( nFlags & G1_QF_TAG )
	{
		m_bFirewall	= 0 != ( nFlags & G1_QF_FIREWALLED );
		m_bWantXML	= 0 != ( nFlags & G1_QF_XML );
	}
	
	m_sSearch = pPacket->ReadString();
	
	if ( pPacket->GetRemaining() >= 1 )
	{
		strData = pPacket->ReadString();
		if ( strData.GetLength() > 1024 ) strData.Empty();
	}
	
	LPCTSTR pszData	= strData;
	LPCTSTR pszEnd	= pszData + _tcslen( pszData );
	int nIterations = 0;
	
	while ( *pszData && pszData < pszEnd )
	{
		if ( nIterations++ > 4 ) break;
		
		if ( (BYTE)*pszData == GGEP_MAGIC )
		{
			if ( ! Settings.Gnutella1.EnableGGEP ) break;
			
			CGGEPBlock pGGEP;
			pGGEP.ReadFromString( pszData );
			
			if ( CGGEPItem* pItem = pGGEP.Find( _T("H"), 21 ) )
			{
				if ( pItem->m_pBuffer[0] > 0 && pItem->m_pBuffer[0] < 3 )
				{
					CopyMemory( &m_pSHA1, &pItem->m_pBuffer[1], 20 );
					m_bSHA1 = TRUE;
				}
				if ( pItem->m_pBuffer[0] == 2 && pItem->m_nLength >= 24 + 20 + 1 )
				{
					CopyMemory( &m_pTiger, &pItem->m_pBuffer[21], 24 );
					m_bTiger = TRUE;
				}
			}
			else if ( CGGEPItem* pItem = pGGEP.Find( _T("u") ) )
			{
				strData = pItem->ToString();

				m_bSHA1		|= CSHA::HashFromURN( strData, &m_pSHA1 );
				m_bTiger	|= CTigerNode::HashFromURN( strData, &m_pTiger );
				m_bED2K		|= CED2K::HashFromURN( strData, &m_pED2K );
			}
			
			break;
		}
		
		LPCTSTR pszSep = _tcschr( pszData, 0x1C );
		int nLength = ( pszSep && *pszSep == 0x1C ) ? pszSep - pszData : _tcslen( pszData );
		
		if ( ! _istalnum( *pszData ) ) nLength = 0;
		
		if ( nLength >= 4 && _tcsncmp( pszData, _T("urn:"), 4 ) == 0 )
		{
			m_bSHA1		|= CSHA::HashFromURN( pszData, &m_pSHA1 );
			m_bTiger	|= CTigerNode::HashFromURN( pszData, &m_pTiger );
			m_bED2K		|= CED2K::HashFromURN( pszData, &m_pED2K );
		}
		else if ( nLength > 5 && _tcsncmp( pszData, _T("<?xml"), 5 ) == 0 )
		{
			m_pXML = CXMLElement::FromString( pszData, TRUE );
			
			if ( m_pXML == NULL ) continue;
			
			CString strSchemaURI = m_pXML->GetAttributeValue( CXMLAttribute::schemaName, NULL );
			m_pSchema = SchemaCache.Get( strSchemaURI );
		}
		
		if ( pszSep && *pszSep == 0x1C ) pszData = pszSep + 1;
		else break;
	}
	
	m_bAndG1 = TRUE;
	return CheckValid();
}

//////////////////////////////////////////////////////////////////////
// CQuerySearch from G2 packet

BOOL CQuerySearch::ReadG2Packet(CG2Packet* pPacket, SOCKADDR_IN* pEndpoint)
{
	if ( ! pPacket->m_bCompound ) return FALSE;
	
	CHAR szType[9];
	DWORD nLength;
	
	m_bAndG1 = FALSE;
	
	while ( pPacket->ReadPacket( szType, nLength ) )
	{
		DWORD nOffset = pPacket->m_nPosition + nLength;
		
		if ( strcmp( szType, "QKY" ) == 0 && nLength >= 4 )
		{
			if ( m_pEndpoint.sin_addr.S_un.S_addr == 0 && pEndpoint != NULL )
				m_pEndpoint = *pEndpoint;
			m_bUDP = ! Network.IsFirewalledAddress( &m_pEndpoint.sin_addr );
			
			m_nKey = pPacket->ReadLongBE();
			DWORD* pZero = (DWORD*)( pPacket->m_pBuffer + pPacket->m_nPosition - 4 );
			*pZero = 0;
		}
		else if ( strcmp( szType, "UDP" ) == 0 && nLength >= 6 )
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
		else if ( strcmp( szType, "I" ) == 0 )
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
		else if ( strcmp( szType, "URN" ) == 0 )
		{
			CString strURN = pPacket->ReadString( nLength );
			if ( strURN.GetLength() + 1 >= (int)nLength ) return FALSE;
			nLength -= strURN.GetLength() + 1;
			
			if ( nLength >= 20 && strURN == _T("sha1") )
			{
				m_bSHA1 = TRUE;
				pPacket->Read( &m_pSHA1, sizeof(SHA1) );
			}
			else if ( nLength >= 44 && ( strURN == _T("bp") || strURN == _T("bitprint") ) )
			{
				m_bSHA1 = TRUE;
				pPacket->Read( &m_pSHA1, sizeof(SHA1) );
				m_bTiger = TRUE;
				pPacket->Read( &m_pTiger, sizeof(TIGEROOT) );
			}
			else if ( nLength >= 24 && ( strURN == _T("ttr") || strURN == _T("tree:tiger/") ) )
			{
				m_bTiger = TRUE;
				pPacket->Read( &m_pTiger, sizeof(TIGEROOT) );
			}
			else if ( nLength >= 16 && strURN == _T("ed2k") )
			{
				m_bED2K = TRUE;
				pPacket->Read( &m_pED2K, sizeof(MD4) );
			}
			else if ( nLength >= 20 && strURN == _T("btih") )
			{
				m_bBTH = TRUE;
				pPacket->Read( &m_pBTH, sizeof(SHA1) );
			}
		}
		else if ( strcmp( szType, "DN" ) == 0 )
		{
			m_sSearch = pPacket->ReadString( nLength );
		}
		else if ( strcmp( szType, "MD" ) == 0 )
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
		else if ( strcmp( szType, "SZR" ) == 0 )
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
		else if ( strcmp( szType, "G1" ) == 0 )
		{
			m_bAndG1 = TRUE;
		}
		
		pPacket->m_nPosition = nOffset;
	}
	
	if ( pPacket->GetRemaining() < 16 ) return FALSE;
	
	pPacket->Read( &m_pGUID, sizeof(GGUID) );
	
	return CheckValid();
}

//////////////////////////////////////////////////////////////////////
// CQuerySearch validity check

BOOL CQuerySearch::CheckValid()
{
	BuildWordList();
	
	if ( m_nWords == 1 )
	{
		if ( _tcslen( *m_pWordPtr) < 3 ) return FALSE;

		if ( _tcsicmp( *m_pWordPtr, _T("mp3") ) == 0 ||
			 _tcsicmp( *m_pWordPtr, _T("ogg") ) == 0 ||

			 _tcsicmp( *m_pWordPtr, _T("jpg") ) == 0 ||
			 _tcsicmp( *m_pWordPtr, _T("gif") ) == 0 ||
			 _tcsicmp( *m_pWordPtr, _T("png") ) == 0 ||
			 _tcsicmp( *m_pWordPtr, _T("bmp") ) == 0 ||

			 _tcsicmp( *m_pWordPtr, _T("mpg") ) == 0 ||
			 _tcsicmp( *m_pWordPtr, _T("avi") ) == 0 ||
			 _tcsicmp( *m_pWordPtr, _T("wmv") ) == 0 ||
			 _tcsicmp( *m_pWordPtr, _T("mov") ) == 0 ||
			 _tcsicmp( *m_pWordPtr, _T("ogm") ) == 0 ||

			 _tcsicmp( *m_pWordPtr, _T("dvd") ) == 0 ||
			 _tcsicmp( *m_pWordPtr, _T("mpeg") ) == 0 ||
			 _tcsicmp( *m_pWordPtr, _T("divx") ) == 0 ||
			 _tcsicmp( *m_pWordPtr, _T("xvid") ) == 0 ||

			 _tcsicmp( *m_pWordPtr, _T("torrent") ) == 0 ||

			 _tcsicmp( *m_pWordPtr, _T("xxx") ) == 0 ||
			 _tcsicmp( *m_pWordPtr, _T("sex") ) == 0 ||
			 _tcsicmp( *m_pWordPtr, _T("fuck") ) == 0 )
		{
			return FALSE;
		}
	}
	
	return m_nWords || m_bSHA1 || m_bTiger || m_bED2K || m_bBTH;
}

//////////////////////////////////////////////////////////////////////
// CQuerySearch schema SHA1 to raw SHA1

BOOL CQuerySearch::GetHashFromXML()
{
	if ( ! m_pXML || m_bSHA1 ) return FALSE;
	
	if ( CXMLElement* pBody = m_pXML->GetFirstElement() )
	{
		CString strHash	= pBody->GetAttributeValue( _T("SHA1"), NULL );

		if ( CSHA::HashFromString( strHash, &m_pSHA1 ) )
		{
			CXMLAttribute* pAttribute = pBody->GetAttribute( _T("SHA1") );
			if ( pAttribute ) pAttribute->Delete();
		}
	}
	
	return m_bSHA1;
}

//////////////////////////////////////////////////////////////////////
// CQuerySearch matching

BOOL CQuerySearch::Match(LPCTSTR pszFilename, QWORD nSize, LPCTSTR pszSchemaURI, CXMLElement* pXML, SHA1* pSHA1, TIGEROOT* pTiger, MD4* pED2K)
{
	if ( nSize < m_nMinSize || nSize > m_nMaxSize ) return FALSE;
	
	if ( m_bSHA1 )
	{
		return pSHA1 != NULL && ( m_pSHA1 == *pSHA1 );
	}
	else if ( m_bTiger )
	{
		return pTiger != NULL && ( m_pTiger == *pTiger );
	}
	else if ( m_bED2K )
	{
		return pED2K != NULL && ( memcmp( &m_pED2K, pED2K, sizeof(MD4) ) == 0 );
	}
	
	if ( pszSchemaURI && *pszSchemaURI && pXML )
	{
		TRISTATE bResult = MatchMetadata( pszSchemaURI, pXML );
		if ( bResult != TS_UNKNOWN ) return ( bResult == TS_TRUE );
		if ( m_sSearch.GetLength() > 0 )
		{
			if ( MatchMetadataShallow( pszSchemaURI, pXML ) ) return TRUE;
		}
	}
	return m_sSearch.GetLength() && WordMatch( pszFilename, m_sSearch );
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

BOOL CQuerySearch::MatchMetadataShallow(LPCTSTR pszSchemaURI, CXMLElement* pXML)
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
				if ( WordMatch( strTarget, m_sSearch ) ) return TRUE;
			}
		}
	}
	else
	{
		for ( POSITION pos = pXML->GetAttributeIterator() ; pos ; )
		{
			CXMLAttribute* pAttribute = pXML->GetNextAttribute( pos );

			CString strTarget = pAttribute->GetValue();

			if ( WordMatch( strTarget, m_sSearch ) ) return TRUE;
		}
	}
	
	return FALSE;
}

BOOL CQuerySearch::WordMatch(LPCTSTR pszString, LPCTSTR pszFind)
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
					if ( _tcsnistr( pszString, pszWord, pszPtr - pszWord ) ) return FALSE;
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
			else if ( *pszPtr == '-' && bSpace && ! bQuote )
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
			if ( _tcsnistr( pszString, pszWord, pszPtr - pszWord ) ) return FALSE;
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
	double nValue, nMinimum, nMaximum;

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

void CQuerySearch::BuildWordList()
{
	m_nWords = 0;

	CharLower( m_sSearch.GetBuffer() );
	m_sSearch.ReleaseBuffer();

	AddStringToWordList( m_sSearch );
	
	if ( m_pXML == NULL ) return;
	
	if ( CXMLElement* pXML = m_pXML->GetFirstElement() )
	{
		if ( m_pSchema != NULL )
		{
			for ( POSITION pos = m_pSchema->GetMemberIterator() ; pos ; )
			{
				CSchemaMember* pMember = m_pSchema->GetNextMember( pos );
				
				if ( pMember->m_bIndexed )
				{
					if ( CXMLAttribute* pAttribute = pXML->GetAttribute( pMember->m_sName ) )
					{
						CharLower( pAttribute->m_sValue.GetBuffer() );
						pAttribute->m_sValue.ReleaseBuffer();
						AddStringToWordList( pAttribute->m_sValue );
					}
				}
			}
		}
		else
		{
			for ( POSITION pos = pXML->GetAttributeIterator() ; pos ; )
			{
				CXMLAttribute* pAttribute = pXML->GetNextAttribute( pos );
				CharLower( pAttribute->m_sValue.GetBuffer() );
				pAttribute->m_sValue.ReleaseBuffer();
				AddStringToWordList( pAttribute->m_sValue );
			}
		}
	}
}

void CQuerySearch::AddStringToWordList(LPCTSTR pszString)
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
			if ( ! bNegate && pszWord + 1 < pszPtr && IsWord( pszWord, 0, pszPtr - pszWord ) )
			{
				if ( ( m_nWords & 0x1F ) == 0 )
				{
					LPCTSTR* pWordPtr = new LPCTSTR[ ( m_nWords | 0x1F ) + 1 ];
					DWORD* pWordLen = new DWORD[ ( m_nWords | 0x1F ) + 1 ];
					if ( m_pWordPtr )
					{
						CopyMemory( pWordPtr, m_pWordPtr, 4 * m_nWords );
						CopyMemory( pWordLen, m_pWordLen, 4 * m_nWords );
						delete [] m_pWordPtr;
						delete [] m_pWordLen;
					}
					m_pWordPtr = pWordPtr;
					m_pWordLen = pWordLen;
				}
				
				m_pWordPtr[ m_nWords ] = pszWord;
				m_pWordLen[ m_nWords ] = pszPtr - pszWord;
				m_nWords++;
			}
			
			pszWord = pszPtr + 1;
			
			if ( *pszPtr == '\"' )
			{
				bQuote = ! bQuote;
				bSpace = TRUE;
			}
			else if ( *pszPtr == '-' && bSpace && ! bQuote )
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
	
	if ( ! bNegate && pszWord + 1 < pszPtr && IsWord( pszWord, 0, pszPtr - pszWord ) )
	{
		if ( ( m_nWords & 0x1F ) == 0 )
		{
			LPCTSTR* pWordPtr = new LPCTSTR[ ( m_nWords | 0x1F ) + 1 ];
			DWORD* pWordLen = new DWORD[ ( m_nWords | 0x1F ) + 1 ];
			if ( m_pWordPtr )
			{
				CopyMemory( pWordPtr, m_pWordPtr, 4 * m_nWords );
				CopyMemory( pWordLen, m_pWordLen, 4 * m_nWords );
				delete [] m_pWordPtr;
				delete [] m_pWordLen;
			}
			m_pWordPtr = pWordPtr;
			m_pWordLen = pWordLen;
		}
		
		m_pWordPtr[ m_nWords ] = pszWord;
		m_pWordLen[ m_nWords ] = pszPtr - pszWord;
		m_nWords++;
	}
}

//////////////////////////////////////////////////////////////////////
// CQuerySearch serialization

void CQuerySearch::Serialize(CArchive& ar)
{
	int nVersion = 6;
	CString strURI;
	
	if ( ar.IsStoring() )
	{
		ar << nVersion;
		
		ar.Write( &m_pGUID, sizeof(GGUID) );
		
		ar << m_sSearch;
		
		ar << m_bSHA1;
		if ( m_bSHA1 ) ar.Write( &m_pSHA1, sizeof(SHA1) );
		ar << m_bTiger;
		if ( m_bTiger ) ar.Write( &m_pTiger, sizeof(TIGEROOT) );
		ar << m_bED2K;
		if ( m_bED2K ) ar.Write( &m_pED2K, sizeof(MD4) );
		ar << m_bBTH;
		if ( m_bBTH ) ar.Write( &m_pBTH, sizeof(SHA1) );
		
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
		
		ar.Read( &m_pGUID, sizeof(GGUID) );
		
		ar >> m_sSearch;
		
		ar >> m_bSHA1;
		if ( m_bSHA1 ) ar.Read( &m_pSHA1, sizeof(SHA1) );
		ar >> m_bTiger;
		if ( m_bTiger ) ar.Read( &m_pTiger, sizeof(TIGEROOT) );
		ar >> m_bED2K;
		if ( m_bED2K ) ar.Read( &m_pED2K, sizeof(MD4) );
		if ( nVersion >= 6 ) ar >> m_bBTH;
		if ( m_bBTH ) ar.Read( &m_pBTH, sizeof(SHA1) );
		
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

CSearchWnd* CQuerySearch::OpenWindow()
{
	if ( this == NULL ) return NULL;
	BuildWordList();
	if ( ! CheckValid() ) return NULL;
	return new CSearchWnd( this );
}
