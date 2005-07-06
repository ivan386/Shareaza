//
// DlgFilePreview.cpp
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
#include "Transfers.h"
#include "Downloads.h"
#include "Download.h"
#include "FragmentedFile.h"
#include "TransferFile.h"
#include "DlgFilePreview.h"
#include "FileExecutor.h"
#include "Plugins.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CFilePreviewDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CFilePreviewDlg, CSkinDialog)
	//{{AFX_MSG_MAP(CFilePreviewDlg)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()
END_MESSAGE_MAP()

BEGIN_INTERFACE_MAP(CFilePreviewDlg, CSkinDialog)
	INTERFACE_PART(CFilePreviewDlg, IID_IDownloadPreviewSite, DownloadPreviewSite)
END_INTERFACE_MAP()

CPtrList CFilePreviewDlg::m_pWindows;


/////////////////////////////////////////////////////////////////////////////
// CFilePreviewDlg dialog

CFilePreviewDlg::CFilePreviewDlg(CDownload* pDownload, CWnd* pParent) : CSkinDialog( CFilePreviewDlg::IDD, pParent )
{
	//{{AFX_DATA_INIT(CFilePreviewDlg)
	//}}AFX_DATA_INIT
	
	m_pDownload	= NULL;
	m_pPlugin	= NULL;
	m_bThread	= FALSE;
	m_hThread	= NULL;
	m_pPlugin	= NULL;
	
	SetDownload( pDownload );
}

CFilePreviewDlg::~CFilePreviewDlg()
{
	if ( POSITION pos = m_pWindows.Find( this ) ) m_pWindows.RemoveAt( pos );
	ASSERT( m_pDownload == NULL );
}

void CFilePreviewDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CFilePreviewDlg)
	DDX_Control(pDX, IDCANCEL, m_wndCancel);
	DDX_Control(pDX, IDC_PROGRESS, m_wndProgress);
	DDX_Control(pDX, IDC_PREVIEW_STATUS, m_wndStatus);
	DDX_Control(pDX, IDC_FILE_NAME, m_wndName);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CFilePreviewDlg operations

void CFilePreviewDlg::SetDownload(CDownload* pDownload)
{
	ASSERT( m_pDownload == NULL );
	m_pDownload = pDownload;
	ASSERT( m_pDownload != NULL );
	
	m_sSourceName = pDownload->m_sLocalName;
	m_sRemoteName = pDownload->m_sRemoteName;
	
	int nPos = m_sSourceName.ReverseFind( '\\' );
	
	if ( nPos >= 0 )
	{
		for ( int nCount = 0 ; nCount < 20 ; nCount++ )
		{
			if ( nCount > 0 )
			{
				m_sTargetName.Format( _T("%sPreview (%i) of %s"),
					(LPCTSTR)m_sSourceName.Left( nPos + 1 ), nCount,
					(LPCTSTR)m_sSourceName.Mid( nPos + 1 ) );
			}
			else
			{
				m_sTargetName.Format( _T("%sPreview of %s"),
					(LPCTSTR)m_sSourceName.Left( nPos + 1 ),
					(LPCTSTR)m_sSourceName.Mid( nPos + 1 ) );
			}
			
			if ( GetFileAttributes( m_sTargetName ) == 0xFFFFFFFF ) break;
		}
	}

	// if user changes extension or extension is lost
	LPCTSTR pszExt1 = _tcsrchr( m_sSourceName, '.' );
	LPCTSTR pszExt2 = _tcsrchr( m_sRemoteName, '.' );
	if ( ! pszExt1 && pszExt2 || pszExt1 && pszExt2 && _tcsicmp( pszExt1, pszExt2 ) != 0 ) 
		m_sTargetName += pszExt2;

    if ( !m_pDownload->GetEmptyFragmentList().empty() )
    {
        FF::SimpleFragmentList oRanges = inverse( m_pDownload->GetEmptyFragmentList() );

        for ( FF::SimpleFragmentList::ConstIterator pFragment
            = oRanges.begin(); pFragment != oRanges.end(); ++pFragment )
	    {
		    m_pRanges.Add( DWORD( pFragment->begin() ) );
		    m_pRanges.Add( DWORD( pFragment->length() ) );
	    }
    		
	    if ( ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) == 0x8000 )
	    {
		    while ( m_pRanges.GetSize() > 2 ) m_pRanges.RemoveAt( 2 );
	    }
    }
}

