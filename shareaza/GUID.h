//
// GUID.h
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

#if !defined(AFX_GUID_H__3ABC5B39_501F_41B0_828A_B7CDFAD0F73B__INCLUDED_)
#define AFX_GUID_H__3ABC5B39_501F_41B0_828A_B7CDFAD0F73B__INCLUDED_

#pragma once

#include "StdAfx.h"
#include "Hashes.h"

class CGUIDBT;

#define GUID_SIZE	16

class CGUID
{
public:
	union
	{
		BYTE	m_b[ 16 ];
		DWORD	m_d[ 4 ];
		QWORD	m_q[ 2 ];
	};
	inline	CGUID();
	inline	CGUID(const CGUID &oGUID);
	inline	CGUID(const CGUIDBT &oGUIDBT);
	inline	~CGUID();
	inline	void	operator = (const CGUID &oGUID);
	inline	void	operator = (const CGUIDBT &oGUIDBT);
	inline	BOOL	operator == (const CGUID &oGUID) const;
	inline	BOOL	operator != (const CGUID &oGUID) const;
};

#define GUIDBT_SIZE 20

class CGUIDBT : public CHashBT
{
public:
	inline	CGUIDBT();
	inline	CGUIDBT(const CGUID &oGUID);
	inline	~CGUIDBT();
	inline	void	operator = (const CGUID &oGUID);
	inline	BOOL	IsShareaza() const;
	inline	CString	GetUserAgent() const;
	inline	BOOL	IsEmpty() const;
};

inline CGUID::CGUID()
{
}

inline CGUID::CGUID(const CGUID &oGUID)
{
	CopyMemory( &m_b, &oGUID.m_b, sizeof m_b );
}

inline CGUID::CGUID(const CGUIDBT &oGUIDBT)
{
	CopyMemory( &m_b, &oGUIDBT.m_b, sizeof m_b );
}

inline CGUID::~CGUID()
{
}

inline void CGUID::operator = (const CGUID &oGUID)
{
	CopyMemory( &m_b, &oGUID.m_b, sizeof m_b );
}

inline void CGUID::operator = (const CGUIDBT &oGUIDBT)
{
	CopyMemory( &m_b, &oGUIDBT.m_b, sizeof m_b );
}

inline BOOL CGUID::operator == (const CGUID &oGUID) const
{
	return memcmp( &m_b, &oGUID.m_b, sizeof m_b ) == 0;
}

inline BOOL CGUID::operator != (const CGUID &oGUID) const
{
	return memcmp( &m_b, &oGUID.m_b, sizeof m_b ) != 0;
}

inline CGUIDBT::CGUIDBT()
{
}

inline CGUIDBT::CGUIDBT(const CGUID &oGUID)
{
	srand( GetTickCount() );
	m_b[ 0 ] = 'R';
	m_b[ 1 ] = 'A';
	m_b[ 2 ] = 'Z';
	m_b[ 3 ] = 'A';
	m_b[ 4 ] = '0' + theApp.m_nVersion[ 0 ];
	m_b[ 5 ] = '0' + theApp.m_nVersion[ 1 ];
	m_b[ 6 ] = '0' + theApp.m_nVersion[ 2 ];
	m_b[ 7 ] = '0' + theApp.m_nVersion[ 3 ];
	int nByte = 8;
	do
	{
		m_b[ nByte ] = rand();
	}
	while ( ++nByte < 16 );
	m_d[ 4 ] = m_d[ 0 ] ^ _byteswap_ulong( m_d[ 3 ] );
}

inline CGUIDBT::~CGUIDBT()
{
}

