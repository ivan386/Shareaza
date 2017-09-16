//
// QueryHit.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2016.
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
#include "DCPacket.h"
#include "Transfer.h"
#include "SchemaCache.h"
#include "Schema.h"
#include "ZLibWarp.h"
#include "XML.h"
#include "GGEP.h"
#include "VendorCache.h"
#include "RouteCache.h"
#include "Security.h"

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
	m_nIndex		( 0 ),
	m_bSize			( FALSE ),
	m_nHitSources	( 0 ),
	m_nPartial		( 0 ),
	m_bPreview		( FALSE ),
	m_nUpSlots		( 0 ),
	m_nUpQueue		( 0 ),
	m_bCollection	( FALSE ),
	m_pSchema		( NULL ),
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

CQueryHit::CQueryHit(const CQueryHit& pHit) :
	m_pNext			( NULL ),
	m_pSchema		( NULL ),
	m_pXML			( NULL )
{
	*this = pHit;
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

	try
	{
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

		oQueryID.validate();

		BYTE nCount = pPacket->ReadByte();
		if ( ! nCount )
		{
			theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("[G1] Got hit packet with zero hit counter") );
			AfxThrowUserException();
		}
		WORD nPort = pPacket->ReadShortLE();
		DWORD nAddress = pPacket->ReadLongLE();
		DWORD nSpeed = pPacket->ReadLongLE();

		BOOL bBrowseHost = FALSE;
		while ( nCount-- )
		{
			CAutoPtr< CQueryHit > pHit( new CQueryHit( PROTOCOL_G1, oQueryID ) );
			if ( ! pHit )
				AfxThrowMemoryException();

			pHit->m_pAddress	= (IN_ADDR&)nAddress;
			pHit->m_sCountry	= theApp.GetCountryCode( pHit->m_pAddress );
			pHit->m_nPort		= nPort;
			pHit->m_nSpeed		= nSpeed;

			pHit->ReadG1Packet( pPacket );

			if ( pFirstHit )
				pLastHit = pLastHit->m_pNext = pHit.Detach();
			else
				pLastHit = pFirstHit = pHit.Detach();
		}

		// Read Vendor Code
		CVendorPtr pVendor = VendorCache.m_pNull;
		if ( pPacket->GetRemaining() >= Hashes::Guid::byteCount + 4u )
		{
			CHAR szaVendor[ 4 ];
			pPacket->Read( szaVendor, 4 );
			TCHAR szVendor[ 5 ] = { (TCHAR)szaVendor[0], (TCHAR)szaVendor[1], (TCHAR)szaVendor[2], (TCHAR)szaVendor[3], 0 };
			if ( Security.IsVendorBlocked( szVendor ) )
			{
				theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("[G1] Got hit packet from invalid client") );
				AfxThrowUserException();
			}
			pVendor = VendorCache.Lookup( szVendor );
			if ( pVendor )
			{
				if ( pVendor->m_bBrowseFlag )
					bBrowseHost = TRUE;
			}
			else
				pVendor = VendorCache.m_pNull;
		}

		BYTE nPublicSize = 0;
		if ( pPacket->GetRemaining() >= Hashes::Guid::byteCount + 1u )
		{
			nPublicSize	= pPacket->ReadByte();
		}
		if ( pPacket->GetRemaining() < Hashes::Guid::byteCount + nPublicSize )
		{
			theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("[G1] Got hit packet with invalid size of public data") );
			AfxThrowUserException();
		}

		BYTE nFlags[2] = { 0, 0 };
		if ( nPublicSize >= 2 )
		{
			nFlags[0]	= pPacket->ReadByte();
			nFlags[1]	= pPacket->ReadByte();
			nPublicSize -= 2;
		}

		WORD nXMLSize = 0;
		if ( nPublicSize >= 2 )
		{
			nXMLSize = pPacket->ReadShortLE();
			nPublicSize -= 2;
		}

		// Skip extra data
		while ( nPublicSize-- )
			pPacket->ReadByte();

		BOOL bChat = FALSE;
		if ( pVendor && pVendor->m_bChatFlag && Hashes::Guid::byteCount + nXMLSize + 1u <= pPacket->GetRemaining() )
		{
			BYTE nPeek = pPacket->PeekByte();
			if ( nPeek != GGEP_MAGIC )
				bChat = ( nPeek & G1_QHD_CHAT ) != 0;
		}

		if ( Hashes::Guid::byteCount + nXMLSize > pPacket->GetRemaining() )
			nXMLSize = 0;

		if ( ( nFlags[0] & G1_QHD_GGEP ) && ( nFlags[1] & G1_QHD_GGEP ) )
		{
			CGGEPBlock pGGEP;
			if ( pGGEP.ReadFromPacket( pPacket ) )
			{
				if ( Settings.Gnutella1.EnableGGEP )
				{
					if ( pGGEP.Find( GGEP_HEADER_BROWSE_HOST ) )
						bBrowseHost = TRUE;
					if ( pGGEP.Find( GGEP_HEADER_CHAT ) )
						bChat = TRUE;
				}
				if ( Hashes::Guid::byteCount + nXMLSize > pPacket->GetRemaining() )
					nXMLSize = 0;
				if ( Hashes::Guid::byteCount > pPacket->GetRemaining() )
				{
					theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("[G1] Got hit packet without GUID") );
					AfxThrowUserException();
				}
			}
			else
			{
				theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("[G1] Got hit packet with malformed GGEP (main part)") );
				AfxThrowUserException();
			}
		}

		if ( nXMLSize > 0 )
		{
			pPacket->Seek( Hashes::Guid::byteCount + nXMLSize, CG1Packet::seekEnd );
			pXML = ReadXML( pPacket, nXMLSize );
			if ( pXML == NULL && nXMLSize > 4 )
			{
				CString strVendorName;
				if ( pVendor )
					strVendorName = pVendor->m_sName;

				theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, L"[G1] Invalid compressed metadata. Vendor: %s", (LPCTSTR)strVendorName );
			}
		}

		if ( ! nPort || Network.IsFirewalledAddress( (IN_ADDR*)&nAddress ) )
		{
			nFlags[0] |= G1_QHD_PUSH;
			nFlags[1] |= G1_QHD_PUSH;
		}

		// Read client ID
		Hashes::Guid oClientID;
		pPacket->Seek( Hashes::Guid::byteCount, CG1Packet::seekEnd );
		pPacket->Read( oClientID );

		DWORD nIndex = 0;
		for ( CQueryHit* pHit = pFirstHit ; pHit ; pHit = pHit->m_pNext, nIndex++ )
		{
			pHit->ParseAttributes( oClientID, pVendor, nFlags, bChat, bBrowseHost );

			pHit->Resolve();

			if ( pXML )
				pHit->ParseXML( pXML, nIndex );

			if ( ! pHit->m_bBogus && ! pHit->CheckValid() )
			{
				pHit->m_bBogus = TRUE;
			}
		}
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
		theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("[G2] Hit Error: No compounded packet") );
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
	CVendorPtr	pVendor		= VendorCache.m_pNull;
	bool		bSpam		= false;
	CString		strNick;
	DWORD		nGroupState[8][4] = {};
	BOOL		bCompound;
	G2_PACKET	nType;
	DWORD		nLength;

	typedef std::pair< DWORD, u_short > AddrPortPair;
	typedef std::map< DWORD, u_short >::iterator NodeIter;

	std::map< DWORD, u_short > pTestNodeList;
	std::pair< NodeIter, bool > nodeTested;

	try
	{
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
					CAutoPtr< CQueryHit > pHit( new CQueryHit( PROTOCOL_G2 ) );
					if ( ! pHit )
						AfxThrowMemoryException();

					pHit->ReadG2Packet( pPacket, nLength );

					if ( pFirstHit )
						pLastHit = pLastHit->m_pNext = pHit.Detach();
					else
						pLastHit = pFirstHit = pHit.Detach();
				}
				else
					theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("[G2 Hit] Error: Got hit descriptor without compound flag") );
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
					theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("[G2] Hit Error: Got hit group without compound flag") );
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
							ip = inet_addr( (LPCSTR)pszIP );
							if ( ip != INADDR_NONE && _tcscmp( (LPCTSTR)CString( inet_ntoa( *(IN_ADDR*)&ip ) ), strNick ) == 0 &&
								nAddress != ip )
								bSpam = true;
							if ( ! strNick.CompareNoCase( _T( VENDOR_CODE ) ) )
								bSpam = true; // VendorCode Nick Spam
						}
						pPacket->m_nPosition = nSkipInner;
					}
				}
				else
					theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("[G2] Hit Error: Got profile without compound flag") );
				break;

			case G2_PACKET_NEIGHBOUR_HUB:
				if ( nLength >= 6 )
				{
					SOCKADDR_IN pHub;
					pHub.sin_addr.S_un.S_addr = pPacket->ReadLongLE();
					pHub.sin_port = htons( pPacket->ReadShortBE() );
					nodeTested = pTestNodeList.insert(
						AddrPortPair( pHub.sin_addr.S_un.S_addr, pHub.sin_port ) );
					if ( ! nodeTested.second )
					{
						// Not a unique IP and port pair
						bSpam = true;
					}
				}
				else
					theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("[G2] Hit Error: Got neighbour hub with invalid length (%u bytes)"), nLength );
				break;

			case G2_PACKET_NODE_GUID:
				if ( nLength == Hashes::Guid::byteCount )
				{
					pPacket->Read( oClientID );
					oClientID.validate();
				}
				else
					theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("[G2] Hit Error: Got node guid with invalid length (%u bytes)"), nLength );
				break;

			case G2_PACKET_NODE_ADDRESS:
			case G2_PACKET_NODE_INFO:
				if ( nLength >= 6 )
				{
					nAddress = pPacket->ReadLongLE();
					if ( Network.IsReserved( (IN_ADDR*)&nAddress ) || Security.IsDenied( (IN_ADDR*)&nAddress ) )
						bSpam = true;
					nPort = pPacket->ReadShortBE();
				}
				else
					theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("[G2] Hit Error: Got node address with invalid length (%u bytes)"), nLength );
				break;

			case G2_PACKET_VENDOR:
				if ( nLength >= 4 )
				{
					CString strVendor = pPacket->ReadString( 4 );
					if ( Security.IsVendorBlocked( strVendor ) )
					{
						theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("[G2] Got hit packet from invalid client") );
						AfxThrowUserException();
					}

					pVendor = VendorCache.Lookup( strVendor );
					if ( !pVendor )
						pVendor = VendorCache.m_pNull;
				}
				else
					theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("[G2] Hit Error: Got vendor with invalid length (%u bytes)"), nLength );
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
				bPush = TRUE;
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
					theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("[G2] Hit Error: Got peer status with invalid length (%u bytes)"), nLength );
				break;

			default:
				theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("[G2] Hit Error: Got unknown type (0x%08I64x +%u)"), nType, pPacket->m_nPosition - 8 );
			}

			pPacket->m_nPosition = nSkip;
		}

		if ( ! oClientID )
		{
			theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("[G2] Hit Error: Node guid missed") );
			AfxThrowUserException();
		}
		if ( pPacket->GetRemaining() < 17 )
		{
			theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("[G2] Hit Error: Too short packet (remaining %u bytes)"), pPacket->GetRemaining() );
			AfxThrowUserException();
		}

		BYTE nHops = pPacket->ReadByte() + 1;
		if ( pnHops )
			*pnHops = nHops;

		pPacket->Read( oSearchID );
	}
	catch ( CException* pException )
	{
		pException->Delete();
		if ( pXML ) delete pXML;
		if ( pFirstHit ) pFirstHit->Delete();
		return NULL;
	}

	if ( ! bPush )
		bPush = ( nPort == 0 || Network.IsFirewalledAddress( (IN_ADDR*)&nAddress ) );

