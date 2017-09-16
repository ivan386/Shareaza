//
// Security.cpp
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
#include "Buffer.h"
#include "LiveList.h"
#include "Settings.h"
#include "Security.h"
#include "ShareazaFile.h"
#include "QuerySearch.h"
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
	return m_pRules.GetCount() + m_pIPRules.size();
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

	for ( CAddressRuleMap::const_iterator i = m_pIPRules.begin(); i != m_pIPRules.end(); ++i )
	{
		if( (*i).second->m_pGUID == pGUID ) return (*i).second;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CSecurity rule modification

void CSecurity::Add(CSecureRule* pRule)
{
	{
		CQuickLock oLock( m_pSection );

		// If an address rule is added, the mask fix is performed and the miss cache is cleared either in whole or just the relevant address
		// erase does support ranges but this should cover auto bans and get reasonable performance without tripping on incorrect masks
		if ( pRule->m_nType == CSecureRule::srAddress )
		{
			pRule->MaskFix();
			if ( *(DWORD*)pRule->m_nMask == 0xffffffff )
			{
				m_Cache.erase( *(DWORD*)pRule->m_nIP );
			}
			else
			{
				m_Cache.clear();
			}
		}

		pRule->MaskFix();

		// special treatment for single IP security rules
		if ( pRule->m_nType == CSecureRule::srAddress &&
			*(DWORD*)pRule->m_nMask == 0xffffffff )
		{
			CAddressRuleMap::iterator i = m_pIPRules.find( *(DWORD*)pRule->m_nIP );
			if ( i == m_pIPRules.end() )
			{
				m_pIPRules[ *(DWORD*)pRule->m_nIP ] = pRule;
			}
			else if ( (*i).second != pRule )
			{
				// replace old rule with new one.
				CSecureRule* pOldRule = (*i).second;
				(*i).second = pRule;
				delete pOldRule;
			}
		}
		else // default procedure for everything else
		{
			CSecureRule* pExistingRule = GetGUID( pRule->m_pGUID );
			if ( pExistingRule == NULL )
			{
				m_pRules.AddHead( pRule );
			}
			else if ( pExistingRule != pRule )
			{
				*pExistingRule = *pRule;
				delete pRule;
			}
		}

	}

	// Check all lists for newly denied hosts
	PostMainWndMessage( WM_SANITY_CHECK );
}

void CSecurity::Remove(CSecureRule* pRule)
{
	CQuickLock oLock( m_pSection );

	if ( POSITION pos = m_pRules.Find( pRule ) )
	{
		m_pRules.RemoveAt( pos );
	}

	// this also accounts for double entries.
	if ( pRule->m_nType == CSecureRule::srAddress )
	{
		pRule->MaskFix();

		if ( pRule->m_nType == CSecureRule::srAddress &&
			*(DWORD*)pRule->m_nMask == 0xffffffff )
		{
			CAddressRuleMap::const_iterator i = m_pIPRules.find( *(DWORD*)pRule->m_nIP );
			if ( i != m_pIPRules.end() )
			{
				m_pIPRules.erase( i );
			}
		}
	}

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

	for ( CAddressRuleMap::const_iterator i = m_pIPRules.begin(); i != m_pIPRules.end(); ++i )
	{
		delete (*i).second;
	}

	m_pIPRules.clear();

	m_Cache.clear();
}

//////////////////////////////////////////////////////////////////////
// CSecurity ban

void CSecurity::Ban(const CShareazaFile* pFile, int nBanLength, BOOL bMessage, LPCTSTR szComment)
{
	CQuickLock oLock( m_pSection );

	DWORD tNow = static_cast< DWORD >( time( NULL ) );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		POSITION posLast = pos;
		CSecureRule* pRule = GetNext( pos );

		if ( pRule->IsExpired( tNow ) )
		{
			m_pRules.RemoveAt( posLast );
			delete pRule;
		}
		else if ( pRule->Match( pFile ) )
		{
			if ( pRule->m_nAction == CSecureRule::srDeny )
			{
				if ( ( nBanLength == banWeek ) && ( pRule->m_nExpire < tNow + 604000 ) )
				{
					pRule->m_nExpire = tNow + 604800;
				}
				else if ( ( nBanLength == banForever ) && ( pRule->m_nExpire != CSecureRule::srIndefinite ) )
				{
					pRule->m_nExpire = CSecureRule::srIndefinite;
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
		pRule->m_sComment	= _T("Session Ban");
		break;
	case ban5Mins:
		pRule->m_nExpire	= tNow + 300;
		pRule->m_sComment	= _T("Temp Ignore");
		break;
	case ban30Mins:
		pRule->m_nExpire	= tNow + 1800;
		pRule->m_sComment	= _T("Temp Ignore");
		break;
	case ban2Hours:
		pRule->m_nExpire	= tNow + 7200;
		pRule->m_sComment	= _T("Temp Ignore");
		break;
	case banWeek:
		pRule->m_nExpire	= tNow + 604800;
		pRule->m_sComment	= _T("Client Block");
		break;
	case banForever:
		pRule->m_nExpire	= CSecureRule::srIndefinite;
		pRule->m_sComment	= _T("Ban");
		break;
	default:
		pRule->m_nExpire	= CSecureRule::srSession;
		pRule->m_sComment	= _T("Session Ban");
	}

	if ( szComment )
		pRule->m_sComment = szComment;

	if ( pFile && ( pFile->m_oSHA1 || pFile->m_oED2K || pFile->m_oTiger ||
		pFile->m_oMD5 || pFile->m_oBTH ) )
	{
		pRule->m_nType = CSecureRule::srContentAny;
		pRule->SetContentWords(
			( pFile->m_oSHA1  ? pFile->m_oSHA1.toUrn()  + _T(" ") : CString() ) +
			( pFile->m_oED2K  ? pFile->m_oED2K.toUrn()  + _T(" ") : CString() ) +
			( pFile->m_oTiger ? pFile->m_oTiger.toUrn() + _T(" ") : CString() ) +
			( pFile->m_oMD5   ? pFile->m_oMD5.toUrn()   + _T(" ") : CString() ) +
			( pFile->m_oBTH   ? pFile->m_oBTH.toUrn()             : CString() ) );
	}

	Add( pRule );

	if ( bMessage && pFile )
		theApp.Message( MSG_NOTICE, IDS_SECURITY_BLOCKED, (LPCTSTR)pFile->m_sName );
}

void CSecurity::Ban(const IN_ADDR* pAddress, int nBanLength, BOOL bMessage, LPCTSTR szComment)
{
	CQuickLock oLock( m_pSection );

	DWORD tNow = static_cast< DWORD >( time( NULL ) );

	CAddressRuleMap::const_iterator i = m_pIPRules.find( *(DWORD*)pAddress );
	if ( i != m_pIPRules.end() )
	{
		CSecureRule* pIPRule = (*i).second;

		if ( pIPRule->m_nAction == CSecureRule::srDeny )
		{
			if ( ( nBanLength == banWeek ) && ( pIPRule->m_nExpire < tNow + 604000 ) )
			{
				pIPRule->m_nExpire = tNow + 604800;
			}
			else if ( ( nBanLength == banForever ) && ( pIPRule->m_nExpire != CSecureRule::srIndefinite ) )
			{
				pIPRule->m_nExpire = CSecureRule::srIndefinite;
			}
			return;
		}
	}

	CSecureRule* pIPRule = new CSecureRule();
	pIPRule->m_nAction	= CSecureRule::srDeny;
	pIPRule->m_nType	= CSecureRule::srAddress;

	switch ( nBanLength )
	{
	case banSession:
		pIPRule->m_nExpire	= CSecureRule::srSession;
		pIPRule->m_sComment	= _T("Session Ban");
		break;
	case ban5Mins:
		pIPRule->m_nExpire	= tNow + 300;
		pIPRule->m_sComment	= _T("Temp Ignore");
		break;
	case ban30Mins:
		pIPRule->m_nExpire	= tNow + 1800;
		pIPRule->m_sComment	= _T("Temp Ignore");
		break;
	case ban2Hours:
		pIPRule->m_nExpire	= tNow + 7200;
		pIPRule->m_sComment	= _T("Temp Ignore");
		break;
	case banWeek:
		pIPRule->m_nExpire	= tNow + 604800;
		pIPRule->m_sComment	= _T("Client Block");
		break;
	case banForever:
		pIPRule->m_nExpire	= CSecureRule::srIndefinite;
		pIPRule->m_sComment	= _T("Ban");
		break;
	default:
		pIPRule->m_nExpire	= CSecureRule::srSession;
		pIPRule->m_sComment	= _T("Session Ban");
	}

	if ( szComment )
		pIPRule->m_sComment = szComment;

	CopyMemory( pIPRule->m_nIP, pAddress, sizeof pIPRule->m_nIP );

	Add( pIPRule );

	if ( bMessage )
	{
		theApp.Message( MSG_NOTICE, IDS_SECURITY_BLOCKED,
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
		if ( pComplain->m_nExpire < nNow )
		{
			pComplain->m_nScore = 1;
		}
		else
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

	// First check the fast IP lookup map.
	CAddressRuleMap::const_iterator i = m_pIPRules.find( *(DWORD*)pAddress );
	if ( i != m_pIPRules.end() )
	{
		CSecureRule* pIPRule = (*i).second;
		if ( pIPRule->IsExpired( nNow ) )
		{
			m_pIPRules.erase( i );
		}
		else
		{
			pIPRule->m_nToday ++;
			pIPRule->m_nEver ++;

			if ( pIPRule->m_nExpire > CSecureRule::srSession && pIPRule->m_nExpire < nNow + 300 )
				// Add 5 min penalty for early access
				pIPRule->m_nExpire = nNow + 300;

			if ( pIPRule->m_nAction == CSecureRule::srAccept )
				return FALSE;
			else if ( pIPRule->m_nAction == CSecureRule::srDeny )
				return TRUE;
		}
	}

	// Second, check the miss cache if the IP has already been checked and found to be OK.
	// if the address is in cache, it is a miss and no lookup is needed
	if ( m_Cache.count( *(DWORD*) pAddress ) )
		return m_bDenyPolicy;

	// Third, check whether the IP is still stored in one of the old rules or the IP range blocking rules.
	for ( POSITION pos = GetIterator() ; pos ; )
	{
		POSITION posLast = pos;
		CSecureRule* pRule = GetNext( pos );

		if ( pRule->IsExpired( nNow ) )
		{
			m_pRules.RemoveAt( posLast );
			delete pRule;
		}
		else if ( pRule->Match( pAddress ) )
		{
			pRule->m_nToday ++;
			pRule->m_nEver ++;

			if ( pRule->m_nExpire > CSecureRule::srSession && pRule->m_nExpire < nNow + 300 )
				// Add 5 min penalty for early access
				pRule->m_nExpire = nNow + 300;

			if ( pRule->m_nAction == CSecureRule::srAccept )
				return FALSE;
			else if ( pRule->m_nAction == CSecureRule::srDeny )
				return TRUE;
		}
	}

	// If the IP is clean, add it to the miss cache
	m_Cache.insert( *(DWORD*) pAddress );

	// In this case, return our default policy
	return m_bDenyPolicy;
}

BOOL CSecurity::IsDenied(LPCTSTR pszContent)
{
	CQuickLock oLock( m_pSection );

	DWORD tNow = static_cast< DWORD >( time( NULL ) );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		POSITION posLast = pos;
		CSecureRule* pRule = GetNext( pos );

		if ( pRule->IsExpired( tNow ) )
		{
			m_pRules.RemoveAt( posLast );
			delete pRule;
		}
		else if ( pRule->Match( pszContent ) )
		{
			pRule->m_nToday ++;
			pRule->m_nEver ++;

			if ( pRule->m_nExpire > CSecureRule::srSession &&
				pRule->m_nExpire < tNow + 300 )
				// Add 5 min penalty for early access
				pRule->m_nExpire = tNow + 300;

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

	DWORD tNow = static_cast< DWORD >( time( NULL ) );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		POSITION posLast = pos;
		CSecureRule* pRule = GetNext( pos );

		if ( pRule->IsExpired( tNow ) )
		{
			m_pRules.RemoveAt( posLast );
			delete pRule;
		}
		else if ( pRule->Match( pFile ) )
		{
			pRule->m_nToday ++;
			pRule->m_nEver ++;

			if ( pRule->m_nExpire > CSecureRule::srSession &&
				pRule->m_nExpire < tNow + 300 )
				// Add 5 min penalty for early access
				pRule->m_nExpire = tNow + 300;

			if ( pRule->m_nAction == CSecureRule::srAccept )
				return FALSE;
			else if ( pRule->m_nAction == CSecureRule::srDeny )
				return TRUE;
		}
	}

	return m_bDenyPolicy;
}

BOOL CSecurity::IsDenied(const CQuerySearch* pQuery, const CString& strContent)
{
	CQuickLock oLock( m_pSection );

	DWORD tNow = static_cast< DWORD >( time( NULL ) );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		POSITION posLast = pos;
		CSecureRule* pRule = GetNext( pos );

		if ( pRule->IsExpired( tNow ) )
		{
			m_pRules.RemoveAt( posLast );
			delete pRule;
		}
		else if ( pRule->Match( pQuery, strContent ) )
		{
			pRule->m_nToday ++;
			pRule->m_nEver ++;

			if ( pRule->m_nAction == CSecureRule::srAccept )
				return FALSE;
			else if ( pRule->m_nAction == CSecureRule::srDeny )
				return TRUE;
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
			delete pRule;
		}
	}

	for ( CAddressRuleMap::const_iterator i = m_pIPRules.begin(); i != m_pIPRules.end(); )
	{
		if ( (*i).second->IsExpired( nNow ) )
		{
			i = m_pIPRules.erase( i );
		}
		else
		{
			++i;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CSecurity load and save

BOOL CSecurity::Load()
{
	CString strFile = Settings.General.UserPath + _T("\\Data\\Security.dat");

	CFile pFile;
	if ( pFile.Open( strFile, CFile::modeRead | CFile::shareDenyWrite | CFile::osSequentialScan ) )
	{
		try
		{
			CArchive ar( &pFile, CArchive::load, 131072 );	// 128 KB buffer
			try
			{
				CQuickLock oLock( m_pSection );

				Serialize( ar );

				ar.Close();
			}
			catch ( CException* pException )
			{
				ar.Abort();
				pFile.Abort();
				pException->Delete();
				theApp.Message( MSG_ERROR, _T("Failed to load security rules: %s"), (LPCTSTR)strFile );
				return FALSE;
			}
			pFile.Close();
		}
		catch ( CException* pException )
		{
			pFile.Abort();
			pException->Delete();
			theApp.Message( MSG_ERROR, _T("Failed to load security rules: %s"), (LPCTSTR)strFile );
			return FALSE;
		}
	}
	else
	{
		theApp.Message( MSG_ERROR, _T("Failed to load security rules: %s"), (LPCTSTR)strFile );
		return FALSE;
	}

	return TRUE;
}

BOOL CSecurity::Save()
{
	CString strTemp = Settings.General.UserPath + _T("\\Data\\Security.tmp");
	CString strFile = Settings.General.UserPath + _T("\\Data\\Security.dat");

	CFile pFile;
	if ( ! pFile.Open( strTemp, CFile::modeWrite | CFile::modeCreate | CFile::shareExclusive | CFile::osSequentialScan ) )
	{
		DeleteFile( strTemp );
		theApp.Message( MSG_ERROR, _T("Failed to save security rules: %s"), (LPCTSTR)strTemp );
		return FALSE;
	}

	try
	{
		CArchive ar( &pFile, CArchive::store, 131072 );	// 128 KB buffer
		try
		{
			CQuickLock oLock( m_pSection );

			Serialize( ar );

			ar.Close();
		}
		catch ( CException* pException )
		{
			ar.Abort();
			pFile.Abort();
			pException->Delete();
			DeleteFile( strTemp );
			theApp.Message( MSG_ERROR, _T("Failed to save security rules: %s"), (LPCTSTR)strTemp );
			return FALSE;
		}
		pFile.Close();
	}
	catch ( CException* pException )
	{
		pFile.Abort();
		pException->Delete();
		DeleteFile( strTemp );
		theApp.Message( MSG_ERROR, _T("Failed to save security rules: %s"), (LPCTSTR)strTemp );
		return FALSE;
	}

	if ( ! MoveFileEx( strTemp, strFile, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING ) )
	{
		DeleteFile( strTemp );
		theApp.Message( MSG_ERROR, _T("Failed to save security rules: %s"), (LPCTSTR)strFile );
		return FALSE;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSecurity serialize

void CSecurity::Serialize(CArchive& ar)
{
	int nVersion = SECURITY_SER_VERSION;

	if ( ar.IsStoring() )
	{
		ar << nVersion;
		ar << m_bDenyPolicy;

		ar.WriteCount( GetCount() );

		for ( POSITION pos = GetIterator() ; pos ; )
		{
			GetNext( pos )->Serialize( ar, nVersion );
		}

		for ( CAddressRuleMap::const_iterator i = m_pIPRules.begin(); i != m_pIPRules.end(); ++i )
		{
			(*i).second->Serialize( ar, nVersion );
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
				pRule->MaskFix();

				// special treatment for single IP security rules
				if ( pRule->m_nType == CSecureRule::srAddress &&
					*(DWORD*)pRule->m_nMask == 0xffffffff )
				{
					m_pIPRules[ *(DWORD*)pRule->m_nIP ] = pRule;
				}
				else
				{
					m_pRules.AddTail( pRule );
				}
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

		for ( CAddressRuleMap::const_iterator i = m_pIPRules.begin(); i != m_pIPRules.end(); ++i )
		{
			pXML->AddElement( (*i).second->ToXML() );
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
					pRule->MaskFix();

					// special treatment for single IP security rules
					if ( pRule->m_nType == CSecureRule::srAddress &&
						*(DWORD*)pRule->m_nMask == 0xffffffff )
					{
						m_pIPRules[ *(DWORD*)pRule->m_nIP ] = pRule;
					}
					else
					{
						m_pRules.AddTail( pRule );
					}
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

				pRule->MaskFix();

				// special treatment for single IP security rules
				if ( pRule->m_nType == CSecureRule::srAddress &&
					*(DWORD*)pRule->m_nMask == 0xffffffff )
				{
					m_pIPRules[ *(DWORD*) pRule->m_nIP ] = pRule;
				}
				else
				{
					m_pRules.AddTail( pRule );
				}

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

BOOL CSecurity::IsClientBad(const CString& sUserAgent) const
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

	// foxy - leecher client. (Tested, does not upload)
	// having something like Authentication which is not defined on specification
	if ( _tcsistr( sUserAgent, _T("foxy") ) )						return TRUE;

	// Check by content filter
	// TODO: Implement user agent filter type
	return IsDenied( sUserAgent );
}

BOOL CSecurity::IsAgentBlocked(const CString& sUserAgent) const
{
	// The remote computer didn't send a "User-Agent", or it sent whitespace
	if ( sUserAgent.IsEmpty() )										return TRUE;

	// Loop through the list of programs to block
	for ( string_set::const_iterator i = Settings.Uploads.BlockAgents.begin() ;
		i != Settings.Uploads.BlockAgents.end(); ++i )
	{
		if ( _tcsistr( sUserAgent, *i ) )							return TRUE;
	}

	// Allow it
	return FALSE;
}

BOOL CSecurity::IsVendorBlocked(const CString& sVendor) const
{
	// foxy - leecher client. (Tested, does not upload)
	// having something like Authentication which is not defined on specification
	if ( _tcsistr( sVendor, _T("foxy") ) )							return TRUE;

	// Allow it
	return FALSE;
}

CLiveList* CSecurity::GetList() const
{
	CQuickLock oLock( m_pSection );

	CLiveList* pLiveList = new CLiveList( 6, (UINT)GetCount() + (UINT)GetCount() / 4u );

	if ( CLiveItem* pDefault = pLiveList->Add( (LPVOID)0 ) )
	{
		pDefault->Set( 0, _T("Default Policy") );
		pDefault->Set( 1, m_bDenyPolicy ? _T("Deny") : _T("Accept") );
		pDefault->Set( 3, _T("X") );
		pDefault->SetImage( 0, m_bDenyPolicy ? 2 : 1 );
	}

	DWORD tNow = static_cast< DWORD >( time( NULL ) );

	int nCount = 1;
	for ( POSITION pos = GetIterator() ; pos ; ++nCount )
	{
		GetNext( pos )->ToList( pLiveList, nCount, tNow );
	}

	for ( CAddressRuleMap::const_iterator i = m_pIPRules.begin(); i != m_pIPRules.end(); ++i, ++nCount )
	{
		(*i).second->ToList( pLiveList, nCount, tNow );
	}

	return pLiveList;
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

BOOL CAdultFilter::IsHitAdult(LPCTSTR pszText) const
{
	if ( pszText )
	{
		return IsFiltered( pszText );
	}
	return FALSE;
}

BOOL CAdultFilter::IsSearchFiltered(LPCTSTR pszText) const
{
	if ( Settings.Search.AdultFilter && pszText )
	{
		return IsFiltered( pszText );
	}
	return FALSE;
}

BOOL CAdultFilter::IsChatFiltered(LPCTSTR pszText) const
{
	if ( Settings.Community.ChatCensor && pszText )
	{
		return IsFiltered( pszText );
	}
	return FALSE;
}

BOOL CAdultFilter::Censor(CString& sText) const
{
	BOOL bModified = FALSE;

	// Check and replace blocked words
	for ( LPCTSTR pszWord = m_pszBlockedWords ; pszWord && *pszWord ; )
	{
		int nWordLen = (int)_tcslen( pszWord );
		if ( ReplaceNoCase( sText, pszWord, CString( _T('*'), nWordLen ) ) )
			bModified = TRUE;
		pszWord += nWordLen + 1;
	}

	return bModified;
}

BOOL CAdultFilter::IsChildPornography(LPCTSTR pszText) const
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

BOOL CAdultFilter::IsFiltered(LPCTSTR pszText) const
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
			size_t nDubiousWords = 0, nWordsPermitted = min( (int)_tcslen( pszText ) / 8, 4 );

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
