//
// PreviewPlugin.h : Declaration of the CPreviewPlugin
//
// Copyright (c) Nikolay Raspopov, 2009.
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

#include "resource.h"       // main symbols
#include "Preview.h"


// CPreviewPlugin

class ATL_NO_VTABLE CPreviewPlugin :
	public CComObjectRootEx< CComMultiThreadModel >,
	public CComCoClass< CPreviewPlugin, &CLSID_PreviewPlugin >,
	public IGeneralPlugin,
	public IDownloadPreviewPlugin2
{
public:
	CPreviewPlugin();

DECLARE_REGISTRY_RESOURCEID(IDR_PREVIEWPLUGIN)

BEGIN_COM_MAP(CPreviewPlugin)
	COM_INTERFACE_ENTRY(IGeneralPlugin)
	COM_INTERFACE_ENTRY(IDownloadPreviewPlugin2)
END_COM_MAP()

DECLARE_PROTECT_FINAL_CONSTRUCT()

public:
	HRESULT FinalConstruct();
	void FinalRelease();

protected:
	CComPtr<IDownloadPreviewSite>	m_pSite;
	bool							m_bCancel;		// Got cancel request
	CHandle							m_hProcess;		// External process handler

	bool Execute(LPCTSTR szCommand);

public:
// IGeneralPlugin
	STDMETHOD(SetApplication)(/* [in] */ IApplication* pApplication);
	STDMETHOD(QueryCapabilities)(/* [out] */ DWORD* pnCaps);
	STDMETHOD(Configure)(void);
	STDMETHOD(OnSkinChanged)(void);

// IDownloadPreviewPlugin
	STDMETHOD(SetSite)(/* [in] */ IDownloadPreviewSite* pSite);
	STDMETHOD(Preview)(/* [in] */ HANDLE hFile, /* [in */ BSTR sTarget);
	STDMETHOD(Cancel)();

// IDownloadPreviewPlugin2
	STDMETHOD(Preview2)(/* [in] */ BSTR sSource, /* [in] */ BSTR sTarget);
};

OBJECT_ENTRY_AUTO(__uuidof(PreviewPlugin), CPreviewPlugin)
