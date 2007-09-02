//
// ComObject.h
//
// Copyright (c) Shareaza Development Team, 2002-2006.
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

#if !defined(AFX_COMOBJECT_H__FC590F3A_2FC9_4CF2_A1CB_9F8B9C89CA4F__INCLUDED_)
#define AFX_COMOBJECT_H__FC590F3A_2FC9_4CF2_A1CB_9F8B9C89CA4F__INCLUDED_

#pragma once

class CComObject : public CCmdTarget
{
public:
	CComObject();
	virtual ~CComObject();

	DECLARE_DYNCREATE(CComObject)

	BOOL		EnableDispatch(REFIID pIID);
	LPUNKNOWN	GetInterface(REFIID pIID, BOOL bAddRef = FALSE);
	LPDISPATCH	GetDispatch(BOOL bAddRef = FALSE);

protected:
	const CLSID*	m_pCLSID;
	CMap< LPUNKNOWN, LPUNKNOWN, const IID*, const IID* > m_pDispatchMap;

	STDMETHOD_(ULONG, ComAddRef)(LPUNKNOWN);
	STDMETHOD_(ULONG, ComRelease)(LPUNKNOWN);
	STDMETHOD(ComQueryInterface)(LPUNKNOWN, REFIID, LPVOID*);
	STDMETHOD(ComGetTypeInfoCount)(LPUNKNOWN, UINT FAR*);
	STDMETHOD(ComGetTypeInfo)(LPUNKNOWN, UINT, LCID, LPTYPEINFO FAR*);
	STDMETHOD(ComGetIDsOfNames)(LPUNKNOWN, REFIID, OLECHAR FAR* FAR*, UINT, LCID lcid, DISPID FAR*);
	STDMETHOD(ComInvoke)(LPUNKNOWN, DISPID, REFIID, LCID, WORD, DISPPARAMS FAR*, VARIANT FAR*, EXCEPINFO FAR*, UINT FAR*);

	DECLARE_OLETYPELIB(CComObject)

private:
	CComObject(const CComObject&);
	CComObject& operator=(const CComObject&);
};

#define DECLARE_DISPATCH() \
    STDMETHOD(GetTypeInfoCount)(UINT FAR*); \
    STDMETHOD(GetTypeInfo)(UINT, LCID, LPTYPEINFO FAR*); \
    STDMETHOD(GetIDsOfNames)(REFIID, OLECHAR FAR* FAR*, UINT, LCID lcid, DISPID FAR*); \
    STDMETHOD(Invoke)(DISPID, REFIID, LCID, WORD, DISPPARAMS FAR*, VARIANT FAR*, EXCEPINFO FAR*, UINT FAR*);

#define IMPLEMENT_UNKNOWN(theClass, localClass)					\
	STDMETHODIMP_(ULONG) theClass##::X##localClass##::AddRef()	\
	{															\
	   METHOD_PROLOGUE(theClass, localClass)					\
	   return pThis->ExternalAddRef();							\
	}															\
	STDMETHODIMP_(ULONG) theClass##::X##localClass##::Release()	\
	{															\
	   METHOD_PROLOGUE(theClass, localClass)					\
	   return pThis->ExternalRelease();							\
	}															\
	STDMETHODIMP theClass##::X##localClass##::QueryInterface(REFIID iid, LPVOID* ppvObj)	\
	{															\
	   METHOD_PROLOGUE(theClass, localClass)					\
	   return pThis->ExternalQueryInterface( &iid, ppvObj );	\
	}

#define IMPLEMENT_DISPATCH_UNKNOWN(theClass, localClass)		\
	STDMETHODIMP_(ULONG) theClass##::X##localClass##::AddRef()	\
	{															\
	   METHOD_PROLOGUE(theClass, localClass)					\
	   return pThis->ComAddRef( this );							\
	}															\
	STDMETHODIMP_(ULONG) theClass##::X##localClass##::Release()	\
	{															\
	   METHOD_PROLOGUE(theClass, localClass)					\
	   return pThis->ComRelease( this );						\
	}															\
	STDMETHODIMP theClass##::X##localClass##::QueryInterface(REFIID iid, LPVOID* ppvObj)	\
	{															\
	   METHOD_PROLOGUE(theClass, localClass)					\
	   return pThis->ComQueryInterface( this, iid, ppvObj );	\
	}

#define IMPLEMENT_DISPATCH_DISPATCH(theClass, localClass) \
	STDMETHODIMP theClass##::X##localClass##::GetTypeInfoCount(UINT FAR* pctinfo) \
	{ \
		METHOD_PROLOGUE(theClass, localClass) \
		return pThis->ComGetTypeInfoCount( this, pctinfo ); \
	} \
	STDMETHODIMP theClass##::X##localClass##::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo) \
	{ \
		METHOD_PROLOGUE(theClass, localClass) \
		return pThis->ComGetTypeInfo( this, itinfo, lcid, pptinfo ); \
	} \
	STDMETHODIMP theClass##::X##localClass##::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, UINT cNames, LCID lcid, DISPID FAR* rgdispid) \
	{ \
		METHOD_PROLOGUE(theClass, localClass) \
		return pThis->ComGetIDsOfNames( this, riid, rgszNames, cNames, lcid, rgdispid ); \
	} \
	STDMETHODIMP theClass##::X##localClass##::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR* pdispparams, VARIANT FAR* pvarResult, EXCEPINFO FAR* pexcepinfo, UINT FAR* puArgErr) \
	{ \
		METHOD_PROLOGUE(theClass, localClass) \
		return pThis->ComInvoke( this, dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr ); \
	} \

#define IMPLEMENT_DISPATCH(theClass, localClass) \
	IMPLEMENT_DISPATCH_UNKNOWN(theClass, localClass) \
	IMPLEMENT_DISPATCH_DISPATCH(theClass, localClass)

#define INTERFACE_TO_CLASS(icClass, icInterface, icIn, icOut)	\
	icClass * icOut = (icClass *)( (BYTE*) icIn - offsetof( icClass, m_x##icInterface ) );

#endif // !defined(AFX_COMOBJECT_H__FC590F3A_2FC9_4CF2_A1CB_9F8B9C89CA4F__INCLUDED_)
