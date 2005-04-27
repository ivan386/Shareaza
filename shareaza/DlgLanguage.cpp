//
// DlgLanguage.cpp
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
#include "Settings.h"
#include "CoolInterface.h"
#include "DlgLanguage.h"
#include "XML.h"
#include "SkinWindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CLanguageDlg, CSkinDialog)
	//{{AFX_MSG_MAP(CLanguageDlg)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	ON_WM_KEYDOWN()
	ON_WM_SETCURSOR()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#define HEADING_HEIGHT	50
#define ITEM_HEIGHT		40
#define TEXT_MARGIN		0


/////////////////////////////////////////////////////////////////////////////
// CLanguageDlg dialog

CLanguageDlg::CLanguageDlg(CWnd* pParent) : CSkinDialog(CLanguageDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLanguageDlg)
	//}}AFX_DATA_INIT
}

void CLanguageDlg::DoDataExchange(CDataExchange* pDX)
{
	CSkinDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLanguageDlg)
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CLanguageDlg message handlers

BOOL CLanguageDlg::OnInitDialog() 
{
	CSkinDialog::OnInitDialog();

	CWaitCursor pCursor;

	//SkinMe( _T("CLanguageDlg"), ID_TOOLS_LANGUAGE );

	m_hArrow	= theApp.LoadStandardCursor( IDC_ARROW );
	m_hHand		= theApp.LoadCursor( IDC_HAND );

	m_bmHeader.LoadBitmap( IDB_WIZARD );

	m_fntNormal.CreateFont( -(theApp.m_nDefaultFontSize + 1), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, theApp.m_sDefaultFont );

	m_fntBold.CreateFont( -(theApp.m_nDefaultFontSize + 5), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, theApp.m_sDefaultFont );

	m_fntSmall.CreateFont( -(theApp.m_nDefaultFontSize - 1), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, theApp.m_sDefaultFont );

	m_pImages.Create( 32, 32, ILC_COLOR16|ILC_MASK, 1, 1 );

	AddEnglishDefault();
	Enumerate();

	m_nHover	= 0;
	m_nDown		= 0;
	m_bKeyMode	= FALSE;

	CRect rc( 0, 0, 438, HEADING_HEIGHT );

	int nLanguagesToDisplay;

	if( GetSystemMetrics( SM_CYSCREEN ) < 768 )
		nLanguagesToDisplay = min(m_pPaths.GetSize(), 10);
	else
		nLanguagesToDisplay = min(m_pPaths.GetSize(), 14);

	rc.bottom += ( nLanguagesToDisplay ) * ITEM_HEIGHT;

	SCROLLINFO pScroll;
	ZeroMemory( &pScroll, sizeof(pScroll) );
	pScroll.cbSize	= sizeof(pScroll);
	pScroll.fMask	= SIF_RANGE|SIF_PAGE|SIF_DISABLENOSCROLL;
	pScroll.nMin	= 0;
	pScroll.nMax	= m_pPaths.GetSize(); 
	pScroll.nPage	= nLanguagesToDisplay + 1;
	SetScrollInfo( SB_VERT, &pScroll, TRUE );

	//if ( m_pSkin )
	//	m_pSkin->CalcWindowRect( &rc );
	//else
		CalcWindowRect( &rc, adjustBorder );

	rc.OffsetRect(	GetSystemMetrics( SM_CXSCREEN ) / 2 -  rc.Width() / 2 - rc.left,
					GetSystemMetrics( SM_CYSCREEN ) / 2 - rc.Height() / 2 - rc.top );
	
	SetWindowPos( NULL, rc.left, rc.top, rc.Width(), rc.Height() , 0 );

	SetTimer( 1, 100, NULL );
	
	return TRUE;
}

void CLanguageDlg::OnDestroy() 
{
	KillTimer( 1 );
	CSkinDialog::OnDestroy();
}

BOOL CLanguageDlg::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}

