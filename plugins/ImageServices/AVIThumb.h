//
// AVIThumb.h
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

#ifndef __AVITHUMB_H_
#define __AVITHUMB_H_

#include "Resource.h"

class ATL_NO_VTABLE CAVIThumb : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CAVIThumb, &CLSID_AVIThumb>,
	public IImageServicePlugin
{
public:
	CAVIThumb();
	virtual ~CAVIThumb();

DECLARE_REGISTRY_RESOURCEID(IDR_AVITHUMB)

BEGIN_COM_MAP(CAVIThumb)
	COM_INTERFACE_ENTRY(IImageServicePlugin)
END_COM_MAP()

// Attributes
protected:
	TCHAR	m_szTemp[128];
	HANDLE	m_hTempFile;
	HANDLE	m_hTempMap;
	LPVOID	m_pTempMap;
protected:
	static LONG				m_nInstances;
	static CRITICAL_SECTION	m_pSection;

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

protected:
	HRESULT LoadImpl(LPCTSTR pszFile, IMAGESERVICEDATA __RPC_FAR *pParams, SAFEARRAY __RPC_FAR *__RPC_FAR *ppImage);

};

#endif //__AVITHUMB_H_
