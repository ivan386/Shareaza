//
// Security.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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
#include "Settings.h"
#include "Security.h"
#include "Network.h"
#include "Buffer.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CSecurity Security;


//////////////////////////////////////////////////////////////////////
// CSecurity construction

CSecurity::CSecurity()
{
	m_bDenyPolicy = FALSE;
}

CSecurity::~CSecurity()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CSecurity rule access

POSITION CSecurity::GetIterator() const
{
	return m_pRules.GetHeadPosition();
}

CSecureRule* CSecurity::GetNext(POSITION& pos) const
{
	return (CSecureRule*)m_pRules.GetNext( pos );
}

int CSecurity::GetCount()
{
	return m_pRules.GetCount();
}

BOOL CSecurity::Check(CSecureRule* pRule) const
{
	return pRule && ( m_pRules.Find( pRule ) != NULL );
}

CSecureRule* CSecurity::GetGUID(const GUID& pGUID) const
{
	for ( POSITION pos = m_pRules.GetHeadPosition() ; pos ; )
	{
		CSecureRule* pRule = (CSecureRule*)m_pRules.GetNext( pos );
		if ( pRule->m_pGUID == pGUID ) return pRule;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CSecurity rule modification

void CSecurity::Add(CSecureRule* pRule)
{
	pRule->MaskFix();

	POSITION pos = m_pRules.Find( pRule );
	if ( pos == NULL ) m_pRules.AddHead( pRule );
}

void CSecurity::Remove(CSecureRule* pRule)
{
	POSITION pos = m_pRules.Find( pRule );
	if ( pos ) m_pRules.RemoveAt( pos );
	delete pRule;
}

void CSecurity::MoveUp(CSecureRule* pRule)
{
	POSITION posMe = m_pRules.Find( pRule );
	if ( posMe == NULL ) return;
	
	POSITION posOther = posMe;
	m_pRules.GetPrev( posOther );

	if ( posOther )
	{
		m_pRules.InsertBefore( posOther, pRule );
		m_pRules.RemoveAt( posMe );
	}
}

void CSecurity::MoveDown(CSecureRule* pRule)
{
	POSITION posMe = m_pRules.Find( pRule );
	if ( posMe == NULL ) return;
	
	POSITION posOther = posMe;
	m_pRules.GetNext( posOther );

	if ( posOther )
	{
		m_pRules.InsertAfter( posOther, pRule );
		m_pRules.RemoveAt( posMe );
	}
}

void CSecurity::Clear()
{
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		delete GetNext( pos );
	}

	m_pRules.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// CSecurity session ban

void CSecurity::SessionBan(IN_ADDR* pAddress, BOOL bMessage)
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return;

	CString strAddress = inet_ntoa( *pAddress );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CSecureRule* pRule = GetNext( pos );

		if ( pRule->Match( pAddress ) )
		{
			if ( pRule->m_nAction == CSecureRule::srDeny )
			{
				if ( bMessage )
				{
					theApp.Message( MSG_SYSTEM, IDS_NETWORK_SECURITY_ALREADY_BLOCKED,
						(LPCTSTR)strAddress );
				}

				return;
			}
		}
	}

	CSecureRule* pRule	= new CSecureRule();
	pRule->m_nAction	= CSecureRule::srDeny;
	pRule->m_nExpire	= CSecureRule::srSession;
	pRule->m_sComment	= _T("Quick Ban");
	CopyMemory( pRule->m_nIP, pAddress, 4 );
	Add( pRule );

	if ( bMessage )
	{
		theApp.Message( MSG_SYSTEM, IDS_NETWORK_SECURITY_BLOCKED,
			(LPCTSTR)strAddress );
	}
}

//////////////////////////////////////////////////////////////////////
// CSecurity access check

BOOL CSecurity::IsDenied(IN_ADDR* pAddress, LPCTSTR pszContent)
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 50 ) ) return FALSE;

	DWORD nNow = time( NULL );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		POSITION posLast = pos;
		CSecureRule* pRule = GetNext( pos );

		if ( pRule->m_nExpire && pRule->IsExpired( nNow ) )
		{
			m_pRules.RemoveAt( posLast );
			delete pRule;
		}
		else if ( pRule->Match( pAddress, pszContent ) )
		{
			pRule->m_nToday ++;
			pRule->m_nEver ++;

			if ( pRule->m_nAction == CSecureRule::srAccept ) return FALSE;
			else if ( pRule->m_nAction == CSecureRule::srDeny ) return TRUE;
		}
	}

	return m_bDenyPolicy;
}

