//
// LibraryBuilderInternals.h
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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

class CLibraryBuilder;


class CLibraryBuilderInternals  
{
// Construction
public:
	CLibraryBuilderInternals(CLibraryBuilder* pBuilder);
	virtual ~CLibraryBuilderInternals();
	
// Attributes
protected:
	CLibraryBuilder* m_pBuilder;
protected:
	BOOL		m_bEnableMP3;
	BOOL		m_bEnableEXE;
	BOOL		m_bEnableImage;
	BOOL		m_bEnableASF;
	BOOL		m_bEnableOGG;
	BOOL		m_bEnableAPE;
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
	BOOL		ExtractMetadata( CString& strPath, HANDLE hFile, SHA1* pSHA1);
protected:
	BOOL		SubmitMetadata( LPCTSTR pszSchemaURI, CXMLElement* pXML);
	BOOL		SubmitCorrupted();
protected:		// ID3v1 and ID3v2 and MP3
	BOOL		ReadID3v1( HANDLE hFile, CXMLElement* pXML = NULL);
	BOOL		CopyID3v1Field(CXMLElement* pXML, LPCTSTR pszAttribute, LPCSTR pszValue, int nLength);
	BOOL		ReadID3v2( HANDLE hFile);
	BOOL		CopyID3v2Field(CXMLElement* pXML, LPCTSTR pszAttribute, BYTE* pBuffer, DWORD nLength, BOOL bSkipLanguage = FALSE);
	BOOL		ReadMP3Frames( HANDLE hFile);
	BOOL		ScanMP3Frame(CXMLElement* pXML, HANDLE hFile, DWORD nIgnore);
protected:		// Module Version
	BOOL		ReadVersion( LPCTSTR pszPath);
	BOOL		CopyVersionField(CXMLElement* pXML, LPCTSTR pszAttribute, BYTE* pBuffer, LPCTSTR pszKey, BOOL bCommaToDot = FALSE);
	CString		GetVersionKey(BYTE* pBuffer, LPCTSTR pszKey);
protected:		// Image Files
	BOOL		ReadJPEG( HANDLE hFile);
	BOOL		ReadGIF( HANDLE hFile);
	BOOL		ReadPNG( HANDLE hFile);
	BOOL		ReadBMP( HANDLE hFile);
protected:		// General Media
	BOOL		ReadASF( HANDLE hFile);
	BOOL		ReadAVI( HANDLE hFile);
	BOOL		ReadMPEG( HANDLE hFile);
	BOOL		ReadOGG( HANDLE hFile);
	BYTE*		ReadOGGPage(HANDLE hFile, DWORD& nBuffer, BYTE nFlags, DWORD nSequence, DWORD nMinSize = 0);
	BOOL		ReadOGGString(BYTE*& pOGG, DWORD& nOGG, CString& str);
	BOOL		ReadAPE( HANDLE hFile);
	BOOL		ReadPDF( HANDLE hFile, LPCTSTR pszPath);
	CString		ReadLine(HANDLE hFile);
	CString		ReadLineReverse(HANDLE hFile);
	BOOL		ReadCollection( HANDLE hFile, SHA1* pSHA1);
	BOOL		ReadCHM(HANDLE hFile, LPCTSTR pszPath);
};

#endif // !defined(AFX_LIBRARYBUILDERINTERNALS_H__5CAE40BD_1963_4A30_A333_89DBB6899803__INCLUDED_)
