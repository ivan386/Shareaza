//
// IEProtocol.h
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

class CBuffer;
class CIEProtocolRequest;

class CZIPFile;

// {18D11ED9-1264-48A1-9E14-20F2C633242B}
class CIEProtocol : public CComObject
{
	DECLARE_DYNAMIC(CIEProtocol)

public:
	CIEProtocol();
	virtual ~CIEProtocol();

	BOOL		Create();
	void		Close();
	HRESULT		OnRequest(LPCTSTR pszURL, CBuffer& oBuffer, CString& sMimeType, BOOL bParseOnly);
	
protected:
	static LPCWSTR					pszProtocols[];
	CCriticalSection				m_pSection;
	CComQIPtr< IInternetSession >	m_pSession;

	// Loads file from zip-collection or simple collection itself
	// p2p-col:[//]{URN|SHA1}/{relative path inside zip}
	HRESULT		OnRequestCollection(LPCTSTR pszURL, CBuffer& oBuffer, CString& sMimeType, BOOL bParseOnly);

	// Loads preview (JPG), file metadata (XML) or icon
	// p2p-file:[//]{URN|SHA1}/{preview|meta|icon16|icon32|icon48}
	HRESULT		OnRequestFile(LPCTSTR pszURL, CBuffer& oBuffer, CString& sMimeType, BOOL bParseOnly);

	// Loads various application data
	// p2p-app:[//]{history}
	HRESULT		OnRequestApplication(LPCTSTR pszURL, CBuffer& oBuffer, CString& sMimeType, BOOL bParseOnly);

// COM
	BEGIN_INTERFACE_PART(ClassFactory, IClassFactory)
		STDMETHOD(CreateInstance)(IUnknown* pUnkOuter, REFIID riid, void** ppvObject);
		STDMETHOD(LockServer)(BOOL fLock);
	END_INTERFACE_PART(ClassFactory)

	DECLARE_OLECREATE(CIEProtocol)
	DECLARE_INTERFACE_MAP()
};


// {E1A67AE5-7041-4AE1-94F7-DE03EF759E27}
class CIEProtocolRequest : public CComObject
{
	DECLARE_DYNAMIC(CIEProtocolRequest)

public:
	CIEProtocolRequest();
	virtual ~CIEProtocolRequest();

protected:
	CComPtr<IInternetProtocolSink>	m_pSink;
	CString							m_strMimeType;	// Data MIME type
	CBuffer							m_oBuffer;		// Requested data

	HRESULT		OnStart(LPCTSTR pszURL, IInternetProtocolSink* pSink, IInternetBindInfo* pBindInfo, DWORD dwFlags);
	HRESULT		OnRead(void* pv, ULONG cb, ULONG* pcbRead);

// COM
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

	DECLARE_OLECREATE(CIEProtocolRequest)
	DECLARE_INTERFACE_MAP()

	friend class CIEProtocol;
};

extern CIEProtocol IEProtocol;
