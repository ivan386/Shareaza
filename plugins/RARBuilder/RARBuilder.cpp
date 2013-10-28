//
// RARBuilder.cpp : Implementation of DLL Exports.
//
// Copyright (c) Shareaza Development Team, 2007.
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
#include "RARBuilder.h"

class CModule : public CAtlDllModuleT< CModule >
{
public:
	CModule();
	virtual ~CModule();
	DECLARE_LIBID( LIBID_RARBuilderLib )
	DECLARE_REGISTRY_APPID_RESOURCEID( IDR_APP, "{F801DAD7-F08D-48EF-B0DF-6B120377E835}" )

protected:
	HMODULE	m_hUnrar;

	bool LoadUnrar();
	void UnloadUnrar();
};

CModule::CModule() :
	m_hUnrar( NULL )
{
	if ( ! LoadUnrar() )
		UnloadUnrar();
}

CModule::~CModule()
{
	UnloadUnrar();
}

bool CModule::LoadUnrar()
{
#ifdef _WIN64
	LPCTSTR szUnRAR = _T("unrar64.dll");
#else
	LPCTSTR szUnRAR = _T("unrar.dll");
#endif
	m_hUnrar = LoadLibrary( szUnRAR );
	if ( ! m_hUnrar )
	{
		TCHAR szPath[ MAX_PATH ] = {};
		GetModuleFileName( _AtlBaseModule.GetModuleInstance(), szPath, MAX_PATH );
		LPTSTR c = _tcsrchr( szPath, _T('\\') );
		if ( ! c )
			return false;
		lstrcpy( c + 1, szUnRAR );
		m_hUnrar = LoadLibrary( szPath );
		if ( ! m_hUnrar )
		{
			*c = _T('\0');
			c = _tcsrchr( szPath, _T('\\') );
			if ( ! c )
				return false;
			lstrcpy( c + 1, szUnRAR );
			m_hUnrar = LoadLibrary( szPath );
			if ( ! m_hUnrar )
				return false;
		}
	}
	fnRAROpenArchiveEx = (tRAROpenArchiveEx)GetProcAddress( m_hUnrar, "RAROpenArchiveEx");
	fnRARCloseArchive = (tRARCloseArchive)GetProcAddress( m_hUnrar, "RARCloseArchive");
	fnRARReadHeaderEx = (tRARReadHeaderEx)GetProcAddress( m_hUnrar, "RARReadHeaderEx");
	fnRARProcessFileW = (tRARProcessFileW)GetProcAddress( m_hUnrar, "RARProcessFileW");
	return ( fnRAROpenArchiveEx && fnRARCloseArchive && fnRARReadHeaderEx &&
		fnRARProcessFileW );
}

void CModule::UnloadUnrar()
{
	if ( m_hUnrar )
	{
		fnRAROpenArchiveEx = NULL;
		fnRARCloseArchive = NULL;
		fnRARReadHeaderEx = NULL;
		fnRARProcessFileW = NULL;
		FreeLibrary( m_hUnrar );
		m_hUnrar = NULL;
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
