//
// DlgDownloadMonitor.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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
#include "Download.h"
#include "FragmentedFile.h"
#include "FragmentBar.h"
#include "GraphLine.h"
#include "GraphItem.h"
#include "Library.h"
#include "FileExecutor.h"
#include "CoolInterface.h"
#include "CoolMenu.h"
#include "ShellIcons.h"
#include "Plugins.h"
#include "Skin.h"
#include "DlgDownloadMonitor.h"
#include "WndMain.h"
#include "WndDownloads.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CDownloadMonitorDlg, CSkinDialog)
	//{{AFX_MSG_MAP(CDownloadMonitorDlg)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_DOWNLOAD_CANCEL, OnDownloadCancel)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_DOWNLOAD_LAUNCH, OnDownloadLaunch)
	ON_BN_CLICKED(IDC_DOWNLOAD_LIBRARY, OnDownloadLibrary)
	ON_BN_CLICKED(IDC_DOWNLOAD_STOP, OnDownloadStop)
	ON_WM_CLOSE()
	ON_WM_SYSCOMMAND()
	ON_WM_CTLCOLOR()
	ON_WM_CONTEXTMENU()
	ON_WM_INITMENUPOPUP()
	ON_MESSAGE(WM_TRAY, OnTray)
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnNeedText)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CList< CDownloadMonitorDlg* > CDownloadMonitorDlg::m_pWindows;


/////////////////////////////////////////////////////////////////////////////
// CDownloadMonitorDlg dialog

CDownloadMonitorDlg::CDownloadMonitorDlg(CDownload* pDownload) : CSkinDialog( CDownloadMonitorDlg::IDD, NULL )
{
	//{{AFX_DATA_INIT(CDownloadMonitorDlg)
	//}}AFX_DATA_INIT

	m_pDownload		= pDownload;
	m_pGraph		= NULL;
	m_bTray			= FALSE;
	m_bCompleted	= FALSE;

	CreateReal( IDD );

	m_pWindows.AddTail( this );
}

CDownloadMonitorDlg::~CDownloadMonitorDlg()
{
	if ( m_pGraph != NULL ) delete m_pGraph;
	if ( POSITION pos = m_pWindows.Find( this ) ) m_pWindows.RemoveAt( pos );
}

void CDownloadMonitorDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CDownloadMonitorDlg)
	DDX_Control(pDX, IDC_DOWNLOAD_VOLUME, m_wndVolume);
	DDX_Control(pDX, IDC_DOWNLOAD_CANCEL, m_wndCancel);
	DDX_Control(pDX, IDC_DOWNLOAD_CLOSE, m_wndClose);
	DDX_Control(pDX, IDC_DOWNLOAD_STOP, m_wndStop);
	DDX_Control(pDX, IDC_PROGRESS, m_wndProgress);
	DDX_Control(pDX, IDC_DOWNLOAD_TIME, m_wndTime);
	DDX_Control(pDX, IDC_DOWNLOAD_STATUS, m_wndStatus);
	DDX_Control(pDX, IDC_DOWNLOAD_SPEED, m_wndSpeed);
	DDX_Control(pDX, IDC_DOWNLOAD_SOURCES, m_wndSources);
	DDX_Control(pDX, IDC_DOWNLOAD_LIBRARY, m_wndLibrary);
	DDX_Control(pDX, IDC_DOWNLOAD_LAUNCH, m_wndLaunch);
	DDX_Control(pDX, IDC_DOWNLOAD_ICON, m_wndIcon);
	DDX_Control(pDX, IDC_DOWNLOAD_GRAPH, m_wndGraph);
	DDX_Control(pDX, IDC_DOWNLOAD_FILE, m_wndFile);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadMonitorDlg operations

