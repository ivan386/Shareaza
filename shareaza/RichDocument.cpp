//
// RichDocument.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2015.
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
#include "CoolInterface.h"
#include "RichDocument.h"
#include "RichElement.h"
#include "XML.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CRichDocument construction

CRichDocument::CRichDocument()
	: m_nCookie		( 0 )
	, m_szMargin	( 8, 8 )
	, m_crBackground( CoolInterface.m_crRichdocBack )
	, m_crText		( CoolInterface.m_crRichdocText )
	, m_crLink		( CoolInterface.m_crTextLink )
	, m_crHover		( CoolInterface.m_crTextLinkHot )
	, m_crHeading	( CoolInterface.m_crRichdocHeading )
{
	CreateFonts();
}

CRichDocument::~CRichDocument()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////
// CRichDocument element access

POSITION CRichDocument::GetIterator() const
{
	return m_pElements.GetHeadPosition();
}

CRichElement* CRichDocument::GetNext(POSITION& pos) const
{
	return m_pElements.GetNext( pos );
}

CRichElement* CRichDocument::GetPrev(POSITION& pos) const
{
	return m_pElements.GetPrev( pos );
}

INT_PTR CRichDocument::GetCount() const
{
	return m_pElements.GetCount();
}

POSITION CRichDocument::Find(CRichElement* pElement) const
{
	return m_pElements.Find( pElement );
}

//////////////////////////////////////////////////////////////////////
// CRichDocument element modification

CRichElement* CRichDocument::Add(CRichElement* pElement, POSITION posBefore)
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( posBefore )
		m_pElements.InsertBefore( posBefore, pElement );
	else
		m_pElements.AddTail( pElement );

	pElement->m_pDocument = this;
	m_nCookie++;

	return pElement;
}

CRichElement* CRichDocument::Add(int nType, LPCTSTR pszText, LPCTSTR pszLink, DWORD nFlags, int nGroup, POSITION posBefore)
{
	return Add( new CRichElement( nType, pszText, pszLink, nFlags, nGroup ), posBefore );
}

CRichElement* CRichDocument::Add(HBITMAP hBitmap, LPCTSTR pszLink, DWORD nFlags, int nGroup, POSITION posBefore )
{
	return Add( new CRichElement( hBitmap, pszLink, nFlags, nGroup ), posBefore );
}

CRichElement* CRichDocument::Add(HICON hIcon, LPCTSTR pszLink, DWORD nFlags, int nGroup, POSITION posBefore )
{
	return Add( new CRichElement( hIcon, pszLink, nFlags, nGroup ), posBefore );
}

void CRichDocument::Remove(CRichElement* pElement)
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( POSITION pos = m_pElements.Find( pElement ) )
	{
		m_pElements.RemoveAt( pos );
		pElement->m_pDocument = NULL;
		m_nCookie++;
	}
}

void CRichDocument::ShowGroup(int nGroup, BOOL bShow)
{
	CSingleLock pLock( &m_pSection, TRUE );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CRichElement* pElement = GetNext( pos );
		if ( pElement->m_nGroup == nGroup ) pElement->Show( bShow );
	}
}

void CRichDocument::ShowGroupRange(int nMin, int nMax, BOOL bShow)
{
	CSingleLock pLock( &m_pSection, TRUE );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		CRichElement* pElement = GetNext( pos );
		if ( pElement->m_nGroup >= nMin && pElement->m_nGroup <= nMax )
			pElement->Show( bShow );
	}
}

void CRichDocument::SetModified()
{
	m_nCookie++;
}

void CRichDocument::Clear()
{
	CSingleLock pLock( &m_pSection, TRUE );

	for ( POSITION pos = GetIterator() ; pos ; )
	{
		delete GetNext( pos );
	}

	m_pElements.RemoveAll();
	m_nCookie++;
}

//////////////////////////////////////////////////////////////////////
// CRichDocument font construction

