//
// PageSingle.cpp
//
// Copyright (c) Shareaza Development Team, 2007-2012.
// This file is part of Shareaza Torrent Wizard (shareaza.sourceforge.net).
//
// Shareaza Torrent Wizard is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Torrent Wizard is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Shareaza; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "StdAfx.h"
#include "TorrentWizard.h"
#include "PageSingle.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CSinglePage, CWizardPage)

BEGIN_MESSAGE_MAP(CSinglePage, CWizardPage)
	ON_BN_CLICKED(IDC_BROWSE_FILE, OnBrowseFile)
	ON_WM_TIMER()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSinglePage property page

CSinglePage::CSinglePage()
	: CWizardPage(CSinglePage::IDD, _T("single"))
{
}

void CSinglePage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_FILE_NAME, m_sFileName);
	DDX_Text(pDX, IDC_FILE_SIZE, m_sFileSize);
}

/////////////////////////////////////////////////////////////////////////////
// CSinglePage message handlers

void CSinglePage::OnReset() 
{
	m_sFileName.Empty();
	m_sFileSize.Empty();

	UpdateData( FALSE );
}

BOOL CSinglePage::OnSetActive() 
{
	SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );

	if ( ! theApp.m_sCommandLineSourceFile.IsEmpty() )
	{
		m_sFileName = theApp.m_sCommandLineSourceFile;
		theApp.m_sCommandLineSourceFile.Empty();

		Next();
	}

	if ( m_sFileName.IsEmpty() )
	{
		SetTimer( 1, 25, NULL );
	}
	else
	{
		Update();
	}

	return CWizardPage::OnSetActive();
}

void CSinglePage::OnTimer(UINT_PTR /*nIDEvent*/) 
{
	KillTimer( 1 );	
	PostMessage( WM_COMMAND, MAKELONG( IDC_BROWSE_FILE, BN_CLICKED ) );
}

void CSinglePage::OnBrowseFile() 
{
	UpdateData( TRUE );
	
	CFileDialog dlg( TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("All Files|*.*||"), this );
	if ( dlg.DoModal() != IDOK ) return;
	
	m_sFileName = dlg.GetPathName();

	Update();
}

void CSinglePage::Update()
{
	HANDLE hFile = CreateFile( CString( _T("\\\\?\\") ) + m_sFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
	
	if ( hFile != INVALID_HANDLE_VALUE )
	{
		DWORD nLow, nHigh;
		nLow = GetFileSize( hFile, &nHigh );
		CloseHandle( hFile );
		
		QWORD nSize = ( (QWORD)nHigh << 32 ) + (QWORD)nLow;
		m_sFileSize = SmartSize( nSize );
	}
	else
	{
		CString strFormat, strMessage;
		strFormat.LoadString( IDS_SINGLE_CANT_OPEN );
		strMessage.Format( strFormat, (LPCTSTR)m_sFileName );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		
		m_sFileName.Empty();
	}
	
	UpdateData( FALSE );
}

LRESULT CSinglePage::OnWizardBack() 
{
	return IDD_WELCOME_PAGE;
}

LRESULT CSinglePage::OnWizardNext() 
{
	UpdateData();
	
	if ( m_sFileName.IsEmpty() || GetFileAttributes( CString( _T("\\\\?\\") ) + m_sFileName ) == 0xFFFFFFFF )
	{
		AfxMessageBox( IDS_SINGLE_NEED_FILE, MB_ICONEXCLAMATION );
		return -1;
	}
	
	return IDD_TRACKER_PAGE;
}
