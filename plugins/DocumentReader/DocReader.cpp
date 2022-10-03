//
// DocReader.cpp
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
#include "DocReader.h"
#include "Palette.h"

LPCWSTR	CDocReader::uriBook			= L"http://www.limewire.com/schemas/book.xsd";
LPCWSTR	CDocReader::uriDocument		= L"http://www.shareaza.com/schemas/wordProcessing.xsd";
LPCWSTR	CDocReader::uriSpreadsheet	= L"http://www.shareaza.com/schemas/spreadsheet.xsd";
LPCWSTR CDocReader::uriPresentation	= L"http://www.shareaza.com/schemas/presentation.xsd";

LPCWSTR	CDocReader::msWordExt		= L".doc.dot";
LPCWSTR	CDocReader::msExcelExt		= L".xls.xlt.xla";
LPCWSTR	CDocReader::msPPointExt		= L".ppt.pot.ppa";
LPCWSTR	CDocReader::msProjectExt	= L".mpp.mpt";
LPCWSTR	CDocReader::msVisioExt		= L".vsd.vst";
LPCWSTR	CDocReader::ooWriterExt		= L".odt.ott.sxw.stw";
LPCWSTR	CDocReader::ooCalcExt		= L".ods.ots.sxc.stc";
LPCWSTR	CDocReader::ooImpressExt	= L".odp.otp.sxi.sti";


// CDocReader
CDocReader::CDocReader()
	: m_pDocProps( NULL )
{
	ODS(_T("CDocReader::CDocReader\n"));
}

CDocReader::~CDocReader()
{
	ODS(_T("CDocReader::~CDocReader\n"));

	delete m_pDocProps;
}

// ILibraryBuilderPlugin Methods

STDMETHODIMP CDocReader::Process(BSTR sFile, ISXMLElement* pXML)
{
	ODS(_T("CDocReader::Process\n"));

	EnterCritical();
	DllAddRef();
	
	// Create a new instance of the control
	delete m_pDocProps;
	m_pDocProps = new CDocumentProperties( FALSE );

	HRESULT hr = E_FAIL;
	LPCWSTR pszExt = _wcslwr( wcsrchr( sFile, '.') );
	LPCWSTR pszSchema = GetSchema( sFile, pszExt );
	LPCWSTR pszFormat = GetDocumentFormat( pszExt );

	if ( ! pszFormat || ! pszSchema )
	{
		DllRelease();
		LeaveCritical();
		return E_UNEXPECTED;
	}
	if ( pszFormat[ 0 ] == 'M' ) // Microsoft
	{
		if ( wcslen( pszExt ) == 5 && pszExt[4] == 'x' )
			hr = ProcessNewMSDocument( sFile, pXML, pszSchema, pszFormat );
		else
			hr = ProcessMSDocument( sFile, pXML, pszSchema, pszFormat );
	}
	else if ( pszFormat[ 0 ] == 'O' ) // OpenOffice or OpenDocument
		hr = ProcessOODocument( sFile, pXML, pszSchema, pszFormat );

	DllRelease();
	LeaveCritical();
	return hr;
}


STDMETHODIMP CDocReader::ProcessMSDocument(BSTR bsFile, ISXMLElement* pXML, LPCWSTR pszSchema, LPCWSTR pszFormat)
{
	ODS(_T("CDocReader::ProcessMSDocument\n"));
	HRESULT hr;
	BSTR bsValue = NULL;
	LPCWSTR pszSingular = NULL;
	CString sTemp;
	LONG nCount = 0;

	hr = m_pDocProps->Open( bsFile, VARIANT_TRUE, dsoOptionOpenReadOnlyIfNoWriteAccess );
	if ( FAILED(hr) )
	{
		m_pDocProps->Close( VARIANT_FALSE );
		return E_INVALIDOBJECT;
	}

	// Check if it was MS document
	VARIANT_BOOL bMSDoc = VARIANT_FALSE;
	hr = m_pDocProps->get_IsOleFile( &bMSDoc );
	if ( FAILED(hr) || bMSDoc == VARIANT_FALSE )
	{
		m_pDocProps->Close( VARIANT_FALSE );
		return E_FAIL;
	}

	pszSingular = wcsrchr( pszSchema, _T('/') ) + 1;

	sTemp.Append( CW2T(pszSingular), static_cast< int >( wcslen( pszSingular ) - 4 ) );
	sTemp.Append( _T("s") );

	// Get a pointer to elements node and create a root element
	ISXMLElement* pPlural;
	ISXMLElements* pElements;

	pXML->get_Elements( &pElements );
	pElements->Create( CComBSTR( sTemp ), &pPlural );
	pElements->Release();

	// Add root element attributes
	ISXMLAttributes* pAttributes;
	pPlural->get_Attributes( &pAttributes );
	pAttributes->Add( CComBSTR( L"xmlns:xsi" ), CComBSTR( L"http://www.w3.org/2001/XMLSchema-instance" ) );
	pAttributes->Add( CComBSTR( L"xsi:noNamespaceSchemaLocation" ), CComBSTR( pszSchema ) );
	pAttributes->Release();

	// Create inner element describing metadata
	ISXMLElement* pSingular;
	pPlural->get_Elements( &pElements );

	sTemp = sTemp.Left( sTemp.GetLength() - 1 );
	pElements->Create( CComBSTR( sTemp ), &pSingular );
	pElements->Release();

	// Get attributes and add all metadata
	pSingular->get_Attributes( &pAttributes );

	hr = m_pDocProps->m_pSummProps->get_Author( &bsValue );
	if ( SUCCEEDED(hr) )
		pAttributes->Add( CComBSTR( L"author" ), bsValue );

	hr = m_pDocProps->m_pSummProps->get_Title( &bsValue );
	if ( SUCCEEDED(hr) )
		pAttributes->Add( CComBSTR( L"title" ), bsValue );

	hr = m_pDocProps->m_pSummProps->get_Subject( &bsValue );
	if ( SUCCEEDED(hr) )
		pAttributes->Add( CComBSTR( L"subject" ), bsValue );

	hr = m_pDocProps->m_pSummProps->get_Keywords( &bsValue );
	if ( SUCCEEDED(hr) )
		pAttributes->Add( CComBSTR( L"keywords" ), bsValue );

	wchar_t buffer[40];
	if ( pszSchema == CDocReader::uriPresentation )
	{
		hr = m_pDocProps->m_pSummProps->get_SlideCount( &nCount );
		if ( SUCCEEDED(hr) )
		{
			_ltow( nCount, buffer, 10 );
			pAttributes->Add( CComBSTR( L"slides" ), CComBSTR( buffer ) );
		}
	}
	else
	{
		hr = m_pDocProps->m_pSummProps->get_PageCount( &nCount );
		// Windows usually doesn't refresh page count automatically (MS bug)
		// So, we will add it when it is greater than 1.
		// Maybe approximate from:
		// m_pDocProps->m_pSummProps->get_LineCount( &nCount ) / 16;
		// (when font size = 10)?

		if ( SUCCEEDED(hr) && nCount > 1 )
		{
			_ltow( nCount, buffer, 10 );
			pAttributes->Add( CComBSTR( L"pages" ), CComBSTR( buffer ) );
		}
	}

	if ( pszSchema != CDocReader::uriBook )
	{
		// Shareaza will shorten it to 100 characters, so there's nothing we can do
		hr = m_pDocProps->m_pSummProps->get_Comments( &bsValue );
		if ( SUCCEEDED(hr) )
			pAttributes->Add( CComBSTR( L"comments" ), bsValue );

		hr = m_pDocProps->m_pSummProps->get_Version( &bsValue );
		if ( SUCCEEDED(hr) )
		{
			if ( _wcsnicmp( bsValue, L"0.0", 3 ) != 0 ) 
				pAttributes->Add( CComBSTR( L"version" ), bsValue );
		}

		hr = m_pDocProps->m_pSummProps->get_RevisionNumber( &bsValue );
		if ( SUCCEEDED(hr) )
			pAttributes->Add( CComBSTR( L"revision" ), bsValue );
	}

	hr = m_pDocProps->m_pSummProps->get_Company( &bsValue );
	if ( SUCCEEDED(hr) )
	{
		if ( pszSchema == CDocReader::uriBook )
			pAttributes->Add( CComBSTR( L"publisher" ), bsValue );
		else
			pAttributes->Add( CComBSTR( L"copyright" ), bsValue );
	}

	// Now add some internal data

	pAttributes->Add( CComBSTR( L"format" ), CComBSTR( pszFormat ) );

	if ( pszSchema == CDocReader::uriBook )
	{
		pAttributes->Add( CComBSTR( L"back" ), CComBSTR( L"Digital" ) );
	}

	// Cleanup
	pAttributes->Release();
	pSingular->Release();
	pPlural->Release();
   
	m_pDocProps->Close( VARIANT_FALSE );
	return S_OK;
}

