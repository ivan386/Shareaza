//
// SecureRule.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2013.
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
#include "LiveList.h"
#include "Security.h"
#include "SecureRule.h"
#include "ShareazaFile.h"
#include "QuerySearch.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CSecureRule construction

CSecureRule::CSecureRule(BOOL bCreate)
{
	m_nType		= srAddress;
	m_nAction	= BYTE( Security.m_bDenyPolicy ? srAccept : srDeny );
	m_nExpire	= srIndefinite;
	m_nToday	= 0;
	m_nEver		= 0;

	m_nIP[0]	= m_nIP[1] = m_nIP[2] = m_nIP[3] = 0;
	m_nMask[0]	= m_nMask[1] = m_nMask[2] = m_nMask[3] = 255;
	m_pContent	= NULL;
	m_nContentLength = 0;

	if ( bCreate ) CoCreateGuid( &m_pGUID );
}

CSecureRule::CSecureRule(const CSecureRule& pRule)
{
	m_pContent = NULL;
	*this = pRule;
}

CSecureRule& CSecureRule::operator=(const CSecureRule& pRule)
{
	m_nType		= pRule.m_nType;
	m_nAction	= pRule.m_nAction;
	m_sComment	= pRule.m_sComment;
	m_pGUID		= pRule.m_pGUID;
	m_nExpire	= pRule.m_nExpire;
	m_nToday	= pRule.m_nToday;
	m_nEver		= pRule.m_nEver;
	m_nIP[0]	= pRule.m_nIP[0];
	m_nIP[1]	= pRule.m_nIP[1];
	m_nIP[2]	= pRule.m_nIP[2];
	m_nIP[3]	= pRule.m_nIP[3];
	m_nMask[0]	= pRule.m_nMask[0];
	m_nMask[1]	= pRule.m_nMask[1];
	m_nMask[2]	= pRule.m_nMask[2];
	m_nMask[3]	= pRule.m_nMask[3];

	delete [] m_pContent;
	m_pContent	= pRule.m_nContentLength ? new TCHAR[ pRule.m_nContentLength ] : NULL;
	m_nContentLength = pRule.m_nContentLength;
	CopyMemory( m_pContent, pRule.m_pContent, m_nContentLength * sizeof( TCHAR ) );

	return *this;
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

BOOL CSecureRule::IsExpired(DWORD nNow, BOOL bSession) const
{
	if ( m_nExpire == srIndefinite ) return FALSE;
	if ( m_nExpire == srSession ) return bSession;
	return m_nExpire < nNow;
}

//////////////////////////////////////////////////////////////////////
// CSecureRule match

BOOL CSecureRule::Match(const IN_ADDR* pAddress) const
{
	return ( m_nType == srAddress ) && pAddress &&
		( pAddress->s_addr & *(DWORD*)m_nMask ) == *(DWORD*)m_nIP;
}

BOOL CSecureRule::Match(LPCTSTR pszContent) const
{
	if ( ( m_nType == srContentAny || m_nType == srContentAll ) && pszContent && m_pContent )
	{
		for ( LPCTSTR pszFilter = m_pContent ; *pszFilter ; )
		{
			BOOL bFound = _tcsistr( pszContent, pszFilter ) != NULL;

			if ( bFound && m_nType == srContentAny )
			{
				return TRUE;
			}
			else if ( ! bFound && m_nType == srContentAll )
			{
				return FALSE;
			}

			pszFilter += _tcslen( pszFilter ) + 1;
		}

		if ( m_nType == srContentAll )
			return TRUE;
	}

	return FALSE;
}

BOOL CSecureRule::Match(const CShareazaFile* pFile) const
{
	if ( ( m_nType == srContentAny || m_nType == srContentAll ) && pFile && m_pContent )
	{
		if ( pFile->m_nSize != 0 && pFile->m_nSize != SIZE_UNKNOWN )
		{
			LPCTSTR pszExt = PathFindExtension( (LPCTSTR)pFile->m_sName );
			if ( *pszExt == _T('.') )
			{
				CString strExtension;
				pszExt++;
				strExtension.Format( _T("size:%s:%I64u"), pszExt, pFile->m_nSize );
				if ( Match( strExtension ) )
					return TRUE;
			}
		}

		if ( Match( pFile->m_sName ) )
			return TRUE;

		return
			( pFile->m_oSHA1  && Match( pFile->m_oSHA1.toUrn() ) ) ||
			( pFile->m_oED2K  && Match( pFile->m_oED2K.toUrn() ) ) ||
			( pFile->m_oTiger && Match( pFile->m_oTiger.toUrn() ) ) ||
			( pFile->m_oMD5   && Match( pFile->m_oMD5.toUrn() ) ) ||
			( pFile->m_oBTH   && Match( pFile->m_oBTH.toUrn() ) );
	}

	return FALSE;
}

BOOL CSecureRule::Match(const CQuerySearch* pQuery, const CString& strContent) const
{
	if ( m_nType != srContentRegExp || ! m_pContent )
		return FALSE;

	CString strFilter = pQuery->BuildRegExp( m_pContent );

	if ( strFilter.IsEmpty() )
		return FALSE;

	return RegExp::Match( strFilter, strContent );
}

//////////////////////////////////////////////////////////////////////
// CSecureRule content list helpers

void CSecureRule::SetContentWords(const CString& strContent)
{
	if ( m_nType == srContentRegExp )
	{
		delete [] m_pContent;
		m_nContentLength = strContent.GetLength() + 2;
		LPTSTR pszContent = new TCHAR[ m_nContentLength ];
		_tcscpy_s( pszContent, m_nContentLength, strContent );
		m_pContent = pszContent;
		pszContent += strContent.GetLength();
		*pszContent++ = 0;
		*pszContent++ = 0;
		return;
	}

	LPTSTR pszContent	= (LPTSTR)(LPCTSTR)strContent;
	int nTotalLength	= 3;
	CList< CString > pWords;

	int nStart = 0, nPos = 0;
	for ( ; *pszContent ; nPos++, pszContent++ )
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
		m_nContentLength = 0;
	}

	if ( pWords.IsEmpty() ) return;

	m_pContent	= new TCHAR[ m_nContentLength = nTotalLength ];
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

CString CSecureRule::GetContentWords() const
{
	if ( m_pContent == NULL )
		return CString();

	if ( m_nType == srContentRegExp )
		return CString( m_pContent );

	ASSERT( m_nType != srAddress );

	CString strWords;
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
		ar << (int)m_nType;
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
		case srContentAny:
		case srContentAll:
		case srContentRegExp:
			strTemp = GetContentWords();
			ar << strTemp;
			break;
		}
	}
	else
	{
		int nType;
		ar >> nType;
		ar >> m_nAction;

		if ( nVersion >= 2 ) ar >> m_sComment;

		if ( nVersion >= 4 )
			ReadArchive( ar, &m_pGUID, sizeof(GUID) );
		else
			CoCreateGuid( &m_pGUID );

		ar >> m_nExpire;
		ar >> m_nEver;

		switch ( nType )
		{
		case 0:
			m_nType = srAddress;
			break;
		case 1:
			m_nType = srContentAny;
			break;
		case 2:
			m_nType = srContentAll;
			break;
		case 3:
			m_nType = srContentRegExp;
			break;
		}

		if ( m_nType == srAddress )
		{
			ReadArchive( ar, m_nIP, 4 );
			ReadArchive( ar, m_nMask, 4 );
			MaskFix();				// Make sure old rules are updated to new format
		}
		else
		{
			if ( nVersion < 5 )
			{
				ASSERT( m_nType == srContentAny );

				BYTE foo;
				ar >> foo;
				switch ( foo )
				{
				case 1:
					m_nType = srContentAll;
					break;
				case 2:
					m_nType = srContentRegExp;
					break;
				}
			}

			if ( nVersion < 3 )
			{
				for ( DWORD_PTR nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
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
	case srContentAny:
	case srContentAll:
	case srContentRegExp:
		pXML->AddAttribute( _T("type"), _T("content") );
		CString str;
		switch ( m_nType )
		{
			case srAddress:
				break;
			case srContentAny:
				str = L"any";
				break;
			case srContentAll:
				str = L"all";
				break;
			case srContentRegExp:
				str = L"regexp";
				break;
		}
		pXML->AddAttribute( _T("content"), GetContentWords() );
		pXML->AddAttribute( _T("match"), str );
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
	m_sComment = pXML->GetAttributeValue( _T("comment") );

	CString strType = pXML->GetAttributeValue( _T("type") );

	if ( strType.CompareNoCase( _T("address") ) == 0 )
	{
		int x[4];

		m_nType = srAddress;

		CString strAddress = pXML->GetAttributeValue( _T("address") );
		if ( _stscanf( strAddress, _T("%d.%d.%d.%d"), &x[0], &x[1], &x[2], &x[3] ) == 4 )
		{
			m_nIP[0] = (BYTE)x[0]; m_nIP[1] = (BYTE)x[1];
			m_nIP[2] = (BYTE)x[2]; m_nIP[3] = (BYTE)x[3];
		}

		CString strMask = pXML->GetAttributeValue( _T("mask") );
		if ( _stscanf( strMask, _T("%d.%d.%d.%d"), &x[0], &x[1], &x[2], &x[3] ) == 4 )
		{
			m_nMask[0] = (BYTE)x[0]; m_nMask[1] = (BYTE)x[1];
			m_nMask[2] = (BYTE)x[2]; m_nMask[3] = (BYTE)x[3];
		}
	}
	else if ( strType.CompareNoCase( _T("content") ) == 0 )
	{
		m_nType = srContentAny;
		CString strMatch = pXML->GetAttributeValue( _T("match") );
		if ( strMatch.CompareNoCase( _T("all") ) == 0 )
			m_nType = srContentAll;
		else if ( strMatch.CompareNoCase( _T("regexp") ) == 0 )
			m_nType = srContentRegExp;
		SetContentWords( pXML->GetAttributeValue( _T("content") ) );
		if ( m_pContent == NULL ) return FALSE;
	}
	else
	{
		return FALSE;
	}

	CString strAction = pXML->GetAttributeValue( _T("action") );

	if ( strAction.CompareNoCase( _T("null") ) == 0 )
	{
		m_nAction = srNull;
	}
	else if ( strAction.CompareNoCase( _T("accept") ) == 0 )
	{
		m_nAction = srAccept;
	}
	else if ( strAction.CompareNoCase( _T("deny") ) == 0 || strAction.IsEmpty() )
	{
		m_nAction = srDeny;
	}
	else
	{
		return FALSE;
	}

	CString strExpire = pXML->GetAttributeValue( _T("expire") );
	m_nExpire = srIndefinite;

	if ( strExpire.CompareNoCase( _T("session") ) == 0 )
	{
		m_nExpire = srSession;
	}
	else if ( strExpire.CompareNoCase( _T("indefinite") ) != 0 )
	{
		_stscanf( strExpire, _T("%lu"), &m_nExpire );
	}

	MaskFix();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSecureRule Gnucelus strings

CString CSecureRule::ToGnucleusString() const
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
		BYTE nFrom[4], nTo[4];

		for ( int nByte = 0 ; nByte < 4 ; ++nByte )
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

	if ( _stscanf( strAddress, _T("%d.%d.%d.%d"), &x[0], &x[1], &x[2], &x[3] ) != 4 )
		return FALSE;

	m_nIP[0] = (BYTE)x[0]; m_nIP[1] = (BYTE)x[1];
	m_nIP[2] = (BYTE)x[2]; m_nIP[3] = (BYTE)x[3];

	nPos = strAddress.Find( '-' );

	if ( nPos >= 0 )
	{
		strAddress = strAddress.Mid( nPos + 1 );

		if ( _stscanf( strAddress, _T("%d.%d.%d.%d"), &x[0], &x[1], &x[2], &x[3] ) != 4 )
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
	DWORD nNetwork = 0 , nOldMask  = 0 , nNewMask = 0;

	for ( int nByte = 0 ; nByte < 4 ; nByte++ )		// convert the byte arrays to dwords
	{
		BYTE nMaskByte = 0;
		BYTE nNetByte = 0;
		nNetByte = m_nIP[ nByte ];
		nMaskByte = m_nMask[ nByte ];
		for ( int nBits = 0 ; nBits < 8 ; nBits++ )
		{
			nNetwork <<= 1;
			if ( nNetByte & 0x80 )
			{
				nNetwork |= 1;
			}
			nNetByte <<= 1;

			nOldMask <<= 1;
			if ( nMaskByte & 0x80 )
			{
				nOldMask |= 1;
			}
			nMaskByte <<= 1;
		}
	}

	DWORD nTempMask = nOldMask;

	for ( int nBits = 0 ; nBits < 32 ; nBits++ )	// get upper contiguous bits from subnet mask
	{
		if ( nTempMask & 0x80000000 )					// check the high bit
		{
			nNewMask >>= 1;							// shift mask down
			nNewMask |= 0x80000000;					// put the bit on
		}
		else
		{
			break;									// found a 0 so ignore the rest
		}
		nTempMask <<= 1;
	}

	if ( nNewMask != nOldMask )						// set rule to expire if mask is invalid
	{
		m_nExpire = srSession;
		return;
	}

	nNetwork &= nNewMask;		// do the & now so we don't have to each time there's a match

	for ( int nByte = 0 ; nByte < 4 ; nByte++ )		// convert the dwords back to byte arrays
	{
		BYTE nNetByte = 0;
		for ( int nBits = 0 ; nBits < 8 ; nBits++ )
		{
			nNetByte <<= 1;
			if ( nNetwork & 0x80000000 )
			{
				nNetByte |= 1;
			}
			nNetwork <<= 1;
		}
		m_nIP[ nByte ] = nNetByte;
	}
}

void CSecureRule::ToList(CLiveList* pLiveList, int nCount, DWORD tNow) const
{
	CLiveItem* pItem = pLiveList->Add( (LPVOID)this );

	pItem->SetImage( 0, m_nAction );

	if ( m_nType == CSecureRule::srAddress )
	{
		if ( *(DWORD*)m_nMask == 0xFFFFFFFF )
		{
			pItem->Format( 0, _T("%u.%u.%u.%u"),
				m_nIP[0], m_nIP[1], m_nIP[2], m_nIP[3] );
		}
		else
		{
			pItem->Format( 0, _T("%u.%u.%u.%u/%u.%u.%u.%u"),
				m_nIP[0], m_nIP[1], m_nIP[2], m_nIP[3],
				m_nMask[0], m_nMask[1], m_nMask[2], m_nMask[3] );
		}
	}
	else
	{
		pItem->Set( 0, GetContentWords() );
	}

	switch ( m_nAction )
	{
	case CSecureRule::srNull:
		pItem->Set( 1, _T("N/A") );
		break;
	case CSecureRule::srAccept:
		pItem->Set( 1, _T("Accept") );
		break;
	case CSecureRule::srDeny:
		pItem->Set( 1, _T("Deny") );
		break;
	}

	if ( m_nExpire == CSecureRule::srIndefinite )
	{
		pItem->Set( 2, _T("Never") );
	}
	else if ( m_nExpire == CSecureRule::srSession )
	{
		pItem->Set( 2, _T("Session") );
	}
	else if ( m_nExpire >= tNow )
	{
		DWORD nTime = ( m_nExpire - tNow );
		pItem->Format( 2, _T("%ud %uh %um"),
			nTime / 86400u, (nTime % 86400u) / 3600u, ( nTime % 3600u ) / 60u );
	}

	pItem->Format( 3, _T("%i"), nCount );
	pItem->Format( 4, _T("%u (%u)"), m_nToday, m_nEver );
	pItem->Set( 5, m_sComment );
}
