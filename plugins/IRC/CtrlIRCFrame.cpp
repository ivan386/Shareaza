// CtrlIRCFrame.cpp : Implementation of CIRCFrame

#include "stdafx.h"
#include "CtrlIRCFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CIRCFrame
CIRCNewMessage::operator =(CIRCNewMessage &rhs)
{
	m_sTargetName	= rhs.m_sTargetName;
	nColorID		= rhs.nColorID;
	m_pMessages.Copy( rhs.m_pMessages );
	return TRUE;
}

#define SIZE_INTERNAL	1982
#define SIZE_BARSLIDE	1983
#define NEWLINE_FORMAT	_T("2")
#define DEST_PORT		6667
CIRCFrame* CIRCFrame::g_pIrcFrame = NULL;

/////////////////////////////////////////////////////////////////////////////
// CIRCFrame construction

CIRCFrame::CIRCFrame()
{
	if ( g_pIrcFrame == NULL ) g_pIrcFrame = this;
	m_nBufferCount			 = 0;
	m_nListWidth			= 170;
	m_nFloodingDelay		= 4000;
	m_nFloodLimit			= 0;
	m_nUpdateFrequency		= 40;
	m_nUpdateChanListFreq	= 100000;
	m_bConnected			= FALSE;
	m_nLocalTextLimit		= 300;
	m_nLocalLinesLimit		= 14;
	m_pszLineJoiner			= _T("\x200D");
	//m_pPanel				= NULL;
	// m_pTray				= ((CMainWnd*)AfxGetMainWnd())->m_pTray;
}

CIRCFrame::~CIRCFrame()
{
	if ( g_pIrcFrame == this ) g_pIrcFrame = NULL;
	//m_pPanel->DestroyWindow();
	if ( IsWindow() )
		Detach();
}

BOOL CIRCFrame::Initialize(CIRCPlugin* pPlugin, HWND hParent)
{
	__super::Initialize( pPlugin, NULL, hParent );

	m_pWindow->ListenForSingleMessage( WM_CREATE );
	m_pWindow->ListenForSingleMessage( WM_CONTEXTMENU );
	m_pWindow->ListenForSingleMessage( WM_SIZE );
	m_pWindow->ListenForSingleMessage( WM_PAINT );
	m_pWindow->ListenForSingleMessage( WM_NCLBUTTONUP );
	m_pWindow->ListenForSingleMessage( WM_SETCURSOR );

	if ( SUCCEEDED( m_pWindow->CreateFrame( m_pPlugin->m_nCmdWindow, hParent ) ) )
	{
		m_pWindow->GetHwnd( &m_hWnd );
		return TRUE;
	}

	return FALSE;
}

void CIRCFrame::OnSkinChanged()
{
	//m_pPanel->Setup();
}

HRESULT CIRCFrame::OnTranslate(MSG* pMessage)
{
	return E_NOTIMPL;
}

HRESULT CIRCFrame::OnMessage(INT nMessage, WPARAM wParam, LPARAM lParam, LRESULT* plResult)
{
	if ( (UINT)nMessage == WM_CREATE )
		m_pWindow->GetHwnd( &m_hWnd );
	if ( m_hWnd == NULL ) return S_FALSE;

	return ProcessWindowMessage( m_hWnd, nMessage, wParam, lParam, *plResult ) ? S_OK : S_FALSE;
}

HRESULT CIRCFrame::OnUpdate(INT nCommandID, STRISTATE* pbVisible, STRISTATE* pbEnabled, STRISTATE* pbChecked)
{
	return E_NOTIMPL;
}

HRESULT CIRCFrame::OnCommand(INT nCommandID)
{
	if ( nCommandID == m_pPlugin->m_nCmdWindow )
	{
		ShowWindow( SW_SHOWNORMAL );
		return S_OK;
	}
	return S_FALSE;
}

HRESULT CIRCFrame::GetWndClassName(BSTR* pszClassName)
{
	return __super::GetWndClassName( pszClassName );
}

LRESULT CIRCFrame::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//m_pPanel = new CComObject< CIRCPanel >;
	//if ( !m_pPanel->Create( m_pPlugin, L"CIRCPanel" ) ) return -1;
	m_wndTab.Create( m_hWnd, 0, 0, WS_CHILD | WS_VISIBLE | TCS_FLATBUTTONS /*| TCS_OWNERDRAWFIXED*/ );
	ShowWindow( SW_SHOWNORMAL );
	return 0;
}

