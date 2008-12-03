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

class CShareazaFile;
class CEDPartImporter;
class CBTInfo;


class CFragmentedFile : public CObject
{
	DECLARE_DYNCREATE( CFragmentedFile )

public:
	CFragmentedFile();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual ~CFragmentedFile();

	struct CVirtualFilePart
	{
		inline bool operator ==(LPCTSTR pszFile) const
		{
			return ! m_sPath.CompareNoCase( pszFile );
		}

		inline bool operator <(const CVirtualFilePart& p) const
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
		inline bool operator()(const CVirtualFilePart& _Left, QWORD _Right) const
		{
			return _Left.m_nOffset > _Right;
		}
	};

	struct Flusher : public std::unary_function< CVirtualFilePart, void >
	{
		inline void operator()(const CVirtualFilePart& p) const
		{
			if ( p.m_pFile )
			{
				FlushFileBuffers( p.m_pFile->GetHandle() );
			}
		}
	};

	struct Releaser : public std::unary_function< CVirtualFilePart, void >
	{
		inline void operator()(CVirtualFilePart& p) const
		{
			if ( p.m_pFile )
			{
				p.m_pFile->Release();
				p.m_pFile = NULL;
			}
		}
	};

	struct Completer : public std::unary_function< CVirtualFilePart, void >
	{
		inline void operator()(const CVirtualFilePart& p) const
		{
			if ( p.m_pFile )
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
		}
	};

	struct EnsureWriter : public std::unary_function< CVirtualFilePart, bool >
	{
		inline bool operator()(const CVirtualFilePart& p) const
		{
			return ! p.m_pFile || p.m_pFile->EnsureWrite();
		}
	};

	typedef std::list< CString > CStringList;
	typedef std::list< QWORD > COffsetList;

	mutable CCriticalSection	m_pSection;
	CVirtualFile				m_oFile;
	QWORD						m_nUnflushed;
	Fragments::List				m_oFList;
	volatile LONG				m_nRefCount;

	BOOL	VirtualRead(QWORD nOffset, char* pBuffer, QWORD nBuffer, QWORD* pnRead);
	BOOL	VirtualWrite(QWORD nOffset, const char* pBuffer, QWORD nBuffer, QWORD* pnWritten);

public:
	ULONG	AddRef();
	ULONG	Release();
	BOOL	Open(LPCTSTR pszFile, QWORD nOffset, QWORD nLength, BOOL bWrite, BOOL bCreate);
	BOOL	Open(const CBTInfo& oInfo, BOOL bWrite, BOOL bCreate);
	BOOL	Flush();
	void	Close();
	BOOL	MakeComplete();
	void	Serialize(CArchive& ar, int nVersion);
	BOOL	EnsureWrite();
	BOOL	Write(QWORD nOffset, LPCVOID pData, QWORD nLength, QWORD* pnWritten = NULL);
	BOOL	Read(QWORD nOffset, LPVOID pData, QWORD nLength, QWORD* pnRead = NULL);
	QWORD	InvalidateRange(QWORD nOffset, QWORD nLength);
	// Check if specified file handled
	BOOL	FindByPath(const CString& sPath) const;

	// Is file has size?
	inline BOOL IsValid() const
	{
		return m_oFList.limit() > 0;
	}
	
	BOOL IsOpen() const;

	// Get total size of whole file (in bytes)
	inline QWORD GetTotal() const
	{
		CQuickLock oLock( m_pSection );

		return m_oFList.limit();
	}

	inline QWORD GetRemaining() const
	{
		CQuickLock oLock( m_pSection );

		return ( m_oFList.limit() == SIZE_UNKNOWN && m_oFList.length_sum() ) ?
			SIZE_UNKNOWN : m_oFList.length_sum();
	}

	// Get completed size of whole file (in bytes)
	inline QWORD GetCompleted() const
	{
		CQuickLock oLock( m_pSection );

		return m_oFList.missing();
	}

	inline Fragments::List GetEmptyFragmentList() const
	{
		CQuickLock oLock( m_pSection );

		return m_oFList;
	}

	inline BOOL IsPositionRemaining(QWORD nOffset) const
	{
		CQuickLock oLock( m_pSection );

		return m_oFList.has_position( nOffset );
	}

	inline BOOL DoesRangeOverlap(QWORD nOffset, QWORD nLength) const
	{
		CQuickLock oLock( m_pSection );

		return m_oFList.overlaps( Fragments::Fragment( nOffset, nOffset + nLength ) );
	}

	inline QWORD GetRangeOverlap(QWORD nOffset, QWORD nLength) const
	{
		CQuickLock oLock( m_pSection );

		return m_oFList.overlapping_sum( Fragments::Fragment( nOffset, nOffset + nLength ) );
	}

	// Get completed size of defined range (in bytes)
	inline QWORD GetCompleted(QWORD nOffset, QWORD nLength) const
	{
		CQuickLock oLock( m_pSection );

		// TODO: Optimize this
		Fragments::List oList( m_oFList );	
		oList.insert( Fragments::Fragment( 0, nOffset ) );
		oList.insert( Fragments::Fragment( nOffset + nLength, m_oFList.limit() ) );

		return oList.missing();
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
