//
// MatchListView.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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
#include "MatchObjects.h"
#include "QueryHit.h"
#include "MatchListView.h"

#include "TigerTree.h"
#include "ED2K.h"
#include "SHA.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

BEGIN_INTERFACE_MAP(CMatchListView, CComObject)
	INTERFACE_PART(CMatchListView, IID_IGenericView, GenericView)
	INTERFACE_PART(CMatchListView, IID_IEnumVARIANT, EnumVARIANT)
END_INTERFACE_MAP()


//////////////////////////////////////////////////////////////////////
// CMatchListView construction

CMatchListView::CMatchListView(LPCTSTR pszName, CMatchList* pList)
{
	EnableDispatch( IID_IGenericView );
	
	m_sName = pszName;
	m_pList = pList;
	m_pList->m_pSection.Lock();
	
	for ( POSITION pos = m_pList->m_pSelectedFiles.GetHeadPosition() ; pos ; )
	{
		m_pSelection.AddTail( m_pList->m_pSelectedFiles.GetNext( pos ) );
	}
	
	for ( pos = m_pList->m_pSelectedHits.GetHeadPosition() ; pos ; )
	{
		m_pSelection.AddTail( m_pList->m_pSelectedHits.GetNext( pos ) );
	}
}

CMatchListView::~CMatchListView()
{
	m_pList->m_pSection.Unlock();
}

//////////////////////////////////////////////////////////////////////
// CMatchListView operations

IGenericView* CMatchListView::Attach(LPCTSTR pszName, CMatchList* pList)
{
	CMatchListView* pView = new CMatchListView( pszName, pList );
	return (IGenericView*)pView->GetInterface( IID_IGenericView, FALSE );
}

POSITION CMatchListView::GetIterator() const
{
	return m_pSelection.GetHeadPosition();
}

int CMatchListView::GetCount() const
{
	return m_pSelection.GetCount();
}

void CMatchListView::GetNext(POSITION& pos, CMatchFile** ppFile, CQueryHit** ppHit) const
{
	LPVOID pItem = (LPVOID)( pos != NULL ? m_pSelection.GetNext( pos ) : NULL );
	
	if ( ppFile != NULL )
	{
		if ( m_pList->m_pSelectedFiles.Find( pItem ) != NULL )
		{
			*ppFile = (CMatchFile*)pItem;
		}
	}
	else
	{
		*ppFile = NULL;
	}
	
	if ( ppHit != NULL )
	{
		if ( m_pList->m_pSelectedHits.Find( pItem ) != NULL )
		{
			*ppHit = (CQueryHit*)pItem;
		}
	}
	else
	{
		*ppHit = NULL;
	}
}

void CMatchListView::GetNext(POSITION& pos, VARIANT* pVar) const
{
	CMatchFile* pFile;
	CQueryHit* pHit;
	
	GetNext( pos, &pFile, &pHit );
	if ( pVar == NULL ) return;
	
	CManagedTiger *pTiger;
	CManagedSHA1 *pSHA1;
	CManagedED2K *pED2K;
	
	if ( pFile != NULL )
	{
		pTiger = &pFile->m_oTiger;
		pSHA1 = &pFile->m_oSHA1;
		pED2K = &pFile->m_oED2K;
	}
	else
	{
		pTiger = &pHit->m_oTiger;
		pSHA1 = &pHit->m_oSHA1;
		pED2K = &pHit->m_oED2K;
	}
	
	CString strURN;
	VariantClear( pVar );
	
	if ( pSHA1->IsValid() && pTiger->IsValid() )
	{
		strURN	= _T("urn:bitprint:")
			+ pSHA1->ToString() + '.'
			+ pTiger->ToString();
	}
	else if ( pSHA1->IsValid() )
	{
		strURN = pSHA1->ToURN();
	}
	else if ( pTiger->IsValid() )
	{
		strURN = pTiger->ToURN();
	}
	else if ( pED2K->IsValid() )
	{
		strURN = pED2K->ToURN();
	}
	else
	{
		return;
	}
	
	pVar->vt		= VT_BSTR;
	pVar->bstrVal	= strURN.AllocSysString();
}

