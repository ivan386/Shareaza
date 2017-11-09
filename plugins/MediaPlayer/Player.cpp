//
// Player.cpp : Implementation of CPlayer
//
// Copyright (c) Nikolay Raspopov, 2009-2012.
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

#include "stdafx.h"
#include "Player.h"

#ifdef _WMP

#define TIME_FACTOR 10000000.	// 100 ns to 1 s

#else

#ifdef _DEBUG

void Dump(IGraphBuilder* pGraph)
{
	ATLTRACE( _T("\nFilter graph:\n") );

	CComPtr< IEnumFilters > pFilters;
	HRESULT hr = pGraph->EnumFilters( &pFilters );
	if ( SUCCEEDED( hr ) )
	{
		for ( int i = 1;; ++i )
		{
			CComPtr< IBaseFilter > pFilter;
			ULONG nGot = 0;
			hr = pFilters->Next( 1, &pFilter, &nGot );
			if ( hr != S_OK )
				break;

			FILTER_INFO fi = {};
			hr = pFilter->QueryFilterInfo( &fi );

			ATLTRACE( _T("%2d. \"%ls\" %ls\n"), i, fi.achName,
				( fi.pGraph ? _T("CONNECTED") : _T("DISCONNECTED") ) );

			CComPtr< IEnumPins > pPins;
			hr = pFilter->EnumPins( &pPins );
			if ( SUCCEEDED( hr ) )
			{
				for ( int j = 1;; ++j )
				{
					CComPtr< IPin > pPin;
					hr = pPins->Next( 1, &pPin, &nGot );
					if ( hr != S_OK )
						break;

					PIN_INFO pi = {};
					hr = pPin->QueryPinInfo( &pi );

					CString sType( _T("-") );
					CComPtr< IEnumMediaTypes > pTypes;
					hr = pPin->EnumMediaTypes( &pTypes );
					if ( SUCCEEDED( hr ) )
					{
						AM_MEDIA_TYPE* pmt = NULL;
						hr = pTypes->Next( 1, &pmt, &nGot );
						if ( hr == S_OK )
						{
							if ( pmt->majortype == MEDIATYPE_Audio )
								sType = _T("Audio");
							else if ( pmt->majortype == MEDIATYPE_Video )
								sType = _T("Video");
							else
								sType = _T("Unknown");

							if ( pmt->pbFormat )
							{
								CoTaskMemFree( pmt->pbFormat );
								pmt->cbFormat = 0;
								pmt->pbFormat = NULL;
							}
							if ( pmt->pUnk )
							{
								pmt->pUnk->Release();
								pmt->pUnk = NULL;
							}
							CoTaskMemFree( pmt );
						}
					}

					ATLTRACE( _T("\t[%d] \"%ls\" %ls %ls %ls\n"), j,
						pi.achName, sType,
						( ( pi.dir == PINDIR_INPUT ) ? _T("IN") : _T("OUT") ),
						( pi.pFilter ? _T("CONNECTED") : _T("DISCONNECTED") ) );
				}
			}
		}
	}

	ATLTRACE( _T("\n") );
}
#else

#define Dump(x) __noop;

#endif

// CPlayerWindow

CPlayerWindow::CPlayerWindow()
{
}

LRESULT CPlayerWindow::OnErase(UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	bHandled = TRUE;
	return 0;   
}

LRESULT CPlayerWindow::OnPaint(UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	RECT rc;
	GetClientRect( &rc );

	HDC hDC = GetWindowDC();
	
	FillRect( hDC, &rc, (HBRUSH)GetStockObject( BLACK_BRUSH ) );

	ReleaseDC( hDC );

	ValidateRect( &rc );

	bHandled = TRUE;
	return 0;   
}

#endif // _WMP

// CPlayer

CPlayer::CPlayer()
	: m_hwndOwner( NULL )
	, m_rcWindow()
	, m_nZoom( smaDefault )
	, m_dAspect( 1.0 )
	, m_dVolume( 1.0 )
	, m_dSpeed( 1.0 )
	, m_nVisSize( 0 )
#ifdef _WMP
#else
	, m_bAudioOnly( FALSE )
#endif
{
}

HRESULT CPlayer::FinalConstruct()
{
	return S_OK;
}

void CPlayer::FinalRelease()
{
	Destroy();
}