STDMETHODIMP CDocReader::ProcessNewMSDocument(BSTR bsFile, ISXMLElement* pXML, LPCWSTR pszSchema, LPCWSTR pszFormat)
{
	WCHAR pszName[MAX_PATH];
	ULONG ulIdx;
	if ( ! FFindQualifiedFileName( bsFile, pszName, &ulIdx ) )
        return STG_E_INVALIDNAME;

	// Open it with the UNZIP library
	unzFile pFile = NULL;
	zlib_filefunc_def FileFunc;
	fill_win32_filefunc( &FileFunc );
	pFile = unzOpen2( CW2A(pszName), &FileFunc );
	
	if ( ! pFile )
	{
		// Probably unicode path...
		// Try to find a short name, since UNZIP can not handle them
		if ( GetShortPathNameW( bsFile, pszName, MAX_PATH ) )
		{
			pFile = unzOpen2( CW2A(pszName), &FileFunc );
			if ( ! pFile ) return STG_E_INVALIDNAME;
		}
		else return E_FAIL; // system doesn't support 8.3 filenames
	}

	// Read docProps\app.xml from the archive (Properties -> Pages, Company and AppVersion)
	// Read docProps\core.xml from the archive 
	//		(cp:coreProperties -> dc:title, subject, creator, keywords, description, revision, category)
	CComBSTR sXML = GetMetadataXML( pFile, "docProps/core.xml" );

	ISXMLElement* pInputXML = NULL;
	bool bSecondFile = false;

	if ( FAILED( pXML->FromString( sXML, &pInputXML ) ) || pInputXML == NULL )
	{
		sXML = GetMetadataXML( pFile, "docProps/app.xml" );
		if ( pInputXML != NULL )
		{
			pInputXML->Delete();
			pInputXML->Release();
		}

		if ( ! sXML.Length() || FAILED( pXML->FromString( sXML, &pInputXML ) ) || pInputXML == NULL ) 
		{
			unzClose( pFile );
			return E_FAIL;
		}

		bSecondFile = true;
	}
	
	ISXMLElements* pElements;
	// Get the Elements collection from the XML document
	if ( FAILED( pInputXML->get_Elements( &pElements ) ) || pElements == NULL )
	{
		if ( bSecondFile )
		{
			unzClose( pFile );
			pInputXML->Delete();
			pInputXML->Release();
			return E_FAIL;
		}

		sXML = GetMetadataXML( pFile, "docProps/app.xml" );

		if ( ! sXML.Length() || FAILED( pInputXML->get_Elements( &pElements ) ) || pElements == NULL ) 
		{
			unzClose( pFile );
			pInputXML->Delete();
			pInputXML->Release();
			return E_FAIL;
		}

		bSecondFile = true;
	}

	// unzClose( pFile );

	BSTR bsValue = NULL;
	LPCWSTR pszSingular = NULL;
	CString sTemp;

	pszSingular = wcsrchr( pszSchema, _T('/') ) + 1;

	sTemp.Append( CW2T(pszSingular), static_cast< int>( wcslen( pszSingular ) - 4 ) );
	sTemp.Append( _T("s") );

	// Get a pointer to elements node and create a root element
	ISXMLElement* pPlural;
	ISXMLElements* pDestElements;

	pXML->get_Elements( &pDestElements );
	pDestElements->Create( CComBSTR( sTemp ), &pPlural );
	pDestElements->Release();

	// Add root element attributes
	ISXMLAttributes* pAttributes;
	pPlural->get_Attributes( &pAttributes );
	pAttributes->Add( CComBSTR( L"xmlns:xsi" ), CComBSTR( L"http://www.w3.org/2001/XMLSchema-instance" ) );
	pAttributes->Add( CComBSTR( L"xsi:noNamespaceSchemaLocation" ), CComBSTR( pszSchema ) );
	pAttributes->Release();

	// Create inner element describing metadata
	ISXMLElement* pSingular;
	pPlural->get_Elements( &pDestElements );

	sTemp = sTemp.Left( sTemp.GetLength() - 1 );
	pDestElements->Create( CComBSTR( sTemp ), &pSingular );
	pDestElements->Release();

	// Get attributes and add all metadata
	pSingular->get_Attributes( &pAttributes );

	ISXMLElement* pData = NULL;

	HRESULT hr = pElements->get_ByName( CComBSTR( L"dc:creator" ), &pData );
	if ( SUCCEEDED(hr) && pData )
	{
		if ( SUCCEEDED(pData->get_Value( &bsValue )) )
			pAttributes->Add( CComBSTR( L"author" ), bsValue );
		pData->Release();
	}

	hr = pElements->get_ByName( CComBSTR( L"dc:title" ), &pData );
	if ( SUCCEEDED(hr) && pData )
	{
		if ( SUCCEEDED(pData->get_Value( &bsValue )) )
			pAttributes->Add( CComBSTR( L"title" ), bsValue );
		pData->Release();
	}

	hr = pElements->get_ByName( CComBSTR( L"dc:subject" ), &pData );
	if ( SUCCEEDED(hr) && pData )
	{
		if ( SUCCEEDED(pData->get_Value( &bsValue )) )
			pAttributes->Add( CComBSTR( L"subject" ), bsValue );
		pData->Release();
	}

	// Loop through all keywords
	CComBSTR bsKeywords;
	do
	{
		hr = pElements->get_ByName( CComBSTR( L"cp:keywords" ), &pData );
		if ( SUCCEEDED(hr) && pData )
		{
			if ( pData && SUCCEEDED(pData->get_Value( &bsValue )) )
			{
				if ( ! bsKeywords.Length() ) 
				{
					bsKeywords.AssignBSTR( bsValue );
				}
				else 
				{
					bsKeywords.Append( L";" );
					bsKeywords.AppendBSTR( bsValue );
				}
				// delete keyword to get the next
				pData->Delete();
			}
			pData->Release();
		}
	}
	while ( pData );
	if ( bsKeywords ) pAttributes->Add( CComBSTR( L"keywords" ), bsKeywords );

	hr = pElements->get_ByName( CComBSTR( L"dc:description" ), &pData );
	if ( SUCCEEDED(hr) && pData )
	{
		if ( SUCCEEDED(pData->get_Value( &bsValue )) )
			pAttributes->Add( CComBSTR( L"comments" ), bsValue );
		pData->Release();
	}

	hr = pElements->get_ByName( CComBSTR( L"cp:revision" ), &pData );
	if ( SUCCEEDED(hr) && pData )
	{
		if ( SUCCEEDED(pData->get_Value( &bsValue )) )
			pAttributes->Add( CComBSTR( L"revision" ), bsValue );
		pData->Release();
	}

	if ( !bSecondFile )
	{
		sXML = GetMetadataXML( pFile, "docProps/app.xml" );
		if ( sXML.Length() > 0 ) 
		{
			pInputXML->Delete();
			pInputXML->Release();
			pInputXML = NULL;

			if ( FAILED( pXML->FromString( sXML, &pInputXML ) ) || pInputXML == NULL )
			{
				// Do nothing, clean up later
			}
			else
			{
				pElements->Release();
				pElements = NULL;

				if ( FAILED( pInputXML->get_Elements( &pElements ) ) || pElements == NULL )
				{
					// Do nothing, cleanup later
				}
				else
				{
					bSecondFile = true;
				}
			}
		}
	}

	unzClose( pFile );
	
	if ( bSecondFile )
	{
		hr = pElements->get_ByName( CComBSTR( L"Pages" ), &pData );
		if ( SUCCEEDED(hr) && pData )
		{
			if ( SUCCEEDED(pData->get_Value( &bsValue )) )
				pAttributes->Add( CComBSTR( L"pages" ), bsValue );
			pData->Release();
		}
		hr = pElements->get_ByName( CComBSTR( L"Slides" ), &pData );
		if ( SUCCEEDED(hr) && pData )
		{
			if ( SUCCEEDED(pData->get_Value( &bsValue )) )
				pAttributes->Add( CComBSTR( L"slides" ), bsValue );
			pData->Release();
		}
		hr = pElements->get_ByName( CComBSTR( L"Company" ), &pData );
		if ( SUCCEEDED(hr) && pData )
		{
			if ( SUCCEEDED(pData->get_Value( &bsValue )) )
				pAttributes->Add( CComBSTR( L"copyright" ), bsValue );
			pData->Release();
		}
	}

	// Now add some internal data

	pAttributes->Add( CComBSTR( L"format" ), CComBSTR( pszFormat ) );

	// Cleanup destination
	pAttributes->Release();
	pSingular->Release();
	pPlural->Release();

	// Cleanup source
	if ( pElements != NULL )
		pElements->Release();

	if ( pInputXML != NULL )
	{
		pInputXML->Delete();
		pInputXML->Release();
	}

	return S_OK;
}

STDMETHODIMP CDocReader::ProcessOODocument(BSTR bsFile, ISXMLElement* pXML, LPCWSTR pszSchema, LPCWSTR pszFormat)
{
	ODS(_T("CDocReader::ProcessOODocument\n"));
	CHECK_NULL_RETURN(bsFile, E_INVALIDARG);

	WCHAR pszName[MAX_PATH];
	ULONG ulIdx;
	if ( ! FFindQualifiedFileName( bsFile, pszName, &ulIdx ) )
        return STG_E_INVALIDNAME;

	// OpenOffice and OpenDocument files are zips
	// Open it with the UNZIP library
	unzFile pFile = NULL;
	zlib_filefunc_def FileFunc;
	fill_win32_filefunc( &FileFunc );
	pFile = unzOpen2( CW2A(pszName), &FileFunc );
	
	if ( ! pFile )
	{
		// Probably unicode path...
		// Try to find a short name, since UNZIP can not handle them
		if ( GetShortPathNameW( bsFile, pszName, MAX_PATH ) )
		{
			pFile = unzOpen2( CW2A(pszName), &FileFunc );
			if ( ! pFile ) return STG_E_INVALIDNAME;
		}
		else return E_FAIL; // system doesn't support 8.3 filenames
	}

	// Read meta.xml from the archive
	CComBSTR sXML = GetMetadataXML( pFile, "meta.xml" );

	// Close the file
	unzClose( pFile );

	if ( ! sXML.Length() ) return E_FAIL;

	ISXMLElement* pInputXML = NULL;

	if ( FAILED( pXML->FromString( sXML, &pInputXML ) ) || pInputXML == NULL )
	{
		return E_FAIL;
	}
	
	ISXMLElements* pElements;
	// Get the Elements collection from the XML document
	if ( FAILED( pInputXML->get_Elements( &pElements ) ) || pElements == NULL )
	{
		pInputXML->Delete();
		pInputXML->Release();
		return E_FAIL;
	}
	
	ISXMLElement* pMeta;
	
	// Get root element of metadata
	if ( FAILED( pElements->get_ByName( L"office:meta", &pMeta ) ) || pMeta == NULL )
	{
		pElements->Release();
		pInputXML->Delete();
		pInputXML->Release();
		return E_FAIL;
	}

	ISXMLElements* pMetaElements;
	// Get metadata elements collection from which we will get all data
	if ( FAILED( pMeta->get_Elements( &pMetaElements ) ) || pMetaElements == NULL )
	{
		pMeta->Release();
		pElements->Release();
		pInputXML->Delete();
		pInputXML->Release();
		return S_FALSE;
	}

	BSTR bsValue = NULL;
	LPCWSTR pszSingular = NULL;
	CString sTemp;

	pszSingular = wcsrchr( pszSchema, _T('/') ) + 1;

	sTemp.Append( CW2T(pszSingular), static_cast< int>( wcslen( pszSingular ) - 4 ) );
	sTemp.Append( _T("s") );

	// Get a pointer to elements node and create a root element
	ISXMLElement* pPlural;
	ISXMLElements* pDestElements;

	pXML->get_Elements( &pDestElements );
	pDestElements->Create( CComBSTR( sTemp ), &pPlural );
	pDestElements->Release();

	// Add root element attributes
	ISXMLAttributes* pAttributes;
	pPlural->get_Attributes( &pAttributes );
	pAttributes->Add( CComBSTR( L"xmlns:xsi" ), CComBSTR( L"http://www.w3.org/2001/XMLSchema-instance" ) );
	pAttributes->Add( CComBSTR( L"xsi:noNamespaceSchemaLocation" ), CComBSTR( pszSchema ) );
	pAttributes->Release();

	// Create inner element describing metadata
	ISXMLElement* pSingular;
	pPlural->get_Elements( &pDestElements );

	sTemp = sTemp.Left( sTemp.GetLength() - 1 );
	pDestElements->Create( CComBSTR( sTemp ), &pSingular );
	pDestElements->Release();

	// Get attributes and add all metadata
	pSingular->get_Attributes( &pAttributes );

	ISXMLElement* pData = NULL;

	HRESULT hr = pMetaElements->get_ByName( CComBSTR( L"meta:initial-creator" ), &pData );
	if ( SUCCEEDED(hr) && pData )
	{
		if ( SUCCEEDED(pData->get_Value( &bsValue )) )
			pAttributes->Add( CComBSTR( L"author" ), bsValue );
		pData->Release();
	}

	hr = pMetaElements->get_ByName( CComBSTR( L"dc:title" ), &pData );
	if ( SUCCEEDED(hr) && pData )
	{
		if ( SUCCEEDED(pData->get_Value( &bsValue )) )
			pAttributes->Add( CComBSTR( L"title" ), bsValue );
		pData->Release();
	}

	hr = pMetaElements->get_ByName( CComBSTR( L"dc:subject" ), &pData );
	if ( SUCCEEDED(hr) && pData )
	{
		if ( SUCCEEDED(pData->get_Value( &bsValue )) )
			pAttributes->Add( CComBSTR( L"subject" ), bsValue );
		pData->Release();
	}

	// Loop through all keywords
	CComBSTR bsKeywords;
	do
	{
		hr = pMetaElements->get_ByName( CComBSTR( L"meta:keyword" ), &pData );
		if ( SUCCEEDED(hr) && pData )
		{
			if ( pData && SUCCEEDED(pData->get_Value( &bsValue )) )
			{
				if ( ! bsKeywords.Length() ) 
				{
					bsKeywords.AssignBSTR( bsValue );
				}
				else 
				{
					bsKeywords.Append( L";" );
					bsKeywords.AppendBSTR( bsValue );
				}
				// delete keyword to get the next
				pData->Delete();
			}
			pData->Release();
		}
	}
	while ( pData );
	if ( bsKeywords ) pAttributes->Add( CComBSTR( L"keywords" ), bsKeywords );

	hr = pMetaElements->get_ByName( CComBSTR( L"dc:description" ), &pData );
	if ( SUCCEEDED(hr) && pData )
	{
		// should be abstract by definition but it corresponds to comments
		// in MS documents
		if ( SUCCEEDED(pData->get_Value( &bsValue )) )
			pAttributes->Add( CComBSTR( L"comments" ), bsValue );
		pData->Release();
	}

	hr = pMetaElements->get_ByName( CComBSTR( L"dc:language" ), &pData );
	if ( SUCCEEDED(hr) && pData )
	{
		if ( pData && SUCCEEDED(pData->get_Value( &bsValue )) )
			pAttributes->Add( CComBSTR( L"language" ), bsValue );
		pData->Release();
	}

	hr = pMetaElements->get_ByName( CComBSTR( L"meta:document-statistic" ), &pData );
	if ( SUCCEEDED(hr) && pData )
	{
		ISXMLAttributes* pStatAttributes;
		hr = pData->get_Attributes( &pStatAttributes );
		if ( SUCCEEDED(hr) && pStatAttributes )
		{
			ISXMLAttribute* pAttribute;
			if ( SUCCEEDED(pStatAttributes->get_ByName( CComBSTR( L"meta:page-count" ), &pAttribute )) &&
				 pAttribute )
			{
				if ( SUCCEEDED(pAttribute->get_Value( &bsValue )) )
				{
					if ( pszSchema == CDocReader::uriPresentation )
						pAttributes->Add( CComBSTR( L"slides" ), bsValue );
					else
						pAttributes->Add( CComBSTR( L"pages" ), bsValue );
				}
				pAttribute->Release();
			}
			pStatAttributes->Release();
		}
		pData->Release();
	}

	// Now add some internal data

	pAttributes->Add( CComBSTR( L"format" ), CComBSTR( pszFormat ) );

	if ( pszSchema == CDocReader::uriBook )
	{
		pAttributes->Add( CComBSTR( L"back" ), CComBSTR( L"Digital" ) );
	}

	// Cleanup destination
	pAttributes->Release();
	pSingular->Release();
	pPlural->Release();

	// Cleanup source
	pMetaElements->Release();
	pMeta->Release();
	pElements->Release();

	pInputXML->Delete();
	pInputXML->Release();

	return S_OK;
}

