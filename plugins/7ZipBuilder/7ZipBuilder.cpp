//
// 7ZipBuilder.cpp : Implementation of DLL Exports.
//
// Copyright (c) Shareaza Development Team, 2007-2011.
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
#include "7ZipBuilder.h"

class CModule : public CAtlDllModuleT< CModule >
{
public :
	CModule();
	virtual ~CModule();
	DECLARE_LIBID( LIBID_SevenZipBuilderLib )
	DECLARE_REGISTRY_APPID_RESOURCEID( IDR_APP, "{B69F80CD-FB15-45E8-B359-92A41CC571A7}" )

protected:
	HMODULE	m_h7zxr;

	bool Load7zxr();
	void Unload7zxr();
};

CModule::CModule() :
	m_h7zxr( NULL )
{
	if ( ! Load7zxr() )
		Unload7zxr();
}

CModule::~CModule()
{
	Unload7zxr();
}

bool CModule::Load7zxr()
{
#ifdef _WIN64
	LPCTSTR sz7zxr = _T("7zxa64.dll");
#else
	LPCTSTR sz7zxr = _T("7zxa.dll");
#endif
	m_h7zxr = LoadLibrary( sz7zxr );
	if ( ! m_h7zxr )
	{
		TCHAR szPath[ MAX_PATH ] = {};
		GetModuleFileName( _AtlBaseModule.GetModuleInstance(), szPath, MAX_PATH );
		LPTSTR c = _tcsrchr( szPath, _T('\\') );
		if ( ! c )
			return false;
		lstrcpy( c + 1, sz7zxr );
		m_h7zxr = LoadLibrary( szPath );
		if ( ! m_h7zxr )
		{
			*c = _T('\0');
			c = _tcsrchr( szPath, _T('\\') );
			if ( ! c )
				return false;
			lstrcpy( c + 1, sz7zxr );
			m_h7zxr = LoadLibrary( szPath );
			if ( ! m_h7zxr )
				return false;
		}
	}
	fnCreateObject = (tCreateObject)GetProcAddress( m_h7zxr, "CreateObject");
	return ( fnCreateObject != NULL );
}

void CModule::Unload7zxr()
{
	if ( m_h7zxr )
	{
		fnCreateObject = NULL;
		FreeLibrary( m_h7zxr );
		m_h7zxr = NULL;
	}
}

CModule _AtlModule;

extern "C" BOOL WINAPI DllMain(HINSTANCE /*hInstance*/, DWORD dwReason, LPVOID lpReserved)
{
    return _AtlModule.DllMain( dwReason, lpReserved ); 
}

STDAPI DllCanUnloadNow(void)
{
    return _AtlModule.DllCanUnloadNow();
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _AtlModule.DllGetClassObject( rclsid, riid, ppv );
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
