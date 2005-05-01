//
// DlgHelp.cpp
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
	//{{AFX_MSG_MAP(CHelpDlg)
	ON_WM_SIZE()
	ON_NOTIFY(RVN_CLICK, IDC_HELP_VIEW, OnClickView)
	//}}AFX_MSG_MAP
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

CHelpDlg::CHelpDlg(LPCTSTR pszName, CWnd* pParent) : CSkinDialog( CHelpDlg::IDD, pParent )
{
	m_sDocument = pszName;
	//{{AFX_DATA_INIT(CHelpDlg)
	//}}AFX_DATA_INIT
}

void CHelpDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHelpDlg)
	DDX_Control(pDX, IDC_BANNER, m_wndBanner);
	//}}AFX_DATA_MAP
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

	CRect rect( 0, 0, 0, 0 );
	m_wndView.Create( WS_CHILD, rect, this, IDC_HELP_VIEW );
	m_wndView.SetDocument( &m_pDocument );
	m_wndView.SetSelectable( TRUE );

	SkinMe( _T("CHelpDlg"), ID_HELP_ABOUT );
	OnSize( 1982, 0, 0 );

	return TRUE;
}

void CHelpDlg::OnSize(UINT nType, int cx, int cy)
{
	if ( nType != 1982 ) CSkinDialog::OnSize( nType, cx, cy );

	if ( m_wndBanner.m_hWnd != NULL && m_wndView.m_hWnd != NULL )
	{
		CRect rcClient, rcBanner;

		GetClientRect( &rcClient );
		m_wndBanner.GetClientRect( &rcBanner );

		rcClient.top += rcBanner.Height();
		rcClient.bottom -= 32;

		m_wndView.SetWindowPos( NULL, rcClient.left, rcClient.top,
			rcClient.Width(), rcClient.Height(), SWP_NOZORDER|SWP_SHOWWINDOW );
	}
}

void CHelpDlg::OnClickView(RVN_ELEMENTEVENT* pNotify, LRESULT *pResult)
{
	if ( CRichElement* pElement = pNotify->pElement )
	{
		theApp.InternalURI( pElement->m_sLink );
	}
}