CComBSTR CDocReader::GetMetadataXML(unzFile pFile, const char* pszFile)
{
	CComBSTR sUnicode;

	int nError = unzGoToFirstFile( pFile );
	if ( nError != UNZ_OK )
		return sUnicode;
	nError = unzLocateFile( pFile, pszFile, 0 );
	if ( nError == UNZ_OK )
	{
		// Open the metadata.xml
		nError = unzOpenCurrentFilePassword( pFile, 0 );
		if ( nError == UNZ_OK )
		{
			unz_file_info pInfo = {};
			nError = unzGetCurrentFileInfo( pFile, &pInfo, NULL, 0, NULL, 0, NULL, 0 );
			if ( nError == UNZ_OK )
			{
				// Prepare a buffer to read into
				UINT nBufferSize = pInfo.uncompressed_size;
				if ( nBufferSize > 8192 ) nBufferSize = 8192;
				CHAR* pBuffer = new (std::nothrow) CHAR[ nBufferSize ];
				if ( pBuffer )
				{
					// Extract the file into buffer
					CStringA sXML;
					do
					{
						nError = unzReadCurrentFile( pFile, pBuffer, nBufferSize );
						if ( nError < 0 ) 
							break;
						else if ( nError > 0 )
							sXML.Append( pBuffer, nBufferSize ); 
					}
					while ( nError > 0 );
					if ( nError == UNZ_OK )
					{
						// Just make a buffer enough to fit string
						WCHAR* pszUnicode = new (std::nothrow) WCHAR[ sXML.GetLength() + 1 ];
						if ( pszUnicode )
						{
							ConvertToUnicodeEx( sXML, (DWORD)sXML.GetLength(),
								pszUnicode, (DWORD)sXML.GetLength(), CP_UTF8 );
							sUnicode = pszUnicode;				
							delete [] pszUnicode;
						}
					}
					delete [] pBuffer;
				}
			}
		}
	}
	return sUnicode; 
}

// IImageServicePlugin Methods

STDMETHODIMP CDocReader::LoadFromFile(BSTR sFile, IMAGESERVICEDATA* pParams, SAFEARRAY** ppImage)
{
	ODS(_T("CDocReader::LoadFromFile\n"));

	EnterCritical();
	DllAddRef();

	// Create a new instance of the control
	delete m_pDocProps;
	m_pDocProps = new CDocumentProperties( TRUE );
	
	HRESULT hr = E_FAIL;

	LPCWSTR pszExt = _wcslwr( wcsrchr( sFile, '.') );
	LPCWSTR pszFormat = GetDocumentFormat( pszExt );

	if ( ! pszFormat )
	{
		DllRelease();
		LeaveCritical();
		return E_UNEXPECTED;
	}
	if ( pszFormat[ 0 ] == 'M' ) // Microsoft
		hr = GetMSThumbnail( sFile, pParams, ppImage );
	else if ( pszFormat[ 0 ] == 'O' ) // OpenOffice or OpenDocument
		hr = GetOOThumbnail( sFile, pParams, ppImage );

	DllRelease();
	LeaveCritical();
	if ( pParams->nFlags & IMAGESERVICE_PARTIAL_IN )
		pParams->nFlags |= IMAGESERVICE_PARTIAL_OUT;

	return hr;
}

STDMETHODIMP CDocReader::GetMSThumbnail(BSTR bsFile, IMAGESERVICEDATA* pParams, SAFEARRAY** ppImage)
{
	ODS(_T("CDocReader::GetMSThumbnail\n"));
	VARIANT va;
	LONG nWidth = 0;
	LONG nHeight = 0;
	LPBYTE pData;

	HRESULT hr = m_pDocProps->Open( bsFile, VARIANT_TRUE, dsoOptionOpenReadOnlyIfNoWriteAccess );
	SysFreeString( bsFile );

	// Opening the document failed; give up
	if ( FAILED(hr) )
	{
		m_pDocProps->Close( VARIANT_FALSE );
		return E_FAIL;
	}

	// Retrieve thumbnail data in safearray if any
	VariantInit( &va );
	hr = m_pDocProps->m_pSummProps->get_Thumbnail( &va );
	hr = m_pDocProps->Close( VARIANT_FALSE );

	// Check if we have it
	if ( va.vt == VT_EMPTY ) return E_FAIL;
	if ( ! ( va.vt & VT_ARRAY ) ) return E_UNEXPECTED;

	BYTE* pclp = NULL;
	LONG nSize = 0;
	PICTDESC pds;
	HBITMAP hBitmap = NULL;
	RGBQUAD pPalette[256];
	BITMAPINFO* pBI = NULL;

	hr = SafeArrayGetUBound( va.parray, 1, &nSize );
	hr = SafeArrayAccessData( va.parray, (void**)&pclp );
	
	nSize += 1;
	SEH_TRY
	// Most MS documents have thumbnails in WMF format
	if ( (DWORD)*pclp == CF_METAFILEPICT )
	{
		DWORD cbDataSize, cbHeaderSize;
		HMETAFILE hwmf;
		// Weird header structure, not found described anywhere
		cbHeaderSize = sizeof(DWORD) + 4 * sizeof(WORD);
		cbDataSize = nSize - cbHeaderSize;

		CLIPMETAHEADER cHeader;
		memcpy( &cHeader, pclp, sizeof(CLIPMETAHEADER) );

		// convert to emf
		hwmf = SetMetaFileBitsEx( cbDataSize, (BYTE*)(pclp + cbHeaderSize) );
		if ( NULL != hwmf )
		{
			pds.cbSizeofstruct = sizeof(PICTDESC);
			pds.picType = PICTYPE_METAFILE;
			pds.wmf.hmeta = hwmf;
			pds.wmf.xExt = cHeader.Width; 
			pds.wmf.yExt = cHeader.Height;

			// We will make such images
			const int nResolution = 300;
			const WORD wColorDepth = 24;

			// We will allocate pBI, do not forget to delete it
			hBitmap = GetBitmapFromMetaFile( pds, nResolution, wColorDepth, &pBI );
			if ( hBitmap == NULL || pBI == NULL ) return S_FALSE;
			nWidth = pBI->bmiHeader.biWidth;
			nHeight = pBI->bmiHeader.biHeight;

 			pParams->nWidth = nWidth;
			pParams->nHeight = nHeight;
			// Component number is required for Image Viewer
			pParams->nComponents = 3;

			if ( pParams->nFlags & IMAGESERVICE_SCANONLY ) return S_OK;
			// Save palette
			SetStdPalette( (void*)pPalette, pBI->bmiHeader.biBitCount );
		}
	}
	// They say MS supports such formats. Couldn't find anywhere.
	// NOT TESTED !!!
	/*else if ( (DWORD)*pclp == CF_DIB || (DWORD)*pclp == CF_BITMAP )
	{
		BITMAP bm;
		BITMAPINFOHEADER* pBmH;

		pBmH = (BITMAPINFOHEADER*)( pclp + 4 );

		bm.bmType = 0;
		bm.bmWidth = nWidth = static_cast<int>(pBmH->biWidth);
		bm.bmHeight = nHeight = static_cast<int>(pBmH->biHeight);

 		pParams->nWidth = nWidth;
		pParams->nHeight = nHeight;
		pParams->nComponents = 3;

		if ( pParams->nFlags & IMAGESERVICE_SCANONLY ) return S_OK;

		bm.bmPlanes = pBmH->biPlanes;
		bm.bmBitsPixel = pBmH->biBitCount;
		bm.bmWidthBytes = ( pBmH->biSizeImage / bm.bmHeight );
		SetStdPalette( (void*)pPalette, bm.bmBitsPixel );
		bm.bmBits = ( pclp + 4 + pBmH->biSize );

		hBitmap = CreateBitmapIndirect( &bm );

		if ( hBitmap )
		{
			pds.cbSizeofstruct = sizeof(PICTDESC);
			pds.picType = PICTYPE_BITMAP;
			pds.bmp.hpal = NULL;
			pds.bmp.hbitmap = (HBITMAP)hBitmap;
		}
	}*/
	// Only some MS Visio documents keep them
	else if ( (DWORD)*pclp == CF_ENHMETAFILE )
	{
		HENHMETAFILE hemf;

		// Remove clipboard data type; save to EMF handle
		hemf = SetEnhMetaFileBits( nSize - sizeof(DWORD), (BYTE*)(pclp + sizeof(DWORD)) );
		if ( NULL != hemf )
		{
			pds.cbSizeofstruct = sizeof(PICTDESC);
			pds.picType = PICTYPE_ENHMETAFILE;
			pds.emf.hemf = hemf;

			// We will make such images
			const int nResolution = 300;
			const WORD wColorDepth = 24;

			// We will allocate pBI, do not forget to delete it
			hBitmap = GetBitmapFromEnhMetaFile( pds, nResolution, wColorDepth, &pBI );
			if ( hBitmap == NULL || pBI == NULL ) return S_FALSE;
			nWidth = pBI->bmiHeader.biWidth;
			nHeight = pBI->bmiHeader.biHeight;

 			pParams->nWidth = nWidth;
			pParams->nHeight = nHeight;
			pParams->nComponents = 3;

			if ( pParams->nFlags & IMAGESERVICE_SCANONLY ) return S_OK;
			// Save pallete
			SetStdPalette( (void*)pPalette, pBI->bmiHeader.biBitCount );
		}
	}
	// Release safearray
	SafeArrayUnaccessData( va.parray );
	SEH_EXCEPT_NULL

	// Check if we have a bitmap
	if ( hBitmap == NULL || pBI == NULL ) return S_FALSE;

	// Get bitmap byte buffer from the handle

	BYTE* pBytes = new BYTE[ pBI->bmiHeader.biSizeImage ];

	BITMAP stBitmap;
	GetObject( hBitmap, sizeof(stBitmap), &stBitmap );
	HDC hScreen = GetDC( NULL );
	// Create bitmap
	HBITMAP hOffscreen = CreateCompatibleBitmap( hScreen, stBitmap.bmWidth, stBitmap.bmHeight );
	HDC hMemSrc = CreateCompatibleDC( NULL );
	HBITMAP OldBmp = (HBITMAP)SelectObject( hMemSrc, hOffscreen );
	// Copy bitmap to the memory DC
	BitBlt( hMemSrc, 0, 0, stBitmap.bmWidth, stBitmap.bmHeight, hScreen, 0, 0, SRCCOPY );
	// We don't need screen DC anymore
	ReleaseDC( NULL, hScreen ); hScreen = NULL;
	// We need to select something before calling GetDIBits
	SelectObject( hMemSrc, OldBmp );
	GetDIBits( hMemSrc, hBitmap, 0, nHeight, (VOID*)pBytes, pBI, DIB_RGB_COLORS );

	if ( hMemSrc ) DeleteDC( hMemSrc );
	if ( hOffscreen ) DeleteObject( hOffscreen );

	// Calculate resulting bitmap scanline width (pitch) in bytes
	unsigned short nAlignSize = sizeof(DWORD) * 8 - 1;
	ULONG nInPitch = ( ( pBI->bmiHeader.biBitCount * pBI->bmiHeader.biWidth ) + nAlignSize ) & ~nAlignSize;
	nInPitch /= 8;
	// We will make 3 bytes out off 1 if bitmap has less than 256 colors
	// For 24-bit images it is not needed
	ULONG nOutPitch = ( pBI->bmiHeader.biWidth * 3 + 3 ) & ~3u;
	LPBYTE pRowOut;

	ULONG nArray = nOutPitch * (ULONG)pBI->bmiHeader.biHeight;
	WORD nBitCount = pBI->bmiHeader.biBitCount;
	// All calculations are performed, delete it now
	if ( pBI != 0 ) delete[] (BYTE*)pBI;

	// Store byte array start position; we will need to delete it later
	BYTE* pStart = pBytes;
	
	// Create 1-dimensional safearray 
	*ppImage = SafeArrayCreateVector( VT_UI1, 0, nArray );
	SafeArrayAccessData( *ppImage, (VOID**)&pData );

	// We are reading scanlines from bottom to top
	for ( int nY = nHeight - 1 ; nY >= 0 ; nY-- )
	{
		pRowOut	= &pData[ nOutPitch * nY ];

		if ( nBitCount == 24 )
		{
			for ( int nX = 0 ; nX < nWidth ; nX++ )
			{
				pRowOut[ 0 ] = pBytes[ 2 ];
				pRowOut[ 1 ] = pBytes[ 1 ];
				pRowOut[ 2 ] = pBytes[ 0 ];
				
				pRowOut += 3;
				pBytes += 3;
			}
			// skip junk at the end of scanline
			pBytes = pBytes + ( nInPitch - nWidth * 3 );
		}
		else
		{
			for ( int nX = 0 ; nX < nWidth ; nX++ )
			{
				RGBQUAD& color = pPalette[ *pBytes++ ];

				pRowOut[ 0 ] = color.rgbRed;
				pRowOut[ 1 ] = color.rgbGreen;
				pRowOut[ 2 ] = color.rgbBlue;

				pRowOut += 3;
			}
			// skip junk at the end of scanline
			pBytes = pBytes + ( nInPitch - nWidth );
		}
	}

	// Unlock safearray; we are ready to deliver it back to Shareaza
	SafeArrayUnaccessData( *ppImage );

	delete [] pStart;

	VariantClear( &va );
	return S_OK;
}

