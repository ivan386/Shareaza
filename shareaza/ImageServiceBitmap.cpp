//
// ImageServiceBitmap.cpp
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "ImageServiceBitmap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CBitmapImageService, CCmdTarget)

BEGIN_INTERFACE_MAP(CBitmapImageService, CCmdTarget)
	INTERFACE_PART(CBitmapImageService, IID_IImageServicePlugin, Service)
END_INTERFACE_MAP()

BEGIN_MESSAGE_MAP(CBitmapImageService, CCmdTarget)
	//{{AFX_MSG_MAP(CBitmapImageService)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBitmapImageService

CBitmapImageService::CBitmapImageService()
{
}

CBitmapImageService::~CBitmapImageService()
{
}

IImageServicePlugin* CBitmapImageService::Create()
{
	CBitmapImageService* pThis = new CBitmapImageService();
	return &pThis->m_xService;
}

/////////////////////////////////////////////////////////////////////////////
// CBitmapImageService operations

IMPLEMENT_UNKNOWN(CBitmapImageService, Service)

STDMETHODIMP CBitmapImageService::XService::LoadFromFile(HANDLE hFile, DWORD nLength, IMAGESERVICEDATA FAR* pParams, SAFEARRAY FAR* FAR* ppImage)
{
	METHOD_PROLOGUE( CBitmapImageService, Service )
	
	LPBYTE pRowBuf, pData;
	BITMAPFILEHEADER pBFH;
	BITMAPINFOHEADER pBIH;
	RGBQUAD pPalette[256];
	LONG nWidth, nHeight;
	DWORD nRead;
	
	DWORD nBaseAddress = SetFilePointer( hFile, 0, NULL, FILE_CURRENT );
	
	ReadFile( hFile, &pBFH, sizeof(pBFH), &nRead, NULL );
	if ( nRead != sizeof(pBFH) ) return E_FAIL;
	ReadFile( hFile, &pBIH, sizeof(pBIH), &nRead, NULL );
	if ( nRead != sizeof(pBIH) ) return E_FAIL;
	
	if ( pBFH.bfType != 'MB' ) return E_FAIL;
	if ( pBIH.biBitCount != 8 && pBIH.biBitCount != 24 ) return E_FAIL;
	
	nWidth	= (int)pBIH.biWidth;
	nHeight	= (int)pBIH.biHeight;
	
	pParams->nWidth		= nWidth;
	pParams->nHeight	= nHeight;
	
	if ( pParams->nFlags & IMAGESERVICE_SCANONLY ) return S_OK;
	
	if ( pBIH.biBitCount == 8 )
	{
		ReadFile( hFile, pPalette, sizeof(RGBQUAD) * 256, &nRead, NULL );
		if ( nRead != sizeof(RGBQUAD) * 256 ) return E_FAIL;
	}
	
	if ( pBFH.bfOffBits )
	{
		if ( SetFilePointer( hFile, nBaseAddress + pBFH.bfOffBits, NULL, FILE_BEGIN )
			 != nBaseAddress + pBFH.bfOffBits ) return E_FAIL;
	}
	
	for ( UINT nInPitch = nWidth * pBIH.biBitCount / 8 ; nInPitch & 3 ; ) nInPitch++;
	for ( UINT nOutPitch = nWidth * 3 ; nOutPitch & 3 ; ) nOutPitch++;

	pRowBuf	= new BYTE[ nInPitch ];

	UINT nArray = nOutPitch * (UINT)nHeight;

	*ppImage = SafeArrayCreateVector( VT_UI1, 0, nArray );
	SafeArrayAccessData( *ppImage, (VOID**)&pData );

	for ( int nY = nHeight - 1 ; nY >= 0 ; nY-- )
	{
		ReadFile( hFile, pRowBuf, nInPitch, &nRead, NULL );

		if ( nRead != nInPitch )
		{
			delete [] pRowBuf;
			SafeArrayUnaccessData( *ppImage );
			if ( pParams->nFlags & IMAGESERVICE_PARTIAL_IN )
			{
				pParams->nFlags |= IMAGESERVICE_PARTIAL_OUT;
				return S_OK;
			}
			else
			{
				SafeArrayDestroy( *ppImage );
				*ppImage = NULL;
				return E_FAIL;
			}
		}


		if ( pBIH.biBitCount == 24 )
		{
			__asm {
			mov eax, nOutPitch
			mov edi, pData
			mul nY
			add edi, eax
			mov esi, pRowBuf
			mov ecx, nWidth
			loop1: mov ax, [esi]
			mov bl, [esi+2]
			mov [edi+1], ah
			mov [edi+2], al
			mov [edi], bl
			add esi, 3
			add edi, 3
			dec ecx
			jnz loop1
			}
		}
		else 
		{
			__asm {
			mov eax, nOutPitch
			mul nY
			mov edi, pData
			add edi, eax
			mov esi, pRowBuf
			mov edx, nWidth
			xor eax, eax
			_loop: mov al, [esi]
			inc esi
			mov ebx, [pPalette+eax*4]
			bswap ebx
			mov [edi], bh
			shr ebx, 16
			mov [edi+1], bx
			add edi, 3
			dec edx
			jnz _loop
			}
		}
	}
	SafeArrayUnaccessData( *ppImage );

	delete [] pRowBuf;

	return S_OK;
}

