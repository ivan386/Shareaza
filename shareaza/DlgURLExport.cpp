//
// DlgURLExport.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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
#include "Settings.h"
#include "Library.h"
#include "SharedFile.h"
#include "SharedFolder.h"
#include "Transfer.h"
#include "Network.h"
#include "TigerTree.h"
#include "SHA.h"
#include "MD5.h"
#include "ED2K.h"
#include "DlgURLExport.h"
#include "DlgURLCopy.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CURLExportDlg, CSkinDialog)
	//{{AFX_MSG_MAP(CURLExportDlg)
	ON_CBN_CLOSEUP(IDC_URL_TOKEN, OnCloseUpUrlToken)
	ON_CBN_CLOSEUP(IDC_URL_PRESET, OnCloseUpUrlPreset)
	ON_BN_CLICKED(IDC_SAVE, OnSave)
	ON_BN_CLICKED(IDC_COPY, OnCopy)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CURLExportDlg dialog

CURLExportDlg::CURLExportDlg(CWnd* pParent) : CSkinDialog(CURLExportDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CURLExportDlg)
	m_sFormat = _T("");
	//}}AFX_DATA_INIT
}

void CURLExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CURLExportDlg)
	DDX_Control(pDX, IDC_SAVE, m_wndSave);
	DDX_Control(pDX, IDC_COPY, m_wndCopy);
	DDX_Control(pDX, IDC_PROGRESS, m_wndProgress);
	DDX_Control(pDX, IDC_URL_TOKEN, m_wndToken);
	DDX_Control(pDX, IDC_URL_PRESET, m_wndPreset);
	DDX_Control(pDX, IDC_URL_FORMAT, m_wndFormat);
	DDX_Control(pDX, IDC_MESSAGE, m_wndMessage);
	DDX_Text(pDX, IDC_URL_FORMAT, m_sFormat);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CURLExportDlg message handlers

BOOL CURLExportDlg::OnInitDialog() 
{
	CSkinDialog::OnInitDialog();
	
	SkinMe( _T("CURLExportDlg"), IDI_WEB_URL );
	
	CString strFormat, strMessage;
	m_wndMessage.GetWindowText( strFormat );
	strMessage.Format( strFormat, m_pFiles.GetCount() );
	m_wndMessage.SetWindowText( strMessage );
	
	m_sFormat = theApp.GetProfileString( _T("Library"), _T("URLExportFormat"), _T("") );
	
	if ( m_sFormat.IsEmpty() )
		m_sFormat = _T("<a href=\"magnet:?xt=[URN]&dn=[NameURI]\">[Name]</a><br>");
	
	UpdateData( FALSE );
	
	return TRUE;
}

void CURLExportDlg::AddFile(CLibraryFile* pFile)
{
	if ( pFile->m_bSHA1 ) m_pFiles.AddTail( (LPVOID)pFile->m_nIndex );
}

void CURLExportDlg::OnCloseUpUrlToken() 
{
	int nToken = m_wndToken.GetCurSel();
	m_wndToken.SetCurSel( -1 );
	if ( nToken < 0 || nToken > 12 ) return;

	LPCTSTR pszTokens[] =
	{
		_T("[TIGER]"), _T("[SHA1]"), _T("[MD5]"), _T("[ED2K]"),
		_T("[Name]"), _T("[NameURI]"), _T("[FileBase]"),
		_T("[FileExt]"), _T("[Size]"), _T("[ByteSize]"), _T("[Path]"),
		_T("[LocalHost]"), _T("[LocalPort]")
	};
	
	m_wndFormat.ReplaceSel( pszTokens[ nToken ] );
	m_wndFormat.SetFocus();
}

void CURLExportDlg::OnCloseUpUrlPreset() 
{
	int nPreset = m_wndPreset.GetCurSel();
	m_wndPreset.SetCurSel( -1 );
	if ( nPreset < 0 || nPreset > 5 ) return;

	LPCTSTR pszPresets[] =
	{
		_T("magnet:?xt=urn:bitprint:[SHA1].[TIGER]&dn=[NameURI]"),
		_T("gnutella://urn:sha1:[SHA1]/[NameURI]/"),
		_T("ed2k://|file|[NameURI]|[ByteSize]|[ED2K]|/"),
		_T("<a href=\"magnet:?xt=urn:bitprint:[SHA1].[TIGER]&dn=[NameURI]\">[Name]</a><br>"),
		_T("<a href=\"gnutella://urn:sha1:[SHA1]/[NameURI]/\">[Name]</a><br>"),
		_T("<a href=\"ed2k://|file|[NameURI]|[ByteSize]|[ED2K]|/\">[Name]</a>"),
	};

	m_wndFormat.SetWindowText( pszPresets[ nPreset ] );
	m_wndFormat.SetFocus();
}

