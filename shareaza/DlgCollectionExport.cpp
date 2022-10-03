//
// DlgCollectionExport.cpp
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
#include "AlbumFolder.h"
#include "CoolInterface.h"
#include "DlgCollectionExport.h"
#include "ImageFile.h"
#include "Library.h"
#include "LibraryFolders.h"
#include "LiveList.h"
#include "Schema.h"
#include "Settings.h"
#include "SharedFile.h"
#include "ThumbCache.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CCollectionExportDlg, CSkinDialog)

BEGIN_MESSAGE_MAP(CCollectionExportDlg, CSkinDialog)
	ON_WM_CTLCOLOR()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDOK, &CCollectionExportDlg::OnOK)
	ON_BN_CLICKED(IDC_TEMPLATES_DELETE, &CCollectionExportDlg::OnTemplatesDeleteOrBack)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_TEMPLATES, &CCollectionExportDlg::OnItemChangedTemplates)
END_MESSAGE_MAP()


CCollectionExportDlg::CCollectionExportDlg(const CAlbumFolder* pFolder, CWnd* pParent)
	: CSkinDialog	( CCollectionExportDlg::IDD, pParent )
	, m_pFolder		( pFolder )
	, m_nSelected	( -1 )
	, m_nStep		( 0 )
	, m_bThumbnails	( FALSE )
{
}

CCollectionExportDlg::~CCollectionExportDlg()
{
}

void CCollectionExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange( pDX );

	DDX_Control(pDX, IDOK, m_wndOK);
	DDX_Control(pDX, IDC_STATIC_AUTHOR, m_wndLblAuthor);
	DDX_Control(pDX, IDC_STATIC_NAME, m_wndLblName);
	DDX_Control(pDX, IDC_STATIC_DESC, m_wndLblDesc);
	DDX_Control(pDX, IDC_STATIC_GROUPBOX, m_wndGroupBox);
	DDX_Control(pDX, IDC_TEMPLATES_EXPLAIN, m_wndExplain);
	DDX_Control(pDX, IDC_TEMPLATES_DELETE, m_wndDelete);
	DDX_Control(pDX, IDC_TEMPLATE_DESC, m_wndDesc);
	DDX_Control(pDX, IDC_TEMPLATE_NAME, m_wndName);
	DDX_Control(pDX, IDC_TEMPLATE_AUTHOR, m_wndAuthor);
	DDX_Control(pDX, IDC_TEMPLATES, m_wndList);
}

