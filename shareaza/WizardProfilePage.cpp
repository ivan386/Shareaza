//
// WizardProfilePage.cpp
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
#include "Settings.h"
#include "GProfile.h"
#include "WorldGPS.h"
#include "Skin.h"
#include "XML.h"
#include "WizardProfilePage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CWizardProfilePage, CWizardPage)

BEGIN_MESSAGE_MAP(CWizardProfilePage, CWizardPage)
	//{{AFX_MSG_MAP(CWizardProfilePage)
	ON_CBN_SELCHANGE(IDC_LOC_COUNTRY, OnSelChangeCountry)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWizardProfilePage property page

CWizardProfilePage::CWizardProfilePage() : CWizardPage(CWizardProfilePage::IDD)
{
	//{{AFX_DATA_INIT(CWizardProfilePage)
	m_nGender = 0;
	m_nAge = 0;
	//}}AFX_DATA_INIT
	m_pWorld = NULL;
}

CWizardProfilePage::~CWizardProfilePage()
{
	delete m_pWorld;
}

void CWizardProfilePage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWizardProfilePage)
	DDX_Text(pDX, IDC_PROFILE_NICK, m_sNick);
	DDX_Control(pDX, IDC_LOC_CITY, m_wndCity);
	DDX_Control(pDX, IDC_LOC_COUNTRY, m_wndCountry);
	DDX_CBString(pDX, IDC_LOC_CITY, m_sLocCity);
	DDX_CBString(pDX, IDC_LOC_COUNTRY, m_sLocCountry);
	DDX_Control(pDX, IDC_PROFILE_AGE, m_wndAge);
	DDX_CBIndex(pDX, IDC_PROFILE_AGE, m_nAge);
	DDX_CBIndex(pDX, IDC_PROFILE_GENDER, m_nGender);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CWizardProfilePage message handlers

BOOL CWizardProfilePage::OnInitDialog()
{
	CWizardPage::OnInitDialog();

	Skin.Apply( _T("CWizardProfilePage"), this );

	for ( int nAge = 13 ; nAge <= 110 ; nAge++ )
	{
		CString str, strYearsOld;
		LoadString( strYearsOld, IDS_WIZARD_YEARS_OLD );
		str.Format( _T(" %i ") + strYearsOld, nAge );
		m_wndAge.SetItemData( m_wndAge.AddString( str ), nAge );
	}

	m_pWorld = new CWorldGPS();
	m_pWorld->Load();

	CWorldCountry* pCountry = m_pWorld->m_pCountry;

	for ( int nCountry = m_pWorld->m_nCountry ; nCountry ; nCountry--, pCountry++ )
	{
		m_wndCountry.SetItemData( m_wndCountry.AddString( pCountry->m_sName ), (LPARAM)pCountry );
	}

	return TRUE;
}

BOOL CWizardProfilePage::OnSetActive()
{
	SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );

	m_sNick = MyProfile.GetNick();

	if ( m_sNick.IsEmpty() )
	{
		TCHAR pBuffer[64];
		DWORD nSize = 64;
		if ( GetUserNameW( pBuffer, &nSize ) ) m_sNick = pBuffer;
	}

	if ( CXMLElement* pVitals = MyProfile.GetXML( _T("vitals") ) )
	{
		CString strGender	= pVitals->GetAttributeValue( _T("gender") );
		CString strAge		= pVitals->GetAttributeValue( _T("age") );

		if ( strGender.CompareNoCase( _T("male") ) == 0 )
		{
			m_nGender = 1;
		}
		else if ( strGender.CompareNoCase( _T("female") ) == 0 )
		{
			m_nGender = 2;
		}

		int nAge = 0;
		_stscanf( strAge, _T("%i"), &nAge );

		for ( int nAgeItem = 0 ; nAgeItem < m_wndAge.GetCount() ; nAgeItem ++ )
		{
			if ( m_wndAge.GetItemData( nAgeItem ) == DWORD( nAge ) )
			{
				m_nAge = nAgeItem;
				break;
			}
		}
	}

	if ( CXMLElement* pLocation = MyProfile.GetXML( _T("location") ) )
	{
		if ( CXMLElement* pPolitical = pLocation->GetElementByName( _T("political") ) )
		{
			m_sLocCountry	= pPolitical->GetAttributeValue( _T("country") );
			m_sLocCity		= pPolitical->GetAttributeValue( _T("city") ) + _T(", ")
							+ pPolitical->GetAttributeValue( _T("state") );
		}
	}

	UpdateData( FALSE );

	OnSelChangeCountry();

	return CWizardPage::OnSetActive();
}

