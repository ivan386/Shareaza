//
// PageProfileContact.cpp
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
#include "GProfile.h"
#include "XML.h"
#include "PageProfileContact.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CContactProfilePage, CSettingsPage)

BEGIN_MESSAGE_MAP(CContactProfilePage, CSettingsPage)
	//{{AFX_MSG_MAP(CContactProfilePage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CContactProfilePage property page

CContactProfilePage::CContactProfilePage() : CSettingsPage( CContactProfilePage::IDD )
{
	//{{AFX_DATA_INIT(CContactProfilePage)
	//}}AFX_DATA_INIT
}

CContactProfilePage::~CContactProfilePage()
{
}

void CContactProfilePage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CContactProfilePage)
	DDX_Text(pDX, IDC_PROFILE_EMAIL, m_sEmail);
	DDX_Text(pDX, IDC_PROFILE_AOL, m_sAOL);
	DDX_Text(pDX, IDC_PROFILE_ICQ, m_sICQ);
	DDX_Text(pDX, IDC_PROFILE_YAHOO, m_sYahoo);
	DDX_Text(pDX, IDC_PROFILE_MSN, m_sMSN);
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_PROFILE_JABBER, m_sJabber);
}

/////////////////////////////////////////////////////////////////////////////
// CContactProfilePage message handlers

BOOL CContactProfilePage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();

	if ( CXMLElement* pContacts = MyProfile.GetXML( _T("contacts") ) )
	{
		for ( POSITION pos1 = pContacts->GetElementIterator() ; pos1 ; )
		{
			CXMLElement* pGroup = pContacts->GetNextElement( pos1 );

			if ( pGroup->IsNamed( _T("group") ) )
			{
				CString strGroup = pGroup->GetAttributeValue( _T("class") );

				if ( CXMLElement* pAddress = pGroup->GetElementByName( _T("address") ) )
				{
					CString strAddress = pAddress->GetAttributeValue( _T("content") );

					if ( strGroup.CompareNoCase( _T("email") ) == 0 )
					{
						m_sEmail = strAddress;
					}
					else if ( strGroup.CompareNoCase( _T("msn") ) == 0 )
					{
						m_sMSN = strAddress;
					}
					else if ( strGroup.CompareNoCase( _T("yahoo") ) == 0 )
					{
						m_sYahoo = strAddress;
					}
					else if ( strGroup.CompareNoCase( _T("icq") ) == 0 )
					{
						m_sICQ = strAddress;
					}
					else if ( strGroup.CompareNoCase( _T("aol") ) == 0 )
					{
						m_sAOL = strAddress;
					}
					else if ( strGroup.CompareNoCase( _T("jabber") ) == 0 )
					{
						m_sJabber = strAddress;
					}
				}
			}
		}
	}

	if ( m_sEmail.Find( '@' ) < 0 || m_sEmail.Find( '.' ) < 0 ) m_sEmail.Empty();

	UpdateData( FALSE );

	return TRUE;
}

void CContactProfilePage::OnOK()
{
	UpdateData();

	if ( m_sEmail.Find( '@' ) < 0 || m_sEmail.Find( '.' ) < 0 ) m_sEmail.Empty();

	AddAddress( _T("Email"), _T("Primary"), m_sEmail );
	AddAddress( _T("MSN"), _T("Primary"), m_sMSN );
	AddAddress( _T("Yahoo"), _T("Primary"), m_sYahoo );
	AddAddress( _T("ICQ"), _T("Primary"), m_sICQ );
	AddAddress( _T("AOL"), _T("Primary"), m_sAOL );
	AddAddress( _T("Jabber"), _T("Primary"), m_sJabber );
}

void CContactProfilePage::AddAddress(LPCTSTR pszClass, LPCTSTR pszName, LPCTSTR pszAddress)
{
	if ( CXMLElement* pContacts = MyProfile.GetXML( _T("contacts"), TRUE ) )
	{
		for ( POSITION pos1 = pContacts->GetElementIterator() ; pos1 ; )
		{
			CXMLElement* pGroup = pContacts->GetNextElement( pos1 );

			if ( pGroup->IsNamed( _T("group") ) &&
				 pGroup->GetAttributeValue( _T("class") ).CompareNoCase( pszClass ) == 0 )
			{
				for ( POSITION pos2 = pGroup->GetElementIterator() ; pos2 ; )
				{
					CXMLElement* pAddress = pGroup->GetNextElement( pos2 );

					if ( pAddress->IsNamed( _T("address") ) &&
						 pAddress->GetAttributeValue( _T("name") ).CompareNoCase( pszName ) == 0 )
					{
						if ( pszAddress && *pszAddress )
						{
							pAddress->AddAttribute( _T("content"), pszAddress );
							return;
						}
						else
						{
							pAddress->Delete();
							break;
						}
					}
				}

				if ( pszAddress && *pszAddress )
				{
					CXMLElement* pAddress = pGroup->AddElement( _T("address") );
					pAddress->AddAttribute( _T("name"), pszName );
					pAddress->AddAttribute( _T("content"), pszAddress );
				}
				else if ( pGroup->GetElementCount() == 0 )
				{
					pGroup->Delete();
				}
				break;
			}
		}

		if ( pszAddress && *pszAddress )
		{
			CXMLElement* pGroup = pContacts->AddElement( _T("group") );
			pGroup->AddAttribute( _T("class"), pszClass );

			CXMLElement* pAddress = pGroup->AddElement( _T("address") );
			pAddress->AddAttribute( _T("name"), pszName );
			pAddress->AddAttribute( _T("content"), pszAddress );
		}
		else if ( pContacts->GetElementCount() == 0 )
		{
			pContacts->Delete();
		}
	}
}
