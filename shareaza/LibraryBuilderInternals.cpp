//
// LibraryBuilderInternals.cpp
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
#include "LibraryFolders.h"
#include "LibraryBuilder.h"
#include "LibraryBuilderInternals.h"

#define _ID3_DEFINE_GENRES
#include "Buffer.h"
#include "Schema.h"
#include "XML.h"
#include "ID3.h"
#include "Packet.h"
#include "CollectionFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals construction

CLibraryBuilderInternals::CLibraryBuilderInternals(CLibraryBuilder* pBuilder)
{
	m_pBuilder = pBuilder;
}

CLibraryBuilderInternals::~CLibraryBuilderInternals()
{
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals load settings

void CLibraryBuilderInternals::LoadSettings()
{
	m_bEnableMP3	= theApp.GetProfileInt( _T("Library"), _T("ScanMP3"), TRUE );
	m_bEnableEXE	= theApp.GetProfileInt( _T("Library"), _T("ScanEXE"), TRUE );
	m_bEnableImage	= theApp.GetProfileInt( _T("Library"), _T("ScanImage"), TRUE );
	m_bEnableASF	= theApp.GetProfileInt( _T("Library"), _T("ScanASF"), TRUE );
	m_bEnableOGG	= theApp.GetProfileInt( _T("Library"), _T("ScanOGG"), TRUE );
	m_bEnableAPE	= theApp.GetProfileInt( _T("Library"), _T("ScanAPE"), TRUE );
	m_bEnableAVI	= theApp.GetProfileInt( _T("Library"), _T("ScanAVI"), TRUE );
	m_bEnablePDF	= theApp.GetProfileInt( _T("Library"), _T("ScanPDF"), TRUE );
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals extract metadata (threaded)

BOOL CLibraryBuilderInternals::ExtractMetadata( CString& strPath, HANDLE hFile, SHA1* pSHA1)
{
	CString strType;
	
	int nExtPos = strPath.ReverseFind( '.' );
	if ( nExtPos > 0 ) strType = strPath.Mid( nExtPos );
	
	strType.MakeLower();
	
	if ( strType == _T(".mp3") )
	{
		if ( ! m_bEnableMP3 ) return FALSE;
		if ( ReadID3v2( hFile ) ) return TRUE;
		if ( ReadID3v1( hFile ) ) return TRUE;
		if ( ReadMP3Frames( hFile ) ) return TRUE;
		return SubmitCorrupted();
	}
	else if ( strType == _T(".exe") || strType == _T(".dll") )
	{
		if ( ! m_bEnableEXE ) return FALSE;
		return ReadVersion( strPath );
	}
	else if ( strType == _T(".asf") || strType == _T(".wma") || strType == _T(".wmv") )
	{
		if ( ! m_bEnableASF ) return FALSE;
		return ReadASF( hFile );
	}
	else if ( strType == _T(".avi") )
	{
		if ( ! m_bEnableAVI ) return FALSE;
		return ReadAVI( hFile );
	}
	else if ( strType == _T(".mpg") || strType == _T(".mpeg") )
	{
		if ( ! m_bEnableASF ) return FALSE;
		return ReadMPEG( hFile );
	}
	else if ( strType == _T(".ogg") )
	{
		if ( ! m_bEnableOGG ) return FALSE;
        return ReadOGG( hFile );
	}
	else if ( strType == _T(".ape") || strType == _T(".mac") || strType == _T(".apl") )
	{
		if ( ! m_bEnableAPE ) return FALSE;
		return ReadAPE( hFile );
	}
	else if ( strType == _T(".jpg") || strType == _T(".jpeg") )
	{
		if ( ! m_bEnableImage ) return FALSE;
		return ReadJPEG( hFile );
	}
	else if ( strType == _T(".gif") )
	{
		if ( ! m_bEnableImage ) return FALSE;
		return ReadGIF( hFile );
	}
	else if ( strType == _T(".png") )
	{
		if ( ! m_bEnableImage ) return FALSE;
		return ReadPNG( hFile );
	}
	else if ( strType == _T(".bmp") )
	{
		if ( ! m_bEnableImage ) return FALSE;
		return ReadBMP( hFile );
	}
	else if ( strType == _T(".pdf") )
	{
		if ( ! m_bEnablePDF ) return FALSE;
		return ReadPDF( hFile, strPath );
	}
	else if ( strType == _T(".co") || strType == _T(".collection") )
	{
		return ReadCollection( hFile, pSHA1 );
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals submit metadata (threaded)

BOOL CLibraryBuilderInternals::SubmitMetadata( LPCTSTR pszSchemaURI, CXMLElement* pXML)
{
	// Ignoring return value from submission
	m_pBuilder->SubmitMetadata( pszSchemaURI, pXML );
	return TRUE;
}

BOOL CLibraryBuilderInternals::SubmitCorrupted()
{
	return m_pBuilder->SubmitCorrupted();
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals ID3v1 (threaded)

BOOL CLibraryBuilderInternals::ReadID3v1( HANDLE hFile, CXMLElement* pXML)
{
	if ( GetFileSize( hFile, NULL ) < 128 ) return FALSE;
	
	ID3V1 pInfo;
	DWORD nRead;

	SetFilePointer( hFile, -128, NULL, FILE_END );
	ReadFile( hFile, &pInfo, sizeof(pInfo), &nRead, NULL );
	
	if ( nRead != sizeof(pInfo) ) return FALSE;
	if ( strncmp( pInfo.szTag, ID3V1_TAG, 3 ) ) return FALSE;
	
	BOOL bIsMP3 = ( pXML == NULL );
	if ( bIsMP3 ) pXML = new CXMLElement( NULL, _T("audio") );
	
	CopyID3v1Field( pXML, _T("title"), pInfo.szSongname, 30 );
	CopyID3v1Field( pXML, _T("artist"), pInfo.szArtist, 30 );
	CopyID3v1Field( pXML, _T("album"), pInfo.szAlbum, 30 );
	CopyID3v1Field( pXML, _T("year"), pInfo.szYear, 4 );
	
	if ( pInfo.nGenre < ID3_GENRES )
	{
		pXML->AddAttribute( _T("genre"), pszID3Genre[ pInfo.nGenre ] );
	}
	
	if ( pInfo.szComment[28] == 0 && pInfo.szComment[29] > 0 )
	{
		CString strTrack;
		strTrack.Format( _T("%i"), (int)pInfo.szComment[29] );
		pXML->AddAttribute( _T("track"), strTrack );
		CopyID3v1Field( pXML, _T("description"), pInfo.szComment, 28 );
	}
	else
	{
		CopyID3v1Field( pXML, _T("description"), pInfo.szComment, 30 );
	}
	
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	
	if ( bIsMP3 )
	{
		ScanMP3Frame( pXML, hFile, sizeof(pInfo) );
		return SubmitMetadata( CSchema::uriAudio, pXML );
	}
	
	return TRUE;
}

BOOL CLibraryBuilderInternals::CopyID3v1Field(CXMLElement* pXML, LPCTSTR pszAttribute, LPCSTR pszValue, int nLength)
{
	CString strValue;
	int nWide = MultiByteToWideChar( CP_ACP, 0, pszValue, nLength, NULL, 0 );
    LPWSTR pszOutput = strValue.GetBuffer( nWide + 1 );
	MultiByteToWideChar( CP_ACP, 0, pszValue, nLength, pszOutput, nWide );
	pszOutput[ nWide ] = 0;
	strValue.ReleaseBuffer();
	
	strValue.TrimLeft();
	strValue.TrimRight();
	if ( strValue.IsEmpty() ) return FALSE;
	
	pXML->AddAttribute( pszAttribute, strValue );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals ID3v2 (threaded)

BOOL CLibraryBuilderInternals::ReadID3v2( HANDLE hFile)
{
	ID3V2_HEADER pHeader;
	DWORD nRead;
	
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	ReadFile( hFile, &pHeader, sizeof(pHeader), &nRead, NULL );
	if ( nRead != sizeof(pHeader) ) return FALSE;
	
	if ( strncmp( pHeader.szTag, ID3V2_TAG, 3 ) ) return FALSE;
	if ( pHeader.nMajorVersion < 2 || pHeader.nMajorVersion > 4 ) return FALSE;
	if ( pHeader.nFlags & ~ID3V2_KNOWNMASK ) return FALSE;
	if ( pHeader.nFlags & ID3V2_UNSYNCHRONISED ) return FALSE;
	
	DWORD nBuffer = SWAP_LONG( pHeader.nSize );
	ID3_DESYNC_SIZE( nBuffer );
	
	if ( nBuffer > 1024 * 1024 * 2 ) return FALSE;
	
	BYTE* pBuffer	= new BYTE[ nBuffer ];
	BYTE* pRelease	= pBuffer;
	
	ReadFile( hFile, pBuffer, nBuffer, &nRead, NULL );
	if ( nRead != nBuffer )
	{
		delete [] pRelease;
		return FALSE;
	}
	
	if ( ( pHeader.nFlags & ID3V2_EXTENDEDHEADER ) && pHeader.nMajorVersion == 3 )
	{
		if ( nBuffer < sizeof(ID3V2_EXTENDED_HEADER_1) )
		{
			delete [] pRelease;
			return FALSE;
		}
		
		ID3V2_EXTENDED_HEADER_1* pExtended = (ID3V2_EXTENDED_HEADER_1*)pBuffer;
		pBuffer += sizeof(ID3V2_EXTENDED_HEADER_1);
		nBuffer -= sizeof(ID3V2_EXTENDED_HEADER_1);
		
		pExtended->nSize = SWAP_LONG( pExtended->nSize );
		
		if ( nBuffer < pExtended->nSize )
		{
			delete [] pRelease;
			return FALSE;
		}
		
		pBuffer += pExtended->nSize;
		nBuffer -= pExtended->nSize;
	}
	else if ( ( pHeader.nFlags & ID3V2_EXTENDEDHEADER ) && pHeader.nMajorVersion == 4 )
	{
		if ( nBuffer < sizeof(ID3V2_EXTENDED_HEADER_2) )
		{
			delete [] pRelease;
			return FALSE;
		}
		
		ID3V2_EXTENDED_HEADER_2* pExtended = (ID3V2_EXTENDED_HEADER_2*)pBuffer;
		pBuffer += sizeof(ID3V2_EXTENDED_HEADER_2);
		nBuffer -= sizeof(ID3V2_EXTENDED_HEADER_2);
		
		pExtended->nSize = SWAP_LONG( pExtended->nSize );
		ID3_DESYNC_SIZE( pExtended->nSize );
		pExtended->nSize -= 6;
		
		if ( nBuffer < pExtended->nSize )
		{
			delete [] pRelease;
			return FALSE;
		}
		
		pBuffer += pExtended->nSize;
		nBuffer -= pExtended->nSize;
	}
	
	CXMLElement* pXML = new CXMLElement( NULL, _T("audio") );
	
	while ( TRUE )
	{
		DWORD nFrameSize = 0;
		CHAR szFrameTag[5];
		
		if ( pHeader.nMajorVersion > 2 )
		{
			ID3V2_FRAME* pFrame = (ID3V2_FRAME*)pBuffer;
			
			if ( nBuffer < sizeof(*pFrame) ) break;
			pBuffer += sizeof(*pFrame);
			nBuffer -= sizeof(*pFrame);
			
			szFrameTag[0] = pFrame->szID[0];
			szFrameTag[1] = pFrame->szID[1];
			szFrameTag[2] = pFrame->szID[2];
			szFrameTag[3] = pFrame->szID[3];
			szFrameTag[4] = 0;
			
			nFrameSize = SWAP_LONG( pFrame->nSize );
			if ( pHeader.nMajorVersion >= 4 ) ID3_DESYNC_SIZE( nFrameSize );
			if ( pFrame->nFlags2 & ~ID3V2_KNOWNFRAME ) szFrameTag[0] = 0;
		}
		else
		{
			ID3V2_FRAME_2* pFrame = (ID3V2_FRAME_2*)pBuffer;
			
			if ( nBuffer < sizeof(*pFrame) ) break;
			pBuffer += sizeof(*pFrame);
			nBuffer -= sizeof(*pFrame);
			
			szFrameTag[0] = pFrame->szID[0];
			szFrameTag[1] = pFrame->szID[1];
			szFrameTag[2] = pFrame->szID[2];
			szFrameTag[3] = szFrameTag[4] = 0;
			nFrameSize = ( pFrame->nSize[0] << 16 ) | ( pFrame->nSize[1] << 8 ) | pFrame->nSize[2];
		}
		
		if ( nBuffer < nFrameSize || ! nFrameSize || ! szFrameTag[0] ) break;
		
		if ( strcmp( szFrameTag, "TIT2" ) == 0 || strcmp( szFrameTag, "TT2" ) == 0)
		{
			CopyID3v2Field( pXML, _T("title"), pBuffer, nFrameSize );
		}
		else if ( strcmp( szFrameTag, "TOPE" ) == 0 || strcmp( szFrameTag, "TOA" ) == 0 || strcmp( szFrameTag, "TPE1" ) == 0 || strcmp( szFrameTag, "TPE2" ) == 0 )
		{
			CopyID3v2Field( pXML, _T("artist"), pBuffer, nFrameSize );
		}
		else if ( strcmp( szFrameTag, "TALB" ) == 0 || strcmp( szFrameTag, "TOT" ) == 0 )
		{
			CopyID3v2Field( pXML, _T("album"), pBuffer, nFrameSize );
		}
		else if ( strcmp( szFrameTag, "TRCK" ) == 0 || strcmp( szFrameTag, "TRK" ) == 0 )
		{
			CopyID3v2Field( pXML, _T("track"), pBuffer, nFrameSize );
		}
		else if ( strcmp( szFrameTag, "TYER" ) == 0 || strcmp( szFrameTag, "TYE" ) == 0 )
		{
			CopyID3v2Field( pXML, _T("year"), pBuffer, nFrameSize );
		}
		else if ( strcmp( szFrameTag, "COMM" ) == 0 || strcmp( szFrameTag, "COM" ) == 0 )
		{
			CopyID3v2Field( pXML, _T("description"), pBuffer, nFrameSize, TRUE );
		}
		else if ( strcmp( szFrameTag, "TLEN" ) == 0 || strcmp( szFrameTag, "TLE" ) == 0 )
		{
			if ( CopyID3v2Field( pXML, _T("seconds"), pBuffer, nFrameSize ) )
			{
				CString strMS = pXML->GetAttributeValue( _T("seconds"), _T("0") );
				int nMS;
				_stscanf( strMS, _T("%lu"), &nMS );
				strMS.Format( _T("%lu"), nMS / 1000 );
				pXML->AddAttribute( _T("seconds"), strMS );
			}
		}
		else if ( strcmp( szFrameTag, "TCON" ) == 0 || strcmp( szFrameTag, "TCO" ) == 0 )
		{
			if ( CopyID3v2Field( pXML, _T("genre"), pBuffer, nFrameSize ) )
			{
				CString strGenre = pXML->GetAttributeValue( _T("genre"), _T("") );
				
				while ( TRUE )
				{
					int nPos1 = strGenre.Find( '(' );
					if ( nPos1 < 0 ) break;
					int nPos2 = strGenre.Find( ')' );
					if ( nPos2 <= nPos1 ) break;
					
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
				
				Replace( strGenre, _T("["), _T("(") );
				Replace( strGenre, _T("]"), _T(")") );
				
				pXML->AddAttribute( _T("genre"), strGenre );
			}
		}
		
		pBuffer += nFrameSize;
		nBuffer -= nFrameSize;
	}
	
	delete [] pRelease;
	
	ScanMP3Frame( pXML, hFile, 0 );
	
	return SubmitMetadata( CSchema::uriAudio, pXML );
}

BOOL CLibraryBuilderInternals::CopyID3v2Field(CXMLElement* pXML, LPCTSTR pszAttribute, BYTE* pBuffer, DWORD nLength, BOOL bSkipLanguage)
{
	CString strValue;
	
	BYTE nEncoding = *pBuffer++;
	nLength--;
	
	if ( bSkipLanguage )
	{
		if ( nLength < 3 ) return FALSE;
		pBuffer += 3;
		nLength -= 3;
	}
	
	if ( nEncoding == 0 )
	{
		LPTSTR pszOutput = strValue.GetBuffer( nLength + 1 );
		
		for ( DWORD nChar = 0, nOut = 0 ; nChar < nLength ; nChar++, nOut++ )
		{
			pszOutput[ nOut ] = (TCHAR)pBuffer[ nChar ];
			if ( pszOutput[ nOut ] == 0 ) break;
		}
		
		strValue.ReleaseBuffer( nOut );
	}
	else if ( nEncoding == 1 && ( nLength & 1 ) == 0 && nLength >= 2 )
	{
		nLength = ( nLength - 2 ) / 2;
		LPTSTR pszOutput = strValue.GetBuffer( nLength + 1 );
		
		if ( pBuffer[0] == 0xFF && pBuffer[1] == 0xFE )
		{
			pBuffer += 2;
			for ( DWORD nChar = 0, nOut = 0 ; nChar < nLength ; nChar++, nOut++ )
			{
				pszOutput[ nOut ] = (TCHAR)pBuffer[ nChar*2+0 ] | ( (TCHAR)pBuffer[ nChar*2+1 ] << 8 );
				if ( pszOutput[ nOut ] == 0 ) break;
			}
			strValue.ReleaseBuffer( nOut );
		}
		else if ( pBuffer[0] == 0xFE && pBuffer[1] == 0xFF )
		{
			pBuffer += 2;
			for ( DWORD nChar = 0, nOut = 0 ; nChar < nLength ; nChar++, nOut++ )
			{
				pszOutput[ nOut ] = (TCHAR)pBuffer[ nChar*2+1 ] | ( (TCHAR)pBuffer[ nChar*2+0 ] << 8 );
				if ( pszOutput[ nOut ] == 0 ) break;
			}
			strValue.ReleaseBuffer( nOut );
		}
		else
		{
			strValue.ReleaseBuffer( 0 );
			return FALSE;
		}
	}
	else if ( nEncoding == 2 && ( nLength & 1 ) == 0 )
	{
		nLength = nLength / 2;
		LPTSTR pszOutput = strValue.GetBuffer( nLength + 1 );
		
		for ( DWORD nChar = 0, nOut = 0 ; nChar < nLength ; nChar++, nOut++ )
		{
			pszOutput[ nOut ] = (TCHAR)pBuffer[ nChar*2+1 ] | ( (TCHAR)pBuffer[ nChar*2+0 ] << 8 );
			if ( pszOutput[ nOut ] == 0 ) break;
		}
		
		strValue.ReleaseBuffer( nOut );
	}
	else if ( nEncoding == 3 )
	{
		int nWide = MultiByteToWideChar( CP_UTF8, 0, (LPCSTR)pBuffer, nLength, NULL, 0 );
		LPTSTR pszOutput = strValue.GetBuffer( nWide + 1 );
		MultiByteToWideChar( CP_UTF8, 0, (LPCSTR)pBuffer, nLength, pszOutput, nWide );
		pszOutput[ nWide ] = 0;
		strValue.ReleaseBuffer();
	}
	
	strValue.TrimLeft();
	strValue.TrimRight();
	
	if ( strValue.GetLength() == 0 || _tcslen( strValue ) == 0 ) return FALSE;
	
	pXML->AddAttribute( pszAttribute, strValue );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals MP3 scan (threaded)

BOOL CLibraryBuilderInternals::ReadMP3Frames( HANDLE hFile)
{
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	
	CXMLElement* pXML = new CXMLElement( NULL, _T("audio") );
	
	if ( ScanMP3Frame( pXML, hFile, 0 ) )
	{
		return SubmitMetadata( CSchema::uriAudio, pXML );
	}
	else
	{
		delete pXML;
		return FALSE;
	}
}

BOOL CLibraryBuilderInternals::ScanMP3Frame(CXMLElement* pXML, HANDLE hFile, DWORD nIgnore)
{
	static DWORD nBitrateTable[16][5] =
	{
		{ 0, 0, 0, 0 },					{ 32, 32, 32, 32, 8 },		{ 64, 48, 40, 48, 16 },
		{ 96, 56, 48, 56, 24 },			{ 128, 64, 56, 64, 32 },	{ 160, 80, 64, 80, 40 },
		{ 192, 96, 80, 96, 48 },		{ 224, 112, 96, 112, 56 },	{ 256, 128, 112, 128, 64 },
		{ 288, 160, 128, 144, 80 },		{ 320, 192, 160, 160, 96 },	{ 352, 224, 192, 176, 112 },
		{ 384, 256, 224, 192, 128 },	{ 416, 320, 256, 224, 144 },{ 448, 384, 320, 256, 160 },
		{ 0, 0, 0, 0 }
	};

	static DWORD nFrequencyTable[4][4] =
	{
		{ 11025, 0, 22050, 44100 },
		{ 12000, 0,  24000, 48000 },
		{ 8000, 0, 16000, 32000 },
		{ 0, 0, 0, 0 }
	};

	BYTE nLayer				= 0;
	BOOL bVariable			= FALSE;
	__int64 nTotalBitrate	= 0;
	DWORD nBaseBitrate		= 0;
	DWORD nBaseFrequency	= 0;
	DWORD nFrameCount		= 0;
	DWORD nFrameSize		= 0;
	DWORD nHeader			= 0;

	DWORD nRead;
	ReadFile( hFile, &nHeader, 4, &nRead, NULL );
	if ( nRead != 4 ) return FALSE;
	nHeader = SWAP_LONG( nHeader );

	for ( DWORD nSeek = 0 ; bVariable || ( nFrameCount < 16 && nSeek < 4096 ) ; nSeek++ )
	{
		DWORD nTime = GetTickCount();
		
		if ( ( nHeader & 0xFFE00000 ) == 0xFFE00000 )
		{
			BYTE nVersion	= (BYTE)( ( nHeader & 0x00180000 ) >> 19 );
			nLayer			= (BYTE)( ( nHeader & 0x00060000 ) >> 17 );
			BYTE nBitIndex	= (BYTE)( ( nHeader & 0x0000F000 ) >> 12 );
			BYTE nFreqIndex	= (BYTE)( ( nHeader & 0x00000C00 ) >> 10 );
			BYTE nChannels	= (BYTE)( ( nHeader & 0x000000C0 ) >> 6 );
			BOOL bPadding	= (BOOL)( nHeader & 0x0200 ) ? TRUE : FALSE;
			
			int nBitColumn = 0;
			
			if ( nVersion == 3 )
			{
				if ( nLayer == 3 ) nBitColumn = 0;
				else if ( nLayer == 2 ) nBitColumn = 1;
				else if ( nLayer == 1 ) nBitColumn = 2;
			}
			else
			{
				if ( nLayer == 3 ) nBitColumn = 3;
				else nBitColumn = 4;
			}
			
			DWORD nBitrate		= nBitrateTable[ nBitIndex ][ nBitColumn ] * 1000;
			DWORD nFrequency	= nFrequencyTable[ nFreqIndex ][ nVersion ];
			
			if ( ! nFrequency ) return FALSE;
			
			if ( nBaseBitrate )
			{
				if ( nBaseBitrate != nBitrate ) bVariable = TRUE;
			}
			else
			{
				nBaseBitrate	= nBitrate;
				nBaseFrequency	= nFrequency;
			}
			
			nFrameSize = ( nLayer == 3 ) ? ( 12 * nBitrate / nFrequency + bPadding ) * 4
				: ( 144 * nBitrate / nFrequency + bPadding );
			
			if ( ! nFrameSize ) return FALSE;
			
			nTotalBitrate += nBitrate / 1000;
			nFrameCount++;
			
			SetFilePointer( hFile, nFrameSize - 4, NULL, FILE_CURRENT );
			ReadFile( hFile, &nHeader, 4, &nRead, NULL );
			if ( nRead != 4 ) break;
			nHeader = SWAP_LONG( nHeader );
		}
		else
		{
			nHeader <<= 8;
			ReadFile( hFile, &nHeader, 1, &nRead, NULL );
			if ( nRead != 1 ) break;
		}
		
		if ( ! m_pBuilder->m_bPriority )
		{
			m_nSleep = ( GetTickCount() - nTime ) * 3;
			if ( m_nSleep > 0 ) Sleep( m_nSleep );
		}
		
		if ( ! m_pBuilder->m_bThread ) return FALSE;
	}
	
	if ( nFrameCount < 16 || ! nFrameSize ) return FALSE;
	
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
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals version information (threaded)

BOOL CLibraryBuilderInternals::ReadVersion( LPCTSTR pszPath)
{
	DWORD dwSize = GetFileVersionInfoSize( (LPTSTR)pszPath, &dwSize );
	if ( dwSize <= 152 ) return FALSE;
	
	BYTE* pBuffer = new BYTE[ dwSize ];
	
	if ( ! GetFileVersionInfo( (LPTSTR)pszPath, NULL, dwSize, pBuffer ) )
	{
		delete [] pBuffer;
		return FALSE;
	}
	
	WCHAR* pLanguage = (WCHAR*)pBuffer + 20 + 26 + 18 + 3;
	
	if ( wcslen( pLanguage ) != 8 )
	{
		delete [] pBuffer;
		return FALSE;
	}
	
	CXMLElement* pXML = new CXMLElement( NULL, _T("application") );
	
	pXML->AddAttribute( _T("os"), _T("Windows") );
	CopyVersionField( pXML, _T("title"), pBuffer, _T("ProductName") );
	CopyVersionField( pXML, _T("version"), pBuffer, _T("ProductVersion"), TRUE );
	CopyVersionField( pXML, _T("fileDescription"), pBuffer, _T("FileDescription") );
	CopyVersionField( pXML, _T("fileVersion"), pBuffer, _T("FileVersion"), TRUE );
	CopyVersionField( pXML, _T("originalFileName"), pBuffer, _T("OriginalFilename") );
	CopyVersionField( pXML, _T("company"), pBuffer, _T("CompanyName") );
	CopyVersionField( pXML, _T("copyright"), pBuffer, _T("LegalCopyright") );
	CopyVersionField( pXML, _T("comments"), pBuffer, _T("comments") );
	
	delete [] pBuffer;

	return SubmitMetadata( _T("http://www.shareaza.com/schemas/application.xsd"), pXML );
}

BOOL CLibraryBuilderInternals::CopyVersionField(CXMLElement* pXML, LPCTSTR pszAttribute, BYTE* pBuffer, LPCTSTR pszKey, BOOL bCommaToDot)
{
	CString strValue = GetVersionKey( pBuffer, pszKey );

	if ( strValue.IsEmpty() ) return FALSE;
	
	if ( bCommaToDot )
	{
		for ( int nPos = -1 ; ( nPos = strValue.Find( _T(", ") ) ) >= 0 ; )
		{
			strValue = strValue.Left( nPos ) + '.' + strValue.Mid( nPos + 2 );
		}
	}

	pXML->AddAttribute( pszAttribute, strValue );

	return TRUE;
}

CString CLibraryBuilderInternals::GetVersionKey(BYTE* pBuffer, LPCTSTR pszKey)
{
	CString strKey, strValue;

	WCHAR* pLanguage = (WCHAR*)pBuffer + 20 + 26 + 18 + 3;

	strKey = _T("\\StringFileInfo\\");
	strKey += pLanguage;
	strKey += _T("\\");
	strKey += pszKey;

	BYTE* pValue = NULL;
	DWORD dwSize = 0;

	if ( ! VerQueryValue( pBuffer, (LPTSTR)(LPCTSTR)strKey, (void**)&pValue, (UINT*)&dwSize ) )
		return strValue;
	
	if ( pValue[1] )
		strValue = (LPCSTR)pValue;
	else
		strValue = (LPCTSTR)pValue;

	return strValue;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals JPEG (threaded)

BOOL CLibraryBuilderInternals::ReadJPEG( HANDLE hFile)
{
	DWORD nRead	= 0;
	WORD wMagic	= 0;
	BYTE nByte	= 0;
	
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	ReadFile( hFile, &wMagic, 2, &nRead, NULL );
	if ( nRead != 2 || wMagic != 0xD8FF ) return SubmitCorrupted();
	
	BYTE nBits = 0, nComponents = 0;
	WORD nWidth = 0, nHeight = 0;
	CString strComment;
	
	for ( DWORD nSeek = 512 ; nSeek > 0 ; nSeek-- )
	{
		ReadFile( hFile, &nByte, 1, &nRead, NULL );
		if ( nRead != 1 ) return FALSE;
		if ( nByte != 0xFF ) continue;

		while ( nByte == 0xFF )
		{
			ReadFile( hFile, &nByte, 1, &nRead, NULL );
			if ( nRead != 1 ) return FALSE;
		}
		
		ReadFile( hFile, &wMagic, 2, &nRead, NULL );
		wMagic = ( wMagic >> 8 ) | ( wMagic << 8 );
		if ( nRead != 2 || wMagic < 2 ) return FALSE;

		switch ( nByte )
		{
		case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC5: case 0xC6: case 0xC7:
		case 0xC9: case 0xCA: case 0xCB: case 0xCD: case 0xCE: case 0xCF:
			ReadFile( hFile, &nBits, 1, &nRead, NULL );
			if ( nRead != 1 ) return FALSE;
			ReadFile( hFile, &nHeight, 2, &nRead, NULL );
			if ( nRead != 2 ) return FALSE;
			nHeight = ( nHeight >> 8 ) | ( nHeight << 8 );
			ReadFile( hFile, &nWidth, 2, &nRead, NULL );
			if ( nRead != 2 ) return FALSE;
			nWidth = ( nWidth >> 8 ) | ( nWidth << 8 );
			ReadFile( hFile, &nComponents, 1, &nRead, NULL );
			if ( nRead != 1 ) return FALSE;
			if ( wMagic < 8 ) return FALSE;
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

	if ( nWidth == 0 || nHeight == 0 ) return FALSE;

	strComment.TrimLeft();
	strComment.TrimRight();

	for ( int nChar = 0 ; nChar < strComment.GetLength() ; nChar++ )
	{
		if ( strComment[ nChar ] < 32 ) strComment.SetAt( nChar, '?' );
	}

	CXMLElement* pXML = new CXMLElement( NULL, _T("image") );
	CString strItem;
	
	strItem.Format( _T("%lu"), nWidth );
	pXML->AddAttribute( _T("width"), strItem );
	strItem.Format( _T("%lu"), nHeight );
	pXML->AddAttribute( _T("height"), strItem );
	
	if ( nComponents == 3 ) pXML->AddAttribute( _T("colors"), _T("16.7M") );
	else if ( nComponents == 1 ) pXML->AddAttribute( _T("colors"), _T("Greyscale") );
	
	if ( strComment.GetLength() ) pXML->AddAttribute( _T("description"), strComment );
	
	return SubmitMetadata( CSchema::uriImage, pXML );
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals GIF (threaded)

BOOL CLibraryBuilderInternals::ReadGIF( HANDLE hFile)
{
	CHAR szMagic[6];
	DWORD nRead;
	
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	ReadFile( hFile, szMagic, 6, &nRead, NULL );
	
	if ( nRead != 6 || ( strncmp( szMagic, "GIF87a", 6 ) && strncmp( szMagic, "GIF89a", 6 ) ) )
		return SubmitCorrupted();
	
	WORD nWidth, nHeight;
	
	ReadFile( hFile, &nWidth, 2, &nRead, NULL );
	if ( nRead != 2 || nWidth == 0 ) return FALSE;
	ReadFile( hFile, &nHeight, 2, &nRead, NULL );
	if ( nRead != 2 || nHeight == 0 ) return FALSE;
	
	CXMLElement* pXML = new CXMLElement( NULL, _T("image") );
	CString strItem;
	
	strItem.Format( _T("%lu"), nWidth );
	pXML->AddAttribute( _T("width"), strItem );
	strItem.Format( _T("%lu"), nHeight );
	pXML->AddAttribute( _T("height"), strItem );
	
	pXML->AddAttribute( _T("colors"), _T("256") );
	
	return SubmitMetadata( _T("http://www.shareaza.com/schemas/image.xsd"), pXML );
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals PNG (threaded)

BOOL CLibraryBuilderInternals::ReadPNG( HANDLE hFile)
{
	BYTE nMagic[8];
	DWORD nRead;
	
	if ( GetFileSize( hFile, NULL ) < 33 ) return SubmitCorrupted();
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	
	ReadFile( hFile, nMagic, 8, &nRead, NULL );
	if ( nRead != 8 ) return SubmitCorrupted();
	if ( nMagic[0] != 137 || nMagic[1] != 80 || nMagic[2] != 78 ) return SubmitCorrupted();
	if ( nMagic[3] != 71 || nMagic[4] != 13 || nMagic[5] != 10 ) return SubmitCorrupted();
	if ( nMagic[6] != 26 || nMagic[7] != 10 ) return SubmitCorrupted();
	
	DWORD nLength, nIHDR;
	
	ReadFile( hFile, &nLength, 4, &nRead, NULL ); nLength = SWAP_LONG( nLength );
	if ( nRead != 4 || nLength < 10 ) return FALSE;
	ReadFile( hFile, &nIHDR, 4, &nRead, NULL );
	if ( nRead != 4 || nIHDR != 'RDHI' ) return FALSE;

	DWORD nWidth, nHeight;
	BYTE nBits, nColors;

	ReadFile( hFile, &nWidth, 4, &nRead, NULL );  nWidth = SWAP_LONG( nWidth );
	if ( nRead != 4 || nWidth <= 0 || nWidth > 0xFFFF ) return FALSE;
	ReadFile( hFile, &nHeight, 4, &nRead, NULL ); nHeight = SWAP_LONG( nHeight );
	if ( nRead != 4 || nHeight <= 0 || nHeight > 0xFFFF ) return FALSE;

	ReadFile( hFile, &nBits, 1, &nRead, NULL );
	if ( nRead != 1 ) return FALSE;
	ReadFile( hFile, &nColors, 1, &nRead, NULL );
	if ( nRead != 1 ) return FALSE;

	CXMLElement* pXML = new CXMLElement( NULL, _T("image") );
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
	
	return SubmitMetadata( _T("http://www.shareaza.com/schemas/image.xsd"), pXML );
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals BMP (threaded)

BOOL CLibraryBuilderInternals::ReadBMP( HANDLE hFile)
{
	BITMAPFILEHEADER pBFH;
	BITMAPINFOHEADER pBIH;
	DWORD nRead;
	
	if ( GetFileSize( hFile, NULL ) < sizeof(pBFH) + sizeof(pBIH) ) return SubmitCorrupted();
	
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	ReadFile( hFile, &pBFH, sizeof(pBFH), &nRead, NULL );
	if ( nRead != sizeof(pBFH) || pBFH.bfType != 'MB' ) return SubmitCorrupted();
	
	ReadFile( hFile, &pBIH, sizeof(pBIH), &nRead, NULL );
	if ( nRead != sizeof(pBIH) || pBIH.biSize != sizeof(pBIH) ) return FALSE;
	
	CXMLElement* pXML = new CXMLElement( NULL, _T("image") );
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

	return SubmitMetadata( _T("http://www.shareaza.com/schemas/image.xsd"), pXML );
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

BOOL CLibraryBuilderInternals::ReadASF( HANDLE hFile)
{
	QWORD nSize;
	DWORD nRead;
	GUID pGUID;
	
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	ReadFile( hFile, &pGUID, sizeof(pGUID), &nRead, NULL );
	if ( nRead != sizeof(pGUID) || ( pGUID != asfHeader1 && pGUID != asfHeader2 ) )
		return SubmitCorrupted();
	ReadFile( hFile, &nSize, sizeof(nSize), &nRead, NULL );
	if ( nRead != sizeof(nSize) ) return SubmitCorrupted();
	
	if ( pGUID == asfHeader1 ) SetFilePointer( hFile, 6, NULL, FILE_CURRENT );
	
	CString strTitle, strAuthor, strCopyright, strDescription, strRating;
	DWORD nBitrate = 0, nVideoWidth = 0, nVideoHeight = 0;
	QWORD nContentLength = 0;
	BOOL bVideo = FALSE;
	BOOL bDRM = FALSE;
	
	while ( TRUE )
	{
		DWORD dwPosition = SetFilePointer( hFile, 0, NULL, FILE_CURRENT );
		
		ReadFile( hFile, &pGUID, sizeof(pGUID), &nRead, NULL );
		if ( nRead != sizeof(pGUID) ) break;
		ReadFile( hFile, &nSize, sizeof(nSize), &nRead, NULL );
		if ( nRead != sizeof(nSize) || nSize >= 0x80000000 ) break;
		
		if ( pGUID == asfProperties1 )
		{
			SetFilePointer( hFile, 48, NULL, FILE_CURRENT );
			ReadFile( hFile, &nContentLength, sizeof(nContentLength), &nRead, NULL );
			if ( nRead != sizeof(nContentLength) ) return FALSE;
		}
		else if ( pGUID == asfProperties2 )
		{
			SetFilePointer( hFile, 40, NULL, FILE_CURRENT );
			ReadFile( hFile, &nContentLength, sizeof(nContentLength), &nRead, NULL );
			if ( nRead != sizeof(nContentLength) ) return FALSE;
			SetFilePointer( hFile, 8, NULL, FILE_CURRENT );
			ReadFile( hFile, &nBitrate, sizeof(nBitrate), &nRead, NULL );
			if ( nRead != sizeof(nBitrate) ) return FALSE;
		}
		else if ( pGUID == asfStream1 )
		{
			ReadFile( hFile, &pGUID, sizeof(pGUID), &nRead, NULL );
			if ( nRead != sizeof(pGUID) ) return FALSE;
			
			if ( pGUID == asfVideo1 )
			{
				bVideo = TRUE;
				SetFilePointer( hFile, 38, NULL, FILE_CURRENT );
				ReadFile( hFile, &nVideoWidth, sizeof(nVideoWidth), &nRead, NULL );
				if ( nRead != sizeof(nVideoWidth) ) return FALSE;
				ReadFile( hFile, &nVideoHeight, sizeof(nVideoHeight), &nRead, NULL );
				if ( nRead != sizeof(nVideoHeight) ) return FALSE;
			}
		}
		else if ( pGUID == asfStream2 )
		{
			ReadFile( hFile, &pGUID, sizeof(pGUID), &nRead, NULL );
			if ( nRead != sizeof(pGUID) ) return FALSE;

			if ( pGUID == asfVideo2 )
			{
				bVideo = TRUE;
				/*
				SetFilePointer( hFile, 68, NULL, FILE_CURRENT );
				ReadFile( hFile, &nVideoWidth, sizeof(nVideoWidth), &nRead, NULL );
				if ( nRead != sizeof(nVideoWidth) ) return FALSE;
				nVideoHeight = nVideoWidth >> 16;
				nVideoWidth &= 0xFFFF;
				*/
			}
		}
		else if ( pGUID == asfContent1 )
		{
			WORD nStrLen[5];
			ReadFile( hFile, nStrLen, sizeof(nStrLen), &nRead, NULL );
			if ( nRead != sizeof(nStrLen) ) break;
			
			for ( int nStr = 0 ; nStr < 5 ; nStr++ )
			{
				if ( ! nStrLen[ nStr ] || nStrLen[ nStr ] & 1 ) continue;
				WCHAR* pStr = new WCHAR[ nStrLen[ nStr ] / 2 ];
				ReadFile( hFile, pStr, nStrLen[ nStr ], &nRead, NULL );
				if ( nRead != nStrLen[ nStr ] ) return FALSE;
				pStr[ nStrLen[ nStr ] / 2 - 1 ] = 0;
				
				switch ( nStr )
				{
				case 0:
					strTitle = pStr;
					break;
				case 1:
					strAuthor = pStr;
					break;
				case 2:
					strCopyright = pStr;
					break;
				case 3:
					strDescription = pStr;
					break;
				case 4:
					strRating = pStr;
					break;
				}
				
				delete [] pStr;
			}
		}
		else if ( pGUID == asfContent2 )
		{
			WORD nCount;
			ReadFile( hFile, &nCount, sizeof(nCount), &nRead, NULL );
			if ( nRead != sizeof(nCount) ) break;
			
			while ( nCount-- )
			{
				WORD nLanguageID, nStreamID, nNameLen, nValueLen;
				BYTE nFieldType;
				WCHAR* pStr;

				ReadFile( hFile, &nFieldType, sizeof(nFieldType), &nRead, NULL );
				if ( nRead != sizeof(nFieldType) ) return FALSE;
				ReadFile( hFile, &nLanguageID, sizeof(nLanguageID), &nRead, NULL );
				if ( nRead != sizeof(nLanguageID) ) return FALSE;
				ReadFile( hFile, &nStreamID, sizeof(nStreamID), &nRead, NULL );
				if ( nRead != sizeof(nStreamID) ) return FALSE;
				ReadFile( hFile, &nNameLen, sizeof(nNameLen), &nRead, NULL );
				if ( nRead != sizeof(nNameLen) ) return FALSE;
				ReadFile( hFile, &nValueLen, sizeof(nValueLen), &nRead, NULL );
				if ( nRead != sizeof(nValueLen) ) return FALSE;
				
				pStr = new WCHAR[ nNameLen + 1 ];
				ReadFile( hFile, pStr, nNameLen * 2, &nRead, NULL );
				if ( nRead != (DWORD)nNameLen * 2 ) return FALSE;
				pStr[ nNameLen ] = 0;
				delete [] pStr;

				pStr = new WCHAR[ nValueLen + 1 ];
				ReadFile( hFile, pStr, nValueLen * 2, &nRead, NULL );
				if ( nRead != (DWORD)nValueLen * 2 ) return FALSE;
				pStr[ nValueLen ] = 0;

				switch ( nFieldType )
				{
				case 1:
					strAuthor = pStr;
					break;
				case 2: case 20:
					strTitle = pStr;
					break;
				case 3:
					strCopyright = pStr;
					break;
				case 4:
					strDescription = pStr;
					break;
				}

				delete [] pStr;
			}
		}
		else if ( pGUID == asfDRM1 || pGUID == asfDRM2 )
		{
			bDRM = TRUE;
		}
		else if ( pGUID == asfData1 || pGUID == asfData2 )
		{
			break;
		}
		
		SetFilePointer( hFile, dwPosition + (DWORD)nSize, NULL, FILE_BEGIN );
	}
	
	CXMLElement* pXML = new CXMLElement( NULL, bVideo ? _T("video") : _T("audio") );
	CString strItem;
	
	if ( strTitle.GetLength() ) pXML->AddAttribute( _T("title"), strTitle );
	
	if ( strDescription.GetLength() ) pXML->AddAttribute( _T("description"), strDescription );
	
	if ( bDRM )
	{
		pXML->AddAttribute( _T("drm"), _T("true") );
	}
	
	if ( bVideo )
	{
		if ( strAuthor.GetLength() ) pXML->AddAttribute( _T("producer"), strAuthor );

		if ( strRating.GetLength() ) pXML->AddAttribute( _T("rating"), strRating );

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
		if ( strAuthor.GetLength() ) pXML->AddAttribute( _T("artist"), strAuthor );

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
	
	return SubmitMetadata( bVideo ? CSchema::uriVideo : CSchema::uriAudio, pXML );
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals MPEG (threaded)

BOOL CLibraryBuilderInternals::ReadMPEG( HANDLE hFile)
{
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	
	DWORD nHeader = 0;
	
	for ( DWORD nSeek = 8192 ; nSeek > 0 ; nSeek--, nHeader <<= 8 )
	{
		DWORD nRead = 0;
		ReadFile( hFile, &nHeader, 1, &nRead, NULL );
		if ( nRead != 1 ) break;
		
		if ( nHeader == 0x000001B3 ) break;
	}
	
	if ( ! nSeek ) return FALSE;
	
	BYTE nBuffer[7];

	ReadFile( hFile, nBuffer, 7, &nHeader, NULL );
	if ( nHeader != 7 ) return FALSE;
	
	CXMLElement* pXML = new CXMLElement( NULL, _T("video") );
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
	
	if ( nFrameIndex >= 1 && nFrameIndex <= 9 )
	{
		pXML->AddAttribute( _T("frameRate"), pszFPS[ nFrameIndex - 1 ] );
	}
	
	return SubmitMetadata( CSchema::uriVideo, pXML );
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals OGG VORBIS (threaded)

BOOL CLibraryBuilderInternals::ReadOGG( HANDLE hFile)
{
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	
	DWORD nDummy, nHeader = 0;
	ReadFile( hFile, &nHeader, 4, &nDummy, NULL );
	
	for ( DWORD nSeek = 0 ; nSeek < 16384 ; nSeek++ )
	{
		if ( nHeader == 'SggO' ) break;
		nHeader >>= 8;
		ReadFile( hFile, (BYTE*)&nHeader + 3, 1, &nDummy, NULL );
	}
	
	if ( nHeader != 'SggO' ) return SubmitCorrupted();
	SetFilePointer( hFile, -4, NULL, FILE_CURRENT );
	
	DWORD nOGG = 0;
	BYTE* pOGG = ReadOGGPage( hFile, nOGG, 0x02, 0, 0x1E );
	
	if ( ! pOGG ) return FALSE;
	
	BYTE nChannels		= pOGG[ 11 ];
	DWORD nFrequency	= *(DWORD*)&pOGG[12];
	DWORD nBitrate		= *(DWORD*)&pOGG[20];
	
	delete [] pOGG;
	BYTE* prOGG = pOGG = ReadOGGPage( hFile, nOGG, 0x00, 1, 1+6+4+4 );
	
	if ( ! pOGG ) return FALSE;
	pOGG += 1 + 6;
	nOGG -= 1 + 6;
	
	CString strComment;
	
	if ( ! ReadOGGString( pOGG, nOGG, strComment ) || nOGG < 4 )
	{
		free( pOGG );
		return FALSE;
	}
	
	DWORD nComments = *(DWORD*)pOGG;
	pOGG += 4; nOGG -= 4;
	
	CXMLElement* pXML = new CXMLElement( NULL, _T("audio") );
	
	for ( ; nComments && nOGG > 4 ; nComments-- )
	{
		if ( ! ReadOGGString( pOGG, nOGG, strComment ) ) break;
		
		int nEquals = strComment.Find( '=' );
		if ( nEquals <= 0 ) continue;
		
		CString strKey		= strComment.Left( nEquals );
		CString strValue	= strComment.Mid( nEquals + 1 );
		
		strKey.TrimLeft(); strKey.TrimRight(); strKey.MakeUpper();
		strValue.TrimLeft(); strValue.TrimRight();
		if ( strValue.IsEmpty() ) continue;
		
		if ( strKey == _T("TITLE") )
		{
			pXML->AddAttribute( _T("title"), strValue );
		}
		else if ( strKey == _T("ALBUM") )
		{
			pXML->AddAttribute( _T("album"), strValue );
		}
		else if ( strKey == _T("TRACKNUMBER") )
		{
			pXML->AddAttribute( _T("track"), strValue );
		}
		else if ( strKey == _T("ARTIST") )
		{
			pXML->AddAttribute( _T("artist"), strValue );
		}
		else if ( strKey == _T("DESCRIPTION") )
		{
			pXML->AddAttribute( _T("description"), strValue );
		}
		else if ( strKey == _T("GENRE") )
		{
			pXML->AddAttribute( _T("genre"), strValue );
		}
		else if ( strKey == _T("DATE") )
		{
			pXML->AddAttribute( _T("year"), strValue );
		}
	}
	
	delete [] prOGG;
	
	if ( nComments )
	{
		if ( pXML ) delete pXML;
		return FALSE;
	}
	
	DWORD nLength = 0;
	
	for ( nComments = 2 ; ; nComments++ )
	{
		DWORD nTime = GetTickCount();
		if ( ! ReadOGGPage( hFile, nOGG, 0xFF, nComments, 0xFFFFFFFF ) ) break;
		nLength = max( nLength, nOGG );
		m_nSleep = ( GetTickCount() - nTime ) * 3;
		if ( m_nSleep > 0 ) Sleep( m_nSleep );
		if ( ! m_pBuilder->m_bThread ) break;
	}
	
	if ( ! m_pBuilder->m_bThread )
	{
		delete pXML;
		return FALSE;
	}
	
	if ( nFrequency > 0 && nLength > 0 && ( nLength / nFrequency ) > 0 )
	{
		strComment.Format( _T("%lu"), nLength / nFrequency );
		pXML->AddAttribute( _T("seconds"), strComment );

		nBitrate = GetFileSize( hFile, NULL ) / ( nLength / nFrequency ) * 8;
	}
	
	strComment.Format( _T("%lu"), nBitrate / 1000 );
	pXML->AddAttribute( _T("bitrate"), strComment );
	
	strComment.Format( _T("%lu"), nFrequency );
	pXML->AddAttribute( _T("sampleRate"), strComment );
	
	return SubmitMetadata( CSchema::uriAudio, pXML );
}

BYTE* CLibraryBuilderInternals::ReadOGGPage(HANDLE hFile, DWORD& nBuffer, BYTE nFlags, DWORD nSequence, DWORD nMinSize)
{
	DWORD nMagic, nRead, nSample;
	BYTE nByte, nChunk;
	
	nBuffer = 0;
	
	ReadFile( hFile, &nMagic, 4, &nRead, NULL );
	if ( nRead != 4 || nMagic != 'SggO' ) return NULL;
	
	ReadFile( hFile, &nByte, 1, &nRead, NULL );
	if ( nRead != 1 || nByte != 0 ) return NULL;
	
	ReadFile( hFile, &nByte, 1, &nRead, NULL );
	if ( nRead != 1 ) return NULL;
	if ( nFlags < 0xFF && nByte != nFlags ) return NULL;
	
	ReadFile( hFile, &nSample, 4, &nRead, NULL );
	if ( nRead != 4 ) return NULL;
	
	SetFilePointer( hFile, 4 + 4, NULL, FILE_CURRENT );
	
	ReadFile( hFile, &nMagic, 4, &nRead, NULL );
	if ( nRead != 4 || nMagic != nSequence ) return NULL;
	
	ReadFile( hFile, &nMagic, 4, &nRead, NULL );
	if ( nRead != 4 ) return NULL;
	
	ReadFile( hFile, &nByte, 1, &nRead, NULL );
	if ( nRead != 1 ) return NULL;
	
	for ( ; nByte ; nByte-- )
	{
		ReadFile( hFile, &nChunk, 1, &nRead, NULL );
		if ( nRead != 1 ) break;
		nBuffer += nChunk;
	}
	
	if ( nByte ) return NULL;
	
	if ( nMinSize < 0xFFFFFFFF )
	{
		if ( nBuffer < nMinSize ) return NULL;
		
		BYTE* pBuffer = new BYTE[ nBuffer ];
		
		ReadFile( hFile, pBuffer, nBuffer, &nRead, NULL );
		
		if ( nRead == nBuffer ) return pBuffer;
		
		delete [] pBuffer;
	}
	else
	{
		SetFilePointer( hFile, nBuffer, NULL, FILE_CURRENT );
		nBuffer = nSample;
		return (BYTE*)TRUE;
	}
	
	return NULL;
}

BOOL CLibraryBuilderInternals::ReadOGGString(BYTE*& pOGG, DWORD& nOGG, CString& str)
{
	if ( nOGG < 4 ) return FALSE;
	
	DWORD nLen = *(DWORD*)pOGG;
	pOGG += 4; nOGG -= 4;
	
	if ( nOGG < nLen ) return FALSE;
	
	LPTSTR pszOut = str.GetBuffer( nLen + 1 );
	for ( ; nLen ; nLen--, nOGG-- ) *pszOut++ = (TCHAR)*pOGG++;
	*pszOut++ = 0;
	str.ReleaseBuffer();
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals APE Monkey's Audio (threaded)

BOOL CLibraryBuilderInternals::ReadAPE( HANDLE hFile)
{
	if ( GetFileSize( hFile, NULL ) < sizeof(APE_HEADER) ) return SubmitCorrupted();
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	
	APE_HEADER pAPE;
	DWORD nRead;
	
	ReadFile( hFile, &pAPE, sizeof(pAPE), &nRead, NULL );
	if ( nRead != sizeof(pAPE) ) return SubmitCorrupted();
	if ( pAPE.cID[0] != 'M' || pAPE.cID[1] != 'A' || pAPE.cID[2] != 'C' ) return SubmitCorrupted();
	if ( pAPE.nSampleRate == 0 ) return SubmitCorrupted();
	
	DWORD nBlocksPerFrame = ( pAPE.nVersion >= 3900 || ( pAPE.nVersion >= 3800 &&
		pAPE.nCompressionLevel == 4000 ) ) ? 73728 : 9216;
	
	DWORD nBlocks	= ( pAPE.nTotalFrames - 1 ) * nBlocksPerFrame + pAPE.nFinalFrameBlocks;
	DWORD nSamples	= nBlocks * pAPE.nChannels;
	
	if ( pAPE.nFormatFlags & 8 )
		nSamples *= 3;
	else if ( ( pAPE.nFormatFlags & 1 ) == 0 )
		nSamples *= 2;
	
	DWORD nDuration	= nSamples / pAPE.nSampleRate;
	
	CXMLElement* pXML = new CXMLElement( NULL, _T("audio") );
	CString strItem;
	
	strItem.Format( _T("%lu"), nDuration );
	pXML->AddAttribute( _T("seconds"), strItem );
	
	strItem.Format( _T("%lu"), pAPE.nSampleRate );
	pXML->AddAttribute( _T("sampleRate"), strItem );
	
	if ( ReadID3v1( hFile, pXML ) )
	{
		return SubmitMetadata( CSchema::uriAudio, pXML );
	}
	
	if ( GetFileSize( hFile, NULL ) < sizeof(APE_HEADER) + sizeof(APE_TAG_FOOTER) )
	{
		return SubmitMetadata( CSchema::uriAudio, pXML );
	}
	
	APE_TAG_FOOTER pFooter;
	
	SetFilePointer( hFile, -(LONG)sizeof(pFooter), NULL, FILE_END );
	ReadFile( hFile, &pFooter, sizeof(pFooter), &nRead, NULL );
	
	if ( nRead != sizeof(pFooter) || strncmp( pFooter.cID, "APETAGEX", 8 ) ||
		 (DWORD)pFooter.nFields > 16 ||
		 ( pFooter.nVersion != 1000 && pFooter.nVersion != 2000 ) )
	{
		return SubmitMetadata( CSchema::uriAudio, pXML );
	}
	
	SetFilePointer( hFile, -(LONG)pFooter.nSize, NULL, FILE_END );
	
	for ( int nTag = 0 ; nTag < pFooter.nFields ; nTag++ )
	{
		DWORD nLength, nFlags;
		
		ReadFile( hFile, &nLength, 4, &nRead, NULL );
		if ( nRead != 4 || nLength > 1024 ) break;
		ReadFile( hFile, &nFlags, 4, &nRead, NULL );
		if ( nRead != 4 ) break;
		
		CString strKey, strValue;
		
		while ( strKey.GetLength() < 64 )
		{
			BYTE nChar;
			ReadFile( hFile, &nChar, 1, &nRead, NULL );
			if ( nRead != 1 || nChar == 0 ) break;
			strKey += (TCHAR)nChar;
		}
		
		if ( nRead != 1 || strKey.GetLength() >= 64 ) break;
		
		LPSTR pszInput = new CHAR[ nLength ];
		ReadFile( hFile, pszInput, nLength, &nRead, NULL );
		if ( nLength != nRead ) break;
		
		int nWide = MultiByteToWideChar( CP_UTF8, 0, pszInput, nLength, NULL, 0 );
		LPWSTR pszWide = new WCHAR[ nWide + 1 ];
		MultiByteToWideChar( CP_UTF8, 0, pszInput, nLength, pszWide, nWide );
		pszWide[ nWide ] = 0;
		strValue = pszWide;
		
		delete [] pszWide;
		delete [] pszInput;
		
		strKey.TrimLeft(); strKey.TrimRight();
		strValue.TrimLeft(); strValue.TrimRight();
		
		if ( strKey.GetLength() && strValue.GetLength() )
		{
			strKey.MakeLower();
			
			if ( strKey == _T("title") )
			{
				pXML->AddAttribute( _T("title"), strValue );
			}
			else if ( strKey == _T("artist") )
			{
				pXML->AddAttribute( _T("artist"), strValue );
			}
			else if ( strKey == _T("album") )
			{
				pXML->AddAttribute( _T("album"), strValue );
			}
			else if ( strKey == _T("comment") )
			{
				pXML->AddAttribute( _T("description"), strValue );
			}
			else if ( strKey == _T("year") )
			{
				pXML->AddAttribute( _T("year"), strValue );
			}
			else if ( strKey == _T("track") )
			{
				pXML->AddAttribute( _T("track"), strValue );
			}
			else if ( strKey == _T("genre") )
			{
				pXML->AddAttribute( _T("genre"), strValue );
			}
			else if ( strKey.Find( _T(" url") ) > 0 )
			{
				pXML->AddAttribute( _T("link"), strValue );
			}
		}
	}
	
	return SubmitMetadata( CSchema::uriAudio, pXML );
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals AVI (threaded)

BOOL CLibraryBuilderInternals::ReadAVI( HANDLE hFile)
{
	if ( GetFileSize( hFile, NULL ) < sizeof(AVI_HEADER) + 16 ) return SubmitCorrupted();
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	
	CHAR szID[5] = { 0, 0, 0, 0, 0 };
	DWORD nRead;
	
	ReadFile( hFile, szID, 4, &nRead, NULL );
	if ( nRead != 4 || strncmp( szID, "RIFF", 4 ) ) return SubmitCorrupted();
	ReadFile( hFile, szID, 4, &nRead, NULL );
	if ( nRead != 4 ) return FALSE;
	ReadFile( hFile, szID, 4, &nRead, NULL );
	if ( nRead != 4 || strncmp( szID, "AVI ", 4 ) ) return SubmitCorrupted();
	ReadFile( hFile, szID, 4, &nRead, NULL );
	if ( nRead != 4 || strncmp( szID, "LIST", 4 ) ) return FALSE;
	ReadFile( hFile, szID, 4, &nRead, NULL );
	if ( nRead != 4 ) return FALSE;
	ReadFile( hFile, szID, 4, &nRead, NULL );
	if ( nRead != 4 || strncmp( szID, "hdrl", 4 ) ) return FALSE;
	ReadFile( hFile, szID, 4, &nRead, NULL );
	if ( nRead != 4 || strncmp( szID, "avih", 4 ) ) return FALSE;
	ReadFile( hFile, szID, 4, &nRead, NULL );
	if ( nRead != 4 ) return FALSE;
	
	AVI_HEADER pHeader;
	ReadFile( hFile, &pHeader, sizeof(pHeader), &nRead, NULL );
	if ( nRead != sizeof(pHeader) ) return FALSE;
	
	ReadFile( hFile, szID, 4, &nRead, NULL );
	if ( nRead != 4 || strncmp( szID, "LIST", 4 ) ) return FALSE;
	ReadFile( hFile, szID, 4, &nRead, NULL );
	if ( nRead != 4 ) return FALSE;
	ReadFile( hFile, szID, 4, &nRead, NULL );
	if ( nRead != 4 || strncmp( szID, "strl", 4 ) ) return FALSE;
	ReadFile( hFile, szID, 4, &nRead, NULL );
	if ( nRead != 4 || strncmp( szID, "strh", 4 ) ) return FALSE;
	ReadFile( hFile, szID, 4, &nRead, NULL );
	if ( nRead != 4 ) return FALSE;
	ReadFile( hFile, szID, 4, &nRead, NULL );
	if ( nRead != 4 || strncmp( szID, "vids", 4 ) ) return FALSE;
	ReadFile( hFile, szID, 4, &nRead, NULL );
	if ( nRead != 4 ) return FALSE;
	
	CXMLElement* pXML = new CXMLElement( NULL, _T("video") );
	CString strItem;
	
	double nTime = (double)pHeader.dwMicroSecPerFrame / 1000000.0f;
	nTime *= (double)pHeader.dwTotalFrames;
	nTime /= 60.0f;
	
	double nRate = 1000000.0f / (double)pHeader.dwMicroSecPerFrame;
	
	strItem.Format( _T("%lu"), pHeader.dwWidth );
	pXML->AddAttribute( _T("width"), strItem );
	strItem.Format( _T("%lu"), pHeader.dwHeight );
	pXML->AddAttribute( _T("height"), strItem );
	strItem.Format( _T("%.3f"), nTime );
	pXML->AddAttribute( _T("minutes"), strItem );
	strItem.Format( _T("%.2f"), nRate );
	pXML->AddAttribute( _T("frameRate"), strItem );
	pXML->AddAttribute( _T("codec"), CString( szID ) );
	
	return SubmitMetadata( CSchema::uriVideo, pXML );
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals PDF (threaded)

BOOL CLibraryBuilderInternals::ReadPDF( HANDLE hFile, LPCTSTR pszPath)
{
	DWORD nOffset, nCount, nPages, nInfo;
	CString strLine, strSeek;
	
	SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
	if ( ReadLine( hFile ).Find( _T("%PDF") ) != 0 ) return FALSE;
	
	SetFilePointer( hFile, -1, NULL, FILE_END );
	strLine = ReadLineReverse( hFile );
	if ( strLine.IsEmpty() ) strLine = ReadLineReverse( hFile );
	if ( strLine != _T("%%EOF") ) return FALSE;

	strLine = ReadLineReverse( hFile );
	if ( ReadLineReverse( hFile ) != _T("startxref") ) return FALSE;
	
	if ( _stscanf( strLine, _T("%lu"), &nOffset ) != 1 ) return FALSE;
	if ( SetFilePointer( hFile, nOffset, NULL, FILE_BEGIN ) != nOffset ) return FALSE;
	
	if ( ReadLine( hFile ) != _T("xref") ) return FALSE;
	strLine = ReadLine( hFile );
	if ( _stscanf( strLine, _T("%lu %lu"), &nOffset, &nCount ) != 2 ) return FALSE;
	if ( nCount > 4096 ) return FALSE;
	
	DWORD* pOffset = new DWORD[ nCount ];
	ZeroMemory( pOffset, sizeof(DWORD) * nCount );
	
	for ( nOffset = 0 ; nOffset < nCount ; nOffset++ )
	{
		strLine = ReadLine( hFile );
		strLine.TrimLeft();
		strLine.TrimRight();
		
		if ( strLine.GetLength() != 18 || strLine.GetAt( 10 ) != ' ' )
		{
			delete [] pOffset;
			return FALSE;
		}
		
		if ( strLine.GetAt( 17 ) == 'n' )
		{
			for ( LPCTSTR pszInt = strLine ; *pszInt == '0' ; pszInt++ );
			
			if ( *pszInt != 0 )
			{
				_stscanf( pszInt, _T("%lu"), &pOffset[ nOffset ] );
			}
		}
	}
	
	if ( ReadLine( hFile ) != _T("trailer") ) return FALSE;
	if ( ReadLine( hFile ) != _T("<<") ) return FALSE;
	
	for ( nOffset = 0 ; ; )
	{
		strLine = ReadLine( hFile );
		if ( strLine.IsEmpty() || strLine == _T(">>") ) break;
		
		if ( _tcsnicmp( strLine, _T("/Info "), 6 ) == 0 )
		{
			_stscanf( strLine.Mid( 6 ), _T("%lu"), &nOffset );
			break;
		}
	}
	
	if ( nOffset == 0 )
	{
		delete [] pOffset;
		return FALSE;
	}
	
	strSeek.Format( _T("%lu 0 obj"), nOffset );
	nPages = nInfo = 0;
	
	for ( nOffset = 0 ; nOffset < nCount ; nOffset++ )
	{
		if ( pOffset[ nOffset ] == 0 ) continue;
		SetFilePointer( hFile, pOffset[ nOffset ], NULL, FILE_BEGIN );
		
		strLine = ReadLine( hFile );
		if ( strLine.Find( _T("obj") ) < 0 ) break;
		
		if ( strLine == strSeek )
		{
			nInfo = SetFilePointer( hFile, 0, NULL, FILE_CURRENT );
		}
		else if ( ReadLine( hFile ) == _T("<<") )
		{
			if ( ReadLine( hFile ) == _T("/Type /Page") ) nPages++;
		}
	}
	
	delete [] pOffset;
	
	if ( nInfo == 0 ) return FALSE;
	SetFilePointer( hFile, nInfo, NULL, FILE_BEGIN );
	
	if ( ReadLine( hFile ) != _T("<<") ) return FALSE;
	
	BOOL bBook = ( _tcsistr( pszPath, _T("book") ) != NULL );
	if ( ! bBook ) return FALSE;
	
	CXMLElement* pXML = new CXMLElement( NULL, bBook ? _T("book") : _T("document") );
	
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
			if ( pszName = _tcschr( pszName, ')' ) )
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
	
	if ( nPages > 0 )
	{
		strLine.Format( _T("%lu"), nPages );
		pXML->AddAttribute( _T("pages"), strLine );
	}
	
	while ( TRUE )
	{
		strLine = ReadLine( hFile );
		if ( strLine.IsEmpty() || strLine.GetAt( 0 ) != '/' ) break;
		
		CString strKey = strLine.SpanExcluding( _T(" \t") );
		strLine = strLine.Mid( strKey.GetLength() );
		strLine.TrimLeft();
		strKey.MakeLower();
		
		if ( strLine.GetLength() >= 2 &&
			 strLine.GetAt( 0 ) == '(' &&
			 strLine.GetAt( strLine.GetLength() - 1 ) == ')' )
		{
			strLine = strLine.Mid( 1, strLine.GetLength() - 2 );
		}
		
		if ( strLine.IsEmpty() ) continue;
		
		if ( ( strLine.GetLength() & 1 ) == 0 && strLine.GetAt( 0 ) == '<' )
		{
			CString strTemp;
			
			for ( int nHex = 0 ; nHex < strLine.GetLength() / 2 - 1 ; nHex++ )
			{
				int nChar;
				if ( _stscanf( strLine.Mid( nHex + 1, 2 ), _T("%x"), &nChar ) == 1 )
				{
					strTemp += (TCHAR)nChar;
				}
			}
			
			strLine = strTemp;
			if ( strLine.IsEmpty() ) continue;
		}
		
		if ( strKey == _T("/title") )
		{
			pXML->AddAttribute( _T("title"), strLine );
		}
		else if ( strKey == _T("/author") )
		{
			pXML->AddAttribute( _T("author"), strLine );
		}
		else if ( strKey == _T("/subject") )
		{
			pXML->AddAttribute( _T("subject"), strLine );
		}
		else if ( strKey == _T("/keywords") )
		{
			pXML->AddAttribute( _T("keywords"), strLine );
		}
	}
	
	if ( bBook )
	{
		pXML->AddAttribute( _T("format"), _T("PDF") );
		pXML->AddAttribute( _T("back"), _T("Digital") );
	}
	
	// TODO: Check bBook, and do uriDocument
	return SubmitMetadata( CSchema::uriBook, pXML );
}

CString CLibraryBuilderInternals::ReadLine(HANDLE hFile)
{
	DWORD nRead, nLength;
	TCHAR cChar;
	CString str;
	
	for ( nLength = 0 ; ReadFile( hFile, &cChar, 1, &nRead, NULL ) && nRead == 1 && nLength++ < 4096 ; )
	{
		if ( cChar == '\r' ) break;
		if ( cChar != '\n' ) str += cChar;
	}
	
	str.TrimLeft();
	str.TrimRight();
	
	return str;
}

CString CLibraryBuilderInternals::ReadLineReverse(HANDLE hFile)
{
	DWORD nRead, nLength;
	TCHAR cChar;
	CString str;
	
	for ( nLength = 0 ; ReadFile( hFile, &cChar, 1, &nRead, NULL ) && nRead == 1 && nLength++ < 4096 ; )
	{
		if ( SetFilePointer( hFile, -2, NULL, FILE_CURRENT ) == 0 ) break;
		if ( cChar == '\r' ) break;
		if ( cChar != '\n' ) str = cChar + str;
	}
	
	str.TrimLeft();
	str.TrimRight();
	
	return str;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderInternals Collection (threaded)

BOOL CLibraryBuilderInternals::ReadCollection( HANDLE hFile, SHA1* pSHA1)
{
	CCollectionFile pCollection;
	if ( ! pCollection.Attach( hFile ) ) return FALSE;
	
	LibraryFolders.MountCollection( pSHA1, &pCollection );
	
	if ( CXMLElement* pMetadata = pCollection.GetMetadata() )
	{
		pMetadata = pMetadata->GetFirstElement()->Clone();
		return SubmitMetadata( pCollection.GetThisURI(), pMetadata );
	}
	
	return TRUE;
}
