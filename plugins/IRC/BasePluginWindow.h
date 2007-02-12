#pragma once
#include "IRC.h"
#include "IRCPlugin.h"
using namespace ATLControls;

class IPluginWindowOwnerImpl : public IPluginWindowOwner
{
	// Attributes
protected:
	CIRCPlugin*				m_pPlugin;
	CComPtr<IApplication>	m_pApplication;
	CComPtr<IPluginWindow>	m_pWindow;
	CString					m_sClassName;
	HWND					m_hParent;

public:
	BOOL Initialize(CIRCPlugin* pPlugin, LPCTSTR pszClassName, HWND hParent = NULL)
	{
		m_pPlugin		= pPlugin;
		m_pApplication	= pPlugin->m_pApplication;
		m_sClassName	= pszClassName;
		m_hParent		= hParent;

		// Get an IUserInterface pointer from the IApplication
		CComPtr< IUserInterface > pUI;
		m_pApplication->get_UserInterface( &pUI );
		pUI->NewWindow( CComBSTR( m_sClassName ), this, &m_pWindow );
		return TRUE;
	}

	HRESULT __stdcall OnTranslate(MSG* pMessage)
	{ return S_OK; }

	HRESULT __stdcall OnMessage(INT nMessage, WPARAM wParam, LPARAM  Param, LRESULT* plResult)
	{ return S_OK; }

	HRESULT __stdcall OnUpdate(INT nCommandID, STRISTATE* pbVisible, STRISTATE* pbEnabled, STRISTATE* pbChecked)
	{ return S_OK; }
	
	HRESULT __stdcall OnCommand(INT nCommandID)
	{ return S_OK; }

	HRESULT __stdcall GetWndClassName(BSTR* psName)
	{
		m_sClassName.SetSysString( psName );
		return S_OK;
	}
};

template <class TWinTraits = CNullTraits>
class ATL_NO_VTABLE CBasePluginWindow : public CComObjectRootEx< CComSingleThreadModel >,
										public CWindowImpl< CBasePluginWindow, CWindow, TWinTraits >,
										public IPluginWindowOwnerImpl
{
public:
	CBasePluginWindow(){}
	virtual ~CBasePluginWindow(){}
	HRESULT FinalConstruct() { return S_OK; }
	void FinalRelease() {}
	virtual HWND Create()
	{
		DWORD dwStyle = TWinTraits::GetWndStyle( 0 );
		DWORD dwExStyle = TWinTraits::GetWndExStyle( 0 );
		return CWindow::Create( m_sClassName, m_hParent, 0, NULL, dwStyle, dwExStyle );
	}

	DECLARE_NOT_AGGREGATABLE(CBasePluginWindow)
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	// Message Map
public:
	BEGIN_MSG_MAP(CBasePluginWindow)
	END_MSG_MAP()
};
