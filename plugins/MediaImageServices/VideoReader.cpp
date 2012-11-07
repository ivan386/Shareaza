//
// VideoReader.cpp : Implementation of CVideoReader
//
// Copyright (c) Nikolay Raspopov, 2005-2012.
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
#include "VideoReader.h"

void CVideoReader::CopyBitmap(char* pDestination, const char* pSource,
	const int width, const int height, const int line_size)
{
	// Down-up bitmap copying and BGR -> RGB converting
	// (it is not a simple reverse copy!)
	char* dst = pDestination;
	const char* src = pSource + sizeof( BITMAPINFOHEADER ) +
		line_size * ( height - 1 );
	for ( LONG j = 0; j < height; ++j, dst += line_size, src -= line_size )
	{
		for ( LONG i = 0; i < width * 3; i += 3 )
		{
			dst[i + 0] = src[i + 2];
			dst[i + 1] = src[i + 1];
			dst[i + 2] = src[i + 0];
		}
	}
}

HRESULT CVideoReader::LoadFrame(IMediaDet* pDet, double total_time,
	const IMAGESERVICEDATA* pParams, SAFEARRAY** ppImage)
{
	ULONG line_size = ((pParams->nWidth * pParams->nComponents) + 3) & (-4);
	ULONG total_size = line_size * pParams->nHeight;
	HRESULT hr = E_OUTOFMEMORY;
	*ppImage = SafeArrayCreateVector (VT_UI1, 0, total_size);
	if ( *ppImage )
	{
		char* pDestination = NULL;
		hr = SafeArrayAccessData (*ppImage, (void**) &pDestination);
		if ( SUCCEEDED( hr ) )
		{
			hr = E_OUTOFMEMORY;
			if ( char* buf = new char [ total_size + sizeof( BITMAPINFOHEADER ) ] )
			{
				// Getting 25% frame
				__try
				{
					hr = pDet->GetBitmapBits ( total_time / 4.0, NULL, buf,
						pParams->nWidth, pParams->nHeight);
					if ( SUCCEEDED( hr ) )
					{
						CopyBitmap( pDestination, buf,
							pParams->nWidth, pParams->nHeight, line_size );
					}
					else
						ATLTRACE( "CVideoReader::LoadFrame : Failed to get 25%% frame!\n" );
				}
				__except ( EXCEPTION_EXECUTE_HANDLER )
				{
					ATLTRACE( "CVideoReader::LoadFrame : GetBitmapBits() exception!\n" );
					hr = E_FAIL;
				}
				if ( FAILED( hr ) )
				{
					// Getting first frame
					__try
					{
						hr = pDet->GetBitmapBits ( 0.0, NULL, buf,
							pParams->nWidth, pParams->nHeight );
						if ( SUCCEEDED( hr ) )
						{
							CopyBitmap( pDestination, buf,
								pParams->nWidth, pParams->nHeight, line_size );
						}
						else
							ATLTRACE( "CVideoReader::LoadFrame : Failed to get first frame!\n" );
					}
					__except ( EXCEPTION_EXECUTE_HANDLER )
					{
						ATLTRACE( "CVideoReader::LoadFrame : GetBitmapBits() exception!\n" );
						hr = E_FAIL;
					}
				}
				delete [] buf;
			}
			else
				ATLTRACE( "CVideoReader::LoadFrame : Out of memory!\n" );

			SafeArrayUnaccessData (*ppImage);
		}
		else
			ATLTRACE( "CVideoReader::LoadFrame : SafeArrayAccessData() out of memory!\n" );
	}
	else
		ATLTRACE( "CVideoReader::LoadFrame : SafeArrayCreateVector() out of memory!\n" );

	return hr;
}

# pragma warning( push )
# pragma warning( disable : 4509 )
# pragma warning( disable : 4127 )	// conditional expression is constant

