//
// DlgCollectionExport.cpp
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
#include "Library.h"
#include "LibraryFolders.h"
#include "AlbumFolder.h"
#include "SharedFile.h"
#include "Schema.h"
#include "XML.h"

#include "SHA.h"
#include "MD5.h"
#include "ED2K.h"
#include "TigerTree.h"

#include "DlgCollectionExport.h"

IMPLEMENT_DYNAMIC(CCollectionExportDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CCollectionExportDlg, CSkinDialog)
END_MESSAGE_MAP()


CCollectionExportDlg::CCollectionExportDlg(CAlbumFolder* pFolder, CWnd* pParent) : CSkinDialog(CCollectionExportDlg::IDD, pParent)
{
	m_pFolder = pFolder;
}

CCollectionExportDlg::~CCollectionExportDlg()
{
}

void CCollectionExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange( pDX );
}

BOOL CCollectionExportDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();
	
	SkinMe( NULL );
	
	return TRUE;
}

void CCollectionExportDlg::OnOK()
{
	CString strPath = BrowseForFolder();
	if ( strPath.IsEmpty() ) return;
	
	CSingleLock pLock( &Library.m_pSection, TRUE );
	
	if ( LibraryFolders.CheckAlbum( m_pFolder ) )
	{
		CXMLElement* pXML = CreateXML();
		CString strXML = pXML->ToString( TRUE, TRUE );
		delete pXML;
		
		CString strFile = strPath + _T("\\Collection.xml");
		CFile pFile;
		
		if ( pFile.Open( strFile, CFile::modeWrite|CFile::modeCreate ) )
		{
			USES_CONVERSION;
			LPCSTR pszXML = T2CA( (LPCTSTR)strXML );
			pFile.Write( pszXML, strlen(pszXML) );
			pFile.Close();
			CSkinDialog::OnOK();
		}
		else
		{
			pLock.Unlock();
			AfxMessageBox( _T("TODO: Can't write to ") + strFile );
		}
	}
	else
	{
		pLock.Unlock();
		AfxMessageBox( _T("TODO: Folder disappeared.") );
	}
}

CString CCollectionExportDlg::BrowseForFolder()
{
	TCHAR szPath[MAX_PATH];
	LPITEMIDLIST pPath;
	LPMALLOC pMalloc;
	BROWSEINFO pBI;
	CString str;
	
	ZeroMemory( &pBI, sizeof(pBI) );
	pBI.hwndOwner		= GetSafeHwnd();
	pBI.pszDisplayName	= szPath;
	pBI.lpszTitle		= _T("Choose output folder:");
	pBI.ulFlags			= BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	
	pPath = SHBrowseForFolder( &pBI );
	
	if ( pPath == NULL ) return str;
	
	SHGetPathFromIDList( pPath, szPath );
	SHGetMalloc( &pMalloc );
	pMalloc->Free( pPath );
	
	str = szPath;
	return str;
}

CXMLElement* CCollectionExportDlg::CreateXML()
{
	CXMLElement* pRoot = new CXMLElement( NULL, _T("collection") );
	pRoot->AddAttribute( _T("xmlns"), _T("http://www.shareaza.com/schemas/Collection.xsd") );
	
	CXMLElement* pProperties = pRoot->AddElement( _T("properties") );
	
	pProperties->AddElement( _T("title") )->SetValue( m_pFolder->m_sName );
	
	if ( m_pFolder->m_pXML != NULL && m_pFolder->m_sSchemaURI.GetLength() > 0 )
	{
		CXMLElement* pMeta = pProperties->AddElement( _T("metadata") );
		pMeta->AddAttribute( _T("xmlns:s"), m_pFolder->m_sSchemaURI );
		pMeta->AddElement( CopyMetadata( m_pFolder->m_pXML ) );
	}
	
	CXMLElement* pContents = pRoot->AddElement( _T("contents") );
	
	for ( POSITION pos = m_pFolder->GetFileIterator() ; pos ; )
	{
		CLibraryFile* pFile = m_pFolder->GetNextFile( pos );
		if ( pFile == NULL ) continue;
		
		CXMLElement* pFileRoot = pContents->AddElement( _T("file") );
		
		if ( pFile->m_oSHA1.IsValid() && pFile->m_oTiger.IsValid() )
		{
			pFileRoot->AddElement( _T("id") )->SetValue(
				_T("urn:bitprint:") + pFile->m_oSHA1.ToString() + '.' +
				pFile->m_oTiger.ToString() );
		}
		else if ( pFile->m_oSHA1.IsValid() )
		{
			pFileRoot->AddElement( _T("id") )->SetValue( pFile->m_oSHA1.ToURN() );
		}
		else if ( pFile->m_oTiger.IsValid() )
		{
			pFileRoot->AddElement( _T("id") )->SetValue( pFile->m_oTiger.ToURN() );
		}
		if ( pFile->m_oMD5.IsValid() ) pFileRoot->AddElement( _T("id") )->SetValue( pFile->m_oMD5.ToURN() );
		if ( pFile->m_oED2K.IsValid() ) pFileRoot->AddElement( _T("id") )->SetValue( pFile->m_oED2K.ToURN() );
		
		CXMLElement* pDescription = pFileRoot->AddElement( _T("description") );
		pDescription->AddElement( _T("name") )->SetValue( pFile->m_sName );
		CString str;
		str.Format( _T("%I64i"), pFile->GetSize() );
		pDescription->AddElement( _T("size") )->SetValue( str );
		
		if ( pFile->m_pMetadata != NULL && pFile->m_bMetadataAuto == FALSE && pFile->m_pSchema != NULL )
		{
			CXMLElement* pMetadata = pFileRoot->AddElement( _T("metadata") );
			pMetadata->AddAttribute( _T("xmlns:s"), pFile->m_pSchema->m_sURI );
			pMetadata->AddElement( CopyMetadata( pFile->m_pMetadata ) );
		}
	}
	
	return pRoot;
}

CXMLElement* CCollectionExportDlg::CopyMetadata(CXMLElement* pMetadata)
{
	pMetadata = pMetadata->Clone();
	pMetadata->SetName( _T("s:") + pMetadata->GetName() );
	
	for ( POSITION pos = pMetadata->GetElementIterator() ; pos ; )
	{
		CXMLNode* pNode = pMetadata->GetNextElement( pos );
		pNode->SetName( _T("s:") + pNode->GetName() );
	}
	
	for ( pos = pMetadata->GetAttributeIterator() ; pos ; )
	{
		CXMLNode* pNode = pMetadata->GetNextAttribute( pos );
		pNode->SetName( _T("s:") + pNode->GetName() );
	}
	
	return pMetadata;
}
