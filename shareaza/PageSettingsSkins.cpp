//
// PageSettingsSkins.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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
#include "Skin.h"
#include "XML.h"
#include "LiveList.h"
#include "Skin.h"
#include "PageSettingsSkins.h"
#include "CoolInterface.h"
#include "SchemaCache.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CSkinsSettingsPage, CSettingsPage)

BEGIN_MESSAGE_MAP(CSkinsSettingsPage, CSettingsPage)
	//{{AFX_MSG_MAP(CSkinsSettingsPage)
	ON_WM_LBUTTONUP()
	ON_WM_SETCURSOR()
	ON_WM_CTLCOLOR()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_SKINS, OnItemChangedSkins)
	ON_BN_CLICKED(IDC_SKINS_BROWSE, OnSkinsBrowse)
	ON_BN_CLICKED(IDC_SKINS_WEB, OnSkinsWeb)
	ON_BN_CLICKED(IDC_SKINS_DELETE, OnSkinsDelete)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSkinsSettingsPage property page

CSkinsSettingsPage::CSkinsSettingsPage() : CSettingsPage( CSkinsSettingsPage::IDD )
{
	//{{AFX_DATA_INIT(CSkinsSettingsPage)
	//}}AFX_DATA_INIT
}

CSkinsSettingsPage::~CSkinsSettingsPage()
{
}

void CSkinsSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSkinsSettingsPage)
	DDX_Control(pDX, IDC_SKINS_DELETE, m_wndDelete);
	DDX_Control(pDX, IDC_SKIN_DESC, m_wndDesc);
	DDX_Control(pDX, IDC_SKIN_NAME, m_wndName);
	DDX_Control(pDX, IDC_SKIN_AUTHOR, m_wndAuthor);
	DDX_Control(pDX, IDC_SKINS, m_wndList);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CSkinsSettingsPage message handlers

BOOL CSkinsSettingsPage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	m_gdiImageList.Create( 16, 16, ILC_COLOR32|ILC_MASK, 1, 1 ) ||
	m_gdiImageList.Create( 16, 16, ILC_COLOR24|ILC_MASK, 1, 1 ) ||
	m_gdiImageList.Create( 16, 16, ILC_COLOR16|ILC_MASK, 1, 1 );
	AddIcon( IDI_SKIN, m_gdiImageList );

	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );
	m_wndList.InsertColumn( 0, _T("Name"), LVCFMT_LEFT, 210, 0 );
	m_wndList.InsertColumn( 1, _T("Author"), LVCFMT_LEFT, 130, 1 );
	m_wndList.InsertColumn( 2, _T("Version"), LVCFMT_LEFT, 42, 2 );
	m_wndList.InsertColumn( 3, _T("Path"), LVCFMT_LEFT, 0, 3 );
	m_wndList.InsertColumn( 4, _T("URL"), LVCFMT_LEFT, 0, 4 );
	m_wndList.InsertColumn( 5, _T("Email"), LVCFMT_LEFT, 0, 5 );
	m_wndList.InsertColumn( 6, _T("Description"), LVCFMT_LEFT, 0, 6 );

	m_wndList.SendMessage( LVM_SETEXTENDEDLISTVIEWSTYLE,
		LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES, LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES );

	if ( Settings.General.LanguageRTL ) 
		m_wndDesc.ModifyStyleEx( WS_EX_RTLREADING|WS_EX_RIGHT|WS_EX_LEFTSCROLLBAR, 
			WS_EX_LTRREADING|WS_EX_LEFT|WS_EX_RIGHTSCROLLBAR, 0 );

	m_nSelected = -1;
	m_wndName.SetWindowText( _T("") );
	m_wndAuthor.SetWindowText( _T("") );
	m_wndDelete.EnableWindow( FALSE );

	CWaitCursor pCursor;

	EnumerateSkins();

	return TRUE;
}

void CSkinsSettingsPage::EnumerateSkins(LPCTSTR pszPath)
{
	WIN32_FIND_DATA pFind;
	CString strPath;
	HANDLE hSearch;

	strPath.Format( _T("%s\\Skins\\%s*.*"),
		(LPCTSTR)Settings.General.Path, pszPath ? pszPath : _T("") );

	hSearch = FindFirstFile( strPath, &pFind );

	if ( hSearch != INVALID_HANDLE_VALUE )
	{
		do
		{
			if ( pFind.cFileName[0] == '.' ) continue;

			if ( pFind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				strPath.Format( _T("%s%s\\"),
					pszPath ? pszPath : _T(""), pFind.cFileName );

				EnumerateSkins( strPath );
			}
			else if (	_tcsistr( pFind.cFileName, _T(".xml") ) != NULL &&
						_tcsicmp( pFind.cFileName, _T("Definitions.xml") ) != 0 &&
						_tcsnicmp( pFind.cFileName, _T("Default-"), 8 ) != 0 )
			{
				AddSkin( pszPath, pFind.cFileName );
			}
		}
		while ( FindNextFile( hSearch, &pFind ) );

		FindClose( hSearch );
	}
}