STDMETHODIMP CDocReader::GetOOThumbnail(BSTR bsFile, IMAGESERVICEDATA* pParams, SAFEARRAY** ppImage)
{
	ODS(_T("CDocReader::GetOOThumbnail\n"));

	CHECK_NULL_RETURN(bsFile, E_INVALIDARG);

	WCHAR pszName[MAX_PATH];
	ULONG ulIdx;
	if ( ! FFindQualifiedFileName( bsFile, pszName, &ulIdx ) )
        return STG_E_INVALIDNAME;

	// OpenOffice and OpenDocument files are zips
	// Open it with the UNZIP library
	unzFile pFile = NULL;
	zlib_filefunc_def FileFunc;
	fill_win32_filefunc( &FileFunc );
	pFile = unzOpen2( CW2A(pszName), &FileFunc );
	
	if ( ! pFile )
	{
		// Probably unicode path...
		// Try to find a short name, since UNZIP can not handle them
		if ( GetShortPathNameW( bsFile, pszName, MAX_PATH ) )
		{
			pFile = unzOpen2( CW2A(pszName), &FileFunc );
			if ( ! pFile )
				return STG_E_INVALIDNAME;
		}
		else
			return E_FAIL; // system doesn't support 8.3 filenames
	}

	HRESULT hr;

	// Load thumbnail.png file to memory. The thumbnail is 24-bit with alpha channel
	// and located in Thumbnails folder.
	CHAR szFile[256] = "Thumbnails/thumbnail.png";
	int nError = unzLocateFile( pFile, szFile, 0 );
	if ( nError == UNZ_OK )
	{
		// Open the metadata.xml
		nError = unzOpenCurrentFilePassword( pFile, 0 );
		if ( nError == UNZ_OK )
		{
			// Prepare a buffer to read into
			UINT nBufferSize = 8192;
			CHAR* pBuffer = new (std::nothrow) CHAR[ nBufferSize ];
			if ( pBuffer )
			{
				CComPtr< ILockBytes > pLockBuffer;
				CComPtr< IStorage > pStorage;
				CComPtr< IStream > FileStream;
				UINT nBytesWritten = 0;

				// Prepare stream to transfer the buffer
				if ( SUCCEEDED( hr = CreateILockBytesOnHGlobal( NULL, TRUE, &pLockBuffer ) ) &&
					 SUCCEEDED( hr = StgCreateDocfileOnILockBytes( pLockBuffer,
						STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_READWRITE, 0, &pStorage ) ) &&
					 SUCCEEDED( hr = pStorage->CreateStream( L"PICTURE",
						STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_READWRITE, 0, 0, &FileStream ) ) )
				{
					// Extract the file into buffer and transfer to stream (buffered reading)
					do
					{
						nError = unzReadCurrentFile( pFile, pBuffer, nBufferSize );
						if ( nError < 0 ) 
							break;
						else if ( nError > 0 )
						{
							hr = FileStream->Write( (void*)pBuffer, nBufferSize, (ULONG*)&nBytesWritten );
							if ( FAILED(hr) || nBufferSize != nBytesWritten ) break;
							pLockBuffer->Flush();
						}
					}
					while ( nError > 0 );

					if ( nError == UNZ_OK ) 
					{
						// Get statistics for the final size of byte array
						STATSTG  BytesStatistics = {};
						pLockBuffer->Stat( &BytesStatistics, STATFLAG_NONAME );
						ULONG nArray = static_cast<ULONG>(BytesStatistics.cbSize.QuadPart);

						// Create 1-dimensional safearray 
						SAFEARRAY* psa = SafeArrayCreateVector( VT_UI1, 0, nArray );
						ULONG nRead = 0;
						LARGE_INTEGER llMove;
						llMove.QuadPart = 0;

						// Go to the beginning of the stream
						FileStream->Seek( llMove, STREAM_SEEK_SET, NULL );
						
						// Read byte array from the stream
						BYTE* pData = NULL;
						SafeArrayAccessData( psa, (VOID**)&pData );
						hr = FileStream->Read( (VOID*)pData, nArray, &nRead );
						SafeArrayUnaccessData( psa );
						if ( SUCCEEDED( hr ) )
						{
							CComPtr<IImageServicePlugin> pPNGReader;
							hr = pPNGReader.CoCreateInstance( CLSID_PNGReader,
								NULL, CLSCTX_ALL );
							if ( SUCCEEDED( hr ) )
							{
								hr = pPNGReader->LoadFromMemory( bsFile, psa,
									pParams, ppImage );
							}
						}
					}
					else
						hr = E_FAIL;
				}
				delete [] pBuffer;
			}
			else
				hr = E_OUTOFMEMORY;
			unzCloseCurrentFile( pFile );
		}
		else
			hr = E_FAIL;
	}
	else
		hr = E_FAIL;

	unzClose( pFile );

	return hr;
}

STDMETHODIMP CDocReader::LoadFromMemory(BSTR sType, SAFEARRAY* pMemory, IMAGESERVICEDATA* pParams, SAFEARRAY** ppImage)
{
	ODS(_T("CDocReader::LoadFromMemory\n"));
	return E_NOTIMPL;
}

STDMETHODIMP CDocReader::SaveToFile(BSTR sFile, IMAGESERVICEDATA* pParams, SAFEARRAY* pImage)
{
	ODS(_T("CDocReader::SaveToFile\n"));
	return E_NOTIMPL;
}

STDMETHODIMP CDocReader::SaveToMemory(BSTR sType, SAFEARRAY** ppMemory, IMAGESERVICEDATA* pParams, SAFEARRAY* pImage)
{
	ODS(_T("CDocReader::SaveToMemory\n"));
	return E_NOTIMPL;
}

// Conversion code is based on http://www.codeguru.com/Cpp/G-M/gdi/article.php/c3685/

HBITMAP CDocReader::GetBitmapFromMetaFile(PICTDESC pds, int nResolution, WORD wBitsPerSample, BITMAPINFO **ppBI)
{
	long nWidth = pds.wmf.xExt;
	long nHeight = pds.wmf.yExt;
	long nDotsWidth = CalculateDotsForHimetric( nResolution, nWidth);
	long nDotsHeight = CalculateDotsForHimetric( nResolution, nHeight);

	// make smaller
	if ( nDotsHeight > 800 )
	{
		double nFactor = static_cast<double>(nDotsHeight) / 800.0;
		nDotsHeight = 800;
		nDotsWidth = static_cast<long>(nDotsWidth / nFactor);
	}

	int nInfoSize;
	nInfoSize = sizeof( BITMAPINFOHEADER );
	int nColorTableSize = 0;
	if ( wBitsPerSample <= 8 )
		nColorTableSize = (int)sizeof(RGBQUAD) * ( 1 << wBitsPerSample );
	nInfoSize += nColorTableSize;

	BITMAPINFO* bmInfo = (LPBITMAPINFO) new (std::nothrow) BYTE[ nInfoSize ];
	
	if ( bmInfo == NULL ) return NULL;
	ZeroMemory( bmInfo, nInfoSize );
	// Align scanline size to dword
	unsigned short nAlignSize = sizeof(DWORD) * 8 - 1;
	DWORD dwEffectiveWidth = ( ( wBitsPerSample * nDotsWidth ) + nAlignSize ) & ~nAlignSize;

	//prepare the bitmap attributes
	memset( &bmInfo->bmiHeader, 0, sizeof(BITMAPINFOHEADER) );
	bmInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmInfo->bmiHeader.biWidth = nDotsWidth;
	bmInfo->bmiHeader.biHeight = nDotsHeight;
	bmInfo->bmiHeader.biCompression = BI_RGB; // no compression
	bmInfo->bmiHeader.biXPelsPerMeter = static_cast<long>(nResolution / 2.54E-2);
	bmInfo->bmiHeader.biYPelsPerMeter=static_cast<long>(nResolution / 2.54E-2);
	bmInfo->bmiHeader.biPlanes = 1;
	bmInfo->bmiHeader.biBitCount = wBitsPerSample;
	// size in bytes
	bmInfo->bmiHeader.biSizeImage = dwEffectiveWidth * nDotsHeight / 8;

	if ( wBitsPerSample <= 8 )
	{
		SetStdPalette( bmInfo->bmiColors, wBitsPerSample );
		bmInfo->bmiHeader.biClrUsed = 1 << wBitsPerSample;
	}

	if ( ppBI != NULL )
	{
		*ppBI = (LPBITMAPINFO) new BYTE[ nInfoSize ];
		//copy BITMAPINFO
		memcpy( *ppBI, bmInfo, nInfoSize );
	}

	//create a temporary dc in memory
	HDC hDC = GetDC(0);
	ASSERT( hDC != NULL );

	HDC tempDC = CreateCompatibleDC( hDC );
	ASSERT( tempDC != NULL );

	//create a new bitmap and select it in the memory dc
	BYTE* pBase;

	HBITMAP hTempBmp = CreateDIBSection( hDC, bmInfo, DIB_RGB_COLORS,(void**)&pBase, 0, 0 );
	ASSERT( hTempBmp != NULL );
	HGDIOBJ hTempObj = SelectObject( tempDC, hTempBmp );
	ASSERT( hTempObj != NULL );

	SaveDC( tempDC );

	SetMapMode( tempDC, MM_ANISOTROPIC );
	SetWindowOrgEx( tempDC, 0, 0, NULL );
	SetWindowExtEx( tempDC, nWidth, nHeight, NULL );
	SetViewportExtEx( tempDC, nDotsWidth, nDotsHeight, NULL );

	// Erase the background.
    HBRUSH hBrBkGnd = CreateSolidBrush( PALETTERGB(0xff, 0xff, 0xff) );
	RECT rc = {0, 0, nWidth, nHeight};
    FillRect( tempDC, &rc, hBrBkGnd );
    DeleteObject( hBrBkGnd );

	SetBkMode( tempDC, TRANSPARENT );

	PlayMetaFile( tempDC, pds.wmf.hmeta );

	SelectObject( tempDC, hTempObj );

	RestoreDC( tempDC, -1 );
	DeleteDC( tempDC );

	ReleaseDC( 0, hDC );
	return hTempBmp;
}

HBITMAP CDocReader::GetBitmapFromEnhMetaFile(PICTDESC pds, int nResolution, WORD wBitsPerSample, BITMAPINFO **ppBI)
{
	ENHMETAHEADER cHeader;
	GetEnhMetaFileHeader( pds.emf.hemf, sizeof(ENHMETAHEADER), &cHeader );
	long nWidth = cHeader.rclFrame.right - cHeader.rclFrame.left;
	long nHeight = cHeader.rclFrame.bottom - cHeader.rclFrame.top;
	long nDotsWidth = CalculateDotsForHimetric( nResolution, nWidth);
	long nDotsHeight = CalculateDotsForHimetric( nResolution, nHeight);

	// make smaller
	if ( nDotsHeight > 800 )
	{
		double nFactor = static_cast<double>(nDotsHeight) / 800.0;
		nDotsHeight = 800;
		nDotsWidth = static_cast<long>(nDotsWidth / nFactor);
	}

	int nInfoSize;
	nInfoSize = sizeof( BITMAPINFOHEADER );
	int nColorTableSize = 0;
	if ( wBitsPerSample <= 8 )
		nColorTableSize = (int)sizeof(RGBQUAD) * ( 1 << wBitsPerSample );
	nInfoSize += nColorTableSize;

	BITMAPINFO* bmInfo = (LPBITMAPINFO) new (std::nothrow) BYTE[ nInfoSize ];
	
	if ( bmInfo == NULL ) return NULL;
	ZeroMemory( bmInfo, nInfoSize );
	// Align scanline size to dword
	unsigned short nAlignSize = sizeof(DWORD) * 8 - 1;
	DWORD dwEffectiveWidth = ( ( wBitsPerSample * nDotsWidth ) + nAlignSize ) & ~nAlignSize;

	//prepare the bitmap attributes
	memset( &bmInfo->bmiHeader, 0, sizeof(BITMAPINFOHEADER) );
	bmInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmInfo->bmiHeader.biWidth = nDotsWidth;
	bmInfo->bmiHeader.biHeight = nDotsHeight;
	bmInfo->bmiHeader.biCompression = BI_RGB; // no compression
	bmInfo->bmiHeader.biXPelsPerMeter = static_cast<long>(nResolution / 2.54E-2);
	bmInfo->bmiHeader.biYPelsPerMeter=static_cast<long>(nResolution / 2.54E-2);
	bmInfo->bmiHeader.biPlanes = 1;
	bmInfo->bmiHeader.biBitCount = wBitsPerSample;
	// size in bytes
	bmInfo->bmiHeader.biSizeImage = dwEffectiveWidth * nDotsHeight / 8;

	if ( wBitsPerSample <= 8 )
	{
		SetStdPalette( bmInfo->bmiColors, wBitsPerSample );
		bmInfo->bmiHeader.biClrUsed = 1 << wBitsPerSample;
	}

	if ( ppBI != NULL )
	{
		*ppBI = (LPBITMAPINFO) new BYTE[ nInfoSize ];
		//copy BITMAPINFO
		memcpy( *ppBI, bmInfo, nInfoSize );
	}

	//create a temporary dc in memory
	HDC hDC = GetDC(0);
	ASSERT( hDC != NULL );

	HDC tempDC = CreateCompatibleDC( hDC );
	ASSERT( tempDC != NULL );

	//create a new bitmap and select it in the memory dc
	BYTE* pBase;

	HBITMAP hTempBmp = CreateDIBSection( hDC, bmInfo, DIB_RGB_COLORS,(void**)&pBase, 0, 0 );
	ASSERT( hTempBmp != NULL );
	HGDIOBJ hTempObj = SelectObject( tempDC, hTempBmp );
	ASSERT( hTempObj != NULL );

	SaveDC( tempDC );

	SetMapMode( tempDC, MM_ANISOTROPIC );
	SetWindowOrgEx( tempDC, 0, 0, NULL );
	SetWindowExtEx( tempDC, nWidth, nHeight, NULL );
	SetViewportExtEx( tempDC, nDotsWidth, nDotsHeight, NULL );

	// Erase the background.
    HBRUSH hBrBkGnd = CreateSolidBrush( PALETTERGB(0xff, 0xff, 0xff) );
	RECT rc = {0, 0, nWidth, nHeight};
    FillRect( tempDC, &rc, hBrBkGnd );
    DeleteObject( hBrBkGnd );

	SetBkMode( tempDC, TRANSPARENT );

	PlayEnhMetaFile( tempDC, pds.emf.hemf, &rc );

	SelectObject( tempDC, hTempObj );

	RestoreDC( tempDC, -1 );
	DeleteDC( tempDC );

	ReleaseDC( 0, hDC );
	return hTempBmp;
}

