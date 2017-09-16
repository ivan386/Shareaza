//
// FileExecutor.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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
#include "FileExecutor.h"
#include "Library.h"
#include "Plugins.h"
#include "Schema.h"
#include "SchemaCache.h"
#include "Security.h"
#include "ShellIcons.h"
#include "XML.h"

#include "DlgTorrentSeed.h"
#include "WndLibrary.h"
#include "WndMedia.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define TOO_MANY_FILES_LIMIT	20

// Some known media players
static const struct
{
	LPCTSTR szPlayer;
	LPCTSTR	szEnqueue;
}
KnownPlayers[] =
{
	// AIMP2
	{ _T("aimp2.exe"),		_T("/ADD_PLAY \"%s\"") },
	// BSPlayer
	{ _T("bsplayer.exe"),	_T("\"%s\" -ADD") },
	// GOM Player
	{ _T("gom.exe"),		_T("/add \"%s\"") },
	// Light Alloy
	{ _T("la.exe"),			_T("/ADD \"%s\"") },
	// MediaPlayer Classic
	{ _T("mplayerc.exe"),	_T("\"%s\" /add") },
	// MediaPlayer Classic 64
	{ _T("mplayerc64.exe"),	_T("\"%s\" /add") },
	// MediaPlayer Classic Home Cinema
	{ _T("mpc-hc.exe"),		_T("\"%s\" /add") },
	// MediaPlayer Classic Home Cinema 64
	{ _T("mpc-hc64.exe"),	_T("\"%s\" /add") },
	// MPlayer
	{ _T("mplayer.exe"),	_T("-enqueue \"%s\"") },
	// SMplayer (GUI for MPlayer)
	{ _T("smplayer.exe"),	_T("-add-to-playlist \"%s\"") },
	// VideoLAN
	{ _T("vlc.exe"),		_T("--one-instance --playlist-enqueue \"%s\"") },
	// WinAmp
	{ _T("winamp.exe"),		_T("/ADD \"%s\"") },
	// Windows Media Player
	{ _T("wmplayer.exe"),	_T("/SHELLHLP_V9 Enqueue \"%s\"") },
	// Zoom Player
	{ _T("zplayer.exe"),	_T("\"/Queue:%s\"") },
	// (end)
	{ NULL, NULL }
};

int PathGetArgsIndex(const CString& str)
{
	if ( str.GetAt( 0 ) == _T('\"') )
	{
		// "command"args
		int quote = str.Find( _T('\"'), 1 );
		if ( quote == -1 )
			// No closing quote
			return -1;
		else
			return quote + 1;
	}
	// command args
	int i = -1;
	for ( ;; )
	{
		int slash = str.Find( _T('\\'), i + 1 );
		if ( slash == -1 ||
			GetFileAttributes( CString( _T("\\\\?\\") ) + str.Mid( 0, slash + 1 ) ) == INVALID_FILE_ATTRIBUTES )
		{
			return str.Find( _T(' '), i + 1 );
		}
		i = slash;
	}
}

void CFileExecutor::DetectFileType(LPCTSTR pszFile, LPCTSTR szType, bool& bVideo, bool& bAudio, bool& bImage)
{
	if ( GetFileAttributes( CString( _T("\\\\?\\") ) + pszFile ) & FILE_ATTRIBUTE_DIRECTORY )
		return;

	if ( CSchemaPtr pSchema = SchemaCache.GuessByFilename( szType ) )
	{
		if ( pSchema->CheckURI( CSchema::uriAudio ) )
			bAudio = true;
		else if ( pSchema->CheckURI( CSchema::uriVideo ) )
			bVideo = true;
		else if ( pSchema->CheckURI( CSchema::uriImage ) )
			bImage = true;
	}

	// Detect type by MIME "Content Type"
	if ( ! bAudio && ! bVideo && ! bImage )
	{
		CString strMime = ShellIcons.GetMIME( szType );
		if ( ! strMime.IsEmpty() )
		{
			CString strMimeMajor = strMime.SpanExcluding( _T("/") );
			if ( strMimeMajor == _T("video") )
				bVideo = true;
			else if ( strMimeMajor == _T("audio") )
				bAudio = true;
			else if ( strMimeMajor == _T("image") )
				bImage = true;
			else if ( strMime == _T("application/x-shockwave-flash") )
				bVideo = true;
		}
	}

	// Detect type by file schema
	if ( ! bAudio && ! bVideo && ! bImage )
	{
		CQuickLock oLock( Library.m_pSection );
		if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( pszFile ) )
		{
			if ( pFile->IsSchemaURI( CSchema::uriAudio ) )
				bAudio = true;
			else if ( pFile->IsSchemaURI( CSchema::uriVideo ) )
				bVideo = true;
			else if ( pFile->IsSchemaURI( CSchema::uriImage ) )
				bImage = true;
		}
	}
}

