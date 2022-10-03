//
// WndSettingsPage.cpp
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
#include "Settings.h"
#include "WndSettingsSheet.h"
#include "WndSettingsPage.h"

#include "Skin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CSettingsPage, CDialog)

BEGIN_MESSAGE_MAP(CSettingsPage, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSettingsPage construction

CSettingsPage::CSettingsPage(UINT nIDTemplate, LPCTSTR pszName)
	: CDialog( nIDTemplate )
{
	if ( pszName )
		m_sName = pszName;

	if ( m_lpszTemplateName )
		LoadDefaultCaption();
}

/////////////////////////////////////////////////////////////////////////////
// CSettingsPage operations

BOOL CSettingsPage::LoadDefaultCaption()
{
	CDialogTemplate pTemplate;
	DLGTEMPLATE* pData;
	LPWORD pWord;

	if ( ! pTemplate.Load( m_lpszTemplateName ) )
		return FALSE;

	pData = (DLGTEMPLATE*)GlobalLock( pTemplate.m_hTemplate );
	if ( ! pData )
		return FALSE;

	if ( ((DLGTEMPLATEEX*)pData)->signature == 0xFFFF )
	{
		pWord = (WORD*)( (DLGTEMPLATEEX*)pData + 1 );
	}
	else
	{
		pWord = (WORD*)( pData + 1 );
	}

	if ( *pWord == 0xFFFF )
	{
		pWord += 2;
	}
	else
	{
		while ( *pWord++ );
	}

	if ( *pWord == 0xFFFF )
	{
		pWord += 2;
	}
	else
	{
		while ( *pWord++ );
	}

	m_sCaption = (wchar_t*)pWord;

	GlobalUnlock( pTemplate.m_hTemplate );

	return m_sCaption.GetLength() > 0;
}

BOOL CSettingsPage::Create(const CRect& rcPage, CWnd* pSheetWnd)
{
	ASSERT_VALID(this);
	ASSERT( m_lpszTemplateName != NULL );

	CDialogTemplate pTemplate;
	LPDLGTEMPLATE pData;

	if ( ! pTemplate.Load( m_lpszTemplateName ) )
		return FALSE;

	pData = (LPDLGTEMPLATE)GlobalLock( pTemplate.m_hTemplate );
	if ( ! pData )
		return FALSE;

	DWORD dwExStyle = Settings.General.LanguageRTL ? WS_EX_RTLREADING|WS_EX_RIGHT|WS_EX_LEFTSCROLLBAR|WS_EX_LAYOUTRTL : 
		WS_EX_LTRREADING|WS_EX_LEFT|WS_EX_RIGHTSCROLLBAR;

	if ( ((DLGTEMPLATEEX*)pData)->signature == 0xFFFF )
	{
		DLGTEMPLATEEX* pEx = (DLGTEMPLATEEX*)pData;
		pEx->style		= WS_CHILDWINDOW|WS_OVERLAPPED|DS_3DLOOK|DS_SETFONT|DS_CONTROL;
		pEx->exStyle	= dwExStyle|WS_EX_WINDOWEDGE|WS_EX_CONTROLPARENT;
	}
	else
	{
		pData->style			= WS_CHILDWINDOW|WS_OVERLAPPED|DS_3DLOOK|DS_SETFONT|DS_CONTROL;
		pData->dwExtendedStyle	= dwExStyle|WS_EX_WINDOWEDGE|WS_EX_CONTROLPARENT;
	}

	GlobalUnlock( pTemplate.m_hTemplate );
	CreateIndirect( pTemplate.m_hTemplate, pSheetWnd );
	SetFont( &theApp.m_gdiFont );

	MoveWindow( rcPage );

	return ( m_hWnd != NULL );
}

CSettingsPage* CSettingsPage::GetPage(CRuntimeClass* pClass) const
{
	return GetSheet()->GetPage( pClass );
}

/////////////////////////////////////////////////////////////////////////////
// CSettingsPage message handlers

BOOL CSettingsPage::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_wndToolTip.Create( this );
	m_wndToolTip.Activate( TRUE );
	m_wndToolTip.SetMaxTipWidth( 200 );
	m_wndToolTip.SetDelayTime( TTDT_AUTOPOP, 20 * 1000 );	// Show the tooltip for 20 seconds

	return TRUE;
}

