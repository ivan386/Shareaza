//
// Skin.cpp
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
#include "CoolInterface.h"
#include "CoolMenu.h"
#include "CtrlCoolBar.h"
#include "Skin.h"
#include "SkinWindow.h"
#include "ImageServices.h"
#include "ImageFile.h"
#include "Plugins.h"
#include "XML.h"
#include "WndChild.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CSkin Skin;


//////////////////////////////////////////////////////////////////////
// CSkin construction

CSkin::CSkin()
{
}

CSkin::~CSkin()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CSkin apply

void CSkin::Apply()
{
	CreateDefault();
	ApplyRecursive( NULL );
	Finalise();
}

//////////////////////////////////////////////////////////////////////
// CSkin clear

void CSkin::Clear()
{
	CString strName;
	POSITION pos;
	
	for ( pos = m_pMenus.GetStartPosition() ; pos ; )
	{
		CMenu* pMenu;
		m_pMenus.GetNextAssoc( pos, strName, (void*&)pMenu );
		delete pMenu;
	}
	
	for ( pos = m_pToolbars.GetStartPosition() ; pos ; )
	{
		CCoolBarCtrl* pBar;
		m_pToolbars.GetNextAssoc( pos, strName, (void*&)pBar );
		delete pBar;
	}
	
	for ( pos = m_pDialogs.GetStartPosition() ; pos ; )
	{
		CXMLElement* pXML;
		m_pDialogs.GetNextAssoc( pos, strName, (void*&)pXML );
		delete pXML;
	}
	
	for ( pos = m_pDocuments.GetStartPosition() ; pos ; )
	{
		CXMLElement* pXML;
		m_pDocuments.GetNextAssoc( pos, strName, (void*&)pXML );
		delete pXML;
	}
	
	for ( pos = m_pSkins.GetHeadPosition() ; pos ; )
	{
		delete (CSkinWindow*)m_pSkins.GetNext( pos );
	}
	
	for ( pos = m_pFontPaths.GetHeadPosition() ; pos ; )
	{
		RemoveFontResource( m_pFontPaths.GetNext( pos ) );
	}
	
	m_pStrings.RemoveAll();
	m_pMenus.RemoveAll();
	m_pToolbars.RemoveAll();
	m_pDocuments.RemoveAll();
	m_pWatermarks.RemoveAll();
	m_pLists.RemoveAll();
	m_pDialogs.RemoveAll();
	m_pSkins.RemoveAll();
	m_pFontPaths.RemoveAll();
	
	if ( m_brDialog.m_hObject != NULL ) m_brDialog.DeleteObject();
	if ( m_bmPanelMark.m_hObject != NULL ) m_bmPanelMark.DeleteObject();
}

//////////////////////////////////////////////////////////////////////
// CSkin caption selector

BOOL CSkin::SelectCaption(CWnd* pWnd, int nIndex)
{
	CString strCaption;
	pWnd->GetWindowText( strCaption );
	
	if ( SelectCaption( strCaption, nIndex ) )
	{
		pWnd->SetWindowText( strCaption );
		return TRUE;
	}
	
	return FALSE;
}

