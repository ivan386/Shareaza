#pragma once
#include "IRC.h"
using namespace ATLControls;

class CIRCPlugin;

template <class TWinTraits = CNullTraits>
class ATL_NO_VTABLE CBasePluginWindow : public CComObjectRootEx< CComSingleThreadModel >,
										public CWindowImpl< CBasePluginWindow, CWindow, TWinTraits >,
										public IPluginWindowOwner
{
	// Attributes
protected:
	CIRCPlugin*				m_pPlugin;
	CComPtr<IApplication>	m_pApplication;
	CComPtr<IPluginWindow>	m_pWindow;
	CString					m_sClassName;

public:
	virtual BOOL Create(CIRCPlugin* pPlugin, LPCTSTR pszClassName);

	HRESULT FinalConstruct() { return S_OK; }
	void FinalRelease() {}

	DECLARE_NOT_AGGREGATABLE(CBasePluginWindow)

	BEGIN_COM_MAP(CBasePluginWindow)
		COM_INTERFACE_ENTRY(IPluginWindowOwner)
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	// IPluginWindowOwner Methods
public:
	virtual HRESULT __stdcall OnTranslate(MSG* pMessage);
	virtual HRESULT __stdcall OnMessage(INT nMessage, WPARAM wParam, LPARAM  Param, LRESULT* plResult);
	virtual HRESULT __stdcall OnUpdate(INT nCommandID, STRISTATE* pbVisible, STRISTATE* pbEnabled, STRISTATE* pbChecked);
	virtual HRESULT __stdcall OnCommand(INT nCommandID) = 0;
	virtual HRESULT __stdcall GetWndClassName(BSTR* psName);

	// Message Map
public:
	BEGIN_MSG_MAP(CBasePluginWindow)
	END_MSG_MAP()
};
