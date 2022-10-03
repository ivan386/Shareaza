//
// DlgFilePreview.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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

CPreviewList		CFilePreviewDlg::m_pWindows;
CCriticalSection	CFilePreviewDlg::m_pSection;

/////////////////////////////////////////////////////////////////////////////
// CFilePreviewDlg dialog

CFilePreviewDlg::CFilePreviewDlg(CDownloadWithExtras* pDownload, DWORD nIndex, CWnd* pParent) :
	CSkinDialog( CFilePreviewDlg::IDD, pParent )
	, m_pDownload	( pDownload )
	, m_sSourceName	( pDownload->GetPath( nIndex ) )
	, m_sDisplayName( pDownload->GetName( nIndex ) )
	, m_nRange		( 0 )
	, m_nPosition	( 0 )
	, m_nScaled		( 0 )
	, m_nOldScaled	( 0 )
{
	CQuickLock oLock( m_pSection );

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

	if ( ! m_pDownload->GetEmptyFragmentList().empty() )
	{
		QWORD nOffset = m_pDownload->GetOffset( nIndex );
		QWORD nLength = m_pDownload->GetLength( nIndex );
		Fragments::List oRanges = inverse( m_pDownload->GetEmptyFragmentList() );
		Fragments::List::const_iterator pItr = oRanges.begin();
		const Fragments::List::const_iterator pEnd = oRanges.end();
		for ( ; pItr != pEnd ; ++pItr )
		{
			if ( pItr->begin() + pItr->size() >= nOffset
				&& nOffset + nLength >= pItr->begin() )
			{
				QWORD nPartOffset =
					max( pItr->begin(), nOffset );
				QWORD nPartLength =
					min( pItr->begin() + pItr->size(), nOffset + nLength )
					- nPartOffset;
				m_pRanges.Add( nPartOffset - nOffset );
				m_pRanges.Add( nPartLength );
			}
		}

		if ( ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) == 0x8000 )
		{
			while ( m_pRanges.GetSize() > 2 )
				m_pRanges.RemoveAt( 2 );
		}
	}

	if ( Create( CFilePreviewDlg::IDD, pParent ) )
	{
		CenterWindow();
		ShowWindow( SW_SHOWNORMAL );
		BringWindowToTop();
	}
}

CFilePreviewDlg::~CFilePreviewDlg()
{
	CQuickLock oLock( m_pSection );

	if ( POSITION pos = m_pWindows.Find( this ) )
		m_pWindows.RemoveAt( pos );

	ASSERT( m_pDownload == NULL );
}

void CFilePreviewDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange( pDX );

	DDX_Control(pDX, IDCANCEL, m_wndCancel);
	DDX_Control(pDX, IDC_PROGRESS, m_wndProgress);
	DDX_Control(pDX, IDC_PREVIEW_STATUS, m_wndStatus);
	DDX_Control(pDX, IDC_FILE_NAME, m_wndName);
}

void CFilePreviewDlg::OnSkinChange(BOOL bSet)
{
	CQuickLock oLock( m_pSection );

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
			pDlg->RemoveSkin();
		}
	}
}

void CFilePreviewDlg::CloseAll()
{
	CQuickLock oLock( m_pSection );

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

	if ( Settings.General.LanguageRTL )
		m_wndProgress.ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );

	m_wndStatus.GetWindowText( m_sStatus );
	m_wndProgress.SetRange( 0, 100 );
	m_wndProgress.SetPos( 0 );

	m_sOldStatus = m_sStatus;

	m_wndName.SetWindowText( m_sDisplayName );

	m_wndCancel.EnableWindow( FALSE );

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

		CFileExecutor::Execute( strExecute );

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

	pLock.Unlock();

	if ( IsThreadAlive() && IsThreadEnabled() && ! m_wndCancel.IsWindowEnabled() )
		m_wndCancel.EnableWindow( TRUE );

	UpdateWindow();
}

void CFilePreviewDlg::OnOK()
{
	OnClose();
}

void CFilePreviewDlg::OnCancel()
{
	OnClose();
}

void CFilePreviewDlg::OnClose()
{
	m_wndCancel.EnableWindow( FALSE );

	if ( IsThreadAlive() )
	{
		CQuickLock oLock( m_pSection );

		Exit();

		if ( m_pPlugin )
			m_pPlugin->Cancel();
	}

	DestroyWindow();
}

void CFilePreviewDlg::OnDestroy()
{
	CloseThread();

	if ( m_pDownload != NULL )
	{
		CQuickLock oTransfersLock( Transfers.m_pSection );

		if ( Downloads.Check( (CDownload*)m_pDownload ) )
			m_pDownload->m_pPreviewWnd = NULL;

		m_pDownload = NULL;
	}

	if ( ! IsWindow( GetSafeHwnd() ) )
		return;

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
	if ( ! RunPlugin() && IsThreadEnabled() )
		RunManual();

	Sleep( 2000 );

	PostMessage( WM_TIMER, 3 );
}

/////////////////////////////////////////////////////////////////////////////
// CFilePreviewDlg plugin execution

