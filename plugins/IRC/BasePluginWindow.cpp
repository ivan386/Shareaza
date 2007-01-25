#include "StdAfx.h"
#include "IRCPlugin.h"
#include "BasePluginWindow.h"

template <class TWinTraits>
BOOL CBasePluginWindow< TWinTraits >::Create(CIRCPlugin* pPlugin, LPCTSTR pszClassName)
{
	m_pPlugin		= pPlugin;
	m_pApplication	= pPlugin->m_pApplication;
	m_sClassName	= pszClassName;

	// Get an IUserInterface pointer from the IApplication
	CComPtr< IUserInterface > pUI;
	m_pApplication->get_UserInterface( &pUI );
	pUI->NewWindow( CComBSTR( m_sClassName ), this, &m_pWindow );

	return TRUE;
}

// IPluginWindowOwner Methods
template <class TWinTraits>
HRESULT CBasePluginWindow< TWinTraits >::OnTranslate(MSG* pMessage)
{
	return E_NOTIMPL;
}

template <class TWinTraits>
HRESULT CBasePluginWindow< TWinTraits >::OnMessage(INT nMessage, WPARAM wParam, LPARAM lParam, LRESULT* plResult)
{
	return ProcessWindowMessage( m_hWnd, nMessage, wParam, lParam, *plResult ) ? S_OK : S_FALSE;
}

template <class TWinTraits>
HRESULT CBasePluginWindow< TWinTraits >::GetWndClassName(BSTR* psName)
{
	m_sClassName.SetSysString( psName );
	return S_OK;
}
