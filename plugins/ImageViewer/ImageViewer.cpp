//
// ImageViewer.cpp
//
// This software is released into the public domain. You are free to
// redistribute and modify without any restrictions.
//
// This is the main application source file, which is largely generated
// automatically.  Important items are commented.
//

#include "StdAfx.h"
#include "Resource.h"
#include <initguid.h>
#include "ImageViewer.h"
#include "ImageViewer_i.c"
#include "ImageViewerPlugin.h"

CComModule _Module;

// The ATL object map lists each COM object in this DLL

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_ImageViewerPlugin, CImageViewerPlugin)
END_OBJECT_MAP()


/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
	if ( dwReason == DLL_PROCESS_ATTACH )
	{
		_Module.Init( ObjectMap, hInstance );
		DisableThreadLibraryCalls( hInstance );
		// Don't forget to call the above, otherwise there will be a performance penalty when
		// Shareaza creates and destroys threads, which we don't want.
	}
	else if ( dwReason == DLL_PROCESS_DETACH )
	{
		_Module.Term();
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
{
	return ( _Module.GetLockCount() == 0 ) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return _Module.GetClassObject( rclsid, riid, ppv );
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
	// Pass FALSE as the argument here, as we don't want to register a type library.
	return _Module.RegisterServer( FALSE );
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
	_Module.UnregisterServer();
	return S_OK;
}
