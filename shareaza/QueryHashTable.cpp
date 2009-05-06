//
// QueryHashTable.cpp
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
#include "QueryHashTable.h"
#include "QueryHashGroup.h"
#include "QueryHashMaster.h"

#include "QuerySearch.h"
#include "Neighbour.h"
#include "Buffer.h"
#include "G1Packet.h"
#include "G2Packet.h"
#include "XML.h"
#include "ZLib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CQueryHashTable construction

CQueryHashTable::CQueryHashTable()
{
	m_bLive		= FALSE;
	m_nCookie	= 0;
	m_pHash		= NULL;
	m_nHash		= 0;
	m_nBits		= 0;
	m_nInfinity	= 1;
	m_nCount	= 0;
	m_pBuffer	= new CBuffer();
	m_pGroup	= NULL;
}

CQueryHashTable::~CQueryHashTable()
{
	if ( m_pGroup ) QueryHashMaster.Remove( this );
	if ( m_pHash ) delete [] m_pHash;
	delete m_pBuffer;
}

//////////////////////////////////////////////////////////////////////
// CQueryHashTable create

void CQueryHashTable::Create()
{
	BOOL bGrouped = m_pGroup != NULL;
	if ( bGrouped ) QueryHashMaster.Remove( this );

	if ( m_pHash ) delete [] m_pHash;

	m_bLive		= TRUE;
	m_nCookie	= GetTickCount();
	m_nBits		= Settings.Library.QueryRouteSize;
	m_nHash		= 1u << m_nBits;
	m_pHash		= new BYTE[ ( m_nHash + 31 ) / 8 ];
	m_nCount	= 0;

	FillMemory( m_pHash, ( m_nHash + 31 ) / 8, 0xFF );

	if ( bGrouped ) QueryHashMaster.Add( this );
}

//////////////////////////////////////////////////////////////////////
// CQueryHashTable clear

void CQueryHashTable::Clear()
{
	if ( ! m_pHash ) return;

	BOOL bGrouped = m_pGroup != NULL;
	if ( bGrouped ) QueryHashMaster.Remove( this );

	m_nCookie	= GetTickCount();
	m_nCount	= 0;

	FillMemory( m_pHash, ( m_nHash + 31 ) / 8, 0xFF );

	if ( bGrouped ) QueryHashMaster.Add( this );
}

//////////////////////////////////////////////////////////////////////
// CQueryHashTable merge tables

const bool CQueryHashTable::Merge(const CQueryHashTable& oSource)
{
	if ( m_pHash == NULL || oSource.m_pHash == NULL ) return FALSE;

	if ( m_nHash == oSource.m_nHash )
	{
		LPBYTE pSourcePtr	= oSource.m_pHash;
		LPBYTE pDestPtr		= m_pHash;

		for ( DWORD nPosition = m_nHash >> 3 ; nPosition ; nPosition-- )
		{
			register BYTE nSourceByte = *pSourcePtr;
			register BYTE nDestByte = *pDestPtr;

#define DO_MERGE(MASKVAL) \
			if ( ! ( nSourceByte & MASKVAL ) && ( nDestByte & MASKVAL ) ) \
			{ \
				*pDestPtr &= ~ MASKVAL; \
				m_nCount++; \
			}

			DO_MERGE(0x01); DO_MERGE(0x02); DO_MERGE(0x04); DO_MERGE(0x08);
			DO_MERGE(0x10); DO_MERGE(0x20); DO_MERGE(0x40); DO_MERGE(0x80);
#undef DO_MERGE

			pSourcePtr ++;
			pDestPtr ++;
		}
	}
	else
	{
		int nDestScale		= 1;
		int nSourceScale	= 1;

		if ( m_nHash > oSource.m_nHash )
		{
			DWORD nIterate = oSource.m_nHash;
			for ( ; nIterate < m_nHash ; nIterate *= 2 ) nDestScale++;
			if ( nIterate != m_nHash ) return FALSE;
		}
		else if ( m_nHash < oSource.m_nHash )
		{
			DWORD nIterate = m_nHash;
			for ( ; nIterate < oSource.m_nHash ; nIterate *= 2 ) nSourceScale++;
			if ( nIterate != oSource.m_nHash ) return FALSE;
		}

		LPBYTE pSourcePtr	= oSource.m_pHash;
		LPBYTE pDestPtr		= m_pHash;
		BYTE nSourceMask	= 0x01;
		BYTE nDestMask		= 0x01;

		for ( DWORD nDest = 0, nSource = 0 ; nDest < m_nHash && nSource < oSource.m_nHash ; )
		{
			BOOL bValue = TRUE;

			for ( int nSample = 0 ; nSample < nSourceScale ; nSample++, nSource++ )
			{
				if ( ( *pSourcePtr & nSourceMask ) == 0 ) bValue = FALSE;

				if ( nSourceMask == 0x80 )
				{
					nSourceMask = 0x01;
					pSourcePtr ++;
				}
				else
				{
					nSourceMask <<= 1;
				}
			}

			for ( int nSample = 0 ; nSample < nDestScale ; nSample++, nDest++ )
			{
				if ( ! bValue && ( *pDestPtr & nDestMask ) )
				{
					*pDestPtr &= ~nDestMask;
					m_nCount++;
				}

				if ( nDestMask == 0x80 )
				{
					nDestMask = 0x01;
					pDestPtr ++;
				}
				else
				{
					nDestMask <<= 1;
				}
			}
		}
	}

	m_nCookie = GetTickCount();

	return TRUE;
}

