// Object.h : Declaration of the CRazaWebHook

#pragma once

#include "resource.h"
#include "RazaWebHook_i.h"

// CRazaWebHook

class ATL_NO_VTABLE CRazaWebHook :
	public CComObjectRootEx< CComMultiThreadModel >,
	public CComCoClass< CRazaWebHook, &CLSID_RazaWebHook >,
	public IDispatchImpl< IRazaWebHook, &IID_IRazaWebHook, &LIBID_RazaWebHookLib, /*wMajor =*/ 1, /*wMinor =*/ 0 >,
	public IObjectWithSiteImpl< CRazaWebHook >,
	public IConnectionPointContainerImpl< CRazaWebHook >
{
public:
	CRazaWebHook();

DECLARE_REGISTRY_RESOURCEID(IDR_OBJECT)

BEGIN_COM_MAP(CRazaWebHook)
	COM_INTERFACE_ENTRY(IRazaWebHook)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
	COM_INTERFACE_ENTRY(IObjectWithSite)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CRazaWebHook)
	IMPLEMENTED_CATEGORY(CATID_SafeForScripting)
	IMPLEMENTED_CATEGORY(CATID_SafeForInitializing)
END_CATEGORY_MAP()

BEGIN_CONNECTION_POINT_MAP(CRazaWebHook)
END_CONNECTION_POINT_MAP()

DECLARE_PROTECT_FINAL_CONSTRUCT()
DECLARE_GET_CONTROLLING_UNKNOWN()

	HRESULT FinalConstruct()
	{
		return CoCreateFreeThreadedMarshaler(
			GetControllingUnknown(), &m_pUnkMarshaler.p);
	}

	void FinalRelease()
	{
		m_pUnkMarshaler.Release();
	}

	CComPtr<IUnknown> m_pUnkMarshaler;

// IObjectWithSite
	STDMETHOD(SetSite)(
		/* [in] */ IUnknown* pUnkSite);

// IRazaWebHook
	STDMETHOD(AddLink)(
		/* [in] */ VARIANT oLink);
};

OBJECT_ENTRY_AUTO(__uuidof(RazaWebHook), CRazaWebHook)