BOOL CDownloadMonitorDlg::CreateReal(UINT nID)
{
	LPCTSTR lpszTemplateName = MAKEINTRESOURCE( nID );
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

void CDownloadMonitorDlg::OnSkinChange(BOOL bSet)
{
	for ( POSITION pos = m_pWindows.GetHeadPosition() ; pos ; )
	{
		CDownloadMonitorDlg* pDlg = m_pWindows.GetNext( pos );

		if ( bSet )
		{
			pDlg->SkinMe( _T("CDownloadMonitorDlg"), IDI_DOWNLOAD_MONITOR );
			pDlg->Invalidate();
		}
		else
		{
			pDlg->m_pSkin = NULL;
		}
	}
}

void CDownloadMonitorDlg::CloseAll()
{
	for ( POSITION pos = m_pWindows.GetHeadPosition() ; pos ; )
	{
		delete m_pWindows.GetNext( pos );
	}
	m_pWindows.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadMonitorDlg message handlers

BOOL CDownloadMonitorDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CDownloadMonitorDlg"), IDI_DOWNLOAD_MONITOR );

	CMenu* pMenu = GetSystemMenu( FALSE );
	pMenu->InsertMenu( 0, MF_BYPOSITION|MF_SEPARATOR, ID_SEPARATOR );
	pMenu->InsertMenu( 0, MF_BYPOSITION|MF_STRING, SC_NEXTWINDOW, _T("&Always on Top") );

	CSingleLock pLock( &Transfers.m_pSection, TRUE );

	if ( Downloads.Check( m_pDownload ) )
	{
		m_sName = m_pDownload->m_sName;
		m_wndIcon.SetIcon( ShellIcons.ExtractIcon(
			ShellIcons.Get( m_sName, 32 ), 32 ) );
		m_wndFile.SetWindowText( m_sName );
	}

	pLock.Unlock();

	m_pGraph	= new CLineGraph();
	m_pItem		= new CGraphItem( 0, 1.0f, RGB( 0xFF, 0, 0 ) );

	m_pGraph->m_bShowLegend		= FALSE;
	m_pGraph->m_bShowAxis		= FALSE;
	m_pGraph->m_crBack			= RGB( 255, 255, 240 );
	m_pGraph->m_crGrid			= RGB( 220, 220, 170 );
	m_pGraph->m_nMinGridVert	= 16;

	m_pGraph->AddItem( m_pItem );

	OnTimer( 1 );

	CenterWindow();
	ShowWindow( SW_SHOW );

	SetTimer( 1, 100, NULL );
	EnableToolTips();

	return TRUE;
}

void CDownloadMonitorDlg::OnDestroy()
{
	KillTimer( 1 );

	if ( m_pDownload != NULL )
	{
		CDownload* pDownload = m_pDownload;
		m_pDownload = NULL;

		CSingleLock pLock( &Transfers.m_pSection );
		if ( pLock.Lock( 250 ) )
		{
			if ( Downloads.Check( pDownload ) )
				pDownload->m_pMonitorWnd = NULL;
		}
	}

	if ( m_bTray )
	{
		Shell_NotifyIcon( NIM_DELETE, &m_pTray );
		m_bTray = FALSE;
	}

	CSkinDialog::OnDestroy();
}

void CDownloadMonitorDlg::PostNcDestroy()
{
	CSkinDialog::PostNcDestroy();
	delete this;
}

void CDownloadMonitorDlg::OnTimer(UINT_PTR /*nIDEvent*/)
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return;

	if ( ! m_pDownload || ! Downloads.Check( m_pDownload ) )
	{
		KillTimer( 1 );
		PostMessage( WM_CLOSE );
		return;
	}

	if ( m_bCompleted ) return;

	bool bCompleted	= m_pDownload->IsCompleted();
	DWORD nSpeed	= m_pDownload->GetMeasuredSpeed();
	CString strText, strFormat, strOf;

	LoadString( strOf, IDS_GENERAL_OF );

	m_pItem->Add( nSpeed );
	m_pGraph->m_nUpdates++;
	m_pGraph->m_nMaximum = max( m_pGraph->m_nMaximum, nSpeed );

	// Update file name if it was changed from the Advanced Edit dialog
	Update( &m_wndFile, m_pDownload->m_sName );

	if ( m_pDownload->IsStarted() )
	{
		if ( Settings.General.LanguageRTL )
		{
			strText.Format( _T("%s %s %.2f%% : Shareaza"),
				(LPCTSTR)m_pDownload->m_sName, (LPCTSTR)strOf, m_pDownload->GetProgress() );
		}
		else
		{
			strText.Format( _T("%.2f%% %s %s : Shareaza"),
				m_pDownload->GetProgress(), (LPCTSTR)strOf, (LPCTSTR)m_pDownload->m_sName );
		}
	}
	else
	{
		strText.Format( _T("%s : Shareaza"),
			(LPCTSTR)m_pDownload->m_sName );
	}

	Update( this, strText );

	if ( m_bTray )
	{
		if ( _tcsncmp( m_pTray.szTip, strText, _countof( m_pTray.szTip ) - 1 ) )
		{
			m_pTray.uFlags = NIF_TIP;
			_tcsncpy( m_pTray.szTip, strText, _countof( m_pTray.szTip ) );

			Shell_NotifyIcon( NIM_MODIFY, &m_pTray );
		}
	}

	if ( bCompleted )
	{
		if ( m_wndClose.GetCheck() )
		{
			PostMessage( WM_CLOSE );
		}
		else
		{
			ShowWindow( SW_SHOWNORMAL );
			SetForegroundWindow();
		}

		m_bCompleted = TRUE;
	}
	else
	{
		if ( IsIconic() || m_bTray ) return;
	}

	int nSourceCount	= m_pDownload->GetSourceCount();
	int nTransferCount	= m_pDownload->GetTransferCount();

	CString strNA;
	LoadString( strNA, IDS_TIP_NA );

	if ( bCompleted )
	{
		LoadString( strText, IDS_DLM_COMPLETED );
		Update( &m_wndStatus, strText );
		Update( &m_wndTime, strNA );
		Update( &m_wndSpeed, strNA );
		LoadString( strText, IDS_DLM_COMPLETED_WORD );
		Update( &m_wndSources, strText );
	}
	else if ( m_pDownload->IsMoving() )
	{
		LoadString( strText, IDS_DLM_MOVING );
		Update( &m_wndStatus, strText );
		Update( &m_wndTime, strNA );
		Update( &m_wndSpeed, strNA );
		LoadString( strText, IDS_DLM_COMPLETED_WORD );
		Update( &m_wndSources, strText );
	}
	else if ( m_pDownload->IsPaused() )
	{
		LoadString( strText, IDS_DLM_PAUSED );
		Update( &m_wndStatus, strText );
		Update( &m_wndTime, strNA );
		Update( &m_wndSpeed, strNA );
		strText.Format( _T("%i"), nSourceCount );
		Update( &m_wndSources, strText );
	}
	else if ( m_pDownload->IsStarted() && m_pDownload->GetProgress() == 100.0f )
	{
		LoadString( strText, IDS_DLM_VERIFY );
		Update( &m_wndStatus, strText );
		Update( &m_wndTime, strNA );
		Update( &m_wndSpeed, strNA );
	}
	else if ( nTransferCount > 0 )
	{
		LoadString( strText, IDS_DLM_DOWNLOADING );
		Update( &m_wndStatus, strText );

		DWORD nTime = m_pDownload->GetTimeRemaining();
		strText.Empty();

		if ( nTime != 0xFFFFFFFF )
		{
			if ( nTime > 86400 )
			{
				LoadString( strFormat, IDS_DLM_TIME_DAH );
				strText.Format( strFormat, nTime / 86400, ( nTime / 3600 ) % 24 );
			}
			else if ( nTime > 3600 )
			{
				LoadString( strFormat, IDS_DLM_TIME_HAM );
				strText.Format( strFormat, nTime / 3600, ( nTime % 3600 ) / 60 );
			}
			else if ( nTime > 60 )
			{
				LoadString( strFormat, IDS_DLM_TIME_MAS );
				strText.Format( strFormat, nTime / 60, nTime % 60 );
			}
			else
			{
				LoadString( strFormat, IDS_DLM_TIME_S );
				strText.Format( strFormat, nTime % 60 );
			}
		}

		Update( &m_wndTime, strText );

		strText = Settings.SmartSpeed( m_pDownload->GetAverageSpeed() );
		Update( &m_wndSpeed, strText );

		strText.Format( _T("%i %s %i"), nTransferCount, (LPCTSTR)strOf, nSourceCount );
		if ( Settings.General.LanguageRTL ) strText = _T("\x202B") + strText;
		Update( &m_wndSources, strText );
	}
	else if ( nSourceCount )
	{
		LoadString( strText, IDS_DLM_DOWNLOADING );
		Update( &m_wndStatus, strText );
		Update( &m_wndTime, strNA );
		Update( &m_wndSpeed, strNA );
		strText.Format( _T("%i"), nSourceCount );
		Update( &m_wndSources, strText );
	}
	else
	{
		LoadString( strText, IDS_DLM_SOURCING );
		Update( &m_wndStatus, strText );
		Update( &m_wndTime, strNA );
		Update( &m_wndSpeed, strNA );
		LoadString( strText, IDS_DLM_NO_SOURCES );
		Update( &m_wndSources, strText );
	}

	if ( m_pDownload->IsStarted() )
	{
		if ( Settings.General.LanguageRTL )
		{
			strText.Format( _T("(%.2f%%) %s %s %s"),
				m_pDownload->GetProgress(),
				(LPCTSTR)Settings.SmartVolume( m_pDownload->m_nSize ),
				(LPCTSTR)strOf,
				(LPCTSTR)Settings.SmartVolume( m_pDownload->GetVolumeComplete() ) );
		}
		else
		{
			strText.Format( _T("%s %s %s (%.2f%%)"),
				(LPCTSTR)Settings.SmartVolume( m_pDownload->GetVolumeComplete() ),
				(LPCTSTR)strOf,
				(LPCTSTR)Settings.SmartVolume( m_pDownload->m_nSize ),
				m_pDownload->GetProgress() );
		}
		Update( &m_wndVolume, strText );
	}
	else
	{
		LoadString( strText, IDS_TIP_NA );
		Update( &m_wndVolume, strText );
	}


	LoadString( strText, bCompleted ? IDS_DLM_OPEN_OPEN : IDS_DLM_OPEN_PREVIEW );
	Update( &m_wndLaunch, strText );
	Update( &m_wndLaunch, m_pDownload->IsStarted() );
	Update( &m_wndStop, ! bCompleted );
	Update( &m_wndClose, ! bCompleted );

	CClientDC dc( this );
	DoPaint( dc );
}