#ifdef LAN_MODE
	bPush = FALSE;	// No push in LAN mode
#endif // LAN_MODE

	DWORD nIndex = 0;
	for ( CQueryHit* pHit = pFirstHit ; pHit ; pHit = pHit->m_pNext, nIndex++ )
	{
		if ( nGroupState[ pHit->m_nGroup ][0] == FALSE )
			pHit->m_nGroup = 0;

		pHit->m_oSearchID	= oSearchID;
		pHit->m_oClientID	= oClientID;
		pHit->m_pAddress	= *(IN_ADDR*)&nAddress;
		pHit->m_sCountry	= theApp.GetCountryCode( pHit->m_pAddress );
		pHit->m_nPort		= nPort;
		pHit->m_pVendor		= pVendor;
		pHit->m_nSpeed		= nGroupState[ pHit->m_nGroup ][3];
		pHit->m_bBusy		= bBusy   ? TRI_TRUE : TRI_FALSE;
		pHit->m_bPush		= bPush   ? TRI_TRUE : TRI_FALSE;
		pHit->m_bStable		= bStable ? TRI_TRUE : TRI_FALSE;
		pHit->m_bMeasured	= pHit->m_bStable;
		pHit->m_nUpSlots	= nGroupState[ pHit->m_nGroup ][2];
		pHit->m_nUpQueue	= nGroupState[ pHit->m_nGroup ][1];
		pHit->m_bChat		= bPeerChat;
		pHit->m_bBrowseHost	= bBrowseHost;
		pHit->m_sNick		= strNick;
		pHit->m_bPreview	&= pHit->m_bPush == TRI_FALSE;

		if ( pHit->m_nUpSlots > 0 )
		{
			pHit->m_bBusy = ( pHit->m_nUpSlots <= pHit->m_nUpQueue ) ? TRI_TRUE : TRI_FALSE;
		}

		pHit->Resolve();

		if ( pXML )
			pHit->ParseXML( pXML, nIndex );

		if ( ! pHit->m_bBogus && ! pHit->CheckValid() )
		{
			pHit->m_bBogus = TRUE;
		}
	}

	if ( bSpam )
	{
		if ( pFirstHit )
		{
#ifndef LAN_MODE
			for ( CQueryHit* pHit = pFirstHit ; pHit ; pHit = pHit->m_pNext )
			{
				pHit->m_bBogus = TRUE;
			}
#endif // LAN_MODE
		}
	}
	else
	{
		// Now add all hub list to the route cache
		for ( NodeIter iter = pTestNodeList.begin( ) ;
			  iter != pTestNodeList.end( ) ; ++iter )
		{
			SOCKADDR_IN pHub = { AF_INET };
			pHub.sin_addr.s_addr = iter->first;
			pHub.sin_port = iter->second;
			Network.NodeRoute->Add( oClientID, &pHub );
		}
	}

	if ( pXML ) delete pXML;

	return pFirstHit;
}

