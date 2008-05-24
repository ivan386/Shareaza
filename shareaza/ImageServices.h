//
// ImageServices.h
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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

class CImageFile;

// NOTE: Dont call CImageServices's methods across thread boundaries!

class CImageServices : public CComObject
{
// Construction
public:
	CImageServices();
	virtual ~CImageServices();

	DECLARE_DYNAMIC(CImageServices)

// Static Load Tool
	static BOOL	LoadBitmap(CBitmap* pBitmap, UINT nResourceID, LPCTSTR pszType);
	static BOOL	IsFileViewable(LPCTSTR pszPath);

// Implementation
protected:
	typedef std::pair< CComQIPtr< IImageServicePlugin >, CLSID > PluginInfo;
	typedef std::map< CString, PluginInfo > services_map;
	typedef services_map::const_iterator const_iterator;

	// Operations
	BOOL		LoadFromMemory(CImageFile* pFile, LPCTSTR pszType, LPCVOID pData, DWORD nLength, BOOL bScanOnly = FALSE, BOOL bPartialOk = FALSE);
	BOOL		LoadFromFile(CImageFile* pFile, LPCTSTR szFilename, BOOL bScanOnly = FALSE, BOOL bPartialOk = FALSE);
	BOOL		SaveToMemory(CImageFile* pFile, LPCTSTR pszType, int nQuality, LPBYTE* ppBuffer, DWORD* pnLength);
//	BOOL		SaveToFile(CImageFile* pFile, LPCTSTR pszType, int nQuality, HANDLE hFile, DWORD* pnLength = NULL);

	BOOL		PostLoad(CImageFile* pFile, const IMAGESERVICEDATA* pParams, SAFEARRAY* pArray);
	SAFEARRAY*	ImageToArray(CImageFile* pFile);
	PluginInfo	GetService(const CString& strFile);

// Attributes
	services_map	m_services;

	friend class CImageFile;
};
