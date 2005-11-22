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

const size_t BUFFER_GROW = 512;

/////////////////////////////////////////////////////////////////////////////
// CJPEGReader load from file

HRESULT STDMETHODCALLTYPE CJPEGReader::LoadFromFile(BSTR sFile, IMAGESERVICEDATA* pParams, SAFEARRAY** ppImage )
{
	ATLTRACE ("LoadFromFile (\"%ls\", 0x%08x, 0x%08x)\n", sFile, pParams, ppImage);
	if (!pParams || !ppImage)
		return E_POINTER;

	ATLTRACE ("Size=%d, Width=%d, Height=%d, Flags=%d%s%s%s, Components=%d, Quality=%d\n",
		pParams->cbSize, pParams->nWidth, pParams->nHeight, pParams->nFlags,
		((pParams->nFlags & IMAGESERVICE_SCANONLY) ? " ScanOnly" : ""),
		((pParams->nFlags & IMAGESERVICE_PARTIAL_IN) ? " PartialIn" : ""),
		((pParams->nFlags & IMAGESERVICE_PARTIAL_OUT) ? " PartialOut" : ""),
		pParams->nComponents, pParams->nQuality);

	struct jpeg_decompress_struct cinfo;
	struct core_source_mgr jsrc;
	struct core_error_mgr jerr;
	try
	{
		*ppImage = NULL;
		
		HandleWrapper file( CreateFile( sFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
			OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL ) );
		if ( file.file_ == INVALID_HANDLE_VALUE )
			throw JPEGException();
		DWORD nLength = GetFileSize( file.file_, NULL );

		cinfo.err				= jpeg_std_error( &jerr );
		jerr.error_exit			= OnErrorExit;
		jerr.emit_message		= OnEmitMessage;
		jerr.bPartial			= false;

		jpeg_create_decompress( &cinfo );
		
		cinfo.src				= &jsrc;
		jsrc.init_source		= OnNull;
		jsrc.fill_input_buffer	= OnFileFill;
		jsrc.skip_input_data	= OnFileSkip;
		jsrc.resync_to_restart	= jpeg_resync_to_restart;
		jsrc.term_source		= OnNull;
		jsrc.next_input_byte	= NULL;
		jsrc.bytes_in_buffer	= 0;
		jsrc.hFile				= file.file_;
		jsrc.dwOffset			= SetFilePointer( jsrc.hFile, 0, NULL, FILE_CURRENT );
		jsrc.dwLength			= nLength;
		jsrc.dwBuffer			= 4096;
		jsrc.pBuffer.reset( new BYTE[ jsrc.dwBuffer ] );
		
		jpeg_read_header( &cinfo, TRUE );
		
		pParams->nWidth			= cinfo.image_width;
		pParams->nHeight		= cinfo.image_height;
		pParams->nComponents	= cinfo.num_components;
		
		if ( pParams->nFlags & IMAGESERVICE_SCANONLY )
		{
			jpeg_destroy_decompress( &cinfo );
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
	}
	catch ( ... )
	{
		// oops
		jpeg_destroy_decompress( &cinfo );
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
	return S_OK;
}

boolean CJPEGReader::OnFileFill(j_decompress_ptr cinfo)
{
	struct core_source_mgr* pCore = (struct core_source_mgr*)cinfo->src;

	if ( pCore->dwLength == 0 )
	{
		static JOCTET pEOI[2] = { 0xFF, JPEG_EOI };
		pCore->next_input_byte = pEOI;
		pCore->bytes_in_buffer = 2;
		return FALSE;
	}
	
	pCore->bytes_in_buffer = min( pCore->dwLength, pCore->dwBuffer );
	pCore->next_input_byte = pCore->pBuffer.get();

	DWORD nRead;
	ReadFile( pCore->hFile, pCore->pBuffer.get(), pCore->bytes_in_buffer, &nRead, NULL );
	pCore->bytes_in_buffer = nRead;
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

HRESULT STDMETHODCALLTYPE CJPEGReader::LoadFromMemory(BSTR sType, SAFEARRAY* pMemory, IMAGESERVICEDATA* pParams, SAFEARRAY** ppImage )
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_source_mgr jsrc;
	struct core_error_mgr jerr;

	try
	{
		BYTE* pSource;
		LONG nSource;
		SafeArrayGetUBound( pMemory, 1, &nSource );
		SafeArrayAccessData( pMemory, (void HUGEP* FAR*)&pSource );

		*ppImage = NULL;

		cinfo.err				= jpeg_std_error( &jerr );
		jerr.error_exit	= OnErrorExit;
		jerr.emit_message	= OnEmitMessage;
		jerr.bPartial			= false;
		
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
	}
	catch ( ... )
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
// CJPEGReader save to memory

HRESULT STDMETHODCALLTYPE CJPEGReader::SaveToMemory(BSTR sType, SAFEARRAY** ppMemory, IMAGESERVICEDATA* pParams, SAFEARRAY* pImage)
{
	struct jpeg_compress_struct cinfo;
	struct core_error_mgr jerr;
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

	try
	{
		if ( (DWORD)nSource != nPitch * pParams->nHeight )
		{
			SafeArrayUnaccessData( pImage );
			return E_INVALIDARG;
		}
		
		jdst.dwLength					= 0;
		jdst.dwBuffer					= BUFFER_GROW;
		jdst.pBuffer.reset( new BYTE[ jdst.dwBuffer ] );
		jdst.next_output_byte		= (JOCTET*)jdst.pBuffer.get();
		jdst.free_in_buffer		= jdst.dwBuffer;

		jdst.init_destination		= OnNull;
		jdst.empty_output_buffer	= OnMemEmpty;
		jdst.term_destination		= OnNull;
		
		cinfo.err = jpeg_std_error( &jerr );
		jerr.error_exit	= OnErrorExit;
		jerr.emit_message	= OnEmitMessage;
		jerr.bPartial			= false;
		jpeg_create_compress( &cinfo );
		cinfo.dest = &jdst;

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
		
		jdst.dwLength += ( BUFFER_GROW - jdst.free_in_buffer );
		
		SafeArrayAllocDescriptor( 1, ppMemory );
		(*ppMemory)->cbElements = 1;
		(*ppMemory)->rgsabound[ 0 ].lLbound		= 0;
		(*ppMemory)->rgsabound[ 0 ].cElements	= jdst.dwLength;
		SafeArrayAllocData( *ppMemory );
		
		BYTE* pOutput = NULL;
		
		if ( *ppMemory == NULL ||
			 FAILED( SafeArrayAccessData( *ppMemory, (void HUGEP* FAR*)&pOutput ) ) )
		{
			return E_FAIL;
		}
		
		CopyMemory( pOutput, jdst.pBuffer.get(), jdst.dwLength );
		SafeArrayUnaccessData( *ppMemory );
		
	}
	catch ( ... )
	{
		jpeg_destroy_compress( &cinfo );
		return E_FAIL;
	}

	return S_OK;
}

boolean CJPEGReader::OnMemEmpty(j_compress_ptr cinfo)
{
	core_dest_mgr* pCore = (core_dest_mgr*)cinfo->dest;
	
	pCore->dwLength += BUFFER_GROW;
	pCore->dwBuffer += BUFFER_GROW;
	
	boost::scoped_array< BYTE > pNew( new BYTE[ pCore->dwBuffer ] );
	CopyMemory( pNew.get(), pCore->pBuffer.get(), pCore->dwLength );
	pCore->pBuffer.swap( pNew );
	
	pCore->next_output_byte = pCore->pBuffer.get() + pCore->dwLength;
	pCore->free_in_buffer = BUFFER_GROW;
	
	return true;
}
