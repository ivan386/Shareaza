//
// Builder.h : Declaration of the CBuilder
//
// Copyright (c) Nikolay Raspopov, 2005-2010.
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

#include "resource.h"
#include "MediaLibraryBuilder.h"

class ATL_NO_VTABLE CBuilder : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CBuilder, &CLSID_Builder>,
	public ILibraryBuilderPlugin
{
public:
	CBuilder () throw()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_BUILDER)

BEGIN_COM_MAP(CBuilder)
	COM_INTERFACE_ENTRY(ILibraryBuilderPlugin)
END_COM_MAP()

// ILibraryBuilderPlugin
public:
	STDMETHOD(Process)(
		/* [in] */ BSTR sFile,
		/* [in] */ ISXMLElement* pXML);
private:
	HRESULT SafeProcess(BSTR sFile, ISXMLElement* pXML);
};

OBJECT_ENTRY_AUTO(__uuidof(Builder), CBuilder)
