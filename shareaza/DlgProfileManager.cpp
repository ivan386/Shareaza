//
// DlgProfileManager.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2010.
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
#include "Skin.h"
#include "DlgProfileManager.h"
#include "PageProfileIdentity.h"
#include "PageProfileContact.h"
#include "PageProfileProfile.h"
#include "PageProfileBio.h"
#include "PageProfileAvatar.h"
#include "PageProfileFavourites.h"
#include "PageProfileFiles.h"
#include "PageProfileCertificate.h"
#include "PageSettingsRich.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CProfileManagerDlg, CSettingsSheet)

BEGIN_MESSAGE_MAP(CProfileManagerDlg, CSettingsSheet)
	ON_COMMAND(IDRETRY, OnApply)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CProfileManagerDlg construction

CProfileManagerDlg::CProfileManagerDlg(CWnd* pParent)
	: CSettingsSheet( pParent, IDS_USER_PROFILE )
{
}

/////////////////////////////////////////////////////////////////////////////
// CProfileManagerDlg operations

BOOL CProfileManagerDlg::Run(LPCTSTR pszWindow)
{
	CProfileManagerDlg pSheet;
	BOOL bResult = ( pSheet.DoModal( pszWindow ) == IDOK );
	return bResult;
}

INT_PTR CProfileManagerDlg::DoModal(LPCTSTR pszWindow)
{
	CIdentityProfilePage	pIdentity;
	CContactProfilePage		pContact;
	CProfileProfilePage		pProfile;
	CBioProfilePage			pBio;
	CAvatarProfilePage		pAvatar;
	CFavouritesProfilePage	pFavourites;
	CFilesProfilePage		pFiles;
	CCertificateProfilePage	pCertificate;

	AddGroup( &pIdentity );
	AddPage( &pContact );
	AddPage( &pProfile );
	AddPage( &pBio );
	AddPage( &pAvatar );
	AddGroup( &pFavourites );
	AddPage( &pFiles );
	AddGroup( &pCertificate );

	if ( pszWindow ) SetActivePage( GetPage( pszWindow ) );

	INT_PTR nReturn = CSettingsSheet::DoModal();

	return nReturn;
}

void CProfileManagerDlg::AddPage(CSettingsPage* pPage)
{
	CString strCaption = Skin.GetDialogCaption( CString( pPage->GetRuntimeClass()->m_lpszClassName ) );
	CSettingsSheet::AddPage( pPage, strCaption.GetLength() ? (LPCTSTR)strCaption : NULL );
}

void CProfileManagerDlg::AddGroup(CSettingsPage* pPage)
{
	if ( pPage->IsKindOf( RUNTIME_CLASS(CRichSettingsPage) ) )
	{
		CString strCaption = ((CRichSettingsPage*)pPage)->m_sCaption;
		CSettingsSheet::AddGroup( pPage, strCaption );
	}
	else
	{
		CString strName( pPage->GetRuntimeClass()->m_lpszClassName );
		CString strCaption = Skin.GetDialogCaption( strName );
		CSettingsSheet::AddGroup( pPage, strCaption.GetLength() ? (LPCTSTR)strCaption : NULL );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CProfileManagerDlg message handlers

BOOL CProfileManagerDlg::OnInitDialog()
{
	CSettingsSheet::OnInitDialog();

	SkinMe( NULL, IDR_MAINFRAME );

	return TRUE;
}

void CProfileManagerDlg::OnOK()
{
	CSettingsSheet::OnOK();
	MyProfile.Save();
}

void CProfileManagerDlg::OnApply()
{
	CSettingsSheet::OnApply();
	MyProfile.Save();
}
