// dllmain.h : Declaration of module class.

class CRazaWebHookModule : public CAtlDllModuleT< CRazaWebHookModule >
{
public :
	DECLARE_LIBID(LIBID_RazaWebHookLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_RAZAWEBHOOK, "{07D1F248-BCA2-4257-8863-C9ACCFBAD83D}")
};

extern class CRazaWebHookModule _AtlModule;
