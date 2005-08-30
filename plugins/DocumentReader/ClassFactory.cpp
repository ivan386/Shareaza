
#pragma once
#include "stdafx.h"
#include "Globals.h"
#include "DocReader.h"
#include "shareaza_i.c"

////////////////////////////////////////////////////////////////////////
// CDocumentClassFactory - IClassFactory Implementation
//
//  This is a fairly simple CF. We don't provide support for licensing
//  in this sample, nor aggregation. We just create and return a new 
//  CDocReader object.
//

////////////////////////////////////////////////////////////////////////
// QueryInterface
//

STDMETHODIMP CDocumentClassFactory::QueryInterface(REFIID riid, void** ppv)
{
	ODS("CDocumentClassFactory::QueryInterface\n");
	CHECK_NULL_RETURN(ppv, E_POINTER);
	
	if ( CLSID_DocReader == riid )
	{
		*ppv = (ILibraryBuilderPlugin*)this;
		this->AddRef();
		return S_OK;
	}
	*ppv = NULL;
	return E_NOINTERFACE;
}

////////////////////////////////////////////////////////////////////////
// AddRef
//
STDMETHODIMP_(ULONG) CDocumentClassFactory::AddRef(void)
{
	TRACE1("CDocumentClassFactory::AddRef - %d\n", m_cRef + 1);
    return ++m_cRef;
}

////////////////////////////////////////////////////////////////////////
// Release
//
STDMETHODIMP_(ULONG) CDocumentClassFactory::Release(void)
{
	TRACE1("CDocumentClassFactory::Release - %d\n", m_cRef - 1);
    if ( 0 != --m_cRef ) return m_cRef;

	ODS("CDocumentClassFactory delete\n");
    LockServer(FALSE);
    return 0;
}

////////////////////////////////////////////////////////////////////////
// IClassFactory
//
////////////////////////////////////////////////////////////////////////
// CreateInstance
//
STDMETHODIMP CDocumentClassFactory::CreateInstance(LPUNKNOWN punk, REFIID riid, void** ppv)
{
	HRESULT hr;
	CComObject<CDocReader>*pDocReader = NULL;

	ODS("CFileClassFactory::CreateInstance\n");
	CHECK_NULL_RETURN(ppv, E_POINTER);	*ppv = NULL;

 // This version does not support Aggregation...
	if (punk) return CLASS_E_NOAGGREGATION;

	if ( IID_ILibraryBuilderPlugin == riid || IID_IImageServicePlugin == riid )
	{
		//CComObject< CDocReader >::CreateInstance(&pDocReader);
		CComObject<CDocReader>*pDocReader = new CComObject<CDocReader>;

		CHECK_NULL_RETURN(pDocReader, E_OUTOFMEMORY);
		hr = pDocReader->QueryInterface( riid, ppv );
		if ( SUCCEEDED(hr) )
		{
			*ppv = pDocReader;
		}
		else return hr;
	}
	LockServer(TRUE); // on success, bump up the lock count

	return hr;
}

////////////////////////////////////////////////////////////////////////
// LockServer
//
STDMETHODIMP CDocumentClassFactory::LockServer(BOOL fLock)
{
	TRACE1("CDocumentClassFactory::LockServer - %d\n", fLock);
	if (fLock) DllAddRef();	else DllRelease();
	return S_OK;
}

