//
// LibraryBuilderInternals.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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


class CLibraryBuilderInternals
{
public:
	static LPCTSTR	pszID3Genre[];

public:
	static BOOL		ExtractMetadata(DWORD nIndex, const CString& strPath, HANDLE hFile);
protected:		// ID3v1 and ID3v2 and MP3
	static BOOL		ReadID3v1(DWORD nIndex, HANDLE hFile, CXMLElement* pXML = NULL);
	static BOOL		CopyID3v1Field(CXMLElement* pXML, LPCTSTR pszAttribute, LPCSTR pszValue, int nLength);
	static BOOL		ReadID3v2(DWORD nIndex, HANDLE hFile);
	static BOOL		CopyID3v2Field(CXMLElement* pXML, LPCTSTR pszAttribute, BYTE* pBuffer, DWORD nLength, BOOL bSkipLanguage = FALSE);
	static BOOL		ReadMP3Frames(DWORD nIndex, HANDLE hFile);
	static BOOL		ScanMP3Frame(CXMLElement* pXML, HANDLE hFile, DWORD nIgnore);
protected:		// Module Version
	static BOOL		ReadVersion(DWORD nIndex, LPCTSTR pszPath);
	static BOOL		CopyVersionField(CXMLElement* pXML, LPCTSTR pszAttribute, BYTE* pBuffer, LPCTSTR pszKey, DWORD nLangId, BOOL bCommaToDot = FALSE);
	static CString	GetVersionKey(BYTE* pBuffer, LPCTSTR pszKey, DWORD nLangId);
	static DWORD	GetBestLanguageId(LPVOID pBuffer);
	static BOOL		GetLanguageId(LPVOID pBuffer, UINT nSize, WORD nLangId, DWORD &nId, bool bOnlyPrimary = false);
protected:		// Windows Installer
	static BOOL		ReadMSI(DWORD nIndex, LPCTSTR pszPath);
	static CString	GetSummaryField(MSIHANDLE hSummaryInfo, UINT nProperty);
protected:		// Image Files
	static BOOL		ReadJPEG(DWORD nIndex, HANDLE hFile);
	static BOOL		ReadGIF(DWORD nIndex, HANDLE hFile);
	static BOOL		ReadPNG(DWORD nIndex, HANDLE hFile);
	static BOOL		ReadBMP(DWORD nIndex, HANDLE hFile);
protected:		// General Media
	static BOOL		ReadASF(DWORD nIndex, HANDLE hFile);
	static BOOL		ReadAVI(DWORD nIndex, HANDLE hFile);
	static BOOL		ReadMPEG(DWORD nIndex, HANDLE hFile);
	static BOOL		ReadOGG(DWORD nIndex, HANDLE hFile);
	static BYTE*	ReadOGGPage(HANDLE hFile, DWORD& nBuffer, BYTE nFlags, DWORD nSequence, DWORD nMinSize = 0);
	static BOOL		ReadOGGString(BYTE*& pOGG, DWORD& nOGG, CString& str);
	static BOOL		ReadAPE(DWORD nIndex, HANDLE hFile, bool bPreferFooter = false);
	static BOOL		ReadMPC(DWORD nIndex, HANDLE hFile);
	static BOOL		ReadPDF(DWORD nIndex, HANDLE hFile, LPCTSTR pszPath);
	static CString	ReadLine(HANDLE hFile, LPCTSTR pszSeparators = NULL);
	static CString	ReadLineReverse(HANDLE hFile, LPCTSTR pszSeparators = NULL);
    static BOOL		ReadCollection(DWORD nIndex, HANDLE hFile);
	static BOOL		ReadCHM(DWORD nIndex, HANDLE hFile, LPCTSTR pszPath);
	static CString	DecodePDFText(CString& strInput);
	static BOOL		ReadTorrent(DWORD nIndex, HANDLE hFile, LPCTSTR pszPath);
};