CString CFileExecutor::GetCustomPlayer()
{
	for ( string_set::const_iterator i = Settings.MediaPlayer.ServicePath.begin() ;
		i != Settings.MediaPlayer.ServicePath.end(); ++i )
	{
		CString sPlayer = *i;
		int nAstrix = sPlayer.ReverseFind( _T('*') );
		sPlayer.Remove( _T('*') );
		if ( nAstrix != -1 )	// Has astrix at the end so Is Selected player
		{
			return sPlayer;
		}
	}
	return CString();
}

TRISTATE CFileExecutor::IsSafeExecute(LPCTSTR szExt, LPCTSTR szFile)
{
	BOOL bSafe = ! szExt || ! *szExt ||
		IsIn( Settings.Library.SafeExecute, szExt + 1 ) ||
		( theApp.m_pfnAssocIsDangerous && ! theApp.m_pfnAssocIsDangerous( szExt ) );

	if ( ! bSafe && szFile )
	{
		CString strPrompt;
		TCHAR szPrettyPath[ 60 ];
		PathCompactPathEx( szPrettyPath, szFile, _countof( szPrettyPath ) - 1, 0 );
		strPrompt.Format( LoadString( IDS_LIBRARY_CONFIRM_EXECUTE ), szPrettyPath );
		switch ( AfxMessageBox( strPrompt,
			MB_ICONQUESTION | MB_YESNOCANCEL | MB_DEFBUTTON2 ) )
		{
		case IDYES:
			// Run it
			return TRI_TRUE;
		case IDNO:
			// Skip it
			return TRI_FALSE;
		default:
			// Cancel file operation
			return TRI_UNKNOWN;
		}
	}

	return bSafe ? TRI_TRUE : TRI_FALSE;
}

TRISTATE CFileExecutor::IsVerified(LPCTSTR szFile)
{
	BOOL bInsecure = FALSE;
	{
		CQuickLock pLock( Library.m_pSection );
		if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( szFile ) )
		{
			bInsecure = ( pFile->m_bVerify == TRI_FALSE ) &&
				( ! Settings.Search.AdultFilter ||
				  ! AdultFilter.IsChildPornography( szFile ) );
		}
	}

	if ( ! bInsecure )
		// Run it
		return TRI_TRUE;

	CString strMessage;
	strMessage.Format( LoadString( IDS_LIBRARY_VERIFY_FAIL ), szFile );
	INT_PTR nResponse = AfxMessageBox( strMessage,
		MB_ICONEXCLAMATION|MB_YESNOCANCEL|MB_DEFBUTTON2 );
	if ( nResponse == IDCANCEL )
		// Cancel file operation
		return TRI_UNKNOWN;
	else if ( nResponse == IDNO )
		// Skip it
		return TRI_FALSE;

	nResponse = AfxMessageBox( LoadString( IDS_LIBRARY_VERIFY_FIX ),
		MB_ICONQUESTION|MB_YESNOCANCEL|MB_DEFBUTTON2 );
	if ( nResponse == IDCANCEL )
		// Cancel file operation
		return TRI_UNKNOWN;
	else if ( nResponse == IDYES )
	{
		// Reset failed verification flag
		CQuickLock pLock( Library.m_pSection );
		if ( CLibraryFile* pFile = LibraryMaps.LookupFileByPath( szFile ) )
		{
			pFile->m_bVerify = TRI_UNKNOWN;
			Library.Update();
		}
	}

	// Run it
	return TRI_TRUE;
}

