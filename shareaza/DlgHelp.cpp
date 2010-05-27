//
// DlgHelp.cpp
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
#include "DlgHelp.h"
#include "Skin.h"
#include "RichElement.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CHelpDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CHelpDlg, CSkinDialog)
	ON_NOTIFY(RVN_CLICK, IDC_HELP_VIEW, OnClickView)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CHelpDlg setup

BOOL CHelpDlg::Show(LPCTSTR pszName, CWnd* pParent)
{
	CHelpDlg dlg( pszName, pParent );
	return dlg.DoModal() == IDOK;
}

/////////////////////////////////////////////////////////////////////////////
// CHelpDlg construction

CHelpDlg::CHelpDlg(LPCTSTR pszName, CWnd* pParent)
	: CSkinDialog( CHelpDlg::IDD, pParent )
{
	m_sDocument = pszName;
}

void CHelpDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
}

/////////////////////////////////////////////////////////////////////////////
// CHelpDlg message handlers

BOOL CHelpDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	if ( CXMLElement* pXML = Skin.GetDocument( m_sDocument ) )
	{
		m_pDocument.LoadXML( pXML );
	}
	else
	{
		PostMessage( WM_CLOSE );
	}

	m_pDocument.m_crBackground = Skin.m_crDialog;

	CRect rcClient;
	GetClientRect( &rcClient );

	rcClient.top += GetBannerHeight();
	rcClient.bottom -= 40;

	m_wndView.Create( WS_CHILD | WS_VISIBLE, rcClient, this, IDC_HELP_VIEW );
	m_wndView.SetDocument( &m_pDocument );
	m_wndView.SetSelectable( TRUE );

	SkinMe( _T("CHelpDlg"), ID_HELP_ABOUT );

	return TRUE;
}

void CHelpDlg::OnClickView(NMHDR* pNotify, LRESULT* /*pResult*/)
{
	if ( CRichElement* pElement = ((RVN_ELEMENTEVENT*)pNotify)->pElement )
	{
		theApp.InternalURI( pElement->m_sLink );
	}
}
