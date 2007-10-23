//
// LibraryBuilderPlugins.cpp
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

CLibraryBuilderPlugins::CLibraryBuilderPlugins()
{
}

CLibraryBuilderPlugins::~CLibraryBuilderPlugins()
{
	Cleanup();
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderPlugins extract

BOOL CLibraryBuilderPlugins::ExtractMetadata(DWORD nIndex, CString& strPath, HANDLE hFile)
{
	CString strType;
	
	int nExtPos = strPath.ReverseFind( '.' );
	if ( nExtPos != -1 ) strType = strPath.Mid( nExtPos );
	
	ToLower( strType );
	
	ILibraryBuilderPlugin* pPlugin = NULL;
	
	if ( m_pMap.Lookup( strType, pPlugin ) )
	{
		if ( pPlugin == NULL ) return FALSE;
	}
	else
	{
		pPlugin = LoadPlugin( strType );
		if ( pPlugin == NULL ) return FALSE;
	}
	
	CXMLElement* pXML	= new CXMLElement();
	ISXMLElement* ppXML	= (ISXMLElement*)CXMLCOM::Wrap( pXML, IID_ISXMLElement );
	
	BSTR bsFile = strPath.AllocSysString();
	
	HRESULT hResult = pPlugin->Process( hFile, bsFile, ppXML );
	
	SysFreeString( bsFile );
	
	ppXML->Release();
	
	BOOL bSuccess = FALSE;
	
	if ( SUCCEEDED( hResult ) )
	{
		if ( CXMLElement* pOuter = pXML->GetFirstElement() )
		{
			CXMLElement* pInner		= pOuter->GetFirstElement();
			CString strSchemaURI	= pOuter->GetAttributeValue( CXMLAttribute::schemaName );
			
			if ( pInner && strSchemaURI.GetLength() )
			{
				pInner = pInner->Detach();
				bSuccess = CLibraryBuilder::SubmitMetadata( nIndex, strSchemaURI, pInner );
			}
		}
	}
	else if ( hResult == E_UNEXPECTED )
	{
		bSuccess = CLibraryBuilder::SubmitCorrupted( nIndex );
	}
	
	delete pXML;
	
	return bSuccess;
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderPlugins cleanup

void CLibraryBuilderPlugins::Cleanup()
{
	for ( POSITION pos = m_pMap.GetStartPosition() ; pos ; )
	{
		ILibraryBuilderPlugin* pPlugin = NULL;
		CString strType;
		
		m_pMap.GetNextAssoc( pos, strType, pPlugin );
		if ( pPlugin ) pPlugin->Release();
	}
	
	m_pMap.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// CLibraryBuilderPlugins load plugin

ILibraryBuilderPlugin* CLibraryBuilderPlugins::LoadPlugin(LPCTSTR pszType)
{
	CLSID pCLSID;
	
	if ( ! Plugins.LookupCLSID( _T("LibraryBuilder"), pszType, pCLSID ) )
	{
		m_pMap.SetAt( pszType, NULL );
		return NULL;
	}
	
	ILibraryBuilderPlugin* pPlugin = NULL;
	
	CoCreateInstance( pCLSID, NULL, CLSCTX_ALL,
		IID_ILibraryBuilderPlugin, (void**)&pPlugin );
	
	m_pMap.SetAt( pszType, pPlugin );
	
	return pPlugin;
}