BOOL CFilePreviewDlg::RunPlugin()
{
	HRESULT hr;

	CSingleLock oLock( &m_pSection, TRUE );

	LPCTSTR szType = PathFindExtension( m_sTargetName ); // ".ext"

	m_pPlugin = Plugins.GetPlugin( _T("DownloadPreview"), szType );
	if ( ! m_pPlugin )
		return FALSE;

	hr = m_pPlugin->SetSite( &m_xDownloadPreviewSite );
	if ( FAILED( hr ) )
		return FALSE;

	oLock.Unlock();

	hr = m_pPlugin->Preview2( CComBSTR( m_sSourceName ), CComBSTR( m_sTargetName ) );

	oLock.Lock();

	m_pPlugin->SetSite( NULL );

	m_pPlugin.Release();

	return SUCCEEDED( hr );
}

/////////////////////////////////////////////////////////////////////////////
// CFilePreviewDlg manual execution

BOOL CFilePreviewDlg::RunManual()
{
	CAtlFile oSourceFile;
	HRESULT hr = oSourceFile.Create( m_sSourceName, GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, OPEN_EXISTING );
	if ( FAILED( hr ) )
		 return FALSE;

	CAtlFile oTargetFile;
	hr = oTargetFile.Create( m_sTargetName, GENERIC_WRITE, 0, CREATE_ALWAYS );
	if ( FAILED( hr ) )
		return FALSE;

	QWORD nProgressRange = 0;
	for ( INT_PTR i = 0 ; i < m_pRanges.GetSize() ; i += 2 )
	{
		nProgressRange += m_pRanges.GetAt( i + 1 );
	}
	UpdateProgress( TRUE, nProgressRange, TRUE, 0 );

	const DWORD BUFFER_SIZE = 4 * 1024 * 1024u; // 4 MB
	auto_array< BYTE > pData( new BYTE[ BUFFER_SIZE ] );
	if ( ! pData.get() )
		return FALSE;

	QWORD nProgressPosition = 0;
	for ( INT_PTR i = 0 ; i < m_pRanges.GetSize() && IsThreadEnabled(); i += 2 )
	{
		QWORD nOffset = m_pRanges.GetAt( i );
		QWORD nLength = m_pRanges.GetAt( i + 1 );

		oSourceFile.Seek( nOffset, FILE_BEGIN );
		// oTargetFile.Seek( nOffset, FILE_BEGIN );

		while ( nLength && IsThreadEnabled() )
		{
			DWORD nChunk = (DWORD)min( (QWORD)BUFFER_SIZE, nLength );

			hr = oSourceFile.Read( pData.get(), nChunk, nChunk );
			if ( FAILED( hr ) || nChunk == 0 )
			{
				theApp.Message( MSG_DEBUG, _T("Preview: read error %d."), GetLastError() );
				Exit();
			}

			if ( nChunk )
			{
				hr = oTargetFile.Write( pData.get(), nChunk, &nChunk );
				if ( FAILED( hr ) || nChunk == 0 )
				{
					theApp.Message( MSG_DEBUG, _T("Preview: write error %d."), GetLastError() );
					Exit();
				}
			}

			nLength -= nChunk;

			nProgressPosition += nChunk;
			UpdateProgress( FALSE, 0, TRUE, nProgressPosition );
		}
	}

	oTargetFile.Close();
	oSourceFile.Close();

	if ( ! IsThreadEnabled() )
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
	CSingleLock oTransfersLock( &Transfers.m_pSection );

	if ( oTransfersLock.Lock( 500 ) )
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
	CQuickLock oLock( m_pSection );

	m_sExecute = pszFile;

	PostMessage( WM_TIMER, 2 );

	return TRUE;
}

void CFilePreviewDlg::UpdateProgress(BOOL bRange, QWORD nRange, BOOL bPosition, QWORD nPosition)
{
	CQuickLock oLock( m_pSection );

	if ( bRange )
		m_nRange = nRange;

	if ( bPosition )
		m_nPosition = nPosition;

	m_nScaled = (DWORD)min( (double)m_nPosition / (double)m_nRange * 100., 100. );

	if ( m_nScaled != m_nOldScaled )
		PostMessage( WM_TIMER, 1 );
}

/////////////////////////////////////////////////////////////////////////////
// CFilePreviewDlg IDownloadPreviewSite

IMPLEMENT_UNKNOWN(CFilePreviewDlg, DownloadPreviewSite)

STDMETHODIMP CFilePreviewDlg::XDownloadPreviewSite::GetSuggestedFilename(BSTR FAR* psFile)
{
	METHOD_PROLOGUE( CFilePreviewDlg, DownloadPreviewSite )

	*psFile = CComBSTR( pThis->m_sTargetName ).Detach();

	return S_OK;
}

STDMETHODIMP CFilePreviewDlg::XDownloadPreviewSite::GetAvailableRanges(SAFEARRAY FAR* FAR* pArray)
{
	METHOD_PROLOGUE( CFilePreviewDlg, DownloadPreviewSite )

	SAFEARRAYBOUND pBound[2] = { { static_cast< ULONG >( pThis->m_pRanges.GetSize() / 2 ), 0 }, { 2, 0 } };
	*pArray = SafeArrayCreate( VT_UI8, 2, pBound );

	QWORD* pTarget;
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

	CQuickLock oLock( pThis->m_pSection );

	pThis->m_sStatus = sMessage;
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
