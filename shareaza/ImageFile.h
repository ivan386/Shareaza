//
// ImageFile.h
//
// Copyright (c) Shareaza Development Team, 2002-2017.
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


class CImageFile
{
// Construction
public:
	CImageFile();
	virtual ~CImageFile();

public:
	BOOL			m_bScanned;
	int				m_nWidth;
	int				m_nHeight;
	DWORD			m_nComponents;
	LPBYTE			m_pImage;
	WORD			m_nFlags;

// Operations
public:
	BOOL	LoadFromMemory(LPCTSTR pszType, LPCVOID pData, DWORD nLength, BOOL bScanOnly = FALSE, BOOL bPartialOk = FALSE);
	BOOL	LoadFromFile(LPCTSTR pszFile, BOOL bScanOnly = FALSE, BOOL bPartialOk = FALSE);
	BOOL	LoadFromURL(LPCTSTR pszURL);
	BOOL	LoadFromResource(HINSTANCE hInstance, UINT nResourceID, LPCTSTR pszType);
	// Get image copy from HBITMAP (24/32-bit only)
	BOOL	LoadFromBitmap(HBITMAP hBitmap, BOOL bAlpha, BOOL bScanOnly = FALSE);
	BOOL	LoadFromService(const IMAGESERVICEDATA* pParams, SAFEARRAY* pArray = NULL);
	BOOL	SaveToMemory(LPCTSTR pszType, int nQuality, LPBYTE* ppBuffer, DWORD* pnLength);
	BOOL	SaveToFile(LPCTSTR pszFile, int nQuality, DWORD* pnLength = NULL);
	DWORD	GetSerialSize() const;
	void	Serialize(CArchive& ar);
	HBITMAP	CreateBitmap(HDC hUseDC = 0);
	BOOL	FitTo(int nNewWidth, int nNewHeight);
	BOOL	Resample(int nNewWidth, int nNewHeight);
//	BOOL	FastResample(int nNewWidth, int nNewHeight);
	BOOL	EnsureRGB(COLORREF crBack = 0xFFFFFFFF);
	BOOL	SwapRGB();

	inline BOOL IsLoaded() const
	{
		return ( m_pImage != NULL );
	}

	static HBITMAP LoadBitmapFromFile(LPCTSTR pszFile);
	static HBITMAP LoadBitmapFromResource(UINT nResourceID, HINSTANCE hInstance = AfxGetResourceHandle());

	enum ImageFlags
	{
		idRemote = 0x1
	};

protected:
	void	Clear();
	BOOL	MonoToRGB();
	BOOL	AlphaToRGB(COLORREF crBack);

private:
	CImageFile(const CImageFile&);
	CImageFile& operator=(CImageFile&);
};
