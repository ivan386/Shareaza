//
// QueryKeys.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "shareaza.h"
#include "QueryKeys.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CQueryKeys construction

CQueryKeys::CQueryKeys()
{
	m_nBits		= 12;
	m_nTable	= 1u << m_nBits;
	m_pTable	= new DWORD[ m_nTable ];
	m_pMap		= new DWORD[ m_nBits * 2 ];

	DWORD* pMap = m_pMap;

	for ( DWORD nCount = m_nBits ; nCount ; nCount-- )
	{
		*pMap++ = 1 << ( rand() % 32 );
		*pMap++ = 1 << ( rand() % 32 );
	}

	BYTE* pFill = (BYTE*)m_pTable;

	for ( DWORD nCount = m_nTable ; nCount ; nCount-- )
	{
		*pFill++ = (BYTE)( rand() & 0xFF );
		*pFill++ = (BYTE)( rand() & 0xFF );
		*pFill++ = (BYTE)( rand() & 0xFF );
		*pFill++ = (BYTE)( rand() & 0xFF );
	}

	// TODO: Add check for invalid (for Shareaza) zero keys
}

CQueryKeys::~CQueryKeys()
{
	delete [] m_pMap;
	delete [] m_pTable;
}

//////////////////////////////////////////////////////////////////////
// CQueryKeys create or lookup

DWORD CQueryKeys::Create(DWORD nAddress)
{
	DWORD* pMap = m_pMap;
	DWORD nHash = 0;

	for ( DWORD nCount = m_nBits, nBit = 1 ; nCount ; nCount--, nBit <<= 1 )
	{
		BOOL bOne = ( nAddress & (*pMap++) ) != 0;
		BOOL bTwo = ( nAddress & (*pMap++) ) != 0;
		if ( bOne ^ bTwo ) nHash |= nBit;
	}

	return m_pTable[ nHash & ( m_nTable - 1 ) ];
}

//////////////////////////////////////////////////////////////////////
// CQueryKeys check

BOOL CQueryKeys::Check(DWORD nAddress, DWORD nKey)
{
	DWORD* pMap = m_pMap;
	DWORD nHash = 0;

	for ( DWORD nCount = m_nBits, nBit = 1 ; nCount ; nCount--, nBit <<= 1 )
	{
		BOOL bOne = ( nAddress & (*pMap++) ) != 0;
		BOOL bTwo = ( nAddress & (*pMap++) ) != 0;
		if ( bOne ^ bTwo ) nHash |= nBit;
	}

	return nKey == m_pTable[ nHash & ( m_nTable - 1 ) ];
}

