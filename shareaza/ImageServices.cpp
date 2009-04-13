//
// ImageServices.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2009.
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

CImageServices ImageServices;

/////////////////////////////////////////////////////////////////////////////
// CImageServices construction

CImageServices::CImageServices()
{
}

CImageServices::~CImageServices()
{
	ASSERT( m_services.size() == 0 ); // Must be already cleared

	Clear();
}

void CImageServices::Clear()
{
	CloseThread();
}

void CImageServices::OnRun()
{
	while( IsThreadEnabled() )
	{
		Doze();

		if ( ! IsThreadEnabled() )
			break;

		ASSERT( m_services.find( m_inCLSID ) == m_services.end() );

		// Create plugin
		CComPtr< IImageServicePlugin > pService;
		HRESULT hr = pService.CoCreateInstance( m_inCLSID );

		if ( SUCCEEDED( hr ) )
		{
			// Add to cache
			CComGITPtr< IImageServicePlugin > oGIT;
			hr = oGIT.Attach( pService );
			ASSERT( SUCCEEDED( hr ) );
			if ( SUCCEEDED( hr ) )
				m_services.insert( services_map::value_type( m_inCLSID, oGIT.Detach() ) );
		}

		m_pReady.SetEvent();
	}

	CQuickLock oLock( m_pSection );

	// Revoke all interfaces
	for ( services_map::iterator i = m_services.begin(); i != m_services.end(); ++i )
		CComGITPtr< IImageServicePlugin > oGIT( (*i).second );
	m_services.clear();
}

/////////////////////////////////////////////////////////////////////////////
// CImageServices load operations

BOOL CImageServices::LoadFromMemory(CImageFile* pFile, LPCTSTR pszType, LPCVOID pData, DWORD nLength, BOOL bScanOnly, BOOL bPartialOk)
{
	ASSERT( pFile );
	ASSERT( pszType );
	ASSERT( pData );

	BOOL bSuccess = FALSE;

	CComPtr< IImageServicePlugin> pService;
	if ( GetService( pszType, &pService ) )
	{
		SAFEARRAY* pInput;
		if ( SUCCEEDED( SafeArrayAllocDescriptor( 1, &pInput ) ) && pInput )
		{
			pInput->cbElements = 1;
			pInput->rgsabound[ 0 ].lLbound = 0;
			pInput->rgsabound[ 0 ].cElements = nLength;
			if ( SUCCEEDED( SafeArrayAllocData( pInput ) ) )
			{
				LPBYTE pTarget;
				if ( SUCCEEDED( SafeArrayAccessData( pInput, (void HUGEP* FAR*)&pTarget ) ) )
				{
					CopyMemory( pTarget, pData, nLength );
					VERIFY( SUCCEEDED( SafeArrayUnaccessData( pInput ) ) );

					CComBSTR bstrType( pszType );
					HINSTANCE hRes = AfxGetResourceHandle();

					SAFEARRAY* pArray = NULL;
					IMAGESERVICEDATA pParams = {};
					pParams.cbSize = sizeof( pParams );
					if ( bScanOnly ) pParams.nFlags |= IMAGESERVICE_SCANONLY;
					if ( bPartialOk ) pParams.nFlags |= IMAGESERVICE_PARTIAL_IN;
					if ( SUCCEEDED( pService->LoadFromMemory( bstrType, pInput,
						&pParams, &pArray ) ) )
					{
						bSuccess = PostLoad( pFile, &pParams, pArray );
					}
					if ( pArray )
					{
						VERIFY( SUCCEEDED( SafeArrayDestroy( pArray ) ) );
					}

					AfxSetResourceHandle( hRes );
				}
				VERIFY( SUCCEEDED( SafeArrayDestroyData( pInput ) ) );
			}
			VERIFY( SUCCEEDED( SafeArrayDestroyDescriptor( pInput ) ) );
		}
	}

	return bSuccess;
}