LPWSTR CDocReader::GetDocumentFormat(LPCWSTR pszExt)
{
	if ( _wcsnicmp( msWordExt, pszExt, 4 ) == 0 ||
		 wcsstr( msProjectExt, pszExt ) != NULL ||
		 wcsstr( msVisioExt, pszExt ) != NULL ||
		 wcsstr( ooWriterExt, pszExt ) != NULL )
	{
		switch ( pszExt[1] )
		{
			case 'd' : return L"Microsoft Word";
			case 'm' : return L"Microsoft Project";
			case 'v' : return L"Microsoft Visio";
			case 'o' :
				if ( wcscmp( pszExt, L".odt" ) == 0 )
					return L"OpenDocument Text";
				else
					return L"OpenDocument Text Template";
			case 's' :
				if ( wcscmp( pszExt, L".sxw" ) == 0 )
					return L"OpenOffice.org 1.0 Text Document";
				else
					return L"OpenOffice.org 1.0 Text Document Template";
		}
	}
	else if ( _wcsnicmp( msPPointExt, pszExt, 4 ) == 0 ||
			  wcsstr( ooImpressExt, pszExt ) != NULL )
	{
		switch ( pszExt[1] )
		{
			case 'p' : return L"Microsoft PowerPoint";
			case 'o' :
				if ( wcscmp( pszExt, L".odp" ) == 0 )
					return L"OpenDocument Presentation";
				else
					return L"OpenDocument Presentation Template";
			case 's' :
				if ( wcscmp( pszExt, L".sxi" ) == 0 )
					return L"OpenOffice.org 1.0 Presentation";
				else
					return L"OpenOffice.org 1.0 Presentation Template";
		}
	}
	else if ( _wcsnicmp( msExcelExt, pszExt, 4 ) == 0 ||
			  wcsstr( ooCalcExt, pszExt ) != NULL )
	{
		switch ( pszExt[1] )
		{
			case 'x' : return L"Microsoft Excel";
			case 'o' :
				if ( wcscmp( pszExt, L".ods" ) == 0 )
					return L"OpenDocument Spreadsheet";
				else
					return L"OpenDocument Spreadsheet Template";
			case 's' :
				if ( wcscmp( pszExt, L".sxc" ) == 0 )
					return L"OpenOffice.org 1.0 Spreadsheet";
				else
					return L"OpenOffice.org 1.0 Spreadsheet Template";
		}
	}
	
	return LPWSTR();
}

LPWSTR CDocReader::GetSchema(BSTR sFile, LPCWSTR pszExt)
{
	if ( LPCWSTR pszName = wcsrchr( sFile, '\\' ) )
	{
		pszName++;
		
		if ( _wcsnicmp( pszName, L"ebook - ", 8 ) == 0 ||
			 _wcsnicmp( pszName, L"(ebook", 6 ) == 0 )
		{
			return (LPWSTR)CDocReader::uriBook;
		}
		else if ( _wcsnicmp( msWordExt, pszExt, 4 ) == 0 ||
				  wcsstr( msProjectExt, pszExt ) != NULL ||
				  wcsstr( msVisioExt, pszExt ) != NULL ||
				  wcsstr( ooWriterExt, pszExt ) != NULL )
		{
			return (LPWSTR)CDocReader::uriDocument;
		}
		else if ( _wcsnicmp( msPPointExt, pszExt, 4 ) == 0 ||
				  wcsstr( ooImpressExt, pszExt ) != NULL )
		{
			return (LPWSTR)CDocReader::uriPresentation;
		}
		else if ( _wcsnicmp( msExcelExt, pszExt, 4 ) == 0 ||
				  wcsstr( ooCalcExt, pszExt ) != NULL )
		{
			return (LPWSTR)CDocReader::uriSpreadsheet;
		}
	}

	return LPWSTR();
}

CDocReader::CDocumentProperties::CDocumentProperties(BOOL bOnlyThumb)
	: m_bstrFileName	( NULL )
	, m_cFilePartIdx	( 0 )
    , m_pStorage		( NULL )
    , m_pPropSetStg		( NULL )
    , m_dwFlags			( dsoOptionDefault )
	, m_fReadOnly		( FALSE )
    , m_wCodePage		( 0 )
    , m_pSummProps		( NULL )
	, m_bOnlyThumb		( bOnlyThumb )
{
	ODS(_T("CDocReader::CDocumentProperties::CDocumentProperties()\n"));
}

CDocReader::CDocumentProperties::~CDocumentProperties(void)
{
    ODS(_T("CDocReader::CDocumentProperties::~CDocumentProperties()\n"));
    ASSERT(m_pStorage == NULL); // We should be closed before delete!
    if ( m_pStorage ) Close( VARIANT_FALSE );
}

////////////////////////////////////////////////////////////////////////
// Implementation
//
////////////////////////////////////////////////////////////////////////
// Open  -- Takes a full or relative file name and loads the property
//   set for the document. Handles both OLE and NTFS5 property sets.
//
HRESULT CDocReader::
	CDocumentProperties::Open(BSTR sFileName, VARIANT_BOOL ReadOnly, dsoFileOpenOptions Options)
{
    HRESULT hr;
    DWORD dwOpenMode;
    WCHAR wszFullName[MAX_PATH];
    ULONG ulIdx;
	
 // Open method called. Ensure we don't have file already open...
	ODS(_T("CDocReader::CDocumentProperties::Open\n"));
    ASSERT(m_pStorage == NULL); // We should only load one at a time per object!
    CHECK_NULL_RETURN((m_pStorage == NULL), E_DOCUMENTOPENED);

 // Validate the name passed and resolve to full path (if relative)...
    CHECK_NULL_RETURN(sFileName, E_INVALIDARG);
    if (!FFindQualifiedFileName(sFileName, wszFullName, &ulIdx))
        return STG_E_INVALIDNAME;

 // Save file name and path index from SearchFile API...
    m_bstrFileName = SysAllocString(wszFullName);
    m_cFilePartIdx = ulIdx;
    if ((m_cFilePartIdx < 1) || (m_cFilePartIdx > SysStringLen(m_bstrFileName))) 
		m_cFilePartIdx = 0;

 // Set open mode flags based on ReadOnly flag (the exclusive access is required for
 // the IPropertySetStorage interface -- which sucks, but we can work around for OLE files)...
    m_fReadOnly = (ReadOnly != VARIANT_FALSE);
    m_dwFlags = Options;
    dwOpenMode = ((m_fReadOnly) ? (STGM_READ | STGM_SHARE_EXCLUSIVE) : (STGM_READWRITE | STGM_SHARE_EXCLUSIVE));

 // If the file is an OLE Storage DocFile...
    if (StgIsStorageFile(m_bstrFileName) == S_OK)
    {
     // Get the data from IStorage...
	    hr = StgOpenStorage(m_bstrFileName, NULL, dwOpenMode, NULL, 0, &m_pStorage);

     // If we failed to gain write access, try to just read access if caller allows
	 // it. This function will open the OLE file in transacted read mode, which
	 // covers cases where the file is in use or is on a read-only share. We can't
	 // save after the open so we force the read-only flag on...
        if (((hr == STG_E_ACCESSDENIED) || (hr == STG_E_SHAREVIOLATION)) && 
            (m_dwFlags & dsoOptionOpenReadOnlyIfNoWriteAccess))
        {
            m_fReadOnly = TRUE;
	        hr = StgOpenStorage(m_bstrFileName, NULL, 
				(STGM_READ | STGM_TRANSACTED | STGM_SHARE_DENY_NONE), NULL, 0, &m_pStorage);
        }
        
	 // If we are lucky, we have a storage to read from, so ask OLE to open the 
	 // associated property set for the file and return the IPSS iface...
	    if (SUCCEEDED(hr))
        {
            hr = m_pStorage->QueryInterface(IID_IPropertySetStorage, (void**)&m_pPropSetStg);
        }
    }
    else if ((v_pfnStgOpenStorageEx) && 
             ((m_dwFlags & dsoOptionOnlyOpenOLEFiles) != dsoOptionOnlyOpenOLEFiles))
    {
     // On Win2K+ we can try and open plain files on NTFS 5.0 drive and get 
     // the NTFS version of OLE properties (saved in alt stream)...
        hr = (v_pfnStgOpenStorageEx)(m_bstrFileName, dwOpenMode, STGFMT_FILE, 0, NULL, 0, 
                IID_IPropertySetStorage, (void**)&m_pPropSetStg);

     // If we failed to gain write access, try to just read access if caller
     // wants us to. This only works for access block, not share violations...
       if ((hr == STG_E_ACCESSDENIED) && (!m_fReadOnly) && 
            (m_dwFlags & dsoOptionOpenReadOnlyIfNoWriteAccess))
        {
            m_fReadOnly = TRUE;
            hr = (v_pfnStgOpenStorageEx)(m_bstrFileName, (STGM_READ | STGM_SHARE_EXCLUSIVE), STGFMT_FILE,
                0, NULL, 0, IID_IPropertySetStorage, (void**)&m_pPropSetStg);
        }
    }
    else
    {  // If we land here, the file is non-OLE file, and not on NTFS5 drive,
	   // so we return an error that file has no valid OLE/NTFS extended properties...
        hr = E_NODOCUMENTPROPS; 
    }

    if ( FAILED(hr) )
    {
        Close( VARIANT_FALSE ); // Force a cleanup on error...
    }
	else
	{
		hr = get_SummaryProperties( &m_pSummProps );
		if ( FAILED(hr) )
		{
			delete m_pSummProps;
		}
	}

    return hr;
}

////////////////////////////////////////////////////////////////////////
// Close  --  Close the open document (optional save before close)
//
HRESULT CDocReader::
	CDocumentProperties::Close(VARIANT_BOOL SaveBeforeClose)
{
	ODS(_T("CDocReader::CDocumentProperties::Close\n"));

 // If caller requests full save on close, try it. Note that this is the
 // only place where Close will return an error (and NOT close)...
    if (SaveBeforeClose != VARIANT_FALSE)
    {
        HRESULT hr = Save();
        RETURN_ON_FAILURE(hr);
    }

 // The rest is just cleanup to restore us back to state where
 // we can be called again. The Zombie call disconnects sub objects
 // and should free them if caller has also released them...
    ZOMBIE_OBJECT(m_pSummProps);
    
    if ( m_pPropSetStg )
	{
		m_pPropSetStg->Release();
		m_pPropSetStg = NULL;
	}
	if ( m_pStorage )
	{
		m_pStorage->Release();
		m_pStorage = NULL;
	}
    FREE_BSTR(m_bstrFileName);
    m_cFilePartIdx = 0;
    m_dwFlags = dsoOptionDefault;
    m_fReadOnly = FALSE;

    return S_OK;
}

////////////////////////////////////////////////////////////////////////
// get_IsReadOnly - Returns User-Friendly Name for File Type
//
HRESULT CDocReader::
	CDocumentProperties::get_IsReadOnly(VARIANT_BOOL* pbReadOnly)
{
	ODS(_T("CDocReader::CDocumentProperties::get_IsReadOnly\n"));
	CHECK_NULL_RETURN(pbReadOnly,  E_POINTER); 
    *pbReadOnly = ((m_fReadOnly) ? VARIANT_TRUE : VARIANT_FALSE);
    return S_OK;
}

