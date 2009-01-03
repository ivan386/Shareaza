//
// Security.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2008.
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
#include "Security.h"
#include "Buffer.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CSecurity Security;
CAdultFilter AdultFilter;
CMessageFilter MessageFilter;


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
	return m_pRules.GetNext( pos );
}

INT_PTR CSecurity::GetCount() const
{
	return m_pRules.GetCount();
}

POSITION CSecurity::GetRegExpIterator() const
{
	return m_pRegExpRules.GetHeadPosition();
}

CSecureRule* CSecurity::GetNextRegExp(POSITION& pos) const
{
	return m_pRegExpRules.GetNext( pos );
}

INT_PTR CSecurity::GetRegExpCount() const
{
	return m_pRegExpRules.GetCount();
}

BOOL CSecurity::Check(CSecureRule* pRule) const
{
	CQuickLock oLock( m_pSection );

	return pRule != NULL && GetGUID( pRule->m_pGUID ) != NULL;
}

CSecureRule* CSecurity::GetGUID(const GUID& pGUID) const
{
	CQuickLock oLock( m_pSection );

	for ( POSITION pos = m_pRules.GetHeadPosition() ; pos ; )
	{
		CSecureRule* pRule = m_pRules.GetNext( pos );
		if ( pRule->m_pGUID == pGUID ) return pRule;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CSecurity rule modification

void CSecurity::Add(CSecureRule* pRule)
{
	{
		CQuickLock oLock( m_pSection );

		pRule->MaskFix();

		CSecureRule* pExistingRule = GetGUID( pRule->m_pGUID );
		if ( pExistingRule == NULL )
		{
			m_pRules.AddHead( pRule );
			if ( pRule->m_nIP[ 0 ] == 2 )
				m_pRegExpRules.AddHead( pRule );
		}
		else if ( pExistingRule != pRule )
		{
			*pExistingRule = *pRule;
			delete pRule;
		}
	}

	// Check all lists for newly denied hosts
	PostMainWndMessage( WM_SANITY_CHECK );
}

void CSecurity::Remove(CSecureRule* pRule)
{
	CQuickLock oLock( m_pSection );

	POSITION pos = m_pRules.Find( pRule );
	if ( pos ) m_pRules.RemoveAt( pos );
	pos = m_pRegExpRules.Find( pRule );
	if ( pos ) m_pRegExpRules.RemoveAt( pos );
	delete pRule;
}

void CSecurity::MoveUp(CSecureRule* pRule)
{
	CQuickLock oLock( m_pSection );

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
	CQuickLock oLock( m_pSection );

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
	CQuickLock oLock( m_pSection );

	for ( POSITION pos = m_Complains.GetStartPosition() ; pos ; )
	{
		DWORD pAddress;
		CComplain* pComplain;
		m_Complains.GetNextAssoc( pos, pAddress, pComplain );
		delete pComplain;
	}
	m_Complains.RemoveAll();

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		delete GetNext( pos );
	}

	m_pRules.RemoveAll();
	m_pRegExpRules.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// CSecurity ban

void CSecurity::BanHelper(const IN_ADDR* pAddress, const CShareazaFile* pFile, int nBanLength, BOOL bMessage)
{
	CQuickLock oLock( m_pSection );

	DWORD tNow = static_cast< DWORD >( time( NULL ) );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CSecureRule* pRule = GetNext( pos );

		if ( ( pRule->m_nType == CSecureRule::srAddress && pRule->Match( pAddress ) ) ||
			 ( pRule->m_nType == CSecureRule::srContent && pRule->Match( pFile ) ) )
		{
			if ( pRule->m_nAction == CSecureRule::srDeny )
			{
				if ( ( nBanLength == banWeek ) && ( pRule->m_nExpire < tNow + 604000 ) )
				{
					pRule->m_nExpire = static_cast< DWORD >( time( NULL ) + 604800 );
				}
				else if ( ( nBanLength == banForever ) && ( pRule->m_nExpire != CSecureRule::srIndefinite ) )
				{
					pRule->m_nExpire = CSecureRule::srIndefinite;
				}
				else if ( bMessage && pAddress )
				{
					theApp.Message( MSG_NOTICE, IDS_NETWORK_SECURITY_ALREADY_BLOCKED,
						(LPCTSTR)CString( inet_ntoa( *pAddress ) ) );
				}
				return;
			}
		}
	}

	CSecureRule* pRule = new CSecureRule();
	pRule->m_nAction = CSecureRule::srDeny;

	switch ( nBanLength )
	{
	case banSession:
		pRule->m_nExpire	= CSecureRule::srSession;
		pRule->m_sComment	= _T("Quick Ban");
		break;
	case ban5Mins:
		pRule->m_nExpire	= static_cast< DWORD >( time( NULL ) + 300 );
		pRule->m_sComment	= _T("Temp Ignore");
		break;
	case ban30Mins:
		pRule->m_nExpire	= static_cast< DWORD >( time( NULL ) + 1800 );
		pRule->m_sComment	= _T("Temp Ignore");
		break;
	case ban2Hours:
		pRule->m_nExpire	= static_cast< DWORD >( time( NULL ) + 7200 );
		pRule->m_sComment	= _T("Temp Ignore");
		break;
	case banWeek:
		pRule->m_nExpire	= static_cast< DWORD >( time( NULL ) + 604800 );
		pRule->m_sComment	= _T("Client Block");
		break;
	case banForever:
		pRule->m_nExpire	= CSecureRule::srIndefinite;
		pRule->m_sComment	= _T("Ban");
		break;
	default:
		pRule->m_nExpire	= CSecureRule::srSession;
		pRule->m_sComment	= _T("Quick Ban");
	}

	if ( pAddress )
		CopyMemory( pRule->m_nIP, pAddress, sizeof pRule->m_nIP );
	else if ( pFile && ( pFile->m_oSHA1 || pFile->m_oED2K || pFile->m_oTiger ||
		pFile->m_oMD5 || pFile->m_oBTH ) )
	{
		pRule->m_nType = CSecureRule::srContent;
		pRule->SetContentWords(
			( pFile->m_oSHA1  ? pFile->m_oSHA1.toUrn()  + _T(" ") : CString() ) +
			( pFile->m_oED2K  ? pFile->m_oED2K.toUrn()  + _T(" ") : CString() ) +
			( pFile->m_oTiger ? pFile->m_oTiger.toUrn() + _T(" ") : CString() ) +
			( pFile->m_oMD5   ? pFile->m_oMD5.toUrn()   + _T(" ") : CString() ) +
			( pFile->m_oBTH   ? pFile->m_oBTH.toUrn()             : CString() ) );
	}

	Add( pRule );

	if ( bMessage && pAddress )
	{
		theApp.Message( MSG_NOTICE, IDS_NETWORK_SECURITY_BLOCKED,
			(LPCTSTR)CString( inet_ntoa( *pAddress ) ) );
	}
}


//////////////////////////////////////////////////////////////////////
// CSecurity complain

bool CSecurity::Complain(const IN_ADDR* pAddress, int nBanLength, int nExpire, int nCount)
{
	CQuickLock oLock( m_pSection );

	DWORD nNow = static_cast< DWORD >( time( NULL ) );
	CComplain* pComplain = NULL;
	if ( m_Complains.Lookup( pAddress->s_addr, pComplain ) )
	{
		pComplain->m_nScore ++;
		if ( pComplain->m_nScore > nCount )
		{
			m_Complains.RemoveKey( pAddress->s_addr );
			delete pComplain;
			Ban( pAddress, nBanLength );
			return true;
		}
	}
	else
	{
		pComplain = new CComplain;
		pComplain->m_nScore = 1;
		m_Complains.SetAt( pAddress->s_addr, pComplain );
	}
	pComplain->m_nExpire = nNow + nExpire;
	return false;
}

//////////////////////////////////////////////////////////////////////
// CSecurity access check

BOOL CSecurity::IsDenied(const IN_ADDR* pAddress)
{
	CQuickLock oLock( m_pSection );

	DWORD nNow = static_cast< DWORD >( time( NULL ) );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		POSITION posLast = pos;
		CSecureRule* pRule = GetNext( pos );

		if ( pRule->IsExpired( nNow ) )
		{
			m_pRules.RemoveAt( posLast );
			POSITION posRegExp = m_pRegExpRules.Find( pRule );
			if ( posRegExp )
				m_pRegExpRules.RemoveAt( posRegExp );
			delete pRule;
		}
		else if ( pRule->Match( pAddress ) )
		{
			pRule->m_nToday ++;
			pRule->m_nEver ++;

			if ( pRule->m_nExpire > CSecureRule::srSession &&
				pRule->m_nExpire < nNow + 300 )
				// Add 5 min penalty for early access
				pRule->m_nExpire = nNow + 300;

			if ( pRule->m_nAction == CSecureRule::srAccept )
				return FALSE;
			else if ( pRule->m_nAction == CSecureRule::srDeny )
				return TRUE;
		}
	}

	return m_bDenyPolicy;
}

