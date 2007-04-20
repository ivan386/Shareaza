//
// ImageServices.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "Plugins.h"
#include "ImageServices.h"
#include "ImageFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CImageServices, CComObject)

/////////////////////////////////////////////////////////////////////////////
// CImageServices construction

CImageServices::CImageServices()
{
}

CImageServices::~CImageServices()
{
}

/////////////////////////////////////////////////////////////////////////////
// CImageServices load operations

BOOL CImageServices::LoadFromMemory(CImageFile* pFile, LPCTSTR pszType, LPCVOID pData, DWORD nLength, BOOL bScanOnly, BOOL bPartialOk)
{
	IImageServicePlugin* pService = GetService( pszType ).first;
	if ( pService == NULL ) return FALSE;

	IMAGESERVICEDATA pParams = {};
	pParams.cbSize		= sizeof(pParams);
	if ( bScanOnly ) pParams.nFlags |= IMAGESERVICE_SCANONLY;
	if ( bPartialOk ) pParams.nFlags |= IMAGESERVICE_PARTIAL_IN;
	
	SAFEARRAY* pInput;
	LPBYTE pTarget;
	
	if ( FAILED( SafeArrayAllocDescriptor( 1, &pInput ) ) || pInput == NULL ) return FALSE;
	
	pInput->cbElements = 1;
	pInput->rgsabound[ 0 ].lLbound = 0;
	pInput->rgsabound[ 0 ].cElements = nLength;
	SafeArrayAllocData( pInput );
	
	if ( FAILED( SafeArrayAccessData( pInput, (void HUGEP* FAR*)&pTarget ) ) ) return FALSE;
	
	CopyMemory( pTarget, pData, nLength );
	SafeArrayUnaccessData( pInput );
	
	SAFEARRAY* pArray = NULL;
	HINSTANCE hRes = AfxGetResourceHandle();
	BSTR bstrType = SysAllocString ( CT2CW (pszType));
	BOOL bSuccess = SUCCEEDED( pService->LoadFromMemory( bstrType, pInput, &pParams, &pArray ) );
	SysFreeString (bstrType);
	AfxSetResourceHandle( hRes );
	
	SafeArrayDestroy( pInput );
	
	return PostLoad( pFile, &pParams, pArray, bSuccess );
}

