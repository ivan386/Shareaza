/*
 ---------------------------------------------------------------------------
 Copyright (c) 2002, Dr Brian Gladman <brg@gladman.me.uk>, Worcester, UK.
 All rights reserved.

 LICENSE TERMS

 The free distribution and use of this software in both source and binary 
 form is allowed (with or without changes) provided that:

   1. distributions of this source code include the above copyright 
      notice, this list of conditions and the following disclaimer;

   2. distributions in binary form include the above copyright
      notice, this list of conditions and the following disclaimer
      in the documentation and/or other associated materials;

   3. the copyright holder's name is not used to endorse products 
      built using this software without specific written permission. 

 ALTERNATIVELY, provided that this notice is retained in full, this product
 may be distributed under the terms of the GNU General Public License (GPL),
 in which case the provisions of the GPL apply INSTEAD OF those given above.
 
 DISCLAIMER

 This software is provided 'as is' with no explicit or implied warranties
 in respect of its properties, including, but not limited to, correctness 
 and/or fitness for purpose.
 ---------------------------------------------------------------------------
 Issue Date: 30/11/2002

 This is a byte oriented version of SHA1 that operates on arrays of bytes
 stored in memory. It runs at 22 cycles per byte on a Pentium P4 processor
*/

//
// SHA.cpp
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
#include "SHA.h"
#include "asm/common.inc"


extern "C" void __stdcall SHA_Add1_p5(CSHA1* pSHA, LPCVOID pData);
extern "C" void __stdcall SHA_Add1_MMX(CSHA1* pSHA, LPCVOID pData);
extern "C" void __stdcall SHA_Add1_SSE2(CSHA1* pSHA, LPCVOID pData);
extern "C" void __stdcall SHA_Add2_p5(CSHA1* pSHA1, LPCVOID pData1, CSHA1* pSHA2, LPCVOID pData2);
extern "C" void __stdcall SHA_Add2_MMX(CSHA1* pSHA1, LPCVOID pData1, CSHA1* pSHA2, LPCVOID pData2);
extern "C" void __stdcall SHA_Add2_SSE2(CSHA1* pSHA1, LPCVOID pData1, CSHA1* pSHA2, LPCVOID pData2);
extern "C" void __stdcall SHA_Add3_p5(CSHA1* pSHA1, LPCVOID pData1, CSHA1* pSHA2, LPCVOID pData2, CSHA1* pSHA3, LPCVOID pData3);
extern "C" void __stdcall SHA_Add3_MMX(CSHA1* pSHA1, LPCVOID pData1, CSHA1* pSHA2, LPCVOID pData2, CSHA1* pSHA3, LPCVOID pData3);
extern "C" void __stdcall SHA_Add3_SSE2(CSHA1* pSHA1, LPCVOID pData1, CSHA1* pSHA2, LPCVOID pData2, CSHA1* pSHA3, LPCVOID pData3);
extern "C" void __stdcall SHA_Add4_p5(CSHA1* pSHA1, LPCVOID pData1, CSHA1* pSHA2, LPCVOID pData2, CSHA1* pSHA3, LPCVOID pData3, CSHA1* pSHA4, LPCVOID pData4);
extern "C" void __stdcall SHA_Add4_MMX(CSHA1* pSHA1, LPCVOID pData1, CSHA1* pSHA2, LPCVOID pData2, CSHA1* pSHA3, LPCVOID pData3, CSHA1* pSHA4, LPCVOID pData4);
extern "C" void __stdcall SHA_Add4_SSE2(CSHA1* pSHA1, LPCVOID pData1, CSHA1* pSHA2, LPCVOID pData2, CSHA1* pSHA3, LPCVOID pData3, CSHA1* pSHA4, LPCVOID pData4);
extern "C" void __stdcall SHA_Add5_p5(CSHA1* pSHA1, LPCVOID pData1, CSHA1* pSHA2, LPCVOID pData2, CSHA1* pSHA3, LPCVOID pData3, CSHA1* pSHA4, LPCVOID pData4, CSHA1* pSHA5, LPCVOID pData5);
extern "C" void __stdcall SHA_Add5_MMX(CSHA1* pSHA1, LPCVOID pData1, CSHA1* pSHA2, LPCVOID pData2, CSHA1* pSHA3, LPCVOID pData3, CSHA1* pSHA4, LPCVOID pData4, CSHA1* pSHA5, LPCVOID pData5);
extern "C" void __stdcall SHA_Add5_SSE2(CSHA1* pSHA1, LPCVOID pData1, CSHA1* pSHA2, LPCVOID pData2, CSHA1* pSHA3, LPCVOID pData3, CSHA1* pSHA4, LPCVOID pData4, CSHA1* pSHA5, LPCVOID pData5);
extern "C" void __stdcall SHA_Add6_p5(CSHA1* pSHA1, LPCVOID pData1, CSHA1* pSHA2, LPCVOID pData2, CSHA1* pSHA3, LPCVOID pData3, CSHA1* pSHA4, LPCVOID pData4, CSHA1* pSHA5, LPCVOID pData5, CSHA1* pSHA6, LPCVOID pData6);
extern "C" void __stdcall SHA_Add6_MMX(CSHA1* pSHA1, LPCVOID pData1, CSHA1* pSHA2, LPCVOID pData2, CSHA1* pSHA3, LPCVOID pData3, CSHA1* pSHA4, LPCVOID pData4, CSHA1* pSHA5, LPCVOID pData5, CSHA1* pSHA6, LPCVOID pData6);
extern "C" void __stdcall SHA_Add6_SSE2(CSHA1* pSHA1, LPCVOID pData1, CSHA1* pSHA2, LPCVOID pData2, CSHA1* pSHA3, LPCVOID pData3, CSHA1* pSHA4, LPCVOID pData4, CSHA1* pSHA5, LPCVOID pData5, CSHA1* pSHA6, LPCVOID pData6);