BOOL CSecurity::IsDenied(LPCTSTR pszContent)
{
	CQuickLock oLock( m_pSection );

	DWORD nNow = static_cast< DWORD >( time( NULL ) );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		POSITION posLast = pos;
		CSecureRule* pRule = GetNext( pos );

		if ( pRule->IsExpired( nNow ) )
		{
			m_pRules.RemoveAt( posLast );
			POSITION posRegExp = m_pRegExpRules.Find( pRule );
			if ( posRegExp )
				m_pRegExpRules.RemoveAt( posRegExp );
			delete pRule;
		}
		else if ( pRule->Match( pszContent ) )
		{
			pRule->m_nToday ++;
			pRule->m_nEver ++;

			if ( pRule->m_nExpire > CSecureRule::srSession &&
				pRule->m_nExpire < nNow + 300 )
				// Add 5 min penalty for early access
				pRule->m_nExpire = nNow + 300;

			if ( pRule->m_nAction == CSecureRule::srAccept )
				return FALSE;
			else if ( pRule->m_nAction == CSecureRule::srDeny )
				return TRUE;
		}
	}

	return m_bDenyPolicy;
}

//////////////////////////////////////////////////////////////////////
// CSecurity check file size, hash

BOOL CSecurity::IsDenied(const CShareazaFile* pFile)
{
	CQuickLock oLock( m_pSection );

	DWORD nNow = static_cast< DWORD >( time( NULL ) );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		POSITION posLast = pos;
		CSecureRule* pRule = GetNext( pos );

		if ( pRule->IsExpired( nNow ) )
		{
			m_pRules.RemoveAt( posLast );
			POSITION posRegExp = m_pRegExpRules.Find( pRule );
			if ( posRegExp )
				m_pRegExpRules.RemoveAt( posRegExp );
			delete pRule;
		}
		else if ( pRule->Match( pFile ) )
		{
			pRule->m_nToday ++;
			pRule->m_nEver ++;

			if ( pRule->m_nExpire > CSecureRule::srSession &&
				pRule->m_nExpire < nNow + 300 )
				// Add 5 min penalty for early access
				pRule->m_nExpire = nNow + 300;

			if ( pRule->m_nAction == CSecureRule::srAccept )
				return FALSE;
			else if ( pRule->m_nAction == CSecureRule::srDeny )
				return TRUE;
		}
	}

	return m_bDenyPolicy;
}