STDMETHODIMP CPlayer::Create(
	/* [in] */ LONG_PTR hWnd)
{
	if ( ! hWnd )
		return E_INVALIDARG;

	m_hwndOwner = hWnd;

	return S_OK;
}

STDMETHODIMP CPlayer::Destroy(void)
{
	Close();

	return S_OK;
}

STDMETHODIMP CPlayer::Reposition(
	/* [in] */ long Left,
	/* [in] */ long Top,
	/* [in] */ long Width,
	/* [in] */ long Height)
{
	m_rcWindow.left = Left;
	m_rcWindow.top = Top;
	m_rcWindow.right = Left + Width;
	m_rcWindow.bottom = Top + Height;

	if ( ! m_pPlayer )
		return S_OK;

#ifdef _WMP

	if ( m_wndPlayer.IsWindow() )
		m_wndPlayer.SetWindowPos( NULL, &m_rcWindow,
			SWP_ASYNCWINDOWPOS | SWP_NOZORDER | SWP_NOACTIVATE );

#else

	if ( m_bAudioOnly )
	{
		if ( m_wndPlayer.IsWindow() )
			m_wndPlayer.SetWindowPos( NULL, &m_rcWindow,
				SWP_ASYNCWINDOWPOS | SWP_NOZORDER | SWP_NOACTIVATE );
	}
	else
	{	
		CComQIPtr< IVideoWindow > pWindow( m_pPlayer );
		if ( ! pWindow )
			return E_NOINTERFACE;

		pWindow->SetWindowPosition( m_rcWindow.left, m_rcWindow.top,
			m_rcWindow.right - m_rcWindow.left, m_rcWindow.bottom - m_rcWindow.top );
	}

#endif

	AdjustVideoPosAndZoom();

	return S_OK;
}

STDMETHODIMP CPlayer::GetVolume(
	/* [out] */ DOUBLE *pnVolume)
{
	HRESULT hr;

	if ( ! pnVolume )
		return E_POINTER;

	*pnVolume = m_dVolume;

	if ( ! m_pPlayer )
		return S_OK;

	long lVolume;

#ifdef _WMP

	CComPtr< IWMPSettings > pSettings;
	hr = m_pPlayer->get_settings( &pSettings );
	if ( FAILED( hr ) )
		return hr;

	hr = pSettings->get_volume( &lVolume );
	if ( FAILED( hr ) )
		return hr;

	// 0 ... 100 -> 0.0 ... 1.0 Conversion
	*pnVolume = lVolume / 100.;

#else

	// TODO: Use IAudioClient under Windows Vista

	CComQIPtr< IBasicAudio > pAudio( m_pPlayer );
	if ( ! pAudio )
		return E_NOINTERFACE;

	hr = pAudio->get_Volume( &lVolume );
	if ( FAILED( hr ) )
		return hr;

	// -6,000 ... 0 -> 0.0 ... 1.0 Conversion
	if ( lVolume < -6000. )
		*pnVolume = 0;
	else 
		*pnVolume = ( lVolume + 6000. ) / 6000.;

#endif

	return S_OK;
}

STDMETHODIMP CPlayer::SetVolume(
	/* [in] */ DOUBLE dVolume)
{
	HRESULT hr;

	m_dVolume = dVolume;

	if ( ! m_pPlayer )
		return S_OK;

	// TODO: Use IAudioClient under Windows Vista

#ifdef _WMP

	CComPtr< IWMPSettings > pSettings;
	hr = m_pPlayer->get_settings( &pSettings );
	if ( FAILED( hr ) )
		return hr;

	// 0.0 ... 1.0 -> 0 ... 100 Conversion
	hr = pSettings->put_volume( (long)( dVolume * 100 ) );
	if ( FAILED( hr ) )
		return hr;

#else

	CComQIPtr< IBasicAudio > pAudio( m_pPlayer );
	if ( ! pAudio )
		return E_NOINTERFACE;

	// 0.0 ... 1.0 -> -6,000 ... 0 Conversion
	hr = pAudio->put_Volume( (long)( ( dVolume * 6000. ) - 6000. ) );
	if ( FAILED( hr ) )
		return hr;

#endif

	return S_OK;
}

STDMETHODIMP CPlayer::GetZoom(
	/* [out] */ MediaZoom *pnZoom)
{
	if ( ! pnZoom )
		return E_POINTER;

	*pnZoom = m_nZoom;

	return S_OK;
}

