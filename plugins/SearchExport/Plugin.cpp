// Plugin.cpp : Implementation of CPlugin

#include "stdafx.h"
#include "Plugin.h"

static const LPCWSTR SearchExport_CHECK	= L"&Export...";

// Insert menu item if no item present only
void InsertCommand(ISMenu* pMenu, int nPos, UINT nID, LPCWSTR szItem)
{
	LONG nCount;
	if ( SUCCEEDED( pMenu->get_Count( &nCount ) ) )
	{
		for ( int i = 0; i < (int)nCount; ++i )
		{
			CComPtr< ISMenu > pItem;
			LONG nItemID;	// note: -1 - submenu, 0 - separator
			if ( SUCCEEDED( pMenu->get_Item( CComVariant( i ), &pItem ) ) && pItem &&
				 SUCCEEDED( pItem->get_CommandID( &nItemID ) ) && (UINT)nItemID == nID )
				// Already in place
				return;
		}
	}

	// Insert new
	pMenu->InsertCommand( nPos, nID, CComBSTR( szItem ), NULL );
}

HRESULT CreateElement(ISXMLElement* pRoot, LPCTSTR szName, ISXMLElement** ppElement)
{
	HRESULT hr;

	CComPtr< ISXMLElements > pRootElements;
	if ( pRoot )
	{
		hr = pRoot->get_Elements( &pRootElements );
		ATLASSERT( SUCCEEDED( hr ) );
		if ( FAILED( hr ) )
			return hr;
	}

	CComPtr< ISXMLElement > pElement;
	hr = pElement.CoCreateInstance( CLSID_ShareazaXML );
	ATLASSERT( SUCCEEDED( hr ) );
	if ( FAILED( hr ) )
		return hr;

	hr = pElement->put_Name( CComBSTR( szName ) );
	ATLASSERT( SUCCEEDED( hr ) );
	if ( FAILED( hr ) )
		return hr;

	if ( pRoot )
	{
		hr = pRootElements->Attach( pElement );
		ATLASSERT( SUCCEEDED( hr ) );
	}

	*ppElement = pElement.Detach();

	return hr;
}

HRESULT CreateElement(ISXMLElement* pRoot, LPCTSTR szName, LPCTSTR szValue)
{
	HRESULT hr;

	CComPtr< ISXMLElement > pElement;
	hr = CreateElement( pRoot, szName, &pElement );
	if ( FAILED( hr ) )
		return hr;

	hr = pElement->put_Value( CComBSTR( szValue ) );
	ATLASSERT( SUCCEEDED( hr ) );

	return hr;
}

HRESULT CreateAttribute(ISXMLElement* pRoot, LPCTSTR szName, LPCTSTR szValue)
{
	HRESULT hr;

	CComPtr< ISXMLAttributes > pRootAttributes;
	hr = pRoot->get_Attributes( &pRootAttributes );
	ATLASSERT( SUCCEEDED( hr ) );
	if ( FAILED( hr ) )
		return hr;

	CComPtr< ISXMLAttribute > pAttr;
	hr = pAttr.CoCreateInstance( CLSID_ShareazaXML );
	ATLASSERT( SUCCEEDED( hr ) );
	if ( FAILED( hr ) )
		return hr;

	hr = pAttr->put_Name( CComBSTR( szName ) );
	ATLASSERT( SUCCEEDED( hr ) );
	if ( FAILED( hr ) )
		return hr;

	hr = pAttr->put_Value( CComBSTR( szValue ) );
	ATLASSERT( SUCCEEDED( hr ) );
	if ( FAILED( hr ) )
		return hr;

	hr = pRootAttributes->Attach( pAttr );
	ATLASSERT( SUCCEEDED( hr ) );

	return hr;
}

class CMultipartFile
{
public:
	CMultipartFile()
	{
	}

	void Add(LPCTSTR szSrc, LPCTSTR szURI)
	{
		int nSrcLen = lstrlen( szSrc );
		int nDstLen = Base64EncodeGetRequiredLength( nSrcLen, 0 );
		CStringA sDest;
		Base64Encode( (const BYTE*)(LPCSTR)CT2CA( szSrc ), nSrcLen,
			sDest.GetBuffer( nDstLen + 1 ), &nDstLen, 0 );
		sDest.ReleaseBuffer();
		m_Parts.AddTail( sDest );
		m_URIs.AddTail( szURI );
	}

