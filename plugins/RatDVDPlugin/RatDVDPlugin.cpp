//
// RatDVDReader.cpp
//
//	Created by:		Rolandas Rudomanskis
//
// Copyright (c) Shareaza Development Team, 2002-2014.
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

#include "stdafx.h"
#include "RatDVDPlugin.h"

LPCWSTR	CRatDVDPlugin::uriVideo			= L"http://www.limewire.com/schemas/video.xsd";

// RatDVDPlugin
CRatDVDPlugin::CRatDVDPlugin()
{
	ODS(_T("CRatDVDPlugin::CRatDVDPlugin\n"));

}

CRatDVDPlugin::~CRatDVDPlugin()
{
	ODS(_T("CRatDVDPlugin::~CRatDVDPlugin\n"));

}

// ILibraryBuilderPlugin Methods

STDMETHODIMP CRatDVDPlugin::Process(BSTR sFile, ISXMLElement* pXML)
{
	ODS(_T("CRatDVDPlugin::Process\n"));

	CHECK_NULL_RETURN(sFile, E_INVALIDARG);

	EnterCritical();
	DllAddRef();

	HRESULT hr = E_FAIL;
	LPCWSTR pszExt = _wcslwr( wcsrchr( sFile, '.') );
	if ( wcsncmp( pszExt, L".ratdvd", 7 ) != 0 )
	{
		DllRelease();
		LeaveCritical();
		return E_UNEXPECTED;
	}

	HANDLE hFile = CreateFile( sFile, GENERIC_READ,
		 FILE_SHARE_READ | FILE_SHARE_DELETE, NULL,
		 OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if ( hFile != INVALID_HANDLE_VALUE )
	{
		hr = ProcessRatDVD( hFile, pXML );

		CloseHandle( hFile );
	}

	DllRelease();
	LeaveCritical();
	return hr;
}

STDMETHODIMP CRatDVDPlugin::ProcessRatDVD(HANDLE hFile, ISXMLElement* pXML)
{
	ODS(_T("CRatDVDPlugin::ProcessRatDVD\n"));

	CHECK_NULL_RETURN(hFile, E_INVALIDARG);

	const DWORD MAX_LENGTH_ALLOWED = 1024;
	SetFilePointer( hFile, 0, NULL, FILE_END );
	bool bContentFound = false, bVersionFound = false;
	CHAR szByte;
	DWORD nRead = 1, nTotalRead = 0;
	std::wstring str;
	DWORD nContentOffset = 0, nVesionOffset = 0;

	// Read the file from end and find content.xml and version.xml offsets
	while ( nRead && nTotalRead <= MAX_LENGTH_ALLOWED )
	{
		if ( SetFilePointer( hFile, -2, NULL, FILE_CURRENT ) == 0 ) break;
		ReadFile( hFile, &szByte, 1, &nRead, NULL );
		nTotalRead++;
		if ( strncmp( &szByte, "K", 1 ) == 0 )
		{
			if ( SetFilePointer( hFile, -2, NULL, FILE_CURRENT ) == 0 ) break;
			ReadFile( hFile, &szByte, 1, &nRead, NULL );
			nTotalRead++;
			if ( strncmp( &szByte, "P", 1 ) == 0 )
			{
				if ( SetFilePointer( hFile, -5, NULL, FILE_CURRENT ) == 0 ) break;
				CHAR szExt[4];
				ReadFile( hFile, &szExt, 4, &nRead, NULL );
				if ( _strnicmp( szExt, ".xml", 4 ) == 0 )
				{
					str.clear();
					// version.xml offset is always 0, so it would be harder to find it
					// if it were located somewhere else
					// Nothing special can be found in it at the moment, maybe they will add later
					while ( szByte && strncmp( &szByte, "\\", 1 ) && nTotalRead <= MAX_LENGTH_ALLOWED )
					{
						if ( SetFilePointer( hFile, -2, NULL, FILE_CURRENT ) == 0 ) break;
						ReadFile( hFile, &szByte, 1, &nRead, NULL );

						LPWSTR pwsz = (LPWSTR)CoTaskMemAlloc( 2 + sizeof(WCHAR) );
						ZeroMemory( pwsz, 2 * sizeof(WCHAR) );
						memcpy( pwsz, &szByte, 1 );

						if ( szByte ) str.insert( 0, pwsz );
						CoTaskMemFree( pwsz );
						nTotalRead++;
					}
					if ( szByte != 0 )
					{
						if ( SetFilePointer( hFile, -5, NULL, FILE_CURRENT ) == 0 ) break;
						ReadFile( hFile, &szExt, 4, &nRead, NULL );
						nTotalRead += 4;
					}
					if ( _strnicmp( szExt, "INFO", 4 ) == 0 || szByte == 0 )
					{
						DWORD nOffset = 0;
						if ( szByte != 0 )
						{
							if ( SetFilePointer( hFile, -8, NULL, FILE_CURRENT ) == 0 ) break;
						}
						else
						{
							if ( SetFilePointer( hFile, -4, NULL, FILE_CURRENT ) == 0 ) break;
						}
						ReadFile( hFile, &nOffset, 4, &nRead, NULL );
						if ( _wcsnicmp( str.c_str(), L"\\content.xm", 11 ) == 0 )
						{
							nContentOffset = nOffset;
							bContentFound = true;
						}
						else if ( _wcsnicmp( str.c_str(), L"version.xm", 11 ) == 0 )
						{
							nVesionOffset = nOffset;
							bVersionFound = true;
						}
					}
				}
				else
					nTotalRead += 4;
			}
		}
		if ( bContentFound && bVersionFound ) break;
	}

	if ( !bContentFound ) return S_FALSE;
	if ( SetFilePointer( hFile, nContentOffset, NULL, FILE_BEGIN ) == 0 ) return S_FALSE;

	// Validate if it starts with "PK"
	nTotalRead = 2;
	CHAR szPK[2] = {};
	ReadFile( hFile, &szPK, 2, &nRead, NULL );
	if ( nRead != 2 || strncmp( szPK, "PK", 2 ) )
		return S_FALSE;

	DWORD nContentLength = 0;
	// Read the .xml file length
	do
	{
		ReadFile( hFile, &szByte, 1, &nRead, NULL );
		if ( nRead != 1 ) return S_FALSE;
		if ( strncmp( &szByte, "I", 1 ) == 0 )
		{
			CHAR szPath[15] = {};
			ReadFile( hFile, &szPath, 15, &nRead, NULL );
			if ( nRead != 15 || _strnicmp( szPath, "NFO\\content.xml", 15 ) ) return S_FALSE;

			// Ok, the chunk is valid, remember the position and read the content length
			DWORD nPos = SetFilePointer( hFile, 0, NULL, FILE_CURRENT );
			SetFilePointer( hFile, nPos - 8 - 16, NULL, FILE_BEGIN );
			ReadFile( hFile, &nContentLength, 4, &nRead, NULL );
			// Return back
			SetFilePointer( hFile, nPos, NULL, FILE_BEGIN );
			break;
		}
		nTotalRead++;
	}
	while ( nTotalRead <= MAX_LENGTH_ALLOWED );

	if ( nContentLength > MAX_LENGTH_ALLOWED * 4 ) return S_FALSE;

	// Read content.xml
	CComBSTR sXML = ReadXML( hFile, nContentLength );

	if ( ! sXML.Length() ) return S_FALSE;

	CComQIPtr< ISXMLElement > pInputXML;

	if ( FAILED( pXML->FromString( sXML, &pInputXML ) ) || pInputXML == NULL )
	{
		return S_FALSE;
	}

	CComQIPtr< ISXMLElements > pElements;
	// Get the Elements collection from the XML document
	if ( FAILED( pInputXML->get_Elements( &pElements ) ) || pElements == NULL )
	{
		pInputXML->Delete();
		return S_FALSE;
	}

	// Get a pointer to elements node and create a root element
	CComQIPtr< ISXMLElement > pPlural;
	CComQIPtr< ISXMLElements > pTempElements;

	pXML->get_Elements( &pTempElements );
	pTempElements->Create( CComBSTR( L"videos" ), &pPlural );
	pTempElements.Release();

	// Add root element attributes
	CComQIPtr< ISXMLAttributes > pAttributes;
	pPlural->get_Attributes( &pAttributes );
	pAttributes->Add( CComBSTR( L"xmlns:xsi" ), CComBSTR( L"http://www.w3.org/2001/XMLSchema-instance" ) );
	pAttributes->Add( CComBSTR( L"xsi:noNamespaceSchemaLocation" ), CComBSTR( CRatDVDPlugin::uriVideo ) );
	pAttributes.Release();

	// Create inner element describing metadata
	CComQIPtr< ISXMLElement > pSingular;
	pPlural->get_Elements( &pTempElements );

	pTempElements->Create( CComBSTR( L"video" ), &pSingular );
	pTempElements.Release();

	// Get attributes and add all metadata
	pSingular->get_Attributes( &pAttributes );

	CComQIPtr< ISXMLElement > pData;
	CComBSTR strValue;

	HRESULT hr = pElements->get_ByName( CComBSTR( L"Titles" ), &pData );
	if ( SUCCEEDED( hr ) && pData )
	{
		if ( SUCCEEDED( pData->get_Elements( &pTempElements ) ) && pTempElements )
		{
			pData.Release();
			if ( SUCCEEDED( pTempElements->get_ByName( CComBSTR( L"Original" ), &pData ) ) && pData )
			{
				if ( SUCCEEDED( pData->get_Value( &strValue ) ) )
					pAttributes->Add( CComBSTR( L"title" ), strValue );
			}
		}
	}

	if ( pData ) pData.Release();
	if ( pTempElements ) pTempElements.Release();

	hr = pElements->get_ByName( CComBSTR( L"tagline" ), &pData );
	if ( SUCCEEDED(hr) && pData )
	{
		if ( SUCCEEDED( pData->get_Value( &strValue ) ) )
			pAttributes->Add( CComBSTR( L"description" ), strValue );
		pData.Release();
	}

	hr = pElements->get_ByName( CComBSTR( L"year" ), &pData );
	if ( SUCCEEDED(hr) && pData )
	{
		if ( SUCCEEDED( pData->get_Value( &strValue ) ) )
			pAttributes->Add( CComBSTR( L"year" ), strValue );
		pData.Release();
	}

	hr = pElements->get_ByName( CComBSTR( L"director" ), &pData );
	if ( SUCCEEDED(hr) && pData )
	{
		if ( SUCCEEDED( pData->get_Value( &strValue ) ) )
			pAttributes->Add( CComBSTR( L"director" ), strValue );
		pData.Release();
	}

	hr = pElements->get_ByName( CComBSTR( L"genre" ), &pData );
	if ( SUCCEEDED(hr) && pData )
	{
		if ( SUCCEEDED( pData->get_Value( &strValue ) ) )
			pAttributes->Add( CComBSTR( L"genre" ), strValue );
		pData.Release();
	}

	hr = pElements->get_ByName( CComBSTR( L"Plot" ), &pData );
	if ( SUCCEEDED( hr ) && pData )
	{
		if ( SUCCEEDED( pData->get_Elements( &pTempElements ) ) && pTempElements )
		{
			pData.Release();
			if ( SUCCEEDED( pTempElements->get_ByName( CComBSTR( L"Original" ), &pData ) ) && pData )
			{
				if ( SUCCEEDED( pData->get_Value( &strValue ) ) )
					pAttributes->Add( CComBSTR( L"realdescription" ), strValue );
			}
		}
	}

	if ( pData ) pData.Release();
	if ( pTempElements ) pTempElements.Release();

	hr = pElements->get_ByName( CComBSTR( L"IMDB" ), &pData );
	if ( SUCCEEDED(hr) && pData )
	{
		if ( SUCCEEDED( pData->get_Value( &strValue ) ) )
		{
			CComBSTR strURL( L"http://www.imdb.com/title/" );
			strURL.Append( strValue );
			pAttributes->Add( CComBSTR( L"imdbLink" ), strURL );
		}
		pData.Release();
	}

	// Loop through all artists
	CComBSTR strActors;
	hr = pElements->get_ByName( CComBSTR( L"Cast" ), &pData );

	if ( SUCCEEDED( hr ) && pData )
	{
		hr = pData->get_Elements( &pTempElements );
		if ( SUCCEEDED( hr ) && pTempElements )
		{
			pData.Release();
			CComQIPtr< ISXMLAttributes > pActorAttributes;
			do
			{
				if ( pData ) pData.Release();
				hr = pTempElements->get_ByName( CComBSTR( L"actor" ), &pData );
				if ( SUCCEEDED(hr) && pData )
				{
					hr = pData->get_Attributes( &pActorAttributes );
					if ( SUCCEEDED(hr) && pActorAttributes )
					{
						CComBSTR strSeparator;
						CComQIPtr< ISXMLAttribute > pSeparator;
						hr = pActorAttributes->get_ByName( CComBSTR( L"separator" ), &pSeparator );
						if ( SUCCEEDED( hr ) && pSeparator )
							pSeparator->get_Value( &strSeparator );

						if ( SUCCEEDED( pData->get_Value( &strValue ) ) )
						{
							if ( strSeparator.Length() )
							{
								CString strTruncated( strValue );
								CString strSep( strSeparator );
								strTruncated = strTruncated.SpanExcluding( strSep );
								strTruncated.Trim( _T("\"") );
								strValue = strTruncated;
							}

							if ( !strActors.Length() )
							{
								strActors.AssignBSTR( strValue );
							}
							else
							{
								strActors.Append( L";" );
								strActors.AppendBSTR( strValue );
							}
							// delete keyword to get the next
							pData->Delete();
						}
					}
					pActorAttributes.Release();
				}
			}
			while ( pData );
		}
	}
	if ( strActors.Length() ) pAttributes->Add( CComBSTR( L"stars" ), strActors );

	// Cleanup destination
	pInputXML->Delete();

	return S_OK;
}

CComBSTR CRatDVDPlugin::ReadXML(HANDLE hFile, DWORD nBytes)
{
	CHAR* pBuffer = new CHAR[ nBytes ];
	DWORD nRead = 0;
	ReadFile( hFile, pBuffer, nBytes, &nRead, NULL );

	if ( nRead != nBytes )
	{
		delete [] pBuffer;
		return "";
	}

	// Just make a buffer enough to fit string
	WCHAR* pszUnicode = new WCHAR[ nBytes + 1 ];

	ConvertToUnicodeEx( pBuffer, nBytes, pszUnicode, nBytes, CP_UTF8 );
	CComBSTR sUnicode( pszUnicode );

	delete [] pszUnicode;

	return sUnicode;
}

// IImageServicePlugin Methods

STDMETHODIMP CRatDVDPlugin::LoadFromFile(BSTR sFile, IMAGESERVICEDATA* pParams, SAFEARRAY** ppImage)
{
	ODS(_T("CRatDVDPlugin::LoadFromFile\n"));

	EnterCritical();
	DllAddRef();

	HRESULT hr = E_FAIL;

	LPCWSTR pszExt = _wcslwr( wcsrchr( sFile, '.') );

	if ( wcsncmp( pszExt, L".ratdvd", 7 ) != 0 )
	{
		DllRelease();
		LeaveCritical();
		return E_UNEXPECTED;
	}

	hr = GetRatDVDThumbnail( sFile, pParams, ppImage );

	DllRelease();
	LeaveCritical();

	return hr;
}

STDMETHODIMP CRatDVDPlugin::GetRatDVDThumbnail(BSTR bsFile, IMAGESERVICEDATA* pParams, SAFEARRAY** ppImage)
{
	ODS(_T("CRatDVDPlugin::GetRatDVDThumbnail\n"));


	CHECK_NULL_RETURN(bsFile, E_INVALIDARG);

	WCHAR pszName[MAX_PATH];
	ULONG ulIdx;
	HRESULT hr = E_FAIL;

	if ( ! FFindQualifiedFileName( bsFile, pszName, &ulIdx ) )
		return STG_E_INVALIDNAME;

	CString strFile( pszName );
	HANDLE hFile = CreateFile( strFile, GENERIC_READ,
		FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN, NULL );

	if ( hFile == INVALID_HANDLE_VALUE ) return hr;

	const DWORD MAX_LENGTH_ALLOWED = 1024;
	SetFilePointer( hFile, 0, NULL, FILE_END );
	bool bFound = false;
	CHAR szByte;
	DWORD nRead = 1, nTotalRead = 0;
	std::wstring str;
	DWORD nContentOffset = 0;

	// Read the image file path from the end and find its offset
	while ( nRead && nTotalRead <= MAX_LENGTH_ALLOWED )
	{
		if ( SetFilePointer( hFile, -2, NULL, FILE_CURRENT ) == 0 ) break;
		ReadFile( hFile, &szByte, 1, &nRead, NULL );
		nTotalRead++;
		if ( strncmp( &szByte, "K", 1 ) == 0 )
		{
			if ( SetFilePointer( hFile, -2, NULL, FILE_CURRENT ) == 0 ) break;
			ReadFile( hFile, &szByte, 1, &nRead, NULL );
			nTotalRead++;
			if ( strncmp( &szByte, "P", 1 ) == 0 )
			{
				if ( SetFilePointer( hFile, -5, NULL, FILE_CURRENT ) == 0 ) break;
				CHAR szExt[4];
				ReadFile( hFile, &szExt, 4, &nRead, NULL );
				if ( _strnicmp( szExt, ".xml", 4 ) ) // check all other extensions having INFO\ path
				{
					str.clear();
					while ( strncmp( &szByte, "\\", 1 ) && nTotalRead <= MAX_LENGTH_ALLOWED )
					{
						if ( SetFilePointer( hFile, -2, NULL, FILE_CURRENT ) == 0 ) break;
						ReadFile( hFile, &szByte, 1, &nRead, NULL );

						LPWSTR pwsz = (LPWSTR)CoTaskMemAlloc( 2 + sizeof(WCHAR) );
						ZeroMemory( pwsz, 2 * sizeof(WCHAR) );
						memcpy( pwsz, &szByte, 1 );

						str.insert( 0, pwsz );
						CoTaskMemFree( pwsz );
						nTotalRead++;
					}
					// store full path
					str.push_back( szExt[3] );
					str.insert( 0, L"NFO" );

					if ( SetFilePointer( hFile, -5, NULL, FILE_CURRENT ) == 0 ) break;
					ReadFile( hFile, &szExt, 4, &nRead, NULL );
					nTotalRead += 4;

					if ( _strnicmp( szExt, "INFO", 4 ) == 0 )
					{
						if ( SetFilePointer( hFile, -8, NULL, FILE_CURRENT ) == 0 ) break;
						ReadFile( hFile, &nContentOffset, 4, &nRead, NULL );
						bFound = true;
					}
				}
				else
					nTotalRead += 4;
			}
		}
		if ( bFound ) break;
	}

	if ( !bFound ) return S_FALSE;
	if ( SetFilePointer( hFile, nContentOffset, NULL, FILE_BEGIN ) == 0 ) return S_FALSE;

	// Validate if it starts with "PK"
	nTotalRead = 2;
	CHAR szPK[2] = {};
	ReadFile( hFile, &szPK, 2, &nRead, NULL );
	if ( nRead != 2 || strncmp( szPK, "PK", 2 ) )
		return S_FALSE;

	DWORD nContentLength = 0;
	DWORD nPathLen = (DWORD)str.length();
	// Read the image file length
	do
	{
		ReadFile( hFile, &szByte, 1, &nRead, NULL );
		if ( nRead != 1 ) return S_FALSE;
		if ( strncmp( &szByte, "I", 1 ) == 0 )
		{
			CHAR* pszPath = new CHAR[ nPathLen + 1 ];
			ReadFile( hFile, pszPath, nPathLen, &nRead, NULL );
			if ( nRead != nPathLen )
			{
				delete [] pszPath;
				return S_FALSE;
			}
			else
			{
				pszPath[ nPathLen ] = 0;
				CString strFirst( pszPath );
				CString strSecond( str.c_str() );
				if ( strFirst.CompareNoCase( strSecond ) )
				{
					delete [] pszPath;
					return S_FALSE;
				}
			}
			// Ok, the chunk is valid, remember the position and read the content length
			DWORD nPos = SetFilePointer( hFile, 0, NULL, FILE_CURRENT );
			SetFilePointer( hFile, nPos - 9 - nPathLen, NULL, FILE_BEGIN );
			ReadFile( hFile, &nContentLength, 4, &nRead, NULL );
			// Return back
			SetFilePointer( hFile, nPos, NULL, FILE_BEGIN );
			delete [] pszPath;
			break;
		}
		nTotalRead++;
	}
	while ( nTotalRead <= MAX_LENGTH_ALLOWED );

	if ( nContentLength > MAX_LENGTH_ALLOWED * 1024 ) return S_FALSE;

	nRead = 0;
	BYTE* pBuffer = new (std::nothrow) BYTE[ nContentLength ];
	if ( ! pBuffer ) return E_OUTOFMEMORY;

	ReadFile( hFile, (VOID*)pBuffer, nContentLength, &nRead, NULL );
	if ( nRead < nContentLength )
	{
		if ( pParams->nFlags & IMAGESERVICE_PARTIAL_IN )
			pParams->nFlags |= IMAGESERVICE_PARTIAL_OUT;
		else
		{
			delete [] pBuffer;
			return S_FALSE;
		}
	}

	ULONG nArray = static_cast<ULONG>(nContentLength);

	// Create 1-dimensional safearray

	SAFEARRAY* psa = SafeArrayCreateVector( VT_UI1, 0, nArray );

	BYTE* pData = NULL;
	hr = SafeArrayAccessData( psa, (VOID**)&pData );
	// copy data from the buffer
	CopyMemory( pData, pBuffer, nContentLength );
	delete [] pBuffer;
	SafeArrayUnaccessData( psa );

	if ( FAILED(hr) ) return E_FAIL;

	// Initialize COM
	HRESULT hr_coinit = CoInitialize( NULL );
	if ( FAILED(hr_coinit) && hr_coinit != RPC_E_CHANGED_MODE ) return E_FAIL;

	CComPtr<IImageServicePlugin> pPNGReader;
	hr = pPNGReader.CoCreateInstance( CLSID_ImageReader, NULL, CLSCTX_ALL );
	if ( FAILED(hr) ) return E_FAIL;

	hr = pPNGReader->LoadFromMemory( CComBSTR( str.c_str() ), psa, pParams, ppImage );

	if ( SUCCEEDED(hr_coinit) ) CoUninitialize();

	return hr;
}

STDMETHODIMP CRatDVDPlugin::LoadFromMemory(BSTR /*sType*/, SAFEARRAY* /*pMemory*/, IMAGESERVICEDATA* /*pParams*/, SAFEARRAY** /*ppImage*/)
{
	ODS(_T("CRatDVDPlugin::LoadFromMemory\n"));

	return E_NOTIMPL;
}

STDMETHODIMP CRatDVDPlugin::SaveToFile(BSTR /*sFile*/, IMAGESERVICEDATA* /*pParams*/, SAFEARRAY* /*pImage*/)
{
	ODS(_T("CRatDVDPlugin::SaveToFile\n"));

	return E_NOTIMPL;
}

STDMETHODIMP CRatDVDPlugin::SaveToMemory(BSTR /*sType*/, SAFEARRAY** /*ppMemory*/, IMAGESERVICEDATA* /*pParams*/, SAFEARRAY* /*pImage*/)
{
	ODS(_T("CRatDVDPlugin::SaveToMemory\n"));

	return E_NOTIMPL;
}
