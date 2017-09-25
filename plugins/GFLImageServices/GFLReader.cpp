//
// GFLReader.cpp : Implementation of CGFLReader
//
// Copyright (c) Nikolay Raspopov, 2005-2014.
// This file is part of SHAREAZA (shareaza.sourceforge.net)
//
// GFL Library, GFL SDK and XnView
// Copyright (c) 1991-2004 Pierre-E Gougelet
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
#include "GFLReader.h"

HRESULT BitmapToSafeArray (SAFEARRAY** const ppImage, const IMAGESERVICEDATA* const pParams,
	const GFL_BITMAP* hGflBitmap) throw ()
{
	HRESULT hr = E_OUTOFMEMORY;
	ULONG line_size = ( ( pParams->nWidth * pParams->nComponents ) + 3 ) & (-4);
	ULONG total_size = line_size * pParams->nHeight;
	*ppImage = SafeArrayCreateVector (VT_UI1, 0, total_size);
	if (*ppImage) {
		BYTE* pDestination = NULL;
		hr = SafeArrayAccessData (*ppImage, (void**) &pDestination);
		if (SUCCEEDED (hr)) {
			BYTE* dst = pDestination;
			const BYTE* src = hGflBitmap->Data;
			for (LONG line = 0; line < pParams->nHeight; ++line,
				dst += line_size, src += hGflBitmap->BytesPerLine)
				CopyMemory (dst, src, hGflBitmap->BytesPerLine);
			SafeArrayUnaccessData (*ppImage);
		} else
			ATLTRACE( "SafeArrayAccessData error: 0x%08x\n", hr );
	} else
		ATLTRACE( "SafeArrayCreateVector error: Out of memory\n" );
	return hr;
}

STDMETHODIMP CGFLReader::LoadFromFile (
	/* [in] */ BSTR sFile,
	/* [in,out] */ IMAGESERVICEDATA* pParams,
	/* [out] */ SAFEARRAY** ppImage )
{
	ATLTRACE( "CGFLReader::LoadFromFile (\"%s\", 0x%08x, 0x%08x)\n", (LPCSTR)CW2A( (LPCWSTR)sFile ), pParams, ppImage );

	if (!pParams || !ppImage) {
		ATLTRACE( "LoadFromFile error: E_POINTER\n" );
		return E_POINTER;
	}

	*ppImage = NULL;

	GFL_BITMAP* hGflBitmap = NULL;

	HRESULT hr = S_OK;
	GFL_FILE_INFORMATION inf = { 0 };
	WCHAR pszPath[MAX_PATH] = { 0 };

	GFL_ERROR err = gflGetFileInformationW( (LPCWSTR)sFile, -1, &inf );

	if ( err != GFL_NO_ERROR )
	{
		if ( GetShortPathNameW( (LPCWSTR)sFile, pszPath, MAX_PATH ) )
			err = gflGetFileInformationW( pszPath, -1, &inf );
		else err = GFL_ERROR_FILE_OPEN;
	}
	if (err == GFL_NO_ERROR)
	{
		pParams->nHeight = inf.Height;
		pParams->nWidth = inf.Width;
		pParams->nComponents = (inf.ComponentsPerPixel == 4) ? 4 : 3;
		if (pParams->nFlags & IMAGESERVICE_SCANONLY)
		{
			// We need only image info
		} else {
			// Copy image
			GFL_LOAD_PARAMS prm;
			ZeroMemory (&prm, sizeof (prm));
			gflGetDefaultLoadParams (&prm);
			prm.Flags = GFL_LOAD_IGNORE_READ_ERROR | GFL_LOAD_ONLY_FIRST_FRAME |
				GFL_LOAD_FORCE_COLOR_MODEL;
			prm.ColorModel = (inf.ComponentsPerPixel == 4) ? GFL_RGBA : GFL_RGB;
			prm.FormatIndex = inf.FormatIndex;
			hr = SAFEgflLoadBitmap ( *pszPath ? pszPath : (LPCWSTR)sFile, &hGflBitmap, &prm, &inf);

			if (SUCCEEDED (hr))
				hr = BitmapToSafeArray (ppImage, pParams, hGflBitmap);
		}
	} else {
		hr = E_FAIL;
		ATLTRACE( "gflGetFileInformationW error: %s\n", gflGetErrorString( err ) );
	}

	if (hGflBitmap)
		gflFreeBitmap (hGflBitmap);

	if (FAILED (hr) && *ppImage)
	{
		SafeArrayDestroy (*ppImage);
		*ppImage = NULL;
	}

	return hr;
}

