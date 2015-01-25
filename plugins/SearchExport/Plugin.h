// Plugin.h : Declaration of the CPlugin

#pragma once

#include "SearchExport.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

// CPlugin

class ATL_NO_VTABLE CPlugin :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CPlugin, &CLSID_Plugin>,
	public IGeneralPlugin,
	public ICommandPlugin
{
public:
	CPlugin() :
		m_nCmdCheck( 0 )
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_PLUGIN)

BEGIN_COM_MAP(CPlugin)
	COM_INTERFACE_ENTRY(IGeneralPlugin)
	COM_INTERFACE_ENTRY(ICommandPlugin)
END_COM_MAP()

DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
		m_pApplication.Release();
		m_pUserInterface.Release();
	}

protected:
	CComPtr< IApplication >		m_pApplication;		// Shareaza application
	CComPtr< IUserInterface >	m_pUserInterface;	// Shareaza GUI
	UINT						m_nCmdCheck;		// Command ID

	HRESULT Export(IGenericView* pGenericView, LONG nCount);

	// Insert menu item if no item present only
	void InsertCommand(LPCTSTR szTitle, const LPCWSTR* szMenu, UINT nID);

// IGeneralPlugin
public:
	STDMETHOD(SetApplication)( 
		/* [in] */ IApplication __RPC_FAR *pApplication);
	STDMETHOD(QueryCapabilities)(
		/* [in] */ DWORD __RPC_FAR *pnCaps);
	STDMETHOD(Configure)();
	STDMETHOD(OnSkinChanged)();

// ICommandPlugin
public:
	STDMETHOD(RegisterCommands)();
	STDMETHOD(InsertCommands)();
    STDMETHOD(OnUpdate)( 
        /* [in] */ UINT nCommandID,
        /* [out][in] */ TRISTATE __RPC_FAR *pbVisible,
        /* [out][in] */ TRISTATE __RPC_FAR *pbEnabled,
        /* [out][in] */ TRISTATE __RPC_FAR *pbChecked);
	STDMETHOD(OnCommand)( 
		/* [in] */ UINT nCommandID);
};

OBJECT_ENTRY_AUTO(__uuidof(Plugin), CPlugin)
