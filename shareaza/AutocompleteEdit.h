#pragma once

class CAutocompleteEdit : public CEdit
{
	DECLARE_DYNAMIC(CAutocompleteEdit)

public:
	CAutocompleteEdit();

	virtual int GetWindowText(LPTSTR lpszStringBuf, int nMaxCount) const;
	virtual void GetWindowText(CString& rString) const;

	void SetRegistryKey(LPCTSTR szSection, LPCTSTR szRoot);

protected:
	class CEnumString : public CComObject, public CStringList
	{
	public:
		CEnumString();
		
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

		DECLARE_OLECREATE(CEnumString)
		DECLARE_INTERFACE_MAP()

	public:
		CString		m_sect;
		CString		m_root;
		size_t		m_iter;

		void AddString(CString& rString) const;
	};

	CEnumString					m_oData;
	CComPtr< IAutoComplete >	m_pIAutoComplete;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()
};
