//
// Strings.h 
//
// Copyright (c) Shareaza Development Team, 2010-2011.
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
CString URLDecodeANSI(LPCTSTR pszInput);

// Decodes URLs with extended characters
CString URLDecodeUnicode(LPCTSTR pszInput);

// Case independent string search
LPCTSTR _tcsistr(LPCTSTR pszString, LPCTSTR pszSubString);
LPCTSTR _tcsnistr(LPCTSTR pszString, LPCTSTR pszSubString, size_t nlen);

// Convert string to integer (64-bit, decimal only, no sign, no spaces allowed). Returns -1 on error.
__int64 atoin(__in_bcount(nLen) const char* pszString, __in size_t nLen);

// Split string using delimiter to string array
void Split(const CString& strSource, TCHAR cDelimiter, CStringArray& pAddIt, BOOL bAddFirstEmpty = FALSE);

// StartsWith("hello world", "hello") is true
BOOL StartsWith(const CString& sInput, LPCTSTR pszText, size_t nLen);

// Load all text from file (Unicode-compatible)
CString LoadFile(LPCTSTR pszPath);

// Replaces a substring with another (case-insensitive)
BOOL ReplaceNoCase(CString& sInStr, LPCTSTR pszOldStr, LPCTSTR pszNewStr);

// Returns "a.a.a.a:port"
CString HostToString(const SOCKADDR_IN* pHost);

// Function is used to split a phrase in asian languages to separate keywords
// to ease keyword matching, allowing user to type as in the natural language.
// Spacebar key is not a convenient way to separate keywords with IME, and user
// may not know how application is keywording their files.
//
// The function splits katakana, hiragana and CJK phrases out of the input string.
// ToDo: "minus" words and quoted phrases for asian languages may not work correctly in all cases.
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