//////////////////////////////////////////////////////////////////////
// CQueryHit from ED2K packet

CQueryHit* CQueryHit::FromEDPacket(CEDPacket* pPacket, const SOCKADDR_IN* pServer, BOOL bUnicode, const Hashes::Guid& oSearchID )
{
	CQueryHit* pFirstHit	= NULL;
	CQueryHit* pLastHit		= NULL;

	try
	{
		Hashes::Ed2kHash oHash;
		if ( pPacket->m_nType == ED2K_S2C_SEARCHRESULTS ||
			 pPacket->m_nType == ED2K_S2CG_SEARCHRESULT )
		{
			DWORD nCount = 1;

			if ( pPacket->m_nType == ED2K_S2C_SEARCHRESULTS )
			{
				if ( pPacket->GetRemaining() < 4 )
					AfxThrowUserException();
				nCount = pPacket->ReadLongLE();
			}

			while ( nCount-- > 0 && pPacket->GetRemaining() >= Hashes::Ed2kHash::byteCount + 10 )
			{
				CAutoPtr< CQueryHit >pHit( new CQueryHit( PROTOCOL_ED2K, oSearchID ) );
				if ( ! pHit )
					AfxThrowMemoryException();

				pHit->m_bBrowseHost = TRUE;
				pHit->m_bChat = TRUE;
				pHit->m_pVendor = VendorCache.Lookup( _T("ED2K") );
				if ( ! pHit->m_pVendor ) pHit->m_pVendor = VendorCache.m_pNull;

				pHit->ReadEDPacket( pPacket, pServer, bUnicode );

				pHit->Resolve();

				if ( pHit->m_bPush == TRI_TRUE )
				{
					//pHit->m_sNick		= _T("(Low ID)");
					pHit->m_nPort		= 0;
				}

				if ( pFirstHit )
					pLastHit = pLastHit->m_pNext = pHit.Detach();
				else
					pLastHit = pFirstHit = pHit.Detach();
			}
		}
		else if (	pPacket->m_nType == ED2K_S2C_FOUNDSOURCES ||
					pPacket->m_nType == ED2K_S2CG_FOUNDSOURCES )
		{
			if ( pPacket->GetRemaining() < Hashes::Ed2kHash::byteCount + 1 )
				AfxThrowUserException();
			pPacket->Read( oHash );

			BYTE nCount = pPacket->ReadByte();

			while ( nCount-- > 0 && pPacket->GetRemaining() >= 6 )
			{
				CAutoPtr< CQueryHit >pHit( new CQueryHit( PROTOCOL_ED2K, oSearchID ) );
				if ( ! pHit )
					AfxThrowMemoryException();

				pHit->m_bBrowseHost = TRUE;
				pHit->m_bChat = TRUE;
				pHit->m_oED2K = oHash;
				pHit->m_pVendor = VendorCache.Lookup( _T("ED2K") );
				if ( ! pHit->m_pVendor ) pHit->m_pVendor = VendorCache.m_pNull;

				pHit->ReadEDAddress( pPacket, pServer );

				pHit->Resolve();

				if ( pFirstHit )
					pLastHit = pLastHit->m_pNext = pHit.Detach();
				else
					pLastHit = pFirstHit = pHit.Detach();
			}
		}
	}
	catch ( CException* pException )
	{
		pException->Delete();
		if ( pFirstHit ) pFirstHit->Delete();
		return NULL;
	}

	return pFirstHit;
}

