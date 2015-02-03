//
// DlgURLCopy.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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
#include "CoolInterface.h"
#include "DlgURLCopy.h"
#include "Download.h"
#include "Downloads.h"
#include "Library.h"
#include "Network.h"
#include "Transfer.h"
#include "Transfers.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CURLCopyDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CURLCopyDlg, CSkinDialog)
	ON_WM_CTLCOLOR()
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_INCLUDE_SELF, &CURLCopyDlg::OnIncludeSelf)
	ON_STN_CLICKED(IDC_URL_HOST, &CURLCopyDlg::OnStnClickedUrlHost)
	ON_STN_CLICKED(IDC_URL_MAGNET, &CURLCopyDlg::OnStnClickedUrlMagnet)
	ON_STN_CLICKED(IDC_URL_ED2K, &CURLCopyDlg::OnStnClickedUrlEd2k)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CURLCopyDlg dialog

CURLCopyDlg::CURLCopyDlg(CWnd* pParent) : CSkinDialog(CURLCopyDlg::IDD, pParent)
{
}

void CURLCopyDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_INCLUDE_SELF, m_wndIncludeSelf);
	DDX_Control(pDX, IDC_MESSAGE, m_wndMessage);
	DDX_Text(pDX, IDC_URL_HOST, m_sHost);
	DDX_Text(pDX, IDC_URL_MAGNET, m_sMagnet);
	DDX_Text(pDX, IDC_URL_ED2K, m_sED2K);
}

void CURLCopyDlg::Add(const CShareazaFile* pFile)
{
	ASSERT( pFile != NULL );
	m_pFile = *pFile;
}

/////////////////////////////////////////////////////////////////////////////
// CURLCopyDlg message handlers

BOOL CURLCopyDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( NULL, IDI_WEB_URL );
	
	m_wndIncludeSelf.ShowWindow( ( Network.IsListening() && m_pFile.m_sURL.IsEmpty() )
		? SW_SHOW : SW_HIDE );

	OnIncludeSelf();

	return TRUE;
}

void CURLCopyDlg::Resolve(CShareazaFile& pFile, CString& sTracker, CString& sWebSeed)
{
	// Use contents of .torrent-file instead of file itself
	CBTInfo pTorrent;
	if ( pFile.m_sPath.GetLength() &&
		_tcsicmp( PathFindExtension( pFile.m_sName ), _T(".torrent") ) == 0 &&
		pTorrent.LoadTorrentFile( pFile.m_sPath + _T("\\") + pFile.m_sName ) )
	{
		pFile = pTorrent;

		// Get trackers
		for ( int i = 0; i < pTorrent.GetTrackerCount(); ++i )
		{
			if ( sTracker.GetLength() ) sTracker += _T("&");
			sTracker += _T("tr=") + URLEncode( pTorrent.GetTrackerAddress( i ) );
		}

		// Get Web-seeds
		for ( POSITION pos = pTorrent.m_sURLs.GetHeadPosition(); pos; )
		{
			if ( sWebSeed.GetLength() ) sWebSeed += _T("&");
			sWebSeed += _T("ws=") + URLEncode( pTorrent.m_sURLs.GetNext( pos ) );
		}
	}

	if ( pFile.m_oBTH )
	{
		CQuickLock oLock( Transfers.m_pSection );

		if ( const CDownload* pDownload = Downloads.FindByBTH( pFile.m_oBTH ) )
		{
			// Refill missed hashes
			if ( ! pFile.m_oSHA1 && pDownload->m_oSHA1 )
				pFile.m_oSHA1 = pDownload->m_oSHA1;
			if ( ! pFile.m_oTiger && pDownload->m_oTiger )
				pFile.m_oTiger = pDownload->m_oTiger;
			if ( ! pFile.m_oED2K && pDownload->m_oED2K )
				pFile.m_oED2K = pDownload->m_oED2K;
			if ( ! pFile.m_oMD5 && pDownload->m_oMD5 )
				pFile.m_oMD5 = pDownload->m_oMD5;

			// Get trackers
			if ( sTracker.IsEmpty() && pDownload->IsTorrent() )
			{
				for ( int i = 0; i < pDownload->m_pTorrent.GetTrackerCount(); ++i )
				{
					if ( sTracker.GetLength() ) sTracker += _T("&");
					sTracker += _T("tr=") + URLEncode( pDownload->m_pTorrent.GetTrackerAddress( i ) );
				}
			}
		}
	}
			
	if ( pFile.m_oBTH )
	{
		CQuickLock oLock( Library.m_pSection );

		if ( const CLibraryFile* pLibraryFile = LibraryMaps.LookupFileByHash( &pFile ) )
		{
			// Refill missed hashes
			if ( ! pFile.m_oSHA1 && pLibraryFile->m_oSHA1 )
				pFile.m_oSHA1 = pLibraryFile->m_oSHA1;
			if ( ! pFile.m_oTiger && pLibraryFile->m_oTiger )
				pFile.m_oTiger = pLibraryFile->m_oTiger;
			if ( ! pFile.m_oED2K && pLibraryFile->m_oED2K )
				pFile.m_oED2K = pLibraryFile->m_oED2K;
			if ( ! pFile.m_oMD5 && pLibraryFile->m_oMD5 )
				pFile.m_oMD5 = pLibraryFile->m_oMD5;
		}
	}
}

