//
// PageFileMetadata.cpp
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
#include "Library.h"
#include "SharedFile.h"
#include "PageFileMetadata.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CFileMetadataPage, CFilePropertiesPage)

BEGIN_MESSAGE_MAP(CFileMetadataPage, CFilePropertiesPage)
	ON_CBN_SELCHANGE(IDC_SCHEMAS, &CFileMetadataPage::OnSelChangeSchemas)
	ON_CBN_CLOSEUP(IDC_SCHEMAS, &CFileMetadataPage::OnCloseUpSchemas)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFileMetadataPage property page

CFileMetadataPage::CFileMetadataPage()
	: CFilePropertiesPage	( CFileMetadataPage::IDD )
	, m_pXML				( NULL )
{
}

void CFileMetadataPage::DoDataExchange(CDataExchange* pDX)
{
	CFilePropertiesPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_SCHEMAS, m_wndSchemas);
}

/////////////////////////////////////////////////////////////////////////////
// CFileMetadataPage message handlers

BOOL CFileMetadataPage::OnInitDialog()
{
	CFilePropertiesPage::OnInitDialog();

	CQuickLock oLock( Library.m_pSection );
	CLibraryListPtr pFiles( GetList() );

	CRect rcClient, rcCombo;
	GetClientRect( &rcClient );

	m_wndSchemas.GetWindowRect( &rcCombo );
	ScreenToClient( &rcCombo );
	rcCombo.top = rcCombo.bottom + 8;
	rcCombo.bottom = rcClient.bottom - 8;

	m_wndData.Create( WS_CHILD|WS_VISIBLE|WS_BORDER|WS_TABSTOP, rcCombo, this, IDC_METADATA );
	m_wndSchemas.m_sNoSchemaText = LoadString( IDS_SEARCH_NO_METADATA );

	BOOL bCollection = FALSE;
	CSchemaPtr pSchema = NULL;

	for ( POSITION pos = pFiles->GetHeadPosition(); pos; )
	{
		if ( CLibraryFile* pFile = pFiles->GetNextFile( pos ) )
		{
			CSchemaPtr pThisSchema = pFile->m_pSchema;

			if ( pThisSchema != NULL && pThisSchema->m_nType == CSchema::stFolder ) bCollection = TRUE;

			if ( pSchema == NULL )
			{
				pSchema = pThisSchema;
			}
			else if ( pSchema != pThisSchema )
			{
				pSchema = NULL;
				break;
			}
		}
	}

	m_wndSchemas.Load( pSchema ? pSchema->GetURI() : _T(""), bCollection ? CSchema::stAny : CSchema::stFile );

	if ( pSchema != NULL )
	{
		m_pSchemaContainer.Attach( pSchema->Instantiate( TRUE ) );
		m_pXML = m_pSchemaContainer->AddElement( pSchema->m_sSingular );

		for ( POSITION pos1 = pFiles->GetHeadPosition(); pos1; )
		{
			if ( CLibraryFile* pFile = pFiles->GetNextFile( pos1 ) )
			{
				if ( pFile->m_pMetadata != NULL && pSchema->Equals( pFile->m_pSchema ) )
				{
					for ( POSITION pos2 = pSchema->GetMemberIterator(); pos2; )
					{
						CSchemaMemberPtr pMember = pSchema->GetNextMember( pos2 );
						CString strOld = pMember->GetValueFrom( m_pXML, NO_VALUE, FALSE, TRUE );
						CString strNew = pMember->GetValueFrom( pFile->m_pMetadata, _T(""), FALSE );
						if ( strOld != MULTI_VALUE )
						{
							if ( strOld == NO_VALUE )
							{
								m_pXML->AddAttribute( pMember->m_sName, strNew );
							}
							else if ( strOld != strNew )
							{
								m_wndData.AddItem( pMember, pMember->GetValueFrom( m_pXML, _T(""), TRUE, TRUE ) );
								m_wndData.AddItem( pMember, pMember->GetValueFrom( pFile->m_pMetadata, _T(""), TRUE ) );
								m_pXML->AddAttribute( pMember->m_sName, MULTI_VALUE );
							}
						}
						else
							m_wndData.AddItem( pMember, pMember->GetValueFrom( pFile->m_pMetadata, _T(""), TRUE ) );
					}
				}
			}
		}
	}

	OnSelChangeSchemas();

	m_wndData.UpdateData( m_pXML, FALSE );

	return TRUE;
}