CQueryHit* CQueryHit::FromDCPacket(CDCPacket* pPacket)
{
	// Search result
	// $SR Nick FileName<0x05>FileSize FreeSlots/TotalSlots<0x05>HubName (HubIP:HubPort)|

	LPSTR szNick = (LPSTR)&pPacket->m_pBuffer[ 4 ];
	pPacket->m_pBuffer[ pPacket->m_nLength - 1 ] = 0; // ASCIIZ

	LPSTR szFileName = strchr( szNick, ' ' );
	if ( ! szFileName )
		return NULL;
	*szFileName++ = 0;

	LPSTR szFileSize = strchr( szFileName, '\x05' );
	if ( ! szFileSize )
		return NULL;
	*szFileSize++ = 0;

	LPSTR szFreeSlots = strchr( szFileSize, ' ' );
	if ( ! szFreeSlots )
		return NULL;
	*szFreeSlots++ = 0;

	LPSTR szTotalSlots = strchr( szFreeSlots, '/' );
	if ( ! szTotalSlots )
		return NULL;
	*szTotalSlots++ = 0;

	LPSTR szHubName = strchr( szTotalSlots, '\x05' );
	if ( ! szHubName )
		return NULL;
	*szHubName++ = 0;

	LPSTR szAddress = strchr( szHubName, ' ' );
	if ( ! szAddress )
		return NULL;
	*szAddress++ = 0;

	int nFreeSlots = atoi( szFreeSlots );
	if ( nFreeSlots < 0 )
		return NULL;
	int nTotalSlots = atoi( szTotalSlots );
	if ( nTotalSlots < 0 || nFreeSlots > nTotalSlots )
		return NULL;
	QWORD nSize = 0;
	if ( sscanf_s( szFileSize, "%I64u", &nSize ) != 1 )
		return NULL;

	IN_ADDR nHubAddress = {};
	int nHubPort = protocolPorts[ PROTOCOL_DC ];
	if ( *szAddress == '(' )
	{
		if ( *++szAddress != ')' )
		{
			// "HubName (IP[:Port])"
			LPSTR szPort = strchr( szAddress, ':' );
			if ( szPort )
			{
				// "HubName (IP:Port)"
				*szPort++ = 0;

				LPSTR szEnd = strchr( szPort, ')' );
				if ( ! szEnd )
					return NULL;
				*szEnd++ = 0;

				if ( *szPort )
				{
					nHubPort = atoi( szPort );
					if ( nHubPort <= 0 || nHubPort > USHRT_MAX )
						return NULL;
				}
			}
			else
			{
				// "HubName (IP)"
				LPSTR szEnd = strchr( szAddress, ')' );
				if ( ! szEnd )
					return NULL;
				*szEnd++ = 0;
			}

			nHubAddress.s_addr = inet_addr( szAddress );
			if ( nHubAddress.s_addr == INADDR_NONE )
				return FALSE;
			if ( Network.IsFirewalledAddress( &nHubAddress ) ||
				 Network.IsReserved( &nHubAddress ) ||
				 Security.IsDenied( &nHubAddress ) )
				// Unaccessible hub
				return NULL;
		}
	}

	Hashes::TigerHash oTiger;
	if ( strncmp( szHubName, "TTH:", 4 ) == 0 )
	{
		if ( ! oTiger.fromString( CA2T( szHubName + 4 ) ) )
			return NULL;
	}

	LPCSTR szNameOnly = strrchr( szFileName, '\\' );
	if ( ! szNameOnly )
	{
		szNameOnly = strrchr( szFileName, '/' );
		if ( ! szNameOnly )
			szNameOnly = szFileName;
		else
			szNameOnly++;
	}
	else
		szNameOnly++;

	CAutoPtr< CQueryHit > pHit( new CQueryHit( PROTOCOL_DC ) );
	if ( ! pHit )
		// Out of memory
		return NULL;

	pHit->m_sName		= UTF8Decode( szNameOnly );
	pHit->m_nSize		= nSize;
	pHit->m_bSize		= TRUE;
	pHit->m_oTiger		= oTiger;
	pHit->m_bChat		= TRUE;
	pHit->m_bBrowseHost	= TRUE;
	pHit->m_sNick		= UTF8Decode( szNick );
	pHit->m_nUpSlots	= nTotalSlots;
	pHit->m_nUpQueue	= nTotalSlots - nFreeSlots;
	pHit->m_bBusy		= nFreeSlots ? TRI_FALSE : TRI_TRUE;
	pHit->m_pVendor		= VendorCache.Lookup( _T("DC++") );
	if ( ! pHit->m_pVendor ) pHit->m_pVendor = VendorCache.m_pNull;

	// Hub
	pHit->m_bPush		= TRI_TRUE; // Always
	pHit->m_pAddress	= nHubAddress;
	pHit->m_nPort		= (WORD)nHubPort;
	pHit->m_sCountry	= theApp.GetCountryCode( nHubAddress );

	pHit->Resolve();

	return pHit.Detach();
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
		DWORD nRealSize = 0;
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
		pszXML = pRaw.get() + 11;
		nSize -= 11;
	}
	else if ( nSize >= 2 && strncmp( (LPCSTR)pRaw.get(), "{}", 2 ) == 0 )
	{
		pszXML = pRaw.get() + 2;
		nSize -= 2;
	}
	else if ( nSize > 1 )
	{
		pszXML = pRaw.get();
	}

	CXMLElement* pRoot = NULL;
	for ( ; nSize && pszXML; pszXML++, nSize-- )
	{
		// Skip up to "<"
		for ( ; nSize && *pszXML && *pszXML != '<'; pszXML++, nSize--);

		if ( nSize < 5 )
			break;

		// Test for "<?xml"
		if (  pszXML[ 0 ] == '<' &&
			  pszXML[ 1 ] == '?' &&
			( pszXML[ 2 ] == 'x' || pszXML[ 2 ] == 'X' ) &&
			( pszXML[ 3 ] == 'm' || pszXML[ 3 ] == 'M' ) &&
			( pszXML[ 4 ] == 'l' || pszXML[ 4 ] == 'L' ) )
		{
			CXMLElement* pXML = CXMLElement::FromBytes( pszXML, nSize, TRUE );

			pszXML += 4;
			nSize -= 4;

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
	}

	return pRoot;
}

//////////////////////////////////////////////////////////////////////
// CQueryHit validity check