CString CURLCopyDlg::CreateMagnet(CShareazaFile& pFile)
{
	CString strURN, sTracker, sWebSeed;

	Resolve( pFile, sTracker, sWebSeed );
	
	if ( pFile.m_oTiger && pFile.m_oSHA1 )
	{
		strURN = _T("xt=urn:bitprint:") + pFile.m_oSHA1.toString() + _T('.') + pFile.m_oTiger.toString();
	}
	else if ( pFile.m_oSHA1 )
	{
		strURN = _T("xt=") + pFile.m_oSHA1.toUrn();
	}
	else if ( pFile.m_oTiger )
	{
		strURN = _T("xt=") + pFile.m_oTiger.toUrn();
	}

	if ( pFile.m_oED2K )
	{
		if ( strURN.GetLength() ) strURN += _T("&");
		strURN += _T("xt=") + pFile.m_oED2K.toUrn();
	}

	if ( pFile.m_oMD5 && ! pFile.m_oTiger && ! pFile.m_oSHA1 && ! pFile.m_oED2K )
	{
		if ( strURN.GetLength() ) strURN += _T("&");
		strURN += _T("xt=") + pFile.m_oMD5.toUrn();
	}

	if ( pFile.m_oBTH )
	{
		if ( strURN.GetLength() ) strURN += _T("&");
		strURN += _T("xt=") + pFile.m_oBTH.toUrn();
	}

	CString sMagnet = strURN;

	if ( pFile.m_nSize != 0 && pFile.m_nSize != SIZE_UNKNOWN )
	{
		if ( sMagnet.GetLength() ) sMagnet += _T("&");
		sMagnet.AppendFormat( _T("xl=%I64u"), pFile.m_nSize );
	}

	if ( pFile.m_sName.GetLength() )
	{
		if ( sMagnet.GetLength() ) sMagnet += _T("&");
		if ( strURN.GetLength() )
			sMagnet += _T("dn=");
		else
			sMagnet += _T("kt=");
		sMagnet += URLEncode( pFile.m_sName );
	}

	if ( sTracker.GetLength() )
	{
		if ( sMagnet.GetLength() ) sMagnet += _T("&");
		sMagnet += sTracker;
	}

	if ( sWebSeed.GetLength() )
	{
		if ( sMagnet.GetLength() ) sMagnet += _T("&");
		sMagnet += sWebSeed;
	}

	sMagnet = _T("magnet:?") + sMagnet;

	return sMagnet;
}

