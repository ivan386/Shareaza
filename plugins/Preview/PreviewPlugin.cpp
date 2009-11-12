// PreviewPlugin.cpp : Implementation of CPreviewPlugin

#include "stdafx.h"
#include "PreviewPlugin.h"
#include "ConfigDlg.h"

// CPreviewPlugin

CPreviewPlugin::CPreviewPlugin()
	: m_pUnkMarshaler( NULL )
{
}

HRESULT CPreviewPlugin::FinalConstruct()
{
	return CoCreateFreeThreadedMarshaler( GetControllingUnknown(), &m_pUnkMarshaler.p );
}

void CPreviewPlugin::FinalRelease()
{
	m_pApp.Release();
	m_pSite.Release();
	m_pUnkMarshaler.Release();
}

// IGeneralPlugin

STDMETHODIMP CPreviewPlugin::SetApplication(/* [in] */ IApplication* pApplication)
{
	m_pApp = pApplication;

	return S_OK;
}

STDMETHODIMP CPreviewPlugin::QueryCapabilities(/* [out] */ DWORD* /* pnCaps */)
{
	return S_OK;
}

STDMETHODIMP CPreviewPlugin::Configure(void)
{
	CConfigDlg dlg;
	dlg.DoModal();

	return S_OK;
}

STDMETHODIMP CPreviewPlugin::OnSkinChanged(void)
{
	return S_OK;
}

// IDownloadPreviewPlugin

STDMETHODIMP CPreviewPlugin::SetSite(/* [in] */ IDownloadPreviewSite* pSite)
{
	m_pSite = pSite;

	return S_OK;
}

STDMETHODIMP CPreviewPlugin::Preview(/* [in] */ HANDLE /* hFile */, /* [in] */ BSTR /* sTarget */)
{
	// TODO: use GetFileInformationByHandleEx (Vista only)

	return E_NOTIMPL;
}

STDMETHODIMP CPreviewPlugin::Cancel()
{
	return E_NOTIMPL;
}

// IDownloadPreviewPlugin2

STDMETHODIMP CPreviewPlugin::Preview2(/* [in] */ BSTR sSource, /* [in] */ BSTR sTarget)
{
	if ( ! sSource || ! sTarget )
		return E_POINTER;

	return E_NOTIMPL;
}
