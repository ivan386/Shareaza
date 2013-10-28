//
// GProfile.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2011.
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

#define WORDLIM(x)  (WORD)( ( (x) < 0 ) ? 0 : ( ( (x) > 65535 ) ? 65535 : (x) ) )

LPCTSTR CGProfile::xmlns = _T("http://www.shareaza.com/schemas/GProfile.xsd");

BEGIN_INTERFACE_MAP(CGProfile, CComObject)
END_INTERFACE_MAP()

CGProfile	MyProfile;


//////////////////////////////////////////////////////////////////////
// CGProfile construction

CGProfile::CGProfile() :
	m_pXML( new CXMLElement( NULL, _T("gProfile") ) )
{
	ASSERT( m_pXML );
	VERIFY( m_pXML->AddAttribute( _T("xmlns"), xmlns ) );
}

CGProfile::~CGProfile()
{
}

//////////////////////////////////////////////////////////////////////
// CGProfile access

BOOL CGProfile::IsValid() const
{
	// The whole identity and location tags are deleted if no attributes are present
	return ( this && m_pXML && (
		m_pXML->GetElementByName( _T("identity") ) ||
		m_pXML->GetElementByName( _T("location") ) ) );
}

CXMLElement* CGProfile::GetXML(LPCTSTR pszElement, BOOL bCreate)
{
	if ( pszElement )
		return m_pXML->GetElementByName( pszElement, bCreate );

	// Remove "avatar" element
	m_pXMLExport.Free();
	m_pXMLExport.Attach( m_pXML->Clone() );
	if ( CXMLElement* pAvatar = m_pXMLExport->GetElementByName( _T("avatar"), FALSE ) )
	{
		pAvatar->Delete();
	}

	return m_pXMLExport;
}

//////////////////////////////////////////////////////////////////////
// CGProfile core

void CGProfile::Create()
{
	// Generate new Gnutella GUID
	Hashes::Guid tmp;
	VERIFY( SUCCEEDED( CoCreateGuid( reinterpret_cast< GUID* > ( &tmp[0] ) ) ) );
	
	VERIFY( tmp.validate() );
	oGUID = tmp;

	CXMLElement* pGnutella = m_pXML->GetElementByName( _T("gnutella"), TRUE );
	VERIFY( pGnutella->AddAttribute( _T("guid"), tmp.toString() ) ) ;

	CreateBT();
}

void CGProfile::CreateBT()
{
	// Convert Gnutella GUID (128 bits) to BitTorrent GUID (160 bits)
	Hashes::BtGuid tmp_bt;
	CopyMemory( &tmp_bt[0], &((Hashes::Guid)oGUID)[0], ((Hashes::Guid)oGUID).byteCount );
	for ( size_t nByte = ((Hashes::Guid)oGUID).byteCount ; nByte < tmp_bt.byteCount ; nByte++ )
		tmp_bt[ nByte ] = GetRandomNum( 0ui8, _UI8_MAX );
	
	VERIFY( tmp_bt.validate() );
	oGUIDBT = tmp_bt;

	CXMLElement* pBitTorrent = m_pXML->GetElementByName( _T("bittorrent"), TRUE );
	VERIFY( pBitTorrent->AddAttribute( _T("guid"), tmp_bt.toString() ) );
}

//////////////////////////////////////////////////////////////////////
// CGProfile loading and saving

BOOL CGProfile::Load()
{
	const CXMLElement* pXML = CXMLElement::FromFile(
		Settings.General.UserPath + _T("\\Data\\Profile.xml"), TRUE );
	if ( pXML == NULL )
	{
		Create();

		return FALSE;
	}

	if ( ! FromXML( pXML ) )
	{
		delete pXML;
		return FALSE;
	}
		
	if ( ! (Hashes::BtGuid)oGUIDBT )
	{
		// Upgrade
		CreateBT();
		Save();
	}

	return TRUE;
}

