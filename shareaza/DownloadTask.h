//
// DownloadTask.h
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

#if !defined(AFX_DOWNLOADTASK_H__F90D6932_4D5C_4670_AA6D_9C8316A0172A__INCLUDED_)
#define AFX_DOWNLOADTASK_H__F90D6932_4D5C_4670_AA6D_9C8316A0172A__INCLUDED_

#pragma once

#include "BTInfo.h"
#include "HttpRequest.h"

class CDownload;

class CDownloadTask : public CWinThread
{
// Construction
public:
	CDownloadTask(CDownload* pDownload, int nTask);
	CDownloadTask(CDownload* pDownload, const CString& strPreviewURL);
	CDownloadTask(CDownload* pDownload, HANDLE hSelectedFile);
	virtual ~CDownloadTask();

	DECLARE_DYNAMIC(CDownloadTask)

// Attributes
public:
	int			m_nTask;
	CDownload*	m_pDownload;
	BOOL		m_bSuccess;
private:
	int			m_nTorrentFile;
public:
	QWORD		m_nSize;
	CString		m_sName;
	CString		m_sFilename;
	CString		m_sPath;
	CBTInfo		m_pTorrent;
	CHttpRequest m_pRequest;
protected:
	CEvent*		m_pEvent;
	HANDLE		m_hSelectedFile;

// Enumerations
public:
	enum { dtaskAllocate, dtaskCopySimple, dtaskCopyTorrent, dtaskPreviewRequest, dtaskCheckHash, dtaskMergeFile };

// Operations
public:
	void	Abort();
	BOOL	WasAborted();
protected:
	void	RunAllocate();
	void	RunCopySimple();
	void	RunCopyTorrent();
	void	RunMerge();
	BOOL	CopyFile(HANDLE hSource, LPCTSTR pszTarget, QWORD nLength);
	void	CreatePathForFile(const CString& strBase, const CString& strPath);
private:
	void	Construct(CDownload* pDownload);

// Utilities
public:
	static CString SafeFilename(LPCTSTR pszName);
	CBuffer* IsPreviewAnswerValid();

// Overrides
public:
	//{{AFX_VIRTUAL(CDownloadTask)
	public:
	virtual BOOL InitInstance();
	virtual int Run();
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CDownloadTask)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_DOWNLOADTASK_H__F90D6932_4D5C_4670_AA6D_9C8316A0172A__INCLUDED_)
