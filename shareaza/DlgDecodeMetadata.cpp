//
// DlgDecodeMetadata.cpp
//
//	Date:			"$Date: 2005/03/11 16:28:40 $"
//	Revision:		"$Revision: 1.2 $"
//  Last change by:	"$Author: thetruecamper $"
//
// Copyright (c) Shareaza Development Team, 2002-2005.
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
#include "Library.h"
#include "SharedFile.h"
#include "XML.h"
#include "Schema.h"
#include "DlgDecodeMetadata.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDecodeMetadataDlg dialog

CDecodeMetadataDlg::CDecodeMetadataDlg(CWnd* pParent) : CSkinDialog(CDecodeMetadataDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDecodeMetadataDlg)
	//}}AFX_DATA_INIT
}

void CDecodeMetadataDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDecodeMetadataDlg)
	DDX_Control(pDX, IDC_CODEPAGES, m_wndCodepages);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CDecodeMetadataDlg message handlers

BOOL CDecodeMetadataDlg::OnInitDialog() 
{
	CSkinDialog::OnInitDialog();
	
	SkinMe( _T("CDecodeMetadataDlg"), IDI_WORLD );
	UpdateData( FALSE );

	return TRUE;
}

void CDecodeMetadataDlg::AddFile(CLibraryFile* pFile)
{
	m_pFiles.AddTail( (LPVOID)pFile->m_nIndex );
}

// TODO: Allow to restore initial metadata from backup 
// if bad encoding was applied by mistake.
//
void CDecodeMetadataDlg::OnOK() 
{
	UpdateData();
	static const unsigned codePages[ 13 ] =
	{
		// arabic, baltic, centr. european, chinese simplified, chinese traditional
		1256, 1257, 1250, 936,  950,
		// cyrillic, greek, hebrew, japanese, korean, thai, turkish, vietnamese
		1251, 1253, 1255,  932,  949, 874, 1254, 1258
	};
	unsigned nCodePage = m_wndCodepages.GetCurSel();
	nCodePage = nCodePage < 13 ? codePages[ nCodePage ] : 1252; // english

	// close dialog and perform decoding in background
	CSkinDialog::OnOK();

	for ( POSITION pos = m_pFiles.GetHeadPosition() ; pos ; )
	{
		DWORD nIndex = (DWORD)m_pFiles.GetNext( pos );
		
		CXMLElement* pXML;
		CLibraryFile* pFile;

		{
			CQuickLock oLock( Library.m_pSection );

			if ( m_pFiles.IsEmpty() ) break;

			pFile = Library.LookupFile( nIndex );
			if ( !pFile || !pFile->m_pMetadata || !pFile->m_pSchema ) continue;

			pXML = pFile->m_pMetadata->Clone();
		}

		for ( POSITION pos = pXML->GetAttributeIterator() ; pos ; )
		{
			CXMLAttribute* pAttribute = pXML->GetNextAttribute( pos );
			// decode only these attributes
			if ( !pAttribute->IsNamed( _T("artist") ) && 
				 !pAttribute->IsNamed( _T("album") ) &&
				 !pAttribute->IsNamed( _T("title") ) &&
				 !pAttribute->IsNamed( _T("description") ) ) continue;
			CString strAttribute = pAttribute->GetValue();

			int nLength = strAttribute.GetLength();
			LPTSTR pszSource = strAttribute.GetBuffer( nLength );
			CHAR* pszDest = new CHAR[ nLength + 1 ];

			{
				const TCHAR* source = pszSource;
				CHAR* dest = pszDest;
				while ( *dest++ = static_cast< CHAR >( *source ), *source++ );
			}

			int nWide = MultiByteToWideChar( nCodePage, 0, pszDest, nLength, NULL, 0 );
			LPTSTR pszOutput = strAttribute.GetBuffer( nWide + 1 );
			MultiByteToWideChar( nCodePage, 0, pszDest, nLength, pszOutput, nWide );
			pszOutput[ nWide ] = 0;

			delete pszDest;
			strAttribute.ReleaseBuffer();

			pAttribute->SetValue( strAttribute );
		}

		CQuickLock oLock( Library.m_pSection );
		// make a clean copy of schema with namespace included
		CXMLElement* pContainer	= pFile->m_pSchema->Instantiate( TRUE );
		if ( pContainer )
		{
			// append modified metadata
			CXMLElement* pMetadata	= pContainer->AddElement( pXML );
			// save metadata by creating XML file
			pFile->SetMetadata( pContainer );
			delete pContainer;
		}
	}
}
