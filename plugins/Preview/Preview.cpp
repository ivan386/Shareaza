// Preview.cpp : Implementation of DLL Exports.


#include "stdafx.h"
#include "resource.h"
#include "Preview_i.h"
#include "dllmain.h"

// Used to determine whether the DLL can be unloaded by OLE
STDAPI DllCanUnloadNow(void)
{
    return _AtlModule.DllCanUnloadNow();
}

// Returns a class factory to create an object of the requested type
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _AtlModule.DllGetClassObject( rclsid, riid, ppv );
}

// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
    return _AtlModule.DllRegisterServer();
}

// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	return _AtlModule.DllUnregisterServer();
}

// DllInstall - Adds/Removes entries to the system registry per user per machine.	
STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
    HRESULT hr = E_FAIL;
    static const wchar_t szUserSwitch[] = _T("user");

    if ( pszCmdLine != NULL )
    {
    	if ( _wcsnicmp( pszCmdLine, szUserSwitch, _countof( szUserSwitch ) ) == 0 )
    	{
    		AtlSetPerUserRegistration( true );
    	}
    }

    if ( bInstall )
    {	
    	hr = DllRegisterServer();
    	if ( FAILED( hr ) )
    	{	
    		DllUnregisterServer();
    	}
    }
    else
    {
    	hr = DllUnregisterServer();
    }

    return hr;
}


