//
// XMLCOM.h
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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

#pragma once

class CXMLNode;
class CXMLElement;
class CXMLAttribute;
class CXMLCOMCol;


class CXMLCOM : public CComObject
{
	DECLARE_DYNCREATE(CXMLCOM)

public:
	CXMLCOM(CXMLNode* pNode = NULL);
	virtual ~CXMLCOM();

// Attributes
public:
	CXMLNode*	m_pNode;

// Operations
public:
	static IUnknown*	Wrap(CXMLNode* pNode, REFIID pIID);
	static CXMLElement*	Unwrap(ISXMLElement* pInterface);

// Automation
public:
	BEGIN_INTERFACE_PART(XMLNode, ISXMLNode)
		DECLARE_DISPATCH()
		STDMETHOD(get_Parent)(ISXMLElement FAR* FAR* ppParent);
		STDMETHOD(get_Type)(SXMLNodeType FAR* pnType);
		STDMETHOD(get_AsNode)(ISXMLNode FAR* FAR* ppNode);
		STDMETHOD(get_AsElement)(ISXMLNode FAR* FAR* ppElement);
		STDMETHOD(get_AsAttribute)(ISXMLNode FAR* FAR* ppAttribute);
		STDMETHOD(get_Name)(BSTR FAR* psName);
		STDMETHOD(put_Name)(BSTR sName);
		STDMETHOD(get_Value)(BSTR FAR* psValue);
		STDMETHOD(put_Value)(BSTR sValue);
		STDMETHOD(Delete)();
		STDMETHOD(IsNamed)(BSTR sName, VARIANT_BOOL FAR* pbResult);
	END_INTERFACE_PART(XMLNode)

	BEGIN_INTERFACE_PART(XMLElement, ISXMLElement)
		DECLARE_DISPATCH()
		STDMETHOD(get_Parent)(ISXMLElement FAR* FAR* ppParent);
		STDMETHOD(get_Type)(SXMLNodeType FAR* pnType);
		STDMETHOD(get_AsNode)(ISXMLNode FAR* FAR* ppNode);
		STDMETHOD(get_AsElement)(ISXMLNode FAR* FAR* ppElement);
		STDMETHOD(get_AsAttribute)(ISXMLNode FAR* FAR* ppAttribute);
		STDMETHOD(get_Name)(BSTR FAR* psName);
		STDMETHOD(put_Name)(BSTR sName);
		STDMETHOD(get_Value)(BSTR FAR* psValue);
		STDMETHOD(put_Value)(BSTR sValue);
		STDMETHOD(Delete)();
		STDMETHOD(IsNamed)(BSTR sName, VARIANT_BOOL FAR* pbResult);
		STDMETHOD(get_Elements)(ISXMLElements FAR* FAR* ppElements);
		STDMETHOD(get_Attributes)(ISXMLAttributes FAR* FAR* ppAttributes);
		STDMETHOD(Detach)();
		STDMETHOD(Clone)(ISXMLElement FAR* FAR* ppClone);
		STDMETHOD(ToString)(BSTR FAR* psValue);
		STDMETHOD(ToStringEx)(VARIANT_BOOL bHeader, VARIANT_BOOL bNewlines, BSTR FAR* psValue);
		STDMETHOD(FromString)(BSTR sXML, ISXMLElement FAR* FAR* ppElement);
		STDMETHOD(GetWords)(BSTR FAR* psWords);
	END_INTERFACE_PART(XMLElement)

