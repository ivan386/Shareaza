// Filter.cpp : Implementation of CFilter

#include "stdafx.h"
#include "Filter.h"

// CEnumMediaTypes

template<>
class ATL_NO_VTABLE _Copy< AM_MEDIA_TYPE* >
{
public:
	static HRESULT copy(AM_MEDIA_TYPE** p1, AM_MEDIA_TYPE** p2)
	{
		if ( ! *p1 )
		{
			*p1 = (AM_MEDIA_TYPE*)CoTaskMemAlloc(
				sizeof( AM_MEDIA_TYPE ) + (*p2)->cbFormat );
			(*p1)->cbFormat = (*p2)->cbFormat;
			(*p1)->pbFormat = (BYTE*)(*p1) + sizeof( AM_MEDIA_TYPE );
		}
		else
		{
			ATLASSERT( (*p1)->cbFormat == (*p2)->cbFormat );
			ATLASSERT( (*p1)->pbFormat == (BYTE*)(*p1) + sizeof( AM_MEDIA_TYPE ) );
		}
		(*p1)->majortype = (*p2)->majortype;
		(*p1)->subtype = (*p2)->subtype;
		(*p1)->bFixedSizeSamples = (*p2)->bFixedSizeSamples;
		(*p1)->bTemporalCompression = (*p2)->bTemporalCompression;
		(*p1)->lSampleSize = (*p2)->lSampleSize;
		(*p1)->formattype = (*p2)->formattype;
		(*p1)->pUnk = (*p2)->pUnk;
		CopyMemory( (*p1)->pbFormat, (*p2)->pbFormat, (*p2)->cbFormat );
		return S_OK;
	}

	static void init(AM_MEDIA_TYPE** p)
	{
		*p = NULL;
	}

	static void destroy(AM_MEDIA_TYPE** p)
	{
		if ( *p )
		{
			CoTaskMemFree( *p );
			*p = NULL;
		}
	}
};

typedef CComEnum < IEnumMediaTypes, &IID_IEnumMediaTypes, AM_MEDIA_TYPE*,
	_Copy< AM_MEDIA_TYPE* > > EnumMediaTypes;

typedef CComObject < EnumMediaTypes > CEnumMediaTypes;

// CEnumPins

typedef CComEnum < IEnumPins, &IID_IEnumPins, IPin*,
	_CopyInterface< IPin > > EnumPins;

typedef CComObject < EnumPins > CEnumPins;

// CFilter

CFilter::CFilter() :
	m_State( State_Stopped )
{
	ZeroMemory( m_AudioFormat, sizeof( m_AudioFormat ) );

	m_AudioFormat[ 0 ].type.majortype = MEDIATYPE_Audio;
	m_AudioFormat[ 0 ].type.subtype = MEDIASUBTYPE_PCM;
	m_AudioFormat[ 0 ].type.bFixedSizeSamples = TRUE;
	m_AudioFormat[ 0 ].type.bTemporalCompression = FALSE;
	m_AudioFormat[ 0 ].type.lSampleSize = 1;
	m_AudioFormat[ 0 ].type.formattype = FORMAT_WaveFormatEx;
	m_AudioFormat[ 0 ].type.cbFormat = sizeof ( m_AudioFormat[ 0 ].wave );
	m_AudioFormat[ 0 ].type.pbFormat = (BYTE*)&m_AudioFormat[ 0 ].wave;
	m_AudioFormat[ 0 ].wave.wFormatTag = WAVE_FORMAT_PCM;
	m_AudioFormat[ 0 ].wave.nChannels = 2;
	m_AudioFormat[ 0 ].wave.nSamplesPerSec = 44100;
	m_AudioFormat[ 0 ].wave.nAvgBytesPerSec = 44100 * 4;
	m_AudioFormat[ 0 ].wave.nBlockAlign = 4;
	m_AudioFormat[ 0 ].wave.wBitsPerSample = 16;
	m_AudioFormat[ 0 ].wave.cbSize = 0;
}

// IMediaFilter

STDMETHODIMP CFilter::Stop(void)
{
	m_State = State_Stopped;

	return S_OK;
}

STDMETHODIMP CFilter::Pause(void)
{
	m_State = State_Paused;

	return S_OK;
}

STDMETHODIMP CFilter::Run( 
	/* [in] */ REFERENCE_TIME tStart)
{
	m_State = State_Running;

	return S_OK;
}

