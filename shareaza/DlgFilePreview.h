//
// DlgFilePreview.h
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

#if !defined(AFX_DLGFILEPREVIEW_H__E2307205_BEF2_4A77_A7FA_BC92F46BCBF6__INCLUDED_)
#define AFX_DLGFILEPREVIEW_H__E2307205_BEF2_4A77_A7FA_BC92F46BCBF6__INCLUDED_

#pragma once

#include "DlgSkinDialog.h"

class CDownload;


class CFilePreviewDlg :
	public CSkinDialog,
	public CThreadImpl
{
// Construction
public:
	CFilePreviewDlg(CDownload* pDownload, CWnd* pParent = NULL);
	virtual ~CFilePreviewDlg();
	DECLARE_DYNAMIC(CFilePreviewDlg)

// Attributes
public:
	CCriticalSection m_pSection;
	CDownload*		m_pDownload;
	CString			m_sDisplayName;
	CString			m_sSourceName;
	CString			m_sTargetName;
	QWORD			m_nRange;
	QWORD			m_nPosition;
	DWORD			m_nScaled;
	DWORD			m_nOldScaled;
	CString			m_sStatus;
	CString			m_sOldStatus;
	CArray< DWORD >	m_pRanges;
protected:
	BOOL			m_bCancel;
	CString			m_sExecute;
protected:
	IDownloadPreviewPlugin*	m_pPlugin;
	static CList< CFilePreviewDlg* > m_pWindows;

// Operations
public:
	BOOL		Create();
	static void	OnSkinChange(BOOL bSet);
	static void	CloseAll();
protected:
	void		SetDownload(CDownload* pDownload);
	void		OnRun();
	BOOL		RunPlugin(HANDLE hFile);
	BOOL		LoadPlugin(LPCTSTR pszType);
	BOOL		RunManual(HANDLE hFile);
	BOOL		QueueDeleteFile(LPCTSTR pszFile);
	BOOL		ExecuteFile(LPCTSTR pszFile);
	void		UpdateProgress(BOOL bRange, QWORD nRange, BOOL bPosition, QWORD nPosition);

// Dialog Data
public:
	//{{AFX_DATA(CFilePreviewDlg)
	enum { IDD = IDD_FILE_PREVIEW };
	CButton	m_wndCancel;
	CProgressCtrl	m_wndProgress;
	CStatic	m_wndStatus;
	CStatic	m_wndName;
	//}}AFX_DATA

// Overrides
public:
	//{{AFX_VIRTUAL(CFilePreviewDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CFilePreviewDlg)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

// IDownloadPreviewSite
protected:
	BEGIN_INTERFACE_PART(DownloadPreviewSite, IDownloadPreviewSite)
		STDMETHOD(GetSuggestedFilename)(BSTR FAR* psFile);
		STDMETHOD(GetAvailableRanges)(SAFEARRAY FAR* FAR* ppArray);
		STDMETHOD(SetProgressRange)(DWORD nRange);
		STDMETHOD(SetProgressPosition)(DWORD nPosition);
		STDMETHOD(SetProgressMessage)(BSTR sMessage);
		STDMETHOD(QueueDeleteFile)(BSTR sTempFile);
		STDMETHOD(ExecuteFile)(BSTR sFile);
	END_INTERFACE_PART(DownloadPreviewSite)

	DECLARE_INTERFACE_MAP()

public:
	afx_msg void OnClose();
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_DLGFILEPREVIEW_H__E2307205_BEF2_4A77_A7FA_BC92F46BCBF6__INCLUDED_)
