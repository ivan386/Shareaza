// dllmain.cpp : Implementation of DllMain.

#include "stdafx.h"
#include "resource.h"
#include "RazaWebHook_i.h"
#include "dllmain.h"
#include "dlldatax.h"

CRazaWebHookModule _AtlModule;

extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if ( dwReason == DLL_PROCESS_ATTACH )
		DisableThreadLibraryCalls( hInstance );

#ifdef _MERGE_PROXYSTUB
	if ( ! PrxDllMain( hInstance, dwReason, lpReserved ) )
		return FALSE;
#endif

	return _AtlModule.DllMain( dwReason, lpReserved ); 
}
