//
// PNGReader.cpp
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
#include "PNGReader.h"

/////////////////////////////////////////////////////////////////////////////
// CPNGReader construction

CPNGReader::CPNGReader()
{
}

CPNGReader::~CPNGReader()
{
}

/////////////////////////////////////////////////////////////////////////////
// CPNGReader load from file

HRESULT STDMETHODCALLTYPE CPNGReader::LoadFromFile(HANDLE hFile, DWORD nLength, IMAGESERVICEDATA __RPC_FAR *pParams, SAFEARRAY __RPC_FAR *__RPC_FAR *ppImage)
{
	png_uint_32 nWidth, nHeight, nRowBytes, nChannels;
    int nBitDepth, nColorType;
	
	struct FileReadStruct pFRS = { hFile, nLength };

	*ppImage = NULL;
	
	png_structp png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING,
		NULL, NULL, NULL );
	
	if ( png_ptr == NULL ) return E_FAIL;
	
	png_infop info_ptr = png_create_info_struct( png_ptr );
	
	if ( info_ptr == NULL )
	{
		png_destroy_read_struct( &png_ptr, png_infopp_NULL, png_infopp_NULL );
		return E_FAIL;
	}
	
	if ( setjmp( png_jmpbuf( png_ptr ) ) )
	{
		png_destroy_read_struct( &png_ptr, &info_ptr, png_infopp_NULL );

		if ( *ppImage )
		{
			SafeArrayUnaccessData( *ppImage );
			
			if ( pParams->nFlags & IMAGESERVICE_PARTIAL_IN )
			{
				pParams->nFlags |= IMAGESERVICE_PARTIAL_OUT;
				return S_OK;
			}
			
			SafeArrayDestroy( *ppImage );
			*ppImage = NULL;
		}

		return E_FAIL;
	}
	
	png_set_read_fn( png_ptr, (void *)&pFRS, OnFileRead );
	
	png_read_info( png_ptr, info_ptr );
	
	png_get_IHDR( png_ptr, info_ptr, &nWidth, &nHeight, &nBitDepth,
		&nColorType, NULL, NULL, NULL );

    if ( nBitDepth == 16 )
		png_set_strip_16(png_ptr);

    if ( nColorType == PNG_COLOR_TYPE_PALETTE )
		png_set_expand( png_ptr );

    if ( nBitDepth < 8 )
		png_set_expand( png_ptr );

    if ( png_get_valid( png_ptr, info_ptr, PNG_INFO_tRNS ) )
		png_set_expand( png_ptr );

    if ( nColorType == PNG_COLOR_TYPE_GRAY ||
		 nColorType == PNG_COLOR_TYPE_GRAY_ALPHA )
		png_set_gray_to_rgb( png_ptr );
	
	png_read_update_info( png_ptr, info_ptr );

    png_get_IHDR( png_ptr, info_ptr, &nWidth, &nHeight, &nBitDepth,
        &nColorType, NULL, NULL, NULL );
	
    nRowBytes = png_get_rowbytes( png_ptr, info_ptr );
    nChannels = png_get_channels( png_ptr, info_ptr );
    
    pParams->nWidth			= (int)nWidth;
	pParams->nHeight		= (int)nHeight;
	pParams->nComponents	= (int)nChannels;
	
	if ( pParams->nFlags & IMAGESERVICE_SCANONLY )
	{
		png_destroy_read_struct( &png_ptr, &info_ptr, png_infopp_NULL );
		return S_OK;
	}
	
	DWORD nPitch = nWidth * nChannels;
	while ( nPitch & 3 ) nPitch++;

	SafeArrayAllocDescriptor( 1, ppImage );
	(*ppImage)->cbElements = 1;
	(*ppImage)->rgsabound[ 0 ].lLbound = 0;
	(*ppImage)->rgsabound[ 0 ].cElements = nPitch * nHeight;
	SafeArrayAllocData( *ppImage );

	BYTE* pOutput = NULL;

	if ( *ppImage == NULL ||
		 FAILED( SafeArrayAccessData( *ppImage, (void HUGEP* FAR*)&pOutput ) ) )
	{
		png_destroy_read_struct( &png_ptr, &info_ptr, png_infopp_NULL );
		return E_FAIL;
	}

	BYTE** pRows = new BYTE*[ nHeight ];

	for ( UINT nY = 0 ; nY < nHeight ; nY++ )
	{
		pRows[ nY ] = pOutput + nY * nPitch;
	}

	png_read_image( png_ptr, pRows );

	delete [] pRows;

	png_read_end( png_ptr, NULL );

	png_destroy_read_struct( &png_ptr, &info_ptr, png_infopp_NULL );

	SafeArrayUnaccessData( *ppImage );
	
	return S_OK;
}

void CPNGReader::OnFileRead(png_structp png_ptr, png_bytep data, png_size_t length)
{
	struct FileReadStruct* pFRS = (struct FileReadStruct*)png_ptr->io_ptr;

	if ( pFRS->dwLength < length ) png_error( png_ptr, "" );

	DWORD nRead;
	ReadFile( pFRS->hFile, data, length, &nRead, NULL );

	if ( nRead != length ) png_error( png_ptr, "" );

	pFRS->dwLength -= length;
}

/////////////////////////////////////////////////////////////////////////////
// CPNGReader load from memory