BOOL CQueryHit::CheckValid() const
{
	if ( m_sName.GetLength() > 160 )
	{
		theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("Got bogus hit packet from %s. Too long name (> 160): %d symbols. File \"%s\"."),
			(LPCTSTR)CString( inet_ntoa( m_pAddress ) ), m_sName.GetLength(), (LPCTSTR)m_sName );
		return FALSE;
	}

	if ( ! HasHash() )
	{
		theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("Got bogus hit packet from %s. No hash. File \"%s\"."),
			(LPCTSTR)CString( inet_ntoa( m_pAddress ) ), (LPCTSTR)m_sName );
		return FALSE;
	}

	// These files always must have metadata in Shareaza clients
	if ( m_pVendor && m_pVendor->m_bExtended && ! m_pXML && ! m_sName.IsEmpty() )
	{
		int nPos = m_sName.ReverseFind( _T('.') );
		if ( nPos >= 0 )
		{
			CString sExt = m_sName.Mid( nPos + 1 );
			if ( sExt.CompareNoCase( _T("wma") ) == 0 ||
				 sExt.CompareNoCase( _T("wmv") ) == 0 )
			{
				theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("Got bogus hit packet from %s. No metadata in extended client. File \"%s\"."),
					(LPCTSTR)CString( inet_ntoa( m_pAddress ) ), (LPCTSTR)m_sName );
				return FALSE;
			}
		}
	}

	int nCurWord = 0, nWords = 1;
	for ( LPCTSTR pszName = m_sName ; *pszName ; pszName++ )
	{
		if ( _istgraph( *pszName ) && *pszName != _T('_') && *pszName != _T(',') && *pszName != _T('.') && *pszName != _T('[') && *pszName != _T(']') )
		{
			if ( ++nCurWord > 50 )
			{
				theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("Got bogus hit packet from %s. Too long word (> 50 symbols) in name. File \"%s\"."),
					(LPCTSTR)CString( inet_ntoa( m_pAddress ) ), (LPCTSTR)m_sName );
				return FALSE;
			}
		}
		else
		{
			if ( nCurWord )
				nWords++;
			nCurWord = 0;
		}
	}
	if ( nWords > 30 )
	{
		theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("Got bogus hit packet from %s. Too many words (> 30) in name: %d words. File \"%s\"."),
			(LPCTSTR)CString( inet_ntoa( m_pAddress ) ), nWords, (LPCTSTR)m_sName );
		return FALSE;
	}

	if ( m_pXML )
	{
		if ( CXMLAttribute* pAttribute = m_pXML->GetAttribute( L"title" ) )
		{
			CString sValue = pAttribute->GetValue();
			if ( _tcsnistr( (LPCTSTR)sValue, L"not related", 11 ) )
			{
				theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("Got bogus hit packet from %s. Bogus matadata: \"%s\". File \"%s\"."),
					(LPCTSTR)CString( inet_ntoa( m_pAddress ) ), (LPCTSTR)sValue, (LPCTSTR)m_sName );
				return FALSE;
			}
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CQueryHit G1 result entry reader

void CQueryHit::ReadG1Packet(CG1Packet* pPacket)
{
	m_nIndex	= pPacket->ReadLongLE();
	m_nSize		= pPacket->ReadLongLE();
	m_bSize		= TRUE;

	if ( Settings.Gnutella1.QueryHitUTF8 ) //Support UTF-8 Query
	{
		m_sName	= pPacket->ReadStringUTF8();
	}
	else
	{
		m_sName	= pPacket->ReadStringASCII();
	}

	while ( pPacket->GetRemaining() )
	{
		BYTE nPeek = pPacket->PeekByte();

		if ( nPeek == GGEP_MAGIC )
		{
			// GGEP extension
			ReadGGEP( pPacket );
		}
		else if ( nPeek == 0 )
		{
			// End of hit
			pPacket->ReadByte();
			break;
		}
		else if ( nPeek == 'u' )
		{
			// HUGE extensions
			pPacket->ReadHUGE( this );
		}
		else if ( nPeek == '<' || nPeek == '{' )
		{
			// XML extensions
			pPacket->ReadXML( m_pSchema, m_pXML );
		}
		else
		{
			pPacket->ReadByte();
		}
	}
}

void CQueryHit::ReadGGEP(CG1Packet* pPacket)
{
	CGGEPBlock pGGEP;
	if ( pGGEP.ReadFromPacket( pPacket ) )
	{
		if ( ! Settings.Gnutella1.EnableGGEP )
			return;

		Hashes::Sha1Hash	oSHA1;
		Hashes::TigerHash	oTiger;
		Hashes::Ed2kHash	oED2K;
		Hashes::BtHash		oBTH;
		Hashes::Md5Hash		oMD5;

		CGGEPItem* pItemPos = pGGEP.GetFirst();
		for ( BYTE nItemCount = 0; pItemPos && nItemCount < pGGEP.GetCount();
			nItemCount++, pItemPos = pItemPos->m_pNext )
		{
			if ( pItemPos->IsNamed( GGEP_HEADER_HASH ) )
			{
				switch ( pItemPos->m_pBuffer[0] )
				{
				case GGEP_H_SHA1:
					if ( pItemPos->m_nLength == 20 + 1 )
					{
						oSHA1 = reinterpret_cast< Hashes::Sha1Hash::RawStorage& >(
							pItemPos->m_pBuffer[ 1 ] );
					}
					else
						theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("[G1] Got query packet with GGEP \"H\" type SH1 unknown size (%d bytes)"), pItemPos->m_nLength );
					break;

				case GGEP_H_BITPRINT:
					if ( pItemPos->m_nLength == 24 + 20 + 1 )
					{
						oSHA1 = reinterpret_cast< Hashes::Sha1Hash::RawStorage& >(
							pItemPos->m_pBuffer[ 1 ] );
						oTiger = reinterpret_cast< Hashes::TigerHash::RawStorage& >(
							pItemPos->m_pBuffer[ 21 ] );
					}
					else
						theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("[G1] Got hit packet with GGEP \"H\" type SH1+TTR unknown size (%d bytes)"), pItemPos->m_nLength );
					break;

				case GGEP_H_MD5:
					if ( pItemPos->m_nLength == 16 + 1 )
					{
						oMD5 = reinterpret_cast< Hashes::Md5Hash::RawStorage& >(
							pItemPos->m_pBuffer[ 1 ] );
					}
					else
						theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("[G1] Got hit packet with GGEP \"H\" type MD5 unknown size (%d bytes)"), pItemPos->m_nLength );
					break;

				case GGEP_H_MD4:
					if ( pItemPos->m_nLength == 16 + 1 )
					{
						oED2K = reinterpret_cast< Hashes::Ed2kHash::RawStorage& >(
							pItemPos->m_pBuffer[ 1 ] );
					}
					else
						theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("[G1] Got hit packet with GGEP \"H\" type MD4 unknown size (%d bytes)"), pItemPos->m_nLength );
					break;

				case GGEP_H_UUID:
					// Unsupported
					break;

				default:
					theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("[G1] Got hit packet with GGEP \"H\" unknown type %d (%d bytes)"), pItemPos->m_pBuffer[0], pItemPos->m_nLength );
				}
			}
			else if ( pItemPos->IsNamed( GGEP_HEADER_URN ) )
			{
				CString strURN( _T("urn:") + pItemPos->ToString() );
				if (      oSHA1.fromUrn(  strURN ) );	// Got SHA1
				else if ( oTiger.fromUrn( strURN ) );	// Got Tiger
				else if ( oED2K.fromUrn(  strURN ) );	// Got ED2K
				else if ( oMD5.fromUrn(   strURN ) );	// Got MD5
				else if ( oBTH.fromUrn(   strURN ) );	// Got BTH base32
				else if ( oBTH.fromUrn< Hashes::base16Encoding >( strURN ) );	// Got BTH base16
				else
					theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("[G1] Got hit packet with unknown GGEP URN: \"%s\""), (LPCTSTR)strURN );
			}
			else if ( pItemPos->IsNamed( GGEP_HEADER_TTROOT ) )
			{
				if ( pItemPos->m_nLength == 24 ||
					// Fix
					pItemPos->m_nLength == 25 )
				{
					oTiger = reinterpret_cast< Hashes::TigerHash::RawStorage& >(
						pItemPos->m_pBuffer[ 0 ] );
				}
				else
					theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("[G1] Got hit packet with GGEP \"TT\" unknown size (%d bytes)"), pItemPos->m_nLength );
			}
			else if ( pItemPos->IsNamed( GGEP_HEADER_LARGE_FILE ) )
			{
				if ( pItemPos->m_nLength <= 8 )
				{
					QWORD nFileSize = 0;

					pItemPos->Read( &nFileSize , pItemPos->m_nLength );

					if ( m_nSize != 0 )
						m_nSize = nFileSize;
					else
						m_nSize = SIZE_UNKNOWN;
				}
				else
					theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("[G1] Got hit packet with GGEP \"LF\" unknown size (%d bytes)"), pItemPos->m_nLength );
			}
			else if ( pItemPos->IsNamed( GGEP_HEADER_ALTS ) )
			{
				// the ip-addresses need not be stored, as they are sent upon the download request in the ALT-loc header
				m_nHitSources += pItemPos->m_nLength / 6;
			}
			else if ( pItemPos->IsNamed( GGEP_HEADER_CREATE_TIME ) );
				// Creation time not supported yet
			else if ( pItemPos->IsNamed( GGEP_HEADER_ALTS_TLS ) );
				// AlternateLocations that support TLS not supported yet
			else
				DEBUG_ONLY( theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("[G1] Got hit packet with unknown GGEP \"%s\" (%d bytes)"), (LPCTSTR)pItemPos->m_sID, pItemPos->m_nLength ) );
		}

		if ( oSHA1  && ! m_oSHA1 )  m_oSHA1  = oSHA1;
		if ( oTiger && ! m_oTiger ) m_oTiger = oTiger;
		if ( oED2K  && ! m_oED2K )  m_oED2K  = oED2K;
		if ( oBTH   && ! m_oBTH )   m_oBTH   = oBTH;
		if ( oMD5   && ! m_oMD5 )   m_oMD5   = oMD5;
	}
	else
	{
		// Fatal error
		theApp.Message( MSG_DEBUG | MSG_FACILITY_SEARCH, _T("[G1] Got hit packet with malformed GGEP") );
		AfxThrowUserException();
	}
}

