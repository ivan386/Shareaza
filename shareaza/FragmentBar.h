//
// FragmentBar.h
//
// Copyright (c) Shareaza Development Team, 2002-2005.
// This file is part of SHAREAZA (www.shareaza.com)
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

#if !defined(AFX_FRAGMENTBAR_H__7BAEB279_99A8_411E_AE38_C45853059F19__INCLUDED_)
#define AFX_FRAGMENTBAR_H__7BAEB279_99A8_411E_AE38_C45853059F19__INCLUDED_

#pragma once

class CDownload;
class CDownloadSource;
class CUpload;
class CUploadFile;


class CFragmentBar
{
// Operations
public:
	static void DrawFragment(CDC* pDC, CRect* prcCell, QWORD nTotal, QWORD nOffset, QWORD nLength, COLORREF crFill, BOOL b3D);
	static void DrawStateBar(CDC* pDC, CRect* prcBar, QWORD nTotal, QWORD nOffset, QWORD nLength, COLORREF crFill, BOOL bTop = FALSE);
	static void DrawDownload(CDC* pDC, CRect* prcBar, CDownload* pDownload, COLORREF crNatural);
	static void DrawSource(CDC* pDC, CRect* prcBar, CDownloadSource* pSource, COLORREF crNatural);
	static void DrawUpload(CDC* pDC, CRect* prcBar, CUploadFile* pFile, COLORREF crNatural);
protected:
	static void DrawSourceImpl(CDC* pDC, CRect* prcBar, CDownloadSource* pSource);

};

#endif // !defined(AFX_FRAGMENTBAR_H__7BAEB279_99A8_411E_AE38_C45853059F19__INCLUDED_)
