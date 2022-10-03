//
// CtrlBrowseProfile.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2017.
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
#include "G2Packet.h"
#include "HostBrowser.h"
#include "XML.h"

#include "RichElement.h"
#include "RichDocument.h"
#include "CoolInterface.h"
#include "CtrlBrowseProfile.h"
#include "Skin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CBrowseProfileCtrl, CWnd)
	//{{AFX_MSG_MAP(CBrowseProfileCtrl)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_NOTIFY(RVN_CLICK, IDC_BROWSE_PROFILE, OnClickView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBrowseProfileCtrl construction

CBrowseProfileCtrl::CBrowseProfileCtrl()
{
	m_pDocument1 = NULL;
	m_pDocument2 = NULL;
}

CBrowseProfileCtrl::~CBrowseProfileCtrl()
{
}

/////////////////////////////////////////////////////////////////////////////
// CBrowseProfileCtrl operations

BOOL CBrowseProfileCtrl::Create(CWnd* pParentWnd)
{
	CRect rect( 0, 0, 0, 0 );
	return CWnd::Create( NULL, NULL, WS_CHILD,
		rect, pParentWnd, IDC_BROWSE_PROFILE, NULL );
}

void CBrowseProfileCtrl::OnSkinChange()
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( m_pDocument1 != NULL )
		m_pDocument1->Clear();
	else
		m_pDocument1 = new CRichDocument();

	m_pdNick = m_pdFullName = m_pdFullLocation = NULL;
	m_pdGenderMale = m_pdGenderFemale = m_pdAge = NULL;
	m_pdContactEmail = m_pdContactMSN = m_pdContactYahoo = m_pdContactICQ = m_pdContactAOL = m_pdContactJabber = NULL;
	m_pdBioText = m_pdInterests = NULL;

	if ( CXMLElement* pXML = Skin.GetDocument( _T("CBrowseHostProfile.1") ) )
	{
		CElementMap pMap;
		m_pDocument1->LoadXML( pXML, &pMap );

		pMap.Lookup( _T("Nick"), m_pdNick );
		pMap.Lookup( _T("FullName"), m_pdFullName );
		pMap.Lookup( _T("FullLocation"), m_pdFullLocation );
		pMap.Lookup( _T("GenderMale"), m_pdGenderMale );
		pMap.Lookup( _T("GenderFemale"), m_pdGenderFemale );
		pMap.Lookup( _T("Age"), m_pdAge );
		pMap.Lookup( _T("ContactEmail"), m_pdContactEmail );
		pMap.Lookup( _T("ContactMSN"), m_pdContactMSN );
		pMap.Lookup( _T("ContactYahoo"), m_pdContactYahoo );
		pMap.Lookup( _T("ContactICQ"), m_pdContactICQ );
		pMap.Lookup( _T("ContactAOL"), m_pdContactAOL );
		pMap.Lookup( _T("ContactJabber"), m_pdContactJabber );
		pMap.Lookup( _T("Interests"), m_pdInterests );
		pMap.Lookup( _T("BioText"), m_pdBioText );
	}

	if ( m_pDocument2 != NULL )
		m_pDocument2->Clear();
	else
		m_pDocument2 = new CRichDocument();

	m_pdBookmarks = NULL;

	if ( CXMLElement* pXML = Skin.GetDocument( _T("CBrowseHostProfile.2") ) )
	{
		CElementMap pMap;
		m_pDocument2->LoadXML( pXML, &pMap );

		pMap.Lookup( _T("Bookmarks"), m_pdBookmarks );
	}

	m_wndDoc1.SetDocument( m_pDocument1 );
	m_wndDoc2.SetDocument( m_pDocument2 );
}

void CBrowseProfileCtrl::Update(CHostBrowser* pBrowser)
{
	CGProfile* pProfile = pBrowser->m_pProfile;

	if ( m_pDocument1 == NULL || m_pDocument2 == NULL ) return;
	if ( pProfile == NULL || pProfile->IsValid() == FALSE ) return;

	if ( ! pBrowser->IsBrowsing() && pBrowser->m_nHits > 0 )
	{
		LoadDefaultHead();
	}

	UpdateDocument1( pProfile );
	UpdateDocument2( pBrowser );

	if ( m_wndDoc1.IsWindowVisible() == FALSE ) m_wndDoc1.ShowWindow( SW_SHOW );
	if ( m_wndDoc2.IsWindowVisible() == FALSE ) m_wndDoc2.ShowWindow( SW_SHOW );
}

