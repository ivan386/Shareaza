//
// GIFReader.cpp
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
#include "GIFReader.h"


/////////////////////////////////////////////////////////////////////////////
// CGIFReader construction

CGIFReader::CGIFReader()
{
}

CGIFReader::~CGIFReader()
{
}

/////////////////////////////////////////////////////////////////////////////
// CGIFReader IImageServicePlugin implementation

HRESULT STDMETHODCALLTYPE CGIFReader::LoadFromFile(HANDLE hFile, DWORD nLength, IMAGESERVICEDATA __RPC_FAR *pParams, SAFEARRAY __RPC_FAR *__RPC_FAR *ppImage)
{
	if ( hFile == INVALID_HANDLE_VALUE ) return E_INVALIDARG;
	if ( pParams == NULL ) return E_POINTER;
	if ( ppImage == NULL ) return E_POINTER;
	
	BYTE* pSource = new BYTE[ nLength ];
	DWORD nRead = 0;
	
	ReadFile( hFile, pSource, nLength, &nRead, NULL );
	
	if ( nRead != nLength )
	{
		delete [] pSource;
		return E_FAIL;
	}
	
	HRESULT hr = LoadImpl( pSource, nLength, pParams, ppImage );
	
	delete [] pSource;
	
	return hr;
}

HRESULT STDMETHODCALLTYPE CGIFReader::LoadFromMemory(SAFEARRAY __RPC_FAR *pMemory, IMAGESERVICEDATA __RPC_FAR *pParams, SAFEARRAY __RPC_FAR *__RPC_FAR *ppImage)
{
	if ( pMemory == NULL ) return E_POINTER;
	if ( pParams == NULL ) return E_POINTER;
	if ( ppImage == NULL ) return E_POINTER;
	
	LONG nSource = 0;
	if ( FAILED( SafeArrayGetUBound( pMemory, 1, &nSource ) ) ) return E_INVALIDARG;
	nSource ++;
	
	BYTE* pSource = NULL;
	SafeArrayAccessData( pMemory, &pSource );
	if ( pSource == NULL ) return E_INVALIDARG;
	
	HRESULT hr = LoadImpl( pSource, nSource, pParams, ppImage );
	
	SafeArrayUnaccessData( pMemory );
	
	return hr;
}

HRESULT STDMETHODCALLTYPE CGIFReader::SaveToFile(HANDLE hFile, IMAGESERVICEDATA __RPC_FAR *pParams, SAFEARRAY __RPC_FAR *pImage)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CGIFReader::SaveToMemory(SAFEARRAY __RPC_FAR *__RPC_FAR *ppMemory, IMAGESERVICEDATA __RPC_FAR *pParams, SAFEARRAY __RPC_FAR *pImage)
{
	return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////
// CGIFReader actually load the image

HRESULT CGIFReader::LoadFromMemory(LPBYTE pSource, DWORD nSource, IMAGESERVICEDATA __RPC_FAR *pParams, SAFEARRAY __RPC_FAR *__RPC_FAR *ppImage)
{
	if ( nSource < sizeof(GIFLSD) ) return E_FAIL;
	GIFLSD* pLSD = (GIFLSD*)pSource;
	pSource += sizeof(GIFLSD);
	nSource -= sizeof(GIFLSD);
	if ( memcmp( pLSD->szTag, "GIF8", 4 ) ) return E_FAIL;
	
	pParams->nWidth			= pLSD->nWidth;
	pParams->nHeight		= pLSD->nHeight;
	pParams->nComponents	= 3;
	
	if ( pParams->nFlags & IMAGESERVICE_SCANONLY ) return S_OK;
	pParams->nFlags &= ~IMAGESERVICE_SCANONLY;
	
	// TODO: Load the GIF image and return S_OK
	
	return E_FAIL;
}


