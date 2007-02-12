//
// ComRichBox.h
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

#pragma once

class CRichTaskBox;
class CRichDocument;
class CRichElement;

class CComRichBox : public CComObject
{
public:
	CComRichBox(CRichTaskBox* pTaskBox, CRichDocument* pDocument, CRichElement* pElement);
	virtual ~CComRichBox();

protected:
	BOOL LoadXMLStyles(CXMLElement* pParent);
	BOOL LoadXMLColour(CXMLElement* pXML, LPCTSTR strName, COLORREF* pnColour);

	// Attributes
public:
	CRichTaskBox*	m_pTaskBox;
	CRichDocument*	m_pDocument;
	CRichElement*	m_pElement;

	// Operations
public:
	static ISRichBox*		Wrap(CRichTaskBox* pTaskBox);
	static ISRichDocument*	Wrap(CRichTaskBox* pTaskBox, CRichDocument* pDocument);
	static ISRichItem*		Wrap(CRichTaskBox* pTaskBox, CRichDocument* pDocument, CRichElement* pElement);

	// ISRichBox
public:
	BEGIN_INTERFACE_PART(SRichBox, ISRichBox)
		DECLARE_DISPATCH()
		STDMETHOD(get_Application)(IApplication FAR* FAR* ppApplication);
		STDMETHOD(get_UserInterface)(IUserInterface FAR* FAR* ppUserInterface);
		STDMETHOD(Create)(HWND hPanel, BSTR bsCaption, INT nIcon);
		STDMETHOD(get_Document)(ISRichDocument FAR* FAR* ppDocument);
		STDMETHOD(put_Document)(ISRichDocument FAR* pDocument);
	END_INTERFACE_PART(SRichBox)

	BEGIN_INTERFACE_PART(SRichDocument, ISRichDocument)
		DECLARE_DISPATCH()
		STDMETHOD(get_Application)(IApplication FAR* FAR* ppApplication);
		STDMETHOD(get_UserInterface)(IUserInterface FAR* FAR* ppUserInterface);
		STDMETHOD(get__NewEnum)(IUnknown FAR* FAR* ppEnum);
		STDMETHOD(get_Item)(VARIANT vIndex, ISRichItem FAR* FAR* ppItem);
		STDMETHOD(get_Count)(LONG FAR* pnCount);
		STDMETHOD(Add)(INT nType, BSTR strText, BSTR strLink, LONG nFlags, INT nGroup, ISRichItem** ppItem);
		STDMETHOD(RemoveAll)();
		STDMETHOD(ShowGroup)(INT nGroup, VARIANT_BOOL bShow);
		STDMETHOD(ShowGroupRange)(INT nMin, INT nMax, VARIANT_BOOL bShow);
		STDMETHOD(CreateFonts)(BSTR strFaceName, INT nSize);
		STDMETHOD(LoadXMLStyles)(ISXMLElement* pParent);
		STDMETHOD(LoadXMLColour)(ISXMLElement* pXML, BSTR strName, LONG* pnColour);
	END_INTERFACE_PART(SRichDocument)

	BEGIN_INTERFACE_PART(EnumVARIANT, IEnumVARIANT)
		STDMETHOD(Next)(THIS_ DWORD celt, VARIANT FAR* rgvar, DWORD FAR* pceltFetched);
		STDMETHOD(Skip)(THIS_ DWORD celt);
		STDMETHOD(Reset)(THIS);
		STDMETHOD(Clone)(THIS_ IEnumVARIANT FAR* FAR* ppenum);
		UINT	m_nIndex;
	END_INTERFACE_PART(EnumVARIANT)

	BEGIN_INTERFACE_PART(SRichElement, ISRichItem)
		DECLARE_DISPATCH()
		STDMETHOD(get_Application)(IApplication FAR* FAR* ppApplication);
		STDMETHOD(get_UserInterface)(IUserInterface FAR* FAR* ppUserInterface);
		STDMETHOD(get_Document)(ISRichDocument FAR* FAR* ppDocument);
		STDMETHOD(Remove)();
		STDMETHOD(Show)(VARIANT_BOOL bShow);
		STDMETHOD(SetText)(BSTR strText);
		STDMETHOD(SetFlags)(LONG nFlags, LONG nMask);
		STDMETHOD(get_Size)(LONG* pnSize);
		STDMETHOD(get_ItemType)(RichElementType* pnType);
	END_INTERFACE_PART(SRichElement)

	DECLARE_INTERFACE_MAP()

protected:
	DECLARE_MESSAGE_MAP()
};
