//
// DlgFilePreview.h
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

#include "DlgSkinDialog.h"

class CDownload;


class CFilePreviewDlg :
	public CSkinDialog,
	public CThreadImpl
{
	DECLARE_DYNAMIC(CFilePreviewDlg)

public:
	CFilePreviewDlg(CDownload* pDownload, CWnd* pParent = NULL);
	virtual ~CFilePreviewDlg();

	enum { IDD = IDD_FILE_PREVIEW };

	BOOL		Create();
	static void	OnSkinChange(BOOL bSet);
	static void	CloseAll();

	CCriticalSection		m_pSection;
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
	CArray< QWORD >	m_pRanges;

protected:
	CButton			m_wndCancel;
	CProgressCtrl	m_wndProgress;
	CStatic			m_wndStatus;
	CStatic			m_wndName;
	BOOL			m_bCancel;
	CString			m_sExecute;
	IDownloadPreviewPlugin*				m_pPlugin;
	static CList< CFilePreviewDlg* >	m_pWindows;

	void		SetDownload(CDownload* pDownload);
	void		OnRun();
	BOOL		RunPlugin(HANDLE hFile);
	BOOL		LoadPlugin(LPCTSTR pszType);
	BOOL		RunManual(HANDLE hFile);
	BOOL		QueueDeleteFile(LPCTSTR pszFile);
	BOOL		ExecuteFile(LPCTSTR pszFile);
	void		UpdateProgress(BOOL bRange, QWORD nRange, BOOL bPosition, QWORD nPosition);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnClose();

	DECLARE_MESSAGE_MAP()

// IDownloadPreviewSite
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
};
