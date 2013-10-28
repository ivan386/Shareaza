//
// DlgNewSearch.cpp
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
#include "QuerySearch.h"
#include "Schema.h"
#include "XML.h"
#include "Skin.h"
#include "DlgNewSearch.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CNewSearchDlg, CSkinDialog)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_CBN_SELCHANGE(IDC_SCHEMAS, OnSelChangeSchemas)
	ON_CBN_CLOSEUP(IDC_SCHEMAS, OnCloseUpSchemas)
	ON_EN_CHANGE(IDC_SEARCH, OnChangeSearch)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CNewSearchDlg dialog

CNewSearchDlg::CNewSearchDlg(CWnd* pParent, CQuerySearch* pSearch, BOOL bLocal, BOOL bAgain)
	: CSkinDialog( CNewSearchDlg::IDD, pParent, FALSE )
	, m_pSearch( pSearch ? pSearch : new CQuerySearch() )
{
	m_bLocal	= bLocal;
	m_bAgain	= bAgain;
}

void CNewSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewSearchDlg)
	DDX_Control(pDX, IDCANCEL, m_wndCancel);
	DDX_Control(pDX, IDOK, m_wndOK);
	DDX_Control(pDX, IDC_SCHEMAS, m_wndSchemas);
	DDX_Control(pDX, IDC_SEARCH, m_wndSearch);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CNewSearchDlg message handlers

BOOL CNewSearchDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CNewSearchDlg"), IDR_SEARCHFRAME );

	SelectCaption( this, m_bLocal ? 2 : ( m_bAgain ? 1 : 0 ) );

	CRect rc;
	CString strText;
	m_wndSchema.Create( WS_CHILD|WS_VISIBLE|WS_BORDER|WS_TABSTOP, rc, this, IDC_METADATA );

	LoadString( strText, IDS_SEARCH_PLAIN_TEXT );
	m_wndSchemas.m_sNoSchemaText = strText;
	m_wndSchemas.Load( Settings.Search.LastSchemaURI );

	m_wndSchemas.Select( m_pSearch->m_pSchema );

	OnSelChangeSchemas();

	if ( m_pSearch->m_pXML )
	{
		m_wndSchema.UpdateData( m_pSearch->m_pXML->GetFirstElement(), FALSE );
	}

	Settings.LoadWindow( _T("NewSearch"), this );

	OnCloseUpSchemas();
	
	if ( m_pSearch->m_oSHA1 )
	{
		m_wndSearch.SetWindowText( m_pSearch->m_oSHA1.toUrn() );
		m_wndSchema.ShowWindow( SW_HIDE );
	}
	else
	{
		m_wndSearch.SetWindowText( m_pSearch->m_sSearch );
	}

	if ( m_wndSchemas.GetCurSel() > 0 ) m_wndSchemas.SetFocus();

	return FALSE;
}

void CNewSearchDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	CSkinDialog::OnGetMinMaxInfo( lpMMI );
	lpMMI->ptMinTrackSize.x = 256;
	lpMMI->ptMinTrackSize.y = 128;
}

void CNewSearchDlg::OnSize(UINT nType, int cx, int cy)
{
	CSkinDialog::OnSize( nType, cx, cy );

	if ( ! IsWindow( m_wndSchema.m_hWnd ) ) return;

	int nSpacing	= 8;
	int nHeight		= 20;
	int nButtonSize	= 72;

	m_wndSearch.SetWindowPos( NULL, nSpacing, nSpacing, cx - nSpacing * 2, nHeight, SWP_NOZORDER );
	m_wndSchemas.SetWindowPos( NULL, nSpacing, nSpacing * 2 + nHeight, cx - nSpacing * 2, nHeight, SWP_NOZORDER );

	if ( cy > nSpacing * 5 + nHeight * 3 + 4 )
	{
		m_wndSchema.SetWindowPos( NULL,
			nSpacing, nSpacing * 3 + nHeight * 2,
			cx - nSpacing * 2, cy - nSpacing * 5 - nHeight * 3 - 4,
			SWP_NOZORDER|SWP_SHOWWINDOW );
	}
	else
	{
		m_wndSchema.ShowWindow( SW_HIDE );
	}

	nHeight += 4;

	m_wndOK.SetWindowPos( NULL, nSpacing - 1, cy - nSpacing - nHeight, nButtonSize, nHeight, SWP_NOZORDER );
	m_wndCancel.SetWindowPos( NULL, nSpacing * 2 + nButtonSize - 1, cy - nSpacing - nHeight, nButtonSize, nHeight, SWP_NOZORDER );
}

void CNewSearchDlg::OnSelChangeSchemas()
{
	CSchemaPtr pSchema = m_wndSchemas.GetSelected();
	m_wndSchema.SetSchema( pSchema, TRUE );
}

void CNewSearchDlg::OnCloseUpSchemas()
{
	CSchemaPtr pSchema = m_wndSchemas.GetSelected();

	CRect rcWindow;
	GetWindowRect( &rcWindow );

	if ( pSchema != NULL )
	{
		if ( rcWindow.Height() <= 200 )
		{
			SetWindowPos( NULL, 0, 0, rcWindow.Width(), 264, SWP_NOMOVE|SWP_NOZORDER );
		}
		PostMessage( WM_KEYDOWN, VK_TAB );
	}
	else
	{
		m_wndSearch.SetFocus();
	}
}

void CNewSearchDlg::OnChangeSearch()
{
	CString strSearch;
	m_wndSearch.GetWindowText( strSearch );

	BOOL bHash = FALSE;
	Hashes::TigerHash oTiger;
	Hashes::Sha1Hash oSHA1;
	Hashes::Ed2kHash oED2K;
	Hashes::Md5Hash oMD5;
	Hashes::BtHash oBTH;

	bHash |= static_cast< BOOL >( oSHA1.fromUrn( strSearch ) );
	bHash |= static_cast< BOOL >( oTiger.fromUrn( strSearch ) );
	bHash |= static_cast< BOOL >( oED2K.fromUrn( strSearch ) );
	bHash |= static_cast< BOOL >( oMD5.fromUrn( strSearch ) );
	bHash |= static_cast< BOOL >( oBTH.fromUrn( strSearch ) ||
		oBTH.fromUrn< Hashes::base16Encoding >( strSearch ) );

	if ( m_wndSchema.IsWindowVisible() == bHash )
	{
		m_wndSchema.ShowWindow( bHash ? SW_HIDE : SW_SHOW );
	}
}

void CNewSearchDlg::OnOK()
{
	Settings.SaveWindow( _T("NewSearch"), this );

	m_wndSearch.GetWindowText( m_pSearch->m_sSearch );

	CSchemaPtr pSchema = m_wndSchemas.GetSelected();

	if ( m_pSearch->m_pXML )
		m_pSearch->m_pXML->Delete();

	m_pSearch->m_pSchema	= NULL;
	m_pSearch->m_pXML		= NULL;

	if ( pSchema != NULL )
	{
		m_pSearch->m_pSchema	= pSchema;
		m_pSearch->m_pXML		= pSchema->Instantiate();

		m_wndSchema.UpdateData( m_pSearch->m_pXML->AddElement( pSchema->m_sSingular ), TRUE );

		Settings.Search.LastSchemaURI = pSchema->GetURI();
	}
	else
	{
		Settings.Search.LastSchemaURI.Empty();
	}

	if ( ! m_pSearch->CheckValid() )
	{
		m_wndSearch.SetFocus();
		return;
	}

	CSkinDialog::OnOK();
}

