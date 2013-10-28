//
// ComObject.h
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

#pragma once


class CComObject : public CCmdTarget
{
	DECLARE_DYNAMIC(CComObject)

public:
	BOOL		EnableDispatch(REFIID pIID);
	LPUNKNOWN	GetInterface(REFIID pIID, BOOL bAddRef = FALSE);
	LPDISPATCH	GetDispatch(BOOL bAddRef = FALSE);

	STDMETHOD_(ULONG, ComAddRef)(LPUNKNOWN);
	STDMETHOD_(ULONG, ComRelease)(LPUNKNOWN);
	STDMETHOD(ComQueryInterface)(LPUNKNOWN, REFIID, LPVOID*);
	STDMETHOD(ComGetTypeInfoCount)(LPUNKNOWN, UINT FAR*);
	STDMETHOD(ComGetTypeInfo)(LPUNKNOWN, UINT, LCID, LPTYPEINFO FAR*);
	STDMETHOD(ComGetIDsOfNames)(LPUNKNOWN, REFIID, OLECHAR FAR* FAR*, UINT, LCID lcid, DISPID FAR*);
	STDMETHOD(ComInvoke)(LPUNKNOWN, DISPID, REFIID, LCID, WORD, DISPPARAMS FAR*, VARIANT FAR*, EXCEPINFO FAR*, UINT FAR*);

	DECLARE_OLETYPELIB(CComObject)

protected:
	CComObject();
	virtual ~CComObject();

	const CLSID*	m_pCLSID;
	CMap< LPUNKNOWN, LPUNKNOWN, const IID*, const IID* > m_pDispatchMap;

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


// To prevent direct method calling of object held by CComObjectPtr
template < class T >
class _StrictCComObjectPtr : public T
{
private:
	STDMETHOD_(ULONG, ComAddRef)(LPUNKNOWN) = 0;
	STDMETHOD_(ULONG, ComRelease)(LPUNKNOWN) = 0;
};


// Smart pointer for CComObject class (like CComPtr)
template < class T >
class CComObjectPtr
{
public:
	CComObjectPtr() throw()
		: p( NULL )
	{
	}

	CComObjectPtr(T* p2) throw()
		: p( p2 )
	{
		if ( p )
			p->ComAddRef( NULL );
	}

	CComObjectPtr(const CComObjectPtr& pObjectPtr) throw()
		: p( pObjectPtr.p )
	{
		if ( p )
			p->ComAddRef( NULL );
	}

	~CComObjectPtr() throw()
	{
		Release();
	}

	CComObjectPtr& operator=(const CComObjectPtr& pObjectPtr) throw()
	{
		if ( *this != pObjectPtr )
		{
			if ( pObjectPtr.p )
				pObjectPtr.p->ComAddRef( NULL );
			Attach( pObjectPtr.p );
		}
		return *this;
	}

	operator T*() const throw()
	{
		return p;
	}

	T& operator*() const throw()
	{
		ASSERT( p != NULL );
		return *p;
	}

	_StrictCComObjectPtr< T >* operator->() const throw()
	{
		ASSERT( p != NULL );
		return (_StrictCComObjectPtr< T >*)p;
	}

	operator bool() const throw()
	{
		return ( p != NULL );
	}

	bool operator!() const throw()
	{
		return ( p == NULL );
	}

	bool operator<(T* pT) const throw()
	{
		return ( p < pT );
	}

	bool operator!=(T* pT) const throw()
	{
		return ! operator==( pT );
	}

	bool operator==(_In_opt_ T* pT) const throw()
	{
		return ( p == pT );
	}

	// Attach to an existing interface (does not ComAddRef)
	void Attach(T* p2) throw()
	{
		Release();
		p = p2;
	}

	// Detach the interface (does not ComRelease)
	T* Detach() throw()
	{
		T* pTemp = p;
		p = NULL;
		return pTemp;
	}

	void Release() throw()
	{
		T* pTemp = p;
		if ( pTemp )
		{
			p = NULL;
			pTemp->ComRelease( NULL );
		}
	}

private:
	T* p;
};
