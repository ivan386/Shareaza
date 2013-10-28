//
// Builder.cpp : Implementation of CBuilder
//
// Copyright (c) Nikolay Raspopov, 2005-2013.
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
#include "Builder.h"

STDMETHODIMP CBuilder::Process (
	/* [in] */ BSTR sFile,
	/* [in] */ ISXMLElement* pXML)
{
	HRESULT hr = E_FAIL;
	__try
	{
		hr = SafeProcess( sFile, pXML );
	}
	__except( GetExceptionCode() != EXCEPTION_CONTINUE_EXECUTION )
	{
		return E_FAIL;
	}
	return hr;
}

HRESULT CBuilder::SafeProcess(BSTR sFile, ISXMLElement* pXML)
{
	CString tmp;

	if ( ! pXML )
		return E_POINTER;

	CComPtr <ISXMLElements> pISXMLRootElements;
	HRESULT hr = pXML->get_Elements(&pISXMLRootElements);
	if ( FAILED( hr ) )
		return hr;
	CComPtr <ISXMLElement> pXMLRootElement;
	hr = pISXMLRootElements->Create (CComBSTR ("videos"), &pXMLRootElement);
	if ( FAILED( hr ) )
		return hr;
	CComPtr <ISXMLAttributes> pISXMLRootAttributes;
	hr = pXMLRootElement->get_Attributes(&pISXMLRootAttributes);
	if ( FAILED( hr ) )
		return hr;
	pISXMLRootAttributes->Add (CComBSTR ("xmlns:xsi"),
		CComBSTR ("http://www.w3.org/2001/XMLSchema-instance"));
	pISXMLRootAttributes->Add (CComBSTR ("xsi:noNamespaceSchemaLocation"),
		CComBSTR ("http://www.limewire.com/schemas/video.xsd"));

	CComPtr <ISXMLElements> pISXMLElements;
	hr = pXMLRootElement->get_Elements(&pISXMLElements);
	if ( FAILED( hr ) )
		return hr;
	CComPtr <ISXMLElement> pXMLElement;
	hr = pISXMLElements->Create (CComBSTR ("video"), &pXMLElement);
	if ( FAILED( hr ) )
		return hr;
	CComPtr <ISXMLAttributes> pISXMLAttributes;
	hr = pXMLElement->get_Attributes(&pISXMLAttributes);
	if ( FAILED( hr ) )
		return hr;

	CComPtr< IMediaDet > pDet;
	hr = pDet.CoCreateInstance( CLSID_MediaDet );
	if ( SUCCEEDED( hr ) )
	{
		hr = pDet->put_Filename(sFile);

		if ( SUCCEEDED( hr ) )
		{
			long lStreams = 0;
			bool bFound = false;
			hr = pDet->get_OutputStreams (&lStreams);
			if ( SUCCEEDED( hr ) )
			{
				AM_MEDIA_TYPE mt = {};
				for (long i = 0; i < lStreams; i++)
				{
					hr = pDet->put_CurrentStream (i);
					if ( SUCCEEDED( hr ) )
					{
						GUID major_type = {};
						hr = pDet->get_StreamType (&major_type);
						if ( major_type == MEDIATYPE_Video )
						{
							hr = pDet->get_StreamMediaType (&mt);
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
								if (mt.cbFormat != 0)
									CoTaskMemFree (mt.pbFormat);
								if (mt.pUnk != NULL)
									mt.pUnk->Release();
								ZeroMemory( &mt, sizeof(AM_MEDIA_TYPE) );
							}
						}
					}
				}
				if (bFound)
				{
					const VIDEOINFOHEADER *pVih = (const VIDEOINFOHEADER*)mt.pbFormat;
					
					// TODO: This is video format not a codec
					/*CString codec;
					if (mt.subtype == MEDIASUBTYPE_Y41P)
					{
						codec = _T("MPEG");
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
						mt.subtype.Data4[7] == 0x71)
					{
						codec.Format(_T("%c%c%c%c"),
							LOBYTE (LOWORD (mt.subtype.Data1)),
							HIBYTE (LOWORD (mt.subtype.Data1)),
							LOBYTE (HIWORD (mt.subtype.Data1)),
							HIBYTE (HIWORD (mt.subtype.Data1)));
					}
					else if (mt.subtype == MEDIASUBTYPE_RGB1)
					{
						codec = _T("RGB1");
					}
					else if (mt.subtype == MEDIASUBTYPE_RGB4)
					{
						codec = _T("RGB4");
					}
					else if (mt.subtype == MEDIASUBTYPE_RGB8)
					{
						codec = _T("RGB8");
					}
					else if (mt.subtype == MEDIASUBTYPE_RGB565)
					{
						codec = _T("RGB565");
					}
					else if (mt.subtype == MEDIASUBTYPE_RGB555)
					{
						codec = _T("RGB555");
					}
					else if (mt.subtype == MEDIASUBTYPE_RGB24)
					{
						codec = _T("RGB24");
					}
					else if (mt.subtype == MEDIASUBTYPE_RGB32)
					{
						codec = _T("RGB32");
					}
					else
					{
#ifdef _DEBUG
						LPWSTR clsid = NULL;
						if (SUCCEEDED (StringFromCLSID (mt.subtype, &clsid)))
						{
							ATLTRACE( "Video format: %s\n", CW2A( clsid ) );
							CoTaskMemFree (clsid);
						}
#endif // _DEBUG
						codec = _T("Unknown");
					}
					pISXMLAttributes->Add (CComBSTR ("codec"), CComBSTR (codec));*/

					int nWidth = pVih->bmiHeader.biWidth;
					int nHeight = pVih->bmiHeader.biHeight;				    
					if (nHeight < 0)
						nHeight = -nHeight;
					tmp.Format (_T("%d"), nWidth);
					pISXMLAttributes->Add (CComBSTR ("width"), CComBSTR (tmp));
					tmp.Format (_T("%d"), nHeight);
					pISXMLAttributes->Add (CComBSTR ("height"), CComBSTR (tmp));

					if ( mt.cbFormat )
						CoTaskMemFree( mt.pbFormat );
					if ( mt.pUnk )
						mt.pUnk->Release();
				}

				double total_time = 0.0;
				hr = pDet->get_StreamLength (&total_time);
				if ( SUCCEEDED( hr ) )
				{
					if ( total_time != .0 ) 
					{
						StringCbPrintf( tmp.GetBuffer( 32 ), 32 * sizeof( TCHAR ),
							_T("%.3f"), total_time / 60.0 );
						tmp.ReleaseBuffer();
						pISXMLAttributes->Add (CComBSTR ("minutes"), CComBSTR (tmp));
					}
				}

				double fps = 0.0;
				hr = pDet->get_FrameRate (&fps);
				if ( SUCCEEDED( hr ) )
				{
					if ( fps != .0 )
					{
						StringCbPrintf( tmp.GetBuffer( 32 ), 32 * sizeof( TCHAR ),
							_T("%.2f"), fps );
						tmp.ReleaseBuffer();
						pISXMLAttributes->Add (CComBSTR ("frameRate"), CComBSTR (tmp));
					}
				}
				return S_OK;
			}
			else
				ATLTRACE( "Cannot get streams: 0x%08x\n", hr);
		}
		else
			ATLTRACE( "Cannot open file: 0x%08x\n", hr);
	}
	else
		ATLTRACE( "Cannot instantiate MediaDet object: 0x%08x\n", hr);

	return hr;
}