STDMETHODIMP CPlayer::SetZoom(
	/* [in] */ MediaZoom nZoom)
{
	m_nZoom = nZoom;

	if ( ! m_pPlayer )
		return S_OK;

#ifdef _WMP

	switch ( nZoom )
	{
	case smzDistort:
	case smzFill:
		m_pPlayer->put_stretchToFit( VARIANT_TRUE );
		break;

	case smzOne:
	case smzDouble:
		m_pPlayer->put_stretchToFit( VARIANT_FALSE );
		break;
	}

#else

	AdjustVideoPosAndZoom();

#endif

	return S_OK;
}

STDMETHODIMP CPlayer::GetAspect(
	/* [out] */ DOUBLE *pdAspect)
{
	if ( ! pdAspect )
		return E_POINTER;

	*pdAspect = m_dAspect;

	return S_OK;
}

STDMETHODIMP CPlayer::SetAspect(
	/* [in] */ DOUBLE dAspect)
{
	m_dAspect = dAspect;

	AdjustVideoPosAndZoom();

	return S_OK;
}

#ifdef _WMP
static STDMETHODIMP SafeRenderFile(IWMPPlayer2* pPlayer, BSTR sFilename) throw()
#else
static STDMETHODIMP SafeRenderFile(IGraphBuilder* pPlayer, BSTR sFilename) throw()
#endif
{
	__try
	{
#ifdef _WMP
		return pPlayer->put_URL( sFilename );
#else
		return pPlayer->RenderFile( sFilename, NULL );
#endif
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		return E_FAIL;
	}
}

STDMETHODIMP CPlayer::Open(
	/* [in] */ BSTR sFilename)
{
	HRESULT hr;

	if ( ! sFilename )
		return E_POINTER;

	if ( m_pPlayer )
		return E_INVALIDARG;

#ifdef _WMP

	AtlAxWinInit();

	if ( ! m_wndPlayer.IsWindow() )
	{
		if ( ! m_wndPlayer.Create( (HWND)m_hwndOwner, m_rcWindow, NULL,
			WS_CHILD | WS_DISABLED | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
			WS_EX_CONTROLPARENT | WS_EX_TOPMOST ) )
			return E_FAIL;
	}

	CComPtr< IAxWinHostWindow > pHost;
	hr = m_wndPlayer.QueryHost( &pHost );
	if ( FAILED( hr ) )
		return hr;

	hr = pHost->CreateControl( CComBSTR(
		_T("{6BF52A52-394A-11d3-B153-00C04F79FAA6}") ), m_wndPlayer, 0 );
	if ( FAILED( hr ) )
		return hr;

	CComPtr< IWMPPlayer > pPlayer1;
	hr = m_wndPlayer.QueryControl( &pPlayer1 );
	if ( FAILED( hr ) )
		return hr;

	m_pPlayer = pPlayer1;
	if ( ! m_pPlayer )
		return E_NOINTERFACE;

	hr = m_pPlayer->put_uiMode( CComBSTR( _T("none") ) );
	hr = m_pPlayer->put_enableContextMenu( VARIANT_FALSE );
	hr = m_pPlayer->put_enabled( VARIANT_FALSE );
	hr = m_pPlayer->put_fullScreen( VARIANT_FALSE );

	CComPtr< IWMPSettings > pSettings;
	hr = m_pPlayer->get_settings( &pSettings );
	if ( FAILED( hr ) )
		return hr;

	hr = pSettings->put_autoStart( VARIANT_FALSE );
	hr = pSettings->put_balance( 0 );
	hr = pSettings->put_enableErrorDialogs( VARIANT_FALSE );
	hr = pSettings->put_invokeURLs( VARIANT_FALSE );
	hr = pSettings->put_mute( VARIANT_FALSE );
	hr = pSettings->put_playCount( 1 );
	hr = pSettings->setMode( CComBSTR( _T("autoRewind") ), VARIANT_FALSE );
	hr = pSettings->setMode( CComBSTR( _T("loop") ), VARIANT_FALSE );
	hr = pSettings->setMode( CComBSTR( _T("showFrame") ), VARIANT_TRUE );
	hr = pSettings->setMode( CComBSTR( _T("shuffle") ), VARIANT_FALSE );

	hr = SafeRenderFile( m_pPlayer, sFilename );
	if ( FAILED( hr ) )
		return hr;

	m_wndPlayer.ShowWindow( SW_SHOW );

#else

	hr = m_pPlayer.CoCreateInstance( CLSID_FilterGraph );
	if ( FAILED( hr ) )
		return hr;

	hr = SafeRenderFile( m_pPlayer, sFilename );
	if ( FAILED( hr ) )
		return hr;

	CComQIPtr< IVideoWindow > pWindow( m_pPlayer );
	long lVisible = 0;
	m_bAudioOnly = ( ! pWindow || pWindow->get_Visible( &lVisible ) == E_NOINTERFACE );

	if ( m_bAudioOnly )
	{
		if ( ! m_wndPlayer.IsWindow() )
		{
			m_wndPlayer.Create( (HWND)m_hwndOwner, m_rcWindow, NULL,
				WS_CHILD | WS_VISIBLE | WS_DISABLED | WS_CLIPSIBLINGS |
				WS_CLIPCHILDREN, WS_EX_CONTROLPARENT | WS_EX_TOPMOST );
		}
	}
	else
	{
		hr = pWindow->put_WindowStyle( WS_CHILD | WS_VISIBLE |
			WS_CLIPSIBLINGS | WS_CLIPCHILDREN );
		hr = pWindow->put_WindowStyleEx( WS_EX_CONTROLPARENT | WS_EX_TOPMOST );
		hr = pWindow->put_Owner( m_hwndOwner );
		hr = pWindow->put_MessageDrain( m_hwndOwner );
	}

	Dump( m_pPlayer );

#endif

	return S_OK;
}

