//
// GProfile.cpp
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
#include "G2Packet.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

LPCTSTR CGProfile::xmlns = _T("http://shareaza.sourceforge.net/schemas/GProfile.xsd");

BEGIN_INTERFACE_MAP(CGProfile, CComObject)
END_INTERFACE_MAP()

CGProfile	MyProfile;


//////////////////////////////////////////////////////////////////////
// CGProfile construction

CGProfile::CGProfile() :
	m_pXML( NULL )
{
	Create();
}

CGProfile::~CGProfile()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CGProfile access

BOOL CGProfile::IsValid() const
{
	if ( this == NULL ) return FALSE;
	if ( m_pXML == NULL ) return FALSE;

	// The whole identity and location tags are deleted if no attributes are present
	CXMLElement* pIdentity = m_pXML->GetElementByName( _T("identity") );
	CXMLElement* pLocation = m_pXML->GetElementByName( _T("location") );
	return pIdentity != NULL || pLocation != NULL;
}

CXMLElement* CGProfile::GetXML(LPCTSTR pszElement, BOOL bCreate)
{
	if ( m_pXML == NULL )
	{
		if ( ! bCreate ) return NULL;
		Create();
	}

	if ( pszElement == NULL ) return m_pXML;

	return m_pXML->GetElementByName( pszElement, bCreate );
}

//////////////////////////////////////////////////////////////////////
// CGProfile core

void CGProfile::Create()
{
	Clear();

	Hashes::Guid tmp;
	Hashes::BtGuid tmp_bt;

	VERIFY( SUCCEEDED( CoCreateGuid( reinterpret_cast< GUID* > ( &tmp[0] ) ) ) );

	// Convert Gnutella GUID (128 bits) to BitTorrent GUID (160 bits)
	CopyMemory( &tmp_bt[0], &tmp[0], tmp.byteCount );
	for ( int nByte = tmp.byteCount ; nByte < tmp_bt.byteCount ; nByte++ )
		tmp_bt[ nByte ] = uchar( rand() & 0xff );

	VERIFY( tmp.validate() );
	VERIFY( tmp_bt.validate() );

	oGUID = tmp;
	oGUIDBT = tmp_bt;

	m_pXML = new CXMLElement( NULL, _T("gProfile") );
	ASSERT( m_pXML );
	VERIFY( m_pXML->AddAttribute( _T("xmlns"), xmlns ) );

	CXMLElement* pGnutella = m_pXML->AddElement( _T("gnutella") );
	ASSERT( pGnutella );
	VERIFY( pGnutella->AddAttribute( _T("guid"), tmp.toString() ) ) ;

	CXMLElement* pBitTorrent = m_pXML->AddElement( _T("bittorrent") );
	ASSERT( pBitTorrent );
	VERIFY( pBitTorrent->AddAttribute( _T("guid"), tmp_bt.toString() ) );
}

void CGProfile::Clear()
{
	if ( m_pXML ) delete m_pXML;
	m_pXML = NULL;
}

//////////////////////////////////////////////////////////////////////
// CGProfile loading and saving

BOOL CGProfile::Load(LPCTSTR pszFile)
{
	CString strFile;

	if ( pszFile != NULL )
		strFile = pszFile;
	else
		strFile = Settings.General.UserPath + _T("\\Data\\Profile.xml");

	CXMLElement* pXML = CXMLElement::FromFile( strFile, TRUE );

	if ( pXML == NULL )
	{
		if ( pszFile == NULL ) Create();
		return FALSE;
	}

	if ( FromXML( pXML ) ) return TRUE;
	delete pXML;

	return FALSE;
}

