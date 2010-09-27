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

CDCPacket::CDCPacket()
	: CPacket( PROTOCOL_DC )
{
}

CDCPacket::~CDCPacket()
{
}

void CDCPacket::Reset()
{
	CPacket::Reset();
}

void CDCPacket::ToBuffer(CBuffer* pBuffer, bool /*bTCP*/) const
{
	ASSERT( m_pBuffer && m_nLength );

	pBuffer->Add( m_pBuffer, m_nLength );
}

BOOL CDCPacket::OnPacket(const SOCKADDR_IN* pHost)
{
	SmartDump( pHost, TRUE, FALSE );

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

BOOL CDCPacket::OnCommonHit(const SOCKADDR_IN* /* pHost */)
{
	if ( CQueryHit* pHit = CQueryHit::FromDCPacket( this ) )
	{
		Network.OnQueryHits( pHit );
	}

	return TRUE;
}