BOOL CCollectionExportDlg::OnInitDialog()
{
	CSkinDialog::OnInitDialog();

	m_gdiImageList.Create( 16, 16, ILC_COLOR32|ILC_MASK, 1, 1 ) ||
	m_gdiImageList.Create( 16, 16, ILC_COLOR24|ILC_MASK, 1, 1 ) ||
	m_gdiImageList.Create( 16, 16, ILC_COLOR16|ILC_MASK, 1, 1 );
	AddIcon( IDI_SKIN, m_gdiImageList );

	m_wndList.SetImageList( &m_gdiImageList, LVSIL_SMALL );

	//Show template name, author and version number columns, hide the rest info
	m_wndList.InsertColumn( 0, _T("Name"), LVCFMT_LEFT, 134, 0 );
	m_wndList.InsertColumn( 1, _T("Author"), LVCFMT_LEFT, 114, 1 );
	m_wndList.InsertColumn( 2, _T("Version"), LVCFMT_LEFT, 32, 2 );
	m_wndList.InsertColumn( 3, _T("Path"), LVCFMT_LEFT, 0, 3 );
	m_wndList.InsertColumn( 4, _T("URL"), LVCFMT_LEFT, 0, 4 );
	m_wndList.InsertColumn( 5, _T("Email"), LVCFMT_LEFT, 0, 5 );
	m_wndList.InsertColumn( 6, _T("Description"), LVCFMT_LEFT, 0, 6 );
	m_wndList.InsertColumn( 7, _T("Thumbnails"), LVCFMT_LEFT, 0, 7 );

	m_wndList.SendMessage( LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT );

	//Translate window
	SkinMe( _T("CCollectionExportDlg"), IDI_COLLECTION );

	if ( Settings.General.LanguageRTL ) 
		m_wndDesc.ModifyStyleEx( WS_EX_RTLREADING|WS_EX_RIGHT|WS_EX_LEFTSCROLLBAR, 
			WS_EX_LTRREADING|WS_EX_LEFT|WS_EX_RIGHTSCROLLBAR, 0 );

	m_nSelected = -1;
	m_wndName.SetWindowText( _T("") );
	m_wndAuthor.SetWindowText( _T("") );
	
	//Get label and button caption for the first screen, save the rest to variables for later use.
	CString str;
	m_wndOK.GetWindowText( str );
	int nPos = str.Find( '|' );
	if ( nPos > 0 )
	{
		m_sBtnNext = str.Left( nPos );
		m_sBtnExport = str.Mid( nPos + 1 );
		m_wndOK.SetWindowText( m_sBtnNext );
	}

	m_wndExplain.GetWindowText( str );
	nPos = str.Find( '|' );
	if ( nPos > 0 )
	{
		m_sLblExplain1 = str.Left( nPos );
		m_sLblExplain2 = str.Mid( nPos + 1 );
		m_wndExplain.SetWindowText( m_sLblExplain1 );
	}

	m_wndDelete.GetWindowText( str );
	nPos = str.Find( '|' );
	if ( nPos > 0 )
	{
		m_sBtnDelete = str.Left( nPos );
		m_sBtnBack = str.Mid( nPos + 1 );
		m_wndDelete.SetWindowText( m_sBtnDelete );
	}

	m_wndDelete.EnableWindow( FALSE );
	m_wndOK.EnableWindow( FALSE );
	m_nStep = 1;

	CWaitCursor pCursor;
	
	//Get templates info from Templates folder and fill in the list
	EnumerateTemplates();

	return TRUE;
}

void CCollectionExportDlg::EnumerateTemplates(LPCTSTR pszPath)
{
	WIN32_FIND_DATA pFind;
	CString strPath;
	HANDLE hSearch;

	strPath.Format( _T("%s\\Templates\\%s*.*"),
		(LPCTSTR)Settings.General.Path, pszPath ? pszPath : _T("") );

	hSearch = FindFirstFile( strPath, &pFind );

	if ( hSearch != INVALID_HANDLE_VALUE )
	{
		do
		{
			if ( pFind.cFileName[0] == _T('.') )
				continue;

			if ( pFind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				strPath.Format( _T("%s%s\\"),
					pszPath ? pszPath : _T(""), pFind.cFileName );
				
				EnumerateTemplates( strPath );
			}
			else if ( _tcsistr( pFind.cFileName, _T(".xml") ) != NULL )
			{
				AddTemplate( pszPath, pFind.cFileName );
			}
		}
		while ( FindNextFile( hSearch, &pFind ) );

		FindClose( hSearch );
	}
}