void CRichDocument::CreateFonts(const LOGFONT* lpDefault, const LOGFONT* lpHeading)
{
	CSingleLock pLock( &m_pSection, TRUE );

	LOGFONT lfDefault = {};
	if ( lpDefault )
	{
		CopyMemory( &lfDefault, lpDefault, sizeof( lfDefault ) );
	}
	else
	{
		CoolInterface.m_fntRichDefault.GetLogFont( &lfDefault );
	}

	lfDefault.lfWeight = FW_NORMAL;
	lfDefault.lfItalic = FALSE;
	lfDefault.lfUnderline = FALSE;
	if ( m_fntNormal.m_hObject ) m_fntNormal.DeleteObject();
	m_fntNormal.CreateFontIndirect( &lfDefault );

	lfDefault.lfWeight = FW_BOLD;
	if ( m_fntBold.m_hObject ) m_fntBold.DeleteObject();
	m_fntBold.CreateFontIndirect( &lfDefault );

	lfDefault.lfWeight = FW_NORMAL;
	lfDefault.lfItalic = TRUE;
	if ( m_fntItalic.m_hObject ) m_fntItalic.DeleteObject();
	m_fntItalic.CreateFontIndirect( &lfDefault );

	lfDefault.lfItalic = FALSE;
	lfDefault.lfUnderline = TRUE;
	if ( m_fntUnder.m_hObject ) m_fntUnder.DeleteObject();
	m_fntUnder.CreateFontIndirect( &lfDefault );

	lfDefault.lfWeight = FW_BOLD;
	if ( m_fntBoldUnder.m_hObject ) m_fntBoldUnder.DeleteObject();
	m_fntBoldUnder.CreateFontIndirect( &lfDefault );

	LOGFONT lfHeading = {};
	if ( lpHeading )
	{
		CopyMemory( &lfHeading, lpHeading, sizeof( lfHeading) );
	}
	else
	{
		CoolInterface.m_fntRichHeading.GetLogFont( &lfHeading );
	}

	if ( m_fntHeading.m_hObject ) m_fntHeading.DeleteObject();
	m_fntHeading.CreateFontIndirect( &lfHeading );
}

//////////////////////////////////////////////////////////////////////
// CRichDocument XML Load

