//
// RazaWebHook.cpp : Implementation of DLL Exports.
//
// Copyright (c) Shareaza Development Team, 2008-2010.
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

#include "stdafx.h"
#include "resource.h"
#include "RazaWebHook_i.h"

class CRazaWebHookModule : public CAtlDllModuleT< CRazaWebHookModule >
{
public :
	DECLARE_LIBID(LIBID_RazaWebHookLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_RAZAWEBHOOK, "{07D1F248-BCA2-4257-8863-C9ACCFBAD83D}")
};

CRazaWebHookModule _AtlModule;

extern "C" BOOL WINAPI DllMain(HINSTANCE /*hInstance*/, DWORD dwReason, LPVOID lpReserved)
{
	// Black-listed processes
	if ( dwReason == DLL_PROCESS_ATTACH )
	{
		TCHAR szName[ MAX_PATH ] = {};
		DWORD dwLength = GetModuleFileName( NULL, szName, _countof( szName ) );

		// Windows Explorer
		if ( lstrcmpi( szName + dwLength - 13, _T("\\explorer.exe") ) == 0 )
			return FALSE;
	}

	return _AtlModule.DllMain( dwReason, lpReserved ); 
}

STDAPI DllCanUnloadNow(void)
{
    return _AtlModule.DllCanUnloadNow();
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

STDAPI DllRegisterServer(void)
{
    return _AtlModule.DllRegisterServer();
}

STDAPI DllUnregisterServer(void)
{
	return _AtlModule.DllUnregisterServer();
}

STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
    HRESULT hr = E_FAIL;
    static const wchar_t szUserSwitch[] = L"user";

    if (pszCmdLine != NULL)
    {
    	if (_wcsnicmp(pszCmdLine, szUserSwitch, _countof(szUserSwitch)) == 0)
    	{
    		AtlSetPerUserRegistration(true);
    	}
    }

    if (bInstall)
    {	
    	hr = DllRegisterServer();
    	if (FAILED(hr))
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
