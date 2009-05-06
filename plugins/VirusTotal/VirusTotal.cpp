// VirusTotal.cpp : Implementation of DLL Exports.

#include "stdafx.h"
#include "VirusTotal_h.h"

class CVirusTotalModule : public CAtlDllModuleT< CVirusTotalModule >
{
public :
	DECLARE_LIBID(LIBID_VirusTotalLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_VIRUSTOTAL, "{0F99F34D-3377-4882-9251-E00D680C6C89}")
};

CVirusTotalModule _AtlModule;

extern "C" BOOL WINAPI DllMain(HINSTANCE, DWORD dwReason, LPVOID lpReserved)
{
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
	static const wchar_t szUserSwitch[] = _T("user");

	if (pszCmdLine != NULL)
	{
		if (_wcsnicmp(pszCmdLine, szUserSwitch, _countof(szUserSwitch)) == 0)
		{
#if _MFC_VER > 0x0800
			AtlSetPerUserRegistration(true);
#endif
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
