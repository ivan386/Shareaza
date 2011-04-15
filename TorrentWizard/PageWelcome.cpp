//
// PageWelcome.cpp
//
// Copyright (c) Shareaza Development Team, 2007-2011.
// This file is part of Shareaza Torrent Wizard (shareaza.sourceforge.net).
//
// Shareaza Torrent Wizard is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Torrent Wizard is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Shareaza; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "StdAfx.h"
#include "TorrentWizard.h"
#include "PageWelcome.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CWelcomePage, CWizardPage)

BEGIN_MESSAGE_MAP(CWelcomePage, CWizardPage)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWelcomePage property page

CWelcomePage::CWelcomePage()
	: CWizardPage(CWelcomePage::IDD, _T("type") )
	, m_nType( 0 )
{
}

void CWelcomePage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);

	DDX_Radio(pDX, IDC_TYPE_SINGLE, m_nType);
}

/////////////////////////////////////////////////////////////////////////////
// CWelcomePage message handlers

void CWelcomePage::OnReset()
{
	m_nType = 0;

	UpdateData( FALSE );
}

BOOL CWelcomePage::OnSetActive() 
{
	SetWizardButtons( PSWIZB_NEXT );
	GetSheet()->GetDlgItem( 2 )->EnableWindow( TRUE );

	if ( ! theApp.m_sCommandLineSourceFile.IsEmpty() )
	{
		m_nType = PathIsDirectory( theApp.m_sCommandLineSourceFile ) ? 1 : 0;

		Next();
	}

	UpdateData( FALSE );

	return CWizardPage::OnSetActive();
}

LRESULT CWelcomePage::OnWizardNext() 
{
	UpdateData();
	
	if ( m_nType < 0 )
	{
		AfxMessageBox( IDS_WELCOME_NEED_TYPE, MB_ICONEXCLAMATION );
		return -1;
	}
	
	return m_nType ? IDD_PACKAGE_PAGE : IDD_SINGLE_PAGE;
}
