//
// ImageFile.h
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

class CImageServices;


class CImageFile : public CComObject
{
// Construction
public:
	CImageFile(CImageServices* pService);
	virtual ~CImageFile();

	DECLARE_DYNAMIC(CImageFile)

// Attributes
protected:
	CImageServices*	m_pService;
public:
	BOOL	m_bScanned;
	int		m_nWidth;
	int		m_nHeight;
	int		m_nComponents;
public:
	BOOL	m_bLoaded;
	LPBYTE	m_pImage;

// Operations
public:
	void	Clear();
public:
	BOOL	LoadFromMemory(LPCTSTR pszType, LPCVOID pData, DWORD nLength, BOOL bScanOnly = FALSE, BOOL bPartialOk = FALSE);
	BOOL	LoadFromFile(LPCTSTR pszType, HANDLE hFile, DWORD nLength, BOOL bScanOnly = FALSE, BOOL bPartialOk = FALSE);
	BOOL	LoadFromFile(LPCTSTR pszFile, BOOL bScanOnly = FALSE, BOOL bPartialOk = FALSE);
	BOOL	LoadFromResource(HINSTANCE hInstance, UINT nResourceID, LPCTSTR pszType, BOOL bScanOnly = FALSE, BOOL bPartialOk = FALSE);
public:
	BOOL	SaveToMemory(LPCTSTR pszType, int nQuality, LPBYTE* ppBuffer, DWORD* pnLength);
	BOOL	SaveToFile(LPCTSTR pszType, int nQuality, HANDLE hFile, DWORD* pnLength = NULL);
	BOOL	SaveToFile(LPCTSTR pszFile, int nQuality);
public:
	DWORD	GetSerialSize() const;
	void	Serialize(CArchive& ar);
public:
	HBITMAP	CreateBitmap(HDC hUseDC = 0);
	BOOL	Resample(int nNewWidth, int nNewHeight);
	BOOL	FastResample(int nNewWidth, int nNewHeight);
	BOOL	EnsureRGB(COLORREF crBack = 0xFFFFFFFF);
	BOOL	MonoToRGB();
	BOOL	AlphaToRGB(COLORREF crBack);
	BOOL	SwapRGB();
};
