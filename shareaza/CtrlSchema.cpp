//
// CtrlSchema.cpp
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
#include "CtrlSchema.h"
#include "Skin.h"
#include "CoolInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CSchemaCtrl, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_VSCROLL()
	ON_WM_SIZE()
	ON_WM_NCPAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
	ON_EN_SETFOCUS(IDC_METADATA_CONTROL, &CSchemaCtrl::OnControlSetFocus)
	ON_EN_CHANGE(IDC_METADATA_CONTROL, &CSchemaCtrl::OnControlEdit)
	ON_CBN_SETFOCUS(IDC_METADATA_CONTROL, &CSchemaCtrl::OnControlSetFocus)
	ON_CBN_SELCHANGE(IDC_METADATA_CONTROL, &CSchemaCtrl::OnControlEdit)
	ON_CBN_EDITCHANGE(IDC_METADATA_CONTROL, &CSchemaCtrl::OnControlEdit)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSchemaCtrl construction

CSchemaCtrl::CSchemaCtrl()
	: m_nCaptionWidth		( 120 )
	, m_nItemHeight			( 32 )
	, m_bShowBorder			( TRUE )
	, m_pSchema				( NULL )
	, m_nScroll				( 0 )
	, m_strMultipleString	( _T( "(" ) + LoadString( IDS_MULTIPLE_VALUES ) + _T( ")" ) )
{
	// Try to get the number of lines to scroll when the mouse wheel is rotated
	if ( ! SystemParametersInfo ( SPI_GETWHEELSCROLLLINES, 0, &m_nScrollWheelLines, 0 ) )
	{
		m_nScrollWheelLines = 3;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSchemaCtrl create and destroy

BOOL CSchemaCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID) 
{
	return CreateEx( WS_EX_CONTROLPARENT | ( Settings.General.LanguageRTL ? WS_EX_LAYOUTRTL : 0 ),
		NULL, NULL, dwStyle | WS_CHILD | WS_VSCROLL | WS_CLIPCHILDREN, rect, pParentWnd, nID );
}

void CSchemaCtrl::OnDestroy() 
{
	SetSchema( NULL );
	CWnd::OnDestroy();
}

void CSchemaCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize( nType, cx, cy );
	m_nScroll = 0;
	Invalidate();
	Layout();
}

/////////////////////////////////////////////////////////////////////////////
// CSchemaCtrl schema selection

void CSchemaCtrl::SetSchema(CSchemaPtr pSchema, BOOL bPromptOnly)
{
	CArray< CWnd* > pRemove;
	pRemove.Append( m_pControls );
	
	m_pControls.RemoveAll();
	m_pCaptions.RemoveAll();
	
	for ( int nControl = 0 ; nControl < pRemove.GetSize() ; nControl++ )
	{
		CWnd* pControl = (CWnd*)pRemove.GetAt( nControl );
		pControl->DestroyWindow();
		delete pControl;
	}
	
	m_nScroll = 0;
	
	if ( ( m_pSchema = pSchema ) == NULL )
	{
		Layout();
		Invalidate();

		for ( POSITION pos = m_pItems.GetStartPosition(); pos; )
		{
			CSchemaMemberPtr pMember;
			CList< CString >* pItems;
			m_pItems.GetNextAssoc( pos, pMember, pItems );
			delete pItems;
		}
		m_pItems.RemoveAll();

		return;
	}

	for ( POSITION posSchema = pSchema->GetMemberIterator() ; posSchema ; )
	{
		CSchemaMemberPtr pMember = pSchema->GetNextMember( posSchema );
		
		if ( bPromptOnly && ! pMember->m_bPrompt || pMember->m_bHidden ) continue;
		
		CWnd* pControl = NULL;
		CRect rc;

		for ( POSITION posMember = pMember->GetItemIterator(); posMember; )
		{
			AddItem( pMember, pMember->GetNextItem( posMember ) );
		}

		CList< CString >* pItems;
		if ( m_pItems.Lookup( pMember, pItems ) )
		{
			CComboBox* pCombo = new CComboBox();
			
			pCombo->Create( WS_CHILD|WS_VISIBLE|WS_BORDER|WS_TABSTOP|CBS_DROPDOWN|CBS_AUTOHSCROLL|WS_VSCROLL, rc, this, IDC_METADATA_CONTROL );
			
			for ( POSITION pos = pItems->GetHeadPosition(); pos; )
			{
				pCombo->AddString( pItems->GetNext( pos ) );
			}

			if ( pMember->m_nMaxLength ) pCombo->LimitText( pMember->m_nMaxLength );
		
			pControl = pCombo;
		}
		else
		{
			CEdit* pEdit = new CEdit();

			pEdit->Create( WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL, rc, this, IDC_METADATA_CONTROL );
			pEdit->ModifyStyleEx( 0, WS_EX_CLIENTEDGE );

			if ( pMember->m_nMaxLength ) pEdit->LimitText( pMember->m_nMaxLength );

			pControl = pEdit;
		}
		
		CString strCaption = pMember->m_sTitle + ':';
		
		m_pCaptions.Add( strCaption );
		m_pControls.Add( pControl );

		SetWindowLongPtr( pControl->GetSafeHwnd(), GWLP_USERDATA, (LONG_PTR)pMember );
		pControl->SetFont( &theApp.m_gdiFont );
	}
	
	Layout();
	Invalidate();
}

