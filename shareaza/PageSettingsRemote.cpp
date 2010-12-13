//
// PageSettingsRemote.cpp
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
#include "CoolInterface.h"
#include "Network.h"
#include "Handshakes.h"
#include "PageSettingsRemote.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CRemoteSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CRemoteSettingsPage, CSettingsPage)
	ON_BN_CLICKED(IDC_REMOTE_ENABLE, &CRemoteSettingsPage::OnBnClickedRemoteEnable)
	ON_EN_CHANGE(IDC_REMOTE_USERNAME, &CRemoteSettingsPage::OnBnClickedRemoteEnable)
	ON_EN_CHANGE(IDC_REMOTE_PASSWORD, &CRemoteSettingsPage::OnNewPassword)
	ON_WM_CTLCOLOR()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////////
// CRemoteSettingsPage construction

CRemoteSettingsPage::CRemoteSettingsPage() : CSettingsPage( CRemoteSettingsPage::IDD )
{
	m_bEnable = FALSE;
}

CRemoteSettingsPage::~CRemoteSettingsPage()
{
}

void CRemoteSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_REMOTE_ENABLE, m_bEnable);
	DDX_Control(pDX, IDC_REMOTE_URL, m_wndURL);
	DDX_Control(pDX, IDC_REMOTE_USERNAME, m_wndUsername);
	DDX_Text(pDX, IDC_REMOTE_USERNAME, m_sUsername);
	DDX_Control(pDX, IDC_REMOTE_PASSWORD, m_wndPassword);
	DDX_Text(pDX, IDC_REMOTE_PASSWORD, m_sPassword);
}

//////////////////////////////////////////////////////////////////////////////
// CRemoteSettingsPage message handlers

BOOL CRemoteSettingsPage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	m_bEnable	= m_bOldEnable		= Settings.Remote.Enable;
	m_sUsername	= m_sOldUsername	= Settings.Remote.Username;
	m_sOldPassword					= Settings.Remote.Password;

	if ( ! m_sOldPassword.IsEmpty() ) m_sPassword = _T("      ");

	UpdateData( FALSE );

	return TRUE;
}

void CRemoteSettingsPage::OnNewPassword()
{
	UpdateData();

	if ( m_sPassword.GetLength() < 3 )		// Password too short
	{
		Settings.Remote.Password = m_sOldPassword;
	}
	else if ( m_sPassword == _T("      ") )	// Password hasn't been edited
	{
		Settings.Remote.Password = m_sOldPassword;
	}
	else
	{
		CSHA pSHA1;
		pSHA1.Add( (LPCTSTR)m_sPassword, m_sPassword.GetLength() * sizeof(TCHAR) );
		pSHA1.Finish();
        Hashes::Sha1Hash tmp;
		pSHA1.GetHash( &tmp[ 0 ] );
		tmp.validate();
        Settings.Remote.Password = tmp.toString();
	}

	OnBnClickedRemoteEnable();
}

void CRemoteSettingsPage::OnBnClickedRemoteEnable()
{
	UpdateData();

	Settings.Remote.Enable		= m_bEnable != FALSE;
	Settings.Remote.Username	= m_sUsername;

	m_wndUsername.EnableWindow( m_bEnable );
	m_wndPassword.EnableWindow( m_bEnable );

	CString strURL;

	if ( m_bEnable && ! m_sUsername.IsEmpty() && ! Settings.Remote.Password.IsEmpty() )
	{
		if ( Network.IsListening() )
		{
			strURL.Format( _T("http://%s:%i/remote/"),
				(LPCTSTR)CString( inet_ntoa( Network.m_pHost.sin_addr ) ),
				(int)ntohs( Network.m_pHost.sin_port ) );
			m_wndURL.EnableWindow( TRUE );
		}
		else if ( Handshakes.IsValid() && Network.m_pHost.sin_port != 0 )
		{
			strURL.Format( _T("http://localhost:%i/remote/"),
				(int)ntohs( Network.m_pHost.sin_port ) );
			m_wndURL.EnableWindow( TRUE );
		}
		else
		{
			LoadString( strURL, IDS_REMOTE_UNAVAILABLE );
			m_wndURL.EnableWindow( FALSE );
		}
	}
	else if ( m_bEnable )
	{
		LoadString( strURL, IDS_REMOTE_ENABLED );
		m_wndURL.EnableWindow( FALSE );
	}
	else
	{
		LoadString( strURL, IDS_REMOTE_DISABLED );
		m_wndURL.EnableWindow( FALSE );
	}

	m_wndURL.SetWindowText( strURL );

}

HBRUSH CRemoteSettingsPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CSettingsPage::OnCtlColor( pDC, pWnd, nCtlColor );

	if ( pWnd == &m_wndURL && m_wndURL.IsWindowEnabled() )
	{
		pDC->SelectObject( &theApp.m_gdiFontLine );
		pDC->SetTextColor( CoolInterface.m_crTextLink );
	}

	return hbr;
}

BOOL CRemoteSettingsPage::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint point;
	GetCursorPos( &point );
	CRect rect;
	m_wndURL.GetWindowRect( &rect );

	if ( rect.PtInRect( point ) && m_wndURL.IsWindowEnabled() )
	{
		SetCursor( AfxGetApp()->LoadCursor( IDC_HAND ) );
		return TRUE;
	}
	else
	{
		return CSettingsPage::OnSetCursor( pWnd, nHitTest, message );
	}
}

void CRemoteSettingsPage::OnLButtonUp(UINT nFlags, CPoint point)
{
	CRect rect;
	m_wndURL.GetWindowRect( &rect );
	ScreenToClient( &rect );

	if ( rect.PtInRect( point ) && m_wndURL.IsWindowEnabled() )
	{
		CString strURL;
		m_wndURL.GetWindowText( strURL );
		ShellExecute( GetSafeHwnd(), NULL, strURL, NULL, NULL, SW_SHOWNORMAL );
	}

	CSettingsPage::OnLButtonUp( nFlags, point );
}

void CRemoteSettingsPage::OnCancel()
{
	Settings.Remote.Enable		= m_bOldEnable != FALSE;
	Settings.Remote.Username	= m_sOldUsername;
	Settings.Remote.Password	= m_sOldPassword;

	CSettingsPage::OnCancel();
}

void CRemoteSettingsPage::OnSkinChange()
{
	if ( ! IsWindow( GetSafeHwnd() ) )
		// Not created yet page
		return;

	CSettingsPage::OnSkinChange();

	OnBnClickedRemoteEnable();
}
