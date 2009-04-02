//
// WizardWelcomePage.cpp
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
#include "WizardSheet.h"
#include "WizardWelcomePage.h"
#include "Skin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CWizardWelcomePage, CWizardPage)

BEGIN_MESSAGE_MAP(CWizardWelcomePage, CWizardPage)
	//{{AFX_MSG_MAP(CWizardWelcomePage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWizardWelcomePage property page

CWizardWelcomePage::CWizardWelcomePage() : CWizardPage(CWizardWelcomePage::IDD)
{
	//{{AFX_DATA_INIT(CWizardWelcomePage)
	//}}AFX_DATA_INIT
}

CWizardWelcomePage::~CWizardWelcomePage()
{
}

void CWizardWelcomePage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWizardWelcomePage)
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CWizardWelcomePage message handlers

BOOL CWizardWelcomePage::OnInitDialog()
{
	CWizardPage::OnInitDialog();

	Skin.Apply( _T("CWizardWelcomePage"), this );

	SetWizardButtons( PSWIZB_NEXT );

	return TRUE;
}

BOOL CWizardWelcomePage::OnSetActive()
{
	SetWizardButtons( PSWIZB_NEXT );
	return CWizardPage::OnSetActive();
}
