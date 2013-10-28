//
// RatDVDPlugin.h
//
//	Created by:		Rolandas Rudomanskis
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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

#pragma once

#include "RatDVDReader.h"

// CRatDVDPlugin

class ATL_NO_VTABLE CRatDVDPlugin :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CRatDVDPlugin, &CLSID_RatDVDReader>,
	public IImageServicePlugin,
	public ILibraryBuilderPlugin
{
public:
	CRatDVDPlugin();
	friend class CRatDVDClassFactory;

	~CRatDVDPlugin();

	DECLARE_REGISTRY_RESOURCEID(IDR_RATDVDPLUGIN)
	DECLARE_NOT_AGGREGATABLE(CRatDVDPlugin)

	BEGIN_COM_MAP(CRatDVDPlugin)
		COM_INTERFACE_ENTRY(IImageServicePlugin)
		COM_INTERFACE_ENTRY(ILibraryBuilderPlugin)
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:
	static LPCWSTR	uriVideo;

	// ILibraryBuilderPlugin Methods
public:
	STDMETHOD(Process)(BSTR sFile, ISXMLElement* pXML);

	// IImageServicePlugin Methods
public:
	STDMETHOD(LoadFromFile)(BSTR sFile, IMAGESERVICEDATA* pParams, SAFEARRAY** ppImage);
	STDMETHOD(LoadFromMemory)(BSTR sType, SAFEARRAY* pMemory,
		IMAGESERVICEDATA* pParams, SAFEARRAY** ppImage);
	STDMETHOD(SaveToFile)(BSTR sFile, IMAGESERVICEDATA* pParams, SAFEARRAY* pImage);
	STDMETHOD(SaveToMemory)(BSTR sType, SAFEARRAY** ppMemory,
		IMAGESERVICEDATA* pParams, SAFEARRAY* pImage);

private:
	STDMETHODIMP ProcessRatDVD(HANDLE hFile, ISXMLElement* pXML);
	STDMETHODIMP GetRatDVDThumbnail(BSTR bsFile, IMAGESERVICEDATA* pParams, SAFEARRAY** ppImage);
	CComBSTR ReadXML(HANDLE hFile, DWORD nBytes);
};

OBJECT_ENTRY_AUTO(__uuidof(RatDVDReader), CRatDVDPlugin)

const CLSID CLSID_ImageReader = { 0xFF5FCD00, 0x2C20, 0x49D8, { 0x84, 0xF6, 0x88, 0x8D, 0x2E, 0x2C, 0x95, 0xDA } };
