//
// VersionChecker.cpp
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
#include "VersionChecker.h"
#include "Library.h"
#include "SharedFile.h"
#include "Transfer.h"
#include "Network.h"
#include "GProfile.h"
#include "DiscoveryServices.h"
#include "SHA.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CVersionChecker VersionChecker;

#define VERSIONCHECKER_FREQUENCY	7


//////////////////////////////////////////////////////////////////////
// CVersionChecker construction

CVersionChecker::CVersionChecker() :
	m_bUpgrade( FALSE ),
	m_hThread( NULL )
{
}

CVersionChecker::~CVersionChecker()
{
	Stop();
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker time check

BOOL CVersionChecker::NeedToCheck()
{
	if ( theApp.m_sVersion.Compare( Settings.VersionCheck.UpgradeVersion ) >= 0 ) // user manually upgraded
	{
		Settings.VersionCheck.UpgradePrompt = _T("");
		Settings.VersionCheck.UpgradeFile = _T("");
		Settings.VersionCheck.UpgradeSHA1 = _T("");
		Settings.VersionCheck.UpgradeTiger = _T("");
		Settings.VersionCheck.UpgradeSize = _T("");
		Settings.VersionCheck.UpgradeSources = _T("");
		Settings.VersionCheck.UpgradeVersion = _T("");
		return (DWORD)CTime::GetCurrentTime().GetTime() >= Settings.VersionCheck.NextCheck;
	}

	if ( ! Settings.VersionCheck.UpdateCheck ) return FALSE;

	m_bUpgrade = ! Settings.VersionCheck.UpgradePrompt.IsEmpty();

	if ( ! Settings.VersionCheck.NextCheck ) return TRUE;
	if ( Settings.VersionCheck.NextCheck == 0xFFFFFFFF ) return FALSE;

	return (DWORD)CTime::GetCurrentTime().GetTime() >= Settings.VersionCheck.NextCheck;
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker start

BOOL CVersionChecker::Start()
{
	Stop();
	
	m_pRequest.Clear();
	
	m_bUpgrade = FALSE;
	
	m_hThread = BeginThread( "VersionChecker", ThreadStart, this, THREAD_PRIORITY_IDLE );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker stop

void CVersionChecker::Stop()
{
	if ( m_hThread )
	{
		SetThreadPriority( m_hThread, THREAD_PRIORITY_NORMAL );
		m_pRequest.Cancel();
		CloseThread( &m_hThread );
	}
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker thread bootstrap

UINT CVersionChecker::ThreadStart(LPVOID pParam)
{
	CVersionChecker* pClass = (CVersionChecker*)pParam;
	pClass->OnRun();
	return 0;
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker thread run

void CVersionChecker::OnRun()
{
	if ( NeedToCheck() && ! CheckUpgradeHash() )
	{
		if ( ExecuteRequest() )
		{
			ProcessResponse();
			m_pResponse.RemoveAll();
		}
		else
		{
			SetNextCheck( VERSIONCHECKER_FREQUENCY );
		}
	}
	m_hThread = NULL;
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker undertake request

BOOL CVersionChecker::ExecuteRequest()
{
	m_pRequest.SetURL( Settings.VersionCheck.UpdateCheckURL +
		_T("?Version=") + theApp.m_sVersion );

	if ( ! m_pRequest.Execute( FALSE ) ) return FALSE;

	int nStatusCode = m_pRequest.GetStatusCode();
	if ( nStatusCode < 200 || nStatusCode > 299 ) return FALSE;

	CString strResponse = m_pRequest.GetResponseString();
	for ( strResponse += '&' ; strResponse.GetLength() ; )
	{
		CString strItem	= strResponse.SpanExcluding( _T("&") );
		strResponse		= strResponse.Mid( strItem.GetLength() + 1 );

		CString strKey = strItem.SpanExcluding( _T("=") );
		if ( strKey.GetLength() == strItem.GetLength() ) continue;
		strItem = URLDecode( strItem.Mid( strKey.GetLength() + 1 ) );

		strItem.TrimLeft();
		strItem.TrimRight();

		m_pResponse.SetAt( strKey, strItem );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker process response

void CVersionChecker::ProcessResponse()
{
	int nDays = VERSIONCHECKER_FREQUENCY;
	CString strValue;
	
	if ( m_pResponse.Lookup( _T("Message"), strValue ) ||
		 m_pResponse.Lookup( _T("MessageBox"), strValue ) )
	{
		m_sMessage = strValue;
	}
	
	if ( m_pResponse.Lookup( _T("Quote"), strValue ) )
	{
		Settings.VersionCheck.Quote = strValue;
	}
	
	if ( m_pResponse.Lookup( _T("SystemMsg"), strValue ) )
	{
		for ( strValue += '\n' ; strValue.GetLength() ; )
		{
			CString strLine	= strValue.SpanExcluding( _T("\r\n") );
			strValue		= strValue.Mid( strLine.GetLength() + 1 );
			if ( strLine.GetLength() ) theApp.Message( MSG_SYSTEM, strLine );
		}
	}
	
	if ( m_pResponse.Lookup( _T("UpgradePrompt"), strValue ) )
	{
		Settings.VersionCheck.UpgradePrompt = strValue;
		
		m_pResponse.Lookup( _T("UpgradeFile"), Settings.VersionCheck.UpgradeFile );
		m_pResponse.Lookup( _T("UpgradeSHA1"), Settings.VersionCheck.UpgradeSHA1 );
		m_pResponse.Lookup( _T("UpgradeTiger"), Settings.VersionCheck.UpgradeTiger );
		m_pResponse.Lookup( _T("UpgradeSize"), Settings.VersionCheck.UpgradeSize );
		m_pResponse.Lookup( _T("UpgradeSources"), Settings.VersionCheck.UpgradeSources );
		m_pResponse.Lookup( _T("UpgradeVersion"), Settings.VersionCheck.UpgradeVersion );

		// Old name
		if ( ! Settings.VersionCheck.UpgradeSHA1.GetLength() )
			m_pResponse.Lookup( _T("UpgradeHash"), Settings.VersionCheck.UpgradeSHA1 );
		
		m_bUpgrade = TRUE;
	}
	else
	{
		Settings.VersionCheck.UpgradePrompt = _T("");
		Settings.VersionCheck.UpgradeFile = _T("");
		Settings.VersionCheck.UpgradeSHA1 = _T("");
		Settings.VersionCheck.UpgradeTiger = _T("");
		Settings.VersionCheck.UpgradeSize = _T("");
		Settings.VersionCheck.UpgradeSources = _T("");
		Settings.VersionCheck.UpgradeVersion = _T("");
		m_bUpgrade = FALSE;
	}
	
	if ( m_pResponse.Lookup( _T("AddDiscovery"), strValue ) )
	{
		strValue.Trim();
		DiscoveryServices.Add( strValue, CDiscoveryService::dsWebCache );
	}

	if ( m_pResponse.Lookup( _T("AddDiscoveryUHC"), strValue ) )
	{
		strValue.Trim();
		DiscoveryServices.Add( strValue, CDiscoveryService::dsGnutella, PROTOCOL_G1 );
	}

	if ( m_pResponse.Lookup( _T("AddDiscoveryKHL"), strValue ) )
	{
		strValue.Trim();
		DiscoveryServices.Add( strValue, CDiscoveryService::dsGnutella, PROTOCOL_G2 );
	}

	if ( m_pResponse.Lookup( _T("NextCheck"), strValue ) )
	{
		_stscanf( strValue, _T("%lu"), &nDays );
	}
	
	SetNextCheck( nDays );
	
	AfxGetMainWnd()->PostMessage( WM_VERSIONCHECK, 0, 0 );
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker NextCheck update

void CVersionChecker::SetNextCheck(int nDays)
{
	CTimeSpan tPeriod( nDays, 0, 0, 0 );
	CTime tNextCheck = CTime::GetCurrentTime() + tPeriod;
	Settings.VersionCheck.NextCheck = (DWORD)tNextCheck.GetTime();
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker check if a download is an upgrade

BOOL CVersionChecker::CheckUpgradeHash(const Hashes::Sha1Hash& oHash, LPCTSTR pszPath)
{
	if ( m_bUpgrade )
	{
		if ( oHash.toString() == Settings.VersionCheck.UpgradeSHA1 )
		{
			if ( _tcsstr( pszPath, _T(".exe") ) )
			{
				m_sUpgradePath = pszPath;
				AfxGetMainWnd()->PostMessage( WM_VERSIONCHECK, 2 );
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL CVersionChecker::CheckUpgradeHash()
{
	if ( m_bUpgrade )
	{
		Hashes::Sha1Hash oSHA1;
		if ( oSHA1.fromString( Settings.VersionCheck.UpgradeSHA1 ) )
		{
			CQuickLock oLock( Library.m_pSection );
			CLibraryFile* pFile = LibraryMaps.LookupFileBySHA1( oSHA1 );
			if ( pFile )
			{
				if ( _tcsstr( pFile->GetPath(), _T(".exe") ) )
				{
					m_sUpgradePath = pFile->GetPath();
					AfxGetMainWnd()->PostMessage( WM_VERSIONCHECK, 2 );
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}
