//
// StdAfx.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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

#pragma warning ( disable : 4201 )	// nonstandard extension used : nameless struct / union
#pragma warning ( disable : 4574 )	// '...' is defined to be '0': did you mean to use '#if ...'?
#pragma warning ( disable : 4986 )	// exception specification does not match previous declaration

#include "StdAfx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const LPCTSTR protocolNames[] =
{
	_T(""),
	_T("Gnutella1"),
	_T("Gnutella2"),
	_T("eDonkey2000"),
	_T("HTTP"),
	_T("FTP"),
	_T("BitTorrent"),
	_T("Kademlia"),
	_T("DC++")
};

const LPCTSTR protocolAbbr[] =
{
	_T(""),
	_T("G1"),
	_T("G2"),
	_T("ED2K"),
	_T("HTTP"),
	_T("FTP"),
	_T("BT"),
	_T("KAD"),
	_T("DC++")
};

// Protocol resource IDs (for icons)
const UINT protocolIDs[] =
{
	ID_NETWORK_NULL,
	ID_NETWORK_G1,
	ID_NETWORK_G2,
	ID_NETWORK_ED2K,
	ID_NETWORK_HTTP,
	ID_NETWORK_FTP,
	ID_NETWORK_BT,
	ID_NETWORK_KAD,
	ID_NETWORK_DC,
	NULL
};

// Protocol default ports
const WORD protocolPorts[] =
{
	6346,	// Unknown
	6346,	// Gnutella
	6346,	// Gnutella2
	4661,	// eDonkey2000
	80,		// HTTP
	21,		// FTP
	6881,	// BitTorrent
	4662,	// Kademlia
	411,	// DC++
	0
};

__int64 GetMicroCount()
{
	static __int64 Freq = 0;
	static __int64 FirstCount = 0;
	if ( Freq < 0 )
	{
		return GetTickCount() * 1000;
	}
	if ( Freq == 0 )
	{
		if ( ! QueryPerformanceFrequency( (LARGE_INTEGER*)&Freq ) )
		{
			Freq = -1;
			return GetMicroCount();
		}
		QueryPerformanceCounter( (LARGE_INTEGER*)&FirstCount );
	}
	__int64 Count = 0;
	QueryPerformanceCounter( (LARGE_INTEGER*)&Count );
	return ( 1000000 * ( Count - FirstCount ) ) / Freq;
}

class InitGetMicroCount
{
public:
	inline InitGetMicroCount() throw() { GetMicroCount(); }
};

InitGetMicroCount initGetMicroCount;

static const UINT primes[] =
{
	11,			13,			17,			19,			23,			29,
	31,			61,			127,		251,		347,		509,
	631,		761,		887,		1021,		1531,		2039,
	3067,		4093,		5119,		6143,		7159,		8191,
	9209,		10223,		11261,		12227,		13309,		14327,
	16381,		20479,		24571,		28669,		32749,		49139,
	65521,		98299,		131071,		196597,		262139,		327673
};

UINT GetBestHashTableSize(UINT nCount)
{
	const UINT* last  = primes + ( sizeof( primes ) / sizeof( primes[ 0 ] ) - 1 );
	const UINT value  = ( nCount + nCount / 5 );
	return * std::lower_bound( primes, last, value, std::less< UINT >() );	// + 20%
}

// Disable exceptions if the memory allocation fails

class NoThrowNew
{
public:
	NoThrowNew() throw()
	{
		std::set_new_handler( &NoThrowNew::OutOfMemoryHandlerStd );
		_set_new_handler( &NoThrowNew::OutOfMemoryHandler );
		AfxSetNewHandler( &NoThrowNew::OutOfMemoryHandlerAfx );
	}

private:
	static void __cdecl OutOfMemoryHandlerStd() throw()
	{
	}

	static int __cdecl OutOfMemoryHandler(size_t /* nSize */) throw()
	{
		return 0;
	}

	static int __cdecl OutOfMemoryHandlerAfx(size_t /* nSize */) throw()
	{
		return 0;
	}
};

NoThrowNew initNoThrowNew;