const bool CQueryHashTable::Merge(const CQueryHashGroup* pSource)
{
	if ( m_pHash == NULL || pSource->m_pHash == NULL )
		return false;

	if ( m_nHash == pSource->m_nHash )
	{
		LPBYTE pSourcePtr	= pSource->m_pHash;
		LPBYTE pDestPtr		= m_pHash;

		for ( DWORD nPosition = m_nHash >> 3 ; nPosition ; nPosition-- )
		{
			register BYTE nDestByte = *pDestPtr;

#define DO_MERGE(MASKVAL) \
			if ( *pSourcePtr++ && ( nDestByte & MASKVAL ) ) \
			{ \
				*pDestPtr &= ~ MASKVAL; \
				m_nCount++; \
			}

			DO_MERGE(0x01); DO_MERGE(0x02); DO_MERGE(0x04); DO_MERGE(0x08);
			DO_MERGE(0x10); DO_MERGE(0x20); DO_MERGE(0x40); DO_MERGE(0x80);
#undef DO_MERGE

			pDestPtr++;
		}
	}
	else
	{
		int nDestScale		= 1;
		int nSourceScale	= 1;

		if ( m_nHash > pSource->m_nHash )
		{
			DWORD nIterate = pSource->m_nHash;
			for ( ; nIterate < m_nHash ; nIterate *= 2 ) nDestScale++;
			if ( nIterate != m_nHash )
				return false;
		}
		else if ( m_nHash < pSource->m_nHash )
		{
			DWORD nIterate = m_nHash;
			for ( ; nIterate < pSource->m_nHash ; nIterate *= 2 ) nSourceScale++;
			if ( nIterate != pSource->m_nHash )
				return false;
		}

		LPBYTE pSourcePtr	= pSource->m_pHash;
		LPBYTE pDestPtr		= m_pHash;
		BYTE nDestMask		= 0x01;

		for ( DWORD nDest = 0, nSource = 0 ; nDest < m_nHash && nSource < pSource->m_nHash ; )
		{
			BOOL bValue = TRUE;

			for ( int nSample = 0 ; nSample < nSourceScale ; nSample++, nSource++ )
			{
				if ( *pSourcePtr++ ) bValue = FALSE;
			}

			for ( int nSample = 0 ; nSample < nDestScale ; nSample++, nDest++ )
			{
				if ( ! bValue && ( *pDestPtr & nDestMask ) )
				{
					*pDestPtr &= ~nDestMask;
					m_nCount++;
				}

				if ( nDestMask == 0x80 )
				{
					nDestMask = 0x01;
					pDestPtr ++;
				}
				else
				{
					nDestMask <<= 1;
				}
			}
		}
	}

	m_nCookie = GetTickCount();

	return true;
}

//////////////////////////////////////////////////////////////////////
// CQueryHashTable packet patch dispatch