//////////////////////////////////////////////////////////////////////
// CMatchListView IGenericView

IMPLEMENT_DISPATCH(CMatchListView, GenericView)

STDMETHODIMP CMatchListView::XGenericView::get_Name(BSTR FAR* psName)
{
	METHOD_PROLOGUE( CMatchListView, GenericView )
	pThis->m_sName.SetSysString( psName );
	return S_OK;
}

STDMETHODIMP CMatchListView::XGenericView::get_Unknown(IUnknown FAR* FAR* ppUnknown)
{
	METHOD_PROLOGUE( CMatchListView, GenericView )
	return E_NOTIMPL;
}

STDMETHODIMP CMatchListView::XGenericView::get_Param(LONG FAR* pnParam)
{
	METHOD_PROLOGUE( CMatchListView, GenericView )
	return E_NOTIMPL;
}

STDMETHODIMP CMatchListView::XGenericView::get__NewEnum(IUnknown FAR* FAR* ppEnum)
{
	METHOD_PROLOGUE( CMatchListView, GenericView )
	*ppEnum = &pThis->m_xEnumVARIANT;
	pThis->m_xEnumVARIANT.m_pos = pThis->GetIterator();
	AddRef();
	return S_OK;
}

STDMETHODIMP CMatchListView::XGenericView::get_Item(VARIANT vIndex, VARIANT FAR* pvItem)
{
	METHOD_PROLOGUE( CMatchListView, GenericView )
	
	VARIANT va;
	VariantInit( &va );
	VariantClear( pvItem );
	
	if ( FAILED( VariantChangeType( &va, (VARIANT FAR*)&vIndex, 0, VT_I4 ) ) ) return E_INVALIDARG;
	
	if ( va.lVal < 0 || va.lVal >= pThis->GetCount() ) return S_OK;
	
	for ( POSITION pos = pThis->GetIterator() ; pos ; )
	{
		BOOL bThis = ( va.lVal-- == 0 );
		
		pThis->GetNext( pos, bThis ? pvItem : NULL );
		if ( bThis ) break;
	}
	
	return S_OK;
}

STDMETHODIMP CMatchListView::XGenericView::get_Count(LONG FAR* pnCount)
{
	METHOD_PROLOGUE( CMatchListView, GenericView )
	*pnCount = pThis->GetCount();
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CMatchListView IEnumVARIANT enumerator

IMPLEMENT_UNKNOWN(CMatchListView, EnumVARIANT)

STDMETHODIMP CMatchListView::XEnumVARIANT::Next(ULONG celt, VARIANT FAR* rgvar, ULONG FAR* pceltFetched)
{
	METHOD_PROLOGUE( CMatchListView, EnumVARIANT )
	
	if ( pceltFetched ) *pceltFetched = 0;
	else if ( celt > 1 ) return E_INVALIDARG;
	
	if ( m_pos == NULL ) return S_FALSE;
	
	VariantInit( &rgvar[0] );
	pThis->GetNext( m_pos, rgvar );
	
	if ( pceltFetched ) (*pceltFetched)++;
	
	return S_OK;
}

STDMETHODIMP CMatchListView::XEnumVARIANT::Skip(ULONG celt) 
{
    METHOD_PROLOGUE( CMatchListView, EnumVARIANT )
	
	while ( celt-- && m_pos ) pThis->GetNext( m_pos, NULL, NULL );
	
    return ( celt == 0 ? S_OK : S_FALSE );
}

STDMETHODIMP CMatchListView::XEnumVARIANT::Reset()
{
    METHOD_PROLOGUE( CMatchListView, EnumVARIANT )
	m_pos = pThis->GetIterator();
    return S_OK;
}

STDMETHODIMP CMatchListView::XEnumVARIANT::Clone(IEnumVARIANT FAR* FAR* ppenum) 
{
    return E_NOTIMPL;
}