void CBrowseProfileCtrl::UpdateDocument1(CGProfile* pProfile)
{
	CSingleLock pLock( &m_pSection, TRUE );
	CSingleLock pDocLock( &m_pDocument1->m_pSection, TRUE );
	CString str;

	if ( m_pdNick != NULL ) m_pdNick->SetText( pProfile->GetNick() );

	if ( CXMLElement* pIdentity = pProfile->GetXML( _T("identity") ) )
	{
		if ( CXMLElement* pName = pIdentity->GetElementByName( _T("name") ) )
		{
			CString strFirst	= pName->GetAttributeValue( _T("first") );
			CString strLast		= pName->GetAttributeValue( _T("last") );

			if ( m_pdFullName != NULL && ( strFirst.GetLength() || strLast.GetLength() ) )
			{
				m_pDocument1->ShowGroup( 1, TRUE );
				m_pdFullName->SetText( strFirst + ' ' + strLast );
			}
			else
			{
				m_pDocument1->ShowGroup( 1, FALSE );
			}
		}
		else
		{
			m_pDocument1->ShowGroup( 1, FALSE );
		}
	}
	else
	{
		m_pDocument1->ShowGroup( 1, FALSE );
	}

	str = pProfile->GetLocation();
	m_pDocument1->ShowGroup( 2, str.GetLength() > 0 );
	if ( m_pdFullLocation != NULL ) m_pdFullLocation->SetText( str );

	if ( CXMLElement* pVitals = pProfile->GetXML( _T("vitals") ) )
	{
		str = pVitals->GetAttributeValue( _T("gender") );
		m_pDocument1->ShowGroup( 3, str.GetLength() > 0 );

		if ( m_pdGenderMale != NULL && m_pdGenderFemale != NULL )
		{
			m_pdGenderMale->Show( str.CompareNoCase( _T("male") ) == 0 );
			m_pdGenderFemale->Show( str.CompareNoCase( _T("female") ) == 0 );
		}

		str = pVitals->GetAttributeValue( _T("age") );
		if ( m_pdAge != NULL ) m_pdAge->SetText( str );
	}
	else
	{
		m_pDocument1->ShowGroup( 3, FALSE );
	}

	BOOL bContact = FALSE;

	str = pProfile->GetContact( _T("Email") );
	bContact |= ( str.GetLength() > 0 );
	m_pDocument1->ShowGroup( 40, str.GetLength() > 0 );
	if ( m_pdContactEmail != NULL )
	{
		m_pdContactEmail->SetText( str );
		m_pdContactEmail->m_sLink = _T("mailto:") + str;
	}

	str = pProfile->GetContact( _T("MSN") );
	bContact |= ( str.GetLength() > 0 );
	m_pDocument1->ShowGroup( 44, str.GetLength() > 0 );
	if ( m_pdContactMSN != NULL )
	{
		m_pdContactMSN->SetText( str );
		m_pdContactMSN->m_sLink = _T("msn:") + str;
	}

	str = pProfile->GetContact( _T("Yahoo") );
	bContact |= ( str.GetLength() > 0 );
	m_pDocument1->ShowGroup( 41, str.GetLength() > 0 );
	if ( m_pdContactYahoo != NULL )
	{
		m_pdContactYahoo->SetText( str );
		m_pdContactYahoo->m_sLink = _T("ymsgr:sendim?") + str;
	}

	str = pProfile->GetContact( _T("ICQ") );
	bContact |= ( str.GetLength() > 0 );
	m_pDocument1->ShowGroup( 42, str.GetLength() > 0 );
	if ( m_pdContactICQ != NULL )
	{
		m_pdContactICQ->SetText( str );
		m_pdContactICQ->m_sLink = _T("http://people.icq.com/people/about_me.php?uin=") + str;
	}

	str = pProfile->GetContact( _T("AOL") );
	bContact |= ( str.GetLength() > 0 );
	m_pDocument1->ShowGroup( 43, str.GetLength() > 0 );
	if ( m_pdContactAOL != NULL )
	{
		m_pdContactAOL->SetText( str );
		m_pdContactAOL->m_sLink = _T("aim:goim?screenname=") + str;
	}

	str = pProfile->GetContact( _T("Jabber") );
	bContact |= ( str.GetLength() > 0 );
	m_pDocument1->ShowGroup( 45, str.GetLength() > 0 );
	if ( m_pdContactJabber != NULL )
	{
		m_pdContactJabber->SetText( str );
	}

	m_pDocument1->ShowGroup( 4, bContact );

	str.Empty();

	if ( CXMLElement* pInterests = pProfile->GetXML( _T("interests") ) )
	{
		for ( POSITION pos = pInterests->GetElementIterator() ; pos ; )
		{
			CXMLElement* pInterest = pInterests->GetNextElement( pos );

			if ( pInterest->IsNamed( _T("interest") ) )
			{
				if ( str.GetLength() ) str += _T(", ");
				str += pInterest->GetValue();
			}
		}
	}

	m_pDocument1->ShowGroup( 5, str.GetLength() > 0 );
	if ( m_pdInterests != NULL ) m_pdInterests->SetText( str );

	str.Empty();
	if ( CXMLElement* pBio = pProfile->GetXML( _T("notes") ) )
		str = pBio->GetValue();
	m_pDocument1->ShowGroup( 6, str.GetLength() > 0 );
	if ( m_pdBioText != NULL ) m_pdBioText->SetText( str );

	m_wndDoc1.InvalidateIfModified();
}