////////////////////////////////////////////////////////////////////////
// get_IsDirty  -- Have any changes been made to the properties?
//
HRESULT CDocReader::
	CDocumentProperties::get_IsDirty(VARIANT_BOOL* pbDirty)
{
    BOOL fDirty = FALSE;
 	ODS(_T("CDocReader::CDocumentProperties::get_IsDirty\n"));

 // Check the status of summary properties...
    if ((m_pSummProps) && (m_pSummProps->FIsDirty()))
        fDirty = TRUE;

    if (pbDirty) // Return status to caller...
        *pbDirty = (VARIANT_BOOL)((fDirty) ? VARIANT_TRUE : VARIANT_FALSE);
 
    return S_OK;
}

////////////////////////////////////////////////////////////////////////
// Save  --  Will save the changes made back to the document.
//
HRESULT CDocReader::
	CDocumentProperties::Save()
{
    HRESULT hr = S_FALSE;
    BOOL fSaveMade = FALSE;

 	ODS(_T("CDocReader::CDocumentProperties::Save\n"));
    CHECK_FLAG_RETURN(m_fReadOnly, E_DOCUMENTREADONLY);

 // Ask SummaryProperties to save its changes...
    if (m_pSummProps)
    {
        hr = m_pSummProps->SaveProperties(TRUE);
        if (FAILED(hr)) return hr;
        fSaveMade = (hr == S_OK);
    }

 // If save was made, commit the root storage before return...
    if ((fSaveMade) && (m_pStorage))
    {
        hr = m_pStorage->Commit(STGC_DEFAULT);
    }

    return hr;
}

////////////////////////////////////////////////////////////////////////
// get_SummaryProperties - Returns SummaryProperties object
//
HRESULT CDocReader::
	CDocumentProperties::get_SummaryProperties(CSummaryProperties** ppSummaryProperties)
{
    HRESULT hr = E_FAIL;

 	ODS(_T("CDocReader::CDocumentProperties::get_SummaryProperties\n"));
    CHECK_NULL_RETURN(ppSummaryProperties,  E_POINTER);
    *ppSummaryProperties = NULL;

    if (m_pSummProps == NULL)
    {
        m_pSummProps = new (std::nothrow) CSummaryProperties(m_bOnlyThumb);
        if (m_pSummProps)
            { hr = m_pSummProps->LoadProperties(m_pPropSetStg, m_fReadOnly, m_dwFlags); }
        else hr = E_OUTOFMEMORY;

        if (FAILED(hr))
        {
            ZOMBIE_OBJECT(m_pSummProps);
            return hr;
        }
    }
	*ppSummaryProperties = m_pSummProps;
    return hr;
}

////////////////////////////////////////////////////////////////////////
// get_Icon - Returns OLE StdPicture object with associated icon 
//
HRESULT CDocReader::
	CDocumentProperties::get_Icon(IDispatch** ppicIcon)
{
	HICON hIco;

	ODS(_T("CDocReader::CDocumentProperties::get_Icon\n"));
	CHECK_NULL_RETURN(ppicIcon,  E_POINTER); *ppicIcon = NULL;
    CHECK_NULL_RETURN(m_pPropSetStg, E_DOCUMENTNOTOPEN);

    if ( (m_bstrFileName) && FGetIconForFile( m_bstrFileName, &hIco ))
    {
		PICTDESC  icoDesc;
		icoDesc.cbSizeofstruct = sizeof(PICTDESC);
		icoDesc.picType = PICTYPE_ICON;
		icoDesc.icon.hicon = hIco;
		return OleCreatePictureIndirect(&icoDesc, IID_IDispatch, TRUE, (void**)ppicIcon);
    }
    return S_FALSE;
}

////////////////////////////////////////////////////////////////////////
// get_Name - Returns the name of the file (no path)
//
HRESULT CDocReader::
	CDocumentProperties::get_Name(BSTR* pbstrName)
{
	ODS(_T("CDocReader::CDocumentProperties::get_Name\n"));
	CHECK_NULL_RETURN(pbstrName,  E_POINTER); *pbstrName = NULL;
    CHECK_NULL_RETURN(m_pPropSetStg, E_DOCUMENTNOTOPEN);

	if (m_bstrFileName != NULL && m_cFilePartIdx > 0)
		*pbstrName = SysAllocString((LPOLESTR)&(m_bstrFileName[m_cFilePartIdx]));

	return S_OK;
}

////////////////////////////////////////////////////////////////////////
// get_Name - Returns the path to the file (no name)
//
HRESULT CDocReader::
	CDocumentProperties::get_Path(BSTR* pbstrPath)
{
	ODS(_T("CDocReader::CDocumentProperties::get_Path\n"));
	CHECK_NULL_RETURN(pbstrPath,  E_POINTER); *pbstrPath = NULL;
    CHECK_NULL_RETURN(m_pPropSetStg, E_DOCUMENTNOTOPEN);

	if (m_bstrFileName != NULL && m_cFilePartIdx > 0)
	    *pbstrPath = SysAllocStringLen(m_bstrFileName, m_cFilePartIdx);
	
	return S_OK;
}

////////////////////////////////////////////////////////////////////////
// get_IsOleFile - Returns True if file is OLE DocFile
//
HRESULT CDocReader::
	CDocumentProperties::get_IsOleFile(VARIANT_BOOL* pIsOleFile)
{
	ODS(_T("CDocReader::CDocumentProperties::get_IsOleFile\n"));
	CHECK_NULL_RETURN(pIsOleFile,  E_POINTER);
    *pIsOleFile = ((m_pStorage) ? VARIANT_TRUE : VARIANT_FALSE);
    return S_OK;
}

////////////////////////////////////////////////////////////////////////
// get_Name - Returns CLSID of OLE DocFile 
//
HRESULT CDocReader::
	CDocumentProperties::get_CLSID(BSTR* pbstrCLSID)
{
    HRESULT hr;
	STATSTG stat;
	LPOLESTR pwszCLSID = NULL;

	ODS(_T("CDocReader::CDocumentProperties::get_CLSID\n"));
	CHECK_NULL_RETURN(pbstrCLSID,  E_POINTER); *pbstrCLSID = NULL;
    CHECK_NULL_RETURN(m_pPropSetStg, E_DOCUMENTNOTOPEN);
    CHECK_NULL_RETURN(m_pStorage, E_MUSTHAVESTORAGE);

    memset(&stat, 0, sizeof(stat));
    hr = m_pStorage->Stat(&stat, STATFLAG_NONAME);
    RETURN_ON_FAILURE(hr);

	hr = StringFromCLSID(stat.clsid, &pwszCLSID);
    if (SUCCEEDED(hr)) *pbstrCLSID = SysAllocString(pwszCLSID);

    FREE_COTASKMEM(pwszCLSID);
	return hr;
}

////////////////////////////////////////////////////////////////////////
// get_ProgID - Returns ProgID of OLE DocFile 
//
HRESULT CDocReader::
	CDocumentProperties::get_ProgID(BSTR* pbstrProgID)
{
    HRESULT hr;
	STATSTG stat;
	LPOLESTR pwszProgID = NULL;

	ODS(_T("CDocReader::CDocumentProperties::get_ProgID\n"));
	CHECK_NULL_RETURN(pbstrProgID,  E_POINTER); *pbstrProgID = NULL;
    CHECK_NULL_RETURN(m_pPropSetStg, E_DOCUMENTNOTOPEN);
    CHECK_NULL_RETURN(m_pStorage, E_MUSTHAVESTORAGE);

    memset(&stat, 0, sizeof(stat));
    hr = m_pStorage->Stat(&stat, STATFLAG_NONAME);
    RETURN_ON_FAILURE(hr);

	hr = ProgIDFromCLSID(stat.clsid, &pwszProgID);
	if (SUCCEEDED(hr)) *pbstrProgID = SysAllocString(pwszProgID);

    FREE_COTASKMEM(pwszProgID);
	return hr;
}

////////////////////////////////////////////////////////////////////////
// get_OleDocumentFormat - Returns ClipFormat of OLE DocFile 
//
HRESULT CDocReader::
	CDocumentProperties::get_OleDocumentFormat(BSTR* pbstrFormat)
{
    HRESULT hr = S_FALSE;
    CLIPFORMAT cf;

	ODS(_T("CDocReader::CDocumentProperties::get_OleDocumentFormat\n"));
	CHECK_NULL_RETURN(pbstrFormat,  E_POINTER); *pbstrFormat = NULL;
    CHECK_NULL_RETURN(m_pPropSetStg, E_DOCUMENTNOTOPEN);
    CHECK_NULL_RETURN(m_pStorage, E_MUSTHAVESTORAGE);

    if (SUCCEEDED(ReadFmtUserTypeStg(m_pStorage, &cf, NULL)) == TRUE)
    {
        TCHAR szName[ MAX_PATH ] = {};
		int i = GetClipboardFormatName( cf, szName, MAX_PATH );
        if ( i > 0)
        {
            szName[i] = '\0';
        }
        else
        {
            wsprintf(szName, _T("ClipFormat 0x%X (%d)"), cf, cf);
        }
        *pbstrFormat = ConvertToBSTR(szName, CP_ACP);
        hr = ((*pbstrFormat) ? S_OK : E_OUTOFMEMORY);
    }

    return hr;
}

////////////////////////////////////////////////////////////////////////
// get_OleDocumentType - Returns User-Friendly Name for File Type
//
HRESULT CDocReader::
	CDocumentProperties::get_OleDocumentType(BSTR* pbstrType)
{
    HRESULT hr = S_FALSE;;
    LPWSTR lpolestr = NULL;

	ODS(_T("CDocReader::CDocumentProperties::get_OleDocumentType\n"));
	CHECK_NULL_RETURN(pbstrType,  E_POINTER); *pbstrType = NULL;
    CHECK_NULL_RETURN(m_pPropSetStg, E_DOCUMENTNOTOPEN);
    CHECK_NULL_RETURN(m_pStorage, E_MUSTHAVESTORAGE);

    if (SUCCEEDED(ReadFmtUserTypeStg(m_pStorage, NULL, &lpolestr)) == TRUE)
    {
        *pbstrType = SysAllocString(lpolestr);
        hr = ((*pbstrType) ? S_OK : E_OUTOFMEMORY);
        FREE_COTASKMEM(lpolestr);
    }

    return hr;
}

////////////////////////////////////////////////////////////////////////
// CSummaryProperties
//
////////////////////////////////////////////////////////////////////////
// Class Constructor/Destructor
//
CDocReader::CDocumentProperties::
	CSummaryProperties::CSummaryProperties(BOOL	bOnlyThumb)
{
	ODS(_T("CSummaryProperties::CSummaryProperties()\n"));
    m_pPropSetStg    = NULL;
    m_dwFlags        = dsoOptionDefault;
    m_fReadOnly      = FALSE;
    m_fExternal      = FALSE;
    m_fDeadObj       = FALSE;
    m_pSummPropList  = NULL;
    m_pDocPropList   = NULL;
    m_wCodePageSI    = CP_ACP;
    m_wCodePageDSI   = CP_ACP;
	m_bOnlyThumb	 = bOnlyThumb;
}

CDocReader::CDocumentProperties::
	CSummaryProperties::~CSummaryProperties(void)
{
    ODS(_T("CSummaryProperties::~CSummaryProperties()\n"));
}