void CDownloadMonitorDlg::Update(CWnd* pWnd, LPCTSTR pszText)
{
	CString strOld;
	pWnd->GetWindowText( strOld );
	if ( strOld != pszText )
		pWnd->SetWindowText( pszText );
}

void CDownloadMonitorDlg::Update(CWnd* pWnd, BOOL bEnabled)
{
	if ( pWnd->IsWindowEnabled() == bEnabled ) return;
	pWnd->EnableWindow( bEnabled );
}

void CDownloadMonitorDlg::OnPaint()
{
	CPaintDC dc( this );
	DoPaint( dc );
}

void CDownloadMonitorDlg::DoPaint(CDC& dc)
{
	CRect rc;

	m_wndProgress.GetWindowRect( &rc );
	ScreenToClient( &rc );

	DrawProgressBar( &dc, &rc );

	m_wndGraph.GetWindowRect( &rc );
	ScreenToClient( &rc );

	dc.Draw3dRect( &rc, 0, 0 );
	rc.DeflateRect( 1, 1 );

	m_pGraph->BufferedPaint( &dc, &rc );
}

void CDownloadMonitorDlg::DrawProgressBar(CDC* pDC, CRect* pRect)
{
	CRect rcCell( pRect );

	pDC->Draw3dRect( &rcCell, 0, 0 );
	rcCell.DeflateRect( 1, 1 );

	if ( Transfers.m_pSection.Lock( 50 ) )
	{
		if ( Downloads.Check( m_pDownload ) )
			CFragmentBar::DrawDownload( pDC, &rcCell, m_pDownload, Skin.m_crDialog );
		Transfers.m_pSection.Unlock();
	}
}