void CWizardProfilePage::OnSelChangeCountry()
{
	CWaitCursor pCursor;

	int nSel = m_wndCountry.GetCurSel();
	if ( nSel < 0 ) return;
	CWorldCountry* pCountry = (CWorldCountry*)m_wndCountry.GetItemData( nSel );
	if ( ! pCountry ) return;

	if ( m_wndCity.GetCount() ) m_wndCity.ResetContent();

	CWorldCity* pCity = pCountry->m_pCity;
	CString strCity;

	for ( int nCity = pCountry->m_nCity ; nCity ; nCity--, pCity++ )
	{
		if ( pCity->m_sName.GetLength() )
		{
			if ( pCity->m_sState.GetLength() )
				strCity = pCity->m_sName + _T(", ") + pCity->m_sState;
			else
				strCity = pCity->m_sName;
		}
		else if ( pCity->m_sState.GetLength() )
		{
			strCity = pCity->m_sState;
		}
		else
		{
			continue;
		}
		m_wndCity.SetItemData( m_wndCity.AddString( strCity ), (LPARAM)pCity );
	}
}

LRESULT CWizardProfilePage::OnWizardBack()
{
	return CWizardPage::OnWizardBack();
}

LRESULT CWizardProfilePage::OnWizardNext()
{
	UpdateData( TRUE );

	if ( CXMLElement* pIdentity = MyProfile.GetXML( _T("identity"), TRUE ) )
	{
		if ( CXMLElement* pHandle = pIdentity->GetElementByName( _T("handle"), TRUE ) )
		{
			pHandle->AddAttribute( _T("primary"), m_sNick );
			if ( m_sNick.IsEmpty() ) pHandle->Delete();
		}
	}

	if ( CXMLElement* pVitals = MyProfile.GetXML( _T("vitals"), TRUE ) )
	{
		if ( m_nAge > 0 )
		{
			CString strAge;
			strAge.Format( _T("%u"), (DWORD)m_wndAge.GetItemData( m_nAge ) );
			pVitals->AddAttribute( _T("age"), strAge );
		}
		else
		{
			pVitals->DeleteAttribute( _T("age") );
		}

		if ( m_nGender == 1 || m_nGender == 3 )
			pVitals->AddAttribute( _T("gender"), _T("male") );
		else if ( m_nGender == 2 || m_nGender == 4 )
			pVitals->AddAttribute( _T("gender"), _T("female") );
		else
			pVitals->DeleteAttribute( _T("gender") );

		if ( pVitals->GetElementCount() == 0 &&
			 pVitals->GetAttributeCount() == 0 ) pVitals->Delete();
	}

	if ( CXMLElement* pLocation = MyProfile.GetXML( _T("location"), TRUE ) )
	{
		if ( CXMLElement* pPolitical = pLocation->GetElementByName( _T("political"), TRUE ) )
		{
			if ( m_sLocCountry.GetLength() )
			{
				pPolitical->AddAttribute( _T("country"), m_sLocCountry );
			}
			else
			{
				pPolitical->DeleteAttribute( _T("country") );
			}

			int nPos = m_sLocCity.Find( _T(", ") );

			if ( nPos >= 0 )
			{
				pPolitical->AddAttribute( _T("city"), m_sLocCity.Left( nPos ) );
				pPolitical->AddAttribute( _T("state"), m_sLocCity.Mid( nPos + 2 ) );
			}
			else if ( m_sLocCity.GetLength() )
			{
				pPolitical->AddAttribute( _T("city"), m_sLocCity );
				pPolitical->DeleteAttribute( _T("state") );
			}
			else
			{
				pPolitical->DeleteAttribute( _T("city") );
				pPolitical->DeleteAttribute( _T("state") );
			}

			if ( pPolitical->GetElementCount() == 0 && pPolitical->GetAttributeCount() == 0 )
				pPolitical->Delete();
		}

		if ( CXMLElement* pCoordinates = pLocation->GetElementByName( _T("coordinates"), TRUE ) )
		{
			int nSel = m_wndCity.GetCurSel();
			CWorldCity* pCity = (CWorldCity*)m_wndCity.GetItemData( m_wndCity.GetCurSel() );

			if ( nSel >= 0 && pCity != NULL )
			{
				CString strValue;

				strValue.Format( _T("%f"), pCity->m_nLatitude );
				pCoordinates->AddAttribute( _T("latitude"), strValue );

				strValue.Format( _T("%f"), pCity->m_nLongitude );
				pCoordinates->AddAttribute( _T("longitude"), strValue );
			}
			else
			{
				pCoordinates->Delete();
			}
		}

		if ( pLocation->GetElementCount() == 0 ) pLocation->Delete();
	}

	/* These popups are pretty annoying. Since this information is underutilized let's 
	   not put so much emphasis on it at this time.

	if ( MyProfile.GetNick().IsEmpty() )
	{
		LoadString( strMessage, IDS_PROFILE_NO_NICK );
		AfxMessageBox( strMessage, MB_ICONEXCLAMATION );
		return -1;
	}

	if ( MyProfile.GetXML( _T("vitals") ) == NULL )
	{
		LoadString( strMessage, IDS_PROFILE_NO_VITALS );
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 ) != IDYES ) return -1;
	}

	if ( MyProfile.GetLocation().IsEmpty() )
	{
		LoadString( strMessage, IDS_PROFILE_NO_LOCATION );
		if ( AfxMessageBox( strMessage, MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 ) != IDYES ) return -1;
	}
	*/

	MyProfile.Save();

	return 0;
}
