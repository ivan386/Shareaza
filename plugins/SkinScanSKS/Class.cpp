//
// Class.cpp : Implementation of CClass
//
// Copyright (c) Shareaza Development Team, 2007-2014.
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
#include "Class.h"

class ATL_NO_VTABLE CZipHandler
{
public:
	inline CZipHandler( LPCTSTR szFilename ) throw()
	{
		hArchive = unzOpen( CW2A( szFilename ) );
		if ( ! hArchive )
		{
			TCHAR szFileShort[ MAX_PATH ];
			if ( GetShortPathName( szFilename, szFileShort, MAX_PATH ) )
				hArchive = unzOpen( CW2A( szFileShort ) );
		}
	}

	inline ~CZipHandler() throw()
	{
		if ( hArchive )
		{
			unzClose( hArchive );
			hArchive = NULL;
		}
	}

	inline operator unzFile() const throw()
	{
		return hArchive;
	}

protected:
	unzFile hArchive;
};

STDMETHODIMP CSkinScanSKS::Process (
	/* [in] */ BSTR sFile,
	/* [in] */ ISXMLElement* pXML)
{
	if ( ! pXML )
		return E_POINTER;

	CZipHandler pFile( sFile );
	if ( ! pFile )
		return E_FAIL;

	unz_global_info pDir = {};
	if ( unzGetGlobalInfo( pFile, &pDir ) != UNZ_OK )
	{
		// Bad format. Call CLibraryBuilder::SubmitCorrupted()
		return E_UNEXPECTED;
	}

	for ( UINT nFile = 0; nFile < pDir.number_entry; nFile++ )
	{
		unz_file_info pInfo = {};
		CHAR szFile[ MAX_PATH ];

		if ( nFile && unzGoToNextFile( pFile ) != UNZ_OK )
		{
			// Bad format. Call CLibraryBuilder::SubmitCorrupted()
			return E_UNEXPECTED;
		}

		if ( unzGetCurrentFileInfo( pFile, &pInfo, szFile,
			MAX_PATH, NULL, 0, NULL, 0 ) != UNZ_OK )
		{
			// Bad format. Call CLibraryBuilder::SubmitCorrupted()
			return E_UNEXPECTED;
		}

		if ( lstrcmpiA( szFile + lstrlenA( szFile ) - 4, ".xml" ) == 0 )
		{
			if ( unzOpenCurrentFile( pFile ) != UNZ_OK )
			{
				// Bad format. Call CLibraryBuilder::SubmitCorrupted()
				return E_UNEXPECTED;
			}

			LPSTR pszXML = new (std::nothrow) CHAR[ pInfo.uncompressed_size + 1 ];
			if ( ! pszXML )
			{
				unzCloseCurrentFile( pFile );
				return E_OUTOFMEMORY;
			}
			ZeroMemory( pszXML, pInfo.uncompressed_size + 1 );

			if ( unzReadCurrentFile( pFile, pszXML, pInfo.uncompressed_size ) < 0 )
			{
				delete [] pszXML;
				unzCloseCurrentFile( pFile );
				// Bad format. Call CLibraryBuilder::SubmitCorrupted()
				return E_UNEXPECTED;
			}

			pszXML[ pInfo.uncompressed_size ] = 0;

			if ( ScanFile( pszXML, pXML ) )
			{
				delete [] pszXML;
				unzCloseCurrentFile( pFile );
				return S_OK;
			}

			delete [] pszXML;
			unzCloseCurrentFile( pFile );
		}
	}

	return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////
// 
// ScanFile() is a helper function which accepts an XML string, decodes
// it, checks if it is a Shareaza skin file, and copies the <manifest>
// metadata to the output.
// 
// pszXML  : The XML string
//
// pOutput : An empty XML element to build metadata in
// 

BOOL CSkinScanSKS::ScanFile(LPCSTR pszXML, ISXMLElement* pOutput)
{
	// Put the XML string in a BSTR
	BOOL bBOMPresent = FALSE;
	if ( lstrlenA( pszXML ) > 3 && (UCHAR)pszXML[0] == 0xEF && 
		(UCHAR)pszXML[1] == 0xBB && (UCHAR)pszXML[2] == 0xBF )
	{
		bBOMPresent = TRUE;
		pszXML += 3;
	}
	int nLength = MultiByteToWideChar(CP_UTF8, 0, pszXML, lstrlenA(pszXML), NULL, 0);
	WCHAR* pszUnicode = new WCHAR[ nLength + 1 ];
	MultiByteToWideChar(CP_UTF8, 0, pszXML, lstrlenA(pszXML), pszUnicode, nLength);
	pszUnicode[ nLength ] = 0;
	CComBSTR sUnicode( pszUnicode );
	delete [] pszUnicode;
	if ( bBOMPresent ) pszXML -= 3;

	// Use the FromString() method in ISXMLElement to decode an XML document
	// from the XML string.  Output is in "pFile".
	CComPtr< ISXMLElement > pFile;
	if ( FAILED( pOutput->FromString( sUnicode, &pFile ) ) ||
		pFile == NULL )
	{
		return FALSE;
	}

	// Test if the root element of the document is called "skin"
	VARIANT_BOOL bNamed = VARIANT_FALSE;
	pFile->IsNamed( CComBSTR( _T("skin") ), &bNamed );
	if ( ! bNamed )
	{
		pFile->Delete();
		return FALSE;
	}

	// Get the Elements collection from the XML document
	CComPtr< ISXMLElements > pElements;
	if ( FAILED( pFile->get_Elements( &pElements ) ) ||
		pElements == NULL )
	{
		pFile->Delete();
		return FALSE;
	}

	// Find the <manifest> element
	CComPtr< ISXMLElement > pManifest;
	if ( FAILED( pElements->get_ByName( CComBSTR( _T("manifest") ), &pManifest ) ) ||
		pManifest == NULL )
	{
		pFile->Delete();
		return FALSE;
	}

	// Add the plural <shareazaSkins> element
	CComPtr< ISXMLElement > pPlural;
	{
		CComPtr< ISXMLElements > pOutputElements;
		if ( FAILED( pOutput->get_Elements( &pOutputElements ) ) ||
			pOutputElements == NULL )
		{
			pFile->Delete();
			return FALSE;
		}
		pOutputElements->Create( CComBSTR( _T("shareazaSkins") ), &pPlural );
	}

	// Add xsi:noNamespaceSchemaLocation="http://www.shareaza.com/schemas/shareazaSkin.xsd"
	// to the Attributes collection of <shareazaSkins>
	{
		CComPtr< ISXMLAttributes > pAttrs;
		pPlural->get_Attributes( &pAttrs );
		pAttrs->Add( CComBSTR( _T("xsi:noNamespaceSchemaLocation") ),
			CComBSTR( _T("http://www.shareaza.com/schemas/shareazaSkin.xsd") ) );
	}

	// Change <manifest> to <shareazaSkin>
	pManifest->put_Name( CComBSTR( _T("shareazaSkin") ) );

	// Detach <manifest> from the file document and add it to the
	// output XML document
	pManifest->Detach();
	{
		CComPtr< ISXMLElements > pPluralElements;
		pPlural->get_Elements( &pPluralElements );
		pPluralElements->Attach( pManifest );
	}

	pFile->Delete();

	return TRUE;
}
