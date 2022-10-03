//
// DlgURLExport.cpp
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
#include "Library.h"
#include "SharedFile.h"
#include "SharedFolder.h"
#include "Transfer.h"
#include "Network.h"
#include "DlgURLExport.h"
#include "DlgURLCopy.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const LPCTSTR pszPresets[] =
{
	_T("[Magnet]"),
	_T("[LinkED2K]"),
	_T("<a href=\"[MagnetURI]\">[Name]</a><br>"),
	_T("<a href=\"[LinkED2K]\">[Name]</a><br>")
};

static const LPCTSTR pszTokens[] =
{
	_T("[TIGER]"),		_T("[SHA1]"),		_T("[MD5]"),		_T("[ED2K]"),		_T("[BTH]"),
	_T("[Name]"),		_T("[NameURI]"),	_T("[FileBase]"),	_T("[FileExt]"),	_T("[Size]"),
	_T("[ByteSize]"),	_T("[Path]"),		_T("[LocalHost]"),	_T("[LocalPort]"),	_T("[Link]"),
	_T("[LinkURI]"),	_T("[Magnet]"),		_T("[MagnetURI]"),	_T("[LinkED2K]")
};

IMPLEMENT_DYNAMIC(CURLExportDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CURLExportDlg, CSkinDialog)
	ON_CBN_CLOSEUP(IDC_URL_TOKEN, OnCloseUpUrlToken)
	ON_CBN_SELCHANGE(IDC_URL_PRESET, OnSelChangeUrlPreset)
	ON_CBN_KILLFOCUS(IDC_URL_PRESET, OnKillFocusUrlPreset)
	ON_BN_CLICKED(IDC_SAVE, OnSave)
	ON_BN_CLICKED(IDC_COPY, OnCopy)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CURLExportDlg dialog

CURLExportDlg::CURLExportDlg(CWnd* pParent) : CSkinDialog(CURLExportDlg::IDD, pParent)
{
}

void CURLExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_SAVE, m_wndSave);
	DDX_Control(pDX, IDC_COPY, m_wndCopy);
	DDX_Control(pDX, IDC_PROGRESS, m_wndProgress);
	DDX_Control(pDX, IDC_URL_TOKEN, m_wndToken);
	DDX_Control(pDX, IDC_URL_PRESET, m_wndPreset);
	DDX_Control(pDX, IDC_URL_FORMAT, m_wndFormat);
	DDX_Control(pDX, IDC_MESSAGE, m_wndMessage);
	DDX_Text(pDX, IDC_URL_FORMAT, m_sFormat);
}

/////////////////////////////////////////////////////////////////////////////
// CURLExportDlg message handlers

BOOL CURLExportDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	SkinMe( _T("CURLExportDlg"), IDI_WEB_URL );

	if ( Settings.General.LanguageRTL ) m_wndProgress.ModifyStyleEx( WS_EX_LAYOUTRTL, 0, 0 );
	CString strFormat, strMessage;
	m_wndMessage.GetWindowText( strFormat );
	strMessage.Format( strFormat, m_pFiles.GetCount() );
	m_wndMessage.SetWindowText( strMessage );

	m_sFormat = Settings.Library.URLExportFormat;

	UpdateData( FALSE );

	return TRUE;
}

void CURLExportDlg::Add(const CShareazaFile* pFile)
{
	ASSERT( pFile != NULL );
	m_pFiles.AddTail( *pFile );
}

void CURLExportDlg::OnKillFocusUrlPreset()
{
	m_wndPreset.SetCurSel( -1 );
}

void CURLExportDlg::OnCloseUpUrlToken()
{
	int nToken = m_wndToken.GetCurSel();
	m_wndToken.SetCurSel( -1 );
	if ( nToken < 0 || nToken >= _countof( pszTokens ) ) return;

	m_wndFormat.ReplaceSel( pszTokens[ nToken ] );
	m_wndFormat.SetFocus();
}

void CURLExportDlg::OnSelChangeUrlPreset()
{
	int nPreset = m_wndPreset.GetCurSel();
	if ( nPreset < 0 || nPreset >= _countof( pszPresets ) ) return;

	m_wndFormat.SetWindowText( pszPresets[ nPreset ] );
}

