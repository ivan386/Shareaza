//
// DlgFolderScan.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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
#include "Library.h"
#include "DlgFolderScan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CFolderScanDlg, CSkinDialog)
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

CCriticalSection	CFolderScanDlg::m_pSection;
CFolderScanDlg*		CFolderScanDlg::m_pDialog = NULL;


/////////////////////////////////////////////////////////////////////////////
// CFolderScanDlg dialog

CFolderScanDlg::CFolderScanDlg(CWnd* pParent)
	: CSkinDialog( CFolderScanDlg::IDD, pParent )
	, m_nCookie		( 0 )
	, m_nFiles		( 0 )
	, m_nVolume		( 0 )
{
}

void CFolderScanDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_SCAN_VOLUME, m_wndVolume);
	DDX_Control(pDX, IDC_SCAN_FILES, m_wndFiles);
	DDX_Control(pDX, IDC_SCAN_FILE, m_wndFile);
}

/////////////////////////////////////////////////////////////////////////////
// CFolderScanDlg operations

BOOL CFolderScanDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CFolderScanDlg"), IDR_LIBRARYFRAME );

	CQuickLock oLock( m_pSection );
	m_pDialog = this;
	m_nCookie = Library.GetScanCount();

	SetTimer( 1, 250, NULL );

	return TRUE;
}

void CFolderScanDlg::OnTimer(UINT_PTR /*nIDEvent*/)
{
	CQuickLock oLock( m_pSection );

	m_wndFile.SetWindowText( m_sName );

	CString strItem;
	strItem.Format( _T("%lu"), m_nFiles );
	m_wndFiles.SetWindowText( strItem );

	m_wndVolume.SetWindowText( Settings.SmartVolume( m_nVolume, KiloBytes ) );

	RedrawWindow();

	if ( m_nCookie != Library.GetScanCount() )
	{
		CSkinDialog::OnCancel();
	}
}

void CFolderScanDlg::OnDestroy()
{
	if ( m_pDialog )
	{
		CQuickLock oLock( m_pSection );
		m_pDialog = NULL;
	}

	CSkinDialog::OnDestroy();
}

void CFolderScanDlg::Update(LPCTSTR pszName, DWORD nVolume)
{
	if ( m_pDialog )
	{
		CQuickLock oLock( m_pSection );
		if ( m_pDialog )
		{
			m_pDialog->m_nFiles ++;
			m_pDialog->m_nVolume += nVolume;
			m_pDialog->m_sName = pszName;
		}				
	}
}
