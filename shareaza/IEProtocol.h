//
// IEProtocol.h
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

#pragma once

#include "Hashes.h"

class CBuffer;
class CIEProtocolRequest;

class CZIPFile;


class CIEProtocol : public CCmdTarget
{
// Construction
public:
	CIEProtocol();
	virtual ~CIEProtocol();
	
	DECLARE_DYNAMIC(CIEProtocol)

// Operations
public:
	BOOL		Create();
	void		Close();
	BOOL		SetCollection(const CHashSHA1 &oSHA1, const LPCTSTR pszPath, CString &sIndex);
	void		SetEmptyCollection();
	
// Attributes
protected:
	CCriticalSection			m_pSection;
	CComPtr<IInternetSession>	m_pSession;
	CEvent*						m_pShutdown;
	LONG						m_nRequests;
protected:
	CHashSHA1					m_oCollSHA1;
	CZIPFile*					m_pCollZIP;
public:
	static CLSID				clsidProtocol;
	static LPCWSTR				pszProtocols[];
	
// Implementation
protected:
	CIEProtocolRequest*	CreateRequest();
	void				OnRequestConstruct(CIEProtocolRequest* pRequest);
	void				OnRequestDestruct(CIEProtocolRequest* pRequest);
	HRESULT				OnRequest(LPCTSTR pszURL, CBuffer* pBuffer, CString* psMimeType, BOOL bParseOnly);
	HRESULT				OnRequestRAZACOL(LPCTSTR pszURL, CBuffer* pBuffer, CString* psMimeType, BOOL bParseOnly);
	
// COM
protected:
	BEGIN_INTERFACE_PART(ClassFactory, IClassFactory)
		STDMETHOD(CreateInstance)(IUnknown* pUnkOuter, REFIID riid, void** ppvObject);
		STDMETHOD(LockServer)(BOOL fLock);
	END_INTERFACE_PART(ClassFactory)
	
	DECLARE_INTERFACE_MAP()
	
	friend class CIEProtocolRequest;
};


class CIEProtocolRequest : public CCmdTarget
{
// Construction
protected:
	CIEProtocolRequest(CIEProtocol* pProtocol);
	virtual ~CIEProtocolRequest();
	
	DECLARE_DYNAMIC(CIEProtocolRequest)
	
// Attributes
protected:
	CCriticalSection				m_pSection;
	CIEProtocol*					m_pProtocol;
	CComPtr<IInternetProtocolSink>	m_pSink;
	CBuffer*						m_pBuffer;
	
// Implementation
protected:
	HRESULT		OnStart(LPCTSTR pszURL, IInternetProtocolSink* pSink, IInternetBindInfo* pBindInfo, DWORD dwFlags);
	HRESULT		OnRead(void* pv, ULONG cb, ULONG* pcbRead);
	HRESULT		OnTerminate();
	
// COM
protected:
	BEGIN_INTERFACE_PART(InternetProtocol, IInternetProtocol)
		STDMETHOD(Abort)(HRESULT hrReason, DWORD dwOptions);
		STDMETHOD(Continue)(PROTOCOLDATA *pProtocolData);
		STDMETHOD(Resume)();
		STDMETHOD(Start)(LPCWSTR szUrl, IInternetProtocolSink *pOIProtSink, IInternetBindInfo *pOIBindInfo, DWORD grfPI, HANDLE_PTR dwReserved);
		STDMETHOD(Suspend)();
		STDMETHOD(Terminate)(DWORD dwOptions);
		STDMETHOD(LockRequest)(DWORD dwOptions);
		STDMETHOD(Read)(void *pv, ULONG cb, ULONG *pcbRead);
		STDMETHOD(Seek)(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
		STDMETHOD(UnlockRequest)();
	END_INTERFACE_PART(InternetProtocol)

	BEGIN_INTERFACE_PART(InternetProtocolInfo , IInternetProtocolInfo )
		STDMETHOD(CombineUrl)(LPCWSTR pwzBaseUrl, LPCWSTR pwzRelativeUrl, DWORD dwCombineFlags, LPWSTR pwzResult, DWORD cchResult, DWORD *pcchResult, DWORD dwReserved);
		STDMETHOD(CompareUrl)(LPCWSTR pwzUrl1, LPCWSTR pwzUrl2, DWORD dwCompareFlags);
		STDMETHOD(ParseUrl)(LPCWSTR pwzUrl, PARSEACTION ParseAction, DWORD dwParseFlags, LPWSTR pwzResult, DWORD cchResult, DWORD *pcchResult, DWORD dwReserved);
		STDMETHOD(QueryInfo)(LPCWSTR pwzUrl, QUERYOPTION OueryOption, DWORD dwQueryFlags, LPVOID pBuffer, DWORD cbBuffer, DWORD *pcbBuf, DWORD dwReserved);
	END_INTERFACE_PART(InternetProtocolInfo )

	DECLARE_INTERFACE_MAP()
	
	friend class CIEProtocol;
};

extern CIEProtocol IEProtocol;
