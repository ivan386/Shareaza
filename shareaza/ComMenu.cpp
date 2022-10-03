//
// ComMenu.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "Application.h"
#include "ComMenu.h"
#include "CoolInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CComMenu, CCmdTarget)
END_MESSAGE_MAP()

BEGIN_INTERFACE_MAP(CComMenu, CComObject)
	INTERFACE_PART(CComMenu, IID_ISMenu, SMenu)
	INTERFACE_PART(CComMenu, IID_IEnumVARIANT, EnumVARIANT)
END_INTERFACE_MAP()


BOOL FindMenuId(HMENU hMenu, UINT nRequestID, HMENU& hParent, UINT& nPosition, HMENU& hResultMenu)
{
	const int nCount = GetMenuItemCount( hMenu );
	if ( nCount > 0 && nCount < (int)nRequestID  )
	{
		for ( int i = 0; i < nCount; ++i )
		{
			if ( HMENU hSubMenu = GetSubMenu( hMenu, i ) )
			{
				// Check help id of submenu menu item
				if ( const UINT nID = GetMenuContextHelpId( hSubMenu ) )
				{
					if ( nID == nRequestID )
					{
						hParent = hMenu;
						hResultMenu = hSubMenu;
						nPosition = i;
						return TRUE;
					}
				}

				// Check submenu recursively
				if ( FindMenuId( hSubMenu, nRequestID, hParent, nPosition, hResultMenu ) )
					return TRUE;
			}
			else
			{
				// Check item id of normal menu item
				const UINT nID = GetMenuItemID( hMenu, i );
				if ( nID == nRequestID )
				{
					hParent = hMenu;
					hResultMenu = NULL;
					nPosition = i;
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CComMenu construction

CComMenu::CComMenu(HMENU hMenu, UINT nPosition)
	: m_hParent		( NULL )
	, m_hMenu		( NULL )
	, m_nPosition	( 0 )
{
	if ( nPosition == 0xFFFFFFFF )
	{
		m_hMenu		= hMenu;
	}
	else
	{
		m_hParent	= hMenu;
		m_hMenu		= GetSubMenu( hMenu, nPosition );
		m_nPosition	= nPosition;
		if ( ! m_hMenu )
		{
			// Try to find item by ID
			if ( ! FindMenuId( hMenu, nPosition, m_hParent, m_nPosition, m_hMenu ) )
			{
				// Item not found
				if ( (int)nPosition >= GetMenuItemCount( hMenu ) )
				{
					// No item
					m_hParent = NULL;
				}
			}
		}
	}

	EnableDispatch( IID_ISMenu );
}

CComMenu::~CComMenu()
{
}

/////////////////////////////////////////////////////////////////////////////
// CComMenu operations

ISMenu* CComMenu::Wrap(HMENU hMenu, UINT nPosition)
{
	CComMenu* pWrap = new CComMenu( hMenu, nPosition );
	if ( ! pWrap )
		// Out of memory
		return NULL;

	if ( ! pWrap->m_hParent && ! pWrap->m_hMenu )
	{
		// Not a item, not a submenu
		delete pWrap;
		return NULL;
	}

	return (ISMenu*)pWrap->GetInterface( IID_ISMenu, FALSE );
}

/////////////////////////////////////////////////////////////////////////////
// CComMenu ISMenu

IMPLEMENT_DISPATCH(CComMenu, SMenu)

STDMETHODIMP CComMenu::XSMenu::get_Application(IApplication FAR* FAR* ppApplication)
{
	METHOD_PROLOGUE( CComMenu, SMenu )
	return CApplication::GetApp( ppApplication );
}

STDMETHODIMP CComMenu::XSMenu::get_UserInterface(IUserInterface FAR* FAR* ppUserInterface)
{
	METHOD_PROLOGUE( CComMenu, SMenu )
	return CApplication::GetUI( ppUserInterface );
}

STDMETHODIMP CComMenu::XSMenu::get__NewEnum(IUnknown FAR* FAR* ppEnum)
{
	METHOD_PROLOGUE( CComMenu, SMenu )
	if ( pThis->m_hMenu == NULL ) return E_FAIL;
	AddRef();
	*ppEnum = &pThis->m_xEnumVARIANT;
	pThis->m_xEnumVARIANT.m_nIndex = 0;
	return S_OK;
}

STDMETHODIMP CComMenu::XSMenu::get_Item(VARIANT vIndex, ISMenu FAR* FAR* ppMenu)
{
	METHOD_PROLOGUE( CComMenu, SMenu )
	if ( pThis->m_hMenu == NULL ) return E_FAIL;

	VARIANT va;
	VariantInit( &va );

	if ( FAILED( VariantChangeType( &va, (VARIANT FAR*)&vIndex, 0, VT_I4 ) ) )
	{
		*ppMenu = NULL;
		return E_INVALIDARG;
	}

	*ppMenu = CComMenu::Wrap( pThis->m_hMenu, (UINT)va.lVal );

	return S_OK;
}

STDMETHODIMP CComMenu::XSMenu::get_Count(LONG FAR* pnCount)
{
	METHOD_PROLOGUE( CComMenu, SMenu )
	if ( pThis->m_hMenu == NULL ) return E_FAIL;
	*pnCount = GetMenuItemCount( pThis->m_hMenu );
	return S_OK;
}

STDMETHODIMP CComMenu::XSMenu::get_ItemType(SMenuType FAR* pnType)
{
	METHOD_PROLOGUE( CComMenu, SMenu )

	if ( pThis->m_hMenu )
	{
		*pnType = mnuMenu;
	}
	else
	{
		*pnType = ( GetMenuItemID( pThis->m_hParent, pThis->m_nPosition ) == ID_SEPARATOR )
			? mnuSeparator : mnuCommand;
	}

	return S_OK;
}

STDMETHODIMP CComMenu::XSMenu::get_CommandID(LONG FAR* pnCommandID)
{
	METHOD_PROLOGUE( CComMenu, SMenu )
	if ( pThis->m_hMenu != NULL ) return E_FAIL;
	*pnCommandID = (LONG)GetMenuItemID( pThis->m_hParent, pThis->m_nPosition );
	return S_OK;
}

STDMETHODIMP CComMenu::XSMenu::put_CommandID(LONG nCommandID)
{
	METHOD_PROLOGUE( CComMenu, SMenu )
	if ( pThis->m_hMenu != NULL ) return E_FAIL;

	MENUITEMINFO pItem = {};
	pItem.cbSize	= sizeof(pItem);
	pItem.fMask		= MIIM_ID;
	pItem.wID		= (WORD)nCommandID;

	SetMenuItemInfo( pThis->m_hParent, pThis->m_nPosition, TRUE, &pItem );

	return S_OK;
}

STDMETHODIMP CComMenu::XSMenu::get_Text(BSTR FAR* psText)
{
	METHOD_PROLOGUE( CComMenu, SMenu )
	if ( pThis->m_hParent == NULL ) return E_FAIL;

	CString str;
	GetMenuString( pThis->m_hParent, pThis->m_nPosition,
		str.GetBuffer( 256 ), 256, MF_BYPOSITION );
	str.ReleaseBuffer();
	*psText = CComBSTR( str ).Detach();

	return S_OK;
}

STDMETHODIMP CComMenu::XSMenu::put_Text(BSTR sText)
{
	METHOD_PROLOGUE( CComMenu, SMenu )
	if ( pThis->m_hParent == NULL ) return E_FAIL;

	CString strText( sText );

	MENUITEMINFO pItem = {};
	pItem.cbSize		= sizeof(pItem);
	pItem.fMask			= MIIM_TYPE;
	pItem.fType			= MFT_STRING;
	pItem.dwTypeData	= (LPTSTR)(LPCTSTR)strText;

	SetMenuItemInfo( pThis->m_hParent, pThis->m_nPosition, TRUE, &pItem );

	return S_OK;
}

STDMETHODIMP CComMenu::XSMenu::get_HotKey(BSTR FAR* /*psText*/)
{
	METHOD_PROLOGUE( CComMenu, SMenu )
	return E_NOTIMPL;
}

STDMETHODIMP CComMenu::XSMenu::put_HotKey(BSTR /*sText*/)
{
	METHOD_PROLOGUE( CComMenu, SMenu )
	return E_NOTIMPL;
}

STDMETHODIMP CComMenu::XSMenu::Remove()
{
	METHOD_PROLOGUE( CComMenu, SMenu )
	if ( pThis->m_hParent == NULL ) return E_FAIL;
	RemoveMenu( pThis->m_hParent, pThis->m_nPosition, MF_BYPOSITION );
	pThis->m_hParent = pThis->m_hMenu = NULL;
	return S_OK;
}

STDMETHODIMP CComMenu::XSMenu::InsertSeparator(LONG nPosition)
{
	METHOD_PROLOGUE( CComMenu, SMenu )

	if ( pThis->m_hMenu == NULL ) return E_FAIL;
	if ( nPosition == -1 ) nPosition = GetMenuItemCount( pThis->m_hMenu );
	::InsertMenu( pThis->m_hMenu, (UINT)nPosition, MF_BYPOSITION|MF_SEPARATOR,
		ID_SEPARATOR, NULL );

	return S_OK;
}

STDMETHODIMP CComMenu::XSMenu::InsertMenu(LONG nPosition, BSTR sText, ISMenu FAR* FAR* ppMenu)
{
	METHOD_PROLOGUE( CComMenu, SMenu )

	if ( pThis->m_hMenu == NULL ) return E_FAIL;
	if ( nPosition == -1 ) nPosition = GetMenuItemCount( pThis->m_hMenu );

	::InsertMenu( pThis->m_hMenu, (UINT)nPosition, MF_BYPOSITION|MF_POPUP,
		(UINT_PTR)CreatePopupMenu(), CString( sText ) );

	if ( ppMenu ) *ppMenu = CComMenu::Wrap( pThis->m_hMenu, nPosition );

	return S_OK;
}

STDMETHODIMP CComMenu::XSMenu::InsertCommand(LONG nPosition, LONG nCommandID, BSTR sText, ISMenu FAR* FAR* ppMenu)
{
	METHOD_PROLOGUE( CComMenu, SMenu )

	if ( pThis->m_hMenu == NULL ) return E_FAIL;
	if ( nPosition == -1 ) nPosition = GetMenuItemCount( pThis->m_hMenu );

	::InsertMenu( pThis->m_hMenu, (UINT)nPosition, MF_BYPOSITION|MF_STRING,
		(UINT)nCommandID, CString( sText ) );

	if ( ppMenu ) *ppMenu = CComMenu::Wrap( pThis->m_hMenu, nPosition );

	return S_OK;
}

STDMETHODIMP CComMenu::XSMenu::get_Position(LONG FAR* pnCommandID)
{
	METHOD_PROLOGUE( CComMenu, SMenu )

	if ( pThis->m_hParent == NULL ) return E_FAIL;

	if ( pnCommandID ) *pnCommandID = pThis->m_nPosition;

	return S_OK;
}

STDMETHODIMP CComMenu::XSMenu::get_Parent(ISMenu FAR* FAR* ppMenu)
{
	METHOD_PROLOGUE( CComMenu, SMenu )

	if ( pThis->m_hParent == NULL ) return E_FAIL;

	if ( ppMenu ) *ppMenu = CComMenu::Wrap( pThis->m_hParent );

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CComMenu::XEnumVARIANT enumerator

IMPLEMENT_UNKNOWN( CComMenu, EnumVARIANT )

STDMETHODIMP CComMenu::XEnumVARIANT::Next(ULONG celt, VARIANT FAR* rgvar, ULONG FAR* pceltFetched)
{
	METHOD_PROLOGUE( CComMenu, EnumVARIANT )

	if ( pceltFetched ) *pceltFetched = 0;
	else if ( celt > 1 ) return E_INVALIDARG;

	VariantInit( &rgvar[0] );

	if ( m_nIndex >= (UINT)GetMenuItemCount( pThis->m_hMenu ) ) return S_FALSE;

	rgvar[0].vt			= VT_DISPATCH;
	rgvar[0].pdispVal	= (IDispatch*)CComMenu::Wrap( pThis->m_hMenu, m_nIndex );

	m_nIndex++;
	if ( pceltFetched ) (*pceltFetched)++;

	return S_OK;
}

STDMETHODIMP CComMenu::XEnumVARIANT::Skip(ULONG celt)
{
    METHOD_PROLOGUE( CComMenu, EnumVARIANT )

	int nCount = GetMenuItemCount( pThis->m_hMenu );

	while ( celt-- && m_nIndex++ < (UINT)nCount );

    return ( celt == 0 ? S_OK : S_FALSE );
}

STDMETHODIMP CComMenu::XEnumVARIANT::Reset()
{
    METHOD_PROLOGUE( CComMenu, EnumVARIANT )
	m_nIndex = 0;
    return S_OK;
}

STDMETHODIMP CComMenu::XEnumVARIANT::Clone(IEnumVARIANT FAR* FAR* /*ppenum*/)
{
    METHOD_PROLOGUE( CComMenu, EnumVARIANT )
    return E_NOTIMPL;
}