STDMETHODIMP CPlayer::Close(void)
{
	Stop();

#ifdef _WMP

	// Detach Windows Media Player control
	CComPtr< IAxWinHostWindow > pHost;
	if ( SUCCEEDED( m_wndPlayer.QueryHost( &pHost ) ) )
	{
		pHost->AttachControl( NULL, NULL );
		pHost.Release();
	}

#else

	if ( m_pPlayer )
		m_pPlayer->Abort();

#endif

	// Destroy window
	if ( m_wndPlayer.IsWindow() )
		m_wndPlayer.DestroyWindow();
	m_wndPlayer.m_hWnd = NULL;

	m_pPlayer.Release();

	return S_OK;
}

STDMETHODIMP CPlayer::Play(void)
{
	HRESULT hr;

	if ( ! m_pPlayer )
		return E_INVALIDARG;

#ifdef _WMP

	m_wndPlayer.SetWindowPos( NULL, &m_rcWindow,
		SWP_ASYNCWINDOWPOS | SWP_NOZORDER | SWP_SHOWWINDOW );

	CComPtr< IWMPControls > pControls;
	hr = m_pPlayer->get_controls( &pControls );
	if ( FAILED( hr ) )
		return hr;

	VARIANT_BOOL bIsAvailable = VARIANT_FALSE;
	hr = pControls->get_isAvailable( CComBSTR( L"play" ), &bIsAvailable );
	if ( FAILED( hr ) )
		return hr;

	if ( ! bIsAvailable )
		return E_NOTIMPL;

	hr = pControls->play();
	if ( hr != S_OK )
		return E_FAIL;

#else

	if ( m_bAudioOnly )
	{
		if ( m_wndPlayer.IsWindow() )
			m_wndPlayer.SetWindowPos( NULL, &m_rcWindow,
				SWP_ASYNCWINDOWPOS | SWP_NOZORDER | SWP_SHOWWINDOW );
	}
	else
	{
		CComQIPtr< IVideoWindow > pWindow( m_pPlayer );
		if ( ! pWindow )
			return E_NOINTERFACE;

		pWindow->SetWindowPosition( m_rcWindow.left, m_rcWindow.top,
			m_rcWindow.right - m_rcWindow.left, m_rcWindow.bottom - m_rcWindow.top );
	}

	CComQIPtr< IMediaControl > pControl( m_pPlayer );
	if ( ! pControl )
		return E_NOINTERFACE;
		
	hr = pControl->Run();
	if ( FAILED( hr ) )
		return hr;

#endif

	// Restore volume level
	SetVolume( m_dVolume );

	// Restore speed
	SetSpeed( m_dSpeed );

	// Restore zoom and aspect ratio
	AdjustVideoPosAndZoom();

	return S_OK;
}

