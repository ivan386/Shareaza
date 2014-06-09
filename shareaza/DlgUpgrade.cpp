//
// DlgUpgrade.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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
#include "VersionChecker.h"
#include "Network.h"
#include "Download.h"
#include "Downloads.h"
#include "ShareazaURL.h"
#include "DlgUpgrade.h"
#include "Transfers.h"
#include "WndMain.h"
#include "WndDownloads.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CUpgradeDlg, CSkinDialog)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CUpgradeDlg dialog

CUpgradeDlg::CUpgradeDlg(CWnd* pParent)
	: CSkinDialog	( CUpgradeDlg::IDD, pParent )
	, m_bCheck		( FALSE )
{
}

void CUpgradeDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_DONT_CHECK, m_bCheck);
	DDX_Text(pDX, IDC_MESSAGE, m_sMessage);
}

/////////////////////////////////////////////////////////////////////////////
// CUpgradeDlg message handlers

BOOL CUpgradeDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CUpgradeDlg"), IDR_MAINFRAME );

	m_sMessage	= Settings.VersionCheck.UpgradePrompt;
	m_bCheck	= FALSE;

	UpdateData( FALSE );

	return TRUE;
}

void CUpgradeDlg::OnOK()
{
	ParseCheckAgain();

	CShareazaURL pURL;

	pURL.m_nAction		= CShareazaURL::uriDownload;
	pURL.m_sName		= Settings.VersionCheck.UpgradeFile;
	pURL.m_sURL			= Settings.VersionCheck.UpgradeSources;
    pURL.m_oSHA1.fromString( Settings.VersionCheck.UpgradeSHA1 );
    pURL.m_oTiger.fromString( Settings.VersionCheck.UpgradeTiger );

	if ( Settings.VersionCheck.UpgradeSize.GetLength() )
	{
		QWORD nSize;
		if ( _stscanf( Settings.VersionCheck.UpgradeSize.GetString(), _T("%I64u"), &nSize ) == 1 && nSize > 0 )
		{
			pURL.m_nSize = nSize;
		}
	}

	if ( ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0 && ! Network.IsWellConnected() )
		Network.Connect( TRUE );

	if ( CMainWnd* pMainWnd = (CMainWnd*)AfxGetMainWnd() )
		pMainWnd->m_pWindows.Open( RUNTIME_CLASS(CDownloadsWnd) );

	CSingleLock pLock( &Transfers.m_pSection, TRUE );

	if ( CDownload* pDownload = Downloads.Add( pURL ) )
	{
		if ( Settings.Downloads.ShowMonitorURLs )
			pDownload->ShowMonitor();
	}

	CSkinDialog::OnOK();
}

void CUpgradeDlg::OnCancel()
{
	ParseCheckAgain();

	CSkinDialog::OnCancel();
}

void CUpgradeDlg::ParseCheckAgain()
{
	UpdateData();

	if ( m_bCheck )
	{
		VersionChecker.SetNextCheck( 31 );
	}
}
