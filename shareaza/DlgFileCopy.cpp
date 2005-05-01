//
// DlgFileCopy.cpp
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "Library.h"
#include "LibraryFolders.h"
#include "SharedFolder.h"
#include "SharedFile.h"
#include "Uploads.h"
#include "DlgFileCopy.h"
#include "CtrlSharedFolder.h"
#include "Skin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CFileCopyDlg, CSkinDialog)
	//{{AFX_MSG_MAP(CFileCopyDlg)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFileCopyDlg dialog

CFileCopyDlg::CFileCopyDlg(CWnd* pParent, BOOL bMove) : CSkinDialog(CFileCopyDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFileCopyDlg)
	//}}AFX_DATA_INIT
	m_bMove		= bMove;
	m_nCookie	= 0;
}

void CFileCopyDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFileCopyDlg)
	DDX_Control(pDX, IDC_MESSAGE_MOVE, m_wndMove);
	DDX_Control(pDX, IDC_MESSAGE_COPY, m_wndCopy);
	DDX_Control(pDX, IDC_FILE_NAME, m_wndFileName);
	DDX_Control(pDX, IDC_PROGRESS_FILE, m_wndFileProg);
	DDX_Control(pDX, IDCANCEL, m_wndCancel);
	DDX_Control(pDX, IDOK, m_wndOK);
	DDX_Control(pDX, IDC_PROGRESS, m_wndProgress);
	DDX_Control(pDX, IDC_PLACEHOLDER, m_wndPlaceholder);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CFileCopyDlg message handlers

void CFileCopyDlg::AddFile(CLibraryFile* pFile)
{
	m_pFiles.AddTail( (LPVOID)pFile->m_nIndex );
}

BOOL CFileCopyDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CFileCopyDlg"), IDR_LIBRARYFRAME );
	SelectCaption( this, m_bMove ? 1 : 0 );

	CString strCaption, strBase;

	CWnd* pMessage = m_bMove ? &m_wndMove : &m_wndCopy;
	pMessage->GetWindowText( strBase );
	strCaption.Format( strBase, m_pFiles.GetCount() );
	pMessage->SetWindowText( strCaption );
	pMessage->ShowWindow( SW_SHOW );

	CRect rc;
	m_wndPlaceholder.GetWindowRect( &rc );
	ScreenToClient( &rc );
	if ( ! m_wndTree.Create( WS_VISIBLE|WS_TABSTOP|WS_BORDER, rc, this, IDC_FOLDERS ) ) return -1;
	m_wndTree.SetMultiSelect( FALSE );

	{
		CQuickLock oLock( Library.m_pSection );

		m_nCookie = Library.m_nUpdateCookie;
		m_wndTree.Update();

		if ( CLibraryFolder* pFolder = LibraryFolders.GetFolder( m_sTarget ) )
		{
			m_wndTree.SelectFolder( pFolder );
		}
	}

	m_wndFileProg.SetRange( 0, 400 );

	m_hThread = NULL;
	m_bThread = FALSE;

	PostMessage( WM_TIMER, 1 );
	SetTimer( 1, 500, NULL );

	return TRUE;
}

void CFileCopyDlg::OnTimer(UINT nIDEvent)
{
	if ( m_hThread != NULL )
	{
		if ( m_bThread ) return;

		StopOperation();

		PostMessage( WM_COMMAND, IDCANCEL );

		return;
	}

	if ( ! m_wndTree.IsWindowEnabled() ) return;

	if ( m_nCookie != Library.m_nUpdateCookie )
	{
		CSingleLock pLock( &Library.m_pSection );

		if ( pLock.Lock( 500 ) )
		{
			m_nCookie = Library.m_nUpdateCookie;
			m_wndTree.Update();
		}
	}

	m_wndOK.EnableWindow( m_wndTree.GetSelectedFolderIterator() != NULL );
}

BOOL CFileCopyDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR* pNotify = (NMHDR*)lParam;

	if ( pNotify->code == NM_DBLCLK )
	{
		PostMessage( WM_COMMAND, IDOK );
		return TRUE;
	}

	return CSkinDialog::OnNotify(wParam, lParam, pResult);
}

void CFileCopyDlg::OnOK()
{
	CSingleLock pLock( &Library.m_pSection, TRUE );

	POSITION pos = m_wndTree.GetSelectedFolderIterator();
	if ( pos == NULL ) return;

	CLibraryFolder* pFolder = m_wndTree.GetNextSelectedFolder( pos );

	m_sTarget = pFolder->m_sPath;

	pLock.Unlock();

	StartOperation();
}

