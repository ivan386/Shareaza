//
// QueryHit.cpp
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
#include "QueryHit.h"
#include "Network.h"
#include "G1Packet.h"
#include "G2Packet.h"
#include "EDPacket.h"
#include "Transfer.h"
#include "SchemaCache.h"
#include "Schema.h"
#include "ZLib.h"
#include "XML.h"
#include "GGEP.h"
#include "VendorCache.h"
#include "RouteCache.h"

#include "SHA.h"
#include "TigerTree.h"
#include "ED2K.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CQueryHit construction

CQueryHit::CQueryHit(PROTOCOLID nProtocol, GGUID* pSearchID)
{
	m_pNext = NULL;
	
	if ( pSearchID != NULL )
		m_pSearchID = *pSearchID;
	else
		m_pSearchID = (GGUID&)GUID_NULL;
	
	m_nProtocol		= nProtocol;
	m_pClientID		= (GGUID&)GUID_NULL;
	m_pAddress.S_un.S_addr = 0;
	m_nPort			= 0;
	m_nSpeed		= 0;
	m_pVendor		= VendorCache.m_pNull;
	
	m_bPush			= TS_UNKNOWN;
	m_bBusy			= TS_UNKNOWN;
	m_bStable		= TS_UNKNOWN;
	m_bMeasured		= TS_UNKNOWN;
	m_bChat			= FALSE;
	m_bBrowseHost	= FALSE;
	
	m_nGroup		= 0;
	m_bSHA1			= FALSE;
	m_bTiger		= FALSE;
	m_bED2K			= FALSE;
	m_bBTH			= FALSE;
	m_nIndex		= 0;
	m_bSize			= FALSE;
	m_nSize			= 0;
	m_nSources		= 0;
	m_nPartial		= 0;
	m_bPreview		= FALSE;
	m_nUpSlots		= 0;
	m_nUpQueue		= 0;	
	m_bCollection	= FALSE;
	
	m_pXML			= NULL;
	m_nRating		= 0;
	
	m_bBogus		= FALSE;
	m_bMatched		= FALSE;
	m_bFiltered		= FALSE;
	m_bDownload		= FALSE;
	m_bNew			= FALSE;
	m_bSelected		= FALSE;
	m_bResolveURL	= TRUE;
}

CQueryHit::~CQueryHit()
{
	if ( m_pXML ) delete m_pXML;
}

//////////////////////////////////////////////////////////////////////
// CQueryHit from G1 packet

CQueryHit* CQueryHit::FromPacket(CG1Packet* pPacket, int* pnHops)
{
	CQueryHit* pFirstHit	= NULL;
	CQueryHit* pLastHit		= NULL;
	CXMLElement* pXML		= NULL;
	GGUID pQueryID;
	
	if ( pPacket->m_nProtocol == PROTOCOL_G2 )
	{
		GNUTELLAPACKET pG1;
		if ( ! ((CG2Packet*)pPacket)->SeekToWrapped() ) return NULL;
		pPacket->Read( &pG1, sizeof(pG1) );
		
		pQueryID = pG1.m_pGUID;
		if ( pnHops ) *pnHops = pG1.m_nHops + 1;
	}
	else
	{
		pQueryID = pPacket->m_pGUID;
		if ( pnHops ) *pnHops = pPacket->m_nHops + 1;
	}
	
	try
	{
		BYTE	nCount		= pPacket->ReadByte();
		WORD	nPort		= pPacket->ReadShortLE();
		DWORD	nAddress	= pPacket->ReadLongLE();
		DWORD	nSpeed		= pPacket->ReadLongLE();
		
		if ( ! nCount ) AfxThrowUserException();
		
		while ( nCount-- )
		{
			CQueryHit* pHit = new CQueryHit( PROTOCOL_G1, &pQueryID );
			if ( pFirstHit ) pLastHit->m_pNext = pHit;
			else pFirstHit = pHit;
			pLastHit = pHit;
			
			pHit->m_pAddress	= (IN_ADDR&)nAddress;
			pHit->m_nPort		= nPort;
			pHit->m_nSpeed		= nSpeed;
			
			pHit->ReadG1Packet( pPacket );
		}

		CVendor*	pVendor		= VendorCache.m_pNull;
		BYTE		nPublicSize	= 0;
		BYTE		nFlags[2]	= { 0, 0 };
		WORD		nXMLSize	= 0;
		BOOL		bChat		= FALSE;
		BOOL		bBrowseHost	= FALSE;

		if ( pPacket->GetRemaining() >= 16 + 5 )
		{
			CHAR szaVendor[ 4 ];
			pPacket->Read( szaVendor, 4 );
			TCHAR szVendor[5] = { szaVendor[0], szaVendor[1], szaVendor[2], szaVendor[3], 0 };
			
			pVendor		= VendorCache.Lookup( szVendor );
			nPublicSize	= pPacket->ReadByte();
		}
		
		if ( pVendor->m_bHTMLBrowse ) bBrowseHost = TRUE;
		
		if ( nPublicSize > pPacket->GetRemaining() - 16 ) AfxThrowUserException();
		
		if ( nPublicSize >= 2 )
		{
			nFlags[0]	= pPacket->ReadByte();
			nFlags[1]	= pPacket->ReadByte();
			nPublicSize -= 2;
		}
		
		if ( nPublicSize >= 2 )
		{
			nXMLSize = pPacket->ReadShortLE();
			nPublicSize -= 2;
			if ( nPublicSize + nXMLSize > pPacket->GetRemaining() - 16 ) AfxThrowUserException();
		}
		
		while ( nPublicSize-- ) pPacket->ReadByte();
		
		if ( pVendor->m_bChatFlag && pPacket->GetRemaining() >= 16 + nXMLSize + 1 )
		{
			bChat = pPacket->PeekByte() == 1;
		}
		
		if ( pPacket->GetRemaining() < 16 + nXMLSize ) nXMLSize = 0;
		
		if ( ( nFlags[0] & G1_QHD_GGEP ) && ( nFlags[1] & G1_QHD_GGEP ) &&
			 Settings.Gnutella1.EnableGGEP )
		{
			ReadGGEP( pPacket, &bBrowseHost );
		}
		
		if ( nXMLSize > 0 )
		{
			pPacket->Seek( 16 + nXMLSize, CG1Packet::seekEnd );
			pXML = ReadXML( pPacket, nXMLSize );
		}
		
		if ( ! nPort || Network.IsFirewalledAddress( &nAddress ) )
		{
			nFlags[0] |= G1_QHD_PUSH;
			nFlags[1] |= G1_QHD_PUSH;
		}
		
		GGUID pClientID;
		
		pPacket->Seek( 16, CG1Packet::seekEnd );
		pPacket->Read( &pClientID, sizeof(pClientID) );
		
		DWORD nIndex = 0;
		
		for ( pLastHit = pFirstHit ; pLastHit ; pLastHit = pLastHit->m_pNext, nIndex++ )
		{
			pLastHit->ParseAttributes( &pClientID, pVendor, nFlags, bChat, bBrowseHost );
			pLastHit->Resolve();
			if ( pXML ) pLastHit->ParseXML( pXML, nIndex );
		}
		
		CheckBogus( pFirstHit );
	}
	catch ( CException* pException )
	{
		pException->Delete();
		if ( pXML ) delete pXML;
		if ( pFirstHit ) pFirstHit->Delete();
		return NULL;
	}
	
	if ( pXML ) delete pXML;
	
	return pFirstHit;
}

