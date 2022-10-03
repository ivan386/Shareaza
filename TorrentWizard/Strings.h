//
// Strings.h 
//
// Copyright (c) Shareaza Development Team, 2010-2015.
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

#pragma once

#include <set>


// Produces two arguments divided by comma, where first argument is a string itself
// and second argument is a string length without null terminator
#define _P(x)	(x),(_countof(x)-1)
#define _PT(x)	_P(_T(x))

#define IsSpace(ch)	((ch) == _T(' ') || (ch) == _T('\t') || (ch) == _T('\r') || (ch) == _T('\n'))

bool IsCharacter(WCHAR nChar);
bool IsHiragana(WCHAR nChar);
bool IsKatakana(WCHAR nChar);
bool IsKanji(WCHAR nChar);
bool IsWord(LPCTSTR pszString, size_t nStart, size_t nLength);
void IsType(LPCTSTR pszString, size_t nStart, size_t nLength, bool& bWord, bool& bDigit, bool& bMix);

typedef enum
{
	sNone		= 0,
	sNumeric	= 1,
	sRegular	= 2,
	sKanji		= 4,
	sHiragana	= 8,
	sKatakana	= 16
} ScriptType;

class CLowerCaseTable
{
public:
	explicit CLowerCaseTable();
	TCHAR operator()(TCHAR cLookup) const;
	CString& operator()(CString& strSource) const;

	// Convert string to lower case and do character substitutes:
	// ".", "_" and "+" to " "; "[" and "{" to "("; "]" and "}" to ")".
	CString& Clean(CString& strSource) const;

private:
	TCHAR cTable[ 65536 ];

	CLowerCaseTable(const CLowerCaseTable&);
	CLowerCaseTable& operator=(const CLowerCaseTable&);
};

extern const CLowerCaseTable ToLower;

// Encode Unicode text to UTF-8 text
CStringA UTF8Encode(__in const CStringW& strInput);
CStringA UTF8Encode(__in_bcount(nInput) LPCWSTR psInput, __in int nInput);

// Decode UTF-8 text to Unicode text
CStringW UTF8Decode(__in const CStringA& strInput);
CStringW UTF8Decode(__in_bcount(nInput) LPCSTR psInput, __in int nInput);

// Encode "hello world" into "hello%20world"
CString URLEncode(LPCTSTR pszInput);

// Decode "hello%20world" back to "hello world"
CString URLDecode(LPCTSTR pszInput);
CString URLDecode(__in_bcount(nInput) LPCSTR psInput, __in int nInput);
CString URLDecode(__in const CStringA& strInput);

// Decodes properly encoded URLs
CString URLDecodeANSI(const TCHAR* __restrict pszInput);

// Decodes URLs with extended characters
CString URLDecodeUnicode(const TCHAR* __restrict pszInput);

// Case independent string search
LPCTSTR _tcsistr(LPCTSTR pszString, LPCTSTR pszSubString);
LPCTSTR _tcsnistr(LPCTSTR pszString, LPCTSTR pszSubString, size_t nlen);

// Convert string to integer (64-bit, decimal only, with sign, no spaces allowed). Returns false on error.
bool atoin(__in_bcount(nLen) const char* pszString, __in size_t nLen, __int64& nNum);

#ifdef __AFXCOLL_H__
// Split string using delimiter to string array
void Split(const CString& strSource, TCHAR cDelimiter, CStringArray& pAddIt, BOOL bAddFirstEmpty = FALSE);
#endif // __AFXCOLL_H__

// StartsWith("hello world", "hello") is true
BOOL StartsWith(const CString& sInput, LPCTSTR pszText, size_t nLen);

#ifdef __AFX_H__
// Load all text from file (Unicode-compatible)
CString LoadFile(LPCTSTR pszPath);
#endif // __AFX_H__

// Replaces a substring with another (case-insensitive)
BOOL ReplaceNoCase(CString& sInStr, LPCTSTR pszOldStr, LPCTSTR pszNewStr);

#ifdef _WINSOCKAPI_
// Returns "a.a.a.a:port"
CString HostToString(const SOCKADDR_IN* pHost);
#endif // _WINSOCKAPI_

// Function is used to split a phrase in Asian languages to separate keywords
// to ease keyword matching, allowing user to type as in the natural language.
// Spacebar key is not a convenient way to separate keywords with IME, and user
// may not know how application is keywording their files.
//
// The function splits katakana, Hiragana and CJK phrases out of the input string.
// ToDo: "minus" words and quoted phrases for Asian languages may not work correctly in all cases.
CString MakeKeywords(const CString& strPhrase, bool bExpression = true);

typedef std::pair< LPCTSTR, size_t > WordEntry;

struct CompareWordEntries : public std::binary_function< WordEntry, WordEntry, bool >
{
	bool operator()(const WordEntry& lhs, const WordEntry& rhs) const
	{
		int cmp = _tcsnicmp( lhs.first, rhs.first, min( lhs.second, rhs.second ) );
		return ( cmp < 0 || cmp == 0 && lhs.second < rhs.second );
	}
};

typedef std::set< WordEntry, CompareWordEntries > WordTable;

void BuildWordTable(LPCTSTR pszWord, WordTable& oWords, WordTable& oNegWords);

// Skip slashes starting from nAdd position
LPCTSTR SkipSlashes(LPCTSTR pszURL, int nAdd = 0);

// Replace all symbols with code less than space by underscore symbols
void SafeString(CString& strInput);

// Escape unsafe symbols
CString Escape(const CString& strValue);

// Unescape unsafe symbols
CString Unescape(const TCHAR* __restrict pszXML, int nLength = -1);