STDMETHODIMP CBitmapImageService::XService::LoadFromMemory(SAFEARRAY FAR* pMemory, IMAGESERVICEDATA FAR* pParams, SAFEARRAY FAR* FAR* ppImage)
{
	METHOD_PROLOGUE( CBitmapImageService, Service )
	
	LPBYTE pData;
	BITMAPFILEHEADER pBFH;
	BITMAPINFOHEADER pBIH;
	RGBQUAD pPalette[256];
	LONG nWidth, nHeight;

	LPBYTE pSource = NULL;
	LONG nMemory = 0;

	SafeArrayGetUBound( pMemory, 1, &nMemory );
	if ( ++nMemory <= sizeof(pBIH) + sizeof(pBFH) ) return E_FAIL;
	SafeArrayAccessData( pMemory, (void**)&pSource );
	
	CopyMemory( &pBFH, pSource, sizeof(pBFH) );
	pSource += sizeof(pBFH);
	nMemory -= sizeof(pBFH);
	CopyMemory( &pBIH, pSource, sizeof(pBIH) );
	pSource += sizeof(pBIH);
	nMemory -= sizeof(pBIH);
	
	if ( pBFH.bfType != 'MB' || ( pBIH.biBitCount != 8 && pBIH.biBitCount != 24 ) )
	{
		SafeArrayUnaccessData( pMemory );
		return E_FAIL;
	}
	
	nWidth	= (int)pBIH.biWidth;
	nHeight	= (int)pBIH.biHeight;
	
	pParams->nWidth		= nWidth;
	pParams->nHeight	= nHeight;
	
	if ( pParams->nFlags & IMAGESERVICE_SCANONLY )
	{
		SafeArrayUnaccessData( pMemory );
		return S_OK;
	}
	
	if ( pBIH.biBitCount == 8 )
	{
		if ( nMemory < sizeof(RGBQUAD) * 256 )
		{
			SafeArrayUnaccessData( pMemory );
			return E_FAIL;
		}
		CopyMemory( pPalette, pSource, sizeof(RGBQUAD) * 256 );
		pSource += sizeof(RGBQUAD) * 256;
		nMemory -= sizeof(RGBQUAD) * 256;
	}
	
	if ( pBFH.bfOffBits )
	{
		SafeArrayUnaccessData( pMemory );
		SafeArrayGetUBound( pMemory, 1, &nMemory );
		if ( ++nMemory <= (LONG)pBFH.bfOffBits ) return E_FAIL;
		SafeArrayAccessData( pMemory, (void**)&pSource );
		pSource += pBFH.bfOffBits;
		nMemory -= pBFH.bfOffBits;
	}
	
	for ( UINT nInPitch = nWidth * pBIH.biBitCount / 8 ; nInPitch & 3 ; ) nInPitch++;
	for ( UINT nOutPitch = nWidth * 3 ; nOutPitch & 3 ; ) nOutPitch++;
	
	UINT nArray = nOutPitch * (UINT)nHeight;
	
	*ppImage = SafeArrayCreateVector( VT_UI1, 0, nArray );
	SafeArrayAccessData( *ppImage, (VOID**)&pData );
	
	for ( int nY = nHeight - 1 ; nY >= 0 ; nY-- )
	{
		if ( nMemory < (LONG)nInPitch )
		{
			SafeArrayUnaccessData( *ppImage );
			SafeArrayUnaccessData( pMemory );

			if ( pParams->nFlags & IMAGESERVICE_PARTIAL_IN )
			{
				pParams->nFlags |= IMAGESERVICE_PARTIAL_OUT;
				return S_OK;
			}
			else
			{
				SafeArrayDestroy( *ppImage );
				*ppImage = NULL;
				return E_FAIL;
			}
		}
		pSource += nInPitch;
		nMemory -= nInPitch;

		if ( pBIH.biBitCount == 24 )
		__asm {
			mov eax, nOutPitch
			mov edi, pData
			mul nY
			add edi, eax
			mov esi, pSource
			mov ecx, nWidth
			loop1: mov ax, [esi]
			mov bl, [esi+2]
			mov [edi+1], ah
			mov [edi+2], al
			mov [edi], bl
			add esi, 3
			add edi, 3
			dec ecx
			jnz loop1
		}
		else 
		{
		__asm {
			mov eax, nOutPitch
			mul nY
			mov edi, pData
			add edi, eax
			mov esi, pSource
			mov edx, nWidth
			xor eax, eax
			_loop: mov al, [esi]
			inc esi
			mov ebx, [pPalette+eax*4]
			bswap ebx
			mov [edi], bh
			shr ebx, 16
			mov [edi+1], bx
			add edi, 3
			dec edx
			jnz _loop
		}
		}
	}
	SafeArrayUnaccessData( *ppImage );
	SafeArrayUnaccessData( pMemory );
	
	return S_OK;
}

STDMETHODIMP CBitmapImageService::XService::SaveToFile(HANDLE hFile, IMAGESERVICEDATA FAR* pParams, SAFEARRAY FAR* pImage)
{
	METHOD_PROLOGUE( CBitmapImageService, Service )
	return E_NOTIMPL;
}

STDMETHODIMP CBitmapImageService::XService::SaveToMemory(SAFEARRAY FAR* FAR* ppMemory, IMAGESERVICEDATA FAR* pParams, SAFEARRAY FAR* pImage)
{
	METHOD_PROLOGUE( CBitmapImageService, Service )
	return E_NOTIMPL;
}

