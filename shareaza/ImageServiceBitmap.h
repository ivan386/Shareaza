//
// ImageServiceBitmap.h
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

#if !defined(AFX_IMAGESERVICEBITMAP_H__FF700A57_852F_4A86_BE0A_D021A67C94AA__INCLUDED_)
#define AFX_IMAGESERVICEBITMAP_H__FF700A57_852F_4A86_BE0A_D021A67C94AA__INCLUDED_

#pragma once


class CBitmapImageService : public CCmdTarget
{
// Construction
public:
	CBitmapImageService();
	virtual ~CBitmapImageService();
	
	DECLARE_DYNAMIC(CBitmapImageService)

// Operations
public:
	static IImageServicePlugin* Create();

// Interface
protected:
	BEGIN_INTERFACE_PART(Service, IImageServicePlugin)
		STDMETHOD(LoadFromFile)(HANDLE hFile, DWORD nLength, IMAGESERVICEDATA FAR* pParams, SAFEARRAY FAR* FAR* ppImage);
		STDMETHOD(LoadFromMemory)(SAFEARRAY FAR* pMemory, IMAGESERVICEDATA FAR* pParams, SAFEARRAY FAR* FAR* ppImage);
		STDMETHOD(SaveToFile)(HANDLE hFile, IMAGESERVICEDATA FAR* pParams, SAFEARRAY FAR* pImage);
		STDMETHOD(SaveToMemory)(SAFEARRAY FAR* FAR* ppMemory, IMAGESERVICEDATA FAR* pParams, SAFEARRAY FAR* pImage);
	END_INTERFACE_PART(Service)
	
	DECLARE_INTERFACE_MAP()
	
// Implementation
protected:
	//{{AFX_VIRTUAL(CBitmapImageService)
	//}}AFX_VIRTUAL
	//{{AFX_MSG(CBitmapImageService)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_IMAGESERVICEBITMAP_H__FF700A57_852F_4A86_BE0A_D021A67C94AA__INCLUDED_)
