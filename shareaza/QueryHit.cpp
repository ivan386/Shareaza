//
// QueryHit.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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
#include "Security.h"

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

CQueryHit::CQueryHit(PROTOCOLID nProtocol, const Hashes::Guid& oSearchID) :
	m_pNext			( NULL ),
	m_oSearchID		( oSearchID ),
	m_nProtocol		( nProtocol ),
	m_nPort			( 0 ),
	m_nSpeed		( 0 ),
	m_pVendor		( VendorCache.m_pNull ),
	m_bPush			( TRI_UNKNOWN ),
	m_bBusy			( TRI_UNKNOWN ),
	m_bStable		( TRI_UNKNOWN ),
	m_bMeasured		( TRI_UNKNOWN ),
	m_bChat			( FALSE ),
	m_bBrowseHost	( FALSE ),	
	m_nGroup		( 0 ),
//	m_bSHA1			( FALSE ),
//	m_bTiger		( FALSE ),
//	m_bED2K			( FALSE ),
//	m_bBTH			( FALSE ),
	m_nIndex		( 0 ),
	m_bSize			( FALSE ),
	m_nSources		( 0 ),
	m_nPartial		( 0 ),
	m_bPreview		( FALSE ),
	m_nUpSlots		( 0 ),
	m_nUpQueue		( 0 ),	
	m_bCollection	( FALSE ),	
	m_pXML			( NULL ),
	m_nRating		( 0 ),	
	m_bBogus		( FALSE ),
	m_bMatched		( FALSE ),
	m_bExactMatch	( FALSE ),
	m_bFiltered		( FALSE ),
	m_bDownload		( FALSE ),
	m_bNew			( FALSE ),
	m_bSelected		( FALSE ),
	m_bResolveURL	( TRUE )
{
	m_pAddress.s_addr = 0;
}

CQueryHit::~CQueryHit()
{
	delete m_pXML;
}

//////////////////////////////////////////////////////////////////////
// CQueryHit from G1 packet