BOOL CSecurity::IsDenied(CQuerySearch::const_iterator itStart, CQuerySearch::const_iterator itEnd, 
						 LPCTSTR pszContent)
{
	CQuickLock oLock( m_pSection );

	DWORD nNow = static_cast< DWORD >( time( NULL ) );

	for ( POSITION pos = GetRegExpIterator() ; pos ; )
	{
		POSITION posLast = pos;
		CSecureRule* pRule = GetNextRegExp( pos );

		BOOL bRuleExpired = FALSE;
		if ( pRule->IsExpired(nNow, TRUE ) )
		{
			m_pRegExpRules.RemoveAt( posLast );
			POSITION posAll = m_pRules.Find( pRule );
			if ( posAll )
				m_pRules.RemoveAt( posAll );
			delete pRule;
			bRuleExpired=TRUE;
		}
		

		if (bRuleExpired==FALSE && pRule->Match( itStart, itEnd, pszContent ) )
		{
			pRule->m_nToday ++;
			pRule->m_nEver ++;

			if ( pRule->m_nAction == CSecureRule::srAccept ) return FALSE;
			else if ( pRule->m_nAction == CSecureRule::srDeny ) return TRUE;
		}
	}

	return m_bDenyPolicy;
}

//////////////////////////////////////////////////////////////////////
// CSecurity expire