STDMETHODIMP CPlayer::Pause(void)
{
	HRESULT hr;

	if ( ! m_pPlayer )
		return E_INVALIDARG;

#ifdef _WMP

	CComPtr< IWMPControls > pControls;
	hr = m_pPlayer->get_controls( &pControls );
	if ( FAILED( hr ) )
		return hr;

	hr = pControls->pause();
	if ( FAILED( hr ) )
		return hr;

#else

	CComQIPtr< IMediaControl > pControl( m_pPlayer );
	if ( ! pControl )
		return E_NOINTERFACE;

	hr = pControl->Pause();
	if ( FAILED( hr ) )
		return hr;

#endif

	return S_OK;
}

STDMETHODIMP CPlayer::Stop(void)
{
	HRESULT hr;
	
	if ( ! m_pPlayer )
		return E_INVALIDARG;

#ifdef _WMP

	if ( m_wndPlayer.IsWindow() )
		m_wndPlayer.ShowWindow( SW_HIDE );

	CComPtr< IWMPControls > pControls;
	hr = m_pPlayer->get_controls( &pControls );
	if ( FAILED( hr ) )
		return hr;

	hr = pControls->stop();
	if ( FAILED( hr ) )
		return hr;

#else

	if ( m_bAudioOnly )
	{
		if ( m_wndPlayer.IsWindow() )
			m_wndPlayer.ShowWindow( SW_HIDE );
	}
	else
	{
		CComQIPtr< IVideoWindow > pWindow( m_pPlayer );
		if ( ! pWindow )
			return E_NOINTERFACE;

		pWindow->put_Visible( OAFALSE );
	}

	CComQIPtr< IMediaControl > pControl( m_pPlayer );
	if ( ! pControl )
		return E_NOINTERFACE;

	hr = pControl->Stop();
	if ( FAILED( hr ) )
		return hr;

#endif

	return S_OK;
}

STDMETHODIMP CPlayer::GetState(
	/* [out] */ MediaState *pnState)
{
	HRESULT hr;

	if ( ! pnState )
		return E_POINTER;

	*pnState = smsNull;

	if ( ! m_pPlayer )
		return S_OK;

#ifdef _WMP

	WMPPlayState state;
	hr = m_pPlayer->get_playState( &state );
	if ( FAILED( hr ) )
		return hr;

	switch ( state )
	{
	case wmppsUndefined:		// Windows Media Player is in an undefined state
	case wmppsMediaEnded:		// The end of the media item has been reached
	case wmppsTransitioning:	// Preparing new media item
	case wmppsReady:			// Ready to begin playing
	case wmppsReconnecting:		// Trying to reconnect for streaming data
	case wmppsStopped:			// Playback is stopped
  		*pnState = smsOpen;
		break;
	case wmppsPlaying:			// Stream is playing
	case wmppsScanForward:		// Stream is scanning forward
	case wmppsScanReverse:		// Stream is scanning in reverse
	case wmppsBuffering:		// Stream is being buffered
	case wmppsWaiting:			// Stream is being buffered
		*pnState = smsPlaying;
		break;
	case wmppsPaused:			// Playback is paused
		*pnState = smsPaused;
		break;
	}

#else

	CComQIPtr< IMediaControl > pControl( m_pPlayer );
	if ( ! pControl )
		return E_NOINTERFACE;

	OAFilterState st;
	hr = pControl->GetState( 250, &st );
	if ( FAILED( hr ) )
		return hr;

	switch ( st )
	{
	case State_Stopped:
		*pnState = smsOpen;
		break;
	case State_Paused:
		*pnState = smsPaused;
		break;
	case State_Running:
		*pnState = smsPlaying;
		break;
	}

#endif

	return S_OK;
}

STDMETHODIMP CPlayer::GetLength(
	/* [out] */ LONGLONG *pnLength)
{
	HRESULT hr;

	if ( ! pnLength )
		return E_POINTER;

	*pnLength = 0;

	if ( ! m_pPlayer )
		return S_OK;

#ifdef _WMP

	CComPtr< IWMPMedia > pMedia;
	hr = m_pPlayer->get_currentMedia( &pMedia );
	if ( FAILED( hr ) )
		return hr;

	double dDuration;
	hr = pMedia->get_duration( &dDuration );
	if ( FAILED( hr ) )
		return hr;

	*pnLength = (LONGLONG)( dDuration * TIME_FACTOR );

#else

	CComQIPtr< IMediaSeeking > pSeek( m_pPlayer );
	if ( ! pSeek )
		return E_NOINTERFACE;

	hr = pSeek->GetDuration( pnLength );
	if ( FAILED( hr ) )
		return hr;

#endif

	return S_OK;
}

