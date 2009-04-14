// MediaPlayer.cpp : Implementation of DLL Exports.

#include "stdafx.h"
#include "MediaPlayer_h.h"
#include "MediaPlayer_i.c"

class CMediaPlayerModule : public CAtlDllModuleT< CMediaPlayerModule >
{
public :
	DECLARE_LIBID(LIBID_MediaPlayerLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_MEDIAPLAYER, "{7F669B06-74D9-42A9-A157-DD08EE5F30BA}")
};

extern class CMediaPlayerModule _AtlModule;

CMediaPlayerModule _AtlModule;

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

    if ( pszCmdLine && ! _wcsnicmp( pszCmdLine, szUserSwitch, _countof( szUserSwitch ) ) )
   		AtlSetPerUserRegistration(true);

    if ( bInstall )
    {	
    	hr = DllRegisterServer();
    	if ( FAILED( hr ) )
    		DllUnregisterServer();
    }
    else
    	hr = DllUnregisterServer();

    return hr;
}
