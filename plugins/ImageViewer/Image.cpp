//
// Image.cpp
//
// This software is released into the public domain. You are free to
// redistribute and modify without any restrictions.
// This file is part of SHAREAZA (shareaza.sourceforge.net), original author Michael Stokes. 
//
// This file contains a utility class CImage, for managing bitmap images.
// It provides a number of key features:
//
// - Loading images via Shareaza ImageServices plugins
// - Conversion from mono to RGB
// - Alpha channel removal (to solid RGB with background colour)
// - Smooth resampling and conversion to HBITMAP
//

#include "StdAfx.h"
#include "ImageViewer.h"
#include "Image.h"


//////////////////////////////////////////////////////////////////////
// CImage construction

CImage::CImage()
{
	m_pImage		= NULL;
	m_nWidth		= 0;
	m_nHeight		= 0;
	m_nComponents	= 0;
	m_bPartial		= FALSE;
}

CImage::~CImage()
{
	if ( m_pImage ) delete [] m_pImage;
}

//////////////////////////////////////////////////////////////////////
// CImage clear

void CImage::Clear()
{
	if ( m_pImage ) delete [] m_pImage;
	
	m_pImage		= NULL;
	m_nWidth		= 0;
	m_nHeight		= 0;
	m_nComponents	= 0;
	m_bPartial		= FALSE;
}

//////////////////////////////////////////////////////////////////////
// CImage ensure RGB