BOOL CCollectionExportDlg::AddTemplate(LPCTSTR pszPath, LPCTSTR pszName)
{
	CString strXML = Settings.General.Path + _T("\\Templates\\");
	if ( pszPath ) strXML += pszPath;
	strXML += pszName;

	strXML = LoadFile( strXML );
	if ( strXML.IsEmpty() ) return FALSE;

	CXMLElement* pXML = NULL;
	
	int nManifest = strXML.Find( _T("<manifest") );
	
	if ( nManifest > 0 )
	{
		CString strManifest = strXML.Mid( nManifest ).SpanExcluding( _T(">") ) + '>';
		
		if ( CXMLElement* pManifest = CXMLElement::FromString( strManifest ) )
		{
			pXML = new CXMLElement( NULL, _T("template") );
			pXML->AddElement( pManifest );
		}
	}
	
	if ( pXML == NULL )
	{
		pXML = CXMLElement::FromString( strXML, TRUE );
		if ( pXML == NULL ) return FALSE;
	}
	
	strXML.Empty();
	
	const CXMLElement* pManifest = pXML->GetElementByName( _T("manifest") );
	
	if ( ! pXML->IsNamed( _T("template") ) || pManifest == NULL )
	{
		delete pXML;
		return FALSE;
	}
	
	CString strIcon		= pManifest->GetAttributeValue( _T("icon") );
	CString	strName		= pManifest->GetAttributeValue( _T("name"), pszName );
	CString strAuthor	= pManifest->GetAttributeValue( _T("author"), _T("Unknown") );
	CString strVersion	= pManifest->GetAttributeValue( _T("version"), _T("Unknown") );
	CString strURL		= pManifest->GetAttributeValue( _T("link") );
	CString strEmail	= pManifest->GetAttributeValue( _T("email") );
	CString strDesc		= pManifest->GetAttributeValue( _T("description") );
	CString strThumbs	= pManifest->GetAttributeValue( _T("thumbnails"), _T("false") );
	
	delete pXML;

	if ( Settings.General.LanguageRTL )
	{
		strName = _T("\x202A") + strName;
		strAuthor = _T("\x202A") + strAuthor;
	}

	if ( strIcon.GetLength() )
	{
		if ( pszPath != NULL )
			strIcon = Settings.General.Path + _T("\\Templates\\") + pszPath + strIcon;
		else
			strIcon = Settings.General.Path + _T("\\Templates\\") + strIcon;
	}
	else
	{
		if ( pszPath != NULL )
			strIcon = Settings.General.Path + _T("\\Templates\\") + pszPath + strIcon + pszName;
		else
			strIcon = Settings.General.Path + _T("\\Templates\\") + strIcon + pszName;

		strIcon = strIcon.Left( strIcon.GetLength() - 3 ) + _T("ico");
	}
	
	if ( strURL.Find( _T("http://") ) == 0 )
	{
	}
	else if ( strURL.Find( _T("www.") ) == 0 )
	{
		strURL = _T("http://") + strURL;
	}
	else
	{
		strURL.Empty();
	}
	
	if ( strEmail.Find( '@' ) < 0 ) strEmail.Empty();
	
	CLiveItem pItem( 8, 0 );
	HICON hIcon;
	
	if ( ExtractIconEx( strIcon, 0, NULL, &hIcon, 1 ) != NULL && hIcon != NULL )
	{
		pItem.SetImage( 0, AddIcon( hIcon, m_gdiImageList ) );
	}
	else
	{
		pItem.SetImage( 0, 0 );
	}
	
	pItem.Set( 0, strName );
	pItem.Set( 1, strAuthor );
	pItem.Set( 2, strVersion );
	pItem.Set( 4, strURL );
	pItem.Set( 5, strEmail );
	pItem.Set( 6, strDesc );
	pItem.Set( 7, strThumbs );
	
	strName.Format( _T("%s%s"), pszPath ? pszPath : _T(""), pszName );
	pItem.Set( 3, strName );
	
	/*int nItem =*/ pItem.Add( &m_wndList, -1, 8 );

	return TRUE;
}

BOOL CCollectionExportDlg::Step1()
{
	CWaitCursor pCursor;

	//Change explanation and button captions
	m_wndExplain.SetWindowText( m_sLblExplain2 );
	m_wndOK.SetWindowText( m_sBtnExport );
	m_wndDelete.SetWindowText( m_sBtnBack );

	//Hide the first screen controls
	m_wndList.ShowWindow( FALSE );
	m_wndAuthor.ShowWindow( FALSE );
	m_wndName.ShowWindow( FALSE );
	m_wndDesc.ShowWindow( FALSE );
	m_wndLblAuthor.ShowWindow( FALSE );
	m_wndLblName.ShowWindow( FALSE );
	m_wndLblDesc.ShowWindow( FALSE );
	m_wndGroupBox.ShowWindow( FALSE );
	if ( m_wndWizard.GetSize() ) // we already viewed the second screen
	{
		m_wndWizard.ShowWindow( SW_SHOW );
		if ( ! m_wndWizard.IsValid() )
			m_wndOK.EnableWindow( FALSE );
		return TRUE;
	}

	// Find position of wizard control
	CRect rcReference1, rcReference2, rcNew;
	m_wndList.GetWindowRect( &rcReference1 );
	ScreenToClient( &rcReference1 );
	m_wndGroupBox.GetWindowRect( &rcReference2 );
	ScreenToClient( &rcReference2 );
	rcNew.left = rcReference1.left;
	rcNew.top = rcReference1.top;
	rcNew.bottom = rcReference1.bottom;
	rcNew.right = rcReference2.right;
	int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );
	m_sXMLPath = Settings.General.Path + _T("\\Templates\\") + m_wndList.GetItemText( nItem, 3 );
	m_bThumbnails = m_wndList.GetItemText( nItem, 7 ).CompareNoCase( _T("true") ) == 0;
	if ( ! m_wndWizard )
	{
		CSingleLock pLock( &Library.m_pSection, TRUE );

		m_wndWizard.Create( WS_CHILD|WS_VISIBLE|WS_BORDER|WS_TABSTOP, rcNew, this, IDC_WIZARD, m_sXMLPath, m_pFolder );
	}
	else
		m_wndWizard.ShowWindow( TRUE );

	if ( ! m_wndWizard.IsValid() )
		m_wndOK.EnableWindow( FALSE );

	return TRUE;
}

