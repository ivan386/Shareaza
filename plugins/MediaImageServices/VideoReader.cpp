//
// VideoReader.cpp : Implementation of CVideoReader
//
// Copyright (c) Nikolay Raspopov, 2005.
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

#include "stdafx.h"
#include <algorithm>
#include "VideoReader.h"

HRESULT CVideoReader::FinalConstruct() throw()
{
	return CoCreateFreeThreadedMarshaler(
		GetControllingUnknown(), &m_pUnkMarshaler.p);
}

void CVideoReader::FinalRelease() throw()
{
	m_pUnkMarshaler.Release();
}

void CopyBitmap (char* pDestination, const char* pSource,
	int width, int height, int line_size)
{
	// Down-up bitmap copying
	//char* dst = pDestination;
	const char* src = pSource + sizeof (BITMAPINFOHEADER);// +
		//line_size * (height - 1);
	std::reverse_copy( src, src + height * line_size, pDestination );
/*	for (LONG j = 0; j < height;
		++j, dst += line_size, src -= line_size) {
		__asm {
			mov edi, dst
			mov esi, src
			mov ecx, width
		loop1:
			mov ax, [esi]
			mov bl, [esi+2]
			mov [edi+1], ah
			mov [edi+2], al
			mov [edi], bl
			add esi, 3
			add edi, 3
			dec ecx
			jnz loop1
		}
	}*/
}

