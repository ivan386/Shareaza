//
// HashTest.cpp
//
// Copyright (c) Shareaza Development Team, 2009.
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

#include "stdafx.h"
#include "..\HashLib\HashLib.h"

static __int64 GetMicroCount()
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

static InitGetMicroCount initGetMicroCount;

int _tmain(int /*argc*/, _TCHAR* /*argv*/[])
{
	const __int64 nCount = 1000;
	const __int64 nBlock = ( 4 * 1024 ) * 1024;	// 4 MB

#ifdef _WIN64
	_tprintf( _T("Platform : 64-bit\n") );
#else
	_tprintf( _T("Platform : 32-bit\n") );
#endif

#ifdef _DEBUG
	_tprintf( _T("Build    : Debug\n") );
#else
	_tprintf( _T("Build    : Release\n") );
#endif

	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST );

	SYSTEM_INFO si = {};
	GetSystemInfo( &si );
	_tprintf( _T("CPUs     : %u\n"), si.dwNumberOfProcessors );

	LPVOID pBuffer = VirtualAlloc( NULL, nBlock, MEM_COMMIT, PAGE_READWRITE );

	_tprintf( _T("\n") );

	{
		_tprintf( _T("MD4  hash: %I64d MB by "), ( nBlock * nCount ) / 1024 / 1024 );
		__int64 nBegin = GetMicroCount();
		CMD4 pMD4;
		for ( int i = 0; i < nCount; ++i )
			pMD4.Add( pBuffer, nBlock );
		pMD4.Finish();
		__int64 nTime = GetMicroCount() - nBegin;
		_tprintf( _T("%5I64d ms, %3I64d MB/s\n"),
			nTime / 1000, ( nBlock * nCount * 1000000 ) / ( nTime * 1024 * 1024 ) );
	}

	{
		_tprintf( _T("MD5  hash: %I64d MB by "), ( nBlock * nCount ) / 1024 / 1024 );
		__int64 nBegin = GetMicroCount();
		CMD5 pMD5;
		for ( int i = 0; i < nCount; ++i )
			pMD5.Add( pBuffer, nBlock );
		pMD5.Finish();
		__int64 nTime = GetMicroCount() - nBegin;
		_tprintf( _T("%5I64d ms, %3I64d MB/s\n"),
			nTime / 1000, ( nBlock * nCount * 1000000 ) / ( nTime * 1024 * 1024 ) );
	}

	{
		_tprintf( _T("SHA1 hash: %I64d MB by "), ( nBlock * nCount ) / 1024 / 1024 );
		__int64 nBegin = GetMicroCount();
		CSHA pSHA;
		for ( int i = 0; i < nCount; ++i )
			pSHA.Add( pBuffer, nBlock );
		pSHA.Finish();
		__int64 nTime = GetMicroCount() - nBegin;
		_tprintf( _T("%5I64d ms, %3I64d MB/s\n"),
			nTime / 1000, ( nBlock * nCount * 1000000 ) / ( nTime * 1024 * 1024 ) );
	}

	 VirtualFree( pBuffer, 0, MEM_RELEASE );

	_tprintf( _T("\nPress ENTER to exit") );
	getchar();

	return 0;
}
