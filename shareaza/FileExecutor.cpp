//
// FileExecutor.cpp
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
				strParam.Format( KnownPlayers[ i ].szEnqueue, strFile );
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
				strParam.Format( KnownPlayers[ i ].szEnqueue, strFile );
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
// CFileExecutor show Bitzi ticket

BOOL CFileExecutor::ShowBitziTicket(DWORD nIndex)
{
	CString str;

	if ( ! Settings.WebServices.BitziOkay )
	{
		if ( AfxMessageBox( LoadString( IDS_LIBRARY_BITZI_MESSAGE ), MB_ICONQUESTION|MB_YESNO ) != IDYES ) return FALSE;
		Settings.WebServices.BitziOkay = TRUE;
		Settings.Save();
	}

	CSingleLock pLock( &Library.m_pSection, TRUE );

	CLibraryFile* pFile = Library.LookupFile( nIndex );
	if ( pFile == NULL ) return FALSE;

	if ( !pFile->m_oSHA1 || !pFile->m_oTiger || !pFile->m_oED2K )
	{
		str.Format( LoadString( IDS_LIBRARY_BITZI_HASHED ), (LPCTSTR)pFile->m_sName );
		pLock.Unlock();
		AfxMessageBox( str, MB_ICONINFORMATION );
		return FALSE;
	}

	CString strURL = Settings.WebServices.BitziWebView;
	CFile hFile;

	if ( hFile.Open( pFile->GetPath(), CFile::modeRead|CFile::shareDenyNone ) )
	{
		strURL = Settings.WebServices.BitziWebSubmit;

		if ( hFile.GetLength() > 0 )
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

			strURL.Replace( _T("(FIRST20)"), str );
		}
		else
			strURL.Replace( _T("(FIRST20)"), _T("0") );
	}
	else
		strURL.Replace( _T("(URN)"), pFile->m_oSHA1.toString() + _T(".") + pFile->m_oTiger.toString() );

	CString strName = pFile->m_sName;
	LPCTSTR pszExt = _tcsrchr( strName, '.' );
	int nExtLen = pszExt ? static_cast< int >( _tcslen( pszExt ) - 1 ) : 0;
	CString strExt = strName.Right( nExtLen );
	strExt.Trim().MakeUpper();

	strURL.Replace( _T("(NAME)"), URLEncode( strName ) );
	strURL.Replace( _T("(SHA1)"), pFile->m_oSHA1.toString() );
	strURL.Replace( _T("(TTH)"), pFile->m_oTiger.toString() );
	strURL.Replace( _T("(ED2K)"), pFile->m_oED2K.toString() );
	strURL.Replace( _T("(AGENT)"), URLEncode( Settings.SmartAgent() ) );

	str.Format( _T("%I64u"), pFile->GetSize() );
	strURL.Replace( _T("(SIZE)"), str );


	CString strINFO = _T("&tag.tiger.tree=") + pFile->m_oTiger.toString();
	if ( pFile->m_oMD5 )
		strINFO += _T("&tag.md5.md5=") + pFile->m_oMD5.toString();
	if ( pFile->m_sComments.Trim().GetLength() )
		strINFO += _T("&tag.subjective.comment=") + URLEncode( pFile->m_sComments );

	if ( pFile->m_pMetadata != NULL && pFile->m_pSchema != NULL )
	{
		CXMLElement* pMetadata = pFile->m_pMetadata;
		int nTemp, nMP3orOGGorWAVTag = 0, nImageTag = 0;
		CString strDescription, strTitle, strMP3orOGGorWAVTag, strImageTag;

		for ( POSITION pos = pMetadata->GetAttributeIterator() ; pos ; )
		{
			CString strReplace;
			CXMLNode* pNode = pMetadata->GetNextAttribute( pos );
			str = pNode->GetName();
			strReplace = pNode->GetValue();

			if ( str.CompareNoCase( _T("link") ) == 0 )
				strINFO += _T("&tag.url.url=") + URLEncode( strReplace );
			else if ( pFile->m_pSchema->CheckURI( CSchema::uriAudio ) )
			{
				if ( str.CompareNoCase( _T("description") ) == 0 )
					strINFO += _T("&tag.objective.description=") + URLEncode( strReplace.Trim() );
				else if ( str.CompareNoCase( _T("title") ) == 0 )
					strINFO += _T("&tag.audiotrack.title=") + URLEncode( strReplace.Trim() );
				else if ( str.CompareNoCase( _T("artist") ) == 0 )
					strINFO += _T("&tag.audiotrack.artist=") + URLEncode( strReplace.Trim() );
				else if ( str.CompareNoCase( _T("album") ) == 0 )
					strINFO += _T("&tag.audiotrack.album=") + URLEncode( strReplace.Trim() );
				else if ( str.CompareNoCase( _T("track") ) == 0 )
				{
					nTemp = _ttoi( strReplace );
					strReplace.Format( _T("%d"), nTemp );

					strINFO += _T("&tag.audiotrack.tracknumber=") + strReplace;
				}
				else if ( str.CompareNoCase( _T("year") ) == 0 )
				{
					nTemp = _ttoi( strReplace );
					strReplace.Format( _T("%d"), nTemp );

					strINFO += _T("&tag.audiotrack.year=") + strReplace;
				}
				// ToDO: Read WAV information in FileExecutor.cpp, bitzi submit is already ready
				else if ( strExt == "MP3" || strExt == "OGG" || strExt == "WAV" )
				{
					if ( str.CompareNoCase( _T("bitrate") ) == 0 )
					{
						if ( strExt == "MP3" )
						{
							strMP3orOGGorWAVTag += _T("&tag.mp3.vbr=");

							if( _tcsstr( strReplace, _T("~") ) )
								strMP3orOGGorWAVTag += _T("y");
							else
								strMP3orOGGorWAVTag += _T("n");
						}

						nTemp = _ttoi( strReplace );
						strReplace.Format( _T("%d"), nTemp );

						if ( strExt == "MP3" )
							strMP3orOGGorWAVTag += _T("&tag.mp3.bitrate=");
						else if ( strExt == "OGG" )
							strMP3orOGGorWAVTag += _T("&tag.vorbis.bitrate=");
						else
							strReplace.Empty();

						if ( strReplace.GetLength() )
						{
							strMP3orOGGorWAVTag += strReplace;
							nMP3orOGGorWAVTag++;
						}
					}
					// ToDO: Read sampleSize of WAV in FileExecutor.cpp, bitzi submit is already ready
					else if ( str.CompareNoCase( _T("sampleSize") ) == 0 )
					{
						nTemp = _ttoi( strReplace );
						strReplace.Format( _T("%d"), nTemp );

						if ( strExt == "WAV" )
						{
							strMP3orOGGorWAVTag += _T("&tag.wav.samplesize=") + strReplace;
							nMP3orOGGorWAVTag++;
						}
					}
					else if ( str.CompareNoCase( _T("seconds") ) == 0 )
					{
						nTemp = (int)( _wtof( strReplace ) * 1000 );
						strReplace.Format( _T("%d"), nTemp );

						if ( strExt == "MP3" )
							strMP3orOGGorWAVTag += _T("&tag.mp3.duration=");
						else if ( strExt == "OGG" )
							strMP3orOGGorWAVTag += _T("&tag.vorbis.duration=");
						else if ( strExt == "WAV" )
							strMP3orOGGorWAVTag += _T("&tag.wav.duration=");
						else
							strReplace.Empty();

						if ( strReplace.GetLength() )
						{
							strMP3orOGGorWAVTag += strReplace;
							nMP3orOGGorWAVTag++;
						}
					}
					else if ( str.CompareNoCase( _T("sampleRate") ) == 0 )
					{
						nTemp = _ttoi( strReplace );
						strReplace.Format( _T("%d"), nTemp );

						if ( strExt == "MP3" )
							strMP3orOGGorWAVTag += _T("&tag.mp3.samplerate=");
						else if ( strExt == "OGG" )
							strMP3orOGGorWAVTag += _T("&tag.vorbis.samplerate=");
						else if ( strExt == "WAV" )
							strMP3orOGGorWAVTag += _T("&tag.wav.samplerate=");
						else
							strReplace.Empty();

						if ( strReplace.GetLength() )
						{
							strMP3orOGGorWAVTag += strReplace;
							nMP3orOGGorWAVTag++;
						}
					}
					else if ( str.CompareNoCase( _T("channels") ) == 0 )
					{
						nTemp = _ttoi( strReplace );
						strReplace.Format( _T("%d"), nTemp );

						if ( strExt == "OGG" )
							strMP3orOGGorWAVTag += _T("&tag.vorbis.channels=");
						else if ( strExt == "WAV" )
							strMP3orOGGorWAVTag += _T("&tag.wav.channels=");
						else
							strReplace.Empty();

						if ( strReplace.GetLength() )
						{
							strMP3orOGGorWAVTag += strReplace;
							nMP3orOGGorWAVTag++;
						}
					}
					else if ( str.CompareNoCase( _T("soundType") ) == 0 )
					{
						if ( strExt == "MP3" )
							if ( ( strReplace == "Stereo" ) || ( strReplace == "Joint Stereo" ) )
								strMP3orOGGorWAVTag += _T("&tag.mp3.stereo=y");
							else if ( ( strReplace == "Dual Channel" ) || ( strReplace == "Single Channel" ) )
								strMP3orOGGorWAVTag += _T("&tag.mp3.stereo=n");
							else
								strReplace.Empty();

						if ( strReplace.GetLength() )
							nMP3orOGGorWAVTag++;
					}
					else if ( str.CompareNoCase( _T("encoder") ) == 0 )
					{
						if ( strExt == "MP3" )
							strMP3orOGGorWAVTag += _T("&tag.mp3.encoder=");
						else if ( strExt == "OGG" )
							strMP3orOGGorWAVTag += _T("&tag.vorbis.encoder=");
						else
							strReplace.Empty();

						if ( strReplace.GetLength() )
							strMP3orOGGorWAVTag += URLEncode( strReplace );
					}
				}
			}
			else if ( pFile->m_pSchema->CheckURI( CSchema::uriImage ) )
			{
				if ( str.CompareNoCase( _T("description") ) == 0 )
					strINFO += _T("&tag.objective.description=") + URLEncode( strReplace.Trim() );
				else if ( str.CompareNoCase( _T("width") ) == 0 )
				{
					nTemp = _ttoi( strReplace );
					strReplace.Format( _T("%d"), nTemp );

					strImageTag += _T("&tag.image.width=") + strReplace;
					nImageTag++;
				}
				else if ( str.CompareNoCase( _T("height") ) == 0 )
				{
					nTemp = _ttoi( strReplace );
					strReplace.Format( _T("%d"), nTemp );

					strImageTag += _T("&tag.image.height=") + strReplace;
					nImageTag++;
				}
				else if ( str.CompareNoCase( _T("colors") ) == 0 )
				{
					if ( strReplace == "2" ) strReplace = "1";
					else if ( strReplace == "16" ) strReplace = "4";
					else if ( strReplace == "256" || strReplace == "Greyscale" ) strReplace = "8";
					else if ( strReplace == "64K" ) strReplace = "16";
					else if ( strReplace == "16.7M" ) strReplace = "24";
					else strReplace = "";

					if ( strReplace.GetLength() )
					{
						strImageTag += _T("&tag.image.bpp=") + strReplace;
						nImageTag++;
					}
				}
			}
			else if ( pFile->m_pSchema->CheckURI( CSchema::uriVideo ) )
			{
				if ( str.CompareNoCase( _T("realdescription") ) == 0 )
					strINFO += _T("&tag.objective.description=") + URLEncode( strReplace.Trim() );
				else if ( str.CompareNoCase( _T("width") ) == 0 )
				{
					nTemp = _ttoi( strReplace );
					strReplace.Format( _T("%d"), nTemp );

					strINFO += _T("&tag.video.width=") + strReplace;
				}
				else if ( str.CompareNoCase( _T("height") ) == 0 )
				{
					nTemp = _ttoi( strReplace );
					strReplace.Format( _T("%d"), nTemp );

					strINFO += _T("&tag.video.height=") + strReplace;
				}
				else if ( str.CompareNoCase( _T("frameRate") ) == 0 )
				{
					nTemp = _ttoi( strReplace );
					strReplace.Format( _T("%d"), nTemp );

					strINFO += _T("&tag.video.fps=") + strReplace;
				}
				else if ( str.CompareNoCase( _T("minutes") ) == 0 )
				{
					nTemp = (int)( _wtof( strReplace ) * 60 * 1000 );
					strReplace.Format( _T("%d"), nTemp );

					strINFO += _T("&tag.video.duration=") + strReplace;
				}
				// ToDO: Read video's bitrate in FileExecutor.cpp, bitzi submit is already ready
				else if ( str.CompareNoCase( _T("bitrate") ) == 0 )
				{
					nTemp = _ttoi( strReplace );
					strReplace.Format( _T("%d"), nTemp );

					strINFO += _T("&tag.video.bitrate=") + strReplace;
				}
				else if ( str.CompareNoCase( _T("codec") ) == 0 )
				{
					strReplace.MakeUpper();
					strINFO += _T("&tag.video.codec=") + URLEncode( strReplace );
				}
			}
			else if ( pFile->m_pSchema->CheckURI( CSchema::uriApplication ) )
			{
				if ( str.CompareNoCase( _T("fileDescription") ) == 0 )
					strDescription = URLEncode( strReplace.Trim() );
				else if ( str.CompareNoCase( _T("title") ) == 0 )
					strTitle = URLEncode( strReplace.Trim() );
			}
			else
			{
				if ( str.CompareNoCase( _T("description") ) == 0 )
					strINFO += _T("&tag.objective.description=") + URLEncode( strReplace.Trim() );
			}
		}

		if ( nMP3orOGGorWAVTag == 4 )
			strINFO += strMP3orOGGorWAVTag;

		if ( nImageTag == 3 )
		{
			strINFO += strImageTag;

			if ( strExt == "BMP" || strExt == "GIF" || strExt == "PNG" )
				strINFO += _T("&tag.image.format=") + strExt;
			else if ( strExt == "JPG" || strExt == "JPEG" || strExt == "JPE" || strExt == "JFIF" )
				strINFO += _T("&tag.image.format=JPEG");
		}

		if ( strDescription.GetLength() )
			strINFO += _T("&tag.objective.description=") + strDescription;
		else if ( strTitle.GetLength() )
			strINFO += _T("&tag.objective.description=") + strTitle;
	}

	if ( strExt == "AVI" )
		strINFO += _T("&tag.video.format=AVI");
	else if ( strExt == "DIVX" || strExt == "DIV" || strExt == "TIX" )
		strINFO += _T("&tag.video.format=DivX");
	else if ( strExt == "XVID" )
		strINFO += _T("&tag.video.format=XviD");
	else if ( strExt == "MKV" )
		strINFO += _T("&tag.video.format=Matroska");
	else if ( strExt == "MOV" || strExt == "QT" )
		strINFO += _T("&tag.video.format=QuickTime");
	else if ( strExt == "RM" || strExt == "RMVB" || strExt == "RAM" || strExt == "RPM" || strExt == "RV" )
		strINFO += _T("&tag.video.format=Real");
	else if ( strExt == "MPG" || strExt == "MPEG" || strExt == "MPE" )
		strINFO += _T("&tag.video.format=MPEG");
	else if ( strExt == "M1V" )
		strINFO += _T("&tag.video.format=MPEG-1");
	else if ( strExt == "MPV2" || strExt == "MP2" || strExt == "M2V" )
		strINFO += _T("&tag.video.format=MPEG-2");
	else if ( strExt == "MP4" || strExt == "M4V" )
		strINFO += _T("&tag.video.format=MPEG-4");
	else if ( strExt == "WM" || strExt == "WMV" || strExt == "WMD" || strExt == "ASF" )
		strINFO += _T("&tag.video.format=Windows Media");
	else if ( strExt == "OGM" )
		strINFO += _T("&tag.video.format=Ogg Media File");
	else if ( strExt == "VP6" )
		strINFO += _T("&tag.video.format=VP6");
	else if ( strExt == "VOB" )
		strINFO += _T("&tag.video.format=DVD");
	else if ( strExt == "IVF" )
		strINFO += _T("&tag.video.format=Indeo Video");
	else if ( pFile->m_pSchema != NULL && pFile->m_pSchema->CheckURI( CSchema::uriVideo ) && strExt.GetLength() )
		strINFO += _T("&tag.video.format=") + URLEncode( strExt );

	strURL.Replace( _T("&(INFO)"), strINFO );

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