BOOL CCollectionExportDlg::Step2()
{
	CWaitCursor pCursor;

	const CString strPath = BrowseForFolder( _T("Select folder for output:") );
	if ( strPath.IsEmpty() ) 
		// No folder selected
		return FALSE;

	CString strTitle;
	CStringA strXMLUTF8;
	{
		CSingleLock pLock( &Library.m_pSection, TRUE );

		if ( ! m_pFolder || ! m_pFolder->GetFileCount() )
			// Folder disappeared
			return FALSE;

		CAutoPtr< CXMLElement > pXML( m_pFolder->CreateXML() );
		if ( ! pXML )
			// Out of memory
			return FALSE;

		strXMLUTF8 = UTF8Encode( pXML->ToString( TRUE, TRUE ) );
		if ( strXMLUTF8.IsEmpty() )
			// Out of memory
			return FALSE;

		strTitle = m_pFolder->m_sName;
	}

	const CString strFile = strPath + _T("\\Collection.xml");

	CFile pFile;
	if ( ! pFile.Open( strFile, CFile::modeWrite | CFile::modeCreate ) )
	{
		// File creation failed
		CString strError;
		strError.Format( _T("Failed to write file: %s"), (LPCTSTR)strFile );
		theApp.Message( MSG_ERROR, _T("%s"), (LPCTSTR)strError );
		AfxMessageBox( strError, MB_OK | MB_ICONERROR );
		return FALSE;
	}

	try
	{
		pFile.Write( (LPCSTR)strXMLUTF8, strXMLUTF8.GetLength() );
		pFile.Close();
	}
	catch ( CException* pException )
	{
		// File write failed
		pFile.Abort();
		pException->Delete();
		CString strError;
		strError.Format( _T("Failed to write file: %s"), (LPCTSTR)strFile );
		theApp.Message( MSG_ERROR, _T("%s"), (LPCTSTR)strError );
		AfxMessageBox( strError, MB_OK | MB_ICONERROR );
		return FALSE;
	}

	for ( int nPosTpl = 0; nPosTpl < m_wndWizard.m_pTemplatePaths.GetSize(); ++nPosTpl )
	{
		CString strTemplateName = m_wndWizard.m_pTemplatePaths.GetAt( nPosTpl );
		if ( strTemplateName.CompareNoCase( m_wndWizard.m_sMainFilePath ) == 0 )
			strTemplateName = _T("index.htm");

		if ( ! strTemplateName.CompareNoCase( m_wndWizard.m_sEvenFilePath ) ||
			 ! strTemplateName.CompareNoCase( m_wndWizard.m_sOddFilePath ) )
			continue;

		const CString strNewFilePath = strPath + _T("\\") + strTemplateName.Left( strTemplateName.ReverseFind( '.' ) ) + _T(".htm");

		CString strSource = LoadFile( DirFromPath( m_sXMLPath ) + _T("\\") +
			( strTemplateName.CompareNoCase( _T("index.htm") ) ? strTemplateName : m_wndWizard.m_sMainFilePath ) );

		// Substitute item IDs with the values from wizard edit boxes.
		// The phrase "Individual file replacement" -- when each file has a unique
		// id substitution.
		for ( POSITION pos = m_wndWizard.m_pItems.GetStartPosition(); pos != NULL; )
		{
			CString strControlID, strMap;
			m_wndWizard.m_pItems.GetNextAssoc( pos, strControlID, strMap );

			int nPosVert = strMap.Find( _T('|') );
			const UINT nFileID = _ttoi( (LPCTSTR)strMap.Left( nPosVert ) ); // File # starting 0
			strMap = strMap.Mid( nPosVert + 1 ); // remove first entry
			nPosVert = strMap.Find( _T('|') );
			const CString strReplaceID = _T("$") + strMap.Left( nPosVert ) + _T("$"); // replacement ID from XML
			strMap = strMap.Mid( nPosVert + 1 );
			const UINT nControlID = _ttoi( (LPCTSTR)strControlID );

			const CEdit* pEdit = (CEdit*)m_wndWizard.GetDlgItem( nControlID );
			if ( ! pEdit || ! pEdit->IsKindOf( RUNTIME_CLASS( CEdit ) ) ) 
			{
				CString strError;
				strError.Format( _T("Control placed badly: %s"), (LPCTSTR)strControlID );
				theApp.Message( MSG_ERROR, _T("%s"), (LPCTSTR)strError );
				AfxMessageBox( strError, MB_OK | MB_ICONERROR );
				return FALSE;
			}

			CString strReplace;
			pEdit->GetWindowText( strReplace );

			if ( nFileID < (UINT)m_wndWizard.m_pFileDocs.GetSize() && strTemplateName.CompareNoCase( _T("index.htm") ) == 0 )
			{
				for ( int nPosDocs = 0; nPosDocs < m_wndWizard.m_pFileDocs.GetSize(); )
				{
					CString strNewReplace;

					//// ensure that the first char is not backslash
					//// it may be entered in XML
					//if ( ! strMap.IsEmpty() && strReplace.Left( 1 ) == '\\' )
					//	strReplace = strReplace.Mid( 1 );

					// Remove path when default file changed
					if ( ! strMap.IsEmpty() && strReplace.Find( _T(':') ) != -1 )
						strNewReplace = strReplace.Mid( strReplace.ReverseFind( _T('\\') ) + 1 );
					else
						strNewReplace = strReplace;
							
					// single filepicker is replaced everywhere
					// e.g. various bullets may be identical
					if ( strMap.IsEmpty() || strMap == "s" )
					{
						ReplaceNoCase( m_wndWizard.m_pFileDocs.GetAt( nPosDocs++ ), strReplaceID, strNewReplace );
					}
					else if ( strMap == "m" ) // individual file doc replacement; multi-file picker
					{
						strNewReplace.Replace( _T('\\'), _T('/') );
						ReplaceNoCase( m_wndWizard.m_pFileDocs.GetAt( nFileID ), strReplaceID, strNewReplace );
					}

					// copy selected images
					if ( ! strMap.IsEmpty() )
					{
						CString strTarget, strSourceFile;

						// if default file left, add old value to target and destination
						// since it may contain a relative path.
						if ( strReplace.Find( _T(':') ) == -1 )
						{
							strReplace.Replace( _T('/'), _T('\\') );
							strTarget = strPath + _T('\\') + strReplace;
							strSourceFile = DirFromPath( m_sXMLPath ) +	_T('\\') + strReplace;
						}
						else
						{
							strTarget = strPath + _T('\\') + strNewReplace;
							strSourceFile = strReplace;
						}
						// check if destination file does not exists
						if ( GetFileAttributes( strTarget ) == INVALID_FILE_ATTRIBUTES )
						{
							// create dirs recursively
							CreateDirectory( strTarget.Left( strTarget.ReverseFind( _T('\\') ) ) );
							if ( ! CopyFile( strSourceFile, strTarget, TRUE ) )
							{
								// File disappeared
								CString strError;
								strError.Format( _T("Failed to write file to: %s"), (LPCTSTR)strTarget );
								theApp.Message( MSG_ERROR, _T("%s"), (LPCTSTR)strError );
								AfxMessageBox( strError, MB_OK | MB_ICONERROR );
								return FALSE;
							}
						}
					}

					if ( strMap == "m" )
						break;
				} // while each even/odd file
			}
					
			// ordinary template ignores individual file replacements
			if ( ! strSource.IsEmpty() && strMap.IsEmpty() || strMap == "s" ) 
			{
				ReplaceNoCase( strSource, strReplaceID, strReplace );
			}
		} // while each wizard row
				
		// combine file docs and embed in "main" template
		if ( strTemplateName.CompareNoCase( _T("index.htm") ) == 0 )
		{
			CString strResult;
			for ( int nPosDocs2 = 0; nPosDocs2 < m_wndWizard.m_pFileDocs.GetSize(); ++nPosDocs2 )
			{
				if ( m_bThumbnails )
				{
					// Create thumbnails for every file
					CString strFilePath = m_wndWizard.m_pFilePaths.GetAt( nPosDocs2 );
					CImageFile pImageFile;
					if ( CThumbCache::Cache( strFilePath, &pImageFile ) )
					{
						CString strTarget;
						strTarget.Format( _T("%s\\thumbs\\%d.jpg"), (LPCTSTR)strPath, nPosDocs2 + 1 );
						CreateDirectory( strTarget.Left( strTarget.ReverseFind( _T('\\') ) ) );
						if ( ! pImageFile.SaveToFile( strTarget, 85 ) )
						{
							// File disappeared
							CString strError;
							strError.Format( _T("Failed to write file to: %s"), (LPCTSTR)strTarget );
							theApp.Message( MSG_ERROR, _T("%s"), (LPCTSTR)strError );
							AfxMessageBox( strError, MB_OK | MB_ICONERROR );
							return FALSE;
						}
					}
				}

				strResult += m_wndWizard.m_pFileDocs.GetAt( nPosDocs2 );
			}
			ReplaceNoCase( strSource, _T("$data$"), strResult );
			ReplaceNoCase( strSource, _T("$title$"), strTitle );
		}

		const CStringA strSourceUTF8 = UTF8Encode( strSource );

		// output to file
		CFile pNewFile;
		if ( ! pNewFile.Open( strNewFilePath , CFile::modeWrite | CFile::modeCreate ) )
		{
			// File creation failed
			CString strError;
			strError.Format( _T("Failed to write file: %s"), (LPCTSTR)strNewFilePath );
			theApp.Message( MSG_ERROR, _T("%s"), (LPCTSTR)strError );
			AfxMessageBox( strError, MB_OK | MB_ICONERROR );
			return FALSE;
		}

		try
		{
			pNewFile.Write( (LPCSTR)strSourceUTF8, strSourceUTF8.GetLength() );
			pNewFile.Close();
		}
		catch ( CException* pException )
		{
			// File write failed
			pNewFile.Abort();
			pException->Delete();
			CString strError;
			strError.Format( _T("Failed to write file: %s"), (LPCTSTR)strNewFilePath );
			theApp.Message( MSG_ERROR, _T("%s"), (LPCTSTR)strError );
			AfxMessageBox( strError, MB_OK | MB_ICONERROR );
			return FALSE;
		}
	} // while each template file
			
	// copy all non-parsed files such as images, stylesheets etc.
	for ( int nPosImg = 0; nPosImg < m_wndWizard.m_pImagePaths.GetSize(); ++nPosImg )
	{
		const CString strFileName = m_wndWizard.m_pImagePaths.GetAt( nPosImg );
		const CString strTarget = strPath + _T('\\') + strFileName;

		// destination file does not exists
		if ( GetFileAttributes( strTarget ) == INVALID_FILE_ATTRIBUTES )
		{
			CString strSource = DirFromPath( m_sXMLPath ) + _T('\\') + strFileName;
			// source file exists
			if ( GetFileAttributes( strSource ) != INVALID_FILE_ATTRIBUTES )
			{
				// create dirs recursively
				CreateDirectory( strTarget.Left( strTarget.ReverseFind( _T('\\') ) ) );
				if ( ! CopyFile( strSource, strTarget, TRUE ) )
				{
					// File disappeared
					CString strError;
					strError.Format( _T("Failed to write file to: %s"), (LPCTSTR)strTarget );
					theApp.Message( MSG_ERROR, _T("%s"), (LPCTSTR)strError );
					AfxMessageBox( strError, MB_OK | MB_ICONERROR );
					return FALSE;
				}
			}
		}
	}

	CSkinDialog::OnOK();

	return TRUE;
}

