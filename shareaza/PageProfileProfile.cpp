//
// PageProfileProfile.cpp
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

#include "StdAfx.h"
#include "Shareaza.h"
#include "Settings.h"
#include "Flags.h"
#include "GProfile.h"
#include "XML.h"
#include "PageProfileProfile.h"
#include "WorldGPS.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CProfileProfilePage, CSettingsPage)

BEGIN_MESSAGE_MAP(CProfileProfilePage, CSettingsPage)
	ON_CBN_SELCHANGE(IDC_LOC_COUNTRY, &CProfileProfilePage::OnSelChangeCountry)
	ON_CBN_SELCHANGE(IDC_LOC_CITY, &CProfileProfilePage::OnSelChangeCity)
	ON_LBN_SELCHANGE(IDC_INTEREST_LIST, &CProfileProfilePage::OnSelChangeInterestList)
	ON_CBN_SELCHANGE(IDC_INTEREST_ALL, &CProfileProfilePage::OnSelChangeInterestAll)
	ON_CBN_EDITCHANGE(IDC_INTEREST_ALL, &CProfileProfilePage::OnEditChangeInterestAll)
	ON_BN_CLICKED(IDC_INTEREST_ADD, &CProfileProfilePage::OnInterestAdd)
	ON_BN_CLICKED(IDC_INTEREST_REMOVE, &CProfileProfilePage::OnInterestRemove)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProfileProfilePage property page

CProfileProfilePage::CProfileProfilePage()
	: CSettingsPage	( CProfileProfilePage::IDD )
	, m_pWorld		( NULL )
{
}

CProfileProfilePage::~CProfileProfilePage()
{
	if ( m_pWorld ) delete m_pWorld;
}

void CProfileProfilePage::DoDataExchange(CDataExchange* pDX)
{
	CSettingsPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_INTEREST_REMOVE, m_wndInterestRemove);
	DDX_Control(pDX, IDC_INTEREST_ADD, m_wndInterestAdd);
	DDX_Control(pDX, IDC_INTEREST_ALL, m_wndInterestAll);
	DDX_Control(pDX, IDC_INTEREST_LIST, m_wndInterestList);
	DDX_Control(pDX, IDC_LOC_CITY, m_wndCity);
	DDX_Control(pDX, IDC_LOC_COUNTRY, m_wndCountry);
	DDX_CBString(pDX, IDC_LOC_CITY, m_sLocCity);
	DDX_CBString(pDX, IDC_LOC_COUNTRY, m_sLocCountry);
	DDX_Text(pDX, IDC_LOC_LAT, m_sLocLatitude);
	DDX_Text(pDX, IDC_LOC_LONG, m_sLocLongitude);
}

/////////////////////////////////////////////////////////////////////////////
// CProfileProfilePage message handlers

BOOL CProfileProfilePage::OnInitDialog()
{
	CSettingsPage::OnInitDialog();
	
	if ( CXMLElement* pLocation = MyProfile.GetXML( _T("location") ) )
	{
		if ( CXMLElement* pPolitical = pLocation->GetElementByName( _T("political") ) )
		{
			m_sLocCountry	= pPolitical->GetAttributeValue( _T("country") );
			m_sLocCity		= pPolitical->GetAttributeValue( _T("city") );
			CString str		= pPolitical->GetAttributeValue( _T("state") );
			if ( str.GetLength() && m_sLocCity.GetLength() )
				m_sLocCity += _T(", ");
            m_sLocCity += str;
		}

		if ( CXMLElement* pCoordinates = pLocation->GetElementByName( _T("coordinates") ) )
		{
			CString str = pCoordinates->GetAttributeValue( _T("latitude") );
			float nValue;

			if ( _stscanf( str, _T("%f"), &nValue ) == 1 )
			{
				m_sLocLatitude.Format( nValue >= 0 ?
					_T("%.2f\xb0 N") : _T("%.2f\xb0 S"), double( fabs( nValue ) ) );
			}

			str = pCoordinates->GetAttributeValue( _T("longitude") );

			if ( _stscanf( str, _T("%f"), &nValue ) == 1 )
			{
				m_sLocLongitude.Format( nValue >= 0 ? _T("%.1f\xb0 E") : _T("%.1f\xb0 W"),
					double( fabs( nValue ) ) );
			}
		}
	}

	if ( CXMLElement* pInterests = MyProfile.GetXML( _T("interests") ) )
	{
		for ( POSITION pos = pInterests->GetElementIterator() ; pos ; )
		{
			CXMLElement* pInterest = pInterests->GetNextElement( pos );

			if ( pInterest->IsNamed( _T("interest") ) )
			{
				m_wndInterestList.AddString( pInterest->GetValue() );
			}
		}
	}

	const int nFlags = Flags.GetCount();
	VERIFY( m_gdiFlags.Create( 18, 18, ILC_COLOR32|ILC_MASK, nFlags, 0 ) ||
			m_gdiFlags.Create( 18, 18, ILC_COLOR24|ILC_MASK, nFlags, 0 ) ||
			m_gdiFlags.Create( 18, 18, ILC_COLOR16|ILC_MASK, nFlags, 0 ) );
	for ( int nFlag = 0 ; nFlag < nFlags ; nFlag++ )
	{
		if ( HICON hIcon = Flags.ExtractIcon( nFlag ) )
		{
			VERIFY( m_gdiFlags.Add( hIcon ) != -1 );
			VERIFY( DestroyIcon( hIcon ) );
		}
	}
	m_wndCountry.SetImageList( &m_gdiFlags );

	m_pWorld = new CWorldGPS();
	m_pWorld->Load();

	const CWorldCountry* pCountry = m_pWorld->m_pCountry;

	int nSelect = -1;
	for ( int nCountry = 0 ; nCountry < (int)m_pWorld->m_nCountry; ++nCountry, ++pCountry )
	{
		const int nImage = Flags.GetFlagIndex( CString( pCountry->m_szID, 2 ) );
		const COMBOBOXEXITEM cbei =
		{
			CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_LPARAM | CBEIF_TEXT,
			nCountry,
			(LPTSTR)(LPCTSTR)pCountry->m_sName,
			pCountry->m_sName.GetLength() + 1,
			nImage,
			nImage,
			0,
			0,
			(LPARAM)pCountry
		};
		m_wndCountry.InsertItem( &cbei );
		if ( pCountry->m_sName.CompareNoCase( m_sLocCountry ) == 0 )
		{
			nSelect = nCountry;
		}
	}
	m_wndCountry.SetCurSel( nSelect );

	UpdateData( FALSE );

	m_wndInterestAdd.EnableWindow( FALSE );
	m_wndInterestRemove.EnableWindow( FALSE );

	OnSelChangeCountry();
	RecalcDropWidth( &m_wndCountry );

	return TRUE;
}

