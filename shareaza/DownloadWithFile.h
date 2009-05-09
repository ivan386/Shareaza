//
// DownloadWithFile.h
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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

#include "DownloadWithTransfers.h"
#include "FragmentedFile.h"

class CDownloadWithFile : public CDownloadWithTransfers
{
// Construction
protected:
	CDownloadWithFile();
	virtual ~CDownloadWithFile();

// Attributes
public:
	TRISTATE		m_bVerify;		// Verify status (TRI_TRUE - verified, TRI_FALSE - failed, TRI_UNKNOWN - not yet)
	DWORD			m_tReceived;
	BOOL			m_bMoving;		// Is complete file moving?

// Operations
public:
	float			GetProgress() const;
	QWORD			GetVolumeComplete() const;
	QWORD			GetVolumeRemaining() const;
	DWORD			GetTimeRemaining() const;
	CString			GetDisplayName() const;
	BOOL			IsFileOpen() const;
	BOOL			IsComplete() const;
	BOOL			PrepareFile();
	BOOL			GetFragment(CDownloadTransfer* pTransfer);
	BOOL			IsPositionEmpty(QWORD nOffset);
	BOOL			AreRangesUseful(const Fragments::List& oAvailable);
	BOOL			IsRangeUseful(QWORD nOffset, QWORD nLength);
	BOOL			IsRangeUsefulEnough(CDownloadTransfer* pTransfer, QWORD nOffset, QWORD nLength);
	BOOL			ClipUploadRange(QWORD nOffset, QWORD& nLength) const;
	BOOL			GetRandomRange(QWORD& nOffset, QWORD& nLength) const;
	BOOL			SubmitData(QWORD nOffset, LPBYTE pData, QWORD nLength);
	QWORD			EraseRange(QWORD nOffset, QWORD nLength);
	BOOL			MakeComplete();
	QWORD			InvalidateFileRange(QWORD nOffset, QWORD nLength);

	// Get list of empty fragments
	inline Fragments::List GetEmptyFragmentList() const
	{
		return m_pFile.get() ? m_pFile->GetEmptyFragmentList() : Fragments::List( 0 );
	}

	// Get list of empty fragments we really want to download
	inline Fragments::List GetWantedFragmentList() const
	{
		return m_pFile.get() ? m_pFile->GetWantedFragmentList() : Fragments::List( 0 );
	}

	inline CFragmentedFile* GetFile()
	{
		m_pFile->AddRef();
		return m_pFile.get();
	}

	inline BOOL FindByPath(const CString& sPath) const
	{
		return m_pFile.get() && m_pFile->FindByPath( sPath );
	}

	// Get amount of subfiles
	inline DWORD GetFileCount() const
	{
		return m_pFile.get() ? m_pFile->GetCount() : 0;
	}

	// Get subfile offset
	QWORD GetOffset(DWORD nIndex) const
	{
		return m_pFile.get() ? m_pFile->GetOffset( nIndex ) : 0;
	}

	// Get subfile length
	QWORD GetLength(DWORD nIndex) const
	{
		return m_pFile.get() ? m_pFile->GetLength( nIndex ) : SIZE_UNKNOWN;
	}

	// Get path of subfile
	inline CString GetPath(DWORD nIndex) const
	{
		return m_pFile.get() ? m_pFile->GetPath( nIndex ) : CString();
	}

	// Get original name of subfile
	inline CString GetName(DWORD nIndex) const
	{
		return m_pFile.get() ? m_pFile->GetName( nIndex ) : CString();
	}

	// Get completed size of subfile (in bytes)
	inline QWORD GetCompleted(DWORD nIndex) const
	{
		return m_pFile.get() ? m_pFile->GetCompleted( nIndex ) : 0;
	}

	// Select subfile (with user interaction)
	inline int SelectFile(CSingleLock* pLock) const
	{
		return m_pFile.get() ? m_pFile->SelectFile( pLock ) : -1;
	}

	// Is file under move operation?
	inline BOOL IsMoving() const
	{
		return m_bMoving;
	}

	// Get last file/disk operation error
	inline DWORD GetFileError() const
	{
		return m_nFileError;
	}

	// Clear file/disk error status
	inline void ClearFileError()
	{
		m_nFileError = ERROR_SUCCESS;
	}

protected:
	virtual CString	GetAvailableRanges() const;
	BOOL			OpenFile();
	void			CloseFile();
	void			AttachFile(auto_ptr< CFragmentedFile >& pFile);
	// Delete file(s)
	void			DeleteFile();
	// Move file(s) to destination. Returns 0 on success or file error number.
	DWORD			MoveFile(LPCTSTR pszDestination, LPPROGRESS_ROUTINE lpProgressRoutine = NULL, LPVOID lpData = NULL);
	BOOL			FlushFile();
	BOOL			ReadFile(QWORD nOffset, LPVOID pData, QWORD nLength, QWORD* pnRead = NULL);
	BOOL			WriteFile(QWORD nOffset, LPCVOID pData, QWORD nLength, QWORD* pnWritten = NULL);
	virtual void	Serialize(CArchive& ar, int nVersion);
	void			SerializeFile(CArchive& ar, int nVersion);
	void			SetVerifyStatus(TRISTATE bVerify);
	BOOL			OnVerify(LPCTSTR pszPath, BOOL bVerified);

private:
	auto_ptr< CFragmentedFile >	m_pFile;// File(s)
	DWORD			m_nFileError;		// Last file/disk error

	Fragments::List	GetPossibleFragments(const Fragments::List& oAvailable, Fragments::Fragment& oLargest);

	// Not supported
//	BOOL			AppendMetadata();
//	BOOL			AppendMetadataID3v1(HANDLE hFile, CXMLElement* pXML);
};