BOOL CImage::EnsureRGB(COLORREF crFill)
{
	if ( m_nWidth == 0 || m_nHeight == 0 || m_nComponents == 0 )
	{
		return FALSE;
	}
	else if ( m_nComponents == 3 )
	{
		return TRUE;
	}
	else if ( m_nComponents == 1 )
	{
		return MonoToRGB();
	}
	else if ( m_nComponents == 4 )
	{
		return AlphaToRGB( crFill );
	}
	else
	{
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// CImage convert mono to RGB

BOOL CImage::MonoToRGB()
{
	if ( m_nComponents == 3 ) return TRUE;
	if ( m_nComponents != 1 ) return FALSE;
	
	DWORD nInPitch	= m_nWidth * 1;
	DWORD nOutPitch	= m_nWidth * 3;
	
	while ( nInPitch & 3 ) nInPitch++;
	while ( nOutPitch & 3 ) nOutPitch++;
	
	BYTE* pNew		= new BYTE[ nOutPitch * m_nHeight ];
	BYTE* pInRow	= m_pImage;
	BYTE* pOutRow	= pNew;
	
	for ( int nY = m_nHeight ; nY ; nY-- )
	{
		BYTE* pInCol	= pInRow;
		BYTE* pOutCol	= pOutRow;
		
		for ( int nX = m_nWidth ; nX ; nX-- )
		{
			*pOutCol++ = *pInCol;
			*pOutCol++ = *pInCol;
			*pOutCol++ = *pInCol++;
		}
		
		pInRow += nInPitch;
		pOutRow += nOutPitch;
	}
	
	delete [] m_pImage;
	
	m_pImage		= pNew;
	m_nComponents	= 3;
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CImage convert alpha channel to RGB

BOOL CImage::AlphaToRGB(COLORREF crBack)
{
	if ( m_nComponents == 3 ) return TRUE;
	if ( m_nComponents != 4 ) return FALSE;

	DWORD nInPitch	= m_nWidth * 4;
	DWORD nOutPitch	= m_nWidth * 3;

	while ( nInPitch & 3 ) nInPitch++;
	while ( nOutPitch & 3 ) nOutPitch++;

	BYTE* pNew		= new BYTE[ nOutPitch * m_nHeight ];
	BYTE* pInRow	= m_pImage;
	BYTE* pOutRow	= pNew;

	for ( int nY = m_nHeight ; nY ; nY-- )
	{
		BYTE* pInCol	= pInRow;
		BYTE* pOutCol	= pOutRow;
		
		for ( int nX = m_nWidth ; nX ; nX-- )
		{
			DWORD nAlpha = (DWORD)pInCol[3];
			
			if ( nAlpha == 255 )
			{
				*pOutCol++ = *pInCol++;
				*pOutCol++ = *pInCol++;
				*pOutCol++ = *pInCol++;
				pInCol++;
			}
			else if ( nAlpha == 0 )
			{
				*pOutCol++ = GetRValue( crBack );
				*pOutCol++ = GetGValue( crBack );
				*pOutCol++ = GetBValue( crBack );
				pInCol += 4;
			}
			else
			{
				*pOutCol++ = (BYTE)( ( (DWORD)(*pInCol++) * nAlpha + (DWORD)GetRValue( crBack ) * ( 255 - nAlpha ) ) / 255 );
				*pOutCol++ = (BYTE)( ( (DWORD)(*pInCol++) * nAlpha + (DWORD)GetGValue( crBack ) * ( 255 - nAlpha ) ) / 255 );
				*pOutCol++ = (BYTE)( ( (DWORD)(*pInCol++) * nAlpha + (DWORD)GetBValue( crBack ) * ( 255 - nAlpha ) ) / 255 );
				pInCol++;
			}
		}
		
		pInRow += nInPitch;
		pOutRow += nOutPitch;
	}

	delete [] m_pImage;

	m_pImage		= pNew;
	m_nComponents	= 3;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CImage resample to a specific size (must be RGB)

HBITMAP CImage::Resample(int nNewWidth, int nNewHeight)
{
	if ( m_nComponents != 3 ) return NULL;
	
	BITMAPINFO pInfo;
	ZeroMemory( &pInfo, sizeof(pInfo) );
	
	pInfo.bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
	pInfo.bmiHeader.biWidth			= (LONG)nNewWidth;
	pInfo.bmiHeader.biHeight		= (LONG)nNewHeight;
	pInfo.bmiHeader.biPlanes		= 1;
	pInfo.bmiHeader.biBitCount		= 24;
	pInfo.bmiHeader.biCompression	= BI_RGB;
	pInfo.bmiHeader.biSizeImage		= nNewWidth * nNewHeight * 3;
	
	HDC hDC = GetDC( 0 );
	HBITMAP hBitmap = CreateDIBitmap( hDC, &pInfo.bmiHeader, 0, NULL, &pInfo, DIB_RGB_COLORS );
	
	if ( hBitmap == NULL )
	{
		ReleaseDC( 0, hDC );
		return NULL;
	}
	
	DWORD nInPitch	= m_nWidth * 3;
	DWORD nOutPitch	= nNewWidth * 3;
	
	while ( nOutPitch & 3 ) nOutPitch++;
	while ( nInPitch & 3 ) nInPitch++;
	
	int* pColInfo = new int[ nNewWidth * 2 ];
	int* pColPtr = pColInfo;
	
	int nFlag = ( nNewWidth == m_nWidth ) ? 0 :  1;
	
	for ( int nX = 0 ; nX < nNewWidth ; nX++ )
	{
		int nFirst = ( nX * m_nWidth / nNewWidth );
		int nCount = ( (nX+1) * m_nWidth / nNewWidth ) - nFirst + nFlag;
		if ( nFirst + nCount >= m_nWidth ) nCount = 1;
		*pColPtr++ = nFirst * 3;
		*pColPtr++ = nCount;
	}
	
	BYTE* pLine = new BYTE[ nOutPitch * m_nComponents ];
	
	nFlag = ( nNewHeight == m_nHeight ) ? 0 :  1;
	
	for ( int nY = 0 ; nY < nNewHeight ; nY++ )
	{
		int nFirst = ( nY * m_nHeight / nNewHeight );
		int nCount = ( (nY+1) * m_nHeight / nNewHeight ) - nFirst + nFlag;
		
		if ( nFirst + nCount >= m_nHeight ) nCount = 1;
		
		BYTE* pRow = m_pImage + nInPitch * nFirst;
		BYTE* pOut = pLine;
		pColPtr = pColInfo;
		
		for ( int nX = 0 ; nX < nNewWidth ; nX++, pColPtr++ )
		{
			BYTE* pIn = pRow + *pColPtr++;
			
			DWORD nRed = 0, nGreen = 0, nBlue = 0, nPixels = 0;
			
			for ( int nYY = nCount ; nYY ; nYY-- )
			{
				for ( int nXX = *pColPtr ; nXX ; nXX-- )
				{
					nRed	+= *pIn++;
					nGreen	+= *pIn++;
					nBlue	+= *pIn++;
					nPixels++;
				}
				
				pIn += nInPitch - *pColPtr - *pColPtr - *pColPtr;
			}
			
			*pOut++ = (BYTE)( nBlue / nPixels );
			*pOut++ = (BYTE)( nGreen / nPixels );
			*pOut++ = (BYTE)( nRed / nPixels );
		}
		
		SetDIBits( hDC, hBitmap, nNewHeight - nY - 1, 1, pLine, &pInfo, DIB_RGB_COLORS );
	}
	
	delete [] pLine;
	delete [] pColInfo;
	
	ReleaseDC( 0, hDC );
	
	return hBitmap;
}

//////////////////////////////////////////////////////////////////////
// CImage load an image file

BOOL CImage::Load(LPCTSTR pszPath)
{
	// Clear any previous image
	
	Clear();
	
	// Open the file
	
	HANDLE hFile = CreateFile( CString( _T("\\\\?\\") ) + pszPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	
	// Make sure it worked

	if ( hFile == INVALID_HANDLE_VALUE ) return FALSE;

	// Get the file length
	
	DWORD nLength = GetFileSize( hFile, NULL );
	
	// Try to load the Shareaza ImageService for this file type
	
	IImageServicePlugin* pService = LoadService( pszPath );
	
	// Make sure it worked

	if ( pService == NULL )
	{
		CloseHandle( hFile );
		return FALSE;
	}

	// Setup the IMAGESERVICEDATA structure
	
	IMAGESERVICEDATA pParams = {};	
	pParams.cbSize		= sizeof(pParams);
	pParams.nFlags		= IMAGESERVICE_PARTIAL_IN;	// Partial images okay
	
	// Ask the ImageService to load from file handle
	
	SAFEARRAY* pArray = NULL;
	HRESULT hr = pService->LoadFromFile( CComBSTR( CString( _T("\\\\?\\") ) + pszPath ), &pParams, &pArray );
	
	// Check the result
	
	if ( hr == E_NOTIMPL )
	{
		// ImageService does not support loading from file.  This is allowed, but inconvenient for us.  It
		// must support loading from memory, so we'll do that instead.
		
		// If an output was created, get rid of it (technically it shouldn't have happened, but anyway)
		
		if ( pArray != NULL ) SafeArrayDestroy( pArray );
		pArray = NULL;
		
		// Create a file mapping for the file
		
		HANDLE hMap = CreateFileMapping( hFile, NULL, PAGE_READONLY, 0, 0, NULL );
		
		if ( hMap != INVALID_HANDLE_VALUE )
		{
			DWORD nPosition = SetFilePointer( hFile, 0, NULL, FILE_CURRENT );
			
			// Map a view of the whole file
			
			if ( LPCVOID pBuffer = MapViewOfFile( hMap, FILE_MAP_READ, 0, nPosition, nLength ) )
			{
				SAFEARRAY* pInput;
				
				// Create a safearray of the appropriate size
				
				if ( SUCCEEDED( SafeArrayAllocDescriptor( 1, &pInput ) ) && pInput != NULL )
				{
					pInput->cbElements = 1;
					pInput->rgsabound[ 0 ].lLbound = 0;
					pInput->rgsabound[ 0 ].cElements = nLength;
					SafeArrayAllocData( pInput );
					
					// Load the data directly into the safearray
					
					LPBYTE pTarget;
					if ( SUCCEEDED( SafeArrayAccessData( pInput, (void HUGEP* FAR*)&pTarget ) ) )
					{
						CopyMemory( pTarget, pBuffer, nLength );
						SafeArrayUnaccessData( pInput );
					}
					
					// Ask the ImageService to load from memory
					LPCTSTR pszType = _tcsrchr( pszPath, '.' );
					if ( pszType == NULL ) return FALSE;
					
					hr = pService->LoadFromMemory( CComBSTR( pszType ), pInput, &pParams, &pArray );

					SafeArrayDestroy( pInput );
				}
				
				UnmapViewOfFile( pBuffer );
			}

			CloseHandle( hMap );
		}
	}
	
	// Release the ImageService and close the file
	
	pService->Release();
	CloseHandle( hFile );
	
	// Make sure the load (whichever one was used), worked
	
	if ( FAILED(hr) )
	{
		// Get rid of the output if there was one
		if ( pArray != NULL ) SafeArrayDestroy( pArray );
		return FALSE;
	}
	
	// Retreive attributes from the IMAGESERVICELOAD structure
	
	m_nWidth		= pParams.nWidth;
	m_nHeight		= pParams.nHeight;
	m_nComponents	= pParams.nComponents;
	m_bPartial		= 0 != ( pParams.nFlags & IMAGESERVICE_PARTIAL_OUT );
	
	// Get the size of the output data
	
	LONG nArray = 0;
	SafeArrayGetUBound( pArray, 1, &nArray );
	nArray++;
	
	// Calculate the expected size (rows must be 32-bit aligned)
	
	LONG nFullSize = pParams.nWidth * pParams.nComponents;
	while ( nFullSize & 3 ) nFullSize++;
	nFullSize *= pParams.nHeight;
	
	// Make sure the size is what we expected
	
	if ( nArray != nFullSize )
	{
		SafeArrayDestroy( pArray );
		return FALSE;
	}
	
	// Allocate memory for the image
	
	m_pImage = new BYTE[ nArray ];
	
	// Copy to our memory and destroy the safearray
	
	LPBYTE pData;
	SafeArrayAccessData( pArray, (VOID**)&pData );
	CopyMemory( m_pImage, pData, nArray );
	SafeArrayUnaccessData( pArray );
	SafeArrayDestroy( pArray );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CImage load an ImageService

IImageServicePlugin* CImage::LoadService(LPCTSTR pszFile)
{
	LPCTSTR pszType = _tcsrchr( pszFile, '.' );
	if ( ! pszType ) return NULL;
	
	ULONG dwCLSID = 128;
	TCHAR szCLSID[128];

	CRegKey pKey;
		
	if ( pKey.Open( HKEY_CURRENT_USER,
		_T("SOFTWARE\\Shareaza\\Shareaza\\Plugins\\ImageService") ) != ERROR_SUCCESS )
		return NULL;
	
	bool bPartial = lstrcmpi( pszType, _T(".partial") ) == 0;
	if ( pKey.QueryStringValue( ( bPartial ? _T(".jpg") : pszType ), szCLSID, &dwCLSID )
		!= ERROR_SUCCESS )
		return NULL;
	
	pKey.Close();

	CLSID pCLSID = {};
	if ( FAILED( CLSIDFromString( szCLSID, &pCLSID ) ) )
		return NULL;

	IImageServicePlugin* pService = NULL;
	HRESULT hResult = CoCreateInstance( pCLSID, NULL, CLSCTX_ALL,
		IID_IImageServicePlugin, (void**)&pService );
	
	return SUCCEEDED(hResult) ? pService : NULL;
}
