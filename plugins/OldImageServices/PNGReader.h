//
// PNGReader.h
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

#ifndef __PNGREADER_H_
#define __PNGREADER_H_

//
// libpng
//

#include "pngusr.h"
#include <png.h>

class ATL_NO_VTABLE CPNGReader : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CPNGReader, &CLSID_PNGReader>,
	public IImageServicePlugin
{
public:
DECLARE_REGISTRY_RESOURCEID(IDR_PNGREADER)

BEGIN_COM_MAP(CPNGReader)
	COM_INTERFACE_ENTRY(IImageServicePlugin)
END_COM_MAP()

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
		/* [in] */ SAFEARRAY* pImage)
	{ 	ATLTRACENOTIMPL ("SaveToFile"); }
	STDMETHOD(SaveToMemory)(
		/* [in] */ BSTR sType,
		/* [out] */ SAFEARRAY** ppMemory,
		/* [in,out] */ IMAGESERVICEDATA* pParams,
		/* [in] */ SAFEARRAY* pImage)
	{ ATLTRACENOTIMPL ("SaveToMemory"); }

// Handlers
private:
	static void OnFileRead(png_structp png_ptr, png_bytep data, png_size_t length);
	static void OnMemRead(png_structp png_ptr, png_bytep data, png_size_t length);

	struct FileReadStruct
	{
		HANDLE	hFile;
		DWORD	dwLength;
	};

	struct MemReadStruct
	{
		BYTE*	pBuffer;
		DWORD	dwLength;
	};
};

#endif //__PNGREADER_H_
