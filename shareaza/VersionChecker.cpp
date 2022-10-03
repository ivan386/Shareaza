//
// VersionChecker.cpp
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
#include "DiscoveryServices.h"
#include "GProfile.h"
#include "Library.h"
#include "Network.h"
#include "SharedFile.h"
#include "Transfer.h"
#include "VersionChecker.h"

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
	m_bVerbose( false )
{
}

CVersionChecker::~CVersionChecker()
{
	Stop();
}

void CVersionChecker::ForceCheck()
{
	m_bVerbose = true;
	Settings.VersionCheck.UpdateCheck = true;
	Settings.VersionCheck.NextCheck = 0;
	Start();
}

void CVersionChecker::ClearVersionCheck()
{
	Settings.VersionCheck.UpgradePrompt.Empty();
	Settings.VersionCheck.UpgradeFile.Empty();
	Settings.VersionCheck.UpgradeSHA1.Empty();
	Settings.VersionCheck.UpgradeTiger.Empty();
	Settings.VersionCheck.UpgradeSize.Empty();
	Settings.VersionCheck.UpgradeSources.Empty();
	Settings.VersionCheck.UpgradeVersion.Empty();
}

BOOL CVersionChecker::IsVersionNewer()
{
	WORD nVersion[ 4 ];
	return ( _stscanf_s( Settings.VersionCheck.UpgradeVersion, _T("%hu.%hu.%hu.%hu"),
		&nVersion[ 0 ], &nVersion[ 1 ], &nVersion[ 2 ], &nVersion[ 3 ] ) == 4 ) &&
		( theApp.m_nVersion[ 0 ] < nVersion[ 0 ] ||
		( theApp.m_nVersion[ 0 ] == nVersion[ 0 ] &&
		( theApp.m_nVersion[ 1 ] < nVersion[ 1 ] ||
		( theApp.m_nVersion[ 1 ] == nVersion[ 1 ] &&
		( theApp.m_nVersion[ 2 ] < nVersion[ 2 ] ||
		( theApp.m_nVersion[ 2 ] == nVersion[ 2 ] &&
		( theApp.m_nVersion[ 3 ] < nVersion[ 3 ] ) ) ) ) ) ) );
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker time check

BOOL CVersionChecker::NeedToCheck()
{
	if ( ! IsVersionNewer() ) // user manually upgraded
	{
		ClearVersionCheck();
	}

	if ( ! Settings.VersionCheck.UpdateCheck ) return FALSE;
	if ( ! Settings.VersionCheck.NextCheck ) return TRUE;
	if ( Settings.VersionCheck.NextCheck == 0xFFFFFFFF ) return FALSE;

	return (DWORD)CTime::GetCurrentTime().GetTime() >= Settings.VersionCheck.NextCheck;
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker start

BOOL CVersionChecker::Start()
{
	if ( IsThreadAlive() )
		return TRUE;

	m_pRequest.Clear();

	return BeginThread( "VersionChecker" );
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker stop

void CVersionChecker::Stop()
{
	if ( IsThreadAlive() )
	{
		m_pRequest.Cancel();
		CloseThread();
	}
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker thread run

void CVersionChecker::OnRun()
{
	if ( NeedToCheck() )
	{
		if ( ExecuteRequest() )
		{
			ProcessResponse();
		}
		else
		{
			SetNextCheck( VERSIONCHECKER_FREQUENCY );
		}

		if ( IsThreadEnabled() )
			PostMainWndMessage( WM_VERSIONCHECK, VC_MESSAGE_AND_CONFIRM );
	}
}

//////////////////////////////////////////////////////////////////////
// CVersionChecker undertake request

BOOL CVersionChecker::ExecuteRequest()
{
	CString strURL = Settings.VersionCheck.UpdateCheckURL
		+ _T("?Version=") + theApp.m_sVersion
#ifdef _WIN64
		+ _T("&Platform=Win64")
#else
		+ _T("&Platform=Win32")
#endif
	;
	if ( ! m_pRequest.SetURL( strURL ) )
		return FALSE;
	
	theApp.Message( MSG_DEBUG | MSG_FACILITY_OUTGOING, _T("[VersionChecker] Request: %s"), (LPCTSTR)strURL );

	if ( ! m_pRequest.Execute( false ) )
		return FALSE;

	int nStatusCode = m_pRequest.GetStatusCode();
	if ( nStatusCode < 200 || nStatusCode > 299 )
		return FALSE;

	CString strOutput = m_pRequest.GetResponseString();

	theApp.Message( MSG_DEBUG | MSG_FACILITY_INCOMING, _T("[VersionChecker] Response: %s"), (LPCTSTR)strOutput );

	for ( strOutput += '&' ; strOutput.GetLength() ; )
	{
		CString strItem	= strOutput.SpanExcluding( _T("&") );
		strOutput		= strOutput.Mid( strItem.GetLength() + 1 );

		CString strKey = strItem.SpanExcluding( _T("=") );
		if ( strKey.GetLength() == strItem.GetLength() )
			continue;

		strItem = URLDecode( strItem.Mid( strKey.GetLength() + 1 ) );
		strItem.Trim();

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
			if ( strLine.GetLength() ) theApp.Message( MSG_NOTICE, strLine );
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
	}
	else
	{
		ClearVersionCheck();
	}

	if ( ! IsVersionNewer() )
	{
		ClearVersionCheck();
	}
	
	if ( m_pResponse.Lookup( _T("AddDiscovery"), strValue ) )
	{
		strValue.Trim();
		theApp.Message( MSG_DEBUG, _T("[VersionChecker] %s = %s"), _T("AddDiscovery"), (LPCTSTR)strValue );
		DiscoveryServices.Add( strValue, CDiscoveryService::dsWebCache );
	}

	if ( m_pResponse.Lookup( _T("AddDiscoveryUHC"), strValue ) )
	{
		strValue.Trim();
		theApp.Message( MSG_DEBUG, _T("[VersionChecker] %s = %s"), _T("AddDiscoveryUHC"), (LPCTSTR)strValue );
		DiscoveryServices.Add( strValue, CDiscoveryService::dsGnutella, PROTOCOL_G1 );
	}

	if ( m_pResponse.Lookup( _T("AddDiscoveryKHL"), strValue ) )
	{
		strValue.Trim();
		theApp.Message( MSG_DEBUG, _T("[VersionChecker] %s = %s"), _T("AddDiscoveryKHL"), (LPCTSTR)strValue );
		DiscoveryServices.Add( strValue, CDiscoveryService::dsGnutella, PROTOCOL_G2 );
	}

	if ( m_pResponse.Lookup( _T("NextCheck"), strValue ) )
	{
		strValue.Trim();
		theApp.Message( MSG_DEBUG, _T("[VersionChecker] %s = %s"), _T("NextCheck"), (LPCTSTR)strValue );
		_stscanf( strValue, _T("%d"), &nDays );
	}
	
	SetNextCheck( nDays );

	m_pResponse.RemoveAll();
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

BOOL CVersionChecker::CheckUpgradeHash(const CLibraryFile* pFile)
{
	if ( IsUpgradeAvailable() )
	{
		CShareazaFile oFilter;
		if ( oFilter.m_oSHA1.fromString( Settings.VersionCheck.UpgradeSHA1 ) )
		{
			oFilter.m_nSize = _tstoi64( Settings.VersionCheck.UpgradeSize );

			CQuickLock oLock( Library.m_pSection );
			if ( ! pFile )
				pFile = LibraryMaps.LookupFileByHash( &oFilter, FALSE, TRUE );

			if ( pFile &&
				validAndEqual( pFile->m_oSHA1, oFilter.m_oSHA1 ) &&
				 pFile->m_nSize == oFilter.m_nSize &&
				_tcsicmp( PathFindExtension( pFile->GetPath() ), _T(".exe") ) == 0 )
			{
				m_sUpgradePath = pFile->GetPath();
				PostMainWndMessage( WM_VERSIONCHECK, VC_UPGRADE );
				return TRUE;
			}
		}
	}
	return FALSE;
}
