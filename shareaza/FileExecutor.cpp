//
// FileExecutor.cpp
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
#include "FileExecutor.h"
#include "Plugins.h"
#include "Skin.h"

#include "Library.h"
#include "SharedFile.h"
#include "SHA.h"
#include "TigerTree.h"
#include "Connection.h"

#include "WindowManager.h"
#include "WndMain.h"
#include "WndMedia.h"
#include "WndLibrary.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CFileExecutor utilities

void CFileExecutor::GetFileComponents(LPCTSTR pszFile, CString& strPath, CString& strType)
{
	CString strFile = pszFile;
	int nPos = strFile.ReverseFind( '\\' );
	if ( nPos >= 0 ) strPath = strFile.Left( nPos );
	nPos = strFile.ReverseFind( '.' );
	if ( nPos >= 0 ) strType = strFile.Mid( nPos + 1 );
	if ( strType.GetLength() ) strType = _T("|") + strType + _T("|");
}

CMediaWnd* CFileExecutor::GetMediaWindow(BOOL bFocus)
{
	CMainWnd* pMainWnd = (CMainWnd*)theApp.m_pSafeWnd;
	if ( pMainWnd == NULL ) return NULL;
	if ( pMainWnd->IsKindOf( RUNTIME_CLASS(CMainWnd) ) == FALSE ) return NULL;
	return (CMediaWnd*)pMainWnd->m_pWindows.Open( RUNTIME_CLASS(CMediaWnd), FALSE, bFocus );
}

CLibraryWnd* CFileExecutor::GetLibraryWindow()
{
	CMainWnd* pMainWnd = (CMainWnd*)theApp.m_pSafeWnd;
	if ( pMainWnd == NULL ) return NULL;
	if ( pMainWnd->IsKindOf( RUNTIME_CLASS(CMainWnd) ) == FALSE ) return NULL;
	return (CLibraryWnd*)pMainWnd->m_pWindows.Open( RUNTIME_CLASS(CLibraryWnd), FALSE, TRUE );
}

//////////////////////////////////////////////////////////////////////
// CFileExecutor execute

