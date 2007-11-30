//
// Skin.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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
#include "Buffer.h"

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
	// experimental values
	m_pStrings.InitHashTable( 1531 );
	m_pMenus.InitHashTable( 83 );
	m_pToolbars.InitHashTable( 61 );
	m_pDocuments.InitHashTable( 61 );
	m_pWatermarks.InitHashTable( 31 );
	m_pLists.InitHashTable( 31 );
	m_pDialogs.InitHashTable( 127 );
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
		m_pMenus.GetNextAssoc( pos, strName, pMenu );
		delete pMenu;
	}
	
	for ( pos = m_pToolbars.GetStartPosition() ; pos ; )
	{
		CCoolBarCtrl* pBar;
		m_pToolbars.GetNextAssoc( pos, strName, pBar );
		delete pBar;
	}
	
	for ( pos = m_pDialogs.GetStartPosition() ; pos ; )
	{
		CXMLElement* pXML;
		m_pDialogs.GetNextAssoc( pos, strName, pXML );
		delete pXML;
	}
	
	for ( pos = m_pDocuments.GetStartPosition() ; pos ; )
	{
		CXMLElement* pXML;
		m_pDocuments.GetNextAssoc( pos, strName, pXML );
		delete pXML;
	}
	
	for ( pos = m_pSkins.GetHeadPosition() ; pos ; )
	{
		delete m_pSkins.GetNext( pos );
	}
	
	BOOL bSuccess = FALSE;
	// First try to remove the fonts using RemoveFontResourceEx
	if ( theApp.m_hGDI32 != NULL )
	{
		int (WINAPI *pfnRemoveFontResourceExW)(LPCWSTR, DWORD, PVOID);
		
		(FARPROC&)pfnRemoveFontResourceExW = GetProcAddress( theApp.m_hGDI32, "RemoveFontResourceExW" );
		
		if ( pfnRemoveFontResourceExW != NULL )
		{
			bSuccess = TRUE;
			for ( pos = m_pFontPaths.GetHeadPosition() ; pos ; )
			{
				bSuccess &= (*pfnRemoveFontResourceExW)( m_pFontPaths.GetNext( pos ), FR_PRIVATE, NULL );
				ASSERT( bSuccess );
			}
			
		}
	}

	// Second try removing the fonts using RemoveFontResource
	if ( !bSuccess )
	{
		for ( pos = m_pFontPaths.GetHeadPosition() ; pos ; )
		{
			ASSERT( RemoveFontResource( m_pFontPaths.GetNext( pos ) ) );
		}
	}
	
	m_pStrings.RemoveAll();
	m_pControlTips.RemoveAll();
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
	HMODULE hModule = ( hInstance != NULL ) ? hInstance : GetModuleHandle( NULL );
	CString strBody( ::LoadHTML( hModule, nResourceID ) );
	CString strPath;
	strPath.Format( _T("%Iu$"), reinterpret_cast< HANDLE_PTR >( hModule ) );
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
		else if ( pSub->IsNamed( _T("controltips") ) )
		{
			if ( ! LoadControlTips( pSub ) ) break;
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
		else if ( pSub->IsNamed( _T("commandMap") ) || pSub->IsNamed( _T("tipMap") ) || 
				  pSub->IsNamed( _T("resourceMap") ) )
		{
			if ( ! LoadResourceMap( pSub ) ) break;
		}
		else if ( pSub->IsNamed( _T("documents" ) ) )
		{
			if ( ! LoadDocuments( pSub ) ) break;
		}
		else if ( pSub->IsNamed( _T("manifest") ) )
		{
			CString strType = pSub->GetAttributeValue( _T("type") );
			ToLower( strType );

			if ( strType == _T("language") )
			{
				Settings.General.Language = pSub->GetAttributeValue( _T("language"), _T("en") );
				theApp.m_bRTL = ( pSub->GetAttributeValue( _T("dir"), _T("ltr") ) == "rtl" );
				theApp.WriteProfileInt( _T("Settings"), _T("LanguageRTL"), theApp.m_bRTL );
				TRACE( _T("Loading language: %s\r\n"), Settings.General.Language );
				TRACE( _T("RTL: %d\r\n"), theApp.m_bRTL );
			}
			else if ( strType == _T("skin") )
			{
				CString strSkinName = pSub->GetAttributeValue( _T("name"), _T("") );

				TRACE( _T("Loading skin: %s\r\n"), strSkinName );
			}
			else
				ASSERT( FALSE );
		}
		else
			ASSERT( FALSE );

		bSuccess = TRUE;
	}

	ASSERT( bSuccess );
	return bSuccess;
}

//////////////////////////////////////////////////////////////////////
// CSkin strings

BOOL CSkin::LoadString(CString& str, UINT nStringID)
{
	if ( m_pStrings.Lookup( nStringID, str ) ||
		( IS_INTRESOURCE( nStringID ) && str.LoadString( nStringID ) ) )
		return TRUE;
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
				
				for (;;)
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
					strValue.Replace( _T("%lu"), _T("%I64i") );
				}
				
				m_pStrings.SetAt( nID, strValue );
			}
			else
				ASSERT( FALSE );
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
			else
				TRACE( _T("LookupCommandID failed in CSkin::LoadStrings\r\n") );
		}
		else
			ASSERT( FALSE );
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSkin dialog control tips

BOOL CSkin::LoadControlTip(CString& str, UINT nCtrlID)
{
	if ( m_pControlTips.Lookup( nCtrlID, str ) ) return TRUE;
	str.Empty();
	return FALSE;
}

