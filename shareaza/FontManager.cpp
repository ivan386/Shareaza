//
// FontManager.cpp
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
#include "FontManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CFontManager::CFontManager(void)
{
	LOGFONT lf;
	theApp.m_gdiFont.GetLogFont( &lf );

	HDC hDC = GetDC( NULL );
	if ( lf.lfHeight < 0 )
		lf.lfHeight = -lf.lfHeight;

	m_nDefaultSize = (WORD)MulDiv( lf.lfHeight, 72, GetDeviceCaps( hDC, LOGPIXELSY ) );
	m_sFontName = lf.lfFaceName;

	ReleaseDC( NULL, hDC );
}

const DLGTEMPLATE* CFontManager::PreCreateDialog(_AFX_OCC_DIALOG_INFO* pOccDialogInfo,
												 const DLGTEMPLATE* pOrigTemplate)
{
	const DLGTEMPLATE *lpNewTemplate = COccManager::PreCreateDialog( pOccDialogInfo, pOrigTemplate );

	if ( !pOccDialogInfo->m_pNewTemplate )
	{
		CDialogTemplate temp( lpNewTemplate );

		temp.SetFont( m_sFontName, m_nDefaultSize );
		pOccDialogInfo->m_pNewTemplate = (DLGTEMPLATE*)temp.Detach();
		return pOccDialogInfo->m_pNewTemplate;
	}
	else
		return lpNewTemplate;
}