void CCollectionExportDlg::OnOK()
{
	switch ( m_nStep )
	{
	case 1:
		if ( ! Step1() )
			return;
		break;

	case 2:
		if ( ! Step2() )
			return;
		break;
	}
	m_nStep++;
}

void CCollectionExportDlg::OnTemplatesDeleteOrBack()
{
	m_wndOK.EnableWindow( TRUE ); // enable if template was invalid
	switch ( m_nStep )
	{
		case 1: // the first screen -- button "Delete"
		{
		if ( m_nSelected < 0 ) return;

			CString strName = m_wndList.GetItemText( m_nSelected, 0 );
			CString strBase = m_wndList.GetItemText( m_nSelected, 3 );

			CString strFormat, strPrompt;

			LoadString( strFormat, IDS_TEMPLATE_DELETE );
			strPrompt.Format( strFormat, (LPCTSTR)strName );

			if ( AfxMessageBox( strPrompt, MB_ICONQUESTION|MB_OKCANCEL|MB_DEFBUTTON2 ) != IDOK ) return;

			CString strPath;
			strPath.Format( _T("%s\\Templates\\%s"),
				(LPCTSTR)Settings.General.Path, (LPCTSTR)strBase );

			DeleteFileEx( strPath, FALSE, TRUE, TRUE );

			int nSlash = strPath.ReverseFind( '\\' );
			strPath = strPath.Left( nSlash ) + _T("\\*.xml");

			WIN32_FIND_DATA pFind;
			HANDLE hSearch = FindFirstFile( strPath, &pFind );

			if ( hSearch != INVALID_HANDLE_VALUE )
			{
				FindClose( hSearch );
			}
			else
			{
				strPath = strPath.Left( strPath.GetLength() - 3 ) + _T("*");
				hSearch = FindFirstFile( strPath, &pFind );

				if ( hSearch != INVALID_HANDLE_VALUE )
				{
					strPath = strPath.Left( strPath.GetLength() - 3 );

					do
					{
						if ( pFind.cFileName[0] == '.' ) continue;
						DeleteFileEx( strPath + pFind.cFileName, FALSE, TRUE, TRUE );
					}
					while ( FindNextFile( hSearch, &pFind ) );

					FindClose( hSearch );
				}

				strPath = strPath.Left( strPath.GetLength() - 1 );
				RemoveDirectory( strPath );
			}
			
			m_wndList.DeleteItem( m_nSelected );
			m_wndName.SetWindowText( _T("") );
			m_wndAuthor.SetWindowText( _T("") );
			m_wndDesc.SetWindowText( _T("") );
			m_wndDelete.EnableWindow( FALSE );

			m_nSelected = -1;
			break;
		}
		case 2: // the second screen -- button "Back"
		{
			//Change explanation and button captions
			m_wndDelete.SetWindowText( m_sBtnDelete );
			m_wndExplain.SetWindowText( m_sLblExplain1 );
			m_wndOK.SetWindowText( m_sBtnNext );

			//Show the first screen controls
			m_wndList.ShowWindow( TRUE );
			m_wndAuthor.ShowWindow( TRUE );
			m_wndName.ShowWindow( TRUE );
			m_wndDesc.ShowWindow( TRUE );
			m_wndLblAuthor.ShowWindow( TRUE );
			m_wndLblName.ShowWindow( TRUE );
			m_wndLblDesc.ShowWindow( TRUE );
			m_wndGroupBox.ShowWindow( TRUE );

			// Hide wizard control
			m_wndWizard.ShowWindow( SW_HIDE );
			
			m_nStep--;
		}
		break;
	}
}

