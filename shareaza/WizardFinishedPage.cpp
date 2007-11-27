//
// WizardFinishedPage.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
, m_bAutoConnect( FALSE )
, m_bConnect( FALSE )
, m_bStartup( FALSE )
{
	//{{AFX_DATA_INIT(CWizardFinishedPage)
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

	m_bAutoConnect	= Settings.Connection.AutoConnect;
	m_bConnect		= TRUE;
	if ( Settings.Live.FirstRun )
		m_bStartup = TRUE;
	else
		m_bStartup = Settings.CheckStartup();

	UpdateData( FALSE );

	return TRUE;
}

BOOL CWizardFinishedPage::OnSetActive()
{
	SetWizardButtons( PSWIZB_BACK | PSWIZB_FINISH );
	return CWizardPage::OnSetActive();
}

LRESULT CWizardFinishedPage::OnWizardBack()
{
	// If this system is capable of handling multiple networks, go to the network settings wizard.
	if ( IsConnectionCapable() )
		return 0;
	else
		return IDD_WIZARD_INTERFACE;
}

BOOL CWizardFinishedPage::OnWizardFinish()
{
	UpdateData();

	Settings.Connection.AutoConnect = m_bAutoConnect;

	if ( Settings.eDonkey.EnableToday )
	{
		if ( HostCache.eDonkey.GetCount() < 3 )
		{
			CDonkeyServersDlg dlg;
			dlg.DoModal();
		}
	}

	if ( m_bConnect && !Network.IsConnected() ) Network.Connect( TRUE );
	else if ( !m_bConnect && Network.IsConnected() ) Network.Disconnect();
	Settings.SetStartup( m_bStartup );

	return CWizardPage::OnWizardFinish();
}
