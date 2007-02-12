//
// ComRichBox.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "CtrlRichTaskBox.h"
#include "RichDocument.h"
#include "RichElement.h"
#include "ComRichBox.h"
#include "CtrlTaskPanel.h"
#include "XMLCOM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CComRichBox, CComObject)
	//{{AFX_MSG_MAP(CComRichBox)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_INTERFACE_MAP(CComRichBox, CComObject)
	INTERFACE_PART(CComRichBox, IID_ISRichBox, SRichBox)
	INTERFACE_PART(CComRichBox, IID_ISRichDocument, SRichDocument)
	INTERFACE_PART(CComRichBox, IID_IEnumVARIANT, EnumVARIANT)
	INTERFACE_PART(CComRichBox, IID_ISRichItem, SRichElement)
END_INTERFACE_MAP()

CComRichBox::CComRichBox(CRichTaskBox* pTaskBox, CRichDocument* pDocument, CRichElement* pElement)
: m_pTaskBox( pTaskBox )
, m_pDocument( pDocument )
, m_pElement( pElement )
{
	if ( pElement && pDocument )
		EnableDispatch( IID_ISRichItem );
	else if ( pDocument )
		EnableDispatch( IID_ISRichDocument );
	else
		EnableDispatch( IID_ISRichBox );
}

CComRichBox::~CComRichBox()
{
}

/////////////////////////////////////////////////////////////////////////////
// CComRichBox operations

ISRichBox* CComRichBox::Wrap(CRichTaskBox* pTaskBox)
{
	CComRichBox* pWrap = new CComRichBox( pTaskBox, NULL, NULL );
	return (ISRichBox*)pWrap->GetInterface( IID_ISRichBox, FALSE );
}

ISRichDocument* CComRichBox::Wrap(CRichTaskBox* pTaskBox, CRichDocument* pDocument)
{
	CComRichBox* pWrap = new CComRichBox( pTaskBox, pDocument, NULL );
	return (ISRichDocument*)pWrap->GetInterface( IID_ISRichDocument, FALSE );
}

ISRichItem* CComRichBox::Wrap(CRichTaskBox* pTaskBox, CRichDocument* pDocument, CRichElement* pElement)
{
	CComRichBox* pWrap = new CComRichBox( pTaskBox, pDocument, pElement );
	return (ISRichItem*)pWrap->GetInterface( IID_ISRichItem, FALSE );
}

BOOL CComRichBox::LoadXMLStyles(CXMLElement* pParent)
{
	return m_pDocument->LoadXMLStyles( pParent );
}

BOOL CComRichBox::LoadXMLColour(CXMLElement* pXML, LPCTSTR strName, COLORREF* pnColour)
{
	return m_pDocument->LoadXMLColour( pXML, strName, pnColour );
}

/////////////////////////////////////////////////////////////////////////////
// CComRichBox ISRichBox

IMPLEMENT_DISPATCH(CComRichBox, SRichBox)

STDMETHODIMP CComRichBox::XSRichBox::get_Application(IApplication FAR* FAR* ppApplication)
{
	METHOD_PROLOGUE( CComRichBox, SRichBox )
	*ppApplication = Application.GetApp();
	return S_OK;
}

STDMETHODIMP CComRichBox::XSRichBox::get_UserInterface(IUserInterface FAR* FAR* ppUserInterface)
{
	METHOD_PROLOGUE( CComRichBox, SRichBox )
	*ppUserInterface = Application.GetUI();
	return S_OK;
}

STDMETHODIMP CComRichBox::XSRichBox::Create(HWND hPanel, BSTR bsCaption, INT nIcon)
{
	METHOD_PROLOGUE( CComRichBox, SRichBox )
	if ( pThis->m_pTaskBox == NULL || hPanel == NULL ) return E_UNEXPECTED;
	CTaskPanel* pPanel = reinterpret_cast< CTaskPanel* >(CWnd::FromHandle( hPanel ));
	pThis->m_pTaskBox->Create( pPanel, bsCaption, nIcon );
	return S_OK;
}

STDMETHODIMP CComRichBox::XSRichBox::get_Document(ISRichDocument FAR* FAR* ppDocument)
{
	METHOD_PROLOGUE( CComRichBox, SRichBox )
	if ( pThis->m_pDocument == NULL ) return E_UNEXPECTED;
	*ppDocument = CComRichBox::Wrap( pThis->m_pTaskBox, pThis->m_pDocument );
	return S_OK;
}