CString CSchemaCtrl::GetSchemaURI() const
{
	return m_pSchema ? m_pSchema->GetURI() : CString();
}

void CSchemaCtrl::AddItem(CSchemaMemberPtr pMember, const CString& strItem)
{
	if ( strItem.GetLength() && strItem != NO_VALUE && strItem != MULTI_VALUE )
	{
		CList< CString >* pItems;
		if ( ! m_pItems.Lookup( pMember, pItems ) )
			m_pItems.SetAt( pMember, pItems = new CList< CString >() );

		for ( POSITION pos = pItems->GetHeadPosition(); pos; )
		{
			POSITION posCmp = pos;
			const CString strExisting = pItems->GetNext( pos );

			int nCmp = strExisting.Compare( strItem );
			if ( nCmp == 0 )
				// Duplicate
				return;
			else if ( nCmp > 0 )
			{
				pItems->InsertBefore( posCmp, strItem );
				return;
			}
		}
		pItems->AddTail( strItem );
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSchemaCtrl data transfer

DWORD CSchemaCtrl::UpdateData(CXMLElement* pBase, BOOL bSaveAndValidate, BOOL bRealSave)
{
	DWORD nModified = 0;
	if ( m_pSchema && pBase && pBase->GetName().CompareNoCase( m_pSchema->m_sSingular ) == 0 )
	{
		POSITION pos = m_pSchema->GetMemberIterator();
	
		for ( INT_PTR nControl = 0 ; nControl < m_pControls.GetSize() && pos ; nControl++ )
		{
			CWnd* pControl = m_pControls.GetAt( nControl );
			CSchemaMemberPtr pMember = NULL;
			while ( pos )
			{
				pMember = m_pSchema->GetNextMember( pos );
				if ( (LONG_PTR)pMember == GetWindowLongPtr( pControl->GetSafeHwnd(), GWLP_USERDATA ) ) break;
				pMember = NULL;
			}
		
			if ( pMember == NULL ) break;
		
			const CString strValue = pMember->GetValueFrom( pBase, NO_VALUE, TRUE, TRUE );

			if ( bSaveAndValidate )
			{
				CString strNewValue;
				pControl->GetWindowText( strNewValue );

				// If value was changed
				if ( strNewValue != m_strMultipleString &&
					// ... but don't set empty value if there is no original value
					! ( strNewValue.IsEmpty() && strValue == NO_VALUE ) &&
					( strNewValue != strValue ) )
					// ... save it
				{
					++ nModified;
					if ( bRealSave )
						pMember->SetValueTo( pBase, strNewValue, TRUE );
				}
			}
			else
			{
				if ( strValue == MULTI_VALUE )
				{
					pControl->SetWindowText( m_strMultipleString );
				}
				else if ( ! strValue.IsEmpty() && ( strValue != NO_VALUE ) )
				{
					pControl->SetWindowText( strValue );
				}
			}
		}
	}
	return nModified;
}

/////////////////////////////////////////////////////////////////////////////
// CSchemaCtrl child control layout

void CSchemaCtrl::Layout()
{
	CRect rcClient, rcNew;
	
	GetClientRect( &rcClient );

	SCROLLINFO pScroll = {};
	pScroll.cbSize	= sizeof(pScroll);
	pScroll.fMask	= SIF_PAGE|SIF_POS|SIF_RANGE;
	pScroll.nPage	= rcClient.Height();
	pScroll.nPos	= m_nScroll;

	HDWP hDWP = BeginDeferWindowPos( static_cast< int >( m_pControls.GetSize() ) );

	int nTop = -m_nScroll;

	for ( INT_PTR nControl = 0 ; nControl < m_pControls.GetSize() ; nControl++ )
	{
		CWnd* pControl	= m_pControls.GetAt( nControl );

		if ( m_nCaptionWidth )
		{
			if ( Settings.General.LanguageRTL )
			{
				rcNew.left		= m_nCaptionWidth;
				rcNew.right		= rcClient.right - 10;
			}
			else
			{
				rcNew.left		= m_nCaptionWidth;
				rcNew.right		= rcClient.right - 10;
			}
			rcNew.top		= nTop + m_nItemHeight / 2 - 9;
			rcNew.bottom	= nTop + m_nItemHeight / 2 + 9;
		}
		else
		{
			rcNew.left		= rcClient.left + 4;
			rcNew.right		= rcClient.right - 4;
			rcNew.top		= nTop + m_nItemHeight - 18 - 4;
			rcNew.bottom	= nTop + m_nItemHeight - 4;
		}
		
		if ( pControl->IsKindOf( RUNTIME_CLASS( CComboBox ) ) )
		{
			rcNew.top --;
			rcNew.bottom += 128;
		}
		
		hDWP = DeferWindowPos( hDWP, pControl->GetSafeHwnd(), NULL, rcNew.left, rcNew.top,
			rcNew.Width(), rcNew.Height(),
			SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
		
		pScroll.nMax += m_nItemHeight;
		nTop += m_nItemHeight;
	}

	EndDeferWindowPos( hDWP );

	pScroll.nMax--;
	SetScrollInfo( SB_VERT, &pScroll );
}

/////////////////////////////////////////////////////////////////////////////
// CSchemaCtrl disable

void CSchemaCtrl::Disable()
{
	if ( m_pSchema == NULL ) return;
	
	POSITION pos = m_pSchema->GetMemberIterator();
	
	for ( INT_PTR nControl = 0 ; nControl < m_pControls.GetSize() && pos ; nControl++ )
	{
		m_pControls.GetAt( nControl )->EnableWindow( FALSE );
	}
	
	return;
}


/////////////////////////////////////////////////////////////////////////////
// CSchemaCtrl enable

void CSchemaCtrl::Enable()
{
	if ( m_pSchema == NULL ) return;
	
	POSITION pos = m_pSchema->GetMemberIterator();
	
	for ( INT_PTR nControl = 0 ; nControl < m_pControls.GetSize() && pos ; nControl++ )
	{
		m_pControls.GetAt( nControl )->EnableWindow( TRUE );
	}
	
	return;
}


/////////////////////////////////////////////////////////////////////////////
// CSchemaCtrl scrolling

void CSchemaCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/) 
{
	SCROLLINFO pScroll = {};

	pScroll.cbSize	= sizeof(pScroll);
	pScroll.fMask	= SIF_ALL;

	GetScrollInfo( SB_VERT, &pScroll );

	switch ( nSBCode )
	{
	case SB_TOP:
		m_nScroll = 0;
		break;
	case SB_BOTTOM:
		m_nScroll = pScroll.nMax - 1;
		break;
	case SB_LINEUP:
		m_nScroll -= 8;
		break;
	case SB_LINEDOWN:
		m_nScroll += 8;
		break;
	case SB_PAGEUP:
		m_nScroll -= pScroll.nPage;
		break;
	case SB_PAGEDOWN:
		m_nScroll += pScroll.nPage;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		m_nScroll = nPos;
		break;
	}

	int nDelta = m_nScroll - pScroll.nPos;
	m_nScroll = pScroll.nPos;

	ScrollBy( nDelta );
}

void CSchemaCtrl::ScrollBy(int nDelta)
{
	int nBefore = m_nScroll;
	
	m_nScroll += nDelta;
	m_nScroll = max( 0, min( GetScrollLimit( SB_VERT ), m_nScroll ) );
	nDelta = m_nScroll - nBefore;
	
	for ( CWnd* pWnd = GetWindow( GW_CHILD ) ; pWnd ; pWnd = pWnd->GetNextWindow() )
	{
		pWnd->ModifyStyle( WS_VISIBLE, 0 );
	}
	
	ScrollWindowEx( 0, -nDelta, NULL, NULL, NULL, NULL, SW_SCROLLCHILDREN|SW_INVALIDATE );
	Layout();
	UpdateWindow();
}

void CSchemaCtrl::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	SetFocus();
}

BOOL CSchemaCtrl::OnMouseWheel(UINT /*nFlags*/, short zDelta, CPoint /*pt*/)
{
	OnVScroll( SB_THUMBPOSITION, (int)( GetScrollPos( SB_VERT ) - 
		zDelta / WHEEL_DELTA * m_nScrollWheelLines * 8 ), NULL );
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CSchemaCtrl painting

BOOL CSchemaCtrl::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}

void CSchemaCtrl::OnNcPaint() 
{
	CWnd::OnNcPaint();

	if ( m_bShowBorder )
	{
		CWindowDC dc( this );
		CRect rc;

		COLORREF crBorder = CoolInterface.m_crSysActiveCaption ;

		GetWindowRect( &rc );
		rc.OffsetRect( -rc.left, -rc.top );
		dc.Draw3dRect( &rc, crBorder, crBorder );
	}
}

void CSchemaCtrl::OnPaint() 
{
	CRect rcClient, rcItem;
	CPaintDC dc( this );

	GetClientRect( &rcClient );
	rcItem.CopyRect( &rcClient );

	rcItem.bottom = rcItem.top + m_nItemHeight;
	rcItem.OffsetRect( 0, -m_nScroll );
	
	CFont* pOldFont = (CFont*)dc.SelectObject( &theApp.m_gdiFont );
	dc.SetBkMode( OPAQUE );

	int nOffset = m_nItemHeight;
	
	if ( ! m_nCaptionWidth ) nOffset -= 18 + 4;

	nOffset = nOffset / 2 - dc.GetTextExtent( _T("Xg") ).cy / 2 - 1;
	
	for ( INT_PTR nControl = 0 ; nControl < m_pControls.GetSize() ; nControl++ )
	{
		// dc.SetBkColor( nControl & 1 ? RGB( 240, 240, 255 ) : RGB( 255, 255, 255 ) );
		dc.SetBkColor( Skin.m_crSchemaRow[ nControl & 1 ] );
		
		dc.ExtTextOut( rcItem.left + 4, rcItem.top + nOffset, ETO_OPAQUE|ETO_CLIPPED,
			&rcItem, m_pCaptions.GetAt( nControl ), NULL );
		
		rcItem.OffsetRect( 0, m_nItemHeight );
	}

	if ( rcItem.top < rcClient.bottom )
	{
		rcItem.SetRect( rcClient.left, rcItem.top, rcClient.right, rcClient.bottom );
		dc.FillSolidRect( &rcItem, CoolInterface.m_crDropdownBox );
	}

	dc.SelectObject( pOldFont );
}

void CSchemaCtrl::OnControlSetFocus()
{
	CWnd* pFocus = GetFocus();
	for ( int i = 0; i < m_pControls.GetCount(); i++)
	{
		CWnd* pControl = m_pControls.GetAt( i );
		if ( pControl == pFocus || pControl == pFocus->GetParent() )
		{
			CRect rcClient, rcControl;
			GetClientRect( &rcClient );
			pFocus->GetWindowRect( &rcControl );
			ScreenToClient( &rcControl );
			if ( rcControl.top < rcClient.top )
			{
				ScrollBy( rcControl.top - rcClient.top - 8 );
			}
			else if ( rcControl.bottom > rcClient.bottom )
			{
				ScrollBy( rcControl.bottom - rcClient.bottom + 8 );
			}
			break;
		}
	}
}

void CSchemaCtrl::OnControlEdit()
{
	GetOwner()->SendMessage( WM_COMMAND, MAKELONG( GetDlgCtrlID(), EN_CHANGE ), (LPARAM)GetSafeHwnd() );
}
