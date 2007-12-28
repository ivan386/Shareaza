//
// ComToolbar.h
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

#if !defined(AFX_COMTOOLBAR_H__CA9210C9_DDBF_4949_AB60_15FFBA51C859__INCLUDED_)
#define AFX_COMTOOLBAR_H__CA9210C9_DDBF_4949_AB60_15FFBA51C859__INCLUDED_

#pragma once

class CCoolBarCtrl;
class CCoolBarItem;


class CComToolbar : public CComObject
{
// Construction
public:
	CComToolbar(CCoolBarCtrl* pBar, CCoolBarItem* pItem);
	virtual ~CComToolbar();

// Attributes
public:
	CCoolBarCtrl* m_pBar;
	CCoolBarItem* m_pItem;

// Operations
public:
	static ISToolbar*		Wrap(CCoolBarCtrl* pBar);
	static ISToolbarItem*	Wrap(CCoolBarCtrl* pBar, CCoolBarItem* pItem);

// ISToolbar
protected:
	BEGIN_INTERFACE_PART(SToolbar, ISToolbar)
		DECLARE_DISPATCH()
		STDMETHOD(get_Application)(IApplication FAR* FAR* ppApplication);
		STDMETHOD(get_UserInterface)(IUserInterface FAR* FAR* ppUserInterface);
		STDMETHOD(get__NewEnum)(IUnknown FAR* FAR* ppEnum);
		STDMETHOD(get_Item)(VARIANT vIndex, ISToolbarItem FAR* FAR* ppItem);
		STDMETHOD(get_Count)(LONG FAR* pnCount);
		STDMETHOD(InsertSeparator)(LONG nPosition);
		STDMETHOD(InsertButton)(LONG nPosition, LONG nCommandID, BSTR sText, ISToolbarItem FAR* FAR* ppItem);
	END_INTERFACE_PART(SToolbar)

	BEGIN_INTERFACE_PART(SToolbarItem, ISToolbarItem)
		DECLARE_DISPATCH()
		STDMETHOD(get_Application)(IApplication FAR* FAR* ppApplication);
		STDMETHOD(get_UserInterface)(IUserInterface FAR* FAR* ppUserInterface);
		STDMETHOD(get_Toolbar)(ISToolbar FAR* FAR* ppToolbar);
		STDMETHOD(get_ItemType)(SToolbarType FAR* pnType);
		STDMETHOD(get_CommandID)(LONG FAR* pnCommandID);
		STDMETHOD(put_CommandID)(LONG nCommandID);
		STDMETHOD(get_Text)(BSTR FAR* psText);
		STDMETHOD(put_Text)(BSTR sText);
		STDMETHOD(Remove)();
	END_INTERFACE_PART(SToolbarItem)

	BEGIN_INTERFACE_PART(EnumVARIANT, IEnumVARIANT)
		STDMETHOD(Next)(THIS_ DWORD celt, VARIANT FAR* rgvar, DWORD FAR* pceltFetched);
		STDMETHOD(Skip)(THIS_ DWORD celt);
		STDMETHOD(Reset)(THIS);
		STDMETHOD(Clone)(THIS_ IEnumVARIANT FAR* FAR* ppenum);
		UINT	m_nIndex;
	END_INTERFACE_PART(EnumVARIANT)

	DECLARE_INTERFACE_MAP()

// Implementation
protected:
	//{{AFX_MSG(CComToolbar)
	//}}AFX_MSG
	//{{AFX_VIRTUAL(CComToolbar)
	//}}AFX_VIRTUAL

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_COMTOOLBAR_H__CA9210C9_DDBF_4949_AB60_15FFBA51C859__INCLUDED_)