STDMETHODIMP CComRichBox::XSRichBox::put_Document(ISRichDocument FAR* pDocument)
{
	METHOD_PROLOGUE( CComRichBox, SRichBox )
	if ( pDocument == NULL ) return E_UNEXPECTED;
	INTERFACE_TO_CLASS(CComRichBox, SRichBox, pDocument, pWrapper)
	pThis->m_pDocument = pWrapper->m_pDocument;
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CComRichBox ISRichDocument

IMPLEMENT_DISPATCH(CComRichBox, SRichDocument)

STDMETHODIMP CComRichBox::XSRichDocument::get_Application(IApplication FAR* FAR* ppApplication)
{
	METHOD_PROLOGUE( CComRichBox, SRichDocument )
	*ppApplication = Application.GetApp();
	return S_OK;
}

STDMETHODIMP CComRichBox::XSRichDocument::get_UserInterface(IUserInterface FAR* FAR* ppUserInterface)
{
	METHOD_PROLOGUE( CComRichBox, SRichDocument )
	*ppUserInterface = Application.GetUI();
	return S_OK;
}

STDMETHODIMP CComRichBox::XSRichDocument::get__NewEnum(IUnknown FAR* FAR* ppEnum)
{
	METHOD_PROLOGUE( CComRichBox, SRichDocument )
	if ( pThis->m_pDocument == NULL ) return E_UNEXPECTED;

	AddRef();
	*ppEnum = &pThis->m_xEnumVARIANT;
	pThis->m_xEnumVARIANT.m_nIndex = 0;

	return S_OK;
}

STDMETHODIMP CComRichBox::XSRichDocument::get_Item(VARIANT vIndex, ISRichItem FAR* FAR* ppItem)
{
	METHOD_PROLOGUE( CComRichBox, SRichDocument )
	if ( pThis->m_pDocument == NULL ) return E_UNEXPECTED;

	VARIANT va;
	VariantInit( &va );

	if ( FAILED( VariantChangeType( &va, (VARIANT FAR*)&vIndex, 0, VT_I4 ) ) )
	{
		*ppItem = NULL;
		return E_FAIL;
	}

	if ( va.lVal >= 0 && va.lVal < pThis->m_pDocument->GetCount() )
	{
		*ppItem = CComRichBox::Wrap( pThis->m_pTaskBox, pThis->m_pDocument, pThis->m_pDocument->GetIndex( va.lVal ) );
		return S_OK;
	}

	*ppItem = NULL;

	return S_OK;
}

STDMETHODIMP CComRichBox::XSRichDocument::get_Count(LONG FAR* pnCount)
{
	METHOD_PROLOGUE( CComRichBox, SRichDocument )
	if ( pThis->m_pDocument == NULL ) return E_UNEXPECTED;
	*pnCount = static_cast< LONG >( pThis->m_pDocument->GetCount() );
	return S_OK;
}

STDMETHODIMP CComRichBox::XSRichDocument::Add(INT nType, BSTR strText, BSTR strLink, LONG nFlags, INT nGroup, ISRichItem** ppItem)
{
	METHOD_PROLOGUE( CComRichBox, SRichDocument )
	if ( pThis->m_pDocument == NULL ) return E_UNEXPECTED;

	*ppItem = NULL;
	CRichElement* pElement = pThis->m_pDocument->Add( nType, strText, strLink, (DWORD)nFlags, nGroup );
	if ( pElement )
		*ppItem = CComRichBox::Wrap( pThis->m_pTaskBox, pThis->m_pDocument, pElement );

	return S_OK;
}

STDMETHODIMP CComRichBox::XSRichDocument::RemoveAll()
{
	METHOD_PROLOGUE( CComRichBox, SRichDocument )
	if ( pThis->m_pDocument == NULL ) return E_UNEXPECTED;
	pThis->m_pDocument->Clear();
	return S_OK;
}

STDMETHODIMP CComRichBox::XSRichDocument::ShowGroup(INT nGroup, VARIANT_BOOL bShow)
{
	METHOD_PROLOGUE( CComRichBox, SRichDocument )
	if ( pThis->m_pDocument == NULL ) return E_UNEXPECTED;
	pThis->m_pDocument->ShowGroup( nGroup, bShow == VARIANT_TRUE );
	return S_OK;
}

STDMETHODIMP CComRichBox::XSRichDocument::ShowGroupRange(INT nMin, INT nMax, VARIANT_BOOL bShow)
{
	METHOD_PROLOGUE( CComRichBox, SRichDocument )
	if ( pThis->m_pDocument == NULL ) return E_UNEXPECTED;
	pThis->m_pDocument->ShowGroupRange( nMin, nMax, bShow == VARIANT_TRUE );
	return S_OK;
}

STDMETHODIMP CComRichBox::XSRichDocument::CreateFonts(BSTR strFaceName, INT nSize)
{
	METHOD_PROLOGUE( CComRichBox, SRichDocument )
	if ( pThis->m_pDocument == NULL ) return E_UNEXPECTED;
	pThis->m_pDocument->CreateFonts( strFaceName, nSize );
	return S_OK;
}

STDMETHODIMP CComRichBox::XSRichDocument::LoadXMLStyles(ISXMLElement* pParent)
{
	METHOD_PROLOGUE( CComRichBox, SRichDocument )
	if ( pThis->m_pDocument == NULL ) return E_UNEXPECTED;
	CXMLElement* pElement = CXMLCOM::Unwrap( pParent );
	if ( pElement )
		pThis->LoadXMLStyles( pElement );
	return S_OK;
}

STDMETHODIMP CComRichBox::XSRichDocument::LoadXMLColour(ISXMLElement* pXML, BSTR strName, LONG* pnColour)
{
	METHOD_PROLOGUE( CComRichBox, SRichDocument )
	if ( pThis->m_pDocument == NULL || pnColour == NULL ) return E_UNEXPECTED;
	CXMLElement* pElement = CXMLCOM::Unwrap( pXML );
	if ( pElement )
	{
		COLORREF pColour;
		pThis->LoadXMLColour( pElement, strName, &pColour );
		*pnColour = pColour;
	}
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CComRichBox::XEnumVARIANT enumerator

IMPLEMENT_UNKNOWN( CComRichBox, EnumVARIANT )

STDMETHODIMP CComRichBox::XEnumVARIANT::Next(ULONG celt, VARIANT FAR* rgvar, ULONG FAR* pceltFetched)
{
	METHOD_PROLOGUE( CComRichBox, EnumVARIANT )

	if ( pceltFetched ) *pceltFetched = 0;
	else if ( celt > 1 ) return E_INVALIDARG;

	VariantInit( &rgvar[0] );
	if ( m_nIndex >= (UINT)pThis->m_pDocument->GetCount() ) return S_FALSE;

	rgvar[0].vt			= VT_DISPATCH;
	rgvar[0].pdispVal	= (IDispatch*)CComRichBox::Wrap( pThis->m_pTaskBox, pThis->m_pDocument,
														 pThis->m_pDocument->GetIndex( m_nIndex ) );
	m_nIndex++;
	if ( pceltFetched ) (*pceltFetched)++;

	return S_OK;
}

STDMETHODIMP CComRichBox::XEnumVARIANT::Skip(ULONG celt)
{
	METHOD_PROLOGUE( CComRichBox, EnumVARIANT )

	UINT nCount = static_cast< UINT >( pThis->m_pDocument->GetCount() );

	while ( celt-- && m_nIndex++ < nCount );

	return ( celt == 0 ? S_OK : S_FALSE );
}

STDMETHODIMP CComRichBox::XEnumVARIANT::Reset()
{
	METHOD_PROLOGUE( CComRichBox, EnumVARIANT )
	m_nIndex = 0;
	return S_OK;
}

STDMETHODIMP CComRichBox::XEnumVARIANT::Clone(IEnumVARIANT FAR* FAR* /*ppenum*/)
{
	METHOD_PROLOGUE( CComRichBox, EnumVARIANT )
	return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////
// CComRichBox ISRichItem

IMPLEMENT_DISPATCH( CComRichBox, SRichElement )

STDMETHODIMP CComRichBox::XSRichElement::get_Application(IApplication FAR* FAR* ppApplication)
{
	METHOD_PROLOGUE( CComRichBox, SRichElement )
	*ppApplication = Application.GetApp();
	return S_OK;
}

STDMETHODIMP CComRichBox::XSRichElement::get_UserInterface(IUserInterface FAR* FAR* ppUserInterface)
{
	METHOD_PROLOGUE( CComRichBox, SRichElement )
	*ppUserInterface = Application.GetUI();
	return S_OK;
}

STDMETHODIMP CComRichBox::XSRichElement::get_Document(ISRichDocument FAR* FAR* ppDocument)
{
	METHOD_PROLOGUE( CComRichBox, SRichElement )
	if ( pThis->m_pDocument == NULL ) return E_UNEXPECTED;
	*ppDocument = CComRichBox::Wrap( pThis->m_pTaskBox, pThis->m_pDocument );
	return S_OK;
}

STDMETHODIMP CComRichBox::XSRichElement::Remove()
{
	METHOD_PROLOGUE( CComRichBox, SRichElement )
	if ( pThis->m_pDocument == NULL || pThis->m_pElement == NULL ) return E_UNEXPECTED;

	pThis->m_pDocument->Remove( pThis->m_pElement );
	delete pThis->m_pElement;
	pThis->m_pElement = NULL;

	return S_OK;
}

STDMETHODIMP CComRichBox::XSRichElement::Show(VARIANT_BOOL bShow)
{
	METHOD_PROLOGUE( CComRichBox, SRichElement )
	if ( pThis->m_pDocument == NULL || pThis->m_pElement == NULL ) return E_UNEXPECTED;

	pThis->m_pElement->Show( bShow == VARIANT_TRUE );
	return S_OK;
}

STDMETHODIMP CComRichBox::XSRichElement::SetText(BSTR strText)
{
	METHOD_PROLOGUE( CComRichBox, SRichElement )
	if ( pThis->m_pDocument == NULL || pThis->m_pElement == NULL ) return E_UNEXPECTED;
	pThis->m_pElement->SetText( strText );
	return S_OK;
}

STDMETHODIMP CComRichBox::XSRichElement::SetFlags(LONG nFlags, LONG nMask)
{
	METHOD_PROLOGUE( CComRichBox, SRichElement )
	if ( pThis->m_pDocument == NULL || pThis->m_pElement == NULL ) return E_UNEXPECTED;
	pThis->m_pElement->SetFlags( DWORD(nFlags), DWORD(nMask) );
	return S_OK;
}

STDMETHODIMP CComRichBox::XSRichElement::get_Size(LONG* pnSize)
{
	METHOD_PROLOGUE( CComRichBox, SRichElement )
	if ( pThis->m_pElement == NULL ) return E_UNEXPECTED;

	CRichElement* pElement = pThis->m_pElement;
	CSize sz( 0, 0 );

	if ( pElement->m_nType == retGap )
	{
		_stscanf( pElement->m_sText, _T("%lu"), &sz.cx );
	}
	else if ( pElement->m_nType == retBitmap && pElement->m_hImage != NULL )
	{
		BITMAP pInfo;
		GetObject( (HBITMAP)pElement->m_hImage, sizeof(pInfo), &pInfo );

		sz.cx = pInfo.bmWidth;
		sz.cy = pInfo.bmHeight;
	}
	else if ( pElement->m_nType == retIcon )
	{
		sz.cx = sz.cy = 16;
		UINT nID;
		_stscanf( pElement->m_sText, _T("%lu.%i.%i"), &nID, &sz.cx, &sz.cy );
	}
	else if ( pElement->m_nType == retEmoticon || pElement->m_nType == retCmdIcon )
	{
		sz.cx = sz.cy = 16;
	}
	else if ( pElement->m_nType == retAnchor )
	{
		sz.cx = sz.cy = 16;
		_stscanf( pElement->m_sText, _T("%i.%i"), &sz.cx, &sz.cy );
	}

	*pnSize = MAKELONG(sz.cx, sz.cy);

	return S_OK;
}

STDMETHODIMP CComRichBox::XSRichElement::get_ItemType(RichElementType* pnType)
{
	METHOD_PROLOGUE( CComRichBox, SRichElement )
	if ( pThis->m_pDocument == NULL || pThis->m_pElement == NULL ) return E_UNEXPECTED;
	*pnType = (RichElementType)pThis->m_pElement->m_nType;
	return S_OK;
}