BOOL CRichDocument::LoadXML(CXMLElement* pBase, CElementMap* pMap, int nGroup)
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( pBase == NULL ) return FALSE;

	CString strTemp;

	if ( pBase->IsNamed( _T("document") ) )
	{
		strTemp = pBase->GetAttributeValue( _T("fontFace") );
		if ( strTemp.GetLength() )
		{
			// Change font face
			LOGFONT lfDefault = {};
			CoolInterface.m_fntRichDefault.GetLogFont( &lfDefault );
			_tcsncpy( lfDefault.lfFaceName, strTemp, LF_FACESIZE );

			LOGFONT lfHeading = {};
			CoolInterface.m_fntRichHeading.GetLogFont( &lfHeading );
			_tcsncpy( lfHeading.lfFaceName, strTemp, LF_FACESIZE );

			CreateFonts( &lfDefault, &lfHeading );
		}

		m_crBackground	= CoolInterface.m_crRichdocBack;
		m_crText		= CoolInterface.m_crRichdocText;
		m_crLink		= CoolInterface.m_crTextLink;
		m_crHover		= CoolInterface.m_crTextLinkHot;
		m_crHeading		= CoolInterface.m_crRichdocHeading;

		CSkin::LoadColour( pBase, _T("crBackground"), &m_crBackground );
		CSkin::LoadColour( pBase, _T("crText"), &m_crText );
		CSkin::LoadColour( pBase, _T("crLink"), &m_crLink );
		CSkin::LoadColour( pBase, _T("crHover"), &m_crHover );
		CSkin::LoadColour( pBase, _T("crHeading"), &m_crHeading );

		strTemp = pBase->GetAttributeValue( _T("leftMargin") );
		if ( strTemp.GetLength() && _stscanf( strTemp, _T("%li"), &m_szMargin.cx ) != 1 )
			theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, _T("Bad [leftMargin] attribute in [document] element"), (LPCTSTR)pBase->ToString() );

		strTemp = pBase->GetAttributeValue( _T("topMargin") );
		if ( strTemp.GetLength() && _stscanf( strTemp, _T("%li"), &m_szMargin.cy ) != 1 )
			theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, _T("Bad [topMargin] attribute in [document] element"), (LPCTSTR)pBase->ToString() );
	}

	for ( POSITION pos = pBase->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML		= pBase->GetNextElement( pos );
		CRichElement* pElement	= NULL;

		if ( pXML->IsNamed( _T("text") ) )
		{
			pElement = new CRichElement( retText );
		}
		else if ( pXML->IsNamed( _T("link") ) )
		{
			pElement = new CRichElement( retLink );
		}
		else if ( pXML->IsNamed( _T("heading") ) )
		{
			pElement = new CRichElement( retHeading );
		}
		else if ( pXML->IsNamed( _T("newline") ) )
		{
			pElement = new CRichElement( retNewline );

			strTemp = pXML->GetAttributeValue( _T("gap") );

			if ( strTemp.GetLength() )
			{
				pElement->m_sText = strTemp;
				strTemp = pXML->GetAttributeValue( _T("indent") );
				if ( strTemp.GetLength() ) pElement->m_sText += '.' + strTemp;
			}
			else
			{
				strTemp = pXML->GetAttributeValue( _T("indent") );
				if ( strTemp.GetLength() ) pElement->m_sText = _T("0.") + strTemp;
			}
		}
		else if ( pXML->IsNamed( _T("gap") ) )
		{
			pElement = new CRichElement( retGap );

			strTemp = pXML->GetAttributeValue( _T("size") );
			if ( strTemp ) pElement->m_sText = strTemp;
		}
		else if ( pXML->IsNamed( _T("bitmap") ) )
		{
			pElement = new CRichElement( retBitmap );
		}
		else if ( pXML->IsNamed( _T("icon") ) )
		{
			pElement = new CRichElement( retIcon );
		}
		else if ( pXML->IsNamed( _T("anchor") ) )
		{
			pElement = new CRichElement( retAnchor );
		}
		else if ( pXML->IsNamed( _T("para") ) )
		{
			CString strAlign = pXML->GetAttributeValue( _T("align") );
			pElement = new CRichElement( retAlign, strAlign );
			Add( pElement );

			if ( pXML->GetElementCount() )
			{
				if ( ! LoadXML( pXML, pMap, nGroup ) )
					return FALSE;

				if ( pElement->m_sText.CompareNoCase( _T("left") ) )
				{
					Add( new CRichElement( retAlign, _T("left") ) );
				}
			}

			continue;
		}
		else if ( pXML->IsNamed( _T("group") ) )
		{
			int nSubGroup = 0;
			if ( _stscanf( pXML->GetAttributeValue( _T("id") ), _T("%i"), &nSubGroup ) == 1 )
			{
				if ( ! LoadXML( pXML, pMap, nSubGroup ) )
					return FALSE;
			}
			else
				theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, _T("Bad [id] attribute in [group] element"), (LPCTSTR)pXML->ToString() );
		}
		else if ( pXML->IsNamed( _T("styles") ) )
		{
			if ( ! LoadXMLStyles( pXML ) )
				return FALSE;
		}
		else
			theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, _T("Unknown element in [document] element"), (LPCTSTR)pXML->ToString() );

		if ( pElement == NULL ) continue;

		strTemp = pXML->GetValue();
		if ( strTemp.GetLength() ) pElement->m_sText = strTemp;

		pElement->m_nGroup = nGroup;
		strTemp = pXML->GetAttributeValue( _T("group") );
		if ( strTemp.GetLength() && _stscanf( strTemp, _T("%i"), &pElement->m_nGroup ) != 1 )
			theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, _T("Bad [group] attribute in [document] element"), (LPCTSTR)pXML->ToString() );

		strTemp = pXML->GetAttributeValue( _T("format") );
		if ( strTemp.FindOneOf( _T( "bB" ) ) >= 0 )	pElement->m_nFlags |= retfBold;
		if ( strTemp.FindOneOf( _T( "iI" ) ) >= 0 )	pElement->m_nFlags |= retfItalic;
		if ( strTemp.FindOneOf( _T( "uU" ) ) >= 0 )	pElement->m_nFlags |= retfUnderline;

		strTemp = pXML->GetAttributeValue( _T("align") );
		if ( strTemp.CompareNoCase( _T("middle") ) == 0 ) pElement->m_nFlags |= retfMiddle;

		if ( CSkin::LoadColour( pXML, _T("colour"), &pElement->m_cColour ) ||
			 CSkin::LoadColour( pXML, _T("color"),  &pElement->m_cColour ) )
		{
			pElement->m_nFlags |= retfColour;
		}

		if ( pElement->m_nType == retIcon )
		{
			strTemp = pXML->GetAttributeValue( _T("command") );
			if ( strTemp.GetLength() )
			{
				pElement->m_nType = retCmdIcon;
				pElement->m_sText = strTemp;
			}
		}

		if ( pElement->m_nType == retIcon || pElement->m_nType == retBitmap || pElement->m_nType == retAnchor )
		{
			strTemp = pXML->GetAttributeValue( _T("res") );
			if ( strTemp.GetLength() ) pElement->m_sText = strTemp;
			strTemp = pXML->GetAttributeValue( _T("path") );
			if ( strTemp.GetLength() ) pElement->m_sText = strTemp;

			strTemp = pXML->GetAttributeValue( _T("width") );
			if ( strTemp.GetLength() )
			{
				if ( pElement->m_sText.GetLength() ) pElement->m_sText += '.';
				pElement->m_sText += strTemp;
				strTemp = pXML->GetAttributeValue( _T("height") );
				if ( strTemp.GetLength() ) pElement->m_sText += '.' + strTemp;
			}
		}

		pElement->m_sLink = pXML->GetAttributeValue( _T("target") );

		if ( pMap )
		{
			strTemp = pXML->GetAttributeValue( _T("id") );
			if ( strTemp.GetLength() ) pMap->SetAt( strTemp, pElement );
		}

		Add( pElement );
	}

	return TRUE;
}