void CDownloadMonitorDlg::OnDownloadLaunch()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 250 ) || ! Downloads.Check( m_pDownload ) ) return;

	bool bComplete = m_pDownload->IsCompleted();

	m_pDownload->Launch( -1, &pLock, FALSE );

	pLock.Unlock();

	if ( bComplete ) PostMessage( WM_CLOSE );
}

void CDownloadMonitorDlg::OnDownloadLibrary()
{
	CWnd* pMainWnd = AfxGetMainWnd();
	if ( ! pMainWnd ) return;

	pMainWnd->PostMessage( WM_COMMAND, ID_VIEW_LIBRARY );
	pMainWnd->PostMessage( WM_SYSCOMMAND, SC_RESTORE );
}

void CDownloadMonitorDlg::OnDownloadStop()
{
	CSingleLock pLock( &Transfers.m_pSection );
	if ( ! pLock.Lock( 250 ) || ! Downloads.Check( m_pDownload ) ) return;

	if ( m_pDownload->IsStarted() )
	{
		CString strFormat, strPrompt;
		::LoadString( strFormat, IDS_DOWNLOAD_CONFIRM_CLEAR );
		strPrompt.Format( strFormat, (LPCTSTR)m_pDownload->m_sName );

		pLock.Unlock();
		if ( AfxMessageBox( strPrompt, MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 ) != IDYES ) return;
		pLock.Lock();
	}

	if ( Downloads.Check( m_pDownload ) && ! m_pDownload->IsMoving() )
	{
		m_pDownload->Remove();
		PostMessage( WM_CLOSE );
	}
}

void CDownloadMonitorDlg::OnDownloadCancel()
{
	PostMessage( WM_CLOSE );
}

void CDownloadMonitorDlg::OnClose()
{
	DestroyWindow();
}

void CDownloadMonitorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	UINT nCommand = nID & 0xFFF0;
	BOOL bShift = GetAsyncKeyState( VK_SHIFT ) & 0x8000;

	if ( nCommand == SC_MAXIMIZE || ( nCommand == SC_MINIMIZE && bShift ) )
	{
		if ( ! m_bTray )
		{
			m_pTray.cbSize				= sizeof(m_pTray);
			m_pTray.hWnd				= GetSafeHwnd();
			m_pTray.uID					= 0;
			m_pTray.uFlags				= NIF_ICON | NIF_MESSAGE | NIF_TIP;
			m_pTray.uCallbackMessage	= WM_TRAY;
			m_pTray.hIcon				= CoolInterface.ExtractIcon( IDI_DOWNLOAD_MONITOR, FALSE );
			_tcsncpy( m_pTray.szTip, Settings.SmartAgent(), _countof( m_pTray.szTip ) );
			Shell_NotifyIcon( NIM_ADD, &m_pTray );
			ShowWindow( SW_HIDE );
			m_bTray = TRUE;
		}
		return;
	}
	else if ( nCommand == SC_RESTORE && m_bTray )
	{
		OnTray( WM_LBUTTONDBLCLK, 0 );
		return;
	}
	else if ( nCommand == SC_NEXTWINDOW )
	{
		if ( GetExStyle() & WS_EX_TOPMOST )
		{
			SetWindowPos( &wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE );
		}
		else
		{
			SetWindowPos( &wndTopMost, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE );
		}
		return;
	}

	CSkinDialog::OnSysCommand( nID, lParam );
}

LRESULT CDownloadMonitorDlg::OnTray(WPARAM /*wParam*/, LPARAM lParam)
{
	if ( LOWORD(lParam) == WM_LBUTTONDBLCLK && m_bTray )
	{
		Shell_NotifyIcon( NIM_DELETE, &m_pTray );
		ShowWindow( SW_SHOWNORMAL );
		SetForegroundWindow();
		m_bTray = FALSE;
	}

	return 0;
}

HBRUSH CDownloadMonitorDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CSkinDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if ( pWnd == &m_wndFile )
	{
		pDC->SelectObject( &theApp.m_gdiFontBold );
	}

	return hbr;
}

void CDownloadMonitorDlg::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CMainWnd* pMainWnd = (CMainWnd*)AfxGetMainWnd();
	if ( ! pMainWnd || ! IsWindow( pMainWnd->m_hWnd ) ) return;

	CDownloadsWnd* pDownWnd = (CDownloadsWnd*)pMainWnd->m_pWindows.Find( RUNTIME_CLASS(CDownloadsWnd) );
	if ( ! pDownWnd ) return;

	if ( ! pDownWnd->Select( m_pDownload ) ) return;

	CMenu* pPopup = ::Skin.GetMenu( _T("CDownloadsWnd.Download") );
	if ( ! pPopup ) return;

	MENUITEMINFO pInfo;
	pInfo.cbSize	= sizeof(pInfo);
	pInfo.fMask		= MIIM_STATE;
	GetMenuItemInfo( pPopup->GetSafeHmenu(), m_pDownload->IsCompleted() ?
		ID_DOWNLOADS_LAUNCH_COMPLETE : ID_DOWNLOADS_LAUNCH_COPY, FALSE, &pInfo );
	pInfo.fState	|= MFS_DEFAULT;
	SetMenuItemInfo( pPopup->GetSafeHmenu(), m_pDownload->IsCompleted() ?
		ID_DOWNLOADS_LAUNCH_COMPLETE : ID_DOWNLOADS_LAUNCH_COPY, FALSE, &pInfo );

	CoolMenu.AddMenu( pPopup, TRUE );

	UINT nID = pPopup->TrackPopupMenu( TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD,
		point.x, point.y, pDownWnd );

	if ( nID && pDownWnd->Select( m_pDownload ) )
	{
		pDownWnd->SendMessage( WM_COMMAND, nID );
	}
}

BOOL CDownloadMonitorDlg::OnNeedText(UINT /*nID*/, NMHDR* pTTTH, LRESULT* /*pResult*/)
{
	// Fix it: It does not get a notification from the static window (!)
	if ( pTTTH->idFrom == IDC_DOWNLOAD_FILE )
	{
		TOOLTIPTEXT* pTTT = (TOOLTIPTEXT*)pTTTH;
		pTTT->lpszText = (LPTSTR)(LPCTSTR)m_sName;
	}

	return TRUE;
}

void CDownloadMonitorDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	DWORD nCheck = ( GetExStyle() & WS_EX_TOPMOST ) ? MF_CHECKED : MF_UNCHECKED;
	pPopupMenu->CheckMenuItem( SC_NEXTWINDOW, MF_BYCOMMAND|nCheck );

	CSkinDialog::OnInitMenuPopup( pPopupMenu, nIndex, bSysMenu );
}
