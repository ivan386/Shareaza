//
// PageWelcome.cpp
//
// Copyright (c) Shareaza Pty. Ltd., 2003.
// This file is part of TorrentAid Torrent Wizard (www.torrentaid.com).
//
// TorrentAid Torrent Wizard is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// TorrentAid is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with TorrentAid; if not, write to the Free Software
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
	//{{AFX_MSG_MAP(CWelcomePage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWelcomePage property page

CWelcomePage::CWelcomePage() : CWizardPage(CWelcomePage::IDD)
{
	//{{AFX_DATA_INIT(CWelcomePage)
	m_nType = 0;
	//}}AFX_DATA_INIT
}

CWelcomePage::~CWelcomePage()
{
}

void CWelcomePage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWelcomePage)
	DDX_Radio(pDX, IDC_TYPE_SINGLE, m_nType);
	//}}AFX_DATA_MAP
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