//////////////////////////////////////////////////////////////////////
// CQueryHit from G2 packet

CQueryHit* CQueryHit::FromPacket(CG2Packet* pPacket, int* pnHops)
{
	if ( pPacket->IsType( G2_PACKET_HIT_WRAP ) )
	{
		return FromPacket( (CG1Packet*)pPacket );
	}
	
	if ( ! pPacket->m_bCompound ) return NULL;
	
	CQueryHit* pFirstHit	= NULL;
	CQueryHit* pLastHit		= NULL;
	CXMLElement* pXML		= NULL;
	
	GGUID		pSearchID;
	GGUID		pClientID, pIncrID;
	BOOL		bClientID	= FALSE;
	
	DWORD		nAddress	= 0;
	WORD		nPort		= 0;
	BOOL		bBusy		= FALSE;
	BOOL		bPush		= FALSE;
	BOOL		bStable		= TRUE;
	BOOL		bBrowseHost	= FALSE;
	BOOL		bPeerChat	= FALSE;
	CVendor*	pVendor		= VendorCache.m_pNull;
	
	CString		strNick;
	DWORD		nGroupState[8][4];
	
	ZeroMemory( nGroupState, sizeof(nGroupState) );
	
	try
	{
		BOOL bCompound;
		CHAR szType[9];
		DWORD nLength;
		
		while ( pPacket->ReadPacket( szType, nLength, &bCompound ) )
		{
			DWORD nSkip = pPacket->m_nPosition + nLength;
			
			if ( bCompound )
			{
				if ( strcmp( szType, "H" ) &&
					 strcmp( szType, "HG" ) &&
					 strcmp( szType, "UPRO" ) )
				{
					pPacket->SkipCompound( nLength );
				}
			}
			
			if ( strcmp( szType, "H" ) == 0 && bCompound )
			{
				CQueryHit* pHit = new CQueryHit( PROTOCOL_G2);
				
				if ( pFirstHit ) pLastHit->m_pNext = pHit;
				else pFirstHit = pHit;
				pLastHit = pHit;
				
				pHit->ReadG2Packet( pPacket, nLength );
			}
			else if ( strcmp( szType, "HG" ) == 0 && bCompound )
			{
				DWORD nQueued = 0, nUploads = 0, nSpeed = 0;
				CHAR szInner[9];
				DWORD nInner;
				
				while ( pPacket->m_nPosition < nSkip && pPacket->ReadPacket( szInner, nInner ) )
				{
					DWORD nSkipInner = pPacket->m_nPosition + nInner;
					
					if ( strcmp( szInner, "SS" ) == 0 && nInner >= 7 )
					{
						nQueued		= pPacket->ReadShortBE();
						nUploads	= pPacket->ReadByte();
						nSpeed		= pPacket->ReadLongBE();
					}
					
					pPacket->m_nPosition = nSkipInner;
				}
				
				if ( pPacket->m_nPosition < nSkip && nSpeed > 0 )
				{
					int nGroup = pPacket->ReadByte();
					
					if ( nGroup >= 0 && nGroup < 8 )
					{
						nGroupState[ nGroup ][0] = TRUE;
						nGroupState[ nGroup ][1] = nQueued;
						nGroupState[ nGroup ][2] = nUploads;
						nGroupState[ nGroup ][3] = nSpeed;
					}
				}
			}
			else if ( strcmp( szType, "NH" ) == 0 && nLength >= 6 )
			{
				SOCKADDR_IN pHub;
				
				pHub.sin_addr.S_un.S_addr	= pPacket->ReadLongLE();
				pHub.sin_port				= htons( pPacket->ReadShortBE() );
				
				pIncrID.n[15] ++;
				Network.NodeRoute->Add( &pIncrID, &pHub );
			}
			else if ( strcmp( szType, "GU" ) == 0 && nLength == 16 )
			{
				pPacket->Read( &pClientID, sizeof(GGUID) );
				bClientID	= TRUE;
				pIncrID		= pClientID;
			}
			else if ( ( strcmp( szType, "NA" ) == 0 || strcmp( szType, "NI" ) == 0 ) && nLength >= 6 )
			{
				nAddress	= pPacket->ReadLongLE();
				nPort		= pPacket->ReadShortBE();
			}
			else if ( strcmp( szType, "V" ) == 0 && nLength >= 4 )
			{
				CString strVendor = pPacket->ReadString( 4 );
				pVendor = VendorCache.Lookup( strVendor );
			}
			else if ( strcmp( szType, "MD" ) == 0 )
			{
				CString strXML	= pPacket->ReadString( nLength );
				LPCTSTR pszXML	= strXML;
				
				while ( pszXML && *pszXML )
				{
					CXMLElement* pPart = CXMLElement::FromString( pszXML, TRUE );
					if ( ! pPart ) break;
					
					if ( ! pXML ) pXML = new CXMLElement( NULL, _T("Metadata") );
					pXML->AddElement( pPart );
					
					pszXML = _tcsstr( pszXML + 1, _T("<?xml") );
				}
			}
			else if ( strcmp( szType, "UPRO" ) == 0 && bCompound )
			{
				CHAR szInner[9];
				DWORD nInner;
				
				while ( pPacket->m_nPosition < nSkip && pPacket->ReadPacket( szInner, nInner ) )
				{
					DWORD nSkipInner = pPacket->m_nPosition + nInner;
					if ( strcmp( szInner, "NICK" ) == 0 )
					{
						strNick = pPacket->ReadString( nInner );
					}
					pPacket->m_nPosition = nSkipInner;
				}
			}
			else if ( strcmp( szType, "BH" ) == 0 )
			{
				bBrowseHost |= 1;
			}
			else if ( strcmp( szType, "BUP" ) == 0 )
			{
				bBrowseHost |= 2;
			}
			else if ( strcmp( szType, "PCH" ) == 0 )
			{
				bPeerChat = TRUE;
			}
			else if ( strcmp( szType, "BUSY" ) == 0 )
			{
				bBusy = TRUE;
			}
			else if ( strcmp( szType, "UNSTA" ) == 0 )
			{
				bStable = FALSE;
			}
			else if ( strcmp( szType, "FW" ) == 0 )
			{
				bPush = TRUE;
			}
			else if ( strcmp( szType, "SS" ) == 0 && nLength > 0 )
			{
				BYTE nStatus = pPacket->ReadByte();
				
				bBusy	= ( nStatus & G2_SS_BUSY ) ? TRUE : FALSE;
				bPush	= ( nStatus & G2_SS_PUSH ) ? TRUE : FALSE;
				bStable	= ( nStatus & G2_SS_STABLE ) ? TRUE : FALSE;
				
				if ( nLength >= 1+4+2+1 )
				{
					nGroupState[0][0] = TRUE;
					nGroupState[0][3] = pPacket->ReadLongBE();
					nGroupState[0][1] = pPacket->ReadShortBE();
					nGroupState[0][2] = pPacket->ReadByte();
				}
			}
			
			pPacket->m_nPosition = nSkip;
		}
		
		if ( ! bClientID ) AfxThrowUserException();
		if ( pPacket->GetRemaining() < 17 ) AfxThrowUserException();
		
		BYTE nHops = pPacket->ReadByte() + 1;
		if ( pnHops ) *pnHops = nHops;
		
		pPacket->Read( &pSearchID, sizeof(GGUID) );
		
		if ( ! bPush ) bPush = ( nPort == 0 || Network.IsFirewalledAddress( &nAddress ) );
		
		DWORD nIndex = 0;
		
		for ( pLastHit = pFirstHit ; pLastHit ; pLastHit = pLastHit->m_pNext, nIndex++ )
		{
			if ( nGroupState[ pLastHit->m_nGroup ][0] == FALSE ) pLastHit->m_nGroup = 0;
			
			pLastHit->m_pSearchID	= pSearchID;
			pLastHit->m_pClientID	= pClientID;
			pLastHit->m_pAddress	= *(IN_ADDR*)&nAddress;
			pLastHit->m_nPort		= nPort;
			pLastHit->m_pVendor		= pVendor;
			pLastHit->m_nSpeed		= nGroupState[ pLastHit->m_nGroup ][3];
			pLastHit->m_bBusy		= bBusy   ? TS_TRUE : TS_FALSE;
			pLastHit->m_bPush		= bPush   ? TS_TRUE : TS_FALSE;
			pLastHit->m_bStable		= bStable ? TS_TRUE : TS_FALSE;
			pLastHit->m_bMeasured	= pLastHit->m_bStable;
			pLastHit->m_nUpSlots	= nGroupState[ pLastHit->m_nGroup ][2];
			pLastHit->m_nUpQueue	= nGroupState[ pLastHit->m_nGroup ][1];
			pLastHit->m_bChat		= bPeerChat;
			pLastHit->m_bBrowseHost	= bBrowseHost;
			pLastHit->m_sNick		= strNick;
			pLastHit->m_bPreview	&= pLastHit->m_bPush == TS_FALSE;
			
			if ( pLastHit->m_nUpSlots > 0 )
			{
				pLastHit->m_bBusy = ( pLastHit->m_nUpSlots <= pLastHit->m_nUpQueue ) ? TS_TRUE : TS_FALSE;
			}
			
			pLastHit->Resolve();
			if ( pXML ) pLastHit->ParseXML( pXML, nIndex );
		}
		
		CheckBogus( pFirstHit );
	}
	catch ( CException* pException )
	{
		pException->Delete();
		if ( pXML ) delete pXML;
		if ( pFirstHit ) pFirstHit->Delete();
		return NULL;
	}
	
	if ( pXML ) delete pXML;
	
	return pFirstHit;
}

