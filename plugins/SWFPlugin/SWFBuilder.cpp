//
// SWFBuilder.cpp : Implementation of CSWFBuilder
//
// Copyright (c) Nikolay Raspopov, 2005.
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
#include "SWFBuilder.h"

STDMETHODIMP CSWFBuilder::Process (
	/* [in] */ BSTR /* sFile */,
	/* [in] */ ISXMLElement* pXML)
{
	if (!pXML)
		return E_POINTER;

	CComPtr <ISXMLElements> pISXMLRootElements;
	HRESULT hr = pXML->get_Elements(&pISXMLRootElements);
	if (FAILED (hr))
		return hr;
	CComPtr <ISXMLElement> pXMLRootElement;
	hr = pISXMLRootElements->Create (CComBSTR ("videos"), &pXMLRootElement);
	if (FAILED (hr))
		return hr;
	CComPtr <ISXMLAttributes> pISXMLRootAttributes;
	hr = pXMLRootElement->get_Attributes(&pISXMLRootAttributes);
	if (FAILED (hr))
		return hr;
	pISXMLRootAttributes->Add (CComBSTR ("xmlns:xsi"),
		CComBSTR ("http://www.w3.org/2001/XMLSchema-instance"));
	pISXMLRootAttributes->Add (CComBSTR ("xsi:noNamespaceSchemaLocation"),
		CComBSTR ("http://www.limewire.com/schemas/video.xsd"));

	CComPtr <ISXMLElements> pISXMLElements;
	hr = pXMLRootElement->get_Elements(&pISXMLElements);
	if (FAILED (hr))
		return hr;
	CComPtr <ISXMLElement> pXMLElement;
	hr = pISXMLElements->Create (CComBSTR ("video"), &pXMLElement);
	if (FAILED (hr))
		return hr;
	CComPtr <ISXMLAttributes> pISXMLAttributes;
	hr = pXMLElement->get_Attributes(&pISXMLAttributes);
	if (FAILED (hr))
		return hr;
	pISXMLAttributes->Add (CComBSTR ("type"), CComBSTR ("Shockwave Flash"));
	pISXMLAttributes->Add (CComBSTR ("codec"), CComBSTR ("SWF"));
	CString tmp;
	tmp.Format (_T("%lu"), cx);
	pISXMLAttributes->Add (CComBSTR ("width"), CComBSTR (tmp));
	tmp.Format (_T("%lu"), cy);
	pISXMLAttributes->Add (CComBSTR ("height"), CComBSTR (tmp));
	return hr;
}
