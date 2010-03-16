//
// Player.cpp : Implementation of CPlayer
//
// Copyright (c) Nikolay Raspopov, 2009.
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

// CPlayer

CPlayer::CPlayer()
	: m_bAudioOnly( FALSE )
	, m_hwndOwner( NULL )
	, m_rcWindow()
	, m_nZoom( smaDefault )
	, m_dAspect( 0.0 )
	, m_dVolume( 0.0 )
	, m_dSpeed( 0.0 )
	, m_nVisSize( 0 )
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

	if ( m_wndPlayer.m_hWnd )
		m_wndPlayer.DestroyWindow();

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

	if ( m_bAudioOnly )
	{
		if ( m_wndPlayer.m_hWnd )
			m_wndPlayer.SetWindowPos( NULL, &m_rcWindow,
				SWP_ASYNCWINDOWPOS | SWP_NOZORDER );
	}
	else
	{	
		if ( ! m_pGraph )
			return S_OK;

		CComQIPtr< IVideoWindow > pWindow( m_pGraph );
		if ( ! pWindow )
			return E_NOINTERFACE;

		pWindow->SetWindowPosition( m_rcWindow.left, m_rcWindow.top,
			m_rcWindow.right - m_rcWindow.left, m_rcWindow.bottom - m_rcWindow.top );
	}

	AdjustVideoPosAndZoom();

	return S_OK;
}

STDMETHODIMP CPlayer::GetVolume(
	/* [out] */ DOUBLE *pnVolume)
{
	if ( ! pnVolume )
		return E_POINTER;

	*pnVolume = m_dVolume;

	if ( ! m_pGraph )
		return S_OK;

	// TODO: Use IAudioClient under Windows Vista

	CComQIPtr< IBasicAudio > pAudio( m_pGraph );
	if ( ! pAudio )
		return E_NOINTERFACE;

	long lVolume = 0;
	HRESULT hr = pAudio->get_Volume( &lVolume );
	if ( FAILED( hr ) )
		return hr;

	// -6,000 ... 0 -> 0.0 ... 1.0 Conversion
	if ( lVolume < -6000. )
		*pnVolume = 0;
	else 
		*pnVolume = ( lVolume + 6000. ) / 6000.;

	return S_OK;
}

STDMETHODIMP CPlayer::SetVolume(
	/* [in] */ DOUBLE nVolume)
{
	m_dVolume = nVolume;

	if ( ! m_pGraph )
		return S_OK;

	// TODO: Use IAudioClient under Windows Vista

	CComQIPtr< IBasicAudio > pAudio( m_pGraph );
	if ( ! pAudio )
		return E_NOINTERFACE;

	// 0.0 ... 1.0 -> -6,000 ... 0 Conversion
	return pAudio->put_Volume( (long)( ( nVolume * 6000. ) - 6000. ) );
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

	if ( ! m_bAudioOnly )
		AdjustVideoPosAndZoom();

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

	if ( ! m_bAudioOnly )
		AdjustVideoPosAndZoom();

	return S_OK;
}

HRESULT SafeRenderFile(IGraphBuilder* pGraph, BSTR sFilename) throw()
{
	__try
	{
		return pGraph->RenderFile( sFilename, NULL );
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		return E_FAIL;
	}
}

STDMETHODIMP CPlayer::Open(
	/* [in] */ BSTR sFilename)
{
	if ( ! sFilename )
		return E_POINTER;

	if ( m_pGraph )
		return E_INVALIDARG;

	HRESULT hr = m_pGraph.CoCreateInstance( CLSID_FilterGraph );
	if ( FAILED( hr ) )
		return hr;

	hr = SafeRenderFile( m_pGraph, sFilename );
	if ( FAILED( hr ) )
		return hr;

	CComQIPtr< IVideoWindow > pWindow( m_pGraph );
	long lVisible = 0;
	m_bAudioOnly = ( ! pWindow || pWindow->get_Visible( &lVisible ) == E_NOINTERFACE );

	//Dump( m_pGraph );

	/*if ( m_bAudioOnly )
	{
		if ( ! m_wndPlayer.m_hWnd )
		{
			m_wndPlayer.Create( (HWND)m_hwndOwner, &m_rcWindow,
				_T("MediaPlayer Window"),
				WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
				WS_EX_TOPMOST );
		}
	}
	else*/
	{
		hr = pWindow->put_WindowStyle( WS_CHILD | WS_VISIBLE |
			WS_CLIPSIBLINGS | WS_CLIPCHILDREN );
		hr = pWindow->put_WindowStyleEx( WS_EX_TOPMOST );
		hr = pWindow->put_Owner( m_hwndOwner );
		hr = pWindow->put_MessageDrain( m_hwndOwner );
	}

	return S_OK;
}

STDMETHODIMP CPlayer::Close(void)
{
	if ( ! m_pGraph )
		// Already closed
		return S_OK;

	Stop();

	m_pGraph->Abort();

	m_pGraph.Release();

	return S_OK;
}

STDMETHODIMP CPlayer::Play(void)
{
	HRESULT hr;

	if ( ! m_pGraph )
		return E_INVALIDARG;

	if ( m_bAudioOnly )
	{
		if ( m_wndPlayer.m_hWnd )
			m_wndPlayer.SetWindowPos( NULL, &m_rcWindow,
				SWP_ASYNCWINDOWPOS | SWP_NOZORDER | SWP_SHOWWINDOW );
	}
	else
	{
		CComQIPtr< IVideoWindow > pWindow( m_pGraph );
		if ( ! pWindow )
			return E_NOINTERFACE;

		pWindow->SetWindowPosition( m_rcWindow.left, m_rcWindow.top,
			m_rcWindow.right - m_rcWindow.left, m_rcWindow.bottom - m_rcWindow.top );
	}

	CComQIPtr< IMediaControl > pControl( m_pGraph );
	if ( ! pControl )
		return E_NOINTERFACE;
		
	hr = pControl->Run();
	if ( FAILED( hr ) )
		return hr;

	// Restore volume level
	SetVolume( m_dVolume );

	// Restore speed
	SetSpeed( m_dSpeed );

	// Restore zoom and aspect ratio
	if ( ! m_bAudioOnly )
		AdjustVideoPosAndZoom();

	return S_OK;
}

