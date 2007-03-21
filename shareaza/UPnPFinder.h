//
// UPnPFinder.h
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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

#ifndef UPNPFINDER_H_INCLUDED
#define UPNPFINDER_H_INCLUDED

#pragma once
#pragma warning( disable: 4355 )

typedef com_ptr< IUPnPDeviceFinder > FinderPointer;
typedef com_ptr< IUPnPDevice > DevicePointer;
typedef com_ptr< IUPnPService > ServicePointer;

CString translateUPnPResult(HRESULT hr);
HRESULT UPnPMessage(HRESULT hr);

class CUPnPFinder
{
// Exceptions
public:
	struct UPnPError : std::exception {};
// Construction
public:
	CUPnPFinder();
	~CUPnPFinder();

	void StartDiscovery();
	void StopAsyncFind();
	void DeletePorts();
	bool AreServicesHealthy();
	void AddDevice(DevicePointer pDevice);
	void RemoveDevice(CComBSTR bsUDN);
	void OnSearchComplete();
	inline bool IsAsyncFindRunning() 
	{
		if ( m_pDeviceFinder && m_bAsyncFindRunning && GetTickCount() - m_tLastEvent > 10000 )
		{
			m_pDeviceFinder->CancelAsyncFind( m_nAsyncFindHandle );
			m_bAsyncFindRunning = false;
		}
		MSG msg;
		while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		return m_bAsyncFindRunning;
	}

	// API functions
	SC_HANDLE (WINAPI *m_pfnOpenSCManager)(LPCTSTR, LPCTSTR, DWORD);
	SC_HANDLE (WINAPI *m_pfnOpenService)(SC_HANDLE, LPCTSTR, DWORD);
	BOOL (WINAPI *m_pfnQueryServiceStatusEx)(SC_HANDLE, SC_STATUS_TYPE, LPBYTE, DWORD, LPDWORD);
	BOOL (WINAPI *m_pfnCloseServiceHandle)(SC_HANDLE);
	BOOL (WINAPI *m_pfnStartService)(SC_HANDLE, DWORD, LPCTSTR*);

// Implementation
private:
	static FinderPointer CreateFinderInstance();
	struct FindDevice : std::unary_function< DevicePointer, bool >
	{
		FindDevice(const CComBSTR& udn) : m_udn( udn ) {}
		result_type operator()(argument_type device) const
		{
			CComBSTR deviceName;
			HRESULT hr = device->get_UniqueDeviceName( &deviceName );

			if ( FAILED( hr ) )
				return UPnPMessage( hr ), false;

			return wcscmp( deviceName.m_str, m_udn ) == 0;
		}
		CComBSTR m_udn;
	};
	void	ProcessAsyncFind(CComBSTR bsSearchType);
	HRESULT	GetDeviceServices(DevicePointer pDevice);
	void	StartPortMapping();
	HRESULT	MapPort(const ServicePointer& service);
	void	DeleteExistingPortMappings(ServicePointer pService);
	void	CreatePortMappings(ServicePointer pService);
	HRESULT SaveServices(com_ptr< IEnumUnknown >, const LONG nTotalItems);

	HRESULT InvokeAction(ServicePointer pService, CComBSTR action, 
		LPCTSTR pszInArgString, CString& strResult);

	// Utility functions
	HRESULT CreateSafeArray(const VARTYPE vt, const ULONG nArgs, SAFEARRAY** ppsa);
	INT_PTR CreateVarFromString(const CString& strArgs, VARIANT*** pppVars);
	INT_PTR	GetStringFromOutArgs(const VARIANT* pvaOutArgs, CString& strArgs);
	void	DestroyVars(const INT_PTR nCount, VARIANT*** pppVars);
	HRESULT GetSafeArrayBounds(SAFEARRAY* psa, LONG* pLBound, LONG* pUBound);
	HRESULT GetVariantElement(SAFEARRAY* psa, LONG pos, VARIANT* pvar);
	CString	GetLocalRoutableIP(ServicePointer pService);

// Public members
public:
	DWORD	m_tLastEvent;	// When the last event was received?

// Private members
private:
	std::vector< DevicePointer >  m_pDevices;
	std::vector< ServicePointer > m_pServices;
	FinderPointer m_pDeviceFinder;

	LONG	m_nAsyncFindHandle;
	bool	m_bAsyncFindRunning;
	bool	m_bPortIsFree;
	CString m_sLocalIP;
	CString m_sExternalIP;
	bool	m_bADSL;		// Is the device ADSL?
	bool	m_ADSLFailed;	// Did port mapping failed for the ADSL device?

	com_ptr< IUPnPDeviceFinderCallback > m_pDeviceFinderCallback;
	com_ptr< IUPnPServiceCallback >      m_pServiceCallback;
};

// DeviceFinder Callback
class CDeviceFinderCallback
	: public IUnknownImplementation< IUPnPDeviceFinderCallback >
{
public:
	CDeviceFinderCallback(CUPnPFinder& instance)
		: m_instance( instance )
	{}

// implementation
private:
	HRESULT __stdcall DeviceAdded(LONG nFindData, IUPnPDevice* pDevice);
	HRESULT __stdcall DeviceRemoved(LONG nFindData, BSTR bsUDN);
	HRESULT __stdcall SearchComplete(LONG nFindData);

private:
	CUPnPFinder& m_instance;
};

// Service Callback 
class CServiceCallback
	: public IUnknownImplementation< IUPnPServiceCallback >
{
public:
	CServiceCallback(CUPnPFinder& instance)
		: m_instance( instance )
	{}

// implementation
private:
	HRESULT __stdcall StateVariableChanged(IUPnPService* pService, LPCWSTR pszStateVarName, VARIANT varValue);
	HRESULT __stdcall ServiceInstanceDied(IUPnPService* pService);

private:
	CUPnPFinder& m_instance;
};

#endif // #ifndef UPNPFINDER_H_INCLUDED
