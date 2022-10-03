//
// MatchListView.h
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

class CMatchList;
class CDownloads;


class CMatchListView : public CComObject
{
	DECLARE_DYNAMIC(CMatchListView)

public:
	static IGenericView* Attach(LPCTSTR pszName, CMatchList* pList);
	static IGenericView* Attach(LPCTSTR pszName, CDownloads* pDownloads);

protected:
	CString					m_sName;
	CList< CShareazaFile >	m_pSelection;

	CMatchListView();
	CMatchListView(LPCTSTR pszName, CMatchList* pList);
	CMatchListView(LPCTSTR pszName, CDownloads* pDownloads);
	virtual ~CMatchListView();

// Automation
	BEGIN_INTERFACE_PART(GenericView, IGenericView)
		DECLARE_DISPATCH()
		STDMETHOD(get_Name)(BSTR FAR* psName);
		STDMETHOD(get_Unknown)(IUnknown FAR* FAR* ppUnknown);
		STDMETHOD(get_Param)(LONG FAR* pnParam);
		STDMETHOD(get_Count)(LONG FAR* pnCount);
		STDMETHOD(get_Item)(VARIANT vIndex, VARIANT FAR* pvItem);
		STDMETHOD(get__NewEnum)(IUnknown FAR* FAR* ppEnum);
	END_INTERFACE_PART(GenericView)

	BEGIN_INTERFACE_PART(EnumVARIANT, IEnumVARIANT)
		STDMETHOD(Next)(THIS_ DWORD celt, VARIANT FAR* rgvar, DWORD FAR* pceltFetched);
		STDMETHOD(Skip)(THIS_ DWORD celt);
		STDMETHOD(Reset)(THIS);
		STDMETHOD(Clone)(THIS_ IEnumVARIANT FAR* FAR* ppenum);
		POSITION m_pos;
	END_INTERFACE_PART(EnumVARIANT)

	DECLARE_INTERFACE_MAP()
};
