#pragma once

class CRegEnum : public CComObject
{
public:
	CRegEnum();

	// Create IAutoComplete object and attach HWND to it
	BOOL AttachTo(HWND hWnd);

	// Set registry key and value template
	//	szSection	: HKCU\SOFTWARE\{CompanyKey}\{ApplicationKey}\{szSection}
	//	szRoot		: Must contain inside a "%i" format specifier (n = 1,2..200)
	void SetRegistryKey(LPCTSTR szSection, LPCTSTR szRoot);

	// Add new string to autocomplete list
	void AddString(const CString& rString) const;

protected:
	BEGIN_INTERFACE_PART(EnumString, IEnumString)
		STDMETHOD(Next)(
			/* [in] */ ULONG celt,
			/* [length_is][size_is][out] */ LPOLESTR* rgelt,
			/* [out] */ ULONG *pceltFetched);
		STDMETHOD(Skip)(
			/* [in] */ ULONG celt);
		STDMETHOD(Reset)(void);
		STDMETHOD(Clone)(
			/* [out] */ IEnumString** ppenum);
	END_INTERFACE_PART(EnumString)

	DECLARE_DYNCREATE(CRegEnum)
	DECLARE_OLECREATE(CRegEnum)
	DECLARE_INTERFACE_MAP()

protected:
	CString						m_sect;
	CString						m_root;
	int							m_iter;
	CComPtr< IAutoComplete >	m_pIAutoComplete;
};

class CAutocompleteEdit : public CEdit
{
	DECLARE_DYNCREATE(CAutocompleteEdit)

public:
	CAutocompleteEdit();

	void SetRegistryKey(LPCTSTR szSection, LPCTSTR szRoot);

	virtual int GetWindowText(LPTSTR lpszStringBuf, int nMaxCount) const;
	virtual void GetWindowText(CString& rString) const;

protected:
	CRegEnum m_oData;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()
};
