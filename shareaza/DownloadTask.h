//
// DownloadTask.h
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

#include "ShareazaThread.h"
#include "FileFragments.hpp"

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


class CDownloadTask : public CRazaThread
{
	DECLARE_DYNAMIC(CDownloadTask)

public:
	static void			Copy(CDownload* pDownload);
	static void			PreviewRequest(CDownload* pDownload, LPCTSTR szURL);
	static void			MergeFile(CDownload* pDownload, LPCTSTR szPath,
		BOOL bValidation = TRUE, const Fragments::List* pGaps = NULL);

	bool				HasSucceeded() const;
	void				Abort();
	bool				WasAborted() const;
	DWORD				GetFileError() const;
	dtask				GetTaskType() const;
	const CHttpRequest*	GetRequest() const;
	CBuffer*			IsPreviewAnswerValid() const;

protected:
	CDownloadTask(CDownload* pDownload, dtask nTask);
	virtual ~CDownloadTask();

	dtask				m_nTask;
	CHttpRequest*		m_pRequest;
	bool				m_bSuccess;
	CString				m_sFilename;
	CString				m_sDestination;
	DWORD				m_nFileError;
	CDownload*			m_pDownload;
	QWORD				m_nSize;
	CString				m_sMergeFilename;	// Source filename
	Fragments::List		m_oMergeGaps;		// Missed ranges in source file
	BOOL				m_bMergeValidation;	// Run validation after merging
	POSITION			m_posTorrentFile;	// Torrent file list current position
	CEvent*				m_pEvent;

	static DWORD CALLBACK CopyProgressRoutine(LARGE_INTEGER TotalFileSize,
		LARGE_INTEGER TotalBytesTransferred, LARGE_INTEGER StreamSize,
		LARGE_INTEGER StreamBytesTransferred, DWORD dwStreamNumber,
		DWORD dwCallbackReason, HANDLE hSourceFile, HANDLE hDestinationFile,
		LPVOID lpData);

	void				Construct(CDownload* pDownload);
//	void				RunAllocate();
	void				RunCopy();
	void				RunPreviewRequest();
	void				RunMerge();
	BOOL				CopyFile(HANDLE hSource, LPCTSTR pszTarget, QWORD nLength);
	void				CreatePathForFile(const CString& strBase, const CString& strPath);
	BOOL				MakeBatchTorrent();
	BOOL				CopyFileToBatch(HANDLE hSource, QWORD nOffset, QWORD nLength, LPCTSTR pszPath);

	virtual int			Run();

	DECLARE_MESSAGE_MAP()
};