BOOL CImageServices::LoadFromFile(CImageFile* pFile, LPCTSTR szFilename, BOOL bScanOnly, BOOL bPartialOk)
{
	PluginInfo service = GetService( szFilename );
	if ( !service.first )
		return FALSE;

	IMAGESERVICEDATA pParams = {};
	pParams.cbSize		= sizeof(pParams);
	if ( bScanOnly ) pParams.nFlags |= IMAGESERVICE_SCANONLY;
	if ( bPartialOk ) pParams.nFlags |= IMAGESERVICE_PARTIAL_IN;

	SAFEARRAY* pArray	= NULL;
	HINSTANCE hRes		= AfxGetResourceHandle();
	BSTR sFile			= SysAllocString (CT2CW (szFilename));
	HRESULT hr			= service.first->LoadFromFile( sFile, &pParams, &pArray );
	SysFreeString (sFile);
	AfxSetResourceHandle( hRes );
	
	if ( hr != E_NOTIMPL ) return PostLoad( pFile, &pParams, pArray, SUCCEEDED( hr ) );
	
	// Second chance - load from memory
	pFile->Clear();
	if ( pArray != NULL ) SafeArrayDestroy( pArray );
	
	BOOL bMapped = FALSE;	
	HANDLE hFile = CreateFile( szFilename, GENERIC_READ,
		FILE_SHARE_READ | ( theApp.m_bNT ? FILE_SHARE_DELETE : 0 ),
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	VERIFY_FILE_ACCESS( hFile, szFilename )
	if ( hFile != INVALID_HANDLE_VALUE )
	{
		HANDLE hMap = CreateFileMapping( hFile, NULL, PAGE_READONLY, 0, 0, NULL );
		if ( hMap )
		{		
			LPCVOID pBuffer = MapViewOfFile( hMap, FILE_MAP_READ, 0, 0, 0 );
			if ( pBuffer )
			{
				bMapped = LoadFromMemory( pFile, szFilename, pBuffer,
					GetFileSize (hFile, NULL), bScanOnly, bPartialOk );
				UnmapViewOfFile( pBuffer );
			}
			CloseHandle( hMap );
		}
		CloseHandle( hFile );
	}
	return bMapped;
}

/////////////////////////////////////////////////////////////////////////////
// CImageServices post load

BOOL CImageServices::PostLoad(CImageFile* pFile, IMAGESERVICEDATA* pParams, SAFEARRAY* pArray, BOOL bSuccess)
{
	pFile->Clear();

	if ( ! bSuccess )
	{
		if ( pArray != NULL ) SafeArrayDestroy( pArray );
		return FALSE;
	}

	pFile->m_bScanned		= TRUE;
	pFile->m_nWidth			= pParams->nWidth;
	pFile->m_nHeight		= pParams->nHeight;
	pFile->m_nComponents	= pParams->nComponents;

	if ( pArray == NULL ) return TRUE;
	
	pFile->m_bLoaded = TRUE;
	
	LONG nArray = 0;
	SafeArrayGetUBound( pArray, 1, &nArray );
	nArray++;
	
	LONG nFullSize = pParams->nWidth * pParams->nComponents;
	while ( nFullSize & 3 ) nFullSize++;
	nFullSize *= pParams->nHeight;
	
	if ( nArray != nFullSize )
	{
		SafeArrayDestroy( pArray );
		return FALSE;
	}
	
	pFile->m_pImage = new BYTE[ nArray ];
	
	LPBYTE pData;
	SafeArrayAccessData( pArray, (VOID**)&pData );
	CopyMemory( pFile->m_pImage, pData, nArray );
	SafeArrayUnaccessData( pArray );
	SafeArrayDestroy( pArray );
	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CImageServices save operations

BOOL CImageServices::SaveToMemory(CImageFile* pFile, LPCTSTR pszType, int nQuality, LPBYTE* ppBuffer, DWORD* pnLength)
{
	*ppBuffer = NULL;
	*pnLength = 0;

	PluginInfo service = GetService( pszType );
	if ( !service.first )
		return FALSE;

	SAFEARRAY* pSource = ImageToArray( pFile );
	if ( pSource == NULL ) return FALSE;

	IMAGESERVICEDATA pParams = {};
	pParams.cbSize		= sizeof(pParams);
	pParams.nWidth		= pFile->m_nWidth;
	pParams.nHeight		= pFile->m_nHeight;
	pParams.nComponents	= pFile->m_nComponents;
	pParams.nQuality	= nQuality;
	
	SAFEARRAY* pOutput = NULL;
	HINSTANCE hRes = AfxGetResourceHandle();
	BSTR bstrType = SysAllocString ( CT2CW (pszType));
	service.first->SaveToMemory( bstrType, &pOutput, &pParams, pSource );
	SysFreeString (bstrType);
	AfxSetResourceHandle( hRes );
	
	SafeArrayDestroy( pSource );
	
	if ( pOutput == NULL ) return FALSE;
	
	SafeArrayGetUBound( pOutput, 1, (PLONG)pnLength );
	(*pnLength)++;

	LPBYTE pEncoded;
	SafeArrayAccessData( pOutput, (VOID**)&pEncoded );
	
	*ppBuffer = new BYTE[ *pnLength ];
	CopyMemory( *ppBuffer, pEncoded, *pnLength );
	
	SafeArrayUnaccessData( pOutput );
	SafeArrayDestroy( pOutput );
	
	return TRUE;
}

/*BOOL CImageServices::SaveToFile(CImageFile* pFile, LPCTSTR pszType, int nQuality, HANDLE hFile, DWORD* pnLength)
{
	if ( pnLength ) *pnLength = 0;
	
	IImageServicePlugin* pService = GetService( pszType );
	if ( pService == NULL ) return FALSE;
	
	SAFEARRAY* pSource = ImageToArray( pFile );
	if ( pSource == NULL ) return FALSE;
	
	IMAGESERVICEDATA pParams = {};
	pParams.cbSize		= sizeof(pParams);
	pParams.nWidth		= pFile->m_nWidth;
	pParams.nHeight		= pFile->m_nHeight;
	pParams.nComponents	= pFile->m_nComponents;
	pParams.nQuality	= nQuality;
	
	DWORD nBefore = SetFilePointer( hFile, 0, NULL, FILE_CURRENT );
	
	HINSTANCE hRes = AfxGetResourceHandle();
	BOOL bSuccess = SUCCEEDED( pService->SaveToFile( hFile, &pParams, pSource ) );
	AfxSetResourceHandle( hRes );
	
	SafeArrayDestroy( pSource );
	
	if ( pnLength )
	{
		DWORD nAfter = SetFilePointer( hFile, 0, NULL, FILE_CURRENT );
		*pnLength = nAfter - nBefore;
	}
	
	return bSuccess;
}*/

/////////////////////////////////////////////////////////////////////////////
// CImageServices pre save utility

SAFEARRAY* CImageServices::ImageToArray(CImageFile* pFile)
{
	SAFEARRAY* pOutput;
	
	if ( FAILED( SafeArrayAllocDescriptor( 1, &pOutput ) ) || pOutput == NULL ) return NULL;
	
	DWORD nLength = pFile->m_nWidth * pFile->m_nComponents;
	while ( nLength & 3 ) nLength ++;
	nLength *= pFile->m_nHeight;
	
	pOutput->cbElements = 1;
	pOutput->rgsabound[ 0 ].lLbound = 0;
	pOutput->rgsabound[ 0 ].cElements = nLength;
	
	if ( FAILED( SafeArrayAllocData( pOutput ) ) ) return NULL;
	
	LPBYTE pTarget;
	if ( FAILED( SafeArrayAccessData( pOutput, (void HUGEP* FAR*)&pTarget ) ) ) return NULL;
	
	CopyMemory( pTarget, pFile->m_pImage, nLength );
	
	SafeArrayUnaccessData( pOutput );
	
	return pOutput;
}

BOOL CImageServices::IsFileViewable(LPCTSTR pszPath)
{
	CString strType = pszPath;
	
	int nExtPos = strType.ReverseFind( '.' );
	if ( nExtPos > 0 ) strType = strType.Mid( nExtPos );
	
	CharLower( strType.GetBuffer() );
	strType.ReleaseBuffer();

	if ( strType.GetLength() )
	{
		// Loads only once and adds
		PluginInfo service = GetService( pszPath );
		if ( !service.first )
			return FALSE;

		if ( Plugins.LookupEnable( service.second, FALSE, strType ) )
			return TRUE;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CImageServices service discovery and control

CImageServices::PluginInfo CImageServices::GetService(const CString& strFile)
{
	int dotPos = strFile.ReverseFind( '.' );
	if ( dotPos < 0 )
		return PluginInfo();
	CString strType( ToLower( strFile.Mid( dotPos ) ) );

	{
		const_iterator pService = m_services.find( strType );
		if ( pService != m_services.end() )
			return pService->second;
	}

	PluginInfo service = LoadService( strType );
	m_services.insert( services_map::value_type( strType, service ) );

	return service;
}

CImageServices::PluginInfo CImageServices::LoadService(const CString& strType)
{
	CLSID oCLSID;

	if ( !Plugins.LookupCLSID( L"ImageService", strType, oCLSID ) )
		return PluginInfo();

	HINSTANCE hRes = AfxGetResourceHandle();
	AfxSetResourceHandle( hRes );

	CComPtr< IImageServicePlugin > pService;
	if ( FAILED( CoCreateInstance( oCLSID, NULL, CLSCTX_ALL,
		IID_IImageServicePlugin, (void**)&pService ) ) )
	{
		return PluginInfo();
	}

	// Just add to the plugin collection for the reference of CLSID.
	// Not a very nice solution but without checking all plugins
	// Shareaza holds only 1 plugin in the Plugins collection... 

    CPlugin* pPlugin = NULL;
	if ( ( pPlugin = Plugins.Find( oCLSID ) ) == NULL ) 
	{
		// Put an empty name, so it won't be displayed in the Settings
		pPlugin = new CPlugin( oCLSID, L"" );
		Plugins.m_pList.AddTail( pPlugin );
	}

	return PluginInfo(
			CComQIPtr< IImageServicePlugin >( static_cast< IImageServicePlugin* >( pService ) ),
			oCLSID );
}

/////////////////////////////////////////////////////////////////////////////
// CImageServices load bitmap

BOOL CImageServices::LoadBitmap(CBitmap* pBitmap, UINT nResourceID, LPCTSTR pszType)
{
	if ( pBitmap->m_hObject == NULL ) pBitmap->DeleteObject();

	CImageFile pFile;
	if ( ! pFile.LoadFromResource( AfxGetResourceHandle(), nResourceID, pszType ) ) return FALSE;
	if ( ! pFile.EnsureRGB() ) return FALSE;
	pBitmap->Attach( pFile.CreateBitmap() );

	return TRUE;
}