STDMETHODIMP CVideoReader::LoadFromFile (
	/* [in] */ BSTR sFile,
	/* [in,out] */ IMAGESERVICEDATA* pParams,
	/* [out] */ SAFEARRAY** ppImage )
{
	ATLTRACE ("LoadFromFile (\"%ls\", 0x%08x, 0x%08x)\n", sFile, pParams, ppImage);

	if ( ! pParams || ! ppImage )
		return E_POINTER;

	*ppImage = NULL;

	ATLTRACE ("Size=%d, Width=%d, Height=%d, Flags=%d%s%s%s, Components=%d, Quality=%d\n",
		pParams->cbSize, pParams->nWidth, pParams->nHeight, pParams->nFlags,
		((pParams->nFlags & IMAGESERVICE_SCANONLY) ? " ScanOnly" : ""),
		((pParams->nFlags & IMAGESERVICE_PARTIAL_IN) ? " PartialIn" : ""),
		((pParams->nFlags & IMAGESERVICE_PARTIAL_OUT) ? " PartialOut" : ""),
		pParams->nComponents, pParams->nQuality);

	IMediaDet* pDet = NULL;
	HRESULT hr = CoCreateInstance( CLSID_MediaDet, NULL, CLSCTX_ALL,
		IID_IMediaDet, (void**)&pDet );
	if ( SUCCEEDED( hr ) )
	{
		hr = pDet->put_Filename (sFile);
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
					GUID major_type;
					hr = pDet->put_CurrentStream( i );
					if ( SUCCEEDED( hr ) )
					{
						hr = pDet->get_StreamType( &major_type );
						if ( major_type == MEDIATYPE_Video )
						{
							hr = pDet->get_StreamMediaType( &mt );
							if ( SUCCEEDED( hr ) && mt.formattype == FORMAT_VideoInfo && 
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
					LPWSTR clsid;
					StringFromCLSID( mt.subtype, &clsid );
					if ( mt.subtype == MEDIASUBTYPE_Y41P )
					{
						ATLTRACE ("Video format: MPEG %ls\n", clsid);
					}
					else
					{
						if ( mt.subtype.Data2 == 0x0000 &&
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
							ATLTRACE ("Video format: %c%c%c%c %ls\n",
								LOBYTE (LOWORD (mt.subtype.Data1)),
								HIBYTE (LOWORD (mt.subtype.Data1)),
								LOBYTE (HIWORD (mt.subtype.Data1)),
								HIBYTE (HIWORD (mt.subtype.Data1)),
								clsid);
						}
						else
							ATLTRACE ("Video format: Unknown %ls\n", clsid);
					}
					CoTaskMemFree( clsid );
					ATLTRACE ("Video size: %dx%dx%d\n",
						pVih->bmiHeader.biWidth, pVih->bmiHeader.biHeight,
						pVih->bmiHeader.biBitCount);							
					pParams->nWidth = pVih->bmiHeader.biWidth;
					pParams->nHeight = pVih->bmiHeader.biHeight;				    
					if ( pParams->nHeight < 0 )
						pParams->nHeight = -pParams->nHeight;
					pParams->nComponents = 3; // 24-bit RGB only
					double total_time = 0;
					hr = pDet->get_StreamLength( &total_time );
					if ( SUCCEEDED( hr ) )
					{
						double fps = 0;
						hr = pDet->get_FrameRate( &fps );
						ATLTRACE ("Video time: %02d:%02d:%02d, %.5g fps\n",
							(int) (total_time / 3600) % 60,
							(int) (total_time / 60) % 60,
							(int) total_time % 60, fps);							
						if (pParams->nFlags & IMAGESERVICE_SCANONLY)
						{
							// OK
						}
						else
						{
							ULONG line_size = ((pParams->nWidth * pParams->nComponents) + 3) & (-4);
							ULONG total_size = line_size * pParams->nHeight;
							hr = E_OUTOFMEMORY;
							*ppImage = SafeArrayCreateVector (VT_UI1, 0, total_size);
							if (*ppImage)
							{
								char* pDestination = NULL;
								hr = SafeArrayAccessData (*ppImage, (void**) &pDestination);
								if ( SUCCEEDED( hr ) )
								{
									hr = E_OUTOFMEMORY;
									char* buf =
										new char [ total_size +sizeof( BITMAPINFOHEADER ) ];
									if (buf)
									{
										// Getting first frame
										HRESULT hr0 = E_FAIL;
										__try {
											hr0 = pDet->GetBitmapBits ( 0.0,
												NULL, buf, pParams->nWidth, pParams->nHeight );
											if ( SUCCEEDED( hr0 ) )
											{
												CopyBitmap( pDestination, buf,
													pParams->nWidth, pParams->nHeight,
													line_size );
											}
										} __except ( EXCEPTION_EXECUTE_HANDLER )
										{
											ATLTRACE ("GetBitmapBits(%ls) exception\n", sFile);
										}
										// Getting 25% frame
										HRESULT hr1 = E_FAIL;
										__try {
											hr1 = pDet->GetBitmapBits ( total_time / 4.0,
												NULL, buf, pParams->nWidth, pParams->nHeight);
											if ( SUCCEEDED( hr1 ) )
											{
												CopyBitmap( pDestination, buf,
													pParams->nWidth, pParams->nHeight,
													line_size );
											}
										} __except ( EXCEPTION_EXECUTE_HANDLER )
										{
											ATLTRACE ("GetBitmapBits(%ls) exception\n", sFile);
										}
										if ( SUCCEEDED( hr0 ) || SUCCEEDED( hr1 ) )
										{
											hr = S_OK;
										}
										else
										{
											ATLTRACE ("GetBitmapBits(%ls) error : 0x%08x, 0x%08x\n", sFile, hr0, hr1);
											hr = E_FAIL;
										}
										delete [] buf;
									}
									SafeArrayUnaccessData (*ppImage);
								}
							}
						}
					}
				}
				if ( mt.cbFormat != 0 )
					CoTaskMemFree ( mt.pbFormat );
				if ( mt.pUnk != NULL )
					mt.pUnk->Release();
			}
			else
				ATLTRACE ("Cannot get streams: 0x%08x\n", hr);
		}
		else
			ATLTRACE ("Cannot open file: 0x%08x\n", hr);
		pDet->Release ();
	}
	else
		ATLTRACE ("Cannot instante MediaDet object: 0x%08x\n", hr);

	if (FAILED (hr) && *ppImage)
	{
		SafeArrayDestroy (*ppImage);
		*ppImage = NULL;
	}

	return hr;
}

STDMETHODIMP CVideoReader::LoadFromMemory (
	/* [in] */ BSTR sType,
	/* [in] */ SAFEARRAY* pMemory,
	/* [in,out] */ IMAGESERVICEDATA* pParams,
	/* [out] */ SAFEARRAY** ppImage )
{
	ATLTRACENOTIMPL ("LoadFromMemory");
}

STDMETHODIMP CVideoReader::SaveToFile (
	/* [in] */ BSTR sFile,
	/* [in,out] */ IMAGESERVICEDATA* pParams,
	/* [in] */ SAFEARRAY* pImage)
{
	ATLTRACENOTIMPL ("SaveToFile");
}

STDMETHODIMP CVideoReader::SaveToMemory (
	/* [in] */ BSTR sType,
	/* [out] */ SAFEARRAY** ppMemory,
	/* [in,out] */ IMAGESERVICEDATA* pParams,
	/* [in] */ SAFEARRAY* pImage)
{
	ATLTRACENOTIMPL ("SaveToMemory");
}
