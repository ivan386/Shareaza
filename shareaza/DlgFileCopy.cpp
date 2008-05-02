//
// DlgFileCopy.cpp
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
#include "XML.h"

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
	m_pFiles.AddTail( pFile->m_nIndex );
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

	if ( Settings.General.LanguageRTL ) 
	{
		m_wndProgress.ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );
		m_wndFileProg.ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );
	}
	m_wndFileProg.SetRange( 0, 400 );

	m_hThread = NULL;
	m_bThread = FALSE;

	PostMessage( WM_TIMER, 1 );
	SetTimer( 1, 500, NULL );

	return TRUE;
}

void CFileCopyDlg::OnTimer(UINT_PTR /*nIDEvent*/)
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

	m_wndProgress.SetRange( 0, short( m_pFiles.GetCount() ) );
	m_wndProgress.SetPos( 0 );

	m_bThread = TRUE;
	m_bCancel = FALSE;
	m_hThread = BeginThread( "DlgFileCopy", ThreadStart, this );
}

void CFileCopyDlg::StopOperation()
{
	if ( m_hThread == NULL ) return;

	CWaitCursor pCursor;

	m_bThread = FALSE;

	CloseThread( &m_hThread );

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
		CSchema* pSchema = NULL;
		CXMLElement* pMetadata = NULL;
		BOOL bMetadataAuto = FALSE;
		int nRating = 0;
		CString sComments;
		CString sShareTags;
		CLibraryFile* pFile;
		{
			CQuickLock oLock( Library.m_pSection );

			if ( m_pFiles.IsEmpty() ) break;

			DWORD nIndex = m_pFiles.RemoveHead();

			pFile = Library.LookupFile( nIndex );

			if ( pFile != NULL && pFile->IsAvailable() )
			{
				strName		= pFile->m_sName;
				strPath		= pFile->m_pFolder->m_sPath;
				pSchema		= pFile->m_pSchema;
				pMetadata	= pFile->m_pMetadata->Clone();
				bMetadataAuto = pFile->m_bMetadataAuto;
				nRating		= pFile->m_nRating;
				sComments	= pFile->m_sComments;
				sShareTags	= pFile->m_sShareTags;
			}
		}

		if ( NULL == pFile || ! pFile->IsAvailable() ) break;

		m_wndProgress.OffsetPos( 1 );

		m_wndFileName.SetWindowText( strName );

		if ( ProcessFile( strName, strPath ) )
		{
			CQuickLock oLock( Library.m_pSection );

			CLibraryFolder* pTargetFolder = LibraryFolders.GetFolder( m_sTarget );
			if ( pTargetFolder )
			{
				bool bNew = false;
				CLibraryFile* pTargetFile = pTargetFolder->GetFile( strName );
				if ( pTargetFile == NULL )
				{
					pTargetFile = new CLibraryFile( pTargetFolder, strName );
					pTargetFolder->m_pFiles.SetAt(
						pTargetFile->GetNameLC(), pTargetFile );
					pTargetFolder->m_nFiles++;
					pTargetFolder->m_nUpdateCookie++;
					bNew = true;
				}
				
				if ( pSchema )
				{
					pTargetFile->m_pSchema = pSchema;
					pSchema = NULL;
				}
				pTargetFile->m_bMetadataAuto = bMetadataAuto;
				pTargetFile->m_nRating = nRating;
				pTargetFile->m_sComments = sComments;
				pTargetFile->m_sShareTags = sShareTags;
				if ( pMetadata )
				{
					if ( pTargetFile->m_pMetadata )
					{
						pMetadata->Merge( pTargetFile->m_pMetadata );
						delete pTargetFile->m_pMetadata;
					}
					pTargetFile->m_pMetadata = pMetadata;
					pMetadata = NULL;
					pTargetFile->ModifyMetadata();
				}

				if ( bNew )
					Library.AddFile( pTargetFile );
			}
		}

		delete pMetadata;
/*
		// Alternate code to check if file is hashing first
		CString sCurrent, sFile;
		int nRemaining;
		LibraryBuilder.UpdateStatus( &sCurrent, &nRemaining );
		sFile = strPath + _T("\\") + strName;

		if ( sFile == sCurrent )
		{
			LoadString ( sFile, IDS_LIBRARY_BITZI_HASHED );
			sCurrent.Format( sFile, strName );
			theApp.Message( MSG_NOTICE, sCurrent  );

			LoadString ( sCurrent, IDS_STATUS_FILEERROR );
			m_wndFileName.SetWindowText( sCurrent );
			
		}
		else
		{
			m_wndFileName.SetWindowText( strName );
			ProcessFile( strName, strPath, bMetaData );
		}
		//
*/
	}


	m_bThread = FALSE;
}

//////////////////////////////////////////////////////////////////////
// CFileCopyDlg file processing

BOOL CFileCopyDlg::ProcessFile(CString& strName, CString& strPath)
{
	if ( strPath.CompareNoCase( m_sTarget ) == 0 ) return FALSE;

	CString sSource, sTarget;

	// Check if we can move the file first
	sSource = strPath + _T("\\") + strName;
	HANDLE hFile = CreateFile( sSource, GENERIC_WRITE, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	VERIFY_FILE_ACCESS( hFile, sSource )
	if ( hFile == INVALID_HANDLE_VALUE )
	{
		CString strMessage, strFormat, strName;
		LoadString( strFormat, IDS_LIBRARY_MOVE_FAIL );

		m_wndFileName.GetWindowText( strName );
		strMessage.Format( strFormat, strName );

		switch ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 ) )
		{
		case IDYES:
			break;
		case IDNO:
		default:
			CloseHandle( hFile );
			return FALSE;
		}
	}
	CloseHandle( hFile );

	// Move the file
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

	CString strError = GetErrorString();

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

	// Try moving the file
	if ( MoveFile( pszSource, pszTarget ) )
	{
		Uploads.OnRename( pszSource, pszTarget );
		return TRUE;
	}

	// Try a copy/delete. (Will usually make a duplicate of the file)
	if ( ProcessCopy( pszSource, pszTarget ) )
	{
		Uploads.OnRename( pszSource, pszTarget );
		return DeleteFile( pszSource );
	}

	Uploads.OnRename( pszSource, pszSource );

	return FALSE;
}

BOOL CFileCopyDlg::ProcessCopy(LPCTSTR pszSource, LPCTSTR pszTarget)
{
	if ( ! CheckTarget( pszTarget ) ) return FALSE;

	if ( theApp.m_hKernel != NULL )
	{
		if ( theApp.m_pfnCopyFileExW != NULL )
		{
			m_wndFileProg.SetPos( 0 );
			m_nFileProg = 0;

			BOOL bResult = theApp.m_pfnCopyFileExW( pszSource, pszTarget, CopyCallback, this,
				&m_bCancel, COPY_FILE_FAIL_IF_EXISTS );

			if ( ! bResult && ! m_bThread ) DeleteFile( pszTarget );

			return bResult;
		}
	}

	return CopyFile( pszSource, pszTarget, TRUE ); // bFailIfExists = TRUE
}

DWORD WINAPI CFileCopyDlg::CopyCallback(LARGE_INTEGER TotalFileSize, LARGE_INTEGER TotalBytesTransferred, LARGE_INTEGER /*StreamSize*/, LARGE_INTEGER /*StreamBytesTransferred*/, DWORD /*dwStreamNumber*/, DWORD /*dwCallbackReason*/, HANDLE /*hSourceFile*/, HANDLE /*hDestinationFile*/, LPVOID lpData)
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