void CFileCopyDlg::OnCancel()
{
	if ( m_hThread )
	{
		StopOperation();
		m_wndFileName.SetWindowText( _T("Operation cancelled") );
		return;
	}

	StopOperation();

	CSkinDialog::OnCancel();
}

//////////////////////////////////////////////////////////////////////
// CFileCopyDlg operation control

void CFileCopyDlg::StartOperation()
{
	if ( m_hThread ) return;

	m_wndTree.EnableWindow( FALSE );
	m_wndOK.EnableWindow( FALSE );

	m_wndProgress.SetRange( 0, m_pFiles.GetCount() );
	m_wndProgress.SetPos( 0 );

	m_bThread = TRUE;
	m_bCancel = FALSE;
	CWinThread* pThread = AfxBeginThread( ThreadStart, this, THREAD_PRIORITY_NORMAL );
	m_hThread = pThread->m_hThread;
}

void CFileCopyDlg::StopOperation()
{
	if ( m_hThread == NULL ) return;

	CWaitCursor pCursor;

	m_bThread = FALSE;

    int nAttempt = 100;
	for ( ; nAttempt > 0 ; nAttempt-- )
	{
		DWORD nCode;

		if ( ! GetExitCodeThread( m_hThread, &nCode ) ) break;
		if ( nCode != STILL_ACTIVE ) break;
		Sleep( 250 );
	}

	if ( nAttempt == 0 )
	{
		TerminateThread( m_hThread, 0 );
		theApp.Message( MSG_DEBUG, _T("WARNING: Terminating FileOp thread.") );
		Sleep( 250 );
	}

	m_hThread = NULL;

	//m_wndCancel.SetWindowText( _T("&Close") );
	CString sText;
	LoadString ( sText, IDS_GENERAL_CLOSE );
	m_wndCancel.SetWindowText( sText );
	m_wndProgress.EnableWindow( FALSE );
}

//////////////////////////////////////////////////////////////////////
// CFileCopyDlg operation thread

UINT CFileCopyDlg::ThreadStart(LPVOID pParam)
{
	CFileCopyDlg* pClass = (CFileCopyDlg*)pParam;
	pClass->OnRun();
	return 0;
}

void CFileCopyDlg::OnRun()
{
	while ( m_bThread )
	{
		CString strName, strPath;
		BOOL bMetaData;

		CLibraryFile* pFile;
		{
			CQuickLock oLock( Library.m_pSection );

			if ( m_pFiles.IsEmpty() ) break;

			DWORD nIndex = (DWORD)m_pFiles.RemoveHead();

			pFile = Library.LookupFile( nIndex );

			if ( pFile != NULL && pFile->IsAvailable() )
			{
				strName		= pFile->m_sName;
				strPath		= pFile->m_pFolder->m_sPath;
				bMetaData	= ( pFile->m_pMetadata != NULL ) && ! pFile->m_bMetadataAuto;
			}
		}

		if ( NULL == pFile || ! pFile->IsAvailable() ) break;

		m_wndProgress.OffsetPos( 1 );
		m_wndFileName.SetWindowText( strName );

		ProcessFile( strName, strPath, bMetaData );
	}

	m_bThread = FALSE;
}

//////////////////////////////////////////////////////////////////////
// CFileCopyDlg file processing

BOOL CFileCopyDlg::ProcessFile(CString& strName, CString& strPath, BOOL bMetaData)
{
	if ( strPath.CompareNoCase( m_sTarget ) == 0 ) return FALSE;

	CString sSource, sTarget;

	if ( bMetaData )
	{
		CString strMetaFolder = strPath + _T("\\Metadata");

		sSource = strMetaFolder + _T("\\") + strName + _T(".xml");

		sTarget = m_sTarget + _T("\\Metadata");
		CreateDirectory( sTarget, NULL );
		sTarget += _T("\\") + strName + _T(".xml");

		if ( sSource.CompareNoCase( sTarget ) == 0 ) return FALSE;

		if ( m_bMove )
		{
			if ( ProcessMove( sSource, sTarget ) )
			{
				Sleep( 50 );
				RemoveDirectory( strMetaFolder );
			}
		}
		else
		{
			ProcessCopy( sSource, sTarget );
		}
	}

	sSource = strPath + _T("\\") + strName;
	sTarget = m_sTarget + _T("\\") + strName;

	if ( sSource.CompareNoCase( sTarget ) == 0 ) return FALSE;

	if ( m_bMove )
	{
		if ( ! ProcessMove( sSource, sTarget ) ) return FALSE;
	}
	else
	{
		if ( ! ProcessCopy( sSource, sTarget ) ) return FALSE;
	}

	return TRUE;
}

