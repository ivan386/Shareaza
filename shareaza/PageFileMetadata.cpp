//
// PageFileMetadata.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "SchemaCache.h"
#include "Schema.h"
#include "XML.h"
#include "PageFileMetadata.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CFileMetadataPage, CFilePropertiesPage)

BEGIN_MESSAGE_MAP(CFileMetadataPage, CFilePropertiesPage)
	//{{AFX_MSG_MAP(CFileMetadataPage)
	ON_CBN_SELCHANGE(IDC_SCHEMAS, OnSelChangeSchemas)
	ON_CBN_CLOSEUP(IDC_SCHEMAS, OnCloseUpSchemas)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFileMetadataPage property page

CFileMetadataPage::CFileMetadataPage() : CFilePropertiesPage(CFileMetadataPage::IDD),
m_pXML(NULL), m_pSchemaContainer(NULL)
{
}

CFileMetadataPage::~CFileMetadataPage()
{
	if ( m_pSchemaContainer )
		delete m_pSchemaContainer;
}

void CFileMetadataPage::DoDataExchange(CDataExchange* pDX)
{
	CFilePropertiesPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFileMetadataPage)
	DDX_Control(pDX, IDC_SCHEMAS, m_wndSchemas);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CFileMetadataPage message handlers

BOOL CFileMetadataPage::OnInitDialog()
{
	CFilePropertiesPage::OnInitDialog();

	CLibraryList* pFiles = GetList();

	CRect rcClient, rcCombo;
	CString strText;
	GetClientRect( &rcClient );

	m_wndSchemas.GetWindowRect( &rcCombo );
	ScreenToClient( &rcCombo );
	rcCombo.top = rcCombo.bottom + 8;
	rcCombo.bottom = rcClient.bottom - 8;

	m_wndData.Create( WS_CHILD|WS_VISIBLE|WS_BORDER|WS_TABSTOP, rcCombo, this, IDC_METADATA );
	LoadString ( strText, IDS_SEARCH_NO_METADATA );
	m_wndSchemas.m_sNoSchemaText = strText;

	BOOL bCollection = FALSE;
	CSchema* pSchema = NULL;

	{
		CQuickLock oLock( Library.m_pSection );

		for ( POSITION pos = pFiles->GetIterator() ; pos ; )
		{
			if ( CLibraryFile* pFile = pFiles->GetNextFile( pos ) )
			{
				CSchema* pThisSchema = pFile->m_pSchema;

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
	}

	m_wndSchemas.Load( pSchema != NULL ? pSchema->GetURI() : _T(""), bCollection ? -1 : 0 );
	OnSelChangeSchemas();

	if ( pSchema != NULL )
	{
		m_pSchemaContainer = pSchema->Instantiate( TRUE );
		m_pXML = m_pSchemaContainer->AddElement( pSchema->m_sSingular );

		{
			CQuickLock oLock( Library.m_pSection );

			for ( POSITION pos1 = pFiles->GetIterator() ; pos1 ; )
			{
				if ( CLibraryFile* pFile = pFiles->GetNextFile( pos1 ) )
				{
					if ( pFile->m_pMetadata != NULL && pSchema->Equals( pFile->m_pSchema ) )
					{
						for ( POSITION pos2 = pSchema->GetMemberIterator() ; pos2 ; )
						{
							CSchemaMember* pMember = pSchema->GetNextMember( pos2 );
							CString strOld = pMember->GetValueFrom( m_pXML, NO_VALUE, FALSE, TRUE );
							if ( strOld != MULTI_VALUE )
							{
								CString strNew = pMember->GetValueFrom( pFile->m_pMetadata, NO_VALUE );
								if ( strOld == NO_VALUE && strNew != NO_VALUE )
								{
									m_pXML->AddAttribute( pMember->m_sName, strNew );
								}
								else if ( strOld != strNew )
								{
									m_pXML->AddAttribute( pMember->m_sName, MULTI_VALUE );
								}
							}
						}
					}
				}
			}
		}

		m_wndData.UpdateData( m_pXML, FALSE );
	}

	return TRUE;
}

void CFileMetadataPage::OnSelChangeSchemas()
{
	CSchema* pSchema = m_wndSchemas.GetSelected();
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
	if ( CSchema* pSchema = m_wndSchemas.GetSelected() ) PostMessage( WM_KEYDOWN, VK_TAB );
}

void CFileMetadataPage::OnOK()
{
	CLibraryList* pFiles = GetList();
	if ( pFiles == NULL ) return;

	if ( pFiles->GetCount() >= 10 )
	{
		CString strFormat, strMessage;
		LoadString( strFormat, IDS_LIBRARY_METADATA_MANY );
		strMessage.Format( strFormat, pFiles->GetCount() );
		if ( AfxMessageBox( strMessage, MB_YESNO|MB_ICONQUESTION ) != IDYES ) return;
	}

	if ( CSchema* pSchema = m_wndSchemas.GetSelected() )
	{
		CQuickLock oLock( Library.m_pSection );

		for ( POSITION pos1 = pFiles->GetIterator() ; pos1 ; )
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
					m_wndData.UpdateData( pXML, TRUE );
					pFile->SetMetadata( pContainer );
					delete pContainer;
				}
			}
		}
	}
	else
	{
		CQuickLock oLock( Library.m_pSection );

		for ( POSITION pos1 = pFiles->GetIterator() ; pos1 ; )
		{
			if ( CLibraryFile* pFile = pFiles->GetNextFile( pos1 ) )
			{
				pFile->SetMetadata( NULL );
			}
		}

		Library.Update();
	}

	CFilePropertiesPage::OnOK();
}
