//
// ImageServices.h
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

#pragma once

#include "ImageFile.h"


class CImageServices : public CComObject
{
// Construction
public:
	CImageServices();
	virtual ~CImageServices();

	DECLARE_DYNAMIC(CImageServices)

// Operations
public:
	void					Cleanup();
protected:
	BOOL					LoadFromMemory(CImageFile* pFile, LPCTSTR pszType, LPCVOID pData, DWORD nLength, BOOL bScanOnly = FALSE, BOOL bPartialOk = FALSE);
	BOOL					LoadFromFile(CImageFile* pFile, LPCTSTR pszType, HANDLE hFile, DWORD nLength, BOOL bScanOnly = FALSE, BOOL bPartialOk = FALSE);
	BOOL					PostLoad(CImageFile* pFile, IMAGESERVICEDATA* pParams, SAFEARRAY* pArray, BOOL bSuccess);
protected:
	BOOL					SaveToMemory(CImageFile* pFile, LPCTSTR pszType, int nQuality, LPBYTE* ppBuffer, DWORD* pnLength);
	BOOL					SaveToFile(CImageFile* pFile, LPCTSTR pszType, int nQuality, HANDLE hFile, DWORD* pnLength = NULL);
	SAFEARRAY*				ImageToArray(CImageFile* pFile);
protected:
	IImageServicePlugin*	GetService(LPCTSTR pszFile, CLSID** ppCLSID = NULL);
	IImageServicePlugin*	LoadService(LPCTSTR pszType, CLSID* pCLSID = NULL);

// Static Load Tool
public:
	static BOOL				LoadBitmap(CBitmap* pBitmap, UINT nResourceID, LPCTSTR pszType);
	
// Attributes
protected:
	CMapStringToPtr			m_pService;
	CMapStringToPtr			m_pCLSID;
	BOOL					m_bCOM;
	
	friend class CImageFile;
};

extern LPCTSTR RT_JPEG;
extern LPCTSTR RT_PNG;
