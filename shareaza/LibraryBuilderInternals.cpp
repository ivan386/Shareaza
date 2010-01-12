//
// LibraryBuilderInternals.cpp
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
#include "Library.h"
#include "LibraryMaps.h"
#include "SharedFile.h"
#include "LibraryFolders.h"
#include "LibraryBuilder.h"
#include "LibraryBuilderInternals.h"

#define _ID3_DEFINE_GENRES
#include "Buffer.h"
#include "Schema.h"
#include "XML.h"
#include "ID3.h"
#include "CollectionFile.h"
#include "BTInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderPlugins construction

CLibraryBuilderInternals::CLibraryBuilderInternals()
{
}

CLibraryBuilderInternals::~CLibraryBuilderInternals()
{
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals extract metadata (threaded)

bool CLibraryBuilderInternals::ExtractMetadata(DWORD nIndex, const CString& strPath, HANDLE hFile)
{
	CString strType = PathFindExtension( strPath );
	strType.MakeLower();

	if ( strType == _T(".mp3") )
	{
		if ( !Settings.Library.ScanMP3 )
			return false;
		if ( Settings.Library.PreferAPETags )
		{
			if ( ReadAPE( nIndex, hFile, true ) )
				return true;
			if ( ReadID3v2( nIndex, hFile ) )
				return true;
		}
		else
		{
			if ( ReadID3v2( nIndex, hFile ) )
				return true;
			if ( ReadAPE( nIndex, hFile, true ) )
				return true;
		}
		if ( ReadID3v1( nIndex, hFile ) )
			return true;
		if ( ReadMP3Frames( nIndex, hFile ) )
			return true;
		return LibraryBuilder.SubmitCorrupted( nIndex );
	}
	else if ( strType == _T(".exe") || strType == _T(".dll") )
	{
		if ( !Settings.Library.ScanEXE )
			return false;
		return ReadVersion( nIndex, strPath );
	}
	else if ( strType == _T(".msi") )
	{
		if ( !Settings.Library.ScanMSI )
			return false;
		return ReadMSI( nIndex, strPath );
	}
	else if ( strType == _T(".asf") || strType == _T(".wma") || strType == _T(".wmv") )
	{
		if ( !Settings.Library.ScanASF )
			return false;
		return ReadASF( nIndex, hFile );
	}
	else if ( strType == _T(".avi") )
	{
		if ( !Settings.Library.ScanAVI )
			return false;
		return ReadAVI( nIndex, hFile );
	}
	else if ( strType == _T(".mpg") || strType == _T(".mpeg") )
	{
		if ( !Settings.Library.ScanMPEG )
			return false;
		return ReadMPEG( nIndex, hFile );
	}
	else if ( strType == _T(".ogg") )
	{
		if ( !Settings.Library.ScanOGG )
			return false;
		return ReadOGG( nIndex, hFile );
	}
	else if ( strType == _T(".ape") || strType == _T(".mac") || strType == _T(".apl") )
	{
		if ( !Settings.Library.ScanAPE )
			return false;
		return ReadAPE( nIndex, hFile );
	}
	else if ( strType == _T(".mpc") || strType == _T(".mpp") || strType == _T(".mp+") )
	{
		if ( !Settings.Library.ScanMPC )
			return false;
		if ( Settings.Library.PreferAPETags )
		{
			if ( ReadMPC( nIndex, hFile ) )
				return true;
			if ( ReadID3v2( nIndex, hFile ) )
				return true;
		}
		else
		{
			if ( ReadID3v2( nIndex, hFile ) )
				return true;
			if ( ReadMPC( nIndex, hFile ) )
				return true;
		}
		return ReadID3v1( nIndex, hFile );
	}
	else if ( strType == _T(".jpg") || strType == _T(".jpeg") )
	{
		if ( !Settings.Library.ScanImage )
			return false;
		return ReadJPEG( nIndex, hFile );
	}
	else if ( strType == _T(".gif") )
	{
		if ( !Settings.Library.ScanImage )
			return false;
		return ReadGIF( nIndex, hFile );
	}
	else if ( strType == _T(".png") )
	{
		if ( !Settings.Library.ScanImage )
			return false;
		return ReadPNG( nIndex, hFile );
	}
	else if ( strType == _T(".bmp") )
	{
		if ( !Settings.Library.ScanImage )
			return false;
		return ReadBMP( nIndex, hFile );
	}
	else if ( strType == _T(".pdf") )
	{
		if ( !Settings.Library.ScanPDF )
			return false;
		return ReadPDF( nIndex, hFile, strPath );
	}
	else if ( strType == _T(".co") ||
		strType == _T(".collection") ||
		strType == _T(".emulecollection") )
	{
		return ReadCollection( nIndex, strPath );
	}
	else if ( strType == _T(".chm") )
	{
		if ( !Settings.Library.ScanCHM )
			return false;
		return ReadCHM( nIndex, hFile, strPath );
	}
	else if ( strType == _T(".torrent") )
	{
		return ReadTorrent( nIndex, hFile, strPath );
	}
	return false;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals ID3v1 (threaded)

bool CLibraryBuilderInternals::ReadID3v1(DWORD nIndex, HANDLE hFile, CXMLElement* pXML)
{
	if ( GetFileSize( hFile, NULL ) < 128 )
		return false;

	ID3V1 pInfo;
	DWORD nRead;

	SetFilePointer( hFile, -128, NULL, FILE_END );
	ReadFile( hFile, &pInfo, sizeof(pInfo), &nRead, NULL );

	if ( nRead != sizeof(pInfo) )
		return false;

	if ( strncmp( pInfo.szTag, ID3V1_TAG, 3 ) )
		return false;

	bool bIsMP3 = ( pXML == NULL );
	if ( bIsMP3 )
		pXML = new CXMLElement( NULL, _T("audio") );

	CopyID3v1Field( pXML, _T("title"), CString( pInfo.szSongname, 30 ) );
	CopyID3v1Field( pXML, _T("artist"), CString( pInfo.szArtist, 30 ) );
	CopyID3v1Field( pXML, _T("album"), CString( pInfo.szAlbum, 30 ) );
	CopyID3v1Field( pXML, _T("year"), CString( pInfo.szYear, 4 ) );

	if ( pInfo.nGenre < ID3_GENRES )
		pXML->AddAttribute( _T("genre"), pszID3Genre[ pInfo.nGenre ] );

	if ( pInfo.szComment[28] == 0 && pInfo.szComment[29] > 0 )
	{
		CString strTrack;
		strTrack.Format( _T("%i"), static_cast< int >( pInfo.szComment[29] ) );
		pXML->AddAttribute( _T("track"), strTrack );
		CopyID3v1Field( pXML, _T("description"), CString( pInfo.szComment, 28 ) );
	}
	else
		CopyID3v1Field( pXML, _T("description"), CString( pInfo.szComment, 30 ) );

	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );

	if ( bIsMP3 )
	{
		ScanMP3Frame( pXML, hFile, sizeof(pInfo) );
		return LibraryBuilder.SubmitMetadata( nIndex, CSchema::uriAudio, pXML ) != 0;
	}

	return pXML->GetAttributeCount() > 0;
}

bool CLibraryBuilderInternals::CopyID3v1Field(CXMLElement* pXML, LPCTSTR pszAttribute, CString strValue)
{
	strValue.Trim();
	if ( strValue.IsEmpty() )
		return false;

	strValue.Replace( _T("\r\n"), _T("; ") );	// Windows style replacement
	strValue.Replace( _T("\n"), _T("; ") );		// Unix style replacement
	strValue.Replace( _T("\r"), _T("; ") );		// Mac style replacement

	pXML->AddAttribute( pszAttribute, strValue );

	return true;
}

int CLibraryBuilderInternals::LookupID3v1Genre(const CString& strGenre) const
{
	// Loop through all the genres
	for ( int nGenre = 0 ; nGenre < ID3_GENRES ; nGenre ++ )
	{
		// Compare with listed genre
		if ( strGenre.CompareNoCase( pszID3Genre[ nGenre ] ) == 0 )
		{
			// Return matching genre
			return nGenre;
		}
	}

	// Return no match found
	return -1;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals ID3v2 (threaded)

bool CLibraryBuilderInternals::ReadID3v2(DWORD nIndex, HANDLE hFile)
{
	ID3V2_HEADER pHeader;
	DWORD nRead;

	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	ReadFile( hFile, &pHeader, sizeof(pHeader), &nRead, NULL );
	if ( nRead != sizeof(pHeader) )
		return false;

	if ( strncmp( pHeader.szTag, ID3V2_TAG, 3 ) )
		return false;
	if ( pHeader.nMajorVersion < 2 || pHeader.nMajorVersion > 4 )
		return false;
	if ( pHeader.nFlags & ~ID3V2_KNOWNMASK )
		return false;
	if ( pHeader.nFlags & ID3V2_UNSYNCHRONISED )
		return false;

	DWORD nBuffer = swapEndianess( pHeader.nSize );
	ID3_DESYNC_SIZE( nBuffer );

	if ( nBuffer > 1024 * 1024 * 2 )
		return false;

	auto_array< BYTE > pRelease( new BYTE[ nBuffer ] );
	BYTE* pBuffer = pRelease.get();

	ReadFile( hFile, pBuffer, nBuffer, &nRead, NULL );
	if ( nRead != nBuffer )
		return false;

	if ( ( pHeader.nFlags & ID3V2_EXTENDEDHEADER ) && pHeader.nMajorVersion == 3 )
	{
		if ( nBuffer < sizeof(ID3V2_EXTENDED_HEADER_1) )
			return false;

		ID3V2_EXTENDED_HEADER_1* pExtended = (ID3V2_EXTENDED_HEADER_1*)pBuffer;
		pBuffer += sizeof(ID3V2_EXTENDED_HEADER_1);
		nBuffer -= sizeof(ID3V2_EXTENDED_HEADER_1);

		pExtended->nSize = swapEndianess( pExtended->nSize );

		if ( nBuffer < pExtended->nSize )
			return false;

		pBuffer += pExtended->nSize;
		nBuffer -= pExtended->nSize;
	}
	else if ( ( pHeader.nFlags & ID3V2_EXTENDEDHEADER ) && pHeader.nMajorVersion == 4 )
	{
		if ( nBuffer < sizeof(ID3V2_EXTENDED_HEADER_2) )
			return false;

		ID3V2_EXTENDED_HEADER_2* pExtended = (ID3V2_EXTENDED_HEADER_2*)pBuffer;
		pBuffer += sizeof(ID3V2_EXTENDED_HEADER_2);
		nBuffer -= sizeof(ID3V2_EXTENDED_HEADER_2);

		pExtended->nSize = swapEndianess( pExtended->nSize );
		ID3_DESYNC_SIZE( pExtended->nSize );
		pExtended->nSize -= 6;

		if ( nBuffer < pExtended->nSize )
			return false;

		pBuffer += pExtended->nSize;
		nBuffer -= pExtended->nSize;
	}

	auto_ptr< CXMLElement > pXML( new CXMLElement( NULL, _T("audio") ) );
	bool bBugInFrameSize = false;

	while ( nBuffer )
	{
		DWORD nFrameSize = 0;
		CHAR szFrameTag[5];

		if ( pHeader.nMajorVersion > 2 )
		{
			ID3V2_FRAME* pFrame = (ID3V2_FRAME*)pBuffer;

			if ( nBuffer < sizeof(*pFrame) )
				break;
			pBuffer += sizeof(*pFrame);
			nBuffer -= sizeof(*pFrame);

			szFrameTag[0] = pFrame->szID[0];
			szFrameTag[1] = pFrame->szID[1];
			szFrameTag[2] = pFrame->szID[2];
			szFrameTag[3] = pFrame->szID[3];
			szFrameTag[4] = 0;

			nFrameSize = swapEndianess( pFrame->nSize );

			if ( pHeader.nMajorVersion >= 4 && !bBugInFrameSize )
			{
				ID3_DESYNC_SIZE( nFrameSize );
				if ( nBuffer < nFrameSize )
					break;
				// iTunes uses old style of size for v.2.4 when converting.
				// TODO: Add a code here to find the correct frame size?
				// Report and solution: http://www.sacredchao.net/quodlibet/ticket/180
			}
			if ( pFrame->nFlags2 & ~ID3V2_KNOWNFRAME )
				szFrameTag[0] = 0;
		}
		else
		{
			ID3V2_FRAME_2* pFrame = (ID3V2_FRAME_2*)pBuffer;

			if ( nBuffer < sizeof(*pFrame) )
				break;
			pBuffer += sizeof(*pFrame);
			nBuffer -= sizeof(*pFrame);

			szFrameTag[0] = pFrame->szID[0];
			szFrameTag[1] = pFrame->szID[1];
			szFrameTag[2] = pFrame->szID[2];
			szFrameTag[3] = szFrameTag[4] = 0;
			nFrameSize = ( pFrame->nSize[0] << 16 ) | ( pFrame->nSize[1] << 8 ) | pFrame->nSize[2];
		}

		if ( nBuffer < nFrameSize || !szFrameTag[0] )
			break;

		if ( strcmp( szFrameTag, "TIT2" ) == 0 || strcmp( szFrameTag, "TT2" ) == 0)
		{
			CopyID3v2Field( pXML.get(), _T("title"), pBuffer, nFrameSize );
		}
		else if ( strcmp( szFrameTag, "TPE1" ) == 0 || strcmp( szFrameTag, "TP1" ) == 0 || strcmp( szFrameTag, "TPE2" ) == 0 || strcmp( szFrameTag, "TP2" ) == 0 )
		{
			CopyID3v2Field( pXML.get(), _T("artist"), pBuffer, nFrameSize );
		}
		else if ( strcmp( szFrameTag, "TOPE" ) == 0 || strcmp( szFrameTag, "TOA" ) == 0 )
		{
			CopyID3v2Field( pXML.get(), _T("origArtist"), pBuffer, nFrameSize );
		}
		else if ( strcmp( szFrameTag, "TALB" ) == 0 || strcmp( szFrameTag, "TAL" ) == 0 )
		{
			CopyID3v2Field( pXML.get(), _T("album"), pBuffer, nFrameSize );
		}
		else if ( strcmp( szFrameTag, "TOAL" ) == 0 || strcmp( szFrameTag, "TOT" ) == 0 )
		{
			CopyID3v2Field( pXML.get(), _T("origAlbum"), pBuffer, nFrameSize );
		}
		else if ( strcmp( szFrameTag, "TRCK" ) == 0 || strcmp( szFrameTag, "TRK" ) == 0 )
		{
			CopyID3v2Field( pXML.get(), _T("track"), pBuffer, nFrameSize );
		}
		else if ( pHeader.nMajorVersion < 4 &&
			( strcmp( szFrameTag, "TYER" ) == 0 || strcmp( szFrameTag, "TYE" ) == 0 ) )
		{
			CopyID3v2Field( pXML.get(), _T("year"), pBuffer, nFrameSize );
		}
		else if ( strcmp( szFrameTag, "COMM" ) == 0 || strcmp( szFrameTag, "COM" ) == 0 )
		{
			if ( CopyID3v2Field( pXML.get(), _T("description"), pBuffer, nFrameSize, true ) )
			{
				if ( CXMLAttribute* pDescr = pXML->GetAttribute( _T("description") ) )
				{
					CString strDescr = pDescr->GetValue();

					// Remove iTune crap
					if ( strDescr.GetLength() && _tcsncmp( strDescr, L"iTunNORM", 8 ) == 0 )
						pXML->RemoveAttribute( pDescr );
				}
			}
		}
		else if ( strcmp( szFrameTag, "TLEN" ) == 0 || strcmp( szFrameTag, "TLE" ) == 0 )
		{
			if ( CopyID3v2Field( pXML.get(), _T("seconds"), pBuffer, nFrameSize ) )
			{
				CString strMS = pXML->GetAttributeValue( _T("seconds"), _T("0") );
				int nMS;
				_stscanf( strMS, _T("%lu"), &nMS );
				strMS.Format( _T("%lu"), nMS / 1000 );
				pXML->AddAttribute( _T("seconds"), strMS );
			}
		}
		else if ( strcmp( szFrameTag, "TCOP" ) == 0 || strcmp( szFrameTag, "TCR" ) == 0 )
		{
			CopyID3v2Field( pXML.get(), _T("copyright"), pBuffer, nFrameSize );
		}
		else if ( strcmp( szFrameTag, "TCON" ) == 0 || strcmp( szFrameTag, "TCO" ) == 0 )
		{
			if ( CopyID3v2Field( pXML.get(), _T("genre"), pBuffer, nFrameSize ) )
			{
				CString strGenre = pXML->GetAttributeValue( _T("genre"), _T("") );

				for ( ;; )
				{
					int nPos1 = strGenre.Find( '(' );
					if ( nPos1 < 0 )
						break;
					int nPos2 = strGenre.Find( ')' );
					if ( nPos2 <= nPos1 )
						break;

					CString strValue = strGenre.Mid( nPos1 + 1, nPos2 - nPos1 - 1 );
					int nGenre = 0;

					if ( strValue.CompareNoCase( _T("RX") ) == 0 )
					{
						strValue = _T("Remix");
					}
					else if ( strValue.CompareNoCase( _T("CR") ) == 0 )
					{
						strValue = _T("Cover");
					}
					else if ( _stscanf( strValue, _T("%i"), &nGenre ) == 1 && nGenre < ID3_GENRES )
					{
						if ( _tcsistr( strGenre, pszID3Genre[ nGenre ] ) == NULL )
						{
							strValue = pszID3Genre[ nGenre ];
						}
						else
						{
							strValue.Empty();
						}
					}
					else
					{
						strValue = _T("[") + strValue + _T("]");
					}

					strGenre = strGenre.Left( nPos1 ) + strValue + strGenre.Mid( nPos2 + 1 );
				}

				strGenre.Replace( _T("["), _T("(") );
				strGenre.Replace( _T("]"), _T(")") );

				pXML->AddAttribute( _T("genre"), strGenre );
			}
		}
		else if ( strcmp( szFrameTag, "TENC" ) == 0 || strcmp( szFrameTag, "TEN" ) == 0 )
		{
			CopyID3v2Field( pXML.get(), _T("encodedby"), pBuffer, nFrameSize );
		}
		else if ( strcmp( szFrameTag, "TSSE" ) == 0 || strcmp( szFrameTag, "TSS" ) == 0 )
		{
			CopyID3v2Field( pXML.get(), _T("encodedby"), pBuffer, nFrameSize );
		}
		else if ( strcmp( szFrameTag, "TCOM" ) == 0 || strcmp( szFrameTag, "TCM" ) == 0 )
		{
			CopyID3v2Field( pXML.get(), _T("composer"), pBuffer, nFrameSize );
		}
		else if ( strcmp( szFrameTag, "WXXX" ) == 0 || strcmp( szFrameTag, "WXX" ) == 0 )
		{
			CopyID3v2Field( pXML.get(), _T("link"), pBuffer, nFrameSize );
		}
		else if ( pHeader.nMajorVersion == 4 && strcmp( szFrameTag, "TDRC" ) == 0 )
		{
			BYTE* pScan = pBuffer;
			DWORD nLength = nFrameSize;
			for ( ; *pScan != '-' && nLength > 0 ; nLength-- )
				pScan++;
			nLength = nFrameSize - nLength;
			auto_array< BYTE > pszYear( new BYTE[ nLength + 1 ] );
			memcpy( pszYear.get(), pBuffer, nLength );
			CopyID3v2Field( pXML.get(), _T("year"), pszYear.get(), nLength );
		}
		else if ( strcmp( szFrameTag, "TPUB" ) == 0 )
		{
			CopyID3v2Field( pXML.get(), _T("publisher"), pBuffer, nFrameSize );
		}
		else if ( strcmp( szFrameTag, "TORY" ) == 0 )
		{
			CopyID3v2Field( pXML.get(), _T("origYear"), pBuffer, nFrameSize );
		}
		// User defined text information frame
		else if ( strcmp( szFrameTag, "TXXX" ) == 0 )
		{
			CopyID3v2Field( pXML.get(), NULL, pBuffer, nFrameSize );
		}

		pBuffer += nFrameSize;
		nBuffer -= nFrameSize;
	}

	ScanMP3Frame( pXML.get(), hFile, 0 );

	return LibraryBuilder.SubmitMetadata( nIndex, CSchema::uriAudio, pXML.release() ) != 0;
}

bool CLibraryBuilderInternals::CopyID3v2Field(CXMLElement* pXML, LPCTSTR pszAttribute, BYTE* pBuffer, DWORD nLength, bool bSkipLanguage)
{
	CString strResult, strValue;

	BYTE nEncoding = *pBuffer++;
	nLength--;

	if ( bSkipLanguage )
	{
		if ( nLength < 3 )
			return false;
		pBuffer += 3;
		nLength -= 3;
		if ( nLength > 0 && pBuffer[ 0 ] == 0 )
		{
			pBuffer += 1;
			nLength -= 1;
		}
	}

	DWORD nOffset = 0;

	while ( nOffset < nLength )
	{
		if ( nEncoding == 0 )
		{
			LPTSTR pszOutput = strValue.GetBuffer( nLength - nOffset + 1 );

			DWORD nOut = 0, nChar = 0;
			for ( ; nChar < nLength - nOffset ; nChar++, nOut++ )
			{
				pszOutput[ nOut ] = (TCHAR)pBuffer[ nOffset + nChar ];
				if ( pszOutput[ nOut ] == 0 )
				{
					nOffset += nOut + 1;
					break;
				}
			}
			strValue.ReleaseBuffer( nOut );
			if ( nChar == nLength - nOffset )
				nOffset += nLength - nOffset;
		}
		else if ( nEncoding == 1 && ( ( nLength - nOffset ) & 1 ) == 0 && nLength - nOffset >= 2 )
		{
			DWORD nNewLength = ( nLength - nOffset - 2 ) / 2;
			LPTSTR pszOutput = strValue.GetBuffer( nNewLength + 1 );

			if ( pBuffer[0] == 0xFF && pBuffer[1] == 0xFE )
			{
				pBuffer += 2;
				DWORD nOut = 0, nChar = 0;
				for ( ; nChar < nNewLength ; nChar++, nOut++ )
				{
					pszOutput[ nOut ] = (TCHAR)pBuffer[ nOffset + nChar*2+0 ] | ( (TCHAR)pBuffer[ nOffset + nChar*2+1 ] << 8 );
					if ( pszOutput[ nOut ] == 0 )
					{
						nOffset += ( nOut + 1 ) * 2;
						break;
					}
				}
				strValue.ReleaseBuffer( nOut );
				pBuffer -= 2;
				if ( nChar == nNewLength )
					nOffset += nLength - nOffset;
			}
			else if ( pBuffer[0] == 0xFE && pBuffer[1] == 0xFF )
			{
				pBuffer += 2;
				DWORD nOut = 0, nChar = 0;
				for ( ; nChar < nNewLength ; nChar++, nOut++ )
				{
					pszOutput[ nOut ] = (TCHAR)pBuffer[ nOffset + nChar*2+1 ] | ( (TCHAR)pBuffer[ nOffset + nChar*2+0 ] << 8 );
					if ( pszOutput[ nOut ] == 0 )
					{
						nOffset += ( nOut + 1 ) * 2;
						break;
					}
				}
				strValue.ReleaseBuffer( nOut );
				pBuffer -= 2;
				if ( nChar == nNewLength )
					nOffset += nLength - nOffset;
			}
			else
			{
				strValue.ReleaseBuffer( 0 );
				return false;
			}
		}
		else if ( nEncoding == 2 && ( ( nLength - nOffset ) & 1 ) == 0 )
		{
			DWORD nNewLength = ( nLength - nOffset ) / 2;
			LPTSTR pszOutput = strValue.GetBuffer( nNewLength + 1 );

			DWORD nOut = 0, nChar = 0;
			for ( ; nChar < nNewLength ; nChar++, nOut++ )
			{
				pszOutput[ nOut ] = (TCHAR)pBuffer[ nOffset + nChar*2+1 ] | ( (TCHAR)pBuffer[ nOffset + nChar*2+0 ] << 8 );
				if ( pszOutput[ nOut ] == 0 )
				{
					nOffset += ( nOut + 1 ) * 2;
					break;
				}
			}
			if ( nChar == nNewLength )
				nOffset += nLength - nOffset;

			strValue.ReleaseBuffer( nOut );
		}
		else if ( nEncoding == 3 )
		{
			strValue = UTF8Decode( (LPCSTR)pBuffer + nOffset, nLength - nOffset );
			nOffset += (DWORD)strnlen( (LPCSTR)pBuffer + nOffset, nLength - nOffset ) + 1;
		}

		strValue.Trim();
		strValue.Replace( L"\r\n", L"; " ); // Windows style replacement
		strValue.Replace( L"\n", L"; " ); // Unix style replacement
		strValue.Replace( L"\r", L"; " ); // Mac style replacement

		if ( strResult.IsEmpty() && ( strValue.IsEmpty() || _tcslen( strValue ) == 0 ) )
			return false;
		else if ( strResult.GetLength() && strValue.GetLength() )
		{
			strResult += '/';
			strResult.Append( strValue );
		}
		else if ( strValue.GetLength() )
		{
			strResult = strValue;
		}
		else
			break;
	}

	if ( !pszAttribute )
	{
		int nSlash = strResult.Find( '/' );
		if ( nSlash > 0 )
		{
			strValue = strResult.Mid( nSlash + 1 + ( ( nEncoding == 1 ) ? 1 : 0 ) );
			if ( strValue.IsEmpty() )
				return false;

			if ( _tcsnicmp( strResult, L"musicbrainz ", 12 ) == 0 )
			{
				CString strField = strResult.Mid( 12, nSlash - 12 );
				if ( strField.CompareNoCase( L"Artist Id" ) == 0 )
					pXML->AddAttribute( L"mbartistid", strValue );
				else if ( strField.CompareNoCase( L"Album Id" ) == 0 )
					pXML->AddAttribute( L"mbalbumid", strValue );
				else if ( strField.CompareNoCase( L"Album Type" ) == 0 )
					pXML->AddAttribute( L"type", strValue );
				else if ( strField.CompareNoCase( L"Album Status" ) == 0 )
					pXML->AddAttribute( L"albumStatus", strValue );
				else if ( strField.CompareNoCase( L"Album Artist Id" ) == 0 )
					pXML->AddAttribute( L"mbalbumartistid", strValue );

				// "Album Artist", "Album Artist Sortname", "Album Release Country", "Non-Album"
				// ToDo: find field names for mbtrmid, mbuniquefileid and cddb
				return true;
			}
			else if ( _tcsnicmp( strResult, L"musicip ", 8 ) == 0 )
			{
				CString strField = strResult.Mid( 8, nSlash - 8 );
				if ( strField.CompareNoCase( L"PUID" ) == 0 )
					pXML->AddAttribute( L"mbpuid", strValue );

				return true;
			}
			else
			{
				// Unsupported user text frame
				return false;
			}
		}
	}

	pXML->AddAttribute( pszAttribute, strResult );

	return true;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals MP3 scan (threaded)

bool CLibraryBuilderInternals::ReadMP3Frames(DWORD nIndex, HANDLE hFile)
{
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );

	auto_ptr< CXMLElement > pXML( new CXMLElement( NULL, _T("audio") ) );

	if ( ! ScanMP3Frame( pXML.get(), hFile, 0 ) )
		return false;

	return LibraryBuilder.SubmitMetadata( nIndex, CSchema::uriAudio, pXML.release() ) != 0;
}

//
// Refer to this doc: http://www.mp3-tech.org/programmer/frame_header.html
//
bool CLibraryBuilderInternals::ScanMP3Frame(CXMLElement* pXML, HANDLE hFile, DWORD nIgnore)
{
	// Bitrate index
	static DWORD nBitrateTable[16][5] =
	{
		// L1 - Layer I
		// L2 - Layer II
		// L3 - Layer III
		// V1 - MPEG Version 1
		// V2 - MPEG Version 2 and Version 2.5

		// Row data:
		// V1,L1	V1,L2	V1,L3  	V2,L1	V2, L2 & L3
		// The first row with zeros is for "free" bitrate (the constant bitrate not higher than value)
		{ 0, 0, 0, 0, 0 },				{ 32, 32, 32, 32, 8 },		{ 64, 48, 40, 48, 16 },
		{ 96, 56, 48, 56, 24 },			{ 128, 64, 56, 64, 32 },	{ 160, 80, 64, 80, 40 },
		{ 192, 96, 80, 96, 48 },		{ 224, 112, 96, 112, 56 },	{ 256, 128, 112, 128, 64 },
		{ 288, 160, 128, 144, 80 },		{ 320, 192, 160, 160, 96 },	{ 352, 224, 192, 176, 112 },
		{ 384, 256, 224, 192, 128 },	{ 416, 320, 256, 224, 144 },{ 448, 384, 320, 256, 160 },
		{ 0, 0, 0, 0, 0 }
		// The last row with zeros is for "bad" bitrate (no value)
	};

	// Sampling rate frequency index
	static DWORD nFrequencyTable[4][4] =
	{
		// Row data:
		// MPEG1	?	MPEG2  	MPEG2.5
		{ 11025, 0, 22050, 44100 },
		{ 12000, 0,  24000, 48000 },
		{ 8000, 0, 16000, 32000 },
		{ 0, 0, 0, 0 }
	};

	static int nChannelTable[4]		= { 2, 2, 2, 1 };
	static CString strSoundType[4]	= { "Stereo", "Joint Stereo", "Dual Channel", "Single Channel" };

	BYTE nLayer					= 0;
	bool bVariable				= false;
	__int64 nTotalBitrate		= 0;
	DWORD nBaseBitrate			= 0;
	DWORD nBaseFrequency		= 0;
	int nBaseChannel			= 0;
	CString strBaseSoundType;
	DWORD nFrameCount			= 0;
	DWORD nFrameSize			= 0;
	DWORD nHeader				= 0;

	DWORD nRead;
	ReadFile( hFile, &nHeader, 4, &nRead, NULL );

	if ( nRead != 4 )
		return false;

	nHeader = swapEndianess( nHeader );

	for ( DWORD nSeek = 0 ; bVariable || ( nFrameCount < 16 && nSeek < 4096 * 2  ) ; nSeek++ )
	{
		// "frame sync"
		// First 11 bits must have bit 1 for MPEG 2.5 extension
		// For other versions--first 12 bits

		if ( ( nHeader & 0xFFE00000 ) == 0xFFE00000 )
		{
			// Version: MPEG Audio version ID
			// 0: MPEG Version 2.5 (later extension of MPEG 2)
			// 1: reserved
			// 2: MPEG Version 2 (ISO/IEC 13818-3)
			// 3: MPEG Version 1 (ISO/IEC 11172-3)

			// We are taking 2 bits at position 20-21 (or 19-20 counting from 0)
			// hex 0x00180000 = binary 110000000000000000000
			BYTE nVersion	= (BYTE)( ( nHeader & 0x00180000 ) >> 19 );
			// Layer description:
			// 0: reserved
			// 1: L3
			// 2: L2
			// 3: L1
			nLayer			= (BYTE)( ( nHeader & 0x00060000 ) >> 17 );	// 0110 0000 0000 0000 0000
			BYTE nBitIndex	= (BYTE)( ( nHeader & 0x0000F000 ) >> 12 );	//      1111 0000 0000 0000
			BYTE nFreqIndex	= (BYTE)( ( nHeader & 0x00000C00 ) >> 10 );	//           1100 0000 0000
			bool bPadding	= ( nHeader & 0x00000200 ) ? true : false;	//           0010 0000 0000
			BYTE nChannels	= (BYTE)( ( nHeader & 0x000000C0 ) >> 6 );	//                1100 0000
			bool bCopyRight = ( nHeader & 0x00000008 ) ? true: false;	//                     1000
			UNUSED_ALWAYS( bCopyRight );

			int nBitColumn = 0;

			if ( nVersion == 3 )
			{
				if ( nLayer == 3 )
					nBitColumn = 0;
				else if ( nLayer == 2 )
					nBitColumn = 1;
				else if ( nLayer == 1 )
					nBitColumn = 2;
			}
			else
			{
				if ( nLayer == 3 )
					nBitColumn = 3;
				else
					nBitColumn = 4;
			}

			DWORD nBitrate		= nBitrateTable[ nBitIndex ][ nBitColumn ] * 1000;
			DWORD nFrequency	= nFrequencyTable[ nFreqIndex ][ nVersion ];

			if ( !nFrequency )
				return false;

			if ( nBaseBitrate )
			{
				if ( nBaseBitrate != nBitrate )
					bVariable = true;
			}
			else
			{
				nBaseBitrate	= nBitrate;
				nBaseFrequency	= nFrequency;
			}

			nBaseChannel = nChannelTable[nChannels];
			strBaseSoundType = strSoundType[nChannels];

			nFrameSize = ( nLayer == 3 ) ? ( 12 * nBitrate / nFrequency + bPadding ) * 4
				: ( 144 * nBitrate / nFrequency + bPadding );

			if ( !nFrameSize )
				return false;

			// Skip frame when it has reserved layer
			if ( nLayer )
			{
				nTotalBitrate += nBitrate / 1000;
				nFrameCount++;
			}
			else if ( nFrameCount == 0 ) // reset base values if it was the first frame
			{
				nBaseBitrate = nBaseFrequency = 0;
			}

			SetFilePointer( hFile, nFrameSize - 4, NULL, FILE_CURRENT );
			ReadFile( hFile, &nHeader, 4, &nRead, NULL );
			if ( nRead != 4 )
				break;
			nHeader = swapEndianess( nHeader );
		}
		else
		{
			nHeader <<= 8;
			ReadFile( hFile, &nHeader, 1, &nRead, NULL );
			if ( nRead != 1 )
				break;
		}
	}

	if ( nFrameCount < 16 || !nFrameSize )
		return false;

	if ( bVariable )
	{
		nBaseBitrate = (DWORD)( nTotalBitrate / nFrameCount ) * 1000;
	}
	else
	{
		DWORD dwFilePosition	= SetFilePointer( hFile, 0, NULL, FILE_CURRENT );
		DWORD dwFileSize		= GetFileSize( hFile, NULL );
		DWORD dwMusicSize		= dwFileSize - dwFilePosition - nIgnore + 4;
		nFrameCount += ( dwMusicSize / nFrameSize ) - 1;
	}

	DWORD nFrameTime	= ( nLayer == 3 ? 384 : 1152 ) * 100000 / nBaseFrequency;
	DWORD nTotalTime	= (DWORD)( (__int64)nFrameCount * (__int64)nFrameTime / 100000 );

	CString strValue;

	strValue.Format( bVariable ? _T("%lu~") : _T("%lu"), nBaseBitrate / 1000 );
	pXML->AddAttribute( _T("bitrate"), strValue );

	strValue.Format( _T("%lu"), nTotalTime );
	pXML->AddAttribute( _T("seconds"), strValue );

	strValue.Format( _T("%lu"), nBaseFrequency );
	pXML->AddAttribute( _T("sampleRate"), strValue );

	strValue.Format( _T("%lu"), nBaseChannel );
	pXML->AddAttribute( _T("channels"), strValue );

	pXML->AddAttribute( _T("soundType"), strBaseSoundType );

	return true;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals version information (threaded)

bool CLibraryBuilderInternals::ReadVersion(DWORD nIndex, LPCTSTR pszPath)
{
	DWORD dwSize = GetFileVersionInfoSize( (LPTSTR)pszPath, &dwSize );
	if ( dwSize <= 152 )
		return false;

	auto_array< BYTE > pBuffer( new BYTE[ dwSize ] );

	if ( !GetFileVersionInfo( (LPTSTR)pszPath, NULL, dwSize, pBuffer.get() ) )
		return false;

	DWORD nLangId = GetBestLanguageId( pBuffer.get() );

	auto_ptr< CXMLElement > pXML( new CXMLElement( NULL, _T("application") ) );

	pXML->AddAttribute( _T("os"), _T("Windows") );

	bool bOur = false;
	bOur |= CopyVersionField( pXML.get(), _T("title"), pBuffer.get(), _T("ProductName"), nLangId );
			CopyVersionField( pXML.get(), _T("version"), pBuffer.get(), _T("ProductVersion"), nLangId, true );
	bOur |= CopyVersionField( pXML.get(), _T("fileDescription"), pBuffer.get(), _T("FileDescription"), nLangId );
			CopyVersionField( pXML.get(), _T("fileVersion"), pBuffer.get(), _T("FileVersion"), nLangId, true );
	bOur |= CopyVersionField( pXML.get(), _T("originalFileName"), pBuffer.get(), _T("OriginalFilename"), nLangId );
	bOur |= CopyVersionField( pXML.get(), _T("company"), pBuffer.get(), _T("CompanyName"), nLangId );
	bOur |= CopyVersionField( pXML.get(), _T("copyright"), pBuffer.get(), _T("LegalCopyright"), nLangId );
	bOur |= CopyVersionField( pXML.get(), _T("comments"), pBuffer.get(), _T("comments"), nLangId );

	//LPCTSTR pszExt = PathFindExtension( pszPath );
	//if ( bOur && pszExt && _tcscmp( pszExt, L".exe" ) == 0 /*&& ValidateManifest( pszPath )*/ )
	//{
	//	// TODO: mark the file as validated OR otherwise submit corrupted OR delete metadata?
	//}

	return LibraryBuilder.SubmitMetadata( nIndex, CSchema::uriApplication, pXML.release() ) != 0;
}

// Return true if word "Shareaza" was found
bool CLibraryBuilderInternals::CopyVersionField(CXMLElement* pXML, LPCTSTR pszAttribute, BYTE* pBuffer,
												LPCTSTR pszKey, DWORD nLangId, bool bCommaToDot)
{
	CString strValue = GetVersionKey( pBuffer, pszKey, nLangId );

	if ( strValue.IsEmpty() )
		return false;

	if ( bCommaToDot )
	{
		for ( int nPos = -1 ; ( nPos = strValue.Find( _T(", ") ) ) >= 0 ; )
		{
			strValue = strValue.Left( nPos ) + '.' + strValue.Mid( nPos + 2 );
		}
	}

	pXML->AddAttribute( pszAttribute, strValue );

	int nPos = strValue.Find( L"Shareaza" );
	bool bOurProduct = false;
	if ( nPos != -1 )
	{
		bool bStartOk = nPos == 0, bEndOk = strValue.GetLength() == nPos + 8;
		if ( !bStartOk && strValue.GetAt( nPos - 1 ) == ' ' )
		{
			bStartOk = true;
		}
		if ( !bEndOk && strValue.GetLength() > nPos + 8 && strValue.GetAt( nPos + 8 ) != ' ' )
		{
			// ShareazaPlus etc.
		}
		else
			bEndOk = true;

		bOurProduct = bStartOk && bEndOk;
	}

	return bOurProduct;
}

CString CLibraryBuilderInternals::GetVersionKey(BYTE* pBuffer, LPCTSTR pszKey, DWORD nLangId)
{
	CString strKey, strValue;

	strKey.Format( L"\\StringFileInfo\\%04x%04x\\", nLangId & 0x0000FFFF, ( nLangId & 0xFFFF0000 ) >> 16 );
	strKey += pszKey;

	BYTE* pValue = NULL;
	DWORD dwSize = 0;

	if ( !VerQueryValue( pBuffer, (LPTSTR)(LPCTSTR)strKey, (void**)&pValue, (UINT*)&dwSize ) )
		return strValue;

	strValue = (LPCTSTR)pValue;

	return strValue.Trim();
}

DWORD CLibraryBuilderInternals::GetBestLanguageId(LPVOID pBuffer)
{
	DWORD nLangCode = 0;
	UINT nLength = 0;
	LPVOID	pTranslation = NULL;

	VerQueryValue( pBuffer, L"\\VarFileInfo\\Translation", &pTranslation, &nLength );

	if ( pTranslation == NULL ) // No Translation block is available
	{
		VerQueryValue( pBuffer, L"\\StringFileInfo\\", &pTranslation, &nLength );
		if ( pTranslation != NULL )
		{
			WCHAR* pLanguage = (WCHAR*)pTranslation + 3;
			if ( wcslen( pLanguage ) != 8 )
				return 0;
			else
			{
				DWORD nSublang = 0;
				// Read the langid just after StringFileInfo block
				swscanf( pLanguage, L"%4x%4x", &nLangCode, &nSublang );
				nLangCode += ( nSublang << 16 );
				return nLangCode;
			}
		}
		else
			return 0;
	}
	// ToDo: get LANGID of the Shareaza user interface
	if ( !GetLanguageId( pTranslation, nLength, GetUserDefaultLangID(), nLangCode, false ) )
	{
		if ( !GetLanguageId( pTranslation, nLength, GetSystemDefaultLangID(), nLangCode, true ) )
		{
			if ( !GetLanguageId( pTranslation, nLength, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), nLangCode, true ) )
			{
				if ( !GetLanguageId( pTranslation, nLength, MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL), nLangCode, true ) )
					nLangCode = *(DWORD*)pTranslation; // Use the first one
			}
		}
	}
	return nLangCode;
}

bool CLibraryBuilderInternals::GetLanguageId(LPVOID pBuffer, UINT nSize, WORD nLangId, DWORD &nId, bool bOnlyPrimary)
{
	LPWORD pData = NULL;
	for ( pData = (LPWORD)pBuffer ; (LPBYTE)pData < ( (LPBYTE)pBuffer ) + nSize ; pData += 2 )
	{
		if ( *pData == nLangId )
		{
			nId = *(DWORD*)pData;
			return true;
		}
	}

	if ( !bOnlyPrimary )
		return false;

	for ( pData = (LPWORD)pBuffer ; (LPBYTE)pData < ( (LPBYTE)pBuffer ) + nSize ; pData += 2 )
	{
		if ( ( *pData & 0x00FF ) == ( nLangId & 0x00FF ) )
		{
			nId = *(DWORD*)pData;
			return true;
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals Shareaza product validation (threaded)
bool CLibraryBuilderInternals::ValidateManifest(LPCTSTR pszPath)
{
	HMODULE hExe = LoadLibrary( pszPath );
	if ( hExe == NULL )
		return false;

	HRSRC hRes = FindResource( hExe, MAKEINTRESOURCE(1), RT_MANIFEST );
	if ( hRes == NULL )
	{
		FreeLibrary( hExe );
		return false;
	}

	HGLOBAL hGlobal = LoadResource( hExe, hRes );
	if ( hGlobal == NULL )
	{
		FreeLibrary( hExe );
		return false;
	}

	DWORD nLength = SizeofResource( hExe, hRes );
	LPCSTR pData = (LPCSTR)LockResource( hGlobal );
	if ( pData == NULL )
	{
		FreeResource( hGlobal );
		FreeLibrary( hExe );
		return false;
	}

	CString strData( pData, nLength );
	UnlockResource( hGlobal );

	bool bValid = false;
	if ( strData.GetLength() )
	{
		//CXMLElement* pElement = CXMLElement::FromString( (LPCTSTR)strData );
		//// Perform validation here

		//delete pElement;
		bValid = true;
	}

	FreeResource( hGlobal );
	FreeLibrary( hExe );

	return bValid;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals MSI (threaded)

bool CLibraryBuilderInternals::ReadMSI(DWORD nIndex, LPCTSTR pszPath)
{
	PMSIHANDLE hSummaryInfo;

	int nError = MsiGetSummaryInformation( NULL, pszPath, 0, &hSummaryInfo );

	if ( nError == ERROR_INSTALL_PACKAGE_INVALID )
		return LibraryBuilder.SubmitCorrupted( nIndex );
	else if ( nError != ERROR_SUCCESS )
		return false;

	auto_ptr< CXMLElement > pXML( new CXMLElement( NULL, _T("application") ) );

	pXML->AddAttribute( _T("os"), _T("Windows") );

	CString strSubject;
	CString str = GetSummaryField( hSummaryInfo, PID_TITLE );
	if ( str.IsEmpty() || str == _T("Installation Database") )
	{
		str = GetSummaryField( hSummaryInfo, PID_SUBJECT );
	}
	else
	{
		strSubject = GetSummaryField( hSummaryInfo, PID_SUBJECT );
		if ( strSubject != str )
			pXML->AddAttribute( _T("fileDescription"), strSubject );
	}

	pXML->AddAttribute( _T("title"), str );
	pXML->AddAttribute( _T("company"), GetSummaryField( hSummaryInfo, PID_AUTHOR ) );

	str = GetSummaryField( hSummaryInfo, PID_KEYWORDS );
	if ( str.Find( _T("Installer,MSI,Database") ) == -1  )
	{
		pXML->AddAttribute( _T("keywords"), str );
	}

	str = GetSummaryField( hSummaryInfo, PID_COMMENTS );
	if ( str != strSubject )
	{
		pXML->AddAttribute( _T("releaseNotes"), str );
	}

	return LibraryBuilder.SubmitMetadata( nIndex, CSchema::uriApplication, pXML.release() ) != 0;
}

CString CLibraryBuilderInternals::GetSummaryField(MSIHANDLE hSummaryInfo, UINT nProperty)
{
	CString strValue;
	UINT nPropType = VT_LPSTR;
	DWORD dwSize = 0;

	if ( MsiSummaryInfoGetProperty( hSummaryInfo, nProperty, &nPropType,
		0, NULL, L"", &dwSize ) == ERROR_MORE_DATA )
	{
		dwSize++;
		MsiSummaryInfoGetProperty( hSummaryInfo, nProperty, &nPropType,
			0, NULL, strValue.GetBuffer( dwSize ), &dwSize );
		strValue.ReleaseBuffer();
	}

	return strValue;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals JPEG (threaded)

bool CLibraryBuilderInternals::ReadJPEG(DWORD nIndex, HANDLE hFile)
{
	DWORD nRead	= 0;
	WORD wMagic	= 0;
	BYTE nByte	= 0;

	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	ReadFile( hFile, &wMagic, 2, &nRead, NULL );
	if ( nRead != 2 || wMagic != 0xD8FF )
		return LibraryBuilder.SubmitCorrupted( nIndex );

	BYTE nBits = 0, nComponents = 0;
	WORD nWidth = 0, nHeight = 0;
	CString strComment;

	for ( DWORD nSeek = 512 ; nSeek > 0 ; nSeek-- )
	{
		ReadFile( hFile, &nByte, 1, &nRead, NULL );
		if ( nRead != 1 )
			return false;
		if ( nByte != 0xFF )
			continue;

		while ( nByte == 0xFF )
		{
			ReadFile( hFile, &nByte, 1, &nRead, NULL );
			if ( nRead != 1 )
				return false;
		}

		ReadFile( hFile, &wMagic, 2, &nRead, NULL );
		wMagic = swapEndianess( wMagic );
		if ( nRead != 2 || wMagic < 2 )
			return false;

		switch ( nByte )
		{
		case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC5: case 0xC6: case 0xC7:
		case 0xC9: case 0xCA: case 0xCB: case 0xCD: case 0xCE: case 0xCF:
			ReadFile( hFile, &nBits, 1, &nRead, NULL );
			if ( nRead != 1 )
				return false;
			ReadFile( hFile, &nHeight, 2, &nRead, NULL );
			if ( nRead != 2 )
				return false;
			nHeight = swapEndianess( nHeight );
			ReadFile( hFile, &nWidth, 2, &nRead, NULL );
			if ( nRead != 2 )
				return false;
			nWidth = swapEndianess( nWidth );
			ReadFile( hFile, &nComponents, 1, &nRead, NULL );
			if ( nRead != 1 )
				return false;
			if ( wMagic < 8 )
				return false;
			SetFilePointer( hFile, wMagic - 8, NULL, FILE_CURRENT );
			break;
		case 0xFE: case 0xEC:
			if ( wMagic > 2 )
			{
				CBuffer pComment;
				pComment.EnsureBuffer( wMagic - 2 );
				pComment.m_nLength = (DWORD)wMagic - 2;
				ReadFile( hFile, pComment.m_pBuffer, wMagic - 2, &nRead, NULL );
				strComment = pComment.ReadString( nRead );
			}
			break;
		case 0xD9: case 0xDA:
			nSeek = 1;
			break;
		default:
			SetFilePointer( hFile, wMagic - 2, NULL, FILE_CURRENT );
			break;
		}
	}

	if ( nWidth == 0 || nHeight == 0 )
		return false;

	strComment.TrimLeft();
	strComment.TrimRight();

	for ( int nChar = 0 ; nChar < strComment.GetLength() ; nChar++ )
	{
		if ( strComment[ nChar ] < 32 )
			strComment.SetAt( nChar, '?' );
	}

	auto_ptr< CXMLElement > pXML( new CXMLElement( NULL, _T("image") ) );
	CString strItem;

	strItem.Format( _T("%lu"), nWidth );
	pXML->AddAttribute( _T("width"), strItem );
	strItem.Format( _T("%lu"), nHeight );
	pXML->AddAttribute( _T("height"), strItem );

	if ( nComponents == 3 )
		pXML->AddAttribute( _T("colors"), _T("16.7M") );
	else if ( nComponents == 1 )
		pXML->AddAttribute( _T("colors"), _T("Greyscale") );

	if ( strComment.GetLength() )
		pXML->AddAttribute( _T("description"), strComment );

	return LibraryBuilder.SubmitMetadata( nIndex, CSchema::uriImage, pXML.release() ) != 0;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals GIF (threaded)

bool CLibraryBuilderInternals::ReadGIF(DWORD nIndex, HANDLE hFile)
{
	CHAR szMagic[6];
	DWORD nRead;

	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	ReadFile( hFile, szMagic, 6, &nRead, NULL );

	if ( nRead != 6 || ( strncmp( szMagic, "GIF87a", 6 ) && strncmp( szMagic, "GIF89a", 6 ) ) )
		return LibraryBuilder.SubmitCorrupted( nIndex );

	WORD nWidth, nHeight;

	ReadFile( hFile, &nWidth, 2, &nRead, NULL );
	if ( nRead != 2 || nWidth == 0 )
		return false;
	ReadFile( hFile, &nHeight, 2, &nRead, NULL );
	if ( nRead != 2 || nHeight == 0 )
		return false;

	auto_ptr< CXMLElement > pXML( new CXMLElement( NULL, _T("image") ) );
	CString strItem;

	strItem.Format( _T("%lu"), nWidth );
	pXML->AddAttribute( _T("width"), strItem );
	strItem.Format( _T("%lu"), nHeight );
	pXML->AddAttribute( _T("height"), strItem );

	pXML->AddAttribute( _T("colors"), _T("256") );

	return LibraryBuilder.SubmitMetadata( nIndex, CSchema::uriImage, pXML.release() ) != 0;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals PNG (threaded)

bool CLibraryBuilderInternals::ReadPNG(DWORD nIndex, HANDLE hFile)
{
	BYTE nMagic[8];
	DWORD nRead;

	if ( GetFileSize( hFile, NULL ) < 33 )
		return LibraryBuilder.SubmitCorrupted( nIndex );
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );

	ReadFile( hFile, nMagic, 8, &nRead, NULL );
	if ( nRead != 8 )
		return LibraryBuilder.SubmitCorrupted( nIndex );
	if ( nMagic[0] != 137 || nMagic[1] != 80 || nMagic[2] != 78 )
		return LibraryBuilder.SubmitCorrupted( nIndex );
	if ( nMagic[3] != 71 || nMagic[4] != 13 || nMagic[5] != 10 )
		return LibraryBuilder.SubmitCorrupted( nIndex );
	if ( nMagic[6] != 26 || nMagic[7] != 10 )
		return LibraryBuilder.SubmitCorrupted( nIndex );

	DWORD nLength, nIHDR;

	ReadFile( hFile, &nLength, 4, &nRead, NULL );
	nLength = swapEndianess( nLength );
	if ( nRead != 4 || nLength < 10 )
		return false;
	ReadFile( hFile, &nIHDR, 4, &nRead, NULL );
	if ( nRead != 4 || nIHDR != 'RDHI' )
		return false;

	DWORD nWidth, nHeight;
	BYTE nBits, nColors;

	ReadFile( hFile, &nWidth, 4, &nRead, NULL );
	nWidth = swapEndianess( nWidth );
	if ( nRead != 4 || nWidth <= 0 || nWidth > 0xFFFF )
		return false;
	ReadFile( hFile, &nHeight, 4, &nRead, NULL );
	nHeight = swapEndianess( nHeight );
	if ( nRead != 4 || nHeight <= 0 || nHeight > 0xFFFF )
		return false;

	ReadFile( hFile, &nBits, 1, &nRead, NULL );
	if ( nRead != 1 )
		return false;
	ReadFile( hFile, &nColors, 1, &nRead, NULL );
	if ( nRead != 1 )
		return false;

	auto_ptr< CXMLElement > pXML( new CXMLElement( NULL, _T("image") ) );
	CString strItem;

	strItem.Format( _T("%lu"), nWidth );
	pXML->AddAttribute( _T("width"), strItem );
	strItem.Format( _T("%lu"), nHeight );
	pXML->AddAttribute( _T("height"), strItem );

	/*
	if ( nColors == 2 || nColors == 4 )
	{
		pXML->AddAttribute( _T("colors"), _T("Greyscale") );
	}
	else
	*/
	{
		switch ( nBits )
		{
		case 1:
			pXML->AddAttribute( _T("colors"), _T("2") );
			break;
		case 2:
			pXML->AddAttribute( _T("colors"), _T("4") );
			break;
		case 4:
			pXML->AddAttribute( _T("colors"), _T("16") );
			break;
		case 8:
			pXML->AddAttribute( _T("colors"), _T("256") );
			break;
		case 16:
			pXML->AddAttribute( _T("colors"), _T("64K") );
			break;
		}
	}

	return LibraryBuilder.SubmitMetadata( nIndex, CSchema::uriImage, pXML.release() ) != 0;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals BMP (threaded)

bool CLibraryBuilderInternals::ReadBMP(DWORD nIndex, HANDLE hFile)
{
	BITMAPFILEHEADER pBFH;
	BITMAPINFOHEADER pBIH;
	DWORD nRead;

	if ( GetFileSize( hFile, NULL ) < sizeof(pBFH) + sizeof(pBIH) )
		return LibraryBuilder.SubmitCorrupted( nIndex );

	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	ReadFile( hFile, &pBFH, sizeof(pBFH), &nRead, NULL );
	if ( nRead != sizeof(pBFH) || pBFH.bfType != 'MB' )
		return LibraryBuilder.SubmitCorrupted( nIndex );

	ReadFile( hFile, &pBIH, sizeof(pBIH), &nRead, NULL );
	if ( nRead != sizeof(pBIH) || pBIH.biSize != sizeof(pBIH) )
		return false;

	auto_ptr< CXMLElement > pXML( new CXMLElement( NULL, _T("image") ) );
	CString strItem;

	strItem.Format( _T("%lu"), pBIH.biWidth );
	pXML->AddAttribute( _T("width"), strItem );
	strItem.Format( _T("%lu"), pBIH.biHeight );
	pXML->AddAttribute( _T("height"), strItem );

	switch ( pBIH.biBitCount )
	{
	case 4:
		pXML->AddAttribute( _T("colors"), _T("16") );
		break;
	case 8:
		pXML->AddAttribute( _T("colors"), _T("256") );
		break;
	case 24:
		pXML->AddAttribute( _T("colors"), _T("16.7M") );
		break;
	}

	return LibraryBuilder.SubmitMetadata( nIndex, CSchema::uriImage, pXML.release() ) != 0;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals ASF (threaded)

static const CLSID asfHeader1 =
{ 0x75B22630, 0x668E, 0x11CF, { 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C } };

static const CLSID asfContent1 =
{ 0x75B22633, 0x668E, 0x11CF, { 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C } };

static const CLSID asfProperties1 =	// ???
{ 0x8CABDCA1, 0xA947, 0x11CF, { 0x8E, 0xE4, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 } };

static const CLSID asfStream1 =
{ 0xB7DC0791, 0xA9B7, 0x11CF, { 0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 } };

static const CLSID asfVideo1 =
{ 0xBC19EFC0, 0x5B4D, 0x11CF, { 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B } };

static const CLSID asfData1 =
{ 0x75b22636, 0x668e, 0x11cf, { 0xa6, 0xd9, 0x00, 0xaa, 0x00, 0x62, 0xce, 0x6c } };

// {D6E229D1-35DA-11d1-9034-00A0C90349BE}
static const CLSID asfHeader2 =
{ 0xD6E229D1, 0x35DA, 0x11d1, { 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE } };

// {D6E229D2-35DA-11d1-9034-00A0C90349BE}
static const CLSID asfData2 =
{ 0xD6E229D2, 0x35DA, 0x11d1, { 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE } };

// {D6E229D0-35DA-11d1-9034-00A0C90349BE}
static const CLSID asfProperties2 =
{ 0xD6E229D0, 0x35DA, 0x11d1, { 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE } };

// {D6E229D4-35DA-11d1-9034-00A0C90349BE}
static const CLSID asfStream2 =
{ 0xD6E229D4, 0x35DA, 0x11d1, { 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE } };

// {D6E229D5-35DA-11d1-9034-00A0C90349BE}
static const CLSID asfContent2 =
{ 0xD6E229D5, 0x35DA, 0x11d1, { 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE } };

// {D6E229E2-35DA-11d1-9034-00A0C90349BE}
static const CLSID asfAudio2 =
{ 0xD6E229E2, 0x35DA, 0x11d1, { 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE } };

// {D6E229E3-35DA-11d1-9034-00A0C90349BE}
static const CLSID asfVideo2 =
{ 0xD6E229E3, 0x35DA, 0x11d1, { 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE } };

// {2211B3FB-BD23-11D2-B4B7-00A0C955FC6E}
static const CLSID asfDRM1 =
{ 0x2211B3FB, 0xBD23, 0x11D2, { 0xB4, 0xB7, 0x00, 0xA0, 0xC9, 0x55, 0xFC, 0x6E } };

// {1EFB1A30-0B62-11D0-A39B-00A0C90348F6}
static const CLSID asfDRM2 =
{ 0x1EFB1A30, 0x0B62, 0x11D0, { 0xA3, 0x9B, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6 } };

bool CLibraryBuilderInternals::ReadASF(DWORD nIndex, HANDLE hFile)
{
	QWORD nSize;
	DWORD nRead;
	GUID pGUID;

	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	ReadFile( hFile, &pGUID, sizeof(pGUID), &nRead, NULL );
	if ( nRead != sizeof(pGUID) || ( pGUID != asfHeader1 && pGUID != asfHeader2 ) )
		return LibraryBuilder.SubmitCorrupted( nIndex );
	ReadFile( hFile, &nSize, sizeof(nSize), &nRead, NULL );
	if ( nRead != sizeof(nSize) )
		return LibraryBuilder.SubmitCorrupted( nIndex );

	if ( pGUID == asfHeader1 )
		SetFilePointer( hFile, 6, NULL, FILE_CURRENT );

	CString strTitle, strAuthor, strCopyright, strDescription, strRating;
	DWORD nBitrate = 0, nVideoWidth = 0, nVideoHeight = 0;
	QWORD nContentLength = 0;
	bool bVideo = false;
	bool bDRM = false;

	for ( ;; )
	{
		DWORD dwPosition = SetFilePointer( hFile, 0, NULL, FILE_CURRENT );

		ReadFile( hFile, &pGUID, sizeof(pGUID), &nRead, NULL );
		if ( nRead != sizeof(pGUID) )
			break;
		ReadFile( hFile, &nSize, sizeof(nSize), &nRead, NULL );
		if ( nRead != sizeof(nSize) || nSize >= 0x80000000 )
			break;

		if ( pGUID == asfProperties1 )
		{
			SetFilePointer( hFile, 48, NULL, FILE_CURRENT );
			ReadFile( hFile, &nContentLength, sizeof(nContentLength), &nRead, NULL );
			if ( nRead != sizeof(nContentLength) )
				return false;
		}
		else if ( pGUID == asfProperties2 )
		{
			SetFilePointer( hFile, 40, NULL, FILE_CURRENT );
			ReadFile( hFile, &nContentLength, sizeof(nContentLength), &nRead, NULL );
			if ( nRead != sizeof(nContentLength) )
				return false;
			SetFilePointer( hFile, 8, NULL, FILE_CURRENT );
			ReadFile( hFile, &nBitrate, sizeof(nBitrate), &nRead, NULL );
			if ( nRead != sizeof(nBitrate) )
				return false;
		}
		else if ( pGUID == asfStream1 )
		{
			ReadFile( hFile, &pGUID, sizeof(pGUID), &nRead, NULL );
			if ( nRead != sizeof(pGUID) )
				return false;

			if ( pGUID == asfVideo1 )
			{
				bVideo = true;
				SetFilePointer( hFile, 38, NULL, FILE_CURRENT );
				ReadFile( hFile, &nVideoWidth, sizeof(nVideoWidth), &nRead, NULL );
				if ( nRead != sizeof(nVideoWidth) )
					return false;
				ReadFile( hFile, &nVideoHeight, sizeof(nVideoHeight), &nRead, NULL );
				if ( nRead != sizeof(nVideoHeight) )
					return false;
			}
		}
		else if ( pGUID == asfStream2 )
		{
			ReadFile( hFile, &pGUID, sizeof(pGUID), &nRead, NULL );
			if ( nRead != sizeof(pGUID) )
				return false;

			if ( pGUID == asfVideo2 )
			{
				bVideo = true;
				/*
				SetFilePointer( hFile, 68, NULL, FILE_CURRENT );
				ReadFile( hFile, &nVideoWidth, sizeof(nVideoWidth), &nRead, NULL );
				if ( nRead != sizeof(nVideoWidth) ) return false;
				nVideoHeight = nVideoWidth >> 16;
				nVideoWidth &= 0xFFFF;
				*/
			}
		}
		else if ( pGUID == asfContent1 )
		{
			WORD nStrLen[5];
			ReadFile( hFile, nStrLen, sizeof(nStrLen), &nRead, NULL );
			if ( nRead != sizeof(nStrLen) )
				break;

			for ( int nStr = 0 ; nStr < 5 ; nStr++ )
			{
				if ( !nStrLen[ nStr ] || nStrLen[ nStr ] & 1 )
					continue;
				auto_array< WCHAR > pStr( new WCHAR[ nStrLen[ nStr ] / 2 ] );
				ReadFile( hFile, pStr.get(), nStrLen[ nStr ], &nRead, NULL );
				if ( nRead != nStrLen[ nStr ] )
					return false;
				pStr[ nStrLen[ nStr ] / 2 - 1 ] = 0;

				switch ( nStr )
				{
				case 0:
					strTitle = pStr.get();
					break;
				case 1:
					strAuthor = pStr.get();
					break;
				case 2:
					strCopyright = pStr.get();
					break;
				case 3:
					strDescription = pStr.get();
					break;
				case 4:
					strRating = pStr.get();
					break;
				}
			}
		}
		else if ( pGUID == asfContent2 )
		{
			WORD nCount;
			ReadFile( hFile, &nCount, sizeof(nCount), &nRead, NULL );
			if ( nRead != sizeof(nCount) )
				break;

			while ( nCount-- )
			{
				WORD nLanguageID, nStreamID, nNameLen, nValueLen;
				BYTE nFieldType;

				ReadFile( hFile, &nFieldType, sizeof(nFieldType), &nRead, NULL );
				if ( nRead != sizeof(nFieldType) )
					return false;
				ReadFile( hFile, &nLanguageID, sizeof(nLanguageID), &nRead, NULL );
				if ( nRead != sizeof(nLanguageID) )
					return false;
				ReadFile( hFile, &nStreamID, sizeof(nStreamID), &nRead, NULL );
				if ( nRead != sizeof(nStreamID) )
					return false;
				ReadFile( hFile, &nNameLen, sizeof(nNameLen), &nRead, NULL );
				if ( nRead != sizeof(nNameLen) )
					return false;
				ReadFile( hFile, &nValueLen, sizeof(nValueLen), &nRead, NULL );
				if ( nRead != sizeof(nValueLen) )
					return false;

				auto_array< WCHAR > pStr1( new WCHAR[ nNameLen + 1 ] );
				ReadFile( hFile, pStr1.get(), nNameLen * 2, &nRead, NULL );
				if ( nRead != (DWORD)nNameLen * 2 )
					return false;
				pStr1[ nNameLen ] = 0;

				auto_array< WCHAR > pStr2( new WCHAR[ nValueLen + 1 ] );
				ReadFile( hFile, pStr2.get(), nValueLen * 2, &nRead, NULL );
				if ( nRead != (DWORD)nValueLen * 2 )
					return false;
				pStr2[ nValueLen ] = 0;

				switch ( nFieldType )
				{
				case 1:
					strAuthor = pStr2.get();
					break;
				case 2: case 20:
					strTitle = pStr2.get();
					break;
				case 3:
					strCopyright = pStr2.get();
					break;
				case 4:
					strDescription = pStr2.get();
					break;
				}
			}
		}
		else if ( pGUID == asfDRM1 || pGUID == asfDRM2 )
		{
			bDRM = true;
		}
		else if ( pGUID == asfData1 || pGUID == asfData2 )
		{
			break;
		}

		SetFilePointer( hFile, dwPosition + (DWORD)nSize, NULL, FILE_BEGIN );
	}

	auto_ptr< CXMLElement > pXML( new CXMLElement(
		NULL, bVideo ? _T("video") : _T("audio") ) );
	CString strItem;

	if ( strTitle.GetLength() )
		pXML->AddAttribute( _T("title"), strTitle );

	if ( strDescription.GetLength() )
		pXML->AddAttribute( _T("description"), strDescription );

	if ( bDRM )
		pXML->AddAttribute( _T("drm"), _T("true") );

	if ( bVideo )
	{
		if ( strAuthor.GetLength() )
			pXML->AddAttribute( _T("producer"), strAuthor );

		if ( strRating.GetLength() )
			pXML->AddAttribute( _T("rating"), strRating );

		if ( nContentLength > 0 )
		{
			DWORD nSeconds = (DWORD)( nContentLength / 10000000 );
			strItem.Format( _T("%lu.%lu"), nSeconds / 60, ( ( nSeconds % 60 ) * 10 / 60 ) );
			pXML->AddAttribute( _T("minutes"), strItem );
		}

		if ( nVideoWidth > 0 && nVideoHeight > 0 )
		{
			strItem.Format( _T("%lu"), nVideoWidth );
			pXML->AddAttribute( _T("width"), strItem );
			strItem.Format( _T("%lu"), nVideoHeight );
			pXML->AddAttribute( _T("height"), strItem );
		}
	}
	else
	{
		if ( strAuthor.GetLength() )
			pXML->AddAttribute( _T("artist"), strAuthor );

		if ( nContentLength > 0 )
		{
			strItem.Format( _T("%lu"), (DWORD)( nContentLength / 10000000 ) );
			pXML->AddAttribute( _T("seconds"), strItem );
		}

		if ( nBitrate > 0 )
		{
			strItem.Format( _T("%lu"), nBitrate / 1000 );
			pXML->AddAttribute( _T("bitrate"), strItem );
		}
	}

	pXML->AddAttribute( _T("codec"), _T("WM") );

	return LibraryBuilder.SubmitMetadata( nIndex, bVideo ? CSchema::uriVideo : CSchema::uriAudio, pXML.release() ) != 0;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals MPEG (threaded)

bool CLibraryBuilderInternals::ReadMPEG(DWORD nIndex, HANDLE hFile)
{
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );

	DWORD nHeader = 0;

	DWORD nSeek = 8192;
	for ( ; nSeek > 0 ; nSeek--, nHeader <<= 8 )
	{
		DWORD nRead = 0;
		ReadFile( hFile, &nHeader, 1, &nRead, NULL );
		if ( nRead != 1 )
			break;

		if ( nHeader == 0x000001B3 )
			break;
	}

	if ( !nSeek )
		return false;

	BYTE nBuffer[7];

	ReadFile( hFile, nBuffer, 7, &nHeader, NULL );
	if ( nHeader != 7 )
		return false;

	auto_ptr< CXMLElement > pXML( new CXMLElement( NULL, _T("video") ) );
	CString strItem;

	DWORD nWidth, nHeight;
	nWidth = ( (DWORD)nBuffer[0] << 4 ) | (DWORD)nBuffer[1] >> 4;
	nHeight = ( ( (DWORD)nBuffer[1] & 0x0F ) << 8 ) | (DWORD)nBuffer[2];

	strItem.Format( _T("%lu"), nWidth );
	pXML->AddAttribute( _T("width"), strItem );
	strItem.Format( _T("%lu"), nHeight );
	pXML->AddAttribute( _T("height"), strItem );
	pXML->AddAttribute( _T("codec"), _T("MPEG") );

	LPCTSTR pszFPS[] = { _T("23.976"), _T("24"), _T("25"), _T("29.97"), _T("30"), _T("50"), _T("59.94"), _T("60") };
	int nFrameIndex = ( nBuffer[3] & 0x0F );

	if ( nFrameIndex >= 1 && nFrameIndex < 9 )
		pXML->AddAttribute( _T("frameRate"), pszFPS[ nFrameIndex - 1 ] );

	return LibraryBuilder.SubmitMetadata( nIndex, CSchema::uriVideo, pXML.release() ) != 0;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals OGG VORBIS (threaded)

bool CLibraryBuilderInternals::ReadOGG(DWORD nIndex, HANDLE hFile)
{
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );

	DWORD nDummy, nHeader = 0;
	ReadFile( hFile, &nHeader, 4, &nDummy, NULL );

	for ( DWORD nSeek = 0 ; nSeek < 16384 ; nSeek++ )
	{
		if ( nHeader == 'SggO' )
			break;
		nHeader >>= 8;
		ReadFile( hFile, (BYTE*)&nHeader + 3, 1, &nDummy, NULL );
	}

	if ( nHeader != 'SggO' )
		return LibraryBuilder.SubmitCorrupted( nIndex );
	SetFilePointer( hFile, -4, NULL, FILE_CURRENT );

	DWORD nOGG = 0;
	BYTE* pOGG = ReadOGGPage( hFile, nOGG, 0x02, 0, 0x1E );

	if ( !pOGG )
		return false;

	BYTE  nChannels		= pOGG[ 11 ];
	DWORD nFrequency	= *(DWORD*)&pOGG[12];
	DWORD nBitrate		= *(DWORD*)&pOGG[20];

	delete [] pOGG;
	BYTE* prOGG = pOGG = ReadOGGPage( hFile, nOGG, 0x00, 1, 1+6+4+4 );

	if ( !pOGG )
		return false;

	pOGG += 1 + 6;
	nOGG -= 1 + 6;

	CString strComment;

	if ( !ReadOGGString( pOGG, nOGG, strComment ) || nOGG < 4 )
	{
		delete[] pOGG;
		return false;
	}

	DWORD nComments = *(DWORD*)pOGG;
	pOGG += 4; nOGG -= 4;

	auto_ptr< CXMLElement > pXML( new CXMLElement( NULL, L"audio" ) );

	for ( ; nComments && nOGG > 4 ; nComments-- )
	{
		if ( !ReadOGGString( pOGG, nOGG, strComment ) )
			break;

		int nEquals = strComment.Find( '=' );
		if ( nEquals <= 0 )
			continue;

		CString strKey		= strComment.Left( nEquals );
		CString strValue	= strComment.Mid( nEquals + 1 );

		strKey.TrimLeft(); strKey.TrimRight();
		CharUpper( strKey.GetBuffer() );
		strKey.ReleaseBuffer();

		// decode UTF-8 string
		int nLength = strValue.GetLength();
		auto_array< CHAR > pszDest( new CHAR[ nLength ] );
		for ( int nLen = 0 ; nLen < nLength ; nLen++ )
			pszDest[ nLen ] = (CHAR)strValue.GetAt( nLen );
		strValue = UTF8Decode( pszDest.get(), nLength );
		strValue.Trim();

		if ( strValue.IsEmpty() )
			continue;

		if ( strKey == L"TITLE" )
		{
			pXML->AddAttribute( L"title", strValue );
		}
		else if ( strKey == L"ALBUM" )
		{
			pXML->AddAttribute( L"album", strValue );
		}
		else if ( strKey == L"ORIGINALALBUM" )
		{
			pXML->AddAttribute( L"origAlbum", strValue );
		}
		else if ( strKey == L"TRACKNUMBER" )
		{
			pXML->AddAttribute( L"track", strValue );
		}
		else if ( strKey == L"ARTIST" )
		{
			pXML->AddAttribute( L"artist", strValue );
		}
		else if ( strKey == L"ORIGINALARTIST" )
		{
			pXML->AddAttribute( L"origArtist", strValue );
		}
		else if ( strKey == L"DESCRIPTION" || strKey == L"COMMENT" )
		{
			pXML->AddAttribute( L"description", strValue );
		}
		else if ( strKey == L"GENRE" )
		{
			pXML->AddAttribute( L"genre", strValue );
		}
		else if ( strKey == L"DATE" )
		{
			pXML->AddAttribute( L"year", strValue );
		}
		else if ( strKey == L"COPYRIGHT" )
		{
			pXML->AddAttribute( L"copyright", strValue );
		}
		else if ( strKey == L"ENCODED-BY" || strKey == L"ENCODEDBY" || strKey == L"ENCODED BY" )
		{
			pXML->AddAttribute( L"encodedby", strValue );
		}
		else if ( strKey == L"COMPOSER" )
		{
			pXML->AddAttribute( L"composer", strValue );
		}
		else if ( strKey == L"ENCODERSETTINGS" || strKey == L"ENCODER" || strKey == L"ENCODING" )
		{
			pXML->AddAttribute( L"qualitynotes", strValue );
		}
		else if ( strKey == L"USERURL" || strKey == L"USER DEFINED URL LINK" )
		{
			pXML->AddAttribute( L"link", strValue );
		}
	}

	delete [] prOGG;

	if ( nComments )
		return false;

	DWORD nLength = 0;

	for ( nComments = 2 ; ; nComments++ )
	{
		if ( !ReadOGGPage( hFile, nOGG, 0xFF, nComments, 0xFFFFFFFF ) )
			break;
		nLength = max( nLength, nOGG );
	}

	if ( nFrequency > 0 && nLength > 0 && ( nLength / nFrequency ) > 0 )
	{
		strComment.Format( L"%lu", nLength / nFrequency );
		pXML->AddAttribute( L"seconds", strComment );

		nBitrate = GetFileSize( hFile, NULL ) / ( nLength / nFrequency ) * 8;
	}

	strComment.Format( L"%lu", nBitrate / 1000 );
	pXML->AddAttribute( L"bitrate", strComment );

	strComment.Format( L"%lu", nFrequency );
	pXML->AddAttribute( L"sampleRate", strComment );

	strComment.Format( L"%lu", nChannels );
	pXML->AddAttribute( L"channels", strComment );

	return LibraryBuilder.SubmitMetadata( nIndex, CSchema::uriAudio, pXML.release() ) != 0;
}

BYTE* CLibraryBuilderInternals::ReadOGGPage(HANDLE hFile, DWORD& nBuffer, BYTE nFlags, DWORD nSequence, DWORD nMinSize)
{
	DWORD nMagic, nRead, nSample;
	BYTE nByte, nChunk;

	nBuffer = 0;

	ReadFile( hFile, &nMagic, 4, &nRead, NULL );
	if ( nRead != 4 || nMagic != 'SggO' )
		return NULL;

	ReadFile( hFile, &nByte, 1, &nRead, NULL );
	if ( nRead != 1 || nByte != 0 )
		return NULL;

	ReadFile( hFile, &nByte, 1, &nRead, NULL );
	if ( nRead != 1 )
		return NULL;
	if ( nFlags < 0xFF && nByte != nFlags )
		return NULL;

	ReadFile( hFile, &nSample, 4, &nRead, NULL );
	if ( nRead != 4 )
		return NULL;

	SetFilePointer( hFile, 4 + 4, NULL, FILE_CURRENT );

	ReadFile( hFile, &nMagic, 4, &nRead, NULL );
	if ( nRead != 4 || nMagic != nSequence )
		return NULL;

	ReadFile( hFile, &nMagic, 4, &nRead, NULL );
	if ( nRead != 4 )
		return NULL;

	ReadFile( hFile, &nByte, 1, &nRead, NULL );
	if ( nRead != 1 )
		return NULL;

	for ( ; nByte ; nByte-- )
	{
		ReadFile( hFile, &nChunk, 1, &nRead, NULL );
		if ( nRead != 1 )
			break;
		nBuffer += nChunk;
	}

	if ( nByte )
		return NULL;

	if ( nMinSize < 0xFFFFFFFF )
	{
		if ( nBuffer < nMinSize )
			return NULL;

		auto_array< BYTE > pBuffer( new BYTE[ nBuffer ] );
		if ( ReadFile( hFile, pBuffer.get(), nBuffer, &nRead, NULL ) &&
			nRead == nBuffer )
			return pBuffer.release();
	}
	else
	{
		SetFilePointer( hFile, nBuffer, NULL, FILE_CURRENT );
		nBuffer = nSample;
		return (BYTE*)true;
	}

	return NULL;
}

bool CLibraryBuilderInternals::ReadOGGString(BYTE*& pOGG, DWORD& nOGG, CString& str)
{
	if ( nOGG < 4 )
		return false;

	DWORD nLen = *(DWORD*)pOGG;
	pOGG += 4;
	nOGG -= 4;

	if ( nOGG < nLen )
		return false;

	LPTSTR pszOut = str.GetBuffer( nLen + 1 );
	for ( ; nLen ; nLen--, nOGG-- )
		*pszOut++ = (TCHAR)*pOGG++;
	*pszOut++ = 0;
	str.ReleaseBuffer();

	return true;
}

bool CLibraryBuilderInternals::ReadMPC(DWORD nIndex, HANDLE hFile)
{
	return ReadAPE( nIndex, hFile, true );
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals APE Monkey's Audio (threaded)

bool CLibraryBuilderInternals::ReadAPE(DWORD nIndex, HANDLE hFile, bool bPreferFooter)
{
	DWORD nFileSize = GetFileSize( hFile, NULL );
	if ( nFileSize < sizeof(APE_TAG_FOOTER) )
		return LibraryBuilder.SubmitCorrupted( nIndex );

	DWORD nRead;
	APE_TAG_FOOTER pFooter;

	auto_ptr< CXMLElement > pXML( new CXMLElement( NULL, L"audio" ) );

	SetFilePointer( hFile, -(LONG)sizeof(pFooter), NULL, FILE_END );
	ReadFile( hFile, &pFooter, sizeof(pFooter), &nRead, NULL );

	if ( nRead != sizeof(pFooter) || strncmp( pFooter.cID, "APETAGEX", 8 ) ||
		( pFooter.nVersion != 1000 && pFooter.nVersion != 2000 ) )
	{
		if ( bPreferFooter )
			// Invalid footer, try to validate header only
			pFooter.nFields = -1;
	}

	SetFilePointer( hFile, -(LONG)pFooter.nSize, NULL, FILE_END );

	bool bHasTotalDiscsField = false, bHasDiscField = false;
	bool bHasTotalTracksField = false, bHasTrackField = false;
	CString strTotalDiscsField, strTotalTracksField, strKeyWords, strArtist;
	CString strDiscField, strTrackField;

	for ( int nTag = 0 ; nTag < pFooter.nFields ; nTag++ )
	{
		DWORD nLength, nFlags;

		ReadFile( hFile, &nLength, 4, &nRead, NULL );
		if ( nRead != 4 || nLength > 1024 * 4 )
			break;
		ReadFile( hFile, &nFlags, 4, &nRead, NULL );
		if ( nRead != 4 )
			break;

		CString strKey, strValue;

		while ( strKey.GetLength() < 255 )
		{
			BYTE nChar;
			ReadFile( hFile, &nChar, 1, &nRead, NULL );
			if ( nRead != 1 || nChar == 0 )
				break;
			strKey += (TCHAR)nChar;
		}

		if ( nRead != 1 || strKey.GetLength() >= 255 )
			break;

		auto_array< CHAR > pszInput( new CHAR[ nLength ] );
		if ( ! ReadFile( hFile, pszInput.get(), nLength, &nRead, NULL ) ||
			nLength != nRead )
			break;

		strValue = UTF8Decode( pszInput.get(), nLength );

		strKey.TrimLeft(); strKey.TrimRight();
		strValue.TrimLeft(); strValue.TrimRight();

		if ( strKey.GetLength() && strValue.GetLength() )
		{
			ToLower( strKey );

			if ( strKey == L"title" )
			{
				pXML->AddAttribute( L"title", strValue );
			}
			else if ( strKey == L"artist" )
			{
				pXML->AddAttribute( L"artist", strValue );
				strArtist = strValue;
			}
			else if ( strKey == L"album" )
			{
				pXML->AddAttribute( L"album", strValue );
			}
			else if ( strKey == L"comment" )
			{
				pXML->AddAttribute( L"description", strValue );
			}
			else if ( strKey == L"year" )
			{
				if ( strValue.GetLength() > 4 && strValue.Find( L"-" ) != -1 )
				{
					pXML->AddAttribute( L"year", strValue.Left( 4 ) );
					pXML->AddAttribute( L"releaseDate", strValue );
				}
				else
					pXML->AddAttribute( L"year", strValue );
			}
			else if ( strKey == L"track" )
			{
				bHasTrackField = true;
				int intSlashPosition = strValue.Find( L"/" );
				if ( strValue.Find( L"/" ) != -1 )
				{
					bHasTotalTracksField = true;
					strTrackField = strValue.Left( intSlashPosition );
					strTotalTracksField = strValue.Right( strValue.GetLength() - intSlashPosition - 1 );
				}
				else
					strTrackField = strValue;
			}
			else if ( strKey == L"totaltracks" )
			{
				bHasTotalTracksField = true;
				strTotalTracksField = strValue;
			}
			else if ( strKey == L"genre" )
			{
				pXML->AddAttribute( L"genre", strValue );
			}
			else if ( strKey.Find( L" url" ) > 0 ) // are there any tag fields containing that?
			{
				pXML->AddAttribute( L"link", strValue );
			}
			else if ( strKey == L"composer" )
			{
				pXML->AddAttribute( L"composer", strValue );
			}
			else if ( strKey == L"publisher" )
			{
				pXML->AddAttribute( L"publisher", strValue );
			}
			else if ( strKey == L"copyright" )
			{
				pXML->AddAttribute( L"copyright", strValue );
			}
			else if ( strKey == L"producer" )
			{
				pXML->AddAttribute( L"producer", strValue );
			}
			else if ( strKey == L"lyricist" )
			{
				pXML->AddAttribute( L"lyricist", strValue );
			}
			else if ( strKey == L"arranger" )
			{
				pXML->AddAttribute( L"arranger", strValue );
			}
			else if ( strKey == L"performer" )
			{
				pXML->AddAttribute( L"performer", strValue );
			}
			else if ( strKey == L"conductor" )
			{
				pXML->AddAttribute( L"conductor", strValue );
			}
			else if ( strKey == L"language" )
			{
				pXML->AddAttribute( L"language", strValue );
			}
			else if ( strKey == L"disc" )
			{
				bHasDiscField = true;
				int intSlashPosition = strValue.Find( L"/" );
				if ( intSlashPosition != -1 )
				{
					bHasTotalDiscsField = true;
					strDiscField = strValue.Left( intSlashPosition );
					strTotalDiscsField = strValue.Right( strValue.GetLength() - intSlashPosition - 1 );
				}
				else
					strDiscField = strValue;
			}
			else if ( strKey == L"totaldiscs" )
			{
				bHasTotalDiscsField = true;
				strTotalDiscsField = strValue;
			}
			else if ( strKey == L"date" )
			{
				pXML->AddAttribute( L"releaseDate", strValue );
			}
			else if ( strKey == L"encoded-by" || strKey == L"encodedby" || strKey == L"encoded by" )
			{
				pXML->AddAttribute( L"encodedby", strValue );
			}
			else if ( strKey == L"involvedpeople" )
			{
				if ( strKeyWords.GetLength() )
					strKeyWords += L"; " + strValue;
				else
					strKeyWords = strValue;
			}
			else if ( strKey == L"lyrics" || strKey == L"unsyncedlyrics" || strKey == L"unsynced lyrics" )
			{
				// pXML->AddAttribute( L"unsyncedlyrics", strValue );
			}
			else if ( strValue != strArtist && ( strKey == L"musicbrainz album artist" || strKey == L"musicbrainz albumartist" || strKey == L"album artist" || strKey == L"albumartist" ) )
			{
				pXML->AddAttribute( L"albumArtist", strValue );
			}
			else if ( strKey == L"musicbrainz album id" )
			{
				pXML->AddAttribute( L"mbalbumid", strValue );
			}
			else if ( strKey == L"musicbrainz album type" )
			{
				pXML->AddAttribute( L"type", strValue );
			}
			else if ( strKey == L"musicbrainz album artist id" )
			{
				pXML->AddAttribute( L"mbalbumartistid", strValue );
			}
			else if ( strKey == L"musicbrainz album status" )
			{
				pXML->AddAttribute( L"albumStatus", strValue );
			}
			else if ( strKey == L"musicbrainz aritst id" )
			{
				pXML->AddAttribute( L"mbartistid", strValue );
			}
			else if ( strKey == L"musicbrainz non-album" && strValue == L"1" )
			{
				pXML->AddAttribute( L"type", L"Non-Album Track" );
			}
			else if ( strKey == L"musicip puid" )
			{
				pXML->AddAttribute( L"mbpuid", strValue );
			}
			else if ( strKey == L"musicbrainz trm id" )
			{
				pXML->AddAttribute( L"mbtrmid", strValue );
			}
			else if ( strKey == L"performersortorder" )
			{
				if ( strKeyWords.GetLength() )
					strKeyWords += L"; " + strValue;
				else
					strKeyWords = strValue;
			}
			else if ( strKey == L"releasetime" )
			{
				pXML->AddAttribute( L"releaseDate", strValue );
			}
			else if ( strKey == L"uniquefileid" )
			{
				pXML->AddAttribute( L"mbuniquefileid", strValue );
			}
			else if ( strKey == L"contentgroup" )
			{
				pXML->AddAttribute( L"releasegroup", strValue );
			}
			else if ( strKey == L"encodersettings" )
			{
				pXML->AddAttribute( L"qualitynotes", strValue );
			}
			else if ( strKey == L"origalbum" )
			{
				pXML->AddAttribute( L"origAlbum", strValue );
			}
			else if ( strKey == L"origartist" )
			{
				pXML->AddAttribute( L"origArtist", strValue );
			}
			else if ( strKey == L"origfilename" )
			{
				pXML->AddAttribute( L"origFilename", strValue );
			}
			else if ( strKey == L"origlyricist" )
			{
				pXML->AddAttribute( L"origLyricist", strValue );
			}
			else if ( strKey == _T("origyear") )
			{
				pXML->AddAttribute( L"origYear", strValue );
			}
			else if ( strKey == L"wwwaudiosource" || strKey == L"wwwaudiofile" )
			{
				pXML->AddAttribute( L"releasegroupLink", strValue );
			}
			else if ( strKey == L"cddbdiscid" )
			{
				pXML->AddAttribute( L"cddb", strValue );
			}
		}
	}

	pXML->AddAttribute( L"keywords", strKeyWords );

	if ( bHasDiscField )
	{
		if ( bHasTotalDiscsField )
			pXML->AddAttribute( L"disc", strDiscField + "/" + strTotalDiscsField );
		else
			pXML->AddAttribute( L"disc", strDiscField );
	}

	if ( bHasTrackField == true )
	{
		if ( bHasTotalTracksField )
			pXML->AddAttribute( L"track", strTrackField + "/" + strTotalTracksField );
		else
			pXML->AddAttribute( L"track", strTrackField );
	}

	if ( nFileSize < sizeof(APE_HEADER) )
		return LibraryBuilder.SubmitCorrupted( nIndex );

	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	APE_HEADER pAPE = {0};
	APE_HEADER_NEW pNewAPE = {0};
	int nValidSize = sizeof(pAPE);

	ReadFile( hFile, &pAPE, nValidSize, &nRead, NULL );

	// We checked if the file size is bigger than the footer, so no check is needed
	// here for the nRead bytes.

	// Signatures we handle although the headers may be invalid.
	// APE tags usually are placed in footer (it's recommended).
	bool bMAC = pAPE.cID[0] == 'M' && pAPE.cID[1] == 'A' && pAPE.cID[2] == 'C';
	bool bMPC = pAPE.cID[0] == 'M' && pAPE.cID[1] == 'P' && pAPE.cID[2] == '+';
	bool bNewAPE = false;

	if ( bMAC && pAPE.nVersion >= APE2_VERSION )
	{
		SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
		nValidSize = sizeof(pNewAPE);
		ReadFile( hFile, &pNewAPE, nValidSize, &nRead, NULL );
		bNewAPE = true;
	}
	else
	{
		// just copy data to new format header and we will use it for the tests
		pNewAPE.nVersion = pAPE.nVersion;
		pNewAPE.nSampleRate = pAPE.nSampleRate;
		pNewAPE.nTotalFrames = pAPE.nTotalFrames;
		pNewAPE.nCompressionLevel = pAPE.nCompressionLevel;
		pNewAPE.nFormatFlags = pAPE.nFormatFlags;
		pNewAPE.nFinalFrameBlocks = pAPE.nFinalFrameBlocks;
		pNewAPE.nChannels = pAPE.nChannels;
		pNewAPE.nHeaderBytes = pAPE.nHeaderBytes;
		ZeroMemory( &pAPE, nValidSize ); // just in case if someone messes up the code below
	}

	bool bValidSignature = bMAC || bMPC;

	if ( (int)nRead != nValidSize || !bValidSignature || pNewAPE.nSampleRate == 0 || bPreferFooter )
	{
		// APE tags in MP3 or MPC footer
		if ( pFooter.nFields > 0 && bPreferFooter )
		{
			if ( !bValidSignature )
				ScanMP3Frame( pXML.get(), hFile, 0 );
			return LibraryBuilder.SubmitMetadata( nIndex, CSchema::uriAudio, pXML.release() ) != 0;
		}
		else // No APE footer and no header in MP3 or invalid APE file
			return bPreferFooter ? false : LibraryBuilder.SubmitCorrupted( nIndex );
	}

	DWORD nSamplesPerFrame;

	if ( bNewAPE )
		nSamplesPerFrame = pNewAPE.nBlocksPerFrame;
	else
	{
		nSamplesPerFrame = ( ( pNewAPE.nVersion >= 3900 ) ||
			( pNewAPE.nVersion >= 3800 && pNewAPE.nCompressionLevel == 4000 ) ) ? 73728 : 9216;
		if ( pNewAPE.nVersion >= 3950 )
			nSamplesPerFrame = 73728 * 4;
	}

	DWORD nSamples = 0;
	if ( pNewAPE.nTotalFrames != 0 )
		nSamples = ( pNewAPE.nTotalFrames - 1 ) * nSamplesPerFrame + pNewAPE.nFinalFrameBlocks;

	if ( pNewAPE.nSampleRate == 0 )
		return LibraryBuilder.SubmitCorrupted( nIndex );

	DWORD nDuration = nSamples / pNewAPE.nSampleRate;

	if ( nDuration <= 0.0 )
		return LibraryBuilder.SubmitCorrupted( nIndex );

	DWORD nBitRate = ( nFileSize * 8 / nDuration + 500 ) / 1000;
	DWORD nBitsPerSample = 0;

	if ( bNewAPE )
	{
		Hashes::Md5Hash	oApeMD5;
		std::memcpy( &oApeMD5, pNewAPE.cFileMD5, sizeof(pNewAPE.cFileMD5) );
		if ( !oApeMD5.validate() )
			return LibraryBuilder.SubmitCorrupted( nIndex );

		nBitsPerSample = pNewAPE.nBitsPerSample;

		// ToDo: We need MD5 hash of the file without tags...
//		if ( validAndUnequal( oApeMD5, oMD5 ) )
//			return LibraryBuilder.SubmitCorrupted( nIndex );
	}
	else
	{
		nBitsPerSample = ( pNewAPE.nFormatFlags & 1 ) ? 8 : ( pNewAPE.nFormatFlags & 8 ) ? 24 : 16;
	}

	if ( nBitsPerSample == 0 )
		return LibraryBuilder.SubmitCorrupted( nIndex );

	DWORD nUncompressedSize = nSamples * pNewAPE.nChannels * ( nBitsPerSample / 8 );
	if ( nUncompressedSize == 0 )
		return LibraryBuilder.SubmitCorrupted( nIndex );

	CString strItem;

	strItem.Format( L"%lu", nBitRate );
	pXML->AddAttribute( L"bitrate", strItem );

	strItem.Format( L"%lu", nDuration );
	pXML->AddAttribute( L"seconds", strItem );

	strItem.Format( L"%lu", pNewAPE.nSampleRate );
	pXML->AddAttribute( L"sampleRate", strItem );

	strItem.Format( L"%lu", pNewAPE.nChannels );
	pXML->AddAttribute( L"channels", strItem );

	ReadID3v1( nIndex, hFile, pXML.get() );

	return LibraryBuilder.SubmitMetadata( nIndex, CSchema::uriAudio, pXML.release() ) != 0;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals AVI (threaded)

#define FCC(ch4) ((((DWORD)(ch4) & 0xFF) << 24) |     \
	(((DWORD)(ch4) & 0xFF00) << 8) |    \
	(((DWORD)(ch4) & 0xFF0000) >> 8) |  \
	(((DWORD)(ch4) & 0xFF000000) >> 24))

#define ReadDwordOrBreak(hFile, nID, nRead) ReadFile( hFile, &nID, 4, &nRead, NULL ); \
	if ( nRead != 4 ) break;

#define ReadValueOrFail(hFile, nID, nRead, nValue, nFile) ReadFile( hFile, &nID, 4, &nRead, NULL ); \
	if ( nRead != 4 || nID != nValue ) return LibraryBuilder.SubmitCorrupted( nFile );

bool CLibraryBuilderInternals::ReadAVI(DWORD nIndex, HANDLE hFile)
{
	if ( GetFileSize( hFile, NULL ) < sizeof(AVI_HEADER) + 16 )
		return LibraryBuilder.SubmitCorrupted( nIndex );
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );

	DWORD nID;
	DWORD nRead, nNextOffset, nPos;
	CStringA strCodec;

	ReadValueOrFail( hFile, nID, nRead, FCC('RIFF'), nIndex )
	ReadFile( hFile, &nID, 4, &nRead, NULL );
	if ( nRead != 4 )
		return LibraryBuilder.SubmitCorrupted( nIndex );
	ReadValueOrFail( hFile, nID, nRead, FCC('AVI '), nIndex )

	// AVI files include two mandatory LIST chunks ('hdrl' and 'movi')
	// So, treat file as corrupted if they are missing

	ReadValueOrFail( hFile, nID, nRead, FCC('LIST'), nIndex )

	// Get next outer LIST offset
	ReadFile( hFile, &nNextOffset, sizeof(DWORD), &nRead, NULL );
	if ( nRead != 4 )
		return LibraryBuilder.SubmitCorrupted( nIndex );

	// Remember position
	nPos = SetFilePointer( hFile, 0, NULL, FILE_CURRENT );

	ReadValueOrFail( hFile, nID, nRead, FCC('hdrl'), nIndex )
	ReadValueOrFail( hFile, nID, nRead, FCC('avih'), nIndex )

	ReadFile( hFile, &nID, 4, &nRead, NULL );
	if ( nRead != 4 )
		return LibraryBuilder.SubmitCorrupted( nIndex );

	AVI_HEADER pHeader;
	ReadFile( hFile, &pHeader, sizeof(pHeader), &nRead, NULL );
	if ( nRead != sizeof(pHeader) )
		return LibraryBuilder.SubmitCorrupted( nIndex );

	// One or more 'strl' chunks must follow the main header
	ReadValueOrFail( hFile, nID, nRead, FCC('LIST'), nIndex )
	ReadFile( hFile, &nID, 4, &nRead, NULL );
	if ( nRead != 4 )
		return LibraryBuilder.SubmitCorrupted( nIndex );

	ReadValueOrFail( hFile, nID, nRead, FCC('strl'), nIndex )
	ReadValueOrFail( hFile, nID, nRead, FCC('strh'), nIndex )

	ReadFile( hFile, &nID, 4, &nRead, NULL );
	if ( nRead != 4 )
		return LibraryBuilder.SubmitCorrupted( nIndex );

	ReadValueOrFail( hFile, nID, nRead, FCC('vids'), nIndex )
	ReadFile( hFile, (BYTE*)strCodec.GetBufferSetLength( 4 ), 4, &nRead, NULL );
	if ( nRead != 4 )
		return LibraryBuilder.SubmitCorrupted( nIndex );

	bool bMoviFound = false, bInfoFound = false;
	auto_ptr< CXMLElement > pXML( new CXMLElement( NULL, L"video" ) );

	do
	{
		nPos += nNextOffset;
		if ( SetFilePointer( hFile, nPos, NULL, FILE_BEGIN ) == INVALID_SET_FILE_POINTER )
			break;
		ReadDwordOrBreak( hFile, nID, nRead )
		nNextOffset = 0;
		ReadDwordOrBreak( hFile, nNextOffset, nRead )
		nPos = SetFilePointer( hFile, 0, NULL, FILE_CURRENT );
		if ( nID == FCC('LIST') )
		{
			ReadDwordOrBreak( hFile, nID, nRead )
			if ( nID == FCC('movi') )
			{
				bMoviFound = true;
			}
			else if ( nID == FCC('INFO') )
			{
				bInfoFound = true;
				for ( ; ; )
				{
					ReadDwordOrBreak( hFile, nID, nRead )
					if ( (CHAR)( nID & 0x000000FF ) != 'I' )
						break;

					DWORD nDataLength = 0;
					ReadDwordOrBreak( hFile, nDataLength, nRead )

					CStringA strData;
					ReadFile( hFile, (BYTE*)strData.GetBufferSetLength( nDataLength ), nDataLength, &nRead, NULL );
					if ( nRead != nDataLength )
						break;
					switch ( nID )
					{
					case FCC('IARL'): // Archival Location
						break;
					case FCC('IART'): // Artist
						pXML->AddAttribute( L"artist", CString( strData ) );
						break;
					case FCC('IAS1'): // Language, not documented (?)
					case FCC('ILNG'):
						pXML->AddAttribute( L"language", CString( strData ) );
						break;
					case FCC('ICMS'): // Commissioned
						break;
					case FCC('ICMT'): // Comments
						pXML->AddAttribute( L"comments", CString( strData ) );
						break;
					case FCC('ICOP'): // Copyright
						pXML->AddAttribute( L"copyright", CString( strData ) );
						break;
					case FCC('IDIT'): // ???
					case FCC('ICRD'): // Creation date. List dates in year-month-day format,
									  // padding one-digit months and days with a zero on the left;
									  // for example, 1553-05-03 for May 3, 1553.
						pXML->AddAttribute( L"releaseDate", CString( strData ) );
						break;
					case FCC('ICRP'): // Cropped, for e.g. "lower-right corner"
						pXML->AddAttribute( L"qualityNotes", CString( strData ) );
						break;
					case FCC('IDIM'): // Dimensions, for example, 8.5 in h, 11 in w.
						break;
					case FCC('IDPI'): // Dots Per Inch
						break;
					case FCC('IENG'): // Engineer
						break;
					case FCC('IGNR'): // Genre
						pXML->AddAttribute( L"genre", CString( strData ) );
						break;
					case FCC('IKEY'): // Keywords
						pXML->AddAttribute( L"keywords", CString( strData ) );
						break;
					case FCC('ILGT'): // Lightness
						break;
					case FCC('IMED'): // Medium
						break;
					case FCC('INAM'): // Name
						pXML->AddAttribute( L"title", CString( strData ) );
						break;
					case FCC('IPLT'): // Palette Setting
						break;
					case FCC('IPRD'): // Product
						pXML->AddAttribute( L"type", CString( strData ) );
						break;
					case FCC('ISBJ'): // Subject
						pXML->AddAttribute( L"description", CString( strData ) );
						break;
					case FCC('ISFT'): // Software
						break;
					case FCC('ISHP'): // Sharpness
						break;
					case FCC('ISRC'): // Source
						pXML->AddAttribute( L"source", CString( strData ) );
						break;
					case FCC('ISRF'): // Source Form
						break;
					case FCC('ITCH'): // Technician
						pXML->AddAttribute( L"releasegroup", CString( strData ) );
						break;
					}

					if ( nDataLength % 2 != 0 ) // Skip padding zero byte if the data size is not even
					{
						ReadFile( hFile, &nID, 1, &nRead, NULL );
						if ( nRead != 1 )
							break;
					}
				}
			}
		}
		if ( bMoviFound && bInfoFound )
			break;
	}
	while ( nNextOffset );

	if ( !bMoviFound )
		return LibraryBuilder.SubmitCorrupted( nIndex );

	CString strItem;

	double nTime = (double)pHeader.dwMicroSecPerFrame / 1000000.0f;
	nTime *= (double)pHeader.dwTotalFrames;
	nTime /= 60.0f;

	double nRate = 1000000.0f / (double)pHeader.dwMicroSecPerFrame;

	strItem.Format( L"%lu", pHeader.dwWidth );
	pXML->AddAttribute( L"width", strItem );
	strItem.Format( L"%lu", pHeader.dwHeight );
	pXML->AddAttribute( L"height", strItem );
	strItem.Format( L"%.3f", nTime );
	pXML->AddAttribute( L"minutes", strItem );
	strItem.Format( L"%.2f", nRate );
	pXML->AddAttribute( L"frameRate", strItem );
	pXML->AddAttribute( L"codec", CString( strCodec ) );

	return LibraryBuilder.SubmitMetadata( nIndex, CSchema::uriVideo, pXML.release() ) != 0;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals PDF (threaded)

bool CLibraryBuilderInternals::ReadPDF(DWORD nIndex, HANDLE hFile, LPCTSTR pszPath)
{
	DWORD nOffset, nCount, nCountStart, nPages = 0, nOffsetPrev, nFileLength = 0, nVersion;

	// TODO: Header should be within the first 1024 KB by specs

	// Check PDF version
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	CString strLine = ReadPDFLine( hFile, false );
	if ( strLine.Find( _T("%PDF") ) == 0 )
		nCount = 7;
	else if ( strLine.Find( _T("%!PS-Adobe") ) == 0 )
		nCount = 21;
	else
		return false;
	if ( _stscanf( strLine.Mid( nCount ), _T("%lu"), &nVersion ) != 1 )
		return false;
	//if ( nVersion > 5 )
	//	return false;
	strLine = ReadPDFLine( hFile, false, true, false );

	// Read PDF header
	bool bLinearized = false;
	if ( ! ReadPDFLine( hFile, false, true ).IsEmpty() )
		return false;
	strLine = ReadPDFLine( hFile, false, true );
	if ( strLine.IsEmpty() )
		strLine = ReadPDFLine( hFile, false, true );
	nCount = 0;
	while ( ! strLine.IsEmpty() && nCount < 9 && nVersion > 1 ) //read dictionary entries only from 8 lines max
	{
		int nData = strLine.Find( _T(" ") );
		if ( nData > 0 )
		{
			CString strEntry = strLine.Left( nData );
			if ( strEntry.CompareNoCase( _T("linearized") ) == 0 )
				bLinearized = ( strLine.Mid( nData + 1 ) == _T("1") );
			else if ( strEntry.CompareNoCase( _T("n") ) == 0 )
			{
				if ( _stscanf( strLine.Mid( nData + 1 ), _T("%lu"), &nPages ) != 1 )
					break;
			}
			else if ( strEntry.CompareNoCase( _T("l") ) == 0 )
			{
				if ( _stscanf( strLine.Mid( nData + 1 ), _T("%lu"), &nFileLength ) != 1 )
					break;
			}
		}
		strLine = ReadPDFLine( hFile, false, true );
		nCount++;
	}

	if ( bLinearized )
	{
		// remember position
		nOffset = SetFilePointer( hFile, 0, NULL, FILE_CURRENT );
		// if file length is not the same as in L data, the document is treated as non-linearized
		DWORD nError;
		if ( SetFilePointer( hFile, nFileLength, NULL, FILE_BEGIN ) == INVALID_SET_FILE_POINTER &&
			( nError = GetLastError() ) != NO_ERROR )
		{
			bLinearized = false;
			nPages = 0;
		}
		else // return back
			SetFilePointer( hFile, nOffset, NULL, FILE_BEGIN );
	}

	// nOffset - the first reference position to which we will go
	// First we validate reference table and find a total number of objects
	nOffset = nOffsetPrev = 0;

	// Linearized document validation
	if ( bLinearized )
	{
		// get total object count
		if ( ReadPDFLine( hFile, false ).IsEmpty() )
			strLine = ReadPDFLine( hFile, false );
		if ( strLine != _T("endobj") )
			return false;

		nOffset = SetFilePointer( hFile, 0, NULL, FILE_CURRENT );
		strLine = ReadPDFLine( hFile, false );
		if ( strLine.IsEmpty() )
			strLine = ReadPDFLine( hFile, false );
		if ( strLine != _T("xref") )
			return false;

		strLine = ReadPDFLine( hFile, false );
		if ( _stscanf( strLine, _T("%lu %lu"), &nCountStart, &nCount ) != 2 )
			return false;

		for ( DWORD nLines = 0 ; nLines < nCount ; nLines++ )
			ReadPDFLine( hFile, false );
		nCount += nCountStart; // total number of objects
	}
	else // Non-linearized document validation
	{
		SetFilePointer( hFile, -1, NULL, FILE_END );
		strLine = ReadPDFLine( hFile, true );
		if ( strLine.IsEmpty() )
			strLine = ReadPDFLine( hFile, true );

		// TODO: %%EOF should be within the last 1024 KB by specs
		if ( strLine != _T("%%EOF") )
			return false;

		strLine = ReadPDFLine( hFile, true );
		if ( ReadPDFLine( hFile, true ) != _T("startxref") )
			return false;

		// get last reference object number
		if ( _stscanf( strLine, _T("%lu"), &nOffset ) != 1 )
			return false;
		if ( ! ReadPDFLine( hFile, true, true ).IsEmpty() )
			return false;

		int nLines;
		for ( nLines = 10 ; nLines; --nLines )
		{
			strLine = ReadPDFLine( hFile, true );
			if ( strLine.CompareNoCase( _T("trailer") ) == 0 )
			{
				ReadPDFLine( hFile, false );
				break;
			}
		}
		if ( nLines == 0 )
			return false;
	}

	// read trailer dictionary
	strLine = ReadPDFLine( hFile, false );
	if ( strLine.CompareNoCase( _T("trailer") ) != 0 )
		return false;
	if ( ! ReadPDFLine( hFile, false, true ).IsEmpty() )
		return false;
	strLine = ReadPDFLine( hFile, false, true );
	if ( strLine.IsEmpty() )
		strLine = ReadPDFLine( hFile, false, true );
	while ( ! strLine.IsEmpty() )
	{
		int nData = strLine.Find( _T(" ") );
		if ( nData > 0 )
		{
			CString strEntry = strLine.Left( nData );
			if ( strEntry.CompareNoCase( _T("size") ) == 0 )
			{
				DWORD nSize;
				if ( _stscanf( strLine.Mid( nData + 1 ), _T("%lu"), &nSize ) != 1 )
					return false;
				if ( bLinearized )
				{
					if ( nSize != nCount )
						return false;
				}
				else
					nCount = nSize;
			}
			else if ( strEntry.CompareNoCase( _T("prev") ) == 0 )
			{
				if ( _stscanf( strLine.Mid( nData + 1 ), _T("%lu"), &nOffsetPrev ) != 1 )
					return false;
			}
			//else if ( strEntry.CompareNoCase( _T("encrypt") ) == 0 )
			//{
			//	// if document encrypted skip it
			//	if ( strLine.Mid( nData + 1 ).CompareNoCase( _T("null") ) != 0 )
			//		return false;
			//}
		}
		strLine = ReadPDFLine( hFile, false, true );
	}
	if ( bLinearized )
	{
		// Linearized docs should have non-zero value
		if ( nOffsetPrev == 0 )
			return false;
	}

	if ( ! nCount )
		return false;
	auto_array< DWORD > pOffset( new DWORD[ nCount ] );
	if ( ! pOffset.get() )
		return false;
	ZeroMemory( pOffset.get(), sizeof( DWORD ) * nCount );

	// The main part: an array is filled with the locations of objects from reference tables
	DWORD nOffsetInfo = 0, nOffsetRoot = 0;
	while ( nOffset != 0 )
	{
		// return back and cycle through all references
		SetFilePointer( hFile, nOffset, NULL, FILE_BEGIN );
		strLine = ReadPDFLine( hFile, false );
		if ( strLine.IsEmpty() )
			strLine = ReadPDFLine( hFile, false );
		if ( strLine != _T("xref") )
			return false;
		strLine = ReadPDFLine( hFile, false );
		DWORD nTemp;
		if ( _stscanf( strLine, _T("%lu %lu"), &nCountStart, &nTemp ) != 2 )
			return false;
		DWORD nTotal = nCountStart + nTemp;
		if ( nCount < nTotal )
			return false;

		// collect objects positions from the references
		for ( DWORD nObjectNo = nCountStart ; nObjectNo < nTotal ; nObjectNo++ )
		{
			strLine = ReadPDFLine( hFile, false );
			strLine.Trim();
			if ( strLine.GetLength() != 18 || strLine.GetAt( 10 ) != ' ' )
				return false;
			if ( strLine.GetAt( 17 ) == 'n' )
			{
				LPCTSTR pszInt = strLine;
				for ( ; *pszInt == '0' ; pszInt++ );
				if ( *pszInt != 0 )
				{
					if ( _stscanf( pszInt, _T("%lu"), &pOffset[ nObjectNo ] ) != 1 )
						return false;
				}
			}
		}
		if ( ReadPDFLine( hFile, false ) != _T("trailer") )
			return false;
		// Only the last data from trailers are used for /Info and /Root positions
		nOffsetPrev = 0;

		if ( ! ReadPDFLine( hFile, false, true ).IsEmpty() )
			return false;
		strLine = ReadPDFLine( hFile, false, true );
		if ( strLine.IsEmpty() )
			strLine = ReadPDFLine( hFile, false, true );
		while ( ! strLine.IsEmpty() )
		{
			int nData = strLine.Find( _T(" ") );
			if ( nData > 0 )
			{
				CString strEntry = strLine.Left( nData );
				if ( strEntry.CompareNoCase( _T("info") ) == 0 )
				{
					if ( _stscanf( strLine.Mid( nData + 1 ), _T("%lu"), &nOffsetInfo ) != 1 )
						return false;
					if ( nCount < nOffsetInfo )
						return false;
				}
				else if ( strEntry.CompareNoCase( _T("prev") ) == 0 )
				{
					if ( _stscanf( strLine.Mid( nData + 1 ), _T("%lu"), &nOffsetPrev ) != 1 )
						return false;
				}
				else if ( strEntry.CompareNoCase( _T("root") ) == 0 )
				{
					if ( _stscanf( strLine.Mid( nData + 1 ), _T("%lu"), &nOffsetRoot ) != 1 )
						return false;
				}
			}
			strLine = ReadPDFLine( hFile, false, true );
		}
		nOffset = nOffsetPrev;
	}

	// collect author, title if file name contains "book" keyword
	bool bBook = ( _tcsistr( pszPath, _T("book") ) != NULL );

	auto_ptr< CXMLElement > pXML( new CXMLElement(
		NULL, bBook ? _T("book") : _T("wordprocessing") ) );

	if ( LPCTSTR pszName = _tcsrchr( pszPath, '\\' ) )
	{
		pszName++;

		if ( _tcsnicmp( pszName, _T("ebook - "), 8 ) == 0 )
		{
			strLine = pszName + 8;
			strLine = strLine.SpanExcluding( _T(".") );
			strLine.TrimLeft();
			strLine.TrimRight();
			pXML->AddAttribute( _T("title"), strLine );
		}
		else if ( _tcsnicmp( pszName, _T("(ebook"), 6 ) == 0 )
		{
			if ( ( pszName = _tcschr( pszName, ')' ) ) != NULL )
			{
				if ( _tcsncmp( pszName, _T(") - "), 4 ) == 0 )
					strLine = pszName + 4;
				else
					strLine = pszName + 1;
				strLine = strLine.SpanExcluding( _T(".") );
				strLine.TrimLeft();
				strLine.TrimRight();
				pXML->AddAttribute( _T("title"), strLine );
			}
		}
	}

	// document information is not available--exit
	if ( nOffsetInfo == 0 && nOffsetRoot == 0 && !bBook )
		return false;

/*	// Get XMP metadata; Not implemented, we should prefer XMP if the file creation time was less
	// than metadata timestamp
	// Get matadata from catalog dictionary if available
	DWORD nOffsetMeta = 0;
	if ( nOffsetRoot != 0 )
	{
		CString strSeek;
		strSeek.Format( _T("%lu 0 obj"), nOffsetRoot );
		SetFilePointer( hFile, pOffset[ nOffsetRoot ], NULL, FILE_BEGIN );
		strLine = ReadLine( hFile, (LPCTSTR)_T("<") );
		if ( strLine == strSeek )
		{
			if ( ReadLine( hFile, (LPCTSTR)_T("<") ).IsEmpty() &&
				 ReadLine( hFile, (LPCTSTR)_T("/") ).IsEmpty() &&
				 ReadLine( hFile, (LPCTSTR)_T("/") ).MakeLower() == _T("type") &&
				 ReadLine( hFile, (LPCTSTR)_T("/") ).MakeLower() == _T("catalog") )
			{
				strLine = ReadLine( hFile, true );
				while ( !strLine.IsEmpty() )
				{
					CString strEntry;
					int nData = 0;
					nData = strLine.Find( _T(" ") );
					strEntry = ToLower( strLine.Left( nData ) );
					if ( strEntry == _T("metadata") )
					{
						if ( _stscanf( strLine.Mid( nData + 1 ), _T("%lu"), &nOffsetMeta ) != 1 )
							return false;
					}
					strLine = ReadLine( hFile, true );
				}
			}
		}
	}

	if ( nOffsetMeta != 0 )
	{
		SetFilePointer( hFile, pOffset[ nOffsetMeta ], NULL, FILE_BEGIN );
		strLine = ReadLine( hFile ); //xxx 0 obj
		strLine = ReadLine( hFile ); //<</Type /Matadata /Subtype /XML /Length xxx
		strLine = ReadLine( hFile ); //stream
		strLine = ReadLine( hFile ); //XML metadata
		strLine = ReadLine( hFile ); //endstream
		strLine = ReadLine( hFile ); //endobj
	}*/

	// No page number in info, count manually
	if ( nPages == 0 )
	{
		int nObjPos = 0;
		for ( nOffset = 0 ; nOffset < nCount ; nOffset++ )
		{
			if ( pOffset[ nOffset ] == 0 )
				continue;
			SetFilePointer( hFile, pOffset[ nOffset ], NULL, FILE_BEGIN );

			strLine = ReadPDFLine( hFile, false, true );
			nObjPos = strLine.Find( _T("obj") );
			if ( nObjPos < 0 )
				break;

			// object after object, so we read more than one
			if ( strLine.Find( _T("obj"), nObjPos + 1 ) != -1 )
				continue;

			if ( ReadPDFLine( hFile, false, true ).IsEmpty() )
			{
				strLine = ReadPDFLine( hFile, false, true );
				if ( strLine.IsEmpty() )
					strLine = ReadPDFLine( hFile, false, true );
				while ( ! strLine.IsEmpty() )
				{
					if ( strLine.CompareNoCase( _T("type") ) == 0 )
					{
						strLine = ReadPDFLine( hFile, false, true );
						if ( strLine.CompareNoCase( _T("page") ) == 0 )
							nPages++;
						break;
					}
					strLine = ReadPDFLine( hFile, false, true );
				}
			}
		}
	}

	// get metadata from info object if available
	if ( nOffsetInfo )
	{
		CString strSeek;
		strSeek.Format( _T("%lu 0 obj"), nOffsetInfo );
		SetFilePointer( hFile, pOffset[ nOffsetInfo ], NULL, FILE_BEGIN );
		strLine = ReadPDFLine( hFile, false, true, false );
		if ( strLine == strSeek )
		{
			if ( ! ReadPDFLine( hFile, false, true ).IsEmpty() )
				return false;
			strLine = ReadPDFLine( hFile, false, true );
			if ( strLine.IsEmpty() )
				strLine = ReadPDFLine( hFile, false, true );
			while ( ! strLine.IsEmpty() )
			{
				CString strEntry;
				int nData = strLine.Find( _T("(") );
				if ( nData > 0 )
				{
					strEntry = strLine.Left( nData ).Trim();
					strLine = strLine.Mid( nData );
				}
				else
				{
					nData = strLine.Find( _T("<") );
					if ( nData > 0 )
					{
						strEntry = strLine.Left( nData ).Trim();
						strLine = strLine.Mid( nData );
					}
					else
					{
						strEntry = strLine;
						strLine.Empty();
					}
				}

				// read further if string reading was stopped at "/>" characters
				// inside parentheses and restore missing character
				if ( strLine.GetAt( 0 ) == _T('(') && strLine.Right( 1 ) != _T(')') )
				{
					DWORD nRead = 1;
					while ( nRead )
					{
						// Restore character
						CHAR cChar;
						SetFilePointer( hFile, -1, NULL, FILE_CURRENT );
						ReadFile( hFile, &cChar, 1, &nRead, NULL );
						strLine += cChar;

						CString strNextLine = ReadPDFLine( hFile, false, true );

						// Workaround for string with embedded zero bytes
						for ( int i = 0; i < strNextLine.GetLength(); i++ )
							strLine += strNextLine.GetAt( i );

						if ( strLine.GetAt( strLine.GetLength() - 1 ) == _T(')') )
							break;
					}
				}
				if ( strEntry.CompareNoCase( _T("title") ) == 0 )
					pXML->AddAttribute( _T("title"), DecodePDFText( strLine ) );
				else if ( strEntry.CompareNoCase( _T("author") ) == 0 )
					pXML->AddAttribute( _T("author"), DecodePDFText( strLine ) );
				else if ( strEntry.CompareNoCase( _T("subject") ) == 0 )
					pXML->AddAttribute( _T("subject"), DecodePDFText( strLine ) );
				else if ( strEntry.CompareNoCase( _T("keywords") ) == 0 )
					pXML->AddAttribute( _T("keywords"), DecodePDFText( strLine ) );
				else if ( strEntry.CompareNoCase( _T("company") ) == 0 )
				{
					if ( bBook )
						pXML->AddAttribute( _T("publisher"), DecodePDFText( strLine ) );
					else
						pXML->AddAttribute( _T("copyright"), DecodePDFText( strLine ) );
				}
				//else if ( strEntry.CompareNoCase( _T("creator") ) == 0 )
				//else if ( strEntry.CompareNoCase( _T("producer") ) == 0 )
				//else if ( strEntry.CompareNoCase( _T("moddate") ) == 0 )
				//else if ( strEntry.CompareNoCase( _T("creationdate") ) == 0 )
				//else if ( strEntry.CompareNoCase( _T("sourcemodified") ) == 0 )

				strLine = ReadPDFLine( hFile, false, true );
			}
		}
	}

	if ( nPages > 0 )
	{
		strLine.Format( _T("%lu"), nPages );
		pXML->AddAttribute( _T("pages"), strLine );
	}

	if ( bBook )
	{
		pXML->AddAttribute( _T("format"), _T("PDF") );
		pXML->AddAttribute( _T("back"), _T("Digital") );
		return LibraryBuilder.SubmitMetadata( nIndex, CSchema::uriBook, pXML.release() ) != 0;
	}
	else
	{
		pXML->AddAttribute( _T("format"), _T("Adobe Acrobat PDF") );
		CString strTemp;
		strTemp.Format( _T("1.%i"), nVersion );
		pXML->AddAttribute( _T("formatVersion"), strTemp );
		return LibraryBuilder.SubmitMetadata( nIndex, CSchema::uriDocument, pXML.release() ) != 0;
	}
}

inline char unhex(TCHAR c)
{
	if ( c >= _T('0') && c <= _T('9') )
		return ( c - _T('0') );
	else if ( c >= _T('A') && c <= _T('F') )
		return ( c - _T('A') + 10 );
	else if ( c >= _T('a') && c <= _T('f') )
		return ( c - _T('a') + 10 );
	else
		return 0;
}

CString	CLibraryBuilderInternals::DecodePDFText(CString strInput)
{
	if ( strInput.GetLength() < 2 )
		return strInput;

	bool bHex = false;
	CHAR nFactor = 1;

	if ( strInput.GetAt( 0 ) == '(' && strInput.Right( 1 ) == _T(")") )
	{
		strInput = strInput.Mid( 1, strInput.GetLength() - 2 );
		// Acrobat Reader doesn't decode (<XX>) strings created
		// by Acrobat Distiller 6 but we do
		if ( strInput.GetAt( 0 ) == '<' && strInput.Right( 1 ) == _T(">") )
		{
			bHex = true; // hexadecimal encoding
			nFactor = 2;
			strInput.Replace( L"\\ ", L"" );
			strInput = strInput.Mid( 1, strInput.GetLength() - 2 );
			if ( strInput.GetLength() % 2 != 0 )
				strInput.Append( _T("0") );
		}
	}
	else if ( strInput.GetAt( 0 ) == '<' )
	{
		bHex = true; // hexadecimal encoding
		nFactor = 2;
		strInput.Replace( L"\\ ", L"" );
		strInput = strInput.Mid( 1, strInput.GetLength() - 2 );

		// the last zero can be omitted
		if ( strInput.GetLength() % 2 != 0 )
			strInput.Append( _T("0") );
	}
	else
		return strInput;

	if ( strInput.IsEmpty() )
		return CString();

	CString strResult, strTemp;
	bool bWide = false;
	DWORD nByte = strInput.GetLength() / nFactor; // string length in bytes

	if ( bHex && strInput.Left( 4 ) == L"FEFF" )
	{
		bWide = true;
	}

	auto_array< WCHAR > pByte( new WCHAR[ nByte + 1 ] );

	if ( bHex )
	{
		LPCTSTR p = strInput;
		if ( bWide )
		{
			for ( DWORD nHex = 0 ; nHex < nByte / 2; nHex++, p += 4 )
			{
				pByte[ nHex ] =
					(WCHAR)unhex( p[ 0 ] ) << 12 |
					(WCHAR)unhex( p[ 1 ] ) <<  8 |
					(WCHAR)unhex( p[ 2 ] ) <<  4 |
					(WCHAR)unhex( p[ 3 ] );
			}
			pByte[ nByte / 2 ] = 0;
		}
		else
		{
			for ( DWORD nHex = 0 ; nHex < nByte; nHex++, p += 2 )
			{
				pByte[ nHex ] =
					(WCHAR)unhex( p[ 0 ] ) <<  4 |
					(WCHAR)unhex( p[ 1 ] );
			}
			pByte[ nByte ] = 0;
		}
	}
	else
	{
		DWORD nShift = 0;
		for ( DWORD nChar = 0 ; nChar < nByte && nChar >= nShift; nChar++ )
		{
			WCHAR nTemp = strInput.GetAt( nChar );
			if ( nTemp == '\\' && nChar + 1 < nByte )
			{
				nTemp = strInput.GetAt( nChar + 1 );
				if ( nTemp == 't' )
					pByte[ nChar - nShift ] = '\t';
				else if ( nTemp == 'r' )
					pByte[ nChar - nShift ] = '\r';
				else if ( nTemp == 'n' )
					pByte[ nChar - nShift ] = '\n';
				else if ( nTemp == 'f' )
					pByte[ nChar - nShift ] = '\f';
				else if ( nTemp == 'b' )
					pByte[ nChar - nShift ] = '\b';
				else if ( nTemp == '\\' )
					pByte[ nChar - nShift ] = '\\';
				else if ( nTemp == '(' )
					pByte[ nChar - nShift ] = '(';
				else if ( nTemp == ')' )
					pByte[ nChar - nShift ] = ')';
				else
				{
					// Octal encoding tests
					int nWChar = 0;
					if ( nChar + 3 < nByte &&
						_stscanf( strInput.Mid( nChar + 1, 3 ), _T("%o"), &nWChar ) == 1 )
					{
						pByte[ nChar - nShift ] = WCHAR(nWChar);
						nShift += 2;
						nChar += 2;
					}
					else if ( nChar + 2 < nByte &&
						_stscanf( strInput.Mid( nChar + 1, 2 ), _T("%o"), &nWChar ) == 1 )
					{
						pByte[ nChar - nShift ] = WCHAR(nWChar);
						nShift++;
						nChar++;
					}
					else if ( _stscanf( strInput.Mid( nChar + 1, 1 ), _T("%o"), &nWChar ) == 1 )
					{
						pByte[ nChar - nShift ] = WCHAR(nWChar);
					}
					// backslash with a space is ignored--the backslash at the end just breaks a line
					// (we replaced separators while reading a file)
					else if ( strInput.Mid( nChar + 1, 1 ) != L" " )
					{
						// if everything else only backslash is ignored
						nShift++;
						continue;
					}
					else
						nShift++;
				}
				nShift++;
				nChar++;
			}
			else
				pByte[ nChar - nShift ] = nTemp;
		}
		nByte -= nShift;
	}

	short bCharsToMove = 0;
	if ( nByte > 2 )
	{
		if ( pByte[ 0 ] == 0xFEFF )	// UTF-16, big endian
			bCharsToMove = 1;
		else if ( pByte[ 0 ] == 0x00FE && pByte[ 1 ] == 0x00FF )
			bCharsToMove = 2;
	}

	if ( bWide || bCharsToMove == 0 )
	{
		CopyMemory( strResult.GetBuffer( nByte ),
			pByte.get() + bCharsToMove, ( nByte - bCharsToMove ) * sizeof( WCHAR ) );
		strResult.ReleaseBuffer( nByte - bCharsToMove );
	}
	else
	{
		nByte = ( nByte - bCharsToMove ) / 2;
		WCHAR* pszDest = strResult.GetBuffer( nByte );
		for ( DWORD nPos = 0 ; nPos < nByte ; nPos++ )
		{
			pszDest[ nPos ] =
				pByte[ nPos * 2 + 1 + bCharsToMove ] |
				pByte[ nPos * 2     + bCharsToMove ] << 8;
		}
		strResult.ReleaseBuffer( nByte );
	}


	// strip off language and country codes
	// could be useful in the future...
	int nEscapeStart = 0;
	do
	{
		nEscapeStart = strResult.Find( L"\x001B" );
		if ( nEscapeStart != -1 )
		{
			int nEscapeEnd = strResult.Find( L"\x001B", nEscapeStart + 1 );
			if ( nEscapeEnd != -1 )
				strResult = strResult.Left( nEscapeStart - 1 ) + strResult.Mid( nEscapeEnd + 1 );
			else
				strResult = strResult.Mid( nEscapeStart + 1 );
		}
	}
	while ( nEscapeStart != -1 );

	return strResult.Trim();
}

CString CLibraryBuilderInternals::ReadPDFLine(HANDLE hFile, bool bReverse, bool bComplex, bool bSplitter)
{
	bool bGt = false, bLt = false;
	DWORD nRead, nLength;
	TCHAR cChar = 0;
	CString str;

	for ( nLength = 0 ; ReadFile( hFile, &cChar, 1, &nRead, NULL ) && nRead == 1 && nLength++ < 4096 ; )
	{
		if ( bReverse )
			if ( SetFilePointer( hFile, -2, NULL, FILE_CURRENT ) == 0 )
				break;
		if ( ! bComplex )
		{
			// lines can end with \r, \n or \r\n
			if ( bReverse )
			{
				if ( cChar == '\r' )
					break;
				if ( cChar == '\n' )
				{
					ReadFile( hFile, &cChar, 1, &nRead, NULL );
					if ( cChar == '\r' )
						SetFilePointer( hFile, -2, NULL, FILE_CURRENT );
					else
						SetFilePointer( hFile, -1, NULL, FILE_CURRENT );
					break;
				}
			}
			else
			{
				if ( cChar == '\n' )
					break;
				if ( cChar == '\r' )
				{
					ReadFile( hFile, &cChar, 1, &nRead, NULL );
					if ( cChar != '\n' )
						SetFilePointer( hFile, -1, NULL, FILE_CURRENT );
					break;
				}
			}
		}
		else
		{
			// Format: <</parameter1/parameter2...>>\r\n
			if ( bSplitter && cChar == '/' )	// "/" - parameter splitter
			{
				if ( str.IsEmpty() )
					continue;					// skip first "/"
				else
					break;						// got ending "/"
			}
			else if ( cChar == '>' )			// ">>" - end of parameters
			{
				if ( bGt )
				{
					if ( bReverse )
					{
						str = str.Right( str.GetLength() - 1 );
						if ( ! str.IsEmpty() )
							SetFilePointer( hFile, 2, NULL, FILE_CURRENT );
					}
					else
					{
						str = str.Left( str.GetLength() - 1 );
						if ( ! str.IsEmpty() )
							SetFilePointer( hFile, -2, NULL, FILE_CURRENT );
					}
					break;
				}
				bGt = true;
			}
			else if ( cChar == '<' )			// "<<" - start of parameters
			{
				if ( bLt )
				{
					if ( bReverse )
					{
						str = str.Right( str.GetLength() - 1 );
						if ( ! str.IsEmpty() )
							SetFilePointer( hFile, 2, NULL, FILE_CURRENT );
					}
					else
					{
						str = str.Left( str.GetLength() - 1 );
						if ( ! str.IsEmpty() )
							SetFilePointer( hFile, -2, NULL, FILE_CURRENT );
					}
					break;
				}
				bLt = true;
			}
			else
			{
				bGt = false;
				bLt = false;
				if ( cChar == '\n' || cChar == '\r' )
					cChar = ' ';
			}
		}
		if ( bReverse )
			str = cChar + str;
		else
			str += cChar;
	}

	str.TrimLeft();

	// workaround to trim from right if zero bytes are present
	// between the beginning and the end
	nLength = str.GetLength();
	while ( nLength && str.GetAt( nLength - 1 ) == ' ' )
	{
		str = str.Left( nLength - 1 );
		nLength--;
	}
	return str;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals Collection (threaded)

bool CLibraryBuilderInternals::ReadCollection(DWORD nIndex, LPCTSTR pszPath)
{
	CCollectionFile pCollection;
	if ( ! pCollection.Open( pszPath ) )
		return false;

	Hashes::Sha1Hash oSHA1;
	{
		CQuickLock oLibraryLock( Library.m_pSection );
		CLibraryFile* pFile = LibraryMaps.LookupFile( nIndex );
		if ( !pFile || !pFile->m_oSHA1 )
			return false;
		oSHA1 = pFile->m_oSHA1;
	}

	LibraryFolders.MountCollection( oSHA1, &pCollection );

	if ( CXMLElement* pMetadata = pCollection.GetMetadata() )
	{
		pMetadata = pMetadata->GetFirstElement()->Clone();
		return LibraryBuilder.SubmitMetadata( nIndex, pCollection.GetThisURI(), pMetadata ) != 0;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals CHM (threaded)

bool CLibraryBuilderInternals::ReadCHM(DWORD nIndex, HANDLE hFile, LPCTSTR pszPath)
{
	CHAR szMagic[4];
	DWORD nVersion, nIHDRSize, nLCID, nRead, nPos, nComprVersion;
	QWORD nContentOffset;
	const DWORD MAX_LENGTH_ALLOWED = 8192;

	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	ReadFile( hFile, szMagic, 4, &nRead, NULL );

	if ( nRead != 4 || strncmp( szMagic, "ITSF", 4 ) )
		return LibraryBuilder.SubmitCorrupted( nIndex );
	if ( GetFileSize( hFile, NULL ) < 510 )
		return LibraryBuilder.SubmitCorrupted( nIndex );

	// Get CHM file version number
	ReadFile( hFile, &nVersion, sizeof(nVersion), &nRead, NULL );
	if ( nRead != sizeof(nVersion) || nVersion < 3 )
		return false; // In Version 2 files, content section data offset is not there

	// Get initial header size
	ReadFile( hFile, &nIHDRSize, sizeof(nIHDRSize), &nRead, NULL );
	if ( nRead != sizeof(nIHDRSize) || nIHDRSize == 0 )
		return LibraryBuilder.SubmitCorrupted( nIndex );
	nPos = nIHDRSize - sizeof(nContentOffset);

	// Get Windows LCID of machine on which the file was compiled;
	// Always located at offset 20
	SetFilePointer( hFile, 20, NULL, FILE_BEGIN );
	ReadFile( hFile, &nLCID, sizeof(nLCID), &nRead, NULL );
	if ( nRead != sizeof(nLCID) )
		return LibraryBuilder.SubmitCorrupted( nIndex );
	if ( !IsValidLocale( nLCID, LCID_SUPPORTED ) )
		nLCID = 1033;

	// Read the last qword from the end of header; it contains content section data offset
	SetFilePointer( hFile, nPos, NULL, FILE_BEGIN );
	ReadFile( hFile, &nContentOffset, sizeof(nContentOffset), &nRead, NULL );
	if ( nRead != sizeof(nContentOffset) )
		return LibraryBuilder.SubmitCorrupted( nIndex );
	if ( nContentOffset == 0 )
		return false;

	// Go to compressed control data and check version;
	// Content section data always takes 110 bytes (?)
	nContentOffset += 110;
	DWORD nError = NO_ERROR;
	DWORD nSizeLow	= (DWORD)( nContentOffset & 0xFFFFFFFF );
	DWORD nSizeHigh	= (DWORD)( nContentOffset >> 32 );

	nSizeLow = SetFilePointer( hFile, nSizeLow, (long*)&nSizeHigh, FILE_BEGIN );
	if ( nSizeLow == INVALID_SET_FILE_POINTER &&
		( nError = GetLastError() ) != NO_ERROR )
	{
		return LibraryBuilder.SubmitCorrupted( nIndex );
	}
	ReadFile( hFile, szMagic, 4, &nRead, NULL );
	if ( nRead != 4 || strncmp( szMagic, "LZXC", 4 ) ) // compression method
		return false;
	ReadFile( hFile, &nComprVersion, sizeof(nComprVersion), &nRead, NULL );
	if ( nRead != sizeof(nComprVersion) || nComprVersion != 2 ) // Note: MS Reader books has version 3
		return false;

	// Read no more than 8192 bytes to find "HHA Version" string
	CHAR szByte[1];
	CHAR szFragment[16] = {}; // // "HA Version" string
	bool bCorrupted = false;
	bool bHFound = false;
	int nFragmentPos = 0;

	for ( nPos = 0; ReadFile( hFile, &szByte, 1, &nRead, NULL ) && nPos++ < MAX_LENGTH_ALLOWED ; )
	{
		if ( nRead != 1 )
		{
			bCorrupted = true;
			break;
		}
		if ( szByte[0] == 'H' )
		{
			nFragmentPos = 0;
			szFragment[0] = 'H';
			bHFound = true;
		}
		else
		{
			nFragmentPos++;
			if ( bHFound )
			{
				if ( IsCharacter( szByte[0] ) )
					szFragment[ nFragmentPos ] = szByte[0];
				else
					szFragment[ nFragmentPos ] = ' ';
			}
		}
		if ( nFragmentPos == 9 )
		{
			if ( !strncmp( szFragment, "HA Version", 10 ) )
			{
				// Remember position two words before;
				// the second word is data entry length
				nPos = SetFilePointer( hFile, 0, NULL, FILE_CURRENT ) - 15;
				break;
			}
			else
			{
				nFragmentPos = 0;
				bHFound = false;
			}
		}
	}
	if ( bCorrupted )
	{
		return LibraryBuilder.SubmitCorrupted( nIndex );
	}
	if ( strncmp( szFragment, "HA Version", 10 ) && nPos == MAX_LENGTH_ALLOWED + 1 )
	{
		return false;
	}

	// Collect author, title if file name contains "book" keyword
	CString strLine;
	bool bBook = ( _tcsistr( pszPath, _T("book") ) != NULL );

	auto_ptr< CXMLElement > pXML( new CXMLElement(
		NULL, bBook ? _T("book") : _T("wordprocessing") ) );

	if ( LPCTSTR pszName = _tcsrchr( pszPath, '\\' ) )
	{
		pszName++;

		if ( _tcsnicmp( pszName, _T("ebook - "), 8 ) == 0 )
		{
			strLine = pszName + 8;
			strLine = strLine.SpanExcluding( _T(".") );
			strLine.TrimLeft();
			strLine.TrimRight();
			pXML->AddAttribute( _T("title"), strLine );
		}
		else if ( _tcsnicmp( pszName, _T("(ebook"), 6 ) == 0 )
		{
			if ( ( pszName = _tcschr( pszName, ')' ) ) != NULL )
			{
				if ( _tcsncmp( pszName, _T(") - "), 4 ) == 0 )
					strLine = pszName + 4;
				else
					strLine = pszName + 1;
				strLine = strLine.SpanExcluding( _T(".") );
				strLine.TrimLeft();
				strLine.TrimRight();
				pXML->AddAttribute( _T("title"), strLine );
			}
		}
	}

	// Meta data extraction
	WORD nData;
	CHARSETINFO csInfo;
	CString strTemp;
	TCHAR *pszBuffer = NULL;
	UINT nCodePage = CP_ACP;
	DWORD nCwc;
	DWORD_PTR charSet = DEFAULT_CHARSET;
	bool bHasTitle = false;

	// Find default ANSI codepage for given LCID
	DWORD nLength = GetLocaleInfo( nLCID, LOCALE_IDEFAULTANSICODEPAGE, NULL, 0 );
	pszBuffer = (TCHAR*)LocalAlloc( LPTR, ( nLength + 1 ) * sizeof(TCHAR) );
	nCwc = GetLocaleInfo( nLCID, LOCALE_IDEFAULTANSICODEPAGE, pszBuffer, nLength );
	if ( nCwc > 0 )
	{
		CString strTemp = pszBuffer;
		strTemp = strTemp.Left( nCwc - 1 );
		_stscanf( strTemp, _T("%lu"), &charSet );
		if ( TranslateCharsetInfo( (LPDWORD)charSet, &csInfo, TCI_SRCCODEPAGE ) )
			nCodePage = csInfo.ciACP;
	}
	SetFilePointer( hFile, nPos, NULL, FILE_BEGIN );

	for ( int nCount = 1 ; nCount < 5 && !bCorrupted ; nCount++ ) // nCount may be up to 6
	{
		// Unknown data
		ReadFile( hFile, &nData, sizeof(nData), &nRead, NULL );
		if ( nRead != sizeof(nData) )
			bCorrupted = true;

		// Entry length
		ReadFile( hFile, &nData, sizeof(nData), &nRead, NULL );
		if ( nRead != sizeof(nData) )
			bCorrupted = true;
		if ( nData == 0 )
			break;
		if ( bCorrupted )
			nData = 1;

		auto_array< CHAR > szMetadata( new CHAR[ nData ] );
		ReadFile( hFile, szMetadata.get(), nData, &nRead, NULL );
		if ( nRead != nData )
			bCorrupted = true;

		if ( nCount == 2 )
			continue;

		// Convert meta data string from ANSI to unicode
		int nWide = MultiByteToWideChar( nCodePage, 0, szMetadata.get(), nData, NULL, 0 );
		LPWSTR pszOutput = strLine.GetBuffer( nWide + 1 );
		MultiByteToWideChar( nCodePage, 0, szMetadata.get(), nData, pszOutput, nWide );
		pszOutput[ nWide ] = 0;
		strLine.ReleaseBuffer();
		strLine.Trim();

		int nPos;

		switch ( nCount )
		{
		case 1: // version number
			nPos = strLine.ReverseFind( ' ' );
			strLine = strLine.Mid( nPos + 1 );
			if ( !bBook )
				pXML->AddAttribute( _T("formatVersion"), strLine );
			break;
		case 2: // unknown data
			break;
		case 3: // redirection url
			ToLower( strLine );
			if ( strLine.Left( 7 ) == _T("ms-its:") )
			{
				nPos = strLine.Find( _T("::"), 7 );
				strTemp = _tcsrchr( pszPath, '\\' );
				strTemp = strTemp.Mid( 1 );
				ToLower( strTemp );
				if ( strLine.Mid( 7, nPos - 7 ).Trim() != strTemp )
					bCorrupted = true; // it requires additional file
			}
			else if ( strLine.Left( 7 ) == _T("http://") )
				bCorrupted = true; // redirects to external resource; may be dangerous
			break;
		case 4: // title
			if ( strLine.IsEmpty() )
				break;
			nPos = strLine.Find( ',' );
			strTemp = strLine.Left( nPos );
			ToLower( strTemp );
			if ( strLine.CompareNoCase( _T("htmlhelp") ) != 0 &&
				 strTemp != _T("arial") && strTemp != _T("tahoma") &&
				 strTemp != _T("times new roman") && strTemp != _T("verdana") &&
				 strLine.CompareNoCase( _T("windows") ) != 0 )
			{
				bHasTitle = true;
				nPos = strLine.ReverseFind( '\\' ); // remove paths in title
				strLine = strLine.Mid( nPos + 1 );
				pXML->AddAttribute( _T("title"), strLine );
			}
			break;
		}
		if ( bCorrupted )
			return LibraryBuilder.SubmitCorrupted( nIndex );
	}

	if ( !bHasTitle )
		return false;

	pXML->AddAttribute( _T("format"), _T("Compiled HTML Help") );
	if ( bBook )
	{
		pXML->AddAttribute( _T("back"), _T("Digital") );
		strTemp = CSchema::uriBook;
	}
	else
		strTemp = CSchema::uriDocument;

	return LibraryBuilder.SubmitMetadata( nIndex, strTemp, pXML.release() ) != 0;
}


//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals TORRENT

bool CLibraryBuilderInternals::ReadTorrent(DWORD nIndex, HANDLE /*hFile*/, LPCTSTR pszPath)
{
	CBTInfo oTorrent;
	if ( oTorrent.LoadTorrentFile( pszPath ) )
	{
		auto_ptr< CXMLElement > pXML( new CXMLElement( NULL, L"torrent" ) );

		if ( oTorrent.m_oBTH )
			pXML->AddAttribute( L"hash", oTorrent.m_oBTH.toString() );
		if ( oTorrent.HasTracker() )
			pXML->AddAttribute( L"tracker", oTorrent.GetTrackerAddress() );
		if ( oTorrent.m_nEncoding )
		{
			CString sEncoding;
			sEncoding.Format( _T("CP%u"), oTorrent.m_nEncoding );
			pXML->AddAttribute( L"encoding", sEncoding );
		}
		if ( oTorrent.m_tCreationDate )
		{
			CTime oTime( (time_t)oTorrent.m_tCreationDate );
			pXML->AddAttribute( L"creationdate", oTime.Format( _T("%Y-%m-%d  %H:%M") ) );
		}
		if ( oTorrent.m_sCreatedBy.GetLength() )
			pXML->AddAttribute( L"createdby", oTorrent.m_sCreatedBy );
		if ( oTorrent.m_sComment.GetLength() )
			pXML->AddAttribute( L"comments", oTorrent.m_sComment );
		pXML->AddAttribute( L"privateflag", oTorrent.m_bPrivate ? L"true" : L"false" );

		return LibraryBuilder.SubmitMetadata( nIndex, CSchema::uriBitTorrent, pXML.release() ) != 0;
	}
	return false;
}