BOOL CFilePreviewDlg::Create()
{
	ASSERT( m_hWnd == NULL );
	ASSERT( m_pDownload != NULL );
	
	LPCTSTR lpszTemplateName = MAKEINTRESOURCE( IDD );
	
	HINSTANCE hInst		= AfxFindResourceHandle( lpszTemplateName, RT_DIALOG );
	HRSRC hResource		= ::FindResource( hInst, lpszTemplateName, RT_DIALOG );
	HGLOBAL hTemplate	= LoadResource( hInst, hResource );
	
	LPCDLGTEMPLATE lpDialogTemplate = (LPCDLGTEMPLATE)LockResource( hTemplate );
	
	BOOL bResult = CreateDlgIndirect( lpDialogTemplate, NULL, hInst );
	
	UnlockResource( hTemplate );
	FreeResource( hTemplate );
	
	return bResult;
}

void CFilePreviewDlg::OnSkinChange(BOOL bSet)
{
	for ( POSITION pos = m_pWindows.GetHeadPosition() ; pos ; )
	{
		CFilePreviewDlg* pDlg = (CFilePreviewDlg*)m_pWindows.GetNext( pos );
		
		if ( bSet )
		{
			pDlg->SkinMe( NULL, ID_DOWNLOADS_LAUNCH_COPY );
			pDlg->Invalidate();
		}
		else
		{
			pDlg->m_pSkin = NULL;
		}
	}
}

void CFilePreviewDlg::CloseAll()
{
	for ( POSITION pos = m_pWindows.GetHeadPosition() ; pos ; )
	{
		delete (CFilePreviewDlg*)m_pWindows.GetNext( pos );
	}
	m_pWindows.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
// CFilePreviewDlg message handlers

BOOL CFilePreviewDlg::OnInitDialog() 
{
	CSkinDialog::OnInitDialog();
	
	SkinMe( NULL, ID_DOWNLOADS_LAUNCH_COPY );
	
	m_nRange	= 100;
	m_nPosition	= 0;
	m_nScaled	= m_nOldScaled = 0;
	
	if ( theApp.m_bRTL ) m_wndProgress.ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );
	m_wndStatus.GetWindowText( m_sStatus );	
	m_wndProgress.SetRange( 0, 1000 );
	m_wndProgress.SetPos( 0 );
	m_sOldStatus = m_sStatus;
	
	m_wndName.SetWindowText( m_sRemoteName );
	m_wndCancel.EnableWindow( FALSE );
	
	m_bThread = TRUE;
	m_bCancel = FALSE;
	
	CWinThread* pThread = AfxBeginThread( ThreadStart, this, THREAD_PRIORITY_NORMAL );
	m_hThread = pThread->m_hThread;
	
	return TRUE;
}

void CFilePreviewDlg::OnCancel() 
{
	if ( m_bThread )
	{
		m_pSection.Lock();
		m_bCancel = TRUE;
		if ( m_pPlugin != NULL ) m_pPlugin->Cancel();
		m_pSection.Unlock();
	}
	else
	{
		PostMessage( WM_CLOSE );
	}
}

void CFilePreviewDlg::OnTimer(UINT nIDEvent) 
{
	if ( nIDEvent == 3 )
	{
		PostMessage( WM_CLOSE );
		return;
	}
	
	CSingleLock pLock( &m_pSection, TRUE );
	
	if ( nIDEvent == 2 && m_sExecute.GetLength() > 0 )
	{
		CString strExecute = m_sExecute;
		m_sExecute.Empty();
		pLock.Unlock();
		CFileExecutor::Execute( strExecute, TRUE );
		return;
	}
	
	if ( m_nScaled != m_nOldScaled )
	{
		m_wndProgress.SetPos( m_nScaled );
		m_nOldScaled = m_nScaled;
	}
	
	if ( m_sStatus != m_sOldStatus )
	{
		m_wndStatus.SetWindowText( m_sStatus );
		m_sOldStatus = m_sStatus;
	}
	
	if ( ! m_wndCancel.IsWindowEnabled() ) m_wndCancel.EnableWindow( TRUE );
}

void CFilePreviewDlg::OnClose()
{
	DestroyWindow();
}

void CFilePreviewDlg::OnDestroy() 
{
	if ( m_hThread != NULL )
	{
        int nAttempt = 100;
		for ( ; nAttempt > 0 ; nAttempt-- )
		{
			DWORD nCode;
			if ( ! GetExitCodeThread( m_hThread, &nCode ) ) break;
			if ( nCode != STILL_ACTIVE ) break;
			Sleep( 50 );
		}
		
		if ( nAttempt == 0 )
		{
			TerminateThread( m_hThread, 0 );
			theApp.Message( MSG_DEBUG, _T("WARNING: Terminating CFilePreviewDlg thread.") );
			Sleep( 250 );
		}
	}
	
	m_bThread	= FALSE;
	m_hThread	= NULL;
	
	if ( m_pDownload != NULL )
	{
		if ( Transfers.m_pSection.Lock( 1000 ) )
		{
			if ( Downloads.Check( m_pDownload ) ) m_pDownload->m_pPreviewWnd = NULL;
			Transfers.m_pSection.Unlock();
		}
		m_pDownload = NULL;
	}
	
	CSkinDialog::OnDestroy();
}

