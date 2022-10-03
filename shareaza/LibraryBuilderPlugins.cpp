//
// LibraryBuilderPlugins.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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
#include "LibraryBuilder.h"
#include "LibraryBuilderPlugins.h"
#include "Plugins.h"
#include "XML.h"
#include "XMLCOM.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderPlugins construction

HRESULT CLibraryBuilderPlugins::SafeProcess(ILibraryBuilderPlugin* pPlugin, BSTR szPath, ISXMLElement* pElement)
{
	__try
	{
		return pPlugin->Process( szPath, pElement );
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		return RPC_E_SERVERFAULT;
	}
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderPlugins extract

bool CLibraryBuilderPlugins::ExtractPluginMetadata(DWORD nIndex, const CString& strPath)
{
	LPCTSTR szType = PathFindExtension( strPath );

	for ( int i = 0; i < 2; ++i )
	{
		CComQIPtr< ILibraryBuilderPlugin > pPlugin( Plugins.GetPlugin( _T( "LibraryBuilder" ), szType ) );
		if ( ! pPlugin )
			break;

		auto_ptr< CXMLElement > pXML( new CXMLElement() );
		CComPtr< ISXMLElement > pISXMLElement;
		pISXMLElement.Attach(
			(ISXMLElement*)CXMLCOM::Wrap( pXML.get(), IID_ISXMLElement ) );
		HRESULT hr = SafeProcess( pPlugin, CComBSTR( strPath ), pISXMLElement );
		if ( SUCCEEDED( hr ) )
		{
			if ( CXMLElement* pOuter = pXML->GetFirstElement() )
			{
				CXMLElement* pInner		= pOuter->GetFirstElement();
				CString strSchemaURI	= pOuter->GetAttributeValue( CXMLAttribute::schemaName );

				if ( pInner && strSchemaURI.GetLength() )
				{
					pInner = pInner->Detach();
					return LibraryBuilder.SubmitMetadata( nIndex, strSchemaURI, pInner ) != 0;
				}
			}
		}
		else if ( hr == E_UNEXPECTED )
		{
			return LibraryBuilder.SubmitCorrupted( nIndex );
		}
		else if ( SERVERLOST( hr ) )
		{
			Plugins.ReloadPlugin( _T("LibraryBuilder"), szType );

			pPlugin.Release();

			// Try again
			continue;
		}

		break;
	}

	return false;
}
