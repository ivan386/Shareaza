//
// DownloadTask.h
//
// Copyright (c) Shareaza Development Team, 2002-2013.
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
#include "ThreadImpl.h"

class CDownload;
class CHttpRequest;


enum dtask
{
	dtaskNone = 0,			// No task
	dtaskAllocate,			// Allocating...
	dtaskCopy,				// Moving...
	dtaskPreviewRequest,	// Previewing...
	dtaskMergeFile			// Merging...
};


class CDownloadTask : public CThreadImpl
{
public:
	CDownloadTask(CDownload* pDownload);
	virtual ~CDownloadTask();

	void				Allocate();
	void				Copy();
	void				PreviewRequest(LPCTSTR szURL);
	void				MergeFile(CList< CString >* pFiles, BOOL bValidation, const Fragments::List* pGaps);

	bool				HasSucceeded() const;
	void				Abort();
	DWORD				GetFileError() const;
	dtask				GetTaskType() const;
	CString				GetRequest() const;
	// Get progress of current operation (0..100%)
	float				GetProgress() const;
	CBuffer*			IsPreviewAnswerValid(const Hashes::Sha1Hash& oRequestedSHA1) const;

private:
	CDownload*			m_pDownload;
	dtask				m_nTask;
	CAutoPtr< CHttpRequest > m_pRequest;
	bool				m_bSuccess;
	CString				m_sDestination;
	DWORD				m_nFileError;
	CList< CString >	m_oMergeFiles;		// Source filenames
	Fragments::List		m_oMergeGaps;		// Missed ranges in source file
	BOOL				m_bMergeValidation;	// Run validation after merging
	float				m_fProgress;		// Progress of current operation (0..100%)

	void				Construct(dtask nTask);

	static DWORD CALLBACK CopyProgressRoutine(LARGE_INTEGER TotalFileSize,
		LARGE_INTEGER TotalBytesTransferred, LARGE_INTEGER StreamSize,
		LARGE_INTEGER StreamBytesTransferred, DWORD dwStreamNumber,
		DWORD dwCallbackReason, HANDLE hSourceFile, HANDLE hDestinationFile,
		LPVOID lpData);

	void				RunAllocate();
	void				RunCopy();
	void				RunPreviewRequest();
	void				RunMerge();
	void				OnRun();
};