STDMETHODIMP CGFLReader::LoadFromMemory (
	/* [in] */ BSTR /* sType */,
	/* [in] */ SAFEARRAY* pMemory,
	/* [in,out] */ IMAGESERVICEDATA* pParams,
	/* [out] */ SAFEARRAY** ppImage )
{
	if (!pMemory || !pParams || !ppImage) {
		ATLTRACE( "LoadFromMemory error: E_POINTER\n" );
		return E_POINTER;
	}

	*ppImage = NULL;

	GFL_BITMAP* hGflBitmap = NULL;
	LONG nSource = 0;
	HRESULT hr = SafeArrayGetUBound (pMemory, 1, &nSource);
	nSource++;
	if (SUCCEEDED (hr)) {
		BYTE* pSource = NULL;
		hr = SafeArrayAccessData (pMemory, (void**) &pSource);
		if (SUCCEEDED (hr)) {
			// Loading image
			GFL_FILE_INFORMATION inf;
			ZeroMemory (&inf, sizeof (inf));
			GFL_ERROR err = gflGetFileInformationFromMemory (pSource, nSource, -1, &inf);
			if (err == GFL_NO_ERROR) {
				pParams->nHeight = inf.Height;
				pParams->nWidth = inf.Width;
				pParams->nComponents = (inf.ComponentsPerPixel == 4) ? 4 : 3;
				if (pParams->nFlags & IMAGESERVICE_SCANONLY) {
					// We need only image info
				} else {
					// Copy image
					GFL_LOAD_PARAMS prm;
					ZeroMemory (&prm, sizeof (prm));
					gflGetDefaultLoadParams (&prm);
					prm.Flags = GFL_LOAD_IGNORE_READ_ERROR | GFL_LOAD_ONLY_FIRST_FRAME |
						GFL_LOAD_FORCE_COLOR_MODEL;
					prm.ColorModel = (inf.ComponentsPerPixel == 4) ? GFL_RGBA : GFL_RGB;
					prm.FormatIndex = inf.FormatIndex;
					hr = SAFEgflLoadBitmapFromMemory (pSource, nSource, &hGflBitmap, &prm, NULL);
					if (SUCCEEDED (hr))
						hr = BitmapToSafeArray (ppImage, pParams, hGflBitmap);
				}
			} else {
				hr = E_FAIL;
				ATLTRACE( "gflGetFileInformationFromMemory error: %s\n", gflGetErrorString( err ) );
			}
			SafeArrayUnaccessData (pMemory);
		} else
			ATLTRACE( "SafeArrayAccessData error: 0x%08x\n", hr );
	} else
		ATLTRACE( "SafeArrayGetUBound error: 0x%08x\n", hr );

	if (hGflBitmap)
		gflFreeBitmap (hGflBitmap);

	if (FAILED (hr) && *ppImage) {
		SafeArrayDestroy (*ppImage);
		*ppImage = NULL;
	}

	return hr;
}