BOOL CSkinsSettingsPage::AddSkin(LPCTSTR pszPath, LPCTSTR pszName)
{
	CString strXML;
	CFile pFile;

	strXML = Settings.General.Path + _T("\\Skins\\");
	if ( pszPath ) strXML += pszPath;
	strXML += pszName;

    if ( ! pFile.Open( strXML, CFile::modeRead ) ) return FALSE;

	DWORD nSource = (DWORD)pFile.GetLength();
	if ( nSource > 4096*1024 ) return FALSE;

	CHAR* pSource = new CHAR[ nSource ];
	pFile.Read( pSource, nSource );
	pFile.Close();

	BYTE* pByte = (BYTE*)pSource;
	DWORD nByte = nSource;

	if ( nByte >= 2 && ( ( pByte[0] == 0xFE && pByte[1] == 0xFF ) || ( pByte[0] == 0xFF && pByte[1] == 0xFE ) ) )
	{
		nByte = nByte / 2 - 1;

		if ( pByte[0] == 0xFE && pByte[1] == 0xFF )
		{
			pByte += 2;

			for ( DWORD nSwap = 0 ; nSwap < nByte ; nSwap ++ )
			{
				register CHAR nTemp = pByte[ ( nSwap << 1 ) + 0 ];
				pByte[ ( nSwap << 1 ) + 0 ] = pByte[ ( nSwap << 1 ) + 1 ];
				pByte[ ( nSwap << 1 ) + 1 ] = nTemp;
			}
		}
		else
		{
			pByte += 2;
		}

		CopyMemory( strXML.GetBuffer( nByte ), pByte, nByte * sizeof( TCHAR ) );
		strXML.ReleaseBuffer( nByte );
	}
	else
	{
		if ( nByte >= 3 && pByte[0] == 0xEF && pByte[1] == 0xBB && pByte[2] == 0xBF )
		{
			pByte += 3; nByte -= 3;
		}

		strXML = UTF8Decode( (LPCSTR)pByte, nByte );
	}

	delete [] pSource;

	CXMLElement* pXML = NULL;

	int nManifest = strXML.Find( _T("<manifest") );

	if ( nManifest > 0 )
	{
		CString strManifest = strXML.Mid( nManifest ).SpanExcluding( _T(">") ) + '>';

		if ( CXMLElement* pManifest = CXMLElement::FromString( strManifest ) )
		{
			pXML = new CXMLElement( NULL, _T("skin") );
			pXML->AddElement( pManifest );
		}
	}

	if ( pXML == NULL )
	{
		pXML = CXMLElement::FromString( strXML, TRUE );
		if ( pXML == NULL ) return FALSE;
	}

	strXML.Empty();

	CXMLElement* pManifest = pXML->GetElementByName( _T("manifest") );

	if ( ! pXML->IsNamed( _T("skin") ) || pManifest == NULL ||
		 ! pManifest->GetAttributeValue( _T("type") ).CompareNoCase( _T("language") ) )
	{
		delete pXML;
		return FALSE;
	}

//	CString	strType		= pManifest->GetAttributeValue( _T("type"), _T("Unknown") );
	CString strIcon		= pManifest->GetAttributeValue( _T("icon") );
	CString	strName		= pManifest->GetAttributeValue( _T("name"), pszName );
	CString strAuthor	= pManifest->GetAttributeValue( _T("author"), _T("Unknown") );
	CString strVersion	= pManifest->GetAttributeValue( _T("Version"), _T("Unknown") );
	CString strURL		= pManifest->GetAttributeValue( _T("link") );
	CString strEmail	= pManifest->GetAttributeValue( _T("email") );
	CString strDesc		= pManifest->GetAttributeValue( _T("description") );

	if ( Settings.General.LanguageRTL ) 
	{
		strName = _T("\x202A") + strName;
		strAuthor = _T("\x202A") + strAuthor;
	}

	delete pXML;

	if ( strIcon.GetLength() )
	{
		if ( pszPath != NULL )
			strIcon = Settings.General.Path + _T("\\Skins\\") + pszPath + strIcon;
		else
			strIcon = Settings.General.Path + _T("\\Skins\\") + strIcon;
	}
	else
	{
		if ( pszPath != NULL )
			strIcon = Settings.General.Path + _T("\\Skins\\") + pszPath + strIcon + pszName;
		else
			strIcon = Settings.General.Path + _T("\\Skins\\") + strIcon + pszName;

		strIcon = strIcon.Left( strIcon.GetLength() - 3 ) + _T("ico");
	}

	if ( strURL.Find( _T("http://") ) == 0 )
	{
	}
	else if ( strURL.Find( _T("www.") ) == 0 )
	{
		strURL = _T("http://") + strURL;
	}
	else
	{
		strURL.Empty();
	}

	if ( strEmail.Find( '@' ) < 0 ) strEmail.Empty();

	CLiveItem pItem( 7, 0 );
	HICON hIcon = NULL;
	ExtractIconEx( strIcon, 0, NULL, &hIcon, 1 );
	if ( hIcon )
	{
		pItem.SetImage( 0, m_gdiImageList.Add( hIcon ));
		DestroyIcon( hIcon );
	}
	else
	{
		pItem.SetImage( 0, 0 );
	}

	pItem.Set( 0, strName );
	pItem.Set( 1, strAuthor );
	pItem.Set( 2, strVersion );
	pItem.Set( 4, strURL );
	pItem.Set( 5, strEmail );
	pItem.Set( 6, strDesc );

	strName.Format( _T("%s%s"), pszPath ? pszPath : _T(""), pszName );
	pItem.Set( 3, strName );

	int nItem = pItem.Add( &m_wndList, -1, 7 );

	if ( theApp.GetProfileInt( _T("Skins"), strName, FALSE ) )
	{
		m_wndList.SetItemState( nItem, 2 << 12, LVIS_STATEIMAGEMASK );
	}
	else
	{
		m_wndList.SetItemState( nItem, 1 << 12, LVIS_STATEIMAGEMASK );
	}

	return TRUE;
}

