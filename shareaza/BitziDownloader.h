//
// BitziDownloader.h
//
// Copyright (c) Shareaza Development Team, 2002-2007.
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

#if !defined(AFX_BITZIDOWNLOADER_H__08C21C98_E7E8_4E32_B3E9_2E149A72CE69__INCLUDED_)
#define AFX_BITZIDOWNLOADER_H__08C21C98_E7E8_4E32_B3E9_2E149A72CE69__INCLUDED_

#pragma once

class CSchema;
class CXMLElement;
class CBitziDownloadDlg;


class CBitziDownloader
{
// Construction
public:
	CBitziDownloader();
	virtual ~CBitziDownloader();

// Attributes
protected:
	CList< DWORD, DWORD > m_pFiles;
	CCriticalSection	m_pSection;
	CBitziDownloadDlg*	m_pDlg;
protected:
	HANDLE				m_hThread;
	HINTERNET			m_hInternet;
	HINTERNET			m_hSession;
	HINTERNET			m_hRequest;
	BOOL				m_bFinished;
	DWORD				m_nDelay;
	DWORD				m_nFailures;
protected:
	DWORD				m_nFileIndex;
	CString				m_sFileName;
	CString				m_sFileSHA1;
	CString				m_sFileTiger;
	CString				m_sURL;
	CString				m_sResponse;
	CXMLElement*		m_pXML;

// Operations
public:
	void		AddFile(DWORD nIndex);
	INT_PTR		GetFileCount();
	BOOL		Start(CBitziDownloadDlg* pDlg = NULL);
	void		Stop();
	BOOL		IsWorking();
protected:
	static UINT		ThreadStart(LPVOID pParam);
	void			OnRun();
	BOOL			BuildRequest();
	BOOL			ExecuteRequest();
	BOOL			DecodeResponse();
	CString			LookupValue(LPCTSTR pszPath);
	CXMLElement*	ImportData(CSchema* pSchema);
	BOOL			SubmitMetaData(CXMLElement* pXML);
	BOOL			MergeMetaData(CXMLElement* pOutput, CXMLElement* pInput);

};

#endif // !defined(AFX_BITZIDOWNLOADER_H__08C21C98_E7E8_4E32_B3E9_2E149A72CE69__INCLUDED_)