const bool CQueryHashTable::PatchTo(CQueryHashTable* pTarget, CNeighbour* pNeighbour)
{
	if ( pTarget->m_pHash == NULL ) return FALSE;
	if ( m_nCookie == pTarget->m_nCookie ) return FALSE;

	m_nCookie	= pTarget->m_nCookie;
	m_nCount	= pTarget->m_nCount;

	BOOL bChanged = FALSE;

	if ( m_pHash == NULL || m_nHash != pTarget->m_nHash )
	{
		if ( m_pHash ) delete [] m_pHash;
		m_pHash = NULL;

		m_nBits		= pTarget->m_nBits;
		m_nHash		= pTarget->m_nHash;
		m_pHash		= new BYTE[ ( m_nHash + 31 ) / 8 ];

		FillMemory( m_pHash, ( m_nHash + 31 ) / 8, 0xFF );

		if ( pNeighbour->m_nProtocol == PROTOCOL_G1 )
		{
			CG1Packet* pReset = CG1Packet::New( G1_PACKET_QUERY_ROUTE, 1 );
			pReset->WriteByte( 0 );
			pReset->WriteLongLE( m_nHash );
			pReset->WriteByte( 2 );
			pNeighbour->Send( pReset );
		}
		else
		{
			CG2Packet* pReset = CG2Packet::New( G2_PACKET_QHT );
			pReset->WriteByte( 0 );
			pReset->WriteLongBE( m_nHash );
			pReset->WriteByte( 1 );
			pNeighbour->Send( pReset );
		}

		bChanged = TRUE;
	}

	BYTE nBits = 4;

	if ( pNeighbour->m_nProtocol == PROTOCOL_G2 )
	{
		nBits = 1;
	}
	else if ( pNeighbour->m_sUserAgent.Find( _T("Shareaza") ) == 0 )
	{
		LPCTSTR pszAgent = pNeighbour->m_sUserAgent;

		if (	_tcsstr( pszAgent, _T(" 1.3") ) ||
				_tcsstr( pszAgent, _T(" 1.2") ) ||
				_tcsstr( pszAgent, _T(" 1.1") ) ||
				_tcsstr( pszAgent, _T(" 1.0") ) )
		{
			return PatchToOldShareaza( pTarget, pNeighbour );
		}

		if (	_tcsstr( pszAgent, _T(" 1.4") ) == NULL &&
				_tcsstr( pszAgent, _T(" 1.5") ) == NULL &&
				_tcsstr( pszAgent, _T(" 1.6.0") ) == NULL )
		{
			nBits = 1;
		}
	}

	BYTE* pBuffer	= new BYTE[ ( m_nHash + 31 ) / ( 8 / nBits ) ];
	BYTE* pHashT	= pTarget->m_pHash;
	BYTE* pHashS	= m_pHash;
//	BYTE  nMask		= 1;

	if ( nBits == 4 )
	{
		static const DWORD changed[ 256 ] =
		{
			0x00000000,0x000000f0,0x0000000f,0x000000ff,0x0000f000,0x0000f0f0,0x0000f00f,0x0000f0ff,
			0x00000f00,0x00000ff0,0x00000f0f,0x00000fff,0x0000ff00,0x0000fff0,0x0000ff0f,0x0000ffff,
			0x00f00000,0x00f000f0,0x00f0000f,0x00f000ff,0x00f0f000,0x00f0f0f0,0x00f0f00f,0x00f0f0ff,
			0x00f00f00,0x00f00ff0,0x00f00f0f,0x00f00fff,0x00f0ff00,0x00f0fff0,0x00f0ff0f,0x00f0ffff,
			0x000f0000,0x000f00f0,0x000f000f,0x000f00ff,0x000ff000,0x000ff0f0,0x000ff00f,0x000ff0ff,
			0x000f0f00,0x000f0ff0,0x000f0f0f,0x000f0fff,0x000fff00,0x000ffff0,0x000fff0f,0x000fffff,
			0x00ff0000,0x00ff00f0,0x00ff000f,0x00ff00ff,0x00fff000,0x00fff0f0,0x00fff00f,0x00fff0ff,
			0x00ff0f00,0x00ff0ff0,0x00ff0f0f,0x00ff0fff,0x00ffff00,0x00fffff0,0x00ffff0f,0x00ffffff,
			0xf0000000,0xf00000f0,0xf000000f,0xf00000ff,0xf000f000,0xf000f0f0,0xf000f00f,0xf000f0ff,
			0xf0000f00,0xf0000ff0,0xf0000f0f,0xf0000fff,0xf000ff00,0xf000fff0,0xf000ff0f,0xf000ffff,
			0xf0f00000,0xf0f000f0,0xf0f0000f,0xf0f000ff,0xf0f0f000,0xf0f0f0f0,0xf0f0f00f,0xf0f0f0ff,
			0xf0f00f00,0xf0f00ff0,0xf0f00f0f,0xf0f00fff,0xf0f0ff00,0xf0f0fff0,0xf0f0ff0f,0xf0f0ffff,
			0xf00f0000,0xf00f00f0,0xf00f000f,0xf00f00ff,0xf00ff000,0xf00ff0f0,0xf00ff00f,0xf00ff0ff,
			0xf00f0f00,0xf00f0ff0,0xf00f0f0f,0xf00f0fff,0xf00fff00,0xf00ffff0,0xf00fff0f,0xf00fffff,
			0xf0ff0000,0xf0ff00f0,0xf0ff000f,0xf0ff00ff,0xf0fff000,0xf0fff0f0,0xf0fff00f,0xf0fff0ff,
			0xf0ff0f00,0xf0ff0ff0,0xf0ff0f0f,0xf0ff0fff,0xf0ffff00,0xf0fffff0,0xf0ffff0f,0xf0ffffff,
			0x0f000000,0x0f0000f0,0x0f00000f,0x0f0000ff,0x0f00f000,0x0f00f0f0,0x0f00f00f,0x0f00f0ff,
			0x0f000f00,0x0f000ff0,0x0f000f0f,0x0f000fff,0x0f00ff00,0x0f00fff0,0x0f00ff0f,0x0f00ffff,
			0x0ff00000,0x0ff000f0,0x0ff0000f,0x0ff000ff,0x0ff0f000,0x0ff0f0f0,0x0ff0f00f,0x0ff0f0ff,
			0x0ff00f00,0x0ff00ff0,0x0ff00f0f,0x0ff00fff,0x0ff0ff00,0x0ff0fff0,0x0ff0ff0f,0x0ff0ffff,
			0x0f0f0000,0x0f0f00f0,0x0f0f000f,0x0f0f00ff,0x0f0ff000,0x0f0ff0f0,0x0f0ff00f,0x0f0ff0ff,
			0x0f0f0f00,0x0f0f0ff0,0x0f0f0f0f,0x0f0f0fff,0x0f0fff00,0x0f0ffff0,0x0f0fff0f,0x0f0fffff,
			0x0fff0000,0x0fff00f0,0x0fff000f,0x0fff00ff,0x0ffff000,0x0ffff0f0,0x0ffff00f,0x0ffff0ff,
			0x0fff0f00,0x0fff0ff0,0x0fff0f0f,0x0fff0fff,0x0fffff00,0x0ffffff0,0x0fffff0f,0x0fffffff,
			0xff000000,0xff0000f0,0xff00000f,0xff0000ff,0xff00f000,0xff00f0f0,0xff00f00f,0xff00f0ff,
			0xff000f00,0xff000ff0,0xff000f0f,0xff000fff,0xff00ff00,0xff00fff0,0xff00ff0f,0xff00ffff,
			0xfff00000,0xfff000f0,0xfff0000f,0xfff000ff,0xfff0f000,0xfff0f0f0,0xfff0f00f,0xfff0f0ff,
			0xfff00f00,0xfff00ff0,0xfff00f0f,0xfff00fff,0xfff0ff00,0xfff0fff0,0xfff0ff0f,0xfff0ffff,
			0xff0f0000,0xff0f00f0,0xff0f000f,0xff0f00ff,0xff0ff000,0xff0ff0f0,0xff0ff00f,0xff0ff0ff,
			0xff0f0f00,0xff0f0ff0,0xff0f0f0f,0xff0f0fff,0xff0fff00,0xff0ffff0,0xff0fff0f,0xff0fffff,
			0xffff0000,0xffff00f0,0xffff000f,0xffff00ff,0xfffff000,0xfffff0f0,0xfffff00f,0xfffff0ff,
			0xffff0f00,0xffff0ff0,0xffff0f0f,0xffff0fff,0xffffff00,0xfffffff0,0xffffff0f,0xffffffff
		};
		static const DWORD changeFlag[ 256 ] =
		{
			0x11111111,0x111111f1,0x1111111f,0x111111ff,0x1111f111,0x1111f1f1,0x1111f11f,0x1111f1ff,
			0x11111f11,0x11111ff1,0x11111f1f,0x11111fff,0x1111ff11,0x1111fff1,0x1111ff1f,0x1111ffff,
			0x11f11111,0x11f111f1,0x11f1111f,0x11f111ff,0x11f1f111,0x11f1f1f1,0x11f1f11f,0x11f1f1ff,
			0x11f11f11,0x11f11ff1,0x11f11f1f,0x11f11fff,0x11f1ff11,0x11f1fff1,0x11f1ff1f,0x11f1ffff,
			0x111f1111,0x111f11f1,0x111f111f,0x111f11ff,0x111ff111,0x111ff1f1,0x111ff11f,0x111ff1ff,
			0x111f1f11,0x111f1ff1,0x111f1f1f,0x111f1fff,0x111fff11,0x111ffff1,0x111fff1f,0x111fffff,
			0x11ff1111,0x11ff11f1,0x11ff111f,0x11ff11ff,0x11fff111,0x11fff1f1,0x11fff11f,0x11fff1ff,
			0x11ff1f11,0x11ff1ff1,0x11ff1f1f,0x11ff1fff,0x11ffff11,0x11fffff1,0x11ffff1f,0x11ffffff,
			0xf1111111,0xf11111f1,0xf111111f,0xf11111ff,0xf111f111,0xf111f1f1,0xf111f11f,0xf111f1ff,
			0xf1111f11,0xf1111ff1,0xf1111f1f,0xf1111fff,0xf111ff11,0xf111fff1,0xf111ff1f,0xf111ffff,
			0xf1f11111,0xf1f111f1,0xf1f1111f,0xf1f111ff,0xf1f1f111,0xf1f1f1f1,0xf1f1f11f,0xf1f1f1ff,
			0xf1f11f11,0xf1f11ff1,0xf1f11f1f,0xf1f11fff,0xf1f1ff11,0xf1f1fff1,0xf1f1ff1f,0xf1f1ffff,
			0xf11f1111,0xf11f11f1,0xf11f111f,0xf11f11ff,0xf11ff111,0xf11ff1f1,0xf11ff11f,0xf11ff1ff,
			0xf11f1f11,0xf11f1ff1,0xf11f1f1f,0xf11f1fff,0xf11fff11,0xf11ffff1,0xf11fff1f,0xf11fffff,
			0xf1ff1111,0xf1ff11f1,0xf1ff111f,0xf1ff11ff,0xf1fff111,0xf1fff1f1,0xf1fff11f,0xf1fff1ff,
			0xf1ff1f11,0xf1ff1ff1,0xf1ff1f1f,0xf1ff1fff,0xf1ffff11,0xf1fffff1,0xf1ffff1f,0xf1ffffff,
			0x1f111111,0x1f1111f1,0x1f11111f,0x1f1111ff,0x1f11f111,0x1f11f1f1,0x1f11f11f,0x1f11f1ff,
			0x1f111f11,0x1f111ff1,0x1f111f1f,0x1f111fff,0x1f11ff11,0x1f11fff1,0x1f11ff1f,0x1f11ffff,
			0x1ff11111,0x1ff111f1,0x1ff1111f,0x1ff111ff,0x1ff1f111,0x1ff1f1f1,0x1ff1f11f,0x1ff1f1ff,
			0x1ff11f11,0x1ff11ff1,0x1ff11f1f,0x1ff11fff,0x1ff1ff11,0x1ff1fff1,0x1ff1ff1f,0x1ff1ffff,
			0x1f1f1111,0x1f1f11f1,0x1f1f111f,0x1f1f11ff,0x1f1ff111,0x1f1ff1f1,0x1f1ff11f,0x1f1ff1ff,
			0x1f1f1f11,0x1f1f1ff1,0x1f1f1f1f,0x1f1f1fff,0x1f1fff11,0x1f1ffff1,0x1f1fff1f,0x1f1fffff,
			0x1fff1111,0x1fff11f1,0x1fff111f,0x1fff11ff,0x1ffff111,0x1ffff1f1,0x1ffff11f,0x1ffff1ff,
			0x1fff1f11,0x1fff1ff1,0x1fff1f1f,0x1fff1fff,0x1fffff11,0x1ffffff1,0x1fffff1f,0x1fffffff,
			0xff111111,0xff1111f1,0xff11111f,0xff1111ff,0xff11f111,0xff11f1f1,0xff11f11f,0xff11f1ff,
			0xff111f11,0xff111ff1,0xff111f1f,0xff111fff,0xff11ff11,0xff11fff1,0xff11ff1f,0xff11ffff,
			0xfff11111,0xfff111f1,0xfff1111f,0xfff111ff,0xfff1f111,0xfff1f1f1,0xfff1f11f,0xfff1f1ff,
			0xfff11f11,0xfff11ff1,0xfff11f1f,0xfff11fff,0xfff1ff11,0xfff1fff1,0xfff1ff1f,0xfff1ffff,
			0xff1f1111,0xff1f11f1,0xff1f111f,0xff1f11ff,0xff1ff111,0xff1ff1f1,0xff1ff11f,0xff1ff1ff,
			0xff1f1f11,0xff1f1ff1,0xff1f1f1f,0xff1f1fff,0xff1fff11,0xff1ffff1,0xff1fff1f,0xff1fffff,
			0xffff1111,0xffff11f1,0xffff111f,0xffff11ff,0xfffff111,0xfffff1f1,0xfffff11f,0xfffff1ff,
			0xffff1f11,0xffff1ff1,0xffff1f1f,0xffff1fff,0xffffff11,0xfffffff1,0xffffff1f,0xffffffff
		};
		DWORD* const pDwordBuffer = reinterpret_cast< DWORD* >( pBuffer );
		const DWORD nEnd = ( m_nHash + 7 ) / 8;
		for ( DWORD nPosition = 0 ; nPosition < nEnd ; ++nPosition )
		{
			if ( ( pDwordBuffer[ nPosition ] = changeFlag[ pHashS[ nPosition ] ]
				& changed[ pHashT[ nPosition ] ^ pHashS[ nPosition ] ] ) != 0 ) bChanged = TRUE;
		}
		if ( bChanged ) std::memcpy( pHashS, pHashT, nEnd );
	}
	else
	{
		const DWORD nEnd = ( m_nHash + 31 ) / 32;
		DWORD* const pDwordBuffer = reinterpret_cast< DWORD* >( pBuffer );
		const DWORD* const pDwordHashS = reinterpret_cast< DWORD* >( pHashS );
		const DWORD* const pDwordHashT = reinterpret_cast< DWORD* >( pHashT );
		for ( DWORD nPosition = 0; nPosition < nEnd; ++nPosition )
		{
			if ( ( pDwordBuffer[ nPosition ]
				= pDwordHashS[ nPosition ] ^ pDwordHashT[ nPosition ] ) != 0 ) bChanged = TRUE;
		}
		if ( bChanged ) std::memcpy( pHashS, pHashT, ( m_nHash + 31 ) / 8 );
	}

	if ( bChanged == FALSE && m_bLive )
	{
		delete [] pBuffer;
		return FALSE;
	}

	DWORD nCompress = 0;
	auto_array< BYTE > pCompress( CZLib::Compress( pBuffer, m_nHash / ( 8 / nBits ), &nCompress ) );

	delete [] pBuffer;

	if ( !pCompress.get() )
		return FALSE;

	DWORD nPacketSize	= 1024;
	BYTE nSequenceMax	= (BYTE)( nCompress / nPacketSize );
	if ( nCompress % nPacketSize ) nSequenceMax++;

	pBuffer = pCompress.get();

	for ( BYTE nSequence = 1 ; nSequence <= nSequenceMax ; nSequence++ )
	{
		CPacket* pPatch = NULL;

		if ( pNeighbour->m_nProtocol == PROTOCOL_G1 )
			pPatch = CG1Packet::New( G1_PACKET_QUERY_ROUTE, 1 );
		else
			pPatch = CG2Packet::New( G2_PACKET_QHT );

		pPatch->WriteByte( 1 );
		pPatch->WriteByte( nSequence );
		pPatch->WriteByte( nSequenceMax );
		pPatch->WriteByte( 1 );
		pPatch->WriteByte( nBits );

		DWORD nPacket = min( nCompress, nPacketSize );

		pPatch->Write( pBuffer, nPacket );

		pBuffer += nPacket;
		nCompress -= nPacket;

		pNeighbour->Send( pPatch );
	}

	m_bLive = TRUE;

	return TRUE;
}

