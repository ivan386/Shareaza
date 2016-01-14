//
// TransferFile.h
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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

class CTransferFile;


class CTransferFiles
{
public:
	CTransferFiles();
	~CTransferFiles();

	typedef CAtlMap< CString, CTransferFile*, CStringElementTraitsI< CString > > CTransferFileMap;
	typedef CList< CTransferFile* > CTransferFileList;

	CTransferFile*		Open(LPCTSTR pszFile, BOOL bWrite);
	void				CommitDeferred();

protected:
	CCriticalSection	m_pSection;
	CTransferFileMap	m_pMap;
	CTransferFileList	m_pDeferred;

	void				QueueDeferred(CTransferFile* pFile);
	void				Remove(CTransferFile* pFile);

	friend class CTransferFile;
};

#define	DEFERRED_MAX	8

class CTransferFile
{
public:
	CTransferFile(LPCTSTR pszPath);

	ULONG		AddRef();
	ULONG		Release();
	HANDLE		GetHandle(BOOL bWrite = FALSE);
	QWORD		GetSize() const;
	BOOL		Read(QWORD nOffset, LPVOID pBuffer, QWORD nBuffer, QWORD* pnRead);
	BOOL		Write(QWORD nOffset, LPCVOID pBuffer, QWORD nBuffer, QWORD* pnWritten);
	BOOL		EnsureWrite();

	inline BOOL	IsOpen() const throw()
	{
		return ( m_hFile != INVALID_HANDLE_VALUE ) || IsFolder();
	}

	inline BOOL	IsExists() const throw()
	{
		return m_bExists;
	}

	inline BOOL	IsWritable() const throw()
	{
		return m_bWrite;
	}

	inline BOOL	IsFolder() const throw()
	{
		return ( m_sPath.GetAt( m_sPath.GetLength() - 1 ) == _T( '\\' ) );
	}

protected:
	virtual ~CTransferFile();

	// Deferred Write Structure
	class DefWrite
	{
	public:
		QWORD	m_nOffset;
		DWORD	m_nLength;
		BYTE*	m_pBuffer;
	};

	CString		m_sPath;
	HANDLE		m_hFile;
	BOOL		m_bExists;			// File exists before open
	BOOL		m_bWrite;			// File opened for write operations
	volatile LONG m_nRefCount;
	DefWrite	m_pDeferred[DEFERRED_MAX];
	int			m_nDeferred;

	BOOL		Open(BOOL bWrite);
	BOOL		CloseWrite();
	void		DeferredWrite(BOOL bOffline = FALSE);

	friend class CTransferFiles;
};

extern CTransferFiles TransferFiles;
