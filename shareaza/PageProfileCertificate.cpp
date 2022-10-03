//
// PageProfileCertificate.cpp
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
#include "GProfile.h"
#include "XML.h"
#include "PageProfileCertificate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CCertificateProfilePage, CSettingsPage)

BEGIN_MESSAGE_MAP(CCertificateProfilePage, CSettingsPage)
	//{{AFX_MSG_MAP(CCertificateProfilePage)
	ON_BN_CLICKED(IDC_GUID_CREATE, OnGuidCreate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCertificateProfilePage property page

CCertificateProfilePage::CCertificateProfilePage() : CSettingsPage( CCertificateProfilePage::IDD )
{
}

CCertificateProfilePage::~CCertificateProfilePage()
{
}

void CCertificateProfilePage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCertificateProfilePage)
	DDX_Text(pDX, IDC_GUID, m_sGUID);
	DDX_Text(pDX, IDC_GUID_TIME, m_sTime);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CCertificateProfilePage message handlers

BOOL CCertificateProfilePage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	wchar_t szGUID[39];
	Hashes::Guid tmp( MyProfile.oGUID );
	szGUID[ StringFromGUID2( *(GUID*)&tmp[ 0 ], szGUID, 39 ) - 2 ] = 0;
	m_sGUID = (CString)&szGUID[1];

	UpdateData( FALSE );

	return TRUE;
}

void CCertificateProfilePage::OnGuidCreate()
{
	MyProfile.Create();

	UpdateData( TRUE );

	wchar_t szGUID[39];
	Hashes::Guid tmp( MyProfile.oGUID );
	szGUID[ StringFromGUID2( *(GUID*)&tmp[ 0 ], szGUID, 39 ) - 2 ] = 0;
	m_sGUID = (CString)&szGUID[1];

	UpdateData( FALSE );
}

void CCertificateProfilePage::OnOK()
{
	UpdateData();

	CSettingsPage::OnOK();
}