void CSecurity::Expire()
{
	CQuickLock oLock( m_pSection );

	DWORD nNow = static_cast< DWORD >( time( NULL ) );

	for ( POSITION pos = m_Complains.GetStartPosition() ; pos ; )
	{
		DWORD pAddress;
		CComplain* pComplain;
		m_Complains.GetNextAssoc( pos, pAddress, pComplain );
		if ( pComplain->m_nExpire < nNow )
		{
			m_Complains.RemoveKey( pAddress );
			delete pComplain;
		}
	}

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		POSITION posLast = pos;
		CSecureRule* pRule = GetNext( pos );

		if ( pRule->IsExpired( nNow ) )
		{
			m_pRules.RemoveAt( posLast );
			POSITION posRegExp = m_pRegExpRules.Find( pRule );
			if ( posRegExp ) 
				m_pRegExpRules.RemoveAt( posRegExp );
			delete pRule;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CSecurity load and save

BOOL CSecurity::Load()
{
	CQuickLock oLock( m_pSection );

	CFile pFile;

	CString strFile = Settings.General.UserPath + _T("\\Data\\Security.dat");

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

BOOL CSecurity::Save()
{
	CQuickLock oLock( m_pSection );

	CFile pFile;

	CString strFile = Settings.General.UserPath + _T("\\Data\\Security.dat");

	if ( pFile.Open( strFile, CFile::modeWrite|CFile::modeCreate ) )
	{
		CArchive ar( &pFile, CArchive::store );
		Serialize( ar );
		ar.Close();
	}

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

		DWORD nNow = static_cast< DWORD >( time( NULL ) );

		for ( DWORD_PTR nCount = ar.ReadCount() ; nCount > 0 ; nCount-- )
		{
			CSecureRule* pRule = new CSecureRule( FALSE );
			pRule->Serialize( ar, nVersion );

			if ( pRule->IsExpired( nNow, TRUE ) )
				delete pRule;
			else
			{
				m_pRules.AddTail( pRule );
				if ( pRule->m_nIP[ 0 ] == 2 )
					m_pRegExpRules.AddTail( pRule );
			}
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
			CQuickLock oLock( m_pSection );
			CSecureRule* pRule	= NULL;
			CString strGUID		= pElement->GetAttributeValue( _T("guid") );
			BOOL bExisting		= FALSE;
			GUID pGUID;

			if ( Hashes::fromGuid( strGUID, &pGUID ) )
			{
				if ( ( pRule = GetGUID( pGUID ) ) != NULL ) bExisting = TRUE;

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
				if ( ! bExisting )
				{
					m_pRules.AddTail( pRule );
					if ( pRule->m_nIP[ 0 ] == 2 )
						m_pRegExpRules.AddTail( pRule );
				}
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
				CQuickLock oLock( m_pSection );
				m_pRules.AddTail( pRule );
				bResult = TRUE;
			}
			else
			{
				delete pRule;
			}
		}
	}

	// Check all lists for newly denied hosts
	PostMainWndMessage( WM_SANITY_CHECK );

	return bResult;
}

BOOL CSecurity::IsClientBad(const CString& sUserAgent)
{
	// No user agent- assume bad
	if ( sUserAgent.IsEmpty() )							return TRUE; // They allowed to connect but no searches were performed

	// Bad/unapproved versions of Shareaza
	// Really obsolete versions of Shareaza should be blocked. (they may have bad settings)
	if ( LPCTSTR szVersion = _tcsistr( sUserAgent, _T("shareaza") ) )	
	{
		szVersion += 8;
		if ( _tcsistr( szVersion, _T(" 0.") ) )			return TRUE;
		if ( _tcsistr( szVersion, _T(" 1.") ) )			return TRUE;	// There can be some 1.x versions of the real Shareaza but most are fakes
		if ( _tcsistr( szVersion, _T(" 2.0") ) )		return TRUE;	// There is also a Shareaza rip-off that identify as Shareaza 2.0.0.0 (The real Shareaza 2.0.0.0 is so old and bad)
		if ( _tcsistr( szVersion, _T(" 3.0") ) )		return TRUE;
		if ( _tcsistr( szVersion, _T(" 3.1") ) )		return TRUE;
		if ( _tcsistr( szVersion, _T(" 3.2") ) )		return TRUE;
		if ( _tcsistr( szVersion, _T(" 3.3") ) )		return TRUE;
		if ( _tcsistr( szVersion, _T(" 3.4") ) )		return TRUE;
		if ( _tcsistr( szVersion, _T(" 6.") ) )			return TRUE;
		if ( _tcsistr( szVersion, _T(" 7.") ) )			return TRUE;
		if ( _tcsistr( szVersion, _T(" pro") ) )		return TRUE;
		return FALSE;
	}

	// LimeWire
	if ( LPCTSTR szVersion = _tcsistr( sUserAgent, _T("LimeWire") ) )	
	{
		szVersion += 8;
		return FALSE;
	}

	// Dianlei: Shareaza rip-off
	// add only based on alpha code, need verification for others
	if ( LPCTSTR szVersion = _tcsistr( sUserAgent, _T("Dianlei") ) )
	{
		szVersion += 7;
		if ( _tcsistr( szVersion, _T(" 1.") ) )			return TRUE;
		if ( _tcsistr( szVersion, _T(" 0.") ) )			return TRUE;
		return FALSE;
	}

	// BearShare
	if ( LPCTSTR szVersion = _tcsistr( sUserAgent, _T("BearShare") ) )
	{
		szVersion += 9;
		if ( _tcsistr( szVersion, _T(" Lite") ) )		return TRUE;
		if ( _tcsistr( szVersion, _T(" Pro") ) )		return TRUE;
		if ( _tcsistr( szVersion, _T(" MP3") ) ) 		return TRUE;	// GPL breaker
		if ( _tcsistr( szVersion, _T(" Music") ) ) 		return TRUE;	// GPL breaker
		if ( _tcsistr( szVersion, _T(" 6.") ) ) 		return TRUE;	// iMesh
		return FALSE;
	}

	// iMesh
	if ( _tcsistr( sUserAgent, _T("iMesh") ) )						return TRUE;

	// Identified Shareaza Leecher Mod
	if ( _tcsistr( sUserAgent, _T("eMule mod (4)") ) )				return TRUE;

	// Fildelarprogram
	if ( _tcsistr( sUserAgent, _T("Fildelarprogram") ) )			return TRUE;
	
	// Trilix
	if ( _tcsistr( sUserAgent, _T("Trilix") ) )						return TRUE;
	
	// Gnutella Turbo (Look into this client some more)
	if ( _tcsistr( sUserAgent, _T("Gnutella Turbo") ) )				return TRUE;
	
	// Mastermax File Sharing
	if ( _tcsistr( sUserAgent, _T("Mastermax File Sharing") ) )		return TRUE;
	
	// Fastload.TV
	if ( _tcsistr( sUserAgent, _T("Fastload.TV") ) )				return TRUE;
	
	// GPL breakers- Clients violating the GPL
	// See http://www.gnu.org/copyleft/gpl.html
	// Some other breakers outside the list

	if ( _tcsistr( sUserAgent, _T("K-Lite") ) )						return TRUE; // Is it bad?

	if ( _tcsistr( sUserAgent, _T("SlingerX") ) )					return TRUE; // Rip-off with bad tweaks

	if ( _tcsistr( sUserAgent, _T("C -3.0.1") ) )					return TRUE;

	if ( _tcsistr( sUserAgent, _T("vagaa") ) )						return TRUE; // Not clear why it's bad

	if ( _tcsistr( sUserAgent, _T("mxie") ) )						return TRUE; // Leechers, do not allow to connect

	if ( _tcsistr( sUserAgent, _T("WinMX") ) )						return TRUE;
	
	if ( _tcsistr( sUserAgent, _T("eTomi") ) )						return TRUE; // outdated rip-off

	// Unknown- Assume OK
	return FALSE;
}

BOOL CSecurity::IsClientBanned(const CString& sUserAgent)
{
	// No user agent- assume OK
	if ( sUserAgent.IsEmpty() )										return FALSE;

	// i2hub - leecher client. (Tested, does not upload)
	if ( _tcsistr( sUserAgent, _T("i2hub 2.0") ) )					return TRUE;

	// Check by content filter
	return IsDenied( sUserAgent );
}

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
	if ( m_nType == srAddress && pAddress != NULL )
	{
		DWORD* pBase = (DWORD*)m_nIP;
		DWORD* pMask = (DWORD*)m_nMask;
		DWORD* pTest = (DWORD*)pAddress;

// This only works if IP's are &ed before entered in the list
		if ( ( ( *pTest ) & ( *pMask ) ) == ( *pBase ) )
		{
			return ! IsExpired( (DWORD)time( NULL ) );
		}
	}
	return FALSE;
}

BOOL CSecureRule::Match(LPCTSTR pszContent) const
{
	if ( m_nType == srContent && pszContent != NULL && m_pContent != NULL )
	{
		if ( m_nIP[0] == 2 )
			return FALSE;

		if ( IsExpired( (DWORD)time( NULL ) ) )
			return FALSE;

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

		if ( m_nIP[0] == 1 )
			return TRUE;
	}

	return FALSE;
}

BOOL CSecureRule::Match(const CShareazaFile* pFile) const
{
	if ( m_nType == srContent && pFile != NULL && m_pContent != NULL )
	{
		if ( pFile->m_nSize != 0 && pFile->m_nSize != SIZE_UNKNOWN )
		{
			LPCTSTR pszExt = PathFindExtension( (LPCTSTR)pFile->m_sName );
			if ( *pszExt == _T('.') )
			{
				CString strExtension;
				pszExt++;
				strExtension.Format( _T("size:%s:%I64i"), pszExt, pFile->m_nSize );
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

BOOL CSecureRule::Match(CQuerySearch::const_iterator itStart, 
						CQuerySearch::const_iterator itEnd, LPCTSTR pszContent) const
{
	CString strFilter = GetRegExpFilter( itStart, itEnd );
	if ( strFilter.GetLength() )
	{
		using namespace regex;
		try
		{
			const rpattern regExpPattern( (LPCTSTR)strFilter, NOCASE, MODE_SAFE );
			match_results results;
			std::wstring strTemp( pszContent, _tcslen(pszContent) );
			rpattern::backref_type matches = regExpPattern.match( strTemp, results );
			if ( matches.matched )
				return TRUE;

		}
		catch (...)	{}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CSecureRule content list helpers

void CSecureRule::SetContentWords(const CString& strContent)
{
	if ( m_nIP[ 0 ] == 2 )
	{
		if ( m_pContent )
			delete [] m_pContent;

		m_nContentLength = strContent.GetLength() + 2;
		LPTSTR pszContent = new TCHAR[ m_nContentLength ];
		_tcscpy( pszContent, strContent );
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

CString CSecureRule::GetContentWords()
{
	CString strWords;

	if ( m_pContent == NULL ) return strWords;

	if ( m_nIP[ 0 ] == 2 )
		return CString( (LPCTSTR)m_pContent );

	for ( LPCTSTR pszFilter = m_pContent ; *pszFilter ; )
	{
		if ( strWords.GetLength() ) strWords += ' ';
		strWords += pszFilter;

		pszFilter += _tcslen( pszFilter ) + 1;
	}

	return strWords;
}

// Build a regular expression filter from the search query words
// Returns an empty string if not applied or if the filter was invalid
CString CSecureRule::GetRegExpFilter(CQuerySearch::const_iterator itStart,
									 CQuerySearch::const_iterator itEnd) const
{
	if ( m_nIP[ 0 ] != 2 ) return CString();

	LPCTSTR pszPattern = m_pContent;
	int nTotal = 0;

	CString strFilter;

	while ( *pszPattern )
	{
		if ( *pszPattern == '<' )
		{
			pszPattern++;
			bool bEnds = false;
			bool bAll = *pszPattern == '_';

			for ( ; *pszPattern ; pszPattern++ )
			{
				if ( *pszPattern == '>' )
				{
					bEnds = true;
					break;
				}
			}

			if ( bEnds )
			{
				if ( bAll )
				{
					// Add all keywords at the "<_>" position
					for ( ; itStart != itEnd ; itStart++ )
					{
						strFilter.AppendFormat( L"%s\\s*", 
							CString( itStart->first, int(itStart->second) ) );
					}
				}
				else
				{
					pszPattern--; // Go back
					int nNumber = 0;

					// Numbers from 1 to 9, no more
					if ( _stscanf( &pszPattern[0], L"%i", &nNumber ) != 1 )
						nNumber = ++nTotal;

					for ( int nWord = 1 ; itStart != itEnd ; itStart++, nWord++ )
					{
						if ( nWord == nNumber )
						{
							strFilter.AppendFormat( L"%s\\s*", 
								CString( itStart->first, int(itStart->second) ) );
							break;
						}
					}
					pszPattern++; // return to the last position
				}
			}
			else
				return CString(); // no closing '>'
		}
		else
		{
			strFilter += *pszPattern; // not replacing
		}
		pszPattern++;
	}

	// Validate
	using namespace regex;
	try
	{
		const rpattern regExpPattern( (LPCTSTR)strFilter );
		UNUSED_ALWAYS( regExpPattern );
	}
	catch (...) 
	{
		theApp.Message( MSG_DEBUG, L"Invalid regexp filter: \"%s\". Ignoring.", (LPCTSTR)strFilter );
		strFilter.Empty();
	}

	return strFilter;
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
			ReadArchive( ar, &m_pGUID, sizeof(GUID) );
		else
			CoCreateGuid( &m_pGUID );

		ar >> m_nExpire;
		ar >> m_nEver;

		switch ( m_nType )
		{
		case srAddress:
			ReadArchive( ar, m_nIP, 4 );
			ReadArchive( ar, m_nMask, 4 );
			MaskFix();				// Make sure old rules are updated to new format
			break;
		case srContent:
			ar >> m_nIP[0];

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
		CString str;
		switch ( m_nIP[0] )
		{
			case 0: str = L"any"; break;
			case 1: str = L"all"; break;
			case 2: str = L"regexp"; break;
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
		CString strValue = pXML->GetAttributeValue( _T("match") );
		if ( strValue.CompareNoCase( _T("any") ) == 0 )
			m_nIP[0] = 0;
		else if ( strValue.CompareNoCase( _T("all") ) == 0 )
			m_nIP[0] = 1;
		else if ( strValue.CompareNoCase( _T("regexp") ) == 0 )
			m_nIP[0] = 2;
		SetContentWords( pXML->GetAttributeValue( _T("content") ) );
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

//////////////////////////////////////////////////////////////////////
// CAdultFilter construction

CAdultFilter::CAdultFilter()
:	m_pszBlockedWords(NULL),
	m_pszDubiousWords(NULL),
	m_pszChildWords(NULL)
{
}

CAdultFilter::~CAdultFilter()
{
	if ( m_pszBlockedWords ) delete [] m_pszBlockedWords;
	m_pszBlockedWords = NULL;

	if ( m_pszDubiousWords ) delete [] m_pszDubiousWords;
	m_pszDubiousWords = NULL;

	if ( m_pszChildWords ) delete [] m_pszChildWords;
	m_pszChildWords = NULL;
}

void CAdultFilter::Load()
{
	CFile pFile;
	CString strFile = Settings.General.Path + _T("\\Data\\AdultFilter.dat");
	CString strBlockedWords, strDubiousWords, strChildWords;

	// Delete current adult filters (if present)
	if ( m_pszBlockedWords ) delete [] m_pszBlockedWords;
	m_pszBlockedWords = NULL;

	if ( m_pszDubiousWords ) delete [] m_pszDubiousWords;
	m_pszDubiousWords = NULL;

	if ( m_pszChildWords ) delete [] m_pszChildWords;
	m_pszChildWords = NULL;

	// Load the adult filter from disk
	if (  pFile.Open( strFile, CFile::modeRead ) )
	{
		try
		{
			CBuffer pBuffer;
			DWORD nLen = (DWORD)pFile.GetLength();
			if ( !pBuffer.EnsureBuffer( nLen ) )
				AfxThrowUserException();

			pBuffer.m_nLength = (DWORD)pFile.GetLength();
			pFile.Read( pBuffer.m_pBuffer, pBuffer.m_nLength );
			pFile.Close();

			pBuffer.ReadLine( strBlockedWords );	// Line 1: words that are blocked
			if ( strBlockedWords.GetLength() && strBlockedWords.GetAt( 0 ) == '#' )
				strBlockedWords.Empty();
			pBuffer.ReadLine( strDubiousWords );	// Line 2: words that may be okay
			if ( strDubiousWords.GetLength() && strDubiousWords.GetAt( 0 ) == '#' )
				strDubiousWords.Empty();
			pBuffer.ReadLine( strChildWords );		// Line 3: words for child pornography
			if ( strChildWords.GetLength() && strChildWords.GetAt( 0 ) == '#' )
				strChildWords.Empty();
		}
		catch ( CException* pException )
		{
			if (pFile.m_hFile != CFile::hFileNull) pFile.Close(); //Check if file is still open, if yes close
			pException->Delete();
		}
	}

	// Insert some defaults if the load failed
	if ( strBlockedWords.IsEmpty() )
		strBlockedWords = L"xxx porn fuck cock cunt vagina pussy nude naked boobs breast hentai "
						  L"lesbian whore shit rape preteen hardcore lolita playboy penthouse "
						  L"topless r-rated x-rated dildo pr0n erotic sexy orgasm nipple fetish "
						  L"upskirt beastiality bestiality pedofil necrofil tits lolicon shemale fisting";
	if ( strDubiousWords.IsEmpty() )
		strDubiousWords = L"ass sex anal gay teen thong babe bikini viagra dick cum sluts";

	if ( strChildWords.IsEmpty() )
		strChildWords = L"child preteen";

	// Load the blocked words into the Adult Filter
	if ( strBlockedWords.GetLength() > 3 )
	{
		LPCTSTR pszPtr = strBlockedWords;
		int nWordLen = 3;
		CList< CString > pWords;

		int nStart = 0, nPos = 0;
		for ( ; *pszPtr ; nPos++, pszPtr++ )
		{
			if ( *pszPtr == ' ' )
			{
				if ( nStart < nPos )
				{
					pWords.AddTail( strBlockedWords.Mid( nStart, nPos - nStart ) );
					nWordLen += ( nPos - nStart ) + 1;
				}
				nStart = nPos + 1;
			}
		}


		if ( nStart < nPos )
		{
			pWords.AddTail( strBlockedWords.Mid( nStart, nPos - nStart ) );
			nWordLen += ( nPos - nStart ) + 1;
		}

		m_pszBlockedWords = new TCHAR[ nWordLen ];
		LPTSTR pszFilter = m_pszBlockedWords;

		for ( POSITION pos = pWords.GetHeadPosition() ; pos ; )
		{
			CString strWord( pWords.GetNext( pos ) );
			ToLower( strWord );

			CopyMemory( pszFilter, (LPCTSTR)strWord, sizeof(TCHAR) * ( strWord.GetLength() + 1 ) );
			pszFilter += strWord.GetLength() + 1;
		}

		*pszFilter++ = 0;
		*pszFilter++ = 0;
	}

	// Load the possibly blocked words into the Adult Filter
	if ( strDubiousWords.GetLength() > 3 )
	{
		LPCTSTR pszPtr = strDubiousWords;
		int nWordLen = 3;
		CList< CString > pWords;

		int nStart = 0, nPos = 0;
		for ( ; *pszPtr ; nPos++, pszPtr++ )
		{
			if ( *pszPtr == ' ' )
			{
				if ( nStart < nPos )
				{
					pWords.AddTail( strDubiousWords.Mid( nStart, nPos - nStart ) );
					nWordLen += ( nPos - nStart ) + 1;
				}
				nStart = nPos + 1;
			}
		}

		if ( nStart < nPos )
		{
			pWords.AddTail( strDubiousWords.Mid( nStart, nPos - nStart ) );
			nWordLen += ( nPos - nStart ) + 1;
		}

		m_pszDubiousWords = new TCHAR[ nWordLen ];
		LPTSTR pszFilter = m_pszDubiousWords;

		for ( POSITION pos = pWords.GetHeadPosition() ; pos ; )
		{
			CString strWord( pWords.GetNext( pos ) );
			ToLower( strWord );

			CopyMemory( pszFilter, (LPCTSTR)strWord, sizeof(TCHAR) * ( strWord.GetLength() + 1 ) );
			pszFilter += strWord.GetLength() + 1;
		}

		*pszFilter++ = 0;
		*pszFilter++ = 0;
	}

	// Load child pornography words into the Adult Filter
	if ( strChildWords.GetLength() > 3 )
	{
		LPCTSTR pszPtr = strChildWords;
		int nWordLen = 3;
		CList< CString > pWords;

		int nStart = 0, nPos = 0;
		for ( ; *pszPtr ; nPos++, pszPtr++ )
		{
			if ( *pszPtr == ' ' )
			{
				if ( nStart < nPos )
				{
					pWords.AddTail( strChildWords.Mid( nStart, nPos - nStart ) );
					nWordLen += ( nPos - nStart ) + 1;
				}
				nStart = nPos + 1;
			}
		}

		if ( nStart < nPos )
		{
			pWords.AddTail( strChildWords.Mid( nStart, nPos - nStart ) );
			nWordLen += ( nPos - nStart ) + 1;
		}

		m_pszChildWords = new TCHAR[ nWordLen ];
		LPTSTR pszFilter = m_pszChildWords;

		for ( POSITION pos = pWords.GetHeadPosition() ; pos ; )
		{
			CString strWord( pWords.GetNext( pos ) );
			ToLower( strWord );

			CopyMemory( pszFilter, (LPCTSTR)strWord, sizeof(TCHAR) * ( strWord.GetLength() + 1 ) );
			pszFilter += strWord.GetLength() + 1;
		}

		*pszFilter++ = 0;
		*pszFilter++ = 0;
	}
}

BOOL CAdultFilter::IsHitAdult(LPCTSTR pszText)
{
	if ( pszText )
	{
		return IsFiltered( pszText );
	}
	return FALSE;
}

BOOL CAdultFilter::IsSearchFiltered(LPCTSTR pszText)
{
	if ( Settings.Search.AdultFilter && pszText )
	{
		return IsFiltered( pszText );
	}
	return FALSE;
}

BOOL CAdultFilter::IsChatFiltered(LPCTSTR pszText)
{
	if ( Settings.Community.ChatCensor && pszText )
	{
		return IsFiltered( pszText );
	}
	return FALSE;
}

BOOL CAdultFilter::Censor(TCHAR* pszText)
{
	BOOL bModified = FALSE;
	if ( ! pszText ) return FALSE;

	LPCTSTR pszWord;

	// Check and replace blocked words
	if ( m_pszBlockedWords )
	{
		for ( pszWord = m_pszBlockedWords ; *pszWord ; )
		{
			TCHAR* pReplace = (TCHAR*)_tcsistr( pszText, pszWord );

			if ( pReplace != NULL )
			{
				TCHAR cExpletives[6] = {'#','@','$','%','&','*'};

				for ( unsigned nLoop = 0 ; nLoop < _tcslen( pszWord ) ; nLoop++ )
				{
					*pReplace = cExpletives[ ( nLoop % 6 ) ];
					pReplace++;
				}

				bModified = TRUE;
			}

			pszWord += _tcslen( pszWord ) + 1;
		}
	}

	return bModified;
}

BOOL CAdultFilter::IsChildPornography(LPCTSTR pszText)
{
	if ( pszText )
	{
		LPCTSTR pszWord;
		bool bFound = false;

		for ( pszWord = m_pszChildWords ; *pszWord ; )
		{
			if ( _tcsistr( pszText, pszWord ) != NULL )
			{
				bFound = true;
				break;
			}
			pszWord += _tcslen( pszWord ) + 1;
		}

		return ( bFound && IsFiltered( pszText ) );
	}

	return FALSE;
}

BOOL CAdultFilter::IsFiltered(LPCTSTR pszText)
{
	if ( pszText )
	{
		LPCTSTR pszWord;

		// Check blocked words
		if ( m_pszBlockedWords )
		{
			for ( pszWord = m_pszBlockedWords ; *pszWord ; )
			{
				if ( _tcsistr( pszText, pszWord ) != NULL ) return TRUE;
				pszWord += _tcslen( pszWord ) + 1;
			}
		}

		// Check dubious words
		if ( m_pszDubiousWords )
		{
			size_t nDubiousWords = 0, nWordsPermitted = min( _tcslen( pszText ) / 8, 4u );

			for ( pszWord = m_pszDubiousWords ; *pszWord ; )
			{
				if ( _tcsistr( pszText, pszWord ) != NULL ) nDubiousWords++;
				if ( nDubiousWords > nWordsPermitted ) return TRUE;
				pszWord += _tcslen( pszWord ) + 1;
			}
		}
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////
// CMessageFilter construction

CMessageFilter::CMessageFilter()
{
	m_pszED2KSpam = NULL;
	m_pszFilteredPhrases = NULL;
}

CMessageFilter::~CMessageFilter()
{
	if ( m_pszED2KSpam ) delete [] m_pszED2KSpam;
	m_pszED2KSpam = NULL;

	if ( m_pszFilteredPhrases ) delete [] m_pszFilteredPhrases;
	m_pszFilteredPhrases = NULL;

}

void CMessageFilter::Load()
{
	CFile pFile;
	CString strFile = Settings.General.Path + _T("\\Data\\MessageFilter.dat");
	CString strFilteredPhrases, strED2KSpamPhrases;

	// Delete current filter (if present)
	if ( m_pszFilteredPhrases ) delete [] m_pszFilteredPhrases;
	m_pszFilteredPhrases = NULL;

	// Load the message filter from disk
	if (  pFile.Open( strFile, CFile::modeRead ) )
	{
		try
		{
			CBuffer pBuffer;
			DWORD nLen = (DWORD)pFile.GetLength();
			if ( !pBuffer.EnsureBuffer( nLen ) )
				AfxThrowUserException();

			pBuffer.m_nLength = nLen;
			pFile.Read( pBuffer.m_pBuffer, pBuffer.m_nLength );
			pFile.Close();

			pBuffer.ReadLine( strED2KSpamPhrases );
			pBuffer.ReadLine( strFilteredPhrases );
		}
		catch ( CException* pException )
		{
			if (pFile.m_hFile != CFile::hFileNull) pFile.Close(); // Check if file is still open, if yes close
			pException->Delete();
		}
	}

	// Insert some defaults if there was a read error

	if ( strED2KSpamPhrases.IsEmpty() )
		strED2KSpamPhrases = _T("Your client is connecting too fast|Join the L33cher Team|PeerFactor|Your client is making too many connections|ZamBoR 2|AUTOMATED MESSAGE:|eMule FX the BEST eMule ever|DI-Emule");

	if ( strFilteredPhrases.IsEmpty() )
		strFilteredPhrases = _T("");


	// Load the ED2K spam into the filter
	if ( strED2KSpamPhrases.GetLength() > 3 )
	{
		LPCTSTR pszPtr = strED2KSpamPhrases;
		int nWordLen = 3;
		CList< CString > pWords;

		int nStart = 0, nPos = 0;
		for ( ; *pszPtr ; nPos++, pszPtr++ )
		{
			if ( *pszPtr == '|' )
			{
				if ( nStart < nPos )
				{
					pWords.AddTail( strED2KSpamPhrases.Mid( nStart, nPos - nStart ) );
					nWordLen += ( nPos - nStart ) + 1;
				}
				nStart = nPos + 1;
			}
		}

		if ( nStart < nPos )
		{
			pWords.AddTail( strED2KSpamPhrases.Mid( nStart, nPos - nStart ) );
			nWordLen += ( nPos - nStart ) + 1;
		}

		m_pszED2KSpam = new TCHAR[ nWordLen ];
		LPTSTR pszFilter = m_pszED2KSpam;

		for ( POSITION pos = pWords.GetHeadPosition() ; pos ; )
		{
			CString strWord( pWords.GetNext( pos ) );
			ToLower( strWord );

			CopyMemory( pszFilter, (LPCTSTR)strWord, sizeof(TCHAR) * ( strWord.GetLength() + 1 ) );
			pszFilter += strWord.GetLength() + 1;
		}

		*pszFilter++ = 0;
		*pszFilter++ = 0;
	}

	// Load the blocked strings into the filter
	if ( strFilteredPhrases.GetLength() > 3 )
	{
		LPCTSTR pszPtr = strFilteredPhrases;
		int nWordLen = 3;
		CList< CString > pWords;

		int nStart = 0, nPos = 0;
		for ( ; *pszPtr ; nPos++, pszPtr++ )
		{
			if ( *pszPtr == '|' )
			{
				if ( nStart < nPos )
				{
					pWords.AddTail( strFilteredPhrases.Mid( nStart, nPos - nStart ) );
					nWordLen += ( nPos - nStart ) + 1;
				}
				nStart = nPos + 1;
			}
		}

		if ( nStart < nPos )
		{
			pWords.AddTail( strFilteredPhrases.Mid( nStart, nPos - nStart ) );
			nWordLen += ( nPos - nStart ) + 1;
		}

		m_pszFilteredPhrases = new TCHAR[ nWordLen ];
		LPTSTR pszFilter = m_pszFilteredPhrases;

		for ( POSITION pos = pWords.GetHeadPosition() ; pos ; )
		{
			CString strWord( pWords.GetNext( pos ) );
			ToLower( strWord );

			CopyMemory( pszFilter, (LPCTSTR)strWord, sizeof(TCHAR) * ( strWord.GetLength() + 1 ) );
			pszFilter += strWord.GetLength() + 1;
		}

		*pszFilter++ = 0;
		*pszFilter++ = 0;
	}
}

BOOL CMessageFilter::IsED2KSpam( LPCTSTR pszText )
{
	if ( Settings.Community.ChatFilterED2K && pszText )
	{
		// Check for Ed2K spam phrases
		if ( m_pszED2KSpam )
		{
			LPCTSTR pszWord;
			for ( pszWord = m_pszED2KSpam ; *pszWord ; )
			{
				if ( _tcsistr( pszText, pszWord ) != NULL ) return TRUE;
				pszWord += _tcslen( pszWord ) + 1;
			}
		}
	}

	return FALSE;
}

BOOL CMessageFilter::IsFiltered( LPCTSTR pszText )
{
	if ( Settings.Community.ChatFilter && pszText )
	{
		// Check for filtered (spam) phrases
		if ( m_pszFilteredPhrases )
		{
			LPCTSTR pszWord;
			for ( pszWord = m_pszFilteredPhrases ; *pszWord ; )
			{
				if ( _tcsistr( pszText, pszWord ) != NULL ) return TRUE;
				pszWord += _tcslen( pszWord ) + 1;
			}
		}
	}

	return FALSE;
}