BOOL CFileExecutor::Execute(LPCTSTR pszFile, LPCTSTR pszExt)
{
	CWaitCursor pCursor;

	TRISTATE bVerified = IsVerified( pszFile );
	if ( bVerified == TRI_UNKNOWN )
		// Cancel operation
		return FALSE;
	else if ( bVerified == TRI_FALSE )
		// Skip file
		return TRUE;

	CString strType;
	if ( ! ( GetFileAttributes( CString( _T("\\\\?\\") ) + pszFile ) & FILE_ATTRIBUTE_DIRECTORY ) )
		strType = CString( PathFindExtension( pszFile ) ).MakeLower();

	// Open known file types
	if ( theApp.Open( pszFile, FALSE ) )
	{
		theApp.Open( pszFile, TRUE );
		return TRUE;
	}

	// Prepare partials
	bool bPartial = false;
	if ( strType == _T(".partial") && pszExt )
	{
		bPartial = true;
		strType = pszExt;
		strType.MakeLower();
	}

	// Detect type
	bool bVideo = false;
	bool bAudio = false;
	bool bImage = false;
	DetectFileType( pszFile, strType, bVideo, bAudio, bImage );

	// Detect dangerous files
	if ( ! ( bAudio || bVideo || bImage ) )
	{
		TRISTATE bSafe = IsSafeExecute( strType, pszFile );
		if ( bSafe == TRI_UNKNOWN )
			// Cancel operation
			return FALSE;
		else if ( bSafe == TRI_FALSE )
			// Skip file
			return TRUE;
	}

	// Handle video and audio files by internal player
	bool bShiftKey = ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) != 0;
	if ( ! bShiftKey && ( bVideo || bAudio ) && Settings.MediaPlayer.EnablePlay &&
		strType.GetLength() > 1 &&
		IsIn( Settings.MediaPlayer.FileTypes, (LPCTSTR)strType + 1 ) )
	{
		if ( CMediaWnd* pWnd = CMediaWnd::GetMediaWindow( FALSE, ! bAudio ) )
		{
			pWnd->PlayFile( pszFile );
			return TRUE;
		}
	}

	// Handle video and audio files by external player
	CString sCustomPlayer = GetCustomPlayer();
	TCHAR pszShortPath[ MAX_PATH ];
	if ( ! bShiftKey && ( bVideo || bAudio ) && ! sCustomPlayer.IsEmpty() )
	{
		// Prepare file path for execution
		if ( Settings.MediaPlayer.ShortPaths )
		{
			if ( GetShortPathName( pszFile, pszShortPath, MAX_PATH ) )
				pszFile = pszShortPath;
		}

		HINSTANCE hResult = ShellExecute( AfxGetMainWnd()->GetSafeHwnd(), _T("open"),
			sCustomPlayer, CString( _T('\"') ) + pszFile + _T('\"'), NULL, SW_SHOWNORMAL );
		if ( hResult > (HINSTANCE)32 )
			return TRUE;
	}

	// Handle all by plugins
	if ( ! bShiftKey )
	{
		if ( Plugins.OnExecuteFile( pszFile, bImage ) )
			return TRUE;
	}

	// TODO: Doesn't work with partial files

	HINSTANCE hResult = ShellExecute( AfxGetMainWnd()->GetSafeHwnd(), NULL,
		pszFile, NULL, NULL, SW_SHOWNORMAL );
	if ( hResult > (HINSTANCE)32 )
		return TRUE;

	return FALSE;
}