STDMETHODIMP CPlayer::GetPosition(
	/* [out] */ LONGLONG *pnPosition)
{
	HRESULT hr;

	if ( ! pnPosition )
		return E_POINTER;

	*pnPosition = 0;

	if ( ! m_pPlayer )
		return S_OK;

#ifdef _WMP

	CComPtr< IWMPControls > pControls;
	hr = m_pPlayer->get_controls( &pControls );
	if ( FAILED( hr ) )
		return hr;

	double dPosition;
	hr = pControls->get_currentPosition( &dPosition );
	if ( FAILED( hr ) )
		return hr;

	*pnPosition = (LONGLONG)( dPosition * TIME_FACTOR );

#else

	CComQIPtr< IMediaSeeking > pSeek( m_pPlayer );
	if ( ! pSeek )
		return E_NOINTERFACE;

	hr = pSeek->GetCurrentPosition( pnPosition );
	if ( FAILED( hr ) )
		return hr;

#endif

	return S_OK;
}

STDMETHODIMP CPlayer::SetPosition(
	/* [in] */ LONGLONG nPosition)
{
	HRESULT hr;

	if ( ! m_pPlayer )
		return E_INVALIDARG;

#ifdef _WMP

	CComPtr< IWMPControls > pControls;
	hr = m_pPlayer->get_controls( &pControls );
	if ( FAILED( hr ) )
		return hr;

	double dPosition = (double) nPosition / TIME_FACTOR ;
	hr = pControls->put_currentPosition( dPosition );
	if ( FAILED( hr ) )
		return hr;

#else

	CComQIPtr< IMediaSeeking > pSeek( m_pPlayer );
	if ( ! pSeek )
		return E_NOINTERFACE;

	hr = pSeek->SetPositions( &nPosition, AM_SEEKING_AbsolutePositioning,
		NULL, AM_SEEKING_NoPositioning );
	if ( FAILED( hr ) )
		return hr;

#endif

	return S_OK;
}

STDMETHODIMP CPlayer::GetSpeed(
	/* [out] */ DOUBLE *pnSpeed)
{
	HRESULT hr;

	if ( ! pnSpeed )
		return E_POINTER;

	*pnSpeed = m_dSpeed;

	if ( ! m_pPlayer )
		return S_OK;

#ifdef _WMP

	CComPtr< IWMPSettings > pSettings;
	hr = m_pPlayer->get_settings( &pSettings );
	if ( FAILED( hr ) )
		return hr;

	hr = pSettings->get_rate( pnSpeed );
	if ( FAILED( hr ) )
		return hr;

#else

	CComQIPtr< IMediaSeeking > pSeek( m_pPlayer );
	if ( ! pSeek )
		return E_NOINTERFACE;

	hr = pSeek->GetRate( pnSpeed );
	if ( FAILED( hr ) )
		return hr;

#endif

	return S_OK;
}

STDMETHODIMP CPlayer::SetSpeed(
	/* [in] */ DOUBLE nSpeed)
{
	HRESULT hr;

	m_dSpeed = nSpeed;

	if ( ! m_pPlayer )
		return S_OK;

#ifdef _WMP

	CComPtr< IWMPSettings > pSettings;
	hr = m_pPlayer->get_settings( &pSettings );
	if ( FAILED( hr ) )
		return hr;

	hr = pSettings->put_rate( nSpeed );
	if ( FAILED( hr ) )
		return hr;

#else

	CComQIPtr< IMediaSeeking > pSeek( m_pPlayer );
	if ( ! pSeek )
		return E_NOINTERFACE;

	hr = pSeek->SetRate( nSpeed );
	if ( FAILED( hr ) )
		return hr;

#endif

	return S_OK;
}

STDMETHODIMP CPlayer::GetPlugin(
	/* [out] */ IAudioVisPlugin **ppPlugin)
{
	if ( ! ppPlugin )
		return E_POINTER;

	*ppPlugin = m_pAudioVisPlugin;

	return S_OK;
}

