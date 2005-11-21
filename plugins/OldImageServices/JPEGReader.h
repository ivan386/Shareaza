//
// JPEGReader.h
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

#ifndef __JPEGREADER_H_
#define __JPEGREADER_H_

#include <exception>

//
// libjpeg
//

#include <jpeglib.h> 

class ATL_NO_VTABLE CJPEGReader : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CJPEGReader, &CLSID_JPEGReader>,
	public IImageServicePlugin
{
public:

DECLARE_REGISTRY_RESOURCEID(IDR_JPEGREADER)

BEGIN_COM_MAP(CJPEGReader)
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
	{ ATLTRACENOTIMPL ("SaveToFile"); }
	STDMETHOD(SaveToMemory)(
		/* [in] */ BSTR sType,
		/* [out] */ SAFEARRAY** ppMemory,
		/* [in,out] */ IMAGESERVICEDATA* pParams,
		/* [in] */ SAFEARRAY* pImage);

private:
// File Helpers
	static boolean OnFileFill(j_decompress_ptr cinfo);
	static void OnFileSkip(j_decompress_ptr cinfo, long num_bytes);

// Memory Helpers
	static boolean OnMemFill(j_decompress_ptr cinfo);
	static void    OnMemSkip(j_decompress_ptr cinfo, long num_bytes);
	static boolean OnMemEmpty(j_compress_ptr cinfo);

	struct core_error_mgr : public jpeg_error_mgr
	{
		bool bPartial;
	};

	struct core_source_mgr : public jpeg_source_mgr
	{
		HANDLE	hFile;
		DWORD	dwOffset;
		DWORD	dwLength;
		boost::scoped_array< BYTE >	pBuffer;
		DWORD	dwBuffer;
	};

	struct core_dest_mgr : public jpeg_destination_mgr
	{
		HANDLE	hFile;
		boost::scoped_array< BYTE >	pBuffer;
		DWORD	dwBuffer;
		DWORD	dwLength;
	};
	struct JPEGException : public std::exception {};
// Generic Helpers
	static void OnEmitMessage(j_common_ptr cinfo, int msg_level)
	{
		if ( msg_level < 1 )
			reinterpret_cast< core_error_mgr* >( cinfo )->bPartial = true;
	}
	static void OnErrorExit(j_common_ptr cinfo)
	{
		throw JPEGException();
	}
	static void OnNull(j_compress_ptr cinfo) {}
	static void OnNull(j_decompress_ptr cinfo) {}
};

#endif //__JPEGREADER_H_
