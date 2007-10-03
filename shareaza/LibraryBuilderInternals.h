//
// LibraryBuilderInternals.h
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

#if !defined(AFX_LIBRARYBUILDERINTERNALS_H__5CAE40BD_1963_4A30_A333_89DBB6899803__INCLUDED_)
#define AFX_LIBRARYBUILDERINTERNALS_H__5CAE40BD_1963_4A30_A333_89DBB6899803__INCLUDED_

#pragma once
#include <MsiQuery.h>

class CLibraryBuilder;

class CLibraryBuilderInternals
{
// Construction
public:
	CLibraryBuilderInternals();
	virtual ~CLibraryBuilderInternals();

protected:
	BOOL		m_bEnableMP3;
	BOOL		m_bEnableEXE;
	BOOL		m_bEnableMSI;
	BOOL		m_bEnableImage;
	BOOL		m_bEnableASF;
	BOOL		m_bEnableOGG;
	BOOL		m_bEnableAPE;
	BOOL		m_bEnableMPC;
	BOOL		m_bEnableAVI;
	BOOL		m_bEnablePDF;
	BOOL		m_bEnableCHM;
protected:
	DWORD		m_nSleep;
public:
	static LPCTSTR	pszID3Genre[];

// Operations
public:
	void		LoadSettings();
	BOOL		ExtractMetadata(DWORD nIndex, CString& strPath, HANDLE hFile, Hashes::Sha1Hash& oSHA1, Hashes::Md5Hash& oMD5);
protected:		// ID3v1 and ID3v2 and MP3
	BOOL		ReadID3v1(DWORD nIndex, HANDLE hFile, CXMLElement* pXML = NULL);
	BOOL		CopyID3v1Field(CXMLElement* pXML, LPCTSTR pszAttribute, LPCSTR pszValue, int nLength);
	BOOL		ReadID3v2(DWORD nIndex, HANDLE hFile);
	BOOL		CopyID3v2Field(CXMLElement* pXML, LPCTSTR pszAttribute, BYTE* pBuffer, DWORD nLength, BOOL bSkipLanguage = FALSE);
	BOOL		ReadMP3Frames(DWORD nIndex, HANDLE hFile);
	BOOL		ScanMP3Frame(CXMLElement* pXML, HANDLE hFile, DWORD nIgnore);
protected:		// Module Version
	BOOL		ReadVersion(DWORD nIndex, LPCTSTR pszPath);
	BOOL		CopyVersionField(CXMLElement* pXML, LPCTSTR pszAttribute, BYTE* pBuffer, LPCTSTR pszKey, DWORD nLangId, BOOL bCommaToDot = FALSE);
	CString		GetVersionKey(BYTE* pBuffer, LPCTSTR pszKey, DWORD nLangId);
	DWORD		GetBestLanguageId(LPVOID pBuffer);
	BOOL		GetLanguageId(LPVOID pBuffer, UINT nSize, WORD nLangId, DWORD &nId, bool bOnlyPrimary = false);
protected:		// Windows Installer
	BOOL		ReadMSI(DWORD nIndex, LPCTSTR pszPath);
	CString		GetSummaryField(MSIHANDLE hSummaryInfo, UINT nProperty);
protected:		// Image Files
	BOOL		ReadJPEG(DWORD nIndex, HANDLE hFile);
	BOOL		ReadGIF(DWORD nIndex, HANDLE hFile);
	BOOL		ReadPNG(DWORD nIndex, HANDLE hFile);
	BOOL		ReadBMP(DWORD nIndex, HANDLE hFile);
protected:		// General Media
	BOOL		ReadASF(DWORD nIndex, HANDLE hFile);
	BOOL		ReadAVI(DWORD nIndex, HANDLE hFile);
	BOOL		ReadMPEG(DWORD nIndex, HANDLE hFile);
	BOOL		ReadOGG(DWORD nIndex, HANDLE hFile);
	BYTE*		ReadOGGPage(HANDLE hFile, DWORD& nBuffer, BYTE nFlags, DWORD nSequence, DWORD nMinSize = 0);
	BOOL		ReadOGGString(BYTE*& pOGG, DWORD& nOGG, CString& str);
	BOOL		ReadAPE(DWORD nIndex, HANDLE hFile, Hashes::Md5Hash& oMD5, bool bPreferFooter = false);
	BOOL		ReadMPC(DWORD nIndex, HANDLE hFile, Hashes::Md5Hash& oMD5);
	BOOL		ReadPDF(DWORD nIndex, HANDLE hFile, LPCTSTR pszPath);
	CString		ReadLine(HANDLE hFile, LPCTSTR pszSeparators = NULL);
	CString		ReadLineReverse(HANDLE hFile, LPCTSTR pszSeparators = NULL);
    BOOL		ReadCollection(DWORD nIndex, HANDLE hFile, const Hashes::Sha1Hash& oSHA1);
	BOOL		ReadCHM(DWORD nIndex, HANDLE hFile, LPCTSTR pszPath);
	CString		DecodePDFText(CString& strInput);
	BOOL		ReadTorrent(DWORD nIndex, HANDLE hFile, LPCTSTR pszPath);
};

#endif // !defined(AFX_LIBRARYBUILDERINTERNALS_H__5CAE40BD_1963_4A30_A333_89DBB6899803__INCLUDED_)