//////////////////////////////////////////////////////////////////////
// CQueryHit from ED2K packet

CQueryHit* CQueryHit::FromPacket(CEDPacket* pPacket, SOCKADDR_IN* pServer, GGUID* pSearchID)
{
	CQueryHit* pFirstHit	= NULL;
	CQueryHit* pLastHit		= NULL;
	MD4 pHash;
	
	if ( pPacket->m_nType == ED2K_S2C_SEARCHRESULTS ||
		 pPacket->m_nType == ED2K_S2CG_SEARCHRESULT )
	{
		DWORD nCount = 1;
		
		if ( pPacket->m_nType == ED2K_S2C_SEARCHRESULTS )
		{
			if ( pPacket->GetRemaining() < 4 ) return NULL;
			nCount = pPacket->ReadLongLE();
		}
		
		while ( nCount-- > 0 && pPacket->GetRemaining() >= sizeof(MD4) + 10 )
		{
			CQueryHit* pHit = new CQueryHit( PROTOCOL_ED2K, pSearchID );
			if ( pFirstHit ) pLastHit->m_pNext = pHit;
			else pFirstHit = pHit;
			pLastHit = pHit;

			// Enable chat for ed2k hits
			pHit->m_bChat = TRUE;
			
			pHit->m_pVendor = VendorCache.m_pED2K;
			if ( ! pHit->ReadEDPacket( pPacket, pServer ) ) break;
			pHit->Resolve();

			if( pHit->m_bPush == TS_TRUE )
			{
				//pHit->m_sNick		= _T("(Low ID)");
				pHit->m_nPort		= 0;
			}
		}
	}
	else if (	pPacket->m_nType == ED2K_S2C_FOUNDSOURCES ||
				pPacket->m_nType == ED2K_S2CG_FOUNDSOURCES )
	{
		if ( pPacket->GetRemaining() < sizeof(MD4) + 1 ) return NULL;
		pPacket->Read( &pHash, sizeof(MD4) );
		BYTE nCount = pPacket->ReadByte();
		
		while ( nCount-- > 0 && pPacket->GetRemaining() >= 6 )
		{
			CQueryHit* pHit = new CQueryHit( PROTOCOL_ED2K, pSearchID );
			if ( pFirstHit ) pLastHit->m_pNext = pHit;
			else pFirstHit = pHit;
			pLastHit = pHit;
			
			// Enable chat for ed2k hits
			pHit->m_bChat = TRUE;

			pHit->m_bED2K = TRUE;
			pHit->m_pED2K = pHash;
			pHit->m_pVendor = VendorCache.m_pED2K;
			pHit->ReadEDAddress( pPacket, pServer );
			pHit->Resolve();
		}
	}

	// Enable chat for ed2k hits
	//pFirstHit->m_bChat = TRUE;
	
	// CheckBogus( pFirstHit );
	
	return pFirstHit;
}

//////////////////////////////////////////////////////////////////////
// CQueryHit bogus checking