STDMETHODIMP CFilter::GetState( 
	/* [in] */ DWORD dwMilliSecsTimeout,
	/* [out] */ __out  FILTER_STATE *State)
{
	if ( ! State )
		return E_POINTER;

	ATLTRACE( _T("Called IMediaFilter::GetState()\n") );

	*State = m_State;

	return S_OK;
}

STDMETHODIMP CFilter::SetSyncSource( 
	/* [in] */ __in_opt  IReferenceClock *pClock)
{
	ATLTRACE( _T("Called IMediaFilter::SetSyncSource( ReferenceClock = 0x%08x )\n"), pClock );

	return S_OK;
}

STDMETHODIMP CFilter::GetSyncSource( 
	/* [out] */ __deref_out_opt  IReferenceClock **pClock)
{
	return E_NOTIMPL;
}

// IBaseFilter

STDMETHODIMP CFilter::EnumPins( 
	/* [out] */ __out  IEnumPins **ppEnum)
{
	if ( ! ppEnum )
		return E_POINTER;

	*ppEnum = NULL;

	ATLTRACE( _T("Called IBaseFilter::EnumPins()\n") );

	CEnumPins* pEnum = NULL;
	CEnumPins::CreateInstance( &pEnum );

	if ( ! m_pPin )
		m_pPin = this;
	HRESULT hr = pEnum->Init( &m_pPin.p, &m_pPin.p + 1, NULL );
	if ( SUCCEEDED( hr ) )
	{
		hr = pEnum->QueryInterface( IID_IEnumPins, (void**)ppEnum );
	}
	return hr;
}

STDMETHODIMP CFilter::FindPin( 
	/* [string][in] */ LPCWSTR Id,
	/* [out] */ __out  IPin **ppPin)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::QueryFilterInfo( 
	/* [out] */ __out  FILTER_INFO *pInfo)
{
	if ( ! pInfo )
		return E_POINTER;

	ATLTRACE( _T("Called IBaseFilter::QueryFilterInfo()\n") );

	CopyMemory( pInfo->achName, szFilterName, sizeof( szFilterName ) );

	pInfo->pGraph = CComQIPtr< IFilterGraph >( m_pGraph ).Detach();

	return S_OK;
}

STDMETHODIMP CFilter::JoinFilterGraph( 
	/* [in] */ __in_opt  IFilterGraph *pGraph,
	/* [string][in] */ __in_opt  LPCWSTR pName)
{
	if ( ! pGraph )
		return E_POINTER;

	ATLTRACE( _T("Called IBaseFilter::JoinFilterGraph()\n") );

	m_pGraph = pGraph;

	return S_OK;
}

STDMETHODIMP CFilter::QueryVendorInfo( 
	/* [string][out] */ __out  LPWSTR *pVendorInfo)
{
	ATLTRACE( _T("Called IBaseFilter::QueryVendorInfo()\n") );

	return E_NOTIMPL;
}

// IPin

STDMETHODIMP CFilter::Connect( 
	/* [in] */ IPin *pReceivePin,
	/* [in] */ __in_opt  const AM_MEDIA_TYPE *pmt)
{
	ATLTRACE( _T("Called IPin::Connect()\n") );

	return E_NOTIMPL;
}

STDMETHODIMP CFilter::ReceiveConnection( 
	/* [in] */ IPin *pConnector,
	/* [in] */ const AM_MEDIA_TYPE *pmt)
{
	if ( ! pConnector || ! pmt )
		return E_POINTER;

	LPOLESTR majortype;
	StringFromCLSID( pmt->majortype, &majortype );
	LPOLESTR subtype;
	StringFromCLSID( pmt->subtype, &subtype );
	LPOLESTR formattype;
	StringFromCLSID( pmt->formattype, &formattype );

	ATLTRACE( _T("Called IPin::ReceiveConnection() : %s, %s, %s\n"),
		( ( pmt->majortype == MEDIATYPE_Video ) ? L"MEDIATYPE_Video"   :
		( ( pmt->majortype == MEDIATYPE_Audio ) ? L"MEDIATYPE_Audio"   :
		majortype ) ),
		( ( pmt->subtype == MEDIASUBTYPE_PCM ) ? L"MEDIASUBTYPE_PCM"  :
		subtype ),
		( ( pmt->formattype == FORMAT_VideoInfo ) ? L"FORMAT_VideoInfo" :
		( ( pmt->formattype == FORMAT_VideoInfo2 ) ? L"FORMAT_VideoInfo2" :
		( ( pmt->formattype == FORMAT_WaveFormatEx ) ? L"FORMAT_WaveFormatEx" :
		formattype ) ) )
		);

	CoTaskMemFree( majortype );
	CoTaskMemFree( subtype );
	CoTaskMemFree( formattype );

	if ( pmt->majortype != MEDIATYPE_Audio ||
		 pmt->subtype != MEDIASUBTYPE_PCM ||
		 pmt->formattype != FORMAT_WaveFormatEx )
		return VFW_E_TYPE_NOT_ACCEPTED;
	
	WAVEFORMATEX* pWave = (WAVEFORMATEX*)pmt->pbFormat;
	if ( pWave->wFormatTag != WAVE_FORMAT_PCM &&
		 pWave->wFormatTag != WAVE_FORMAT_EXTENSIBLE )
		return VFW_E_TYPE_NOT_ACCEPTED;

	return S_OK;
}