void CSettingsPage::OnSkinChange()
{
	if ( ! IsWindow( GetSafeHwnd() ) )
		// Not created yet page
		return;

	if ( m_sName.IsEmpty() )
		m_sName = GetRuntimeClass()->m_lpszClassName;

	SetWindowText( m_sCaption );

	Skin.Apply( m_sName, this, 0, &m_wndToolTip );
}

void CSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );
}

void CSettingsPage::SetModified(BOOL bChanged)
{
	ASSERT_VALID(this);
	GetSheet()->SetModified( bChanged );
}

BOOL CSettingsPage::OnApply()
{
	ASSERT_VALID(this);
	OnOK();
	return TRUE;
}

void CSettingsPage::OnReset()
{
	ASSERT_VALID(this);
	OnCancel();
}

void CSettingsPage::OnOK()
{
	ASSERT_VALID(this);
}

void CSettingsPage::OnCancel()
{
	ASSERT_VALID(this);
}

BOOL CSettingsPage::OnSetActive()
{
	ASSERT_VALID(this);
	return TRUE;
}

BOOL CSettingsPage::OnKillActive()
{
	ASSERT_VALID(this);
	if ( ! UpdateData() ) return FALSE;
	return TRUE;
}

BOOL CSettingsPage::OnEraseBkgnd(CDC* pDC)
{
	CRect rc;
	GetClientRect( &rc );
	pDC->FillSolidRect( &rc, Skin.m_crDialog );
	return TRUE;
}

HBRUSH CSettingsPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor( pDC, pWnd, nCtlColor );

	if ( nCtlColor == CTLCOLOR_DLG || nCtlColor == CTLCOLOR_STATIC )
	{
		pDC->SetBkColor( Skin.m_crDialog );
		hbr = Skin.m_brDialog;
	}

	return hbr;
}

BOOL CSettingsPage::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MOUSELAST )
	{
		MSG msg;
		CopyMemory( &msg, pMsg, sizeof(MSG) );
		HWND hWndParent = ::GetParent( msg.hwnd );

		while ( hWndParent && hWndParent != m_hWnd )
		{
			msg.hwnd = hWndParent;
			hWndParent = ::GetParent( hWndParent );
		}

		if ( msg.hwnd )
		{
			m_wndToolTip.RelayEvent( &msg );
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

/////////////////////////////////////////////////////////////////////////////
// CComboBoxPath

IMPLEMENT_DYNAMIC(CComboBoxPath, CComboBox)

BEGIN_MESSAGE_MAP(CComboBoxPath, CComboBox)
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

static void TrackPathMenu( const CPoint& point, const CString& sPath )
{
	if ( CMenu* pMenu = Skin.GetMenu( _T( "Path.Menu" ) ) )
	{
		CoolMenu.AddMenu( pMenu, TRUE );
		switch ( pMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD, point.x, point.y, AfxGetMainWnd() ) )
		{
		case ID_PATH_EXPLORE:
			if ( GetFileAttributes( CString( _T( "\\\\?\\" ) ) + sPath ) != INVALID_FILE_ATTRIBUTES )
			{
				ShellExecute( AfxGetMainWnd()->GetSafeHwnd(), NULL, CString( _T( "\\\\?\\" ) ) + sPath, NULL, NULL, SW_SHOWDEFAULT );
			}
			break;

		case ID_PATH_COPY:
			theApp.SetClipboardText( sPath );
			break;
		}
	}
}

void CComboBoxPath::OnRButtonDown(UINT /*nFlags*/, CPoint point)
{
	CString sPath;
	GetWindowText( sPath );
	ClientToScreen( &point );
	TrackPathMenu( point, sPath );
}
