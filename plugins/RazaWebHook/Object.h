//
// Object.h : Declaration of the CRazaWebHook
//
// Copyright (c) Shareaza Development Team, 2008-2010.
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
#include "RazaWebHook_i.h"

// CRazaWebHook

typedef IDispatchImpl< IRazaWebHook, &IID_IRazaWebHook, &LIBID_RazaWebHookLib, /*wMajor =*/ 1, /*wMinor =*/ 0 > IRazaWebHookDispatchImpl;

class ATL_NO_VTABLE CRazaWebHook :
	public CComObjectRootEx< CComMultiThreadModel >,
	public CComCoClass< CRazaWebHook, &CLSID_RazaWebHook >,
	public IRazaWebHookDispatchImpl,
	public IObjectWithSiteImpl< CRazaWebHook >
{
public:
	CRazaWebHook();

DECLARE_REGISTRY_RESOURCEID(IDR_OBJECT)

BEGIN_COM_MAP(CRazaWebHook)
	COM_INTERFACE_ENTRY(IRazaWebHook)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_IID(DIID_DWebBrowserEvents2, IDispatch)
	COM_INTERFACE_ENTRY(IObjectWithSite)
END_COM_MAP()

DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct();
	void FinalRelease();

 protected:
	DWORD					m_dwCookie;
	CString					m_sURL;

	bool IsEnabled() const;
	bool IsHooked(const CString& sExt) const;
	void Connect();
	void Disconnect();
	void AddLink(const CString& sURL);

// IDispatchImpl
	STDMETHOD(Invoke)(
		/* [in] */ DISPID dispIdMember,
		/* [in] */ REFIID riid,
		/* [in] */ LCID lcid,
		/* [in] */ WORD wFlags,
		/* [out][in] */ DISPPARAMS *pDispParams,
		/* [out] */ VARIANT *pVarResult,
		/* [out] */ EXCEPINFO *pExcepInfo,
		/* [out] */ UINT *puArgErr);

// IObjectWithSite
	STDMETHOD(SetSite)(
		/* [in] */ IUnknown* pUnkSite);

// IRazaWebHook
	STDMETHOD(AddLink)(
		/* [in] */ VARIANT oLink);
};

OBJECT_ENTRY_AUTO(__uuidof(RazaWebHook), CRazaWebHook)
