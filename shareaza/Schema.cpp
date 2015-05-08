//
// Schema.cpp
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
#include "Settings.h"
#include "Schema.h"
#include "SchemaMember.h"
#include "SchemaChild.h"
#include "ShellIcons.h"
#include "CoolInterface.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CSchema construction

CSchema::CSchema()
	: m_nType			( stFile )
	, m_nAvailability	( saDefault )
	, m_bPrivate		( FALSE )
	, m_nIcon16			( -1 )
	, m_nIcon32			( -1 )
	, m_nIcon48			( -1 )
{
	m_pTypeFilters.InitHashTable( 127 );
}

CSchema::~CSchema()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CSchema member access

POSITION CSchema::GetFilterIterator() const
{
	return m_pTypeFilters.GetStartPosition();
}

void CSchema::GetNextFilter(POSITION& pos, CString& sType, BOOL& bResult) const
{
	m_pTypeFilters.GetNextAssoc( pos, sType, bResult );
}

BOOL CSchema::FilterType(LPCTSTR pszFile) const
{
	if ( m_pTypeFilters.IsEmpty() )
		return FALSE;

	LPCTSTR pszExt = PathFindExtension( pszFile );
	if ( ! *pszExt )
		return FALSE;

	BOOL bValue;
	return m_pTypeFilters.Lookup( pszExt + 1, bValue ) ? bValue : FALSE;
}

CString CSchema::GetFilterSet() const
{
	CString sFilters = _T("|");
	for ( POSITION pos = m_pTypeFilters.GetStartPosition(); pos; )
	{
		CString sType;
		BOOL bResult;
		m_pTypeFilters.GetNextAssoc( pos, sType, bResult );
		if ( bResult )
		{
			sFilters += sType;
			sFilters += _T('|');
		}
	}
	return sFilters.MakeLower();
}

POSITION CSchema::GetMemberIterator() const
{
	return m_pMembers.GetHeadPosition();
}

CSchemaMemberPtr CSchema::GetNextMember(POSITION& pos) const
{
	return m_pMembers.GetNext( pos );
}

CSchemaMemberPtr CSchema::GetMember(LPCTSTR pszName) const
{
	if ( ! pszName || ! *pszName ) return NULL;

	for ( POSITION pos = m_pMembers.GetHeadPosition(); pos ; )
	{
		CSchemaMemberPtr pMember = m_pMembers.GetNext( pos );
		if ( pMember->m_sName.CompareNoCase( pszName ) == 0 ) return pMember;
	}
	return NULL;
}

CSchemaMember* CSchema::GetWritableMember(LPCTSTR pszName) const
{
	if ( !pszName || !*pszName ) return NULL;

	for ( POSITION pos = m_pMembers.GetHeadPosition(); pos; )
	{
		CSchemaMember* pMember = m_pMembers.GetNext( pos );
		if ( pMember->m_sName.CompareNoCase( pszName ) == 0 ) return pMember;
	}
	return NULL;
}

INT_PTR CSchema::GetMemberCount() const
{
	return m_pMembers.GetCount();
}

CString CSchema::GetFirstMemberName() const
{
	if ( m_pMembers.GetCount() )
	{
		CSchemaMemberPtr pMember = m_pMembers.GetHead();
		return pMember->m_sName;
	}

	return CString( _T("title") );
}

//////////////////////////////////////////////////////////////////////
// CSchema clear