void CLanguageDlg::OnPaint() 
{
	CPaintDC dc( this );
	CRect rc;
	int nScroll = GetScrollPos( SB_VERT );

	GetClientRect( &rc );

	CDC mdc;
	mdc.CreateCompatibleDC( &dc );
	CBitmap* pOldBmp = (CBitmap*)mdc.SelectObject( &m_bmHeader );
	dc.BitBlt( 0, 0, rc.Width(), HEADING_HEIGHT, &mdc, 0, 0, SRCCOPY );
	mdc.SelectObject( pOldBmp );
	mdc.DeleteDC();

	rc.top += HEADING_HEIGHT;
	
	CFont* pOldFont = (CFont*)dc.SelectObject( &m_fntNormal );

	for ( int nCount = 0 ; nCount < m_pPaths.GetSize() ; nCount++ )
	{
		if ( nScroll > 0 )
		{
			nScroll --;
		}
		else
		{
			PaintItem( nCount, &dc, &rc );
			rc.OffsetRect( 0, rc.Height() );
		}
	}

	dc.SelectObject( pOldFont );
}

void CLanguageDlg::PaintItem(int nItem, CDC* pDC, CRect* pRect)
{
	pRect->bottom = pRect->top + ITEM_HEIGHT;

	BOOL bHover	= m_nHover == ( nItem + 1 );
	BOOL bDown	= m_nDown  == ( nItem + 1 );

	CRect rc( pRect );

	pDC->Draw3dRect( &rc, CoolInterface.m_crBackNormal, CoolInterface.m_crBackNormal );
	rc.DeflateRect( 1, 1 );

	COLORREF crBack;

	if ( bHover || bDown )
	{
		pDC->Draw3dRect( &rc, CoolInterface.m_crBorder, CoolInterface.m_crBorder );
		pDC->SetBkColor( crBack = ( bDown && bHover ? CoolInterface.m_crBackCheckSel : CoolInterface.m_crBackSel ) );
	}
	else
	{
		pDC->Draw3dRect( &rc, CoolInterface.m_crBackNormal, CoolInterface.m_crBackNormal );
		pDC->SetBkColor( crBack = CoolInterface.m_crBackNormal );
	}

	rc.DeflateRect( 1, 1 );
	
	CPoint ptIcon( rc.left + 4, ( rc.top + rc.bottom ) / 2 - 16 );

	if ( bHover != bDown )
	{
		pDC->FillSolidRect( ptIcon.x - 1, ptIcon.y - 1, 34, 2, crBack );
		pDC->FillSolidRect( ptIcon.x - 1, ptIcon.y + 1, 2, 32, crBack );

		ptIcon.Offset( 1, 1 );

		pDC->SetTextColor( CoolInterface.m_crShadow );
		ImageList_DrawEx( m_pImages.GetSafeHandle(), nItem,
			pDC->GetSafeHdc(), ptIcon.x, ptIcon.y, 32, 32, crBack, CLR_NONE,
			ILD_MASK );

		ptIcon.Offset( -2, -2 );

		ImageList_DrawEx( m_pImages.GetSafeHandle(), nItem,
			pDC->GetSafeHdc(), ptIcon.x, ptIcon.y, 32, 32, CLR_NONE, CLR_NONE,
			ILD_NORMAL );

		pDC->ExcludeClipRect( ptIcon.x, ptIcon.y, ptIcon.x + 34, ptIcon.y + 34 );
	}
	else
	{
		ImageList_DrawEx( m_pImages.GetSafeHandle(), nItem,
			pDC->GetSafeHdc(), ptIcon.x, ptIcon.y, 32, 32, crBack, CLR_NONE,
			ILD_NORMAL );

		pDC->ExcludeClipRect( ptIcon.x, ptIcon.y, ptIcon.x + 32, ptIcon.y + 32 );
	}

	CRect rcText(	rc.left + 46, rc.top + TEXT_MARGIN,
					rc.right - TEXT_MARGIN, rc.top + 20 + TEXT_MARGIN );

	pDC->SetTextColor( bHover || bDown ? CoolInterface.m_crCmdTextSel : CoolInterface.m_crCmdText );
	pDC->SelectObject( &m_fntBold );
	pDC->ExtTextOut( rcText.left + 1, rcText.top + 1, ETO_CLIPPED|ETO_OPAQUE, &rcText,
		m_pTitles.GetAt( nItem ), NULL );
	pDC->ExcludeClipRect( &rcText );

	rcText.left += 2;
	rcText.top = rcText.bottom;
	rcText.bottom = rc.bottom - TEXT_MARGIN;

	pDC->SelectObject( &m_fntSmall );
	DrawWrappedText( pDC, &rcText, m_pPrompts.GetAt( nItem ) );

	pDC->FillSolidRect( &rc, crBack );
}

