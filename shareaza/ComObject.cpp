//
// ComObject.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "ComObject.h"
#include "ShareazaOM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CComObject, CCmdTarget)
IMPLEMENT_OLETYPELIB(CComObject, LIBID_Shareaza, 1, 0);


/////////////////////////////////////////////////////////////////////////////
// CComObject construction

CComObject::CComObject()
{
	EnableTypeLib();
	m_pCLSID = NULL;
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

	m_pDispatchMap.SetAt( pUnknown, (LPVOID)&pIID );

	return TRUE;
}

LPUNKNOWN CComObject::GetInterface(REFIID pIID, BOOL bAddRef)
{
	LPUNKNOWN pInterface = CCmdTarget::GetInterface( &pIID );

	if ( !pInterface && pIID == IID_IDispatch )
	{
		if ( POSITION pos = m_pDispatchMap.GetStartPosition() )
		{
			IID* pDispIID;
			m_pDispatchMap.GetNextAssoc( pos, (void*&)pInterface, (void*&)pDispIID );
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

STDMETHODIMP_(ULONG) CComObject::ComAddRef(LPUNKNOWN pUnk)
{
	return ExternalAddRef();
}

STDMETHODIMP_(ULONG) CComObject::ComRelease(LPUNKNOWN pUnk)
{
	return ExternalRelease();
}

STDMETHODIMP CComObject::ComQueryInterface(LPUNKNOWN pUnk, REFIID iid, LPVOID* ppvObj)
{
	if ( iid == IID_IDispatch )
	{
		IID* pIID;
		if ( m_pDispatchMap.Lookup( pUnk, (void*&)pIID ) )
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

STDMETHODIMP CComObject::ComGetTypeInfoCount(LPUNKNOWN pUnk, UINT FAR* pctinfo)
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

	IID* pIID;
	if ( !m_pDispatchMap.Lookup( pUnk, (void*&)pIID ) ) return E_INVALIDARG;

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

/////////////////////////////////////////////////////////////////////////////
// GUID tools

CString GUIDX::Encode(const void * pInGUID)
{
	const GUID* pGUID = reinterpret_cast<const GUID*>(pInGUID);
	CString str;
	str.Format( _T("{%.8X-%.4X-%.4X-%.2X%.2X-%.2X%.2X%.2X%.2X%.2X%.2X}"),
		pGUID->Data1, pGUID->Data2, pGUID->Data3,
		pGUID->Data4[0], pGUID->Data4[1], pGUID->Data4[2], pGUID->Data4[3],
		pGUID->Data4[4], pGUID->Data4[5], pGUID->Data4[6], pGUID->Data4[7] );
	return str;
}

bool GUIDX::Decode(LPCTSTR pszIn, LPVOID pOutGUID)
{
	ASSERT( pOutGUID != NULL );
	
	if ( pszIn == NULL ) return false;
	
	if ( _tcslen(pszIn) == 38 )
	{
		if ( pszIn[0] != '{' || pszIn[37] != '}' ) return false;
	}
	else if ( _tcslen(pszIn) == 36 )
	{
		pszIn --;
	}
	else return false;
	
	BYTE* pGUID = reinterpret_cast<BYTE*>(pOutGUID);
	
	if ( ! Unhex( pszIn + 1, pGUID + 3 ) ) return false;
	if ( ! Unhex( pszIn + 3, pGUID + 2 ) ) return false;
	if ( ! Unhex( pszIn + 5, pGUID + 1 ) ) return false;
	if ( ! Unhex( pszIn + 7, pGUID + 0 ) ) return false;
	if ( ! Unhex( pszIn + 10, pGUID + 5 ) ) return false;
	if ( ! Unhex( pszIn + 12, pGUID + 4 ) ) return false;
	if ( ! Unhex( pszIn + 15, pGUID + 7 ) ) return false;
	if ( ! Unhex( pszIn + 17, pGUID + 6 ) ) return false;
	if ( ! Unhex( pszIn + 20, pGUID + 8 ) ) return false;
	if ( ! Unhex( pszIn + 22, pGUID + 9 ) ) return false;
	if ( ! Unhex( pszIn + 25, pGUID + 10 ) ) return false;
	if ( ! Unhex( pszIn + 27, pGUID + 11 ) ) return false;
	if ( ! Unhex( pszIn + 29, pGUID + 12 ) ) return false;
	if ( ! Unhex( pszIn + 31, pGUID + 13 ) ) return false;
	if ( ! Unhex( pszIn + 33, pGUID + 14 ) ) return false;
	if ( ! Unhex( pszIn + 35, pGUID + 15 ) ) return false;
	
	return true;
}

bool GUIDX::Unhex(LPCTSTR psz, LPBYTE pOut)
{
	register TCHAR c = *psz++;
	if ( c >= '0' && c <= '9' )
		*pOut = ( c - '0' ) << 4;
	else if ( c >= 'A' && c <= 'F' )
		*pOut = ( c - 'A' + 10 ) << 4;
	else if ( c >= 'a' && c <= 'f' )
		*pOut = ( c - 'a' + 10 ) << 4;
	else
		return false;
	c = *psz;
	if ( c >= '0' && c <= '9' )
		*pOut |= ( c - '0' );
	else if ( c >= 'A' && c <= 'F' )
		*pOut |= ( c - 'A' + 10 );
	else if ( c >= 'a' && c <= 'f' )
		*pOut |= ( c - 'a' + 10 );
	else
		return false;
	return true;
}