STDMETHODIMP CPlayer::Pause(void)
{
	if ( ! m_pGraph )
		return E_INVALIDARG;

	CComQIPtr< IMediaControl > pControl( m_pGraph );
	if ( ! pControl )
		return E_NOINTERFACE;

	return pControl->Pause();
}

STDMETHODIMP CPlayer::Stop(void)
{
	if ( ! m_pGraph )
		return E_INVALIDARG;

	if ( m_bAudioOnly )
	{
		if ( m_wndPlayer.m_hWnd )
			m_wndPlayer.ShowWindow( SW_HIDE );
	}
	else
	{
		CComQIPtr< IVideoWindow > pWindow( m_pGraph );
		if ( ! pWindow )
			return E_NOINTERFACE;

		pWindow->put_Visible( OAFALSE );
	}

	CComQIPtr< IMediaControl > pControl( m_pGraph );
	if ( ! pControl )
		return E_NOINTERFACE;

	return pControl->Stop();
}

STDMETHODIMP CPlayer::GetState(
	/* [out] */ MediaState *pnState)
{
	if ( ! pnState )
		return E_POINTER;

	*pnState = smsNull;

	if ( ! m_pGraph )
		return S_OK;

	CComQIPtr< IMediaControl > pControl( m_pGraph );
	if ( ! pControl )
		return E_NOINTERFACE;

	OAFilterState st = State_Stopped;
	if ( SUCCEEDED( pControl->GetState( 250, &st ) ) )
	{
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
	}

	return S_OK;
}

STDMETHODIMP CPlayer::GetLength(
	/* [out] */ LONGLONG *pnLength)
{
	if ( ! pnLength )
		return E_POINTER;

	*pnLength = 0;

	if ( ! m_pGraph )
		return S_OK;

	CComQIPtr< IMediaSeeking > pSeek( m_pGraph );
	if ( ! pSeek )
		return E_NOINTERFACE;

	return pSeek->GetDuration( pnLength );
}

STDMETHODIMP CPlayer::GetPosition(
	/* [out] */ LONGLONG *pnPosition)
{
	if ( ! pnPosition )
		return E_POINTER;

	*pnPosition = 0;

	if ( ! m_pGraph )
		return S_OK;

	CComQIPtr< IMediaSeeking > pSeek( m_pGraph );
	if ( ! pSeek )
		return E_NOINTERFACE;

	return pSeek->GetCurrentPosition( pnPosition );
}

STDMETHODIMP CPlayer::SetPosition(
	/* [in] */ LONGLONG nPosition)
{
	if ( ! m_pGraph )
		return E_INVALIDARG;

	CComQIPtr< IMediaSeeking > pSeek( m_pGraph );
	if ( ! pSeek )
		return E_NOINTERFACE;

	return pSeek->SetPositions( &nPosition, AM_SEEKING_AbsolutePositioning,
		NULL, AM_SEEKING_NoPositioning );
}

STDMETHODIMP CPlayer::GetSpeed(
	/* [out] */ DOUBLE *pnSpeed)
{
	if ( ! pnSpeed )
		return E_POINTER;

	*pnSpeed = m_dSpeed;

	if ( ! m_pGraph )
		return S_OK;

	CComQIPtr< IMediaSeeking > pSeek( m_pGraph );
	if ( ! pSeek )
		return E_NOINTERFACE;

	HRESULT hr = pSeek->GetRate( pnSpeed );
	if ( FAILED( hr ) )
		return hr;

	return S_OK;
}

STDMETHODIMP CPlayer::SetSpeed(
	/* [in] */ DOUBLE nSpeed)
{
	m_dSpeed = nSpeed;

	if ( ! m_pGraph )
		return S_OK;

	CComQIPtr< IMediaSeeking > pSeek( m_pGraph );
	if ( ! pSeek )
		return E_NOINTERFACE;

	return pSeek->SetRate( nSpeed );
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

	if ( m_bAudioOnly )
	{
		if ( m_wndPlayer.m_hWnd && m_wndPlayer.IsWindowVisible() )
			*pbVisible = VARIANT_TRUE;
		else
			*pbVisible = VARIANT_FALSE;
	}
	else
		*pbVisible = VARIANT_TRUE;

	return S_OK;
}

HRESULT CPlayer::AdjustVideoPosAndZoom()
{
	if ( ! m_pGraph )
		return E_INVALIDARG;

	CComQIPtr< IBasicVideo > pVideo( m_pGraph );
	if ( ! pVideo )
		return E_NOINTERFACE;

	long VideoWidth, VideoHeight;
	HRESULT hr = pVideo->GetVideoSize( &VideoWidth, &VideoHeight );
	if ( FAILED( hr ) )
		return hr;

	long WindowWidth = m_rcWindow.right - m_rcWindow.left;
	long WindowHeight = m_rcWindow.bottom - m_rcWindow.top;

	if ( m_dAspect > 0 )
	{
		// Stretch video to match aspect ratio
		if ( (double)VideoWidth / VideoHeight > m_dAspect )
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
	else if ( m_nZoom == smzFill )
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

	hr = pVideo->SetDestinationPosition( tr.left, tr.top, tr.Width(), tr.Height() );
	if ( FAILED( hr ) )
		return hr;

	return S_OK;
}