BOOL CQueryHit::CheckBogus(CQueryHit* pFirstHit)
{
	CString strBase, strTest;
	int nBogus = 0;
	
	if ( pFirstHit == NULL ) return TRUE;
	
	for ( CQueryHit* pHit = pFirstHit->m_pNext ; pHit ; pHit = pHit->m_pNext )
	{
		LPCTSTR pszBase = pFirstHit->m_sName;
		LPCTSTR pszTest = pHit->m_sName;
		
		if ( *pszBase == 0 || *pszTest == 0 ) continue;
		
		BOOL bDots = FALSE;
		BOOL bDiff = FALSE;
		
		while ( TRUE )
		{
			while ( *pszBase && ( *pszBase == '!' || *pszBase == '@' || ! _istgraph( *pszBase ) ) ) pszBase++;
			while ( *pszTest && ( *pszTest == '!' || *pszTest == '@' || ! _istgraph( *pszTest ) ) ) pszTest++;
			
			if ( ! *pszBase || ! *pszTest ) break;
			
			if ( *pszBase == '.' || *pszTest == '.' )
			{
				bDots = TRUE;
				
				if ( *pszBase == '.' && *pszTest == '.' )
				{
					if ( bDiff )
					{
						bDiff = FALSE;
						break;
					}
				}
				else
				{
					bDiff = FALSE;
					break;
				}
			}
			
			if ( ! bDiff && tolower( *pszBase ) != tolower( *pszTest ) ) bDiff = TRUE;
			
			pszBase++;
			pszTest++;
		}
		
		if ( bDots && bDiff )
		{
			if ( ++nBogus >= 2 ) break;
		}
	}
	
	if ( nBogus < 2 ) return FALSE;
	
	for ( pHit = pFirstHit ; pHit ; pHit = pHit->m_pNext )
	{
		pHit->m_bBogus = TRUE;
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CQueryHit XML metadata reader

CXMLElement* CQueryHit::ReadXML(CG1Packet* pPacket, int nSize)
{
	BYTE* pRaw = new BYTE[ nSize ];
	pPacket->Read( pRaw, nSize );
	
	CString strXML;
	
	if ( nSize >= 9 && strncmp( (LPCSTR)pRaw, "{deflate}", 9 ) == 0 )
	{
		BYTE* pText = (BYTE*)CZLib::Decompress( pRaw + 9, nSize - 10, (DWORD*)&nSize );
		
		if ( pText != NULL )
		{
			LPTSTR pOut = strXML.GetBuffer( nSize );
			for ( int nPos = 0 ; nPos < nSize ; nPos++ ) pOut[ nPos ] = (TCHAR)pText[ nPos ];
			strXML.ReleaseBuffer( nSize );
			delete [] pText;
		}
	}
	else if ( nSize >= 11 && strncmp( (LPCSTR)pRaw, "{plaintext}", 11 ) == 0 )
	{
		LPCSTR pszRaw = (LPCSTR)pRaw + 11;
		if ( strlen( pszRaw ) == (DWORD)nSize - 12 ) strXML = pszRaw;
	}
	else
	{
		LPCSTR pszRaw = (LPCSTR)pRaw;
		if ( strlen( pszRaw ) == (DWORD)nSize - 1 ) strXML = pszRaw;
	}
	
	delete [] pRaw;
	
	CXMLElement* pRoot	= NULL;
	LPCTSTR pszXML		= strXML;
	
	while ( pszXML && *pszXML )
	{
		CXMLElement* pXML = CXMLElement::FromString( pszXML, TRUE );
		if ( ! pXML ) break;
		
		if ( ! pRoot ) pRoot = new CXMLElement( NULL, _T("Metadata") );
		pRoot->AddElement( pXML );
		
		pszXML = _tcsstr( pszXML + 1, _T("<?xml") );
	}
	
	return pRoot;
}

//////////////////////////////////////////////////////////////////////
// CQueryHit GGEP reader

BOOL CQueryHit::ReadGGEP(CG1Packet* pPacket, BOOL* pbBrowseHost)
{
	CGGEPBlock pGGEP;
	
	if ( ! pGGEP.ReadFromPacket( pPacket ) ) return FALSE;
	
	if ( pGGEP.Find( _T("BH") ) ) *pbBrowseHost = TRUE;
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CQueryHit G1 result entry reader

void CQueryHit::ReadG1Packet(CG1Packet* pPacket)
{
	CString strData;
	
	m_nIndex	= pPacket->ReadLongLE();
	m_bSize		= TRUE;
	m_nSize		= pPacket->ReadLongLE();
	m_sName		= pPacket->ReadString();
	strData		= pPacket->ReadString();
	
	if ( m_sName.GetLength() > 160 )
	{
		m_bBogus = TRUE;
	}
	else
	{
		int nCurWord = 0, nMaxWord = 0;
		
		for ( LPCTSTR pszName = m_sName ; *pszName ; pszName++ )
		{
			if ( _istgraph( *pszName ) )
			{
				nCurWord++;
				nMaxWord = max( nMaxWord, nCurWord );
			}
			else
			{
				nCurWord = 0;
			}
		}
		
		if ( nMaxWord > 30 ) m_bBogus = TRUE;
	}
	
	LPCTSTR pszData	= strData;
	LPCTSTR pszEnd	= pszData + _tcslen( pszData );
	
	while ( *pszData && pszData < pszEnd )
	{
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
					m_bSHA1 = ! CSHA::IsNull(&m_pSHA1);
				}
				if ( pItem->m_pBuffer[0] == 2 && pItem->m_nLength >= 24 + 20 + 1 )
				{
					CopyMemory( &m_pTiger, &pItem->m_pBuffer[21], 24 );
					m_bTiger = ! CTigerNode::IsNull(&m_pTiger);
				}
			}
			else if ( CGGEPItem* pItem = pGGEP.Find( _T("u"), 5 + 32 ) )
			{
				strData = pItem->ToString();
				
				m_bSHA1		|= CSHA::HashFromURN( strData, &m_pSHA1 );
				m_bTiger	|= CTigerNode::HashFromURN( strData, &m_pTiger );
				m_bED2K		|= CED2K::HashFromURN( strData, &m_pED2K );
			}
			
			if ( CGGEPItem* pItem = pGGEP.Find( _T("ALT"), 6 ) )
			{
				// the ip-addresses need not be stored, as they are sent upon the download request in the ALT-loc header
				m_nSources = pItem->m_nLength / 6;	// 6 bytes per source (see ALT GGEP extension specification)
			}
			
			break;
		}
		
		LPCTSTR pszSep = _tcschr( pszData, 0x1C );
		int nLength = pszSep ? pszSep - pszData : _tcslen( pszData );
		
		if ( _tcsnicmp( pszData, _T("urn:"), 4 ) == 0 )
		{
			m_bSHA1		|= CSHA::HashFromURN( pszData, &m_pSHA1 );
			m_bTiger	|= CTigerNode::HashFromURN( pszData, &m_pTiger );
			m_bED2K		|= CED2K::HashFromURN( pszData, &m_pED2K );
		}
		else if ( nLength > 4 )
		{
			AutoDetectSchema( pszData );
		}
		
		if ( pszSep ) pszData = pszSep + 1;
		else break;
	}
}

//////////////////////////////////////////////////////////////////////
// CQueryHit G1 attributes suffix

void CQueryHit::ParseAttributes(GGUID* pClientID, CVendor* pVendor, BYTE* nFlags, BOOL bChat, BOOL bBrowseHost)
{
	m_pClientID		= *pClientID;
	m_pVendor		= pVendor;
	m_bChat			= bChat;
	m_bBrowseHost	= bBrowseHost;
	
	if ( nFlags[1] & G1_QHD_PUSH )
		m_bPush		= ( nFlags[0] & G1_QHD_PUSH ) ? TS_TRUE : TS_FALSE;
	if ( nFlags[0] & G1_QHD_BUSY )
		m_bBusy		= ( nFlags[1] & G1_QHD_BUSY ) ? TS_TRUE : TS_FALSE;
	if ( nFlags[0] & G1_QHD_STABLE )
		m_bStable	= ( nFlags[1] & G1_QHD_STABLE ) ? TS_TRUE : TS_FALSE;
	if ( nFlags[0] & G1_QHD_SPEED )
		m_bMeasured	= ( nFlags[1] & G1_QHD_SPEED ) ? TS_TRUE : TS_FALSE;
}

