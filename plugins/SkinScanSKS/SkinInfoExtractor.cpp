//
// SkinInfoExtractor.cpp
//
//	Date:			"$Date: 2005/05/11 17:22:56 $"
//	Revision:		"$Revision: 1.1 $"
//  Last change by:	"$Author: spooky23 $"
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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
#include "SkinScanSKS.h"
#include "SkinInfoExtractor.h"
#include <zlib.h>
#include "unzip/Unzip.h"


///////////////////////////////////////////////////////////////////////////////
// 
// The Process() method in ILibraryBuilderPlugin is called when a file
// from the library needs to be scanned for metadata by this plugin.
// 
// hFile : An open handle to the file, with read access.  Don't assume the
//         file pointer is at the start of the file, and be sure not to
//         close the handle.
// 
// sFile : The path and filename of the file to scan.  This can be used
//         instead of hFile if you need to access the file via some other
//         method.
//
// pXML  : A root XML element in which to create the metadata elements.
//         The metadata plural element should be a child of this root element.
// 
// The XML you create should be in the following form:
// 
// <root>
//  <shareazaSkins xsi:noNamespaceSchemaLocation="http://www.shareaza.com/schemas/shareazaSkin.xsd">
//   <shareazaSkin name="X" description="Y"/>
//  </shareazaSkins>
// </root>
//
// Return S_OK if successful, or an error code if not.
// 

HRESULT STDMETHODCALLTYPE CSkinInfoExtractor::Process(HANDLE hFile, BSTR sFile, ISXMLElement __RPC_FAR* pXML)
{
	//
	// Open the file with the UNZIP library
	//

	LPTSTR pszFile = GetSysString( sFile );
	unzFile pFile = unzOpen( pszFile );
	delete [] pszFile;

	//
	// If the file can't be opened, fail now
	//

	if ( ! pFile ) return E_FAIL;

	unz_global_info pDir;
	
	//
	// Get the global info data from the ZIP file
	//

	if ( unzGetGlobalInfo( pFile, &pDir ) != UNZ_OK )
	{
		pDir.number_entry = 0;
	}
	
	//
	// Loop through the files in the ZIP file
	//

	for ( UINT nFile = 0 ; nFile < pDir.number_entry ; nFile++ )
	{
		unz_file_info pInfo;
		CHAR szFile[256];
		
		//
		// Try to seek to the next compressed file
		//

		if ( nFile && unzGoToNextFile( pFile ) != UNZ_OK )
		{
			break;
		}
		
		//
		// Get information about the current file
		//

		if ( unzGetCurrentFileInfo( pFile, &pInfo, szFile, sizeof(szFile), NULL, 0, NULL, 0 ) != UNZ_OK )
		{
			break;
		}
		
		//
		// If the compressed file is XML, examine it further
		//

		if (	strstr( szFile, ".xml" ) != NULL ||
				strstr( szFile, ".XML" ) != NULL )
		{
			//
			// Try to open the compressed file
			//

			if ( unzOpenCurrentFile( pFile ) != UNZ_OK ) break;
			
			//
			// Allocate memory to decompress the file
			//

			LPSTR pszXML = new CHAR[ pInfo.uncompressed_size + 1 ];
			if ( ! pszXML ) break;
			
			//
			// Try to read (and decompress) the XML file
			//

			if ( unzReadCurrentFile( pFile, pszXML, pInfo.uncompressed_size ) > 0 )
			{
				//
				// Make the XML buffer a null-terminated string
				//

				pszXML[ pInfo.uncompressed_size ] = 0;

				//
				// Call the ScanFile helper function
				//

				if ( ScanFile( pszXML, pXML ) )
				{
					//
					// Successful, so free the data and close the file, and return
					// S_OK to Shareaza
					//

					delete [] pszXML;
					unzCloseCurrentFile( pFile );
					unzClose( pFile );
					return S_OK;
				}
			}
			
			//
			// No good -- free the buffer and close the compressed file
			//

			delete [] pszXML;
			unzCloseCurrentFile( pFile );
		}
	}
	
	//
	// Reached the end of the file without finding a valid skin XML file,
	// so close the ZIP file and return E_FAIL
	//

	unzClose( pFile );

	return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////
// 
// Utility function to convert a BSTR to a LPTSTR
// 

LPTSTR CSkinInfoExtractor::GetSysString(BSTR bstr)
{
	int nLen = SysStringLen( bstr );

	LPTSTR psz = new TCHAR[ nLen + 1 ];

	LPTSTR pszOut = psz;
	LPCWSTR pszIn = (LPCWSTR)bstr;

	while ( nLen-- ) *pszOut++ = (TCHAR)*pszIn++;
	*pszOut++ = 0;
	
	return psz;
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

BOOL CSkinInfoExtractor::ScanFile(LPCSTR pszXML, ISXMLElement* pOutput)
{
	ISXMLElement* pFile = NULL;
	CComBSTR str, strVal;
	
	//
	// Put the XML string in a BSTR
	//

	str = pszXML;

	//
	// Use the FromString() method in ISXMLElement to decode an XML document
	// from the XML string.  Output is in "pFile".
	//
	
	if ( FAILED( pOutput->FromString( str, &pFile ) ) || pFile == NULL )
	{
		return FALSE;
	}
	
	//
	// Test if the root element of the document is called "skin"
	//

	VARIANT_BOOL bNamed = VARIANT_FALSE;
	str = _T("skin");

	pFile->IsNamed( str, &bNamed );

	if ( ! bNamed )
	{
		pFile->Delete();
		pFile->Release();
		return FALSE;
	}

	ISXMLElements* pElements;
	
	//
	// Get the Elements collection from the XML document
	//

	if ( FAILED( pFile->get_Elements( &pElements ) ) || pElements == NULL )
	{
		pFile->Delete();
		pFile->Release();
		return FALSE;
	}

	//
	// Find the <manifest> element
	//

	ISXMLElement* pManifest;
	str = _T("manifest");

	if ( FAILED( pElements->get_ByName( str, &pManifest ) ) || pManifest == NULL )
	{
		pElements->Release();
		pFile->Delete();
		pFile->Release();
		return FALSE;
	}
	
	pElements->Release();

	//
	// Add the plural <shareazaSkins> element
	//

	ISXMLElement* pPlural;

	str = _T("shareazaSkins");
	pOutput->get_Elements( &pElements );
	pElements->Create( str, &pPlural );
	pElements->Release();

	//
	// Add xsi:noNamespaceSchemaLocation="http://www.shareaza.com/schemas/shareazaSkin.xsd"
	// to the Attributes collection of <shareazaSkins>
	//

	{
		ISXMLAttributes* pAttrs;
		pPlural->get_Attributes( &pAttrs );

		str = _T("xsi:noNamespaceSchemaLocation");
		strVal = _T("http://www.shareaza.com/schemas/shareazaSkin.xsd");

		pAttrs->Add( str, strVal );
		pAttrs->Release();
	}

	//
	// Change <manifest> to <shareazaSkin>
	//

	str = _T("shareazaSkin");
	pManifest->put_Name( str );

	//
	// Detach <manifest> from the file document and add it to the
	// output XML document
	//

	{
		pManifest->Detach();
		pPlural->get_Elements( &pElements );
		pElements->Attach( pManifest );
		pElements->Release();
	}
	
	//
	// Cleanup
	//

	pManifest->Release();
	pPlural->Release();
	pFile->Delete();
	pFile->Release();
	
	//
	// Return success
	//

	return TRUE;
}
