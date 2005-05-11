//
// SkinInfoExtractor.h
//
//	Date:			"$Date: 2005/05/11 17:22:56 $"
//	Revision:		"$Revision: 1.1 $"
//  Last change by:	"$Author: spooky23 $"
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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

#ifndef __SKININFOEXTRACTOR_H_
#define __SKININFOEXTRACTOR_H_

#include "Resource.h"


class ATL_NO_VTABLE CSkinInfoExtractor : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSkinInfoExtractor, &CLSID_SkinInfoExtractor>,
	public ILibraryBuilderPlugin
{
// Construction
public:
	CSkinInfoExtractor() {}

	DECLARE_REGISTRY_RESOURCEID(IDR_SKININFOEXTRACTOR)

	BEGIN_COM_MAP(CSkinInfoExtractor)
		COM_INTERFACE_ENTRY(ILibraryBuilderPlugin)
	END_COM_MAP()

// ILibraryBuilderPlugin
public:
	virtual HRESULT STDMETHODCALLTYPE Process(
            /* [in] */ HANDLE hFile,
            /* [in] */ BSTR sFile,
            /* [in] */ ISXMLElement __RPC_FAR *pXML);

// Internals
protected:
	LPTSTR	GetSysString(BSTR bstr);
	BOOL	ScanFile(LPCSTR pszXML, ISXMLElement* pOutput);

};

#endif //__SKININFOEXTRACTOR_H_
