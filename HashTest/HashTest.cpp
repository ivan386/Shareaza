//
// HashTest.cpp
//
// Copyright (c) Shareaza Development Team, 2009-2010.
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
	const __int64 nCount = 100;
	const __int64 nBlock = 10 * 1024 * 1024;	// 10 MB

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

	SYSTEM_INFO si = {};
	GetSystemInfo( &si );
	_tprintf( _T("CPUs     : %u\n"), si.dwNumberOfProcessors );

	LPVOID pBuffer = VirtualAlloc( NULL, nBlock, MEM_COMMIT, PAGE_READWRITE );

	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST );

	_tprintf( _T("\n") );

	{
		ZeroMemory( pBuffer, nBlock );
		_tprintf( _T("MD4  hash: %I64d MB by "), nBlock / 1024 / 1024 );
		__int64 nBest = 0, nWorst = 0;
		for ( int i = 0; i < nCount; ++i )
		{
			const __int64 nBegin = GetMicroCount();
			CMD4 pMD4;
			pMD4.Add( pBuffer, nBlock );
			pMD4.Finish();
			__int64 nTime = GetMicroCount() - nBegin;
			if ( i == 0 || nTime < nBest )
				nBest = nTime;
			if ( i == 0 || nTime > nWorst )
				nWorst = nTime;
		}
		const __int64 nError = ( 100 * ( nWorst - nBest ) ) / nWorst;
		const __int64 nSpeed = ( nBlock * 1000000 ) / nBest;
		_tprintf( _T("%6I64d ms (error %I64d%%), %3I64d MB/s\n"),
			nBest / 1000, nError, nSpeed / ( 1024 * 1024 ) );
	}

	{
		ZeroMemory( pBuffer, nBlock );
		_tprintf( _T("MD5  hash: %I64d MB by "), nBlock / 1024 / 1024 );
		__int64 nBest = 0, nWorst = 0;
		for ( int i = 0; i < nCount; ++i )
		{
			const __int64 nBegin = GetMicroCount();
			CMD5 pMD5;
			pMD5.Add( pBuffer, nBlock );
			pMD5.Finish();
			__int64 nTime = GetMicroCount() - nBegin;
			if ( i == 0 || nTime < nBest )
				nBest = nTime;
			if ( i == 0 || nTime > nWorst )
				nWorst = nTime;
		}
		const __int64 nError = ( 100 * ( nWorst - nBest ) ) / nWorst;
		const __int64 nSpeed = ( nBlock * 1000000 ) / nBest;
		_tprintf( _T("%6I64d ms (error %I64d%%), %3I64d MB/s\n"),
			nBest / 1000, nError, nSpeed / ( 1024 * 1024 ) );
	}

	{
		ZeroMemory( pBuffer, nBlock );
		_tprintf( _T("SHA1 hash: %I64d MB by "), nBlock / 1024 / 1024 );
		__int64 nBest = 0, nWorst = 0;
		for ( int i = 0; i < nCount; ++i )
		{
			const __int64 nBegin = GetMicroCount();
			CSHA pSHA;
			pSHA.Add( pBuffer, nBlock );
			pSHA.Finish();
			__int64 nTime = GetMicroCount() - nBegin;
			if ( i == 0 || nTime < nBest )
				nBest = nTime;
			if ( i == 0 || nTime > nWorst )
				nWorst = nTime;
		}
		const __int64 nError = ( 100 * ( nWorst - nBest ) ) / nWorst;
		const __int64 nSpeed = ( nBlock * 1000000 ) / nBest;
		_tprintf( _T("%6I64d ms (error %I64d%%), %3I64d MB/s\n"),
			nBest / 1000, nError, nSpeed / ( 1024 * 1024 ) );
	}

	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_NORMAL );

	VirtualFree( pBuffer, 0, MEM_RELEASE );

	_tprintf( _T("\nPress ENTER to exit") );
	getchar();

	return 0;
}