////////////////////////////////////////////////////////////////////////
// SummaryProperties Implementation
//
////////////////////////////////////////////////////////////////////////
// FMTID_SummaryInformation Properties...
//  
HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_Title(BSTR* pbstrTitle)
{
    ODS(_T("CSummaryProperties::get_Title\n"));
	CHECK_NULL_RETURN(pbstrTitle, E_POINTER);
	return ReadProperty(m_pSummPropList, PIDSI_TITLE, VT_BSTR, ((void*)pbstrTitle));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::put_Title(BSTR bstrTitle)
{
    TRACE1("CSummaryProperties::put_Title(%S)\n", (LPCSTR)CW2A((LPCWSTR)bstrTitle) );
	return WriteProperty(&m_pSummPropList, PIDSI_TITLE, VT_BSTR, ((void*)bstrTitle));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_Subject(BSTR* pbstrSubject)
{
    ODS(_T("CSummaryProperties::get_Subject\n"));
	CHECK_NULL_RETURN(pbstrSubject, E_POINTER);
	return ReadProperty(m_pSummPropList, PIDSI_SUBJECT, VT_BSTR, ((void*)pbstrSubject));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::put_Subject(BSTR bstrSubject)
{
    TRACE1("CSummaryProperties::put_Subject(%S)\n", (LPCSTR)CW2A((LPCWSTR)bstrSubject));
	return WriteProperty(&m_pSummPropList, PIDSI_SUBJECT, VT_BSTR, ((void*)bstrSubject));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_Author(BSTR* pbstrAuthor)
{
    ODS(_T("CSummaryProperties::get_Author\n"));
	CHECK_NULL_RETURN(pbstrAuthor, E_POINTER);
	return ReadProperty(m_pSummPropList, PIDSI_AUTHOR, VT_BSTR, ((void*)pbstrAuthor));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::put_Author(BSTR bstrAuthor)
{
    TRACE1("CSummaryProperties::put_Author(%S)\n", (LPCSTR)CW2A((LPCWSTR)bstrAuthor));
	return WriteProperty(&m_pSummPropList, PIDSI_AUTHOR, VT_BSTR, ((void*)bstrAuthor));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_Keywords(BSTR* pbstrKeywords)
{
    ODS(_T("CSummaryProperties::get_Keywords\n"));
	CHECK_NULL_RETURN(pbstrKeywords, E_POINTER);
	return ReadProperty(m_pSummPropList, PIDSI_KEYWORDS, VT_BSTR, ((void*)pbstrKeywords));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::put_Keywords(BSTR bstrKeywords)
{
    TRACE1("CSummaryProperties::put_Keywords(%S)\n", (LPCSTR)CW2A((LPCWSTR)bstrKeywords));
	return WriteProperty(&m_pSummPropList, PIDSI_KEYWORDS, VT_BSTR, ((void*)bstrKeywords));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_Comments(BSTR* pbstrComments)
{
    ODS(_T("CSummaryProperties::get_Comments\n"));
	CHECK_NULL_RETURN(pbstrComments, E_POINTER);
	return ReadProperty(m_pSummPropList, PIDSI_COMMENTS, VT_BSTR, ((void*)pbstrComments));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::put_Comments(BSTR bstrComments)
{
    TRACE1("CSummaryProperties::put_Comments(%S)\n", (LPCSTR)CW2A((LPCWSTR)bstrComments));
	return WriteProperty(&m_pSummPropList, PIDSI_COMMENTS, VT_BSTR, ((void*)bstrComments));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_Template(BSTR* pbstrTemplate)
{
    ODS(_T("CSummaryProperties::get_Template\n"));
	CHECK_NULL_RETURN(pbstrTemplate, E_POINTER);
	return ReadProperty(m_pSummPropList, PIDSI_TEMPLATE, VT_BSTR, ((void*)pbstrTemplate));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_LastSavedBy(BSTR* pbstrLastSavedBy)
{
    ODS(_T("CSummaryProperties::get_LastSavedBy\n"));
	CHECK_NULL_RETURN(pbstrLastSavedBy, E_POINTER);
	return ReadProperty(m_pSummPropList, PIDSI_LASTAUTHOR, VT_BSTR, ((void*)pbstrLastSavedBy));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::put_LastSavedBy(BSTR bstrLastSavedBy)
{
    TRACE1("CSummaryProperties::put_LastSavedBy(%S)\n", (LPCSTR)CW2A((LPCWSTR)bstrLastSavedBy));
	return WriteProperty(&m_pSummPropList, PIDSI_LASTAUTHOR, VT_BSTR, ((void*)bstrLastSavedBy));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_RevisionNumber(BSTR* pbstrRevisionNumber)
{
    ODS(_T("CSummaryProperties::get_RevisionNumber\n"));
	CHECK_NULL_RETURN(pbstrRevisionNumber, E_POINTER);
	return ReadProperty(m_pSummPropList, PIDSI_REVNUMBER, VT_BSTR, ((void*)pbstrRevisionNumber));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_TotalEditTime(long* plTotalEditTime)
{
    ODS(_T("CSummaryProperties::get_TotalEditTime\n"));
	CHECK_NULL_RETURN(plTotalEditTime, E_POINTER);
	return ReadProperty(m_pSummPropList, PIDSI_EDITTIME, VT_I4, ((void*)plTotalEditTime));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_DateLastPrinted(VARIANT* pdtDateLastPrinted)
{
    ODS(_T("CSummaryProperties::get_DateLastPrinted\n"));
	CHECK_NULL_RETURN(pdtDateLastPrinted, E_POINTER);
	return ReadProperty(m_pSummPropList, PIDSI_LASTPRINTED, VT_DATE, ((void*)pdtDateLastPrinted));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_DateCreated(VARIANT* pdtDateCreated)
{
    ODS(_T("CSummaryProperties::get_DateCreated\n"));
	CHECK_NULL_RETURN(pdtDateCreated, E_POINTER);
	return ReadProperty(m_pSummPropList, PIDSI_CREATE_DTM, VT_DATE, ((void*)pdtDateCreated));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_DateLastSaved(VARIANT* pdtDateLastSaved)
{
    ODS(_T("CSummaryProperties::get_DateLastSaved\n"));
	CHECK_NULL_RETURN(pdtDateLastSaved, E_POINTER);
	return ReadProperty(m_pSummPropList, PIDSI_LASTSAVE_DTM, VT_DATE, ((void*)pdtDateLastSaved));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_PageCount(long* plPageCount)
{
    ODS(_T("CSummaryProperties::get_PageCount\n"));
	CHECK_NULL_RETURN(plPageCount, E_POINTER);
	return ReadProperty(m_pSummPropList, PIDSI_PAGECOUNT, VT_I4, ((void*)plPageCount));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_WordCount(long* plWordCount)
{
    ODS(_T("CSummaryProperties::get_WordCount\n"));
	CHECK_NULL_RETURN(plWordCount, E_POINTER);
	return ReadProperty(m_pSummPropList, PIDSI_WORDCOUNT, VT_I4, ((void*)plWordCount));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_CharacterCount(long* plCharacterCount)
{
    ODS(_T("CSummaryProperties::get_CharacterCount\n"));
	CHECK_NULL_RETURN(plCharacterCount, E_POINTER);
	return ReadProperty(m_pSummPropList, PIDSI_CHARCOUNT, VT_I4, ((void*)plCharacterCount));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_Thumbnail(VARIANT* pvtThumbnail)
{
    HRESULT hr = S_FALSE;
    CDocProperty* pitem;

    ODS(_T("CSummaryProperties::get_Thumbnail\n"));
	CHECK_NULL_RETURN(pvtThumbnail, E_POINTER);
    CHECK_FLAG_RETURN(m_fDeadObj, E_INVALIDOBJECT);

 // Get thumbnail item from the collection (if it was added).
    pitem = GetPropertyFromList(m_pSummPropList, PIDSI_THUMBNAIL, FALSE);
    if (pitem) hr = pitem->get_Value(pvtThumbnail);
    return hr;
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_ApplicationName(BSTR* pbstrAppName)
{
    ODS(_T("CSummaryProperties::get_ApplicationName\n"));
	CHECK_NULL_RETURN(pbstrAppName, E_POINTER);
	return ReadProperty(m_pSummPropList, PIDSI_APPNAME, VT_BSTR, ((void*)pbstrAppName));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_DocumentSecurity(long* plDocSecurity)
{
    ODS(_T("CSummaryProperties::get_DocumentSecurity\n"));
	CHECK_NULL_RETURN(plDocSecurity, E_POINTER);
	return ReadProperty(m_pSummPropList, PIDSI_DOC_SECURITY, VT_I4, ((void*)plDocSecurity));
}

////////////////////////////////////////////////////////////////////////
// FMTID_DocSummaryInformation Properties...
//  
HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_Category(BSTR* pbstrCategory)
{
    ODS(_T("CSummaryProperties::get_Category\n"));
	CHECK_NULL_RETURN(pbstrCategory, E_POINTER);
	return ReadProperty(m_pDocPropList, PID_CATEGORY, VT_BSTR, ((void*)pbstrCategory));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::put_Category(BSTR bstrCategory)
{
    TRACE1("CSummaryProperties::put_Category(%S)\n", (LPCSTR)CW2A((LPCWSTR)bstrCategory));
	return WriteProperty(&m_pDocPropList, PID_CATEGORY, VT_BSTR, ((void*)bstrCategory));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_PresentationFormat(BSTR* pbstrPresFormat)
{
    ODS(_T("CSummaryProperties::get_PresentationFormat\n"));
	CHECK_NULL_RETURN(pbstrPresFormat, E_POINTER);
	return ReadProperty(m_pDocPropList, PID_PRESFORMAT, VT_BSTR, ((void*)pbstrPresFormat));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_ByteCount(long* plByteCount)
{
    ODS(_T("CSummaryProperties::get_ByteCount\n"));
	CHECK_NULL_RETURN(plByteCount, E_POINTER);
	return ReadProperty(m_pDocPropList, PID_BYTECOUNT, VT_I4, ((void*)plByteCount));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_LineCount(long* plLineCount)
{
    ODS(_T("CSummaryProperties::get_LineCount\n"));
	CHECK_NULL_RETURN(plLineCount, E_POINTER);
	return ReadProperty(m_pDocPropList, PID_LINECOUNT, VT_I4, ((void*)plLineCount));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_ParagraphCount(long* plParagraphCount)
{
    ODS(_T("CSummaryProperties::get_ParagraphCount\n"));
	CHECK_NULL_RETURN(plParagraphCount, E_POINTER);
	return ReadProperty(m_pDocPropList, PID_PARACOUNT, VT_I4, ((void*)plParagraphCount));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_SlideCount(long* plSlideCount)
{
    ODS(_T("CSummaryProperties::get_SlideCount\n"));
	CHECK_NULL_RETURN(plSlideCount, E_POINTER);
	return ReadProperty(m_pDocPropList, PID_SLIDECOUNT, VT_I4, ((void*)plSlideCount));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_NoteCount(long* plNoteCount)
{
    ODS(_T("CSummaryProperties::get_NoteCount\n"));
	CHECK_NULL_RETURN(plNoteCount, E_POINTER);
	return ReadProperty(m_pDocPropList, PID_NOTECOUNT, VT_I4, ((void*)plNoteCount));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_HiddenSlideCount(long* plHiddenSlideCount)
{
    ODS(_T("CSummaryProperties::get_HiddenSlideCount\n"));
	CHECK_NULL_RETURN(plHiddenSlideCount, E_POINTER);
	return ReadProperty(m_pDocPropList, PID_HIDDENCOUNT, VT_I4, ((void*)plHiddenSlideCount));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_MultimediaClipCount(long* plMultimediaClipCount)
{
    ODS(_T("CSummaryProperties::get_MultimediaClipCount\n"));
	CHECK_NULL_RETURN(plMultimediaClipCount, E_POINTER);
	return ReadProperty(m_pDocPropList, PID_MMCLIPCOUNT, VT_I4, ((void*)plMultimediaClipCount));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_Manager(BSTR* pbstrManager)
{
    ODS(_T("CSummaryProperties::get_Manager\n"));
	CHECK_NULL_RETURN(pbstrManager, E_POINTER);
	return ReadProperty(m_pDocPropList, PID_MANAGER, VT_BSTR, ((void*)pbstrManager));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::put_Manager(BSTR bstrManager)
{
    TRACE1("CSummaryProperties::put_Manager(%S)\n", (LPCSTR)CW2A((LPCWSTR)bstrManager));
	return WriteProperty(&m_pDocPropList, PID_MANAGER, VT_BSTR, ((void*)bstrManager));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_Company(BSTR* pbstrCompany)
{
    ODS(_T("CSummaryProperties::get_Company\n"));
	CHECK_NULL_RETURN(pbstrCompany, E_POINTER);
	return ReadProperty(m_pDocPropList, PID_COMPANY, VT_BSTR, ((void*)pbstrCompany));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::put_Company(BSTR bstrCompany)
{
    TRACE1("CSummaryProperties::put_Company(%S)\n", (LPCSTR)CW2A((LPCWSTR)bstrCompany));
	return WriteProperty(&m_pDocPropList, PID_COMPANY, VT_BSTR, ((void*)bstrCompany));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_CharacterCountWithSpaces(long* plCharCountWithSpaces)
{
    ODS(_T("CSummaryProperties::get_CharacterCountWithSpaces\n"));
	CHECK_NULL_RETURN(plCharCountWithSpaces, E_POINTER);
	return ReadProperty(m_pDocPropList, PID_CCHWITHSPACES, VT_I4, ((void*)plCharCountWithSpaces));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_SharedDocument(VARIANT_BOOL* pbSharedDocument)
{
    ODS(_T("CSummaryProperties::get_SharedDocument\n"));
	CHECK_NULL_RETURN(pbSharedDocument, E_POINTER);
	return ReadProperty(m_pDocPropList, PID_SHAREDDOC, VT_BOOL, ((void*)pbSharedDocument));
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_Version(BSTR* pbstrVersion)
{
    LONG nVersion = 0;
    ODS(_T("CSummaryProperties::get_Version\n"));
	CHECK_NULL_RETURN(pbstrVersion, E_POINTER); *pbstrVersion = NULL;
    if (SUCCEEDED(ReadProperty(m_pDocPropList, PID_VERSION, VT_I4, (void*)&nVersion)))
    {
        TCHAR szVersion[128];
        wsprintf(szVersion, _T("%d.%d"), (LONG)(HIWORD(nVersion)), (LONG)(LOWORD(nVersion)));
        *pbstrVersion = ConvertToBSTR(szVersion, CP_ACP);
    }
    return S_OK;
}

HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::get_DigitalSignature(VARIANT* pvtDigSig)
{
    HRESULT hr = S_FALSE;
    CDocProperty* pitem;

    ODS(_T("CSummaryProperties::get_DigitalSignature\n"));
	CHECK_NULL_RETURN(pvtDigSig, E_POINTER);
    CHECK_FLAG_RETURN(m_fDeadObj, E_INVALIDOBJECT);

 // Get DigSig data as CF_BLOB...
    pitem = GetPropertyFromList(m_pDocPropList, PID_DIGSIG, FALSE);
    if (pitem) hr = pitem->get_Value(pvtDigSig);
    return hr;
}

////////////////////////////////////////////////////////////////////////
// Internal Functions
//  
////////////////////////////////////////////////////////////////////////
// LoadProperties -- Reads in both property sets and provides link list
//   of the properties for each (we keep separatelists because each may
//   have overlapping PROPIDs and different CodePages).
//  
HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::LoadProperties(IPropertySetStorage* pPropSS, BOOL fIsReadOnly, dsoFileOpenOptions dwFlags)
{
    HRESULT hr = S_OK;;
    IPropertyStorage *pProps;

 // We only do this once (when loading from file)...
	TRACE2("CSummaryProperties::LoadProperties(ReadOnly=%d, Flags=%d)\n", fIsReadOnly, dwFlags);
    ASSERT(m_pSummPropList == NULL); ASSERT(m_pDocPropList == NULL);

 // First, load the FMTID_SummaryInformation properties...
    hr = OpenPropertyStorage(pPropSS, FMTID_SummaryInformation, fIsReadOnly, dwFlags, &pProps);

    if ( SUCCEEDED(hr) )
    {
     // Load all the properties into a list set (and save the code page). The list
     // may return NULL if no properties are found, but that is OK. We just return
     // blank values for items as if they were set to zero...
        hr = LoadPropertySetList( pProps, &m_wCodePageSI, &m_pSummPropList, m_bOnlyThumb );
        pProps->Release();
    }
    else
    {
     // In cases where the propset is not in the file and it is read-only open
     // or a case where DontAutoCreate flag is used, we just treat as read-only
     // with no properties. Otherwise we return error that propset is invalid...
        if (hr == STG_E_FILENOTFOUND) 
        { // We allow partial open if NoAutoCreate is set.
            if ((fIsReadOnly) || (dwFlags & dsoOptionDontAutoCreate))
            {
                fIsReadOnly = TRUE;
                hr = S_FALSE;
            }
            else hr = E_INVALIDPROPSET;
        }
    }
    RETURN_ON_FAILURE(hr);

 // Second, load the FMTID_DocSummaryInformation properties...
    hr = OpenPropertyStorage(pPropSS, FMTID_DocSummaryInformation, fIsReadOnly, dwFlags, &pProps);
    if ( SUCCEEDED(hr) )
    {
     // Load all the properties into a list set (and save the code page). The list
     // may return NULL if no properties are found, but that is OK. We just return
     // blank values for items as if they were set to zero...
        hr = LoadPropertySetList(pProps, &m_wCodePageDSI, &m_pDocPropList, m_bOnlyThumb);
        pProps->Release();
    }
    else
    {
     // In cases where the propset is not in the file and it is read-only open
     // or a case where DontAutoCreate flag is used, we just treat as read-only
     // with no properties. Otherwise we return error that propset is invalid...
        if (hr == STG_E_FILENOTFOUND) 
        { // We allow partial open if NoAutoCreate is set.
            if ((fIsReadOnly) || (dwFlags & dsoOptionDontAutoCreate))
            {
                fIsReadOnly = TRUE;
                hr = S_FALSE;
            }
            else hr = E_INVALIDPROPSET;
        }
    }
    
 // If all wen well, store the parameters passed for later use...
    if (SUCCEEDED(hr))
    {
        m_pPropSetStg = pPropSS;
        m_fReadOnly = fIsReadOnly;
        m_dwFlags = dwFlags;
    }

    return hr;
}

////////////////////////////////////////////////////////////////////////
// ReadProperty -- Reads property of standard type and copies to pv...
//  
HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::ReadProperty(CDocProperty* pPropList, PROPID pid, VARTYPE vt, void* pv)
{
	HRESULT hr = S_FALSE;
    CDocProperty* pitem;
	VARIANT vtTmp;

	TRACE1("CSummaryProperties::ReadProperty(id=%u)\n", pid);

    CHECK_FLAG_RETURN(m_fDeadObj, E_INVALIDOBJECT);

    pitem = GetPropertyFromList(pPropList, pid, FALSE);
    if ( pitem )
    {
	    VariantInit(&vtTmp);
        hr = pitem->get_Value(&vtTmp);
        if ( SUCCEEDED(hr) )
        {
         // If data returned is not in the expected type, try to convert it...
            if ( ( vtTmp.vt != vt ) && 
                ( FAILED(VariantChangeType( &vtTmp, &vtTmp, 0, vt )) ) )
                return S_FALSE; // E_UNEXPECTED; FIX - 2/18/2000 (return S_FALSE same as missing).

         // Return the native data based on the VT type...
            switch (vt)
		    {
		    case VT_BSTR: *((BSTR*)pv) = SysAllocString( vtTmp.bstrVal ); break;
            case VT_I4:   *((LONG*)pv) = vtTmp.lVal; break;
            case VT_BOOL: *((VARIANT_BOOL*)pv) = vtTmp.boolVal; break;
		    case VT_DATE: VariantCopy( ( (VARIANT*)pv ), &vtTmp ); break;
		    }
            VariantClear( &vtTmp );
        }
    }

	return hr;
}

////////////////////////////////////////////////////////////////////////
// WriteProperty -- Writes property of standard type (can append the list)...
//  
HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::WriteProperty(CDocProperty** ppPropList, PROPID pid, VARTYPE vt, void* pv)
{
	HRESULT hr = S_FALSE;
    CDocProperty* pitem;
	VARIANT vtItem; 

 // Going to add property to list. Make sure we allow writes, and make sure prop list exists...
	TRACE1("CSummaryProperties::WriteProperty(id=%u)\n", pid);
    CHECK_NULL_RETURN(ppPropList,  E_POINTER);
    CHECK_FLAG_RETURN(m_fDeadObj,  E_INVALIDOBJECT);
    CHECK_FLAG_RETURN(m_fReadOnly, E_DOCUMENTREADONLY);

 // We load Variant based on selected VT (only handle values of 32-bit or lower)
 // This would be a problem for generic read/write, but for summary and doc summary
 // properties, these are the only common types this component handles (so we are OK with this).
	vtItem.vt = vt;
    switch ( vt )
    {
      case VT_BSTR: vtItem.bstrVal = ((BSTR)pv); break;
      case VT_I4:   vtItem.lVal    = ((LONG)(LONG_PTR)pv); break;
      //case VT_BOOL: vtItem.boolVal = ((VARIANT_BOOL)((LONG)pv)); break;
	  //case VT_DATE: VariantCopy(&vtItem, ((VARIANT*)pv)); break;
      default: return E_INVALIDARG;
    }

 // Find the cached item in the list and update it...
    pitem = GetPropertyFromList(*ppPropList, pid, TRUE);
    if (pitem)
    {
        hr = pitem->put_Value(&vtItem);

     // Special case where we are adding new property to list that had 
     // no properties to start with (NTFS5 files mostly, since normal 
     // Office/OLE files will have at least one summ/docsumm prop...
        if ( ( *ppPropList == NULL ) && SUCCEEDED(hr) )
            *ppPropList = pitem;
    }
    else
    {
     // If it is not in the list, nor can be added...
        hr = E_ACCESSDENIED;
    }

	return hr;
}

////////////////////////////////////////////////////////////////////////
// SaveProperties -- Save all changes.
//  
HRESULT CDocReader::CDocumentProperties::
	CSummaryProperties::SaveProperties(BOOL fCommitChanges)
{
    HRESULT hr = S_FALSE;
    IPropertyStorage *pProps;
    ULONG cSaved, cTotalItems = 0;

 // We don't need to do save if in read-only mode...
	TRACE1("CSummaryProperties::SaveProperties(Commit=%u)\n", (ULONG)fCommitChanges);
    CHECK_FLAG_RETURN(m_fReadOnly, E_DOCUMENTREADONLY);

 // Save the Summary properties..
    if (m_pSummPropList)
    {
        hr = OpenPropertyStorage(m_pPropSetStg, FMTID_SummaryInformation, FALSE, 0, &pProps);
        if ( SUCCEEDED(hr) )
        {
         // Save all the changed items in the list...
            hr = SavePropertySetList(pProps, m_wCodePageSI, m_pSummPropList, &cSaved);
            
         // Commit the changes to the file...
            if ( SUCCEEDED(hr) && (cSaved) && (fCommitChanges) )
                hr = pProps->Commit( STGC_DEFAULT );

            cTotalItems = cSaved;
            pProps->Release();
        }
    }

 // Then save doc summary properties...
    if ( SUCCEEDED(hr) && ( m_pDocPropList ) )
    {
        hr = OpenPropertyStorage( m_pPropSetStg, FMTID_DocSummaryInformation, FALSE, 0, &pProps );
        if (SUCCEEDED(hr))
        {
         // Save all the changed items in the list...
            hr = SavePropertySetList(pProps, m_wCodePageDSI, m_pDocPropList, &cSaved);
            
         // Commit the changes to the file...
            if ( SUCCEEDED(hr) && (cSaved) && (fCommitChanges) )
                hr = pProps->Commit(STGC_DEFAULT);

            cTotalItems += cSaved;
            pProps->Release();
        }
    }

    return ( FAILED(hr) ? hr : ( (cTotalItems) ? S_OK : S_FALSE ) );
}

////////////////////////////////////////////////////////////////////////
// GetPropertyFromList -- Enumerates a list and finds item with the 
//    matching id. It can also add a new item (if flag set).
//  
CDocProperty* CDocReader::CDocumentProperties::
	CSummaryProperties::GetPropertyFromList(CDocProperty* plist, PROPID id, BOOL fAppendNew)
{
    CDocProperty* pitem = plist;
    CDocProperty* plast = pitem;

	ODS(_T("CSummaryProperties::FindPropertyInList\n"));

 // Loop the list until you find the item...
    while (pitem)
    {
        if ( pitem->GetID() == id ) break;

        plast = pitem;
        pitem = pitem->GetNextProperty();
    }

// If no match is found, we can add a new item to end of the list...
    if ( ( pitem == NULL ) && (plast) && (fAppendNew) )
    {
     // Create the item...
        pitem = new (std::nothrow) CDocProperty();
        if ( pitem )
        {
            VARIANT var; var.vt = VT_EMPTY;
            if ( FAILED(pitem->InitProperty( NULL, id, &var, TRUE, (plast->AppendLink( pitem )) )) )
            { // If we fail, try to reverse the append and kill the new object...
                plast->AppendLink( ( pitem->GetNextProperty() ) );
                pitem = NULL;
            }
        }
    }

    return pitem;
}

////////////////////////////////////////////////////////////////////////
// FIsDirty -- Checks both lists to see if changes were made.
//  
BOOL CDocReader::CDocumentProperties::
	CSummaryProperties::FIsDirty()
{
    BOOL fDirty = FALSE;
    CDocProperty* pitem;
    
	ODS(_T("CSummaryProperties::FIsDirty\n"));

 // Loop through summary items and see if any have changed...
    pitem = m_pSummPropList;
    while (pitem)
    {
        if (pitem->IsDirty()){ fDirty = TRUE; break; }
        pitem = pitem->GetNextProperty();
    }

 // Look through doc summary items to see if those changed...
    if (!fDirty)
    {
        pitem = m_pDocPropList;
        while (pitem)
        {
            if (pitem->IsDirty()){ fDirty = TRUE; break; }
            pitem = pitem->GetNextProperty();
        }
    }

    return fDirty;
}

////////////////////////////////////////////////////////////////////////
// Disconnect -- Parent is closing, so disconnect object.
//  
void CDocReader::CDocumentProperties::
	CSummaryProperties::Disconnect()
{
    CDocProperty *pnext;
	ODS(_T("CSummaryProperties::Disconnect\n"));

 // Loop through both lists and disconnect item (this should free them)...
    m_pSummPropList = NULL;
    while ( m_pSummPropList )
    {
        pnext = m_pSummPropList->GetNextProperty();
        m_pSummPropList->Disconnect(); m_pSummPropList = pnext;
    }

    m_pDocPropList = NULL;
    while (m_pDocPropList)
    {
        pnext = m_pDocPropList->GetNextProperty();
        m_pDocPropList->Disconnect(); m_pDocPropList = pnext;
    }


 // We are now dead ourselves (release prop storage ref)...
    m_pPropSetStg = NULL;
    m_fDeadObj = TRUE;
    return;
}