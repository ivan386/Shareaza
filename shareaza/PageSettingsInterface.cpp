//
// PageSettingsInterface.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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
#include "PageSettingsInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CInterfaceSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CInterfaceSettingsPage, CSettingsPage)
	//{{AFX_MSG_MAP(CInterfaceSettingsPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CInterfaceSettingsPage property page

CInterfaceSettingsPage::CInterfaceSettingsPage() : CSettingsPage(CInterfaceSettingsPage::IDD)
{
	//{{AFX_DATA_INIT(CInterfaceSettingsPage)
	//}}AFX_DATA_INIT
}

CInterfaceSettingsPage::~CInterfaceSettingsPage()
{
}

void CInterfaceSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInterfaceSettingsPage)
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CInterfaceSettingsPage message handlers

BOOL CInterfaceSettingsPage::OnInitDialog() 
{
	CSettingsPage::OnInitDialog();
	
	return TRUE;
}

void CInterfaceSettingsPage::OnOK() 
{
	CSettingsPage::OnOK();
}