inline void CGUIDBT::operator = (const CGUID &oGUID)
{
	srand( GetTickCount() );
	m_b[ 0 ] = 'R';
	m_b[ 1 ] = 'A';
	m_b[ 2 ] = 'Z';
	m_b[ 3 ] = 'A';
	m_b[ 4 ] = '0' + theApp.m_nVersion[ 0 ];
	m_b[ 5 ] = '0' + theApp.m_nVersion[ 1 ];
	m_b[ 6 ] = '0' + theApp.m_nVersion[ 2 ];
	m_b[ 7 ] = '0' + theApp.m_nVersion[ 3 ];
	int nByte = 8;
	do
	{
		m_b[ nByte ] = rand();
	}
	while ( ++nByte < 16 );
	m_d[ 4 ] = m_d[ 0 ] ^ _byteswap_ulong( m_d[ 3 ] );
}

inline BOOL CGUIDBT::IsShareaza() const
{
	return ( ( ( m_q[ 0 ] | m_q[ 1 ] ) != 0 ) && ( m_d[ 4 ] == ( m_d[ 0 ] ^ _byteswap_ulong( m_d[ 3 ] ) ) ) );
}

inline CString CGUIDBT::GetUserAgent() const
{
	CString strAgent;
	if ( m_b[ 0 ] == '-' && m_b[ 7 ] == '-' )
	{
		strAgent.Format( _T(" %i.%i.%i.%i"), m_b[ 3 ] - '0', m_b[ 4 ] - '0', m_b[ 5 ] - '0', m_b[ 6 ] - '0' );
		     if ( m_b[ 1 ] == 'A' && m_b[ 2 ] == 'Z' ) strAgent = _T("Azureus") + strAgent;
		else if ( m_b[ 1 ] == 'M' && m_b[ 2 ] == 'T' ) strAgent = _T("MoonlightTorrent") + strAgent;
		else if ( m_b[ 1 ] == 'L' && m_b[ 2 ] == 'T' ) strAgent = _T("libtorrent") + strAgent;
		else if ( m_b[ 1 ] == 'B' && m_b[ 2 ] == 'X' ) strAgent = _T("Bittorrent X") + strAgent;
		else if ( m_b[ 1 ] == 'T' && m_b[ 2 ] == 'S' ) strAgent = _T("Torrentstorm") + strAgent;
		else if ( m_b[ 1 ] == 'S' && m_b[ 2 ] == 'S' ) strAgent = _T("Swarmscope") + strAgent;
		else if ( m_b[ 1 ] == 'X' && m_b[ 2 ] == 'T' ) strAgent = _T("XanTorrent") + strAgent;
		else if ( m_b[ 1 ] == 'B' && m_b[ 2 ] == 'B' ) strAgent = _T("BitBuddy") + strAgent;
		else if ( m_b[ 1 ] == 'T' && m_b[ 2 ] == 'N' ) strAgent = _T("TorrentDOTnet") + strAgent;
		else strAgent.Format( _T("%c%c") + strAgent, m_b[ 1 ], m_b[ 2 ] );	// Unknown client using this naming.
	}
	else if ( m_b[ 4 ] == '-' && m_b[ 5 ] == '-' && m_b[ 6 ] == '-' && m_b[ 7 ] == '-' )
	{
		strAgent.Format( _T(" %i%i%i"), m_b[ 1 ] - '0', m_b[ 2 ] - '0', m_b[ 3 ] - '0' );
			 if ( m_b[ 0 ] == 'A' ) strAgent = _T("ABC") + strAgent;
		else if ( m_b[ 0 ] == 'S' ) strAgent = _T("Shadow") + strAgent;
		else if ( m_b[ 0 ] == 'T' ) strAgent = _T("BitTornado") + strAgent;
		else if ( m_b[ 0 ] == 'U' ) strAgent = _T("UPnP NAT BT") + strAgent;
		else strAgent.Format( _T("%c") + strAgent, m_b[ 0 ] );				// Unknown client using this naming.
	}
	return strAgent;
}

inline BOOL CGUIDBT::IsEmpty() const
{
	return ( m_d[ 0 ] | m_d[ 1 ] | m_d[ 2 ] | m_d[ 3 ] | m_d[ 4 ] ) == 0;
}

#endif // !defined(AFX_GUID_H__3ABC5B39_501F_41B0_828A_B7CDFAD0F73B__INCLUDED_)
