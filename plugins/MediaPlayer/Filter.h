// Filter.h : Declaration of the CFilter

#pragma once

#include "MediaPlayer_h.h"


static const WCHAR szFilterName[] = { L"AudioSpy Filter" };
static const WCHAR szPinName[] = { L"In" };

// CAudioFormat

typedef struct
{
	AM_MEDIA_TYPE	type;
	WAVEFORMATEX	wave;
} CAudioFormat;

// CFilter

class ATL_NO_VTABLE CFilter :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CFilter, &CLSID_Filter>,
	public IBaseFilter,		// -> IMediaFilter -> IPersist-> IUnknown
	public IPin,			// -> IUnknown
	public IMemInputPin,	// -> IUnknown
	public IMediaSeeking,	// -> IUnknown
	public IDispatchImpl<IMediaPosition, &IID_IMediaPosition, &LIBID_MediaPlayerLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CFilter();

DECLARE_REGISTRY_RESOURCEID(IDR_FILTER)

BEGIN_COM_MAP(CFilter)
	COM_INTERFACE_ENTRY(IPersist)
	COM_INTERFACE_ENTRY(IMediaFilter)
	COM_INTERFACE_ENTRY(IBaseFilter)
	COM_INTERFACE_ENTRY(IPin)
	COM_INTERFACE_ENTRY(IMemInputPin)
	COM_INTERFACE_ENTRY(IMediaSeeking)
	COM_INTERFACE_ENTRY(IMediaPosition)
END_COM_MAP()

DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

protected:
	CComQIPtr< IFilterGraph >	m_pGraph;
	CComQIPtr< IPin >			m_pPin;
	CAudioFormat				m_AudioFormat[ 1 ];
	FILTER_STATE				m_State;

// IPersist
public:
	STDMETHOD(GetClassID)(CLSID *pClassID)
	{
		if ( ! pClassID )
			return E_POINTER;

		*pClassID = CFilter::GetObjectCLSID();

		return S_OK;
	}

// IMediaFilter
public:
	STDMETHOD(Stop)(void);
	STDMETHOD(Pause)(void);
	STDMETHOD(Run)( 
		/* [in] */ REFERENCE_TIME tStart);
	STDMETHOD(GetState)( 
		/* [in] */ DWORD dwMilliSecsTimeout,
		/* [out] */ __out  FILTER_STATE *State);
	STDMETHOD(SetSyncSource)( 
		/* [in] */ __in_opt  IReferenceClock *pClock);
	STDMETHOD(GetSyncSource)( 
		/* [out] */ __deref_out_opt  IReferenceClock **pClock);
        
// IBaseFilter
public:
	STDMETHOD(EnumPins)( 
		/* [out] */ __out  IEnumPins **ppEnum);
	STDMETHOD(FindPin)( 
		/* [string][in] */ LPCWSTR Id,
		/* [out] */ __out  IPin **ppPin);
	STDMETHOD(QueryFilterInfo)( 
		/* [out] */ __out  FILTER_INFO *pInfo);
	STDMETHOD(JoinFilterGraph)( 
		/* [in] */ __in_opt  IFilterGraph *pGraph,
		/* [string][in] */ __in_opt  LPCWSTR pName);
	STDMETHOD(QueryVendorInfo)( 
		/* [string][out] */ __out  LPWSTR *pVendorInfo);

// IPin
public:
	STDMETHOD(Connect)( 
		/* [in] */ IPin *pReceivePin,
		/* [in] */ __in_opt  const AM_MEDIA_TYPE *pmt);
	STDMETHOD(ReceiveConnection)( 
		/* [in] */ IPin *pConnector,
		/* [in] */ const AM_MEDIA_TYPE *pmt);
	STDMETHOD(Disconnect)(void);
	STDMETHOD(ConnectedTo)( 
		/* [out] */ __out  IPin **pPin);
	STDMETHOD(ConnectionMediaType)( 
		/* [out] */ __out  AM_MEDIA_TYPE *pmt);
	STDMETHOD(QueryPinInfo)( 
		/* [out] */ __out  PIN_INFO *pInfo);
	STDMETHOD(QueryDirection)(
		/* [out] */ __out  PIN_DIRECTION *pPinDir);
	STDMETHOD(QueryId)( 
		/* [out] */ __out  LPWSTR *Id);
	STDMETHOD(QueryAccept)( 
		/* [in] */ const AM_MEDIA_TYPE *pmt);
	STDMETHOD(EnumMediaTypes)( 
		/* [out] */ __out  IEnumMediaTypes **ppEnum);
	STDMETHOD(QueryInternalConnections)( 
		/* [out] */ __out_ecount_part_opt(*nPin, *nPin)  IPin **apPin,
		/* [out][in] */ ULONG *nPin);
	STDMETHOD(EndOfStream)(void);
	STDMETHOD(BeginFlush)(void);
	STDMETHOD(EndFlush)(void);
	STDMETHOD(NewSegment)( 
		/* [in] */ REFERENCE_TIME tStart,
		/* [in] */ REFERENCE_TIME tStop,
		/* [in] */ double dRate);

