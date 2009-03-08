// Player.cpp : Implementation of CPlayer

#include "stdafx.h"
#include "Player.h"

// CPlayer

CPlayer::CPlayer() :
	m_hWnd( NULL )
{
}

STDMETHODIMP CPlayer::Create(
	/* [in] */ HWND hWnd)
{
	if ( ! hWnd )
		return E_INVALIDARG;

	if ( m_hWnd )
		return E_FAIL;

	m_hWnd = MCIWndCreate( hWnd, _AtlBaseModule.GetModuleInstance(),  
		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
		MCIWNDF_NOAUTOSIZEWINDOW | MCIWNDF_NOERRORDLG | MCIWNDF_NOMENU |
		MCIWNDF_NOOPEN | MCIWNDF_NOPLAYBAR, NULL );
	if ( ! m_hWnd )
		return E_FAIL;

	MCIWndSetTimers( m_hWnd, 1000, 1000 );
	MCIWndUseTime( m_hWnd );
	
	return S_OK;
}

STDMETHODIMP CPlayer::Destroy(void)
{
	if ( ! m_hWnd )
		return E_FAIL;

	MCIWndDestroy( m_hWnd );
	m_hWnd = NULL;

	return S_OK;
}

STDMETHODIMP CPlayer::Reposition(
	/* [in] */ RECT *prcWnd)
{
	if ( ! prcWnd )
		return E_POINTER;

	if ( ! m_hWnd || prcWnd->right < prcWnd->left || prcWnd->bottom < prcWnd->top )
		return E_INVALIDARG;

	return MoveWindow( m_hWnd, prcWnd->left, prcWnd->top,
		prcWnd->right - prcWnd->left, prcWnd->bottom - prcWnd->top, TRUE ) ? S_OK : E_FAIL;
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

	if ( ! m_hWnd )
		return E_INVALIDARG;

	*pnVolume = MCIWndGetVolume( m_hWnd ) / 1000.;

	return S_OK;
}

STDMETHODIMP CPlayer::SetVolume(
	/* [in] */ DOUBLE nVolume)
{
	if ( ! m_hWnd )
		return E_INVALIDARG;

	return MCIWndSetVolume( m_hWnd, (LONG)( nVolume * 1000. ) ) == 0 ? S_OK : E_FAIL;
}

STDMETHODIMP CPlayer::GetZoom(
	/* [out] */ MediaZoom *pnZoom)
{
	if ( ! pnZoom )
		return E_POINTER;

	*pnZoom = smaDefault;

	if ( ! m_hWnd )
		return E_INVALIDARG;

	return E_NOTIMPL;
}

STDMETHODIMP CPlayer::SetZoom(
	/* [in] */ MediaZoom nZoom)
{
	if ( ! m_hWnd )
		return E_INVALIDARG;

	return E_NOTIMPL;
}

STDMETHODIMP CPlayer::GetAspect(
	/* [out] */ DOUBLE *pnAspect)
{
	if ( ! pnAspect )
		return E_POINTER;

	*pnAspect = 1.0;

	if ( ! m_hWnd )
		return E_INVALIDARG;

	return E_NOTIMPL;
}

STDMETHODIMP CPlayer::SetAspect(
	/* [in] */ DOUBLE nAspect)
{
	if ( ! m_hWnd )
		return E_INVALIDARG;

	return E_NOTIMPL;
}

STDMETHODIMP CPlayer::Open(
	/* [in] */ BSTR sFilename)
{
	if ( ! sFilename )
		return E_POINTER;

	if ( ! m_hWnd )
		return E_INVALIDARG;

	TCHAR szFilename[ MAX_PATH ];
	GetShortPathName( sFilename, szFilename, MAX_PATH );

	return ( MCIWndOpen( m_hWnd, szFilename, 0 ) == 0 ) ? S_OK : E_FAIL;
}

STDMETHODIMP CPlayer::Close(void)
{
	if ( ! m_hWnd )
		return E_INVALIDARG;

	return ( MCIWndClose( m_hWnd ) == 0 ) ? S_OK : E_FAIL;
}

STDMETHODIMP CPlayer::Play(void)
{
	if ( ! m_hWnd )
		return E_INVALIDARG;

	return ( MCIWndPlay( m_hWnd ) == 0 ) ? S_OK : E_FAIL;
}

STDMETHODIMP CPlayer::Pause(void)
{
	if ( ! m_hWnd )
		return E_INVALIDARG;

	return ( MCIWndPause( m_hWnd ) == 0 ) ? S_OK : E_FAIL;
}

STDMETHODIMP CPlayer::Stop(void)
{
	if ( ! m_hWnd )
		return E_INVALIDARG;

	return ( MCIWndStop( m_hWnd ) == 0 ) ? S_OK : E_FAIL;
}

STDMETHODIMP CPlayer::GetState(
	/* [out] */ MediaState *pnState)
{
	if ( ! pnState )
		return E_POINTER;

	*pnState = smsNull;

	if ( ! m_hWnd )
		return S_OK;

	char buf[ 16 ] = {};
	switch ( MCIWndGetMode( m_hWnd, buf, sizeof( buf ) ) )
	{
	case MCI_MODE_OPEN:
	case MCI_MODE_STOP:
		*pnState = smsOpen;
		break;
	case MCI_MODE_PAUSE:
		*pnState = smsPaused;
		break;
	case MCI_MODE_PLAY:
	case MCI_MODE_SEEK:
		*pnState = smsPlaying;
		break;
	}
	return S_OK;
}

STDMETHODIMP CPlayer::GetLength(
	/* [out] */ LONGLONG *pnLength)
{
	if ( ! pnLength )
		return E_POINTER;

	*pnLength = 0;

	if ( ! m_hWnd )
		return E_INVALIDARG;

	*pnLength = (LONGLONG)MCIWndGetLength( m_hWnd ) * 10000;

	return S_OK;
}

STDMETHODIMP CPlayer::GetPosition(
	/* [out] */ LONGLONG *pnPosition)
{
	if ( ! pnPosition )
		return E_POINTER;

	*pnPosition = 0;

	if ( ! m_hWnd )
		return E_INVALIDARG;

	*pnPosition = (LONGLONG)MCIWndGetPosition( m_hWnd ) * 10000;

	return S_OK;
}

STDMETHODIMP CPlayer::SetPosition(
	/* [in] */ LONGLONG nPosition)
{
	if ( ! m_hWnd )
		return E_INVALIDARG;

	return ( MCIWndSeek( m_hWnd, nPosition / 10000 ) == 0 ) ? S_OK : E_FAIL;
}

STDMETHODIMP CPlayer::GetSpeed(
	/* [out] */ DOUBLE *pnSpeed)
{
	if ( ! pnSpeed )
		return E_POINTER;

	*pnSpeed = 1.;

	if ( ! m_hWnd )
		return E_INVALIDARG;

	*pnSpeed = MCIWndGetSpeed( m_hWnd ) / 1000.;

	return S_OK;
}

STDMETHODIMP CPlayer::SetSpeed(
	/* [in] */ DOUBLE nSpeed)
{
	if ( ! m_hWnd )
		return E_INVALIDARG;

	return ( MCIWndSetSpeed( m_hWnd, nSpeed * 1000 ) == 0 ) ? S_OK : E_FAIL;
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
