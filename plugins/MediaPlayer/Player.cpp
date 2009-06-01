// Player.cpp : Implementation of CPlayer

#include "stdafx.h"
#include "Player.h"

#define SAFE_RELEASE(comPtr)  \
              if ((comPtr))  \
                { (comPtr).Release(); }

// CPlayer

CPlayer::CPlayer() :
	m_hwndOwner( NULL ),
	m_rcWindow(),
	m_nZoom(smaDefault),
	m_dAspect(0.0)
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
	/* [in] */ HWND hWnd)
{
	if ( ! hWnd )
		return E_INVALIDARG;

	m_hwndOwner = hWnd;

	if ( m_pGraph )
		return E_INVALIDARG;

	HRESULT hr = m_pGraph.CoCreateInstance( CLSID_FilterGraph );
	if ( SUCCEEDED( hr ) )
	{
		m_pControl = m_pGraph;
		if (!m_pControl)
			hr = E_NOINTERFACE;
	}

	if ( SUCCEEDED( hr ) )
	{
		m_pEvent = m_pGraph;
		if (!m_pEvent)
			hr = E_NOINTERFACE;
	}

	if ( SUCCEEDED( hr ) )
	{
		m_pVideo = m_pGraph;
		if (!m_pVideo)
			hr = E_NOINTERFACE;
	}

	if ( SUCCEEDED( hr ) )
	{
		m_pWindow = m_pGraph;
		if (!m_pWindow)
			hr = E_NOINTERFACE;
	}

	if ( FAILED( hr ) )
	{
		SAFE_RELEASE(m_pControl);
		SAFE_RELEASE(m_pEvent);
		SAFE_RELEASE(m_pVideo);
		SAFE_RELEASE(m_pWindow);
		SAFE_RELEASE(m_pGraph);
	}

	return hr;
}

STDMETHODIMP CPlayer::Destroy(void)
{
	if ( ! m_pGraph )
		return E_INVALIDARG;

	Close();

	m_pWindow->put_Owner( NULL );

	m_pWindow.Release();
	m_pVideo.Release();
	m_pEvent.Release();
	m_pControl.Release();
	m_pGraph.Release();

	return S_OK;
}

STDMETHODIMP CPlayer::Reposition(
	/* [in] */ RECT *prcWnd)
{
	if ( ! prcWnd )
		return E_POINTER;

	m_rcWindow = *prcWnd;

	if ( ! m_pGraph )
		return E_INVALIDARG;

	HRESULT hr = m_pWindow->SetWindowPosition( m_rcWindow.left, m_rcWindow.top,
		m_rcWindow.right - m_rcWindow.left, m_rcWindow.bottom - m_rcWindow.top );

	if ( FAILED( hr ) )
		return hr;

	return AdjustVideoPosAndZoom();
}

STDMETHODIMP CPlayer::SetLogoBitmap(
	/* [in] */ HBITMAP hLogo)
{
	return E_NOTIMPL;
}

STDMETHODIMP CPlayer::GetVolume(
	/* [out] */ DOUBLE *pnVolume)
{
	if ( ! pnVolume )
		return E_POINTER;

	*pnVolume = 1.;

	if ( ! m_pGraph )
		return E_INVALIDARG;

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
	if ( ! m_pGraph )
		return E_INVALIDARG;

	// TODO: Use IAudioClient under Windows Vista

	CComQIPtr< IBasicAudio > pAudio( m_pGraph );
	if ( ! pAudio )
		return E_NOINTERFACE;

	// 0.0 ... 1.0 -> -10,000 ... 0 Conversion
	return pAudio->put_Volume( (long)( ( nVolume * 6000. ) - 6000. ) );
}

STDMETHODIMP CPlayer::GetZoom(
	/* [out] */ MediaZoom *pnZoom)
{
	if ( ! pnZoom )
		return E_POINTER;

	if ( ! m_pGraph )
		return E_INVALIDARG;

	*pnZoom = m_nZoom;

	return S_OK;
}

STDMETHODIMP CPlayer::SetZoom(
	/* [in] */ MediaZoom nZoom)
{
	if ( ! m_pGraph )
		return E_INVALIDARG;

	m_nZoom = nZoom;

	return AdjustVideoPosAndZoom();
}

STDMETHODIMP CPlayer::GetAspect(
	/* [out] */ DOUBLE *pdAspect)
{
	if ( ! pdAspect )
		return E_POINTER;

	*pdAspect = m_dAspect;

	if ( ! m_pGraph )
		return E_INVALIDARG;

	return S_OK;
}

STDMETHODIMP CPlayer::SetAspect(
	/* [in] */ DOUBLE dAspect)
{
	if ( ! m_pGraph )
		return E_INVALIDARG;

	m_dAspect = dAspect;

	return AdjustVideoPosAndZoom();
}

HRESULT FindPin(IBaseFilter* pFilter, int count, PIN_DIRECTION dir, IPin** ppPin)
{
	*ppPin = NULL;

	CComPtr< IEnumPins > pPins;
	HRESULT hr = pFilter->EnumPins( &pPins );
	if ( FAILED( hr ) )
		return hr;

	pPins->Reset();

	for (;;)
	{
		CComPtr< IPin > pPin;
		hr = pPins->Next( 1, &pPin, NULL );
		if ( hr != S_OK )
			break;

		PIN_INFO PinInfo = {};
		hr = pPin->QueryPinInfo( &PinInfo );
		if ( FAILED( hr ) )
			break;

		if ( PinInfo.dir == dir )
		{
			if ( count-- == 0 )
			{
				*ppPin = pPin.Detach();
				return S_OK;
			}
		}
	}

	return E_FAIL;
}

