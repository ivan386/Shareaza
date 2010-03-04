//
// DlgFileProperties.cpp
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
#include "DlgFileProperties.h"
#include "Library.h"
#include "SharedFile.h"
#include "SharedFolder.h"
#include "ShellIcons.h"
#include "Schema.h"
#include "XML.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CFilePropertiesDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CFilePropertiesDlg, CSkinDialog)
	//{{AFX_MSG_MAP(CFilePropertiesDlg)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_CBN_SELCHANGE(IDC_SCHEMAS, OnSelChangeSchemas)
	ON_WM_DESTROY()
	ON_CBN_CLOSEUP(IDC_SCHEMAS, OnCloseUpSchemas)
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFilePropertiesDlg dialog

CFilePropertiesDlg::CFilePropertiesDlg(CWnd* pParent, DWORD nIndex) : CSkinDialog( 0, pParent )
{
	m_nIndex	= nIndex;
	m_bHexHash	= FALSE;
	m_nWidth	= 0;

	//{{AFX_DATA_INIT(CFilePropertiesDlg)
	m_sName = _T("");
	m_sSize = _T("");
	m_sType = _T("");
	m_sPath = _T("");
	m_sIndex = _T("");
	m_sSHA1 = _T("");
	m_sTiger = _T("");
	//}}AFX_DATA_INIT
}

void CFilePropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFilePropertiesDlg)
	DDX_Control(pDX, IDC_FILE_HASH_LABEL, m_wndHash);
	DDX_Control(pDX, IDC_FILE_ICON, m_wndIcon);
	DDX_Control(pDX, IDCANCEL, m_wndCancel);
	DDX_Control(pDX, IDOK, m_wndOK);
	DDX_Control(pDX, IDC_SCHEMAS, m_wndSchemas);
	DDX_Text(pDX, IDC_FILE_NAME, m_sName);
	DDX_Text(pDX, IDC_FILE_SIZE, m_sSize);
	DDX_Text(pDX, IDC_FILE_TYPE, m_sType);
	DDX_Text(pDX, IDC_FILE_PATH, m_sPath);
	DDX_Text(pDX, IDC_FILE_INDEX, m_sIndex);
	DDX_Text(pDX, IDC_FILE_SHA1, m_sSHA1);
	DDX_Text(pDX, IDC_FILE_TIGER, m_sTiger);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CFilePropertiesDlg message handlers

BOOL CFilePropertiesDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CFilePropertiesDlg"), IDI_PROPERTIES );

	CRect rc;
	GetWindowRect( &rc );
	m_nWidth = rc.Width();

	m_wndSchema.Create( WS_CHILD|WS_VISIBLE|WS_BORDER|WS_TABSTOP, rc, this, IDC_METADATA );

	Update();

	if ( ! Settings.LoadWindow( _T("CFilePropertiesDlg"), this ) )
	{
		GetWindowRect( &rc );
		rc.bottom++;
		MoveWindow( &rc );
	}

	PostMessage( WM_TIMER, 1 );

	return TRUE;
}

void CFilePropertiesDlg::Update()
{
	CQuickLock oLock( Library.m_pSection );

	CLibraryFile* pFile = Library.LookupFile( m_nIndex );

	if ( pFile == NULL )
	{
		PostMessage( WM_COMMAND, IDCANCEL );
		return;
	}

	m_sName = pFile->m_sName;
	m_sPath = pFile->GetFolder();
	m_sSize = Settings.SmartVolume( pFile->GetSize() );
	m_sIndex.Format( _T("# %lu"), pFile->m_nIndex );

	if ( pFile->m_oSHA1 )
	{
		if ( m_bHexHash )
			m_sSHA1 = _T("sha1:") + pFile->m_oSHA1.toString< Hashes::base16Encoding >();
		else
			m_sSHA1 = pFile->m_oSHA1.toShortUrn();
	}
	else
	{
		LoadString(m_sSHA1, IDS_GENERAL_NOURNAVAILABLE );
	}

	m_sTiger = pFile->m_oTiger.toShortUrn();

	CString strExt = pFile->m_sName;
	int nPeriod = strExt.ReverseFind( '.' );
	if ( nPeriod > 0 ) strExt = strExt.Mid( nPeriod );

	CString strMIME, strText;
	HICON hIcon;

	if ( ShellIcons.Lookup( strExt, NULL, &hIcon, &m_sType, &strMIME ) ) m_wndIcon.SetIcon( hIcon );
	if ( strMIME.GetLength() ) m_sType += _T(" (") + strMIME + _T(")");

	UpdateData( FALSE );

	LoadString ( strText, IDS_SEARCH_NO_METADATA );
	m_wndSchemas.m_sNoSchemaText = strText;
	m_wndSchemas.Load( pFile->m_pSchema ? (LPCTSTR)pFile->m_pSchema->GetURI() : NULL );

	OnSelChangeSchemas();

	if ( pFile->m_pMetadata )
	{
		CXMLElement* pXML = pFile->m_pMetadata->Clone();

		if ( pFile->m_oSHA1 )
            pXML->AddAttribute( _T("SHA1"), pFile->m_oSHA1.toString() );
		else if ( CXMLAttribute* pSHA1 = pXML->GetAttribute( _T("SHA1") ) )
			pSHA1->Delete();

		m_wndSchema.UpdateData( pXML, FALSE );

		delete pXML;
	}

}

void CFilePropertiesDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	CSkinDialog::OnGetMinMaxInfo( lpMMI );

	if ( m_nWidth )
	{
		lpMMI->ptMinTrackSize.x = m_nWidth;
		lpMMI->ptMinTrackSize.y = 256;
		lpMMI->ptMaxTrackSize.x = m_nWidth;
	}
}

void CFilePropertiesDlg::OnSize(UINT nType, int cx, int cy)
{
	if ( nType != 1982 ) CSkinDialog::OnSize( nType, cx, cy );

	if ( ! IsWindow( m_wndSchema.m_hWnd ) ) return;

	CRect rc;

	m_wndSchemas.GetWindowRect( &rc );
	ScreenToClient( &rc );

	m_wndSchema.SetWindowPos( NULL, rc.left, rc.bottom + 8, rc.Width(),
		cy - 24 - 16 - ( rc.bottom + 8 ), SWP_NOZORDER );

	m_wndOK.GetWindowRect( &rc );
	ScreenToClient( &rc );

	m_wndOK.SetWindowPos( NULL, rc.left, cy - 32, 0, 0, SWP_NOZORDER|SWP_NOSIZE );
	m_wndCancel.SetWindowPos( NULL, rc.right + 8, cy - 32, 0, 0, SWP_NOZORDER|SWP_NOSIZE );
}

void CFilePropertiesDlg::OnTimer(UINT_PTR /*nIDEvent*/)
{
	CRect rc;
	GetClientRect( &rc );
	OnSize( 1982, rc.Width(), rc.Height() );
}

void CFilePropertiesDlg::OnSelChangeSchemas()
{
	CSchemaPtr pSchema = m_wndSchemas.GetSelected();
	m_wndSchema.SetSchema( pSchema );
}

void CFilePropertiesDlg::OnCloseUpSchemas()
{
	if ( CSchemaPtr pSchema = m_wndSchemas.GetSelected() )
	{
		PostMessage( WM_KEYDOWN, VK_TAB );
	}
}

void CFilePropertiesDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	CRect rc;

	m_wndHash.GetWindowRect( &rc );
	ScreenToClient( &rc );

	if ( rc.PtInRect( point ) )
	{
		m_bHexHash = ! m_bHexHash;
		Update();
	}

	CSkinDialog::OnLButtonUp(nFlags, point);
}

void CFilePropertiesDlg::OnOK()
{
	{
		CQuickLock oLock( Library.m_pSection );

		if ( CLibraryFile* pFile = Library.LookupFile( m_nIndex ) )
		{
			if ( CSchemaPtr pSchema = m_wndSchemas.GetSelected() )
			{
				CXMLElement* pXML		= pSchema->Instantiate( TRUE );
				CXMLElement* pSingular	= pXML->AddElement( pSchema->m_sSingular );

				m_wndSchema.UpdateData( pSingular, TRUE );

				if ( CXMLAttribute* pSHA1 = pSingular->GetAttribute( _T("SHA1") ) )
					pSHA1->Delete();

				pFile->SetMetadata( pXML );
				delete pXML;
			}
			else
			{
				pFile->ClearMetadata();
			}

			Library.Update();
		}
	}

	CSkinDialog::OnOK();
}

void CFilePropertiesDlg::OnDestroy()
{
	Settings.SaveWindow( _T("CFilePropertiesDlg"), this );
	CSkinDialog::OnDestroy();
}

