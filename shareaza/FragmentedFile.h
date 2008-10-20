//
// FragmentedFile.h
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

#include "FileFragments.hpp"
#include "TransferFile.h"

class CEDPartImporter;


class CFragmentedFile
{
public:
	CFragmentedFile();
	virtual ~CFragmentedFile();

private:
	struct CVirtualFilePart
	{
		bool operator ==(LPCTSTR pszFile) const
		{
			return ! m_sPath.CompareNoCase( pszFile );
		}

		bool operator <(const CVirtualFilePart& p) const
		{
			return ( m_nOffset < p.m_nOffset );
		}

		CString			m_sPath;	// Full filename
		CTransferFile*	m_pFile;	// Opened file handler
		QWORD			m_nOffset;	// File offset (0 - for first/single file)
		QWORD			m_nLength;	// File size
		BOOL			m_bWrite;	// File opened for write
	};

	typedef std::list< CVirtualFilePart > CVirtualFile;

	struct Greater : public std::binary_function< CVirtualFilePart, QWORD, bool >
	{
		bool operator()(const CVirtualFilePart& _Left, const QWORD& _Right) const
		{
			return _Left.m_nOffset > _Right;
		}
	};

	struct Flusher : public std::unary_function< CVirtualFilePart, void >
	{
		void operator()(const CVirtualFilePart& p) const
		{
			FlushFileBuffers( p.m_pFile->GetHandle() );
		}
	};

	struct Releaser : public std::unary_function< CVirtualFilePart, void >
	{
		void operator()(const CVirtualFilePart& p) const
		{
			p.m_pFile->Release( TRUE );
		}
	};

	struct Completer : public std::unary_function< CVirtualFilePart, void >
	{
		void operator()(const CVirtualFilePart& p) const
		{
			HANDLE hFile = p.m_pFile->GetHandle( TRUE );
			if ( hFile != INVALID_HANDLE_VALUE )
			{
				LARGE_INTEGER nLength;
				nLength.QuadPart = p.m_nLength;
				SetFilePointerEx( hFile, nLength, NULL, FILE_BEGIN );
				SetEndOfFile( hFile );
			}
		}
	};

	typedef std::list< CString > CStringList;
	typedef std::list< QWORD > COffsetList;

	mutable CCriticalSection	m_pSection;
	CVirtualFile				m_oFile;
	QWORD						m_nUnflushed;
	Fragments::List				m_oFList;

	BOOL	VirtualRead(QWORD nOffset, char* pBuffer, QWORD nBuffer, QWORD* pnRead);
	BOOL	VirtualWrite(QWORD nOffset, const char* pBuffer, QWORD nBuffer, QWORD* pnWritten);

public:
	BOOL	Open(LPCTSTR pszFile, QWORD nOffset, QWORD nLength, BOOL bWrite, BOOL bCreate);
	BOOL	Flush();
	void	Close();
	void	Clear();
	BOOL	MakeComplete();
	void	Serialize(CArchive& ar, int nVersion);

	BOOL	IsPositionRemaining(QWORD nOffset) const;
	BOOL	DoesRangeOverlap(QWORD nOffset, QWORD nLength) const;
	QWORD	GetRangeOverlap(QWORD nOffset, QWORD nLength) const;
	BOOL	WriteRange(QWORD nOffset, LPCVOID pData, QWORD nLength);
	BOOL	ReadRange(QWORD nOffset, LPVOID pData, QWORD nLength);
	QWORD	InvalidateRange(QWORD nOffset, QWORD nLength);
	
	// Combined size
	QWORD	GetSize() const;

	inline BOOL IsValid() const
	{
		CQuickLock oLock( m_pSection );

		return m_oFList.limit() > 0;
	}
	
	inline BOOL IsOpen() const
	{
		CQuickLock oLock( m_pSection );

		return ! m_oFile.empty();
	}

//	inline QWORD GetTotal() const
//	{
//		CQuickLock oLock( m_pSection );
//
//		return m_oFList.limit();
//	}
	
	inline QWORD GetRemaining() const
	{
		CQuickLock oLock( m_pSection );

		return ( m_oFList.limit() == SIZE_UNKNOWN && m_oFList.length_sum() ) ?
			SIZE_UNKNOWN : m_oFList.length_sum();
	}
	
	inline QWORD GetCompleted() const
	{
		CQuickLock oLock( m_pSection );

		return m_oFList.missing();
	}
	
	const Fragments::List& GetEmptyFragmentList() const
	{
		CQuickLock oLock( m_pSection );

		return m_oFList;
	}
	
//	inline QWORD GetEmptyFragmentCount() const
//	{
//		CQuickLock oLock( m_pSection );
//
//		return m_oFList.size();
//	}
	
//	inline BOOL IsFlushNeeded() const
//	{
//		CQuickLock oLock( m_pSection );
//
//		return ( m_pFile != NULL ) && ( m_nUnflushed > 0 );
//	}

	friend class CEDPartImporter;
};
