//
// DownloadWithFile.h
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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

#if !defined(AFX_DOWNLOADWITHFILE_H__79FE6B65_04DF_4CD5_A1BC_9AF8429664DC__INCLUDED_)
#define AFX_DOWNLOADWITHFILE_H__79FE6B65_04DF_4CD5_A1BC_9AF8429664DC__INCLUDED_

#pragma once

#include "DownloadWithTransfers.h"
#include "FileFragments.hpp"

class CFragmentedFile;

class CDownloadWithFile : public CDownloadWithTransfers
{
// Construction
public:
	CDownloadWithFile();
	virtual ~CDownloadWithFile();

// Attributes
public:
	CFragmentedFile*	m_pFile;
	DWORD				m_tReceived;
	BOOL				m_bDiskFull;

// Operations
public:
	float			GetProgress() const;
	QWORD			GetVolumeComplete() const;
	QWORD			GetVolumeRemaining() const;
	DWORD			GetTimeRemaining() const;
	CString			GetDisplayName() const;
public:
    const FF::SimpleFragmentList& GetEmptyFragmentList() const;
    FF::SimpleFragmentList GetPossibleFragments(const FF::SimpleFragmentList& oAvailable, FF::SimpleFragment& oLargest);
	BOOL			GetFragment(CDownloadTransfer* pTransfer);
	BOOL			IsPositionEmpty(QWORD nOffset);
    BOOL            AreRangesUseful(const FF::SimpleFragmentList& oAvailable);
	BOOL			IsRangeUseful(QWORD nOffset, QWORD nLength);
	virtual CString	GetAvailableRanges() const;
	BOOL			ClipUploadRange(QWORD nOffset, QWORD& nLength) const;
	BOOL			GetRandomRange(QWORD& nOffset, QWORD& nLength) const;
	BOOL			PrepareFile();
	BOOL			SubmitData(QWORD nOffset, LPBYTE pData, QWORD nLength);
	QWORD			EraseRange(QWORD nOffset, QWORD nLength);
	BOOL			MakeComplete();
protected:
	BOOL			OpenFile();
	void			CloseFile();
	void			DeleteFile(BOOL bForce = FALSE);
	BOOL			RunFile(DWORD tNow);
	BOOL			WriteMetadata(LPCTSTR pszPath);
	BOOL			AppendMetadata();
	BOOL			AppendMetadataID3v1(HANDLE hFile, CXMLElement* pXML);
protected:
	virtual void	Serialize(CArchive& ar, int nVersion);

	friend class CDownloadTransfer;
};

#endif // !defined(AFX_DOWNLOADWITHFILE_H__79FE6B65_04DF_4CD5_A1BC_9AF8429664DC__INCLUDED_)