void CProfileProfilePage::OnSelChangeCountry()
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
	RecalcDropWidth( &m_wndCity );
}

void CProfileProfilePage::OnSelChangeCity()
{
	int nSel = m_wndCity.GetCurSel();
	if ( nSel < 0 ) return;
	CWorldCity* pCity = (CWorldCity*)m_wndCity.GetItemData( nSel );
	if ( ! pCity ) return;

	m_sLocLatitude.Format( pCity->m_nLatitude >= 0 ? _T("%.2f° N") : _T("%.2f° S"),
		double( fabs( pCity->m_nLatitude ) ) );

	m_sLocLongitude.Format( pCity->m_nLongitude >= 0 ? _T("%.1f° E") : _T("%.1f° W"),
		double( fabs( pCity->m_nLongitude ) ) );

	GetDlgItem( IDC_LOC_LAT )->SetWindowText( m_sLocLatitude );
	GetDlgItem( IDC_LOC_LONG )->SetWindowText( m_sLocLongitude );
}

void CProfileProfilePage::OnSelChangeInterestList()
{
	m_wndInterestRemove.EnableWindow( m_wndInterestList.GetCurSel() >= 0 );
}

void CProfileProfilePage::OnSelChangeInterestAll()
{
	m_wndInterestAdd.EnableWindow( m_wndInterestAll.GetCurSel() >= 0 || m_wndInterestAll.GetWindowTextLength() > 0 );
}

void CProfileProfilePage::OnEditChangeInterestAll()
{
	m_wndInterestAdd.EnableWindow( m_wndInterestAll.GetCurSel() >= 0 || m_wndInterestAll.GetWindowTextLength() > 0 );
}

void CProfileProfilePage::OnInterestAdd()
{
	CString str;
	m_wndInterestAll.GetWindowText( str );
	m_wndInterestList.AddString( str );
}

void CProfileProfilePage::OnInterestRemove()
{
	int nItem = m_wndInterestList.GetCurSel();
	if ( nItem >= 0 ) m_wndInterestList.DeleteString( nItem );
	m_wndInterestRemove.EnableWindow( FALSE );
}

void CProfileProfilePage::OnOK()
{
	UpdateData();

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
				if ( CXMLAttribute* pAttr = pPolitical->GetAttribute( _T("country") ) )
					pAttr->Delete();
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
				if ( CXMLAttribute* pAttr = pPolitical->GetAttribute( _T("state") ) )
					pAttr->Delete();
			}
			else
			{
				if ( CXMLAttribute* pAttr = pPolitical->GetAttribute( _T("city") ) )
					pAttr->Delete();
				if ( CXMLAttribute* pAttr = pPolitical->GetAttribute( _T("state") ) )
					pAttr->Delete();
			}

			if ( pPolitical->GetElementCount() == 0 && pPolitical->GetAttributeCount() == 0 )
				pPolitical->Delete();
		}

		if ( CXMLElement* pCoordinates = pLocation->GetElementByName( _T("coordinates"), TRUE ) )
		{
			CString strValue;
			float nValue = 0;

			if ( _stscanf( m_sLocLatitude, _T("%f"), &nValue ) == 1 )
			{
				if ( m_sLocLatitude.Find( 'S' ) >= 0 ) nValue *= -1;
				strValue.Format( _T("%f"), double( nValue ) );
				pCoordinates->AddAttribute( _T("latitude"), strValue );
			}

			if ( _stscanf( m_sLocLongitude, _T("%f"), &nValue ) == 1 )
			{
				if ( m_sLocLongitude.Find( 'W' ) >= 0 ) nValue *= -1;
				strValue.Format( _T("%f"), double( nValue ) );
				pCoordinates->AddAttribute( _T("longitude"), strValue );
			}

			if ( nValue == 0 ) pCoordinates->Delete();
		}

		if ( pLocation->GetElementCount() == 0 ) pLocation->Delete();
	}

	if ( CXMLElement* pInterests = MyProfile.GetXML( _T("interests"), TRUE ) )
	{
		pInterests->DeleteAllElements();

		for ( int nItem = 0 ; nItem < m_wndInterestList.GetCount() ; nItem++ )
		{
			CString str;
			m_wndInterestList.GetText( nItem, str );
			CXMLElement* pInterest = pInterests->AddElement( _T("interest") );
			pInterest->SetValue( str );
		}

		if ( pInterests->GetElementCount() == 0 ) pInterests->Delete();
	}

	CSettingsPage::OnOK();
}
