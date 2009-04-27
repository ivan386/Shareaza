//
// GFLImageServices.cpp : Implementation of DLL Exports.
//
// Copyright (c) Nikolay Raspopov, 2005.
// This file is part of SHAREAZA (www.shareaza.com)
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
#include "GFLImageServices.h"

#define REG_IMAGESERVICE_KEY _T("Software\\Shareaza\\Shareaza\\Plugins\\ImageService")

class CGFLImageServicesModule : public CAtlDllModuleT< CGFLImageServicesModule >
{
public :
	DECLARE_LIBID (LIBID_GFLImageServicesLib)
	DECLARE_REGISTRY_APPID_RESOURCEID (IDR_GFLIMAGESERVICES, "{4DD7500D-4ACC-4833-AB1D-887C59199DC5}")
};

CGFLImageServicesModule	_AtlModule;
HINSTANCE				_hModuleInstance = NULL;
typedef ATL::CAtlMap <ATL::CString, ATL::CString> CAtlStrStrMap;
CAtlStrStrMap			_ExtMap;

inline void FillExtMap ()
{
	CString tmp;
	_ExtMap.RemoveAll ();
	GFL_INT32 count = gflGetNumberOfFormat ();
	ATLTRACE( _T("Total %d formats:\n"), count );
	for (GFL_INT32 i = 0; i < count; ++i) {
		GFL_FORMAT_INFORMATION info;
		GFL_ERROR err = gflGetFormatInformationByIndex (i, &info);
		if (err == GFL_NO_ERROR && (info.Status & GFL_READ)) {
			CString name (info.Name);
			CString desc (info.Description);
			ATLTRACE( _T("%3d. %7s %32s :"), i, name, desc );
			for (GFL_UINT32 j = 0; j < info.NumberOfExtension; ++j) {
				CString ext (info.Extension [j]);
				ext = ext.MakeLower ();
				ATLTRACE( _T(" .%s"), ext );
				if (!_ExtMap.Lookup (ext, tmp))
					_ExtMap.SetAt (ext, name);
			}
			ATLTRACE( _T("\n") );
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
			ATLTRACE (_T("gflLibraryInit failed\n"));
			return FALSE;
		}
		gflEnableLZW( GFL_TRUE );
		FillExtMap ();
		return TRUE;
	}
	__except ( EXCEPTION_EXECUTE_HANDLER )
	{
		ATLTRACE (_T("Exception in DLL_PROCESS_ATTACH\n"));
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
		ATLTRACE (_T("Exception in DLL_PROCESS_DETACH\n"));
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
		ATLTRACE (_T("FALSE in _AtlModule.DllMain () call\n"));
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
	CString ext, tmp;
	POSITION pos = _ExtMap.GetStartPosition ();
	while (pos) {
		_ExtMap.GetNextAssoc (pos, ext, tmp);
		if ( ext == _T("vst") ) continue;
		ext.Insert (0, _T('.'));
		ATLTRACE (_T("Add %s\n"), ext);
		SHSetValue (HKEY_CURRENT_USER, REG_IMAGESERVICE_KEY, ext, REG_SZ,
			_T("{FF5FCD00-2C20-49D8-84F6-888D2E2C95DA}"),
			38 * sizeof (TCHAR));
	}

	return hr;
}

STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer ();

	// Unregistering extensions using GFL
	CString ext, tmp;
	POSITION pos = _ExtMap.GetStartPosition ();
	while (pos) {
		_ExtMap.GetNextAssoc (pos, ext, tmp);
		if ( ext == _T("vst") ) continue;
		ext.Insert (0, _T('.'));
		ATLTRACE (_T("Remove %s\n"), ext);
		SHDeleteValue (HKEY_CURRENT_USER, REG_IMAGESERVICE_KEY, ext);
	}

	return hr;
}

HRESULT SAFEgflLoadBitmap (const char * filename, GFL_BITMAP **bitmap, const GFL_LOAD_PARAMS *params, GFL_FILE_INFORMATION *info) throw ()
{
	HRESULT hr = E_FAIL;
	__try {
		GFL_ERROR err = gflLoadBitmap (filename, bitmap, params, info);
		if (err == GFL_NO_ERROR)
			hr = S_OK;
		else {
			ATLTRACE (L"gflLoadBitmap() error : %s\n", CA2T (gflGetErrorString (err)));
		}
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		ATLTRACE (L"gflLoadBitmap() exception\n");
	}
	return hr;
}

HRESULT SAFEgflLoadBitmapFromMemory (const GFL_UINT8 * data, GFL_UINT32 data_length, GFL_BITMAP **bitmap, const GFL_LOAD_PARAMS *params, GFL_FILE_INFORMATION *info) throw ()
{
	HRESULT hr = E_FAIL;
	__try {
		GFL_ERROR err = gflLoadBitmapFromMemory (data, data_length, bitmap, params, info);
		if (err == GFL_NO_ERROR)
			hr = S_OK;
		else {
			ATLTRACE (L"gflLoadBitmapFromMemory() error : %s\n", CA2T (gflGetErrorString (err)));
		}
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		ATLTRACE (L"gflLoadBitmapFromMemory() exception\n");
	}
	return hr;
}

HRESULT SAFEgflSaveBitmapIntoMemory (GFL_UINT8 ** data, GFL_UINT32 * data_length, const GFL_BITMAP *bitmap, const GFL_SAVE_PARAMS *params) throw ()
{
	HRESULT hr = E_FAIL;
	__try {
		GFL_ERROR err = gflSaveBitmapIntoMemory (data, data_length, bitmap, params);
		if (err == GFL_NO_ERROR)
			hr = S_OK;
		else {
			ATLTRACE (L"gflSaveBitmapIntoMemory() error : %s\n", CA2T (gflGetErrorString (err)));
		}
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		ATLTRACE (L"gflSaveBitmapIntoMemory() exception\n");
	}
	return hr;
}

HRESULT SAFEgflSaveBitmap (char *filename, const GFL_BITMAP *bitmap, const GFL_SAVE_PARAMS *params) throw ()
{
	HRESULT hr = E_FAIL;
	__try {
		GFL_ERROR err = gflSaveBitmap (filename, bitmap, params);
		if (err == GFL_NO_ERROR)
			hr = S_OK;
		else {
			ATLTRACE (L"gflSaveBitmap() error : %s\n", CA2T (gflGetErrorString (err)));
		}
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		ATLTRACE (L"gflSaveBitmap() exception\n");
	}
	return hr;
}

int GetFormatIndexByExt (LPCTSTR ext)
{
	CString name;
	if (_ExtMap.Lookup (ext, name))
		return gflGetFormatIndexByName (CT2CA (name));
	else
		return -1;
}
