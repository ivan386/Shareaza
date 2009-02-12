//
// PageProfileBio.cpp : implementation file
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
#include "GProfile.h"
#include "XML.h"
#include "PageProfileBio.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CBioProfilePage, CSettingsPage)

BEGIN_MESSAGE_MAP(CBioProfilePage, CSettingsPage)
	//{{AFX_MSG_MAP(CBioProfilePage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBioProfilePage property page

CBioProfilePage::CBioProfilePage() : CSettingsPage( CBioProfilePage::IDD )
{
	//{{AFX_DATA_INIT(CBioProfilePage)
	//}}AFX_DATA_INIT
}

CBioProfilePage::~CBioProfilePage()
{
}

void CBioProfilePage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBioProfilePage)
	DDX_Control(pDX, IDC_PROFILE_BIO, m_wndText);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CBioProfilePage message handlers

BOOL CBioProfilePage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	if ( CXMLElement* pNotes = MyProfile.GetXML( _T("notes") ) )
	{
		m_wndText.SetWindowText( pNotes->GetValue() );
	}

	UpdateData( FALSE );

	return TRUE;
}

void CBioProfilePage::OnOK()
{
	if ( CXMLElement* pNotes = MyProfile.GetXML( _T("notes"), TRUE ) )
	{
		CString str;
		m_wndText.GetWindowText( str );

		if ( str.GetLength() )
			pNotes->SetValue( str );
		else
			pNotes->Delete();
	}

	CSettingsPage::OnOK();
}