BOOL CGProfile::Save()
{
	CFile pFile;
	if ( ! pFile.Open( Settings.General.UserPath + _T("\\Data\\Profile.xml"),
		CFile::modeWrite | CFile::modeCreate ) )
		return FALSE;

	CStringA strUTF8 = UTF8Encode( m_pXML->ToString( TRUE, TRUE ) );
	pFile.Write( (LPCSTR)strUTF8, strUTF8.GetLength() );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CGProfile XML assignment

BOOL CGProfile::FromXML(const CXMLElement* pXML)
{
	// Checking XML for validness
	if ( pXML == NULL ||
		 pXML->GetAttributeValue( _T("xmlns") ).CompareNoCase( xmlns ) ||
		 pXML->IsNamed( _T("gProfile") ) == FALSE )
		 return FALSE;

	// Loading Gnutella GUID (required)
	const CXMLElement* pGnutella = pXML->GetElementByName( _T("gnutella") );
	if ( pGnutella == NULL )
		return FALSE;

	Hashes::Guid tmp;
	if ( ! tmp.fromString( pGnutella->GetAttributeValue( _T("guid") ) ) )
		return FALSE;

	oGUID = tmp;

	// Loading BitTorrent GUID (optional)
	if ( const CXMLElement* pBitTorrent = pXML->GetElementByName( _T("bittorrent") ) )
	{
		Hashes::BtGuid tmp_bt;
		if ( tmp_bt.fromString( pBitTorrent->GetAttributeValue( _T("guid") ) ) )
			oGUIDBT = tmp_bt;
	}

	// Replace XML
	m_pXML.Free();
	m_pXML.Attach( const_cast< CXMLElement* >( pXML ) );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CGProfile field access

CString CGProfile::GetNick() const
{
	if ( const CXMLElement* pIdentity = m_pXML->GetElementByName( _T("identity") ) )
	{
		if ( const CXMLElement* pHandle = pIdentity->GetElementByName( _T("handle") ) )
		{
			return pHandle->GetAttributeValue( _T("primary") );
		}
	}
	return CString();
}

CString CGProfile::GetLocation() const
{
	if ( const CXMLElement* pLocation = m_pXML->GetElementByName( _T("location") ) )
	{
		if ( const CXMLElement* pPolitical = pLocation->GetElementByName( _T("political") ) )
		{
			CString strCity = pPolitical->GetAttributeValue( _T("city") );
			CString strState = pPolitical->GetAttributeValue( _T("state") );
			CString strCountry = pPolitical->GetAttributeValue( _T("country") );

			CString str = strCity;
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
	}
	return CString();
}

CString CGProfile::GetContact(LPCTSTR pszType) const
{
	if ( const CXMLElement* pContacts = m_pXML->GetElementByName( _T("contacts") ) )
	{
		for ( POSITION pos = pContacts->GetElementIterator() ; pos ; )
		{
			const CXMLElement* pGroup = pContacts->GetNextElement( pos );

			if ( pGroup->IsNamed( _T("group") ) &&
				 pGroup->GetAttributeValue( _T("class") ).CompareNoCase( pszType ) == 0 )
			{
				if ( const CXMLElement* pAddress = pGroup->GetElementByName( _T("address") ) )
				{
					return pAddress->GetAttributeValue( _T("content") );
				}
			}
		}
	}
	return CString();
}

DWORD CGProfile::GetPackedGPS() const
{
	if ( const CXMLElement* pLocation = m_pXML->GetElementByName( _T("location") ) )
	{
		if ( const CXMLElement* pCoordinates = pLocation->GetElementByName( _T("coordinates") ) )
		{
			float nLatitude = 0, nLongitude = 0;
			if ( _stscanf( pCoordinates->GetAttributeValue( _T("latitude") ) , _T("%f"), &nLatitude  ) == 1 &&
				 _stscanf( pCoordinates->GetAttributeValue( _T("longitude") ), _T("%f"), &nLongitude ) == 1 )
			{
				return ( (DWORD)WORDLIM( ( nLatitude  + 90.0f )  * 65535.0f / 180.0f ) << 16 ) +
						 (DWORD)WORDLIM( ( nLongitude + 180.0f ) * 65535.0f / 360.0f );
			}
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////
// CGProfile create avatar packet

CG2Packet* CGProfile::CreateAvatar() const
{
	const CXMLElement* pAvatar = m_pXML->GetElementByName( _T("avatar") );
	if ( pAvatar == NULL )
		return NULL;

	CString strPath = pAvatar->GetAttributeValue( _T("path") );
	if ( strPath.IsEmpty() )
		return NULL;

	CFile pFile;
	if ( ! pFile.Open( strPath, CFile::modeRead ) )
		return NULL;

	int nPos = strPath.ReverseFind( '\\' );
	if ( nPos >= 0 )
		strPath = strPath.Mid( nPos + 1 );

	CG2Packet* pPacket = CG2Packet::New( G2_PACKET_PROFILE_AVATAR );
	if ( ! pPacket )
		return NULL;

	pPacket->WritePacket( G2_PACKET_NAME, pPacket->GetStringLen( strPath ) );
	pPacket->WriteString( strPath, FALSE );

	pPacket->WritePacket( G2_PACKET_BODY, (DWORD)pFile.GetLength() );
	if ( LPBYTE pBody = pPacket->WriteGetPointer( (DWORD)pFile.GetLength() ) )
	{
		pFile.Read( pBody, (DWORD)pFile.GetLength() );
	}

	return pPacket;
}

//////////////////////////////////////////////////////////////////////
// CGProfile serialize

void CGProfile::Serialize(CArchive& ar, int /*nVersion*/ /* BROWSER_SER_VERSION */)
{
	BOOL bXMLPresent = FALSE;

	if ( ar.IsStoring() )
	{
		bXMLPresent = ( m_pXML->GetElementCount() != 0 );
		ar << bXMLPresent;
	}
	else
	{
		ar >> bXMLPresent;
	}
	if ( m_pXML && bXMLPresent )
		m_pXML->Serialize( ar );
}
