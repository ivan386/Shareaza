//
// ImageFile.h
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

class CImageServices;

// NOTE: Dont call CImageFile's Load/Save methods across thread boundaries!

class CImageFile : public CComObject
{
// Construction
public:
	CImageFile();
	virtual ~CImageFile();

	DECLARE_DYNAMIC(CImageFile)

public:
	BOOL			m_bScanned;
	int				m_nWidth;
	int				m_nHeight;
	int				m_nComponents;
	BOOL			m_bLoaded;
	LPBYTE			m_pImage;
	CImageServices	m_ImageServices;

// Operations
public:
	BOOL	LoadFromMemory(LPCTSTR pszType, LPCVOID pData, DWORD nLength, BOOL bScanOnly = FALSE, BOOL bPartialOk = FALSE);
	BOOL	LoadFromFile(LPCTSTR pszFile, BOOL bScanOnly = FALSE, BOOL bPartialOk = FALSE);
	BOOL	LoadFromURL(LPCTSTR pszURL);
	BOOL	LoadFromResource(HINSTANCE hInstance, UINT nResourceID, LPCTSTR pszType, BOOL bScanOnly = FALSE, BOOL bPartialOk = FALSE);
	BOOL	SaveToMemory(LPCTSTR pszType, int nQuality, LPBYTE* ppBuffer, DWORD* pnLength);
//	BOOL	SaveToFile(LPCTSTR pszType, int nQuality, HANDLE hFile, DWORD* pnLength = NULL);
//	BOOL	SaveToFile(LPCTSTR pszFile, int nQuality);
	DWORD	GetSerialSize() const;
	void	Serialize(CArchive& ar);
	HBITMAP	CreateBitmap(HDC hUseDC = 0);
	BOOL	Resample(int nNewWidth, int nNewHeight);
//	BOOL	FastResample(int nNewWidth, int nNewHeight);
	BOOL	EnsureRGB(COLORREF crBack = 0xFFFFFFFF);
	BOOL	SwapRGB();

protected:
	void	Clear();
	BOOL	MonoToRGB();
	BOOL	AlphaToRGB(COLORREF crBack);
};
