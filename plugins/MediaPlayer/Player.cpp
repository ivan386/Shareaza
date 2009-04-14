// Player.cpp : Implementation of CPlayer

#include "stdafx.h"
#include "Player.h"

// CPlayer

CPlayer::CPlayer() :
	m_hwndOwner( NULL ),
	m_rcWindow()
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
		m_pEvent = m_pGraph;
		m_pWindow = m_pGraph;
		if ( ! m_pControl || ! m_pEvent || ! m_pWindow )
			hr = E_NOINTERFACE;
	}

	if ( FAILED( hr ) )
		Destroy();

	return hr;
}

STDMETHODIMP CPlayer::Destroy(void)
{
	if ( ! m_pGraph )
		return E_INVALIDARG;

	Close();

	m_pWindow->put_Owner( NULL );

	m_pWindow.Release();
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

	return m_pWindow->SetWindowPosition( m_rcWindow.left, m_rcWindow.top,
		m_rcWindow.right - m_rcWindow.left, m_rcWindow.bottom - m_rcWindow.top );
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

	// -10,000 ... 0 -> 0.0 ... 1.0 Conversion
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

	*pnZoom = smaDefault;

	if ( ! m_pGraph )
		return E_INVALIDARG;

	return E_NOTIMPL;
}

STDMETHODIMP CPlayer::SetZoom(
	/* [in] */ MediaZoom nZoom)
{
	if ( ! m_pGraph )
		return E_INVALIDARG;

	return E_NOTIMPL;
}

STDMETHODIMP CPlayer::GetAspect(
	/* [out] */ DOUBLE *pnAspect)
{
	if ( ! pnAspect )
		return E_POINTER;

	*pnAspect = 1.0;

	if ( ! m_pGraph )
		return E_INVALIDARG;

	return E_NOTIMPL;
}

STDMETHODIMP CPlayer::SetAspect(
	/* [in] */ DOUBLE nAspect)
{
	if ( ! m_pGraph )
		return E_INVALIDARG;

	return E_NOTIMPL;
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

#ifdef _DEBUG
	CComPtr< IPin > pAudioInPin;
	ATLTRACE( _T("Filter Graph for: %s\n"), sFilename );
	CComQIPtr< IFilterGraph > pFilterGraph( m_pGraph );
	if ( pFilterGraph )
	{
		CComPtr< IEnumFilters > pFilters;
		hr = pFilterGraph->EnumFilters( &pFilters );
		if ( SUCCEEDED( hr ) )
		{
			pFilters->Reset();
			for (;;)
			{
				CComPtr< IBaseFilter > pFilter;
				hr = pFilters->Next( 1, &pFilter, NULL );
				if ( hr != S_OK )
					break;

				FILTER_INFO FilterInfo = {};
				pFilter->QueryFilterInfo( &FilterInfo );

				ATLTRACE( _T("Filter: \"%s\"\n"), FilterInfo.achName );

				bool bOutPin = false;

				CComPtr< IEnumPins > pPins;
				hr = pFilter->EnumPins( &pPins );
				if ( SUCCEEDED( hr ) )
				{
					pPins->Reset();
					for (;;)
					{
						CComPtr< IPin > pPin;
						hr = pPins->Next( 1, &pPin, NULL );
						if ( hr != S_OK )
							break;

						PIN_INFO PinInfo = {};
						pPin->QueryPinInfo( &PinInfo );

						AM_MEDIA_TYPE MediaType = {};
						pPin->ConnectionMediaType( &MediaType );

						if ( ! pAudioInPin )
						{
							if ( PinInfo.dir == PINDIR_OUTPUT )
								bOutPin = true;
							if ( PinInfo.dir == PINDIR_INPUT && 
								 MediaType.formattype == FORMAT_WaveFormatEx )
								pAudioInPin = pPin;
						}

						ATLTRACE( _T("\tPin: %s %s \"%s\"\n"),
							( ( PinInfo.dir == PINDIR_INPUT )  ? _T(" in") :
							( ( PinInfo.dir == PINDIR_OUTPUT ) ? _T("out") :
							_T("...") ) ),
							( ( MediaType.formattype == FORMAT_WaveFormatEx ) ? _T("audio ") :
							( ( MediaType.formattype == FORMAT_VideoInfo )    ? _T("video ") :
							( ( MediaType.formattype == FORMAT_VideoInfo2 )   ? _T("video2") :
							_T("......") ) ) ),
							PinInfo.achName );

						if ( PinInfo.pFilter )
							PinInfo.pFilter->Release();
					}
				}

				if ( FilterInfo.pGraph )
					FilterInfo.pGraph->Release();

				if ( pAudioInPin && bOutPin )
					pAudioInPin.Release();
			}
		}
	}

	if ( pAudioInPin )
	{
		// Found filter with audio input pin and without any output pins

		ATLTRACE( _T("Connecting to...\n") );

		CComPtr< IPin > pSource;
		hr = pAudioInPin->ConnectedTo( &pSource );
		if ( SUCCEEDED( hr ) )
		{
			AM_MEDIA_TYPE MediaType = {};
			hr = pSource->ConnectionMediaType( &MediaType );

			CComPtr< IBaseFilter > pInfTree;
			hr = pInfTree.CoCreateInstance( CLSID_InfTee );
			hr = m_pGraph->AddFilter( pInfTree, L"Tee" );
			if ( FAILED( hr ) )
				return hr;

			CComPtr< IBaseFilter > pMyFilter;
			hr = pMyFilter.CoCreateInstance( CLSID_Filter );
			hr = m_pGraph->AddFilter( pMyFilter, L"Spy" );
			if ( FAILED( hr ) )
				return hr;

			hr = m_pGraph->Disconnect( pAudioInPin );
			hr = m_pGraph->Disconnect( pSource );

			ATLTRACE( _T("Connecting Audio source to Tee...\n") );

			CComPtr< IPin > pInfTreeInPin;
			hr = FindPin( pInfTree, 0, PINDIR_INPUT, &pInfTreeInPin );
			hr = pInfTreeInPin->ReceiveConnection( pSource, &MediaType );
			hr = pSource->Connect( pInfTreeInPin, &MediaType );
			hr = m_pGraph->Connect( pSource, pInfTreeInPin );

			ATLTRACE( _T("Connecting Tee to Audio filter...\n") );

			CComPtr< IPin > pInfTreeOut1Pin;
			hr = FindPin( pInfTree, 0, PINDIR_OUTPUT, &pInfTreeOut1Pin );
			hr = m_pGraph->Connect( pInfTreeOut1Pin, pAudioInPin );

			ATLTRACE( _T("Connecting Tee to My filter...\n") );

			CComPtr< IPin > pInfTreeOut2Pin;
			hr = FindPin( pInfTree, 1, PINDIR_OUTPUT, &pInfTreeOut2Pin );
			CComPtr< IPin > pMyFilterPin;
			hr = FindPin( pMyFilter, 0, PINDIR_INPUT, &pMyFilterPin );
			hr = m_pGraph->Connect( pInfTreeOut2Pin, pMyFilterPin );
		}
	}
#endif // _DEBUG

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

	HRESULT hr =  m_pControl->Run();
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