//////////////////////////////////////////////////////////////////////
// CQueryHit G2 result entry reader

void CQueryHit::ReadG2Packet(CG2Packet* pPacket, DWORD nLength)
{
	DWORD nPacket, nEnd = pPacket->m_nPosition + nLength;
	CHAR szType[9];
	
	m_bResolveURL = FALSE;
	
	while ( pPacket->m_nPosition < nEnd && pPacket->ReadPacket( szType, nPacket ) )
	{
		DWORD nSkip = pPacket->m_nPosition + nPacket;
		
		if ( strcmp( szType, "URN" ) == 0 )
		{
			CString strURN = pPacket->ReadString( nPacket );
			if ( strURN.GetLength() + 1 >= (int)nPacket ) AfxThrowUserException();
			nPacket -= strURN.GetLength() + 1;
			
			if ( nPacket >= 20 && strURN == _T("sha1") )
			{
				pPacket->Read( &m_pSHA1, sizeof(SHA1) );
				m_bSHA1 = ! CSHA::IsNull(&m_pSHA1);
			}
			else if ( nPacket >= 44 && ( strURN == _T("bp") || strURN == _T("bitprint") ) )
			{
				pPacket->Read( &m_pSHA1, sizeof(SHA1) );
				m_bSHA1 = ! CSHA::IsNull(&m_pSHA1);

				pPacket->Read( &m_pTiger, sizeof(TIGEROOT) );
				m_bTiger = ! CTigerNode::IsNull(&m_pTiger);
			}
			else if ( nPacket >= 24 && ( strURN == _T("ttr") || strURN == _T("tree:tiger/") ) )
			{
				pPacket->Read( &m_pTiger, sizeof(TIGEROOT) );
				m_bTiger = ! CTigerNode::IsNull(&m_pTiger);
			}
			else if ( nPacket >= 16 && strURN == _T("ed2k") )
			{
				m_bED2K = TRUE;
				pPacket->Read( &m_pED2K, sizeof(MD4) );
			}
			else if ( nPacket >= 20 && strURN == _T("btih") )
			{
				m_bBTH = TRUE;
				pPacket->Read( &m_pBTH, sizeof(SHA1) );
			}
		}
		else if ( strcmp( szType, "URL" ) == 0 )
		{
			if ( nPacket == 0 )
			{
				m_bResolveURL = TRUE;
			}
			else
			{
				m_sURL = pPacket->ReadString( nPacket );
			}
		}
		else if ( strcmp( szType, "DN" ) == 0 )
		{
			if ( m_bSize )
			{
				m_sName = pPacket->ReadString( nPacket );
			}
			else if ( nPacket > 4 )
			{
				m_bSize	= TRUE;
				m_nSize = pPacket->ReadLongBE();
				m_sName = pPacket->ReadString( nPacket - 4 );
			}
		}
		else if ( strcmp( szType, "MD" ) == 0 && nPacket > 0 )
		{
			CString strXML = pPacket->ReadString( nPacket );
			
			if ( CXMLElement* pXML = CXMLElement::FromString( strXML ) )
			{
				if ( CXMLAttribute* pURI = pXML->GetAttribute( CXMLAttribute::schemaName ) )
				{
					m_sSchemaPlural		= pXML->GetName();
					m_sSchemaURI		= pURI->GetValue();
					CXMLElement* pChild	= pXML->GetFirstElement();
					
					if ( pChild != NULL && m_sSchemaPlural.GetLength() > 0 && m_sSchemaURI.GetLength() > 0 )
					{
						m_pXML->Delete();
						m_pXML = pChild->Detach();
					}
				}
				else if ( CSchema* pSchema = SchemaCache.Guess( pXML->GetName() ) )
				{
					m_pXML->Delete();
					m_sSchemaPlural	= pSchema->m_sPlural;
					m_sSchemaURI	= pSchema->m_sURI;
					m_pXML			= pXML;
					pXML			= NULL;
				}
				
				pXML->Delete();
			}
		}
		else if ( strcmp( szType, "SZ" ) == 0 )
		{
			if ( nPacket == 4 )
			{
				m_bSize	= TRUE;
				m_nSize = pPacket->ReadLongBE();
			}
			else if ( nPacket == 8 )
			{
				m_bSize	= TRUE;
				m_nSize = pPacket->ReadInt64();
			}
		}
		else if ( strcmp( szType, "G" ) == 0 && nPacket >= 1 )
		{
			m_nGroup = pPacket->ReadByte();
			if ( m_nGroup < 0 ) m_nGroup = 0;
			if ( m_nGroup > 7 ) m_nGroup = 7;
		}
		else if ( strcmp( szType, "ID" ) == 0 && nPacket >= 4 )
		{
			m_nIndex = pPacket->ReadLongBE();
		}
		else if ( strcmp( szType, "CSC" ) == 0 && nPacket >= 2 )
		{
			m_nSources = pPacket->ReadShortBE();
		}
		else if ( strcmp( szType, "PART" ) == 0 && nPacket >= 4 )
		{
			m_nPartial = pPacket->ReadLongBE();
		}
		else if ( strcmp( szType, "COM" ) == 0 )
		{
			CString strXML = pPacket->ReadString( nPacket );
			
			if ( CXMLElement* pComment = CXMLElement::FromString( strXML ) )
			{
				m_nRating = -1;
				_stscanf( pComment->GetAttributeValue( _T("rating") ), _T("%i"), &m_nRating );
				m_nRating = max( 0, min( 6, m_nRating + 1 ) );
				m_sComments = pComment->GetValue();
				Replace( m_sComments, _T("{n}"), _T("\r\n") );
				delete pComment;
			}
		}
		else if ( strcmp( szType, "PVU" ) == 0 )
		{
			if ( nPacket == 0 )
			{
				m_bPreview = TRUE;
			}
			else
			{
				m_bPreview = TRUE;
				m_sPreview = pPacket->ReadString( nPacket );
			}
		}
		else if ( strcmp( szType, "BOGUS" ) == 0 )
		{
			m_bBogus = TRUE;
		}
		else if ( strcmp( szType, "COLLECT" ) == 0 )
		{
			m_bCollection = TRUE;
		}
		
		pPacket->m_nPosition = nSkip;
	}
	
	if ( ! m_bSHA1 && ! m_bTiger && ! m_bED2K && ! m_bBTH ) AfxThrowUserException();
}

//////////////////////////////////////////////////////////////////////
// CQueryHit ED2K result entry reader