// IMemInputPin
public:
	STDMETHOD(GetAllocator)( 
		/* [out] */ __out  IMemAllocator **ppAllocator);
	STDMETHOD(NotifyAllocator)( 
		/* [in] */ IMemAllocator *pAllocator,
		/* [in] */ BOOL bReadOnly);
	STDMETHOD(GetAllocatorRequirements)( 
		/* [out] */  __out  ALLOCATOR_PROPERTIES *pProps);
	STDMETHOD(Receive)( 
		/* [in] */ IMediaSample *pSample);
	STDMETHOD(ReceiveMultiple)( 
		/* [size_is][in] */ __in_ecount(nSamples)  IMediaSample **pSamples,
		/* [in] */ long nSamples,
		/* [out] */ __out  long *nSamplesProcessed);
	STDMETHOD(ReceiveCanBlock)(void);

// IMediaSeeking
	STDMETHOD(GetCapabilities)( 
		/* [out] */ __out  DWORD *pCapabilities);
	STDMETHOD(CheckCapabilities)( 
		/* [out][in] */ DWORD *pCapabilities);
	STDMETHOD(IsFormatSupported)( 
		/* [in] */ const GUID *pFormat);
	STDMETHOD(QueryPreferredFormat)( 
		/* [out] */ __out  GUID *pFormat);
	STDMETHOD(GetTimeFormat)( 
		/* [out] */ __out  GUID *pFormat);
	STDMETHOD(IsUsingTimeFormat)( 
		/* [in] */ const GUID *pFormat);
	STDMETHOD(SetTimeFormat)( 
		/* [in] */ const GUID *pFormat);
	STDMETHOD(GetDuration)( 
		/* [out] */ __out  LONGLONG *pDuration);
	STDMETHOD(GetStopPosition)( 
		/* [out] */ __out  LONGLONG *pStop);
	STDMETHOD(GetCurrentPosition)( 
		/* [out] */ __out  LONGLONG *pCurrent);
	STDMETHOD(ConvertTimeFormat)( 
		/* [out] */ __out  LONGLONG *pTarget,
		/* [in] */ __in_opt  const GUID *pTargetFormat,
		/* [in] */ LONGLONG Source,
		/* [in] */ __in_opt  const GUID *pSourceFormat);
	STDMETHOD(SetPositions)( 
		/* [out][in] */ __inout_opt  LONGLONG *pCurrent,
		/* [in] */ DWORD dwCurrentFlags,
		/* [out][in] */ __inout_opt  LONGLONG *pStop,
		/* [in] */ DWORD dwStopFlags);
	STDMETHOD(GetPositions)( 
		/* [out] */ __out_opt  LONGLONG *pCurrent,
		/* [out] */ __out_opt  LONGLONG *pStop);
	STDMETHOD(GetAvailable)( 
		/* [out] */ __out_opt  LONGLONG *pEarliest,
		/* [out] */ __out_opt  LONGLONG *pLatest);
	STDMETHOD(SetRate)( 
		/* [in] */ double dRate);
	STDMETHOD(GetRate)( 
		/* [out] */ __out  double *pdRate);
	STDMETHOD(GetPreroll)( 
		/* [out] */ __out  LONGLONG *pllPreroll);

// IMediaPosition
public:
	STDMETHOD(get_Duration)( 
		/* [retval][out] */ __RPC__out REFTIME *plength);
	STDMETHOD(put_CurrentPosition)( 
		/* [in] */ REFTIME llTime);
	STDMETHOD(get_CurrentPosition)( 
		/* [retval][out] */ __RPC__out REFTIME *pllTime);
	STDMETHOD(get_StopTime)( 
		/* [retval][out] */ __RPC__out REFTIME *pllTime);
	STDMETHOD(put_StopTime)( 
		/* [in] */ REFTIME llTime);
	STDMETHOD(get_PrerollTime)( 
		/* [retval][out] */ __RPC__out REFTIME *pllTime);
	STDMETHOD(put_PrerollTime)( 
		/* [in] */ REFTIME llTime);
	STDMETHOD(put_Rate)( 
		/* [in] */ double dRate);
	STDMETHOD(get_Rate)( 
		/* [retval][out] */ __RPC__out double *pdRate);
	STDMETHOD(CanSeekForward)( 
		/* [retval][out] */ __RPC__out LONG *pCanSeekForward);
	STDMETHOD(CanSeekBackward)( 
		/* [retval][out] */ __RPC__out LONG *pCanSeekBackward);};

OBJECT_ENTRY_AUTO(__uuidof(Filter), CFilter)
