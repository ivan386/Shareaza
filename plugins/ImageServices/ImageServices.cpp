//
// ImageServices.cpp
//
// Copyright (c) Michael Stokes, 2002-2004.
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

#include "StdAfx.h"
#include "Resource.h"
#include <initguid.h>
#include "ImageServices.h"
#include "JPEGReader.h"
#include "PNGReader.h"
//#include "GIFReader.h"
#include "AVIThumb.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_JPEGReader, CJPEGReader)
	OBJECT_ENTRY(CLSID_PNGReader, CPNGReader)
//	OBJECT_ENTRY(CLSID_GIFReader, CGIFReader)
	OBJECT_ENTRY(CLSID_AVIThumb, CAVIThumb)
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

STDAPI DllRegisterServer()
{
	return _Module.RegisterServer( FALSE );
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer()
{
	_Module.UnregisterServer();
	return S_OK;
}