//////////////////////////////////////////////////////////////////////
// CQueryHit G1 attributes suffix

void CQueryHit::ParseAttributes(const Hashes::Guid& oClientID, CVendorPtr pVendor, BYTE* nFlags, BOOL bChat, BOOL bBrowseHost)
{
	m_oClientID		= oClientID;
	m_pVendor		= pVendor ? pVendor : VendorCache.m_pNull;
	m_bChat			= bChat;
	m_bBrowseHost	= bBrowseHost;

	// Flags: 'I understand' first byte, 'Yes/No' - second byte
	// REMEMBER THAT THE PUSH BIT IS SET OPPOSITE THAN THE OTHERS
	if ( nFlags[1] & G1_QHD_PUSH )
		m_bPush		= ( nFlags[0] & G1_QHD_PUSH ) ? TRI_TRUE : TRI_FALSE;
	if ( nFlags[0] & G1_QHD_BUSY )
		m_bBusy		= ( nFlags[1] & G1_QHD_BUSY ) ? TRI_TRUE : TRI_FALSE;
	if ( nFlags[0] & G1_QHD_STABLE )
		m_bStable	= ( nFlags[1] & G1_QHD_STABLE ) ? TRI_TRUE : TRI_FALSE;
	if ( nFlags[0] & G1_QHD_SPEED )
		m_bMeasured	= ( nFlags[1] & G1_QHD_SPEED ) ? TRI_TRUE : TRI_FALSE;

#ifdef LAN_MODE
	m_bPush = TRI_FALSE;	// No push in LAN mode
#endif // LAN_MODE
}

//////////////////////////////////////////////////////////////////////
// CQueryHit G2 result entry reader