BYTE CSHA1::SHA_PADDING[64] =
	{
		0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

CSHA1::tpAdd1 CSHA1::pAdd1 = &SHA_Add1_p5;
CSHA1::tpAdd2 CSHA1::pAdd2 = &SHA_Add2_p5;
CSHA1::tpAdd3 CSHA1::pAdd3 = &SHA_Add3_p5;
CSHA1::tpAdd4 CSHA1::pAdd4 = &SHA_Add4_p5;
CSHA1::tpAdd5 CSHA1::pAdd5 = &SHA_Add5_p5;
CSHA1::tpAdd6 CSHA1::pAdd6 = &SHA_Add6_p5;

void CSHA1::Init()
{
	if ( SupportsSSE2() )
	{
		pAdd1 = &SHA_Add1_SSE2;
		pAdd2 = &SHA_Add2_SSE2;
		pAdd3 = &SHA_Add3_SSE2;
		pAdd4 = &SHA_Add4_SSE2;
		pAdd5 = &SHA_Add5_SSE2;
		pAdd6 = &SHA_Add6_SSE2;
	}
	else if ( SupportsMMX() )
	{
		pAdd1 = &SHA_Add1_MMX;
		pAdd2 = &SHA_Add2_MMX;
		pAdd3 = &SHA_Add3_MMX;
		pAdd4 = &SHA_Add4_MMX;
		pAdd5 = &SHA_Add5_MMX;
		pAdd6 = &SHA_Add6_MMX;
	}
	else
	{
		pAdd1 = &SHA_Add1_p5;
		pAdd2 = &SHA_Add2_p5;
		pAdd3 = &SHA_Add3_p5;
		pAdd4 = &SHA_Add4_p5;
		pAdd5 = &SHA_Add5_p5;
		pAdd6 = &SHA_Add6_p5;
	}
}