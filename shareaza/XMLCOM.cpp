//
// XMLCOM.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2012.
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
#include "XMLCOM.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CXMLCOM, CComObject)

IMPLEMENT_DYNCREATE(CXMLCOMCol, CComObject)

// {30FC662A-D72A-4f79-B63A-ACD4FBFE68A3}
IMPLEMENT_OLECREATE_FLAGS(CXMLCOM, CLIENT_NAME _T(".XML"),
	afxRegFreeThreading|afxRegApartmentThreading,
	0x30fc662a, 0xd72a, 0x4f79, 0xb6, 0x3a, 0xac, 0xd4, 0xfb, 0xfe, 0x68, 0xa3)

// {D73ABD28-3A2A-4e36-AD6F-2AA8F011FBE3}
IMPLEMENT_OLECREATE_FLAGS(CXMLCOMCol, CLIENT_NAME _T(".XMLCollection"),
	afxRegFreeThreading|afxRegApartmentThreading,
	0xd73abd28, 0x3a2a, 0x4e36, 0xad, 0x6f, 0x2a, 0xa8, 0xf0, 0x11, 0xfb, 0xe3)

BEGIN_INTERFACE_MAP(CXMLCOM, CComObject)
	INTERFACE_PART(CXMLCOM, IID_ISXMLNode, XMLNode)
	INTERFACE_PART(CXMLCOM, IID_ISXMLElement, XMLElement)
	INTERFACE_PART(CXMLCOM, IID_ISXMLAttribute, XMLAttribute)
END_INTERFACE_MAP()

BEGIN_INTERFACE_MAP(CXMLCOMCol, CComObject)
	INTERFACE_PART(CXMLCOMCol, IID_ISXMLElements, XMLElements)
	INTERFACE_PART(CXMLCOMCol, IID_ISXMLAttributes, XMLAttributes)
	INTERFACE_PART(CXMLCOMCol, IID_IEnumVARIANT, EnumVARIANT)
END_INTERFACE_MAP()


//////////////////////////////////////////////////////////////////////
// CXMLCOM construction

CXMLCOM::CXMLCOM(CXMLNode* pNode)
{
	m_pNode = pNode ? pNode : new CXMLElement();

	EnableDispatch( IID_ISXMLNode );
	EnableDispatch( IID_ISXMLElement );
	EnableDispatch( IID_ISXMLAttribute );
}

CXMLCOM::~CXMLCOM()
{
}

//////////////////////////////////////////////////////////////////////
// CXMLCOM wrapper

IUnknown* CXMLCOM::Wrap(CXMLNode* pNode, REFIID pIID)
{
	if ( pNode == NULL ) return NULL;

	if ( pIID == IID_ISXMLElement )
	{
		if ( pNode->m_nNode != CXMLNode::xmlElement ) return NULL;
	}
	else if ( pIID == IID_ISXMLAttribute )
	{
		if ( pNode->m_nNode != CXMLNode::xmlAttribute ) return NULL;
	}

	CXMLCOM* pWrap = new CXMLCOM( pNode );

	IUnknown* pCom = pWrap->GetInterface( pIID, FALSE );
	if ( pCom == NULL ) delete pWrap;

	return pCom;
}

//////////////////////////////////////////////////////////////////////
// CXMLCOM unwrapper

CXMLElement* CXMLCOM::Unwrap(ISXMLElement* pInterface)
{
	if ( pInterface == NULL ) return NULL;
	INTERFACE_TO_CLASS(CXMLCOM, XMLElement, pInterface, pWrapper)
	return (CXMLElement*)pWrapper->m_pNode;
}

//////////////////////////////////////////////////////////////////////
// CXMLCOM ISXMLNode

IMPLEMENT_DISPATCH( CXMLCOM, XMLNode )