void CSkinsSettingsPage::OnItemChangedSkins(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
//	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	*pResult = 0;

	m_nSelected = m_wndList.GetNextItem( -1, LVNI_SELECTED );

	if ( m_nSelected >= 0 )
	{
		m_wndName.SetWindowText( m_wndList.GetItemText( m_nSelected, 0 ) );
		m_wndAuthor.SetWindowText( m_wndList.GetItemText( m_nSelected, 1 ) );
		m_wndDesc.SetWindowText( m_wndList.GetItemText( m_nSelected, 6 ) );
		m_wndDelete.EnableWindow( TRUE );
	}
	else
	{
		m_wndName.SetWindowText( _T("") );
		m_wndAuthor.SetWindowText( _T("") );
		m_wndDesc.SetWindowText( _T("") );
		m_wndDelete.EnableWindow( FALSE );
	}
}

HBRUSH CSkinsSettingsPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CSettingsPage::OnCtlColor( pDC, pWnd, nCtlColor );

	if ( m_nSelected >= 0 )
	{
		if ( pWnd == &m_wndName )
		{
			if ( m_wndList.GetItemText( m_nSelected, 4 ).GetLength() )
			{
				pDC->SetTextColor( CoolInterface.m_crTextLink );
				pDC->SelectObject( &theApp.m_gdiFontLine );
			}
		}
		else if ( pWnd == &m_wndAuthor )
		{
			if ( m_wndList.GetItemText( m_nSelected, 5 ).GetLength() )
			{
				pDC->SetTextColor( CoolInterface.m_crTextLink );
				pDC->SelectObject( &theApp.m_gdiFontLine );
			}
		}
	}

	return hbr;
}

BOOL CSkinsSettingsPage::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if ( m_nSelected >= 0 )
	{
		CPoint point;
		CRect rc;

		GetCursorPos( &point );
		m_wndName.GetWindowRect( &rc );

		if ( rc.PtInRect( point ) )
		{
			if ( m_wndList.GetItemText( m_nSelected, 4 ).GetLength() )
			{
				SetCursor( theApp.LoadCursor( IDC_HAND ) );
				return TRUE;
			}
		}

		m_wndAuthor.GetWindowRect( &rc );

		if ( rc.PtInRect( point ) )
		{
			if ( m_wndList.GetItemText( m_nSelected, 5 ).GetLength() )
			{
				SetCursor( theApp.LoadCursor( IDC_HAND ) );
				return TRUE;
			}
		}
	}

	return CSettingsPage::OnSetCursor( pWnd, nHitTest, message );
}

