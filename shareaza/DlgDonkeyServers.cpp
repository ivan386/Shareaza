//
// DlgDonkeyServers.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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
#include "HostCache.h"
#include "DlgDonkeyServers.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CDonkeyServersDlg, CSkinDialog)
	//{{AFX_MSG_MAP(CDonkeyServersDlg)
	ON_EN_CHANGE(IDC_URL, OnChangeURL)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDonkeyServersDlg dialog

CDonkeyServersDlg::CDonkeyServersDlg(CWnd* pParent) : CSkinDialog(CDonkeyServersDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDonkeyServersDlg)
	m_sURL = _T("");
	//}}AFX_DATA_INIT
	m_hInternet = NULL;
}

CDonkeyServersDlg::~CDonkeyServersDlg()
{
	ASSERT( m_hInternet == NULL );
}

void CDonkeyServersDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDonkeyServersDlg)
	DDX_Control(pDX, IDC_URL, m_wndURL);
	DDX_Control(pDX, IDOK, m_wndOK);
	DDX_Control(pDX, IDC_PROGRESS, m_wndProgress);
	DDX_Text(pDX, IDC_URL, m_sURL);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CDonkeyServersDlg message handlers

BOOL CDonkeyServersDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CDonkeyServersDlg") );

	m_sURL = Settings.eDonkey.ServerListURL;
	UpdateData( FALSE );

	m_wndOK.EnableWindow( m_sURL.Find( _T("http://") ) == 0 );
	m_wndProgress.SetRange( 0, 100 );
	m_wndProgress.SetPos( 0 );

	return TRUE;
}

void CDonkeyServersDlg::OnChangeURL()
{
	UpdateData();
	m_wndOK.EnableWindow( m_sURL.Find( _T("http://") ) == 0 );
}

void CDonkeyServersDlg::OnOK()
{
	UpdateData();

	if ( m_sURL.Find( _T("http://") ) != 0 ) return;

	Settings.eDonkey.ServerListURL = m_sURL;

	CString strAgent = Settings.SmartAgent();
	m_hInternet = InternetOpen( strAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );
	if ( m_hInternet == NULL ) return;

	BeginThread( "DlgDonkeyServices" );

	m_wndOK.EnableWindow( FALSE );
	m_wndURL.EnableWindow( FALSE );
}

void CDonkeyServersDlg::OnCancel()
{
	OnTimer( 2 );
	CSkinDialog::OnCancel();
}

void CDonkeyServersDlg::OnTimer(UINT_PTR nIDEvent)
{
	if ( m_hInternet != NULL )
	{
		InternetCloseHandle( m_hInternet );
		m_hInternet = NULL;
	}

	CloseThread();

	if ( nIDEvent == 1 ) EndDialog( IDOK );
}

/////////////////////////////////////////////////////////////////////////////
// CDonkeyServersDlg thread

void CDonkeyServersDlg::OnRun()
{
	if ( m_hInternet == NULL ) return;

	HINTERNET hRequest = InternetOpenUrl( m_hInternet, m_sURL, NULL, 0,
		INTERNET_FLAG_RELOAD|INTERNET_FLAG_DONT_CACHE, 0 );

	if ( hRequest == NULL )
	{
		InternetCloseHandle( m_hInternet );
		m_hInternet = NULL;
		PostMessage( WM_TIMER, FALSE );
		return;
	}

	DWORD nLength, nlLength = 4;
	DWORD nRemaining = 0;
	const DWORD nBufferLength = 1024;
	auto_array< BYTE > pBuffer( new BYTE[ nBufferLength ] );
	CMemFile pFile;

	if ( HttpQueryInfo( hRequest, HTTP_QUERY_CONTENT_LENGTH|HTTP_QUERY_FLAG_NUMBER,
		&nLength, &nlLength, NULL ) )
	{
		m_wndProgress.PostMessage( PBM_SETRANGE32, 0, nLength );
	}

	nLength = 0;

	while ( InternetQueryDataAvailable( hRequest, &nRemaining, 0, 0 ) && nRemaining > 0 )
	{
		nLength += nRemaining;
		m_wndProgress.PostMessage( PBM_SETPOS, nLength );

		while ( nRemaining > 0 )
		{
			DWORD nBuffer = min( nRemaining, nBufferLength );
			InternetReadFile( hRequest, pBuffer.get(), nBuffer, &nBuffer );
			pFile.Write( pBuffer.get(), nBuffer );
			nRemaining -= nBuffer;
		}
	}

	pFile.Seek( 0, CFile::begin );

	BOOL bSuccess = HostCache.ImportMET( &pFile );
	if ( bSuccess ) HostCache.Save();

	InternetCloseHandle( m_hInternet );
	m_hInternet = NULL;

	PostMessage( WM_TIMER, bSuccess ? 1 : 0 );
}

