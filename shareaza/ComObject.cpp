//
// ComObject.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "ComObject.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CComObject, CCmdTarget)

IMPLEMENT_OLETYPELIB(CComObject, LIBID_Shareaza, 1, 0)

/////////////////////////////////////////////////////////////////////////////
// CComObject construction

CComObject::CComObject() :
	m_pCLSID( NULL )
{
	EnableTypeLib();
}

CComObject::~CComObject()
{
}

/////////////////////////////////////////////////////////////////////////////
// CComObject operations

BOOL CComObject::EnableDispatch(REFIID pIID)
{
	LPUNKNOWN pUnknown = CCmdTarget::GetInterface( &pIID );

	if ( !pUnknown ) return FALSE;

	m_pDispatchMap.SetAt( pUnknown, &pIID );

	return TRUE;
}

LPUNKNOWN CComObject::GetInterface(REFIID pIID, BOOL bAddRef)
{
	LPUNKNOWN pInterface = CCmdTarget::GetInterface( &pIID );

	if ( !pInterface && pIID == IID_IDispatch )
	{
		if ( POSITION pos = m_pDispatchMap.GetStartPosition() )
		{
			const IID* pDispIID;
			m_pDispatchMap.GetNextAssoc( pos, pInterface, pDispIID );
		}
	}

	if ( pInterface && bAddRef ) pInterface->AddRef();
	return pInterface;
}

LPDISPATCH CComObject::GetDispatch(BOOL bAddRef)
{
	return (LPDISPATCH)CComObject::GetInterface( IID_IDispatch, bAddRef );
}

/////////////////////////////////////////////////////////////////////////////
// CComObject IUnknown implementation

STDMETHODIMP_(ULONG) CComObject::ComAddRef(LPUNKNOWN /*pUnk*/)
{
	return ExternalAddRef();
}

STDMETHODIMP_(ULONG) CComObject::ComRelease(LPUNKNOWN /*pUnk*/)
{
	return ExternalRelease();
}

STDMETHODIMP CComObject::ComQueryInterface(LPUNKNOWN pUnk, REFIID iid, LPVOID* ppvObj)
{
	if ( iid == IID_IDispatch )
	{
		const IID* pIID;
		if ( m_pDispatchMap.Lookup( pUnk, pIID ) )
		{
			*ppvObj = pUnk;
			ComAddRef( pUnk );
			return S_OK;
		}
	}

	return ExternalQueryInterface( &iid, ppvObj );
}

/////////////////////////////////////////////////////////////////////////////
// CComObject IDispatch implementation

STDMETHODIMP CComObject::ComGetTypeInfoCount(LPUNKNOWN /*pUnk*/, UINT FAR* pctinfo)
{
	if ( !pctinfo ) return E_INVALIDARG;
	*pctinfo = GetTypeInfoCount();
	return NOERROR;
}

STDMETHODIMP CComObject::ComGetTypeInfo(LPUNKNOWN pUnk, UINT itinfo, LCID lcid,
									 ITypeInfo FAR* FAR* pptinfo)
{
	if ( !pptinfo ) return E_INVALIDARG;
	if ( itinfo != 0 ) return DISP_E_BADINDEX;

	const IID* pIID;
	if ( !m_pDispatchMap.Lookup( pUnk, pIID ) ) return E_INVALIDARG;

	return GetTypeInfoOfGuid( lcid, *pIID, pptinfo );
}

STDMETHODIMP CComObject::ComGetIDsOfNames(	LPUNKNOWN pUnk, REFIID riid,
											OLECHAR FAR* FAR* rgszNames,
											UINT cNames, LCID lcid,
											DISPID FAR* rgdispid)
{
	if ( riid != IID_NULL ) return DISP_E_UNKNOWNINTERFACE;
	if ( !rgszNames || cNames < 1 ) return E_INVALIDARG;
	if ( !rgdispid ) return E_INVALIDARG;

	LPTYPEINFO pTypeInfo;
	SCODE sc;

	sc = ComGetTypeInfo( pUnk, 0, lcid, &pTypeInfo );

	if ( SUCCEEDED( sc ) )
	{
		sc = pTypeInfo->GetIDsOfNames( rgszNames, cNames, rgdispid );
		if ( sc == TYPE_E_ELEMENTNOTFOUND ) sc = DISP_E_UNKNOWNNAME;
		pTypeInfo->Release();
	}

	return sc;
}

STDMETHODIMP CComObject::ComInvoke(	LPUNKNOWN pUnk, DISPID dispidMember, REFIID riid,
									LCID lcid, WORD wFlags, DISPPARAMS FAR* pdispparams,
									VARIANT FAR* pvarResult, EXCEPINFO FAR* pexcepinfo,
									UINT FAR* puArgErr)
{
	if ( pdispparams == NULL ) return E_INVALIDARG;
	if ( riid != IID_NULL ) return DISP_E_UNKNOWNINTERFACE;

	if ( !IsInvokeAllowed( dispidMember ) ) return E_UNEXPECTED;

	LPTYPEINFO pTypeInfo;
	HRESULT hr;
	SCODE sc;

	sc = ComGetTypeInfo( pUnk, 0, lcid, &pTypeInfo );
	if ( FAILED( sc ) ) return sc;

	hr = DispInvoke(	pUnk, pTypeInfo, dispidMember, wFlags, pdispparams,
						pvarResult, pexcepinfo, puArgErr );

	pTypeInfo->Release();

	return hr;
}