BOOL CSecurity::IsAccepted(IN_ADDR* pAddress, LPCTSTR pszContent)
{
	return ! IsDenied( pAddress, pszContent );
}

//////////////////////////////////////////////////////////////////////
// CSecurity expire

void CSecurity::Expire()
{
	DWORD nNow = time( NULL );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		POSITION posLast = pos;
		CSecureRule* pRule = GetNext( pos );

		if ( pRule->m_nExpire && pRule->IsExpired( nNow ) )
		{
			m_pRules.RemoveAt( posLast );
			delete pRule;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CSecurity load and save

BOOL CSecurity::Load()
{
	CFile pFile;

	CString strFile = Settings.General.Path + _T("\\Data\\Security.dat");

	if ( ! pFile.Open( strFile, CFile::modeRead ) ) return FALSE;
	
	try
	{
		CArchive ar( &pFile, CArchive::load );
		Serialize( ar );
		ar.Close();
	}
	catch ( CException* pException )
	{
		pException->Delete();
	}

	pFile.Close();

	return TRUE;
}

BOOL CSecurity::Save(BOOL bLock)
{
	if ( bLock ) bLock = Network.m_pSection.Lock( 250 );

	CFile pFile;

	CString strFile = Settings.General.Path + _T("\\Data\\Security.dat");

	if ( pFile.Open( strFile, CFile::modeWrite|CFile::modeCreate ) )
	{
		CArchive ar( &pFile, CArchive::store );
		Serialize( ar );
		ar.Close();
	}

	if ( bLock ) Network.m_pSection.Unlock();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSecurity serialize

void CSecurity::Serialize(CArchive& ar)
{
	int nVersion = 4;

	if ( ar.IsStoring() )
	{
		ar << nVersion;
		ar << m_bDenyPolicy;

		ar.WriteCount( GetCount() );

		for ( POSITION pos = GetIterator() ; pos ; )
		{
			CSecureRule* pRule = GetNext( pos );
			pRule->Serialize( ar, nVersion );
		}
	}
	else
	{
		Clear();

		ar >> nVersion;
		ar >> m_bDenyPolicy;

		DWORD nNow = time( NULL );

		for ( int nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
		{
			CSecureRule* pRule = new CSecureRule( FALSE );
			pRule->Serialize( ar, nVersion );

			if ( pRule->IsExpired( nNow, TRUE ) )
				delete pRule;
			else
				m_pRules.AddTail( pRule );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CSecurity XML

LPCTSTR CSecurity::xmlns = _T("http://www.shareaza.com/schemas/Security.xsd");

CXMLElement* CSecurity::ToXML(BOOL bRules)
{
	CXMLElement* pXML = new CXMLElement( NULL, _T("security") );
	pXML->AddAttribute( _T("xmlns"), CSecurity::xmlns );

	if ( bRules )
	{
		for ( POSITION pos = GetIterator() ; pos ; )
		{
			pXML->AddElement( GetNext( pos )->ToXML() );
		}
	}

	return pXML;
}

BOOL CSecurity::FromXML(CXMLElement* pXML)
{
	if ( ! pXML->IsNamed( _T("security") ) ) return FALSE;

	int nCount = 0;

	for ( POSITION pos = pXML->GetElementIterator() ; pos ; )
	{
		CXMLElement* pElement = pXML->GetNextElement( pos );

		if ( pElement->IsNamed( _T("rule") ) )
		{
			CSecureRule* pRule	= NULL;
			CString strGUID		= pElement->GetAttributeValue( _T("guid") );
			BOOL bExisting		= FALSE;
			GUID pGUID;
			
			if ( GUIDX::Decode( strGUID, &pGUID ) )
			{
				if ( pRule = GetGUID( pGUID ) ) bExisting = TRUE;
				
				if ( pRule == NULL )
				{
					pRule = new CSecureRule( FALSE );
					pRule->m_pGUID = pGUID;
				}
			}
			else
			{
				pRule = new CSecureRule();
			}

			if ( pRule->FromXML( pElement ) )
			{
				if ( ! bExisting ) m_pRules.AddTail( pRule );
				nCount++;
			}
			else
			{
				if ( ! bExisting ) delete pRule;
			}
		}
	}

	return nCount > 0;
}

//////////////////////////////////////////////////////////////////////
// CSecurity import

BOOL CSecurity::Import(LPCTSTR pszFile)
{
	CSingleLock pLock( &Network.m_pSection );
	if ( ! pLock.Lock( 250 ) ) return FALSE;

	CString strText;
	CBuffer pBuffer;
	CFile pFile;

	if ( ! pFile.Open( pszFile, CFile::modeRead ) ) return FALSE;
	pBuffer.EnsureBuffer( (DWORD)pFile.GetLength() );
	pBuffer.m_nLength = (DWORD)pFile.GetLength();
	pFile.Read( pBuffer.m_pBuffer, pBuffer.m_nLength );
	pFile.Close();
	
	CXMLElement* pXML = CXMLElement::FromBytes( pBuffer.m_pBuffer, pBuffer.m_nLength, TRUE );
	BOOL bResult = FALSE;
	
	if ( pXML != NULL )
	{
		bResult = FromXML( pXML );
		delete pXML;
	}
	else
	{
		CString strLine;
		
		while ( pBuffer.ReadLine( strLine ) )
		{
			strLine.TrimLeft();
			strLine.TrimRight();
			if ( strLine.IsEmpty() ) continue;
			if ( strLine.GetAt( 0 ) == ';' ) continue;

			CSecureRule* pRule = new CSecureRule();

			if ( pRule->FromGnucleusString( strLine ) )
			{
				m_pRules.AddTail( pRule );
				bResult = TRUE;
			}
			else
			{
				delete pRule;
			}
		}
	}
	
	return bResult;
}

//////////////////////////////////////////////////////////////////////
// CSecureRule construction

CSecureRule::CSecureRule(BOOL bCreate)
{
	m_nType		= srAddress;
	m_nAction	= ( Security.m_bDenyPolicy ? srAccept : srDeny );
	m_nExpire	= srIndefinite;
	m_nToday	= 0;
	m_nEver		= 0;

	m_nIP[0]	= m_nIP[1] = m_nIP[2] = m_nIP[3] = 0;
	m_nMask[0]	= m_nMask[1] = m_nMask[2] = m_nMask[3] = 255;
	m_pContent	= NULL;

	if ( bCreate ) CoCreateGuid( &m_pGUID );
}

CSecureRule::~CSecureRule()
{
	if ( m_pContent ) delete [] m_pContent;
}

//////////////////////////////////////////////////////////////////////
// CSecureRule remove and reset

void CSecureRule::Remove()
{
	Security.Remove( this );
}

void CSecureRule::Reset()
{
	m_nToday = m_nEver = 0;
}

//////////////////////////////////////////////////////////////////////
// CSecureRule expiry check

BOOL CSecureRule::IsExpired(DWORD nNow, BOOL bSession)
{
	if ( m_nExpire == srIndefinite ) return FALSE;
	if ( m_nExpire == srSession ) return bSession;
	return m_nExpire < nNow;
}

//////////////////////////////////////////////////////////////////////
// CSecureRule match

BOOL CSecureRule::Match(IN_ADDR* pAddress, LPCTSTR pszContent)
{
	if ( m_nExpire > srSession )
	{
		if ( m_nExpire <= (DWORD)time( NULL ) ) return FALSE;
	}

	if ( m_nType == srAddress && pAddress != NULL )
	{
		DWORD* pBase = (DWORD*)m_nIP;
		DWORD* pMask = (DWORD*)m_nMask;
		DWORD* pTest = (DWORD*)pAddress;

// This only works if IP's are &ed before entered in the list
		if ( ( ( *pTest ) & ( *pMask ) ) == ( *pBase ) )
		{
			return TRUE;
		}
	}
	else if ( m_nType == srContent && pszContent != NULL && m_pContent != NULL )
	{
		for ( LPCTSTR pszFilter = m_pContent ; *pszFilter ; )
		{
			BOOL bFound = _tcsistr( pszContent, pszFilter ) != NULL;

			if ( bFound && m_nIP[0] == 0 )
			{
				return TRUE;
			}
			else if ( ! bFound && m_nIP[0] == 1 )
			{
				return FALSE;
			}

			pszFilter += _tcslen( pszFilter ) + 1;
		}

		if ( m_nIP[0] == 1 ) return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CSecureRule content list helpers

void CSecureRule::SetContentWords(const CString& strContent)
{
	LPTSTR pszContent	= (LPTSTR)(LPCTSTR)strContent;
	int nTotalLength	= 3;
	CStringList pWords;

	for ( int nStart = 0, nPos = 0 ; *pszContent ; nPos++, pszContent++ )
	{
		if ( *pszContent == ' ' || *pszContent == '\t' )
		{
			if ( nStart < nPos )
			{
				pWords.AddTail( strContent.Mid( nStart, nPos - nStart ) );
				nTotalLength += nPos - nStart + 1;
			}
			nStart = nPos + 1;
		}
	}

	if ( nStart < nPos )
	{
		pWords.AddTail( strContent.Mid( nStart, nPos - nStart ) );
		nTotalLength += nPos - nStart + 1;
	}

	if ( m_pContent )
	{
		delete [] m_pContent;
		m_pContent = NULL;
	}

	if ( pWords.IsEmpty() ) return;

	m_pContent	= new TCHAR[ nTotalLength ];
	pszContent	= m_pContent;

	for ( POSITION pos = pWords.GetHeadPosition() ; pos ; )
	{
		CString strWord = pWords.GetNext( pos );
		CopyMemory( pszContent, (LPCTSTR)strWord, ( strWord.GetLength() + 1 ) * sizeof(TCHAR) );
		pszContent += strWord.GetLength() + 1;
	}

	*pszContent++ = 0;
	*pszContent++ = 0;
}

CString CSecureRule::GetContentWords()
{
	CString strWords;

	if ( m_pContent == NULL ) return strWords;

	for ( LPCTSTR pszFilter = m_pContent ; *pszFilter ; )
	{
		if ( strWords.GetLength() ) strWords += ' ';
		strWords += pszFilter;

		pszFilter += _tcslen( pszFilter ) + 1;
	}

	return strWords;
}

//////////////////////////////////////////////////////////////////////
// CSecureRule serialize

void CSecureRule::Serialize(CArchive& ar, int nVersion)
{
	CString strTemp;

	if ( ar.IsStoring() )
	{
		ar << m_nType;
		ar << m_nAction;
		ar << m_sComment;

		ar.Write( &m_pGUID, sizeof(GUID) );

		ar << m_nExpire;
		ar << m_nEver;

		switch ( m_nType )
		{
		case srAddress:
			ar.Write( m_nIP, 4 );
			ar.Write( m_nMask, 4 );
			break;
		case srContent:
			ar << m_nIP[0];
			strTemp = GetContentWords();
			ar << strTemp;
			break;
		}
	}
	else
	{
		ar >> m_nType;
		ar >> m_nAction;

		if ( nVersion >= 2 ) ar >> m_sComment;

		if ( nVersion >= 4 )
			ar.Read( &m_pGUID, sizeof(GUID) );
		else
			CoCreateGuid( &m_pGUID );

		ar >> m_nExpire;
		ar >> m_nEver;

		switch ( m_nType )
		{
		case srAddress:
			ar.Read( m_nIP, 4 );
			ar.Read( m_nMask, 4 );
			MaskFix();				// Make sure old rules are updated to new format
			break;
		case srContent:
			ar >> m_nIP[0];

			if ( nVersion < 3 )
			{
				for ( int nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
				{
					CString strWord;
					ar >> strWord;

					strTemp += ' ';
					strTemp += strWord;
				}
			}
			else
			{
				ar >> strTemp;
			}
			SetContentWords( strTemp );
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CSecureRule XML

CXMLElement* CSecureRule::ToXML()
{
	CXMLElement* pXML = new CXMLElement( NULL, _T("rule") );
	CString strValue;

	if ( m_sComment.GetLength() )
	{
		pXML->AddAttribute( _T("comment"), m_sComment );
	}

	switch ( m_nType )
	{
	case srAddress:
		pXML->AddAttribute( _T("type"), _T("address") );
		strValue.Format( _T("%lu.%lu.%lu.%lu"),
			m_nIP[0], m_nIP[1], m_nIP[2], m_nIP[3] );
		pXML->AddAttribute( _T("address"), strValue );
		if ( *(DWORD*)m_nMask != 0xFFFFFFFF )
		{
			strValue.Format( _T("%lu.%lu.%lu.%lu"),
				m_nMask[0], m_nMask[1], m_nMask[2], m_nMask[3] );
			pXML->AddAttribute( _T("mask"), strValue );
		}
		break;
	case srContent:
		pXML->AddAttribute( _T("type"), _T("content") );
		pXML->AddAttribute( _T("content"), GetContentWords() );
		pXML->AddAttribute( _T("match"), m_nIP[0] != 1 ? _T("any") : _T("all") );
		break;
	}

	switch ( m_nAction )
	{
	case srNull:
		pXML->AddAttribute( _T("action"), _T("null") );
		break;
	case srAccept:
		pXML->AddAttribute( _T("action"), _T("accept") );
		break;
	case srDeny:
		pXML->AddAttribute( _T("action"), _T("deny") );
		break;
	}

	if ( m_nExpire == srSession )
	{
		pXML->AddAttribute( _T("expire"), _T("session") );
	}
	else if ( m_nExpire > srSession )
	{
		strValue.Format( _T("%lu"), m_nExpire );
		pXML->AddAttribute( _T("expire"), strValue );
	}

	wchar_t szGUID[39];
	szGUID[ StringFromGUID2( *(GUID*)&m_pGUID, szGUID, 39 ) - 2 ] = 0;
	pXML->AddAttribute( _T("guid"), (CString)&szGUID[1] );
	
	return pXML;
}

BOOL CSecureRule::FromXML(CXMLElement* pXML)
{
	CString strValue;

	m_sComment = pXML->GetAttributeValue( _T("comment") );

	strValue = pXML->GetAttributeValue( _T("type") );

	if ( strValue.CompareNoCase( _T("address") ) == 0 )
	{
		int x[4];

		m_nType = srAddress;

		strValue = pXML->GetAttributeValue( _T("address") );
		if ( _stscanf( strValue, _T("%lu.%lu.%lu.%lu"), &x[0], &x[1], &x[2], &x[3] ) == 4 )
		{
			m_nIP[0] = (BYTE)x[0]; m_nIP[1] = (BYTE)x[1];
			m_nIP[2] = (BYTE)x[2]; m_nIP[3] = (BYTE)x[3];
		}

		strValue = pXML->GetAttributeValue( _T("mask") );
		if ( _stscanf( strValue, _T("%lu.%lu.%lu.%lu"), &x[0], &x[1], &x[2], &x[3] ) == 4 )
		{
			m_nMask[0] = (BYTE)x[0]; m_nMask[1] = (BYTE)x[1];
			m_nMask[2] = (BYTE)x[2]; m_nMask[3] = (BYTE)x[3];
		}
	}
	else if ( strValue.CompareNoCase( _T("content") ) == 0 )
	{
		m_nType = srContent;
		SetContentWords( pXML->GetAttributeValue( _T("content") ) );
		m_nIP[0] = pXML->GetAttributeValue( _T("match") ).CompareNoCase( _T("all") ) == 0;
		if ( m_pContent == NULL ) return FALSE;
	}
	else
	{
		return FALSE;
	}

	strValue = pXML->GetAttributeValue( _T("action") );

	if ( strValue.CompareNoCase( _T("null") ) == 0 )
	{
		m_nAction = srNull;
	}
	else if ( strValue.CompareNoCase( _T("accept") ) == 0 )
	{
		m_nAction = srAccept;
	}
	else if ( strValue.CompareNoCase( _T("deny") ) == 0 || strValue.IsEmpty() )
	{
		m_nAction = srDeny;
	}
	else
	{
		return FALSE;
	}

	strValue = pXML->GetAttributeValue( _T("expire") );
	m_nExpire = srIndefinite;

	if ( strValue.CompareNoCase( _T("session") ) == 0 )
	{
		m_nExpire = srSession;
	}
	else if ( strValue.CompareNoCase( _T("indefinite") ) != 0 )
	{
		_stscanf( strValue, _T("%lu"), &m_nExpire );
	}
	
	MaskFix();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSecureRule Gnucelus strings

CString CSecureRule::ToGnucleusString()
{
	CString strRule;

	if ( m_nType != srAddress ) return strRule;
	if ( m_nAction != srDeny ) return strRule;

	if ( *(DWORD*)m_nMask == 0xFFFFFFFF )
	{
		strRule.Format( _T("%lu.%lu.%lu.%lu"),
			m_nIP[0], m_nIP[1], m_nIP[2], m_nIP[3] );
	}
	else
	{
		int nFrom[4], nTo[4];

		for ( int nByte = 0 ; nByte < 4 ; nByte++ )
		{
			nFrom[ nByte ]	= m_nIP[ nByte ] & m_nMask[ nByte ];
			nTo[ nByte ]	= m_nIP[ nByte ] | ( ~m_nMask[ nByte ] );
		}

		strRule.Format( _T("%lu.%lu.%lu.%lu-%lu.%lu.%lu.%lu"),
			nFrom[0], nFrom[1], nFrom[2], nFrom[3],
			nTo[0], nTo[1], nTo[2], nTo[3] );
	}

	strRule += ':';
	strRule += m_sComment;
	strRule += ':';

	return strRule;
}

BOOL CSecureRule::FromGnucleusString(CString& str)
{
	int nPos, x[4];

	nPos = str.Find( ':' );
	if ( nPos < 1 ) return FALSE;

	CString strAddress = str.Left( nPos );
	str = str.Mid( nPos + 1 );

	if ( _stscanf( strAddress, _T("%lu.%lu.%lu.%lu"), &x[0], &x[1], &x[2], &x[3] ) != 4 )
		return FALSE;

	m_nIP[0] = (BYTE)x[0]; m_nIP[1] = (BYTE)x[1];
	m_nIP[2] = (BYTE)x[2]; m_nIP[3] = (BYTE)x[3];

	nPos = strAddress.Find( '-' );

	if ( nPos >= 0 )
	{
		strAddress = strAddress.Mid( nPos + 1 );

		if ( _stscanf( strAddress, _T("%lu.%lu.%lu.%lu"), &x[0], &x[1], &x[2], &x[3] ) != 4 )
			return FALSE;

		for ( int nByte = 0 ; nByte < 4 ; nByte++ )
		{
			BYTE nTop = (BYTE)x[ nByte ], nBase = (BYTE)x[ nByte ];

			for ( BYTE nValue = m_nIP[ nByte ] ; nValue < nTop ; nValue++ )
			{
				m_nMask[ nByte ] &= ~( nValue ^ nBase );
			}
		}
	}

	m_nType		= srAddress;
	m_nAction	= srDeny;
	m_nExpire	= srIndefinite;
	m_sComment	= str.SpanExcluding( _T(":") );
	
	MaskFix();

	return TRUE;
}
//////////////////////////////////////////////////////////////////////
// CSecureRule Netmask Fix
void  CSecureRule::MaskFix()
{
	DWORD pNetwork = 0 , pOldMask  = 0 , pNewMask = 0;

	for ( int nByte = 0 ; nByte < 4 ; nByte++ )		// convert the byte arrays to dwords
	{
		BYTE nMaskByte = 0;
		BYTE nNetByte = 0;
		nNetByte = m_nIP[ nByte ];
		nMaskByte = m_nMask[ nByte ];
		for ( int nBits = 0 ; nBits < 8 ; nBits++ )
		{
			pNetwork <<= 1;
			if( nNetByte & 0x80 )
			{
				pNetwork |= 1;
			}
			nNetByte <<= 1;

			pOldMask <<= 1;
			if( nMaskByte & 0x80 )
			{
				pOldMask |= 1;
			}
			nMaskByte <<= 1;
		}
	}

	for ( int nBits = 0 ; nBits < 32 ; nBits++ )	// get upper contiguous bits from subnet mask
	{
		if( pOldMask & 0x80000000 )					// check the high bit
		{
			pNewMask >>= 1;							// shift mask down
			pNewMask |= 0x80000000;					// put the bit on
		}
		else
		{
			break;									// found a 0 so ignore the rest
		}
		pOldMask <<= 1;
	}

	pNetwork &= pNewMask;		// do the & now so we don't have to each time there's a match	

	for ( int nByte = 0 ; nByte < 4 ; nByte++ )		// convert the dwords back to byte arrays
	{
		BYTE nMaskByte = 0;
		BYTE nNetByte = 0;
		for ( int nBits = 0 ; nBits < 8 ; nBits++ )
		{
			nNetByte <<= 1;
			if( pNetwork & 0x80000000 )
			{
				nNetByte |= 1;
			}
			pNetwork <<= 1;

			nMaskByte <<= 1;
			if( pNewMask & 0x80000000 )
			{
				nMaskByte |= 1;
			}
			pNewMask <<= 1;
		}
		m_nIP[ nByte ] = nNetByte;
		m_nMask[ nByte ] = nMaskByte;
	}
}
