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

//
// libjpeg
//

#undef FAR
#define FILE void
#include <jpeglib.h>


class ATL_NO_VTABLE CJPEGReader : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CJPEGReader, &CLSID_JPEGReader>,
	public IImageServicePlugin
{
public:
	CJPEGReader();
	virtual ~CJPEGReader();

DECLARE_REGISTRY_RESOURCEID(IDR_JPEGREADER)

BEGIN_COM_MAP(CJPEGReader)
	COM_INTERFACE_ENTRY(IImageServicePlugin)
END_COM_MAP()

// IImageServicePlugin
public:
    virtual HRESULT STDMETHODCALLTYPE LoadFromFile( 
        /* [in] */ HANDLE hFile,
        /* [in] */ DWORD nLength,
        /* [out][in] */ IMAGESERVICEDATA __RPC_FAR *pParams,
        /* [out] */ SAFEARRAY __RPC_FAR *__RPC_FAR *ppImage);
    
    virtual HRESULT STDMETHODCALLTYPE LoadFromMemory( 
        /* [in] */ SAFEARRAY __RPC_FAR *pMemory,
        /* [out][in] */ IMAGESERVICEDATA __RPC_FAR *pParams,
        /* [out] */ SAFEARRAY __RPC_FAR *__RPC_FAR *ppImage);

    virtual HRESULT STDMETHODCALLTYPE SaveToFile( 
        /* [in] */ HANDLE hFile,
        /* [out][in] */ IMAGESERVICEDATA __RPC_FAR *pParams,
        /* [in] */ SAFEARRAY __RPC_FAR *pImage);
    
    virtual HRESULT STDMETHODCALLTYPE SaveToMemory( 
        /* [out] */ SAFEARRAY __RPC_FAR *__RPC_FAR *ppMemory,
        /* [out][in] */ IMAGESERVICEDATA __RPC_FAR *pParams,
        /* [in] */ SAFEARRAY __RPC_FAR *pImage);

// Structures
protected:
	struct core_error_mgr
	{
		struct jpeg_error_mgr base;
		jmp_buf jump;
		BOOL bPartial;
	};

	struct core_source_mgr
	{
		jpeg_source_mgr base;
		HANDLE	hFile;
		DWORD	dwOffset;
		DWORD	dwLength;
		LPBYTE	pBuffer;
		DWORD	dwBuffer;
	};

	struct core_dest_mgr
	{
		jpeg_destination_mgr base;
		HANDLE	hFile;
		LPBYTE	pBuffer;
		DWORD	dwBuffer;
		DWORD	dwLength;
	};
	
// File Helpers
protected:
	METHODDEF(boolean) OnFileFill(j_decompress_ptr cinfo);
	METHODDEF(void) OnFileSkip(j_decompress_ptr cinfo, long num_bytes);

// Memory Helpers
protected:
	METHODDEF(boolean) OnMemFill(j_decompress_ptr cinfo);
	METHODDEF(void) OnMemSkip(j_decompress_ptr cinfo, long num_bytes);
	METHODDEF(boolean) OnMemEmpty(j_compress_ptr cinfo);

// Generic Helpers
protected:
	METHODDEF(void) OnEmitMessage(j_common_ptr cinfo, int msg_level);
	METHODDEF(void) OnErrorExit(j_common_ptr cinfo);
	METHODDEF(void) OnNull(j_compress_ptr cinfo);
	METHODDEF(void) OnNull(j_decompress_ptr cinfo);
};

#endif //__JPEGREADER_H_
