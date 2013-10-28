//
// PageSettingsRich.cpp
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
#include "CoolInterface.h"
#include "Skin.h"
#include "XML.h"
#include "Skin.h"
#include "RichDocument.h"
#include "RichElement.h"
#include "WndSettingsSheet.h"
#include "PageSettingsRich.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CRichSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CRichSettingsPage, CSettingsPage)
	ON_NOTIFY(RVN_CLICK, IDC_RICH_VIEW, OnClickView)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CRichSettingsPage property page

CRichSettingsPage::CRichSettingsPage(LPCTSTR pszName)
	: CSettingsPage( CRichSettingsPage::IDD, pszName )
	, m_pDocument( NULL )
{
	// Early caption load for settings tree items
	if ( CXMLElement* pXML = Skin.GetDocument( m_sName ) )
	{
		m_sCaption = pXML->GetAttributeValue( _T("title"), m_sName );
	}
}

CRichSettingsPage::~CRichSettingsPage()
{
	delete m_pDocument;
}

void CRichSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
}

/////////////////////////////////////////////////////////////////////////////
// CRichSettingsPage message handlers

BOOL CRichSettingsPage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	CRect rc;
	GetClientRect( &rc );
	m_wndView.Create( WS_VISIBLE, rc, this, IDC_RICH_VIEW );

	return TRUE;
}

void CRichSettingsPage::OnSkinChange()
{
	CSettingsPage::OnSkinChange();

	if ( ! IsWindow( GetSafeHwnd() ) )
		// Not created yet page
		return;

	// (Re)Load document
	if ( CXMLElement* pXML = Skin.GetDocument( m_sName ) )
	{
		m_sCaption = pXML->GetAttributeValue( _T("title"), m_sName );
		SetWindowText( m_sCaption );

		delete m_pDocument;
		m_pDocument = new CRichDocument();
		m_pDocument->LoadXML( pXML );
		m_pDocument->m_crBackground = Skin.m_crDialog;
		m_wndView.SetDocument( m_pDocument );
	}
}

void CRichSettingsPage::OnClickView(NMHDR* pNotify, LRESULT* /*pResult*/)
{
	CRichElement* pElement = ((RVN_ELEMENTEVENT*) pNotify )->pElement;
	if ( ! pElement ) return;

	if ( _tcsncmp( pElement->m_sLink, _T("raza:page:"), 10 ) == 0 )
	{
		CString strPage = pElement->m_sLink.Mid( 10 );
		GetSheet()->SetActivePage( GetSheet()->GetPage( strPage ) );
	}
}
