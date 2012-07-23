//
// PageOutput.cpp
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
#include "PageOutput.h"
#include "PageWelcome.h"
#include "PageSingle.h"
#include "PagePackage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(COutputPage, CWizardPage)

BEGIN_MESSAGE_MAP(COutputPage, CWizardPage)
	ON_BN_CLICKED(IDC_CLEAR_FOLDERS, OnClearFolders)
	ON_BN_CLICKED(IDC_BROWSE_FOLDER, OnBrowseFolder)
	ON_BN_CLICKED(IDC_AUTO_PIECE_SIZE, OnClickedAutoPieceSize)
	ON_CBN_CLOSEUP(IDC_PIECE_SIZE, OnCloseupPieceSize)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COutputPage property page

COutputPage::COutputPage()
	: CWizardPage(COutputPage::IDD, _T("save"))
	, m_bAutoPieces( TRUE )
	, m_nPieceIndex( 0 )
	, m_bSHA1( TRUE )
	, m_bED2K( TRUE )
	, m_bMD5( TRUE )
{
}

void COutputPage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_TORRENT_NAME, m_wndName);
	DDX_Control(pDX, IDC_FOLDER, m_wndFolders);
	DDX_CBString(pDX, IDC_FOLDER, m_sFolder);
	DDX_Text(pDX, IDC_TORRENT_NAME, m_sName);
	DDX_Check(pDX, IDC_AUTO_PIECE_SIZE, m_bAutoPieces);
	DDX_CBIndex(pDX, IDC_PIECE_SIZE, m_nPieceIndex);
	DDX_Control(pDX, IDC_PIECE_SIZE, m_wndPieceSize);
	DDX_Check(pDX, IDC_CHECK_SHA1, m_bSHA1);
	DDX_Check(pDX, IDC_CHECK_ED2K, m_bED2K);
	DDX_Check(pDX, IDC_CHECK_MD5, m_bMD5);
}

/////////////////////////////////////////////////////////////////////////////
// COutputPage message handlers

BOOL COutputPage::OnInitDialog() 
{
	CWizardPage::OnInitDialog();
	
	int nCount = theApp.GetProfileInt( _T("Folders"), _T("Count"), 0 );
	m_bAutoPieces = theApp.GetProfileInt( _T("Folders"), _T("AutoPieceSize"), TRUE );
	m_nPieceIndex = theApp.GetProfileInt( _T("Folders"), _T("PieceSize"), 0 );
	m_bSHA1 = theApp.GetProfileInt( _T("Folders"), _T("SHA1"), TRUE );
	m_bED2K = theApp.GetProfileInt( _T("Folders"), _T("ED2K"), TRUE );
	m_bMD5 = theApp.GetProfileInt( _T("Folders"), _T("MD5"), TRUE );

	for ( int nItem = 0 ; nItem < nCount ; nItem++ )
	{
		CString strName, strURL;
		strName.Format( _T("%.3i.Path"), nItem + 1 );
		strURL = theApp.GetProfileString( _T("Folders"), strName );
		if ( strURL.GetLength() ) 
			m_wndFolders.AddString( strURL );
	}
	
	OnReset();
	
	return TRUE;
}

void COutputPage::OnReset() 
{
	m_sName.Empty();
	m_sFolder.Empty();
	m_wndPieceSize.EnableWindow( !m_bAutoPieces );
	UpdateData( FALSE );
}

BOOL COutputPage::OnSetActive() 
{
	SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );

	GET_PAGE( CWelcomePage, pWelcome );

	if ( pWelcome->m_nType == 0 )
	{
		GET_PAGE( CSinglePage, pSingle );
		
		CString strFile = pSingle->m_sFileName;
		
		if ( LPCTSTR pszSlash = _tcsrchr( strFile, '\\' ) )
		{
			m_sName = pszSlash + 1;
			m_sName += _T(".torrent");
			
			if ( m_sFolder.IsEmpty() ) 
				m_sFolder = strFile.Left( (int)( pszSlash - strFile ) );
		}
	}
	else
	{
		GET_PAGE( CPackagePage, pPackage );

		CString sName = pPackage->m_wndList.GetItemText( 0, 0 );
		
		// Get same part of first and last files
		int nCount = pPackage->m_wndList.GetItemCount();
		if ( nCount > 1 )
		{
			CString sName2 = pPackage->m_wndList.GetItemText( nCount - 1, 0 );
			LPCTSTR pszName1 = sName;
			LPCTSTR pszName2 = sName2;
			for ( int i = 0; *pszName1 && *pszName2; ++pszName1, ++pszName2, ++i )
			{
				if ( *pszName1 != *pszName2 )
				{
					sName = sName.Left( i + 1 );
					break;
				}
			}
		}

		// Use parent folder name as torrent name
		int nSlash = sName.ReverseFind( _T('\\') );
		if ( nSlash != -1 )
		{
			sName = sName.Left( nSlash );
			nSlash = sName.ReverseFind( _T('\\') );
			if ( nSlash != -1 )
			{
				m_sName = sName.Mid( nSlash + 1 ) + _T(".torrent");
			}
		}

		if ( m_sFolder.IsEmpty() ) 
			m_sFolder = theApp.GetProfileString( _T("Folders"), _T("Last") );
		if ( ! m_sFolder.IsEmpty() && m_sName.IsEmpty() )
		{
			m_sName = PathFindFileName( m_sFolder );
			m_sName += _T(".torrent");
		}
	}

	if ( ! theApp.m_sCommandLineDestination.IsEmpty() )
	{
		m_sFolder = theApp.m_sCommandLineDestination;
		theApp.m_sCommandLineDestination.Empty();

		Next();
	}

	UpdateData( FALSE );
		
	return CWizardPage::OnSetActive();
}