BOOL CRichDocument::LoadXMLStyles(CXMLElement* pParent)
{
	for ( POSITION pos = pParent->GetElementIterator() ; pos ; )
	{
		CXMLElement* pXML = pParent->GetNextElement( pos );
		if ( ! pXML->IsNamed( _T("style") ) )
		{
			theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, _T("Unknown element in [styles] element"), (LPCTSTR)pXML->ToString() );
			continue;
		}

		CString strName = pXML->GetAttributeValue( _T("name") );
		strName.MakeLower();
		bool bDefault = ( strName == _T("default") || strName.IsEmpty() );
		bool bHeading = ( strName == _T("heading") );

		LOGFONT lf = {};
		if ( bDefault )
		{
			CoolInterface.m_fntRichDefault.GetLogFont( &lf );
		}
		else if ( bHeading )
		{
			CoolInterface.m_fntRichHeading.GetLogFont( &lf );
		}

		if ( CXMLElement* pFont = pXML->GetElementByName( _T("font") ) )
		{
			CString strFontFace = pFont->GetAttributeValue( _T("face") );
			if ( ! strFontFace.IsEmpty() )
			{
				_tcsncpy( lf.lfFaceName, strFontFace, LF_FACESIZE );
			}
			CString strSize = pFont->GetAttributeValue( _T("size") );
			if ( ! strSize.IsEmpty() )
			{
				int height;
				if ( _stscanf( strSize, _T("%i"), &height ) == 1 )
				{
					lf.lfHeight = - height;
				}
				else
					theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, _T("Bad [size] attribute in [font] element"), (LPCTSTR)pFont->ToString() );
			}
			CString strWeight = pFont->GetAttributeValue( _T("weight") );
			if ( ! strWeight.IsEmpty() )
			{
				int weight;
				if ( _stscanf( strWeight, _T("%i"), &weight ) == 1 )
				{
					lf.lfWeight = weight;
				}
				else
					theApp.Message( MSG_ERROR, IDS_SKIN_ERROR, _T("Bad [weight] attribute in [font] element"), (LPCTSTR)pFont->ToString() );
			}
		}

		CXMLElement* pColours = pXML->GetElementByName( _T("colours") );
		if ( pColours == NULL ) pColours = pXML->GetElementByName( _T("colors") );
		if ( pColours == NULL ) pColours = pXML;

		if ( bDefault )
		{
			CSkin::LoadColour( pColours, _T("text"), &m_crText );
			CSkin::LoadColour( pColours, _T("link"), &m_crLink );
			CSkin::LoadColour( pColours, _T("hover"), &m_crHover );

			// Create specified fonts (using default font as heading font)
			CreateFonts( &lf, NULL );
		}
		else if ( bHeading )
		{
			CSkin::LoadColour( pColours, _T("text"), &m_crHeading );

			// Create heading font
			if ( m_fntHeading.m_hObject ) m_fntHeading.DeleteObject();
			m_fntHeading.CreateFontIndirect( &lf );
		}
	}

	return TRUE;
}
