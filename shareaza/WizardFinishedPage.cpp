//
// WizardFinishedPage.cpp
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
#include "WizardSheet.h"
#include "WizardFinishedPage.h"
#include "Network.h"
#include "Skin.h"
#include "HostCache.h"
#include "DlgDonkeyServers.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CWizardFinishedPage, CWizardPage)

BEGIN_MESSAGE_MAP(CWizardFinishedPage, CWizardPage)
	//{{AFX_MSG_MAP(CWizardFinishedPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWizardFinishedPage property page

CWizardFinishedPage::CWizardFinishedPage() : CWizardPage(CWizardFinishedPage::IDD)
{
	//{{AFX_DATA_INIT(CWizardFinishedPage)
	m_bAutoConnect = FALSE;
	m_bConnect = FALSE;
	m_bStartup = FALSE;
	//}}AFX_DATA_INIT
}

CWizardFinishedPage::~CWizardFinishedPage()
{
}

void CWizardFinishedPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWizardFinishedPage)
	DDX_Check(pDX, IDC_WIZARD_AUTO, m_bAutoConnect);
	DDX_Check(pDX, IDC_WIZARD_CONNECT, m_bConnect);
	DDX_Check(pDX, IDC_WIZARD_STARTUP, m_bStartup);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CWizardFinishedPage message handlers

BOOL CWizardFinishedPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	Skin.Apply( _T("CWizardFinishedPage"), this );

	m_bConnect		= TRUE;
	m_bAutoConnect	= TRUE;
	m_bStartup		= TRUE;

	UpdateData( FALSE );

	return TRUE;
}

BOOL CWizardFinishedPage::OnSetActive()
{
	SetWizardButtons( PSWIZB_BACK | PSWIZB_FINISH );
	return CWizardPage::OnSetActive();
}

BOOL CWizardFinishedPage::OnWizardFinish()
{
	UpdateData();

	Settings.Connection.AutoConnect = m_bAutoConnect;

	if ( Settings.eDonkey.EnableToday )
	{
		if ( HostCache.eDonkey.m_nHosts < 3 )
		{
			CDonkeyServersDlg dlg;
			dlg.DoModal();
		}
	}

	if ( m_bConnect ) Network.Connect( TRUE );
	Settings.SetStartup( m_bStartup );

	return CWizardPage::OnWizardFinish();
}
