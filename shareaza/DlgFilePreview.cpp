//
// DlgFilePreview.cpp
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "Transfers.h"
#include "Downloads.h"
#include "DownloadWithExtras.h"
#include "FragmentedFile.h"
#include "TransferFile.h"
#include "DlgFilePreview.h"
#include "FileExecutor.h"
#include "Plugins.h"

#include "DownloadTask.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CFilePreviewDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CFilePreviewDlg, CSkinDialog)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

BEGIN_INTERFACE_MAP(CFilePreviewDlg, CSkinDialog)
	INTERFACE_PART(CFilePreviewDlg, IID_IDownloadPreviewSite, DownloadPreviewSite)
END_INTERFACE_MAP()

const DWORD BUFFER_SIZE = 40960u;
CList< CFilePreviewDlg* > CFilePreviewDlg::m_pWindows;

/////////////////////////////////////////////////////////////////////////////
// CFilePreviewDlg dialog

CFilePreviewDlg::CFilePreviewDlg(CDownloadWithExtras* pDownload, DWORD nIndex, CWnd* pParent) :
	CSkinDialog( CFilePreviewDlg::IDD, pParent )
	, m_pDownload	( pDownload )
	, m_sSourceName	( pDownload->GetPath( nIndex ) )
	, m_sDisplayName( pDownload->GetName( nIndex ) )
	, m_pPlugin		( NULL )
	, m_nRange		( 0 )
	, m_nPosition	( 0 )
	, m_nScaled		( 0 )
	, m_nOldScaled	( 0 )
	, m_bCancel		( FALSE )
{
	int nPos = m_sSourceName.ReverseFind( '\\' );
	if ( nPos >= 0 )
	{
		CString strFileName = m_sDisplayName;
		strFileName.Replace( _T('\\'), _T('_') );

		for ( int nCount = 0 ; nCount < 20 ; nCount++ )
		{
			if ( nCount > 0 )
			{
				m_sTargetName.Format( _T("%sPreview (%i) of %s"),
					(LPCTSTR)m_sSourceName.Left( nPos + 1 ), nCount,
					(LPCTSTR)strFileName );
			}
			else
			{
				m_sTargetName.Format( _T("%sPreview of %s"),
					(LPCTSTR)m_sSourceName.Left( nPos + 1 ),
					(LPCTSTR)strFileName );
			}

			if ( GetFileAttributes( m_sTargetName ) == 0xFFFFFFFF )
				break;

			m_sTargetName.Empty();
		}
	}

	QWORD nOffset = m_pDownload->GetOffset( nIndex );
	QWORD nLength = m_pDownload->GetLength( nIndex );
	if ( ! m_pDownload->GetEmptyFragmentList().empty() )
	{
		Fragments::List oRanges = inverse( m_pDownload->GetEmptyFragmentList() );

		for ( Fragments::List::const_iterator pFragment = oRanges.begin();
			pFragment != oRanges.end(); ++pFragment )
		{
			if ( pFragment->begin() + pFragment->size() >= nOffset &&
				 nOffset + nLength >= pFragment->begin() )
			{				
				QWORD nPartOffset =
					max( pFragment->begin(), nOffset );
				QWORD nPartLength =
					min( pFragment->begin() + pFragment->size(), nOffset + nLength ) -
					nPartOffset;
				m_pRanges.Add( nPartOffset - nOffset );
				m_pRanges.Add( nPartLength );
			}
		}

		if ( ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) == 0x8000 )
		{
			while ( m_pRanges.GetSize() > 2 ) m_pRanges.RemoveAt( 2 );
		}
	}
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

BOOL CFilePreviewDlg::Create()
{
	ASSERT( m_hWnd == NULL );
	ASSERT( m_pDownload != NULL );

	LPCTSTR lpszTemplateName = MAKEINTRESOURCE( IDD );
	BOOL bResult = FALSE;
	HINSTANCE hInst		= AfxFindResourceHandle( lpszTemplateName, RT_DIALOG );
	HRSRC hResource		= ::FindResource( hInst, lpszTemplateName, RT_DIALOG );
	if ( hResource )
	{
		HGLOBAL hTemplate = LoadResource( hInst, hResource );
		if ( hTemplate )
		{
			LPCDLGTEMPLATE lpDialogTemplate = (LPCDLGTEMPLATE)LockResource( hTemplate );
			if ( lpDialogTemplate )
			{
				bResult = CreateDlgIndirect( lpDialogTemplate, NULL, hInst );
			}
			FreeResource( hTemplate );
		}
	}
	return bResult;
}

void CFilePreviewDlg::OnSkinChange(BOOL bSet)
{
	for ( POSITION pos = m_pWindows.GetHeadPosition() ; pos ; )
	{
		CFilePreviewDlg* pDlg = m_pWindows.GetNext( pos );

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
		delete m_pWindows.GetNext( pos );
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

	if ( Settings.General.LanguageRTL ) m_wndProgress.ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );
	m_wndStatus.GetWindowText( m_sStatus );
	m_wndProgress.SetRange( 0, 1000 );
	m_wndProgress.SetPos( 0 );
	m_sOldStatus = m_sStatus;

	m_wndName.SetWindowText( m_sDisplayName );
	m_wndCancel.EnableWindow( FALSE );

	m_bCancel = FALSE;

	BeginThread( "DlgFilePreview" );

	return TRUE;
}

