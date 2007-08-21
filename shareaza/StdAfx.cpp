//
// StdAfx.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const CLowerCaseTable ToLower;

CLowerCaseTable::CLowerCaseTable()
{
	for ( size_t i = 0; i < 65536; ++i ) cTable[ i ] = TCHAR( i );
	cTable[ 65536 ] = 0;
	CharLower( &cTable[ 1 ] );
	// turkish capital I with dot is converted to "i"
	cTable[ 304 ] = 105;
	// convert fullwidth latin characters to halfwidth
	for ( size_t i = 65281 ; i < 65313 ; ++i ) cTable[ i ] = TCHAR( i - 65248 );
	for ( size_t i = 65313 ; i < 65339 ; ++i ) cTable[ i ] = TCHAR( i - 65216 );
	for ( size_t i = 65339 ; i < 65375 ; ++i ) cTable[ i ] = TCHAR( i - 65248 );
	// convert circled katakana to ordinary katakana
	for ( size_t i = 13008 ; i < 13028 ; ++i ) cTable[ i ] = TCHAR( 2 * i - 13566 );
	for ( size_t i = 13028 ; i < 13033 ; ++i ) cTable[ i ] = TCHAR( i - 538 );
	for ( size_t i = 13033 ; i < 13038 ; ++i ) cTable[ i ] = TCHAR( 3 * i - 26604 );
	for ( size_t i = 13038 ; i < 13043 ; ++i ) cTable[ i ] = TCHAR( i - 528 );
	for ( size_t i = 13043 ; i < 13046 ; ++i ) cTable[ i ] = TCHAR( 2 * i - 13571 );
	for ( size_t i = 13046 ; i < 13051 ; ++i ) cTable[ i ] = TCHAR( i - 525 );
	cTable[ 13051 ] = TCHAR( 12527 );
	for ( size_t i = 13052 ; i < 13055 ; ++i ) cTable[ i ] = TCHAR( i - 524 );
	// map Katakana middle dot to space, since no API identifies it as a punctuation
	cTable[ 12539 ] = cTable[ 65381 ] = L' ';
	// map CJK Fullwidth space to halfwidth space
	cTable[ 12288 ] = L' ';
	// convert japanese halfwidth sound marks to fullwidth
	// all forms should be mapped; we need NFKD here
	cTable[ 65392 ] = TCHAR( 12540 );
	cTable[ 65438 ] = TCHAR( 12443 );
	cTable[ 65439 ] = TCHAR( 12444 );
}

CString& CLowerCaseTable::operator()(CString& strSource) const
{
	const int nLength = strSource.GetLength();
	const LPTSTR str = strSource.GetBuffer() + nLength;

	bool bSigma = false;
	for ( int i = -nLength; i; ++i ) 
	{
		if ( str[ i ] == 0x3a3 )
		{
			bSigma = true;
			break;
		}
	}

	if ( bSigma )
	{
		// Lowercase greek final sigmas first (word endings)
		const regex::rpattern regExpPattern( _T("(\\w+)\x3a3(\\W|$)"), _T("$1\x3c2$2"), 
		regex::GLOBAL|regex::MULTILINE|regex::NOBACKREFS, regex::MODE_SAFE );
		regex::subst_results results;
		std::wstring strTemp( strSource, nLength );
		regExpPattern.substitute( strTemp, results );
		strSource.SetString( strTemp.c_str(), nLength );
	}

	// Lowercase now everything. Not final sigmas are taken from the table
	for ( int i = -nLength; i; ++i )
		str[ i ] = ( *this )( str[ i ] );

	strSource.ReleaseBuffer( nLength );

	return strSource;
}

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
