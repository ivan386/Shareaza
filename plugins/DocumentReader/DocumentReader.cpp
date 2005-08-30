// DocumentReader.cpp : Implementation of DLL Exports.
#pragma once
#include "stdafx.h"
#include "Globals.h"
#include "resource.h"
#include "DocumentReader.h"
#include "dlldatax.h"

////////////////////////////////////////////////////////////////////////
// Globals for this module.
//
HINSTANCE         v_hModule;             // DLL module handle
ULONG             v_cLocks;              // Count of server locks
CRITICAL_SECTION  v_csSynch;             // Critical Section
HANDLE            v_hPrivateHeap;        // Private Heap for Component
BOOL              v_fRunningOnNT;        // Flag Set When on Unicode OS
PFN_STGOPENSTGEX  v_pfnStgOpenStorageEx; // StgOpenStorageEx (Win2K/XP only)

class CDocumentReaderModule : public CAtlDllModuleT< CDocumentReaderModule >
{
public :
	CDocumentReaderModule();
	DECLARE_LIBID(LIBID_DocumentReaderLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_DOCUMENTREADER, "{BEC42E3F-4B6B-49A3-A099-EB3D6752AA02}")
	HRESULT DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
	DWORD m_dwThreadID;
};

CDocumentReaderModule _AtlModule;

CDocumentReaderModule::CDocumentReaderModule()
{
	m_dwThreadID = GetCurrentThreadId();
}

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
#ifdef _MERGE_PROXYSTUB
    if (!PrxDllMain(hInstance, dwReason, lpReserved))
        return FALSE;
#endif

	switch ( dwReason )
	{
	case DLL_PROCESS_ATTACH:
		ODS("DllMain - Attach\n");
		v_hModule = hInstance; v_cLocks = 0;
        v_hPrivateHeap = HeapCreate(0, 0x1000, 0);
        v_fRunningOnNT = ( ( GetVersion() & 0x80000000 ) != 0x80000000 );
        v_pfnStgOpenStorageEx = ( ( v_fRunningOnNT ) ? 
			(PFN_STGOPENSTGEX)GetProcAddress( GetModuleHandle("OLE32"), "StgOpenStorageEx" ) : NULL );
		InitializeCriticalSection( &v_csSynch );
		DisableThreadLibraryCalls( hInstance );
		break;

	case DLL_PROCESS_DETACH:
		ODS("DllMain - Detach\n");
        if ( v_hPrivateHeap ) HeapDestroy( v_hPrivateHeap );
        DeleteCriticalSection( &v_csSynch );
		break;
	}

    return _AtlModule.DllMain( dwReason, lpReserved ); 
}

// Used to determine whether the DLL can be unloaded by OLE
STDAPI DllCanUnloadNow(void)
{
#ifdef _MERGE_PROXYSTUB
    HRESULT hr = PrxDllCanUnloadNow();
    if ( FAILED(hr) ) return hr;
#endif
    return ( _AtlModule.GetLockCount() == 0 ) ? S_OK : S_FALSE;
}

// Returns a class factory to create an object of the requested type
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
#ifdef _MERGE_PROXYSTUB
    if ( PrxDllGetClassObject(rclsid, riid, ppv) == S_OK ) return S_OK;
#endif
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
	LPWSTR  pwszModule;

	// If we can't find the path to the DLL, we can't register...
	if (!FGetModuleFileName( v_hModule, &pwszModule) )
		return E_UNEXPECTED;

    // registers object, typelib and all interfaces in typelib
    HRESULT hr = _AtlModule.DllRegisterServer();

#ifdef _MERGE_PROXYSTUB
    if ( FAILED(hr) ) return hr;
    hr = PrxDllRegisterServer();
#endif
	return hr;
}

// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer();
#ifdef _MERGE_PROXYSTUB
    if ( FAILED(hr) ) return hr;
    hr = PrxDllRegisterServer();
    if ( FAILED(hr) ) return hr;
    hr = PrxDllUnregisterServer();
#endif
	return hr;
}
HRESULT CDocumentReaderModule::DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	ODS("CDocumentReaderModule::DllGetClassObject\n");
	HRESULT hr;
	CDocumentClassFactory* pcf;

	CHECK_NULL_RETURN(ppv, E_POINTER);
	*ppv = NULL;

 // The only components we can create
	if ( rclsid != CLSID_DocReader )
		return CLASS_E_CLASSNOTAVAILABLE;

 // Create the needed class factory...
	pcf = new CDocumentClassFactory();
	CHECK_NULL_RETURN( pcf, E_OUTOFMEMORY );

 // Get requested interface.
	if ( SUCCEEDED(hr = pcf->QueryInterface(rclsid, ppv)) )
        { pcf->LockServer(TRUE); }
    else
        { *ppv = NULL; delete pcf; }

	return hr;
}
