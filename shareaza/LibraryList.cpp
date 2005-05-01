//
// LibraryList.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2005.
// This file is part of SHAREAZA (www.shareaza.com)
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
#include "Library.h"
#include "LibraryList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

BEGIN_INTERFACE_MAP(CLibraryList, CComObject)
	INTERFACE_PART(CLibraryList, IID_IGenericView, GenericView)
	INTERFACE_PART(CLibraryList, IID_IEnumVARIANT, EnumVARIANT)
END_INTERFACE_MAP()


//////////////////////////////////////////////////////////////////////
// CLibraryList construction

CLibraryList::CLibraryList(int nBlockSize)
{
	m_pList		= NULL;
	m_nCount	= 0;
	m_nBuffer	= 0;
	m_nBlock	= nBlockSize;
}

CLibraryList::~CLibraryList()
{
	if ( m_pList != NULL ) free( m_pList );
}

//////////////////////////////////////////////////////////////////////
// CLibraryList file access

CLibraryFile* CLibraryList::GetNextFile(POSITION& pos) const
{
	return Library.LookupFile( GetNext( pos ) );
}

//////////////////////////////////////////////////////////////////////
// CLibraryList list merging

int CLibraryList::Merge(CLibraryList* pList)
{
	int nCount = 0;

	if ( pList == NULL ) return 0;

	for ( POSITION pos = pList->GetIterator() ; pos ; )
	{
		DWORD nItem = pList->GetNext( pos );

		if ( Find( nItem ) == NULL )
		{
			AddTail( nItem );
			nCount++;
		}
	}

	return nCount;
}

//////////////////////////////////////////////////////////////////////
// CLibraryList IGenericView

IMPLEMENT_DISPATCH(CLibraryList, GenericView)

STDMETHODIMP CLibraryList::XGenericView::get_Name(BSTR FAR* psName)
{
	METHOD_PROLOGUE( CLibraryList, GenericView )
	CString strName( _T("CLibraryFileView") );
	strName.SetSysString( psName );
	return S_OK;
}

STDMETHODIMP CLibraryList::XGenericView::get_Unknown(IUnknown FAR* FAR* ppUnknown)
{
	METHOD_PROLOGUE( CLibraryList, GenericView )
	return E_NOTIMPL;
}

STDMETHODIMP CLibraryList::XGenericView::get_Param(LONG FAR* pnParam)
{
	METHOD_PROLOGUE( CLibraryList, GenericView )
	return E_NOTIMPL;
}

STDMETHODIMP CLibraryList::XGenericView::get__NewEnum(IUnknown FAR* FAR* ppEnum)
{
	METHOD_PROLOGUE( CLibraryList, GenericView )
	*ppEnum = &pThis->m_xEnumVARIANT;
	pThis->m_xEnumVARIANT.m_pos = pThis->GetHeadPosition();
	AddRef();
	return S_OK;
}

STDMETHODIMP CLibraryList::XGenericView::get_Item(VARIANT vIndex, VARIANT FAR* pvItem)
{
	METHOD_PROLOGUE( CLibraryList, GenericView )

	VARIANT va;
	VariantInit( &va );
	VariantClear( pvItem );

	if ( FAILED( VariantChangeType( &va, (VARIANT FAR*)&vIndex, 0, VT_I4 ) ) ) return E_INVALIDARG;

	if ( va.lVal < 0 || va.lVal >= pThis->GetCount() ) return S_OK;

	for ( POSITION pos = pThis->GetHeadPosition() ; pos ; )
	{
		DWORD nItem = pThis->GetNext( pos );

		if ( va.lVal-- == 0 )
		{
			pvItem->vt		= VT_I4;
			pvItem->lVal	= nItem;
			break;
		}
	}

	return S_OK;
}

STDMETHODIMP CLibraryList::XGenericView::get_Count(LONG FAR* pnCount)
{
	METHOD_PROLOGUE( CLibraryList, GenericView )
	*pnCount = pThis->GetCount();
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CLibraryList IEnumVARIANT enumerator

IMPLEMENT_UNKNOWN(CLibraryList, EnumVARIANT)

STDMETHODIMP CLibraryList::XEnumVARIANT::Next(ULONG celt, VARIANT FAR* rgvar, ULONG FAR* pceltFetched)
{
	METHOD_PROLOGUE( CLibraryList, EnumVARIANT )

	if ( pceltFetched ) *pceltFetched = 0;
	else if ( celt > 1 ) return E_INVALIDARG;

	if ( m_pos == NULL ) return S_FALSE;

	VariantInit( &rgvar[0] );
	rgvar[0].vt		= VT_I4;
	rgvar[0].lVal	= pThis->GetNext( m_pos );

	if ( pceltFetched ) (*pceltFetched)++;

	return S_OK;
}

STDMETHODIMP CLibraryList::XEnumVARIANT::Skip(ULONG celt)
{
    METHOD_PROLOGUE( CLibraryList, EnumVARIANT )

	while ( celt-- && m_pos ) pThis->GetNext( m_pos );

    return ( celt == 0 ? S_OK : S_FALSE );
}

STDMETHODIMP CLibraryList::XEnumVARIANT::Reset()
{
    METHOD_PROLOGUE( CLibraryList, EnumVARIANT )
	m_pos = pThis->GetHeadPosition();
    return S_OK;
}

STDMETHODIMP CLibraryList::XEnumVARIANT::Clone(IEnumVARIANT FAR* FAR* ppenum)
{
    return E_NOTIMPL;
}