BOOL CImageServices::LoadFromFile(CImageFile* pFile, LPCTSTR szFilename, BOOL bScanOnly, BOOL bPartialOk)
{
	ASSERT( pFile );
	ASSERT( szFilename );

	BOOL bSuccess = FALSE;

	CComPtr< IImageServicePlugin> pService;
	if ( GetService( szFilename, &pService ) )
	{
		CComBSTR bstrFile( szFilename );
		HINSTANCE hRes = AfxGetResourceHandle();

		SAFEARRAY* pArray = NULL;
		IMAGESERVICEDATA pParams = {};
		pParams.cbSize = sizeof( pParams );
		if ( bScanOnly ) pParams.nFlags |= IMAGESERVICE_SCANONLY;
		if ( bPartialOk ) pParams.nFlags |= IMAGESERVICE_PARTIAL_IN;
		HRESULT hr = pService->LoadFromFile( bstrFile, &pParams, &pArray );
		if ( SUCCEEDED( hr ) )
		{
			bSuccess = PostLoad( pFile, &pParams, pArray );
		}
		if ( pArray )
		{
			VERIFY( SUCCEEDED( SafeArrayDestroy( pArray ) ) );
		}

		AfxSetResourceHandle( hRes );

		// Second chance - load from memory
		if ( ! bSuccess && ( hr == E_NOTIMPL ) )
		{
			HANDLE hFile = CreateFile( szFilename, GENERIC_READ,
				FILE_SHARE_READ | FILE_SHARE_DELETE,
				NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
			VERIFY_FILE_ACCESS( hFile, szFilename )
			if ( hFile != INVALID_HANDLE_VALUE )
			{
				if ( GetFileSize( hFile, NULL ) < 10*1024*1024 )	// Max size 10 MB
				{
					HANDLE hMap = CreateFileMapping( hFile, NULL, PAGE_READONLY,
						0, 0, NULL );
					if ( hMap )
					{
						LPCVOID pBuffer = MapViewOfFile( hMap, FILE_MAP_READ,
							0, 0, 0 );
						if ( pBuffer )
						{
							bSuccess = LoadFromMemory( pFile, szFilename, pBuffer,
								GetFileSize( hFile, NULL ), bScanOnly, bPartialOk );

							VERIFY( UnmapViewOfFile( pBuffer ) );
						}
						VERIFY( CloseHandle( hMap ) );
					}
				}
				VERIFY( CloseHandle( hFile ) );
			}
		}
	}
	return bSuccess;
}

/////////////////////////////////////////////////////////////////////////////
// CImageServices post load

BOOL CImageServices::PostLoad(CImageFile* pFile, const IMAGESERVICEDATA* pParams, SAFEARRAY* pArray)
{
	ASSERT( pFile );
	ASSERT( pParams );

	pFile->m_bScanned		= TRUE;
	pFile->m_nWidth			= pParams->nWidth;
	pFile->m_nHeight		= pParams->nHeight;
	pFile->m_nComponents	= pParams->nComponents;
	if ( pArray == NULL )
	{
		// Scanned only
		return TRUE;
	}

	LONG nArray = 0;
	if ( SUCCEEDED( SafeArrayGetUBound( pArray, 1, &nArray ) ) )
	{
		nArray++;
		LONG nFullSize = pParams->nWidth * pParams->nComponents;
		while ( nFullSize & 3 ) nFullSize++;
		nFullSize *= pParams->nHeight;
		if ( nArray == nFullSize )
		{
			pFile->m_pImage = new BYTE[ nArray ];
			if ( pFile->m_pImage )
			{
				LPBYTE pData;
				if ( SUCCEEDED( SafeArrayAccessData( pArray, (VOID**)&pData ) ) )
				{
					CopyMemory( pFile->m_pImage, pData, nArray );
					pFile->m_bLoaded = TRUE;

					VERIFY( SUCCEEDED( SafeArrayUnaccessData( pArray ) ) );
				}
			}
		}
	}

	return pFile->m_bLoaded;
}

/////////////////////////////////////////////////////////////////////////////
// CImageServices save operations

BOOL CImageServices::SaveToMemory(CImageFile* pFile, LPCTSTR pszType, int nQuality, LPBYTE* ppBuffer, DWORD* pnLength)
{
	*ppBuffer = NULL;
	*pnLength = 0;

	CComPtr< IImageServicePlugin> pService;
	if ( ! GetService( pszType, &pService ) )
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
	pService->SaveToMemory( bstrType, &pOutput, &pParams, pSource );
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
	// Get file extension
	CString strType( PathFindExtension( pszPath ) );
	if ( strType.IsEmpty() )
		return FALSE;
	strType.MakeLower();

	CLSID oCLSID;
	return Plugins.LookupCLSID( L"ImageService", strType, oCLSID, FALSE );
}

/////////////////////////////////////////////////////////////////////////////
// CImageServices service discovery and control

bool CImageServices::GetService(LPCTSTR szFilename, IImageServicePlugin** ppIImageServicePlugin)
{
	// Get file extension
	CString strType( PathFindExtension( szFilename ) ); // ".ext"
	strType.MakeLower();

	// Get plugin CLSID
	CLSID oCLSID;
	if ( ! Plugins.LookupCLSID( L"ImageService", strType, oCLSID, FALSE ) )
		// Unknown or disabled extension
		return false;

	// Check cached one
	DWORD dwCachedIndex;
	{
		CQuickLock oLock( m_pSection );
		services_map::iterator i = m_services.find( oCLSID );
		if ( i == m_services.end() )
		{
			// Create new one
			m_inCLSID = oCLSID;							// Set input parameter
			if ( ! BeginThread( "ImageServices" ) )
				return false;
			Wakeup();									// Start process
			WaitForSingleObject( m_pReady, INFINITE );	// Wait for result
			i = m_services.find( oCLSID );				// Get result
			if ( i == m_services.end() )
				// No plugin
				return false;
		}
		dwCachedIndex = (*i).second;
	}

	// Just add to the plugin collection for the reference of CLSID.
	// Not a very nice solution but without checking all plugins
	// Shareaza holds only 1 plugin in the Plugins collection...
/*
	CPlugin* pPlugin = Plugins.Find( oCLSID );
	if ( ! pPlugin )
	{
		// Put an empty name, so it won't be displayed in the Settings
		pPlugin = new CPlugin( oCLSID, L"" );
		Plugins.m_pList.AddTail( pPlugin );
	}
*/

	// Get interface from cache
	CComGITPtr< IImageServicePlugin > oGIT( dwCachedIndex );
	HRESULT hr = oGIT.CopyTo( ppIImageServicePlugin );
	ASSERT( SUCCEEDED( hr ) );

	oGIT.Detach();

	return SUCCEEDED( hr );
}
