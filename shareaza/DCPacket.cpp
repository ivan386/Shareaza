//
// DCPacket.cpp
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "DCPacket.h"
#include "DCClient.h"
#include "Network.h"
#include "QueryHit.h"
#include "Security.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CDCPacket::CDCPacketPool CDCPacket::POOL;

CDCPacket::CDCPacket() : CPacket( PROTOCOL_DC )
{
}

CDCPacket::~CDCPacket()
{
}

void CDCPacket::ToBuffer(CBuffer* pBuffer) const
{
	ASSERT( m_pBuffer && m_nLength );

	pBuffer->Add( m_pBuffer, m_nLength );
}

BOOL CDCPacket::OnPacket(SOCKADDR_IN* pHost)
{
	if ( m_nLength > 4 && memcmp( m_pBuffer, "$SR ", 4 ) == 0 )
	{
		if ( ! OnCommonHit( pHost ) )
		{
			theApp.Message( MSG_ERROR, IDS_PROTOCOL_BAD_HIT,
				(LPCTSTR)CString( inet_ntoa( pHost->sin_addr ) ) );
		}
		return TRUE;
	}

	// Unknown packet
	return FALSE;
}

BOOL CDCPacket::OnCommonHit(SOCKADDR_IN* /* pHost */)
{
	// Search result
	// $SR Nick FileName<0x05>FileSize FreeSlots/TotalSlots<0x05>HubName (HubIP:HubPort)|

	std::string strParams( (const char*)m_pBuffer + 4, m_nLength - 5 );
	std::string::size_type nPos = strParams.find( ' ' );
	if ( nPos == std::string::npos )
		return FALSE;
	std::string strNick = strParams.substr( 0, nPos );
	strParams = strParams.substr( nPos + 1 );

	nPos = strParams.find( '\x05' );
	if ( nPos == std::string::npos )
		return FALSE;
	std::string strFilename = strParams.substr( 0, nPos );
	strParams = strParams.substr( nPos + 1 );
	nPos = strFilename.rfind( '\\' );
	if ( nPos != std::string::npos )
	{
		// Cut off path
		strFilename = strFilename.substr( nPos + 1 );
	}

	nPos = strParams.find( ' ' );
	if ( nPos == std::string::npos )
		return FALSE;
	std::string strSize = strParams.substr( 0, nPos );
	strParams = strParams.substr( nPos + 1 );
	QWORD nSize = 0;
	if ( sscanf_s( strSize.c_str(), "%I64u", &nSize ) != 1 )
		return FALSE;

	nPos = strParams.find( '/' );
	if ( nPos == std::string::npos )
		return FALSE;
	std::string strFreeSlots = strParams.substr( 0, nPos );
	strParams = strParams.substr( nPos + 1 );
	DWORD nFreeSlots = 0;
	if ( sscanf_s( strFreeSlots.c_str(), "%u", &nFreeSlots ) != 1 )
		return FALSE;

	nPos = strParams.find( '\x05' );
	if ( nPos == std::string::npos )
		return FALSE;
	std::string strTotalSlots = strParams.substr( 0, nPos );
	strParams = strParams.substr( nPos + 1 );
	DWORD nTotalSlots = 0;
	if ( sscanf_s( strTotalSlots.c_str(), "%u", &nTotalSlots ) != 1 )
		return FALSE;
	if ( ! nTotalSlots || nTotalSlots < nFreeSlots )
		// No upload - useless hit
		return FALSE;

	nPos = strParams.find( ' ' );
	if ( nPos == std::string::npos )
		return FALSE;
	std::string strHubName = strParams.substr( 0, nPos );
	strParams = strParams.substr( nPos + 1 );
	Hashes::TigerHash oTiger;
	if ( strHubName.substr( 0, 4 ) == "TTH:" )
	{
		if ( ! oTiger.fromString( CA2W( strHubName.substr( 4 ).c_str() ) ) )
			return FALSE;
	}

	int len = strParams.size();
	if ( strParams[ 0 ] != '(' || strParams[ len - 1 ] != ')' )
		return FALSE;
	nPos = strParams.find( ':' );
	if ( nPos == std::string::npos )
		return FALSE;
	DWORD nHubAddress = inet_addr( strParams.substr( 1, nPos - 1 ).c_str() );
	int nHubPort = atoi( strParams.substr( nPos + 1, len - nPos - 2 ).c_str() );
	if ( nHubPort <= 0 || nHubPort > USHRT_MAX || nHubAddress == INADDR_NONE ||
		Network.IsFirewalledAddress( (const IN_ADDR*)&nHubAddress ) ||
		Network.IsReserved( (const IN_ADDR*)&nHubAddress ) ||
		Security.IsDenied( (const IN_ADDR*)&nHubAddress ) )
		// Unaccessible hub
		return FALSE;

	CQueryHit* pHit = new CQueryHit( PROTOCOL_DC );
	if ( ! pHit )
		// Out of memory
		return FALSE;

	pHit->m_sName		= CA2W( strFilename.c_str() );
	pHit->m_nSize		= nSize;
	pHit->m_bSize		= TRUE;
	if ( oTiger )
		pHit->m_oTiger	= oTiger;
	pHit->m_pAddress	= *(const IN_ADDR*)&nHubAddress;
	pHit->m_sCountry	= theApp.GetCountryCode( *(const IN_ADDR*)&nHubAddress );
	pHit->m_nPort		= (WORD)nHubPort;
	pHit->m_bChat		= TRUE;
	pHit->m_bBrowseHost	= TRUE;
	pHit->m_sNick		= CA2W( strNick.c_str() );
	pHit->m_nUpSlots	= nTotalSlots;
	pHit->m_nUpQueue	= nTotalSlots - nFreeSlots;
	pHit->m_bBusy		= nFreeSlots ? TRI_TRUE : TRI_FALSE;

	Network.OnQueryHits( pHit );

	return TRUE;
}