const bool CQueryHashTable::PatchToOldShareaza(CQueryHashTable* pTarget, CNeighbour* pNeighbour)
{
	DWORD nPacketSize = 4096;

	BYTE* pBuffer	= new BYTE[ nPacketSize ];
	BYTE* pHashT	= pTarget->m_pHash;
	BYTE* pHashS	= m_pHash;
	DWORD nPosition	= 0;

	for ( BYTE nSequence = 1 ; nPosition < m_nHash ; nSequence++ )
	{
		CG1Packet* pPatch = CG1Packet::New( G1_PACKET_QUERY_ROUTE, 1 );

		pPatch->WriteByte( 1 );
		pPatch->WriteByte( nSequence );
		pPatch->WriteByte( (BYTE)( m_nHash / nPacketSize ) );
		pPatch->WriteByte( 1 );
		pPatch->WriteByte( 4 );

		BYTE nMask = 1;

		DWORD nCount = 0;
		for ( ; nCount < nPacketSize ; nCount++, nPosition++ )
		{
			BYTE nPatch = ( *pHashT & nMask ) != ( *pHashS & nMask ) ? 7 : 0;

			*pHashS = ( *pHashS & ~nMask ) | ( *pHashT & nMask );

			if ( nCount & 1 )
				pBuffer[ nCount >> 1 ] |= nPatch;
			else
				pBuffer[ nCount >> 1 ] = nPatch << 4;

			if ( nMask == 0x80 )
			{
				nMask = 1;
				pHashT ++;
				pHashS ++;
			}
			else
			{
				nMask <<= 1;
			}
		}

		pPatch->WriteZLib( pBuffer, nCount / 2 );
		pNeighbour->Send( pPatch );
	}

	delete [] pBuffer;
	m_bLive = TRUE;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CQueryHashTable packet handler

const bool CQueryHashTable::OnPacket(CPacket* pPacket)
{
	if ( pPacket->m_nLength < 1 ) return FALSE;

	if ( pPacket->m_nProtocol == PROTOCOL_G1 )
	{
		CG1Packet* pG1 = (CG1Packet*)pPacket;
		if ( pG1->m_nTTL != 1 )   return FALSE;
		if ( pG1->m_nHops != 0 )  return FALSE;
	}
	else
	{
		CG2Packet* pG2 = (CG2Packet*)pPacket;
		DWORD nLength = pG2->m_nLength;
		if ( pG2->m_bCompound ) pG2->SkipCompound( nLength );
	}

	BYTE nVariant = pPacket->ReadByte();

	if ( nVariant == 0 )
	{
		return OnReset( pPacket );
	}
	else if ( nVariant == 1 )
	{
		return OnPatch( pPacket );
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CQueryHashTable reset handler

const bool CQueryHashTable::OnReset(CPacket* pPacket)
{
	if ( pPacket->m_nLength != 6 ) return FALSE;

	DWORD nHashSize	= 0;

	BOOL bGrouped = m_pGroup != NULL;
	if ( bGrouped ) QueryHashMaster.Remove( this );

	if ( pPacket->m_nProtocol == PROTOCOL_G1 )
	{
		nHashSize	= pPacket->ReadLongLE();
		m_nInfinity	= pPacket->ReadByte();
	}
	else
	{
		nHashSize	= pPacket->ReadLongBE();
		m_nInfinity	= pPacket->ReadByte();
	}

	if ( nHashSize < 64 ) return FALSE;	// Minimum size

	if ( nHashSize != m_nHash || m_pHash == NULL )
	{
		if ( m_pHash ) delete [] m_pHash;
		m_pHash = NULL;

		for ( m_nHash = 1, m_nBits = 0 ; m_nHash < nHashSize ; m_nBits++ ) m_nHash *= 2;
		if ( m_nHash != nHashSize ) return FALSE;
		if ( m_nBits > 24 ) return FALSE;

		m_pHash	= new BYTE[ ( m_nHash + 31 ) / 8 ];
	}

	FillMemory( m_pHash, ( m_nHash + 31 ) / 8, 0xFF );

	if ( bGrouped ) QueryHashMaster.Add( this );

	m_bLive		= FALSE;
	m_nCookie	= GetTickCount();
	m_nCount	= 0;

	m_pBuffer->Clear();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CQueryHashTable patch handler

const bool CQueryHashTable::OnPatch(CPacket* pPacket)
{
	if ( pPacket->m_nLength < 5 ) return FALSE;
	if ( m_pHash == NULL ) return FALSE;
	if ( m_pBuffer == NULL ) return FALSE;

	BYTE nSequence		= pPacket->ReadByte();
	BYTE nMaximum		= pPacket->ReadByte();
	BYTE nCompression	= pPacket->ReadByte();
	BYTE nBits			= pPacket->ReadByte();

	if ( nBits != 1 && nBits != 4 && nBits != 8 ) return FALSE;
	if ( nSequence < 1 || nSequence > nMaximum ) return FALSE;
	if ( nCompression > 1 ) return FALSE;

	if ( nSequence == 1 ) m_pBuffer->Clear();

	m_pBuffer->Add(	pPacket->m_pBuffer + pPacket->m_nPosition,
					pPacket->m_nLength - pPacket->m_nPosition );

	if ( nSequence < nMaximum ) return TRUE;

	if ( nCompression == 1 )
	{
		if ( ! m_pBuffer->Inflate() )
		{
			m_pBuffer->Clear();
			return FALSE;
		}
	}

	if ( m_pBuffer->m_nLength != m_nHash / ( 8 / nBits ) )
	{
		m_pBuffer->Clear();
		return FALSE;
	}

	BYTE* pData		= m_pBuffer->m_pBuffer;
	BYTE* pHash		= m_pHash;

	BOOL bGroup		= ( m_pGroup != NULL && m_pGroup->m_nHash == m_nHash );
	BYTE* pGroup	= bGroup ? m_pGroup->m_pHash : NULL;

	if ( nBits == 1 )
	{
		for ( DWORD nPosition = ( m_nHash >> 3 ) ; nPosition ; nPosition--, pHash++, pData++ )
		{
			for ( BYTE nMask = 1 ; ; nMask <<= 1 )
			{
				if ( *pData & nMask )
				{
					if ( *pHash & nMask )
					{
						m_nCount ++;
						*pHash &= ~nMask;
						if ( bGroup )
						{
#ifdef _DEBUG
							ASSERT( *pGroup < 255 );
							if ( *pGroup == 0 ) m_pGroup->m_nCount++;
#endif
							(*pGroup) ++;
						}
					}
					else
					{
						m_nCount --;
						*pHash |= nMask;

						if ( bGroup )
						{
#ifdef _DEBUG
							ASSERT( *pGroup );
							if ( *pGroup == 1 ) m_pGroup->m_nCount--;
#endif
							(*pGroup) --;
						}
					}
				}

				pGroup++;
				if ( nMask == 0x80 ) break;
			}
		}
	}
	else
	{
		BYTE nMask = 1;

		for ( DWORD nPosition = 0 ; nPosition < m_nHash ; nPosition++, pData++, pGroup++ )
		{
			if ( nBits == 8 )
			{
				if ( *pData )
				{
					if ( *pHash & nMask )
					{
						m_nCount++;
						*pHash &= ~nMask;
						if ( bGroup ) (*pGroup) ++;
					}
					else
					{
						m_nCount--;
						*pHash |= nMask;
						if ( bGroup ) (*pGroup) --;
					}
				}
			}
			else
			{
				if ( *pData & 0xF0 )
				{
					if ( *pHash & nMask )
					{
						m_nCount++;
						*pHash &= ~nMask;
						if ( bGroup ) (*pGroup) ++;
					}
					else
					{
						m_nCount--;
						*pHash |= nMask;
						if ( bGroup ) (*pGroup) --;
					}
				}

				if ( ++nPosition >= m_nHash ) return FALSE;
				pGroup ++;
				nMask <<= 1;

				if ( *pData & 0x0F )
				{
					if ( *pHash & nMask )
					{
						m_nCount++;
						*pHash &= ~nMask;
						if ( bGroup ) (*pGroup) ++;
					}
					else
					{
						m_nCount--;
						*pHash |= nMask;
						if ( bGroup ) (*pGroup) --;
					}
				}
			}

			if ( nMask == 0x80 )
			{
				pHash++;
				nMask = 1;
			}
			else
			{
				nMask <<= 1;
			}
		}
	}

	m_bLive		= TRUE;
	m_nCookie	= GetTickCount();

	if ( bGroup ) QueryHashMaster.Invalidate();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CQueryHashTable add strings and words

const int CQueryHashTable::AddString(const CString& strString)
{
	if ( m_pHash == NULL ) return 0;
	return Add( strString, 0, strString.GetLength() );
}

const int CQueryHashTable::AddExactString(const CString& strString)
{
	if ( m_pHash == NULL ) return 0;
	return AddExact( strString, 0, strString.GetLength() );
}

const int CQueryHashTable::Add(LPCTSTR pszString, size_t nStart, size_t nLength)
{
	bool bWord = IsWord( pszString, nStart, nLength );
	if ( ! nLength || !bWord && nLength < 4 )
		return 0;

	if ( !bWord )
		return AddExact( pszString, nStart, nLength );

	m_nCookie = GetTickCount();

	DWORD nHash	= HashWord( pszString, nStart, nLength, m_nBits );
	BYTE* pHash	= m_pHash + ( nHash >> 3 );
	BYTE nMask	= BYTE( 1 << ( nHash & 7 ) );

	if ( *pHash & nMask )
	{
		m_nCount++;
		*pHash &= ~nMask;
	}

	if ( nLength >= 5 )
	{
		nHash	= HashWord( pszString, nStart, nLength - 1, m_nBits );
		pHash	= m_pHash + ( nHash >> 3 );
		nMask	= BYTE( 1 << ( nHash & 7 ) );

		if ( *pHash & nMask )
		{
			m_nCount++;
			*pHash &= ~nMask;
		}

		nHash	= HashWord( pszString, nStart, nLength - 2, m_nBits );
		pHash	= m_pHash + ( nHash >> 3 );
		nMask	= BYTE( 1 << ( nHash & 7 ) );

		if ( *pHash & nMask )
		{
			m_nCount++;
			*pHash &= ~nMask;
		}

		return 3;
	}

	return 1;
}

const int CQueryHashTable::AddExact(LPCTSTR pszString, size_t nStart, size_t nLength)
{
	if ( ! nLength )
		return 0;

	m_nCookie = GetTickCount();
	DWORD nHash	= HashWord( pszString, nStart, nLength, m_nBits );
	BYTE* pHash	= m_pHash + ( nHash >> 3 );
	BYTE nMask	= BYTE( 1 << ( nHash & 7 ) );

	if ( *pHash & nMask )
	{
		m_nCount++;
		*pHash &= ~nMask;
	}

	return 1;
}

const bool CQueryHashTable::CheckString(const CString& strString) const
{
	if ( ! m_bLive || m_pHash == NULL || strString.IsEmpty() ) return TRUE;

	DWORD nHash	= HashWord( strString, 0, strString.GetLength(), m_nBits );
	BYTE* pHash	= m_pHash + ( nHash >> 3 );
	BYTE nMask	= BYTE( 1 << ( nHash & 7 ) );

	return ! ( *pHash & nMask );
}

const bool CQueryHashTable::CheckHash(const DWORD nHash) const
{
	if ( ! m_bLive || m_pHash == NULL ) return TRUE;

	DWORD lHash	= nHash >> (32 - m_nBits);
	BYTE* pHash	= m_pHash + ( lHash >> 3 );
	BYTE nMask	= BYTE( 1 << ( lHash & 7 ) );

	return ! ( *pHash & nMask );
}


//////////////////////////////////////////////////////////////////////
// CQueryHashTable check query object

const bool CQueryHashTable::Check(const CQuerySearch& oSearch) const
{
	if ( ! m_bLive || m_pHash == NULL ) return TRUE;

	if ( !oSearch.m_oURNs.empty() )
	{
		CQuerySearch::const_hash_iterator iUrnEnd( oSearch.urnEnd() );
		for ( CQuerySearch::const_hash_iterator iUrn( oSearch.urnBegin() )
			; iUrn != iUrnEnd ; ++iUrn )
		{
			if ( CheckHash(*iUrn) ) return TRUE;
		}
		return FALSE;
	}

	DWORD nWordHits		= 0;
	DWORD nWords		= 0;

	if ( !oSearch.m_oKeywordHashList.empty() )
	{
		CQuerySearch::const_hash_iterator iKeywordEnd( oSearch.keywordEnd() );
		for ( CQuerySearch::const_hash_iterator iKeyword( oSearch.keywordBegin() )
			; iKeyword != iKeywordEnd ; ++iKeyword )
		{
			nWords++;
			if ( CheckHash(*iKeyword) ) nWordHits++;
		}

	}

	return ( nWords >= 3 )
		? nWordHits * 3 / nWords >= 2 // at least 2/3 matches
		: nWordHits == nWords;
}

//////////////////////////////////////////////////////////////////////
// CQueryHashTable hash functions

const DWORD CQueryHashTable::HashWord(LPCTSTR pszString, size_t nStart, size_t nLength, DWORD nBits)
{
	DWORD nNumber	= 0;
	int nByte		= 0;

	for ( pszString += nStart; nLength ; nLength--, pszString++ )
	{
		// A known bad using of tolower() with unicode chars but as is...
		int nValue = tolower( *pszString ) & 0xFF;

		nValue = nValue << ( nByte * 8 );
		nByte = ( nByte + 1 ) & 3;

		nNumber = nNumber ^ nValue;
	}

	return HashNumber( nNumber, nBits );
}

const DWORD CQueryHashTable::HashNumber(DWORD nNumber, int nBits)
{
	unsigned __int64 nProduct	= (unsigned __int64)nNumber * (unsigned __int64)0x4F1BBCDC;
	unsigned __int64 nHash		= nProduct << 32;
	nHash = nHash >> ( 32 + ( 32 - nBits ) );
	return (DWORD)nHash;
}

//////////////////////////////////////////////////////////////////////
// CQueryHashTable calculate percent full

const int CQueryHashTable::GetPercent() const
{
	if ( ! m_pHash || ! m_nHash ) return 0;
	return m_nCount * 100 / m_nHash;
}

void CQueryHashTable::Draw(HDC hDC, const RECT* pRC)
{
	if ( ! m_pHash ) return;
	SetStretchBltMode( hDC, HALFTONE );
	BITMAP bm = { 0, 1024, 1024, 128, 1, 1, m_pHash };
	HBITMAP hBmp = CreateBitmapIndirect( &bm );
	HDC hMemDC = CreateCompatibleDC( hDC );
	HBITMAP hOldBmp = (HBITMAP)SelectObject( hMemDC, hBmp );
	StretchBlt( hDC, pRC->left, pRC->top, pRC->right - pRC->left, pRC->bottom - pRC->top,
		hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY );
	SelectObject( hMemDC, hOldBmp );
	DeleteDC( hMemDC );
	DeleteObject( hBmp );
}
