//
// ImageService.cpp : Implementation of CImageService
//
// Copyright (c) Nikolay Raspopov, 2009-2012.
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

#include "stdafx.h"
#include "ImageService.h"

CImageService::CImageService() throw()
	: m_hShell32( NULL )
	, m_pfnSHCreateItemFromParsingName( NULL )
{
}

HRESULT CImageService::FinalConstruct() throw()
{
	m_hShell32 = LoadLibrary( _T("shell32.dll") );
	if ( ! m_hShell32 )
		return E_FAIL;

	(FARPROC&)m_pfnSHCreateItemFromParsingName = GetProcAddress( m_hShell32,
		"SHCreateItemFromParsingName" );
	if ( ! m_pfnSHCreateItemFromParsingName )
		return E_FAIL;

	return m_pCache.CoCreateInstance( CLSID_LocalThumbnailCache );
}

void CImageService::FinalRelease() throw()
{
	m_pCache.Release();

	FreeLibrary( m_hShell32 );
	m_hShell32 = NULL;
}

HRESULT CImageService::SafeGetThumbnail(IThumbnailCache* pCache, IShellItem* pItem, ISharedBitmap** ppBitmap) throw()
{
	__try
	{
		return pCache->GetThumbnail( pItem, 256, WTS_EXTRACT, ppBitmap, NULL, NULL );
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		return E_FAIL;
	}
}

HRESULT CImageService::LoadFromBitmap(HBITMAP hBitmap, IMAGESERVICEDATA* pParams, SAFEARRAY** ppImage) throw()
{
	BITMAP bmInfo;
	if ( ! GetObject( hBitmap, sizeof( BITMAP ), &bmInfo ) )
		return E_FAIL;

	if ( bmInfo.bmType != 0 || bmInfo.bmPlanes != 1 || ! bmInfo.bmBits ||
		bmInfo.bmWidth <= 0 || bmInfo.bmHeight <= 0 )
		// Unsupported format
		return E_FAIL;

	pParams->nComponents = 3;
	pParams->nWidth = bmInfo.bmWidth;
	pParams->nHeight = bmInfo.bmHeight;

	if ( ( pParams->nFlags & IMAGESERVICE_SCANONLY ) )
		return S_OK;

	DWORD line_size = ( pParams->nWidth * pParams->nComponents + 3 ) & ~3;
	DWORD total_size = line_size * pParams->nHeight;

	*ppImage = SafeArrayCreateVector( VT_UI1, 0, total_size );
	if ( ! *ppImage )
		return E_OUTOFMEMORY;

	BYTE* dst = NULL;
	if ( FAILED( SafeArrayAccessData( *ppImage, (void**)&dst ) ) )
		return E_OUTOFMEMORY;

	HDC hDC = GetDC( NULL );
	BITMAPINFOHEADER bmi = { sizeof( BITMAPINFOHEADER ), bmInfo.bmWidth, - bmInfo.bmHeight, 1, 24, BI_RGB };
	GetDIBits( hDC, hBitmap, 0, bmInfo.bmHeight, dst, (BITMAPINFO*)&bmi, DIB_RGB_COLORS );
	ReleaseDC( NULL, hDC );

	// BGR -> RGB
	for ( LONG j = 0; j < bmInfo.bmHeight; ++j, dst += line_size )
	{
		for ( LONG i = 0; i < bmInfo.bmWidth * 3; i += 3 )
		{
			BYTE c = dst[i + 0];
			dst[i + 0] = dst[i + 2];
			dst[i + 2] = c;
		}
	}

	SafeArrayUnaccessData( *ppImage );

	return S_OK;
}

STDMETHODIMP CImageService::LoadFromFile (
	/* [in] */ BSTR sFile,
	/* [in,out] */ IMAGESERVICEDATA* pParams,
	/* [out] */ SAFEARRAY** ppImage ) throw()
{
	ATLTRACE( "CImageService::LoadFromFile (\"%s\", 0x%08x, 0x%08x)\n", (LPCSTR)CW2A( (LPCWSTR)sFile ), pParams, ppImage );

	if ( ! pParams || ! ppImage )
		return E_POINTER;

	*ppImage = NULL;

	CComPtr< IShellItem > pItem;
	HRESULT hr = m_pfnSHCreateItemFromParsingName( sFile, NULL, IID_PPV_ARGS( &pItem ) );
	if ( SUCCEEDED( hr ) )
	{
		CComPtr< ISharedBitmap > pBitmap;
		hr = SafeGetThumbnail( m_pCache, pItem, &pBitmap );
		if ( SUCCEEDED( hr ) )
		{
			HBITMAP hBitmap = NULL;
			hr = pBitmap->GetSharedBitmap( &hBitmap );
			if ( SUCCEEDED( hr ) )
			{
				hr = LoadFromBitmap( hBitmap, pParams, ppImage );
			}
		}
	}

	if ( FAILED( hr ) && *ppImage )
	{
		SafeArrayDestroy( *ppImage );
		*ppImage = NULL;
	}

	return hr;
}

STDMETHODIMP CImageService::LoadFromMemory (
	/* [in] */ BSTR /*sType*/,
	/* [in] */ SAFEARRAY* /*pMemory*/,
	/* [in,out] */ IMAGESERVICEDATA* /*pParams*/,
	/* [out] */ SAFEARRAY** /*ppImage*/ ) throw()
{
	ATLTRACENOTIMPL( "CImageService::LoadFromMemory" );
}

STDMETHODIMP CImageService::SaveToFile (
	/* [in] */ BSTR /*sFile*/,
	/* [in,out] */ IMAGESERVICEDATA* /*pParams*/,
	/* [in] */ SAFEARRAY* /*pImage*/) throw()
{
	ATLTRACENOTIMPL( "CImageService::SaveToFile" );
}

STDMETHODIMP CImageService::SaveToMemory (
	/* [in] */ BSTR /*sType*/,
	/* [out] */ SAFEARRAY** /*ppMemory*/,
	/* [in,out] */ IMAGESERVICEDATA* /*pParams*/,
	/* [in] */ SAFEARRAY* /*pImage*/) throw()
{
	ATLTRACENOTIMPL( "CImageService::SaveToMemory" );
}