void CURLCopyDlg::OnIncludeSelf()
{
	UpdateData();

	BOOL bIncludeSelf = ( m_wndIncludeSelf.GetCheck() == BST_CHECKED );

	m_sMagnet = CreateMagnet( m_pFile );

	CString strSelf = m_pFile.GetURL( Network.m_pHost.sin_addr, htons( Network.m_pHost.sin_port ) );

	if ( bIncludeSelf )
	{
		if ( strSelf.GetLength() )
			m_sMagnet += _T("&xs=") + URLEncode( strSelf );
	}
	else
	{
		if ( m_pFile.m_sURL.GetLength() )
			m_sMagnet += _T("&xs=") + URLEncode( m_pFile.m_sURL );
	}

	if ( m_pFile.m_oED2K &&
		m_pFile.m_nSize != 0 && m_pFile.m_nSize != SIZE_UNKNOWN &&
		m_pFile.m_sName.GetLength() )
	{
		m_sED2K.Format( _T("ed2k://|file|%s|%I64u|%s|/"),
			(LPCTSTR)URLEncode( m_pFile.m_sName ),
			m_pFile.m_nSize,
			(LPCTSTR)m_pFile.m_oED2K.toString() );

		if ( bIncludeSelf )
		{
			m_sED2K += _T("|sources,") + HostToString( &Network.m_pHost ) + _T("|/");
		}
	}
	else
	{
		m_sED2K.Empty();
	}

	if ( bIncludeSelf )
	{
		m_sHost = strSelf;
	}
	else if ( m_pFile.m_sURL.GetLength() )
	{
		m_sHost = m_pFile.m_sURL;
	}
	else
	{
		m_sHost.Empty();
	}

	UpdateData( FALSE );
}

HBRUSH CURLCopyDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CSkinDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if ( pWnd && pWnd != &m_wndMessage )
	{
		TCHAR szName[32];
		GetClassName( pWnd->GetSafeHwnd(), szName, 32 );

		if ( ! _tcsicmp( szName, _T("Static") ) )
		{
			pDC->SetTextColor( CoolInterface.m_crTextLink );
			pDC->SelectObject( &theApp.m_gdiFontLine );
		}
	}

	return hbr;
}

BOOL CURLCopyDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint point;
	GetCursorPos( &point );

	for ( pWnd = GetWindow( GW_CHILD ) ; pWnd ; pWnd = pWnd->GetNextWindow() )
	{
		TCHAR szName[32];
		GetClassName( pWnd->GetSafeHwnd(), szName, 32 );

		if ( ! _tcsicmp( szName, _T("Static") ) && pWnd != &m_wndMessage )
		{
			CString strText;
			CRect rc;

			pWnd->GetWindowRect( &rc );

			if ( rc.PtInRect( point ) )
			{
				pWnd->GetWindowText( strText );

				if ( strText.GetLength() )
				{
					SetCursor( theApp.LoadCursor( IDC_HAND ) );
					return TRUE;
				}
			}
		}
	}

	return CSkinDialog::OnSetCursor( pWnd, nHitTest, message );
}

void CURLCopyDlg::OnStnClickedUrlHost()
{
	UpdateData();

	if ( m_sHost.GetLength() )
	{
		theApp.SetClipboardText( m_sHost );

		CSkinDialog::OnOK();
	}
}

void CURLCopyDlg::OnStnClickedUrlMagnet()
{
	UpdateData();

	if ( m_sMagnet.GetLength() )
	{
		theApp.SetClipboardText( m_sMagnet );

		CSkinDialog::OnOK();
	}
}

void CURLCopyDlg::OnStnClickedUrlEd2k()
{
	UpdateData();

	if ( m_sED2K.GetLength() )
	{
		theApp.SetClipboardText( m_sED2K );

		CSkinDialog::OnOK();
	}
}