void COutputPage::OnBrowseFolder() 
{
	TCHAR szPath[MAX_PATH];
	LPITEMIDLIST pPath;
	LPMALLOC pMalloc;
	BROWSEINFO pBI;
	
	ZeroMemory( &pBI, sizeof(pBI) );
	pBI.hwndOwner		= GetSafeHwnd();
	pBI.pszDisplayName	= szPath;
	pBI.lpszTitle		= _T("Select folder:");
	pBI.ulFlags			= BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	
	pPath = SHBrowseForFolder( &pBI );
	if ( pPath == NULL ) return;
	
	SHGetPathFromIDList( pPath, szPath );
	SHGetMalloc( &pMalloc );
	pMalloc->Free( pPath );
	pMalloc->Release();
	
	UpdateData( TRUE );
	m_sFolder = szPath;
	UpdateData( FALSE );
}

void COutputPage::OnClearFolders() 
{
	theApp.WriteProfileInt( _T("Folders"), _T("Count"), 0 );
	theApp.WriteProfileInt( _T("Folders"), _T("AutoPieceSize"), m_bAutoPieces );
	theApp.WriteProfileInt( _T("Folders"), _T("SHA1"), m_bSHA1 );
	theApp.WriteProfileInt( _T("Folders"), _T("ED2K"), m_bED2K );
	theApp.WriteProfileInt( _T("Folders"), _T("MD5"), m_bMD5 );
	theApp.WriteProfileInt( _T("Folders"), _T("PieceSize"), m_nPieceIndex );
	m_sFolder.Empty();
	UpdateData( FALSE );
	m_wndFolders.ResetContent();
	m_wndFolders.SetFocus();
}

LRESULT COutputPage::OnWizardBack() 
{
	return IDD_COMMENT_PAGE;
}

LRESULT COutputPage::OnWizardNext() 
{
	UpdateData();
	
	if ( m_sFolder.IsEmpty() )
	{
		AfxMessageBox( IDS_OUTPUT_NEED_FOLDER, MB_ICONEXCLAMATION );
		m_wndFolders.SetFocus();
		return -1;
	}
	
	if ( GetFileAttributes( CString( _T("\\\\?\\") ) + m_sFolder ) == 0xFFFFFFFF )
	{
		CString strFormat, strMessage;
		
		strFormat.LoadString( IDS_OUTPUT_CREATE_FOLDER );
		strMessage.Format( strFormat, (LPCTSTR)m_sFolder );

		if ( IDYES != AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) )
			return -1;
		
		if ( ! CreateDirectory( CString( _T("\\\\?\\") ) + m_sFolder, NULL ) )
		{
			strFormat.LoadString( IDS_OUTPUT_CANT_CREATE_FOLDER );
			strMessage.Format( strFormat, (LPCTSTR)m_sFolder );
			
			AfxMessageBox( IDS_OUTPUT_CANT_CREATE_FOLDER, MB_ICONEXCLAMATION );
			m_wndFolders.SetFocus();
			return -1;
		}
	}
	
	if ( m_sName.IsEmpty() )
	{
		AfxMessageBox( IDS_OUTPUT_NEED_FILE, MB_ICONEXCLAMATION );
		m_wndName.SetFocus();
		return -1;
	}

	if ( _tcsicmp( PathFindExtension( m_sName ), _T(".torrent") ) != 0 )
	{
		UINT nResp = AfxMessageBox( IDS_OUTPUT_EXTENSION, MB_ICONQUESTION|MB_YESNOCANCEL );
		
		if ( nResp == IDYES )
		{
			m_sName += _T(".torrent");
			UpdateData( FALSE );
		}
		else if ( nResp != IDNO )
		{
			m_wndName.SetFocus();
			return -1;
		}
	}
	
	CString strPath = m_sFolder + '\\' + m_sName;
	
	if ( GetFileAttributes( CString( _T("\\\\?\\") ) + strPath ) != INVALID_FILE_ATTRIBUTES )
	{
		CString strFormat, strMessage;
		
		strFormat.LoadString( IDS_OUTPUT_REPLACE_FILE );
		strMessage.Format( strFormat, (LPCTSTR)strPath );
		
		if ( IDYES != AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO ) )
			return -1;
		
		DeleteFile( CString( _T("\\\\?\\") ) + strPath );
	}
	
	if ( m_wndFolders.FindStringExact( -1, m_sFolder ) < 0 )
	{
		m_wndFolders.AddString( m_sFolder );
		
		CString strName;
		int nCount = theApp.GetProfileInt( _T("Folders"), _T("Count"), 0 );
		strName.Format( _T("%.3i.Path"), ++nCount );
		theApp.WriteProfileInt( _T("Folders"), _T("Count"), nCount );
		theApp.WriteProfileString( _T("Folders"), strName, m_sFolder );
	}
	
	theApp.WriteProfileString( _T("Folders"), _T("Last"), m_sFolder );
	theApp.WriteProfileInt( _T("Folders"), _T("AutoPieceSize"), m_bAutoPieces );
	theApp.WriteProfileInt( _T("Folders"), _T("SHA1"), m_bSHA1 );
	theApp.WriteProfileInt( _T("Folders"), _T("ED2K"), m_bED2K );
	theApp.WriteProfileInt( _T("Folders"), _T("MD5"), m_bMD5 );
	theApp.WriteProfileInt( _T("Folders"), _T("PieceSize"), m_nPieceIndex );

	return IDD_FINISHED_PAGE;
}

void COutputPage::OnClickedAutoPieceSize()
{
	UpdateData( TRUE );
	m_wndPieceSize.EnableWindow( !m_bAutoPieces );	
}

void COutputPage::OnCloseupPieceSize()
{
	UpdateData( TRUE );
}
