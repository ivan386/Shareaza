//
// DownloadTask.h
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

#include "BTInfo.h"
#include "HttpRequest.h"
#include "ShareazaThread.h"


class CDownload;

class CDownloadTask : public CRazaThread
{
public:
	enum dtask { dtaskAllocate, dtaskCopySimple, dtaskCopyTorrent, 
		dtaskPreviewRequest, dtaskCheckHash, dtaskMergeFile, dtaskCreateBatch
	};

	CDownloadTask(CDownload* pDownload, dtask nTask, LPCTSTR szParam1 = NULL);
	virtual ~CDownloadTask();

	DECLARE_DYNAMIC(CDownloadTask)

	void			Abort();
	BOOL			WasAborted();
	static CString	SafeFilename(LPCTSTR pszName);
	CBuffer*		IsPreviewAnswerValid();

	dtask		m_nTask;
	CDownload*	m_pDownload;
	BOOL		m_bSuccess;
	QWORD		m_nSize;
	CString		m_sName;
	CString		m_sFilename;
	CString		m_sPath;
	CBTInfo		m_pTorrent;
	CHttpRequest m_pRequest;
	DWORD		m_dwFileError;
	CString		m_sMergeFilename;

private:
	void	Construct(CDownload* pDownload);

protected:
	int			m_nTorrentFile;
	CEvent*		m_pEvent;

	void	RunAllocate();
	void	RunCopySimple();
	void	RunCopyTorrent();
	void	RunMerge();
	BOOL	CopyFile(HANDLE hSource, LPCTSTR pszTarget, QWORD nLength);
	void	CreatePathForFile(const CString& strBase, const CString& strPath);
	BOOL	MakeBatchTorrent();
	BOOL	CopyFileToBatch(HANDLE hSource, QWORD nOffset, QWORD nLength, LPCTSTR pszPath);

	static DWORD CALLBACK CopyProgressRoutine(LARGE_INTEGER TotalFileSize,
		LARGE_INTEGER TotalBytesTransferred, LARGE_INTEGER StreamSize,
		LARGE_INTEGER StreamBytesTransferred, DWORD dwStreamNumber,
		DWORD dwCallbackReason, HANDLE hSourceFile, HANDLE hDestinationFile,
		LPVOID lpData);

	virtual int Run();

	DECLARE_MESSAGE_MAP()
};
