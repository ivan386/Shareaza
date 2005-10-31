//
// AVIThumb.cpp
//
// Copyright (c) Michael Stokes, 2002-2004.
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
#include "ImageServices.h"
#include "AVIThumb.h"
#include <vfw.h>

#define BUFFER_SIZE	102400
#define FAIL_KEY	_T("Software\\Shareaza\\Shareaza\\Plugins\\ImageService\\Failed")


/////////////////////////////////////////////////////////////////////////////
// CAVIThumb construction

LONG				CAVIThumb::m_nInstances = 0;
CRITICAL_SECTION	CAVIThumb::m_pSection;

CAVIThumb::CAVIThumb()
{
	// Setup the critical section
	
	if ( InterlockedIncrement( &m_nInstances ) == 1 )
		InitializeCriticalSection( &m_pSection );
	
	// Create a temp filename
	
	GetTempPath( 100, m_szTemp );
	lstrcat( m_szTemp, _T("AVIThumb0000.avi") );
	
	LPTSTR pszNum = m_szTemp + lstrlen( m_szTemp ) - 8;
	pszNum[0] += (BYTE)( GetTickCount() % 10 );
	pszNum[1] += (BYTE)( ( GetTickCount() / 10 ) % 10 );
	pszNum[2] += (BYTE)( ( GetTickCount() / 100 ) % 10 );
	pszNum[3] += (BYTE)( ( GetTickCount() / 1000 ) % 10 );
	
	// Open temp file and create file mapping
	
	m_pTempMap	= NULL;
	m_hTempMap	= INVALID_HANDLE_VALUE;
	
	m_hTempFile	= CreateFile( m_szTemp, GENERIC_READ|GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	
	if ( m_hTempFile != INVALID_HANDLE_VALUE )
	{
		m_hTempMap = CreateFileMapping( m_hTempFile, NULL, PAGE_READWRITE, 0, BUFFER_SIZE, NULL );
		
		if ( m_hTempMap != INVALID_HANDLE_VALUE )
		{
			m_pTempMap = MapViewOfFile( m_hTempMap, FILE_MAP_WRITE, 0, 0, BUFFER_SIZE );
			if ( m_pTempMap ) return;
			CloseHandle( m_hTempMap );
		}
		
		CloseHandle( m_hTempFile );
		
		m_hTempFile = INVALID_HANDLE_VALUE;
		m_hTempMap	= INVALID_HANDLE_VALUE;
	}
}

CAVIThumb::~CAVIThumb()
{
	if ( m_pTempMap ) UnmapViewOfFile( m_pTempMap );
	if ( m_hTempMap != INVALID_HANDLE_VALUE ) CloseHandle( m_hTempMap );
	if ( m_hTempFile != INVALID_HANDLE_VALUE ) CloseHandle( m_hTempFile );
	
	DeleteFile( m_szTemp );
	
	if ( InterlockedDecrement( &m_nInstances ) == 0 )
		DeleteCriticalSection( &m_pSection );
}

/////////////////////////////////////////////////////////////////////////////
// CAVIThumb operations

HRESULT STDMETHODCALLTYPE CAVIThumb::LoadFromFile(HANDLE hFile, DWORD nLength, IMAGESERVICEDATA __RPC_FAR *pParams, SAFEARRAY __RPC_FAR *__RPC_FAR *ppImage)
{
	HRESULT hResult = E_FAIL;
	
	if ( m_pTempMap == NULL ) return E_FAIL;
	
	EnterCriticalSection( &m_pSection );
	
	// This is an unfortunate hack in the otherwise very elegant ImageServices
	// architecture: for this plugin only (AVI thumbnailer), Shareaza will sneak
	// in the actual file path instead of a file handle or memory block, so that
	// the legacy AVIFile APIs can be used.  This sucks, improve it so that no
	// filename is ever necessary.
	
	if ( nLength == 0xFEFEFEFE )
	{
		AVIFileInit();
		hResult = LoadImpl( (LPCTSTR)hFile, pParams, ppImage );
		AVIFileExit();
	}
	else
	{
		ZeroMemory( m_pTempMap, BUFFER_SIZE );
		nLength = min( nLength, BUFFER_SIZE );
		ReadFile( hFile, m_pTempMap, nLength, &nLength, NULL );
		
		AVIFileInit();
		hResult = LoadImpl( m_szTemp, pParams, ppImage );
		AVIFileExit();
	}
	
	LeaveCriticalSection( &m_pSection );
	
	return hResult;
}

HRESULT STDMETHODCALLTYPE CAVIThumb::LoadFromMemory(SAFEARRAY __RPC_FAR *pMemory, IMAGESERVICEDATA __RPC_FAR *pParams, SAFEARRAY __RPC_FAR *__RPC_FAR *ppImage)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CAVIThumb::SaveToFile(HANDLE hFile, IMAGESERVICEDATA __RPC_FAR *pParams, SAFEARRAY __RPC_FAR *pImage)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CAVIThumb::SaveToMemory(SAFEARRAY __RPC_FAR *__RPC_FAR *ppMemory, IMAGESERVICEDATA __RPC_FAR *pParams, SAFEARRAY __RPC_FAR *pImage)
{
	return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////
// CAVIThumb actually load

HRESULT CAVIThumb::LoadImpl(LPCTSTR pszFile, IMAGESERVICEDATA __RPC_FAR *pParams, SAFEARRAY __RPC_FAR *__RPC_FAR *ppImage)
{
	AVISTREAMINFO pInfo;
	PAVISTREAM pStream;
	
	if ( AVIStreamOpenFromFile( &pStream, pszFile, streamtypeVIDEO, 0,
		OF_READ|OF_SHARE_DENY_NONE, NULL ) )
	{
		return E_FAIL;
	}
	
	if ( AVIStreamInfo( pStream, &pInfo, sizeof(pInfo) ) ||
		 pInfo.rcFrame.right == 0 || pInfo.rcFrame.bottom == 0 )
	{
		AVIStreamRelease( pStream );
		return E_FAIL;
	}
	
	TCHAR szCodec[5] = { 0, 0, 0, 0, 0 };
	CopyMemory( szCodec, &pInfo.fccHandler, 4 );
	
	DWORD nProtect = 0;
	CRegKey pKey;
	
	pKey.Create( HKEY_LOCAL_MACHINE, FAIL_KEY );
	pKey.QueryDWORDValue( szCodec, nProtect );
	
	if ( nProtect )
	{
		AVIStreamRelease( pStream );
		return E_UNEXPECTED;
	}
	
	pParams->nWidth			= pInfo.rcFrame.right;
	pParams->nHeight		= pInfo.rcFrame.bottom;
	pParams->nComponents	= 3;
	
	if ( pParams->nFlags & IMAGESERVICE_SCANONLY )
	{
		AVIStreamRelease( pStream );
		return S_OK;
	}
	
	pKey.SetDWORDValue( szCodec, 1 );
	pKey.Close();
	pKey.Open( HKEY_LOCAL_MACHINE, FAIL_KEY );
	
	BITMAPINFOHEADER pBIH;
	ZeroMemory( &pBIH, sizeof(pBIH) );
	
	pBIH.biSize				= sizeof(pBIH);
	pBIH.biWidth			= pParams->nWidth;
	pBIH.biHeight			= pParams->nHeight;
	pBIH.biPlanes			= 1;
	pBIH.biBitCount			= 24;
	pBIH.biCompression		= BI_RGB;
	pBIH.biSizeImage		= pParams->nWidth * pParams->nHeight * 3;
	
	PGETFRAME pFrames = AVIStreamGetFrameOpen( pStream, &pBIH );
	
	if ( pFrames == NULL )
	{
		pKey.DeleteValue( szCodec );
		AVIStreamRelease( pStream );
		return E_FAIL;
	}
	
	for ( UINT nInPitch = pParams->nWidth * 3 ; nInPitch & 3 ; ) nInPitch++;
	for ( UINT nOutPitch = pParams->nWidth * 3 ; nOutPitch & 3 ; ) nOutPitch++;
	
	UINT nArray = nOutPitch * (UINT)pParams->nHeight;
	
	SafeArrayAllocDescriptor( 1, ppImage );
	(*ppImage)->cbElements = 1;
	(*ppImage)->rgsabound[ 0 ].lLbound = 0;
	(*ppImage)->rgsabound[ 0 ].cElements = nArray;
	SafeArrayAllocData( *ppImage );
	
	BYTE* pOutput = NULL;
	
	if ( *ppImage == NULL ||
		 FAILED( SafeArrayAccessData( *ppImage, (void HUGEP* FAR*)&pOutput ) ) )
	{
		pKey.DeleteValue( szCodec );
		AVIStreamRelease( pStream );
		return E_FAIL;
	}
	
	UINT nFrame = AVIStreamLength( pStream ) / 2;
	BOOL bSuccess = FALSE;
	
	for ( ; ; nFrame++ )
	{
		LPBITMAPINFOHEADER pFrame = NULL;
		
		int nKeyFrame = AVIStreamNextKeyFrame( pStream, nFrame );
		
		if ( nKeyFrame >= 0 )
		{
			nFrame = nKeyFrame;
			pFrame = (LPBITMAPINFOHEADER)AVIStreamGetFrame( pFrames, nKeyFrame );
		}
		else
		{
			nKeyFrame = AVIStreamNextSample( pStream, nFrame );
			
			if ( nKeyFrame >= 0 )
			{
				nFrame = nKeyFrame;
				pFrame = (LPBITMAPINFOHEADER)AVIStreamGetFrame( pFrames, nKeyFrame );
			}
		}
		
		if ( pFrame == NULL || pFrame->biBitCount != 24 || bSuccess )
		{
			if ( bSuccess ) break;
			
			SafeArrayUnaccessData( *ppImage );
			SafeArrayDestroy( *ppImage );
			*ppImage = NULL;
			
			AVIStreamGetFrameClose( pFrames );
			AVIStreamRelease( pStream );
			pKey.DeleteValue( szCodec );
			
			return E_FAIL;
		}
		
		BYTE* pSource = ((BYTE*)pFrame) + pFrame->biSize;
		DWORD nIntensity = 0;
		
		for ( int nY = pParams->nHeight - 1 ; nY >= 0 ; nY-- )
		{
			BYTE* pRowOut	= &pOutput[ nY * nOutPitch ];
			BYTE* pRowIn	= pSource;
			DWORD nRowInt	= 0;
			
			for ( int nX = 0 ; nX < pParams->nWidth ; nX++ )
			{
				BYTE bRed, bGreen, bBlue;
				
				bBlue	= *pRowIn++;
				bGreen	= *pRowIn++;
				bRed	= *pRowIn++;
				
				*pRowOut++	= bRed;
				*pRowOut++	= bGreen;
				*pRowOut++	= bBlue;
				
				nRowInt += bRed;
				nRowInt += bGreen;
				nRowInt += bBlue;
			}
			
			nIntensity += nRowInt / ( pParams->nWidth * 3 );
			pSource += nInPitch;
		}
		
		nIntensity /= pParams->nHeight;
		bSuccess = TRUE;
		
		if ( nIntensity > 100 ) break;
	}
	
	SafeArrayUnaccessData( *ppImage );
	
	AVIStreamGetFrameClose( pFrames );
	AVIStreamRelease( pStream );
	
	pParams->nFlags &= ~IMAGESERVICE_SCANONLY;
	pKey.DeleteValue( szCodec );
	
	return S_OK;
}

/*
HRESULT STDMETHODCALLTYPE CAVIThumb::LoadImpl(LPCTSTR pszFile, IMAGESERVICEDATA __RPC_FAR *pParams, SAFEARRAY __RPC_FAR *__RPC_FAR *ppImage)
{
	AVISTREAMINFO pInfo;
	PAVISTREAM pStream;
	PAVIFILE pFile;
	
	if ( AVIFileOpen( &pFile, pszFile, OF_READ|OF_SHARE_DENY_NONE, NULL ) )
	{
		return E_FAIL;
	}

	if ( AVIFileGetStream( pFile, &pStream, streamtypeVIDEO, 0 ) )
	{
		AVIFileRelease( pFile );
		return E_FAIL;
	}

	AVIFileRelease( pFile );

	if ( AVIStreamInfo( pStream, &pInfo, sizeof(pInfo) ) ||
		 pInfo.rcFrame.right == 0 || pInfo.rcFrame.bottom == 0 )
	{
		AVIStreamRelease( pStream );
		return E_FAIL;
	}

	pParams->nWidth			= pInfo.rcFrame.right;
	pParams->nHeight		= pInfo.rcFrame.bottom;
	pParams->nComponents	= 3;
	
	if ( pParams->nFlags & IMAGESERVICE_SCANONLY )
	{
		AVIStreamRelease( pStream );
		return S_OK;
	}

	DWORD nFormat;
	if ( AVIStreamFormatSize( pStream, 0, (LONG*)&nFormat ) )
	{
		AVIStreamRelease( pStream );
		return E_FAIL;
	}

	LPBYTE pFormat = new BYTE[ nFormat ];
	LPBITMAPINFOHEADER pSrcBIH = (LPBITMAPINFOHEADER)pFormat;

	if ( AVIStreamReadFormat( pStream, 0, pFormat, (LONG*)&nFormat ) )
	{
		AVIStreamRelease( pStream );
		return E_FAIL;
	}

	BITMAPINFOHEADER pBIH;
	ZeroMemory( &pBIH, sizeof(pBIH) );

	pBIH.biSize				= sizeof(pBIH);
	pBIH.biWidth			= pParams->nWidth;
	pBIH.biHeight			= pParams->nHeight;
	pBIH.biPlanes			= 1;
	pBIH.biBitCount			= 24;
	pBIH.biCompression		= BI_RGB;
	pBIH.biSizeImage		= pParams->nWidth * pParams->nHeight * 3;
	
	HIC hIC = ICLocate( ICTYPE_VIDEO, pInfo.fccHandler, pSrcBIH, &pBIH, ICMODE_DECOMPRESS );

	if ( hIC == NULL )
	{
		delete [] pFormat;
		AVIStreamRelease( pStream );
		return E_FAIL;
	}

	if ( ICDecompressBegin( hIC, pSrcBIH, &pBIH ) != ICERR_OK )
	{
		ICClose( hIC );
		delete [] pFormat;
		AVIStreamRelease( pStream );
		return E_FAIL;
	}

	for ( UINT nPitch = pParams->nWidth * 3 ; nPitch & 3 ; ) nPitch++;

	BYTE* pOutput;
	UINT nArray = nPitch * (UINT)pParams->nHeight;
	*ppImage = SafeArrayCreateVector( VT_UI1, 0, nArray );
	SafeArrayAccessData( *ppImage, (VOID**)&pOutput );

	for ( UINT nFrame = 0 ; ; nFrame += 2 )
	{
		LPBYTE pBuffer;
		DWORD nBuffer;
		
		if ( AVIStreamSampleSize( pStream, nFrame, (LONG*)&nBuffer ) ) break;
		pBuffer = new BYTE[ nBuffer ];

		if ( AVIStreamRead( pStream, nFrame, 1, pBuffer, nBuffer, (LONG*)&nBuffer, NULL ) )
		{
			delete [] pBuffer;
			break;
		}
		
		if ( ICDecompress( hIC, 0, pSrcBIH, pBuffer, &pBIH, pOutput ) != ICERR_OK )
		{
			delete [] pBuffer;
			break;
		}

		delete [] pBuffer;

		BYTE* pSource		= pOutput;
		DWORD nIntensity	= 0;

		for ( int nY = pParams->nHeight - 1 ; nY >= 0 ; nY-- )
		{
			BYTE* pRow = pSource;
			DWORD nRowInt = 0;

			for ( int nX = pParams->nWidth ; nX ; nX-- )
			{
				BYTE bRed, bGreen, bBlue;

				bBlue	= pRow[0];
				bGreen	= pRow[1];
				bRed	= pRow[2];

				*pRow++	= bRed;
				*pRow++	= bGreen;
				*pRow++	= bBlue;

				nRowInt += bRed;
				nRowInt += bGreen;
				nRowInt += bBlue;
			}

			nIntensity += nRowInt / ( pParams->nWidth * 3 );

			pSource += nPitch;
		}

		nIntensity /= pParams->nHeight;

		if ( nIntensity > 20 )
		{
			nFrame++;
			break;
		}
	}

	SafeArrayUnaccessData( *ppImage );

	delete [] pFormat;

	ICDecompressEnd( hIC );
	ICClose( hIC );
	AVIStreamRelease( pStream );
	
	if ( nFrame == 0 )
	{
		SafeArrayDestroy( *ppImage );
		*ppImage = NULL;
		return E_FAIL;
	}

	pParams->nFlags &= ~IMAGESERVICE_SCANONLY;

	return S_OK;
}
*/
