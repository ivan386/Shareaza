//
// Free implementation of the MD4 hash algorithm
// MD4C.C - RSA Data Security, Inc., MD4 message-digest algorithm
//

/*
	Copyright (C) 1990-2, RSA Data Security, Inc. All rights reserved.

	License to copy and use this software is granted provided that it
	is identified as the "RSA Data Security, Inc. MD4 Message-Digest
	Algorithm" in all material mentioning or referencing this software
	or this function.

	License is also granted to make and use derivative works provided
	that such works are identified as "derived from the RSA Data
	Security, Inc. MD4 Message-Digest Algorithm" in all material
	mentioning or referencing the derived work.  

	RSA Data Security, Inc. makes no representations concerning either
	the merchantability of this software or the suitability of this
	software for any particular purpose. It is provided "as is"
	without express or implied warranty of any kind.  

	These notices must be retained in any copies of any part of this
	documentation and/or software.  
*/

//
// MD4.cpp
//
// Copyright (c) Shareaza Development Team, 2002-2004.
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

#include "StdAfx.h"
#include "MD4.h"
#include "asm/common.inc"


extern "C" void __stdcall MD4_Add1_p5	(CMD4* pMD4, LPCVOID pData);
extern "C" void __stdcall MD4_Add1_MMX	(CMD4* pMD4, LPCVOID pData);
extern "C" void __stdcall MD4_Add1_SSE2	(CMD4* pMD4, LPCVOID pData);
extern "C" void __stdcall MD4_Add2_p5	(CMD4* pMD41, LPCVOID pData1, CMD4* pMD42, LPCVOID pData2);
extern "C" void __stdcall MD4_Add2_MMX	(CMD4* pMD41, LPCVOID pData1, CMD4* pMD42, LPCVOID pData2);
extern "C" void __stdcall MD4_Add2_SSE2	(CMD4* pMD41, LPCVOID pData1, CMD4* pMD42, LPCVOID pData2);
extern "C" void __stdcall MD4_Add3_p5	(CMD4* pMD41, LPCVOID pData1, CMD4* pMD42, LPCVOID pData2, CMD4* pMD43, LPCVOID pData3);
extern "C" void __stdcall MD4_Add3_MMX	(CMD4* pMD41, LPCVOID pData1, CMD4* pMD42, LPCVOID pData2, CMD4* pMD43, LPCVOID pData3);
extern "C" void __stdcall MD4_Add3_SSE2	(CMD4* pMD41, LPCVOID pData1, CMD4* pMD42, LPCVOID pData2, CMD4* pMD43, LPCVOID pData3);
extern "C" void __stdcall MD4_Add4_p5	(CMD4* pMD41, LPCVOID pData1, CMD4* pMD42, LPCVOID pData2, CMD4* pMD43, LPCVOID pData3, CMD4* pMD44, LPCVOID pData4);
extern "C" void __stdcall MD4_Add4_MMX	(CMD4* pMD41, LPCVOID pData1, CMD4* pMD42, LPCVOID pData2, CMD4* pMD43, LPCVOID pData3, CMD4* pMD44, LPCVOID pData4);
extern "C" void __stdcall MD4_Add4_SSE2	(CMD4* pMD41, LPCVOID pData1, CMD4* pMD42, LPCVOID pData2, CMD4* pMD43, LPCVOID pData3, CMD4* pMD44, LPCVOID pData4);
extern "C" void __stdcall MD4_Add5_p5	(CMD4* pMD41, LPCVOID pData1, CMD4* pMD42, LPCVOID pData2, CMD4* pMD43, LPCVOID pData3, CMD4* pMD44, LPCVOID pData4, CMD4* pMD45, LPCVOID pData5);
extern "C" void __stdcall MD4_Add5_MMX	(CMD4* pMD41, LPCVOID pData1, CMD4* pMD42, LPCVOID pData2, CMD4* pMD43, LPCVOID pData3, CMD4* pMD44, LPCVOID pData4, CMD4* pMD45, LPCVOID pData5);
extern "C" void __stdcall MD4_Add5_SSE2	(CMD4* pMD41, LPCVOID pData1, CMD4* pMD42, LPCVOID pData2, CMD4* pMD43, LPCVOID pData3, CMD4* pMD44, LPCVOID pData4, CMD4* pMD45, LPCVOID pData5);
extern "C" void __stdcall MD4_Add6_p5	(CMD4* pMD41, LPCVOID pData1, CMD4* pMD42, LPCVOID pData2, CMD4* pMD43, LPCVOID pData3, CMD4* pMD44, LPCVOID pData4, CMD4* pMD45, LPCVOID pData5, CMD4* pMD46, LPCVOID pData6);
extern "C" void __stdcall MD4_Add6_MMX	(CMD4* pMD41, LPCVOID pData1, CMD4* pMD42, LPCVOID pData2, CMD4* pMD43, LPCVOID pData3, CMD4* pMD44, LPCVOID pData4, CMD4* pMD45, LPCVOID pData5, CMD4* pMD46, LPCVOID pData6);
extern "C" void __stdcall MD4_Add6_SSE2	(CMD4* pMD41, LPCVOID pData1, CMD4* pMD42, LPCVOID pData2, CMD4* pMD43, LPCVOID pData3, CMD4* pMD44, LPCVOID pData4, CMD4* pMD45, LPCVOID pData5, CMD4* pMD46, LPCVOID pData6);

BYTE CMD4::MD4_PADDING[64] =
{
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

CMD4::tpAdd1 CMD4::pAdd1 = &MD4_Add1_p5;
CMD4::tpAdd2 CMD4::pAdd2 = &MD4_Add2_p5;
CMD4::tpAdd3 CMD4::pAdd3 = &MD4_Add3_p5;
CMD4::tpAdd4 CMD4::pAdd4 = &MD4_Add4_p5;
CMD4::tpAdd5 CMD4::pAdd5 = &MD4_Add5_p5;
CMD4::tpAdd6 CMD4::pAdd6 = &MD4_Add6_p5;

void CMD4::Init()
{
	if ( SupportsSSE2() )
	{
		pAdd1 = &MD4_Add1_SSE2;
		pAdd2 = &MD4_Add2_SSE2;
		pAdd3 = &MD4_Add3_SSE2;
		pAdd4 = &MD4_Add4_SSE2;
		pAdd5 = &MD4_Add5_SSE2;
		pAdd6 = &MD4_Add6_SSE2;
	}
	else if ( SupportsMMX() )
	{
		pAdd1 = &MD4_Add1_MMX;
		pAdd2 = &MD4_Add2_MMX;
		pAdd3 = &MD4_Add3_MMX;
		pAdd4 = &MD4_Add4_MMX;
		pAdd5 = &MD4_Add5_MMX;
		pAdd6 = &MD4_Add6_MMX;
	}
	else
	{
		pAdd1 = &MD4_Add1_p5;
		pAdd2 = &MD4_Add2_p5;
		pAdd3 = &MD4_Add3_p5;
		pAdd4 = &MD4_Add4_p5;
		pAdd5 = &MD4_Add5_p5;
		pAdd6 = &MD4_Add6_p5;
	}
}