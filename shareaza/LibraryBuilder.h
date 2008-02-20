//
// LibraryBuilder.h
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


class CLibraryFile;
class CXMLElement;


class CLibraryBuilder
{
public:
	CLibraryBuilder();
	virtual ~CLibraryBuilder();

	void		Add(CLibraryFile* pFile);			// Add file to list
	void		Remove(DWORD nIndex);				// Remove file from list
	void		Remove(LPCTSTR szPath);				// Remove file from list
	void		Remove(CLibraryFile* pFile);		// Remove file from list
	void		RequestPriority(LPCTSTR pszPath);	// Place file to the begin of list
	void		Skip(DWORD nIndex);					// Move file to the end of list
	BOOL		IsAlive() const;
	void		StartThread();
	void		StopThread();
	void		BoostPriority(BOOL bPriority);
	BOOL		GetBoostPriority() const;

	CString		GetCurrent() const;					// Hashing filename
	size_t		GetRemaining() const;				// Hashing queue size
	DWORD		GetProgress() const;				// Hashing file progress (0..100%)

	static int	SubmitMetadata(DWORD nIndex, LPCTSTR pszSchemaURI, CXMLElement*& pXML);
	static BOOL	SubmitCorrupted(DWORD nIndex);

	static BOOL RefreshMetadata(const CString& sPath);

protected:
	class __declspec(novtable) CFileInfo
	{
	public:
		inline CFileInfo(DWORD n = 0) throw() :
			nIndex( n ), nNextAccessTime( 0 )
		{
		}
		inline CFileInfo(const CFileInfo& i) throw() :
			nIndex( i.nIndex ), nNextAccessTime( i.nNextAccessTime)
		{
		}
		inline bool operator==(const CFileInfo& i) const throw()
		{
			return ( nIndex == i.nIndex );
		}
		DWORD		nIndex;							// Library file index
		QWORD		nNextAccessTime;				// Next access time
	};
	typedef std::list< CFileInfo > CFileInfoList;

	mutable CMutex				m_pSection;			// Guarding
	CFileInfoList				m_pFiles;			// File list
	HANDLE						m_hThread;
	BOOL						m_bThread;			// FALSE - termination request
	BOOL						m_bPriority;
	CString						m_sPath;			// Hashing filename
	DWORD						m_nProgress;		// Hashing file progress (0..100%)
	LARGE_INTEGER				m_nLastCall;		// (ticks)
	LARGE_INTEGER				m_nFreq;			// (Hz)
	QWORD						m_nReaded;			// (bytes)
	__int64						m_nElapsed;			// (mks)

	// Get next file from list doing all possible tests
	// Returns 0 if no file available, sets m_bThread = FALSE if no files left.
	DWORD		GetNextFileToHash(CString& sPath);
	static UINT	ThreadStart(LPVOID pParam);
	void		OnRun();
	BOOL		HashFile(LPCTSTR szPath, HANDLE hFile, DWORD nIndex);
	static BOOL	DetectVirtualFile(LPCTSTR szPath, HANDLE hFile, QWORD& nOffset, QWORD& nLength);
	static BOOL	DetectVirtualID3v1(HANDLE hFile, QWORD& nOffset, QWORD& nLength);
	static BOOL	DetectVirtualID3v2(HANDLE hFile, QWORD& nOffset, QWORD& nLength);
};

extern CLibraryBuilder LibraryBuilder;