STDMETHODIMP CFilter::Disconnect(void)
{
	ATLTRACE( _T("Called IPin::Disconnect()\n") );

	return E_NOTIMPL;
}

STDMETHODIMP CFilter::ConnectedTo( 
	/* [out] */ __out  IPin **pPin)
{
	if ( ! pPin )
		return E_POINTER;

	*pPin = NULL;

	ATLTRACE( _T("Called IPin::ConnectedTo()\n") );

	return S_OK;
}

STDMETHODIMP CFilter::ConnectionMediaType( 
	/* [out] */ __out  AM_MEDIA_TYPE *pmt)
{
	if ( ! pmt )
		return E_POINTER;

	ATLTRACE( _T("Called IPin::ConnectionMediaType() - VFW_E_NOT_CONNECTED\n") );

	return VFW_E_NOT_CONNECTED;
}

STDMETHODIMP CFilter::QueryPinInfo( 
	/* [out] */ __out  PIN_INFO *pInfo)
{
	if ( ! pInfo )
		return E_POINTER;

	ATLTRACE( _T("Called IPin::QueryPinInfo()\n") );

	CopyMemory( pInfo->achName, szPinName, sizeof( szPinName ) );
	pInfo->dir = PINDIR_INPUT;

	pInfo->pFilter = CComQIPtr< IBaseFilter >( this ).Detach();

	return S_OK;
}

STDMETHODIMP CFilter::QueryDirection(
	/* [out] */ __out  PIN_DIRECTION *pPinDir)
{
	if ( ! pPinDir )
		return E_POINTER;

	ATLTRACE( _T("Called IPin::QueryDirection()\n") );

	*pPinDir = PINDIR_INPUT;

	return S_OK;
}

STDMETHODIMP CFilter::QueryId( 
	/* [out] */ __out  LPWSTR *Id)
{
	if ( ! Id )
		return E_POINTER;

	ATLTRACE( _T("Called IPin::QueryId()\n") );

	*Id = (LPWSTR)CoTaskMemAlloc( sizeof( szPinName ) );
	CopyMemory( *Id, szPinName, sizeof( szPinName ) );

	return S_OK;
}

STDMETHODIMP CFilter::QueryAccept( 
	/* [in] */ const AM_MEDIA_TYPE *pmt)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::EnumMediaTypes( 
	/* [out] */ __out  IEnumMediaTypes **ppEnum)
{
	if ( ! ppEnum )
		return E_POINTER;

	*ppEnum = NULL;

	ATLTRACE( _T("Called IPin::EnumMediaTypes()\n") );

	CEnumMediaTypes* pEnum = NULL;
	CEnumMediaTypes::CreateInstance( &pEnum );

	AM_MEDIA_TYPE* type = &m_AudioFormat[ 0 ].type;
	HRESULT hr = pEnum->Init( &type, &type + 1, NULL, AtlFlagCopy );
	if ( SUCCEEDED( hr ) )
	{
		hr = pEnum->QueryInterface( IID_IEnumMediaTypes, (void**)ppEnum );
	}
	return hr;
}

STDMETHODIMP CFilter::QueryInternalConnections( 
	/* [out] */ __out_ecount_part_opt(*nPin, *nPin)  IPin **apPin,
	/* [out][in] */ ULONG *nPin)
{
	if ( ! apPin || ! nPin )
		return E_POINTER;

	*apPin = NULL;
	*nPin = 0;

	ATLTRACE( _T("Called IPin::QueryInternalConnections()\n") );

	return E_NOTIMPL;
}

STDMETHODIMP CFilter::EndOfStream(void)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::BeginFlush(void)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::EndFlush(void)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::NewSegment( 
	/* [in] */ REFERENCE_TIME tStart,
	/* [in] */ REFERENCE_TIME tStop,
	/* [in] */ double dRate)
{
	return E_NOTIMPL;
}