	void AddFile(LPCTSTR szFilename)
	{
		CAtlFile fileIn;
		ULONGLONG nSrcLen;
		if ( SUCCEEDED( fileIn.Create( szFilename, GENERIC_READ, 0, OPEN_EXISTING ) ) &&
			 SUCCEEDED( fileIn.GetSize( nSrcLen ) ) )
		{
			CAutoVectorPtr< BYTE > szSrc( new BYTE[ (DWORD)nSrcLen ] );
			if ( szSrc )
			{
				if ( SUCCEEDED( fileIn.Read( szSrc, (DWORD)nSrcLen ) ) )
				{
					int nDstLen = Base64EncodeGetRequiredLength( (DWORD)nSrcLen, 0 );
					CStringA sDest;
					Base64Encode( szSrc, (DWORD)nSrcLen,
						sDest.GetBuffer( nDstLen + 1 ), &nDstLen, 0 );
					sDest.ReleaseBuffer();
					m_Parts.AddTail( sDest );
					m_URIs.AddTail( PathFindFileName ( szFilename ) );
				}
			}
		}
	}

	void AddFolder(LPCTSTR szFolder)
	{
		CString sFolder( szFolder );
		sFolder.TrimRight( _T('\\') ) += _T("\\");

		WIN32_FIND_DATA wfa = {};
		HANDLE hFind = FindFirstFile ( sFolder + _T("*.*"), &wfa );
		if ( hFind != INVALID_HANDLE_VALUE )
		{
			do
			{
				if ( ! ( wfa.dwFileAttributes & ( FILE_ATTRIBUTE_DIRECTORY |
					FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM ) ) )
				{
					AddFile( sFolder + wfa.cFileName );
				}
			}
			while ( FindNextFile( hFind, &wfa ) );
		}
	}

	BOOL Save(LPCTSTR szFilename)
	{
		// Header
		const CHAR szBoundary[] = { "_Next_Part_" };
		CStringA strHeader;
		strHeader.Format( 
			"From: <Shareaza>\r\n"
			"Subject: File List\r\n"
			"MIME-Version: 1.0\r\n"
			"Content-Type: multipart/related;\r\n"
			"	boundary=\"%s\";\r\n"
			"	type=\"text/html\"\r\n"
			"\r\n"
			"This is a multi-part message in MIME format.\r\n"
			"\r\n",
			szBoundary );

		CAtlFile fileOut;
		if ( SUCCEEDED( fileOut.Create( szFilename, GENERIC_WRITE, 0, CREATE_ALWAYS ) ) &&
			 SUCCEEDED( fileOut.Write( strHeader, strHeader.GetLength() ) ) )
		{
			// Content
			for ( POSITION pos1 = m_Parts.GetHeadPosition(),
				pos2 = m_URIs.GetHeadPosition(); pos1 && pos2; )
			{
				CStringA& sPart = m_Parts.GetNext( pos1 );
				CString& sURI = m_URIs.GetNext( pos2 );

				TCHAR szType[ 64 ] = { _T("application/octet-stream") };
				DWORD dwType = REG_SZ, dwTypeSize = sizeof( szType );
				SHGetValue( HKEY_CLASSES_ROOT, PathFindExtension( sURI ),
					_T("Content Type"), &dwType, szType, &dwTypeSize );

				strHeader.Format (
					"--%s\r\n"
					"Content-Type: %s\r\n"
					"Content-Transfer-Encoding: base64\r\n"
					"Content-Location: http://A08274791746.com/%s\r\n"
					"\r\n",
					szBoundary, (LPCSTR)CT2A( szType ), (LPCSTR)CT2A( sURI ) );

				fileOut.Write( strHeader, strHeader.GetLength() );
				fileOut.Write( sPart, sPart.GetLength() );
				fileOut.Write( "\r\n\r\n", 4 );
			}

			// Footer
			fileOut.Write( "--", 2 );
			fileOut.Write( szBoundary, sizeof( szBoundary ) - 1 );
			fileOut.Write( "--\r\n\r\n", 6 );

			return TRUE;
		}
		return FALSE;
	}

protected:
	CAtlList< CStringA >	m_Parts;	// Base64 encoded parts
	CAtlList< CString >		m_URIs;		// Part's filenames
};

