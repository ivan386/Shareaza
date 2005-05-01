//
// PageFileMetadata.cpp
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
#include "Shareaza.h"
#include "Library.h"
#include "SharedFile.h"
#include "SchemaCache.h"
#include "Schema.h"
#include "XML.h"
#include "SHA.h"
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

CFileMetadataPage::CFileMetadataPage() : CFilePropertiesPage(CFileMetadataPage::IDD)
{
	//{{AFX_DATA_INIT(CFileMetadataPage)
	//}}AFX_DATA_INIT
}

CFileMetadataPage::~CFileMetadataPage()
{
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

	m_wndSchemas.Load( pSchema != NULL ? pSchema->m_sURI : _T(""), bCollection ? -1 : 0 );
	OnSelChangeSchemas();

	if ( pSchema != NULL )
	{
		CXMLElement* pContainer	= pSchema->Instantiate( TRUE );
		CXMLElement* pXML		= pContainer->AddElement( pSchema->m_sSingular );

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
							CString strOld = pMember->GetValueFrom( pXML, _T("(~ns~)") );
							CString strNew = pMember->GetValueFrom( pFile->m_pMetadata /* , _T("(~ns~)") */ );

							if ( strNew != _T("(~ns~)") && strOld != _T("(~mt~)") )
							{
								if ( strOld == _T("(~ns~)") )
								{
									pXML->AddAttribute( pMember->m_sName, strNew );
								}
								else if ( strOld != strNew )
								{
									pXML->AddAttribute( pMember->m_sName, _T("(~mt~)") );
								}
							}
						}
					}
				}
			}
		}

		m_wndData.UpdateData( pXML, FALSE );
		delete pContainer;
	}

	return TRUE;
}

void CFileMetadataPage::OnSelChangeSchemas()
{
	CSchema* pSchema = m_wndSchemas.GetSelected();
	m_wndData.SetSchema( pSchema );
}

void CFileMetadataPage::OnCloseUpSchemas()
{
	if ( CSchema* pSchema = m_wndSchemas.GetSelected() ) PostMessage( WM_KEYDOWN, VK_TAB );
}

BOOL CFileMetadataPage::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB )
	{
		if ( m_wndData.OnTab() ) return TRUE;
	}

	return CPropertyPage::PreTranslateMessage( pMsg );
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
				if ( pSchema->Equals( pFile->m_pSchema ) && pFile->m_pMetadata != NULL )
				{
					CXMLElement* pContainer	= pSchema->Instantiate( TRUE );
					CXMLElement* pXML		= pContainer->AddElement( pFile->m_pMetadata->Clone() );
					m_wndData.UpdateData( pXML, TRUE );
					if ( pContainer ) pFile->SetMetadata( pContainer );
					delete pContainer;
				}
				else
				{
					CXMLElement* pContainer	= pSchema->Instantiate( TRUE );
					CXMLElement* pXML		= pContainer->AddElement( pSchema->m_sSingular );
					m_wndData.UpdateData( pXML, TRUE );
					if ( pContainer ) pFile->SetMetadata( pContainer );
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