STDMETHODIMP CVideoReader::LoadFromFile (
	/* [in] */ BSTR sFile,
	/* [in,out] */ IMAGESERVICEDATA* pParams,
	/* [out] */ SAFEARRAY** ppImage )
{
	ATLTRACE( "CVideoReader::LoadFromFile : Loading \"%s\"...\n", (LPCSTR)CW2A( (LPCWSTR)sFile ) );

	if ( ! pParams || ! ppImage )
		return E_POINTER;

	*ppImage = NULL;

	CComPtr< IMediaDet > pDet;
	HRESULT hr = pDet.CoCreateInstance( CLSID_MediaDet );
	if ( SUCCEEDED( hr ) )
	{
		__try
		{
			hr = pDet->put_Filename(sFile);
		}
		__except( GetExceptionCode() != EXCEPTION_CONTINUE_EXECUTION )
		{
			ATLTRACE( "CVideoReader::LoadFromFile : put_Filename() exception!\n" );
			return E_FAIL;
		}

		if ( SUCCEEDED( hr ) )
		{
			long lStreams = 0;
			bool bFound = false;
			hr = pDet->get_OutputStreams( &lStreams );
			if ( SUCCEEDED( hr ) )
			{
				AM_MEDIA_TYPE mt = {};
				for ( long i = 0; i < lStreams; i++ )
				{
					hr = pDet->put_CurrentStream( i );
					if ( SUCCEEDED( hr ) )
					{
						GUID major_type = {};
						hr = pDet->get_StreamType( &major_type );
						if ( major_type == MEDIATYPE_Video )
						{
							hr = pDet->get_StreamMediaType( &mt );
							if ( SUCCEEDED( hr ) &&
								 mt.formattype == FORMAT_VideoInfo && 
								 mt.cbFormat >= sizeof(VIDEOINFOHEADER) &&
								 mt.pbFormat != NULL )
							{
								bFound = true;
								break;
							}
							if ( !bFound )
							{
								if ( mt.cbFormat != 0 )
									CoTaskMemFree( mt.pbFormat );
								if ( mt.pUnk != NULL )
									mt.pUnk->Release();
								ZeroMemory( &mt, sizeof(AM_MEDIA_TYPE) );
							}
						}
					}
				}
				if ( bFound )
				{
					VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)mt.pbFormat;
					LPWSTR clsid = NULL;
					StringFromCLSID( mt.subtype, &clsid );
					if ( mt.subtype == MEDIASUBTYPE_Y41P )
					{
						ATLTRACE( "Video format: MPEG %s\n", (LPCSTR)CW2A( clsid ) );
					}
					else if (
						mt.subtype.Data2 == 0x0000 &&
						mt.subtype.Data3 == 0x0010 &&
						mt.subtype.Data4[0] == 0x80 &&
						mt.subtype.Data4[1] == 0x00 &&
						mt.subtype.Data4[2] == 0x00 &&
						mt.subtype.Data4[3] == 0xAA &&
						mt.subtype.Data4[4] == 0x00 &&
						mt.subtype.Data4[5] == 0x38 &&
						mt.subtype.Data4[6] == 0x9B &&
						mt.subtype.Data4[7] == 0x71 )
					{
						ATLTRACE( "Video format: %c%c%c%c %s\n",
							LOBYTE (LOWORD (mt.subtype.Data1)),
							HIBYTE (LOWORD (mt.subtype.Data1)),
							LOBYTE (HIWORD (mt.subtype.Data1)),
							HIBYTE (HIWORD (mt.subtype.Data1)),
							(LPCSTR)CW2A( clsid ) );
					}
					else
						ATLTRACE( "Video format: Unknown %s\n", (LPCSTR)CW2A( clsid ) );
					CoTaskMemFree( clsid );

					ATLTRACE( "Video size: %dx%dx%d\n",
						pVih->bmiHeader.biWidth, pVih->bmiHeader.biHeight,
						pVih->bmiHeader.biBitCount);							
					pParams->nWidth = pVih->bmiHeader.biWidth;
					pParams->nHeight = pVih->bmiHeader.biHeight;				    
					if ( pParams->nHeight < 0 )
						pParams->nHeight = -pParams->nHeight;
					pParams->nComponents = 3; // 24-bit RGB only

					double total_time = 0.0;
					hr = pDet->get_StreamLength( &total_time );
					if ( SUCCEEDED( hr ) )
					{
						double fps = 0.0;
						hr = pDet->get_FrameRate( &fps );
						ATLTRACE( "Video time: %02d:%02d:%02d, %.5g fps\n",
							(int) (total_time / 3600) % 60,
							(int) (total_time / 60) % 60,
							(int) total_time % 60, fps);
						if (pParams->nFlags & IMAGESERVICE_SCANONLY)
						{
							// OK
						}
						// Avoid trying to get an image while debugging as
						// some codecs call ExitProcess() if they detect a
						// debugger is active.
						else if ( IsDebuggerPresent() )
						{
							hr = E_ACCESSDENIED;
							ATLTRACE( "Debugger detected!\n" );
						}
						else
						{
							hr = LoadFrame( pDet, total_time, pParams, ppImage );
						}
					}
					else
						ATLTRACE( "CVideoReader::LoadFromFile : Cannot get stream length: 0x%08x\n", hr);
				}
				else
				{
					hr = E_INVALIDARG;
					ATLTRACE( "CVideoReader::LoadFromFile : Video stream not found!\n" );
				}

				if ( mt.cbFormat )
					CoTaskMemFree ( mt.pbFormat );
				if ( mt.pUnk )
					mt.pUnk->Release();
			}
			else
				ATLTRACE( "CVideoReader::LoadFromFile : Cannot get streams: 0x%08x\n", hr);
		}
		else
			ATLTRACE( "CVideoReader::LoadFromFile : Cannot open file: 0x%08x\n", hr);
	}
	else
		ATLTRACE( "CVideoReader::LoadFromFile : Cannot create MediaDet object: 0x%08x\n", hr);

	if (FAILED (hr) && *ppImage)
	{
		SafeArrayDestroy (*ppImage);
		*ppImage = NULL;
	}

	return hr;
}

STDMETHODIMP CVideoReader::LoadFromMemory (
	/* [in] */ BSTR /*sType*/,
	/* [in] */ SAFEARRAY* /*pMemory*/,
	/* [in,out] */ IMAGESERVICEDATA* /*pParams*/,
	/* [out] */ SAFEARRAY** /*ppImage*/ )
{
	ATLTRACENOTIMPL( "CVideoReader::LoadFromMemory" );
}

STDMETHODIMP CVideoReader::SaveToFile (
	/* [in] */ BSTR /*sFile*/,
	/* [in,out] */ IMAGESERVICEDATA* /*pParams*/,
	/* [in] */ SAFEARRAY* /*pImage*/)
{
	ATLTRACENOTIMPL( "CVideoReader::SaveToFile" );
}

STDMETHODIMP CVideoReader::SaveToMemory (
	/* [in] */ BSTR /*sType*/,
	/* [out] */ SAFEARRAY** /*ppMemory*/,
	/* [in,out] */ IMAGESERVICEDATA* /*pParams*/,
	/* [in] */ SAFEARRAY* /*pImage*/)
{
	ATLTRACENOTIMPL( "CVideoReader::SaveToMemory" );
}

# pragma warning( pop )