CQueryHit* CQueryHit::FromG1Packet(CG1Packet* pPacket, int* pnHops)
{
	CQueryHit* pFirstHit	= NULL;
	CQueryHit* pLastHit		= NULL;
	CXMLElement* pXML		= NULL;
	Hashes::Guid oQueryID;
	
	if ( pPacket->m_nProtocol == PROTOCOL_G2 )
	{
		GNUTELLAPACKET pG1;
		if ( ! static_cast< CG2Packet* >( static_cast< CPacket* >( pPacket ) )->
			SeekToWrapped() ) return NULL;
		pPacket->Read( &pG1, sizeof(pG1) );
		
		oQueryID = pG1.m_oGUID;
		if ( pnHops ) *pnHops = pG1.m_nHops + 1;
	}
	else
	{
		oQueryID = pPacket->m_oGUID;
		if ( pnHops ) *pnHops = pPacket->m_nHops + 1;
	}
	
	try
	{
		BYTE	nCount		= pPacket->ReadByte();
		WORD	nPort		= pPacket->ReadShortLE();
		DWORD	nAddress	= pPacket->ReadLongLE();
		DWORD	nSpeed		= pPacket->ReadLongLE();
		
		if ( Network.IsReserved( (IN_ADDR*)&nAddress ), false )
			AfxThrowUserException();

		if ( ! nCount ) 
			AfxThrowUserException();
		
		while ( nCount-- )
		{
			CQueryHit* pHit = new CQueryHit( PROTOCOL_G1, oQueryID );
			if ( pFirstHit ) pLastHit->m_pNext = pHit;
			else pFirstHit = pHit;
			pLastHit = pHit;
			
			pHit->m_pAddress	= (IN_ADDR&)nAddress;
			pHit->m_sCountry	= theApp.GetCountryCode( pHit->m_pAddress );
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

		if ( pPacket->GetRemaining() >= 16u + 5u )
		{
			CHAR szaVendor[ 4 ];
			pPacket->Read( szaVendor, 4 );
			TCHAR szVendor[5] = { szaVendor[0], szaVendor[1], szaVendor[2], szaVendor[3], 0 };
			
			pVendor = VendorCache.Lookup( szVendor );
			if ( ! pVendor ) pVendor = VendorCache.m_pNull;
			nPublicSize	= pPacket->ReadByte();
		}
		
		if ( pVendor && pVendor->m_bHTMLBrowse ) bBrowseHost = TRUE;
		
		if ( nPublicSize + 16u > pPacket->GetRemaining() ) 
			AfxThrowUserException();
		
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
			if ( nPublicSize + nXMLSize + 16u > pPacket->GetRemaining() ) 
				AfxThrowUserException();
		}

		while ( nPublicSize-- ) pPacket->ReadByte();

		if ( pVendor && pVendor->m_bChatFlag && pPacket->GetRemaining() >= 16u + nXMLSize + 1u )
		{
			bChat = pPacket->PeekByte() == 1;
		}

		if ( pPacket->GetRemaining() < 16u + nXMLSize ) nXMLSize = 0;

		if ( ( nFlags[0] & G1_QHD_GGEP ) && ( nFlags[1] & G1_QHD_GGEP ) &&
			 Settings.Gnutella1.EnableGGEP )
		{
			ReadGGEP( pPacket, &bBrowseHost, &bChat );
		}

		// Read first 2 bytes of private data
		if ( pPacket->GetRemaining() >= 16u + 2u )
		{
			WORD nID = pPacket->ReadShortLE();
			if ( nID > 0 )
				theApp.Message( MSG_DEBUG, L"Private ID: 0x%x", nID );
		}

		if ( nXMLSize > 0 )
		{
			pPacket->Seek( 16 + nXMLSize, CG1Packet::seekEnd );
			pXML = ReadXML( pPacket, nXMLSize );
			if ( pXML == NULL && nXMLSize > 1 )
			{
				CString strVendorName;
				if ( pVendor ) strVendorName = pVendor->m_sName;
				theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, L"Invalid compressed metadata. Vendor: %s", strVendorName );
			}
		}

		if ( ! nPort || Network.IsFirewalledAddress( &nAddress ) )
		{
			nFlags[0] |= G1_QHD_PUSH;
			nFlags[1] |= G1_QHD_PUSH;
		}
		
		Hashes::Guid oClientID;
		
		pPacket->Seek( 16, CG1Packet::seekEnd );
		pPacket->Read( oClientID );
		
		DWORD nIndex = 0;
		
		for ( pLastHit = pFirstHit ; pLastHit ; pLastHit = pLastHit->m_pNext, nIndex++ )
		{
			pLastHit->ParseAttributes( oClientID, pVendor, nFlags, bChat, bBrowseHost );
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

CQueryHit* CQueryHit::FromG2Packet(CG2Packet* pPacket, int* pnHops)
{
	if ( pPacket->IsType( G2_PACKET_HIT_WRAP ) )
	{
		return FromG1Packet( (CG1Packet*)pPacket );
	}
	
	if ( ! pPacket->m_bCompound )
	{
		theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: No compounded packet") );
		return NULL;
	}

	CQueryHit* pFirstHit	= NULL;
	CQueryHit* pLastHit		= NULL;
	CXMLElement* pXML		= NULL;
	
	Hashes::Guid oSearchID;
	Hashes::Guid oClientID;
	
	DWORD		nAddress	= 0;
	WORD		nPort		= 0;
	BOOL		bBusy		= FALSE;
	BOOL		bPush		= FALSE;
	BOOL		bStable		= TRUE;
	BOOL		bBrowseHost	= FALSE;
	BOOL		bPeerChat	= FALSE;
	CVendor*	pVendor		= VendorCache.m_pNull;
	
	CString		strNick;
	DWORD		nGroupState[8][4] = {};
	
	typedef std::pair< DWORD, u_short > AddrPortPair;
	typedef std::map< DWORD, u_short >::iterator NodeIter;

	std::map< DWORD, u_short > pTestNodeList;
	std::pair< NodeIter, bool > nodeTested;

	try
	{
		BOOL bCompound; 
		G2_PACKET nType;
		DWORD nLength;
		bool bSpam = false;
		
		while ( pPacket->ReadPacket( nType, nLength, &bCompound ) )
		{
			DWORD nSkip = pPacket->m_nPosition + nLength;
			
			if ( bCompound )
			{
				if ( nType != G2_PACKET_HIT_DESCRIPTOR &&
					 nType != G2_PACKET_HIT_GROUP &&
					 nType != G2_PACKET_PROFILE )
				{
					pPacket->SkipCompound( nLength );
				}
			}

			switch ( nType )
			{
			case G2_PACKET_HIT_DESCRIPTOR:
				if ( bCompound )
				{
					CQueryHit* pHit = new CQueryHit( PROTOCOL_G2 );
					if ( pFirstHit )
						pLastHit->m_pNext = pHit;
					else
						pFirstHit = pHit;
					pLastHit = pHit;
					if ( ! pHit->ReadG2Packet( pPacket, nLength ) )
						AfxThrowUserException();
				}
				else
					theApp.Message( MSG_DEBUG, _T("[G2 Hit] Error: Got hit descriptor without compound flag") );
				break;

			case G2_PACKET_HIT_GROUP:
				if ( bCompound )
				{
					DWORD nQueued = 0, nUploads = 0, nSpeed = 0;
					G2_PACKET nInnerType;
					DWORD nInner;
					
					while ( pPacket->m_nPosition < nSkip && pPacket->ReadPacket( nInnerType, nInner ) )
					{
						DWORD nSkipInner = pPacket->m_nPosition + nInner;
						
						if ( nInnerType == G2_PACKET_PEER_STATUS && nInner >= 7 )
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
				else
					theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: Got hit group without compound flag") );
				break;

			case G2_PACKET_PROFILE:
				if ( bCompound )
				{
					G2_PACKET nInnerType;
					DWORD nInner;
					DWORD ip;
					while ( pPacket->m_nPosition < nSkip && pPacket->ReadPacket( nInnerType, nInner ) )
					{
						DWORD nSkipInner = pPacket->m_nPosition + nInner;
						if ( nInnerType == G2_PACKET_NICK )
						{
							strNick = pPacket->ReadString( nInner );
							CT2A pszIP( (LPCTSTR)strNick );
							ip = inet_addr( pszIP );
							if ( ip != INADDR_NONE && strcmp( inet_ntoa( *(IN_ADDR*)&ip ), pszIP ) == 0 &&
								nAddress != ip )
								bSpam = true;
							if ( strNick.Compare( L"RAZA" ) == 0 || strNick.Compare( L"RAZB" ) == 0 )
								bSpam = true; // VendorCode Nick Spam
						}
						pPacket->m_nPosition = nSkipInner;
					}
				}
				else
					theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: Got profile without compound flag") );
				break;

			case G2_PACKET_NEIGHBOUR_HUB:
				if ( nLength >= 6 )
				{
					SOCKADDR_IN pHub;					
					pHub.sin_addr.S_un.S_addr = pPacket->ReadLongLE();
					pHub.sin_port = htons( pPacket->ReadShortBE() );
					nodeTested = pTestNodeList.insert(
						AddrPortPair( pHub.sin_addr.S_un.S_addr, pHub.sin_port ) );
#ifndef LAN_MODE
					if ( ! nodeTested.second )
					{
						// Not a unique IP and port pair
						bSpam = true;
					}
#endif // LAN_MODE
				}
				else
					theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: Got neighbour hub with invalid length (%u bytes)"), nLength );
				break;

			case G2_PACKET_NODE_GUID:
				if ( nLength == 16 )
				{
					pPacket->Read( oClientID );
					oClientID.validate();
				}
				else
					theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: Got node guid with invalid length (%u bytes)"), nLength );
				break;

			case G2_PACKET_NODE_ADDRESS:
			case G2_PACKET_NODE_INFO:
				if ( nLength >= 6 )
				{
					nAddress = pPacket->ReadLongLE();
					if ( Network.IsReserved( (IN_ADDR*)&nAddress, false ) ||
						 Security.IsDenied( (IN_ADDR*)&nAddress ) )
						bSpam = true;
					nPort = pPacket->ReadShortBE();
				}
				else
					theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: Got node address with invalid length (%u bytes)"), nLength );
				break;

			case G2_PACKET_VENDOR:
				if ( nLength >= 4 )
				{
					CString strVendor = pPacket->ReadString( 4 );
					pVendor = VendorCache.Lookup( strVendor );
					if ( !pVendor )
						pVendor = VendorCache.m_pNull;
				}
				else
					theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: Got vendor with invalid length (%u bytes)"), nLength );
				break;

			case G2_PACKET_METADATA:
				{
					CString strXML	= pPacket->ReadString( nLength );
					LPCTSTR pszXML	= strXML;					
					while ( pszXML && *pszXML )
					{
						CXMLElement* pPart = CXMLElement::FromString( pszXML, TRUE );
						if ( !pPart )
							break;
						
						if ( ! pXML ) pXML = new CXMLElement( NULL, _T("Metadata") );
						pXML->AddElement( pPart );
						
						pszXML = _tcsstr( pszXML + 1, _T("<?xml") );
					}
				}
				break;

			case G2_PACKET_BROWSE_HOST:
				bBrowseHost |= 1;
				break;

			case G2_PACKET_BROWSE_PROFILE:
				bBrowseHost |= 2;
				break;

			case G2_PACKET_PEER_CHAT:
				bPeerChat = TRUE;
				break;

			case G2_PACKET_PEER_BUSY:
				bBusy = TRUE;
				break;

			case G2_PACKET_PEER_UNSTABLE:
				bStable = FALSE;
				break;

			case G2_PACKET_PEER_FIREWALLED:
#ifndef LAN_MODE
				bPush = TRUE;
#endif // LAN_MODE
				break;

			case G2_PACKET_PEER_STATUS:
				if ( nLength > 0 )
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
				else
					theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: Got peer status with invalid length (%u bytes)"), nLength );
				break;

			default:
				theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: Got unknown type (0x%08I64x +%u)"), nType, pPacket->m_nPosition - 8 );
			}

			pPacket->m_nPosition = nSkip;
		}
		
		if ( ! oClientID )
		{
			theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: Node guid missed") );
			AfxThrowUserException();
		}
		if ( pPacket->GetRemaining() < 17 )
		{
			theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: Too short packet (remaining %u bytes)"), pPacket->GetRemaining() );
			AfxThrowUserException();
		}

		BYTE nHops = pPacket->ReadByte() + 1;
		if ( pnHops )
			*pnHops = nHops;
		
		pPacket->Read( oSearchID );
		oSearchID.validate();
		
		if ( !bPush )
			bPush = ( nPort == 0 || Network.IsFirewalledAddress( &nAddress ) );
		
		DWORD nIndex = 0;
		
		for ( pLastHit = pFirstHit ; pLastHit ; pLastHit = pLastHit->m_pNext, nIndex++ )
		{
			if ( nGroupState[ pLastHit->m_nGroup ][0] == FALSE )
				pLastHit->m_nGroup = 0;
			
			pLastHit->m_oSearchID	= oSearchID;
			pLastHit->m_oClientID	= oClientID;
			pLastHit->m_pAddress	= *(IN_ADDR*)&nAddress;
			pLastHit->m_sCountry	= theApp.GetCountryCode( pLastHit->m_pAddress );
			pLastHit->m_nPort		= nPort;
			pLastHit->m_pVendor		= pVendor;
			pLastHit->m_nSpeed		= nGroupState[ pLastHit->m_nGroup ][3];
			pLastHit->m_bBusy		= bBusy   ? TRI_TRUE : TRI_FALSE;
			pLastHit->m_bPush		= bPush   ? TRI_TRUE : TRI_FALSE;
			pLastHit->m_bStable		= bStable ? TRI_TRUE : TRI_FALSE;
			pLastHit->m_bMeasured	= pLastHit->m_bStable;
			pLastHit->m_nUpSlots	= nGroupState[ pLastHit->m_nGroup ][2];
			pLastHit->m_nUpQueue	= nGroupState[ pLastHit->m_nGroup ][1];
			pLastHit->m_bChat		= bPeerChat;
			pLastHit->m_bBrowseHost	= bBrowseHost;
			pLastHit->m_sNick		= strNick;
			pLastHit->m_bPreview	&= pLastHit->m_bPush == TRI_FALSE;
			
			if ( pLastHit->m_nUpSlots > 0 )
			{
				pLastHit->m_bBusy = ( pLastHit->m_nUpSlots <= pLastHit->m_nUpQueue ) ? TRI_TRUE : TRI_FALSE;
			}
			
			pLastHit->Resolve();
			if ( pXML && pLastHit->ParseXML( pXML, nIndex ) ) 
			{
				// Everything is fine
			}
			else if ( pLastHit->m_sName.GetLength() > 0 )
			{
				// These files always must have metadata in Shareaza clients
				LPCTSTR pszExt = PathFindExtension( (LPCTSTR)pLastHit->m_sName );
				if ( _tcsicmp( pszExt, L".wma" ) == 0 || _tcsicmp( pszExt, L".wmw" ) == 0 )
				{
					CString strVendorCode( pLastHit->m_pVendor->m_sCode );
					if ( strVendorCode == L"RAZA" || strVendorCode == L"RAZB" || 
						 strVendorCode == L"RZCB" )
					{
						bSpam = true;
					}
				}
			}
		}
		
		if ( bSpam )
		{
			if ( pFirstHit )
			{
				for ( CQueryHit* pHit = pFirstHit ; pHit ; pHit = pHit->m_pNext )
				{
					pHit->m_bBogus = TRUE;
				}
			}
		}
		else if ( !CheckBogus( pFirstHit ) )
		{
			// Now add all hub list to the route cache
			for ( NodeIter iter = pTestNodeList.begin( ) ; 
				  iter != pTestNodeList.end( ) ; iter++ )
			{
				SOCKADDR_IN pHub;

				pHub.sin_addr.S_un.S_addr	= iter->first;
				pHub.sin_port				= iter->second;

				oClientID[15]++;
				oClientID.validate();
				Network.NodeRoute->Add( oClientID, &pHub );
			}
		}
	}
	catch ( CException* pException )
	{
		pException->Delete();
		if ( pXML )
			delete pXML;
		if ( pFirstHit )
			pFirstHit->Delete();
		return NULL;
	}
	
	if ( pXML )
		delete pXML;
	
	return pFirstHit;
}

//////////////////////////////////////////////////////////////////////
// CQueryHit from ED2K packet

CQueryHit* CQueryHit::FromEDPacket(CEDPacket* pPacket, SOCKADDR_IN* pServer, DWORD m_nServerFlags, const Hashes::Guid& oSearchID )
{
	CQueryHit* pFirstHit	= NULL;
	CQueryHit* pLastHit		= NULL;
    Hashes::Ed2kHash oHash;
	
	if ( pPacket->m_nType == ED2K_S2C_SEARCHRESULTS ||
		 pPacket->m_nType == ED2K_S2CG_SEARCHRESULT )
	{
		DWORD nCount = 1;
		
		if ( pPacket->m_nType == ED2K_S2C_SEARCHRESULTS )
		{
			if ( pPacket->GetRemaining() < 4 ) return NULL;
			nCount = pPacket->ReadLongLE();
		}
		
        while ( nCount-- > 0 && pPacket->GetRemaining() >= Hashes::Ed2kHash::byteCount + 10 )
		{
			CQueryHit* pHit = new CQueryHit( PROTOCOL_ED2K, oSearchID );
			if ( pFirstHit ) pLastHit->m_pNext = pHit;
			else pFirstHit = pHit;
			pLastHit = pHit;

			// Enable chat for ed2k hits
			pHit->m_bChat = TRUE;
			
			pHit->m_pVendor = VendorCache.Lookup( _T("ED2K") );
			if ( ! pHit->m_pVendor ) pHit->m_pVendor = VendorCache.m_pNull;
			if ( ! pHit->ReadEDPacket( pPacket, pServer, m_nServerFlags ) ) break;
			pHit->Resolve();

			if ( pHit->m_bPush == TRI_TRUE )
			{
				//pHit->m_sNick		= _T("(Low ID)");
				pHit->m_nPort		= 0;
			}
		}
	}
	else if (	pPacket->m_nType == ED2K_S2C_FOUNDSOURCES ||
				pPacket->m_nType == ED2K_S2CG_FOUNDSOURCES )
	{
        if ( pPacket->GetRemaining() < Hashes::Ed2kHash::byteCount + 1 ) return NULL;
		pPacket->Read( oHash );

		BYTE nCount = pPacket->ReadByte();
		
		while ( nCount-- > 0 && pPacket->GetRemaining() >= 6 )
		{
			CQueryHit* pHit = new CQueryHit( PROTOCOL_ED2K, oSearchID );
			if ( pFirstHit ) pLastHit->m_pNext = pHit;
			else pFirstHit = pHit;
			pLastHit = pHit;
			
			// Enable chat for ed2k hits
			pHit->m_bChat = TRUE;

			pHit->m_oED2K = oHash;
			pHit->m_pVendor = VendorCache.Lookup( _T("ED2K") );
			if ( ! pHit->m_pVendor ) pHit->m_pVendor = VendorCache.m_pNull;
			pHit->ReadEDAddress( pPacket, pServer );
			pHit->Resolve();
		}
	}

	// Enable chat for ed2k hits
	//pFirstHit->m_bChat = TRUE;
	
	//CheckBogus( pFirstHit );
	
	return pFirstHit;
}

//////////////////////////////////////////////////////////////////////
// CQueryHit bogus checking

BOOL CQueryHit::CheckBogus(CQueryHit* pFirstHit)
{
	typedef std::vector< std::wstring > StringList;
	std::wstring strTemp;

	if ( pFirstHit == NULL ) return TRUE;

	StringList pList;
	if ( pFirstHit->m_oSHA1 )
	{
		strTemp.assign( pFirstHit->m_oSHA1.toUrn() );
		pList.push_back( strTemp );
	}
	if ( pFirstHit->m_oED2K )
	{
		strTemp.assign( pFirstHit->m_oED2K.toUrn() );
		pList.push_back( strTemp );
	}
	if ( pFirstHit->m_oBTH )
	{
		strTemp.assign( pFirstHit->m_oBTH.toUrn() );
		pList.push_back( strTemp );
	}
	if ( pFirstHit->m_oMD5 )
	{
		strTemp.assign( pFirstHit->m_oMD5.toUrn() );
		pList.push_back( strTemp );
	}
	
	for ( CQueryHit* pHit = pFirstHit->m_pNext ; pHit ; pHit = pHit->m_pNext )
	{
		if ( pHit->m_oSHA1 )
		{
			strTemp.assign( pHit->m_oSHA1.toUrn() );
			pList.push_back( strTemp );
		}
		if ( pHit->m_oED2K )
		{
			strTemp.assign( pHit->m_oED2K.toUrn() );
			pList.push_back( strTemp );
		}
		if ( pHit->m_oBTH )
		{
			strTemp.assign( pHit->m_oBTH.toUrn() );
			pList.push_back( strTemp );
		}
		if ( pHit->m_oMD5 )
		{
			strTemp.assign( pHit->m_oMD5.toUrn() );
			pList.push_back( strTemp );
		}
	}
	
	size_t nBogus = pList.size();

	StringList::iterator it, it2;
	bool bDuplicate = false;

	for ( it = pList.begin() ; it != pList.end() && !it->empty() ; )
	{
		for ( it2 = it + 1 ; it2 != pList.end() ; )
		{
			if ( *it2 == *it )
			{
				it2 = pList.erase( it2 ); // remove duplicates
				bDuplicate = true;
			}
			else
				++it2;
		}
		if ( bDuplicate )
		{
			it = pList.erase( it );
			bDuplicate = false;
		}
		else
			++it;
	}
	
	nBogus -= pList.size();

	if ( nBogus == 0 ) return FALSE;
	
	for ( CQueryHit* pHit = pFirstHit ; pHit ; pHit = pHit->m_pNext )
	{
		if ( pHit->m_oSHA1 )
			strTemp.assign( pHit->m_oSHA1.toUrn() );
		else if ( pHit->m_oED2K )
			strTemp.assign( pHit->m_oED2K.toUrn() );
		else if ( pHit->m_oBTH )
			strTemp.assign( pHit->m_oBTH.toUrn() );
		else if ( pHit->m_oMD5 )
			strTemp.assign( pHit->m_oMD5.toUrn() );
		else continue;

		if ( std::find( pList.begin(), pList.end(), strTemp ) == pList.end() )
			pHit->m_bBogus = TRUE;
	}
	
	return TRUE;
}

BOOL CQueryHit::HasBogusMetadata()
{
	if ( m_pXML == NULL ) return FALSE;

	CXMLAttribute* pAttribute = NULL;
	if ( ( pAttribute = m_pXML->GetAttribute( L"title" ) ) != NULL )
	{
		if ( _tcsnistr( (LPCTSTR)pAttribute->GetValue(), L"not related", 11 ) )
			return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CQueryHit XML metadata reader

CXMLElement* CQueryHit::ReadXML(CG1Packet* pPacket, int nSize)
{
	if ( nSize < 2 )
		// Empty packet
		return NULL;

	auto_array< BYTE > pRaw( new BYTE[ nSize ] );
	if ( ! pRaw.get() )
		// Out of memory
		return NULL;

	pPacket->Read( pRaw.get(), nSize );

	LPBYTE pszXML = NULL;
	if ( nSize >= 9 && strncmp( (LPCSTR)pRaw.get(), "{deflate}", 9 ) == 0 )
	{
		// Deflate data
		DWORD nRealSize;
		auto_array< BYTE > pText(
			CZLib::Decompress( pRaw.get() + 9, nSize - 10, &nRealSize ) );
		if ( ! pText.get() )
			// Invalid data
			return NULL;
		pRaw = pText;

		pszXML = pRaw.get();
		nSize = (int)nRealSize;
	}
	else if ( nSize >= 11 && strncmp( (LPCSTR)pRaw.get(), "{plaintext}", 11 ) == 0 )
	{
		if ( nSize > 12 )
		{
			pszXML = pRaw.get() + 11;
			nSize -= 12;
		}
	}
	else if ( nSize >= 2 && strncmp( (LPCSTR)pRaw.get(), "{}", 2 ) == 0 )
	{
		if ( nSize > 3 )
		{
			pszXML = pRaw.get() + 2;
			nSize -= 3;
		}
	}
	else if ( nSize > 1 )
	{
		pszXML = pRaw.get();
		nSize -= 1;
	}

	CXMLElement* pRoot	= NULL;
	for( ; pszXML && *pszXML;
		pszXML = (LPBYTE)strstr( (LPCSTR)( pszXML + 1 ), "<?xml" ) )
	{
		CXMLElement* pXML = CXMLElement::FromBytes( pszXML, nSize, TRUE );
		if ( ! pXML )
			// Invalid XML
			break;

		if ( ! pRoot )
		{
			pRoot = new CXMLElement( NULL, _T("Metadata") );
			if ( ! pRoot )
				// Out of memory
				break;
		}

		pRoot->AddElement( pXML );
	}

	return pRoot;
}

//////////////////////////////////////////////////////////////////////
// CQueryHit GGEP reader

BOOL CQueryHit::ReadGGEP(CG1Packet* pPacket, BOOL* pbBrowseHost, BOOL* pbChat)
{
	CGGEPBlock pGGEP;
	
	if ( ! pGGEP.ReadFromPacket( pPacket ) ) return FALSE;
	
	if ( pGGEP.Find( GGEP_HEADER_BROWSE_HOST ) ) *pbBrowseHost = TRUE;
	if ( pGGEP.Find( GGEP_HEADER_CHAT ) ) *pbChat = TRUE;
	
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

	if ( Settings.Gnutella1.QueryHitUTF8 ) //Support UTF-8 Query
	{
		m_sName	= pPacket->ReadStringUTF8();	
	}
	else
	{
		m_sName	= pPacket->ReadStringASCII();
	}

	strData		= pPacket->ReadStringASCII();

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
		if ( (BYTE)( (*pszData) & 0xff ) == GGEP_MAGIC )
		{
			if ( ! Settings.Gnutella1.EnableGGEP ) break;
			
			CGGEPBlock pGGEP;
			pGGEP.ReadFromString( pszData );
			
			if ( CGGEPItem* pItem = pGGEP.Find( GGEP_HEADER_HASH, 21 ) )
			{
				if ( pItem->m_pBuffer[0] > 0 && pItem->m_pBuffer[0] < 3 )
				{
                    std::copy( &pItem->m_pBuffer[1], &pItem->m_pBuffer[1] + 20, &m_oSHA1[ 0 ] );
                    m_oSHA1.validate();
				}
				if ( pItem->m_pBuffer[0] == 2 && pItem->m_nLength >= 24 + 20 + 1 )
				{
                    std::copy( &pItem->m_pBuffer[21], &pItem->m_pBuffer[21] + 24, &m_oTiger[ 0 ] );
                    m_oTiger.validate();
				}
			}
			else if ( CGGEPItem* pItem = pGGEP.Find( GGEP_HEADER_URN, 5 + 32 ) )
			{
				strData = pItem->ToString();
				
				if ( !m_oSHA1 ) m_oSHA1.fromUrn( strData );
				if ( !m_oTiger ) m_oTiger.fromUrn( strData );
				if ( !m_oED2K ) m_oED2K.fromUrn( strData );
				if ( !m_oBTH ) m_oBTH.fromUrn( pszData );
				if ( !m_oMD5 ) m_oMD5.fromUrn( pszData );
			}
			
			if ( CGGEPItem* pItem = pGGEP.Find( GGEP_HEADER_ALTS, 6 ) )
			{
				// the ip-addresses need not be stored, as they are sent upon the download request in the ALT-loc header
				m_nSources = pItem->m_nLength / 6;	// 6 bytes per source (see ALT GGEP extension specification)
			}
			
			break;
		}
		
		LPCTSTR pszSep = _tcschr( pszData, 0x1C );
		size_t nLength = pszSep ? pszSep - pszData : _tcslen( pszData );
		
		if ( _tcsnicmp( pszData, _T("urn:"), 4 ) == 0 )
		{
			if ( !m_oSHA1 ) m_oSHA1.fromUrn( pszData );
			if ( !m_oTiger ) m_oTiger.fromUrn( pszData );
			if ( !m_oED2K ) m_oED2K.fromUrn( pszData );
			if ( !m_oBTH ) m_oBTH.fromUrn( pszData );
			if ( !m_oMD5 ) m_oMD5.fromUrn( pszData );
		}
		else if ( nLength > 4 )
		{
			AutoDetectSchema( pszData );
		}
		
		if ( pszSep ) pszData = pszSep + 1;
		else break;
	}
	if ( !m_oSHA1 && !m_oTiger && !m_oED2K && !m_oBTH && !m_oMD5 )
		AfxThrowUserException();
}

//////////////////////////////////////////////////////////////////////
// CQueryHit G1 attributes suffix

void CQueryHit::ParseAttributes(const Hashes::Guid& oClientID, CVendor* pVendor, BYTE* nFlags, BOOL bChat, BOOL bBrowseHost)
{
	m_oClientID		= oClientID;
	m_pVendor		= pVendor ? pVendor : VendorCache.m_pNull;
	m_bChat			= bChat;
	m_bBrowseHost	= bBrowseHost;
	
	if ( nFlags[1] & G1_QHD_PUSH )
		m_bPush		= ( nFlags[0] & G1_QHD_PUSH ) ? TRI_TRUE : TRI_FALSE;
	if ( nFlags[0] & G1_QHD_BUSY )
		m_bBusy		= ( nFlags[1] & G1_QHD_BUSY ) ? TRI_TRUE : TRI_FALSE;
	if ( nFlags[0] & G1_QHD_STABLE )
		m_bStable	= ( nFlags[1] & G1_QHD_STABLE ) ? TRI_TRUE : TRI_FALSE;
	if ( nFlags[0] & G1_QHD_SPEED )
		m_bMeasured	= ( nFlags[1] & G1_QHD_SPEED ) ? TRI_TRUE : TRI_FALSE;
}

//////////////////////////////////////////////////////////////////////
// CQueryHit G2 result entry reader

bool CQueryHit::ReadG2Packet(CG2Packet* pPacket, DWORD nLength)
{
	DWORD nPacket, nEnd = pPacket->m_nPosition + nLength;
	G2_PACKET nType;

	m_bResolveURL = FALSE;

	try
	{
		while ( pPacket->m_nPosition < nEnd && pPacket->ReadPacket( nType, nPacket ) )
		{
			DWORD nSkip = pPacket->m_nPosition + nPacket;
			
			switch ( nType )
			{
			case G2_PACKET_URN:
				{
					CString strURN = pPacket->ReadString( nPacket ); // Null terminated
					if ( strURN.GetLength() == (int)nPacket )
					{
						theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: Got malformed URN (%s)"), (LPCTSTR)strURN );
						AfxThrowUserException();
					}
					nPacket -= strURN.GetLength() + 1;
					if ( nPacket >= 20 && strURN == _T("sha1") )
					{
						pPacket->Read( m_oSHA1 );
						m_oSHA1.validate();
					}
					else if ( nPacket >= 44 && ( strURN == _T("bp") || strURN == _T("bitprint") ) )
					{
						pPacket->Read( m_oSHA1 );
						m_oSHA1.validate();

						pPacket->Read( m_oTiger );
						m_oTiger.validate();
					}
					else if ( nPacket >= 24 && ( strURN == _T("ttr") || strURN == _T("tree:tiger/") ) )
					{
						pPacket->Read( m_oTiger );
						m_oTiger.validate();
					}
					else if ( nPacket >= 16 && strURN == _T("ed2k") )
					{
						pPacket->Read( m_oED2K );
						m_oED2K.validate();
					}
					else if ( nPacket >= 20 && strURN == _T("btih") )
					{
						pPacket->Read( m_oBTH );
						m_oBTH.validate();
					}
					else if ( nPacket >= 16 && strURN == _T("md5") )
					{
						pPacket->Read( m_oMD5 );
						m_oSHA1.validate();
					}
					else
					{
						theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: Got unknown URN (%s)"), (LPCTSTR)strURN );
						AfxThrowUserException();
					}
				}
				break;

			case G2_PACKET_URL:
				if ( nPacket == 0 )
				{
					m_bResolveURL = TRUE;
				}
				else
				{
					m_sURL = pPacket->ReadString( nPacket );
				}
				break;

			case G2_PACKET_DESCRIPTIVE_NAME:
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
				else
				{
					theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: Got invalid descriptive name") );
					AfxThrowUserException();
				}
				break;

			case G2_PACKET_METADATA:
				if ( nPacket > 0 )
				{
					CString strXML = pPacket->ReadString( nPacket ); // Not null terminated
					if ( strXML.GetLength() != (int)nPacket )
					{
						theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: Got too short metadata (%s)"), (LPCTSTR)strXML );
						AfxThrowUserException();
					}
					if ( CXMLElement* pXML = CXMLElement::FromString( strXML ) )
					{
						if ( CXMLAttribute* pURI = pXML->GetAttribute( CXMLAttribute::schemaName ) )
						{
							m_sSchemaPlural		= pXML->GetName();
							m_sSchemaURI		= pURI->GetValue();
							CXMLElement* pChild	= pXML->GetFirstElement();						
							if ( pChild != NULL && m_sSchemaPlural.GetLength() > 0 &&
								m_sSchemaURI.GetLength() > 0 )
							{
								m_pXML->Delete();
								m_pXML = pChild->Detach();
							}
						}
						else if ( CSchema* pSchema = SchemaCache.Guess( pXML->GetName() ) )
						{
							m_pXML->Delete();
							m_sSchemaPlural	= pSchema->m_sPlural;
							m_sSchemaURI	= pSchema->GetURI();
							m_pXML			= pXML;
							pXML			= NULL;
						}
						else
						{
							theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: Got unknown metadata schema (%s)"), (LPCTSTR)strXML );
						}
						pXML->Delete();
					}
					else
					{
						theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: Got invalid metadata (%s)"), (LPCTSTR)strXML );
						AfxThrowUserException();
					}
				}
				else
					theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: Got empty metadata") );
				break;

			case G2_PACKET_SIZE:
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
				else
					theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: Got invalid packet size") );
				break;

			case G2_PACKET_GROUP_ID:
				if ( nPacket >= 1 )
				{
					m_nGroup = pPacket->ReadByte();
					if ( m_nGroup < 0 ) m_nGroup = 0;
					if ( m_nGroup > 7 ) m_nGroup = 7;
				}
				else
					theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: Got invalid group id") );
				break;

			case G2_PACKET_OBJECT_ID:
				if ( nPacket >= 4 )
				{
					m_nIndex = pPacket->ReadLongBE();
				}
				else
					theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: Got invalid object id") );
				break;

			case G2_PACKET_CACHED_SOURCES:
				if ( nPacket >= 2 )
				{
					m_nSources = pPacket->ReadShortBE();
				}
				else
					theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: Got invalid cached sources") );
				break;

			case G2_PACKET_PARTIAL:
				if ( nPacket >= 4 )
				{
					m_nPartial = pPacket->ReadLongBE();
				}
				else
					theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: Got invalid partial") );
				break;

			case G2_PACKET_COMMENT:
				{
					CString strXML = pPacket->ReadString( nPacket ); // Not null terminated
					if ( strXML.GetLength() != (int)nPacket )
					{
						theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: Got too short comment (%s)"), (LPCTSTR)strXML );
						AfxThrowUserException();
					}
					if ( CXMLElement* pComment = CXMLElement::FromString( strXML ) )
					{
						m_nRating = -1;
						_stscanf( pComment->GetAttributeValue( _T("rating") ), _T("%i"), &m_nRating );
						m_nRating = max( 0, min( 6, m_nRating + 1 ) );
						m_sComments = pComment->GetValue();
						m_sComments.Replace( _T("{n}"), _T("\r\n") );
						delete pComment;
					}
					else
					{
						theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: Got invalid comment (%s)"), (LPCTSTR)strXML );
						AfxThrowUserException();
					}
				}
				break;

			case G2_PACKET_PREVIEW_URL:
				m_bPreview = TRUE;
				if ( nPacket != 0 )
				{
					m_sPreview = pPacket->ReadString( nPacket );
				}
				break;

			case G2_PACKET_BOGUS:
#ifndef LAN_MODE
				m_bBogus = TRUE;
#endif // LAN_MODE
				break;

			case G2_PACKET_COLLECTION:
				m_bCollection = TRUE;
				break;

			default:
				theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: Got unknown type (0x%08I64x +%u)"), nType, pPacket->m_nPosition - 8 );
			}

			pPacket->m_nPosition = nSkip;
		}
		
		if ( ! m_oSHA1 && ! m_oTiger && ! m_oED2K && ! m_oBTH && ! m_oMD5 )
		{
			theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: Got no hash") );
			AfxThrowUserException();
		}
		return true;
	}
	catch ( CException* pException )
	{
		pException->Delete();
		return false;
	}
}

//////////////////////////////////////////////////////////////////////
// CQueryHit ED2K result entry reader

BOOL CQueryHit::ReadEDPacket(CEDPacket* pPacket, SOCKADDR_IN* pServer, DWORD m_nServerFlags )
{
	CString strLength(_T("")), strBitrate(_T("")), strCodec(_T(""));
	DWORD nLength = 0;
	pPacket->Read( m_oED2K );
	
	ReadEDAddress( pPacket, pServer );
	
	DWORD nTags = pPacket->ReadLongLE();

	ULARGE_INTEGER nSize;
	nSize.QuadPart = 0;
	
	while ( nTags-- > 0 )
	{
		if ( pPacket->GetRemaining() < 1 ) return FALSE;

		CEDTag pTag;
		if ( ! pTag.Read( pPacket, m_nServerFlags ) ) 
		{
			theApp.Message( MSG_ERROR | MSG_FACILITY_SEARCH, _T("ED2K search result packet read error") ); //debug check
			return FALSE;
		}
	
		if ( pTag.m_nKey == ED2K_FT_FILENAME )
		{
			m_sName = pTag.m_sValue;
		}
		else if ( pTag.m_nKey == ED2K_FT_FILESIZE )
		{
			if ( pTag.m_nValue <= 0xFFFFFFFF )
				nSize.LowPart = (DWORD)pTag.m_nValue;
			else
			{
				nSize.LowPart =  (DWORD)(   pTag.m_nValue & 0x00000000FFFFFFFF );
				nSize.HighPart = (DWORD)( ( pTag.m_nValue & 0xFFFFFFFF00000000 ) >> 32 );
			}
		}
		else if ( pTag.m_nKey == ED2K_FT_FILESIZEUPPER )
		{
			nSize.HighPart = (DWORD)pTag.m_nValue;
		}
		else if ( pTag.m_nKey == ED2K_FT_LASTSEENCOMPLETE )
		{
			//theApp.Message( MSG_NOTICE,_T("Last seen complete"));
		}
		else if ( pTag.m_nKey == ED2K_FT_SOURCES )
		{
			m_nSources = (DWORD)pTag.m_nValue;
			if ( m_nSources == 0 )
				m_bResolveURL = FALSE;
			else
				m_nSources--;
		}
		else if ( pTag.m_nKey == ED2K_FT_COMPLETESOURCES )
		{
			if ( ! pTag.m_nValue && m_bSize ) //If there are no complete sources
			{
				//Assume this file is 50% complete. (we can't tell yet, but at least this will warn the user)
				m_nPartial = (DWORD)m_nSize >> 2;
				//theApp.Message( MSG_NOTICE, _T("ED2K_FT_COMPLETESOURCES tag reports no complete sources.") );				
			}
			else
			{
				//theApp.Message( MSG_NOTICE, _T("ED2K_FT_COMPLETESOURCES tag reports complete sources present.") );
			}
		}
		else if ( pTag.m_nKey == ED2K_FT_LENGTH )
		{	//Length- new style (DWORD)
			nLength = (DWORD)pTag.m_nValue;	
		}
		else if ( ( pTag.m_nKey == ED2K_FT_BITRATE ) )
		{	//Bitrate- new style
			strBitrate.Format( _T("%lu"), pTag.m_nValue );
		}
		else if  ( ( pTag.m_nKey == ED2K_FT_CODEC ) )
		{	//Codec - new style
			strCodec = pTag.m_sValue;
		}
		else if  ( pTag.m_nKey == ED2K_FT_FILERATING )
		{	// File Rating

			// The server returns rating as a full range (1-255).
			// If the majority of ratings are "very good", take it up to "excellent"

			m_nRating = ( pTag.m_nValue & 0xFF );

			if ( m_nRating >= 250 )			// Excellent
				m_nRating = 6;			 
			else if ( m_nRating >= 220 )	// Very good
				m_nRating = 5;	
			else if ( m_nRating >= 180 )	// Good
				m_nRating = 4;	
			else if ( m_nRating >= 120 )	// Average
				m_nRating = 3;	
			else if ( m_nRating >= 80 )		// Poor
				m_nRating = 2;
			else							// Fake
				m_nRating = 1;

			// the percentage of clients that have given ratings is:
			// = ( pTag.m_nValue >> 8 ) & 0xFF;
			// We could use this in the future to weight the rating...
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
		else
		{
			/*
			//*** Debug check. Remove this when it's working
			CString s;
			s.Format ( _T("Tag: %u sTag: %s Type: %u"), pTag.m_nKey, pTag.m_sKey, pTag.m_nType );
			theApp.Message( MSG_NOTICE, s );

			if ( pTag.m_nType == 2 )
				s.Format ( _T("Value: %s"), pTag.m_sValue);
			else
				s.Format ( _T("Value: %d"), pTag.m_nValue);
			theApp.Message( MSG_NOTICE, s );
			*/
		}
	}

	if ( nSize.QuadPart )
	{
		m_bSize = TRUE;
		m_nSize = nSize.QuadPart;
	}
	else
	{
		// eMule doesn't share 0 byte files, thus it should mean the file size is unknown.
		// It means also a hit for the currently downloaded file but such files have empty 
		// file names.
		m_nSize = SIZE_UNKNOWN;
	}

	// Verify and set metadata
	CString strType;

	// Check we have a valid name
	if ( m_sName.GetLength() )
	{
		int nExtPos = m_sName.ReverseFind( '.' );
		if ( nExtPos != -1 ) 
		{
			strType = m_sName.Mid( nExtPos );
			ToLower( strType );
		}
	}
	else
	{
		m_bBogus = TRUE;
	}

	// If we can determine type, we can add metadata
	if ( strType.GetLength() )
	{
		// Determine type
		CSchema* pSchema = NULL;

		if ( ( pSchema = SchemaCache.Get( CSchema::uriAudio ) ) != NULL &&
			 pSchema->FilterType( strType ) )
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
		else if ( ( pSchema = SchemaCache.Get( CSchema::uriVideo ) ) != NULL &&
				  pSchema->FilterType( strType ) )
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
		else if ( ( pSchema = SchemaCache.Get( CSchema::uriApplication ) ) != NULL &&
				  pSchema->FilterType( strType ) )
		{	// Application
			m_sSchemaURI = CSchema::uriApplication;
		}
		else if ( ( pSchema = SchemaCache.Get( CSchema::uriImage ) ) != NULL && 
					pSchema->FilterType( strType ) )
		{	// Image
			m_sSchemaURI = CSchema::uriImage;
		}
		else if ( ( pSchema = SchemaCache.Get( CSchema::uriBook ) ) != NULL && 
					pSchema->FilterType( strType ) )
		{	// eBook
			m_sSchemaURI = CSchema::uriBook;
		}
		else if ( ( pSchema = SchemaCache.Get( CSchema::uriDocument ) ) != NULL && 
					pSchema->FilterType( strType ) )
		{	// Document
			m_sSchemaURI = CSchema::uriDocument;
		}
		else if ( ( pSchema = SchemaCache.Get( CSchema::uriPresentation ) ) != NULL && 
					pSchema->FilterType( strType ) )
		{	// Presentation
			m_sSchemaURI = CSchema::uriPresentation;
		}
		else if ( ( pSchema = SchemaCache.Get( CSchema::uriSpreadsheet ) ) != NULL && 
					pSchema->FilterType( strType ) )
		{	// Spreadsheet
			m_sSchemaURI = CSchema::uriSpreadsheet;
		}
		else if ( ( pSchema = SchemaCache.Get( CSchema::uriROM ) ) != NULL && 
					pSchema->FilterType( strType ) )
		{	// ROM Image
			m_sSchemaURI = CSchema::uriROM;
		}
	}
	
	return TRUE;
}

void CQueryHit::ReadEDAddress(CEDPacket* pPacket, SOCKADDR_IN* pServer)
{
	DWORD nAddress = m_pAddress.S_un.S_addr = pPacket->ReadLongLE();
	if ( Network.IsReserved( (IN_ADDR*)&nAddress ), false )
		nAddress = 0;
	m_nPort = pPacket->ReadShortLE();
	
	Hashes::Guid::iterator i = m_oClientID.begin();
	*i++ = pServer->sin_addr.S_un.S_addr;
	*i++ = htons( pServer->sin_port );
	*i++ = nAddress;
	*i++ = m_nPort;
	
	if ( nAddress == 0 )
	{
		m_bResolveURL = FALSE;
		m_bPush = TRI_UNKNOWN;
	}
	else if ( CEDPacket::IsLowID( nAddress ) || Network.IsFirewalledAddress( &nAddress ) || ! m_nPort )
	{
		m_bPush = TRI_TRUE;
	}
	else
	{
		m_bPush = TRI_FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// CQueryHit resolve

void CQueryHit::Resolve()
{
	if ( m_bPreview && m_oSHA1 && m_sPreview.IsEmpty() )
	{
		m_sPreview.Format( _T("http://%s:%i/gnutella/preview/v1?%s"),
			(LPCTSTR)CString( inet_ntoa( m_pAddress ) ), m_nPort,
			(LPCTSTR)m_oSHA1.toUrn() );
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
		if ( m_bPush == TRI_TRUE )
		{
			m_sURL.Format( _T("ed2kftp://%lu@%s:%i/%s/%I64i/"),
				m_oClientID.begin()[2],
				(LPCTSTR)CString( inet_ntoa( (IN_ADDR&)m_oClientID.begin()[0] ) ),
				m_oClientID.begin()[1],
				(LPCTSTR)m_oED2K.toString(), m_bSize ? m_nSize : 0 );
		}
		else
		{
			m_sURL.Format( _T("ed2kftp://%s:%i/%s/%I64i/"),
				(LPCTSTR)CString( inet_ntoa( m_pAddress ) ), m_nPort,
				(LPCTSTR)m_oED2K.toString(), m_bSize ? m_nSize : 0 );
		}
		return;
	}

	if ( Settings.Downloads.RequestHash || m_nIndex == 0 )
	{
		m_sURL = GetURL( m_pAddress, m_nPort );
		if ( m_sURL.GetLength() )
			return;
	}

	if ( Settings.Downloads.RequestURLENC )
	{
		m_sURL.Format( _T("http://%s:%i/get/%lu/%s"),
			(LPCTSTR)CString( inet_ntoa( m_pAddress ) ), m_nPort, m_nIndex,
			(LPCTSTR)URLEncode( m_sName ) );
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
					if ( m_pXML )
						delete m_pXML;
					m_pXML = pHit->Detach();
					pIndex->Delete();

					if ( HasBogusMetadata() )
						m_bBogus = TRUE;
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

	m_oSearchID		= pOther->m_oSearchID;
	m_oClientID		= pOther->m_oClientID;
	m_pAddress		= pOther->m_pAddress;
	m_sCountry		= pOther->m_sCountry;
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

	m_oSHA1			= pOther->m_oSHA1;
	m_oTiger		= pOther->m_oTiger;
	m_oED2K			= pOther->m_oED2K;
	m_oBTH			= pOther->m_oBTH;
	m_oMD5			= pOther->m_oMD5;
	m_sURL			= pOther->m_sURL;
	m_sName			= pOther->m_sName;
	m_nIndex		= pOther->m_nIndex;
	m_bSize			= pOther->m_bSize;
	m_nSize			= pOther->m_nSize;
	m_sSchemaURI	= pOther->m_sSchemaURI;
	m_sSchemaPlural	= pOther->m_sSchemaPlural;
	m_pXML			= pOther->m_pXML;

	m_bMatched		= pOther->m_bMatched;
	m_bExactMatch	= pOther->m_bExactMatch;
	m_bBogus		= pOther->m_bBogus;
	m_bFiltered		= pOther->m_bFiltered;
	m_bDownload		= pOther->m_bDownload;

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
	
	if ( m_bPush != TRI_TRUE ) nRating += 4;
	if ( m_bBusy != TRI_TRUE ) nRating += 2;
	if ( m_bStable == TRI_TRUE ) nRating ++;

	return nRating;
}

//////////////////////////////////////////////////////////////////////
// CQueryHit serialize

void CQueryHit::Serialize(CArchive& ar, int nVersion)
{
	if ( ar.IsStoring() )
	{
		ASSERT( m_pVendor );
		if (m_pVendor == NULL) AfxThrowUserException();

		ar.Write( &m_oSearchID[ 0 ], Hashes::Guid::byteCount );

		ar << m_nProtocol;
		ar.Write( &m_oClientID[ 0 ], Hashes::Guid::byteCount );
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

        SerializeOut( ar, m_oSHA1 );
        SerializeOut( ar, m_oTiger );
        SerializeOut( ar, m_oED2K );

		SerializeOut( ar, m_oBTH );
		SerializeOut( ar, m_oMD5 );

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
		ar << m_bExactMatch;
		ar << m_bBogus;
		ar << m_bDownload;
	}
	else
	{
		ReadArchive( ar, &m_oSearchID[ 0 ], Hashes::Guid::byteCount );
		m_oSearchID.validate();
		
		if ( nVersion >= 9 ) ar >> m_nProtocol;
		ReadArchive( ar, &m_oClientID[ 0 ], Hashes::Guid::byteCount );
		m_oClientID.validate();
		ReadArchive( ar, &m_pAddress, sizeof(IN_ADDR) );
		m_sCountry = theApp.GetCountryCode( m_pAddress );
		ar >> m_nPort;
		ar >> m_nSpeed;
		ar >> m_sSpeed;
		CString sCode;
		ar >> sCode;
		m_pVendor = VendorCache.Lookup( sCode );
		if ( ! m_pVendor ) m_pVendor = VendorCache.m_pNull;

		ar >> m_bPush;
		ar >> m_bBusy;
		ar >> m_bStable;
		ar >> m_bMeasured;
		ar >> m_nUpSlots;
		ar >> m_nUpQueue;
		ar >> m_bChat;
		ar >> m_bBrowseHost;

        SerializeIn( ar, m_oSHA1, nVersion );
        SerializeIn( ar, m_oTiger, nVersion );
        SerializeIn( ar, m_oED2K, nVersion );

		if ( nVersion >= 13 )
		{
			SerializeIn( ar, m_oBTH, nVersion );
			SerializeIn( ar, m_oMD5, nVersion );
		}

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
		if ( nVersion >= 12 ) ar >> m_bExactMatch;
		ar >> m_bBogus;
		ar >> m_bDownload;
		
		if ( m_nSources == 0 && m_sURL.GetLength() ) m_nSources = 1;
	}
}

void CQueryHit::Ban(int nBanLength)
{
	Security.Ban( &m_pAddress, nBanLength );
}
