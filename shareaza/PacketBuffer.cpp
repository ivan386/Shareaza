//
// PacketBuffer.cpp
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
#include "PacketBuffer.h"
#include "G1Packet.h"
#include "Neighbours.h"
#include "Buffer.h"
#include "Statistics.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CG1PacketBuffer construction

CG1PacketBuffer::CG1PacketBuffer(CBuffer* pBuffer)
{
	m_pType = new CG1PacketBufferType[ G1_PACKTYPE_MAX ];

	m_pBuffer	= pBuffer;
	m_nTotal	= 0;
	m_nCycle	= 1;
	m_nIterate	= 0;
	m_nDropped	= 0;
}

CG1PacketBuffer::~CG1PacketBuffer()
{
	Clear();

	delete [] m_pType;
}

//////////////////////////////////////////////////////////////////////
// CG1PacketBuffer add

void CG1PacketBuffer::Add(CG1Packet* pPacket, BOOL bBuffered)
{
	if ( ! bBuffered || ! pPacket->m_nTypeIndex )
	{
		pPacket->ToBuffer( m_pBuffer );
		return;
	}

	BOOL bFresh = m_pType[ pPacket->m_nTypeIndex ].Add( pPacket );

	if ( bFresh )
	{
		m_nTotal++;
	}
	else
	{
		m_nDropped++;
		Statistics.Current.Gnutella1.Lost++;
	}
}

//////////////////////////////////////////////////////////////////////
// CG1PacketBuffer packet selection

CG1Packet* CG1PacketBuffer::GetPacketToSend(DWORD dwExpire)
{
	static int nVolumeByType[ G1_PACKTYPE_MAX ] = { 0, 1, 2, 5, 1, 1, 5, 1, 4 };

	for ( int nCycle = G1_PACKTYPE_MAX * 2 ; nCycle ; nCycle--, m_nCycle++, m_nIterate = 0 )
	{
		m_nCycle %= G1_PACKTYPE_MAX;
		if ( ! m_nCycle ) continue;

		if ( m_nIterate >= nVolumeByType[ m_nCycle ] ) continue;

		CG1Packet* pPacket = m_pType[ m_nCycle ].Get( dwExpire, &m_nTotal, &m_nDropped );

		if ( pPacket )
		{
			m_nIterate++;
			return pPacket;
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CG1PacketBuffer clear

void CG1PacketBuffer::Clear()
{
	for ( int nType = 0 ; nType < G1_PACKTYPE_MAX ; nType++ )
	{
		m_pType[ nType ].Clear();
	}

	m_nTotal = 0;
}


//////////////////////////////////////////////////////////////////////
// CG1PacketBufferType construction

CG1PacketBufferType::CG1PacketBufferType()
{
	m_nCapacity	= Settings.Gnutella1.PacketBufferSize;
	m_pBuffer	= new CG1Packet*[ m_nCapacity ];
	m_pTime		= new DWORD[ m_nCapacity ];
	m_nHead		= 0;
	m_nCount	= 0;
}

CG1PacketBufferType::~CG1PacketBufferType()
{
	Clear();

	delete [] m_pTime;
	delete [] m_pBuffer;
}

//////////////////////////////////////////////////////////////////////
// CG1PacketBufferType add

BOOL CG1PacketBufferType::Add(CG1Packet* pPacket)
{
	BOOL bFresh = TRUE;

	if ( m_nCount == m_nCapacity )
	{
		m_nCount--;
		m_pBuffer[ ( m_nHead + m_nCount ) % m_nCapacity ]->Release();
		bFresh = FALSE;
	}
	
	if ( ! m_nHead-- ) m_nHead += m_nCapacity;
	m_nCount++;

	m_pBuffer[ m_nHead ]	= pPacket;
	m_pTime[ m_nHead ]		= GetTickCount() + Settings.Gnutella1.PacketBufferTime;

	pPacket->AddRef();

	return bFresh;
}

CG1Packet* CG1PacketBufferType::Get(DWORD dwExpire, int* pnTotal, int* pnDropped)
{
	CG1Packet* pPacket;
	DWORD dwPacket;

	do
	{
		if ( ! m_nCount ) return NULL;

		pPacket		= m_pBuffer[ m_nHead ];
		dwPacket	= m_pTime[ m_nHead ];

		m_nHead = ( m_nHead + 1 ) % m_nCapacity;
		m_nCount--;

		if ( dwPacket < dwExpire )
		{
			if ( pnTotal ) (*pnTotal) --;
			if ( pnDropped ) (*pnDropped) ++;
			Statistics.Current.Gnutella1.Lost++;

			pPacket->Release();
			pPacket = NULL;
		}
	}
	while ( pPacket == NULL );

	return pPacket;
}

void CG1PacketBufferType::Clear()
{
	while ( m_nCount-- > 0 )
	{
		m_pBuffer[ ( m_nHead + m_nCount ) % m_nCapacity ]->Release();
	}

	m_nHead = 0;
}

