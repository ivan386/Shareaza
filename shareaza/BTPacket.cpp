//
// BTPacket.cpp
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "BTPacket.h"
#include "Buffer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CBTPacket::CBTPacketPool CBTPacket::POOL;


//////////////////////////////////////////////////////////////////////
// CBTPacket construction

CBTPacket::CBTPacket() : CPacket( PROTOCOL_BT )
{
	m_nType	= 0;
}

CBTPacket::~CBTPacket()
{
}

//////////////////////////////////////////////////////////////////////
// CBTPacket serialize

void CBTPacket::ToBuffer(CBuffer* pBuffer) const
{
	ASSERT( pBuffer != NULL );

	if ( m_nType == BT_PACKET_KEEPALIVE )
	{
		DWORD nZero = 0;
		pBuffer->Add( &nZero, 4 );
	}
	else
	{
		DWORD nLength = SWAP_LONG( m_nLength + 1 );
		pBuffer->Add( &nLength, 4 );
		pBuffer->Add( &m_nType, 1 );
		pBuffer->Add( m_pBuffer, m_nLength );
	}
}

//////////////////////////////////////////////////////////////////////
// CBTPacket unserialize

CBTPacket* CBTPacket::ReadBuffer(CBuffer* pBuffer)
{
	ASSERT( pBuffer != NULL );
	if ( pBuffer->m_nLength < 4 ) return NULL;

	DWORD nLength = *(DWORD*)pBuffer->m_pBuffer;
	nLength = SWAP_LONG( nLength );
	if ( pBuffer->m_nLength < 4 + nLength ) return NULL;

	if ( nLength == 0 )
	{
		pBuffer->Remove( 4 );
		return CBTPacket::New( BT_PACKET_KEEPALIVE );
	}

	CBTPacket* pPacket = CBTPacket::New( pBuffer->m_pBuffer[4] );
	pPacket->Write( pBuffer->m_pBuffer + 5, nLength - 1 );

	pBuffer->Remove( 4 + nLength );
	return pPacket;
}

//////////////////////////////////////////////////////////////////////
// CBTPacket debugging

LPCTSTR CBTPacket::GetType() const
{
	static TCHAR szTypeBuffer[16];
	_stprintf( szTypeBuffer, _T("%i"), int( m_nType ) );
	return szTypeBuffer;
}