STDMETHODIMP CPlayer::Open(
	/* [in] */ BSTR sFilename)
{
	HRESULT hr;

	if ( ! sFilename )
		return E_POINTER;

	if ( ! m_pGraph )
		return E_INVALIDARG;

	hr = m_pGraph->RenderFile( sFilename, NULL );
	if ( FAILED( hr ) )
		return hr;

	m_pWindow->put_WindowStyle( WS_CHILD | WS_VISIBLE |
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN );
	m_pWindow->put_Owner( (OAHWND)m_hwndOwner );

	return Play();
}

STDMETHODIMP CPlayer::Close(void)
{
	if ( ! m_pGraph )
		return E_INVALIDARG;

	Stop(); 

	return S_OK;
}

STDMETHODIMP CPlayer::Play(void)
{
	if ( ! m_pGraph )
		return E_INVALIDARG;

	m_pWindow->SetWindowPosition( m_rcWindow.left, m_rcWindow.top,
		m_rcWindow.right - m_rcWindow.left, m_rcWindow.bottom - m_rcWindow.top );

	// Handle pending zoom and aspect changes
	HRESULT hr = AdjustVideoPosAndZoom();

	if ( FAILED( hr ) )
		return hr;

	hr = m_pControl->Run();
	if ( FAILED( hr ) )
		return hr;

	return S_OK;
}

STDMETHODIMP CPlayer::Pause(void)
{
	if ( ! m_pGraph )
		return E_INVALIDARG;

	return m_pControl->Pause();
}

STDMETHODIMP CPlayer::Stop(void)
{
	if ( ! m_pGraph )
		return E_INVALIDARG;

	m_pWindow->put_Visible( OAFALSE );

	return m_pControl->Stop();
}

STDMETHODIMP CPlayer::GetState(
	/* [out] */ MediaState *pnState)
{
	if ( ! pnState )
		return E_POINTER;

	*pnState = smsNull;

	if ( ! m_pGraph )
		return S_OK;

	OAFilterState st = State_Stopped;
	if ( SUCCEEDED( m_pControl->GetState( 250, &st ) ) )
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
		return E_INVALIDARG;

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
		return E_INVALIDARG;

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

	*pnSpeed = 1.;

	if ( ! m_pGraph )
		return E_INVALIDARG;

	CComQIPtr< IMediaSeeking > pSeek( m_pGraph );
	if ( ! pSeek )
		return E_NOINTERFACE;

	return pSeek->GetRate( pnSpeed );
}

STDMETHODIMP CPlayer::SetSpeed(
	/* [in] */ DOUBLE nSpeed)
{
	if ( ! m_pGraph )
		return E_INVALIDARG;

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

	*ppPlugin = NULL;

	return E_NOTIMPL;
}

STDMETHODIMP CPlayer::SetPlugin(
	/* [in] */ IAudioVisPlugin *pPlugin)
{
	if ( ! pPlugin )
		return E_POINTER;

	return E_NOTIMPL;
}

STDMETHODIMP CPlayer::GetPluginSize(
	/* [out] */ LONG *pnSize)
{
	if ( ! pnSize )
		return E_POINTER;

	return E_NOTIMPL;
}

STDMETHODIMP CPlayer::SetPluginSize(
	/* [in] */ LONG nSize)
{
	return E_NOTIMPL;
}

// Adjusts video position and zoom according to aspect ratio, zoom level and zoom type
HRESULT CPlayer::AdjustVideoPosAndZoom(void)
{
	long VideoWidth, VideoHeight;
	long WindowWidth, WindowHeight;
	CRect tr;

	HRESULT hr = m_pVideo->GetVideoSize(&VideoWidth, &VideoHeight);
	if ( FAILED( hr ) )
		return hr;

	WindowWidth = this->m_rcWindow.right - this->m_rcWindow.left;
	WindowHeight = this->m_rcWindow.bottom - this->m_rcWindow.top;

	if (m_dAspect > 0)
	{
		// Stretch video to match aspect ratio
		if ((double)VideoWidth / VideoHeight > m_dAspect) {
			VideoHeight = (long)(VideoWidth / m_dAspect);
		} else {
			VideoWidth = (long)(VideoHeight * m_dAspect);
		}
	}

	if (m_nZoom > 1)
	{
		VideoWidth *= m_nZoom;
		VideoHeight *= m_nZoom;
	}
	else if (m_nZoom == smzFill)
	{
		double VideoAspect = m_dAspect > 0 ? m_dAspect : (double)VideoWidth / VideoHeight;
		double WindowAspect = (double)WindowWidth / WindowHeight;

		if (WindowAspect > VideoAspect)
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

	if (m_nZoom == smzDistort)
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

	hr = m_pVideo->SetDestinationPosition(tr.left, tr.top, tr.Width(), tr.Height());
	if ( FAILED( hr ) )
		return hr;

	return S_OK;
}
