//
// DlgDownload.cpp
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
#include "Download.h"
#include "ShareazaURL.h"
#include "DlgDownload.h"
#include "Settings.h"

IMPLEMENT_DYNAMIC(CDownloadDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CDownloadDlg, CSkinDialog)
	ON_EN_CHANGE(IDC_URL, OnChangeURL)
	ON_BN_CLICKED(IDC_TORRENT_FILE, OnTorrentFile)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDownloadDlg dialog

CDownloadDlg::CDownloadDlg(CWnd* pParent, CDownload* pDownload) : CSkinDialog( CDownloadDlg::IDD, pParent )
{
	m_pDownload = pDownload;
	m_pURL = NULL;
}

CDownloadDlg::~CDownloadDlg()
{
	if ( m_pURL != NULL ) delete m_pURL;
}

void CDownloadDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TORRENT_FILE, m_wndTorrentFile);
	DDX_Control(pDX, IDOK, m_wndOK);
	DDX_Control(pDX, IDC_URL, m_wndURL);
	DDX_Text(pDX, IDC_URL, m_sURL);
}

/////////////////////////////////////////////////////////////////////////////
// CDownloadDlg message handlers

BOOL CDownloadDlg::OnInitDialog() 
{
	CSkinDialog::OnInitDialog();
	
	SkinMe( NULL, IDR_DOWNLOADSFRAME );
	m_wndTorrentFile.EnableWindow( m_pDownload == NULL );
	
	if ( OpenClipboard() )
	{
		if ( HGLOBAL hData = GetClipboardData( CF_TEXT ) )
		{
			DWORD nData = GlobalSize( hData );
			LPVOID pData = GlobalLock( hData );
			
			LPSTR pszData = new CHAR[ nData + 1 ];
			CopyMemory( pszData, pData, nData );
			pszData[ nData ] = 0;
			CString str = pszData;
			delete [] pszData;
			
			GlobalUnlock( hData );
			
			str.Trim( _T(" \t\r\n") );
			CShareazaURL pURL;
			if( pURL.Parse( str ) )
			{
				m_sURL = str;
				UpdateData( FALSE );
				OnChangeURL();
			}
		}
		
		CloseClipboard();
	}
	
	return TRUE;
}

void CDownloadDlg::OnChangeURL() 
{
	UpdateData();
	
	if ( m_pDownload && FALSE )
	{
		int nIP[4];
		
		if ( _stscanf( m_sURL, _T("%i.%i.%i.%i"), &nIP[0], &nIP[1], &nIP[2], &nIP[3] ) == 4 )
		{
			m_wndOK.EnableWindow( TRUE );
			return;
		}
	}
	
	CShareazaURL pURL;
	
	m_wndOK.EnableWindow( pURL.Parse( m_sURL ) &&
		( m_pDownload == NULL || pURL.m_nAction == CShareazaURL::uriSource ) );
}

void CDownloadDlg::OnTorrentFile() 
{
	CFileDialog dlg( TRUE, _T("torrent"), ( Settings.Downloads.TorrentPath + "\\." ) , OFN_HIDEREADONLY,
		_T("Torrent Files|*.torrent|All Files|*.*||"), this );
	
	if ( dlg.DoModal() != IDOK ) return;
	
	CBTInfo* pTorrent = new CBTInfo();
	
	if ( pTorrent->LoadTorrentFile( dlg.GetPathName() ) )
	{
		CShareazaURL* pURL = new CShareazaURL( pTorrent );
		
		if ( AfxGetMainWnd()->PostMessage( WM_URL, (WPARAM)pURL ) )
		{
			EndDialog( IDCANCEL );
			return;
		}
		
		delete pURL;
	}
	
	delete pTorrent;
}

void CDownloadDlg::OnOK() 
{
	UpdateData( TRUE );

	if ( m_pURL != NULL ) delete m_pURL;
	m_pURL = new CShareazaURL();
	
	if ( m_pURL->Parse( m_sURL ) ) CSkinDialog::OnOK();
}

CShareazaURL* CDownloadDlg::GetURL()
{
	CShareazaURL* pURL = m_pURL;
	m_pURL = NULL;
	return pURL;
}