// IMemInputPin

STDMETHODIMP CFilter::GetAllocator( 
	/* [out] */ __out  IMemAllocator **ppAllocator)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::NotifyAllocator( 
	/* [in] */ IMemAllocator *pAllocator,
	/* [in] */ BOOL bReadOnly)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::GetAllocatorRequirements( 
	/* [out] */  __out  ALLOCATOR_PROPERTIES *pProps)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::Receive( 
	/* [in] */ IMediaSample *pSample)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::ReceiveMultiple( 
	/* [size_is][in] */ __in_ecount(nSamples)  IMediaSample **pSamples,
	/* [in] */ long nSamples,
	/* [out] */ __out  long *nSamplesProcessed)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::ReceiveCanBlock(void)
{
	return E_NOTIMPL;
}

// IMediaSeeking

STDMETHODIMP CFilter::GetCapabilities( 
	/* [out] */ __out  DWORD *pCapabilities)
{
	ATLTRACE( _T("Called IMediaSeeking::GetCapabilities() - E_NOTIMPL\n") );

	return E_NOTIMPL;
}

STDMETHODIMP CFilter::CheckCapabilities( 
	/* [out][in] */ DWORD *pCapabilities)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::IsFormatSupported( 
	/* [in] */ const GUID *pFormat)
{
	ATLTRACE( _T("Called IMediaSeeking::IsFormatSupported()\n") );

	return S_OK;// E_NOTIMPL;
}

STDMETHODIMP CFilter::QueryPreferredFormat( 
	/* [out] */ __out  GUID *pFormat)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::GetTimeFormat( 
	/* [out] */ __out  GUID *pFormat)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::IsUsingTimeFormat( 
	/* [in] */ const GUID *pFormat)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::SetTimeFormat( 
	/* [in] */ const GUID *pFormat)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::GetDuration( 
	/* [out] */ __out  LONGLONG *pDuration)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::GetStopPosition( 
	/* [out] */ __out  LONGLONG *pStop)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::GetCurrentPosition( 
	/* [out] */ __out  LONGLONG *pCurrent)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::ConvertTimeFormat( 
	/* [out] */ __out  LONGLONG *pTarget,
	/* [in] */ __in_opt  const GUID *pTargetFormat,
	/* [in] */ LONGLONG Source,
	/* [in] */ __in_opt  const GUID *pSourceFormat)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::SetPositions( 
	/* [out][in] */ __inout_opt  LONGLONG *pCurrent,
	/* [in] */ DWORD dwCurrentFlags,
	/* [out][in] */ __inout_opt  LONGLONG *pStop,
	/* [in] */ DWORD dwStopFlags)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::GetPositions( 
	/* [out] */ __out_opt  LONGLONG *pCurrent,
	/* [out] */ __out_opt  LONGLONG *pStop)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::GetAvailable( 
	/* [out] */ __out_opt  LONGLONG *pEarliest,
	/* [out] */ __out_opt  LONGLONG *pLatest)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::SetRate( 
	/* [in] */ double dRate)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::GetRate( 
	/* [out] */ __out  double *pdRate)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::GetPreroll( 
	/* [out] */ __out  LONGLONG *pllPreroll)
{
	return E_NOTIMPL;
}

// IMediaPosition

STDMETHODIMP CFilter::get_Duration( 
	/* [retval][out] */ __RPC__out REFTIME *plength)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::put_CurrentPosition( 
	/* [in] */ REFTIME llTime)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::get_CurrentPosition( 
	/* [retval][out] */ __RPC__out REFTIME *pllTime)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::get_StopTime( 
	/* [retval][out] */ __RPC__out REFTIME *pllTime)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::put_StopTime( 
	/* [in] */ REFTIME llTime)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::get_PrerollTime( 
	/* [retval][out] */ __RPC__out REFTIME *pllTime)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::put_PrerollTime( 
	/* [in] */ REFTIME llTime)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::put_Rate( 
	/* [in] */ double dRate)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::get_Rate( 
	/* [retval][out] */ __RPC__out double *pdRate)
{
	ATLTRACE( _T("Called IMediaPosition::get_Rate() - E_NOTIMPL\n") );

	return E_NOTIMPL;
}

STDMETHODIMP CFilter::CanSeekForward( 
	/* [retval][out] */ __RPC__out LONG *pCanSeekForward)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFilter::CanSeekBackward( 
	/* [retval][out] */ __RPC__out LONG *pCanSeekBackward)
{
	return E_NOTIMPL;
}
