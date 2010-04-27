//
// ComToolbar.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "Settings.h"
#include "Application.h"
#include "ComToolbar.h"
#include "CtrlCoolBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CComToolbar, CComObject)
	//{{AFX_MSG_MAP(CComToolbar)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_INTERFACE_MAP(CComToolbar, CComObject)
	INTERFACE_PART(CComToolbar, IID_ISToolbar, SToolbar)
	INTERFACE_PART(CComToolbar, IID_ISToolbarItem, SToolbarItem)
	INTERFACE_PART(CComToolbar, IID_IEnumVARIANT, EnumVARIANT)
END_INTERFACE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CComToolbar construction

CComToolbar::CComToolbar(CCoolBarCtrl* pBar, CCoolBarItem* pItem)
{
	m_pBar	= pBar;
	m_pItem	= pItem;

	if ( pItem )
		EnableDispatch( IID_ISToolbarItem );
	else
		EnableDispatch( IID_ISToolbar );
}

CComToolbar::~CComToolbar()
{
}

/////////////////////////////////////////////////////////////////////////////
// CComToolbar operations

ISToolbar* CComToolbar::Wrap(CCoolBarCtrl* pBar)
{
	CComToolbar* pWrap = new CComToolbar( pBar, NULL );
	return (ISToolbar*)pWrap->GetInterface( IID_ISToolbar, FALSE );
}

ISToolbarItem* CComToolbar::Wrap(CCoolBarCtrl* pBar, CCoolBarItem* pItem)
{
	CComToolbar* pWrap = new CComToolbar( pBar, pItem );
	return (ISToolbarItem*)pWrap->GetInterface( IID_ISToolbarItem, FALSE );
}

/////////////////////////////////////////////////////////////////////////////
// CComToolbar ISMenu

IMPLEMENT_DISPATCH(CComToolbar, SToolbar)

STDMETHODIMP CComToolbar::XSToolbar::get_Application(IApplication FAR* FAR* ppApplication)
{
	METHOD_PROLOGUE( CComToolbar, SToolbar )
	return CApplication::GetApp( ppApplication );
}

STDMETHODIMP CComToolbar::XSToolbar::get_UserInterface(IUserInterface FAR* FAR* ppUserInterface)
{
	METHOD_PROLOGUE( CComToolbar, SToolbar )
	return CApplication::GetUI( ppUserInterface );
}

STDMETHODIMP CComToolbar::XSToolbar::get__NewEnum(IUnknown FAR* FAR* ppEnum)
{
	METHOD_PROLOGUE( CComToolbar, SToolbar )
	if ( pThis->m_pBar == NULL ) return E_UNEXPECTED;

	AddRef();
	*ppEnum = &pThis->m_xEnumVARIANT;
	pThis->m_xEnumVARIANT.m_nIndex = 0;

	return S_OK;
}

STDMETHODIMP CComToolbar::XSToolbar::get_Item(VARIANT vIndex, ISToolbarItem FAR* FAR* ppItem)
{
	METHOD_PROLOGUE( CComToolbar, SToolbar )
	if ( pThis->m_pBar == NULL ) return E_UNEXPECTED;

	VARIANT va;
	VariantInit( &va );

	if ( FAILED( VariantChangeType( &va, (VARIANT FAR*)&vIndex, 0, VT_I4 ) ) )
	{
		*ppItem = NULL;
		return E_FAIL;
	}

	if ( va.lVal >= 0 && va.lVal < pThis->m_pBar->GetCount() )
	{
		*ppItem = CComToolbar::Wrap( pThis->m_pBar, pThis->m_pBar->GetIndex( va.lVal ) );
		return S_OK;
	}

	if ( CCoolBarItem* pItem = pThis->m_pBar->GetID( va.lVal ) )
	{
		*ppItem = CComToolbar::Wrap( pThis->m_pBar, pItem );
		return S_OK;
	}

	*ppItem = NULL;

	return S_OK;
}

STDMETHODIMP CComToolbar::XSToolbar::get_Count(LONG FAR* pnCount)
{
	METHOD_PROLOGUE( CComToolbar, SToolbar )
	if ( pThis->m_pBar == NULL ) return E_UNEXPECTED;
	*pnCount = static_cast< LONG >( pThis->m_pBar->GetCount() );
	return S_OK;
}

STDMETHODIMP CComToolbar::XSToolbar::InsertSeparator(LONG /*nPosition*/)
{
	METHOD_PROLOGUE( CComToolbar, SToolbar )
	if ( pThis->m_pBar == NULL ) return E_UNEXPECTED;
	pThis->m_pBar->Add( ID_SEPARATOR );
	return S_OK;
}