void CLanguageDlg::DrawWrappedText(CDC* pDC, CRect* pBox, LPCTSTR pszText)
{
	CPoint pt = pBox->TopLeft();

	LPCTSTR pszWord = pszText;
	LPCTSTR pszScan = pszText;

	for ( ; ; pszScan++ )
	{

#ifdef UNICODE
		if ( *pszScan != NULL && (unsigned short)*pszScan > 32 ) continue;
#else
		if ( *pszScan != NULL && (unsigned char)*pszScan > 32 ) continue;
#endif
		
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

			pDC->ExtTextOut( pt.x, pt.y, ETO_CLIPPED|ETO_OPAQUE, &rc,
				pszWord, nLen, NULL );
			pDC->ExcludeClipRect( &rc );
			
			pt.x += sz.cx;
		}

		pszWord = pszScan + 1;
		if ( ! *pszScan ) break;
	}
}


void CLanguageDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO pInfo;
	
	pInfo.cbSize	= sizeof(pInfo);
	pInfo.fMask		= SIF_ALL & ~SIF_TRACKPOS;
	
	GetScrollInfo( SB_VERT, &pInfo );
	int nDelta = pInfo.nPos;
	
	switch ( nSBCode )
	{
	case SB_BOTTOM:
		pInfo.nPos = pInfo.nMax - pInfo.nPage;
		break;
	case SB_LINEDOWN:
		pInfo.nPos ++;
		break;
	case SB_LINEUP:
		pInfo.nPos --;
		break;
	case SB_PAGEDOWN:
		pInfo.nPos += pInfo.nPage;
		break;
	case SB_PAGEUP:
		pInfo.nPos -= pInfo.nPage;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		pInfo.nPos = nPos;
		break;
	case SB_TOP:
		pInfo.nPos = 0;
		break;
	}
	
	pInfo.nPos = max( 0, min( pInfo.nPos, pInfo.nMax - (int)pInfo.nPage + 1 ) );
	if ( pInfo.nPos == nDelta ) return;
	
	SetScrollInfo( SB_VERT, &pInfo, TRUE );

	m_nHover	= 0;
	m_nDown		= 0;
	Invalidate();
}

BOOL CLanguageDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	OnVScroll( SB_THUMBPOSITION, (int)( GetScrollPos( SB_VERT ) - zDelta / WHEEL_DELTA ), NULL );
	return TRUE;
}

void CLanguageDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	CRect rc;
	int nScroll = GetScrollPos( SB_VERT );

	GetClientRect( &rc );
	rc.top += HEADING_HEIGHT;

	if ( rc.PtInRect( point ) )
	{
		int nHover = ( point.y - rc.top ) / ITEM_HEIGHT + 1 + nScroll;

		if ( nHover != m_nHover )
		{
			m_nHover = nHover;
			Invalidate();
		}
	}
	else if ( m_nHover )
	{
		m_nHover = 0;
		Invalidate();
	}

	m_bKeyMode = FALSE;
	
	CSkinDialog::OnMouseMove( nFlags, point );
}

BOOL CLanguageDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if ( nHitTest == HTCLIENT )
	{
		CPoint pt;

		GetCursorPos( &pt );
		ScreenToClient( &pt );

		SetCursor( pt.y > HEADING_HEIGHT ? m_hHand : m_hArrow );
		return TRUE;
	}
	
	return CSkinDialog::OnSetCursor( pWnd, nHitTest, message );
}

void CLanguageDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_nDown = m_nHover;
	SetCapture();
	Invalidate();
}

void CLanguageDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	int nSelected = m_nDown && ( m_nDown == m_nHover ) ? m_nDown : 0;

	m_nDown = m_nHover = 0;

	ReleaseCapture();
	Invalidate();
	UpdateWindow();

	if ( nSelected ) Execute( nSelected );
}

void CLanguageDlg::OnTimer(UINT nIDEvent) 
{
	if ( ! m_nHover || m_bKeyMode ) return;

	CPoint pt;
	CRect rc;

	GetClientRect( &rc );
	GetCursorPos( &pt );
	ScreenToClient( &pt );

	if ( ! rc.PtInRect( pt ) )
	{
		m_nHover = 0;
		Invalidate();
	}
}

BOOL CLanguageDlg::PreTranslateMessage(MSG* pMsg) 
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		OnKeyDown( pMsg->wParam, LOWORD( pMsg->lParam ), HIWORD( pMsg->lParam ) );
		return TRUE;
	}
	
	return CSkinDialog::PreTranslateMessage( pMsg );
}

void CLanguageDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch ( nChar )
	{
	case VK_ESCAPE:
		PostMessage( WM_CLOSE );
		return;
	case VK_UP:
		if ( ! m_nDown )
		{
			m_nHover--;
			m_bKeyMode = TRUE;
			if ( m_nHover < 1 ) m_nHover = m_pPaths.GetSize();
			Invalidate();
		}
		return;
	case VK_DOWN:
		if ( ! m_nDown )
		{
			m_nHover++;
			m_bKeyMode = TRUE;
			if ( m_nHover > m_pPaths.GetSize() ) m_nHover = 1;
			Invalidate();
		}
		return;
	case VK_RETURN:
		if ( m_nHover && ! m_nDown ) Execute( m_nHover );
		return;
	}
	
	CSkinDialog::OnKeyDown( nChar, nRepCnt, nFlags );
}

void CLanguageDlg::AddEnglishDefault()
{
	m_pPaths.Add( _T("") );
	m_pTitles.Add( _T("English (Default)") );
	m_pPrompts.Add( _T("Click here to select English as your natural language.") );
	m_pImages.Add( theApp.LoadIcon( IDI_FLAG_ENGLISH ) );
}

void CLanguageDlg::Enumerate(LPCTSTR pszPath)
{
	WIN32_FIND_DATA pFind;
	CString strPath;
	HANDLE hSearch;

	strPath.Format( _T("%s\\Skins\\%s*.*"),
		(LPCTSTR)Settings.General.Path, pszPath ? pszPath : _T("") );

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
				
				Enumerate( strPath );
			}
			else if (	_tcsistr( pFind.cFileName, _T(".xml") ) != NULL &&
						_tcsicmp( pFind.cFileName, _T("Definitions.xml") ) &&
						_tcsicmp( pFind.cFileName, _T("Default-en.xml") ) )
			{
				AddSkin( pszPath, pFind.cFileName );
			}
		}
		while ( FindNextFile( hSearch, &pFind ) );

		FindClose( hSearch );
	}
}