LRESULT CIRCFrame::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	bHandled = TRUE;
	RECT rc;
	GetClientRect( &rc );
	if ( rc.right - rc.left < 32 || rc.bottom - rc.top < 32 ) return 0;

	//m_pPanel->SetWindowPos( NULL, 0, 0, PANEL_WIDTH, rc.bottom - rc.top, SWP_NOZORDER );
	rc.bottom -= TOOLBAR_HEIGHT;
	rc.top    += IRCHEADER_HEIGHT;
	rc.left   += PANEL_WIDTH;

	//m_wndMainBar.SetWindowPos( NULL, rc.left, rc.bottom, rc.right - rc.left,
	//	TOOLBAR_HEIGHT, SWP_NOZORDER|SWP_SHOWWINDOW );

	m_wndTab.SetWindowPos( NULL, rc.left, rc.bottom - TABBAR_HEIGHT,
		rc.right - rc.left, TABBAR_HEIGHT, SWP_NOZORDER|SWP_SHOWWINDOW );

	return 0;
}

void CIRCTabCtrl::SetTabColor(int nItem, COLORREF cRGB)
{
	TC_ITEM tci = {};
	tci.mask = TCIF_PARAM;
	tci.lParam = cRGB;
	SetItem( nItem, &tci );
	RedrawWindow();
	PostMessage( WM_DRAWITEM );
}

COLORREF CIRCTabCtrl::GetTabColor(int nItem)
{
	TC_ITEM tci = {};
	tci.mask = TCIF_PARAM;
	GetItem( nItem, &tci );
	return (COLORREF)tci.lParam;
}

LRESULT CIRCTabCtrl::OnEraseBkgnd(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	/*Settings.IRC.Colors[ ID_COLOR_TABS ]*/;
	COLORREF cBorder = 15132390;
	HBRUSH hbr = CreateSolidBrush( cBorder );

	HDC hdc = (HDC)wParam;
	RECT rect;
	GetWindowRect( &rect );
	ScreenToClient( &rect );
	SetBkMode( hdc, OPAQUE );
	SetBkColor( hdc, cBorder );
	FillRect( hdc, &rect, hbr );

	bHandled = TRUE;
	return 0;
}

LRESULT CIRCTabCtrl::OnDrawItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT)lParam;

	if ( lpDrawItem->CtlType == ODT_TAB )
	{
		RECT rect = lpDrawItem->rcItem;
		int nTabIndex = lpDrawItem->itemID;
		/*Settings.IRC.Colors[ ID_COLOR_TABS ]*/;
		COLORREF cBorder = 15132390;
		HBRUSH hbr = CreateSolidBrush( cBorder );
		HDC hOldDC = ::GetDC(NULL);
		HDC hDC = lpDrawItem->hDC;

		lpDrawItem->itemAction = ODA_DRAWENTIRE;
		SetBkMode( hDC, OPAQUE );
		SetBkColor( hDC, cBorder );
		FillRect( hDC, &rect, hbr );
		SetTextColor( hDC, GetTabColor( nTabIndex ) );

		TC_ITEM tci ={};
		TCHAR pszBuffer[ 40 ] = {0};
		tci.mask = TCIF_TEXT|TCIF_IMAGE;
		tci.pszText = pszBuffer;
		tci.cchTextMax = 39;
		if ( !GetItem( nTabIndex, &tci ) ) return 0;

		DrawText( hDC, pszBuffer, -1, &rect, DT_SINGLELINE | DT_VCENTER | DT_CENTER );
		lpDrawItem->hDC = hOldDC;
		bHandled = TRUE;
	}

	return 0;
}

LRESULT CIRCFrame::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	RECT rcClient;
	PAINTSTRUCT ps;

	// Get the client rectangle, and begin painting
	GetClientRect( &rcClient );
	HDC hDC = BeginPaint( &ps );

	::SetBkMode( hDC, OPAQUE );
	::SetBkColor( hDC, RGB(200,200,200) );

	CComBSTR bsText( L"Empty frame area..." );
	::ExtTextOut( hDC, 6, 6, ETO_OPAQUE, &rcClient, bsText, bsText.Length(), NULL );

	// Finish painting
	EndPaint( &ps );
	return 0;
}