BOOL CFileCopyDlg::CheckTarget(LPCTSTR pszTarget)
{
	if ( GetFileAttributes( pszTarget ) == 0xFFFFFFFF ) return TRUE;

	CString strFormat, strMessage;

	LoadString( strFormat, IDS_LIBRARY_TARGET_EXISTS );
	strMessage.Format( strFormat, pszTarget );

	switch ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNOCANCEL|MB_DEFBUTTON2 ) )
	{
	case IDYES:
		break;
	case IDCANCEL:
		m_bThread = FALSE;
	case IDNO:
	default:
		return FALSE;
	}

	if ( DeleteFile( pszTarget ) ) return TRUE;

	CString strError = theApp.GetErrorString();

	LoadString( strFormat, IDS_LIBRARY_DELETE_FAIL );
	strMessage.Format( strFormat, pszTarget );
	strMessage += _T("\r\n\r\n") + strError;

	AfxMessageBox( strMessage, MB_ICONEXCLAMATION );

	return FALSE;
}

BOOL CFileCopyDlg::ProcessMove(LPCTSTR pszSource, LPCTSTR pszTarget)
{
	if ( ! CheckTarget( pszTarget ) ) return FALSE;

	Uploads.OnRename( pszSource );

	if ( MoveFile( pszSource, pszTarget ) )
	{
		Uploads.OnRename( pszSource, pszTarget );
		return TRUE;
	}

	if ( ProcessCopy( pszSource, pszTarget ) )
	{
		Uploads.OnRename( pszSource, pszTarget );
		return DeleteFile( pszSource );
	}

	Uploads.OnRename( pszSource, pszSource );

	return FALSE;
}

typedef DWORD (WINAPI *LPPROGRESS_ROUTINE_X)(LARGE_INTEGER TotalFileSize, LARGE_INTEGER TotalBytesTransferred, LARGE_INTEGER StreamSize, LARGE_INTEGER StreamBytesTransferred, DWORD dwStreamNumber, DWORD dwCallbackReason, HANDLE hSourceFile, HANDLE hDestinationFile, LPVOID lpData OPTIONAL);

BOOL CFileCopyDlg::ProcessCopy(LPCTSTR pszSource, LPCTSTR pszTarget)
{
	if ( ! CheckTarget( pszTarget ) ) return FALSE;

	HINSTANCE hKernel = LoadLibrary( _T("kernel32.dll") );

	if ( hKernel != NULL )
	{
		BOOL (WINAPI *pfnCopyFileEx)(LPCTSTR, LPCTSTR, LPPROGRESS_ROUTINE_X, LPVOID, LPBOOL, DWORD);

		(FARPROC&)pfnCopyFileEx = GetProcAddress( hKernel, "CopyFileExW" );

		if ( pfnCopyFileEx != NULL )
		{
			m_wndFileProg.SetPos( 0 );
			m_nFileProg = 0;

			BOOL bResult = (*pfnCopyFileEx)( pszSource, pszTarget, CopyCallback, this,
				&m_bCancel, 1 );	// COPY_FILE_FAIL_IF_EXISTS

			FreeLibrary( hKernel );

			if ( ! bResult && ! m_bThread ) DeleteFile( pszTarget );

			return bResult;
		}

		FreeLibrary( hKernel );
	}

	return CopyFile( pszSource, pszTarget, TRUE );
}

DWORD WINAPI CFileCopyDlg::CopyCallback(LARGE_INTEGER TotalFileSize, LARGE_INTEGER TotalBytesTransferred, LARGE_INTEGER StreamSize, LARGE_INTEGER StreamBytesTransferred, DWORD dwStreamNumber, DWORD dwCallbackReason, HANDLE hSourceFile, HANDLE hDestinationFile, LPVOID lpData)
{
	CFileCopyDlg* pDlg = (CFileCopyDlg*)lpData;

	if ( ! pDlg->m_bThread ) return 1;

	if ( TotalFileSize.LowPart )
	{
		double nProgress = ( (double)TotalBytesTransferred.LowPart / (double)TotalFileSize.LowPart );
		int iProgress = (int)( nProgress * 400 );

		if ( iProgress != pDlg->m_nFileProg )
		{
			pDlg->m_wndFileProg.SetPos( pDlg->m_nFileProg = iProgress );
		}
	}

	return 0;
}