void CCollectionExportDlg::OnItemChangedTemplates(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	*pResult = 0;
	
	int nItem = m_wndList.GetNextItem( -1, LVNI_SELECTED );
	if ( nItem == m_nSelected ) return; // selection is the same

	// Selection changed, destroy control
	m_wndWizard.DestroyWindow();

	m_nSelected = nItem;
	
	if ( nItem >= 0 )
	{
		m_wndName.SetWindowText( m_wndList.GetItemText( nItem, 0 ) );
		m_wndAuthor.SetWindowText( m_wndList.GetItemText( nItem, 1 ) );
		m_wndDesc.SetWindowText( m_wndList.GetItemText( nItem, 6 ) );
		m_wndDelete.EnableWindow( TRUE );
		m_wndOK.EnableWindow( TRUE );
	}
	else
	{
		m_wndName.SetWindowText( _T("") );
		m_wndAuthor.SetWindowText( _T("") );
		m_wndDesc.SetWindowText( _T("") );
		m_wndDelete.EnableWindow( FALSE );
		m_wndOK.EnableWindow( FALSE );
	}
}

HBRUSH CCollectionExportDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CSkinDialog::OnCtlColor( pDC, pWnd, nCtlColor );
	
	if ( m_nSelected >= 0 )
	{
		if ( pWnd == &m_wndName )
		{
			if ( m_wndList.GetItemText( m_nSelected, 4 ).GetLength() )
			{
				pDC->SetTextColor( CoolInterface.m_crTextLink );
				pDC->SelectObject( &theApp.m_gdiFontLine );
			}
		}
		else if ( pWnd == &m_wndAuthor )
		{
			if ( m_wndList.GetItemText( m_nSelected, 5 ).GetLength() )
			{
				pDC->SetTextColor( CoolInterface.m_crTextLink );
				pDC->SelectObject( &theApp.m_gdiFontLine );
			}
		}
	}

	return hbr;
}

