//
// ShareMonkeyData.h
//
// Copyright © Shareaza Development Team, 2002-2009.
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

#pragma once

#include "ThreadImpl.h"
#include "MetaPanel.h"

class CXMLElement;
class CLibraryFileView;
class CSchema;

class CShareMonkeyData :
	public CMetaPanel,
	public CThreadImpl
{
public:
	CShareMonkeyData(INT_PTR nOffset, int nRequestType = CShareMonkeyData::stProductMatch);
	virtual ~CShareMonkeyData();

protected:
	CCriticalSection	m_pSection;

protected:
	DWORD				m_nFileIndex;
	HINTERNET			m_hInternet;
	HINTERNET			m_hSession;
	HINTERNET			m_hRequest;
	DWORD				m_nDelay;
	DWORD				m_nFailures;
	CSchema*			m_pSchema;
	CXMLElement*		m_pXML;
	CXMLElement*		m_pRazaXML;
	CLibraryFileView*	m_pFileView;
	CString				m_sResponse;
	int					m_nRequestType;
	INT_PTR				m_nOffset;

public:
	CString				m_sURL;
	CString				m_sComparisonURL;
	CString				m_sBuyURL;
	CString				m_sSessionID;
	CString				m_sProductID;
	CString				m_sCategoryID;
	CString				m_sCountry;
	CString				m_sProductName;
	CString				m_sDescription;
	CString				m_sStatus;

// Operations
public:
	BOOL		Start(CLibraryFileView* pView, DWORD nFileIndex);
	void		Stop();
	CSchema*	GetSchema() { return m_pSchema; }

	enum WebRequestType
	{
		stProductMatch,
		stStoreMatch,
		stComparison
	};

protected:
	void			OnRun();
	void			Clear();
	BOOL			BuildRequest();
	BOOL			ExecuteRequest();
	BOOL			DecodeResponse(CString& strMessage);
	BOOL			ImportData(CXMLElement* pRoot);

	inline bool NotifyWindow(LPCTSTR pszMessage = NULL) const;
};
