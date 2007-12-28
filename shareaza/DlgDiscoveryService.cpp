//
// DlgDiscoveryService.cpp
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
#include "Network.h"
#include "DiscoveryServices.h"
#include "DlgDiscoveryService.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CDiscoveryServiceDlg, CSkinDialog)
	//{{AFX_MSG_MAP(CDiscoveryServiceDlg)
	ON_EN_CHANGE(IDC_ADDRESS, OnChangeAddress)
	ON_CBN_SELCHANGE(IDC_SERVICE_TYPE, OnSelChangeServiceType)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDiscoveryServiceDlg dialog

CDiscoveryServiceDlg::CDiscoveryServiceDlg(CWnd* pParent, CDiscoveryService* pService) : CSkinDialog(CDiscoveryServiceDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDiscoveryServiceDlg)
	m_sAddress = _T("");
	m_nType = -1;
	//}}AFX_DATA_INIT
	m_pService	= pService;
	m_bNew		= FALSE;
}

CDiscoveryServiceDlg::~CDiscoveryServiceDlg()
{
	if ( m_pService && m_bNew ) delete m_pService;
}

void CDiscoveryServiceDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDiscoveryServiceDlg)
	DDX_Control(pDX, IDOK, m_wndOK);
	DDX_Text(pDX, IDC_ADDRESS, m_sAddress);
	DDX_CBIndex(pDX, IDC_SERVICE_TYPE, m_nType);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryServiceDlg message handlers

BOOL CDiscoveryServiceDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CDiscoveryServiceDlg"), IDR_DISCOVERYFRAME );

	CSingleLock pLock( &Network.m_pSection, TRUE );

	m_bNew = ! DiscoveryServices.Check( m_pService );
	if ( m_bNew ) m_pService = new CDiscoveryService();

	m_sAddress	= m_pService->m_sAddress;
	m_nType		= m_pService->m_nType - 1;

	if ( m_bNew ) m_nType = 1;

	pLock.Unlock();

	UpdateData( FALSE );

	OnChangeAddress();

	return TRUE;
}

void CDiscoveryServiceDlg::OnChangeAddress()
{
	UpdateData();

	m_wndOK.EnableWindow( m_nType ?
		_tcsncmp( m_sAddress, _T("http://"), 7 ) == 0 :
		_tcschr( m_sAddress, '.' ) != NULL );
}

void CDiscoveryServiceDlg::OnSelChangeServiceType()
{
	OnChangeAddress();
}

void CDiscoveryServiceDlg::OnOK()
{
	UpdateData();

	CSingleLock pLock( &Network.m_pSection, TRUE );

	if ( ! m_bNew && ! DiscoveryServices.Check( m_pService ) )
		m_pService = new CDiscoveryService();

	m_pService->m_sAddress	= m_sAddress;
	m_pService->m_nType		= m_nType + 1;

	if ( m_pService->m_nType == CDiscoveryService::dsGnutella )
	{
		if ( _tcsnicmp( m_sAddress, _T("gnutella1:host:"),  15 ) == 0 )
		{
			m_pService->m_bGnutella1 = TRUE;
			m_pService->m_bGnutella2 = FALSE;
			m_pService->m_nSubType = 1;
		}
		else if ( _tcsnicmp( m_sAddress, _T("gnutella2:host:"), 15 ) == 0 )
		{
			m_pService->m_bGnutella1 = FALSE;
			m_pService->m_bGnutella2 = TRUE;
			m_pService->m_nSubType = 2;
		}
		else if ( _tcsnicmp( m_sAddress, _T("uhc:"), 4 ) == 0 )
		{
			m_pService->m_bGnutella1 = TRUE;
			m_pService->m_bGnutella2 = FALSE;
			m_pService->m_nSubType = 3;
		}
		else if ( _tcsnicmp( m_sAddress, _T("ukhl:"), 5 ) == 0 )
		{
			m_pService->m_bGnutella1 = FALSE;
			m_pService->m_bGnutella2 = TRUE;
			m_pService->m_nSubType = 4;
		}
	}

	DiscoveryServices.Add( m_pService );
	m_pService = NULL;

	pLock.Unlock();

	CSkinDialog::OnOK();
}