BOOL CCollectionExportDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if ( m_nSelected >= 0 && m_nStep == 1 )
	{
		CPoint point;
		CRect rc;
		
		GetCursorPos( &point );
		m_wndName.GetWindowRect( &rc );
		
		if ( rc.PtInRect( point ) )
		{
			if ( m_wndList.GetItemText( m_nSelected, 4 ).GetLength() )
			{
				SetCursor( theApp.LoadCursor( IDC_HAND ) );
				return TRUE;
			}
		}

		m_wndAuthor.GetWindowRect( &rc );

		if ( rc.PtInRect( point ) )
		{
			if ( m_wndList.GetItemText( m_nSelected, 5 ).GetLength() )
			{
				SetCursor( theApp.LoadCursor( IDC_HAND ) );
				return TRUE;
			}
		}
	}

	return CSkinDialog::OnSetCursor( pWnd, nHitTest, message );
}

void CCollectionExportDlg::OnLButtonUp(UINT /*nFlags*/, CPoint point) 
{
	CRect rc;

	if ( m_nSelected < 0 ) return;

	ClientToScreen( &point );
	m_wndName.GetWindowRect( &rc );
	
	if ( rc.PtInRect( point ) )
	{
		CString strURL = m_wndList.GetItemText( m_nSelected, 4 );

		if ( strURL.GetLength() )
		{
			ShellExecute( GetSafeHwnd(), _T("open"), strURL,
				NULL, NULL, SW_SHOWNORMAL );
		}
		return;
	}

	m_wndAuthor.GetWindowRect( &rc );
	
	if ( rc.PtInRect( point ) )
	{
		CString strEmail = m_wndList.GetItemText( m_nSelected, 5 );

		if ( strEmail.GetLength() )
		{
			ShellExecute( GetSafeHwnd(), _T("open"), _T("mailto:") + strEmail,
				NULL, NULL, SW_SHOWNORMAL );
		}
		return;
	}
}

BOOL CCollectionExportDlg::PreTranslateMessage(MSG* pMsg) 
{
	if ( pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB )
	{
		if ( m_wndWizard )
			if ( m_wndWizard.IsWindowVisible() ) 
				if ( m_wndWizard.OnTab() ) return TRUE; // TODO: when template is invalid tab key does not work.
	}

	return CSkinDialog::PreTranslateMessage( pMsg );
}

CString CCollectionExportDlg::DirFromPath(LPCTSTR szPath)
{
	CString strDir(szPath);
	int nIndex( strDir.ReverseFind( '\\' ) );

	if ( nIndex != -1 ) strDir = strDir.Left( nIndex );
	return strDir;
}
