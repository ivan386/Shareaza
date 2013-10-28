//
// Image.h
//
// This software is released into the public domain. You are free to
// redistribute and modify without any restrictions.
// This file is part of SHAREAZA (shareaza.sourceforge.net), original author Michael Stokes. 
//

#pragma once


class CImage  
{
public:
	CImage();
	virtual ~CImage();
	
	BYTE*	m_pImage;		// Pointer to image data
	int		m_nWidth;		// Width
	int		m_nHeight;		// Height
	int		m_nComponents;	// Components (1=mono, 3=RGB, 4=RGBA)
	BOOL	m_bPartial;		// Is it partially loaded?
	
	BOOL	Load(LPCTSTR pszFile);
	void	Clear();
	BOOL	EnsureRGB(COLORREF crFill = 0xFFFFFFFF);
	BOOL	MonoToRGB();
	BOOL	AlphaToRGB(COLORREF crFill);
	HBITMAP	Resample(int nWidth, int nHeight);
	
// Internal Helpers
protected:
	IImageServicePlugin*	LoadService(LPCTSTR pszFile);
};
