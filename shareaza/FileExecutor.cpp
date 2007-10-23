//
// FileExecutor.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

#include "XML.h"
#include "Schema.h"

#include "Library.h"
#include "SharedFile.h"
#include "SHA.h"
#include "TigerTree.h"
#include "ED2K.h"
#include "MD5.h"
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

void CFileExecutor::GetFileComponents(LPCTSTR pszFile, CString& strPath, CString& strType, CString& strShortPath)
{
	TCHAR pszShortPath[ MAX_PATH ];
	CString strFile = pszFile;

	if ( GetShortPathNameW( strFile, pszShortPath, MAX_PATH ) ) 
		strShortPath.SetString( pszShortPath );
	else strShortPath.Empty();

	int nPos = strFile.ReverseFind( '\\' );
	if ( nPos >= 0 ) strPath = strFile.Left( nPos );
	nPos = strFile.ReverseFind( '.' );
	if ( nPos >= 0 ) strType = strFile.Mid( nPos + 1 );
	if ( strType.GetLength() ) strType = _T("|") + ToLower( strType ) + _T("|");
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

BOOL CFileExecutor::Execute(LPCTSTR pszFile, BOOL bForce, BOOL bHasThumbnail, LPCTSTR pszExt)
{
	CString strPath, strShortPath, strType;
	CWaitCursor pCursor;

	GetFileComponents( pszFile, strPath, strType, strShortPath );

	if ( strType.GetLength() > 0 && _tcsistr( _T("|co|collection|"), strType ) != NULL )
	{
		if ( CLibraryWnd* pWnd = GetLibraryWindow() )
		{
			pWnd->OnCollection( pszFile );
			return TRUE;
		}
	}

	bool bPartial = false;
	if ( _tcsistr( _T("|partial|"), strType ) != NULL && pszExt )
	{
		bPartial = true;
		strType.SetString( _T("|") );
		strType.Append( pszExt );
		strType.Append( _T("|") );
	}

	BOOL bShiftKey = ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) != 0;
	BOOL bPreviewEnabled = FALSE;

	// If thumbnailing and Image Viewer are enabled, do not warn about safety
	if ( ! bShiftKey )
	{
		CLSID clsid;
		CString strPureExtension( strType ); 
		strPureExtension.Replace( _T("|"), _T("") );
		strPureExtension.Insert( 0, '.' );
		bPreviewEnabled = Plugins.LookupCLSID( _T("ImageService"), strPureExtension, clsid );
		Hashes::fromGuid( _T("{2EE9D739-7726-41cf-8F18-4B1B8763BC63}"), &clsid );

		// We won't care if extensions are disabled for the partial files
		// A workaround to get Image Viewer executed for image partials
		if ( !bPartial )
			bPreviewEnabled &= bHasThumbnail && Plugins.LookupEnable( clsid, FALSE, strPureExtension );
	}

	if ( bForce == NULL && strType.GetLength() &&
		_tcsistr( Settings.Library.SafeExecute, strType ) == NULL && ! bPreviewEnabled )
	{
		CString strFormat, strPrompt;

		Skin.LoadString( strFormat, IDS_LIBRARY_CONFIRM_EXECUTE );
		strPrompt.Format( strFormat, pszFile );

		int nResult = AfxMessageBox( strPrompt,
			MB_ICONQUESTION|MB_YESNOCANCEL|MB_DEFBUTTON2 );

		if ( nResult == IDCANCEL ) return FALSE;
		else if ( nResult == IDNO ) return TRUE;
	}

	if ( Settings.MediaPlayer.EnablePlay && strType.GetLength() && ! bShiftKey )
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

	CString strFile;
	if ( Settings.MediaPlayer.ShortPaths && ! strShortPath.IsEmpty() )
		strFile = strShortPath;
	else
		strFile.Format( _T("\"%s\""), pszFile );

	if ( ! bShiftKey )
	{
		if ( _tcsistr( Settings.MediaPlayer.FileTypes, strType ) != NULL && 
			 ! Settings.MediaPlayer.ServicePath.IsEmpty() )
		{
			CString strExecPath;
			int nBackSlash = Settings.MediaPlayer.ServicePath.ReverseFind( '\\' );
			strExecPath = Settings.MediaPlayer.ServicePath.Left( nBackSlash );
			ShellExecute( AfxGetMainWnd()->GetSafeHwnd(), _T("open"), Settings.MediaPlayer.ServicePath, 
				strFile, strExecPath, SW_SHOWNORMAL );
			return TRUE;
		}
		
		if ( bPreviewEnabled && Plugins.OnExecuteFile( pszFile, bHasThumbnail || bPartial ) )
			return TRUE;
	}
	
	// Todo: Doesn't work with partial files
	ShellExecute( AfxGetMainWnd()->GetSafeHwnd(),
			NULL, strFile, NULL, strPath, SW_SHOWNORMAL );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CFileExecutor enqueue

BOOL CFileExecutor::Enqueue(LPCTSTR pszFile, BOOL /*bForce*/, LPCTSTR pszExt)
{
	CString strPath, strShortPath, strType;
	CWaitCursor pCursor;

	GetFileComponents( pszFile, strPath, strType, strShortPath );

	if ( Plugins.OnEnqueueFile( pszFile ) ) return TRUE;

	CString strFile = Settings.MediaPlayer.ShortPaths ? strShortPath : pszFile;
	if ( pszExt && _tcsistr( _T("|partial|"), strType ) != NULL ) 
	{
		strType.SetString( _T("|") );
		strType.Append( pszExt );
		strType.Append( _T("|") );
	}

	BOOL bShiftKey = ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) != 0;

	if ( Settings.MediaPlayer.EnableEnqueue && strType.GetLength() && ! bShiftKey )
	{
		if ( _tcsistr( Settings.MediaPlayer.FileTypes, strType ) != NULL )
		{
			if ( CMediaWnd* pWnd = GetMediaWindow( FALSE ) )
			{
				pWnd->EnqueueFile( strFile );
				return TRUE;
			}
		}
	}

	CString strExecPath;
	int nBackSlash = Settings.MediaPlayer.ServicePath.ReverseFind( '\\' );

	if ( _tcsistr( Settings.MediaPlayer.FileTypes, strType ) != NULL && 
		 ! Settings.MediaPlayer.ServicePath.IsEmpty() && ! bShiftKey )
	{
		CString strCommand;
		DWORD nBufferSize = MAX_PATH;
		HRESULT (WINAPI *pfnAssocQueryStringW)(ASSOCF, ASSOCSTR, LPCWSTR, LPCWSTR, LPWSTR, DWORD*);

		if ( theApp.m_hShlWapi != NULL )
			(FARPROC&)pfnAssocQueryStringW = GetProcAddress( theApp.m_hShlWapi, "AssocQueryStringW" );
		else
			pfnAssocQueryStringW = NULL;

		if ( pfnAssocQueryStringW )
		{
			// Sometimes ShellExecute doesn't work, so we find the verb stuff manually
			HRESULT hr = (*pfnAssocQueryStringW)( ASSOCF_OPEN_BYEXENAME, ASSOCSTR_COMMAND, 
				Settings.MediaPlayer.ServicePath, _T("Enqueue"), 
				strCommand.GetBuffer( MAX_PATH ), &nBufferSize );
			strCommand.ReleaseBuffer();

			if ( SUCCEEDED( hr ) )
			{
				int nFind = strCommand.Find( _T("%1") );
				if ( nFind != -1 )
				{
					strCommand.SetString( strCommand.Left( nFind ) + strFile + strCommand.Mid( nFind + 2 ) );
					ToLower( strCommand );
					
					CString strServiceLC = Settings.MediaPlayer.ServicePath;
					ToLower( strServiceLC );
					
					nFind = strCommand.Find( strServiceLC );
					strCommand.SetString( strCommand.Mid( strServiceLC.GetLength() + nFind ) );
					if ( strCommand.Left( 1 ) == _T("\"") ) 
						strCommand.SetString( strCommand.Mid( 1 ).Trim() );

					strExecPath = Settings.MediaPlayer.ServicePath.Left( nBackSlash );
					ShellExecute( NULL, NULL, Settings.MediaPlayer.ServicePath, 
						strCommand, strExecPath, SW_SHOWNORMAL );
					return TRUE;
				}
			}
		}
	}

	// Todo: Doesn't work with partial files
	int nError = (int)(DWORD_PTR)ShellExecute( NULL, _T("Enqueue"), strFile, NULL, strPath, SW_SHOWNORMAL );

	if ( nError <= SE_ERR_DLLNOTFOUND )
	{
		CString strExecutable = Settings.MediaPlayer.ServicePath.Mid( nBackSlash + 1 ).MakeLower();
		CString strParam;

		if ( strExecutable == L"mplayerc.exe" )
		{
			strParam.Format( _T("\"%s\" /add"), strFile );
		} 
		else if ( strExecutable == L"wmplayer.exe" )
		{
			strParam.Format( _T("/SHELLHLP_V9 Enqueue \"%s\""), strFile );
		}
		else if ( strExecutable == L"vlc.exe" )
		{
			strParam.Format( _T("--one-instance --playlist-enqueue \"%s\""), strFile );
		}
		if ( strParam.GetLength() )
			ShellExecute( NULL, NULL, Settings.MediaPlayer.ServicePath, strParam, 
				strExecPath, SW_SHOWNORMAL );
	}
	

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
	
	if ( !pFile->m_oSHA1 || !pFile->m_oTiger || !pFile->m_oED2K )
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

	if ( hFile.Open( pFile->GetPath(), CFile::modeRead|CFile::shareDenyNone ) )
	{
		strURL = Settings.Library.BitziWebSubmit;

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

			Replace( strURL, _T("(FIRST20)"), str );
		}
		else 
			Replace( strURL, _T("(FIRST20)"), _T("0") );
	}
	else
		Replace( strURL, _T("(URN)"), pFile->m_oSHA1.toString() + _T(".") + pFile->m_oTiger.toString() );

	CString strName = pFile->m_sName;
	LPCTSTR pszExt = _tcsrchr( strName, '.' );
	int nExtLen = pszExt ? static_cast< int >( _tcslen( pszExt ) - 1 ) : 0;
	CString strExt = strName.Right( nExtLen );
	strExt.Trim().MakeUpper();

	Replace( strURL, _T("(NAME)"), URLEncode( strName ) );
	Replace( strURL, _T("(SHA1)"), pFile->m_oSHA1.toString() );
	Replace( strURL, _T("(TTH)"), pFile->m_oTiger.toString() );
	Replace( strURL, _T("(ED2K)"), pFile->m_oED2K.toString() );
	Replace( strURL, _T("(AGENT)"), URLEncode( Settings.SmartAgent() ) );

	str.Format( _T("%I64i"), pFile->GetSize() );
	Replace( strURL, _T("(SIZE)"), str );


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

			if ( str == "link" )
				strINFO += _T("&tag.url.url=") + URLEncode( strReplace );
			else if ( pFile->m_pSchema->m_sURI.CompareNoCase( CSchema::uriAudio ) == 0 )
			{
				if ( str == "description" )
					strINFO += _T("&tag.objective.description=") + URLEncode( strReplace.Trim() );
				else if ( str == "title" )
					strINFO += _T("&tag.audiotrack.title=") + URLEncode( strReplace.Trim() );
				else if ( str == "artist" )
					strINFO += _T("&tag.audiotrack.artist=") + URLEncode( strReplace.Trim() );
				else if ( str == "album" )
					strINFO += _T("&tag.audiotrack.album=") + URLEncode( strReplace.Trim() );
				else if ( str == "track" )
				{
					nTemp = _ttoi( strReplace );
					strReplace.Format( _T("%d"), nTemp );

					strINFO += _T("&tag.audiotrack.tracknumber=") + strReplace;
				}
				else if ( str == "year" )
				{
					nTemp = _ttoi( strReplace );
					strReplace.Format( _T("%d"), nTemp );

					strINFO += _T("&tag.audiotrack.year=") + strReplace;
				}
				// ToDO: Read WAV information in FileExecutor.cpp, bitzi submit is already ready
				else if ( strExt == "MP3" || strExt == "OGG" || strExt == "WAV" )
				{
					if ( str == "bitrate" )
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
					else if ( str == "sampleSize" )
					{
						nTemp = _ttoi( strReplace );
						strReplace.Format( _T("%d"), nTemp );

						if ( strExt == "WAV" )
						{
							strMP3orOGGorWAVTag += _T("&tag.wav.samplesize=") + strReplace;
							nMP3orOGGorWAVTag++;
						}
					}
					else if ( str == "seconds" )
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
					else if ( str == "sampleRate" )
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
					else if ( str == "channels" )
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
					else if ( str == "soundType" )
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
					else if ( str == "encoder" )
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
			else if ( pFile->m_pSchema->m_sURI.CompareNoCase( CSchema::uriImage ) == 0 )
			{
				if ( str == "description" )
					strINFO += _T("&tag.objective.description=") + URLEncode( strReplace.Trim() );
				else if ( str == "width" )
				{
					nTemp = _ttoi( strReplace );
					strReplace.Format( _T("%d"), nTemp );

					strImageTag += _T("&tag.image.width=") + strReplace;
					nImageTag++;
				}
				else if ( str == "height" )
				{
					nTemp = _ttoi( strReplace );
					strReplace.Format( _T("%d"), nTemp );

					strImageTag += _T("&tag.image.height=") + strReplace;
					nImageTag++;
				}
				else if ( str == "colors" )
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
			else if ( pFile->m_pSchema->m_sURI.CompareNoCase( CSchema::uriVideo ) == 0 )
			{
				if ( str == "realdescription" )
					strINFO += _T("&tag.objective.description=") + URLEncode( strReplace.Trim() );
				else if ( str == "width" )
				{
					nTemp = _ttoi( strReplace );
					strReplace.Format( _T("%d"), nTemp );

					strINFO += _T("&tag.video.width=") + strReplace;
				}
				else if ( str == "height" )
				{
					nTemp = _ttoi( strReplace );
					strReplace.Format( _T("%d"), nTemp );

					strINFO += _T("&tag.video.height=") + strReplace;
				}
				else if ( str == "frameRate" )
				{
					nTemp = _ttoi( strReplace );
					strReplace.Format( _T("%d"), nTemp );

					strINFO += _T("&tag.video.fps=") + strReplace;
				}
				else if ( str == "minutes" )
				{
					nTemp = (int)( _wtof( strReplace ) * 60 * 1000 );
					strReplace.Format( _T("%d"), nTemp );

					strINFO += _T("&tag.video.duration=") + strReplace;
				}
				// ToDO: Read video's bitrate in FileExecutor.cpp, bitzi submit is already ready
				else if ( str == "bitrate" )
				{
					nTemp = _ttoi( strReplace );
					strReplace.Format( _T("%d"), nTemp );

					strINFO += _T("&tag.video.bitrate=") + strReplace;
				}
				else if ( str == "codec" )
				{
					strReplace.MakeUpper();
					strINFO += _T("&tag.video.codec=") + URLEncode( strReplace );
				}
			}
			else if ( pFile->m_pSchema->m_sURI.CompareNoCase( CSchema::uriApplication ) == 0 )
			{
				if ( str == "fileDescription" )
					strDescription = URLEncode( strReplace.Trim() );
				else if ( str == "title" )
					strTitle = URLEncode( strReplace.Trim() );
			}
			else
			{
				if ( str == "description" )
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
	else if ( pFile->m_pSchema != NULL && pFile->m_pSchema->m_sURI.CompareNoCase( CSchema::uriVideo ) == 0 && strExt.GetLength() )
		strINFO += _T("&tag.video.format=") + URLEncode( strExt );

	Replace( strURL, _T("&(INFO)"), strINFO );

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