HRESULT CPlugin::Export(IGenericView* pGenericView, LONG nCount)
{
	HRESULT hr;

	HWND hWnd = NULL;
	m_pUserInterface->get_MainWindowHwnd( &hWnd );

	TCHAR szFilename[ MAX_PATH ] = {};
	OPENFILENAME ofn = {
		sizeof( OPENFILENAME ), hWnd, NULL,
		_T("MHT Files (*.mht)\0*.mht\0All Files (*.*)\0*.*\0\0"),
		NULL, 0, 1, szFilename, MAX_PATH, NULL, 0, NULL, NULL,
		OFN_EXPLORER | OFN_HIDEREADONLY | OFN_NOCHANGEDIR |
		OFN_OVERWRITEPROMPT | OFN_NOREADONLYRETURN, 0, 0, _T("mht")
	};
	if ( ! GetSaveFileName( &ofn ) )
		return S_OK;

	CComPtr< ISXMLElement > pRoot;
	hr = CreateElement( NULL, _T("collection"), &pRoot );
	if ( FAILED( hr ) )
		return hr;

	hr = CreateAttribute( pRoot, _T("xmlns"),
		_T("http://www.shareaza.com/schemas/Collection.xsd") );
	if ( FAILED( hr ) )
		return hr;

	CComPtr< ISXMLElement > pContents;
	hr = CreateElement( pRoot, _T("contents"), &pContents );
	if ( FAILED( hr ) )
		return hr;

	for ( LONG i = 0; i < nCount; ++i )
	{
		CComVariant pItem;
		hr = pGenericView->get_Item( CComVariant( i ), &pItem );
		if ( FAILED( hr ) || pItem.vt != VT_DISPATCH )
			continue;

		CComQIPtr< IShareazaFile > pShareazaFile( pItem.pdispVal );
		if ( ! pShareazaFile )
			continue;

		CComBSTR bstrSHA1, bstrTiger, bstrED2K, bstrMD5;
		hr = pShareazaFile->get_Hash( URN_SHA1,  ENCODING_BASE32, &bstrSHA1 );
		hr = pShareazaFile->get_Hash( URN_TIGER, ENCODING_BASE32, &bstrTiger );
		hr = pShareazaFile->get_Hash( URN_ED2K,  ENCODING_BASE16, &bstrED2K );
		hr = pShareazaFile->get_Hash( URN_MD5,   ENCODING_BASE16, &bstrMD5 );

		CComPtr< ISXMLElement > pFileRoot;
		hr = CreateElement( pContents, _T("file"), &pFileRoot );
		if ( FAILED( hr ) )
			return hr;

		CString str;
		str.Format( _T("%d"), i + 1 );
		hr = CreateElement( pFileRoot, _T("number"), str );
		if ( FAILED( hr ) )
			return hr;

		if ( bstrSHA1.Length() )
		{
			hr = CreateElement( pFileRoot, _T("id"), bstrSHA1 );
		}
		if ( bstrTiger.Length() )
		{
			hr = CreateElement( pFileRoot, _T("id"), bstrTiger );
		}
		if ( bstrED2K.Length() )
		{
			hr = CreateElement( pFileRoot, _T("id"), bstrED2K );
		}
		if ( bstrMD5.Length() )
		{
			hr = CreateElement( pFileRoot, _T("id"), bstrMD5 );
		}

		CComPtr< ISXMLElement > pDescription;
		hr = CreateElement( pFileRoot, _T("description"), &pDescription );
		if ( FAILED( hr ) )
			return hr;

		CComBSTR bstrName;
		hr = pShareazaFile->get_Name( &bstrName );
		hr = CreateElement( pDescription, _T("name"), bstrName );
		if ( FAILED( hr ) )
			return hr;

		ULONGLONG nSize = 0;
		hr = pShareazaFile->get_Size( &nSize );
		str.Format( _T("%I64i"), nSize );
		hr = CreateElement( pDescription, _T("size"), str );

		/*if ( pBestHit->m_pXML )
		{
			CXMLElement* pMetadata = pFileRoot->AddElement( _T("metadata") );
			pMetadata->AddAttribute( _T("xmlns:s"), pBestHit->m_sSchemaURI );					
			pMetadata->AddElement( pBestHit->m_pXML->Clone() );
		}*/
	}

	CComBSTR bstrXML;
	hr = pRoot->ToString( &bstrXML );
	ATLASSERT( SUCCEEDED( hr ) );
	if ( FAILED( hr ) )
		return hr;

	// ISXMLElement is just a wrapper so we need to
	// delete root element content explicit 
	hr = pRoot->Delete();
	ATLASSERT( SUCCEEDED( hr ) );

	CString strXML;
	strXML.Format(
		_T("<?xml version=\"1.0\"?>\r\n")
		//_T("<?xml-stylesheet type=\"text/xsl\" href=\"http://A08274791746.com/template.xslt\"?>")
		_T("%s"),
		bstrXML );

	CMultipartFile fileExport;
	fileExport.Add( strXML, _T("index.xml") );
	//fileExport.AddFolder( Settings.General.Path + _T("\\File List Template\\") );
	if ( fileExport.Save( ofn.lpstrFile ) )
	{
		ShellExecute ( hWnd, NULL, ofn.lpstrFile, NULL, NULL, SW_SHOWNORMAL);
	}

	return S_OK;
}