STDMETHODIMP CComToolbar::XSToolbar::InsertButton(LONG nPosition, LONG nCommandID, BSTR sText, ISToolbarItem FAR* FAR* ppItem)
{
	METHOD_PROLOGUE( CComToolbar, SToolbar )
	if ( pThis->m_pBar == NULL ) return E_UNEXPECTED;

	CCoolBarItem* pItem = pThis->m_pBar->Add( (UINT)nCommandID, CString( sText ), (int)nPosition );
	if ( ppItem ) *ppItem = CComToolbar::Wrap( pThis->m_pBar, pItem );

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CComToolbar ISToolbarItem

IMPLEMENT_DISPATCH(CComToolbar, SToolbarItem)

STDMETHODIMP CComToolbar::XSToolbarItem::get_Application(IApplication FAR* FAR* ppApplication)
{
	METHOD_PROLOGUE( CComToolbar, SToolbarItem )
	return CApplication::GetApp( ppApplication );
}

STDMETHODIMP CComToolbar::XSToolbarItem::get_UserInterface(IUserInterface FAR* FAR* ppUserInterface)
{
	METHOD_PROLOGUE( CComToolbar, SToolbarItem )
	return CApplication::GetUI( ppUserInterface );
}

STDMETHODIMP CComToolbar::XSToolbarItem::get_Toolbar(ISToolbar FAR* FAR* ppToolbar)
{
	METHOD_PROLOGUE( CComToolbar, SToolbarItem )
	if ( pThis->m_pBar == NULL ) return E_UNEXPECTED;
	*ppToolbar = CComToolbar::Wrap( pThis->m_pBar );
	return E_NOTIMPL;
}

STDMETHODIMP CComToolbar::XSToolbarItem::get_ItemType(SToolbarType FAR* pnType)
{
	METHOD_PROLOGUE( CComToolbar, SToolbarItem )
	if ( pThis->m_pItem == NULL ) return E_UNEXPECTED;

	if ( pThis->m_pItem->m_nCtrlID )
		*pnType = tbControl;
	else if ( pThis->m_pItem->m_nID == ID_SEPARATOR )
		*pnType = tbSeparator;
	else
		*pnType = tbButton;

	return S_OK;
}

STDMETHODIMP CComToolbar::XSToolbarItem::get_CommandID(LONG FAR* pnCommandID)
{
	METHOD_PROLOGUE( CComToolbar, SToolbarItem )
	if ( pThis->m_pItem == NULL ) return E_UNEXPECTED;
	*pnCommandID = (LONG)pThis->m_pItem->m_nID;
	return S_OK;
}

STDMETHODIMP CComToolbar::XSToolbarItem::put_CommandID(LONG nCommandID)
{
	METHOD_PROLOGUE( CComToolbar, SToolbarItem )
	if ( pThis->m_pItem == NULL ) return E_UNEXPECTED;
	pThis->m_pItem->m_nID = (UINT)nCommandID;
	pThis->m_pItem->SetImage( pThis->m_pItem->m_nID );
	return S_OK;
}

STDMETHODIMP CComToolbar::XSToolbarItem::get_Text(BSTR FAR* psText)
{
	METHOD_PROLOGUE( CComToolbar, SToolbarItem )
	if ( pThis->m_pItem == NULL ) return E_UNEXPECTED;
	*psText = CComBSTR( pThis->m_pItem->m_sText ).Detach();
	return S_OK;
}

STDMETHODIMP CComToolbar::XSToolbarItem::put_Text(BSTR sText)
{
	METHOD_PROLOGUE( CComToolbar, SToolbarItem )
	if ( pThis->m_pItem == NULL ) return E_UNEXPECTED;
	pThis->m_pItem->SetText( CString( sText ) );
	return S_OK;
}

STDMETHODIMP CComToolbar::XSToolbarItem::Remove()
{
	METHOD_PROLOGUE( CComToolbar, SToolbarItem )
	if ( pThis->m_pItem == NULL ) return E_UNEXPECTED;
	return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////
// CComToolbar::XEnumVARIANT enumerator

IMPLEMENT_UNKNOWN( CComToolbar, EnumVARIANT )

STDMETHODIMP CComToolbar::XEnumVARIANT::Next(ULONG celt, VARIANT FAR* rgvar, ULONG FAR* pceltFetched)
{
	METHOD_PROLOGUE( CComToolbar, EnumVARIANT )

	if ( pceltFetched ) *pceltFetched = 0;
	else if ( celt > 1 ) return E_INVALIDARG;

	VariantInit( &rgvar[0] );

	if ( m_nIndex >= (UINT)pThis->m_pBar->GetCount() ) return S_FALSE;

	rgvar[0].vt			= VT_DISPATCH;
	rgvar[0].pdispVal	= (IDispatch*)CComToolbar::Wrap(
		pThis->m_pBar, pThis->m_pBar->GetIndex( m_nIndex ) );

	m_nIndex++;
	if ( pceltFetched ) (*pceltFetched)++;

	return S_OK;
}

STDMETHODIMP CComToolbar::XEnumVARIANT::Skip(ULONG celt)
{
    METHOD_PROLOGUE( CComToolbar, EnumVARIANT )

	UINT nCount = static_cast< UINT >( pThis->m_pBar->GetCount() );

	while ( celt-- && m_nIndex++ < nCount );

    return ( celt == 0 ? S_OK : S_FALSE );
}

STDMETHODIMP CComToolbar::XEnumVARIANT::Reset()
{
    METHOD_PROLOGUE( CComToolbar, EnumVARIANT )
	m_nIndex = 0;
    return S_OK;
}

STDMETHODIMP CComToolbar::XEnumVARIANT::Clone(IEnumVARIANT FAR* FAR* /*ppenum*/)
{
    METHOD_PROLOGUE( CComToolbar, EnumVARIANT )
    return E_NOTIMPL;
}