HRESULT STDMETHODCALLTYPE CPNGReader::LoadFromMemory(SAFEARRAY __RPC_FAR *pMemory, IMAGESERVICEDATA __RPC_FAR *pParams, SAFEARRAY __RPC_FAR *__RPC_FAR *ppImage)
{
	png_uint_32 nWidth, nHeight, nRowBytes, nChannels;
    int nBitDepth, nColorType;
	
	struct MemReadStruct pMRS = { NULL, 0 };
	
	*ppImage = NULL;
	
	png_structp png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING,
		NULL, NULL, NULL );
	
	if ( png_ptr == NULL ) return E_FAIL;
	
	png_infop info_ptr = png_create_info_struct( png_ptr );
	
	if ( info_ptr == NULL )
	{
		png_destroy_read_struct( &png_ptr, png_infopp_NULL, png_infopp_NULL );
		return E_FAIL;
	}
	
	if ( setjmp( png_jmpbuf( png_ptr ) ) )
	{
		png_destroy_read_struct( &png_ptr, &info_ptr, png_infopp_NULL );
		
		if ( pMRS.pBuffer ) SafeArrayUnaccessData( pMemory );
		
		if ( *ppImage )
		{
			SafeArrayUnaccessData( *ppImage );

			if ( pParams->nFlags & IMAGESERVICE_PARTIAL_IN )
			{
				pParams->nFlags |= IMAGESERVICE_PARTIAL_OUT;
				return S_OK;
			}
			
			SafeArrayDestroy( *ppImage );
			*ppImage = NULL;
		}
		
		return E_FAIL;
	}
	
	SafeArrayGetUBound( pMemory, 1, (LONG*)&pMRS.dwLength );
	pMRS.dwLength++;
	
	SafeArrayAccessData( pMemory, (void**)&pMRS.pBuffer );
	
	png_set_read_fn( png_ptr, (void *)&pMRS, OnMemRead );
	
	png_read_info( png_ptr, info_ptr );
	
	png_get_IHDR( png_ptr, info_ptr, &nWidth, &nHeight, &nBitDepth,
		&nColorType, NULL, NULL, NULL );
	
    if ( nBitDepth == 16 )
		png_set_strip_16(png_ptr);
	
    if ( nColorType == PNG_COLOR_TYPE_PALETTE )
		png_set_expand( png_ptr );
	
    if ( nBitDepth < 8 )
		png_set_expand( png_ptr );
	
    if ( png_get_valid( png_ptr, info_ptr, PNG_INFO_tRNS ) )
		png_set_expand( png_ptr );
	
    if ( nColorType == PNG_COLOR_TYPE_GRAY ||
		 nColorType == PNG_COLOR_TYPE_GRAY_ALPHA )
		png_set_gray_to_rgb( png_ptr );
	
	png_read_update_info( png_ptr, info_ptr );
	
    png_get_IHDR( png_ptr, info_ptr, &nWidth, &nHeight, &nBitDepth,
        &nColorType, NULL, NULL, NULL );
	
    nRowBytes = png_get_rowbytes( png_ptr, info_ptr );
    nChannels = png_get_channels( png_ptr, info_ptr );
    
    pParams->nWidth			= (int)nWidth;
	pParams->nHeight		= (int)nHeight;
	pParams->nComponents	= (int)nChannels;
	
	if ( pParams->nFlags & IMAGESERVICE_SCANONLY )
	{
		png_destroy_read_struct( &png_ptr, &info_ptr, png_infopp_NULL );
		SafeArrayUnaccessData( pMemory );
		return S_OK;
	}
	
	DWORD nPitch = nWidth * nChannels;
	while ( nPitch & 3 ) nPitch++;
	
	SafeArrayAllocDescriptor( 1, ppImage );
	(*ppImage)->cbElements = 1;
	(*ppImage)->rgsabound[ 0 ].lLbound = 0;
	(*ppImage)->rgsabound[ 0 ].cElements = nPitch * nHeight;
	SafeArrayAllocData( *ppImage );
	
	BYTE* pOutput = NULL;
	
	if ( *ppImage == NULL ||
		 FAILED( SafeArrayAccessData( *ppImage, (void HUGEP* FAR*)&pOutput ) ) )
	{
		png_destroy_read_struct( &png_ptr, &info_ptr, png_infopp_NULL );
		SafeArrayUnaccessData( pMemory );
		return E_FAIL;
	}
	
	BYTE** pRows = new BYTE*[ nHeight ];
	
	for ( UINT nY = 0 ; nY < nHeight ; nY++ )
	{
		pRows[ nY ] = pOutput + nY * nPitch;
	}
	
	png_read_image( png_ptr, pRows );

	delete [] pRows;
	
	png_read_end( png_ptr, NULL );
	
	png_destroy_read_struct( &png_ptr, &info_ptr, png_infopp_NULL );
	
	SafeArrayUnaccessData( pMemory );
	SafeArrayUnaccessData( *ppImage );
	
	return S_OK;
}

void CPNGReader::OnMemRead(png_structp png_ptr, png_bytep data, png_size_t length)
{
	struct MemReadStruct* pMRS = (struct MemReadStruct*)png_ptr->io_ptr;

	if ( pMRS->dwLength < length ) png_error( png_ptr, "" );

	CopyMemory( data, pMRS->pBuffer, length );

	pMRS->pBuffer += length;
	pMRS->dwLength -= length;
}

HRESULT STDMETHODCALLTYPE CPNGReader::SaveToFile(HANDLE hFile, IMAGESERVICEDATA __RPC_FAR *pParams, SAFEARRAY __RPC_FAR *pImage)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CPNGReader::SaveToMemory(SAFEARRAY __RPC_FAR *__RPC_FAR *ppMemory, IMAGESERVICEDATA __RPC_FAR *pParams, SAFEARRAY __RPC_FAR *pImage)
{
	return E_NOTIMPL;
}
