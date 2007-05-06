//
// LibraryBuilder.h
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

#if !defined(AFX_LIBRARYBUILDER_H__B2779061_437E_4C10_AC9F_1E2AC7885C40__INCLUDED_)
#define AFX_LIBRARYBUILDER_H__B2779061_437E_4C10_AC9F_1E2AC7885C40__INCLUDED_

#pragma once

class CLibraryFile;
class CXMLElement;


class CLibraryBuilder
{
public:
	CLibraryBuilder();
	virtual ~CLibraryBuilder();

	void		Add(CLibraryFile* pFile);
	void		Remove(CLibraryFile* pFile);
	int			GetRemaining();
	void		RequestPriority(LPCTSTR pszPath);
	BOOL		StartThread();
	BOOL		IsAlive() const;
	void		StopThread();
	void		BoostPriority(BOOL bPriority);
	BOOL		GetBoostPriority();
	CString		GetCurrent();

	static int	SubmitMetadata(DWORD nIndex, LPCTSTR pszSchemaURI, CXMLElement*& pXML);
	static BOOL	SubmitCorrupted(DWORD nIndex);

protected:
	CMutex						m_pSection;			// Common guarding
	std::list< DWORD >			m_pFiles;
	CMutex						m_pPrioritySection;	// m_pPriority guarding
	std::list< CString >		m_pPriority;
	HANDLE						m_hThread;
	BOOL						m_bThread;			// FALSE - termination request
	BOOL						m_bPriority;
	CString						m_sPath;			// Hashing filename
	CMutex						m_pDelaySection;	// m_nLastCall, m_nFreq, m_nReaded, m_nElapsed guarding
	LARGE_INTEGER				m_nLastCall;		// (ticks)
	LARGE_INTEGER				m_nFreq;			// (Hz)
	QWORD						m_nReaded;			// (bytes)
	QWORD						m_nElapsed;			// (mks)

	void		Remove(LPCTSTR szPath);
	void		Skip();
	UINT		GetNextFileToHash();
	void		Clear();
	BOOL		ReadFileWithPriority(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, BOOL bPriority = TRUE);
	static UINT	ThreadStart(LPVOID pParam);
	void		OnRun();
	BOOL		HashFile(LPCTSTR szPath, HANDLE hFile, Hashes::Sha1Hash& oSHA1, Hashes::Md5Hash& oMD5, DWORD nIndex);
	static BOOL	DetectVirtualFile(LPCTSTR szPath, HANDLE hFile, QWORD& nOffset, QWORD& nLength);
	static BOOL	DetectVirtualID3v1(HANDLE hFile, QWORD& nOffset, QWORD& nLength);
	static BOOL	DetectVirtualID3v2(HANDLE hFile, QWORD& nOffset, QWORD& nLength);
};

extern CLibraryBuilder LibraryBuilder;

#endif // !defined(AFX_LIBRARYBUILDER_H__B2779061_437E_4C10_AC9F_1E2AC7885C40__INCLUDED_)