BOOL CSkin::SelectCaption(CString& strCaption, int nIndex)
{
	for ( strCaption += '|' ; ; nIndex-- )
	{
		CString strSection = strCaption.SpanExcluding( _T("|") );
		strCaption = strCaption.Mid( strSection.GetLength() + 1 );
		if ( strSection.IsEmpty() ) break;

		if ( nIndex <= 0 )
		{
			strCaption = strSection;
			return TRUE;
		}
	}
	
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CSkin recursive folder applicator

void CSkin::ApplyRecursive(LPCTSTR pszPath)
{
	WIN32_FIND_DATA pFind;
	CString strPath;
	HANDLE hSearch;

	strPath.Format( _T("%s\\Skins\\%s*.*"), (LPCTSTR)Settings.General.Path,
		pszPath ? pszPath : _T("") );

	hSearch = FindFirstFile( strPath, &pFind );

	if ( hSearch != INVALID_HANDLE_VALUE )
	{
		do
		{
			if ( pFind.cFileName[0] == '.' ) continue;

			if ( pFind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				strPath.Format( _T("%s%s\\"),
					pszPath ? pszPath : _T(""), pFind.cFileName );

				ApplyRecursive( strPath );
			}
			else if (	_tcsistr( pFind.cFileName, _T(".xml") ) != NULL &&
						_tcsicmp( pFind.cFileName, _T("Definitions.xml") ) )
			{
				strPath.Format( _T("%s%s"),
					pszPath ? pszPath : _T(""), pFind.cFileName );

				if ( theApp.GetProfileInt( _T("Skins"), strPath, FALSE ) )
				{
					LoadFromFile( Settings.General.Path + _T("\\Skins\\") + strPath );
				}
			}
		}
		while ( FindNextFile( hSearch, &pFind ) );

		FindClose( hSearch );
	}
}

//////////////////////////////////////////////////////////////////////
// CSkin root load

BOOL CSkin::LoadFromFile(LPCTSTR pszFile)
{
	CXMLElement* pXML = CXMLElement::FromFile( pszFile );
	if ( pXML == NULL ) return FALSE;
	
	CString strPath = pszFile;
	int nSlash = strPath.ReverseFind( '\\' );
	if ( nSlash >= 0 ) strPath = strPath.Left( nSlash + 1 );
	
	BOOL bResult = LoadFromXML( pXML, strPath );
	
	delete pXML;
	
	return bResult;
}

BOOL CSkin::LoadFromResource(HINSTANCE hInstance, UINT nResourceID)
{
	HMODULE hModule = hInstance != NULL ? (HMODULE)hInstance : GetModuleHandle( NULL );
	HRSRC hRes = FindResource( hModule, MAKEINTRESOURCE( nResourceID ), MAKEINTRESOURCE( 23 ) );
	
	if ( hRes == NULL ) return FALSE;
	
	CString strBody;
	
	DWORD nSize			= SizeofResource( hModule, hRes );
	HGLOBAL hMemory		= ::LoadResource( hModule, hRes );
	LPTSTR pszOutput	= strBody.GetBuffer( nSize + 1 );
	LPCSTR pszInput		= (LPCSTR)LockResource( hMemory );
	
	while ( nSize-- ) *pszOutput++ = *pszInput++;
	*pszOutput++ = 0;
	
	strBody.ReleaseBuffer();

	CString strPath;
	strPath.Format( _T("%lu$"), (DWORD)hModule );
	
	return LoadFromString( strBody, strPath );
}

BOOL CSkin::LoadFromString(const CString& strXML, const CString& strPath)
{
	CXMLElement* pXML = CXMLElement::FromString( strXML, TRUE );
	if ( pXML == NULL ) return FALSE;

	BOOL bSuccess = LoadFromXML( pXML, strPath );

	delete pXML;
	return bSuccess;
}

BOOL CSkin::LoadFromXML(CXMLElement* pXML, const CString& strPath)
{
	if ( ! pXML->IsNamed( _T("skin") ) ) return FALSE;
	
	BOOL bSuccess = FALSE;
	
	for ( POSITION pos = pXML->GetElementIterator() ; pos ; )
	{
		CXMLElement* pSub = pXML->GetNextElement( pos );
		bSuccess = FALSE;
		
		if ( pSub->IsNamed( _T("commandImages") ) )
		{
			if ( ! LoadCommandImages( pSub, strPath ) ) break;
		}
		else if ( pSub->IsNamed( _T("watermarks" ) ) )
		{
			if ( ! LoadWatermarks( pSub, strPath ) ) break;
		}
		else if ( pSub->IsNamed( _T("windowSkins" ) ) )
		{
			if ( ! LoadWindowSkins( pSub, strPath ) ) break;
		}
		else if ( pSub->IsNamed( _T("menus") ) )
		{
			if ( ! LoadMenus( pSub ) ) break;
		}
		else if ( pSub->IsNamed( _T("toolbars") ) )
		{
			if ( ! LoadToolbars( pSub ) ) break;
		}
		else if ( pSub->IsNamed( _T("dialogs") ) )
		{
			if ( ! LoadDialogs( pSub ) ) break;
		}
		else if ( pSub->IsNamed( _T("strings") ) || pSub->IsNamed( _T("commandTips") ) )
		{
			if ( ! LoadStrings( pSub ) ) break;
		}
		else if ( pSub->IsNamed( _T("fonts" ) ) )
		{
			if ( ! LoadFonts( pSub, strPath ) ) break;
		}
		else if ( pSub->IsNamed( _T("colourScheme") ) )
		{
			if ( ! LoadColourScheme( pSub ) ) break;
		}
		else if ( pSub->IsNamed( _T("listColumns") ) )
		{
			if ( ! LoadListColumns( pSub ) ) break;
		}
		else if ( pSub->IsNamed( _T("commandMap") ) )
		{
			if ( ! LoadCommandMap( pSub ) ) break;
		}
		else if ( pSub->IsNamed( _T("documents" ) ) )
		{
			if ( ! LoadDocuments( pSub ) ) break;
		}
		else if ( pSub->IsNamed( _T("manifest") ) )
		{
			CString strType = pSub->GetAttributeValue( _T("type") );
			strType.MakeLower();
			
			if ( strType == _T("language") )
			{
				Settings.General.Language = pSub->GetAttributeValue( _T("language"), _T("en") );
			}
		}
		
		bSuccess = TRUE;
	}
	
	return bSuccess;
}

//////////////////////////////////////////////////////////////////////
// CSkin strings

BOOL CSkin::LoadString(CString& str, UINT nStringID)
{
	if ( m_pStrings.Lookup( nStringID, str ) ) return TRUE;
	if ( str.LoadString( nStringID ) ) return TRUE;
	str.Empty();
	return FALSE;
}

BOOL CSkin::LoadStrings(CXMLElement* pBase)
{
	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );
		
		if ( pXML->IsNamed( _T("string") ) )
		{
			if ( UINT nID = LookupCommandID( pXML ) )
			{
				CString strValue = pXML->GetAttributeValue( _T("value") );
				
				while ( TRUE )
				{
					int nPos = strValue.Find( _T("\\n") );
					if ( nPos < 0 ) break;
					strValue = strValue.Left( nPos ) + _T("\n") + strValue.Mid( nPos + 2 );
				}
				
				// Hack for I64 compliance
				
				if ( nID == IDS_DOWNLOAD_FRAGMENT_REQUEST || nID == IDS_DOWNLOAD_USEFUL_RANGE ||
					 nID == IDS_UPLOAD_CONTENT || nID == IDS_UPLOAD_PARTIAL_CONTENT ||
					 nID == IDS_DOWNLOAD_VERIFY_DROP )
				{
					Replace( strValue, _T("%lu"), _T("%I64i") );
				}
				
				m_pStrings.SetAt( nID, strValue );
			}
		}
		else if ( pXML->IsNamed( _T("tip") ) )
		{
			if ( UINT nID = LookupCommandID( pXML ) )
			{
				CString strMessage = pXML->GetAttributeValue( _T("message") );
				CString strTip = pXML->GetAttributeValue( _T("tip") );
				if ( strTip.GetLength() ) strMessage += '\n' + strTip;
				m_pStrings.SetAt( nID, strMessage );
			}
		}
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSkin menus

CMenu* CSkin::GetMenu(LPCTSTR pszName)
{
	LPCTSTR* pszModeSuffix = m_pszModeSuffix[ Settings.General.GUIMode ];
	CString strName( pszName );
	CMenu* pMenu = NULL;
	
	for ( int nModeTry = 0 ; pszModeSuffix[ nModeTry ] ; nModeTry++ )
	{
		if ( m_pMenus.Lookup( strName + pszModeSuffix[ nModeTry ], (void*&)pMenu ) )
			return pMenu;
		
		for ( UINT nItem = 0 ; nItem < m_mnuDefault.GetMenuItemCount() ; nItem++ )
		{
			CString strItem;
			
			m_mnuDefault.GetMenuString( nItem, strItem, MF_BYPOSITION );
			
			if ( strItem.CompareNoCase( strName + pszModeSuffix[ nModeTry ] ) == 0 )
			{
				return m_mnuDefault.GetSubMenu( nItem );
			}
		}
	}
	
	return NULL;
}

BOOL CSkin::LoadMenus(CXMLElement* pBase)
{
	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );
		if ( pXML->IsNamed( _T("menu") ) && ! LoadMenu( pXML ) ) return FALSE;
	}
	
	return TRUE;
}

BOOL CSkin::LoadMenu(CXMLElement* pXML)
{
	CString strName = pXML->GetAttributeValue( _T("name") );
	if ( strName.IsEmpty() ) return FALSE;
	
	CMenu* pMenu = NULL;
	
	if ( m_pMenus.Lookup( strName, (void*&)pMenu ) )
	{
		delete pMenu;
		m_pMenus.RemoveKey( strName );
	}
	
	pMenu = new CMenu();
	
	if ( pXML->GetAttributeValue( _T("type"), _T("popup") ).CompareNoCase( _T("bar") ) == 0 )
	{
		pMenu->CreateMenu();
	}
	else
	{
		pMenu->CreatePopupMenu();
	}
	
	if ( CreateMenu( pXML, pMenu->GetSafeHmenu() ) )
	{
		m_pMenus.SetAt( strName, pMenu );
		return TRUE;
	}
	else
	{
		delete pMenu;
		return FALSE;
	}
}