STDMETHODIMP CPlayer::SetPlugin(
	/* [in] */ IAudioVisPlugin *pPlugin)
{
	m_pAudioVisPlugin = pPlugin;

	return S_OK;
}

STDMETHODIMP CPlayer::GetPluginSize(
	/* [out] */ LONG *pnSize)
{
	if ( ! pnSize )
		return E_POINTER;

	*pnSize = m_nVisSize;

	return S_OK;
}

STDMETHODIMP CPlayer::SetPluginSize(
	/* [in] */ LONG nSize)
{
	m_nVisSize = nSize;

	return S_OK;
}

STDMETHODIMP CPlayer::IsWindowVisible(
	/* [out] */ VARIANT_BOOL* pbVisible )
{
	if ( ! pbVisible )
		return E_POINTER;

#ifdef _WMP

	if ( m_wndPlayer.IsWindow() && m_wndPlayer.IsWindowVisible() )
		*pbVisible = VARIANT_TRUE;
	else
		*pbVisible = VARIANT_FALSE;

#else

	if ( m_bAudioOnly )
	{
		if ( m_wndPlayer.IsWindow() && m_wndPlayer.IsWindowVisible() )
			*pbVisible = VARIANT_TRUE;
		else
			*pbVisible = VARIANT_FALSE;
	}
	else
		*pbVisible = VARIANT_TRUE;

#endif

	return S_OK;
}

HRESULT CPlayer::AdjustVideoPosAndZoom()
{
	HRESULT hr;

	if ( ! m_pPlayer )
		return E_INVALIDARG;

	long VideoWidth, VideoHeight;

#ifdef _WMP

	CComPtr< IWMPMedia > pMedia;
	hr = m_pPlayer->get_currentMedia( &pMedia );
	if ( FAILED( hr ) )
		return hr;

	hr = pMedia->get_imageSourceWidth( &VideoWidth );
	if ( FAILED( hr ) )
		return hr;

	hr = pMedia->get_imageSourceHeight( &VideoHeight );
	if ( FAILED( hr ) )
		return hr;

#else

	if ( m_bAudioOnly )
		return S_OK;

	CComQIPtr< IBasicVideo > pVideo( m_pPlayer );
	if ( ! pVideo )
		return E_NOINTERFACE;

	hr = pVideo->GetVideoSize( &VideoWidth, &VideoHeight );
	if ( FAILED( hr ) )
		return hr;

#endif

	long WindowWidth = m_rcWindow.right - m_rcWindow.left;
	long WindowHeight = m_rcWindow.bottom - m_rcWindow.top;

	if ( m_dAspect > 0 )
	{
		// Stretch video to match aspect ratio
		if ( (double)VideoWidth > m_dAspect * VideoHeight )
		{
			VideoHeight = (long)(VideoWidth / m_dAspect);
		}
		else
		{
			VideoWidth = (long)(VideoHeight * m_dAspect);
		}
	}

	if ( m_nZoom > 1 )
	{
		VideoWidth *= m_nZoom;
		VideoHeight *= m_nZoom;
	}
	else if ( m_nZoom == smzFill && VideoHeight && WindowHeight )
	{
		double VideoAspect = m_dAspect > 0 ? m_dAspect : (double)VideoWidth / VideoHeight;
		double WindowAspect = (double)WindowWidth / WindowHeight;

		if ( WindowAspect > VideoAspect )
		{
			VideoHeight = WindowHeight;
			VideoWidth = (long)(VideoHeight * VideoAspect);
		}
		else
		{
			VideoWidth = WindowWidth;
			VideoHeight = (long)(VideoWidth / VideoAspect);
		}
	}

	CRect tr;
	if ( m_nZoom == smzDistort )
	{
		tr.left = 0;
		tr.top = 0;
		tr.right = WindowWidth;
		tr.bottom = WindowHeight;
	}
	else
	{
		tr.left = this->m_rcWindow.left + (WindowWidth >> 1) - (VideoWidth >> 1);
		tr.top = this->m_rcWindow.top + (WindowHeight >> 1) - (VideoHeight >> 1);
		tr.bottom = tr.top + VideoHeight;
		tr.right = tr.left + VideoWidth;
	}

#ifdef _WMP

	// TODO: ???

#else

	hr = pVideo->SetDestinationPosition( tr.left, tr.top, tr.Width(), tr.Height() );
	if ( FAILED( hr ) )
		return hr;

#endif

	return S_OK;
}