BOOL CQueryHit::ReadEDPacket(CEDPacket* pPacket, SOCKADDR_IN* pServer)
{
	CString strLength(_T("")), strBitrate(_T("")), strCodec(_T(""));
	DWORD nLength = 0;
	m_bED2K = TRUE;
	pPacket->Read( &m_pED2K, sizeof(MD4) );
	
	ReadEDAddress( pPacket, pServer );
	
	DWORD nTags = pPacket->ReadLongLE();
	
	while ( nTags-- > 0 )
	{
		if ( pPacket->GetRemaining() < 1 ) return FALSE;

		CEDTag pTag;
		if ( ! pTag.Read( pPacket ) ) 
		{
			theApp.Message( MSG_ERROR, _T("ED2K search result packet read error") ); //debug check
			return FALSE;
		}
	
		if ( pTag.m_nKey == ED2K_FT_FILENAME )
		{
			m_sName = pTag.m_sValue;
		}
		else if ( pTag.m_nKey == ED2K_FT_FILESIZE )
		{
			m_bSize = TRUE;
			m_nSize = pTag.m_nValue;
		}
		else if ( pTag.m_nKey == ED2K_FT_LASTSEENCOMPLETE )
		{
			//theApp.Message( MSG_SYSTEM,_T("Last seen complete"));
		}
		else if ( pTag.m_nKey == ED2K_FT_SOURCES )
		{
			m_nSources = pTag.m_nValue;
			if ( m_nSources == 0 )
				m_bResolveURL = FALSE;
			else
				m_nSources--;
		}
		else if ( pTag.m_nKey == ED2K_FT_COMPLETESOURCES )
		{
			if ( ! pTag.m_nValue ) //If there are no complete sources
			{
				//Assume this file is 50% complete. (we can't tell yet, but at least this will warn the user)
				m_nPartial = (DWORD)m_nSize >> 2;
				//theApp.Message( MSG_SYSTEM, _T("ED2K_FT_COMPLETESOURCES tag reports no complete sources.") );				
			}
			else
			{
				//theApp.Message( MSG_SYSTEM, _T("ED2K_FT_COMPLETESOURCES tag reports complete sources present.") );
			}
		}
		else if ( pTag.m_nKey == ED2K_FT_LENGTH )
		{	//Length- new style (DWORD)
			nLength = pTag.m_nValue;	
		}
		else if ( ( pTag.m_nKey == ED2K_FT_BITRATE ) )
		{	//Bitrate- new style
			strBitrate.Format( _T("%lu"), pTag.m_nValue );
		}
		else if  ( ( pTag.m_nKey == ED2K_FT_CODEC ) )
		{	//Codec - new style
			strCodec = pTag.m_sValue;
		}
		//Note: Maybe ignore these keys? They seem to have a lot of bad values....
		else if ( ( pTag.m_nKey == 0 ) && ( pTag.m_nType == ED2K_TAG_STRING ) && ( pTag.m_sKey == _T("length") )  )
		{	//Length- old style (As a string- x:x:x, x:x or x)
			DWORD nSecs = 0, nMins = 0, nHours = 0;

			if ( pTag.m_sValue.GetLength() < 3 )
			{
				_stscanf( pTag.m_sValue, _T("%i"), &nSecs );
			}
			else if ( pTag.m_sValue.GetLength() < 6 )
			{
				_stscanf( pTag.m_sValue, _T("%i:%i"), &nMins, &nSecs );
			}
			else 
			{
				_stscanf( pTag.m_sValue, _T("%i:%i:%i"), &nHours, &nMins, &nSecs );
			}

			nLength = (nHours * 60 * 60) + (nMins * 60) + (nSecs);
		}
		else if ( ( pTag.m_nKey == 0 ) && ( pTag.m_nType == ED2K_TAG_INT ) && ( pTag.m_sKey == _T("bitrate") ) )
		{	//Bitrate- old style			
			strBitrate.Format( _T("%lu"), pTag.m_nValue );
		}
		else if ( ( pTag.m_nKey == 0 ) && ( pTag.m_nType == ED2K_TAG_STRING ) && ( pTag.m_sKey == _T("codec") ) )
		{	//Codec - old style
			strCodec = pTag.m_sValue;
		}
		else//*** Debug check. Remove this when it's working
		{
			CString s;
			s.Format ( _T("Tag: %u sTag: %s Type: %u"), pTag.m_nKey, pTag.m_sKey, pTag.m_nType );
			theApp.Message( MSG_SYSTEM, s );

			if ( pTag.m_nType == 2 )
				s.Format ( _T("Value: %s"), pTag.m_sValue);
			else
				s.Format ( _T("Value: %d"), pTag.m_nValue);
			theApp.Message( MSG_SYSTEM, s );

		}
	}

	// Verify and set metadata
	CString strType;

	// Check we have a valid name
	if ( m_sName.GetLength() )
	{
		int nExtPos = m_sName.ReverseFind( '.' );
		if ( nExtPos > 0 ) 
		{
			strType = m_sName.Mid( nExtPos );
			strType = CharLower( strType.GetBuffer() );
		}
	}

	// If we can determine type, we can add metadata
	if ( strType.GetLength() )
	{
		// Determine type
		// Note: Maybe should use library plug in for this?
		if ( strType == _T(".mp3") ||  strType == _T(".ogg") ||  strType == _T(".wav") ||  strType == _T(".mid") ||
			 strType == _T(".ape") || strType == _T(".mac") || strType == _T(".apl") || strType == _T(".ra"))
		{	// Audio
			m_sSchemaURI = CSchema::uriAudio;

			// Add metadata
			if ( nLength > 0 )
			{
				strLength.Format( _T("%lu"), nLength );
				if ( m_pXML == NULL ) m_pXML = new CXMLElement( NULL, _T("audio") );
				m_pXML->AddAttribute( _T("seconds"), strLength );
			}
			if ( strBitrate.GetLength() )
			{
				if ( m_pXML == NULL ) m_pXML = new CXMLElement( NULL, _T("audio") );
				m_pXML->AddAttribute( _T("bitrate"), strBitrate );
			}/*
			if ( strCodec.GetLength() )
			{
				m_sSchemaURI = CSchema::uriVideo;
				if ( m_pXML == NULL ) m_pXML = new CXMLElement( NULL, _T("audio") );
				m_pXML->AddAttribute( _T("codec"), strCodec );
			}*/
		}
		else if ( strType == _T(".avi") || strType == _T(".mpg") || strType == _T(".mpeg") || strType == _T(".ogm") || strType == _T(".mkv") ||
				  strType == _T(".asf") || strType == _T(".wma") || strType == _T(".wmv") || strType == _T(".rm") )
		{	// Video
			m_sSchemaURI = CSchema::uriVideo;
			
			// Add metadata
			if ( nLength > 0 )
			{
				double nMins = (double)nLength / (double)60;	
				strLength.Format( _T("%.3f"), nMins );
				if ( m_pXML == NULL ) m_pXML = new CXMLElement( NULL, _T("video") );
				m_pXML->AddAttribute( _T("minutes"), strLength );
			}/*
			if ( strBitrate.GetLength() )
			{
				if ( m_pXML == NULL ) m_pXML = new CXMLElement( NULL, _T("video") );
				m_pXML->AddAttribute( _T("bitrate"), strBitrate );
			}*/
			if ( strCodec.GetLength() )
			{
				if ( m_pXML == NULL ) m_pXML = new CXMLElement( NULL, _T("video") );
				m_pXML->AddAttribute( _T("codec"), strCodec );
			}
		}
		else if ( strType == _T(".exe") || strType == _T(".dll") || strType == _T(".iso") || strType == _T(".bin") || strType == _T(".cue") )
		{	// Application
			m_sSchemaURI = CSchema::uriApplication;
		}
		else if ( strType == _T(".jpg") || strType == _T(".jpeg") || strType == _T(".gif") || strType == _T(".png") || strType == _T(".bmp") )
		{	// Image
			m_sSchemaURI = CSchema::uriImage;
		}
		else if ( strType == _T(".pdf") || strType == _T(".doc") || strType == _T(".txt") || strType == _T(".xls") )
		{	// Document
			m_sSchemaURI = CSchema::uriBook;
		}
	}
	
	return TRUE;
}

