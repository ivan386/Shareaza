//
// DlgFileCopy.h
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

#if !defined(AFX_DLGFILECOPY_H__35C04685_7B85_4022_A702_983686FCBD9B__INCLUDED_)
#define AFX_DLGFILECOPY_H__35C04685_7B85_4022_A702_983686FCBD9B__INCLUDED_

#pragma once

#include "DlgSkinDialog.h"
#include "CtrlSharedFolder.h"

class CLibraryFile;


class CFileCopyDlg : public CSkinDialog
{
// Construction
public:
	CFileCopyDlg(CWnd* pParent = NULL, BOOL bMove = FALSE);

// Dialog Data
public:
	//{{AFX_DATA(CFileCopyDlg)
	enum { IDD = IDD_FILE_COPY };
	CStatic	m_wndMove;
	CStatic	m_wndCopy;
	CStatic	m_wndFileName;
	CProgressCtrl	m_wndFileProg;
	CButton	m_wndCancel;
	CButton	m_wndOK;
	CProgressCtrl	m_wndProgress;
	CStatic	m_wndPlaceholder;
	//}}AFX_DATA

// Attributes
public:
	CString				m_sTarget;
protected:
	BOOL				m_bMove;
	CPtrList			m_pFiles;
	CLibraryFolderCtrl	m_wndTree;
	DWORD				m_nCookie;
protected:
	HANDLE				m_hThread;
	BOOL				m_bThread;
	BOOL				m_bCancel;
	int					m_nFileProg;

// Operations
public:
	void		AddFile(CLibraryFile* pFile);
protected:
	void		StartOperation();
	void		StopOperation();
	void		OnRun();
	BOOL		ProcessFile(CString& strName, CString& strPath, BOOL bMetaData);
	BOOL		CheckTarget(LPCTSTR pszTarget);
	BOOL		ProcessMove(LPCTSTR pszSource, LPCTSTR pszTarget);
	BOOL		ProcessCopy(LPCTSTR pszSource, LPCTSTR pszTarget);
protected:
	static UINT			ThreadStart(LPVOID pParam);
	static DWORD WINAPI	CopyCallback(LARGE_INTEGER TotalFileSize, LARGE_INTEGER TotalBytesTransferred, LARGE_INTEGER StreamSize, LARGE_INTEGER StreamBytesTransferred, DWORD dwStreamNumber, DWORD dwCallbackReason, HANDLE hSourceFile, HANDLE hDestinationFile, LPVOID lpData);


// Overrides
public:
	//{{AFX_VIRTUAL(CFileCopyDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CFileCopyDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnTimer(UINT nIDEvent);
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#define IDC_FOLDERS	100

#endif // !defined(AFX_DLGFILECOPY_H__35C04685_7B85_4022_A702_983686FCBD9B__INCLUDED_)
