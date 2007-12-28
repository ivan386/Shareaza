//
// DownloadWithFile.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

#if !defined(AFX_DOWNLOADWITHFILE_H__79FE6B65_04DF_4CD5_A1BC_9AF8429664DC__INCLUDED_)
#define AFX_DOWNLOADWITHFILE_H__79FE6B65_04DF_4CD5_A1BC_9AF8429664DC__INCLUDED_

#pragma once

#include "DownloadWithTransfers.h"
#include "FileFragments.hpp"

class CFragmentedFile;

class CDownloadWithFile : public CDownloadWithTransfers
{
// Construction
protected:
	CDownloadWithFile();
	virtual ~CDownloadWithFile();

// Attributes
public:
	CFragmentedFile*	m_pFile;
	BOOL				m_bDiskFull;
	DWORD				m_tReceived;

// Operations
public:
	float			GetProgress() const;
	QWORD			GetVolumeComplete() const;
	QWORD			GetVolumeRemaining() const;
	DWORD			GetTimeRemaining() const;
	CString			GetDisplayName() const;
public:
	const Fragments::List& GetEmptyFragmentList() const;
	BOOL			GetFragment(CDownloadTransfer* pTransfer);
	BOOL			IsPositionEmpty(QWORD nOffset);
	BOOL			AreRangesUseful(const Fragments::List& oAvailable);
	BOOL			IsRangeUseful(QWORD nOffset, QWORD nLength);
	BOOL			IsRangeUsefulEnough(CDownloadTransfer* pTransfer, QWORD nOffset, QWORD nLength);
	BOOL			ClipUploadRange(QWORD nOffset, QWORD& nLength) const;
	BOOL			PrepareFile();
	BOOL			GetRandomRange(QWORD& nOffset, QWORD& nLength) const;
	BOOL			SubmitData(QWORD nOffset, LPBYTE pData, QWORD nLength);
	QWORD			EraseRange(QWORD nOffset, QWORD nLength);
	BOOL			MakeComplete();
protected:
	virtual CString	GetAvailableRanges() const;
	BOOL			OpenFile();
	void			CloseFile();
	void			DeleteFile(BOOL bForce = FALSE);
	BOOL			RunFile(DWORD tNow);
	BOOL			WriteMetadata(LPCTSTR pszPath);
	BOOL			AppendMetadata();
	virtual void	Serialize(CArchive& ar, int nVersion);
private:
	Fragments::List	GetPossibleFragments(const Fragments::List& oAvailable, Fragments::Fragment& oLargest);
	BOOL			AppendMetadataID3v1(HANDLE hFile, CXMLElement* pXML);
	
};

#endif // !defined(AFX_DOWNLOADWITHFILE_H__79FE6B65_04DF_4CD5_A1BC_9AF8429664DC__INCLUDED_)
