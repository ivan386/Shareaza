//
// LibraryBuilderInternals.h
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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

class CXMLElement;

class CLibraryBuilderInternals
{
private:
	static LPCTSTR	pszID3Genre[];

public:
	int				LookupID3v1Genre(const CString& strGenre) const;
	static bool		ExtractMetadata(DWORD nIndex, const CString& strPath, HANDLE hFile);

private:
	// ID3v1 and ID3v2 and MP3
	static bool		ReadID3v1(DWORD nIndex, HANDLE hFile, CXMLElement* pXML = NULL);
	static bool		CopyID3v1Field(CXMLElement* pXML, LPCTSTR pszAttribute, CString strValue);
	static bool		ReadID3v2(DWORD nIndex, HANDLE hFile);
	static bool		CopyID3v2Field(CXMLElement* pXML, LPCTSTR pszAttribute, BYTE* pBuffer, DWORD nLength, bool bSkipLanguage = false);
	static bool		ReadMP3Frames(DWORD nIndex, HANDLE hFile);
	static bool		ScanMP3Frame(CXMLElement* pXML, HANDLE hFile, DWORD nIgnore);

	// Module Version
	static bool		ReadVersion(DWORD nIndex, LPCTSTR pszPath);
	static bool		CopyVersionField(CXMLElement* pXML, LPCTSTR pszAttribute, BYTE* pBuffer, LPCTSTR pszKey, DWORD nLangId, bool bCommaToDot = false);
	static CString	GetVersionKey(BYTE* pBuffer, LPCTSTR pszKey, DWORD nLangId);
	static DWORD	GetBestLanguageId(LPVOID pBuffer);
	static bool		GetLanguageId(LPVOID pBuffer, UINT nSize, WORD nLangId, DWORD &nId, bool bOnlyPrimary = false);

	// Module Manifest Validation
	static bool		ValidateManifest(LPCTSTR pszPath);

	// Windows Installer
	static bool		ReadMSI(DWORD nIndex, LPCTSTR pszPath);
	static CString	GetSummaryField(MSIHANDLE hSummaryInfo, UINT nProperty);

	// Image Files
	static bool		ReadJPEG(DWORD nIndex, HANDLE hFile);
	static bool		ReadGIF(DWORD nIndex, HANDLE hFile);
	static bool		ReadPNG(DWORD nIndex, HANDLE hFile);
	static bool		ReadBMP(DWORD nIndex, HANDLE hFile);

	// General Media
	static bool		ReadASF(DWORD nIndex, HANDLE hFile);
	static bool		ReadAVI(DWORD nIndex, HANDLE hFile);
	static bool		ReadMPEG(DWORD nIndex, HANDLE hFile);
	static bool		ReadOGG(DWORD nIndex, HANDLE hFile);
	static BYTE*	ReadOGGPage(HANDLE hFile, DWORD& nBuffer, BYTE nFlags, DWORD nSequence, DWORD nMinSize = 0);
	static bool		ReadOGGString(BYTE*& pOGG, DWORD& nOGG, CString& str);
	static bool		ReadAPE(DWORD nIndex, HANDLE hFile, bool bPreferFooter = false);
	static bool		ReadMPC(DWORD nIndex, HANDLE hFile);
	static bool		ReadPDF(DWORD nIndex, HANDLE hFile, LPCTSTR pszPath);
	static CString	ReadLine(HANDLE hFile, LPCTSTR pszSeparators = NULL);
	static CString	ReadLineReverse(HANDLE hFile, LPCTSTR pszSeparators = NULL);
	static bool		ReadCollection(DWORD nIndex, HANDLE hFile);
	static bool		ReadCHM(DWORD nIndex, HANDLE hFile, LPCTSTR pszPath);
	static CString	DecodePDFText(CString& strInput);
	static bool		ReadTorrent(DWORD nIndex, HANDLE hFile, LPCTSTR pszPath);
};