void CSkinsSettingsPage::OnLButtonUp(UINT /*nFlags*/, CPoint point)
{
	CRect rc;

	if ( m_nSelected < 0 ) return;

	ClientToScreen( &point );
	m_wndName.GetWindowRect( &rc );

	if ( rc.PtInRect( point ) )
	{
		CString strURL = m_wndList.GetItemText( m_nSelected, 4 );

		if ( strURL.GetLength() )
		{
			ShellExecute( GetSafeHwnd(), _T("open"), strURL,
				NULL, NULL, SW_SHOWNORMAL );
		}
		return;
	}

	m_wndAuthor.GetWindowRect( &rc );

	if ( rc.PtInRect( point ) )
	{
		CString strEmail = m_wndList.GetItemText( m_nSelected, 5 );

		if ( strEmail.GetLength() )
		{
			ShellExecute( GetSafeHwnd(), _T("open"), _T("mailto:") + strEmail,
				NULL, NULL, SW_SHOWNORMAL );
		}
		return;
	}
}

void CSkinsSettingsPage::OnSkinsBrowse()
{
	CFileDialog dlg( TRUE, _T("sks"), _T("*.sks"), OFN_HIDEREADONLY,
		_T("Skin Packages|*.sks|") +
		SchemaCache.GetFilter( CSchema::uriAllFiles ) +
		_T("|"), this );

	if ( dlg.DoModal() != IDOK ) return;

	CString strFile = dlg.GetPathName();

	ShellExecute( GetSafeHwnd(), _T("open"), strFile, NULL, NULL, SW_SHOWNORMAL );
}

void CSkinsSettingsPage::OnSkinsWeb()
{
	const CString strWebSite(WEB_SITE_T);

	ShellExecute( GetSafeHwnd(), _T("open"),
		strWebSite + _T("?id=addon&Version=") + theApp.m_sVersion,
		NULL, NULL, SW_SHOWNORMAL );
}

void CSkinsSettingsPage::OnOK()
{
	BOOL bChanged = FALSE;

	for ( int nItem = 0 ; nItem < m_wndList.GetItemCount() ; nItem++ )
	{
		CString strSkin = m_wndList.GetItemText( nItem, 3 );

		BOOL bOn = m_wndList.GetItemState( nItem, LVIS_STATEIMAGEMASK ) == ( 2 << 12 );

		if ( theApp.GetProfileInt( _T("Skins"), strSkin, FALSE ) != (UINT)bOn )
		{
			bChanged = TRUE;
		}

		theApp.WriteProfileInt( _T("Skins"), strSkin, bOn );
	}

	if ( bChanged ) PostMainWndMessage( WM_SKINCHANGED );

	CSettingsPage::OnOK();
}

void CSkinsSettingsPage::OnSkinsDelete()
{
	if ( m_nSelected < 0 ) return;

	CString strName = m_wndList.GetItemText( m_nSelected, 0 );
	CString strBase = m_wndList.GetItemText( m_nSelected, 3 );

	CString strFormat, strPrompt;

	Skin.LoadString( strFormat, IDS_SKIN_DELETE );
	strPrompt.Format( strFormat, (LPCTSTR)strName );

	if ( AfxMessageBox( strPrompt, MB_ICONQUESTION|MB_OKCANCEL|MB_DEFBUTTON2 ) != IDOK ) return;

	theApp.WriteProfileString( _T("Skins"), strBase, NULL );

	CString strPath;
	strPath.Format( _T("%s\\Skins\\%s"),
		(LPCTSTR)Settings.General.Path, (LPCTSTR)strBase );

	DeleteFileEx( strPath, FALSE, TRUE, TRUE );

	int nSlash = strPath.ReverseFind( '\\' );
	strPath = strPath.Left( nSlash ) + _T("\\*.xml");

	WIN32_FIND_DATA pFind;
	HANDLE hSearch = FindFirstFile( strPath, &pFind );

	if ( hSearch != INVALID_HANDLE_VALUE )
	{
		FindClose( hSearch );
	}
	else
	{
		strPath = strPath.Left( strPath.GetLength() - 3 ) + _T("*");
		hSearch = FindFirstFile( strPath, &pFind );

		if ( hSearch != INVALID_HANDLE_VALUE )
		{
			strPath = strPath.Left( strPath.GetLength() - 3 );

			do
			{
				if ( pFind.cFileName[0] == '.' ) continue;
				DeleteFileEx( strPath + pFind.cFileName, FALSE, TRUE, TRUE );
			}
			while ( FindNextFile( hSearch, &pFind ) );

			FindClose( hSearch );
		}

		strPath = strPath.Left( strPath.GetLength() - 1 );
		RemoveDirectory( strPath );
	}

	m_wndList.DeleteItem( m_nSelected );
	m_wndName.SetWindowText( _T("") );
	m_wndAuthor.SetWindowText( _T("") );
	m_wndDesc.SetWindowText( _T("") );
	m_wndDelete.EnableWindow( FALSE );
	m_nSelected = -1;
}