BOOL CFileExecutor::Execute(const CStringList& pList)
{
	if ( pList.GetCount() > TOO_MANY_FILES_LIMIT )
	{
		CString sMessage;
		sMessage.Format( LoadString( IDS_TOO_MANY_FILES ), pList.GetCount() );
		if ( MsgBox( sMessage, MB_ICONQUESTION | MB_YESNO, 0,
			&Settings.Library.TooManyWarning ) != IDYES )
		{
			return FALSE;
		}
	}

	for ( POSITION pos = pList.GetHeadPosition() ; pos ; )
	{
		if ( ! CFileExecutor::Execute( pList.GetNext( pos ) ) )
			return FALSE;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CFileExecutor enqueue

BOOL CFileExecutor::Enqueue(LPCTSTR pszFile, LPCTSTR pszExt)
{
	CWaitCursor pCursor;

	TRISTATE bVerified = IsVerified( pszFile );
	if ( bVerified == TRI_UNKNOWN )
		// Cancel operation
		return FALSE;
	else if ( bVerified == TRI_FALSE )
		// Skip file
		return TRUE;

	CString strType;
	if ( ! ( GetFileAttributes( CString( _T("\\\\?\\") ) + pszFile ) & FILE_ATTRIBUTE_DIRECTORY ) )
		strType = CString( PathFindExtension( pszFile ) ).MakeLower();

	// Prepare partials
	if ( strType == _T(".partial") && pszExt )
	{
		strType = pszExt;
		strType.MakeLower();
	}

	// Detect type
	bool bVideo = false;
	bool bAudio = false;
	bool bImage = false;
	DetectFileType( pszFile, strType, bVideo, bAudio, bImage );

	// Detect dangerous files
	if ( ! ( bAudio || bVideo || bImage ) )
	{
		TRISTATE bSafe = IsSafeExecute( strType, pszFile );
		if ( bSafe == TRI_UNKNOWN )
			return FALSE;
		else if ( bSafe == TRI_FALSE )
			return TRUE;
	}

	// Handle video and audio files by internal player
	bool bShiftKey = ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) != 0;
	if ( ! bShiftKey && ( bVideo || bAudio ) && Settings.MediaPlayer.EnableEnqueue &&
		strType.GetLength() > 1 &&
		IsIn( Settings.MediaPlayer.FileTypes, (LPCTSTR)strType + 1 ) )
	{
		if ( CMediaWnd* pWnd = CMediaWnd::GetMediaWindow( FALSE, FALSE ) )
		{
			pWnd->EnqueueFile( pszFile );
			return TRUE;
		}
	}

	// Handle all by plugins
	if ( ! bShiftKey )
	{
		if ( Plugins.OnEnqueueFile( pszFile ) )
			return TRUE;
	}

	// Delay between first and second in row runs
	static DWORD nRunCount = 0;
	static DWORD nLastRun = 0;
	if ( GetTickCount() - nLastRun > 2000 )
		nRunCount = 0;
	if ( ++nRunCount == 2 )
		Sleep( 2000 );
	nLastRun = GetTickCount();

	// Prepare short path
	CString strFile = pszFile;
	if ( Settings.MediaPlayer.ShortPaths )
	{
		TCHAR pszShortPath[ MAX_PATH ];
		if ( GetShortPathName( pszFile, pszShortPath, MAX_PATH ) )
			strFile = pszShortPath;
	}

	// Handle video and audio files by external player
	CString sCustomPlayer = GetCustomPlayer();
	if ( ! bShiftKey && ( bVideo || bAudio ) && ! sCustomPlayer.IsEmpty() )
	{
		// Try Shell "enqueue" verb
		CString strCommand, strParam;
		DWORD nBufferSize = MAX_PATH;
		HRESULT hr = AssocQueryString( ASSOCF_OPEN_BYEXENAME, ASSOCSTR_COMMAND,
			sCustomPlayer, _T("enqueue"),
			strCommand.GetBuffer( MAX_PATH ), &nBufferSize );
		strCommand.ReleaseBuffer();
		int nPos = PathGetArgsIndex( strCommand );
		if ( nPos != -1 )
		{
			strParam = strCommand.Mid( nPos ).Trim();
			strCommand = strCommand.Left( nPos );
		}
		strCommand = strCommand.Trim( _T("\" ") );
		if ( hr == S_OK )
		{
			int nFind = strParam.Find( _T("%1") );
			if ( nFind != -1 )
			{
				strParam.Replace( _T("%1"), strFile );
				HINSTANCE hResult = ShellExecute( AfxGetMainWnd()->GetSafeHwnd(), NULL,
					strCommand, strParam, NULL, SW_SHOWNORMAL );
				if ( hResult > (HINSTANCE)32 )
					return TRUE;
			}
		}

		// Try to create "enqueue" verb from default verb for known players
		CString strExecutable = PathFindFileName( sCustomPlayer );
		for ( int i = 0; KnownPlayers[ i ].szPlayer; ++i )
		{
			if ( strExecutable.CompareNoCase( KnownPlayers[ i ].szPlayer ) == 0 )
			{
				strParam.Format( KnownPlayers[ i ].szEnqueue, (LPCTSTR)strFile );
				break;
			}
		}
		if ( ! strParam.IsEmpty() )
		{
			HINSTANCE hResult = ShellExecute( AfxGetMainWnd()->GetSafeHwnd(), NULL,
				sCustomPlayer, strParam, NULL, SW_SHOWNORMAL );
			if ( hResult > (HINSTANCE)32 )
				return TRUE;
		}
	}

	// Try Shell "enqueue" verb
	HINSTANCE hResult = ShellExecute( AfxGetMainWnd()->GetSafeHwnd(), _T("enqueue"),
		strFile, NULL, NULL, SW_SHOWNORMAL );
	if ( hResult > (HINSTANCE)32 )
		return TRUE;

	// Try to create "enqueue" verb from default verb for known players
	CString strCommand;
	DWORD nBufferSize = MAX_PATH;
	HRESULT hr = AssocQueryString( 0, ASSOCSTR_COMMAND, strType, NULL,
		strCommand.GetBuffer( MAX_PATH ), &nBufferSize );
	strCommand.ReleaseBuffer();
	int nPos = PathGetArgsIndex( strCommand );
	if ( nPos != -1 )
	{
		strCommand = strCommand.Left( nPos );
	}
	strCommand = strCommand.Trim( _T("\" ") );
	if ( hr == S_OK )
	{
		CString strParam, strExecutable = PathFindFileName( strCommand );
		for ( int i = 0; KnownPlayers[ i ].szPlayer; ++i )
		{
			if ( strExecutable.CompareNoCase( KnownPlayers[ i ].szPlayer ) == 0 )
			{
				strParam.Format( KnownPlayers[ i ].szEnqueue, (LPCTSTR)strFile );
				break;
			}
		}
		if ( ! strParam.IsEmpty() )
		{
			hResult = ShellExecute( AfxGetMainWnd()->GetSafeHwnd(), NULL,
				strCommand, strParam, NULL, SW_SHOWNORMAL );
			if ( hResult > (HINSTANCE)32 )
				return TRUE;
		}
	}

	// Try default verb
	hResult = ShellExecute( AfxGetMainWnd()->GetSafeHwnd(), NULL,
		strFile, NULL, NULL, SW_SHOWNORMAL );
	if ( hResult > (HINSTANCE)32 )
		return TRUE;

	return FALSE;
}

BOOL CFileExecutor::Enqueue(const CStringList& pList)
{
	if ( pList.GetCount() > TOO_MANY_FILES_LIMIT )
	{
		CString sMessage;
		sMessage.Format( LoadString( IDS_TOO_MANY_FILES ), pList.GetCount() );
		if ( MsgBox( sMessage, MB_ICONQUESTION | MB_YESNO, 0,
			&Settings.Library.TooManyWarning ) != IDYES )
		{
			return FALSE;
		}
	}

	for ( POSITION pos = pList.GetHeadPosition() ; pos ; )
	{
		if ( ! CFileExecutor::Enqueue( pList.GetNext( pos ) ) )
			return FALSE;
	}

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

	HSZ hszService	= DdeCreateStringHandle( hInstance, L"IExplore", CP_WINUNICODE );
	HSZ hszTopic	= DdeCreateStringHandle( hInstance, L"WWW_OpenURL", CP_WINUNICODE );

	if ( HCONV hConv = DdeConnect( hInstance, hszService, hszTopic, NULL ) )
	{
		CString strCommand;

		strCommand.Format( _T("\"%s\",,0"), pszURL );
		CT2A pszCommand( (LPCTSTR)strCommand );

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
