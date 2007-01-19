//
// IRC.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "StdAfx.h"
#include "IRC.h"

HINSTANCE         v_hModule;             // DLL module handle
HINSTANCE		  v_hResources;			 // DLL resource handle
CRITICAL_SECTION  v_csSynch;             // Critical Section
BOOL              v_fRunningOnNT;        // Flag Set When on Unicode OS

class CIRCModule : public CAtlDllModuleT< CIRCModule >, public CAtlBaseModule
{
public :
	DECLARE_LIBID(LIBID_IRCLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_IRC, "{3541E716-D0E4-48BB-B3BC-F418321651F5}")
};

CIRCModule	_AtlModule;

extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	switch ( dwReason )
	{
	case DLL_PROCESS_ATTACH:
		v_hModule = hInstance;
		v_hResources = _AtlModule.GetResourceInstance();
		v_fRunningOnNT = ( ( GetVersion() & 0x80000000 ) != 0x80000000 );
		InitializeCriticalSection( &v_csSynch );
		DisableThreadLibraryCalls( hInstance );
		break;

	case DLL_PROCESS_DETACH:
		DeleteCriticalSection( &v_csSynch );
		break;
	}

	return _AtlModule.DllMain( dwReason, lpReserved ); 
}

// Used to determine whether the DLL can be unloaded by OLE
STDAPI DllCanUnloadNow(void)
{
    return ( _AtlModule.GetLockCount() == 0 ) ? S_OK : S_FALSE;
}


// Returns a class factory to create an object of the requested type
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _AtlModule.DllGetClassObject( rclsid, riid, ppv );
}


// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
    // registers object, typelib and all interfaces in typelib
    HRESULT hr = _AtlModule.DllRegisterServer();
	return hr;
}


// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer();
	return hr;
}