void CQueryHit::ReadG2Packet(CG2Packet* pPacket, DWORD nLength)
{
	DWORD nPacket, nEnd = pPacket->m_nPosition + nLength;
	G2_PACKET nType;

	m_bResolveURL = FALSE;

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
					m_oMD5.validate();
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
				if ( m_pXML )
				{
					theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: Got extra metadata (%s)"), (LPCTSTR)strXML );
				}
				else if ( ( m_pXML = CXMLElement::FromString( strXML ) ) != NULL )
				{
					if ( ! SchemaCache.Normalize( m_pSchema, m_pXML ) )
					{
						m_pXML->Delete();
						m_pXML = NULL;
						theApp.Message( MSG_DEBUG, _T("[G2] Hit Error: Got unknown metadata schema (%s)"), (LPCTSTR)strXML );
					}
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
				m_nHitSources += pPacket->ReadShortBE();
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

	if ( ! HasHash() )
		AfxThrowUserException();
}

//////////////////////////////////////////////////////////////////////
// CQueryHit ED2K result entry reader

void CQueryHit::ReadEDPacket(CEDPacket* pPacket, const SOCKADDR_IN* pServer, BOOL bUnicode)
{
	CString strLength, strBitrate, strCodec;
	DWORD nLength = 0;
	pPacket->Read( m_oED2K );

	ReadEDAddress( pPacket, pServer );

	DWORD nTags = pPacket->ReadLongLE();

	ULARGE_INTEGER nSize = {};

	while ( nTags-- > 0 )
	{
		if ( pPacket->GetRemaining() < 1 )
			AfxThrowUserException();

		CEDTag pTag;
		if ( ! pTag.Read( pPacket, bUnicode ) )
			AfxThrowUserException();

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
		else if ( pTag.m_nKey == ED2K_FT_FILESIZE_HI )
		{
			nSize.HighPart = (DWORD)pTag.m_nValue;
		}
		else if ( pTag.m_nKey == ED2K_FT_LASTSEENCOMPLETE )
		{
			//theApp.Message( MSG_NOTICE,_T("Last seen complete"));
		}
		else if ( pTag.m_nKey == ED2K_FT_SOURCES )
		{
			m_nHitSources = (DWORD)pTag.m_nValue;
			if ( m_nHitSources == 0 )
				m_bResolveURL = FALSE;
			else
				m_nHitSources--;
		}
		else if ( pTag.m_nKey == ED2K_FT_COMPLETE_SOURCES )
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
			strBitrate.Format( _T("%I64u"), pTag.m_nValue );
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
		else if ( ( pTag.m_nKey == 0 ) &&
				  ( pTag.m_nType == ED2K_TAG_STRING ) &&
				  ( pTag.m_sKey == _T("length") )  )
		{	//Length- old style (As a string- x:x:x, x:x or x)
			DWORD nSecs = 0, nMins = 0, nHours = 0;

			if ( pTag.m_sValue.GetLength() < 3 )
			{
				_stscanf( pTag.m_sValue, _T("%lu"), &nSecs );
			}
			else if ( pTag.m_sValue.GetLength() < 6 )
			{
				_stscanf( pTag.m_sValue, _T("%lu:%lu"), &nMins, &nSecs );
			}
			else
			{
				_stscanf( pTag.m_sValue, _T("%lu:%lu:%lu"), &nHours, &nMins, &nSecs );
			}

			nLength = (nHours * 60 * 60) + (nMins * 60) + (nSecs);
		}
		else if ( ( pTag.m_nKey == 0 ) &&
				  ( pTag.m_nType == ED2K_TAG_INT ) &&
				  ( pTag.m_sKey == _T("bitrate") ) )
		{	//Bitrate- old style
			strBitrate.Format( _T("%I64u"), pTag.m_nValue );
		}
		else if ( ( pTag.m_nKey == 0 ) &&
				  ( pTag.m_nType == ED2K_TAG_STRING ) &&
				  ( pTag.m_sKey == _T("codec") ) )
		{	//Codec - old style
			strCodec = pTag.m_sValue;
		}
		else
		{
			/*
			// Debug check. Remove this when it's working
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

	// Check we have a valid name
	if ( m_sName.GetLength() )
	{
		// Determine type
		m_pSchema = SchemaCache.GuessByFilename( m_sName );
		if ( m_pSchema )
		{
			if ( m_pXML ) m_pXML->Delete();
			m_pXML = new CXMLElement( NULL, m_pSchema->m_sSingular );

			if ( m_pSchema->CheckURI( CSchema::uriAudio ) )
			{
				// Audio
				if ( nLength > 0 )
				{
					strLength.Format( _T("%lu"), nLength );
					m_pXML->AddAttribute( _T("seconds"), strLength );
				}
				if ( strBitrate.GetLength() )
				{
					m_pXML->AddAttribute( _T("bitrate"), strBitrate );
				}/*
				if ( strCodec.GetLength() )
				{
					m_pXML->AddAttribute( _T("codec"), strCodec );
				}*/
			}
			else if ( m_pSchema->CheckURI( CSchema::uriVideo ) )
			{
				// Video
				if ( nLength > 0 )
				{
					strLength.Format( _T("%.3f"), (double)nLength / (double)60 );
					m_pXML->AddAttribute( _T("minutes"), strLength );
				}/*
				if ( strBitrate.GetLength() )
				{
					m_pXML->AddAttribute( _T("bitrate"), strBitrate );
				}*/
				if ( strCodec.GetLength() )
				{
					m_pXML->AddAttribute( _T("codec"), strCodec );
				}
			}
		}
	}
	else
	{
		m_bBogus = TRUE;
	}
}

void CQueryHit::ReadEDAddress(CEDPacket* pPacket, const SOCKADDR_IN* pServer)
{
	DWORD nAddress = m_pAddress.S_un.S_addr = pPacket->ReadLongLE();
	if ( ! CEDPacket::IsLowID( nAddress ) && Security.IsDenied( (IN_ADDR*)&nAddress ) )
		AfxThrowUserException();

	m_nPort = pPacket->ReadShortLE();

	Hashes::Guid::iterator i = m_oClientID.begin();
	*i++ = pServer->sin_addr.S_un.S_addr;
	*i++ = htons( pServer->sin_port );
	*i++ = nAddress;
	*i++ = m_nPort;
	m_oClientID.validate();

	if ( nAddress == 0 )
	{
		m_bResolveURL = FALSE;
		m_bPush = TRI_UNKNOWN;
	}
	else if ( CEDPacket::IsLowID( nAddress ) ||
		Network.IsFirewalledAddress( (IN_ADDR*)&nAddress ) || ! m_nPort )
	{
		m_bPush = TRI_TRUE;
	}
	else
	{
		m_bPush = TRI_FALSE;
	}

#ifdef LAN_MODE
	m_bPush = TRI_FALSE;	// No push in LAN mode
#endif // LAN_MODE
}

//////////////////////////////////////////////////////////////////////
// CQueryHit resolve

void CQueryHit::Resolve()
{
	if ( m_bPreview && m_oSHA1 && m_sPreview.IsEmpty() )
	{
		m_sPreview.Format( _T("http://%s:%u/gnutella/preview/v1?%s"),
			(LPCTSTR)CString( inet_ntoa( m_pAddress ) ), m_nPort,
			(LPCTSTR)m_oSHA1.toUrn() );
	}

	if ( m_sURL.GetLength() )
	{
		m_nHitSources ++;
		return;
	}
	else if ( ! m_bResolveURL )
		return;

	m_nHitSources++;

	if ( m_nProtocol == PROTOCOL_ED2K )
	{
		if ( m_bPush == TRI_TRUE )
		{
			m_sURL.Format( _T("ed2kftp://%lu@%s:%u/%s/%I64u/"),
				m_oClientID.begin()[2],
				(LPCTSTR)CString( inet_ntoa( (IN_ADDR&)m_oClientID.begin()[0] ) ),
				m_oClientID.begin()[1],
				(LPCTSTR)m_oED2K.toString(), m_bSize ? m_nSize : 0 );
		}
		else
		{
			m_sURL.Format( _T("ed2kftp://%s:%u/%s/%I64u/"),
				(LPCTSTR)CString( inet_ntoa( m_pAddress ) ), m_nPort,
				(LPCTSTR)m_oED2K.toString(), m_bSize ? m_nSize : 0 );
		}
		return;
	}
	else if ( m_nProtocol == PROTOCOL_DC )
	{
		m_sURL.Format( _T("dchub://%s@%s:%u/TTH:%s/%I64u/"),
			(LPCTSTR)URLEncode( m_sNick ),
			(LPCTSTR)CString( inet_ntoa( m_pAddress ) ),
			m_nPort,
			(LPCTSTR)m_oTiger.toString(),
			m_bSize ? m_nSize : 0 );
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
		m_sURL.Format( _T("http://%s:%u/get/%lu/%s"),
			(LPCTSTR)CString( inet_ntoa( m_pAddress ) ), m_nPort, m_nIndex,
			(LPCTSTR)URLEncode( m_sName ) );
	}
	else
	{
		m_sURL.Format( _T("http://%s:%u/get/%lu/%s"),
			(LPCTSTR)CString( inet_ntoa( m_pAddress ) ), m_nPort, m_nIndex,
			(LPCTSTR)m_sName );
	}
}