void CFileMetadataPage::OnSelChangeSchemas()
{
	CSchemaPtr pSchema = m_wndSchemas.GetSelected();
	CString strSelectedURI = m_wndData.GetSchemaURI();

	if ( pSchema && ! pSchema->CheckURI( strSelectedURI ) )
	{
		if ( strSelectedURI.IsEmpty() )
		{
			m_wndData.SetSchema( pSchema );
			return;
		}

		CString strBody( ::LoadHTML( GetModuleHandle( NULL ), IDR_XML_SCHEMA_MAPS ) );

		if ( CXMLElement* pXML = CXMLElement::FromString( strBody, TRUE ) )
		{
			if ( pXML->IsNamed( L"schemaMappings" ) )
			{
				for ( POSITION pos = pXML->GetElementIterator() ; pos ; )
				{
					CXMLElement* pMapping = pXML->GetNextElement( pos );
					if ( pMapping && pMapping->IsNamed( L"schemaMapping" ) )
					{
						CXMLAttribute* pSourceURI = pMapping->GetAttribute( L"sourceURI" );
						if ( pSourceURI && pSourceURI->GetValue() == m_wndData.GetSchemaURI() )
						{
							// Add attributes which correspond to other schema
							// We don't need to delete the old ones because, after
							// submitting new data, they will be ignored.
							// It will also allow to save the old ones if we switch schema back.
							AddCrossAttributes( pMapping, pSchema->GetURI() );
							break;
						}
					}
				}
			}
			delete pXML;
		}

		m_wndData.SetSchema( pSchema );
		if ( m_pXML )
		{
			// Change schema of data
			m_pXML->SetName( pSchema->m_sSingular );
			m_wndData.UpdateData( m_pXML, FALSE );
		}
	}
	else
		m_wndData.SetSchema( pSchema );
}

void CFileMetadataPage::AddCrossAttributes(CXMLElement* pXML, LPCTSTR pszTargetURI)
{
	if ( pXML == NULL ) return;
	CXMLElement* pTargetURI = NULL;

	for ( POSITION pos = pXML->GetElementIterator() ; pos ; )
	{
		pTargetURI = pXML->GetNextElement( pos );
		if ( pTargetURI && pTargetURI->IsNamed( L"target" ) )
		{
			CXMLAttribute* pURI = pTargetURI->GetAttribute( L"uri" );
			if ( pURI && _tcscmp( pURI->GetValue(), pszTargetURI ) == 0 )
				break;
			else
				pTargetURI = NULL;
		}
		else
			pTargetURI = NULL;
	}

	if ( pTargetURI == NULL ) return;

	for ( POSITION pos = pTargetURI->GetElementIterator() ; pos ; )
	{
		CXMLElement* pAttribute = pTargetURI->GetNextElement( pos );
		{
			if ( pAttribute && pAttribute->IsNamed( L"attribute" ) )
			{
				CXMLAttribute* pFrom = pAttribute->GetAttribute( L"from" );
				CXMLAttribute* pTo = pAttribute->GetAttribute( L"to" );
				if ( pFrom && pTo )
				{
					CString strFrom = pFrom->GetValue();
					CString strTo = pTo->GetValue();
					if ( strFrom.IsEmpty() || strTo.IsEmpty() ) continue;

					CString strValue = m_pXML->GetAttributeValue( strFrom );
					if ( strValue.GetLength() && strValue != MULTI_VALUE )
						m_pXML->AddAttribute( strTo, strValue );
				}
			}
		}
	}
}

void CFileMetadataPage::OnCloseUpSchemas()
{
	if ( CSchemaPtr pSchema = m_wndSchemas.GetSelected() ) PostMessage( WM_KEYDOWN, VK_TAB );
}

void CFileMetadataPage::OnOK()
{
	UpdateData();

	DWORD nModified = UpdateFileData( FALSE );

	if ( nModified >= 10 )
	{
		CString strMessage;
		strMessage.Format( LoadString( IDS_LIBRARY_METADATA_MANY ), nModified );
		if ( AfxMessageBox( strMessage, MB_YESNO|MB_ICONQUESTION ) != IDYES ) return;
	}

	CProgressDialog dlgProgress( LoadString( ID_LIBRARY_REFRESH_METADATA ) + _T("...") );

	UpdateFileData( TRUE );

	CFilePropertiesPage::OnOK();
}

DWORD CFileMetadataPage::UpdateFileData(BOOL bRealSave)
{
	CQuickLock oLock( Library.m_pSection );

	DWORD nModified = 0;
	CLibraryListPtr pFiles( GetList() );
	if ( pFiles )
	{
		if ( CSchemaPtr pSchema = m_wndSchemas.GetSelected() )
		{
			for ( POSITION pos1 = pFiles->GetHeadPosition() ; pos1 ; )
			{
				if ( CLibraryFile* pFile = pFiles->GetNextFile( pos1 ) )
				{
					CXMLElement* pContainer	= pSchema->Instantiate( TRUE );
					if ( pContainer )
					{
						CXMLElement* pXML = NULL;

						if ( pFile->m_pMetadata != NULL )
						{
							pXML = pContainer->AddElement( pFile->m_pMetadata->Clone() );
							// Change schema
							pXML->SetName( pSchema->m_sSingular );
						}
						else
							pXML = pContainer->AddElement( pSchema->m_sSingular );

						// Save changed data to pXML
						nModified += m_wndData.UpdateData( pXML, TRUE, bRealSave );

						if ( bRealSave )
							pFile->SetMetadata( pContainer );

						delete pContainer;
					}
				}
			}
		}
		else
		{
			for ( POSITION pos1 = pFiles->GetHeadPosition() ; pos1 ; )
			{
				if ( CLibraryFile* pFile = pFiles->GetNextFile( pos1 ) )
				{
					if ( pFile->m_pMetadata )
					{
						++ nModified;

						if ( bRealSave )
							pFile->ClearMetadata();
					}
				}
			}
		}

		if ( bRealSave && nModified )
			Library.Update();
	}
	return nModified;
}