STDMETHODIMP CGFLReader::SaveToFile (
	/* [in] */ BSTR sFile,
	/* [in,out] */ IMAGESERVICEDATA* pParams,
	/* [in] */ SAFEARRAY* pImage)
{
	if (!pParams || !pImage)
		return E_POINTER;

	CString ext (sFile);
	int dot = ext.ReverseFind ('.');
	if (dot != -1)
		ext = ext.Mid (dot + 1);
	ext.MakeLower();

	LONG nSource = 0;
	HRESULT hr = SafeArrayGetUBound (pImage, 1, &nSource);
	nSource++;
	if (SUCCEEDED (hr))
	{
		BYTE* pSource = NULL;
		hr = SafeArrayAccessData (pImage, (void**) &pSource);
		if (SUCCEEDED (hr))
		{
			hr = E_OUTOFMEMORY;
			GFL_BITMAP* hGflBitmap = gflAllockBitmapEx (
				(pParams->nComponents == 4) ? GFL_RGBA : GFL_RGB,
				pParams->nWidth, pParams->nHeight, 8, 4, NULL);
			if (hGflBitmap)
			{
				ATLASSERT (nSource == (((pParams->nWidth * pParams->nComponents) + 3) & (-4)) * pParams->nHeight);
				CopyMemory (hGflBitmap->Data, pSource, nSource);
				GFL_SAVE_PARAMS params = {};
				gflGetDefaultSaveParams (&params);
				params.FormatIndex = GetFormatIndexByExt( CT2A( ext ) );
				params.Quality = (GFL_INT16) pParams->nQuality;
				hr = SAFEgflSaveBitmap( sFile, hGflBitmap, &params );
				gflFreeBitmap (hGflBitmap);
			}
			SafeArrayUnaccessData (pImage);
		}
	}
	ATLTRACE( "SaveToFile (\"%s\", 0x%08x, 0x%08x) : %s\n", (LPCSTR)CW2A( (LPCWSTR)sFile ), pParams, pImage, SUCCEEDED( hr ) ? "OK" : "ERROR" );
	return hr;
}

STDMETHODIMP CGFLReader::SaveToMemory (
	/* [in] */ BSTR sType,
	/* [out] */ SAFEARRAY** ppMemory,
	/* [in,out] */ IMAGESERVICEDATA* pParams,
	/* [in] */ SAFEARRAY* pImage)
{
	if (!ppMemory || !pParams || !pImage)
		return E_POINTER;

	*ppMemory = NULL;

	CString ext (sType);
	int dot = ext.ReverseFind ('.');
	if (dot != -1)
		ext = ext.Mid (dot + 1);
	ext.MakeLower();

	LONG nSource = 0;
	HRESULT hr = SafeArrayGetUBound (pImage, 1, &nSource);
	nSource++;
	if (SUCCEEDED (hr)) {
		BYTE* pSource = NULL;
		hr = SafeArrayAccessData (pImage, (void**) &pSource);
		if (SUCCEEDED (hr)) {
			hr = E_OUTOFMEMORY;
			GFL_BITMAP* hGflBitmap = gflAllockBitmapEx (
				(pParams->nComponents == 4) ? GFL_RGBA : GFL_RGB,
				pParams->nWidth, pParams->nHeight, 8, 4, NULL);
			if (hGflBitmap) {
				ATLASSERT (nSource == (((pParams->nWidth * pParams->nComponents) + 3) & (-4)) *
					pParams->nHeight);
				CopyMemory (hGflBitmap->Data, pSource, nSource);
				GFL_SAVE_PARAMS params;
				gflGetDefaultSaveParams (&params);
				params.FormatIndex = GetFormatIndexByExt( CT2A( ext ) );
				params.Quality = (GFL_INT16) pParams->nQuality;
				GFL_UINT8* data = NULL;
				GFL_UINT32 size = 0;
				hr = SAFEgflSaveBitmapIntoMemory (
					&data, &size, hGflBitmap, &params);
				if (SUCCEEDED (hr)) {
					ATLASSERT (data);
					ATLASSERT (size);
					hr = E_OUTOFMEMORY;
					*ppMemory = SafeArrayCreateVector (VT_UI1, 0, size);
					if (*ppMemory) {
						BYTE* pDestination = NULL;
						hr = SafeArrayAccessData (*ppMemory, (void**) &pDestination);
						if (SUCCEEDED (hr)) {
							CopyMemory (pDestination, data, size);
							SafeArrayUnaccessData (*ppMemory);
						}
					}
					gflMemoryFree (data);
				}
				gflFreeBitmap (hGflBitmap);
			}
			SafeArrayUnaccessData (pImage);
		}
	}

	if (FAILED (hr) && *ppMemory) {
		SafeArrayDestroy (*ppMemory);
		*ppMemory = NULL;
	}

	ATLTRACE( "SaveToMemory (\"%s\", 0x%08x, 0x%08x, 0x%08x) : %s\n", (LPCSTR)CW2A( (LPCWSTR)sType ), ppMemory, pParams, pImage, SUCCEEDED( hr ) ? "OK" : "ERROR" );

	return hr;
}