void CSchema::Clear()
{
	for ( POSITION pos = m_pMembers.GetHeadPosition(); pos ; )
	{
		delete m_pMembers.GetNext( pos );
	}

	for ( POSITION pos = m_pContains.GetHeadPosition() ; pos ; )
	{
		delete m_pContains.GetNext( pos );
	}

	m_pMembers.RemoveAll();
	m_pContains.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// CSchema load

BOOL CSchema::Load(LPCTSTR pszFile)
{
	CString strFile( pszFile );
	PathRemoveExtension( strFile.GetBuffer() );
	strFile.ReleaseBuffer();

	if ( ! LoadSchema( strFile + _T(".xsd") ) ) return FALSE;
	
	m_sIcon = strFile + _T(".ico");

	LoadDescriptor( strFile + _T(".xml") );

	m_sIcon = m_sIcon.Left( m_sIcon.GetLength() - 4 );
	m_sIcon += _T("XP.ico");
	if ( ! LoadIcon() )
	{
		m_sIcon = m_sIcon.Left( m_sIcon.GetLength() - 6 );
		m_sIcon += _T(".ico");
		LoadIcon();
	}

	if ( m_sTitle.IsEmpty() )
	{
		m_sTitle = m_sSingular;
		m_sTitle.SetAt( 0, TCHAR( toupper( m_sTitle.GetAt( 0 ) ) ) );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSchema load schema

BOOL CSchema::LoadSchema(LPCTSTR pszFile)
{
	const CXMLElement* pRoot = CXMLElement::FromFile( pszFile );
	if ( NULL == pRoot ) return FALSE;

	BOOL bResult = FALSE;

	m_sURI = pRoot->GetAttributeValue( _T("targetNamespace"), _T("") );

	const CXMLElement* pPlural = pRoot->GetElementByName( _T("element") );

	if ( pPlural && m_sURI.GetLength() )
	{
		m_sPlural = pPlural->GetAttributeValue( _T("name") );

		const CXMLElement* pComplexType = pPlural->GetFirstElement();
		
		if ( pComplexType && pComplexType->IsNamed( _T("complexType") ) && m_sPlural.GetLength() )
		{
			const CXMLElement* pElement = pComplexType->GetFirstElement();

			if ( pElement && pElement->IsNamed( _T("element") ) )
			{
				m_sSingular = pElement->GetAttributeValue( _T("name") );

				if ( pElement->GetElementCount() )
				{
					bResult = LoadPrimary( pRoot, pElement->GetFirstElement() );
				}
				else
				{
					const CString strType = pElement->GetAttributeValue( _T("type") );
					bResult = LoadPrimary( pRoot, GetType( pRoot, strType ) );
				}

				if ( m_sSingular.IsEmpty() ) bResult = FALSE;
			}
		}
	}
	
	if ( const CXMLElement* pMapping = pRoot->GetElementByName( _T("mapping") ) )
	{
		for ( POSITION pos = pMapping->GetElementIterator() ; pos ; )
		{
			if ( const CXMLElement* pNetwork = pMapping->GetNextElement( pos ) )
			{
				BOOL bFound = pNetwork->IsNamed( _T("network") );

				const CString strName = pNetwork->GetAttributeValue( _T("name") );
				if ( ! bFound || strName != _T("ed2k") )
					continue;
				else
				{
					m_sDonkeyType = pNetwork->GetAttributeValue( _T("value") );
					break;
				}
			}
		}
	}

	delete pRoot;
	
	return bResult;
}

BOOL CSchema::LoadPrimary(const CXMLElement* pRoot, const CXMLElement* pType)
{
	if ( ! pRoot || ! pType ) return FALSE;
	
	if ( ! pType->IsNamed( _T("complexType") ) &&
		 ! pType->IsNamed( _T("all") ) ) return FALSE;

	for ( POSITION pos = pType->GetElementIterator() ; pos ; )
	{
		const CXMLElement* pElement	= pType->GetNextElement( pos );
		const CString strElement = pElement->GetName();

		if ( strElement.CompareNoCase( _T("attribute") ) == 0 ||
			 strElement.CompareNoCase( _T("element") ) == 0 )
		{
			CSchemaMember* pMember = new CSchemaMember( this );

			if ( pMember->LoadSchema( pRoot, pElement ) )
			{
				m_pMembers.AddTail( pMember );
			}
			else
			{
				delete pMember;
				return FALSE;
			}
		}
		else if ( strElement.CompareNoCase( _T("all") ) == 0 )
		{
			if ( ! LoadPrimary( pRoot, pElement ) ) return FALSE;
		}
	}

	return TRUE;
}

const CXMLElement* CSchema::GetType(const CXMLElement* pRoot, LPCTSTR pszName) const
{
	if ( ! pszName || ! *pszName ) return NULL;

	for ( POSITION pos = pRoot->GetElementIterator() ; pos ; )
	{
		const CXMLElement* pElement = pRoot->GetNextElement( pos );

		const CString strElement = pElement->GetName();

		if ( strElement.CompareNoCase( _T("simpleType") ) == 0 ||
			 strElement.CompareNoCase( _T("complexType") ) == 0 )
		{
			if ( pElement->GetAttributeValue( _T("name"), _T("?") ).CompareNoCase( pszName ) == 0 )
			{
				return pElement;
			}
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CSchema load descriptor

BOOL CSchema::LoadDescriptor(LPCTSTR pszFile)
{
	CXMLElement* pRoot = CXMLElement::FromFile( pszFile );
	if ( NULL == pRoot ) return FALSE;
	
	if ( ! CheckURI( pRoot->GetAttributeValue( _T("location") ) ) ||
		 ! pRoot->IsNamed( _T("schemaDescriptor") ) )
	{
		delete pRoot;
		return FALSE;
	}
	
	for ( POSITION pos = pRoot->GetElementIterator() ; pos ; )
	{
		const CXMLElement* pElement = pRoot->GetNextElement( pos );
		
		if ( pElement->IsNamed( _T("object") ) )
		{
			const CString strType = pElement->GetAttributeValue( _T("type") );
			if ( strType.CompareNoCase( _T("file") ) == 0 )
				m_nType = stFile;
			else if ( strType.CompareNoCase( _T("folder") ) == 0 || strType.CompareNoCase( _T("album") ) == 0 )
				m_nType = stFolder;
			
			const CString strAvailability = pElement->GetAttributeValue( _T("availability") );
			if ( strAvailability.CompareNoCase( _T("system") ) == 0 )
				m_nAvailability = saSystem;
			else if ( strAvailability.CompareNoCase( _T("advanced") ) == 0 )
				m_nAvailability = saAdvanced;
			else
				m_nAvailability = saDefault;
			
			if ( pElement->GetAttribute( _T("private") ) ) m_bPrivate = TRUE;
		}
		else if ( pElement->IsNamed( _T("titles") ) )
		{
			LoadDescriptorTitles( pElement );
		}
		else if ( pElement->IsNamed( _T("images") ) )
		{
			LoadDescriptorIcons( pElement );
		}
		else if ( pElement->IsNamed( _T("members") ) )
		{
			LoadDescriptorMembers( pElement );
		}
		else if ( pElement->IsNamed( _T("extends") ) )
		{
			LoadDescriptorExtends( pElement );
		}
		else if ( pElement->IsNamed( _T("contains") ) )
		{
			LoadDescriptorContains( pElement );
		}
		else if ( pElement->IsNamed( _T("typeFilter") ) )
		{
			LoadDescriptorTypeFilter( pElement );
		}
		else if ( pElement->IsNamed( _T("headerContent") ) )
		{
			LoadDescriptorHeaderContent( pElement );
		}
		else if ( pElement->IsNamed( _T("viewContent") ) )
		{
			LoadDescriptorViewContent( pElement );
		}/* // ToDo: Add this to schemas
		else if ( pElement->IsNamed( _T("donkeyType") ) )
		{
			LoadDescriptorDonkeyType( pElement );
		}*/
	}

	delete pRoot;

	return TRUE;
}

void CSchema::LoadDescriptorTitles(const CXMLElement* pElement)
{
	for ( POSITION pos = pElement->GetElementIterator() ; pos ; )
	{
		const CXMLElement* pTitle = pElement->GetNextElement( pos );

		if ( pTitle->IsNamed( _T("title") ) )
		{
			if ( pTitle->GetAttributeValue( _T("language") ).
					CompareNoCase( Settings.General.Language ) == 0 )
			{
				m_sTitle = pTitle->GetValue();
				break;
			}
			else if ( m_sTitle.IsEmpty() )
			{
				m_sTitle = pTitle->GetValue();
			}
		}
	}
}

void CSchema::LoadDescriptorIcons(const CXMLElement* pElement)
{
	for ( POSITION pos = pElement->GetElementIterator() ; pos ; )
	{
		const CXMLElement* pIcon = pElement->GetNextElement( pos );

		if ( pIcon->IsNamed( _T("icon") ) )
		{
			int nSlash = m_sIcon.ReverseFind( '\\' );
			if ( nSlash >= 0 ) m_sIcon = m_sIcon.Left( nSlash + 1 );
			m_sIcon += pIcon->GetAttributeValue( _T("path") );
		}
	}
}

void CSchema::LoadDescriptorMembers(const CXMLElement* pElement)
{
	BOOL bPrompt = FALSE;
	
	for ( POSITION pos = pElement->GetElementIterator() ; pos ; )
	{
		const CXMLElement* pDisplay = pElement->GetNextElement( pos );
		
		if ( pDisplay->IsNamed( _T("member") ) )
		{
			const CString strMember = pDisplay->GetAttributeValue( _T("name") );
			
			if ( CSchemaMember* pMember = GetWritableMember( strMember ) )
			{
				pMember->LoadDescriptor( pDisplay );
				bPrompt |= pMember->m_bPrompt;
			}
		}
	}
	
	if ( bPrompt ) return;
	
	for ( POSITION pos = m_pMembers.GetHeadPosition(); pos ; )
	{
		m_pMembers.GetNext( pos )->m_bPrompt = TRUE;
	}
}

void CSchema::LoadDescriptorExtends(const CXMLElement* pElement)
{
	for ( POSITION pos = pElement->GetElementIterator() ; pos ; )
	{
		const CXMLElement* pExtend = pElement->GetNextElement( pos );

		if ( pExtend->IsNamed( _T("schema") ) )
		{
			CString strURI = pExtend->GetAttributeValue( _T("location") );
			if ( strURI.GetLength() ) m_pExtends.AddTail( strURI );
		}
	}
}

void CSchema::LoadDescriptorContains(const CXMLElement* pElement)
{
	for ( POSITION pos = pElement->GetElementIterator() ; pos ; )
	{
		const CXMLElement* pExtend = pElement->GetNextElement( pos );

		if ( pExtend->IsNamed( _T("object") ) )
		{
			CSchemaChild* pChild = new CSchemaChild( this );

			if ( pChild && pChild->Load( pExtend ) )
			{
				m_pContains.AddTail( pChild );
			}
			else
			{
				delete pChild;
			}
		}
	}
}

void CSchema::LoadDescriptorTypeFilter(const CXMLElement* pElement)
{
	for ( POSITION pos = pElement->GetElementIterator() ; pos ; )
	{
		const CXMLElement* pType = pElement->GetNextElement( pos );

		if ( pType->GetName().CompareNoCase( _T("type") ) == 0 )
		{
			m_pTypeFilters.SetAt( pType->GetAttributeValue( _T("extension") ), TRUE );
		}
	}
}

void CSchema::LoadDescriptorHeaderContent(const CXMLElement* pElement)
{
	for ( POSITION pos = pElement->GetElementIterator() ; pos ; )
	{
		const CXMLElement* pXML = pElement->GetNextElement( pos );
		
		BOOL bLanguage = pXML->GetAttributeValue( _T("language") ).
			CompareNoCase( Settings.General.Language ) == 0;

		if ( pXML->IsNamed( _T("title") ) )
		{
			if ( bLanguage || m_sHeaderTitle.IsEmpty() )
				m_sHeaderTitle = pXML->GetValue().Trim();
		}
		else if ( pXML->IsNamed( _T("subtitle") ) )
		{
			if ( bLanguage || m_sHeaderSubtitle.IsEmpty() )
				m_sHeaderSubtitle = pXML->GetValue().Trim();
		}
	}
}

void CSchema::LoadDescriptorViewContent(const CXMLElement* pElement)
{
	m_sLibraryView = pElement->GetAttributeValue( _T("preferredView") );
	
	for ( POSITION pos = pElement->GetElementIterator() ; pos ; )
	{
		const CXMLElement* pXML = pElement->GetNextElement( pos );

		BOOL bLanguage = pXML->GetAttributeValue( _T("language") ).
			CompareNoCase( Settings.General.Language ) == 0;
		
		if ( pXML->IsNamed( _T("tileLine1") ) )
		{
			if ( bLanguage || m_sTileLine1.IsEmpty() )
				m_sTileLine1 = pXML->GetValue().Trim();
		}
		else if ( pXML->IsNamed( _T("tileLine2") ) )
		{
			if ( bLanguage || m_sTileLine2.IsEmpty() )
				m_sTileLine2 = pXML->GetValue().Trim();
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CSchema load icon

BOOL CSchema::LoadIcon()
{
	HICON hIcon16 = NULL, hIcon32 = NULL, hIcon48 = NULL;

	::LoadIcon( m_sIcon, &hIcon16, &hIcon32, &hIcon48 );

	if ( hIcon16 )
	{
		if ( Settings.General.LanguageRTL ) hIcon16 = CreateMirroredIcon( hIcon16 );
		m_nIcon16 = ShellIcons.Add( hIcon16, 16 );
		DestroyIcon( hIcon16 );
	}
	if ( hIcon32 )
	{
		if ( Settings.General.LanguageRTL ) hIcon32 = CreateMirroredIcon( hIcon32 );
		m_nIcon32 = ShellIcons.Add( hIcon32, 32 );
		DestroyIcon( hIcon32 );
	}
	if ( hIcon48 )
	{
		if ( Settings.General.LanguageRTL ) hIcon48 = CreateMirroredIcon( hIcon48 );
		m_nIcon48 = ShellIcons.Add( hIcon48, 48 );
		DestroyIcon( hIcon48 );
	}

	return hIcon16 || hIcon32 || hIcon48;
}

//////////////////////////////////////////////////////////////////////
// CSchema contained object helpers

CSchemaChildPtr CSchema::GetContained(LPCTSTR pszURI) const
{
	for ( POSITION pos = m_pContains.GetHeadPosition() ; pos ; )
	{
		CSchemaChildPtr pChild = m_pContains.GetNext( pos );
		if ( pChild->m_sURI.CompareNoCase( pszURI ) == 0 ) return pChild;
	}
	return NULL;
}

CString CSchema::GetContainedURI(Type nType) const
{
	for ( POSITION pos = m_pContains.GetHeadPosition() ; pos ; )
	{
		CSchemaChildPtr pChild = m_pContains.GetNext( pos );
		if ( pChild->m_nType == nType ) return pChild->m_sURI;
	}
	return CString();
}

//////////////////////////////////////////////////////////////////////
// CSchema instantiate

CXMLElement* CSchema::Instantiate(BOOL bNamespace) const
{
	CXMLElement* pElement = new CXMLElement( NULL, m_sPlural );
	pElement->AddAttribute( CXMLAttribute::schemaName, m_sURI );
	if ( bNamespace ) pElement->AddAttribute( _T("xmlns:xsi"), CXMLAttribute::xmlnsInstance );
	return pElement;
}

//////////////////////////////////////////////////////////////////////
// CSchema validate instance

BOOL CSchema::Validate(CXMLElement* pXML, BOOL bFix) const
{
	if ( pXML == NULL ) return FALSE;
	
	if ( ! pXML->IsNamed( m_sPlural ) ) return FALSE;
	if ( ! CheckURI( pXML->GetAttributeValue( CXMLAttribute::schemaName ) ) ) return FALSE;
	
	CXMLElement* pBody = pXML->GetFirstElement();
	if ( pBody == NULL ) return FALSE;
	if ( ! pBody->IsNamed( m_sSingular ) ) return FALSE;
	
	for ( POSITION pos = m_pMembers.GetHeadPosition(); pos ; )
	{
		CSchemaMemberPtr pMember = m_pMembers.GetNext( pos );
		
		CString str = pMember->GetValueFrom( pBody, NO_VALUE, FALSE );
		if ( str == NO_VALUE ) continue;
		
		if ( pMember->m_bNumeric )
		{
			float nNumber = 0.0f;
			bool bValid = true;

			if ( str.GetLength() && _stscanf( str, L"%f", &nNumber ) != 1 ) 
				bValid = false;
			if ( nNumber < pMember->m_nMinOccurs || nNumber > pMember->m_nMaxOccurs )
				bValid = false;
			if ( !bValid )
			{
				if ( !bFix ) return FALSE;
				pMember->SetValueTo( pBody );
			}
		}
		else if ( pMember->m_bYear )
		{
			int nYear = 0;
			if ( _stscanf( str, L"%i", &nYear ) != 1 || nYear < 1000 || nYear > 9999 )
			{
				if ( !bFix ) return FALSE;
				pMember->SetValueTo( pBody );
			}
		}
		else if ( pMember->m_bGUID )
		{
			Hashes::Guid tmp;
			if ( !(Hashes::fromGuid( str, &tmp[ 0 ] ) && tmp.validate() ) )
			{
				if ( !bFix ) return FALSE;
				pMember->SetValueTo( pBody );
			}
		}
		else if ( pMember->m_nMaxLength > 0 )
		{
			if ( str.GetLength() > pMember->m_nMaxLength )
			{
				if ( ! bFix ) return FALSE;
				
				str = str.Left( pMember->m_nMaxLength );
				pMember->SetValueTo( pBody, str );
			}
		}
		else if ( pMember->m_bBoolean )
		{
			if ( str == L"1" || str.CompareNoCase( L"true" ) == 0 )
				str = L"true";
			else if ( str == L"0" || str.CompareNoCase( L"false" ) == 0 )
				str = L"false";
			else if ( !bFix ) return FALSE;
			pMember->SetValueTo( pBody );
		}
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSchema indexed words

CString CSchema::GetIndexedWords(const CXMLElement* pXML) const
{
	CString str;
	
	if ( pXML == NULL ) return str;
	
	for ( POSITION pos = m_pMembers.GetHeadPosition(); pos ; )
	{
		CSchemaMemberPtr pMember = m_pMembers.GetNext( pos );
		
		if ( pMember->m_bIndexed )
		{
			CString strMember = pMember->GetValueFrom( pXML, NULL, FALSE );
			
			if ( strMember.GetLength() )
			{
				if ( str.GetLength() ) str += _T(' ');
				str += strMember;
			}
		}
	}
	ToLower( str );	
	return str;
}

CString CSchema::GetVisibleWords(const CXMLElement* pXML) const
{
	CString str;
	
	if ( pXML == NULL ) return str;
	
	for ( POSITION pos = m_pMembers.GetHeadPosition(); pos ; )
	{
		CSchemaMemberPtr pMember = m_pMembers.GetNext( pos );
		
		if ( !pMember->m_bHidden )
		{
			CString strMember = pMember->GetValueFrom( pXML, NULL, FALSE );
			
			if ( strMember.GetLength() )
			{
				if ( str.GetLength() ) str += ' ';
				str += strMember;
			}
		}
	}
	return str;
}

//////////////////////////////////////////////////////////////////////
// CSchema member resolution

void CSchema::ResolveTokens(CString& str, CXMLElement* pXML) const
{
	for ( ;; )
	{
		int nOpen = str.Find( '{' );
		if ( nOpen < 0 ) break;
		int nClose = str.Find( '}' );
		if ( nClose <= nOpen ) break;
		
		CString strMember = str.Mid( nOpen + 1, nClose - nOpen - 1 );
		strMember.TrimLeft(); strMember.TrimRight();

		CString strValue;

		if ( CSchemaMemberPtr pMember = GetMember( strMember ) )
		{
			strValue = pMember->GetValueFrom( pXML, NULL, TRUE );
		}

		str = str.Left( nOpen ) + strValue + str.Mid( nClose + 1 );
	}
}

//////////////////////////////////////////////////////////////////////
// CSchema common schema URIs

LPCTSTR	CSchema::uriApplication				= _T("http://www.shareaza.com/schemas/application.xsd");
LPCTSTR	CSchema::uriAudio					= _T("http://www.limewire.com/schemas/audio.xsd");
LPCTSTR CSchema::uriArchive					= _T("http://www.shareaza.com/schemas/archive.xsd");
LPCTSTR	CSchema::uriBook					= _T("http://www.limewire.com/schemas/book.xsd");
LPCTSTR	CSchema::uriImage					= _T("http://www.shareaza.com/schemas/image.xsd");
LPCTSTR	CSchema::uriVideo					= _T("http://www.limewire.com/schemas/video.xsd");
LPCTSTR	CSchema::uriROM						= _T("http://www.shareaza.com/schemas/rom.xsd");
LPCTSTR	CSchema::uriDocument				= _T("http://www.shareaza.com/schemas/wordProcessing.xsd");
LPCTSTR	CSchema::uriSpreadsheet				= _T("http://www.shareaza.com/schemas/spreadsheet.xsd");
LPCTSTR CSchema::uriPresentation			= _T("http://www.shareaza.com/schemas/presentation.xsd");
LPCTSTR CSchema::uriCollection				= _T("http://www.shareaza.com/schemas/collection.xsd");

LPCTSTR CSchema::uriLibrary					= _T("http://www.shareaza.com/schemas/libraryRoot.xsd");

LPCTSTR CSchema::uriFolder					= _T("http://www.shareaza.com/schemas/folder.xsd");
LPCTSTR CSchema::uriCollectionsFolder		= _T("http://www.shareaza.com/schemas/collectionsFolder.xsd");
LPCTSTR CSchema::uriFavouritesFolder		= _T("http://www.shareaza.com/schemas/favouritesFolder.xsd");
LPCTSTR CSchema::uriSearchFolder			= _T("http://www.shareaza.com/schemas/searchFolder.xsd");
LPCTSTR CSchema::uriAllFiles				= _T("http://www.shareaza.com/schemas/allFiles.xsd");

LPCTSTR	CSchema::uriApplicationRoot			= _T("http://www.shareaza.com/schemas/applicationRoot.xsd");
LPCTSTR	CSchema::uriApplicationAll			= _T("http://www.shareaza.com/schemas/applicationAll.xsd");

LPCTSTR	CSchema::uriBookRoot				= _T("http://www.shareaza.com/schemas/bookRoot.xsd");
LPCTSTR	CSchema::uriBookAll					= _T("http://www.shareaza.com/schemas/bookAll.xsd");

LPCTSTR	CSchema::uriImageRoot				= _T("http://www.shareaza.com/schemas/imageRoot.xsd");
LPCTSTR	CSchema::uriImageAll				= _T("http://www.shareaza.com/schemas/imageAll.xsd");
LPCTSTR	CSchema::uriImageAlbum				= _T("http://www.shareaza.com/schemas/imageAlbum.xsd");

LPCTSTR	CSchema::uriMusicRoot				= _T("http://www.shareaza.com/schemas/musicRoot.xsd");
LPCTSTR	CSchema::uriMusicAll				= _T("http://www.shareaza.com/schemas/musicAll.xsd");
LPCTSTR	CSchema::uriMusicAlbumCollection	= _T("http://www.shareaza.com/schemas/musicAlbumCollection.xsd");
LPCTSTR	CSchema::uriMusicArtistCollection	= _T("http://www.shareaza.com/schemas/musicArtistCollection.xsd");
LPCTSTR	CSchema::uriMusicGenreCollection	= _T("http://www.shareaza.com/schemas/musicGenreCollection.xsd");
LPCTSTR	CSchema::uriMusicAlbum				= _T("http://www.shareaza.com/schemas/musicAlbum.xsd");
LPCTSTR	CSchema::uriMusicArtist				= _T("http://www.shareaza.com/schemas/musicArtist.xsd");
LPCTSTR	CSchema::uriMusicGenre				= _T("http://www.shareaza.com/schemas/musicGenre.xsd");

LPCTSTR	CSchema::uriVideoRoot				= _T("http://www.shareaza.com/schemas/videoRoot.xsd");
LPCTSTR	CSchema::uriVideoAll				= _T("http://www.shareaza.com/schemas/videoAll.xsd");
LPCTSTR	CSchema::uriVideoSeriesCollection	= _T("http://www.shareaza.com/schemas/videoSeriesCollection.xsd");
LPCTSTR	CSchema::uriVideoSeries				= _T("http://www.shareaza.com/schemas/videoSeries.xsd");
LPCTSTR	CSchema::uriVideoFilmCollection		= _T("http://www.shareaza.com/schemas/videoFilmCollection.xsd");
LPCTSTR	CSchema::uriVideoFilm				= _T("http://www.shareaza.com/schemas/videoFilm.xsd");
LPCTSTR	CSchema::uriVideoMusicCollection	= _T("http://www.shareaza.com/schemas/videoMusicCollection.xsd");

LPCTSTR	CSchema::uriDocumentRoot			= _T("http://www.shareaza.com/schemas/documentRoot.xsd");
LPCTSTR	CSchema::uriDocumentAll				= _T("http://www.shareaza.com/schemas/documentAll.xsd");

LPCTSTR	CSchema::uriGhostFolder				= _T("http://www.shareaza.com/schemas/ghostFolder.xsd");

LPCTSTR CSchema::uriComments				= _T("http://www.shareaza.com/schemas/comments.xsd");

LPCTSTR CSchema::uriBitTorrent				= _T("http://www.shareaza.com/schemas/bittorrent.xsd");
