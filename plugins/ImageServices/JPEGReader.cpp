//
// JPEGReader.cpp
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
#include "JPEGReader.h"

#define BUFFER_GROW	512


/////////////////////////////////////////////////////////////////////////////
// CJPEGReader construction

CJPEGReader::CJPEGReader()
{
}

CJPEGReader::~CJPEGReader()
{
}

/////////////////////////////////////////////////////////////////////////////
// CJPEGReader load from file

HRESULT STDMETHODCALLTYPE CJPEGReader::LoadFromFile(HANDLE hFile, DWORD nLength, IMAGESERVICEDATA __RPC_FAR *pParams, SAFEARRAY __RPC_FAR *__RPC_FAR *ppImage)
{
	struct jpeg_decompress_struct cinfo;
	struct core_source_mgr jsrc;
	struct core_error_mgr jerr;
	
	*ppImage = NULL;
	
	if ( setjmp( jerr.jump ) )
	{
		jpeg_destroy_decompress( &cinfo );
		
		delete [] jsrc.pBuffer;
		
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
	
	cinfo.err				= jpeg_std_error( &jerr.base );
	jerr.base.error_exit	= OnErrorExit;
	jerr.base.emit_message	= OnEmitMessage;
	jerr.bPartial			= FALSE;
	
	jpeg_create_decompress( &cinfo );
	
	cinfo.src					= &jsrc.base;
	jsrc.base.init_source		= OnNull;
	jsrc.base.fill_input_buffer	= OnFileFill;
	jsrc.base.skip_input_data	= OnFileSkip;
	jsrc.base.resync_to_restart	= jpeg_resync_to_restart;
	jsrc.base.term_source		= OnNull;
	jsrc.base.next_input_byte	= NULL;
	jsrc.base.bytes_in_buffer	= 0;
	jsrc.hFile					= hFile;
	jsrc.dwOffset				= SetFilePointer( hFile, 0, NULL, FILE_CURRENT );
	jsrc.dwLength				= nLength;
	jsrc.dwBuffer				= 4096;
	jsrc.pBuffer				= new BYTE[ jsrc.dwBuffer ];
	
	jpeg_read_header( &cinfo, TRUE );
	
	pParams->nWidth			= cinfo.image_width;
	pParams->nHeight		= cinfo.image_height;
	pParams->nComponents	= cinfo.num_components;
	
	if ( pParams->nFlags & IMAGESERVICE_SCANONLY )
	{
		jpeg_destroy_decompress( &cinfo );
		delete [] jsrc.pBuffer;
		return TRUE;
	}
	
	jpeg_start_decompress( &cinfo );
	
	DWORD nPitch = cinfo.output_width * cinfo.output_components;
	while ( nPitch & 3 ) nPitch++;
	
	SafeArrayAllocDescriptor( 1, ppImage );
	(*ppImage)->cbElements = 1;
	(*ppImage)->rgsabound[ 0 ].lLbound = 0;
	(*ppImage)->rgsabound[ 0 ].cElements = nPitch * cinfo.output_height;
	SafeArrayAllocData( *ppImage );
	
	BYTE* pOutput = NULL;
	
	if ( *ppImage == NULL ||
		 FAILED( SafeArrayAccessData( *ppImage, (void HUGEP* FAR*)&pOutput ) ) )
	{
		jpeg_finish_decompress( &cinfo );
		jpeg_destroy_decompress( &cinfo );
		delete [] jsrc.pBuffer;
		return E_FAIL;
	}
	
	while ( cinfo.output_scanline < cinfo.output_height )
	{
		BYTE* pLine = pOutput + nPitch * cinfo.output_scanline;
		jpeg_read_scanlines( &cinfo, &pLine, 1 );
	}
	
	SafeArrayUnaccessData( *ppImage );
	
	jpeg_finish_decompress( &cinfo );
	jpeg_destroy_decompress( &cinfo );
	
	delete [] jsrc.pBuffer;
	
	if ( jerr.bPartial )
	{
		if ( ( pParams->nFlags & IMAGESERVICE_PARTIAL_IN ) == 0 )
		{
			SafeArrayDestroy( *ppImage );
			*ppImage = NULL;
			return E_FAIL;
		}
		
		pParams->nFlags |= IMAGESERVICE_PARTIAL_OUT;
	}
	
	return S_OK;
}

boolean CJPEGReader::OnFileFill(j_decompress_ptr cinfo)
{
	struct core_source_mgr* pCore = (struct core_source_mgr*)cinfo->src;

	if ( pCore->dwLength == 0 )
	{
		static JOCTET pEOI[2] = { 0xFF, JPEG_EOI };
		pCore->base.next_input_byte = pEOI;
		pCore->base.bytes_in_buffer = 2;
		return FALSE;
	}
	
	pCore->base.bytes_in_buffer = min( pCore->dwLength, pCore->dwBuffer );
	pCore->base.next_input_byte = pCore->pBuffer;

	DWORD nRead;
	ReadFile( pCore->hFile, pCore->pBuffer, pCore->base.bytes_in_buffer, &nRead, NULL );
	pCore->base.bytes_in_buffer = nRead;
	pCore->dwLength -= nRead;

	return TRUE;
}

void CJPEGReader::OnFileSkip(j_decompress_ptr cinfo, long num_bytes)
{
	if ( num_bytes > 0 )
	{
		while ( num_bytes > (long)cinfo->src->bytes_in_buffer )
		{
			num_bytes -= (long) cinfo->src->bytes_in_buffer;
			OnFileFill( cinfo );
		}
		cinfo->src->next_input_byte += (size_t)num_bytes;
		cinfo->src->bytes_in_buffer -= (size_t)num_bytes;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CJPEGReader load from memory

HRESULT STDMETHODCALLTYPE CJPEGReader::LoadFromMemory(SAFEARRAY __RPC_FAR *pMemory, IMAGESERVICEDATA __RPC_FAR *pParams, SAFEARRAY __RPC_FAR *__RPC_FAR *ppImage)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_source_mgr jsrc;
	struct core_error_mgr jerr;

	BYTE* pSource;
	LONG nSource;
	SafeArrayGetUBound( pMemory, 1, &nSource );
	SafeArrayAccessData( pMemory, (void HUGEP* FAR*)&pSource );

	*ppImage = NULL;

	if ( setjmp( jerr.jump ) )
	{
		jpeg_destroy_decompress( &cinfo );
		SafeArrayUnaccessData( pMemory );

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

	cinfo.err				= jpeg_std_error( &jerr.base );
	jerr.base.error_exit	= OnErrorExit;
	jerr.base.emit_message	= OnEmitMessage;
	jerr.bPartial			= FALSE;
	
	jpeg_create_decompress( &cinfo );
	
	cinfo.src				= &jsrc;
	jsrc.next_input_byte	= (JOCTET*)pSource;
	jsrc.bytes_in_buffer	= ++nSource;

	jsrc.init_source		= OnNull;
	jsrc.fill_input_buffer	= OnMemFill;
	jsrc.skip_input_data	= OnMemSkip;
	jsrc.resync_to_restart	= jpeg_resync_to_restart;
	jsrc.term_source		= OnNull;

	jpeg_read_header( &cinfo, TRUE );

	pParams->nWidth			= cinfo.image_width;
	pParams->nHeight		= cinfo.image_height;
	pParams->nComponents	= cinfo.num_components;

	if ( pParams->nFlags & IMAGESERVICE_SCANONLY )
	{
		jpeg_destroy_decompress( &cinfo );
		SafeArrayUnaccessData( pMemory );
		return TRUE;
	}

	jpeg_start_decompress( &cinfo );

	DWORD nPitch = cinfo.output_width * cinfo.output_components;
	while ( nPitch & 3 ) nPitch++;

	SafeArrayAllocDescriptor( 1, ppImage );
	(*ppImage)->cbElements = 1;
	(*ppImage)->rgsabound[ 0 ].lLbound = 0;
	(*ppImage)->rgsabound[ 0 ].cElements = nPitch * cinfo.output_height;
	SafeArrayAllocData( *ppImage );

	BYTE* pOutput = NULL;

	if ( *ppImage == NULL ||
		 FAILED( SafeArrayAccessData( *ppImage, (void HUGEP* FAR*)&pOutput ) ) )
	{
		jpeg_finish_decompress( &cinfo );
		jpeg_destroy_decompress( &cinfo );
		SafeArrayUnaccessData( pMemory );
		return E_FAIL;
	}

	while ( cinfo.output_scanline < cinfo.output_height )
	{
		BYTE* pLine = pOutput + nPitch * cinfo.output_scanline;
		jpeg_read_scanlines( &cinfo, &pLine, 1 );
	}

	SafeArrayUnaccessData( *ppImage );

	jpeg_finish_decompress( &cinfo );
	jpeg_destroy_decompress( &cinfo );

	SafeArrayUnaccessData( pMemory );

	if ( jerr.bPartial )
	{
		if ( ( pParams->nFlags & IMAGESERVICE_PARTIAL_IN ) == 0 )
		{
			SafeArrayDestroy( *ppImage );
			*ppImage = NULL;
			return E_FAIL;
		}
		
		pParams->nFlags |= IMAGESERVICE_PARTIAL_OUT;
	}
	
	return S_OK;
}

boolean CJPEGReader::OnMemFill(j_decompress_ptr cinfo)
{
	static JOCTET pEOI[2] = { 0xFF, JPEG_EOI };

    cinfo->src->next_input_byte = pEOI;
    cinfo->src->bytes_in_buffer = 2;

	return FALSE;
}

void CJPEGReader::OnMemSkip(j_decompress_ptr cinfo, long num_bytes)
{
	if ( num_bytes > 0 )
	{
		while ( num_bytes > (long)cinfo->src->bytes_in_buffer )
		{
			num_bytes -= (long) cinfo->src->bytes_in_buffer;
			OnMemFill( cinfo );
		}
		cinfo->src->next_input_byte += (size_t)num_bytes;
		cinfo->src->bytes_in_buffer -= (size_t)num_bytes;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CJPEGReader save to file

HRESULT STDMETHODCALLTYPE CJPEGReader::SaveToFile(HANDLE hFile, IMAGESERVICEDATA __RPC_FAR *pParams, SAFEARRAY __RPC_FAR *pImage)
{
	if ( pParams == NULL || pImage == NULL ) return E_POINTER;
	
	return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////
// CJPEGReader save to memory

HRESULT STDMETHODCALLTYPE CJPEGReader::SaveToMemory(SAFEARRAY __RPC_FAR *__RPC_FAR *ppMemory, IMAGESERVICEDATA __RPC_FAR *pParams, SAFEARRAY __RPC_FAR *pImage)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	core_dest_mgr jdst;
	
	if ( ppMemory == NULL || pParams == NULL || pImage == NULL ) return E_POINTER;
	*ppMemory = NULL;
	
	BYTE* pSource;
	LONG nSource;
	SafeArrayGetUBound( pImage, 1, &nSource );
	SafeArrayAccessData( pImage, (void HUGEP* FAR*)&pSource );
	nSource++;
	
	DWORD nPitch = pParams->nWidth * pParams->nComponents;
	while ( nPitch & 3 ) nPitch++;
	
	if ( (DWORD)nSource != nPitch * pParams->nHeight )
	{
		SafeArrayUnaccessData( pImage );
		return E_INVALIDARG;
	}
	
	jdst.dwLength					= 0;
	jdst.dwBuffer					= BUFFER_GROW;
	jdst.pBuffer					= new BYTE[ jdst.dwBuffer ];
	jdst.base.next_output_byte		= (JOCTET*)jdst.pBuffer;
	jdst.base.free_in_buffer		= jdst.dwBuffer;

	jdst.base.init_destination		= OnNull;
	jdst.base.empty_output_buffer	= OnMemEmpty;
	jdst.base.term_destination		= OnNull;
	
	cinfo.err = jpeg_std_error( &jerr );
	jpeg_create_compress( &cinfo );
	cinfo.dest = &jdst.base;

	cinfo.image_width		= pParams->nWidth;
	cinfo.image_height		= pParams->nHeight;
	cinfo.input_components	= pParams->nComponents;
	cinfo.in_color_space	= pParams->nComponents == 3 ? JCS_RGB : JCS_GRAYSCALE;
	
	if ( pParams->nQuality <= 0 ) pParams->nQuality = 50;
	if ( pParams->nQuality > 100 ) pParams->nQuality = 100;
	
	jpeg_set_defaults( &cinfo );
	jpeg_set_quality( &cinfo, pParams->nQuality, pParams->nQuality <= 25 );
	
	jpeg_start_compress( &cinfo, TRUE );
	
	while ( cinfo.next_scanline < cinfo.image_height )
	{
		jpeg_write_scanlines( &cinfo, &pSource, 1 );
		pSource += nPitch;
	}
	
	SafeArrayUnaccessData( pImage );
	
	jpeg_finish_compress( &cinfo );
	jpeg_destroy_compress( &cinfo );
	
	jdst.dwLength += ( BUFFER_GROW - jdst.base.free_in_buffer );
	
	SafeArrayAllocDescriptor( 1, ppMemory );
	(*ppMemory)->cbElements = 1;
	(*ppMemory)->rgsabound[ 0 ].lLbound		= 0;
	(*ppMemory)->rgsabound[ 0 ].cElements	= jdst.dwLength;
	SafeArrayAllocData( *ppMemory );
	
	BYTE* pOutput = NULL;
	
	if ( *ppMemory == NULL ||
		 FAILED( SafeArrayAccessData( *ppMemory, (void HUGEP* FAR*)&pOutput ) ) )
	{
		delete [] jdst.pBuffer;
		return E_FAIL;
	}
	
	CopyMemory( pOutput, jdst.pBuffer, jdst.dwLength );
	SafeArrayUnaccessData( *ppMemory );
	
	delete [] jdst.pBuffer;
	
	return S_OK;
}

boolean CJPEGReader::OnMemEmpty(j_compress_ptr cinfo)
{
	core_dest_mgr* pCore = (core_dest_mgr*)cinfo->dest;
	
	pCore->dwLength += BUFFER_GROW;
	pCore->dwBuffer += BUFFER_GROW;
	
	BYTE* pNew = new BYTE[ pCore->dwBuffer ];
	CopyMemory( pNew, pCore->pBuffer, pCore->dwLength );
	delete [] pCore->pBuffer;
	pCore->pBuffer = pNew;
	
	pCore->base.next_output_byte = pCore->pBuffer + pCore->dwLength;
	pCore->base.free_in_buffer = BUFFER_GROW;
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// CJPEGReader common event handlers

void CJPEGReader::OnEmitMessage(j_common_ptr cinfo, int msg_level)
{
	struct core_error_mgr* pCore = (struct core_error_mgr*)cinfo->err;
	if ( msg_level < 1 ) pCore->bPartial = TRUE;
}

void CJPEGReader::OnErrorExit(j_common_ptr cinfo)
{
	struct core_error_mgr* pCore = (struct core_error_mgr*)cinfo->err;
	longjmp( pCore->jump, 1 );
}

void CJPEGReader::OnNull(j_compress_ptr cinfo)
{
}

void CJPEGReader::OnNull(j_decompress_ptr cinfo)
{
}