// IGeneralPlugin

STDMETHODIMP CPlugin::SetApplication( 
	/* [in] */ IApplication __RPC_FAR *pApplication)
{
	if ( ! pApplication )
		return E_POINTER;

	m_pApplication = pApplication;
	return m_pApplication->get_UserInterface( &m_pUserInterface );
}

STDMETHODIMP CPlugin::QueryCapabilities(
	/* [in] */ DWORD __RPC_FAR *pnCaps)
{
	if ( ! pnCaps )
		return E_POINTER;

	return S_OK;
}

STDMETHODIMP CPlugin::Configure()
{
	return S_OK;
}

STDMETHODIMP CPlugin::OnSkinChanged()
{
	// Recreate lost menu items
	return InsertCommands();
}

// ICommandPlugin

STDMETHODIMP CPlugin::RegisterCommands()
{
	if ( ! m_pUserInterface )
		return E_UNEXPECTED;

	HRESULT hr = m_pUserInterface->RegisterCommand( CComBSTR( L"SearchExportPlugin_Check" ),
		LoadIcon( _AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE( IDI_ICON ) ),
		&m_nCmdCheck );
	if ( SUCCEEDED( hr ) )
	{
		return S_OK;
	}

	return E_FAIL;
}

STDMETHODIMP CPlugin::InsertCommands()
{
	if ( ! m_pUserInterface )
		return E_UNEXPECTED;

	CComPtr< ISMenu > pSearchMenu;
	if ( SUCCEEDED( m_pUserInterface->GetMenu( CComBSTR( L"CSearchWnd" ),
		VARIANT_FALSE, &pSearchMenu ) ) && pSearchMenu )
	{
		InsertCommand( pSearchMenu, 13, m_nCmdCheck, SearchExport_CHECK );
	}

	return E_FAIL;
}

STDMETHODIMP CPlugin::OnUpdate( 
    /* [in] */ UINT nCommandID,
    /* [out][in] */ TRISTATE __RPC_FAR *pbVisible,
    /* [out][in] */ TRISTATE __RPC_FAR *pbEnabled,
    /* [out][in] */ TRISTATE __RPC_FAR *pbChecked)
{
	if ( ! pbVisible || ! pbEnabled || ! pbChecked )
		return E_POINTER;

	if ( ! m_pUserInterface )
		return E_UNEXPECTED;

	if ( nCommandID == m_nCmdCheck )
	{
		*pbEnabled = TRI_FALSE;
		*pbVisible = TRI_TRUE;
		*pbChecked = TRI_UNKNOWN;

		CComPtr< IGenericView > pGenericView;
		HRESULT hr = m_pUserInterface->get_ActiveView( &pGenericView );
		if ( SUCCEEDED( hr ) && pGenericView )
		{
			LONG nCount = 0;
			hr = pGenericView->get_Count( &nCount );
			if ( SUCCEEDED( hr ) && nCount )
			{
				*pbEnabled = TRI_TRUE;
			}
		}
		return S_OK;
	}

	return S_FALSE;
}

STDMETHODIMP CPlugin::OnCommand( 
	/* [in] */ UINT nCommandID)
{
	ATLTRACE( _T("CPlugin::OnCommand( %d )\n"), nCommandID );

	if ( ! m_pUserInterface )
	{
		ATLTRACE( _T("CPlugin::OnCommand : No user interface.\n") );
		return E_UNEXPECTED;
	}

	if ( nCommandID == m_nCmdCheck )
	{
		CComPtr< IGenericView > pGenericView;
		HRESULT hr = m_pUserInterface->get_ActiveView( &pGenericView );
		if ( SUCCEEDED( hr ) && pGenericView )
		{
			LONG nCount = 0;
			hr = pGenericView->get_Count( &nCount );
			if ( SUCCEEDED( hr ) && nCount )
			{
				return Export( pGenericView, nCount );
			}
			else
				ATLTRACE( _T("CPlugin::OnCommand() : No files selected: 0x%08x\n"), hr );
		}
		else
			ATLTRACE( _T("CPlugin::OnCommand() : Active view get error: 0x%08x\n"), hr );
	}

	return S_FALSE;
}
