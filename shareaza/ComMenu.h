//
// ComMenu.h
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

#pragma once


class CComMenu : public CComObject
{
// Construction
public:
	CComMenu(HMENU hMenu, UINT nPosition = 0xFFFFFFFF);
	virtual ~CComMenu();

// Attributes
public:
	HMENU	m_hParent;
	HMENU	m_hMenu;
	UINT	m_nPosition;

// Operations
public:
	static ISMenu*	Wrap(HMENU hMenu, UINT nPosition = 0xFFFFFFFF);

// ISMenu
protected:
	BEGIN_INTERFACE_PART(SMenu, ISMenu)
		DECLARE_DISPATCH()
		STDMETHOD(get_Application)(IApplication FAR* FAR* ppApplication);
		STDMETHOD(get_UserInterface)(IUserInterface FAR* FAR* ppUserInterface);
		STDMETHOD(get__NewEnum)(IUnknown FAR* FAR* ppEnum);
		STDMETHOD(get_Item)(VARIANT vIndex, ISMenu FAR* FAR* ppMenu);
		STDMETHOD(get_Count)(LONG FAR* pnCount);
		STDMETHOD(get_ItemType)(SMenuType FAR* pnType);
		STDMETHOD(get_CommandID)(LONG FAR* pnCommandID);
		STDMETHOD(put_CommandID)(LONG nCommandID);
		STDMETHOD(get_Text)(BSTR FAR* psText);
		STDMETHOD(put_Text)(BSTR sText);
		STDMETHOD(get_HotKey)(BSTR FAR* psText);
		STDMETHOD(put_HotKey)(BSTR sText);
		STDMETHOD(Remove)();
		STDMETHOD(InsertSeparator)(LONG nPosition);
		STDMETHOD(InsertMenu)(LONG nPosition, BSTR sText, ISMenu FAR* FAR* ppMenu);
		STDMETHOD(InsertCommand)(LONG nPosition, LONG nCommandID, BSTR sText, ISMenu FAR* FAR* ppMenu);
		STDMETHOD(get_Position)(LONG FAR* pnCommandID);
		STDMETHOD(get_Parent)(ISMenu FAR* FAR* ppMenu);
	END_INTERFACE_PART(SMenu)

	BEGIN_INTERFACE_PART(EnumVARIANT, IEnumVARIANT)
		STDMETHOD(Next)(THIS_ DWORD celt, VARIANT FAR* rgvar, DWORD FAR* pceltFetched);
		STDMETHOD(Skip)(THIS_ DWORD celt);
		STDMETHOD(Reset)(THIS);
		STDMETHOD(Clone)(THIS_ IEnumVARIANT FAR* FAR* ppenum);
		UINT	m_nIndex;
	END_INTERFACE_PART(EnumVARIANT)

	DECLARE_INTERFACE_MAP()

	DECLARE_MESSAGE_MAP()
};
