//
// Builder.cpp : Implementation of CBuilder
//
// Copyright (c) Nikolay Raspopov, 2005-2014.
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
	if (!pXML)
		return E_POINTER;

	CComPtr <ISXMLElements> pISXMLRootElements;
	HRESULT hr = pXML->get_Elements(&pISXMLRootElements);
	if (FAILED (hr))
		return hr;
	CComPtr <ISXMLElement> pXMLRootElement;
	hr = pISXMLRootElements->Create (CComBSTR ("images"), &pXMLRootElement);
	if (FAILED (hr))
		return hr;
	CComPtr <ISXMLAttributes> pISXMLRootAttributes;
	hr = pXMLRootElement->get_Attributes(&pISXMLRootAttributes);
	if (FAILED (hr))
		return hr;
	pISXMLRootAttributes->Add (CComBSTR ("xmlns:xsi"),
		CComBSTR ("http://www.w3.org/2001/XMLSchema-instance"));
	pISXMLRootAttributes->Add (CComBSTR ("xsi:noNamespaceSchemaLocation"),
		CComBSTR ("http://www.shareaza.com/schemas/image.xsd"));

	CComPtr <ISXMLElements> pISXMLElements;
	hr = pXMLRootElement->get_Elements(&pISXMLElements);
	if (FAILED (hr))
		return hr;
	CComPtr <ISXMLElement> pXMLElement;
	hr = pISXMLElements->Create (CComBSTR ("image"), &pXMLElement);
	if (FAILED (hr))
		return hr;
	CComPtr <ISXMLAttributes> pISXMLAttributes;
	hr = pXMLElement->get_Attributes(&pISXMLAttributes);
	if (FAILED (hr))
		return hr;

	GFL_FILE_INFORMATION inf = {};
	GFL_ERROR err = gflGetFileInformationW( (LPCWSTR)sFile, -1, &inf );
	if ( err != GFL_NO_ERROR )
	{
		WCHAR pszPath[ MAX_PATH * 2 ] = {};
		if ( GetShortPathNameW( (LPCWSTR)sFile, pszPath, MAX_PATH * 2 ) )
			err = gflGetFileInformationW( pszPath, -1, &inf );
		else err = GFL_ERROR_FILE_OPEN;
	}

	if ( err == GFL_NO_ERROR )
	{
		if ( inf.Height > 0 )
		{
			CString height;
			height.Format( _T("%d"), inf.Height );
			pISXMLAttributes->Add( CComBSTR( "height" ), CComBSTR( height ) );
		}

		if ( inf.Width > 0 )
		{
			CString width;
			width.Format( _T("%d"), inf.Width );
			pISXMLAttributes->Add( CComBSTR( "width" ), CComBSTR( width ) );
		}

		if ( *inf.Description )
		{
			pISXMLAttributes->Add( CComBSTR( "description" ), CComBSTR( inf.Description ) );
		}

		CString colors;
		GFL_UINT16 bits = inf.ComponentsPerPixel * inf.BitsPerComponent;
		if ( inf.ColorModel == GFL_CM_GREY )
			colors = _T("Greyscale");
		else if ( bits == 0 )
			; // No bits
		else if ( bits == 1 )
			colors = _T("2");
		else if ( bits == 2 )
			colors = _T("4");
		else if ( bits <= 4 )
			colors = _T("16");
		else if ( bits <= 8 )
			colors = _T("256");
		else if ( bits <= 16 )
			colors = _T("64K");
		else if ( bits <= 24 )
			colors = _T("16.7M");
		else
			colors = _T("16.7M + Alpha");
		if ( colors.GetLength() )
			pISXMLAttributes->Add( CComBSTR ("colors"), CComBSTR( colors ) );
	} else
		hr = E_FAIL;
	return hr;
}