BOOL CSkin::CreateMenu(CXMLElement* pRoot, HMENU hMenu)
{
	for ( POSITION pos = pRoot->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML	= pRoot->GetNextElement( pos );
		CString strText		= pXML->GetAttributeValue( _T("text") );
		
		int nAmp = strText.Find( '_' );
		if ( nAmp >= 0 ) strText.SetAt( nAmp, '&' );
		
		if ( pXML->IsNamed( _T("item") ) )
		{
			if ( UINT nID = LookupCommandID( pXML ) )
			{
				CString strKeys = pXML->GetAttributeValue( _T("shortcut") );
				
				if ( strKeys.GetLength() ) strText += '\t' + strKeys;
				
				AppendMenu( hMenu, MF_STRING, nID, strText );
			}
		}
		else if ( pXML->IsNamed( _T("menu") ) )
		{
			HMENU hSubMenu = CreatePopupMenu();
			
			if ( ! CreateMenu( pXML, hSubMenu ) )
			{
				DestroyMenu( hSubMenu );
				return FALSE;
			}
			
			AppendMenu( hMenu, MF_STRING|MF_POPUP, (UINT)hSubMenu, strText );
		}
		else if ( pXML->IsNamed( _T("separator") ) )
		{
			AppendMenu( hMenu, MF_SEPARATOR, ID_SEPARATOR, NULL );
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSkin toolbars

BOOL CSkin::CreateToolBar(LPCTSTR pszName, CCoolBarCtrl* pBar)
{
	if ( pszName == NULL ) return FALSE;
	
	for ( CWnd* pChild = pBar->GetWindow( GW_CHILD ) ; pChild ; pChild = pChild->GetNextWindow() )
	{
		pChild->ShowWindow( SW_HIDE );
	}
	
	CString strPath = pszName;
	strPath += _T(".Toolbar");
	
	if ( m_pWatermarks.Lookup( strPath, strPath ) )
	{
		HBITMAP hBitmap = LoadBitmap( strPath );
		pBar->SetWatermark( hBitmap );
	}
	else
	{
		pBar->SetWatermark( NULL );
	}
	
	pBar->Clear();
	
	LPCTSTR* pszModeSuffix = m_pszModeSuffix[ Settings.General.GUIMode ];
	CCoolBarCtrl* pBase = NULL;
	CString strName( pszName );
	
	for ( int nModeTry = 0 ; pszModeSuffix[ nModeTry ] ; nModeTry++ )
	{
		if ( m_pToolbars.Lookup( strName + pszModeSuffix[ nModeTry ], (void*&)pBase ) ) break;
	}
	
	if ( pBase != NULL )
	{
		pBar->Copy( pBase );
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CSkin::LoadToolbars(CXMLElement* pBase)
{
	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );
		if ( pXML->IsNamed( _T("toolbar") ) && ! CreateToolBar( pXML ) ) return FALSE;
	}

	return TRUE;
}

BOOL CSkin::CreateToolBar(CXMLElement* pBase)
{
	CCoolBarCtrl* pBar = new CCoolBarCtrl();
	
	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );
		
		if ( pXML->IsNamed( _T("button") ) )
		{
			if ( UINT nID = LookupCommandID( pXML ) )
			{
				CCoolBarItem* pItem = pBar->Add( nID, pXML->GetAttributeValue( _T("text") ) );
				CString strTemp = pXML->GetAttributeValue( _T("colour") );
				
				if ( strTemp.GetLength() == 6 )
				{
					int nRed, nGreen, nBlue;
					_stscanf( strTemp.Mid( 0, 2 ), _T("%x"), &nRed );
					_stscanf( strTemp.Mid( 2, 2 ), _T("%x"), &nGreen );
					_stscanf( strTemp.Mid( 4, 2 ), _T("%x"), &nBlue );
					pItem->m_crText = RGB( nRed, nGreen, nBlue );
				}
				
				strTemp = pXML->GetAttributeValue( _T("tip") );
				if ( strTemp.GetLength() ) pItem->SetTip( strTemp );
				
				strTemp = pXML->GetAttributeValue( _T("visible") );
				if ( strTemp.GetLength() ) pItem->Show( FALSE );
			}
		}
		else if ( pXML->IsNamed( _T("separator") ) )
		{
			pBar->Add( ID_SEPARATOR );
		}
		else if ( pXML->IsNamed( _T("rightalign") ) )
		{
			pBar->Add( ID_RIGHTALIGN );
		}
		else if ( pXML->IsNamed( _T("control") ) )
		{
			UINT nID, nWidth, nHeight = 0;
			CString strTemp;
			
			strTemp = pXML->GetAttributeValue( _T("id") );
			if ( _stscanf( strTemp, _T("%lu"), &nID ) == 1 )
			{
				strTemp = pXML->GetAttributeValue( _T("width") );
				
				if ( _stscanf( strTemp, _T("%lu"), &nWidth ) == 1 )
				{
					strTemp = pXML->GetAttributeValue( _T("height") );
					_stscanf( strTemp, _T("%lu"), &nHeight );
					pBar->Add( nID, nWidth, nHeight );
				}
			}
		}
		else if ( pXML->IsNamed( _T("label") ) )
		{
			CCoolBarItem* pItem = pBar->Add( 1, pXML->GetAttributeValue( _T("text") ) );
			pItem->m_crText = 0;
			pItem->SetTip( pXML->GetAttributeValue( _T("tip") ) );
		}
	}
	
	CString strName = pBase->GetAttributeValue( _T("name") );
	
	CCoolBarCtrl* pOld = NULL;
	if ( m_pToolbars.Lookup( strName, (void*&)pOld ) && pOld ) delete pOld;
	
	m_pToolbars.SetAt( strName, pBar );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSkin documents

BOOL CSkin::LoadDocuments(CXMLElement* pBase)
{
	for ( POSITION posDoc = pBase->GetElementIterator() ; posDoc ; )
	{
		CXMLElement* pDoc = pBase->GetNextElement( posDoc );
		if ( ! pDoc->IsNamed( _T("document") ) ) continue;
		
		CString strName = pDoc->GetAttributeValue( _T("name") );
		
		CXMLElement* pOld = NULL;
		if ( m_pDocuments.Lookup( strName, (void*&)pOld ) ) delete pOld;
		
		m_pDocuments.SetAt( strName, pDoc->Detach() );
	}
	
	return TRUE;
}

CXMLElement* CSkin::GetDocument(LPCTSTR pszName)
{
	CXMLElement* pXML = NULL;

	if ( m_pDocuments.Lookup( pszName, (void*&)pXML ) ) return pXML;
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CSkin watermarks

HBITMAP CSkin::GetWatermark(LPCTSTR pszName)
{
	CString strPath;
	if ( m_pWatermarks.Lookup( pszName, strPath ) ) return LoadBitmap( strPath );
	return NULL;
}

BOOL CSkin::GetWatermark(CBitmap* pBitmap, LPCTSTR pszName)
{
	ASSERT( pBitmap != NULL );
	if ( pBitmap->m_hObject != NULL ) pBitmap->DeleteObject();
	HBITMAP hBitmap = GetWatermark( pszName );
	if ( hBitmap != NULL ) pBitmap->Attach( hBitmap );
	return ( hBitmap != NULL );
}

BOOL CSkin::LoadWatermarks(CXMLElement* pSub, const CString& strPath)
{
	for ( POSITION posMark = pSub->GetElementIterator() ; posMark ; )
	{
		CXMLElement* pMark = pSub->GetNextElement( posMark );
	
		if ( pMark->IsNamed( _T("watermark") ) )
		{
			CString strName	= pMark->GetAttributeValue( _T("target") );
			CString strFile	= pMark->GetAttributeValue( _T("path") );
			
			if ( strName.GetLength() && strFile.GetLength() )
			{
				strFile = strPath + strFile;
				m_pWatermarks.SetAt( strName, strFile );
			}
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSkin list column translations

BOOL CSkin::Translate(LPCTSTR pszName, CHeaderCtrl* pCtrl)
{
	CString strEdit;

	if ( ! m_pLists.Lookup( pszName, strEdit ) ) return FALSE;

	TCHAR szColumn[128];
	HD_ITEM pColumn;
	
	pColumn.mask		= HDI_TEXT;
	pColumn.pszText		= szColumn;
	pColumn.cchTextMax	= 126;

	for ( int nItem = 0 ; nItem < pCtrl->GetItemCount() ; nItem++ )
	{
		pCtrl->GetItem( nItem, &pColumn );

		_tcscat( szColumn, _T("=") );

		LPCTSTR pszFind = _tcsistr( strEdit, szColumn );

		if ( pszFind )
		{
			pszFind += _tcslen( szColumn );

			CString strNew = pszFind;
			strNew = strNew.SpanExcluding( _T("|") );

			_tcscpy( szColumn, strNew );
			pCtrl->SetItem( nItem, &pColumn );
		}
	}

	return TRUE;
}

BOOL CSkin::LoadListColumns(CXMLElement* pBase)
{
	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );
		if ( ! pXML->IsNamed( _T("list") ) ) continue;

		CString strName = pXML->GetAttributeValue( _T("name") );
		if ( strName.IsEmpty() ) continue;

		CString strEdit;

		for ( POSITION posCol = pXML->GetElementIterator() ; posCol ; )
		{
			CXMLElement* pCol = pXML->GetNextElement( posCol );
			if ( ! pCol->IsNamed( _T("column") ) ) continue;
			
			CString strFrom	= pCol->GetAttributeValue( _T("from") );
			CString strTo	= pCol->GetAttributeValue( _T("to") );

			if ( strFrom.IsEmpty() || strTo.IsEmpty() ) continue;
			
			if ( strEdit.GetLength() ) strEdit += '|';
			strEdit += strFrom;
			strEdit += '=';
			strEdit += strTo;
		}

		m_pLists.SetAt( strName, strEdit );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSkin dialogs

BOOL CSkin::Apply(LPCTSTR pszName, CDialog* pDialog, UINT nIconID)
{
	if ( nIconID )
	{
		HICON hIcon = CoolInterface.ExtractIcon( nIconID );
		
		if ( hIcon == NULL )
		{
			hIcon = (HICON)LoadImage( AfxGetInstanceHandle(),
				MAKEINTRESOURCE( nIconID ), IMAGE_ICON, 16, 16, 0 );
		}
		
		if ( hIcon != NULL ) pDialog->SetIcon( hIcon, FALSE );
	}
	
	CString strName;

	if ( pszName == NULL )
		strName = pDialog->GetRuntimeClass()->m_lpszClassName;
	else
		strName = pszName;
	
	CWnd* pWnd = pDialog->GetWindow( GW_CHILD );
	CString strCaption;
	
	for ( int nCount = 0 ; pWnd ; nCount++, pWnd = pWnd->GetNextWindow() )
	{
		TCHAR szClass[3] = { 0, 0, 0 };
		GetClassName( pWnd->GetSafeHwnd(), szClass, 3 );
		strCaption += szClass;
	}
	
	if ( theApp.GetProfileInt( _T(""), _T("DialogScan"), FALSE ) )
	{
		//USES_CONVERSION;
		//LPCSTR pszOutput;
		CFile pFile;
		
		if ( pFile.Open( _T("\\Dialog.xml"), CFile::modeReadWrite ) )
		{
			pFile.Seek( 0, CFile::end );
		}
		else if ( ! pFile.Open( _T("\\Dialog.xml"), CFile::modeWrite|CFile::modeCreate ) )
		{
			return FALSE;
		}

		#ifdef _UNICODE
		pFile.Write( "<dialog name=\"", 14 );

		int nBytes = WideCharToMultiByte( CP_ACP, 0, strName, strName.GetLength(), NULL, 0, NULL, NULL );
		LPSTR pBytes = new CHAR[nBytes];
		WideCharToMultiByte( CP_ACP, 0, strName, strName.GetLength(), pBytes, nBytes, NULL, NULL );
		pFile.Write( pBytes, nBytes );
		delete [] pBytes;

		pFile.Write( "\" cookie=\"", 10 );

		nBytes = WideCharToMultiByte( CP_ACP, 0, strCaption, strCaption.GetLength(), NULL, 0, NULL, NULL );
		pBytes = new CHAR[nBytes];
		WideCharToMultiByte( CP_ACP, 0, strCaption, strCaption.GetLength(), pBytes, nBytes, NULL, NULL );
		pFile.Write( pBytes, nBytes );
		delete [] pBytes;

		pFile.Write( "\" caption=\"", 11 );
		pDialog->GetWindowText( strCaption );

		nBytes = WideCharToMultiByte( CP_ACP, 0, strCaption, strCaption.GetLength(), NULL, 0, NULL, NULL );
		pBytes = new CHAR[nBytes];
		WideCharToMultiByte( CP_ACP, 0, strCaption, strCaption.GetLength(), pBytes, nBytes, NULL, NULL );
		pFile.Write( pBytes, nBytes );
		delete [] pBytes;

		pFile.Write( "\">\r\n", 4 );

		#else
		pFile.Write( "<dialog name=\"", 14 );
		pFile.Write( strName, strlen(strName) );
		pFile.Write( "\" cookie=\"", 10 );
		pFile.Write( strCaption, strlen(strCaption) );
		pFile.Write( "\" caption=\"", 11 );
		pDialog->GetWindowText( strCaption );
		pFile.Write( strCaption, strlen(strCaption) );
		pFile.Write( "\">\r\n", 4 );
		#endif
		

		/*
		pFile.Write( "<dialog name=\"", 14 );
		pszOutput = T2A(strName);
		pFile.Write( pszOutput, strlen(pszOutput) );
		pFile.Write( "\" cookie=\"", 10 );
		pszOutput = T2A(strCaption);
		pFile.Write( pszOutput, strlen(pszOutput) );
		pFile.Write( "\" caption=\"", 11 );
		pDialog->GetWindowText( strCaption );
		pszOutput = T2A(strCaption);
		pFile.Write( pszOutput, strlen(pszOutput) );
		pFile.Write( "\">\r\n", 4 );
		*/
		
		for ( pWnd = pDialog->GetWindow( GW_CHILD ) ; pWnd ; pWnd = pWnd->GetNextWindow() )
		{
			TCHAR szClass[64];
			
			GetClassName( pWnd->GetSafeHwnd(), szClass, 64 );
			strCaption.Empty();
			
			if ( _tcsistr( szClass, _T("Static") ) ||
				 _tcsistr( szClass, _T("Button") ) )
			{
				pWnd->GetWindowText( strCaption );
			}
			
			if ( strCaption.GetLength() )
			{
				Replace( strCaption, _T("&"), _T("_") );
				Replace( strCaption, _T("\""), _T("&quot;") );
				pFile.Write( "\t<control caption=\"", 19 );

				//pszOutput = T2A(strCaption);
				//pFile.Write( pszOutput, strlen(pszOutput) );
				#ifdef _UNICODE
				int nBytes = WideCharToMultiByte( CP_ACP, 0, strCaption, strCaption.GetLength(), NULL, 0, NULL, NULL );
				LPSTR pBytes = new CHAR[nBytes];
				WideCharToMultiByte( CP_ACP, 0, strCaption, strCaption.GetLength(), pBytes, nBytes, NULL, NULL );
				pFile.Write( pBytes, nBytes );
				delete [] pBytes;
				#else
				pFile.Write( strCaption, strlen(strCaption) );
				#endif

        		pFile.Write( "\"/>\r\n", 5 );
			}
			else
			{
				pFile.Write( "\t<control/>\r\n", 10+3 );
			}
		}
		
		pFile.Write( "</dialog>\r\n\r\n", 13 );
		pFile.Close();
		
		return TRUE;
	}
	
	CXMLElement* pBase = NULL;
	if ( ! m_pDialogs.Lookup( strName, (void*&)pBase ) ) return FALSE;
	
	if ( strCaption != pBase->GetAttributeValue( _T("cookie") ) ) return FALSE;
	
	strCaption = pBase->GetAttributeValue( _T("caption") );
	if ( strCaption.GetLength() ) pDialog->SetWindowText( strCaption );
	
	pWnd = pDialog->GetWindow( GW_CHILD );
	
	for ( POSITION pos = pBase->GetElementIterator() ; pos && pWnd ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );
		
		if ( pXML->IsNamed( _T("control") ) )
		{
			strCaption = pXML->GetAttributeValue( _T("caption") );
			Replace( strCaption, _T("{n}"), _T("\r\n") );
			
			if ( strCaption.GetLength() )
			{
				int nPos = strCaption.Find( '_' );
				if ( nPos >= 0 ) strCaption.SetAt( nPos, '&' );
				pWnd->SetWindowText( strCaption );
			}
			
			pWnd = pWnd->GetNextWindow();
		}
	}
	
	return TRUE;
}

CString CSkin::GetDialogCaption(LPCTSTR pszName)
{
	CXMLElement* pBase = NULL;
	CString strCaption;
	
	if ( m_pDialogs.Lookup( pszName, (void*&)pBase ) )
	{
		strCaption = pBase->GetAttributeValue( _T("caption") );
	}
	
	return strCaption;
}

BOOL CSkin::LoadDialogs(CXMLElement* pBase)
{
	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );

		if ( pXML->IsNamed( _T("dialog") ) )
		{
			CString strName = pXML->GetAttributeValue( _T("name") );
			CXMLElement* pOld;

			if ( m_pDialogs.Lookup( strName, (void*&)pOld ) ) delete pOld;

			pXML->Detach();
			m_pDialogs.SetAt( strName, pXML );
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSkin window skins

CSkinWindow* CSkin::GetWindowSkin(LPCTSTR pszWindow, LPCTSTR pszAppend)
{
	CString strWindow;
	strWindow.Format( _T("|%s%s|"), pszWindow, pszAppend ? pszAppend : _T("") );
	
	for ( POSITION pos = m_pSkins.GetHeadPosition() ; pos ; )
	{
		CSkinWindow* pSkin = (CSkinWindow*)m_pSkins.GetNext( pos );
		if ( pSkin->m_sTargets.Find( strWindow ) >= 0 ) return pSkin;
	}
	
	return NULL;
}

CSkinWindow* CSkin::GetWindowSkin(CWnd* pWnd)
{
	LPCTSTR* pszModeSuffix = m_pszModeSuffix[ Settings.General.GUIMode ];
	BOOL bPanel = FALSE;
	
	if ( pWnd->IsKindOf( RUNTIME_CLASS(CChildWnd) ) )
	{
		CChildWnd* pChild = (CChildWnd*)pWnd;
		bPanel = pChild->m_bPanelMode;
	}
	
	for ( CRuntimeClass* pClass = pWnd->GetRuntimeClass() ; pClass ; pClass = pClass->m_pBaseClass )
	{
		if ( bPanel )
		{
			CSkinWindow* pSkin = GetWindowSkin( CString( pClass->m_lpszClassName ), _T(".Panel") );
			if ( pSkin != NULL ) return pSkin;
		}
		
		for ( int nSuffix = 0 ; pszModeSuffix[ nSuffix ] != NULL ; nSuffix ++ )
		{
			if ( pszModeSuffix[ nSuffix ][0] != 0 || ! bPanel )
			{
				CSkinWindow* pSkin = GetWindowSkin(
					CString( pClass->m_lpszClassName ), pszModeSuffix[ nSuffix ] );
				if ( pSkin != NULL ) return pSkin;
			}
		}
	}
	
	return NULL;
}

BOOL CSkin::LoadWindowSkins(CXMLElement* pSub, const CString& strPath)
{
	for ( POSITION posSkin = pSub->GetElementIterator() ; posSkin ; )
	{
		CXMLElement* pSkinElement = pSub->GetNextElement( posSkin );
		
		if ( pSkinElement->IsNamed( _T("windowSkin") ) )
		{
			CSkinWindow* pSkin = new CSkinWindow();
			
			if ( pSkin->Parse( pSkinElement, strPath ) )
			{
				m_pSkins.AddHead( pSkin );
			}
			else
			{
				delete pSkin;
			}
		}
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSkin colour scheme

BOOL CSkin::LoadColourScheme(CXMLElement* pBase)
{
	CMapStringToPtr pColours;

	pColours.SetAt( _T("system.base.window"), &CoolInterface.m_crWindow );
	pColours.SetAt( _T("system.base.midtone"), &CoolInterface.m_crMidtone );
	pColours.SetAt( _T("system.base.highlight"), &CoolInterface.m_crHighlight );
	pColours.SetAt( _T("system.base.text"), &CoolInterface.m_crText );
	pColours.SetAt( _T("system.base.hitext"), &CoolInterface.m_crHiText );

	pColours.SetAt( _T("system.back.normal"), &CoolInterface.m_crBackNormal );
	pColours.SetAt( _T("system.back.selected"), &CoolInterface.m_crBackSel );
	pColours.SetAt( _T("system.back.checked"), &CoolInterface.m_crBackCheck );
	pColours.SetAt( _T("system.back.checked.selected"), &CoolInterface.m_crBackCheckSel );
	pColours.SetAt( _T("system.margin"), &CoolInterface.m_crMargin );
	pColours.SetAt( _T("system.border"), &CoolInterface.m_crBorder );
	pColours.SetAt( _T("system.shadow"), &CoolInterface.m_crShadow );
	pColours.SetAt( _T("system.text"), &CoolInterface.m_crCmdText );
	pColours.SetAt( _T("system.text.selected"), &CoolInterface.m_crCmdTextSel );
	pColours.SetAt( _T("system.disabled"), &CoolInterface.m_crDisabled );
	pColours.SetAt( _T("tooltip.back"), &CoolInterface.m_crTipBack );
	pColours.SetAt( _T("tooltip.text"), &CoolInterface.m_crTipText );
	pColours.SetAt( _T("tooltip.border"), &CoolInterface.m_crTipBorder );
	
	pColours.SetAt( _T("taskpanel.back"), &CoolInterface.m_crTaskPanelBack );
	pColours.SetAt( _T("taskbox.caption.back"), &CoolInterface.m_crTaskBoxCaptionBack );
	pColours.SetAt( _T("taskbox.caption.text"), &CoolInterface.m_crTaskBoxCaptionText );
	pColours.SetAt( _T("taskbox.primary.back"), &CoolInterface.m_crTaskBoxPrimaryBack );
	pColours.SetAt( _T("taskbox.primary.text"), &CoolInterface.m_crTaskBoxPrimaryText );
	pColours.SetAt( _T("taskbox.caption.hover"), &CoolInterface.m_crTaskBoxCaptionHover );
	pColours.SetAt( _T("taskbox.client"), &CoolInterface.m_crTaskBoxClient );
	
	pColours.SetAt( _T("dialog.back"), &m_crDialog );
	pColours.SetAt( _T("panel.caption.back"), &m_crPanelBack );
	pColours.SetAt( _T("panel.caption.text"), &m_crPanelText );
	pColours.SetAt( _T("panel.caption.border"), &m_crPanelBorder );
	pColours.SetAt( _T("banner.back"), &m_crBannerBack );
	pColours.SetAt( _T("banner.text"), &m_crBannerText );
	pColours.SetAt( _T("schema.row1"), &m_crSchemaRow[0] );
	pColours.SetAt( _T("schema.row2"), &m_crSchemaRow[1] );
	
	BOOL bSystem = FALSE, bNonBase = FALSE;
	
	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );
		if ( ! pXML->IsNamed( _T("colour") ) ) continue;

		CString strName		= pXML->GetAttributeValue( _T("name") );
		CString strValue	= pXML->GetAttributeValue( _T("value") );
		strName.MakeLower();

		COLORREF* pColour;

		if ( pColours.Lookup( strName, (void*&)pColour ) && strValue.GetLength() == 6 )
		{
			int nRed, nGreen, nBlue;

			_stscanf( strValue.Mid( 0, 2 ), _T("%x"), &nRed );
			_stscanf( strValue.Mid( 2, 2 ), _T("%x"), &nGreen );
			_stscanf( strValue.Mid( 4, 2 ), _T("%x"), &nBlue );

			if ( strName.Find( _T("system.") ) >= 0 )
			{
				bSystem = TRUE;

				if ( ! bNonBase && strName.Find( _T(".base.") ) < 0 )
				{
					bNonBase = TRUE;
					CoolInterface.CalculateColours( TRUE );
				}
			}

			*pColour = RGB( nRed, nGreen, nBlue );
		}
	}

	if ( bSystem && ! bNonBase ) CoolInterface.CalculateColours( TRUE );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSkin command lookup helper

UINT CSkin::LookupCommandID(CXMLElement* pXML, LPCTSTR pszName)
{
	CString strID = pXML->GetAttributeValue( pszName ? pszName : _T("id") );
	UINT nID = 0;
	
	if ( _istdigit( *(LPCTSTR)strID ) )
	{
		_stscanf( strID, _T("%lu"), &nID );
	}
	else
	{
		nID = CoolInterface.NameToID( strID );
	}
	
	return nID;
}

//////////////////////////////////////////////////////////////////////
// CSkin command map

BOOL CSkin::LoadCommandMap(CXMLElement* pBase)
{
	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );

		if (  pXML->IsNamed( _T("command") ) )
		{
			CString strTemp = pXML->GetAttributeValue( _T("code") );
			UINT nID;

			if ( _stscanf( strTemp, _T("%lu"), &nID ) != 1 )
				return FALSE;

			strTemp = pXML->GetAttributeValue( _T("id") );

			CoolInterface.NameCommand( nID, strTemp );
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSkin fonts

BOOL CSkin::LoadFonts(CXMLElement* pBase, const CString& strPath)
{
	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );
		
		if ( pXML->IsNamed( _T("font") ) )
		{
			CString strName		= pXML->GetAttributeValue( _T("name") );
			CString strFace		= pXML->GetAttributeValue( _T("face") );
			CString strSize		= pXML->GetAttributeValue( _T("size") );
			CString strWeight	= pXML->GetAttributeValue( _T("weight") );
			
			CFont* pFont = NULL;

			if ( strName.CompareNoCase( _T("system.plain") ) == 0 )
			{
				pFont = &CoolInterface.m_fntNormal;
			}
			else if ( strName.CompareNoCase( _T("system.bold") ) == 0 )
			{
				pFont = &CoolInterface.m_fntBold;
			}
			else if ( strName.CompareNoCase( _T("panel.caption") ) == 0 )
			{
				pFont = &CoolInterface.m_fntCaption;
			}
			else
			{
				continue;
			}
			
			if ( pFont->m_hObject ) pFont->DeleteObject();

			if ( strWeight.CompareNoCase( _T("bold") ) == 0 )
				strWeight = _T("700");
			else if ( strWeight.CompareNoCase( _T("normal") ) == 0 )
				strWeight = _T("400");

			int nFontSize = 11, nFontWeight = FW_NORMAL;

			_stscanf( strSize, _T("%i"), &nFontSize );
			_stscanf( strWeight, _T("%i"), &nFontWeight );

			pFont->CreateFont( -nFontSize, 0, 0, 0, nFontWeight, FALSE, FALSE, FALSE,
				DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
				DEFAULT_PITCH|FF_DONTCARE, strFace );
		}
		else if ( pXML->IsNamed( _T("import") ) )
		{
			CString strFile = strPath + pXML->GetAttributeValue( _T("path") );
			BOOL bSuccess = FALSE;
			
			if ( HINSTANCE hGDI = LoadLibrary( _T("gdi32") ) )
			{
				int (WINAPI *pfnAddFontResourceEx)(LPCTSTR, DWORD, PVOID);
				
#ifdef _UNICODE
				(FARPROC&)pfnAddFontResourceEx = GetProcAddress( hGDI, "AddFontResourceExW" );
#else
				(FARPROC&)pfnAddFontResourceEx = GetProcAddress( hGDI, "AddFontResourceExA" );
#endif
				
				if ( pfnAddFontResourceEx != NULL )
				{
					bSuccess = (*pfnAddFontResourceEx)( strFile, FR_PRIVATE, NULL );
				}
				
				FreeLibrary( hGDI );
			}
			
			if ( ! bSuccess )
			{
				bSuccess = AddFontResource( strFile );
			}
			
			if ( bSuccess ) m_pFontPaths.AddTail( strFile );
		}
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSkin command images

BOOL CSkin::LoadCommandImages(CXMLElement* pBase, const CString& strPath)
{
	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );
		
		if ( pXML->IsNamed( _T("icon") ) )
		{
			UINT nID = LookupCommandID( pXML );
			if ( nID == 0 ) continue;
			
			CString strFile = strPath;
			strFile += pXML->GetAttributeValue( _T("res") );
			strFile += pXML->GetAttributeValue( _T("path") );
			
			int nPos = strFile.Find( '$' );
			HICON hIcon = NULL;
			
			if ( nPos < 0 )
			{
				if ( ExtractIconEx( strFile, 0, NULL, &hIcon, 1 ) != NULL && hIcon != NULL )
				{
					CoolInterface.AddIcon( nID, hIcon );
				}
			}
			else
			{
				HINSTANCE hInstance = NULL;
				UINT nIconID = 0;
				
				if ( _stscanf( strFile.Left( nPos ), _T("%lu"), &hInstance ) != 1 ) return TRUE;
				if ( _stscanf( strFile.Mid( nPos + 1 ), _T("%lu"), &nIconID ) != 1 ) return TRUE;
				
				hIcon = (HICON)LoadImage( hInstance, MAKEINTRESOURCE(nIconID), IMAGE_ICON, 16, 16, 0 );
				if ( hIcon != NULL ) CoolInterface.AddIcon( nID, hIcon );
			}
		}
		else if ( pXML->IsNamed( _T("bitmap") ) )
		{
			if ( ! LoadCommandBitmap( pXML, strPath ) ) return FALSE;
		}
	}
	
	return TRUE;
}

BOOL CSkin::LoadCommandBitmap(CXMLElement* pBase, const CString& strPath)
{
	static LPCTSTR pszNames[] = { _T("id"), _T("id1"), _T("id2"), _T("id3"), _T("id4"), _T("id5"), _T("id6"), _T("id7"), _T("id8"), _T("id9"), NULL };
	
	CString strFile = strPath;
	strFile += pBase->GetAttributeValue( _T("id") );
	strFile += pBase->GetAttributeValue( _T("path") );
	
	HBITMAP hBitmap = LoadBitmap( strFile );
	if ( hBitmap == NULL ) return TRUE;
	
	strFile = pBase->GetAttributeValue( _T("mask") );
	COLORREF crMask = RGB( 0, 255, 0 );
	
	if ( strFile.GetLength() == 6 )
	{
		int nRed, nGreen, nBlue;
		_stscanf( strFile.Mid( 0, 2 ), _T("%x"), &nRed );
		_stscanf( strFile.Mid( 2, 2 ), _T("%x"), &nGreen );
		_stscanf( strFile.Mid( 4, 2 ), _T("%x"), &nBlue );
		crMask = RGB( nRed, nGreen, nBlue );
	}
	
	CoolInterface.ConfirmImageList();
	int nBase = ImageList_AddMasked( CoolInterface.m_pImages.m_hImageList, hBitmap, crMask );
	
	if ( nBase < 0 )
	{
		DeleteObject( hBitmap );
		return FALSE;
	}
	
	int nIndex = 0;
	
	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );
		if ( ! pXML->IsNamed( _T("image") ) ) continue;
		
		strFile = pXML->GetAttributeValue( _T("index") );
		if ( strFile.GetLength() ) _stscanf( strFile, _T("%i"), &nIndex );
		nIndex += nBase;
		
		for ( int nName = 0 ; pszNames[ nName ] ; nName++ )
		{
			UINT nID = LookupCommandID( pXML, pszNames[ nName ] );
			if ( nID ) CoolInterface.m_pImageMap.SetAt( (LPVOID)nID, (LPVOID)nIndex );
			if ( nName && ! nID ) break;
		}
		
		nIndex -= nBase;
		nIndex ++;
	}
	
	DeleteObject( hBitmap );
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSkin default skin

void CSkin::CreateDefault()
{
	// Clear
	
	Clear();
	
	Settings.General.Language = _T("en");
	
	// Base UI
	
	CoolInterface.Clear();
	CoolInterface.CreateFonts();
	CoolInterface.CalculateColours( FALSE );
	
	// Colour Scheme
	
	m_crDialog					= CoolInterface.GetDialogBkColor();
	m_crPanelBack				= RGB( 60, 60, 60 );
	m_crPanelText				= RGB( 255, 255, 255 );
	m_crPanelBorder				= RGB( 0, 0, 0 );
	m_crBannerBack				= RGB( 122, 161, 230 );
	m_crBannerText				= RGB( 250, 250, 255 );
	m_crSchemaRow[0]			= RGB( 245, 245, 255 );
	m_crSchemaRow[1]			= RGB( 214, 223, 247 );
	
	// Command Icons
	
	CoolInterface.AddIcon( ID_CHECKMARK, theApp.LoadIcon( IDI_CHECKMARK ) );
	
	// Default Menu
	
	CMenu* pMenuBar = new CMenu();
	pMenuBar->LoadMenu( IDR_MAINFRAME );
	m_pMenus.SetAt( _T("CMainWnd"), pMenuBar );
	if ( m_mnuDefault.m_hMenu == NULL ) m_mnuDefault.LoadMenu( IDR_POPUPS );
	
	// Load Definitions
	
	LoadFromResource( NULL, IDR_XML_DEFINITIONS );
	LoadFromResource( NULL, IDR_XML_DEFAULT );
	
	// Copying
	
	CoolInterface.CopyIcon( ID_HELP_FAQ, ID_HELP_WEB_1 );
	CoolInterface.CopyIcon( ID_HELP_FAQ, ID_HELP_WEB_2 );
	CoolInterface.CopyIcon( ID_HELP_FAQ, ID_HELP_WEB_3 );
	CoolInterface.CopyIcon( ID_HELP_FAQ, ID_HELP_WEB_4 );
	CoolInterface.CopyIcon( ID_HELP_FAQ, ID_HELP_WEB_5 );
	CoolInterface.CopyIcon( ID_HELP_FAQ, ID_HELP_WEB_6 );
	
	// Plugins
	
	Plugins.RegisterCommands();
}

void CSkin::Finalise()
{
	m_brDialog.CreateSolidBrush( m_crDialog );
	
	if ( HBITMAP hPanelMark = GetWatermark( _T("CPanelWnd.Caption") ) )
	{
		m_bmPanelMark.Attach( hPanelMark );
	}
	else if ( m_crPanelBack == RGB( 60, 60, 60 ) )
	{
		m_bmPanelMark.LoadBitmap( IDB_PANEL_MARK );
	}
	
	CoolMenu.SetWatermark( GetWatermark( _T("CCoolMenu") ) );
	
	Plugins.OnSkinChanged();
}

//////////////////////////////////////////////////////////////////////
// CSkin popup menu helper

UINT CSkin::TrackPopupMenu(LPCTSTR pszMenu, const CPoint& point, UINT nDefaultID, UINT nFlags)
{
	CMenu* pPopup = GetMenu( pszMenu );
	if ( pPopup == NULL ) return 0;
	
	if ( nDefaultID != 0 )
	{
		MENUITEMINFO pInfo;
		pInfo.cbSize	= sizeof(pInfo);
		pInfo.fMask		= MIIM_STATE;
		GetMenuItemInfo( pPopup->GetSafeHmenu(), nDefaultID, FALSE, &pInfo );
		pInfo.fState	|= MFS_DEFAULT;
		SetMenuItemInfo( pPopup->GetSafeHmenu(), nDefaultID, FALSE, &pInfo );
	}
	
	return pPopup->TrackPopupMenu( TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|nFlags,
		point.x, point.y, AfxGetMainWnd() );
}

//////////////////////////////////////////////////////////////////////
// CSkin draw wrapped text

void CSkin::DrawWrappedText(CDC* pDC, CRect* pBox, LPCTSTR pszText, BOOL bExclude)
{
	CPoint pt = pBox->TopLeft();
	
	LPCTSTR pszWord = pszText;
	LPCTSTR pszScan = pszText;

	UINT nFlags = ETO_CLIPPED | ( bExclude ? ETO_OPAQUE : 0 );
	
	for ( ; ; pszScan++ )
	{
		if ( *pszScan != NULL && (unsigned char)*pszScan > 32 ) continue;
		
		if ( pszWord < pszScan )
		{
			int nLen = pszScan - pszWord + ( *pszScan ? 1 : 0 );
			CSize sz = pDC->GetTextExtent( pszWord, nLen );

			if ( pt.x > pBox->left && pt.x + sz.cx > pBox->right )
			{
				pt.x = pBox->left;
				pt.y += sz.cy;
			}

			CRect rc( pt.x, pt.y, pt.x + sz.cx, pt.y + sz.cy );

			pDC->ExtTextOut( pt.x, pt.y, nFlags, &rc, pszWord, nLen, NULL );
			if ( bExclude ) pDC->ExcludeClipRect( &rc );
			
			pt.x += sz.cx;
			pBox->top = pt.y + sz.cy;
		}

		pszWord = pszScan + 1;
		if ( ! *pszScan ) break;
	}
}

//////////////////////////////////////////////////////////////////////
// CSkin load bitmap helper

HBITMAP CSkin::LoadBitmap(CString& strName)
{
	CImageServices pServices;
	CImageFile pFile( &pServices );
	int nPos = strName.Find( '$' );
	
	if ( nPos < 0 )
	{
		pFile.LoadFromFile( strName );
	}
	else
	{
		HINSTANCE hInstance = NULL;
		UINT nID = 0;
		
		if ( _stscanf( strName.Left( nPos ), _T("%lu"), &hInstance ) != 1 ) return NULL;
		if ( _stscanf( strName.Mid( nPos + 1 ), _T("%lu"), &nID ) != 1 ) return NULL;
		
		if ( LPCTSTR pszType = _tcsrchr( strName, '.' ) )
		{
			pszType ++;
			pFile.LoadFromResource( hInstance, nID, pszType );
		}
		else
		{
			return (HBITMAP)LoadImage( hInstance, MAKEINTRESOURCE(nID), IMAGE_BITMAP, 0, 0, 0 );
		}
	}
	
	return pFile.EnsureRGB() ? pFile.CreateBitmap() : NULL;
}

//////////////////////////////////////////////////////////////////////
// CSkin mode suffixes

LPCTSTR CSkin::m_pszModeSuffix[3][4] =
{
	{ _T(".Windowed"), _T(""), NULL, NULL },			// Windowed
	{ _T(".Tabbed"), _T(""), NULL, NULL },				// Tabbed
	{ _T(".Basic"), _T(".Tabbed"), _T(""), NULL }		// Basic
};
