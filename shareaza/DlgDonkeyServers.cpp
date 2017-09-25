//
// DlgDonkeyServers.cpp
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
#include "Buffer.h"
#include "HostCache.h"
#include "DlgDonkeyServers.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CDonkeyServersDlg, CSkinDialog)
	ON_EN_CHANGE(IDC_URL, OnChangeURL)
	ON_WM_TIMER()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDonkeyServersDlg dialog

CDonkeyServersDlg::CDonkeyServersDlg(CWnd* pParent) :
	CSkinDialog(CDonkeyServersDlg::IDD, pParent)
{
}

void CDonkeyServersDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_URL, m_wndURL);
	DDX_Control(pDX, IDOK, m_wndOK);
	DDX_Control(pDX, IDC_PROGRESS, m_wndProgress);
	DDX_Text(pDX, IDC_URL, m_sURL);
}

/////////////////////////////////////////////////////////////////////////////
// CDonkeyServersDlg message handlers

BOOL CDonkeyServersDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CDonkeyServersDlg") );

	m_sURL = Settings.eDonkey.ServerListURL;

	m_wndOK.EnableWindow( m_sURL.Find( _T("http://") ) == 0 );
	m_wndProgress.SetRange( 0, 100 );
	m_wndProgress.SetPos( 0 );

	UpdateData( FALSE );

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

	if ( m_sURL.Find( _T("http://") ) != 0 )
		return;

	if ( ! m_pRequest.SetURL( m_sURL ) )
		return;

	if ( ! m_pRequest.Execute( true ) )
		return;

	Settings.eDonkey.ServerListURL = m_sURL;

	m_wndOK.EnableWindow( FALSE );
	m_wndURL.EnableWindow( FALSE );

	SetTimer( 1, 250, NULL );
}

void CDonkeyServersDlg::OnCancel()
{
	KillTimer( 1 );

	m_pRequest.Cancel();

	CSkinDialog::OnCancel();
}

void CDonkeyServersDlg::OnTimer(UINT_PTR nIDEvent)
{
	CSkinDialog::OnTimer( nIDEvent );

	if ( m_pRequest.IsPending() )
	{
		int n = m_wndProgress.GetPos();
		if ( ++n >= 100 )
			n = 0;
		m_wndProgress.SetPos( n );
	}
	else
	{
		KillTimer( 1 );

		if ( m_pRequest.GetStatusSuccess() )
		{
			const CBuffer* pBuffer = m_pRequest.GetResponseBuffer();

			CMemFile pFile;
			pFile.Write( pBuffer->m_pBuffer, pBuffer->m_nLength );
			pFile.Seek( 0, CFile::begin );

			if ( HostCache.ImportMET( &pFile ) )
				HostCache.Save();
		}
		else
		{
			CString strError;
			strError.Format( LoadString( IDS_DOWNLOAD_DROPPED ), (LPCTSTR)m_sURL );
			AfxMessageBox( strError, MB_OK | MB_ICONEXCLAMATION );
		}

		EndDialog( IDOK );
	}

	UpdateWindow();
}
