//
// MatchListView.cpp
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
#include "MatchObjects.h"
#include "QueryHit.h"
#include "Download.h"
#include "Downloads.h"
#include "Transfers.h"
#include "MatchListView.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CMatchListView, CComObject)

BEGIN_INTERFACE_MAP(CMatchListView, CComObject)
	INTERFACE_PART(CMatchListView, IID_IGenericView, GenericView)
	INTERFACE_PART(CMatchListView, IID_IEnumVARIANT, EnumVARIANT)
END_INTERFACE_MAP()


//////////////////////////////////////////////////////////////////////
// CMatchListView construction

CMatchListView::CMatchListView()
{
	EnableDispatch( IID_IGenericView );

	m_xEnumVARIANT.m_pos = NULL;
}

CMatchListView::CMatchListView(LPCTSTR pszName, CMatchList* pList)
	: m_sName( pszName )
{
	EnableDispatch( IID_IGenericView );

	CQuickLock oLock( pList->m_pSection );

	for ( POSITION pos = pList->m_pSelectedFiles.GetHeadPosition() ; pos ; )
	{
		m_pSelection.AddTail( *pList->m_pSelectedFiles.GetNext( pos ) );
	}

	for ( POSITION pos = pList->m_pSelectedHits.GetHeadPosition() ; pos ; )
	{
		m_pSelection.AddTail( *pList->m_pSelectedHits.GetNext( pos ) );
	}

	m_xEnumVARIANT.m_pos = m_pSelection.GetHeadPosition();
}

CMatchListView::CMatchListView(LPCTSTR pszName, CDownloads* pDownloads)
	: m_sName( pszName )
{
	EnableDispatch( IID_IGenericView );

	CQuickLock oLock( Transfers.m_pSection );

	for ( POSITION pos = pDownloads->GetIterator() ; pos ; )
	{
		const CDownload* pDownload = pDownloads->GetNext( pos );

		if ( pDownload->m_bSelected )
		{
			m_pSelection.AddTail( *pDownload );
		}
	}

	m_xEnumVARIANT.m_pos = m_pSelection.GetHeadPosition();
}

CMatchListView::~CMatchListView()
{
}

//////////////////////////////////////////////////////////////////////
// CMatchListView operations

IGenericView* CMatchListView::Attach(LPCTSTR pszName, CMatchList* pList)
{
	CMatchListView* pView = new CMatchListView( pszName, pList );
	return pView ? (IGenericView*)pView->GetInterface( IID_IGenericView, FALSE ) : NULL;
}

IGenericView* CMatchListView::Attach(LPCTSTR pszName, CDownloads* pDownloads)
{
	CMatchListView* pView = new CMatchListView( pszName, pDownloads );
	return pView ? (IGenericView*)pView->GetInterface( IID_IGenericView, FALSE ) : NULL;
}

//////////////////////////////////////////////////////////////////////
// CMatchListView IGenericView

IMPLEMENT_DISPATCH(CMatchListView, GenericView)

STDMETHODIMP CMatchListView::XGenericView::get_Name(BSTR FAR* psName)
{
	METHOD_PROLOGUE( CMatchListView, GenericView )

	if ( ! psName )
		return E_POINTER;

	*psName = CComBSTR( pThis->m_sName ).Detach();

	return S_OK;
}

STDMETHODIMP CMatchListView::XGenericView::get_Unknown(IUnknown FAR* FAR* /*ppUnknown*/)
{
	METHOD_PROLOGUE( CMatchListView, GenericView )
	return E_NOTIMPL;
}

STDMETHODIMP CMatchListView::XGenericView::get_Param(LONG FAR* /*pnParam*/)
{
	METHOD_PROLOGUE( CMatchListView, GenericView )
	return E_NOTIMPL;
}

STDMETHODIMP CMatchListView::XGenericView::get__NewEnum(IUnknown FAR* FAR* ppEnum)
{
	METHOD_PROLOGUE( CMatchListView, GenericView )

	if ( ! ppEnum )
		return E_POINTER;

	*ppEnum = &pThis->m_xEnumVARIANT;

	pThis->m_xEnumVARIANT.m_pos = pThis->m_pSelection.GetHeadPosition();

	AddRef();

	return S_OK;
}

STDMETHODIMP CMatchListView::XGenericView::get_Item(VARIANT vIndex, VARIANT FAR* pvItem)
{
	METHOD_PROLOGUE( CMatchListView, GenericView )

	if ( ! pvItem )
		return E_POINTER;

	VariantClear( pvItem );

	CComVariant va;
	if ( FAILED( va.ChangeType( VT_I4, &vIndex ) ) )
		return E_INVALIDARG;

	if ( va.lVal < 0 || va.lVal >= pThis->m_pSelection.GetCount() )
		return S_OK;

	for ( POSITION pos = pThis->m_pSelection.GetHeadPosition(); pos; )
	{
		BOOL bThis = ( va.lVal-- == 0 );

		CShareazaFile& pFile = pThis->m_pSelection.GetNext( pos );
		if ( bThis )
		{
			pvItem->vt = VT_DISPATCH;
			pvItem->punkVal = pFile.GetDispatch( TRUE );
			break;
		}
	}

	return S_OK;
}

STDMETHODIMP CMatchListView::XGenericView::get_Count(LONG FAR* pnCount)
{
	METHOD_PROLOGUE( CMatchListView, GenericView )
	
	if ( ! pnCount )
		return E_POINTER;

	*pnCount = static_cast< LONG >( pThis->m_pSelection.GetCount() );

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CMatchListView IEnumVARIANT enumerator

IMPLEMENT_UNKNOWN(CMatchListView, EnumVARIANT)

STDMETHODIMP CMatchListView::XEnumVARIANT::Next(ULONG celt, VARIANT FAR* rgvar, ULONG FAR* pceltFetched)
{
	METHOD_PROLOGUE( CMatchListView, EnumVARIANT )

	if ( ! rgvar )
		return E_POINTER;

	ULONG nCount = 0;
	for ( ULONG i = 0; i < celt; ++i )
	{
		VariantInit( &rgvar[ i ] );
		if ( m_pos )
		{
			rgvar[ i ].vt = VT_DISPATCH;
			rgvar[ i ].punkVal = pThis->m_pSelection.GetNext( m_pos ).GetDispatch( TRUE );
			++nCount;
		}
	}

	if ( pceltFetched )
		*pceltFetched = nCount;

	return ( celt == nCount ) ? S_OK : S_FALSE;
}

STDMETHODIMP CMatchListView::XEnumVARIANT::Skip(ULONG celt)
{
    METHOD_PROLOGUE( CMatchListView, EnumVARIANT )

	ULONG nCount = 0;
	for ( ULONG i = 0; i < celt && m_pos; ++i )
	{
		pThis->m_pSelection.GetNext( m_pos );
		++nCount;
	}

    return ( celt == nCount ) ? S_OK : S_FALSE;
}

STDMETHODIMP CMatchListView::XEnumVARIANT::Reset()
{
    METHOD_PROLOGUE( CMatchListView, EnumVARIANT )

	m_pos = pThis->m_pSelection.GetHeadPosition();

    return S_OK;
}

STDMETHODIMP CMatchListView::XEnumVARIANT::Clone(IEnumVARIANT FAR* FAR* /*ppenum*/)
{
    return E_NOTIMPL;
}