BOOL CFileExecutor::Execute(LPCTSTR pszFile, BOOL bForce)
{
	CString strPath, strType;
	CWaitCursor pCursor;
	
	GetFileComponents( pszFile, strPath, strType );
	
	if ( strType.GetLength() > 0 && _tcsistr( _T("|co|collection|"), strType ) != NULL )
	{
		if ( CLibraryWnd* pWnd = GetLibraryWindow() )
		{
			pWnd->OnCollection( pszFile );
			return TRUE;
		}
	}
	
	if ( bForce == NULL && strType.GetLength() &&
		_tcsistr( Settings.Library.SafeExecute, strType ) == NULL )
	{
		CString strFormat, strPrompt;
		
		Skin.LoadString( strFormat, IDS_LIBRARY_CONFIRM_EXECUTE );
		strPrompt.Format( strFormat, pszFile );
		
		int nResult = AfxMessageBox( strPrompt,
			MB_ICONQUESTION|MB_YESNOCANCEL|MB_DEFBUTTON2 );
		
		if ( nResult == IDCANCEL ) return FALSE;
		else if ( nResult == IDNO ) return TRUE;
	}
	
	if ( Plugins.OnExecuteFile( pszFile ) ) return TRUE;
	
	if ( Settings.MediaPlayer.EnablePlay && strType.GetLength() &&
		 ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0 )
	{
		if ( _tcsistr( Settings.MediaPlayer.FileTypes, strType ) != NULL )
		{
			BOOL bAudio = _tcsistr( _T("|ape|mid|mp3|ogg|wav|wma|"), strType ) != NULL;
			
			if ( CMediaWnd* pWnd = GetMediaWindow( ! bAudio ) )
			{
				pWnd->PlayFile( pszFile );
				return TRUE;
			}
		}
	}
	
	ShellExecute( AfxGetMainWnd()->GetSafeHwnd(),
		NULL, pszFile, NULL, strPath, SW_SHOWNORMAL );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CFileExecutor enqueue

BOOL CFileExecutor::Enqueue(LPCTSTR pszFile, BOOL bForce)
{
	CString strPath, strType;
	CWaitCursor pCursor;
	
	GetFileComponents( pszFile, strPath, strType );
	
	if ( Plugins.OnEnqueueFile( pszFile ) ) return TRUE;
	
	if ( Settings.MediaPlayer.EnableEnqueue && strType.GetLength() &&
		 ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0 )
	{
		if ( _tcsistr( Settings.MediaPlayer.FileTypes, strType ) != NULL )
		{
			if ( CMediaWnd* pWnd = GetMediaWindow( FALSE ) )
			{
				pWnd->EnqueueFile( pszFile );
				return TRUE;
			}
		}
	}
	
	ShellExecute( AfxGetMainWnd()->GetSafeHwnd(),
		_T("Enqueue"), pszFile, NULL, strPath, SW_SHOWNORMAL );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CFileExecutor show Bitzi ticket

BOOL CFileExecutor::ShowBitziTicket(DWORD nIndex)
{
	CString str;
	
	if ( ! Settings.Library.BitziOkay )
	{
		Skin.LoadString( str, IDS_LIBRARY_BITZI_MESSAGE );
		if ( AfxMessageBox( str, MB_ICONQUESTION|MB_YESNO ) != IDYES ) return FALSE;
		Settings.Library.BitziOkay = TRUE;
		Settings.Save();
	}
	
	CSingleLock pLock( &Library.m_pSection, TRUE );
	
	CLibraryFile* pFile = Library.LookupFile( nIndex );
	if ( pFile == NULL ) return FALSE;
	
	if ( pFile->m_bSHA1 == FALSE || pFile->m_bTiger == FALSE )
	{
		CString strFormat;
		Skin.LoadString( strFormat, IDS_LIBRARY_BITZI_HASHED );
		str.Format( strFormat, (LPCTSTR)pFile->m_sName );
		pLock.Unlock();
		AfxMessageBox( str, MB_ICONINFORMATION );
		return FALSE;
	}
	
	CString strURL = Settings.Library.BitziWebView;
	CFile hFile;
	
	if ( hFile.Open( pFile->GetPath(), CFile::modeRead|CFile::shareDenyNone ) && hFile.GetLength() > 0 )
	{
		static LPCTSTR pszHex = _T("0123456789ABCDEF");
		BYTE nBuffer[20];
		int nPeek = hFile.Read( nBuffer, 20 );
		hFile.Close();
		str.Empty();
		
		for ( int nByte = 0 ; nByte < nPeek ; nByte++ )
		{
			str += pszHex[ (BYTE)nBuffer[ nByte ] >> 4 ];
			str += pszHex[ (BYTE)nBuffer[ nByte ] & 15 ];
		}
		
		strURL = Settings.Library.BitziWebSubmit;
		Replace( strURL, _T("(FIRST20)"), str );
	}
	
	Replace( strURL, _T("(NAME)"), CConnection::URLEncode( pFile->m_sName ) );
	Replace( strURL, _T("(SHA1)"), CSHA::HashToString( &pFile->m_pSHA1 ) );
	Replace( strURL, _T("(TTH)"), CTigerNode::HashToString( &pFile->m_pTiger ) );
	Replace( strURL, _T("(AGENT)"), CConnection::URLEncode( Settings.SmartAgent( Settings.Library.BitziAgent ) ) );
	
	str.Format( _T("%I64i"), pFile->GetSize() );
	Replace( strURL, _T("(SIZE)"), str );
	
	pLock.Unlock();
	
	DisplayURL( strURL );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CFileExecutor display a URL

BOOL CFileExecutor::DisplayURL(LPCTSTR pszURL)
{
	ShellExecute( AfxGetMainWnd()->GetSafeHwnd(), _T("open"), pszURL, NULL, NULL, SW_SHOWNORMAL );
	return TRUE;
	
#if 0
	DWORD dwFilterFlags = 0;
	BOOL bSuccess = FALSE;
	DWORD hInstance = 0;
	
	UINT uiResult = DdeInitialize( &hInstance, DDECallback, dwFilterFlags, 0 );
	if ( uiResult != DMLERR_NO_ERROR ) return FALSE;
	
#ifdef _UNICODE
	HSZ hszService	= DdeCreateStringHandle( hInstance, L"IExplore", CP_WINUNICODE );
	HSZ hszTopic	= DdeCreateStringHandle( hInstance, L"WWW_OpenURL", CP_WINUNICODE );
#else
	HSZ hszService	= DdeCreateStringHandle( hInstance, "IExplore", CP_WINANSI );
	HSZ hszTopic	= DdeCreateStringHandle( hInstance, "WWW_OpenURL", CP_WINANSI );
#endif
	
	if ( HCONV hConv = DdeConnect( hInstance, hszService, hszTopic, NULL ) )
	{
		CString strCommand;
		USES_CONVERSION;
		
		strCommand.Format( _T("\"%s\",,0"), pszURL );
		LPCSTR pszCommand = T2CA( (LPCTSTR)strCommand );
		
		DdeClientTransaction( (LPBYTE)pszCommand, pszCommand,
			 hConv, 0, 0, XTYP_EXECUTE, 4000, NULL );
		
		DdeDisconnect( hConv );
	}
	
	DdeFreeStringHandle( hInstance, hszTopic );
	DdeFreeStringHandle( hInstance, hszService );
	
	DdeUninitialize( hInstance );
	
	return bSuccess;
#endif
}

HDDEDATA CALLBACK CFileExecutor::DDECallback(UINT wType, UINT wFmt, HCONV hConv, HSZ hsz1, HSZ hsz2, HDDEDATA hData, DWORD dwData1, DWORD dwData2)
{
	return (HDDEDATA)NULL;
}
