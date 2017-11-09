//
// GFLLibraryBuilder.cpp : Implementation of DLL Exports.
//
// Copyright (c) Nikolay Raspopov, 2005-2014.
// This file is part of SHAREAZA (shareaza.sourceforge.net)
//
// GFL Library, GFL SDK and XnView
// Copyright (c) 1991-2004 Pierre-E Gougelet
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
#include "GFLLibraryBuilder.h"

#define REG_LIBRARYBUILDER_KEY _T("Software\\Shareaza\\Shareaza\\Plugins\\LibraryBuilder")

class CGFLLibraryBuilderModule : public CAtlDllModuleT< CGFLLibraryBuilderModule >
{
public :
	DECLARE_LIBID(LIBID_GFLLibraryBuilderLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_GFLLIBRARYBUILDER, "{F74AD137-A43F-46FD-A1FE-6532C3FC3E88}")
};

CGFLLibraryBuilderModule _AtlModule;
HINSTANCE				_hModuleInstance = NULL;
typedef ATL::CAtlMap < ATL::CStringA, GFL_INT32 > CAtlStrStrMap;
CAtlStrStrMap			_ExtMap;

inline void FillExtMap()
{
	_ExtMap.RemoveAll();

	GFL_INT32 count = gflGetNumberOfFormat();
	ATLTRACE( "Total %d formats:\n", count );
	for ( GFL_INT32 i = 0; i < count; ++i )
	{
		GFL_FORMAT_INFORMATION info = {};
		GFL_ERROR err = gflGetFormatInformationByIndex (i, &info);
		if ( err == GFL_NO_ERROR && (info.Status & GFL_READ) )
		{
			ATLTRACE( "%3d. %7s %32s #%d :", i, info.Name, info.Description, info.Index );
			for (GFL_UINT32 j = 0; j < info.NumberOfExtension; ++j)
			{
				CStringA ext( info.Extension [j] );
				ext.MakeLower();

				// GFL bug fix for short extensions
				if ( ext == "pspbrus" )		// PaintShopPro Brush
					ext = "pspbrush";
				else if ( ext == "pspfram" )	// PaintShopPro Frame
					ext = "pspframe";
				else if ( ext == "pspimag" )	// PaintShopPro Image
					ext = "pspimage";

				ATLTRACE( " .%s", (LPCSTR)ext );
				GFL_INT32 index;
				if ( ! _ExtMap.Lookup( ext, index ) )
					_ExtMap.SetAt( ext, info.Index );
			}
			ATLTRACE( "\n" );
		}
	}
}

BOOL SafeGFLInit() throw()
{
	__try
	{
		// Library initialization
		if ( gflLibraryInit () != GFL_NO_ERROR )
		{
			ATLTRACE ("gflLibraryInit failed\n");
			return FALSE;
		}
		gflEnableLZW( GFL_TRUE );
		FillExtMap ();
		return TRUE;
	}
	__except ( EXCEPTION_EXECUTE_HANDLER )
	{
		ATLTRACE ("Exception in DLL_PROCESS_ATTACH\n");
		return FALSE;
	}
}

void SafeGFLExit() throw()
{
	__try
	{
		gflLibraryExit ();
	}
	__except ( EXCEPTION_EXECUTE_HANDLER )
	{
		ATLTRACE ("Exception in DLL_PROCESS_DETACH\n");
	}
}

extern "C" BOOL WINAPI DllMain (HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (_AtlModule.DllMain( dwReason, lpReserved ) )
	{
		switch (dwReason)
		{
		case DLL_PROCESS_ATTACH:
			_hModuleInstance = hInstance;
			return SafeGFLInit();

		case DLL_PROCESS_DETACH:
			SafeGFLExit();
			break;
		}
		return TRUE;
	}
	else
	{
		ATLTRACE ("FALSE in _AtlModule.DllMain () call\n");
		return FALSE;
	}
}

STDAPI DllCanUnloadNow(void)
{
    return _AtlModule.DllCanUnloadNow ();
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _AtlModule.DllGetClassObject (rclsid, riid, ppv);
}

STDAPI DllRegisterServer(void)
{
    HRESULT hr = _AtlModule.DllRegisterServer ();

	// Registering extensions using GFL
	for ( POSITION pos = _ExtMap.GetStartPosition (); pos; )
	{
		CStringA ext;
		GFL_INT32 index;
		_ExtMap.GetNextAssoc( pos, ext, index );
		if ( ext == "pdf" || ext == "ps" || ext == "eps" || ext == "vst" )
			continue;
		ext.Insert( 0, '.' );
		ATLTRACE ("Add %s\n", (LPCSTR)ext );
		SHSetValue (HKEY_CURRENT_USER, REG_LIBRARYBUILDER_KEY, CA2T( ext ), REG_SZ,
			_T("{6C9E61BE-E58F-4AE1-A304-6FF1D183804C}"),
			38 * sizeof (TCHAR));
	}

	return hr;
}

STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer ();

	// Unregistering extensions using GFL
	for ( POSITION pos = _ExtMap.GetStartPosition (); pos; )
	{
		CStringA ext;
		GFL_INT32 index;
		_ExtMap.GetNextAssoc( pos, ext, index );
		if ( ext == "pdf" || ext == "ps" || ext == "eps" || ext == "vst" )
			continue;
		ext.Insert( 0, '.' );
		ATLTRACE ("Remove %s\n", (LPCSTR)ext );
		SHDeleteValue( HKEY_CURRENT_USER, REG_LIBRARYBUILDER_KEY, CA2T( ext ) );
	}

	return hr;
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