STDMETHODIMP CXMLCOM::XXMLNode::get_Parent(ISXMLElement FAR* FAR* ppParent)
{
	METHOD_PROLOGUE( CXMLCOM, XMLNode )
	if ( ppParent == NULL ) return E_INVALIDARG;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	CXMLElement* pParent = pThis->m_pNode->GetParent();
	if ( pParent )
		*ppParent = (ISXMLElement*)Wrap( pParent, IID_ISXMLElement );
	else
		*ppParent = NULL;
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLNode::get_Type(SXMLNodeType FAR* pnType)
{
	METHOD_PROLOGUE( CXMLCOM, XMLNode )
	if ( pnType == NULL ) return E_INVALIDARG;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	*pnType = (SXMLNodeType)pThis->m_pNode->GetType();
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLNode::get_AsNode(ISXMLNode FAR* FAR* ppNode)
{
	METHOD_PROLOGUE( CXMLCOM, XMLNode )
	if ( ppNode == NULL ) return E_INVALIDARG;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	*ppNode = (ISXMLNode*)Wrap( pThis->m_pNode, IID_ISXMLNode );
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLNode::get_AsElement(ISXMLNode FAR* FAR* ppElement)
{
	METHOD_PROLOGUE( CXMLCOM, XMLNode )
	if ( ppElement == NULL ) return E_INVALIDARG;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	*ppElement = (ISXMLElement*)Wrap( pThis->m_pNode, IID_ISXMLElement );
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLNode::get_AsAttribute(ISXMLNode FAR* FAR* ppAttribute)
{
	METHOD_PROLOGUE( CXMLCOM, XMLNode )
	if ( ppAttribute == NULL ) return E_INVALIDARG;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	*ppAttribute = (ISXMLAttribute*)Wrap( pThis->m_pNode, IID_ISXMLAttribute );
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLNode::get_Name(BSTR FAR* psName)
{
	METHOD_PROLOGUE( CXMLCOM, XMLNode )
	if ( psName == NULL ) return E_INVALIDARG;
	*psName = NULL;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	*psName = CComBSTR( pThis->m_pNode->GetName() ).Detach();
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLNode::put_Name(BSTR sName)
{
	METHOD_PROLOGUE( CXMLCOM, XMLNode )
	if ( sName == NULL ) return E_INVALIDARG;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	pThis->m_pNode->SetName( CString( sName ) );
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLNode::get_Value(BSTR FAR* psValue)
{
	METHOD_PROLOGUE( CXMLCOM, XMLNode )
	if ( psValue == NULL ) return E_INVALIDARG;
	*psValue = NULL;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	*psValue = CComBSTR( pThis->m_pNode->GetValue() ).Detach();
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLNode::put_Value(BSTR sValue)
{
	METHOD_PROLOGUE( CXMLCOM, XMLNode )
	if ( sValue == NULL ) return E_INVALIDARG;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	pThis->m_pNode->SetValue( CString( sValue ) );
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLNode::Delete()
{
	METHOD_PROLOGUE( CXMLCOM, XMLNode )
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	pThis->m_pNode->Delete();
	pThis->m_pNode = NULL;
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLNode::IsNamed(BSTR sName, VARIANT_BOOL FAR* pbResult)
{
	METHOD_PROLOGUE( CXMLCOM, XMLNode )
	if ( sName == NULL || pbResult == NULL ) return E_INVALIDARG;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	*pbResult = pThis->m_pNode->IsNamed( CString( sName ) ) ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

//////////////////////////////////////////////////////////////////////
// CXMLCOM ISXMLElement

IMPLEMENT_DISPATCH( CXMLCOM, XMLElement )

STDMETHODIMP CXMLCOM::XXMLElement::get_Parent(ISXMLElement FAR* FAR* ppParent)
{
	METHOD_PROLOGUE( CXMLCOM, XMLElement )
	if ( ppParent == NULL ) return E_INVALIDARG;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	CXMLElement* pParent = pThis->m_pNode->GetParent();
	if ( pParent )
		*ppParent = (ISXMLElement*)Wrap( pParent, IID_ISXMLElement );
	else
		*ppParent = NULL;
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLElement::get_Type(SXMLNodeType FAR* pnType)
{
	METHOD_PROLOGUE( CXMLCOM, XMLElement )
	if ( pnType == NULL ) return E_INVALIDARG;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	*pnType = (SXMLNodeType)pThis->m_pNode->GetType();
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLElement::get_AsNode(ISXMLNode FAR* FAR* ppNode)
{
	METHOD_PROLOGUE( CXMLCOM, XMLElement )
	if ( ppNode == NULL ) return E_INVALIDARG;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	*ppNode = (ISXMLNode*)Wrap( pThis->m_pNode, IID_ISXMLNode );
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLElement::get_AsElement(ISXMLNode FAR* FAR* ppElement)
{
	METHOD_PROLOGUE( CXMLCOM, XMLElement )
	if ( ppElement == NULL ) return E_INVALIDARG;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	*ppElement = (ISXMLElement*)Wrap( pThis->m_pNode, IID_ISXMLElement );
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLElement::get_AsAttribute(ISXMLNode FAR* FAR* ppAttribute)
{
	METHOD_PROLOGUE( CXMLCOM, XMLElement )
	if ( ppAttribute == NULL ) return E_INVALIDARG;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	*ppAttribute = (ISXMLAttribute*)Wrap( pThis->m_pNode, IID_ISXMLAttribute );
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLElement::get_Name(BSTR FAR* psName)
{
	METHOD_PROLOGUE( CXMLCOM, XMLElement )
	if ( psName == NULL ) return E_INVALIDARG;
	*psName = NULL;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	*psName = CComBSTR( pThis->m_pNode->GetName() ).Detach();
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLElement::put_Name(BSTR sName)
{
	METHOD_PROLOGUE( CXMLCOM, XMLElement )
	if ( sName == NULL ) return E_INVALIDARG;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	pThis->m_pNode->SetName( CString( sName ) );
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLElement::get_Value(BSTR FAR* psValue)
{
	METHOD_PROLOGUE( CXMLCOM, XMLElement )
	if ( psValue == NULL ) return E_INVALIDARG;
	*psValue = NULL;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	*psValue = CComBSTR( pThis->m_pNode->GetValue() ).Detach();
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLElement::put_Value(BSTR sValue)
{
	METHOD_PROLOGUE( CXMLCOM, XMLElement )
	if ( sValue == NULL ) return E_INVALIDARG;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	pThis->m_pNode->SetValue( CString( sValue ) );
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLElement::Delete()
{
	METHOD_PROLOGUE( CXMLCOM, XMLElement )
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	pThis->m_pNode->Delete();
	pThis->m_pNode = NULL;
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLElement::IsNamed(BSTR sName, VARIANT_BOOL FAR* pbResult)
{
	METHOD_PROLOGUE( CXMLCOM, XMLElement )
	if ( sName == NULL || pbResult == NULL ) return E_INVALIDARG;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	*pbResult = pThis->m_pNode->IsNamed( CString( sName ) ) ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLElement::get_Elements(ISXMLElements FAR* FAR* ppElements)
{
	METHOD_PROLOGUE( CXMLCOM, XMLElement )
	if ( ppElements == NULL ) return E_INVALIDARG;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	*ppElements = CXMLCOMCol::WrapElements( (CXMLElement*)pThis->m_pNode );
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLElement::get_Attributes(ISXMLAttributes FAR* FAR* ppAttributes)
{
	METHOD_PROLOGUE( CXMLCOM, XMLElement )
	if ( ppAttributes == NULL ) return E_INVALIDARG;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	*ppAttributes = CXMLCOMCol::WrapAttributes( (CXMLElement*)pThis->m_pNode );
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLElement::Detach()
{
	METHOD_PROLOGUE( CXMLCOM, XMLElement )
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	((CXMLElement*)pThis->m_pNode)->Detach();
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLElement::Clone(ISXMLElement FAR* FAR* ppClone)
{
	METHOD_PROLOGUE( CXMLCOM, XMLElement )
	if ( ppClone == NULL ) return E_INVALIDARG;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	*ppClone = (ISXMLElement*)Wrap( ((CXMLElement*)pThis->m_pNode)->Clone(), IID_ISXMLElement );
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLElement::ToString(BSTR FAR* psValue)
{
	METHOD_PROLOGUE( CXMLCOM, XMLElement )
	if ( psValue == NULL ) return E_INVALIDARG;
	*psValue = NULL;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	*psValue = CComBSTR( ((CXMLElement*)pThis->m_pNode)->ToString() ).Detach();
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLElement::ToStringEx(VARIANT_BOOL bHeader, VARIANT_BOOL bNewlines, BSTR FAR* psValue)
{
	METHOD_PROLOGUE( CXMLCOM, XMLElement )
	if ( psValue == NULL ) return E_INVALIDARG;
	*psValue = NULL;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	*psValue = CComBSTR( ((CXMLElement*)pThis->m_pNode)->ToString( (BOOL)bHeader, (BOOL)bNewlines ) ).Detach();
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLElement::FromString(BSTR sXML, ISXMLElement FAR* FAR* ppElement)
{
	METHOD_PROLOGUE( CXMLCOM, XMLElement )
	if ( sXML == NULL || ppElement == NULL ) return E_INVALIDARG;
	CXMLElement* pElement = CXMLElement::FromString( CString( sXML ) );
	if ( ! pElement ) return E_FAIL;
	*ppElement = (ISXMLElement*)Wrap( pElement, IID_ISXMLElement );
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLElement::GetWords(BSTR FAR* psWords)
{
	METHOD_PROLOGUE( CXMLCOM, XMLElement )
	if ( psWords == NULL ) return E_INVALIDARG;
	*psWords = NULL;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	*psWords = CComBSTR( ((CXMLElement*)pThis->m_pNode)->GetRecursiveWords() ).Detach();
	return S_OK;
}


//////////////////////////////////////////////////////////////////////
// CXMLCOM ISXMLAttribute

IMPLEMENT_DISPATCH( CXMLCOM, XMLAttribute )

STDMETHODIMP CXMLCOM::XXMLAttribute::get_Parent(ISXMLElement FAR* FAR* ppParent)
{
	METHOD_PROLOGUE( CXMLCOM, XMLAttribute )
	if ( ppParent == NULL ) return E_INVALIDARG;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	CXMLElement* pParent = pThis->m_pNode->GetParent();
	if ( pParent )
		*ppParent = (ISXMLElement*)Wrap( pParent, IID_ISXMLElement );
	else
		*ppParent = NULL;
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLAttribute::get_Type(SXMLNodeType FAR* pnType)
{
	METHOD_PROLOGUE( CXMLCOM, XMLAttribute )
	if ( pnType == NULL ) return E_INVALIDARG;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	*pnType = (SXMLNodeType)pThis->m_pNode->GetType();
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLAttribute::get_AsNode(ISXMLNode FAR* FAR* ppNode)
{
	METHOD_PROLOGUE( CXMLCOM, XMLAttribute )
	if ( ppNode == NULL ) return E_INVALIDARG;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	*ppNode = (ISXMLNode*)Wrap( pThis->m_pNode, IID_ISXMLNode );
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLAttribute::get_AsElement(ISXMLNode FAR* FAR* ppElement)
{
	METHOD_PROLOGUE( CXMLCOM, XMLAttribute )
	if ( ppElement == NULL ) return E_INVALIDARG;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	*ppElement = (ISXMLElement*)Wrap( pThis->m_pNode, IID_ISXMLElement );
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLAttribute::get_AsAttribute(ISXMLNode FAR* FAR* ppAttribute)
{
	METHOD_PROLOGUE( CXMLCOM, XMLAttribute )
	if ( ppAttribute == NULL ) return E_INVALIDARG;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	*ppAttribute = (ISXMLAttribute*)Wrap( pThis->m_pNode, IID_ISXMLAttribute );
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLAttribute::get_Name(BSTR FAR* psName)
{
	METHOD_PROLOGUE( CXMLCOM, XMLAttribute )
	if ( psName == NULL ) return E_INVALIDARG;
	*psName = NULL;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	*psName = CComBSTR( pThis->m_pNode->GetName() ).Detach();
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLAttribute::put_Name(BSTR sName)
{
	METHOD_PROLOGUE( CXMLCOM, XMLAttribute )
	if ( sName == NULL ) return E_INVALIDARG;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	pThis->m_pNode->SetName( CString( sName ) );
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLAttribute::get_Value(BSTR FAR* psValue)
{
	METHOD_PROLOGUE( CXMLCOM, XMLAttribute )
	if ( psValue == NULL ) return E_INVALIDARG;
	*psValue = NULL;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	*psValue = CComBSTR( pThis->m_pNode->GetValue() ).Detach();
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLAttribute::put_Value(BSTR sValue)
{
	METHOD_PROLOGUE( CXMLCOM, XMLAttribute )
	if ( sValue == NULL ) return E_INVALIDARG;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	pThis->m_pNode->SetValue( CString( sValue ) );
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLAttribute::Delete()
{
	METHOD_PROLOGUE( CXMLCOM, XMLAttribute )
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	pThis->m_pNode->Delete();
	pThis->m_pNode = NULL;
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLAttribute::IsNamed(BSTR sName, VARIANT_BOOL FAR* pbResult)
{
	METHOD_PROLOGUE( CXMLCOM, XMLAttribute )
	if ( sName == NULL || pbResult == NULL ) return E_INVALIDARG;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	*pbResult = pThis->m_pNode->IsNamed( CString( sName ) ) ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CXMLCOM::XXMLAttribute::Detach()
{
	METHOD_PROLOGUE( CXMLCOM, XMLAttribute )
	return E_NOTIMPL;
}

STDMETHODIMP CXMLCOM::XXMLAttribute::Clone(ISXMLAttribute FAR* FAR* ppClone)
{
	METHOD_PROLOGUE( CXMLCOM, XMLAttribute )
	if ( ppClone == NULL ) return E_INVALIDARG;
	if ( ! pThis->m_pNode ) return E_UNEXPECTED;
	*ppClone = (ISXMLAttribute*)Wrap( ((CXMLAttribute*)pThis->m_pNode)->Clone(), IID_ISXMLAttribute );
	return S_OK;
}

//////////////////////////////////////////////////////////////////////
// CXMLCOMCol construction

CXMLCOMCol::CXMLCOMCol(CXMLElement* pElement)
{
	m_pElement = pElement;

	EnableDispatch( IID_ISXMLElements );
	EnableDispatch( IID_ISXMLAttributes );
}

CXMLCOMCol::~CXMLCOMCol()
{
}

//////////////////////////////////////////////////////////////////////
// CXMLCOMCol wrapper

ISXMLElements* CXMLCOMCol::WrapElements(CXMLElement* pElement)
{
	CXMLCOMCol* pWrap = new CXMLCOMCol( pElement );

	ISXMLElements* pCom = (ISXMLElements*)pWrap->GetInterface( IID_ISXMLElements, FALSE );
	if ( pCom == NULL ) delete pWrap;

	return pCom;
}

ISXMLAttributes* CXMLCOMCol::WrapAttributes(CXMLElement* pElement)
{
	CXMLCOMCol* pWrap = new CXMLCOMCol( pElement );

	ISXMLAttributes* pCom = (ISXMLAttributes*)pWrap->GetInterface( IID_ISXMLAttributes, FALSE );
	if ( pCom == NULL ) delete pWrap;

	return pCom;
}

//////////////////////////////////////////////////////////////////////
// CXMLCOMCol ISXMLElements

IMPLEMENT_DISPATCH( CXMLCOMCol, XMLElements )

STDMETHODIMP CXMLCOMCol::XXMLElements::get__NewEnum(IUnknown FAR* FAR* ppEnum)
{
	METHOD_PROLOGUE( CXMLCOMCol, XMLElements )
	AddRef();
	*ppEnum = &pThis->m_xEnumVARIANT;
	pThis->m_xEnumVARIANT.m_posCurrent = pThis->m_pElement->GetElementIterator();
	pThis->m_xEnumVARIANT.m_bAttributes = FALSE;
	return S_OK;
}

STDMETHODIMP CXMLCOMCol::XXMLElements::get_Item(VARIANT vIndex, ISXMLElement FAR* FAR* ppElement)
{
	METHOD_PROLOGUE( CXMLCOMCol, XMLElements )

	CXMLElement* pElement = NULL;

	if ( vIndex.vt == VT_BSTR )
	{
		pElement = pThis->m_pElement->GetElementByName( CString( vIndex.bstrVal ) );
	}
	else
	{
		VARIANT va;
		VariantInit( &va );

		if ( SUCCEEDED( VariantChangeType( &va, (VARIANT FAR*)&vIndex, 0, VT_I4 ) ) )
		{
			for ( POSITION pos = pThis->m_pElement->GetElementIterator() ; pos ; vIndex.lVal-- )
			{
				pElement = pThis->m_pElement->GetNextElement( pos );
				if ( vIndex.lVal == 0 ) break;
				pElement = NULL;
			}
		}
	}

	*ppElement = (ISXMLElement*)CXMLCOM::Wrap( pElement, IID_ISXMLElement );

	return S_OK;
}

STDMETHODIMP CXMLCOMCol::XXMLElements::get_Count(LONG FAR* pnCount)
{
	METHOD_PROLOGUE( CXMLCOMCol, XMLElements )
	*pnCount = (LONG)pThis->m_pElement->GetElementCount();
	return S_OK;
}

STDMETHODIMP CXMLCOMCol::XXMLElements::Create(BSTR strName, ISXMLElement FAR* FAR* ppElement)
{
	METHOD_PROLOGUE( CXMLCOMCol, XMLElements )
	*ppElement = (ISXMLElement*)CXMLCOM::Wrap( pThis->m_pElement->AddElement( CString( strName ) ), IID_ISXMLElement );
	return S_OK;
}

STDMETHODIMP CXMLCOMCol::XXMLElements::Attach(ISXMLElement FAR* pElement)
{
	METHOD_PROLOGUE( CXMLCOMCol, XMLElements )
	INTERFACE_TO_CLASS( CXMLCOM, XMLElement, pElement, prWrap )
	pThis->m_pElement->AddElement( (CXMLElement*)prWrap->m_pNode );
	return S_OK;
}

STDMETHODIMP CXMLCOMCol::XXMLElements::RemoveAll()
{
	METHOD_PROLOGUE( CXMLCOMCol, XMLElements )
	pThis->m_pElement->DeleteAllElements();
	return S_OK;
}

STDMETHODIMP CXMLCOMCol::XXMLElements::get_First(ISXMLElement FAR* FAR* ppElement)
{
	METHOD_PROLOGUE( CXMLCOMCol, XMLElements )
	*ppElement = (ISXMLElement*)CXMLCOM::Wrap( pThis->m_pElement->GetFirstElement(), IID_ISXMLElement );
	return S_OK;
}

STDMETHODIMP CXMLCOMCol::XXMLElements::get_ByName(BSTR sName, ISXMLElement FAR* FAR* ppElement)
{
	METHOD_PROLOGUE( CXMLCOMCol, XMLElements )
	*ppElement = (ISXMLElement*)CXMLCOM::Wrap( pThis->m_pElement->GetElementByName( CString( sName ) ), IID_ISXMLElement );
	return S_OK;
}

//////////////////////////////////////////////////////////////////////
// CXMLCOMCol ISXMLAttributes

IMPLEMENT_DISPATCH( CXMLCOMCol, XMLAttributes )

STDMETHODIMP CXMLCOMCol::XXMLAttributes::get__NewEnum(IUnknown FAR* FAR* ppEnum)
{
	METHOD_PROLOGUE( CXMLCOMCol, XMLAttributes )
	AddRef();
	*ppEnum = &pThis->m_xEnumVARIANT;
	pThis->m_xEnumVARIANT.m_posCurrent = pThis->m_pElement->GetAttributeIterator();
	pThis->m_xEnumVARIANT.m_bAttributes = TRUE;
	return S_OK;
}

STDMETHODIMP CXMLCOMCol::XXMLAttributes::get_Item(VARIANT vIndex, ISXMLAttribute FAR* FAR* ppAttribute)
{
	METHOD_PROLOGUE( CXMLCOMCol, XMLAttributes )

	CXMLAttribute* pAttribute = NULL;

	if ( vIndex.vt == VT_BSTR )
	{
		pAttribute = pThis->m_pElement->GetAttribute( CString( vIndex.bstrVal ) );
	}
	else
	{
		VARIANT va;
		VariantInit( &va );

		if ( SUCCEEDED( VariantChangeType( &va, (VARIANT FAR*)&vIndex, 0, VT_I4 ) ) )
		{
			for ( POSITION pos = pThis->m_pElement->GetAttributeIterator() ; pos ; vIndex.lVal-- )
			{
				pAttribute = pThis->m_pElement->GetNextAttribute( pos );
				if ( vIndex.lVal == 0 ) break;
				pAttribute = NULL;
			}
		}
	}

	*ppAttribute = (ISXMLAttribute*)CXMLCOM::Wrap( pAttribute, IID_ISXMLAttribute );

	return S_OK;
}

STDMETHODIMP CXMLCOMCol::XXMLAttributes::get_Count(LONG FAR* pnCount)
{
	METHOD_PROLOGUE( CXMLCOMCol, XMLAttributes )
	*pnCount = (LONG)pThis->m_pElement->GetAttributeCount();
	return S_OK;
}

STDMETHODIMP CXMLCOMCol::XXMLAttributes::Add(BSTR strName, BSTR strValue)
{
	METHOD_PROLOGUE( CXMLCOMCol, XMLAttributes )
	pThis->m_pElement->AddAttribute( CString( strName ), CString( strValue ) );
	return S_OK;
}

STDMETHODIMP CXMLCOMCol::XXMLAttributes::Create(BSTR strName, ISXMLAttribute FAR* FAR* ppAttribute)
{
	METHOD_PROLOGUE( CXMLCOMCol, XMLAttributes )
	*ppAttribute = (ISXMLAttribute*)CXMLCOM::Wrap( pThis->m_pElement->AddAttribute( CString( strName ) ), IID_ISXMLElement );
	return S_OK;
}

STDMETHODIMP CXMLCOMCol::XXMLAttributes::Attach(ISXMLAttribute FAR* pAttribute)
{
	METHOD_PROLOGUE( CXMLCOMCol, XMLAttributes )
	INTERFACE_TO_CLASS( CXMLCOM, XMLAttribute, pAttribute, prWrap )
	pThis->m_pElement->AddAttribute( (CXMLAttribute*)prWrap->m_pNode );
	return S_OK;
}

STDMETHODIMP CXMLCOMCol::XXMLAttributes::RemoveAll()
{
	METHOD_PROLOGUE( CXMLCOMCol, XMLAttributes )
	pThis->m_pElement->DeleteAllAttributes();
	return S_OK;
}

STDMETHODIMP CXMLCOMCol::XXMLAttributes::get_ByName(BSTR sName, ISXMLAttribute FAR* FAR* ppAttribute)
{
	METHOD_PROLOGUE( CXMLCOMCol, XMLAttributes )
	*ppAttribute = (ISXMLAttribute*)CXMLCOM::Wrap( pThis->m_pElement->GetAttribute( CString( sName ) ), IID_ISXMLAttribute );
	return S_OK;
}

STDMETHODIMP CXMLCOMCol::XXMLAttributes::get_Get(BSTR sName, BSTR FAR* psValue)
{
	METHOD_PROLOGUE( CXMLCOMCol, XMLAttributes )
	*psValue = CComBSTR( pThis->m_pElement->GetAttributeValue(
		CString( sName ) ) ).Detach();
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CXMLCOMCol::XEnumVARIANT enumerator

IMPLEMENT_UNKNOWN( CXMLCOMCol, EnumVARIANT )

STDMETHODIMP CXMLCOMCol::XEnumVARIANT::Next(ULONG celt, VARIANT FAR* rgvar, ULONG FAR* pceltFetched)
{
	METHOD_PROLOGUE( CXMLCOMCol, EnumVARIANT )

	ULONG i;

	if ( pceltFetched ) *pceltFetched = 0;
	else if ( celt > 1 ) return E_INVALIDARG;

	for ( i = 0 ; i < celt ; i++ ) VariantInit( &rgvar[i] );

	for ( i = 0 ; i < celt ; i++ )
	{
		if ( m_bAttributes )
		{
			CXMLAttribute* pAttribute = pThis->m_pElement->GetNextAttribute( m_posCurrent );
			if ( ! pAttribute ) break;

			rgvar[i].vt			= VT_DISPATCH;
			rgvar[i].pdispVal	= (IDispatch*)CXMLCOM::Wrap( pAttribute, IID_ISXMLAttribute );
		}
		else
		{
			CXMLElement* pElement = pThis->m_pElement->GetNextElement( m_posCurrent );
			if ( ! pElement ) break;

			rgvar[i].vt			= VT_DISPATCH;
			rgvar[i].pdispVal	= (IDispatch*)CXMLCOM::Wrap( pElement, IID_ISXMLElement );
		}

		if ( pceltFetched ) (*pceltFetched)++;
	}

	if ( i < celt ) return ResultFromScode( S_FALSE );

	return S_OK;
}

STDMETHODIMP CXMLCOMCol::XEnumVARIANT::Skip(ULONG celt)
{
    METHOD_PROLOGUE( CXMLCOMCol, EnumVARIANT )

	if ( m_bAttributes )
	{
		while ( celt > 0 && pThis->m_pElement->GetNextAttribute( m_posCurrent ) ) celt--;
	}
	else
	{
		while ( celt > 0 && pThis->m_pElement->GetNextElement( m_posCurrent ) ) celt--;
	}

    return ( celt == 0 ? S_OK : ResultFromScode( S_FALSE ) );
}

STDMETHODIMP CXMLCOMCol::XEnumVARIANT::Reset()
{
    METHOD_PROLOGUE( CXMLCOMCol, EnumVARIANT )
	m_posCurrent = m_bAttributes ? pThis->m_pElement->GetAttributeIterator() : pThis->m_pElement->GetElementIterator();
    return S_OK;
}

STDMETHODIMP CXMLCOMCol::XEnumVARIANT::Clone(IEnumVARIANT FAR* FAR* /*ppenum*/)
{
    METHOD_PROLOGUE( CXMLCOMCol, EnumVARIANT )
    return E_NOTIMPL;
}