void CQueryHit::ReadEDAddress(CEDPacket* pPacket, SOCKADDR_IN* pServer)
{
	DWORD nAddress = m_pAddress.S_un.S_addr = pPacket->ReadLongLE();
	m_nPort = pPacket->ReadShortLE();
	
	m_pClientID.w[0] = pServer->sin_addr.S_un.S_addr;
	m_pClientID.w[1] = htons( pServer->sin_port );
	m_pClientID.w[2] = nAddress;
	m_pClientID.w[3] = m_nPort;
	
	if ( nAddress == 0 )
	{
		m_bResolveURL = FALSE;
		m_bPush = TS_UNKNOWN;
	}
	else if ( CEDPacket::IsLowID( nAddress ) || Network.IsFirewalledAddress( &nAddress ) || ! m_nPort )
	{
		m_bPush = TS_TRUE;
	}
	else
	{
		m_bPush = TS_FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// CQueryHit resolve

void CQueryHit::Resolve()
{
	if ( m_bPreview && m_bSHA1 && m_sPreview.IsEmpty() )
	{
		m_sPreview.Format( _T("http://%s:%i/gnutella/preview/v1?%s"),
			(LPCTSTR)CString( inet_ntoa( m_pAddress ) ), m_nPort,
			(LPCTSTR)CSHA::HashToString( &m_pSHA1, TRUE ) );
	}
	
	if ( m_sURL.GetLength() )
	{
		m_nSources ++;
		return;
	}
	else if ( ! m_bResolveURL )
		return;
	
	m_nSources++;
	
	if ( m_nProtocol == PROTOCOL_ED2K )
	{
		if ( m_bPush == TS_TRUE )
		{
			m_sURL.Format( _T("ed2kftp://%lu@%s:%i/%s/%I64i/"),
				m_pClientID.w[2],
				(LPCTSTR)CString( inet_ntoa( (IN_ADDR&)m_pClientID.w[0] ) ),
				m_pClientID.w[1],
				(LPCTSTR)CED2K::HashToString( &m_pED2K ), m_nSize );
		}
		else
		{
			m_sURL.Format( _T("ed2kftp://%s:%i/%s/%I64i/"),
				(LPCTSTR)CString( inet_ntoa( m_pAddress ) ), m_nPort,
				(LPCTSTR)CED2K::HashToString( &m_pED2K ), m_nSize );
		}
		return;
	}
	
	if ( m_nIndex == 0 || m_bTiger || m_bED2K || Settings.Downloads.RequestHash )
	{
		if ( m_bSHA1 )
		{
			m_sURL.Format( _T("http://%s:%i/uri-res/N2R?%s"),
				(LPCTSTR)CString( inet_ntoa( m_pAddress ) ), m_nPort,
				(LPCTSTR)CSHA::HashToString( &m_pSHA1, TRUE ) );
			return;
		}
		else if ( m_bTiger )
		{
			m_sURL.Format( _T("http://%s:%i/uri-res/N2R?%s"),
				(LPCTSTR)CString( inet_ntoa( m_pAddress ) ), m_nPort,
				(LPCTSTR)CTigerNode::HashToString( &m_pTiger, TRUE ) );
			return;
		}
		else if ( m_bED2K )
		{
			m_sURL.Format( _T("http://%s:%i/uri-res/N2R?%s"),
				(LPCTSTR)CString( inet_ntoa( m_pAddress ) ), m_nPort,
				(LPCTSTR)CED2K::HashToString( &m_pED2K, TRUE ) );
			return;
		}
	}
	
	if ( Settings.Downloads.RequestURLENC )
	{
		m_sURL.Format( _T("http://%s:%i/get/%lu/%s"),
			(LPCTSTR)CString( inet_ntoa( m_pAddress ) ), m_nPort, m_nIndex,
			(LPCTSTR)CTransfer::URLEncode( m_sName ) );
	}
	else
	{
		m_sURL.Format( _T("http://%s:%i/get/%lu/%s"),
			(LPCTSTR)CString( inet_ntoa( m_pAddress ) ), m_nPort, m_nIndex,
			(LPCTSTR)m_sName );
	}
}

//////////////////////////////////////////////////////////////////////
// CQueryHit XML metadata suffix

BOOL CQueryHit::ParseXML(CXMLElement* pMetaData, DWORD nRealIndex)
{
	CString strRealIndex;
	strRealIndex.Format( _T("%i"), nRealIndex );
	
	for ( POSITION pos1 = pMetaData->GetElementIterator() ; pos1 && ! m_pXML ; )
	{
		CXMLElement* pXML = pMetaData->GetNextElement( pos1 );
		
		for ( POSITION pos2 = pXML->GetElementIterator() ; pos2 ; )
		{
			CXMLElement* pHit		= pXML->GetNextElement( pos2 );
			CXMLAttribute* pIndex	= pHit->GetAttribute( _T("index") );
			
			if ( pIndex != NULL && pIndex->GetValue() == strRealIndex )
			{
				m_sSchemaPlural	= pXML->GetName();
				m_sSchemaURI	= pXML->GetAttributeValue( CXMLAttribute::schemaName, _T("") );
				
				if ( m_sSchemaPlural.GetLength() > 0 && m_sSchemaURI.GetLength() > 0 )
				{
					if ( m_pXML ) delete m_pXML;
					m_pXML = pHit->Detach();
					pIndex->Delete();
				}
				
				break;
			}
		}
	}
	
	return m_pXML != NULL;
}

//////////////////////////////////////////////////////////////////////
// CQueryHit schema auto-detect

BOOL CQueryHit::AutoDetectSchema(LPCTSTR pszInfo)
{
	if ( m_pXML ) return TRUE;

	if ( _tcsstr( pszInfo, _T(" Kbps") ) != NULL && _tcsstr( pszInfo,  _T(" kHz ") ) != NULL )
	{
		if ( AutoDetectAudio( pszInfo ) ) return TRUE;
	}
	
	return FALSE;
}

BOOL CQueryHit::AutoDetectAudio(LPCTSTR pszInfo)
{
	int nBitrate	= 0;
	int nFrequency	= 0;
	int nMinutes	= 0;
	int nSeconds	= 0;
	BOOL bVariable	= FALSE;

	if ( _stscanf( pszInfo, _T("%i Kbps %i kHz %i:%i"), &nBitrate, &nFrequency,
		&nMinutes, &nSeconds ) != 4 )
	{
		bVariable = TRUE;
		if ( _stscanf( pszInfo, _T("%i Kbps(VBR) %i kHz %i:%i"), &nBitrate, &nFrequency,
			&nMinutes, &nSeconds ) != 4 ) return FALSE;
	}

	m_sSchemaURI = CSchema::uriAudio;

	m_pXML = new CXMLElement( NULL, _T("audio") );

	CString strValue;
	strValue.Format( _T("%lu"), nMinutes * 60 + nSeconds );
	m_pXML->AddAttribute( _T("seconds"), strValue );

	strValue.Format( bVariable ? _T("%lu~") : _T("%lu"), nBitrate );
	m_pXML->AddAttribute( _T("bitrate"), strValue );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CQueryHit copy and delete helpers

void CQueryHit::Copy(CQueryHit* pOther)
{
	if ( m_pXML ) delete m_pXML;

	m_pSearchID		= pOther->m_pSearchID;
	m_pClientID		= pOther->m_pClientID;
	m_pAddress		= pOther->m_pAddress;
	m_nPort			= pOther->m_nPort;
	m_nSpeed		= pOther->m_nSpeed;
	m_sSpeed		= pOther->m_sSpeed;
	m_pVendor		= pOther->m_pVendor;
	m_bPush			= pOther->m_bPush;
	m_bBusy			= pOther->m_bBusy;
	m_bStable		= pOther->m_bStable;
	m_bMeasured		= pOther->m_bMeasured;
	m_bChat			= pOther->m_bChat;
	m_bBrowseHost	= pOther->m_bBrowseHost;

	m_bSHA1			= pOther->m_bSHA1;
	m_bTiger		= pOther->m_bTiger;
	m_bED2K			= pOther->m_bED2K;
	m_sURL			= pOther->m_sURL;
	m_sName			= pOther->m_sName;
	m_nIndex		= pOther->m_nIndex;
	m_bSize			= pOther->m_bSize;
	m_nSize			= pOther->m_nSize;
	m_sSchemaURI	= pOther->m_sSchemaURI;
	m_sSchemaPlural	= pOther->m_sSchemaPlural;
	m_pXML			= pOther->m_pXML;

	m_bMatched		= pOther->m_bMatched;
	m_bBogus		= pOther->m_bBogus;
	m_bFiltered		= pOther->m_bFiltered;
	m_bDownload		= pOther->m_bDownload;

	if ( m_bSHA1 ) m_pSHA1 = pOther->m_pSHA1;
	if ( m_bTiger ) m_pTiger = pOther->m_pTiger;
	if ( m_bED2K ) m_pED2K = pOther->m_pED2K;

	pOther->m_pXML = NULL;
}

void CQueryHit::Delete()
{
	for ( CQueryHit* pHit = this ; pHit ; )
	{
		CQueryHit* pNext = pHit->m_pNext;
		delete pHit;
		pHit = pNext;
	}
}

//////////////////////////////////////////////////////////////////////
// CQueryHit rating

int CQueryHit::GetRating()
{
	int nRating = 0;
	
	if ( m_bPush != TS_TRUE ) nRating += 4;
	if ( m_bBusy != TS_TRUE ) nRating += 2;
	if ( m_bStable == TS_TRUE ) nRating ++;

	return nRating;
}

//////////////////////////////////////////////////////////////////////
// CQueryHit serialize

void CQueryHit::Serialize(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		ar.Write( &m_pSearchID, sizeof(GGUID) );
		
		ar << m_nProtocol;
		ar.Write( &m_pClientID, sizeof(GGUID) );
		ar.Write( &m_pAddress, sizeof(IN_ADDR) );
		ar << m_nPort;
		ar << m_nSpeed;
		ar << m_sSpeed;
		ar << m_pVendor->m_sCode;
		
		ar << m_bPush;
		ar << m_bBusy;
		ar << m_bStable;
		ar << m_bMeasured;
		ar << m_nUpSlots;
		ar << m_nUpQueue;
		ar << m_bChat;
		ar << m_bBrowseHost;
		
		ar << m_bSHA1;
		if ( m_bSHA1 ) ar.Write( &m_pSHA1, sizeof(SHA1) );
		ar << m_bTiger;
		if ( m_bTiger ) ar.Write( &m_pTiger, sizeof(TIGEROOT) );
		ar << m_bED2K;
		if ( m_bED2K ) ar.Write( &m_pED2K, sizeof(MD4) );

		ar << m_sURL;
		ar << m_sName;
		ar << m_nIndex;
		ar << m_bSize;
		ar << m_nSize;
		ar << m_nSources;
		ar << m_nPartial;
		ar << m_bPreview;
		ar << m_sPreview;
		ar << m_bCollection;
		
		if ( m_pXML == NULL ) m_sSchemaURI.Empty();
		ar << m_sSchemaURI;
		ar << m_sSchemaPlural;
		if ( m_sSchemaURI.GetLength() ) m_pXML->Serialize( ar );
		ar << m_nRating;
		ar << m_sComments;
		
		ar << m_bMatched;
		ar << m_bBogus;
		ar << m_bDownload;
	}
	else
	{
		ar.Read( &m_pSearchID, sizeof(GGUID) );
		
		if ( nVersion >= 9 ) ar >> m_nProtocol;
		ar.Read( &m_pClientID, sizeof(GGUID) );
		ar.Read( &m_pAddress, sizeof(IN_ADDR) );
		ar >> m_nPort;
		ar >> m_nSpeed;
		ar >> m_sSpeed;
		ar >> m_sSchemaURI;
		m_pVendor = VendorCache.Lookup( m_sSchemaURI );

		ar >> m_bPush;
		ar >> m_bBusy;
		ar >> m_bStable;
		ar >> m_bMeasured;
		ar >> m_nUpSlots;
		ar >> m_nUpQueue;
		ar >> m_bChat;
		ar >> m_bBrowseHost;

		ar >> m_bSHA1;
		if ( m_bSHA1 ) ar.Read( &m_pSHA1, sizeof(SHA1) );
		ar >> m_bTiger;
		if ( m_bTiger ) ar.Read( &m_pTiger, sizeof(TIGEROOT) );
		ar >> m_bED2K;
		if ( m_bED2K ) ar.Read( &m_pED2K, sizeof(MD4) );

		ar >> m_sURL;
		ar >> m_sName;
		ar >> m_nIndex;
		ar >> m_bSize;
		
		if ( nVersion >= 10 )
		{
			ar >> m_nSize;
		}
		else
		{
			DWORD nSize;
			ar >> nSize;
			m_nSize = nSize;
		}
		
		ar >> m_nSources;
		ar >> m_nPartial;
		ar >> m_bPreview;
		ar >> m_sPreview;
		if ( nVersion >= 11 ) ar >> m_bCollection;
		
		ar >> m_sSchemaURI;
		ar >> m_sSchemaPlural;
		
		if ( m_sSchemaURI.GetLength() )
		{
			m_pXML = new CXMLElement();
			m_pXML->Serialize( ar );
		}
		
		ar >> m_nRating;
		ar >> m_sComments;
		
		ar >> m_bMatched;
		ar >> m_bBogus;
		ar >> m_bDownload;
		
		if ( m_nSources == 0 && m_sURL.GetLength() ) m_nSources = 1;
	}
}