	BEGIN_INTERFACE_PART(XMLAttribute, ISXMLAttribute)
		DECLARE_DISPATCH()
		STDMETHOD(get_Parent)(ISXMLElement FAR* FAR* ppParent);
		STDMETHOD(get_Type)(SXMLNodeType FAR* pnType);
		STDMETHOD(get_AsNode)(ISXMLNode FAR* FAR* ppNode);
		STDMETHOD(get_AsElement)(ISXMLNode FAR* FAR* ppElement);
		STDMETHOD(get_AsAttribute)(ISXMLNode FAR* FAR* ppAttribute);
		STDMETHOD(get_Name)(BSTR FAR* psName);
		STDMETHOD(put_Name)(BSTR sName);
		STDMETHOD(get_Value)(BSTR FAR* psValue);
		STDMETHOD(put_Value)(BSTR sValue);
		STDMETHOD(Delete)();
		STDMETHOD(IsNamed)(BSTR sName, VARIANT_BOOL FAR* pbResult);
		STDMETHOD(Detach)();
		STDMETHOD(Clone)(ISXMLAttribute FAR* FAR* ppClone);
	END_INTERFACE_PART(XMLAttribute)

	DECLARE_OLECREATE(CXMLCOM)

	DECLARE_INTERFACE_MAP()

	friend class CXMLCOMCol;

};

class CXMLCOMCol : public CComObject
{
	DECLARE_DYNCREATE(CXMLCOMCol)

public:
	CXMLCOMCol(CXMLElement* pElement = NULL);
	virtual ~CXMLCOMCol();

// Attributes
public:
	CXMLElement*	m_pElement;

// Operations
public:
	static ISXMLElements*	WrapElements(CXMLElement* pElement);
	static ISXMLAttributes*	WrapAttributes(CXMLElement* pElement);

// Automation
protected:
	BEGIN_INTERFACE_PART(XMLElements, ISXMLElements)
		DECLARE_DISPATCH()
		STDMETHOD(get__NewEnum)(IUnknown FAR* FAR* ppEnum);
		STDMETHOD(get_Item)(VARIANT vIndex, ISXMLElement FAR* FAR* ppElement);
		STDMETHOD(get_Count)(LONG FAR* pnCount);
		STDMETHOD(Create)(BSTR strName, ISXMLElement FAR* FAR* ppElement);
		STDMETHOD(Attach)(ISXMLElement FAR* pElement);
		STDMETHOD(RemoveAll)();
		STDMETHOD(get_First)(ISXMLElement FAR* FAR* ppElement);
		STDMETHOD(get_ByName)(BSTR sName, ISXMLElement FAR* FAR* ppElement);
	END_INTERFACE_PART(XMLElements)

	BEGIN_INTERFACE_PART(XMLAttributes, ISXMLAttributes)
		DECLARE_DISPATCH()
		STDMETHOD(get__NewEnum)(IUnknown FAR* FAR* ppEnum);
		STDMETHOD(get_Item)(VARIANT vIndex, ISXMLAttribute FAR* FAR* ppAttribute);
		STDMETHOD(get_Count)(LONG FAR* pnCount);
		STDMETHOD(Add)(BSTR strName, BSTR strValue);
		STDMETHOD(Create)(BSTR strName, ISXMLAttribute FAR* FAR* ppAttribute);
		STDMETHOD(Attach)(ISXMLAttribute FAR* pAttribute);
		STDMETHOD(RemoveAll)();
		STDMETHOD(get_ByName)(BSTR sName, ISXMLAttribute FAR* FAR* ppAttribute);
		STDMETHOD(get_Get)(BSTR sName, BSTR FAR* psValue);
	END_INTERFACE_PART(XMLAttributes)

	BEGIN_INTERFACE_PART(EnumVARIANT, IEnumVARIANT)
		STDMETHOD(Next)(THIS_ DWORD celt, VARIANT FAR* rgvar, DWORD FAR* pceltFetched);
		STDMETHOD(Skip)(THIS_ DWORD celt);
		STDMETHOD(Reset)(THIS);
		STDMETHOD(Clone)(THIS_ IEnumVARIANT FAR* FAR* ppenum);
		POSITION	m_posCurrent;
		BOOL		m_bAttributes;
	END_INTERFACE_PART(EnumVARIANT)

	DECLARE_OLECREATE(CXMLCOMCol)

	DECLARE_INTERFACE_MAP()
};