void CURLExportDlg::OnSave() 
{
	UpdateData();
	
	if ( m_sFormat.IsEmpty() ) return;
	
	theApp.WriteProfileString( _T("Library"), _T("URLExportFormat"), m_sFormat );
	
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
	
	m_wndProgress.SetRange( 0, m_pFiles.GetCount() );
	m_wndCopy.EnableWindow( FALSE );
	m_wndSave.EnableWindow( FALSE );
	
	for ( POSITION pos = m_pFiles.GetHeadPosition() ; pos ; )
	{
		m_wndProgress.OffsetPos( 1 );
		
		DWORD nIndex = (DWORD)m_pFiles.GetNext( pos );
		
		CString strLine = m_sFormat;
		{
			CQuickLock oLock( Library.m_pSection );
			CLibraryFile* pFile = Library.LookupFile( nIndex );
			if ( ! pFile ) continue;
			
			CString strItem;
			
			Replace( strLine, _T("[Name]"), pFile->m_sName );
			Replace( strLine, _T("[NameURI]"), CTransfer::URLEncode( pFile->m_sName ) );
			Replace( strLine, _T("[Size]"), Settings.SmartVolume( pFile->m_nSize, FALSE ) );
			if ( pFile->m_pFolder != NULL ) Replace( strLine, _T("[Path]"), pFile->m_pFolder->m_sPath );
			
			strItem.Format( _T("%I64i"), pFile->m_nSize );
			Replace( strLine, _T("[ByteSize]"), strItem );
			
			strItem = CTigerNode::HashToString( &pFile->m_pTiger );
			Replace( strLine, _T("[TIGER]"), strItem );
			strItem = CSHA::HashToString( &pFile->m_pSHA1 );
			Replace( strLine, _T("[SHA1]"), strItem );
			strItem = CMD5::HashToString( &pFile->m_pMD5 );
			Replace( strLine, _T("[MD5]"), strItem );
			strItem = CED2K::HashToString( &pFile->m_pED2K );
			Replace( strLine, _T("[ED2K]"), strItem );
			
			int nDot = pFile->m_sName.ReverseFind( '.' );
			
			if ( nDot > 0 )
			{
				Replace( strLine, _T("[FileBase]"), pFile->m_sName.Left( nDot ) );
				Replace( strLine, _T("[FileExt]"), pFile->m_sName.Mid( nDot + 1 ) );
			}
			else
			{
				Replace( strLine, _T("[FileBase]"), pFile->m_sName );
				Replace( strLine, _T("[FileExt]"), _T("") );
			}
			
			if ( Network.IsListening() )
			{
				strItem = inet_ntoa( Network.m_pHost.sin_addr );
				Replace( strLine, _T("[LocalHost]"), strItem );
				strItem.Format( _T("%lu"), htons( Network.m_pHost.sin_port ) );
				Replace( strLine, _T("[LocalPort]"), strItem );
			}
			
			strLine += _T("\r\n");
		}
		
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
	
	theApp.WriteProfileString( _T("Library"), _T("URLExportFormat"), m_sFormat );
	
	CWaitCursor pCursor;
	CString strOutput;
	
	m_wndProgress.SetRange( 0, m_pFiles.GetCount() );
	m_wndCopy.EnableWindow( FALSE );
	m_wndSave.EnableWindow( FALSE );
	
	for ( POSITION pos = m_pFiles.GetHeadPosition() ; pos ; )
	{
		m_wndProgress.OffsetPos( 1 );
		
		DWORD nIndex = (DWORD)m_pFiles.GetNext( pos );
		
		CString strLine = m_sFormat;
		{
			CQuickLock oLock( Library.m_pSection );
			CLibraryFile* pFile = Library.LookupFile( nIndex );
			if ( ! pFile ) continue;
			
			CString strItem;
			
			Replace( strLine, _T("[Name]"), pFile->m_sName );
			Replace( strLine, _T("[NameURI]"), CTransfer::URLEncode( pFile->m_sName ) );
			Replace( strLine, _T("[Size]"), Settings.SmartVolume( pFile->m_nSize, FALSE ) );
			if ( pFile->m_pFolder != NULL ) Replace( strLine, _T("[Path]"), pFile->m_pFolder->m_sPath );
			
			strItem.Format( _T("%I64i"), pFile->m_nSize );
			Replace( strLine, _T("[ByteSize]"), strItem );
			
			strItem = CTigerNode::HashToString( &pFile->m_pTiger );
			Replace( strLine, _T("[TIGER]"), strItem );
			strItem = CSHA::HashToString( &pFile->m_pSHA1 );
			Replace( strLine, _T("[SHA1]"), strItem );
			strItem = CMD5::HashToString( &pFile->m_pMD5 );
			Replace( strLine, _T("[MD5]"), strItem );
			strItem = CED2K::HashToString( &pFile->m_pED2K );
			Replace( strLine, _T("[ED2K]"), strItem );
			
			int nDot = pFile->m_sName.ReverseFind( '.' );
			
			if ( nDot > 0 )
			{
				Replace( strLine, _T("[FileBase]"), pFile->m_sName.Left( nDot ) );
				Replace( strLine, _T("[FileExt]"), pFile->m_sName.Mid( nDot + 1 ) );
			}
			else
			{
				Replace( strLine, _T("[FileBase]"), pFile->m_sName );
				Replace( strLine, _T("[FileExt]"), _T("") );
			}
			
			if ( Network.IsListening() )
			{
				strItem = inet_ntoa( Network.m_pHost.sin_addr );
				Replace( strLine, _T("[LocalHost]"), strItem );
				strItem.Format( _T("%lu"), htons( Network.m_pHost.sin_port ) );
				Replace( strLine, _T("[LocalPort]"), strItem );
			}
		}
		
		strLine += _T("\r\n");
		strOutput += strLine;
	}
	
	CURLCopyDlg::SetClipboardText( strOutput );
	
	EndDialog( IDOK );
}
