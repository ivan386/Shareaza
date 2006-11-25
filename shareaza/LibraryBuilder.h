//
// LibraryBuilder.h
//
// Copyright (c) Shareaza Development Team, 2002-2006.
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

#if !defined(AFX_LIBRARYBUILDER_H__B2779061_437E_4C10_AC9F_1E2AC7885C40__INCLUDED_)
#define AFX_LIBRARYBUILDER_H__B2779061_437E_4C10_AC9F_1E2AC7885C40__INCLUDED_

#pragma once

class CLibraryFile;
class CXMLElement;
class CLibraryBuilderInternals;
class CLibraryBuilderPlugins;

class CLibraryBuilder
{
// Construction
public:
	CLibraryBuilder();
	virtual ~CLibraryBuilder();

// Attributes
protected:
	CCriticalSection	m_pSection;
	CList< DWORD >		m_pFiles;
	CList< CString >	m_pPriority;
	HANDLE				m_hThread;
	BOOL				m_bThread;
	BOOL				m_bPriority;
	DWORD				m_nIndex;
	CString				m_sPath;
	DWORD				m_tActive;

	CCriticalSection	m_pDelaySection;
	LARGE_INTEGER		m_nLastCall;		// (ticks)
	LARGE_INTEGER		m_nFreq;			// (Hz)
	QWORD				m_nReaded;			// (bytes)
	QWORD				m_nElapsed;			// (mks)

protected:
	CLibraryBuilderInternals*	m_pInternals;
	CLibraryBuilderPlugins*		m_pPlugins;

// Operations
public:
	void		Add(CLibraryFile* pFile);
	void		Remove(CLibraryFile* pFile);
	INT_PTR		GetRemaining();
	CString		GetCurrentFile();
	void		RequestPriority(LPCTSTR pszPath);
	void		Clear();
	BOOL		StartThread();
	void		StopThread();
	void		BoostPriority(BOOL bPriority);
	BOOL		GetBoostPriority();
	//BOOL		SanityCheck();
	void		UpdateStatus(CString& strFileName, int* pRemaining );
	BOOL		ReadFileWithPriority(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, BOOL bPriority = TRUE);
protected:
	void		ReHashCurrentFile();
	static UINT	ThreadStart(LPVOID pParam);
	void		OnRun();
    BOOL		HashFile(HANDLE hFile, BOOL bPriority, Hashes::Sha1Hash& oSHA1);
	BOOL		SubmitMetadata(LPCTSTR pszSchemaURI, CXMLElement*& pXML);
	BOOL		SubmitCorrupted();
	BOOL		DetectVirtualFile(HANDLE hFile, QWORD& nOffset, QWORD& nLength);
	BOOL		DetectVirtualID3v1(HANDLE hFile, QWORD& nOffset, QWORD& nLength);
	BOOL		DetectVirtualID3v2(HANDLE hFile, QWORD& nOffset, QWORD& nLength);

	friend class CLibraryBuilderInternals;
	friend class CLibraryBuilderPlugins;

};

extern CLibraryBuilder LibraryBuilder;

#endif // !defined(AFX_LIBRARYBUILDER_H__B2779061_437E_4C10_AC9F_1E2AC7885C40__INCLUDED_)
