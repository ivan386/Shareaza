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
	TRISTATE		m_bVerify;			// Verify status (TRI_TRUE - verified, TRI_FALSE - failed, TRI_UNKNOWN - not yet)
	DWORD			m_tReceived;
private:
	auto_ptr< CFragmentedFile >	m_pFile;// File(s)
	DWORD			m_nFileError;		// Last file/disk error

// Operations
public:
	float				GetProgress() const;
	QWORD				GetVolumeComplete() const;
	QWORD				GetVolumeRemaining() const;
	DWORD				GetTimeRemaining() const;
	CString				GetDisplayName() const;
	BOOL				IsValid() const;
	BOOL				IsFileOpen() const;
	BOOL				IsComplete() const;
	BOOL				PrepareFile();
	BOOL				IsPositionEmpty(QWORD nOffset);
	BOOL				ClipUploadRange(QWORD nOffset, QWORD& nLength) const;
	BOOL				GetRandomRange(QWORD& nOffset, QWORD& nLength) const;
	BOOL				SubmitData(QWORD nOffset, LPBYTE pData, QWORD nLength);
	QWORD				EraseRange(QWORD nOffset, QWORD nLength);
	BOOL				MakeComplete();
	QWORD				InvalidateFileRange(QWORD nOffset, QWORD nLength);
	// Get list of all fragments which must be downloaded
	Fragments::List		GetFullFragmentList() const;
	// Get list of empty fragments
	Fragments::List		GetEmptyFragmentList() const;
	CFragmentedFile*	GetFile();
	BOOL				FindByPath(const CString& sPath) const;
	DWORD				GetFileCount() const;
	QWORD				GetOffset(DWORD nIndex) const;
	QWORD				GetLength(DWORD nIndex) const;
	CString				GetPath(DWORD nIndex) const;
	CString				GetName(DWORD nIndex) const;
	QWORD				GetCompleted(DWORD nIndex) const;
	int					SelectFile(CSingleLock* pLock) const;
	DWORD				GetFileError() const;
	void				SetFileError(DWORD nFileError);
	void				ClearFileError();
	DWORD				MoveFile(LPCTSTR pszDestination, LPPROGRESS_ROUTINE lpProgressRoutine = NULL, LPVOID lpData = NULL);
protected:
	// Open files of this download
	BOOL				OpenFile();
	// Close files of this download
	void				CloseFile();
	// Close files of this download, clear its list but save all other data
	void				ClearFile();
	// Replace files by specified ones, old one will be dropped
	void				AttachFile(auto_ptr< CFragmentedFile >& pFile);
	// Delete files of this download
	void				DeleteFile();
	// Flush unsaved data to disk
	BOOL				FlushFile();
	BOOL				ReadFile(QWORD nOffset, LPVOID pData, QWORD nLength, QWORD* pnRead = NULL);
	BOOL				WriteFile(QWORD nOffset, LPCVOID pData, QWORD nLength, QWORD* pnWritten = NULL);
	void				SerializeFile(CArchive& ar, int nVersion);
	void				SetVerifyStatus(TRISTATE bVerify);
	BOOL				OnVerify(LPCTSTR pszPath, BOOL bVerified);

	// Not supported
//	BOOL				AppendMetadata();
//	BOOL				AppendMetadataID3v1(HANDLE hFile, CXMLElement* pXML);

protected:
	virtual CString	GetAvailableRanges() const;
	virtual void	Serialize(CArchive& ar, int nVersion);
};
