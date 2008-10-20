//
// TransferFile.h
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

class CTransferFile;

class CTransferFiles
{
// Construction
public:
	CTransferFiles();
	virtual ~CTransferFiles();

// Attributes
public:
	CCriticalSection	m_pSection;
	CMap< CString, const CString&, CTransferFile*, CTransferFile* > m_pMap;
	CList< CTransferFile* > m_pDeferred;

// Operations
public:
	CTransferFile*	Open(LPCTSTR pszFile, BOOL bWrite, BOOL bCreate);
	void			Close();
	void			CommitDeferred();
protected:
	void			QueueDeferred(CTransferFile* pFile);
	void			Remove(CTransferFile* pFile);

	friend class CTransferFile;
};

#define	DEFERRED_MAX	8

class CTransferFile
{
public:
	CTransferFile(LPCTSTR pszPath);
	virtual ~CTransferFile();

	void		AddRef();
	void		Release(BOOL bWrite);
	HANDLE		GetHandle(BOOL bWrite = FALSE);
	QWORD		GetSize() const;
	BOOL		Read(QWORD nOffset, LPVOID pBuffer, QWORD nBuffer, QWORD* pnRead);
	BOOL		Write(QWORD nOffset, LPCVOID pBuffer, QWORD nBuffer, QWORD* pnWritten);

	inline BOOL	IsOpen() const
	{
		return ( m_hFile != INVALID_HANDLE_VALUE );
	}

	inline BOOL	IsExists() const
	{
		return m_bExists;
	}

protected:
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
	DWORD		m_nReference;
	DefWrite	m_pDeferred[DEFERRED_MAX];
	int			m_nDeferred;

	BOOL		Open(BOOL bWrite, BOOL bCreate);
	BOOL		EnsureWrite();
	BOOL		CloseWrite();
	void		DeferredWrite(BOOL bOffline = FALSE);

	friend class CTransferFiles;
};

extern CTransferFiles TransferFiles;