void CFilePreviewDlg::PostNcDestroy() 
{
	CSkinDialog::PostNcDestroy();
	delete this;
}

/////////////////////////////////////////////////////////////////////////////
// CFilePreviewDlg thread run

UINT CFilePreviewDlg::ThreadStart(LPVOID pParam)
{
	CFilePreviewDlg* pClass = (CFilePreviewDlg*)pParam;
	pClass->OnRun();
	return 0;
}

void CFilePreviewDlg::OnRun()
{
	HANDLE hFile = CreateFile( m_sSourceName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	
	if ( hFile != INVALID_HANDLE_VALUE )
	{
		if ( ! RunPlugin( hFile ) && ! m_bCancel ) RunManual( hFile );
		CloseHandle( hFile );
	}
	
	m_bThread = FALSE;
	PostMessage( WM_TIMER, 3 );
}

/////////////////////////////////////////////////////////////////////////////
// CFilePreviewDlg plugin execution

BOOL CFilePreviewDlg::RunPlugin(HANDLE hFile)
{
	CString strType;
	
	int nExtPos = m_sTargetName.ReverseFind( '.' );
	if ( nExtPos > 0 ) strType = m_sTargetName.Mid( nExtPos );
	CharLower( strType.GetBuffer() );
	strType.ReleaseBuffer();
	
	if ( ! LoadPlugin( strType ) ) return FALSE;
	
	HRESULT hr = S_FALSE;
	
	if ( SUCCEEDED( m_pPlugin->SetSite( &m_xDownloadPreviewSite ) ) )
	{
		BSTR bsFile = m_sTargetName.AllocSysString();
		hr = m_pPlugin->Preview( hFile, bsFile );
		SysFreeString( bsFile );
	}
	
	m_pSection.Lock();
	m_pPlugin->Release();
	m_pPlugin = NULL;
	m_pSection.Unlock();
	
	CoUninitialize();
	
	if ( hr != S_OK ) Sleep( 1000 );
	
	return ( hr != S_FALSE );	// Fall through if it's S_FALSE
}

BOOL CFilePreviewDlg::LoadPlugin(LPCTSTR pszType)
{
	CLSID pCLSID;
	
	if ( ! Plugins.LookupCLSID( _T("DownloadPreview"), pszType, pCLSID ) ) return FALSE;
	
	if ( FAILED( CoInitializeEx( NULL, COINIT_MULTITHREADED ) ) ) return FALSE;
	
	HRESULT hResult = CoCreateInstance( pCLSID, NULL, CLSCTX_INPROC_SERVER,
		IID_IDownloadPreviewPlugin, (void**)&m_pPlugin );
	
	if ( SUCCEEDED( hResult ) )
	{
		return TRUE;
	}
	else
	{
		CoUninitialize();
		return FALSE;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CFilePreviewDlg manual execution

#define BUFFER_SIZE 40960

BOOL CFilePreviewDlg::RunManual(HANDLE hFile)
{
	HANDLE hTarget = CreateFile( m_sTargetName, GENERIC_WRITE, FILE_SHARE_READ,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	
	if ( hTarget == INVALID_HANDLE_VALUE ) return FALSE;
	
	m_nRange = m_nPosition = 0;
	
	for ( int nRange = 0 ; nRange < m_pRanges.GetSize() ; nRange += 2 )
	{
		m_nRange += m_pRanges.GetAt( nRange + 1 );
	}
	
	UpdateProgress( TRUE, m_nRange, TRUE, m_nPosition );
	
	BYTE* pData = new BYTE[ BUFFER_SIZE ];
	
	for ( int nRange = 0 ; nRange < m_pRanges.GetSize() ; nRange += 2 )
	{
		DWORD nOffset = m_pRanges.GetAt( nRange );
		DWORD nLength = m_pRanges.GetAt( nRange + 1 );
		
		SetFilePointer( hFile, nOffset, 0, FILE_BEGIN );
		// SetFilePointer( hTarget, nOffset, 0, FILE_BEGIN );
		
		while ( nLength )
		{
			DWORD nChunk = min( DWORD(BUFFER_SIZE), nLength );
			
			ReadFile( hFile, pData, nChunk, &nChunk, NULL );
			
			if ( nChunk == 0 )
			{
				theApp.Message( MSG_DEBUG, _T("Preview: read error.") );
				m_bCancel = TRUE;
			}
			
			WriteFile( hTarget, pData, nChunk, &nChunk, NULL );
			
			if ( nChunk == 0 )
			{
				theApp.Message( MSG_DEBUG, _T("Preview: write error.") );
				m_bCancel = TRUE;
			}
			
			nLength -= nChunk;
			
			if ( m_bCancel ) break;
			
			UpdateProgress( FALSE, 0, TRUE, m_nPosition + nChunk );
		}
	}
	
	delete [] pData;
	
	CloseHandle( hTarget );
	
	if ( m_bCancel )
	{
		DeleteFile( m_sTargetName );
		return FALSE;
	}
	
	QueueDeleteFile( m_sTargetName );
	ExecuteFile( m_sTargetName );
	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CFilePreviewDlg utilities

BOOL CFilePreviewDlg::QueueDeleteFile(LPCTSTR pszFile)
{
	CSingleLock pLock( &Transfers.m_pSection );

	if ( pLock.Lock( 500 ) )
	{
		if ( Downloads.Check( m_pDownload ) )
		{
			m_pDownload->AddPreviewName( pszFile );
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CFilePreviewDlg::ExecuteFile(LPCTSTR pszFile)
{
	m_pSection.Lock();
	m_sExecute = pszFile;
	PostMessage( WM_TIMER, 2 );
	m_pSection.Unlock();

	return TRUE;
}

void CFilePreviewDlg::UpdateProgress(BOOL bRange, QWORD nRange, BOOL bPosition, QWORD nPosition)
{
	m_pSection.Lock();
	
	if ( bRange ) m_nRange = nRange;
	if ( bPosition ) m_nPosition = nPosition;
	
	m_nScaled = (DWORD)( (double)m_nPosition / (double)m_nRange * 1000.0f );
	BOOL bRefresh = ( m_nScaled != m_nOldScaled );
	
	m_pSection.Unlock();
	if ( bRefresh ) PostMessage( WM_TIMER, 1 );
}

/////////////////////////////////////////////////////////////////////////////
// CFilePreviewDlg IDownloadPreviewSite

IMPLEMENT_UNKNOWN(CFilePreviewDlg, DownloadPreviewSite)

STDMETHODIMP CFilePreviewDlg::XDownloadPreviewSite::GetSuggestedFilename(BSTR FAR* psFile)
{
	METHOD_PROLOGUE( CFilePreviewDlg, DownloadPreviewSite )
	pThis->m_sTargetName.SetSysString( psFile );
	return S_OK;
}

STDMETHODIMP CFilePreviewDlg::XDownloadPreviewSite::GetAvailableRanges(SAFEARRAY FAR* FAR* pArray)
{
	METHOD_PROLOGUE( CFilePreviewDlg, DownloadPreviewSite )
	
	SAFEARRAYBOUND pBound[2] = { { pThis->m_pRanges.GetSize() / 2, 0 }, { 2, 0 } };
	*pArray = SafeArrayCreate( VT_I4, 2, pBound );
	
	DWORD* pTarget;
	SafeArrayAccessData( *pArray, (void**)&pTarget );
	
	for ( int nRange = 0 ; nRange < pThis->m_pRanges.GetSize() ; nRange++, pTarget++ )
	{
		*pTarget = pThis->m_pRanges.GetAt( nRange );
	}
	
	SafeArrayUnaccessData( *pArray );
	
	return S_OK;
}

STDMETHODIMP CFilePreviewDlg::XDownloadPreviewSite::SetProgressRange(DWORD nRange)
{
	METHOD_PROLOGUE( CFilePreviewDlg, DownloadPreviewSite )
	pThis->UpdateProgress( TRUE, nRange, FALSE, 0 );
	return S_OK;
}

STDMETHODIMP CFilePreviewDlg::XDownloadPreviewSite::SetProgressPosition(DWORD nPosition)
{
	METHOD_PROLOGUE( CFilePreviewDlg, DownloadPreviewSite )
	pThis->UpdateProgress( FALSE, 0, TRUE, nPosition );
	return S_OK;
}

STDMETHODIMP CFilePreviewDlg::XDownloadPreviewSite::SetProgressMessage(BSTR sMessage)
{
	METHOD_PROLOGUE( CFilePreviewDlg, DownloadPreviewSite )
	pThis->m_pSection.Lock();
	pThis->m_sStatus = sMessage;
	pThis->m_pSection.Unlock();
	pThis->PostMessage( WM_TIMER );
	return S_OK;
}

STDMETHODIMP CFilePreviewDlg::XDownloadPreviewSite::QueueDeleteFile(BSTR sTempFile)
{
	METHOD_PROLOGUE( CFilePreviewDlg, DownloadPreviewSite )
	return pThis->QueueDeleteFile( CString( sTempFile ) ) ? S_OK : E_FAIL;
}

STDMETHODIMP CFilePreviewDlg::XDownloadPreviewSite::ExecuteFile(BSTR sFile)
{
	METHOD_PROLOGUE( CFilePreviewDlg, DownloadPreviewSite )
	return pThis->ExecuteFile( CString( sFile ) ) ? S_OK : E_FAIL;
}