BOOL CLanguageDlg::AddSkin(LPCTSTR pszPath, LPCTSTR pszName)
{
	CString strXML;
	CFile pFile;
	
	strXML = Settings.General.Path + _T("\\Skins\\");
	if ( pszPath != NULL ) strXML += pszPath;
	strXML += pszName;

    if ( ! pFile.Open( strXML, CFile::modeRead ) ) return FALSE;
	
	DWORD nSource = (DWORD)pFile.GetLength();
	if ( nSource > 4096*1024 ) return FALSE;
	
	CHAR* pSource = new CHAR[ nSource ];
	pFile.Read( pSource, nSource );
	pFile.Close();
	
	BYTE* pByte = (BYTE*)pSource;
	DWORD nByte = nSource;
	
	if ( nByte >= 2 && ( ( pByte[0] == 0xFE && pByte[1] == 0xFF ) || ( pByte[0] == 0xFF && pByte[1] == 0xFE ) ) )
	{
		nByte = nByte / 2 - 1;
		
		if ( pByte[0] == 0xFE && pByte[1] == 0xFF )
		{
			pByte += 2;
			
			for ( DWORD nSwap = 0 ; nSwap < nByte ; nSwap ++ )
			{
				register CHAR nTemp = pByte[ ( nSwap << 1 ) + 0 ];
				pByte[ ( nSwap << 1 ) + 0 ] = pByte[ ( nSwap << 1 ) + 1 ];
				pByte[ ( nSwap << 1 ) + 1 ] = nTemp;
			}
		}
		else
		{
			pByte += 2; 
		}
		
		CopyMemory( strXML.GetBuffer( nByte ), pByte, nByte * 2 );
		strXML.ReleaseBuffer( nByte );
	}
	else
	{
		if ( nByte >= 3 && pByte[0] == 0xEF && pByte[1] == 0xBB && pByte[2] == 0xBF )
		{
			pByte += 3; nByte -= 3;
		}
		
		DWORD nWide = MultiByteToWideChar( CP_UTF8, 0, (LPCSTR)pByte, nByte, NULL, 0 );
		
		MultiByteToWideChar( CP_UTF8, 0, (LPCSTR)pByte, nByte, strXML.GetBuffer( nWide ), nWide );
		strXML.ReleaseBuffer( nWide );
	}
	
	delete [] pSource;
	
	CXMLElement* pXML = NULL;
	
	int nManifest = strXML.Find( _T("<manifest") );
	
	if ( nManifest > 0 )
	{
		CString strManifest = strXML.Mid( nManifest ).SpanExcluding( _T(">") ) + '>';
		
		if ( CXMLElement* pManifest = CXMLElement::FromString( strManifest ) )
		{
			pXML = new CXMLElement( NULL, _T("skin") );
			pXML->AddElement( pManifest );
		}
	}
	
	if ( pXML == NULL )
	{
		pXML = CXMLElement::FromString( strXML, TRUE );
		if ( pXML == NULL ) return FALSE;
	}
	
	strXML.Empty();
	
	CXMLElement* pManifest = pXML->GetElementByName( _T("manifest") );
	
	if ( ! pXML->IsNamed( _T("skin") ) || pManifest == NULL ||
		 pManifest->GetAttributeValue( _T("type") ).CompareNoCase( _T("language") ) )
	{
		delete pXML;
		return FALSE;
	}
	
	CString	strName		= pManifest->GetAttributeValue( _T("name"), pszName );
	CString	strPrompt	= pManifest->GetAttributeValue( _T("prompt") );
	CString strIcon		= pManifest->GetAttributeValue( _T("icon") );
	
	delete pXML;
	
	if ( pszPath != NULL ) strXML += pszPath;
	strXML += pszName;
	
	m_pPaths.Add( strXML );
	m_pTitles.Add( strName );
	m_pPrompts.Add( strPrompt );
	
	if ( strIcon.GetLength() )
	{
		if ( pszPath )
			strIcon = Settings.General.Path + _T("\\Skins\\") + pszPath + strIcon;
		else
			strIcon = Settings.General.Path + _T("\\Skins\\") + strIcon;
	}
	else
	{
		if ( pszPath )
			strIcon = Settings.General.Path + _T("\\Skins\\") + pszPath + strIcon + pszName;
		else
			strIcon = Settings.General.Path + _T("\\Skins\\") + strIcon + pszName;

		strIcon = strIcon.Left( strIcon.GetLength() - 3 ) + _T("ico");
	}

	HICON hIcon;

	if ( ExtractIconEx( strIcon, 0, &hIcon, NULL, 1 ) != NULL && hIcon != NULL )
	{
		m_pImages.Add( hIcon );
	}
	else
	{
		hIcon = theApp.LoadIcon( IDR_MAINFRAME );
		m_pImages.Add( hIcon );
	}
	
	return TRUE;
}

void CLanguageDlg::Execute(int nSelected)
{
	for ( int nItem = 0 ; nItem < m_pPaths.GetSize() ; nItem++ )
	{
		theApp.WriteProfileInt( _T("Skins"), m_pPaths.GetAt( nItem ),
			( nSelected - 1 ) == nItem );
	}

	EndDialog( IDOK );
}

void CLanguageDlg::OnClose() 
{
	EndDialog( IDCANCEL );
}