BOOL CGProfile::Save(LPCTSTR pszFile)
{
	CString strXML;
	CFile pFile;

	if ( pszFile != NULL )
		strXML = pszFile;
	else
		strXML = Settings.General.UserPath + _T("\\Data\\Profile.xml");

	if ( ! pFile.Open( strXML, CFile::modeWrite|CFile::modeCreate ) ) return FALSE;

	if ( m_pXML != NULL )
		strXML = m_pXML->ToString( TRUE, TRUE );
	else
		strXML.Empty();

	int nASCII = WideCharToMultiByte( CP_UTF8, 0, strXML, strXML.GetLength(), NULL, 0, NULL, NULL );
	LPSTR pszASCII = new CHAR[ nASCII ];
	WideCharToMultiByte( CP_UTF8, 0, strXML, strXML.GetLength(), pszASCII, nASCII, NULL, NULL );
	pFile.Write( pszASCII, nASCII );
	delete [] pszASCII;

	pFile.Close();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CGProfile XML assignment

BOOL CGProfile::FromXML(CXMLElement* pXML)
{
	Clear();

	if ( pXML == NULL ) return FALSE;
	if ( pXML->GetAttributeValue( _T("xmlns") ).CompareNoCase( xmlns ) ) return FALSE;
	if ( pXML->IsNamed( _T("gProfile") ) == FALSE ) return FALSE;

	// Loading Gnutella GUID
	CXMLElement* pGnutella = pXML->GetElementByName( _T("gnutella") );
	if ( pGnutella == NULL ) return FALSE;

	CString strGUID = pGnutella->GetAttributeValue( _T("guid") );

	Hashes::Guid tmp;
	if ( ! tmp.fromString( strGUID ) ) return FALSE;
	oGUID = tmp;

	// Loading BitTorrent GUID
	Hashes::BtGuid tmp_bt;
	CXMLElement* pBitTorrent = pXML->GetElementByName( _T("bittorrent") );
	if ( pBitTorrent )
	{
		CString strGUID = pBitTorrent->GetAttributeValue( _T("guid") );
		if ( ! tmp_bt.fromString( strGUID )) return FALSE;
	}
	else
	{
		// Convert Gnutella GUID (128 bits) to BitTorrent GUID (160 bits)
		CopyMemory( &tmp_bt[0], &tmp[0], tmp.byteCount );
		for ( int nByte = tmp.byteCount ; nByte < tmp_bt.byteCount ; nByte++ )
			tmp_bt[ nByte ] = uchar( rand() & 0xff );
		VERIFY( tmp_bt.validate() );

		pBitTorrent = pXML->AddElement( _T("bittorrent") );
		pBitTorrent->AddAttribute( _T("guid"), tmp_bt.toString() );
	}
	oGUIDBT = tmp_bt;

	m_pXML = pXML;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CGProfile field access

CString CGProfile::GetNick() const
{
	CString str;
	if ( m_pXML == NULL ) return str;
	CXMLElement* pIdentity = m_pXML->GetElementByName( _T("identity") );
	if ( pIdentity == NULL ) return str;
	CXMLElement* pHandle = pIdentity->GetElementByName( _T("handle") );
	if ( pHandle == NULL ) return str;
	str = pHandle->GetAttributeValue( _T("primary") );
	return str;
}

CString CGProfile::GetLocation() const
{
	CString str;
	if ( m_pXML == NULL ) return str;
	CXMLElement* pLocation = m_pXML->GetElementByName( _T("location") );
	if ( pLocation == NULL ) return str;
	CXMLElement* pPolitical = pLocation->GetElementByName( _T("political") );
	if ( pPolitical == NULL ) return str;

	CString strCity = pPolitical->GetAttributeValue( _T("city") );
	CString strState = pPolitical->GetAttributeValue( _T("state") );
	CString strCountry = pPolitical->GetAttributeValue( _T("country") );

	str = strCity;
	if ( strState.GetLength() )
	{
		if ( str.GetLength() ) str += _T(", ");
		str += strState;
	}
	if ( strCountry.GetLength() )
	{
		if ( str.GetLength() ) str += _T(", ");
		str += strCountry;
	}

	return str;
}

CString CGProfile::GetContact(LPCTSTR pszType) const
{
	CString str;

	if ( m_pXML == NULL ) return str;
	CXMLElement* pContacts = m_pXML->GetElementByName( _T("contacts") );
	if ( pContacts == NULL ) return str;

	for ( POSITION pos = pContacts->GetElementIterator() ; pos ; )
	{
		CXMLElement* pGroup = pContacts->GetNextElement( pos );

		if ( pGroup->IsNamed( _T("group") ) &&
			 pGroup->GetAttributeValue( _T("class") ).CompareNoCase( pszType ) == 0 )
		{
			if ( CXMLElement* pAddress = pGroup->GetElementByName( _T("address") ) )
			{
				str = pAddress->GetAttributeValue( _T("content") );
				break;
			}
		}
	}

	return str;
}

DWORD CGProfile::GetPackedGPS() const
{
	if ( m_pXML == NULL ) return 0;
	CXMLElement* pLocation = m_pXML->GetElementByName( _T("location") );
	if ( pLocation == NULL ) return 0;
	CXMLElement* pCoordinates = pLocation->GetElementByName( _T("coordinates") );
	if ( pCoordinates == NULL ) return 0;

	float nLatitude = 0, nLongitude = 0;
	_stscanf( pCoordinates->GetAttributeValue( _T("latitude") ), _T("%f"), &nLatitude );
	_stscanf( pCoordinates->GetAttributeValue( _T("longitude") ), _T("%f"), &nLongitude );
	if ( nLatitude == 0 || nLongitude == 0 ) return 0;

#define WORDLIM(x)  (WORD)( (x) < 0 ? 0 : ( (x) > 65535 ? 65535 : (x) ) )
	WORD nLat = WORDLIM( ( nLatitude + 90.0f )   * 65535.0f / 180.0f );
	WORD nLon = WORDLIM( ( nLongitude + 180.0f ) * 65535.0f / 360.0f );
#undef WORDLIM

	return (DWORD)nLon + ( (DWORD)nLat << 16 );
}

//////////////////////////////////////////////////////////////////////
// CGProfile create avatar packet

CG2Packet* CGProfile::CreateAvatar()
{
	if ( m_pXML == NULL ) return NULL;

	CXMLElement* pAvatar = m_pXML->GetElementByName( _T("avatar") );
	if ( pAvatar == NULL ) return NULL;
	CString strPath = pAvatar->GetAttributeValue( _T("path") );
	if ( strPath.IsEmpty() ) return NULL;

	CFile pFile;
	if ( ! pFile.Open( strPath, CFile::modeRead ) ) return NULL;

	int nPos = strPath.ReverseFind( '\\' );
	if ( nPos >= 0 ) strPath = strPath.Mid( nPos + 1 );

	CG2Packet* pPacket = CG2Packet::New( G2_PACKET_PROFILE_AVATAR );

	pPacket->WritePacket( G2_PACKET_NAME, pPacket->GetStringLen( strPath ) );
	pPacket->WriteString( strPath, FALSE );

	pPacket->WritePacket( G2_PACKET_BODY, (DWORD)pFile.GetLength() );
	LPBYTE pBody = pPacket->WriteGetPointer( (DWORD)pFile.GetLength() );

	if ( pBody == NULL )
	{
		theApp.Message( MSG_ERROR, _T("Memory allocation error in CGProfile::CreateAvatar()") );
	}
	else
	{
		pFile.Read( pBody, (DWORD)pFile.GetLength() );
	}
	pFile.Close();

	return pPacket;
}

//////////////////////////////////////////////////////////////////////
// CGProfile serialize

void CGProfile::Serialize(CArchive& ar)
{
	BOOL bXMLPresent = FALSE;

	if ( ar.IsStoring() )
	{
		bXMLPresent = ( m_pXML != NULL );
		ar << bXMLPresent;
	}
	else
	{
		ar >> bXMLPresent;
		Create();
	}
	if ( m_pXML && bXMLPresent )
		m_pXML->Serialize( ar );
}
