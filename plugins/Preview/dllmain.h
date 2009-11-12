// dllmain.h : Declaration of module class.

class CPreviewModule : public CAtlDllModuleT< CPreviewModule >
{
public :
	DECLARE_LIBID(LIBID_PreviewLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_PREVIEW, "{78537474-C03F-4ADE-A817-BD7CC0A4D2B4}")
};

extern class CPreviewModule _AtlModule;
