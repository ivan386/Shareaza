//
// DlgUpgrade.cpp
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
#include "VersionChecker.h"
#include "Network.h"
#include "Downloads.h"
#include "ShareazaURL.h"
#include "SHA.h"
#include "TigerTree.h"
#include "DlgUpgrade.h"
#include "WndMain.h"
#include "WndDownloads.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CUpgradeDlg, CSkinDialog)
	//{{AFX_MSG_MAP(CUpgradeDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CUpgradeDlg dialog

CUpgradeDlg::CUpgradeDlg(CWnd* pParent) : CSkinDialog(CUpgradeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUpgradeDlg)
	m_bCheck = FALSE;
	m_sMessage = _T("");
	//}}AFX_DATA_INIT
}

void CUpgradeDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUpgradeDlg)
	DDX_Check(pDX, IDC_DONT_CHECK, m_bCheck);
	DDX_Text(pDX, IDC_MESSAGE, m_sMessage);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CUpgradeDlg message handlers

BOOL CUpgradeDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CUpgradeDlg"), IDR_MAINFRAME );

	m_sMessage	= VersionChecker.m_sUpgradePrompt;
	m_bCheck	= FALSE;

	UpdateData( FALSE );

	return TRUE;
}

void CUpgradeDlg::OnOK()
{
	ParseCheckAgain();

	CShareazaURL pURL;

	pURL.m_nAction		= CShareazaURL::uriDownload;
	pURL.m_sName		= VersionChecker.m_sUpgradeFile;
	pURL.m_sURL			= VersionChecker.m_sUpgradeSources;

	if ( VersionChecker.m_sUpgradeSHA1.GetLength() )
	{
		pURL.m_bSHA1 = TRUE;
		CSHA::HashFromString( VersionChecker.m_sUpgradeSHA1, &pURL.m_pSHA1 );
	}

	if ( VersionChecker.m_sUpgradeTiger.GetLength() )
	{
		pURL.m_bTiger = TRUE;
		CTigerNode::HashFromString( VersionChecker.m_sUpgradeTiger, &pURL.m_pTiger );
	}

	if ( VersionChecker.m_sUpgradeSize.GetLength() )
	{
		QWORD nSize;
		if ( ( _stscanf( VersionChecker.m_sUpgradeSize.GetString(), _T("%I64i"), &nSize ) == 1 ) && ( nSize > 0 ) )
		{
			pURL.m_bSize = TRUE;
			pURL.m_nSize = nSize;
		}
	}

	Downloads.Add( &pURL );

	if ( ! Network.IsWellConnected() ) Network.Connect( TRUE );

	CMainWnd* pMainWnd = (CMainWnd*)AfxGetMainWnd();
	pMainWnd->m_pWindows.Open( RUNTIME_CLASS(CDownloadsWnd) );

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