//////////////////////////////////////////////////////////////////////
// CQueryHit XML metadata suffix

BOOL CQueryHit::ParseXML(CXMLElement* pMetaData, DWORD nRealIndex)
{
	for ( POSITION pos1 = pMetaData->GetElementIterator() ; pos1 ; )
	{
		CXMLElement* pXML = pMetaData->GetNextElement( pos1 );

		for ( POSITION pos2 = pXML->GetElementIterator() ; pos2 ; )
		{
			CXMLElement* pHit = pXML->GetNextElement( pos2 );

			CString sIndex = pHit->GetAttributeValue( _T("index"), _T("-1") );
			if ( _tstoi( sIndex ) == (int)nRealIndex )
			{
				if ( m_pXML ) m_pXML->Delete();
				m_pXML = NULL;
				if ( SchemaCache.Normalize( m_pSchema, pHit ) )
				{
					m_pXML = pHit->Detach();
				}
				return TRUE;
			}
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CQueryHit copy and delete helpers

CQueryHit& CQueryHit::operator=(const CQueryHit& pOther)
{
	m_pNext;
	m_oSearchID		= pOther.m_oSearchID;
	m_nProtocol		= pOther.m_nProtocol;
	m_oClientID		= pOther.m_oClientID;
	m_pAddress		= pOther.m_pAddress;
	m_sCountry		= pOther.m_sCountry;
	m_nPort			= pOther.m_nPort;
	m_nSpeed		= pOther.m_nSpeed;
	m_sSpeed		= pOther.m_sSpeed;
	m_pVendor		= pOther.m_pVendor;
	m_bPush			= pOther.m_bPush;
	m_bBusy			= pOther.m_bBusy;
	m_bStable		= pOther.m_bStable;
	m_bMeasured		= pOther.m_bMeasured;
	m_bChat			= pOther.m_bChat;
	m_bBrowseHost	= pOther.m_bBrowseHost;
	m_sNick			= pOther.m_sNick;
	m_nGroup		= pOther.m_nGroup;
	m_nIndex		= pOther.m_nIndex;
	m_bSize			= pOther.m_bSize;
	m_nPartial		= pOther.m_nPartial;
	m_bPreview		= pOther.m_bPreview;
	m_sPreview		= pOther.m_sPreview;
	m_nUpSlots		= pOther.m_nUpSlots;
	m_nUpQueue		= pOther.m_nUpQueue;
	m_bCollection	= pOther.m_bCollection;
	m_pSchema		= pOther.m_pSchema;
	if ( m_pXML ) delete m_pXML;
	m_pXML			= pOther.m_pXML ? pOther.m_pXML->Clone() : NULL;
	m_nRating		= pOther.m_nRating;
	m_sComments		= pOther.m_sComments;
	m_bBogus		= pOther.m_bBogus;
	m_bMatched		= pOther.m_bMatched;
	m_bExactMatch	= pOther.m_bExactMatch;
	m_bFiltered		= pOther.m_bFiltered;
	m_bDownload		= pOther.m_bDownload;
	m_bNew			= pOther.m_bNew;
	m_bSelected		= pOther.m_bSelected;
	m_bResolveURL	= pOther.m_bResolveURL;
	m_nHitSources	= pOther.m_nHitSources;

// CShareazaFile
	m_sName			= pOther.m_sName;
	m_nSize			= pOther.m_nSize;
	m_oSHA1			= pOther.m_oSHA1;
	m_oTiger		= pOther.m_oTiger;
	m_oED2K			= pOther.m_oED2K;
	m_oBTH			= pOther.m_oBTH;
	m_oMD5			= pOther.m_oMD5;
	m_sPath			= pOther.m_sPath;
	m_sURL			= pOther.m_sURL;

	return *this;
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

void CQueryHit::Serialize(CArchive& ar, int nVersion /* MATCHLIST_SER_VERSION */)
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
		ar << m_nHitSources;
		ar << m_nPartial;
		ar << m_bPreview;
		ar << m_sPreview;
		ar << m_bCollection;

		ar << ( ( m_pSchema && m_pXML ) ? m_pSchema->GetURI() : CString() );
		ar << ( ( m_pSchema && m_pXML ) ? m_pSchema->m_sPlural : CString() );
		if ( m_pSchema && m_pXML ) m_pXML->Serialize( ar );
		ar << m_nRating;
		ar << m_sComments;

		ar << m_bMatched;
		ar << m_bExactMatch;
		ar << m_bBogus;
		ar << m_bDownload;
		ar << m_sNick;
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

		ar >> m_nHitSources;
		ar >> m_nPartial;
		ar >> m_bPreview;
		ar >> m_sPreview;
		if ( nVersion >= 11 ) ar >> m_bCollection;

		CString sSchemaURI, sSchemaPlural;
		ar >> sSchemaURI;
		ar >> sSchemaPlural; // unused
		if ( sSchemaURI.GetLength() )
		{
			m_pSchema = SchemaCache.Get( sSchemaURI );
			m_pXML = new CXMLElement();
			m_pXML->Serialize( ar );
		}

		ar >> m_nRating;
		ar >> m_sComments;

		ar >> m_bMatched;
		if ( nVersion >= 12 ) ar >> m_bExactMatch;
		ar >> m_bBogus;
		ar >> m_bDownload;
		if ( nVersion >= 15 ) ar >> m_sNick;

		if ( m_nHitSources == 0 && m_sURL.GetLength() ) m_nHitSources = 1;
	}
}

void CQueryHit::Ban(int nBanLength)
{
	Security.Ban( &m_pAddress, nBanLength );
}