//
// VideoReader.h : Declaration of the CVideoReader
//
// Copyright (c) Nikolay Raspopov, 2005-2009.
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

#include "resource.h"
#include "MediaImageServices.h"

class ATL_NO_VTABLE CVideoReader : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CVideoReader, &CLSID_VideoReader>,
	public IImageServicePlugin
{
public:
	CVideoReader () throw()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_VIDEOREADER)

BEGIN_COM_MAP(CVideoReader)
	COM_INTERFACE_ENTRY(IImageServicePlugin)
END_COM_MAP()

protected:
	void CopyBitmap(char* pDestination, const char* pSource,
		const int width, const int height, const int line_size);
	HRESULT LoadFrame(IMediaDet* pDet, double total_time,
		const IMAGESERVICEDATA* pParams, SAFEARRAY** ppImage);

// IImageServicePlugin
public:
	STDMETHOD(LoadFromFile)(
		/* [in] */ BSTR sFile,
		/* [in,out] */ IMAGESERVICEDATA* pParams,
		/* [out] */ SAFEARRAY** ppImage );
	STDMETHOD(LoadFromMemory)(
		/* [in] */ BSTR sType,
		/* [in] */ SAFEARRAY* pMemory,
		/* [in,out] */ IMAGESERVICEDATA* pParams,
		/* [out] */ SAFEARRAY** ppImage );
	STDMETHOD(SaveToFile)(
		/* [in] */ BSTR sFile,
		/* [in,out] */ IMAGESERVICEDATA* pParams,
		/* [in] */ SAFEARRAY* pImage);
	STDMETHOD(SaveToMemory)(
		/* [in] */ BSTR sType,
		/* [out] */ SAFEARRAY** ppMemory,
		/* [in,out] */ IMAGESERVICEDATA* pParams,
		/* [in] */ SAFEARRAY* pImage);
};

OBJECT_ENTRY_AUTO(__uuidof(VideoReader), CVideoReader)