BOOL CSkin::LoadControlTips(CXMLElement* pBase)
{
	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );

		if ( pXML->IsNamed( _T("tip") ) )
		{
			if ( UINT nID = LookupCommandID( pXML ) )
			{
				CString strMessage = pXML->GetAttributeValue( _T("message") );
				strMessage.Replace( _T("{n}"), _T("\r\n") );
				m_pControlTips.SetAt( nID, strMessage );
			}
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSkin menus

CMenu* CSkin::GetMenu(LPCTSTR pszName) const
{
	ASSERT( Settings.General.GUIMode == GUI_WINDOWED || 
		Settings.General.GUIMode == GUI_TABBED ||
		Settings.General.GUIMode == GUI_BASIC );
	ASSERT( pszName != NULL ); 
	CString strName( pszName );
	CMenu* pMenu = NULL;
	for ( int nModeTry = 0 ;
		m_pszModeSuffix[ Settings.General.GUIMode ][ nModeTry ] ; nModeTry++ )
	{
		if ( m_pMenus.Lookup( strName +
			m_pszModeSuffix[ Settings.General.GUIMode ][ nModeTry ], pMenu ) )
			break;
	}
	ASSERT( pMenu != NULL && ::IsMenu( pMenu->m_hMenu ) );	
	return pMenu;
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
	
	if ( m_pMenus.Lookup( strName, pMenu ) )
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

CMenu* CSkin::CreatePopupMenu(LPCTSTR pszName)
{
	CMenu* pMenu = new CMenu();
	if ( pMenu )
	{
		if ( pMenu->CreatePopupMenu() )
		{
			m_pMenus.SetAt( pszName, pMenu );
			return pMenu;
		}
		delete pMenu;
	}
	return NULL;
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
				
				VERIFY( AppendMenu( hMenu, MF_STRING, nID, strText ) );
			}
		}
		else if ( pXML->IsNamed( _T("menu") ) )
		{
			HMENU hSubMenu = ::CreatePopupMenu();
			
			if ( ! CreateMenu( pXML, hSubMenu ) )
			{
				DestroyMenu( hSubMenu );
				return FALSE;
			}
			
			VERIFY( AppendMenu( hMenu, MF_STRING|MF_POPUP, (UINT_PTR)hSubMenu, strText ) );
		}
		else if ( pXML->IsNamed( _T("separator") ) )
		{
			VERIFY( AppendMenu( hMenu, MF_SEPARATOR, ID_SEPARATOR, NULL ) );
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// CSkin toolbars

BOOL CSkin::CreateToolBar(LPCTSTR pszName, CCoolBarCtrl* pBar)
{
	if ( pszName == NULL ) return FALSE;
	
	if (pBar->m_hWnd)
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
	
	ASSERT( Settings.General.GUIMode == GUI_WINDOWED || 
		Settings.General.GUIMode == GUI_TABBED ||
		Settings.General.GUIMode == GUI_BASIC );
	LPCTSTR* pszModeSuffix = m_pszModeSuffix[ Settings.General.GUIMode ];
	CCoolBarCtrl* pBase = NULL;
	CString strName( pszName );
	
	for ( int nModeTry = 0 ; nModeTry < 3 && pszModeSuffix[ nModeTry ] ; nModeTry++ )
	{
		if ( m_pToolbars.Lookup( strName + pszModeSuffix[ nModeTry ], pBase ) ) break;
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

CCoolBarCtrl* CSkin::GetToolBar(LPCTSTR pszName) const
{
	CCoolBarCtrl* pBar;
	return m_pToolbars.Lookup( pszName, pBar ) ? pBar : NULL;
}

BOOL CSkin::LoadToolbars(CXMLElement* pBase)
{
	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );
		BOOL bToolbar = pXML->IsNamed( _T("toolbar") );

		ASSERT( bToolbar );
		if ( bToolbar && ! CreateToolBar( pXML ) ) return FALSE;
	}

	return TRUE;
}

CCoolBarCtrl* CSkin::CreateToolBar(LPCTSTR pszName)
{
	CCoolBarCtrl* pBar = new CCoolBarCtrl();
	if ( pBar )
	{
		m_pToolbars.SetAt( pszName, pBar );
		return pBar;
	}
	return NULL;
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
			pBar->Add( UINT( ID_RIGHTALIGN ) );
		}
		else if ( pXML->IsNamed( _T("control") ) )
		{
			UINT nWidth, nHeight = 0;
			CString strTemp;
			
			UINT nID = LookupCommandID( pXML );
			if ( nID )
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
	if ( m_pToolbars.Lookup( strName, pOld ) && pOld ) delete pOld;
	
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
		if ( m_pDocuments.Lookup( strName, pOld ) ) delete pOld;
		
		m_pDocuments.SetAt( strName, pDoc->Detach() );
	}
	
	return TRUE;
}

CXMLElement* CSkin::GetDocument(LPCTSTR pszName)
{
	CXMLElement* pXML = NULL;

	if ( m_pDocuments.Lookup( pszName, pXML ) ) return pXML;
	
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
	
	if ( theApp.m_bRTL ) 
		pCtrl->ModifyStyleEx( 0, WS_EX_LAYOUTRTL, 0 );
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

CString CSkin::GetHeaderTranslation(LPCTSTR pszClassName, LPCTSTR pszHeaderName)
{
	CString strEdit;
	if ( ! m_pLists.Lookup( pszClassName, strEdit ) ) 
		return CString( pszHeaderName );

	CString strOriginal( pszHeaderName );
	strOriginal += L"=";
	LPCTSTR pszFind = _tcsistr( strEdit, strOriginal );

	if ( pszFind )
	{
		pszFind += strOriginal.GetLength();
		CString strNew = pszFind;
		strNew = strNew.SpanExcluding( _T("|") );
		return strNew;
	}
	return CString( pszHeaderName );
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

BOOL CSkin::Apply(LPCTSTR pszName, CDialog* pDialog, UINT nIconID, CToolTipCtrl* pWndTooltips)
{
	if ( nIconID )
	{
		CoolInterface.SetIcon( nIconID, FALSE, FALSE, pDialog );
	}
	
	CString strName;

	if ( pszName == NULL )
		strName = pDialog->GetRuntimeClass()->m_lpszClassName;
	else
		strName = pszName;
	
	CWnd* pWnd = pDialog->GetWindow( GW_CHILD );
	CString strCaption, strTip;
	
	for ( int nCount = 0 ; pWnd ; nCount++, pWnd = pWnd->GetNextWindow() )
	{
		TCHAR szClass[3] = { 0, 0, 0 };
		LoadControlTip( strTip, pWnd->GetDlgCtrlID() );

		if ( pWndTooltips && strTip.GetLength() )
		{
			pWndTooltips->AddTool( pWnd, strTip );
		}

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
				strCaption.Replace( _T("&"), _T("_") );
				strCaption.Replace( _T("\""), _T("&quot;") );
				pFile.Write( "\t<control caption=\"", 19 );

				//pszOutput = T2A(strCaption);
				//pFile.Write( pszOutput, strlen(pszOutput) );
				int nBytes = WideCharToMultiByte( CP_ACP, 0, strCaption, strCaption.GetLength(), NULL, 0, NULL, NULL );
				LPSTR pBytes = new CHAR[nBytes];
				WideCharToMultiByte( CP_ACP, 0, strCaption, strCaption.GetLength(), pBytes, nBytes, NULL, NULL );
				pFile.Write( pBytes, nBytes );
				delete [] pBytes;

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
	if ( ! m_pDialogs.Lookup( strName, pBase ) ) return FALSE;
	
	if ( strCaption != pBase->GetAttributeValue( _T("cookie") ) ) return FALSE;
	
	strCaption = pBase->GetAttributeValue( _T("caption") );
	if ( strCaption.GetLength() ) pDialog->SetWindowText( strCaption );
	
	pWnd = pDialog->GetWindow( GW_CHILD );
	
	for ( POSITION pos = pBase->GetElementIterator() ; pos && pWnd ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );

		TCHAR szClass[3] = { 0, 0, 0 };
		GetClassName( pWnd->GetSafeHwnd(), szClass, 3 );

		// Needed for some controls like Schema combo box
		if ( theApp.m_bRTL && (CString)szClass != "Ed" ) 
			pWnd->ModifyStyleEx( 0, WS_EX_LAYOUTRTL|WS_EX_RTLREADING, 0 );

		if ( pXML->IsNamed( _T("control") ) )
		{
			strCaption = pXML->GetAttributeValue( _T("caption") );
			strTip = pXML->GetAttributeValue( L"tip" );
			if ( pWndTooltips && strTip.GetLength() )
			{
				pWndTooltips->AddTool( pWnd, strTip );
			}

			strCaption.Replace( _T("{n}"), _T("\r\n") );
			
			if ( strCaption.GetLength() )
			{	
				if ( (CString) szClass != "Co" )
				{
					int nPos = strCaption.Find( '_' );
					if ( nPos >= 0 ) strCaption.SetAt( nPos, '&' );
					pWnd->SetWindowText( strCaption );
				}
				else
				{
					CArray< CString > pItems;
					CString strTemp;
					int nNum;

					Split( strCaption, _T("|"), pItems, TRUE );
					CComboBox* pCombo = (CComboBox*) pWnd;
					nNum = pCombo->GetCount();
					if ( nNum == pItems.GetSize() ) 
					{
						for ( int nCount = 0; nCount < nNum; nCount++ )
						{
							pCombo->DeleteString( 0 );
							strTemp = pItems.GetAt( nCount );
							pCombo->AddString( (LPCTSTR) strTemp );
						}
					}
				}
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
	
	if ( m_pDialogs.Lookup( pszName, pBase ) )
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

			if ( m_pDialogs.Lookup( strName, pOld ) ) delete pOld;

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
		CSkinWindow* pSkin = m_pSkins.GetNext( pos );
		if ( pSkin->m_sTargets.Find( strWindow ) >= 0 ) return pSkin;
	}
	
	return NULL;
}

CSkinWindow* CSkin::GetWindowSkin(CWnd* pWnd)
{
	ASSERT( Settings.General.GUIMode == GUI_WINDOWED || 
		Settings.General.GUIMode == GUI_TABBED ||
		Settings.General.GUIMode == GUI_BASIC );
	LPCTSTR* pszModeSuffix = m_pszModeSuffix[ Settings.General.GUIMode ];
	BOOL bPanel = FALSE;
	
	ASSERT(pWnd != NULL);

	if ( pWnd->IsKindOf( RUNTIME_CLASS(CChildWnd) ) )
	{
		CChildWnd* pChild = (CChildWnd*)pWnd;
		bPanel = pChild->m_bPanelMode;
	}
	#ifdef _AFXDLL
	for ( CRuntimeClass* pClass = pWnd->GetRuntimeClass() ; pClass && pClass->m_pfnGetBaseClass ; pClass = pClass->m_pfnGetBaseClass() )
	#else
	for ( CRuntimeClass* pClass = pWnd->GetRuntimeClass() ; pClass ; pClass = pClass->m_pBaseClass )
	#endif
	{
		if ( bPanel )
		{
			CSkinWindow* pSkin = GetWindowSkin( CString( pClass->m_lpszClassName ), _T(".Panel") );
			if ( pSkin != NULL ) return pSkin;
		}
		
		for ( int nSuffix = 0 ; nSuffix < 3 && pszModeSuffix[ nSuffix ] != NULL ; nSuffix ++ )
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
	pColours.SetAt( _T("system.base.text"), &CoolInterface.m_crText );
	pColours.SetAt( _T("system.base.hitext"), &CoolInterface.m_crHiText );
	pColours.SetAt( _T("system.base.hiborder"), &CoolInterface.m_crHiBorder );
	pColours.SetAt( _T("system.base.highlight"), &CoolInterface.m_crHighlight );
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
	pColours.SetAt( _T("tooltip.warnings"), &CoolInterface.m_crTipWarnings );
	
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

//	The color is controlled by media player plugin, thus we can not skin it.
//	pColours.SetAt( _T("media.window"), &CoolInterface.m_crMediaWindow );
	pColours.SetAt( _T("media.window.text"), &CoolInterface.m_crMediaWindowText );
	pColours.SetAt( _T("media.status"), &CoolInterface.m_crMediaStatus );
	pColours.SetAt( _T("media.status.text"), &CoolInterface.m_crMediaStatusText );
	pColours.SetAt( _T("media.panel"), &CoolInterface.m_crMediaPanel );
	pColours.SetAt( _T("media.panel.text"), &CoolInterface.m_crMediaPanelText );
	pColours.SetAt( _T("media.panel.active"), &CoolInterface.m_crMediaPanelActive );
	pColours.SetAt( _T("media.panel.active.text"), &CoolInterface.m_crMediaPanelActiveText );
	pColours.SetAt( _T("media.panel.caption"), &CoolInterface.m_crMediaPanelCaption );
	pColours.SetAt( _T("media.panel.caption.text"), &CoolInterface.m_crMediaPanelCaptionText );

	pColours.SetAt( _T("traffic.window.back"), &CoolInterface.m_crTrafficWindowBack );
	pColours.SetAt( _T("traffic.window.text"), &CoolInterface.m_crTrafficWindowText );
	pColours.SetAt( _T("traffic.window.grid"), &CoolInterface.m_crTrafficWindowGrid );

	pColours.SetAt( _T("monitor.history.back"), &CoolInterface.m_crMonitorHistoryBack );
	pColours.SetAt( _T("monitor.history.back.max"), &CoolInterface.m_crMonitorHistoryBackMax );
	pColours.SetAt( _T("monitor.history.text"), &CoolInterface.m_crMonitorHistoryText );
	pColours.SetAt( _T("monitor.download.line"), &CoolInterface.m_crMonitorDownloadLine );
	pColours.SetAt( _T("monitor.upload.line"), &CoolInterface.m_crMonitorUploadLine );
	pColours.SetAt( _T("monitor.download.bar"), &CoolInterface.m_crMonitorDownloadBar );
	pColours.SetAt( _T("monitor.upload.bar"), &CoolInterface.m_crMonitorUploadBar );

	pColours.SetAt( _T("schema.rating"), &CoolInterface.m_crRatingNull );
	pColours.SetAt( _T("schema.rating0"), &CoolInterface.m_crRating0 );
	pColours.SetAt( _T("schema.rating1"), &CoolInterface.m_crRating1 );
	pColours.SetAt( _T("schema.rating2"), &CoolInterface.m_crRating2 );
	pColours.SetAt( _T("schema.rating3"), &CoolInterface.m_crRating3 );
	pColours.SetAt( _T("schema.rating4"), &CoolInterface.m_crRating4 );
	pColours.SetAt( _T("schema.rating5"), &CoolInterface.m_crRating5 );

	pColours.SetAt( _T("richdoc.back"), &CoolInterface.m_crRichdocBack );
	pColours.SetAt( _T("richdoc.text"), &CoolInterface.m_crRichdocText );
	pColours.SetAt( _T("richdoc.heading"), &CoolInterface.m_crRichdocHeading );

	pColours.SetAt( _T("system.textalert"), &CoolInterface.m_crTextAlert );
	pColours.SetAt( _T("system.textstatus"), &CoolInterface.m_crTextStatus );
	pColours.SetAt( _T("system.textlink"), &CoolInterface.m_crTextLink );
	pColours.SetAt( _T("system.textlink.selected"), &CoolInterface.m_crTextLinkHot );

	pColours.SetAt( _T("system.base.chat.in"), &CoolInterface.m_crChatIn );
	pColours.SetAt( _T("system.base.chat.out"), &CoolInterface.m_crChatOut );
	pColours.SetAt( _T("system.base.chat.null"), &CoolInterface.m_crChatNull );
	pColours.SetAt( _T("system.base.search.null"), &CoolInterface.m_crSearchNull );
	pColours.SetAt( _T("system.base.search.exists"), &CoolInterface.m_crSearchExists );
	pColours.SetAt( _T("system.base.search.exists.hit"), &CoolInterface.m_crSearchExistsHit );
	pColours.SetAt( _T("system.base.search.exists.selected"), &CoolInterface.m_crSearchExistsSelected );
	pColours.SetAt( _T("system.base.search.queued"), &CoolInterface.m_crSearchQueued );
	pColours.SetAt( _T("system.base.search.queued.hit"), &CoolInterface.m_crSearchQueuedHit );
	pColours.SetAt( _T("system.base.search.queued.selected"), &CoolInterface.m_crSearchQueuedSelected );
	pColours.SetAt( _T("system.base.search.ghostrated"), &CoolInterface.m_crSearchGhostrated );
	pColours.SetAt( _T("system.base.search.highrated"), &CoolInterface.m_crSearchHighrated );
	pColours.SetAt( _T("system.base.search.collection"), &CoolInterface.m_crSearchCollection );
	pColours.SetAt( _T("system.base.search.torrent"), &CoolInterface.m_crSearchTorrent );
	pColours.SetAt( _T("system.base.transfer.source"), &CoolInterface.m_crTransferSource );
	pColours.SetAt( _T("system.base.transfer.ranges"), &CoolInterface.m_crTransferRanges );
	pColours.SetAt( _T("system.base.transfer.completed"), &CoolInterface.m_crTransferCompleted );
	pColours.SetAt( _T("system.base.transfer.seeding"), &CoolInterface.m_crTransferVerifyPass );
	pColours.SetAt( _T("system.base.transfer.failed"), &CoolInterface.m_crTransferVerifyFail );
	pColours.SetAt( _T("system.base.transfer.completed.selected"), &CoolInterface.m_crTransferCompletedSelected );
	pColours.SetAt( _T("system.base.transfer.seeding.selected"), &CoolInterface.m_crTransferVerifyPassSelected );
	pColours.SetAt( _T("system.base.transfer.failed.selected"), &CoolInterface.m_crTransferVerifyFailSelected );
	pColours.SetAt( _T("system.base.network.null"), &CoolInterface.m_crNetworkNull );
	pColours.SetAt( _T("system.base.network.g1"), &CoolInterface.m_crNetworkG1 );
	pColours.SetAt( _T("system.base.network.g2"), &CoolInterface.m_crNetworkG2 );
	pColours.SetAt( _T("system.base.network.ed2k"), &CoolInterface.m_crNetworkED2K );
	pColours.SetAt( _T("system.base.network.up"), &CoolInterface.m_crNetworkUp );
	pColours.SetAt( _T("system.base.network.down"), &CoolInterface.m_crNetworkDown );
	pColours.SetAt( _T("system.base.security.allow"), &CoolInterface.m_crSecurityAllow );
	pColours.SetAt( _T("system.base.security.deny"), &CoolInterface.m_crSecurityDeny );

	pColours.SetAt( _T("dropdownbox.back"), &CoolInterface.m_crDropdownBox );
	pColours.SetAt( _T("dropdownbox.text"), &CoolInterface.m_crDropdownText );
	pColours.SetAt( _T("resizebar.edge"), &CoolInterface.m_crResizebarEdge );
	pColours.SetAt( _T("resizebar.face"), &CoolInterface.m_crResizebarFace );
	pColours.SetAt( _T("resizebar.shadow"), &CoolInterface.m_crResizebarShadow );
	pColours.SetAt( _T("resizebar.highlight"), &CoolInterface.m_crResizebarHighlight );
	pColours.SetAt( _T("fragmentbar.shaded"), &CoolInterface.m_crFragmentShaded );
	pColours.SetAt( _T("fragmentbar.complete"), &CoolInterface.m_crFragmentComplete );
	pColours.SetAt( _T("fragmentbar.source1"), &CoolInterface.m_crFragmentSource1 );
	pColours.SetAt( _T("fragmentbar.source2"), &CoolInterface.m_crFragmentSource2 );
	pColours.SetAt( _T("fragmentbar.source3"), &CoolInterface.m_crFragmentSource3 );
	pColours.SetAt( _T("fragmentbar.source4"), &CoolInterface.m_crFragmentSource4 );
	pColours.SetAt( _T("fragmentbar.source5"), &CoolInterface.m_crFragmentSource5 );
	pColours.SetAt( _T("fragmentbar.source6"), &CoolInterface.m_crFragmentSource6 );
	pColours.SetAt( _T("fragmentbar.pass"), &CoolInterface.m_crFragmentPass );
	pColours.SetAt( _T("fragmentbar.fail"), &CoolInterface.m_crFragmentFail );
	pColours.SetAt( _T("fragmentbar.request"), &CoolInterface.m_crFragmentRequest );
	pColours.SetAt( _T("fragmentbar.border"), &CoolInterface.m_crFragmentBorder );
	pColours.SetAt( _T("fragmentbar.border.selected"), &CoolInterface.m_crFragmentBorderSelected );
	pColours.SetAt( _T("fragmentbar.border.simplebar"), &CoolInterface.m_crFragmentBorderSimpleBar );
	pColours.SetAt( _T("fragmentbar.border.simplebar.selected"), &CoolInterface.m_crFragmentBorderSimpleBarSelected );

	pColours.SetAt( _T("system.environment.window"), &CoolInterface.m_crSysWindow );
	pColours.SetAt( _T("system.environment.btnface"), &CoolInterface.m_crSysBtnFace );
	pColours.SetAt( _T("system.environment.3dshadow"), &CoolInterface.m_crSys3DShadow );
	pColours.SetAt( _T("system.environment.3dhighlight"), &CoolInterface.m_crSys3DHighlight );
	pColours.SetAt( _T("system.environment.activecaption"), &CoolInterface.m_crSysActiveCaption );

	BOOL bSystem = FALSE, bNonBase = FALSE;
	
	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );
		if ( ! pXML->IsNamed( _T("colour") ) ) continue;

		CString strName		= pXML->GetAttributeValue( _T("name") );
		CString strValue	= pXML->GetAttributeValue( _T("value") );
		ToLower( strName );

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

UINT CSkin::LookupCommandID(CXMLElement* pXML, LPCTSTR pszName) const
{
	return CoolInterface.NameToID(
		pXML->GetAttributeValue( pszName ? pszName : _T("id") ) );
}

//////////////////////////////////////////////////////////////////////
// CSkin command map

BOOL CSkin::LoadResourceMap(CXMLElement* pBase)
{
	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pBase->GetNextElement( pos );

		if ( pXML->IsNamed( L"command" ) || pXML->IsNamed( L"control" ) || pXML->IsNamed( L"resource" ) )
		{
			CString strTemp = pXML->GetAttributeValue( _T("code") );
			UINT nID;

			if ( _stscanf( strTemp, _T("%lu"), &nID ) != 1 )
				return FALSE;

			CoolInterface.NameCommand( nID, pXML->GetAttributeValue( _T("id") ) );
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
			CString strLanguage	= pXML->GetAttributeValue( _T("language") );
			CString strFace		= pXML->GetAttributeValue( _T("face") );
			CString strSize		= pXML->GetAttributeValue( _T("size") );
			CString strWeight	= pXML->GetAttributeValue( _T("weight") );
			
			if ( ( Settings.General.Language.CompareNoCase( strLanguage ) == 0 ) ||
				 strLanguage.IsEmpty() )
			{
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
				else
					strWeight = _T("400");

				int nFontSize = theApp.m_nDefaultFontSize, nFontWeight = FW_NORMAL;

				_stscanf( strSize, _T("%i"), &nFontSize );
				_stscanf( strWeight, _T("%i"), &nFontWeight );

				pFont->CreateFontW( -nFontSize, 0, 0, 0, nFontWeight, FALSE, FALSE, FALSE,
					DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
					DEFAULT_PITCH|FF_DONTCARE, strFace );

				if ( strName.CompareNoCase( _T("system.plain") ) == 0 )
				{
					pFont = &CoolInterface.m_fntUnder;
					if ( pFont->m_hObject ) pFont->DeleteObject();

					pFont->CreateFontW( -nFontSize, 0, 0, 0, nFontWeight, FALSE, TRUE, FALSE,
							DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
							DEFAULT_PITCH|FF_DONTCARE, strFace );

					pFont = &CoolInterface.m_fntItalic;
					if ( pFont->m_hObject ) pFont->DeleteObject();

					pFont->CreateFontW( -nFontSize, 0, 0, 0, nFontWeight, TRUE, FALSE, FALSE,
							DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
							DEFAULT_PITCH|FF_DONTCARE, strFace );
				}
				else if ( strName.CompareNoCase( _T("system.bold") ) == 0 )
				{
					pFont = &CoolInterface.m_fntBoldItalic;
					if ( pFont->m_hObject ) pFont->DeleteObject();

					pFont->CreateFontW( -nFontSize, 0, 0, 0, nFontWeight, TRUE, FALSE, FALSE,
							DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
							DEFAULT_PITCH|FF_DONTCARE, strFace );
				}
			}
		}
		else if ( pXML->IsNamed( _T("import") ) )
		{
			CString strFile = strPath + pXML->GetAttributeValue( _T("path") );
			BOOL bSuccess = FALSE;

			if ( theApp.m_hGDI32 != NULL )
			{
				int (WINAPI *pfnAddFontResourceExW)(LPCWSTR, DWORD, PVOID);
				
				(FARPROC&)pfnAddFontResourceExW = GetProcAddress( theApp.m_hGDI32, "AddFontResourceExW" );
				
				if ( pfnAddFontResourceExW != NULL )
				{
					bSuccess = (*pfnAddFontResourceExW)( strFile, FR_PRIVATE, NULL );
				}
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
					if ( theApp.m_bRTL ) hIcon = CreateMirroredIcon( hIcon );
					CoolInterface.AddIcon( nID, hIcon );
					VERIFY( DestroyIcon( hIcon ) );
				}
			}
			else
			{
				HINSTANCE hInstance = NULL;
				UINT nIconID = LookupCommandID( pXML, L"res" );
				
				if ( _stscanf( strFile.Left( nPos ), _T("%Iu"), &hInstance ) != 1 ) return TRUE;
				// if ( _stscanf( strFile.Mid( nPos + 1 ), _T("%lu"), &nIconID ) != 1 ) continue;


				hIcon = (HICON)LoadImage( hInstance, MAKEINTRESOURCE(nIconID), IMAGE_ICON, 16, 16, 0 );
				if ( hIcon != NULL )
				{
					if ( theApp.m_bRTL ) hIcon = CreateMirroredIcon( hIcon );
					CoolInterface.AddIcon( nID, hIcon );
					VERIFY( DestroyIcon( hIcon ) );
				}
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
	CString strFile;
	UINT nID = LookupCommandID( pBase );
	// If nID is 0 then we don't want to include it in strFile because
	// strFile must be a file system path rather than a resource path.
	if ( nID )
	{
	strFile.Format( _T("%s%lu%s"),
		strPath,
		nID,
		pBase->GetAttributeValue( _T("path") ) );
	}
	else
	{
	strFile.Format( _T("%s%s"),
		strPath,
		pBase->GetAttributeValue( _T("path") ) );
	}

	HBITMAP hBitmap = LoadBitmap( strFile );
	if ( hBitmap == NULL ) return TRUE;
	if ( theApp.m_bRTL ) hBitmap = CreateMirroredBitmap( hBitmap );

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

	BOOL bResult = CoolInterface.Add( this, pBase, hBitmap, crMask );
	DeleteObject( hBitmap );
	return bResult;
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
	
	HICON hIcon = theApp.LoadIcon( IDI_CHECKMARK );
	if ( hIcon )
	{
		if ( theApp.m_bRTL ) hIcon = CreateMirroredIcon( hIcon );
		CoolInterface.AddIcon( ID_CHECKMARK, hIcon );
		VERIFY( DestroyIcon( hIcon ) );
	}
	
	// Default Menu
	
	CMenu* pMenuBar = new CMenu();
	pMenuBar->LoadMenu( IDR_MAINFRAME );
	m_pMenus.SetAt( _T("CMainWnd"), pMenuBar );
	
	// Load Definitions
	
	LoadFromResource( NULL, IDR_XML_DEFINITIONS );
	LoadFromResource( NULL, IDR_XML_DEFAULT );
	
	// Copying
	CoolInterface.CopyIcon( ID_HELP_FAQ, ID_HELP_GUIDE );
	CoolInterface.CopyIcon( ID_HELP_FAQ, ID_HELP_UPDATE );
	
	CoolInterface.CopyIcon( ID_HELP_FAQ, ID_HELP_WEB_1 );
	CoolInterface.CopyIcon( ID_HELP_FAQ, ID_HELP_WEB_2 );
	CoolInterface.CopyIcon( ID_HELP_FAQ, ID_HELP_WEB_3 );
	CoolInterface.CopyIcon( ID_HELP_FAQ, ID_HELP_WEB_4 );
	CoolInterface.CopyIcon( ID_HELP_FAQ, ID_HELP_WEB_5 );
	CoolInterface.CopyIcon( ID_HELP_FAQ, ID_HELP_WEB_6 );
	
	// Plugins
	Plugins.RegisterCommands();
	Plugins.OnSkinChanged();
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
}

//////////////////////////////////////////////////////////////////////
// CSkin popup menu helper

UINT CSkin::TrackPopupMenu(LPCTSTR pszMenu, const CPoint& point, UINT nDefaultID, UINT nFlags) const
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

// GetTextFlowChange determines the direction of text and its change 
// at word boundaries.
// Returns the direction of the first "words iceland" and the position
// where the next "iceland" starts at the word boundary.
// If there is no change in direction it returns 0.

int CSkin::GetTextFlowChange(LPCTSTR pszText, BOOL* bIsRTL)
{
	TRISTATE bTextIsRTL = TRI_UNKNOWN;
	BOOL bChangeFound   = FALSE;
	LPCTSTR pszWord = pszText;
	LPCTSTR pszScan = pszText;

	int nPos;
	for ( nPos = 0; ; pszScan++, nPos++ )
	{
		// get the first word with punctuation marks and whitespaces
		if ( (unsigned short)*pszScan > 32 && (unsigned short)*pszScan != 160 ) continue;

		if ( pszWord < pszScan )
		{
			int nLen = static_cast< int >( pszScan - pszWord );
			WORD* nCharType = new WORD[ nLen + 1 ];

			TCHAR* pszTestWord = new TCHAR[ nLen + 1 ];
			_tcsncpy( pszTestWord, pszWord, nLen );
			pszTestWord[ nLen ] = 0;

			GetStringTypeEx( LOCALE_NEUTRAL, CT_CTYPE2, pszTestWord, nLen + 1, (LPWORD)nCharType );
			delete [] pszTestWord;

			for ( int i = 0 ; i < nLen ; i++ )
			{
				if ( nCharType[ i ] == C2_LEFTTORIGHT )
				{
					if ( bTextIsRTL == TRI_UNKNOWN )
					{
						bTextIsRTL = TRI_FALSE;
						*bIsRTL = FALSE;
					}
					else if ( bTextIsRTL == TRI_TRUE )
					{
						bChangeFound = TRUE;
						break;
					}
				}
				else if ( nCharType[ i ] == C2_RIGHTTOLEFT )
				{
					if ( bTextIsRTL == TRI_UNKNOWN )
					{
						bTextIsRTL = TRI_TRUE;
						*bIsRTL = TRUE;
					}
					else if ( bTextIsRTL == TRI_FALSE )
					{
						bChangeFound = TRUE;
						break;
					}
				}
			}
			BOOL bLeadingWhiteSpace = ( nCharType[ 0 ] == C2_WHITESPACE );
			delete [] nCharType;

			if ( bChangeFound ) return nPos - nLen + ( bLeadingWhiteSpace ? 1 : 0 );
			pszWord = pszScan;
		}
		if ( ! *pszScan ) break;
	}
	return 0;
}

void CSkin::DrawWrappedText(CDC* pDC, CRect* pBox, LPCTSTR pszText, CPoint ptStart, BOOL bExclude)
{
	// TODO: Wrap mixed text in RTL and LTR layouts correctly

	if ( pszText == NULL ) return;
	if ( ptStart.x == 0 && ptStart.y == 0 ) ptStart = pBox->TopLeft();

	UINT nAlignOptionsOld = pDC->GetTextAlign(); // backup settings
	UINT nFlags = ETO_CLIPPED | ( bExclude ? ETO_OPAQUE : 0 );

	unsigned short nLenFull = static_cast< unsigned short >( _tcslen( pszText ) );

	// Collect stats about the text from the start
	BOOL bIsRTLStart, bNormalFlow;
	int nTestStart	= GetTextFlowChange( pszText, &bIsRTLStart );

	// Guess text direction ( not always works )
	bNormalFlow = theApp.m_bRTL ? bIsRTLStart : !bIsRTLStart;

	TCHAR* pszSource = NULL;
	LPCTSTR pszWord	 = NULL;
	LPCTSTR pszScan  = NULL;

	if ( nTestStart )
	{
		// Get the source string to draw and truncate initial string to pass it recursively
		pszSource = new TCHAR[ nTestStart + 1 ];
		_tcsncpy( pszSource, pszText, nTestStart );
		pszSource[ nTestStart ] = 0;
		if ( !bNormalFlow )
		{
			// swap whitespaces
			CString str = pszSource;
			if ( pszSource[ 0 ] == ' ' || (unsigned short)pszSource[ 0 ] == 160 )
			{
				str = str + _T(" ");
				str = str.Right( nTestStart );
			}
			else if ( pszSource[ nTestStart - 1 ] == ' ' || 
					  (unsigned short)pszSource[ nTestStart - 1 ] == 160 )
			{
				str = _T(" ") + str;
				str = str.Left( nTestStart );
			}
			_tcsncpy( pszSource, str.GetBuffer( nTestStart ), nTestStart );
		}
		nLenFull = static_cast< unsigned short >( nTestStart );
		pszText += nTestStart;
	}
	else 
		pszSource = (TCHAR*)pszText;

	pszWord = pszSource;
	pszScan = pszSource;

	if ( !bNormalFlow ) 
	{
		if ( bIsRTLStart != theApp.m_bRTL ) pDC->SetTextAlign( TA_RTLREADING );
		pszScan += nLenFull - 1;
		pszWord += nLenFull;
		for ( int nEnd = nLenFull - 1; nEnd >= 0 ; nEnd-- )
		{
			if ( nEnd ) pszScan--;
			if ( nEnd && (unsigned short)*pszScan > 32 && (unsigned short)*pszScan != 160 ) continue;

			if ( pszWord >= pszScan )
			{
				int nLen = static_cast< int >( pszWord - pszScan );
				CSize sz;
				GetTextExtentPoint32( pDC->m_hAttribDC, pszScan, nLen, &sz );

				if ( ptStart.x > pBox->left && ptStart.x + sz.cx > pBox->right )
				{
					ptStart.x = pBox->left;
					ptStart.y += sz.cy;
				}

				// Add extra point in x-axis; it cuts off the 1st word character otherwise
				const short nExtraPoint = ( theApp.m_bRTL ) ? 1 : 0;
				CRect rc( ptStart.x, ptStart.y, ptStart.x + sz.cx + nExtraPoint, ptStart.y + sz.cy );
				
				pDC->ExtTextOut( ptStart.x, ptStart.y, nFlags, &rc,
					pszScan, nLen, NULL );
				if ( bExclude ) pDC->ExcludeClipRect( &rc );
				
				ptStart.x += sz.cx + nExtraPoint;
				pBox->top = ptStart.y + sz.cy;
			}
			pszWord = pszScan;
		}
	}
	else
	{
		for ( ; ; pszScan++ )
		{
			if ( *pszScan != NULL && (unsigned short)*pszScan > 32 &&
				 (unsigned short)*pszScan != 160 ) continue;
			
			if ( pszWord <= pszScan )
			{
				int nLen = static_cast< int >( pszScan - pszWord + ( *pszScan ? 1 : 0 ) );
				CSize sz = pDC->GetTextExtent( pszWord, nLen );

				if ( ptStart.x > pBox->left && ptStart.x + sz.cx > pBox->right )
				{
					ptStart.x = pBox->left;
					ptStart.y += sz.cy;
				}

				// Add extra point in x-axis; it cuts off the 1st word character otherwise
				const short nExtraPoint = ( theApp.m_bRTL ) ? 1 : 0;

				CRect rc( ptStart.x, ptStart.y, ptStart.x + sz.cx + nExtraPoint, ptStart.y + sz.cy );

				pDC->ExtTextOut( ptStart.x, ptStart.y, nFlags, &rc,
					pszWord, nLen, NULL );
				if ( bExclude ) pDC->ExcludeClipRect( &rc );
				
				ptStart.x += sz.cx + nExtraPoint;
				pBox->top = ptStart.y + sz.cy;
			}

			pszWord = pszScan + 1;
			if ( ! *pszScan ) break;
		}
	}
	if ( nTestStart ) delete [] pszSource;
	// reset align options back
	pDC->SetTextAlign( nAlignOptionsOld );
	if ( nTestStart ) DrawWrappedText( pDC, pBox, pszText, ptStart, bExclude );
}

//////////////////////////////////////////////////////////////////////
// CSkin load bitmap helper

HBITMAP CSkin::LoadBitmap(CString& strName)
{
	CImageFile pFile;
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
	{ _T(".Windowed"), _T(""), NULL, NULL },			// GUI_WINDOWED
	{ _T(".Tabbed"), _T(""), NULL, NULL },				// GUI_TABBED
	{ _T(".Basic"), _T(".Tabbed"), _T(""), NULL }		// GUI_BASIC
};