void CURLExportDlg::OnSave()
{
	UpdateData();

	if ( m_sFormat.IsEmpty() ) return;

	Settings.Library.URLExportFormat = m_sFormat;

	LPCTSTR pszExt = ( m_sFormat.Find( '<' ) >= 0 ) ? _T("htm") : _T("txt");
	LPCTSTR pszFilter = ( m_sFormat.Find( '<' ) >= 0 ) ?
		_T("HTML Files|*.htm;*.html|Text Files|*.txt|All Files|*.*||") :
		_T("Text Files|*.txt|HTML Files|*.htm;*.html|All Files|*.*||");

	CFileDialog dlg( FALSE, pszExt, NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		pszFilter, this );

	if ( dlg.DoModal() != IDOK ) return;

	CFile pOutput;

	if ( ! pOutput.Open( dlg.GetPathName(), CFile::modeWrite|CFile::modeCreate ) )
		return;

	CWaitCursor pCursor;

	m_wndProgress.SetRange( 0, short( m_pFiles.GetCount() ) );
	m_wndCopy.EnableWindow( FALSE );
	m_wndSave.EnableWindow( FALSE );

	for ( POSITION pos = m_pFiles.GetHeadPosition() ; pos ; )
	{
		m_wndProgress.OffsetPos( 1 );

		CString strLine = m_sFormat;

		MakeURL( m_pFiles.GetNext( pos ), strLine );

		int nBytes = WideCharToMultiByte( CP_ACP, 0, strLine, strLine.GetLength(), NULL, 0, NULL, NULL );
		LPSTR pBytes = new CHAR[nBytes];
		WideCharToMultiByte( CP_ACP, 0, strLine, strLine.GetLength(), pBytes, nBytes, NULL, NULL );
		pOutput.Write( pBytes, nBytes );
		delete [] pBytes;
	}

	pOutput.Close();

	EndDialog( IDOK );
}

void CURLExportDlg::OnCopy()
{
	UpdateData();

	if ( m_sFormat.IsEmpty() ) return;

	Settings.Library.URLExportFormat = m_sFormat;

	CWaitCursor pCursor;
	CString strOutput;

	m_wndProgress.SetRange( 0, short( m_pFiles.GetCount() ) );
	m_wndCopy.EnableWindow( FALSE );
	m_wndSave.EnableWindow( FALSE );

	for ( POSITION pos = m_pFiles.GetHeadPosition() ; pos ; )
	{
		m_wndProgress.OffsetPos( 1 );

		CString strLine = m_sFormat;

		MakeURL( m_pFiles.GetNext( pos ), strLine );

		strOutput += strLine;
	}

	theApp.SetClipboardText( strOutput );

	EndDialog( IDOK );
}

void CURLExportDlg::MakeURL(CShareazaFile pFile, CString& strLine)
{
	CString sMagnet = CURLCopyDlg::CreateMagnet( pFile );
	strLine.Replace( pszTokens[ 16 ], sMagnet );

	CString sMagnetURI = sMagnet;
	sMagnetURI.Replace( _T("&"), _T("&amp;") );
	strLine.Replace( pszTokens[ 17 ], sMagnetURI );

	CString sED2K;
	if ( pFile.m_oED2K &&
		pFile.m_nSize != 0 && pFile.m_nSize != SIZE_UNKNOWN &&
		pFile.m_sName.GetLength() )
	{
		sED2K.Format( _T("ed2k://|file|%s|%I64u|%s|/"),
			(LPCTSTR)URLEncode( pFile.m_sName ),
			pFile.m_nSize,
			(LPCTSTR)pFile.m_oED2K.toString() );
	}
	strLine.Replace( pszTokens[ 18 ], sED2K );

	strLine.Replace( pszTokens[ 5 ], pFile.m_sName );
	strLine.Replace( pszTokens[ 6 ], URLEncode( pFile.m_sName ) );
	
	strLine.Replace( pszTokens[ 14 ], pFile.m_sURL );
	strLine.Replace( pszTokens[ 15 ], URLEncode( pFile.m_sURL ) );

	strLine.Replace( pszTokens[ 11 ], pFile.m_sPath );

	if ( pFile.m_nSize != 0 && pFile.m_nSize != SIZE_UNKNOWN )
	{
		CString strItem;
		strLine.Replace( pszTokens[ 9 ], Settings.SmartVolume( pFile.m_nSize ) );
		strItem.Format( _T("%I64u"), pFile.m_nSize );
		strLine.Replace( pszTokens[ 10 ], strItem );
	}
	else
	{
		strLine.Replace( pszTokens[ 9 ], _T("") );
		strLine.Replace( pszTokens[ 10 ], _T("") );
	}

	strLine.Replace( pszTokens[ 0 ], pFile.m_oTiger.toString() );
	strLine.Replace( pszTokens[ 1 ], pFile.m_oSHA1.toString() );
	strLine.Replace( pszTokens[ 2 ], pFile.m_oMD5.toString() );
	strLine.Replace( pszTokens[ 3 ], pFile.m_oED2K.toString() );
	strLine.Replace( pszTokens[ 4 ], pFile.m_oBTH.toString() );

	int nDot = pFile.m_sName.ReverseFind( _T('.') );
	if ( nDot > 0 )
	{
		strLine.Replace( pszTokens[ 7 ], pFile.m_sName.Left( nDot ) );
		strLine.Replace( pszTokens[ 8 ], pFile.m_sName.Mid( nDot + 1 ) );
	}
	else
	{
		strLine.Replace( pszTokens[ 7 ], pFile.m_sName );
		strLine.Replace( pszTokens[ 8 ], _T("") );
	}

	if ( Network.IsListening() )
	{
		CString strItem = CString( inet_ntoa( Network.m_pHost.sin_addr ) );
		strLine.Replace( pszTokens[ 12 ], strItem );
		strItem.Format( _T("%lu"), htons( Network.m_pHost.sin_port ) );
		strLine.Replace( pszTokens[ 13 ], strItem );
	}

	strLine += _T("\r\n");
}
