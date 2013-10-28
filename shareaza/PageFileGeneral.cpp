//
// PageFileGeneral.cpp
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
#include "Library.h"
#include "SharedFolder.h"
#include "SharedFile.h"
#include "ShellIcons.h"
#include "PageFileGeneral.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CFileGeneralPage, CFilePropertiesPage)

BEGIN_MESSAGE_MAP(CFileGeneralPage, CFilePropertiesPage)
	//{{AFX_MSG_MAP(CFileGeneralPage)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFileGeneralPage property page

CFileGeneralPage::CFileGeneralPage() : 
	CFilePropertiesPage( CFileGeneralPage::IDD ),
	m_sSHA1(), m_sTiger(), m_sType(), m_sSize(), m_sPath(),
	m_sModified(), m_sIndex(), m_sMD5(), m_sED2K()
{
}

CFileGeneralPage::~CFileGeneralPage()
{
}

void CFileGeneralPage::DoDataExchange(CDataExchange* pDX)
{
	CFilePropertiesPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFileGeneralPage)
	DDX_Text(pDX, IDC_FILE_SHA1, m_sSHA1);
	DDX_Text(pDX, IDC_FILE_TIGER, m_sTiger);
	DDX_Text(pDX, IDC_FILE_TYPE, m_sType);
	DDX_Text(pDX, IDC_FILE_SIZE, m_sSize);
	DDX_Text(pDX, IDC_FILE_PATH, m_sPath);
	DDX_Text(pDX, IDC_FILE_MODIFIED, m_sModified);
	DDX_Text(pDX, IDC_FILE_INDEX, m_sIndex);
	DDX_Text(pDX, IDC_FILE_MD5, m_sMD5);
	DDX_Text(pDX, IDC_FILE_ED2K, m_sED2K);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CFileGeneralPage message handlers

BOOL CFileGeneralPage::OnInitDialog()
{
	CFilePropertiesPage::OnInitDialog();

	{
		CQuickLock oLock( Library.m_pSection );
		CLibraryFile* pFile = GetFile();
		if ( ! pFile ) return TRUE;

		m_sPath = pFile->GetFolder();
		m_sSize = Settings.SmartVolume( pFile->GetSize() );
		m_sType = ShellIcons.GetTypeString( pFile->m_sName );
		m_sIndex.Format( _T("# %lu"), pFile->m_nIndex );

		m_sSHA1 = pFile->m_oSHA1.toShortUrn();		
		m_sTiger = pFile->m_oTiger.toShortUrn();
		m_sMD5 = pFile->m_oMD5.toShortUrn();
		m_sED2K = pFile->m_oED2K.toShortUrn();
		
		if ( m_sSHA1.IsEmpty() && m_sED2K.IsEmpty() && m_sTiger.IsEmpty() && m_sMD5.IsEmpty() )
		{
			LoadString(m_sSHA1, IDS_GENERAL_NOURNAVAILABLE );
		}

		CString strDate, strTime;
		SYSTEMTIME pTime;

		FileTimeToSystemTime( &pFile->m_pTime, &pTime );
		SystemTimeToTzSpecificLocalTime( NULL, &pTime, &pTime );

		GetDateFormat( LOCALE_USER_DEFAULT, DATE_LONGDATE, &pTime, NULL, strDate.GetBuffer( 64 ), 64 );
		GetTimeFormat( LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT, &pTime, NULL, strTime.GetBuffer( 64 ), 64 );
		strDate.ReleaseBuffer(); strTime.ReleaseBuffer();

		m_sModified = strDate + _T(", ") + strTime;
	}

	UpdateData( FALSE );

	return TRUE;
}

void CFileGeneralPage::OnOK()
{
	UpdateData();

	// Nothing to update now

	CFilePropertiesPage::OnOK();
}

