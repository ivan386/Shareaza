//
// HashTest.cpp
//
// Copyright (c) Shareaza Development Team, 2009-2011.
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

// For ToLower()
//#include "..\Shareaza\Strings.h"
//#include "..\Shareaza\Strings.cpp"


__int64 GetMicroCount() throw()
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

// HashWord() function from Shareaza sources
DWORD HashWord(LPCTSTR pszString, size_t nStart, size_t nLength, DWORD nBits) throw()
{
	DWORD nNumber	= 0;
	int nByte		= 0;

	for ( pszString += nStart; nLength ; --nLength, ++pszString )
	{
		nNumber ^= ( tolower( *pszString ) & 0xFF ) << ( nByte * 8 );
		nByte = ( nByte + 1 ) & 3;
	}

	return ( nNumber * 0x4F1BBCDC ) >> ( 32 - nBits );
}


class InitGetMicroCount
{
public:
	inline InitGetMicroCount() throw() { GetMicroCount(); }
};

static InitGetMicroCount initGetMicroCount;

int _tmain(int /*argc*/, _TCHAR* /*argv*/[])
{
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

	_tprintf( _T("\nHashWord() function tests:\n\n") );
	_tprintf( _T("Hash test 1... %s\n"),
		( HashWord( L"\x30a2\x30cb\x30e1", 0,  3, 10 ) ==  46 ) ? _T("OK") : _T("FAIL") );
	_tprintf( _T("Hash test 2... %s\n"),
		( HashWord( L"\x58f0\x512a",       0,  2, 10 ) == 731 ) ? _T("OK") : _T("FAIL") );
	_tprintf( _T("Hash test 3... %s\n"),
		( HashWord( L"\x0001\x0028",       0,  2, 10 ) == 658 ) ? _T("OK") : _T("FAIL") );
	_tprintf( _T("Hash test 4... %s\n"),
		( HashWord( L"\xff01\x9428",       0,  2, 10 ) == 658 ) ? _T("OK") : _T("FAIL") );
	_tprintf( _T("Hash test 5... %s\n"),
		( HashWord( L"\x0000",             0,  1, 10 ) ==   0 ) ? _T("OK") : _T("FAIL") );
	_tprintf( _T("Hash test 6... %s\n"),
		( HashWord( L"\x0001",             0,  1, 10 ) == 316 ) ? _T("OK") : _T("FAIL") );
// No UTF-32 support for unmanaged C++
//	_tprintf( _T("Hash test 6... %s\n"),
//		( HashWord( L"\x10400",            0,  1, 10 ) == 316 ) ? _T("OK") : _T("FAIL") );
//	_tprintf( _T("Hash test 7... %s\n"),
//		( HashWord( L"\x10428",            0,  1, 10 ) == 658 ) ? _T("OK") : _T("FAIL") );

	_tprintf( _T("\nMeasuring performance:\n\n") );
	const int nCount = 100;
	const __int64 nBlock = 10 * 1024 * 1024;	// 10 MB
	LPVOID pBuffer = VirtualAlloc( NULL, nBlock, MEM_COMMIT, PAGE_READWRITE );
	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST );

	{
		__int64 nBest = 0, nError = 0;
		do
		{
			__int64 nWorst = 0;
			memset( pBuffer, nBlock, 0xf0 );
			_tprintf( _T("HashWord : %I64d MB by "), nBlock / 1024 / 1024 );
			DWORD foo = 0;
			for ( int i = 0; i < nCount; ++i )
			{
				const __int64 nBegin = GetMicroCount();
				foo ^= HashWord( (LPCTSTR)pBuffer, 0, nBlock / sizeof( TCHAR ), 20 );
				__int64 nTime = GetMicroCount() - nBegin;
				if ( i == 0 || nTime < nBest )
					nBest = nTime;
				if ( i == 0 || nTime > nWorst )
					nWorst = nTime;
			}
			nError = ( 100 * ( nWorst - nBest ) ) / nWorst;
			const __int64 nSpeed = ( nBlock * 1000000 ) / nBest;
			_tprintf( _T("%3I64d ms (inaccuracy %2I64d%%), %3I64d MB/s        \r"),
				nBest / 1000, nError, nSpeed / ( 1024 * 1024 ) );
		} while ( nError > 9 );
	}
	_tprintf( _T("\n") );

	{
		__int64 nBest = 0, nError = 0;
		do
		{
			__int64 nWorst = 0;
			memset( pBuffer, nBlock, 0xf0 );
			_tprintf( _T("MD4  hash: %I64d MB by "), nBlock / 1024 / 1024 );
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
			nError = ( 100 * ( nWorst - nBest ) ) / nWorst;
			const __int64 nSpeed = ( nBlock * 1000000 ) / nBest;
			_tprintf( _T("%3I64d ms (inaccuracy %2I64d%%), %3I64d MB/s        \r"),
				nBest / 1000, nError, nSpeed / ( 1024 * 1024 ) );
		} while ( nError > 9 );
	}
	_tprintf( _T("\n") );

	{
		__int64 nBest = 0, nError = 0;
		do
		{
			__int64 nWorst = 0;
			memset( pBuffer, nBlock, 0xf0 );
			_tprintf( _T("MD5  hash: %I64d MB by "), nBlock / 1024 / 1024 );
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
			nError = ( 100 * ( nWorst - nBest ) ) / nWorst;
			const __int64 nSpeed = ( nBlock * 1000000 ) / nBest;
			_tprintf( _T("%3I64d ms (inaccuracy %2I64d%%), %3I64d MB/s        \r"),
				nBest / 1000, nError, nSpeed / ( 1024 * 1024 ) );
		} while ( nError > 9 );
	}
	_tprintf( _T("\n") );

	{
		__int64 nBest = 0, nError = 0;
		do
		{
			__int64 nWorst = 0;
			memset( pBuffer, nBlock, 0xf0 );
			_tprintf( _T("SHA1 hash: %I64d MB by "), nBlock / 1024 / 1024 );
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
			nError = ( 100 * ( nWorst - nBest ) ) / nWorst;
			const __int64 nSpeed = ( nBlock * 1000000 ) / nBest;
			_tprintf( _T("%3I64d ms (inaccuracy %2I64d%%), %3I64d MB/s        \r"),
				nBest / 1000, nError, nSpeed / ( 1024 * 1024 ) );
		} while ( nError > 9 );
	}
	_tprintf( _T("\n") );

	VirtualFree( pBuffer, 0, MEM_RELEASE );

	_tprintf( _T("\nPress ENTER to exit") );
	getchar();

	return 0;
}