void CFilePreviewDlg::OnTimer(UINT_PTR nIDEvent)
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
	if ( IsThreadAlive() )
	{
		m_pSection.Lock();
		m_bCancel = TRUE;
		if ( m_pPlugin != NULL ) m_pPlugin->Cancel();
		m_pSection.Unlock();
	}
	else
	{
		DestroyWindow();
	}
}

void CFilePreviewDlg::OnDestroy()
{
	CloseThread();

	if ( m_pDownload != NULL )
	{
		CSingleLock oLock( &Transfers.m_pSection, FALSE );
		if ( oLock.Lock( 1000 ) )
		{
			if ( Downloads.Check( (CDownload*)m_pDownload ) )
				m_pDownload->m_pPreviewWnd = NULL;
		}
		m_pDownload = NULL;
	}

	CSkinDialog::OnDestroy();
}

void CFilePreviewDlg::PostNcDestroy()
{
	delete this;
}

/////////////////////////////////////////////////////////////////////////////
// CFilePreviewDlg thread run

void CFilePreviewDlg::OnRun()
{
	HANDLE hFile = CreateFile( m_sSourceName, GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	VERIFY_FILE_ACCESS( hFile, m_sSourceName )
	if ( hFile != INVALID_HANDLE_VALUE )
	{
		if ( ! RunPlugin( hFile ) && ! m_bCancel ) RunManual( hFile );
		CloseHandle( hFile );
	}

	Sleep( 2000 );

	PostMessage( WM_TIMER, 3 );
}

/////////////////////////////////////////////////////////////////////////////
// CFilePreviewDlg plugin execution

BOOL CFilePreviewDlg::RunPlugin(HANDLE hFile)
{
	CString strType;

	int nExtPos = m_sTargetName.ReverseFind( '.' );
	if ( nExtPos != -1 ) strType = m_sTargetName.Mid( nExtPos );
	ToLower( strType );

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

	return ( hr != S_FALSE );	// Fall through if it's S_FALSE
}

BOOL CFilePreviewDlg::LoadPlugin(LPCTSTR pszType)
{
	CLSID pCLSID;

	if ( ! Plugins.LookupCLSID( _T("DownloadPreview"), pszType, pCLSID ) ) return FALSE;

	HRESULT hResult = CoCreateInstance( pCLSID, NULL, CLSCTX_ALL,
		IID_IDownloadPreviewPlugin, (void**)&m_pPlugin );

	return SUCCEEDED( hResult );
}

/////////////////////////////////////////////////////////////////////////////
// CFilePreviewDlg manual execution

BOOL CFilePreviewDlg::RunManual(HANDLE hFile)
{
	HANDLE hTarget = CreateFile( m_sTargetName, GENERIC_WRITE, 0,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	VERIFY_FILE_ACCESS( hTarget, m_sTargetName )
	if ( hTarget == INVALID_HANDLE_VALUE ) return FALSE;

	m_nRange = m_nPosition = 0;

	for ( QWORD nRange = 0 ; nRange < (QWORD)m_pRanges.GetSize() ; nRange += 2 )
	{
		m_nRange += m_pRanges.GetAt( nRange + 1 );
	}

	UpdateProgress( TRUE, m_nRange, TRUE, m_nPosition );

	BYTE* pData = new BYTE[ BUFFER_SIZE ];

	for ( QWORD nRange = 0 ; nRange < (QWORD)m_pRanges.GetSize() ; nRange += 2 )
	{
		QWORD nOffset = m_pRanges.GetAt( nRange );
		QWORD nLength = m_pRanges.GetAt( nRange + 1 );

		DWORD nOffsetLow	= (DWORD)( nOffset & 0x00000000FFFFFFFF );
		DWORD nOffsetHigh	= (DWORD)( ( nOffset & 0xFFFFFFFF00000000 ) >> 32 );
		SetFilePointer( hFile, nOffsetLow, (PLONG)&nOffsetHigh, FILE_BEGIN );
		// SetFilePointer( hTarget, nOffsetLow, (PLONG)&nOffsetHigh, FILE_BEGIN );

		while ( nLength )
		{
			DWORD nChunk = (DWORD)min( BUFFER_SIZE, nLength );

			if ( ! ReadFile( hFile, pData, nChunk, &nChunk, NULL ) || nChunk == 0 )
			{
				theApp.Message( MSG_DEBUG, _T("Preview: read error %d."), GetLastError() );
				m_bCancel = TRUE;
			}

			if ( ! WriteFile( hTarget, pData, nChunk, &nChunk, NULL ) || nChunk == 0 )
			{
				theApp.Message( MSG_DEBUG, _T("Preview: write error %d."), GetLastError() );
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
		DeleteFileEx( m_sTargetName, FALSE, FALSE, TRUE );
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
		if ( Downloads.Check( (CDownload*)m_pDownload ) )
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

	SAFEARRAYBOUND pBound[2] = { { static_cast< ULONG >( pThis->m_pRanges.GetSize() / 2 ), 0 }, { 2, 0 } };
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
