//
// PageSettingsRich.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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

IMPLEMENT_DYNCREATE(CRichSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CRichSettingsPage, CSettingsPage)
	//{{AFX_MSG_MAP(CRichSettingsPage)
	//}}AFX_MSG_MAP
	ON_NOTIFY(RVN_CLICK, IDC_RICH_VIEW, OnClickView)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CRichSettingsPage property page

CRichSettingsPage::CRichSettingsPage(LPCTSTR pszName) : CSettingsPage(CRichSettingsPage::IDD)
{
	if ( pszName == NULL )
		m_sName = GetRuntimeClass()->m_lpszClassName;
	else
		m_sName = pszName;
	
	m_pDocument = NULL;
	
	if ( CXMLElement* pXML = Skin.GetDocument( m_sName ) )
	{
		m_sCaption = pXML->GetAttributeValue( _T("title"), m_sName );
		
		m_pDocument = new CRichDocument();
		m_pDocument->CreateFonts( theApp.m_sFont1, 11 );
		m_pDocument->m_crBackground = CCoolInterface::GetDialogBkColor();
		m_pDocument->LoadXML( pXML );
	}
}

CRichSettingsPage::~CRichSettingsPage()
{
	if ( m_pDocument ) delete m_pDocument;
}

void CRichSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRichSettingsPage)
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CRichSettingsPage message handlers

BOOL CRichSettingsPage::OnInitDialog() 
{
	CSettingsPage::OnInitDialog();
	
	SetWindowText( m_sCaption );
	Skin.Apply( m_sName, this );

	CRect rc;
	GetClientRect( &rc );
	m_wndView.Create( WS_VISIBLE, rc, this, IDC_RICH_VIEW );
	m_wndView.SetDocument( m_pDocument );
	
	return TRUE;
}

void CRichSettingsPage::OnClickView(RVN_ELEMENTEVENT* pNotify, LRESULT *pResult)
{
	CRichElement* pElement = pNotify->pElement;
	if ( ! pElement ) return;

	if ( _tcsncmp( pElement->m_sLink, _T("raza:page:"), 10 ) == 0 )
	{
		CString strPage = pElement->m_sLink.Mid( 10 );
		GetSheet()->SetActivePage( GetSheet()->GetPage( strPage ) );
	}
}
