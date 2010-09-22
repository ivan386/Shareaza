//
// StdAfx.cpp
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

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