void CBrowseProfileCtrl::UpdateDocument2(CHostBrowser* pBrowser)
{
	CSingleLock pLock( &m_pSection, TRUE );

	int nBookmarks = 0;
	CGProfile* pProfile = pBrowser->m_pProfile;

	CSingleLock pDocLock( &m_pDocument2->m_pSection, TRUE );

	m_pDocument2->ShowGroup( 2, pBrowser->m_bCanChat );

	if ( m_pdBookmarks != NULL )
	{
		POSITION pos = m_pDocument2->Find( m_pdBookmarks );
		if ( pos == NULL ) return;

		POSITION posSave = pos;
		m_pDocument2->GetNext( pos );

		while ( pos )
		{
			CRichElement* pElement = m_pDocument2->GetNext( pos );
			if ( pElement->m_nGroup != 1 ) break;
			pElement->Delete();
		}

		m_pDocument2->GetNext( posSave );

		if ( CXMLElement* pBookmarks = pProfile->GetXML( _T("bookmarks") ) )
		{
			for ( pos = pBookmarks->GetElementIterator() ; pos ; )
			{
				CXMLElement* pBookmark = pBookmarks->GetNextElement( pos );
				if ( ! pBookmark->IsNamed( _T("bookmark") ) ) continue;

				CString strTitle	= pBookmark->GetAttributeValue( _T("title") );
				CString strURL		= pBookmark->GetAttributeValue( _T("url") );

				if ( strTitle.IsEmpty() ) continue;
				if ( strURL.Find( _T("http://") ) != 0 ) continue;

				m_pDocument2->Add( retIcon, MAKEINTRESOURCE(IDI_WEB_URL), strURL, 0, 1, posSave );
				m_pDocument2->Add( retGap, _T("5"), NULL, 0, 1, posSave );
				m_pDocument2->Add( retLink, strTitle, strURL, retfMiddle, 1, posSave );
				m_pDocument2->Add( retNewline, _T("2"), NULL, 0, 1, posSave );

				nBookmarks ++;
			}
		}
	}

	if ( nBookmarks )
		m_pDocument2->ShowGroup( 3, TRUE );
	else
		m_pDocument2->ShowGroup( 3, FALSE );

	m_wndDoc2.InvalidateIfModified();
}

void CBrowseProfileCtrl::OnHeadPacket(CG2Packet* pPacket)
{
	CString strFile;
	G2_PACKET nType;
	DWORD nLength;

	while ( pPacket->ReadPacket( nType, nLength ) )
	{
		DWORD nNext = pPacket->m_nPosition + nLength;

		if ( nType == G2_PACKET_NAME )
		{
			strFile = pPacket->ReadString( nLength );
		}
		else if ( nType == G2_PACKET_BODY )
		{
			CSingleLock pLock( &m_pSection, TRUE );

			if ( m_imgHead.LoadFromMemory( PathFindExtension( strFile ),
				 (LPCVOID)( pPacket->m_pBuffer + pPacket->m_nPosition ), nLength ) &&
				 m_imgHead.EnsureRGB( CoolInterface.m_crWindow ) &&
				 m_imgHead.Resample( 128, 128 ) )
			{
				// Ok
			}
		}

		pPacket->m_nPosition = nNext;
	}

	PostMessage( WM_TIMER, 1 );
}

void CBrowseProfileCtrl::LoadDefaultHead()
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( m_imgHead.IsLoaded() ) return;

	if ( m_imgHead.LoadFromFile( Settings.General.Path + _T("\\Data\\DefaultAvatar.png") ) &&
		m_imgHead.EnsureRGB( CoolInterface.m_crWindow ) &&
		m_imgHead.Resample( 128, 128 ) )
	{
		Invalidate();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBrowseProfileCtrl message handlers

int CBrowseProfileCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if ( CWnd::OnCreate( lpCreateStruct ) == -1 ) return -1;

	CRect rect;
	m_wndDoc1.Create( WS_CHILD, rect, this, IDC_BROWSE_PROFILE );
	m_wndDoc2.Create( WS_CHILD, rect, this, IDC_BROWSE_PROFILE );
	m_wndDoc1.SetSelectable( TRUE );

	return 0;
}

void CBrowseProfileCtrl::OnDestroy()
{
	CWnd::OnDestroy();

	if ( m_pDocument2 != NULL ) delete m_pDocument2;
	m_pDocument2 = NULL;

	if ( m_pDocument1 != NULL ) delete m_pDocument1;
	m_pDocument1 = NULL;
}

void CBrowseProfileCtrl::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize( nType, cx, cy );

	CRect rc;
	GetClientRect( &rc );
	rc.left += 148;

	m_wndDoc1.SetWindowPos( NULL, rc.left, rc.top, rc.Width() / 2,
		rc.Height(), SWP_NOZORDER );

	m_wndDoc2.SetWindowPos( NULL, ( rc.left + rc.right ) / 2, rc.top,
		rc.Width() - rc.Width() / 2, rc.Height(), SWP_NOZORDER );
}

void CBrowseProfileCtrl::OnPaint()
{
	CSingleLock pLock( &m_pSection, TRUE );
	CPaintDC dc( this );
	CRect rcPanel;

	GetClientRect( &rcPanel );

	if ( m_imgHead.IsLoaded() )
	{
		CRect rcHead( 10, 10, 138, 138 );
		CDC dcMem;
		CBitmap bmHead;
		bmHead.Attach( m_imgHead.CreateBitmap() );
		dcMem.CreateCompatibleDC( &dc );
		CBitmap* pOldBmp = (CBitmap*)dcMem.SelectObject( &bmHead );
		dc.BitBlt( rcHead.left, rcHead.top, rcHead.Width(), rcHead.Height(),
			&dcMem, 0, 0, SRCCOPY );
		dc.ExcludeClipRect( &rcHead );
		dcMem.SelectObject( pOldBmp );
		bmHead.DeleteObject();
	}

	if ( m_wndDoc1.IsWindowVisible() )
	{
		rcPanel.right = rcPanel.left + 148;
	}

	dc.FillSolidRect( &rcPanel, CoolInterface.m_crWindow );
}

void CBrowseProfileCtrl::OnTimer(UINT_PTR nIDEvent)
{
	if ( nIDEvent == 1 )
	{
		Invalidate();
	}
}

void CBrowseProfileCtrl::OnClickView(NMHDR* pNotify, LRESULT* /*pResult*/)
{
	if ( CRichElement* pElement = ((RVN_ELEMENTEVENT*) pNotify)->pElement )
	{
		theApp.InternalURI( pElement->m_sLink );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBrowseProfileCtrl serialize

void CBrowseProfileCtrl::Serialize(CArchive& ar, int /*nVersion*/ /* BROWSER_SER_VERSION */)
{
	CSingleLock pLock( &m_pSection, TRUE );

	m_imgHead.Serialize( ar );

	if ( ar.IsLoading() )
	{
		PostMessage( WM_TIMER, 1 );
	}
}
