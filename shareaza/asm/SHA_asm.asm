; #####################################################################################################################
;
; SHA_asm.asm
;
; Copyright (c) Shareaza Development Team, 2002-2004.
; This file is part of SHAREAZA (www.shareaza.com)
;
; Shareaza is free software; you can redistribute it
; and/or modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version 2 of
; the License, or (at your option) any later version.
;
; Shareaza is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with Shareaza; if not, write to the Free Software
; Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
;
; #####################################################################################################################
;
; SHA_asm - Implementation of SHA-1 for x86
;
; created				4.7.2004		by Camper			thetruecamper at gmx dot net
;
; modified				2.9.2004		by Camper			ICQ # 105491945
;
; #####################################################################################################################

                        .586p
                        .model      flat, stdcall 
                        option      casemap:none                    ; case sensitive
                        option      prologue:none                   ; we generate our own entry/exit code
                        option      epilogue:none

; #####################################################################################################################

						include		<common.inc>

m_nHash0				equ			0								; offsets as found in SHA.h
m_nHash1				equ			4
m_nHash2				equ			8
m_nHash3				equ			12
m_nHash4				equ			16

m_nCount0				equ			24
m_nCount1				equ			28

m_nBuffer				equ         32

						.data
						
						ALIGN		16
const_FFFFFFFFFFFFFFFF	dq			0FFFFFFFFFFFFFFFFH
						dq			0FFFFFFFFFFFFFFFFH
const_5A8279995A827999	dq			05A8279995A827999H
						dq			05A8279995A827999H
const_6ED9EBA16ED9EBA1	dq			06ED9EBA16ED9EBA1H
						dq			06ED9EBA16ED9EBA1H
const_8F1BBCDC8F1BBCDC	dq			08F1BBCDC8F1BBCDCH
						dq			08F1BBCDC8F1BBCDCH
const_CA62C1D6CA62C1D6	dq			0CA62C1D6CA62C1D6H
						dq			0CA62C1D6CA62C1D6H

RND_CH                  MACRO       const:REQ
; t=a; a=rotl32(a,5)+e+k+w[i]+((b&c)^(~b&d)); e=d; d=c; c=rotl32(b,30); b=t
;      a=rotl32(a,5)+e+k+w[i]+(d^(b&(c^d)));
                        mov         reg_temp1, reg_a                        ; t=a
                        mov			reg_temp2, reg_c
                        rol         reg_a, 5
                        xor			reg_temp2, reg_d
                        add         reg_a, reg_e
                        and			reg_temp2, reg_b
                        add			reg_a, const
						xor			reg_temp2, reg_d
                        add         reg_a, [_w+count*4]
                        ror			reg_b, 2
                        add			reg_a, reg_temp2
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d
reg_d                   textequ     reg_c
reg_c					textequ     reg_b
reg_b                   textequ     reg_temp1
reg_temp1               textequ     reg_t
count                   =           count + 1
                        ENDM                                                ; RND_CH

RND_PARITY              MACRO       const:REQ
; t=a; a=rotl32(a,5)+e+k+w[i]+(b^c^d); e=d; d=c; c=rotl32(b,30); b=t
                        mov         reg_temp1, reg_a                        ; t=a
                        rol         reg_a, 5
                        mov			reg_temp2, reg_d
                        add         reg_a, reg_e
                        xor			reg_temp2, reg_c
                        add			reg_a, const
                        xor			reg_temp2, reg_b
                        add			reg_a, [_w+count*4]
                        ror			reg_b, 2
                        add			reg_a, reg_temp2
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d                                   ; e=d
reg_d                   textequ     reg_c                                   ; d=c
reg_c                   textequ     reg_b                                   ; c=rotl(b,30)
reg_b                   textequ     reg_temp1                               ; b=t
reg_temp1               textequ     reg_t
count                   =           count + 1
                        ENDM                                                ; RND_PARITY

RND_MAJ                 MACRO       const:REQ
; t=a; a=rotl32(a,5)+e+k+w[i]+((b&c)^(b&d)^(c&d)); e=d; d=c; c=rotl32(b,30); b=t
;      a=rotl32(a,5)+e+k+w[i]+((c&d)^(b&(c^d)))
						mov			reg_temp2, reg_d
						mov			reg_temp1, reg_a
						rol			reg_a, 5
						xor			reg_temp2, reg_c
						add			reg_a, reg_e
						and			reg_temp2, reg_b
						add			reg_a, const
						mov			reg_e, reg_c
						add			reg_a, [_w+count*4]
						and			reg_e, reg_d
						xor			reg_temp2, reg_e
						ror			reg_b, 2
						add			reg_a, reg_temp2
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d
reg_d                   textequ     reg_c
reg_c                   textequ     reg_b
reg_b					textequ		reg_temp1
reg_temp1				textequ		reg_t
count                   =           count + 1
                        ENDM                                                ; RND_MAJ
                        
INIT_REG_ALIAS          MACRO
reg_i_1                 textequ     <eax>
reg_i_2                 textequ     <ebx>
reg_i_3                 textequ     <ecx>
reg_i_15                textequ     <edx>
reg_i_16                textequ     <esi>
                        ENDM
                        
                        .code

                        ALIGN       16

SHA_Compile_p5          PROC

__this                  textequ     <[esp+40+320]>                       ; pusha + 2 * ret addr in between
_w                      textequ     <esp+4>

                        INIT_REG_ALIAS

count                   =           0
                        REPEAT      16
                        IF          count eq 0
                        mov         reg_i_16, [ebp+count*4]
                        bswap       reg_i_16
                        mov         [_w+count*4], reg_i_16
                        ELSEIF      count eq 1
                        mov         reg_i_15, [ebp+count*4]
                        bswap       reg_i_15
                        mov         [_w+count*4], reg_i_15
                        ELSEIF      count eq 13
                        mov         reg_i_3, [ebp+count*4]
                        bswap       reg_i_3
                        mov         [_w+count*4], reg_i_3
                        ELSEIF      count eq 14
                        mov         reg_i_2, [ebp+count*4]
                        bswap       reg_i_2
                        mov         [_w+count*4], reg_i_2
                        ELSE
                        mov         reg_i_1, [ebp+count*4]
                        bswap       reg_i_1
                        mov         [_w+count*4], reg_i_1
                        ENDIF
count                   =           count + 1
                        ENDM
count                   =           16
                        REPEAT      64
                        xor         reg_i_3, reg_i_16                       ; w[i-16]^w[i-3]
reg_i_14                textequ     reg_i_16                                ; we forget w[i-16]
                        IF          count le 77
                        mov         reg_i_14, [_w+(count-14)*4]
                        xor         reg_i_3, reg_i_14
                        ELSE
                        xor         reg_i_3, [_w+(count-14)*4]
                        ENDIF
                        xor         reg_i_3, [_w+(count-8)*4]
                        rol         reg_i_3, 1
                        mov         [_w+count*4], reg_i_3
;now we prepare for the next iteration                        
reg_i_0                 textequ     reg_i_3
reg_i_3                 textequ     reg_i_2
reg_i_2                 textequ     reg_i_1
reg_i_1                 textequ     reg_i_0
reg_i_16                textequ     reg_i_15
reg_i_15                textequ     reg_i_14
count                   =           count + 1
                        ENDM

reg_a                   textequ     <eax>
reg_b                   textequ     <ebx>
reg_c                   textequ     <ecx>
reg_d                   textequ     <edx>
reg_e                   textequ     <esi>
reg_temp1               textequ     <edi>
reg_temp2               textequ     <ebp>

                        mov         reg_temp2, __this
                        mov         reg_a, [reg_temp2+m_nHash0]
                        mov         reg_b, [reg_temp2+m_nHash1]
                        mov         reg_c, [reg_temp2+m_nHash2]
                        mov         reg_d, [reg_temp2+m_nHash3]
                        mov         reg_e, [reg_temp2+m_nHash4]

count                   =           0

                        REPEAT      20
                        RND_CH      05a827999H
                        ENDM
                        REPEAT      20
                        RND_PARITY  06ed9eba1H
                        ENDM
                        REPEAT      20
                        RND_MAJ     08f1bbcdcH
                        ENDM
                        REPEAT      20
                        RND_PARITY  0ca62c1d6H
                        ENDM

                        mov         reg_temp2, __this
                        add         [reg_temp2+m_nHash0], reg_a
                        add         [reg_temp2+m_nHash1], reg_b
                        add         [reg_temp2+m_nHash2], reg_c
                        add         [reg_temp2+m_nHash3], reg_d
                        add         [reg_temp2+m_nHash4], reg_e
 
                        ret

SHA_Compile_p5          ENDP

                        ALIGN       16

SHA_Add_p5              PROC        PUBLIC, _this:DWORD, _Data:DWORD, _nLength:DWORD

                        pusha
__this                  textequ     <[esp+36+320]>                              ; different offset due to pusha
__Data                  textequ     <[esp+40+320]>
__nLength               textequ     <[esp+44+320]>

                        sub         esp, 320

                        mov         ecx, __nLength
                        and         ecx, ecx
                        jz          get_out
                        xor         edx, edx
                        mov         ebp, __Data
                        mov         edi, __this
                        mov         ebx, [edi+m_nCount0]
                        mov         eax, ebx
                        add         ebx, ecx
                        mov         [edi+m_nCount0], ebx
                        adc         [edi+m_nCount1], edx

                        and         eax, 63
                        jnz         partial_buffer
full_blocks:            mov         ecx, __nLength
                        and         ecx, ecx
                        jz          get_out
                        sub         ecx, 64
                        jb          end_of_stream
                        mov         __nLength, ecx
                        call        SHA_Compile_p5
                        mov         ebp, __Data
                        add         ebp, 64
                        mov         __Data, ebp
                        jmp         full_blocks

end_of_stream:          mov         edi, __this
                        mov         esi, ebp
                        lea         edi, [edi+m_nBuffer]
                        add         ecx, 64
                        rep movsb
                        jmp         get_out

partial_buffer:         add         ecx, eax                                ; eax = offset in buffer, ecx = _nLength
                        cmp         ecx, 64
                        jb          short_stream                            ; we can't fill the buffer
                        mov         ecx, -64
                        add         ecx, eax
                        add         __nLength, ecx                          ; _nlength += (offset-64)
@@:                     mov         bl, [ebp]
                        inc         ebp
                        mov         byte ptr [edi+m_nBuffer+64+ecx], bl
                        inc         ecx
                        jnz         @B                                      ; offset = 64
                        mov         __Data, ebp
                        lea         ebp, [edi+m_nBuffer]
                        call        SHA_Compile_p5
                        mov         ebp, __Data
                        jmp         full_blocks

short_stream:           sub         ecx, eax                                ;  --> ecx=_nLength
                        mov         esi, ebp
                        lea         edi, [edi+m_nBuffer+eax]
                        rep movsb

get_out:                add         esp, 320
                        popa
                        ret 12

SHA_Add_p5              ENDP

SHA_Add1_SSE2			PROC		PUBLIC, _this:DWORD, _Data:DWORD
SHA_Add1_SSE2			ENDP
SHA_Add1_MMX			PROC		PUBLIC, _this:DWORD, _Data:DWORD
SHA_Add1_MMX			ENDP
SHA_Add1_p5				PROC		PUBLIC, _this:DWORD, _Data:DWORD
                        pusha
__this                  textequ     <[esp+36+320]>
__Data                  textequ     <[esp+40+320]>
__nLength				textequ		<dword ptr [esp+12+320]>
                        sub         esp, 320
                        mov         ebp, __Data
                        mov         edi, __this
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			edx, edx
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], edx
						mov         __nLength, ecx
full_blocks:			call        SHA_Compile_p5
                        mov         ebp, __Data
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data, ebp
                        jnz			full_blocks
						add			esp, 320
                        popa
                        ret 8
SHA_Add1_p5				ENDP

SHA_Add2_MMX			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD
SHA_Add2_MMX			ENDP
SHA_Add2_p5				PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD
                        pusha
__this1                 textequ     <[esp+36+320]>
__Data1                 textequ     <[esp+40+320]>
__this2                 textequ     <[esp+44+320]>
__Data2                 textequ     <[esp+48+320]>
__nLength				textequ		<dword ptr [esp+12+320]>
                        sub         esp, 320
                        mov         ebp, __Data1
                        mov         edi, __this1
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			edx, edx
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], edx
						mov         __nLength, ecx
full_blocks1:			call        SHA_Compile_p5
                        mov         ebp, __Data1
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data1, ebp
                        jnz			full_blocks1

                        mov         ebp, __Data2
                        mov         edi, __this2
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			edx, edx
                        mov			__this1, edi
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], edx
						mov         __nLength, ecx
full_blocks2:			call        SHA_Compile_p5
                        mov         ebp, __Data2
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data2, ebp
                        jnz			full_blocks2

						add			esp, 320
                        popa
                        ret 16
SHA_Add2_p5				ENDP

SHA_Add3_p5				PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD
                        pusha
__this1                 textequ     <[esp+36+320]>
__Data1                 textequ     <[esp+40+320]>
__this2                 textequ     <[esp+44+320]>
__Data2                 textequ     <[esp+48+320]>
__this3                 textequ     <[esp+52+320]>
__Data3                 textequ     <[esp+56+320]>
__nLength				textequ		<dword ptr [esp+12+320]>
                        sub         esp, 320
                        mov         ebp, __Data1
                        mov         edi, __this1
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			edx, edx
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], edx
						mov         __nLength, ecx
full_blocks1:			call        SHA_Compile_p5
                        mov         ebp, __Data1
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data1, ebp
                        jnz			full_blocks1

                        mov         ebp, __Data2
                        mov         edi, __this2
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			edx, edx
                        mov			__this1, edi
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], edx
						mov         __nLength, ecx
full_blocks2:			call        SHA_Compile_p5
                        mov         ebp, __Data2
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data2, ebp
                        jnz			full_blocks2

                        mov         ebp, __Data3
                        mov         edi, __this3
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			edx, edx
                        mov			__this1, edi
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], edx
						mov         __nLength, ecx
full_blocks3:			call        SHA_Compile_p5
                        mov         ebp, __Data3
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data3, ebp
                        jnz			full_blocks3

						add			esp, 320
                        popa
                        ret 24
SHA_Add3_p5				ENDP

SHA_Add4_p5				PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD, _this4:DWORD, _Data4:DWORD
                        pusha
__this1                 textequ     <[esp+36+320]>
__Data1                 textequ     <[esp+40+320]>
__this2                 textequ     <[esp+44+320]>
__Data2                 textequ     <[esp+48+320]>
__this3                 textequ     <[esp+52+320]>
__Data3                 textequ     <[esp+56+320]>
__this4                 textequ     <[esp+60+320]>
__Data4                 textequ     <[esp+64+320]>
__nLength				textequ		<dword ptr [esp+12+320]>
                        sub         esp, 320
                        mov         ebp, __Data1
                        mov         edi, __this1
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			edx, edx
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], edx
						mov         __nLength, ecx
full_blocks1:			call        SHA_Compile_p5
                        mov         ebp, __Data1
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data1, ebp
                        jnz			full_blocks1

                        mov         ebp, __Data2
                        mov         edi, __this2
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			edx, edx
                        mov			__this1, edi
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], edx
						mov         __nLength, ecx
full_blocks2:			call        SHA_Compile_p5
                        mov         ebp, __Data2
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data2, ebp
                        jnz			full_blocks2

                        mov         ebp, __Data3
                        mov         edi, __this3
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			edx, edx
                        mov			__this1, edi
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], edx
						mov         __nLength, ecx
full_blocks3:			call        SHA_Compile_p5
                        mov         ebp, __Data3
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data3, ebp
                        jnz			full_blocks3

                        mov         ebp, __Data4
                        mov         edi, __this4
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			edx, edx
                        mov			__this1, edi
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], edx
						mov         __nLength, ecx
full_blocks4:			call        SHA_Compile_p5
                        mov         ebp, __Data4
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data4, ebp
                        jnz			full_blocks4

						add			esp, 320
                        popa
                        ret 32
SHA_Add4_p5				ENDP

SHA_Add5_MMX			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD, _this4:DWORD, _Data4:DWORD, _this5:DWORD, _Data5:DWORD
SHA_Add5_MMX			ENDP
SHA_Add5_p5				PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD, _this4:DWORD, _Data4:DWORD, _this5:DWORD, _Data5:DWORD
                        pusha
__this1                 textequ     <[esp+36+320]>
__Data1                 textequ     <[esp+40+320]>
__this2                 textequ     <[esp+44+320]>
__Data2                 textequ     <[esp+48+320]>
__this3                 textequ     <[esp+52+320]>
__Data3                 textequ     <[esp+56+320]>
__this4                 textequ     <[esp+60+320]>
__Data4                 textequ     <[esp+64+320]>
__this5                 textequ     <[esp+68+320]>
__Data5                 textequ     <[esp+72+320]>
__nLength				textequ		<dword ptr [esp+12+320]>
                        sub         esp, 320
                        mov         ebp, __Data1
                        mov         edi, __this1
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			edx, edx
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], edx
						mov         __nLength, ecx
full_blocks1:			call        SHA_Compile_p5
                        mov         ebp, __Data1
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data1, ebp
                        jnz			full_blocks1

                        mov         ebp, __Data2
                        mov         edi, __this2
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			edx, edx
                        mov			__this1, edi
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], edx
						mov         __nLength, ecx
full_blocks2:			call        SHA_Compile_p5
                        mov         ebp, __Data2
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data2, ebp
                        jnz			full_blocks2

                        mov         ebp, __Data3
                        mov         edi, __this3
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			edx, edx
                        mov			__this1, edi
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], edx
						mov         __nLength, ecx
full_blocks3:			call        SHA_Compile_p5
                        mov         ebp, __Data3
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data3, ebp
                        jnz			full_blocks3

                        mov         ebp, __Data4
                        mov         edi, __this4
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			edx, edx
                        mov			__this1, edi
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], edx
						mov         __nLength, ecx
full_blocks4:			call        SHA_Compile_p5
                        mov         ebp, __Data4
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data4, ebp
                        jnz			full_blocks4

                        mov         ebp, __Data5
                        mov         edi, __this5
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			edx, edx
                        mov			__this1, edi
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], edx
						mov         __nLength, ecx
full_blocks5:			call        SHA_Compile_p5
                        mov         ebp, __Data5
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data5, ebp
                        jnz			full_blocks5

						add			esp, 320
                        popa
                        ret 40
SHA_Add5_p5				ENDP

SHA_Add6_MMX			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD, _this4:DWORD, _Data4:DWORD, _this5:DWORD, _Data5:DWORD, _this6:DWORD, _Data6:DWORD
SHA_Add6_MMX			ENDP
SHA_Add6_p5				PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD, _this4:DWORD, _Data4:DWORD, _this5:DWORD, _Data5:DWORD, _this6:DWORD, _Data6:DWORD
                        pusha
__this1                 textequ     <[esp+36+320]>
__Data1                 textequ     <[esp+40+320]>
__this2                 textequ     <[esp+44+320]>
__Data2                 textequ     <[esp+48+320]>
__this3                 textequ     <[esp+52+320]>
__Data3                 textequ     <[esp+56+320]>
__this4                 textequ     <[esp+60+320]>
__Data4                 textequ     <[esp+64+320]>
__this5                 textequ     <[esp+68+320]>
__Data5                 textequ     <[esp+72+320]>
__this6                 textequ     <[esp+76+320]>
__Data6                 textequ     <[esp+80+320]>
__nLength				textequ		<dword ptr [esp+12+320]>
                        sub         esp, 320
                        mov         ebp, __Data1
                        mov         edi, __this1
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			edx, edx
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], edx
						mov         __nLength, ecx
full_blocks1:			call        SHA_Compile_p5
                        mov         ebp, __Data1
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data1, ebp
                        jnz			full_blocks1

                        mov         ebp, __Data2
                        mov         edi, __this2
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			edx, edx
                        mov			__this1, edi
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], edx
						mov         __nLength, ecx
full_blocks2:			call        SHA_Compile_p5
                        mov         ebp, __Data2
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data2, ebp
                        jnz			full_blocks2

                        mov         ebp, __Data3
                        mov         edi, __this3
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			edx, edx
                        mov			__this1, edi
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], edx
						mov         __nLength, ecx
full_blocks3:			call        SHA_Compile_p5
                        mov         ebp, __Data3
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data3, ebp
                        jnz			full_blocks3

                        mov         ebp, __Data4
                        mov         edi, __this4
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			edx, edx
                        mov			__this1, edi
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], edx
						mov         __nLength, ecx
full_blocks4:			call        SHA_Compile_p5
                        mov         ebp, __Data4
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data4, ebp
                        jnz			full_blocks4

                        mov         ebp, __Data5
                        mov         edi, __this5
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			edx, edx
                        mov			__this1, edi
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], edx
						mov         __nLength, ecx
full_blocks5:			call        SHA_Compile_p5
                        mov         ebp, __Data5
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data5, ebp
                        jnz			full_blocks5

                        mov         ebp, __Data6
                        mov         edi, __this6
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			edx, edx
                        mov			__this1, edi
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], edx
						mov         __nLength, ecx
full_blocks6:			call        SHA_Compile_p5
                        mov         ebp, __Data6
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data6, ebp
                        jnz			full_blocks6

						add			esp, 320
                        popa
                        ret 48
SHA_Add6_p5				ENDP

						.xmm

xmm_a					textequ		<xmm0>
xmm_b					textequ		<xmm1>
xmm_c					textequ		<xmm2>
xmm_d					textequ		<xmm3>
xmm_e					textequ		<xmm4>
xmm_temp1				textequ		<xmm5>
xmm_temp2				textequ		<xmm6>
xmm_temp3				textequ		<xmm7>

RND_CH_SSE2				MACRO       const:REQ
; t=rotl32(a,5)+e+k+w[i]+((b&c)^(~b&d)); e=d; d=c; c=rotl32(b,30); b=a; a=t
; t=rotl32(a,5)+e+k+w[i]+(d^(b&(c^d)));
						pshufd		xmm_temp3, xmm_a, 10100000b						; rotate not native to MMX
						movdqa		xmm_temp1, xmm_c
						psrlq		xmm_temp3, 32-5
						pxor		xmm_temp1, xmm_d
						paddd		xmm_temp3, xmm_e
						pand		xmm_temp1, xmm_b
						IF			const eq 05A827999H
						paddd		xmm_temp3, const_5A8279995A827999
						ELSE
						.ERR
						ENDIF
						pxor		xmm_temp1, xmm_d
						paddd		xmm_temp3, [__w+count*16]
						pshufd		xmm_b, xmm_b, 10100000b
						paddd		xmm_temp3, xmm_temp1
						psrlq		xmm_b, 2
reg_t					textequ		xmm_e
xmm_e					textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		xmm_temp3
xmm_temp3				textequ		reg_t
count                   =           count + 1
                        ENDM                                                ; RND_CH_SSE2

RND_PARITY_SSE2			MACRO       const:REQ
; t=rotl32(a,5)+e+k+w[i]+(b^c^d); e=d; d=c; c=rotl32(b,30); b=a; a=t

						pshufd		xmm_temp3, xmm_a, 10100000b						; rotate not native to MMX
						pshufd		xmm_temp1, xmm_b, 10100000b
						psrlq		xmm_temp3, 32-5
						pxor		xmm_b, xmm_c
						paddd		xmm_temp3, xmm_e
						pxor		xmm_b, xmm_d
						IF			const eq 06ED9EBA1H
						paddd		xmm_temp3, const_6ED9EBA16ED9EBA1
						ELSEIF		const eq 0CA62C1D6H
						paddd		xmm_temp3, const_CA62C1D6CA62C1D6
						ELSE
						.ERR
						ENDIF
						paddd		xmm_temp3, [__w+count*16]
						psrlq		xmm_temp1, 2
						paddd		xmm_temp3, xmm_b
reg_t					textequ		xmm_e
xmm_e					textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_temp1
xmm_temp1				textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		xmm_temp3
xmm_temp3				textequ		reg_t
count                   =           count + 1

                        ENDM                                                ; RND_PARITY_SSE2

RND_MAJ_SSE2			MACRO       const:REQ

; t=rotl32(a,5)+e+k+w[i]+((b&c)^(b&d)^(c&d)); e=d; d=c; c=rotl32(b,30); b=a; a=t
; t=rotl32(a,5)+e+k+w[i]+((c&d)^(b&(c^d)))

						movdqa		xmm_temp1, xmm_c
						pshufd		xmm_temp3, xmm_a, 10100000b						; rotate not native to MMX
						movdqa		xmm_temp2, xmm_d
						psrlq		xmm_temp3, 32-5
						pxor		xmm_temp1, xmm_d
						paddd		xmm_temp3, xmm_e
						pand		xmm_temp2, xmm_c
						IF			const eq 08F1BBCDCH
						paddd		xmm_temp3, const_8F1BBCDC8F1BBCDC
						ELSE
						.ERR
						ENDIF
						pand		xmm_temp1, xmm_b
						paddd		xmm_temp3, [__w+count*16]
						pxor		xmm_temp2, xmm_temp1
						pshufd		xmm_b, xmm_b, 10100000b
						paddd		xmm_temp3, xmm_temp2
						psrlq		xmm_b, 2
reg_t					textequ		xmm_e
xmm_e					textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		xmm_temp3
xmm_temp3				textequ		reg_t
count                   =           count + 1

                        ENDM                                                ; RND_MAJ_SSE2

xmm_i_16				textequ		<xmm0>
xmm_i_15				textequ		<xmm1>
xmm_i_3					textequ		<xmm2>
xmm_i_2					textequ		<xmm3>
xmm_i_1					textequ		<xmm4>
xmm_t1					textequ		<xmm5>
xmm_t2					textequ		<xmm6>

						ALIGN		16

SHA_Add2_SSE2			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD
                        pusha
__this1                 textequ     <[ebp+36]>
__Data1                 textequ     <[ebp+40]>
__this2                 textequ     <[ebp+44]>
__Data2                 textequ     <[ebp+48]>
__Digest				textequ		<esp+320*4>							; temporary space for digest
__w						textequ		<esp>
						mov			ebp, esp
                        sub         esp, 320*4+20*4
                        and			esp, 0fffffff0h						; align stack pointer
                        mov         esi, __Data1
                        mov			edi, __Data2
                        mov			ebx, __this1
                        mov			edx, __this2

                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			ecx, ecx
                        add         [ebx+m_nCount0], eax
                        adc         [ebx+m_nCount1], ecx
                        add         [edx+m_nCount0], eax
                        adc         [edx+m_nCount1], ecx

                        movdqu      xmm_a, [ebx+m_nHash0]
                        movdqu		xmm_temp1, [edx+m_nHash0]
                        movdqa		xmm_b, xmm_a
                        movdqa		xmm_c, xmm_a
                        movdqa		xmm_d, xmm_a
                        shufps		xmm_a, xmm_temp1, 00000000b
                        shufps		xmm_b, xmm_temp1, 01010101b
                        shufps		xmm_c, xmm_temp1, 10101010b
                        shufps		xmm_d, xmm_temp1, 11111111b
                        movd        xmm_e, [ebx+m_nHash4]
                        movd		xmm_temp1, [edx+m_nHash4]
                        shufps		xmm_e, xmm_temp1, 0

						movdqa		[__Digest+0*16], xmm_a
						movdqa		[__Digest+1*16], xmm_b
						movdqa		[__Digest+2*16], xmm_c
						movdqa		[__Digest+3*16], xmm_d
						movdqa		[__Digest+4*16], xmm_e

                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64

full_blocks:
count                   =           0
                        REPEAT      16
                        IF          count eq 0
                        mov         eax, [esi+count*4]
                        bswap       eax
                        movd		xmm_i_16, eax
                        mov			eax, [edi+count*4]
                        bswap		eax
                        movd		xmm_t1, eax
                        punpcklqdq	xmm_i_16, xmm_t1
                        movdqa      [__w+count*16], xmm_i_16
                        ELSEIF      count eq 1
                        mov         eax, [esi+count*4]
                        bswap       eax
                        movd		xmm_i_15, eax
                        mov			eax, [edi+count*4]
                        bswap		eax
                        movd		xmm_t1, eax
                        punpcklqdq	xmm_i_15, xmm_t1
                        movdqa      [__w+count*16], xmm_i_15
                        ELSEIF      count eq 13
                        mov         eax, [esi+count*4]
                        bswap       eax
                        movd		xmm_i_3, eax
                        mov			eax, [edi+count*4]
                        bswap		eax
                        movd		xmm_t1, eax
                        punpcklqdq	xmm_i_3, xmm_t1
                        movdqa      [__w+count*16], xmm_i_3
                        ELSEIF      count eq 14
                        mov         eax, [esi+count*4]
                        bswap       eax
                        movd		xmm_i_2, eax
                        mov			eax, [edi+count*4]
                        bswap		eax
                        movd		xmm_t1, eax
                        punpcklqdq	xmm_i_2, xmm_t1
                        movdqa      [__w+count*16], xmm_i_2
                        ELSE
                        mov         eax, [esi+count*4]
                        bswap       eax
                        movd		xmm_i_1, eax
                        mov			eax, [edi+count*4]
                        bswap		eax
                        movd		xmm_t1, eax
                        punpcklqdq	xmm_i_1, xmm_t1
                        movdqa      [__w+count*16], xmm_i_1
                        ENDIF
count                   =           count + 1
                        ENDM
                        REPEAT      64
                        pxor        xmm_i_3, xmm_i_16                       ; w[i-16]^w[i-3]
xmm_i_14                textequ     xmm_i_16                                ; we forget w[i-16]
                        IF          count le 77
                        movdqa      xmm_i_14, [__w+(count-14)*16]
                        pxor        xmm_i_3, xmm_i_14
                        ELSE
                        pxor        xmm_i_3, [__w+(count-14)*16]
                        ENDIF
                        pxor        xmm_i_3, [__w+(count-8)*16]
                        pshufd		xmm_i_3, xmm_i_3, 10100000b
                        psrlq		xmm_i_3, 31
                        movdqa      [__w+count*16], xmm_i_3
xmm_i_0                 textequ     xmm_i_3
xmm_i_3                 textequ     xmm_i_2
xmm_i_2                 textequ     xmm_i_1
xmm_i_1                 textequ     xmm_i_0
xmm_i_16                textequ     xmm_i_15
xmm_i_15                textequ     xmm_i_14
count                   =           count + 1
                        ENDM

						movdqa		xmm_a, [__Digest+0*16]
						movdqa		xmm_b, [__Digest+1*16]
						movdqa		xmm_c, [__Digest+2*16]
						movdqa		xmm_d, [__Digest+3*16]
						movdqa		xmm_e, [__Digest+4*16]

count                   =           0

                        REPEAT      20
                        RND_CH_SSE2 05a827999H
                        ENDM
                        REPEAT      20
                        RND_PARITY_SSE2 06ed9eba1H
                        ENDM
                        REPEAT      20
                        RND_MAJ_SSE2 08f1bbcdcH
                        ENDM
                        REPEAT      20
                        RND_PARITY_SSE2 0ca62c1d6H
                        ENDM

						paddd		xmm_a, [__Digest+0*16]
						paddd		xmm_b, [__Digest+1*16]
						paddd		xmm_c, [__Digest+2*16]
						paddd		xmm_d, [__Digest+3*16]
						paddd		xmm_e, [__Digest+4*16]
						movdqa		[__Digest+0*16], xmm_a
						movdqa		[__Digest+1*16], xmm_b
						movdqa		[__Digest+2*16], xmm_c
						movdqa		[__Digest+3*16], xmm_d
						movdqa		[__Digest+4*16], xmm_e
 
						add			esi, 64
						add			edi, 64
						dec			ecx
                        jnz			full_blocks

						movd		eax, xmm_a
						psrldq		xmm_a, 8
						mov			[ebx+m_nHash0], eax
						movd		ecx, xmm_a
						mov			[edx+m_nHash0], ecx
						movd		eax, xmm_b
						psrldq		xmm_b, 8
						mov			[ebx+m_nHash1], eax
						movd		ecx, xmm_b
						mov			[edx+m_nHash1], ecx
						movd		eax, xmm_c
						psrldq		xmm_c, 8
						mov			[ebx+m_nHash2], eax
						movd		ecx, xmm_c
						mov			[edx+m_nHash2], ecx
						movd		eax, xmm_d
						psrldq		xmm_d, 8
						mov			[ebx+m_nHash3], eax
						movd		ecx, xmm_d
						mov			[edx+m_nHash3], ecx
						movd		eax, xmm_e
						psrldq		xmm_e, 8
						mov			[ebx+m_nHash4], eax
						movd		ecx, xmm_e
						mov			[edx+m_nHash4], ecx

						mov			esp, ebp
                        popa
                        ret 16
SHA_Add2_SSE2			ENDP

						.mmx

RND_CH3_MMX				MACRO       const:REQ
; t=rotl32(a,5)+e+k+w[i]+((b&c)^(~b&d)); e=d; d=c; c=rotl32(b,30); b=a; a=t
; t=rotl32(a,5)+e+k+w[i]+(d^(b&(c^d)));
						movq		mmx_temp3, mmx_a						; rotate not native to MMX
                        mov         reg_temp1, reg_a                        ; t=a
						psrld		mmx_temp3, 27
                        mov			reg_temp2, reg_c
						movq		mmx_temp2, mmx_a
                        rol         reg_a, 5
						pslld		mmx_temp2, 5
                        xor			reg_temp2, reg_d
						por			mmx_temp3, mmx_temp2
                        add         reg_a, reg_e
						paddd		mmx_temp3, mmx_e
                        and			reg_temp2, reg_b
						IF			const eq 05A827999H
						paddd		mmx_temp3, const_5A8279995A827999
						ELSE
						.ERR
						ENDIF
                        add			reg_a, const
						paddd		mmx_temp3, [__w+count*8]
						xor			reg_temp2, reg_d
						movq		mmx_temp1, mmx_c
                        add         reg_a, [__w+count*4+2*320]
						pxor		mmx_temp1, mmx_d
                        ror			reg_b, 2
						pand		mmx_temp1, mmx_b
                        add			reg_a, reg_temp2
						pxor		mmx_temp1, mmx_d
						paddd		mmx_temp3, mmx_temp1
						movq		mmx_temp2, mmx_b
						pslld		mmx_b, 30
						psrld		mmx_temp2, 2
						por			mmx_b, mmx_temp2
reg_t					textequ		mmx_e
mmx_e					textequ		mmx_d
mmx_d					textequ		mmx_c
mmx_c					textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		mmx_temp3
mmx_temp3				textequ		reg_t
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d
reg_d                   textequ     reg_c
reg_c					textequ     reg_b
reg_b                   textequ     reg_temp1
reg_temp1               textequ     reg_t
count                   =           count + 1
                        ENDM                                                ; RND_CH3_MMX


RND_PARITY3_MMX			MACRO       const:REQ
; t=rotl32(a,5)+e+k+w[i]+(b^c^d); e=d; d=c; c=rotl32(b,30); b=a; a=t
						movq		mmx_temp3, mmx_a						; rotate not native to MMX
                        mov         reg_temp1, reg_a                        ; t=a
						movq		mmx_temp2, mmx_a
                        rol         reg_a, 5
						movq		mmx_temp1, mmx_b
                        mov			reg_temp2, reg_d
						psrld		mmx_temp3, 27
                        add         reg_a, reg_e
						pslld		mmx_temp2, 5
                        xor			reg_temp2, reg_c
						pxor		mmx_temp1, mmx_c
                        add			reg_a, const
						por			mmx_temp3, mmx_temp2
						movq		mmx_temp2, mmx_b
                        xor			reg_temp2, reg_b
						pxor		mmx_temp1, mmx_d
						paddd		mmx_temp3, mmx_e
                        add			reg_a, [__w+count*4+2*320]
						IF			const eq 06ED9EBA1H
						paddd		mmx_temp3, const_6ED9EBA16ED9EBA1
						ELSEIF		const eq 0CA62C1D6H
						paddd		mmx_temp3, const_CA62C1D6CA62C1D6
						ELSE
						.ERR
						ENDIF
						psrld		mmx_temp2, 2
                        ror			reg_b, 2
						pslld		mmx_b, 30
						paddd		mmx_temp3, [__w+count*8]
                        add			reg_a, reg_temp2
						por			mmx_b, mmx_temp2
						paddd		mmx_temp3, mmx_temp1
reg_t					textequ		mmx_e
mmx_e					textequ		mmx_d
mmx_d					textequ		mmx_c
mmx_c					textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		mmx_temp3
mmx_temp3				textequ		reg_t
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d                                   ; e=d
reg_d                   textequ     reg_c                                   ; d=c
reg_c                   textequ     reg_b                                   ; c=rotl(b,30)
reg_b                   textequ     reg_temp1                               ; b=t
reg_temp1               textequ     reg_t
count                   =           count + 1
                        ENDM                                                ; RND_PARITY3_MMX

RND_MAJ3_MMX			MACRO       const:REQ

; t=rotl32(a,5)+e+k+w[i]+((b&c)^(b&d)^(c&d)); e=d; d=c; c=rotl32(b,30); b=a; a=t
; t=rotl32(a,5)+e+k+w[i]+((c&d)^(b&(c^d)))
						movq		mmx_temp3, mmx_a						; rotate not native to MMX
						mov			reg_temp2, reg_d
						movq		mmx_temp2, mmx_a
						mov			reg_temp1, reg_a
						movq		mmx_temp1, mmx_c
						rol			reg_a, 5
						psrld		mmx_temp3, 27
						xor			reg_temp2, reg_c
						pslld		mmx_temp2, 5
						add			reg_a, reg_e
						pxor		mmx_temp1, mmx_d
						and			reg_temp2, reg_b
						por			mmx_temp3, mmx_temp2
						add			reg_a, const
						movq		mmx_temp2, mmx_d
						mov			reg_e, reg_c
						pand		mmx_temp1, mmx_b
						add			reg_a, [__w+count*4+2*320]
						paddd		mmx_temp3, mmx_e
						pand		mmx_temp2, mmx_c
						and			reg_e, reg_d
						IF			const eq 08F1BBCDCH
						paddd		mmx_temp3, const_8F1BBCDC8F1BBCDC
						ELSE
						.ERR
						ENDIF
						pxor		mmx_temp2, mmx_temp1
						xor			reg_temp2, reg_e
						movq		mmx_temp1, mmx_b
						paddd		mmx_temp3, [__w+count*8]
						ror			reg_b, 2
						pslld		mmx_b, 30
						psrld		mmx_temp1, 2
						add			reg_a, reg_temp2
						paddd		mmx_temp3, mmx_temp2
						por			mmx_b, mmx_temp1
reg_t					textequ		mmx_e
mmx_e					textequ		mmx_d
mmx_d					textequ		mmx_c
mmx_c					textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		mmx_temp3
mmx_temp3				textequ		reg_t
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d
reg_d                   textequ     reg_c
reg_c                   textequ     reg_b
reg_b					textequ		reg_temp1
reg_temp1				textequ		reg_t
count                   =           count + 1

                        ENDM                                                ; RND_MAJ3_MMX

mmx_i_16				textequ		<mm0>
mmx_i_15				textequ		<mm1>
mmx_i_3					textequ		<mm2>
mmx_i_2					textequ		<mm3>
mmx_i_1					textequ		<mm4>
mmx_t1					textequ		<mm5>
mmx_t2					textequ		<mm6>
reg_i_16				textequ		<ebx>
reg_i_15				textequ		<ecx>
reg_i_3					textequ		<edx>
reg_i_2					textequ		<esi>
reg_i_1					textequ		<eax>

SHA_Compile3_MMX		PROC
__w                     textequ     <esp+4>
__Digest				textequ		<esp+320*3+4>
__Data1					textequ		<[esp+320*3+20*3+4]>
__Data2					textequ		<[esp+320*3+20*3+4+4]>
__Data3					textequ		<[esp+320*3+20*3+8+4]>
__this1					textequ		<[esp+320*3+20*3+12+4]>
__this2					textequ		<[esp+320*3+20*3+16+4]>
__this3					textequ		<[esp+320*3+20*3+20+4]>
__Frame					textequ		<[esp+320*3+20*3+24+4]>
__Length				textequ		<[esp+320*3+20*3+28+4]>

						mov			esi, __Data1
						mov			edi, __Data2
						mov			ebp, __Data3
count                   =           0
                        REPEAT      16
                        IF			count eq 0
                        mov         eax, [esi+count*4]
                        bswap       eax
                        movd		mmx_i_16, eax
                        mov			eax, [edi+count*4]
                        bswap		eax
                        movd		mmx_t1, eax
                        mov			reg_i_16, [ebp+count*4]
                        bswap		reg_i_16
                        punpckldq	mmx_i_16, mmx_t1
                        mov			[__w+count*4+2*320], reg_i_16
                        movq		[__w+count*8], mmx_i_16
                        ELSEIF		count eq 1
                        mov         eax, [esi+count*4]
                        bswap       eax
                        movd		mmx_i_15, eax
                        mov			eax, [edi+count*4]
                        bswap		eax
                        movd		mmx_t1, eax
                        mov			reg_i_15, [ebp+count*4]
                        bswap		reg_i_15
                        punpckldq	mmx_i_15, mmx_t1
                        mov			[__w+count*4+2*320], reg_i_15
                        movq		[__w+count*8], mmx_i_15
                        ELSEIF		count eq 13
                        mov         eax, [esi+count*4]
                        bswap       eax
                        movd		mmx_i_3, eax
                        mov			eax, [edi+count*4]
                        bswap		eax
                        movd		mmx_t1, eax
                        mov			reg_i_3, [ebp+count*4]
                        bswap		reg_i_3
                        punpckldq	mmx_i_3, mmx_t1
                        mov			[__w+count*4+2*320], reg_i_3
                        movq		[__w+count*8], mmx_i_3
                        ELSEIF		count eq 14
                        mov         eax, [esi+count*4]
                        bswap       eax
                        movd		mmx_i_2, eax
                        mov			eax, [edi+count*4]
                        bswap		eax
                        movd		mmx_t1, eax
                        mov			eax, [ebp+count*4]
                        bswap		eax
                        punpckldq	mmx_i_2, mmx_t1
                        mov			[__w+count*4+2*320], eax
                        movq		[__w+count*8], mmx_i_2
                        ELSE
                        mov         eax, [esi+count*4]
                        bswap       eax
                        movd		mmx_i_1, eax
                        mov			eax, [edi+count*4]
                        bswap		eax
                        movd		mmx_t1, eax
                        mov			reg_i_1, [ebp+count*4]
                        bswap		reg_i_1
                        punpckldq	mmx_i_1, mmx_t1
                        mov			[__w+count*4+2*320], reg_i_1
                        movq		[__w+count*8], mmx_i_1
                        ENDIF
count                   =           count + 1
                        ENDM
                        mov			reg_i_2, [__w+(count-2)*4+2*320]
                        REPEAT      64
                        pxor        mmx_i_3, mmx_i_16                       ; w[i-16]^w[i-3]
                        xor			reg_i_3, reg_i_16
mmx_i_14                textequ     mmx_i_16                                ; we forget w[i-16]
reg_i_14				textequ		reg_i_16
                        IF          count le 77
                        movq		mmx_i_14, [__w+(count-14)*8]
                        mov			reg_i_14, [__w+(count-14)*4+2*320]
                        pxor        mmx_i_3, mmx_i_14
                        xor			reg_i_3, reg_i_14
                        ELSE
                        pxor        mmx_i_3, [__w+(count-14)*8]
                        xor			reg_i_3, [__w+(count-14)*4+2*320]
                        ENDIF
                        pxor        mmx_i_3, [__w+(count-8)*8]
                        xor			reg_i_3, [__w+(count-8)*4+2*320]
                        movq		mmx_t1, mmx_i_3
                        rol			reg_i_3, 1
                        psrld		mmx_i_3, 31
                        mov			[__w+count*4+2*320], reg_i_3
                        pslld		mmx_t1, 1
                        por			mmx_i_3, mmx_t1
                        movq		[__w+count*8], mmx_i_3
mmx_i_0                 textequ     mmx_i_3
mmx_i_3                 textequ     mmx_i_2
mmx_i_2                 textequ     mmx_i_1
mmx_i_1                 textequ     mmx_i_0
mmx_i_16                textequ     mmx_i_15
mmx_i_15                textequ     mmx_i_14
reg_i_0                 textequ     reg_i_3
reg_i_3                 textequ     reg_i_2
reg_i_2                 textequ     reg_i_1
reg_i_1                 textequ     reg_i_0
reg_i_16                textequ     reg_i_15
reg_i_15                textequ     reg_i_14
count					=			count + 1
                        ENDM
mmx_a					textequ		<mm0>
mmx_b					textequ		<mm1>
mmx_c					textequ		<mm2>
mmx_d					textequ		<mm3>
mmx_e					textequ		<mm4>
mmx_temp1				textequ		<mm5>
mmx_temp2				textequ		<mm6>
mmx_temp3				textequ		<mm7>
reg_a                   textequ     <eax>
reg_b                   textequ     <ebx>
reg_c                   textequ     <ecx>
reg_d                   textequ     <edx>
reg_e                   textequ     <esi>
reg_temp1               textequ     <edi>
reg_temp2               textequ     <ebp>

						movq		mmx_a, [__Digest+0*8]
                        mov         reg_a, [__Digest+40+0*4]
						movq		mmx_b, [__Digest+1*8]
                        mov         reg_b, [__Digest+40+1*4]
						movq		mmx_c, [__Digest+2*8]
                        mov         reg_c, [__Digest+40+2*4]
						movq		mmx_d, [__Digest+3*8]
                        mov         reg_d, [__Digest+40+3*4]
						movq		mmx_e, [__Digest+4*8]
                        mov         reg_e, [__Digest+40+4*4]

count                   =           0

                        REPEAT      20
                        RND_CH3_MMX 05a827999H
                        ENDM
                        REPEAT      20
                        RND_PARITY3_MMX 06ed9eba1H
                        ENDM
                        REPEAT      20
                        RND_MAJ3_MMX 08f1bbcdcH
                        ENDM
                        REPEAT      20
                        RND_PARITY3_MMX 0ca62c1d6H
                        ENDM

						paddd		mmx_a, [__Digest+0*8]
						paddd		mmx_b, [__Digest+1*8]
						paddd		mmx_c, [__Digest+2*8]
						paddd		mmx_d, [__Digest+3*8]
						paddd		mmx_e, [__Digest+4*8]
						movq		[__Digest+0*8], mmx_a
                        add         [__Digest+40+0*4], reg_a
						movq		[__Digest+1*8], mmx_b
                        add         [__Digest+40+1*4], reg_b
						movq		[__Digest+2*8], mmx_c
                        add         [__Digest+40+2*4], reg_c
						movq		[__Digest+3*8], mmx_d
                        add         [__Digest+40+3*4], reg_d
						movq		[__Digest+4*8], mmx_e
                        add         [__Digest+40+4*4], reg_e
 
                        ret

SHA_Compile3_MMX		ENDP

SHA_Add3_SSE2			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD
SHA_Add3_SSE2			ENDP
SHA_Add3_MMX			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD
                        pusha
__this1                 textequ     <[ebp+36]>
__Data1                 textequ     <[ebp+40]>
__this2                 textequ     <[ebp+44]>
__Data2                 textequ     <[ebp+48]>
__this3                 textequ     <[ebp+52]>
__Data3                 textequ     <[ebp+56]>
__Digest				textequ		<esp+320*3>							; temporary space for digest

						mov			ebp, esp
                        sub         esp, 320*3+20*3+20
                        and			esp, 0fffffff8h						; align stack pointer
                        mov         eax, __Data1
                        mov			ebx, __Data2
                        mov			ecx, __Data3
                        mov			edx, __this1
                        mov			esi, __this2
                        mov			edi, __this3
__Data1					textequ		<[esp+320*3+20*3]>
__Data2					textequ		<[esp+320*3+20*3+4]>
__Data3					textequ		<[esp+320*3+20*3+8]>
__Frame					textequ		<[esp+320*3+20*3+12]>
__Length				textequ		<[esp+320*3+20*3+16]>
						mov			__Data1, eax
						mov			__Data2, ebx
						mov			__Data3, ecx
						mov			__Frame, ebp

                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			ecx, ecx
                        add         [edx+m_nCount0], eax
                        adc         [edx+m_nCount1], ecx
                        add         [esi+m_nCount0], eax
                        adc         [esi+m_nCount1], ecx
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], ecx

						movq		mmx_a, [edx+m_nHash0]
						movq		mmx_temp1, [esi+m_nHash0]
						movq		mmx_b, mmx_a
						punpckldq	mmx_a, mmx_temp1
						punpckhdq	mmx_b, mmx_temp1
						movq		mmx_c, [edx+m_nHash2]
						movq		mmx_temp1, [esi+m_nHash2]
						movq		mmx_d, mmx_c
						punpckldq	mmx_c, mmx_temp1
						punpckhdq	mmx_d, mmx_temp1
						movd		mmx_e, [edx+m_nHash4]
						movd		mmx_temp1, [esi+m_nHash4]
						punpckldq	mmx_e, mmx_temp1
						movq		mmx_temp1, [edi+m_nHash0]
						movq		mmx_temp2, [edi+m_nHash2]
						mov			eax, [edi+m_nHash4]
						movq		[__Digest+0*8], mmx_a
						movq		[__Digest+1*8], mmx_b
						movq		[__Digest+2*8], mmx_c
						movq		[__Digest+3*8], mmx_d
						movq		[__Digest+4*8], mmx_e
						movq		[__Digest+5*8], mmx_temp1
						movq		[__Digest+6*8], mmx_temp2
						mov			[__Digest+7*8], eax

                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        mov			__Length, ecx

full_blocks:			call        SHA_Compile3_MMX
						add			dword ptr __Data1, 64
						add			dword ptr __Data2, 64
						add			dword ptr __Data3, 64
						dec			dword ptr __Length
                        jnz			full_blocks

						mov			ebp, __Frame
						mov			edx, __this1
						mov			esi, __this2
						mov			edi, __this3
						movq		mmx_a, [__Digest+0*8]
						movq		mmx_b, [__Digest+1*8]
						movq		mmx_c, [__Digest+2*8]
						movq		mmx_d, [__Digest+3*8]
						movq		mmx_e, [__Digest+4*8]
						movq		mmx_temp1, mmx_a
						punpckldq	mmx_a, mmx_b
						punpckhdq	mmx_temp1, mmx_b
						movq		[edx+m_nHash0], mmx_a
						movq		[esi+m_nHash0], mmx_temp1
						movq		mmx_temp1, mmx_c
						punpckldq	mmx_c, mmx_d
						punpckhdq	mmx_temp1, mmx_d
						movq		[edx+m_nHash2], mmx_c
						movq		[esi+m_nHash2], mmx_temp1
						movq		mmx_temp1, mmx_e
						punpckhdq	mmx_temp1, mmx_temp1
						movd		[edx+m_nHash4], mmx_e
						movd		[esi+m_nHash4], mmx_temp1
						movq		mmx_temp1, [__Digest+5*8]
						movq		mmx_temp2, [__Digest+6*8]
						mov			eax, [__Digest+7*8]
						movq		[edi+m_nHash0], mmx_temp1
						movq		[edi+m_nHash2], mmx_temp2
						mov			[edi+m_nHash4], eax

						mov			esp, ebp
                        popa
                        emms
                        ret 24
SHA_Add3_MMX			ENDP

RND_CH4_CH4_MMX			MACRO       const1:REQ, const2:REQ, phase:REQ
; t=rotl32(a,5)+e+k+w[i]+((b&c)^(~b&d)); e=d; d=c; c=rotl32(b,30); b=a; a=t
; t=rotl32(a,5)+e+k+w[i]+(d^(b&(c^d)));
						IF			count2 eq 0
						movq		mmx_temp3, mmx_a						; rotate not native to MMX
                        mov         reg_a, [__Digest+0*16+8+phase]
						psrld		mmx_temp3, 27
                        mov         reg_b, [__Digest+1*16+8+phase]
						movq		mmx_temp2, mmx_a
                        mov         reg_c, [__Digest+2*16+8+phase]
						pslld		mmx_temp2, 5
                        mov         reg_d, [__Digest+3*16+8+phase]
						por			mmx_temp3, mmx_temp2
                        mov         reg_e, [__Digest+4*16+8+phase]
                        ELSE
						movq		mmx_temp3, mmx_a						; rotate not native to MMX
						psrld		mmx_temp3, 27
						movq		mmx_temp2, mmx_a
						pslld		mmx_temp2, 5
						por			mmx_temp3, mmx_temp2
						ENDIF
						paddd		mmx_temp3, mmx_e
						IF			const1 eq 05A827999H
						paddd		mmx_temp3, const_5A8279995A827999
						ELSE
						.ERR
						ENDIF
                        mov         reg_temp1, reg_a
                        mov			reg_temp2, reg_c
						paddd		mmx_temp3, [__w+count1*16]
                        rol         reg_a, 5
                        xor			reg_temp2, reg_d
						movq		mmx_temp1, mmx_c
                        add         reg_a, reg_e
                        and			reg_temp2, reg_b
						pxor		mmx_temp1, mmx_d
                        add			reg_a, const2
						xor			reg_temp2, reg_d
						pand		mmx_temp1, mmx_b
                        add         reg_a, [__w+count2*16+8+phase]
                        ror			reg_b, 2
						pxor		mmx_temp1, mmx_d
                        add			reg_a, reg_temp2
                        mov         reg_e, reg_a
						paddd		mmx_temp3, mmx_temp1
                        mov			reg_temp2, reg_b
                        rol         reg_a, 5
						movq		mmx_temp2, mmx_b
                        add         reg_a, reg_d
                        xor			reg_temp2, reg_c
						pslld		mmx_b, 30
                        and			reg_temp2, reg_temp1
                        add			reg_a, const2
						psrld		mmx_temp2, 2
						xor			reg_temp2, reg_c
                        add         reg_a, [__w+(count2+1)*16+8+phase]
						por			mmx_b, mmx_temp2
                        ror			reg_temp1, 2
                        add			reg_a, reg_temp2
reg_t					textequ		mmx_e
mmx_e					textequ		mmx_d
mmx_d					textequ		mmx_c
mmx_c					textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		mmx_temp3
mmx_temp3				textequ		reg_t
count1					=			count1 + 1
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d
reg_d                   textequ     reg_c
reg_c					textequ     reg_b
reg_b                   textequ     reg_temp1
reg_temp1               textequ     reg_t
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d
reg_d                   textequ     reg_c
reg_c					textequ     reg_b
reg_b                   textequ     reg_temp1
reg_temp1               textequ     reg_t
count2					=			count2 + 2
                        ENDM                                                ; RND_CH4_CH4_MMX
RND_CH4_PARITY4_MMX		MACRO       const1:REQ, const2:REQ, phase:REQ
; t=rotl32(a,5)+e+k+w[i]+((b&c)^(~b&d)); e=d; d=c; c=rotl32(b,30); b=a; a=t
; t=rotl32(a,5)+e+k+w[i]+(d^(b&(c^d)));
						movq		mmx_temp3, mmx_a						; rotate not native to MMX
                        mov         reg_temp1, reg_a                        ; t=a
                        rol         reg_a, 5
						psrld		mmx_temp3, 27
                        mov			reg_temp2, reg_d
                        add         reg_a, reg_e
						movq		mmx_temp2, mmx_a
                        xor			reg_temp2, reg_c
                        add			reg_a, const2
						pslld		mmx_temp2, 5
                        xor			reg_temp2, reg_b
                        add			reg_a, [__w+count2*16+8+phase]
						por			mmx_temp3, mmx_temp2
                        ror			reg_b, 2
                        add			reg_a, reg_temp2
						paddd		mmx_temp3, mmx_e
                        mov         reg_e, reg_a                        ; t=a
                        rol         reg_a, 5
						IF			const1 eq 05A827999H
						paddd		mmx_temp3, const_5A8279995A827999
						ELSE
						.ERR
						ENDIF
                        mov			reg_temp2, reg_c
                        add         reg_a, reg_d
						paddd		mmx_temp3, [__w+count1*16]
                        xor			reg_temp2, reg_b
                        add			reg_a, const2
						movq		mmx_temp1, mmx_c
                        xor			reg_temp2, reg_temp1
                        add			reg_a, [__w+(count2+1)*16+8+phase]
						pxor		mmx_temp1, mmx_d
                        ror			reg_temp1, 2
                        add			reg_a, reg_temp2
						pand		mmx_temp1, mmx_b
						pxor		mmx_temp1, mmx_d
						paddd		mmx_temp3, mmx_temp1
						movq		mmx_temp2, mmx_b
						pslld		mmx_b, 30
						psrld		mmx_temp2, 2
						por			mmx_b, mmx_temp2

reg_t					textequ		mmx_e
mmx_e					textequ		mmx_d
mmx_d					textequ		mmx_c
mmx_c					textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		mmx_temp3
mmx_temp3				textequ		reg_t
count1					=			count1 + 1
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d                                   ; e=d
reg_d                   textequ     reg_c                                   ; d=c
reg_c                   textequ     reg_b                                   ; c=rotl(b,30)
reg_b                   textequ     reg_temp1                               ; b=t
reg_temp1               textequ     reg_t
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d                                   ; e=d
reg_d                   textequ     reg_c                                   ; d=c
reg_c                   textequ     reg_b                                   ; c=rotl(b,30)
reg_b                   textequ     reg_temp1                               ; b=t
reg_temp1               textequ     reg_t
count2					=			count2 + 2
                        ENDM                                                ; RND_CH4_PARITY4_MMX

RND_PARITY4_MAJ4_MMX	MACRO       const1:REQ, const2:REQ, phase:REQ
; t=rotl32(a,5)+e+k+w[i]+(b^c^d); e=d; d=c; c=rotl32(b,30); b=a; a=t
						movq		mmx_temp3, mmx_a						; rotate not native to MMX
						mov			reg_temp2, reg_d
						mov			reg_temp1, reg_a
						movq		mmx_temp2, mmx_a
						rol			reg_a, 5
						xor			reg_temp2, reg_c
						movq		mmx_temp1, mmx_b
						add			reg_a, reg_e
						and			reg_temp2, reg_b
						psrld		mmx_temp3, 27
						add			reg_a, const2
						mov			reg_e, reg_c
						pslld		mmx_temp2, 5
						add			reg_a, [__w+count2*16+8+phase]
						and			reg_e, reg_d
						pxor		mmx_temp1, mmx_c
						xor			reg_temp2, reg_e
						ror			reg_b, 2
						por			mmx_temp3, mmx_temp2
						add			reg_a, reg_temp2
						mov			reg_temp2, reg_c
						movq		mmx_temp2, mmx_b
						mov			reg_e, reg_a
						rol			reg_a, 5
						pxor		mmx_temp1, mmx_d
						xor			reg_temp2, reg_b
						add			reg_a, reg_d
						paddd		mmx_temp3, mmx_e
						and			reg_temp2, reg_temp1
						add			reg_a, const2
						IF			const1 eq 06ED9EBA1H
						paddd		mmx_temp3, const_6ED9EBA16ED9EBA1
						ELSEIF		const1 eq 0CA62C1D6H
						paddd		mmx_temp3, const_CA62C1D6CA62C1D6
						ELSE
						.ERR
						ENDIF
						mov			reg_d, reg_b
						add			reg_a, [__w+(count2+1)*16+8+phase]
						psrld		mmx_temp2, 2
						and			reg_d, reg_c
						xor			reg_temp2, reg_d
						pslld		mmx_b, 30
						ror			reg_temp1, 2
						add			reg_a, reg_temp2
						paddd		mmx_temp3, [__w+count1*16]
						por			mmx_b, mmx_temp2
						paddd		mmx_temp3, mmx_temp1
reg_t					textequ		mmx_e
mmx_e					textequ		mmx_d
mmx_d					textequ		mmx_c
mmx_c					textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		mmx_temp3
mmx_temp3				textequ		reg_t
count1					=			count1 + 1
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d
reg_d                   textequ     reg_c
reg_c                   textequ     reg_b
reg_b					textequ		reg_temp1
reg_temp1				textequ		reg_t
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d
reg_d                   textequ     reg_c
reg_c                   textequ     reg_b
reg_b					textequ		reg_temp1
reg_temp1				textequ		reg_t
count2					=			count2 + 2
                        ENDM                                                ; RND_PARITY4_MAJ4_MMX
RND_PARITY4_PARITY4_MMX	MACRO       const1:REQ, const2:REQ, phase:REQ
; t=rotl32(a,5)+e+k+w[i]+(b^c^d); e=d; d=c; c=rotl32(b,30); b=a; a=t
						movq		mmx_temp3, mmx_a						; rotate not native to MMX
                        mov         reg_temp1, reg_a                        ; t=a
                        rol         reg_a, 5
						movq		mmx_temp2, mmx_a
                        mov			reg_temp2, reg_d
                        add         reg_a, reg_e
						movq		mmx_temp1, mmx_b
                        xor			reg_temp2, reg_c
                        add			reg_a, const2
						psrld		mmx_temp3, 27
                        xor			reg_temp2, reg_b
                        add			reg_a, [__w+count2*16+8+phase]
						pslld		mmx_temp2, 5
                        ror			reg_b, 2
                        add			reg_a, reg_temp2
						pxor		mmx_temp1, mmx_c
                        mov         reg_e, reg_a
                        rol         reg_a, 5
						por			mmx_temp3, mmx_temp2
                        mov			reg_temp2, reg_c
                        add         reg_a, reg_d
						movq		mmx_temp2, mmx_b
                        xor			reg_temp2, reg_b
                        add			reg_a, const2
						pxor		mmx_temp1, mmx_d
                        xor			reg_temp2, reg_temp1
                        add			reg_a, [__w+(count2+1)*16+8+phase]
						paddd		mmx_temp3, mmx_e
                        ror			reg_temp1, 2
                        add			reg_a, reg_temp2
						IF			const1 eq 06ED9EBA1H
						paddd		mmx_temp3, const_6ED9EBA16ED9EBA1
						ELSEIF		const1 eq 0CA62C1D6H
						paddd		mmx_temp3, const_CA62C1D6CA62C1D6
						ELSE
						.ERR
						ENDIF
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d                                   ; e=d
reg_d                   textequ     reg_c                                   ; d=c
reg_c                   textequ     reg_b                                   ; c=rotl(b,30)
reg_b                   textequ     reg_temp1                               ; b=t
reg_temp1               textequ     reg_t
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d                                   ; e=d
reg_d                   textequ     reg_c                                   ; d=c
reg_c                   textequ     reg_b                                   ; c=rotl(b,30)
reg_b                   textequ     reg_temp1                               ; b=t
reg_temp1               textequ     reg_t
count2					=			count2 + 2
						IF			count2 eq 80
                        add         [__Digest+0*16+8+phase], reg_a
						psrld		mmx_temp2, 2
                        add         [__Digest+1*16+8+phase], reg_b
						pslld		mmx_b, 30
                        add         [__Digest+2*16+8+phase], reg_c
						paddd		mmx_temp3, [__w+count1*16]
                        add         [__Digest+3*16+8+phase], reg_d
						por			mmx_b, mmx_temp2
                        add         [__Digest+4*16+8+phase], reg_e
						paddd		mmx_temp3, mmx_temp1
						ELSE
						psrld		mmx_temp2, 2
						pslld		mmx_b, 30
						paddd		mmx_temp3, [__w+count1*16]
						por			mmx_b, mmx_temp2
						paddd		mmx_temp3, mmx_temp1
						ENDIF

reg_t					textequ		mmx_e
mmx_e					textequ		mmx_d
mmx_d					textequ		mmx_c
mmx_c					textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		mmx_temp3
mmx_temp3				textequ		reg_t
count1					=			count1 + 1
                        ENDM                                                ; RND_PARITY4_PARITY4_MMX

RND_MAJ4_CH4_MMX		MACRO       const1:REQ, const2:REQ, phase:REQ

; t=rotl32(a,5)+e+k+w[i]+((b&c)^(b&d)^(c&d)); e=d; d=c; c=rotl32(b,30); b=a; a=t
; t=rotl32(a,5)+e+k+w[i]+((c&d)^(b&(c^d)))
						IF			count2 eq 0
						movq		mmx_temp3, mmx_a						; rotate not native to MMX
                        mov         reg_a, [__Digest+0*16+8+phase]
						movq		mmx_temp2, mmx_a
                        mov         reg_b, [__Digest+1*16+8+phase]
						movq		mmx_temp1, mmx_c
                        mov         reg_c, [__Digest+2*16+8+phase]
						psrld		mmx_temp3, 27
                        mov         reg_d, [__Digest+3*16+8+phase]
						pslld		mmx_temp2, 5
                        mov         reg_e, [__Digest+4*16+8+phase]
                        ELSE
						movq		mmx_temp3, mmx_a						; rotate not native to MMX
						movq		mmx_temp2, mmx_a
						movq		mmx_temp1, mmx_c
						psrld		mmx_temp3, 27
						pslld		mmx_temp2, 5
						ENDIF
						pxor		mmx_temp1, mmx_d
						por			mmx_temp3, mmx_temp2
						movq		mmx_temp2, mmx_d
						mov         reg_temp1, reg_a
                        mov			reg_temp2, reg_c
						pand		mmx_temp1, mmx_b
                        rol         reg_a, 5
                        xor			reg_temp2, reg_d
						paddd		mmx_temp3, mmx_e
                        add         reg_a, reg_e
                        and			reg_temp2, reg_b
						pand		mmx_temp2, mmx_c
                        add			reg_a, const2
						xor			reg_temp2, reg_d
						IF			const1 eq 08F1BBCDCH
						paddd		mmx_temp3, const_8F1BBCDC8F1BBCDC
						ELSE
						.ERR
						ENDIF
                        add         reg_a, [__w+count2*16+8+phase]
                        ror			reg_b, 2
						pxor		mmx_temp2, mmx_temp1
                        add			reg_a, reg_temp2
                        mov         reg_e, reg_a
						movq		mmx_temp1, mmx_b
                        mov			reg_temp2, reg_b
                        rol         reg_a, 5
						paddd		mmx_temp3, [__w+count1*16]
                        add         reg_a, reg_d
                        xor			reg_temp2, reg_c
						pslld		mmx_b, 30
                        and			reg_temp2, reg_temp1
                        add			reg_a, const2
						psrld		mmx_temp1, 2
						xor			reg_temp2, reg_c
                        add         reg_a, [__w+(count2+1)*16+8+phase]
						paddd		mmx_temp3, mmx_temp2
                        ror			reg_temp1, 2
                        add			reg_a, reg_temp2
						por			mmx_b, mmx_temp1
reg_t					textequ		mmx_e
mmx_e					textequ		mmx_d
mmx_d					textequ		mmx_c
mmx_c					textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		mmx_temp3
mmx_temp3				textequ		reg_t
count1					=			count1 + 1
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d
reg_d                   textequ     reg_c
reg_c					textequ     reg_b
reg_b                   textequ     reg_temp1
reg_temp1               textequ     reg_t
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d
reg_d                   textequ     reg_c
reg_c					textequ     reg_b
reg_b                   textequ     reg_temp1
reg_temp1               textequ     reg_t
count2					=			count2 + 2
                        ENDM                                                ; RND_MAJ4_CH4_MMX

RND_MAJ4_PARITY4_MMX	MACRO       const1:REQ, const2:REQ, phase:REQ
; t=rotl32(a,5)+e+k+w[i]+((b&c)^(b&d)^(c&d)); e=d; d=c; c=rotl32(b,30); b=a; a=t
; t=rotl32(a,5)+e+k+w[i]+((c&d)^(b&(c^d)))
						movq		mmx_temp3, mmx_a						; rotate not native to MMX
                        mov         reg_temp1, reg_a                        ; t=a
                        rol         reg_a, 5
						movq		mmx_temp2, mmx_a
                        mov			reg_temp2, reg_d
                        add         reg_a, reg_e
						movq		mmx_temp1, mmx_c
                        xor			reg_temp2, reg_c
                        add			reg_a, const2
						psrld		mmx_temp3, 27
                        xor			reg_temp2, reg_b
                        add			reg_a, [__w+count2*16+8+phase]
						pslld		mmx_temp2, 5
						pxor		mmx_temp1, mmx_d
						por			mmx_temp3, mmx_temp2
						movq		mmx_temp2, mmx_d
                        ror			reg_b, 2
                        add			reg_a, reg_temp2
						pand		mmx_temp1, mmx_b
                        mov         reg_e, reg_a                        ; t=a
                        rol         reg_a, 5
						paddd		mmx_temp3, mmx_e
                        mov			reg_temp2, reg_c
                        add         reg_a, reg_d
						pand		mmx_temp2, mmx_c
                        xor			reg_temp2, reg_b
                        add			reg_a, const2
						IF			const1 eq 08F1BBCDCH
						paddd		mmx_temp3, const_8F1BBCDC8F1BBCDC
						ELSE
						.ERR
						ENDIF
                        xor			reg_temp2, reg_temp1
                        add			reg_a, [__w+(count2+1)*16+8+phase]
						pxor		mmx_temp2, mmx_temp1
                        ror			reg_temp1, 2
                        add			reg_a, reg_temp2
						movq		mmx_temp1, mmx_b
						paddd		mmx_temp3, [__w+count1*16]
						pslld		mmx_b, 30
						psrld		mmx_temp1, 2
						paddd		mmx_temp3, mmx_temp2
						por			mmx_b, mmx_temp1
reg_t					textequ		mmx_e
mmx_e					textequ		mmx_d
mmx_d					textequ		mmx_c
mmx_c					textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		mmx_temp3
mmx_temp3				textequ		reg_t
count1					=			count1 + 1
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d                                   ; e=d
reg_d                   textequ     reg_c                                   ; d=c
reg_c                   textequ     reg_b                                   ; c=rotl(b,30)
reg_b                   textequ     reg_temp1                               ; b=t
reg_temp1               textequ     reg_t
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d                                   ; e=d
reg_d                   textequ     reg_c                                   ; d=c
reg_c                   textequ     reg_b                                   ; c=rotl(b,30)
reg_b                   textequ     reg_temp1                               ; b=t
reg_temp1               textequ     reg_t
count2					=			count2 + 2
                        ENDM                                                ; RND_MAJ4_PARITY4_MMX
SHA_Add4_MMX			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD, _this4:DWORD, _Data4:DWORD
                        pusha
__this1                 textequ     <[ebp+36]>
__Data1                 textequ     <[ebp+40]>
__this2                 textequ     <[ebp+44]>
__Data2                 textequ     <[ebp+48]>
__this3                 textequ     <[ebp+52]>
__Data3                 textequ     <[ebp+56]>
__this4                 textequ     <[ebp+60]>
__Data4                 textequ     <[ebp+64]>
__Digest				textequ		<esp+320*4>
						mov			ebp, esp
                        sub         esp, 320*4+20*4+24
                        and			esp, 0fffffff0h						; align stack pointer
                        mov         eax, __Data1
                        mov			ebx, __Data2
                        mov			ecx, __Data3
                        mov			edx, __Data4
__w						textequ		<esp>
__Data1					textequ		<[esp+320*4+20*4]>
__Data2					textequ		<[esp+320*4+20*4+4]>
__Data3					textequ		<[esp+320*4+20*4+8]>
__Data4					textequ		<[esp+320*4+20*4+12]>
__Frame					textequ		<[esp+320*4+20*4+16]>
__Length				textequ		<[esp+320*4+20*4+20]>
						mov			__Data1, eax
						mov			__Data2, ebx
						mov			__Data3, ecx
						mov			__Data4, edx
                        mov			ebx, __this1
                        mov			edx, __this2
                        mov			esi, __this3
                        mov			edi, __this4
						mov			__Frame, ebp
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			ecx, ecx
                        add         [ebx+m_nCount0], eax
                        adc         [ebx+m_nCount1], ecx
                        add         [edx+m_nCount0], eax
                        adc         [edx+m_nCount1], ecx
                        add         [esi+m_nCount0], eax
                        adc         [esi+m_nCount1], ecx
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], ecx
						movq		mmx_a, [ebx+m_nHash0]
						movq		mmx_temp1, [edx+m_nHash0]
						movq		mmx_c, [esi+m_nHash0]
						movq		mmx_temp2, [edi+m_nHash0]
						movq		mmx_b, mmx_a
						movq		mmx_d, mmx_c
						punpckldq	mmx_a, mmx_temp1
						punpckhdq	mmx_b, mmx_temp1
						punpckldq	mmx_c, mmx_temp2
						punpckhdq	mmx_d, mmx_temp2
						movq		[__Digest+0*16+0], mmx_a
						movq		[__Digest+0*16+8], mmx_c
						movq		[__Digest+1*16+0], mmx_b
						movq		[__Digest+1*16+8], mmx_d
						movq		mmx_a, [ebx+m_nHash2]
						movq		mmx_temp1, [edx+m_nHash2]
						movq		mmx_c, [esi+m_nHash2]
						movq		mmx_temp2, [edi+m_nHash2]
						movq		mmx_b, mmx_a
						movq		mmx_d, mmx_c
						punpckldq	mmx_a, mmx_temp1
						punpckhdq	mmx_b, mmx_temp1
						punpckldq	mmx_c, mmx_temp2
						punpckhdq	mmx_d, mmx_temp2
						movq		[__Digest+2*16+0], mmx_a
						movq		[__Digest+2*16+8], mmx_c
						movq		[__Digest+3*16+0], mmx_b
						movq		[__Digest+3*16+8], mmx_d
						mov			eax, [ebx+m_nHash4]
						mov			ecx, [edx+m_nHash4]
						mov			[__Digest+4*16+0], eax
						mov			[__Digest+4*16+4], ecx
						mov			eax, [esi+m_nHash4]
						mov			ecx, [edi+m_nHash4]
						mov			[__Digest+4*16+8], eax
						mov			[__Digest+4*16+12], ecx
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        mov			__Length, ecx
mmx_i_16				textequ		<mm0>
mmx_i_15				textequ		<mm1>
mmx_i_3					textequ		<mm2>
mmx_i_2					textequ		<mm3>
mmx_i_1					textequ		<mm4>
mmx_t1					textequ		<mm5>
mmx_t2					textequ		<mm6>
full_blocks:			mov			esi, __Data1
						mov			edi, __Data2
count                   =           0
                        REPEAT      16
                        IF          count eq 0
                        mov         eax, [esi+count*4]
                        mov			ebx, [edi+count*4]
                        bswap       eax
                        bswap		ebx
                        movd		mmx_i_16, eax
                        movd		mmx_t1, ebx
                        punpckldq	mmx_i_16, mmx_t1
                        movq		[__w+count*16], mmx_i_16
                        ELSEIF      count eq 1
                        mov         eax, [esi+count*4]
                        mov			ebx, [edi+count*4]
                        bswap       eax
                        bswap		ebx
                        movd		mmx_i_15, eax
                        movd		mmx_t1, ebx
                        punpckldq	mmx_i_15, mmx_t1
                        movq		[__w+count*16], mmx_i_15
                        ELSEIF      count eq 13
                        mov         eax, [esi+count*4]
                        mov			ebx, [edi+count*4]
                        bswap       eax
                        bswap		ebx
                        movd		mmx_i_3, eax
                        movd		mmx_t1, ebx
                        punpckldq	mmx_i_3, mmx_t1
                        movq		[__w+count*16], mmx_i_3
                        ELSEIF      count eq 14
                        mov         eax, [esi+count*4]
                        mov			ebx, [edi+count*4]
                        bswap       eax
                        bswap		ebx
                        movd		mmx_i_2, eax
                        movd		mmx_t1, ebx
                        punpckldq	mmx_i_2, mmx_t1
                        movq		[__w+count*16], mmx_i_2
                        ELSE
                        mov         eax, [esi+count*4]
                        mov			ebx, [edi+count*4]
                        bswap       eax
                        bswap		ebx
                        movd		mmx_i_1, eax
                        movd		mmx_t1, ebx
                        punpckldq	mmx_i_1, mmx_t1
                        movq		[__w+count*16], mmx_i_1
                        ENDIF
count                   =           count + 1
                        ENDM
                        REPEAT      64
                        pxor        mmx_i_3, mmx_i_16                       ; w[i-16]^w[i-3]
mmx_i_14                textequ     mmx_i_16                                ; we forget w[i-16]
                        IF          count le 77
                        movq		mmx_i_14, [__w+(count-14)*16]
                        pxor        mmx_i_3, mmx_i_14
                        ELSE
                        pxor        mmx_i_3, [__w+(count-14)*16]
                        ENDIF
                        pxor        mmx_i_3, [__w+(count-8)*16]
                        movq		mmx_t1, mmx_i_3
                        psrld		mmx_i_3, 31
                        pslld		mmx_t1, 1
                        por			mmx_i_3, mmx_t1
                        movq		[__w+count*16], mmx_i_3
mmx_i_0                 textequ     mmx_i_3
mmx_i_3                 textequ     mmx_i_2
mmx_i_2                 textequ     mmx_i_1
mmx_i_1                 textequ     mmx_i_0
mmx_i_16                textequ     mmx_i_15
mmx_i_15                textequ     mmx_i_14
count					=			count + 1
                        ENDM
						mov			esi, __Data3
						mov			edi, __Data4
count                   =           0
                        REPEAT      16
                        IF          count eq 0
                        mov         eax, [esi+count*4]
                        mov			ebx, [edi+count*4]
                        bswap       eax
                        bswap		ebx
                        movd		mmx_i_16, eax
                        movd		mmx_t1, ebx
                        punpckldq	mmx_i_16, mmx_t1
                        movq		[__w+count*16+8], mmx_i_16
                        ELSEIF      count eq 1
                        mov         eax, [esi+count*4]
                        mov			ebx, [edi+count*4]
                        bswap       eax
                        bswap		ebx
                        movd		mmx_i_15, eax
                        movd		mmx_t1, ebx
                        punpckldq	mmx_i_15, mmx_t1
                        movq		[__w+count*16+8], mmx_i_15
                        ELSEIF      count eq 13
                        mov         eax, [esi+count*4]
                        mov			ebx, [edi+count*4]
                        bswap       eax
                        bswap		ebx
                        movd		mmx_i_3, eax
                        movd		mmx_t1, ebx
                        punpckldq	mmx_i_3, mmx_t1
                        movq		[__w+count*16+8], mmx_i_3
                        ELSEIF      count eq 14
                        mov         eax, [esi+count*4]
                        mov			ebx, [edi+count*4]
                        bswap       eax
                        bswap		ebx
                        movd		mmx_i_2, eax
                        movd		mmx_t1, ebx
                        punpckldq	mmx_i_2, mmx_t1
                        movq		[__w+count*16+8], mmx_i_2
                        ELSE
                        mov         eax, [esi+count*4]
                        mov			ebx, [edi+count*4]
                        bswap       eax
                        bswap		ebx
                        movd		mmx_i_1, eax
                        movd		mmx_t1, ebx
                        punpckldq	mmx_i_1, mmx_t1
                        movq		[__w+count*16+8], mmx_i_1
                        ENDIF
count                   =           count + 1
                        ENDM
                        REPEAT      64
                        pxor        mmx_i_3, mmx_i_16                       ; w[i-16]^w[i-3]
mmx_i_14                textequ     mmx_i_16                                ; we forget w[i-16]
                        IF          count le 77
                        movq		mmx_i_14, [__w+(count-14)*16+8]
                        pxor        mmx_i_3, mmx_i_14
                        ELSE
                        pxor        mmx_i_3, [__w+(count-14)*16+8]
                        ENDIF
                        pxor        mmx_i_3, [__w+(count-8)*16+8]
                        movq		mmx_t1, mmx_i_3
                        psrld		mmx_i_3, 31
                        pslld		mmx_t1, 1
                        por			mmx_i_3, mmx_t1
                        movq		[__w+count*16+8], mmx_i_3
mmx_i_0                 textequ     mmx_i_3
mmx_i_3                 textequ     mmx_i_2
mmx_i_2                 textequ     mmx_i_1
mmx_i_1                 textequ     mmx_i_0
mmx_i_16                textequ     mmx_i_15
mmx_i_15                textequ     mmx_i_14
count					=			count + 1
                        ENDM
mmx_a					textequ		<mm0>
mmx_b					textequ		<mm1>
mmx_c					textequ		<mm2>
mmx_d					textequ		<mm3>
mmx_e					textequ		<mm4>
mmx_temp1				textequ		<mm5>
mmx_temp2				textequ		<mm6>
mmx_temp3				textequ		<mm7>
reg_a                   textequ     <eax>
reg_b                   textequ     <ebx>
reg_c                   textequ     <ecx>
reg_d                   textequ     <edx>
reg_e                   textequ     <esi>
reg_temp1               textequ     <edi>
reg_temp2               textequ     <ebp>

						movq		mmx_a, [__Digest+0*16]
						movq		mmx_b, [__Digest+1*16]
						movq		mmx_c, [__Digest+2*16]
						movq		mmx_d, [__Digest+3*16]
						movq		mmx_e, [__Digest+4*16]
count1					=			0
count2					=			0
						REPEAT      10
                        RND_CH4_CH4_MMX			05a827999H, 05a827999H, 0
                        ENDM
                        REPEAT      10
                        RND_CH4_PARITY4_MMX		05a827999H, 06ed9eba1H, 0
                        ENDM
                        REPEAT      10
                        RND_PARITY4_MAJ4_MMX	06ed9eba1H, 08f1bbcdcH, 0
                        ENDM
                        REPEAT      10
                        RND_PARITY4_PARITY4_MMX	06ed9eba1H, 0ca62c1d6H, 0
                        ENDM
count2					=			0
                        REPEAT      10
                        RND_MAJ4_CH4_MMX		08f1bbcdcH, 05a827999H, 4
                        ENDM
                        REPEAT      10
                        RND_MAJ4_PARITY4_MMX	08f1bbcdcH, 06ed9eba1H, 4
                        ENDM
                        REPEAT      10
                        RND_PARITY4_MAJ4_MMX	0ca62c1d6H, 08f1bbcdcH, 4
                        ENDM
                        REPEAT      10
                        RND_PARITY4_PARITY4_MMX	0ca62c1d6H, 0ca62c1d6H, 4
                        ENDM

						paddd		mmx_a, [__Digest+0*16]
						paddd		mmx_b, [__Digest+1*16]
						paddd		mmx_c, [__Digest+2*16]
						paddd		mmx_d, [__Digest+3*16]
						paddd		mmx_e, [__Digest+4*16]
						movq		[__Digest+0*16], mmx_a
						movq		[__Digest+1*16], mmx_b
						movq		[__Digest+2*16], mmx_c
						movq		[__Digest+3*16], mmx_d
						movq		[__Digest+4*16], mmx_e
 
						add			dword ptr __Data1, 64
						add			dword ptr __Data2, 64
						add			dword ptr __Data3, 64
						add			dword ptr __Data4, 64
						dec			dword ptr __Length
                        jnz			full_blocks
						mov			ebp, __Frame
						mov			ebx, __this1
						mov			edx, __this2
						mov			esi, __this3
						mov			edi, __this4
						movq		mmx_a, [__Digest+0*16+0]
						movq		mmx_c, [__Digest+0*16+8]
						movq		mmx_temp1, [__Digest+1*16+0]
						movq		mmx_temp2, [__Digest+1*16+8]
						movq		mmx_b, mmx_a
						movq		mmx_d, mmx_c
						punpckldq	mmx_a, mmx_temp1
						punpckldq	mmx_c, mmx_temp2
						punpckhdq	mmx_b, mmx_temp1
						punpckhdq	mmx_d, mmx_temp2
						movq		[ebx+m_nHash0], mmx_a
						movq		[edx+m_nHash0], mmx_b
						movq		[esi+m_nHash0], mmx_c
						movq		[edi+m_nHash0], mmx_d
						movq		mmx_a, [__Digest+2*16+0]
						movq		mmx_c, [__Digest+2*16+8]
						movq		mmx_temp1, [__Digest+3*16+0]
						movq		mmx_temp2, [__Digest+3*16+8]
						movq		mmx_b, mmx_a
						movq		mmx_d, mmx_c
						punpckldq	mmx_a, mmx_temp1
						punpckldq	mmx_c, mmx_temp2
						punpckhdq	mmx_b, mmx_temp1
						punpckhdq	mmx_d, mmx_temp2
						movq		[ebx+m_nHash2], mmx_a
						movq		[edx+m_nHash2], mmx_b
						movq		[esi+m_nHash2], mmx_c
						movq		[edi+m_nHash2], mmx_d
						mov			eax, [__Digest+4*16+0]
						mov			ecx, [__Digest+4*16+4]
						mov			[ebx+m_nHash4], eax
						mov			[edx+m_nHash4], ecx
						mov			eax, [__Digest+4*16+8]
						mov			ecx, [__Digest+4*16+12]
						mov			[esi+m_nHash4], eax
						mov			[edi+m_nHash4], ecx
						mov			esp, ebp
                        popa
                        emms
                        ret 32
                        
SHA_Add4_MMX			ENDP

						.xmm

RND_CH34_SSE2			MACRO       const:REQ
; t=rotl32(a,5)+e+k+w[i]+((b&c)^(~b&d)); e=d; d=c; c=rotl32(b,30); b=a; a=t
; t=rotl32(a,5)+e+k+w[i]+(d^(b&(c^d)));
						movdqa		xmm_temp3, xmm_a
						movdqa		xmm_temp2, xmm_a
						movdqa		xmm_temp1, xmm_c
						psrld		xmm_temp3, 27
						pslld		xmm_temp2, 5
						pxor		xmm_temp1, xmm_d
						por			xmm_temp3, xmm_temp2
						movdqa		xmm_temp2, xmm_b
						pand		xmm_temp1, xmm_b
						paddd		xmm_temp3, xmm_e
						pslld		xmm_b, 30
						pxor		xmm_temp1, xmm_d
						IF			const eq 05A827999H
						paddd		xmm_temp3, const_5A8279995A827999
						ELSE
						.ERR
						ENDIF
						psrld		xmm_temp2, 2
						paddd		xmm_temp3, [__w+count*16]
						por			xmm_b, xmm_temp2
						paddd		xmm_temp3, xmm_temp1

reg_t					textequ		xmm_e
xmm_e					textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		xmm_temp3
xmm_temp3				textequ		reg_t
count                   =           count + 1
                        ENDM                                                ; RND_CH34_SSE2

RND_PARITY34_SSE2		MACRO       const:REQ
; t=rotl32(a,5)+e+k+w[i]+(b^c^d); e=d; d=c; c=rotl32(b,30); b=a; a=t
						movdqa		xmm_temp3, xmm_a
						movdqa		xmm_temp2, xmm_a
						movdqa		xmm_temp1, xmm_c
						psrld		xmm_temp3, 27
						pslld		xmm_temp2, 5
						pxor		xmm_temp1, xmm_b
						por			xmm_temp3, xmm_temp2
						movdqa		xmm_temp2, xmm_b
						pxor		xmm_temp1, xmm_d
						paddd		xmm_temp3, xmm_e
						IF			const eq 06ED9EBA1H
						paddd		xmm_temp3, const_6ED9EBA16ED9EBA1
						ELSEIF		const eq 0CA62C1D6H
						paddd		xmm_temp3, const_CA62C1D6CA62C1D6
						ELSE
						.ERR
						ENDIF
						psrld		xmm_temp2, 2
						pslld		xmm_b, 30
						paddd		xmm_temp3, [__w+count*16]
						por			xmm_b, xmm_temp2
						paddd		xmm_temp3, xmm_temp1
reg_t					textequ		xmm_e
xmm_e					textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		xmm_temp3
xmm_temp3				textequ		reg_t
count                   =           count + 1
                        ENDM                                                ; RND_PARITY34_SSE2

RND_MAJ34_SSE2			MACRO       const:REQ

; t=rotl32(a,5)+e+k+w[i]+((b&c)^(b&d)^(c&d)); e=d; d=c; c=rotl32(b,30); b=a; a=t
; t=rotl32(a,5)+e+k+w[i]+((c&d)^(b&(c^d)))
						movdqa		xmm_temp3, xmm_a
						movdqa		xmm_temp2, xmm_a
						movdqa		xmm_temp1, xmm_c
						psrld		xmm_temp3, 27
						pslld		xmm_temp2, 5
						pxor		xmm_temp1, xmm_d
						por			xmm_temp3, xmm_temp2
						movdqa		xmm_temp2, xmm_d
						pand		xmm_temp1, xmm_b
						paddd		xmm_temp3, xmm_e
						pand		xmm_temp2, xmm_c
						IF			const eq 08F1BBCDCH
						paddd		xmm_temp3, const_8F1BBCDC8F1BBCDC
						ELSE
						.ERR
						ENDIF
						pxor		xmm_temp2, xmm_temp1
						movdqa		xmm_temp1, xmm_b
						paddd		xmm_temp3, [__w+count*16]
						pslld		xmm_b, 30
						psrld		xmm_temp1, 2
						paddd		xmm_temp3, xmm_temp2
						por			xmm_b, xmm_temp1
reg_t					textequ		xmm_e
xmm_e					textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		xmm_temp3
xmm_temp3				textequ		reg_t
count                   =           count + 1
                        ENDM                                                ; RND_MAJ34_SSE2

SHA_Add4_SSE2			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD, _this4:DWORD, _Data4:DWORD
                        pusha
__this1                 textequ     <[ebp+36]>
__Data1                 textequ     <[ebp+40]>
__this2                 textequ     <[ebp+44]>
__Data2                 textequ     <[ebp+48]>
__this3                 textequ     <[ebp+52]>
__Data3                 textequ     <[ebp+56]>
__this4                 textequ     <[ebp+60]>
__Data4                 textequ     <[ebp+64]>
__Digest				textequ		<esp+320*4>
						mov			ebp, esp
                        sub         esp, 320*4+20*4+24
                        and			esp, 0fffffff0h						; align stack pointer
                        mov         eax, __Data1
                        mov			ebx, __Data2
                        mov			ecx, __Data3
                        mov			edx, __Data4
__w						textequ		<esp>
__Data1					textequ		<[esp+320*4+20*4]>
__Data2					textequ		<[esp+320*4+20*4+4]>
__Data3					textequ		<[esp+320*4+20*4+8]>
__Data4					textequ		<[esp+320*4+20*4+12]>
__Frame					textequ		<[esp+320*4+20*4+16]>
__Length				textequ		<[esp+320*4+20*4+20]>
						mov			__Data1, eax
						mov			__Data2, ebx
						mov			__Data3, ecx
						mov			__Data4, edx
                        mov			ebx, __this1
                        mov			edx, __this2
                        mov			esi, __this3
                        mov			edi, __this4
						mov			__Frame, ebp
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			ecx, ecx
                        add         [ebx+m_nCount0], eax
                        adc         [ebx+m_nCount1], ecx
                        add         [edx+m_nCount0], eax
                        adc         [edx+m_nCount1], ecx
                        add         [esi+m_nCount0], eax
                        adc         [esi+m_nCount1], ecx
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], ecx
                        movdqu		xmm0, [ebx+m_nHash0]
                        movdqu		xmm1, [edx+m_nHash0]
                        movdqu		xmm2, [esi+m_nHash0]
                        movdqu		xmm3, [edi+m_nHash0]
                        movdqa		xmm4, xmm0
                        movdqa		xmm5, xmm2
                        punpckldq	xmm0, xmm1
                        punpckldq	xmm2, xmm3
                        punpckhdq	xmm4, xmm1
                        punpckhdq	xmm5, xmm3
                        movdqa		xmm6, xmm0
                        movdqa		xmm7, xmm4
                        punpcklqdq	xmm0, xmm2
                        punpcklqdq	xmm4, xmm5
                        punpckhqdq	xmm6, xmm2
                        punpckhqdq	xmm7, xmm5
                        movdqa		[__Digest+0*16], xmm0
                        movdqa		[__Digest+1*16], xmm6
                        movdqa		[__Digest+2*16], xmm4
                        movdqa		[__Digest+3*16], xmm7
						mov			eax, [ebx+m_nHash4]
						mov			ecx, [edx+m_nHash4]
						mov			[__Digest+4*16+0], eax
						mov			[__Digest+4*16+4], ecx
						mov			eax, [esi+m_nHash4]
						mov			ecx, [edi+m_nHash4]
						mov			[__Digest+4*16+8], eax
						mov			[__Digest+4*16+12], ecx
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        mov			__Length, ecx
full_blocks:			mov			ebx, __Data1
						mov			edx, __Data2
						mov			esi, __Data3
						mov			edi, __Data4
count					=			0
xmm_i_16				textequ		<xmm0>
xmm_i_15				textequ		<xmm1>
xmm_i_3					textequ		<xmm2>
xmm_i_2					textequ		<xmm3>
xmm_i_1					textequ		<xmm4>
xmm_t1					textequ		<xmm5>
xmm_t2					textequ		<xmm6>
xmm_t3					textequ		<xmm7>
                        REPEAT      16
                        IF          count eq 0
                        mov         eax, [ebx+count*4]
                        mov			ecx, [edx+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_i_16, eax
                        movd		xmm_t1, ecx
                        punpckldq	xmm_i_16, xmm_t1
                        mov         eax, [esi+count*4]
                        mov			ecx, [edi+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_t2, eax
                        movd		xmm_t3, ecx
                        punpckldq	xmm_t2, xmm_t3
                        shufps		xmm_i_16, xmm_t2, 01000100b
                        movdqa		[__w+count*16], xmm_i_16
                        ELSEIF      count eq 1
                        mov         eax, [ebx+count*4]
                        mov			ecx, [edx+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_i_15, eax
                        movd		xmm_t1, ecx
                        punpckldq	xmm_i_15, xmm_t1
                        mov         eax, [esi+count*4]
                        mov			ecx, [edi+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_t2, eax
                        movd		xmm_t3, ecx
                        punpckldq	xmm_t2, xmm_t3
                        shufps		xmm_i_15, xmm_t2, 01000100b
                        movdqa		[__w+count*16], xmm_i_15
                        ELSEIF      count eq 13
                        mov         eax, [ebx+count*4]
                        mov			ecx, [edx+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_i_3, eax
                        movd		xmm_t1, ecx
                        punpckldq	xmm_i_3, xmm_t1
                        mov         eax, [esi+count*4]
                        mov			ecx, [edi+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_t2, eax
                        movd		xmm_t3, ecx
                        punpckldq	xmm_t2, xmm_t3
                        shufps		xmm_i_3, xmm_t2, 01000100b
                        movdqa		[__w+count*16], xmm_i_3
                        ELSEIF      count eq 14
                        mov         eax, [ebx+count*4]
                        mov			ecx, [edx+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_i_2, eax
                        movd		xmm_t1, ecx
                        punpckldq	xmm_i_2, xmm_t1
                        mov         eax, [esi+count*4]
                        mov			ecx, [edi+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_t2, eax
                        movd		xmm_t3, ecx
                        punpckldq	xmm_t2, xmm_t3
                        shufps		xmm_i_2, xmm_t2, 01000100b
                        movdqa		[__w+count*16], xmm_i_2
                        ELSE
                        mov         eax, [ebx+count*4]
                        mov			ecx, [edx+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_i_1, eax
                        movd		xmm_t1, ecx
                        punpckldq	xmm_i_1, xmm_t1
                        mov         eax, [esi+count*4]
                        mov			ecx, [edi+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_t2, eax
                        movd		xmm_t3, ecx
                        punpckldq	xmm_t2, xmm_t3
                        shufps		xmm_i_1, xmm_t2, 01000100b
                        movdqa		[__w+count*16], xmm_i_1
                        ENDIF
count                   =           count + 1
                        ENDM
                        REPEAT      64
                        pxor        xmm_i_3, xmm_i_16						; w[i-16]^w[i-3]
xmm_i_14				textequ		xmm_i_16								; we forget w[i-16]
                        IF          count le 77
                        movdqa		xmm_i_14, [__w+(count-14)*16]
                        pxor        xmm_i_3, xmm_i_14
                        ELSE
                        pxor        xmm_i_3, [__w+(count-14)*16]
                        ENDIF
                        pxor        xmm_i_3, [__w+(count-8)*16]
                        movdqa		xmm_t1, xmm_i_3
                        psrld		xmm_i_3, 31
                        pslld		xmm_t1, 1
                        por			xmm_i_3, xmm_t1
                        movdqa		[__w+count*16], xmm_i_3
xmm_i_0                 textequ     xmm_i_3
xmm_i_3                 textequ     xmm_i_2
xmm_i_2                 textequ     xmm_i_1
xmm_i_1                 textequ     xmm_i_0
xmm_i_16                textequ     xmm_i_15
xmm_i_15                textequ     xmm_i_14
count					=			count + 1
                        ENDM
xmm_a					textequ		<xmm0>
xmm_b					textequ		<xmm1>
xmm_c					textequ		<xmm2>
xmm_d					textequ		<xmm3>
xmm_e					textequ		<xmm4>
xmm_temp1				textequ		<xmm5>
xmm_temp2				textequ		<xmm6>
xmm_temp3				textequ		<xmm7>
						movdqa		xmm_a, [__Digest+0*16]
						movdqa		xmm_b, [__Digest+1*16]
						movdqa		xmm_c, [__Digest+2*16]
						movdqa		xmm_d, [__Digest+3*16]
						movdqa		xmm_e, [__Digest+4*16]
count                   =           0
                        REPEAT      20
                        RND_CH34_SSE2 05a827999H
                        ENDM
                        REPEAT      20
                        RND_PARITY34_SSE2 06ed9eba1H
                        ENDM
                        REPEAT      20
                        RND_MAJ34_SSE2 08f1bbcdcH
                        ENDM
                        REPEAT      20
                        RND_PARITY34_SSE2 0ca62c1d6H
                        ENDM
						paddd		xmm_a, [__Digest+0*16]
						paddd		xmm_b, [__Digest+1*16]
						paddd		xmm_c, [__Digest+2*16]
						paddd		xmm_d, [__Digest+3*16]
						paddd		xmm_e, [__Digest+4*16]
						movdqa		[__Digest+0*16], xmm_a
						movdqa		[__Digest+1*16], xmm_b
						movdqa		[__Digest+2*16], xmm_c
						movdqa		[__Digest+3*16], xmm_d
						movdqa		[__Digest+4*16], xmm_e
						add			dword ptr __Data1, 64
						add			dword ptr __Data2, 64
						add			dword ptr __Data3, 64
						add			dword ptr __Data4, 64
						dec			dword ptr __Length
                        jnz			full_blocks
						mov			ebp, __Frame
						mov			ebx, __this1
						mov			edx, __this2
						mov			esi, __this3
						mov			edi, __this4
                        movdqa		xmm0, [__Digest+0*16]
                        movdqa		xmm1, [__Digest+1*16]
                        movdqa		xmm2, [__Digest+2*16]
                        movdqa		xmm3, [__Digest+3*16]
                        movdqa		xmm4, xmm0
                        movdqa		xmm5, xmm2
                        punpckldq	xmm0, xmm1
                        punpckldq	xmm2, xmm3
                        punpckhdq	xmm4, xmm1
                        punpckhdq	xmm5, xmm3
                        movdqa		xmm6, xmm0
                        movdqa		xmm7, xmm4
                        punpcklqdq	xmm0, xmm2
                        punpcklqdq	xmm4, xmm5
                        punpckhqdq	xmm6, xmm2
                        punpckhqdq	xmm7, xmm5
                        movdqu		[ebx+m_nHash0], xmm0
                        movdqu		[edx+m_nHash0], xmm6
                        movdqu		[esi+m_nHash0], xmm4
                        movdqu		[edi+m_nHash0], xmm7
						mov			eax, [__Digest+4*16+0]
						mov			ecx, [__Digest+4*16+4]
						mov			[ebx+m_nHash4], eax
						mov			[edx+m_nHash4], ecx
						mov			eax, [__Digest+4*16+8]
						mov			ecx, [__Digest+4*16+12]
						mov			[esi+m_nHash4], eax
						mov			[edi+m_nHash4], ecx
						mov			esp, ebp
                        popa
                        ret 32
SHA_Add4_SSE2			ENDP

RND_CH5_SSE2			MACRO       const:REQ
; t=rotl32(a,5)+e+k+w[i]+((b&c)^(~b&d)); e=d; d=c; c=rotl32(b,30); b=a; a=t
; t=rotl32(a,5)+e+k+w[i]+(d^(b&(c^d)));
						movdqa		xmm_temp3, xmm_a
						movdqa		xmm_temp2, xmm_a
						movdqa		xmm_temp1, xmm_c
                        mov         reg_temp1, reg_a                        ; t=a
						psrld		xmm_temp3, 27
                        mov			reg_temp2, reg_c
						pslld		xmm_temp2, 5
                        rol         reg_a, 5
						pxor		xmm_temp1, xmm_d
                        xor			reg_temp2, reg_d
						por			xmm_temp3, xmm_temp2
                        add         reg_a, reg_e
						movdqa		xmm_temp2, xmm_b
                        and			reg_temp2, reg_b
						pand		xmm_temp1, xmm_b
                        add			reg_a, const
						paddd		xmm_temp3, xmm_e
						xor			reg_temp2, reg_d
						pslld		xmm_b, 30
                        add         reg_a, [__w+count*4+320*4]
						pxor		xmm_temp1, xmm_d
                        ror			reg_b, 2
						IF			const eq 05A827999H
						paddd		xmm_temp3, const_5A8279995A827999
						ELSE
						.ERR
						ENDIF
                        add			reg_a, reg_temp2
						psrld		xmm_temp2, 2
						paddd		xmm_temp3, [__w+count*16]
						por			xmm_b, xmm_temp2
						paddd		xmm_temp3, xmm_temp1
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d
reg_d                   textequ     reg_c
reg_c					textequ     reg_b
reg_b                   textequ     reg_temp1
reg_temp1               textequ     reg_t
reg_t					textequ		xmm_e
xmm_e					textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		xmm_temp3
xmm_temp3				textequ		reg_t
count                   =           count + 1
                        ENDM                                                ; RND_CH5_SSE2

RND_PARITY5_SSE2		MACRO       const:REQ
; t=rotl32(a,5)+e+k+w[i]+(b^c^d); e=d; d=c; c=rotl32(b,30); b=a; a=t
						movdqa		xmm_temp3, xmm_a
                        mov         reg_temp1, reg_a                        ; t=a
						movdqa		xmm_temp2, xmm_a
                        rol         reg_a, 5
						movdqa		xmm_temp1, xmm_c
                        mov			reg_temp2, reg_d
						psrld		xmm_temp3, 27
                        add         reg_a, reg_e
						pslld		xmm_temp2, 5
                        xor			reg_temp2, reg_c
						pxor		xmm_temp1, xmm_b
                        add			reg_a, const
						por			xmm_temp3, xmm_temp2
                        xor			reg_temp2, reg_b
						movdqa		xmm_temp2, xmm_b
                        add			reg_a, [__w+count*4+320*4]
						pxor		xmm_temp1, xmm_d
                        ror			reg_b, 2
						paddd		xmm_temp3, xmm_e
                        add			reg_a, reg_temp2
						IF			const eq 06ED9EBA1H
						paddd		xmm_temp3, const_6ED9EBA16ED9EBA1
						ELSEIF		const eq 0CA62C1D6H
						paddd		xmm_temp3, const_CA62C1D6CA62C1D6
						ELSE
						.ERR
						ENDIF
						psrld		xmm_temp2, 2
						pslld		xmm_b, 30
						paddd		xmm_temp3, [__w+count*16]
						por			xmm_b, xmm_temp2
						paddd		xmm_temp3, xmm_temp1
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d                                   ; e=d
reg_d                   textequ     reg_c                                   ; d=c
reg_c                   textequ     reg_b                                   ; c=rotl(b,30)
reg_b                   textequ     reg_temp1                               ; b=t
reg_temp1               textequ     reg_t
reg_t					textequ		xmm_e
xmm_e					textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		xmm_temp3
xmm_temp3				textequ		reg_t
count                   =           count + 1
                        ENDM                                                ; RND_PARITY5_SSE2

RND_MAJ5_SSE2			MACRO       const:REQ

; t=rotl32(a,5)+e+k+w[i]+((b&c)^(b&d)^(c&d)); e=d; d=c; c=rotl32(b,30); b=a; a=t
; t=rotl32(a,5)+e+k+w[i]+((c&d)^(b&(c^d)))
						movdqa		xmm_temp3, xmm_a
						mov			reg_temp2, reg_d
						movdqa		xmm_temp2, xmm_a
						mov			reg_temp1, reg_a
						movdqa		xmm_temp1, xmm_c
						rol			reg_a, 5
						psrld		xmm_temp3, 27
						xor			reg_temp2, reg_c
						pslld		xmm_temp2, 5
						add			reg_a, reg_e
						pxor		xmm_temp1, xmm_d
						and			reg_temp2, reg_b
						por			xmm_temp3, xmm_temp2
						add			reg_a, const
						movdqa		xmm_temp2, xmm_d
						mov			reg_e, reg_c
						pand		xmm_temp1, xmm_b
						add			reg_a, [__w+count*4+320*4]
						paddd		xmm_temp3, xmm_e
						pand		xmm_temp2, xmm_c
						IF			const eq 08F1BBCDCH
						paddd		xmm_temp3, const_8F1BBCDC8F1BBCDC
						ELSE
						.ERR
						ENDIF
						and			reg_e, reg_d
						pxor		xmm_temp2, xmm_temp1
						movdqa		xmm_temp1, xmm_b
						xor			reg_temp2, reg_e
						paddd		xmm_temp3, [__w+count*16]
						pslld		xmm_b, 30
						ror			reg_b, 2
						psrld		xmm_temp1, 2
						paddd		xmm_temp3, xmm_temp2
						por			xmm_b, xmm_temp1
						add			reg_a, reg_temp2
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d
reg_d                   textequ     reg_c
reg_c                   textequ     reg_b
reg_b					textequ		reg_temp1
reg_temp1				textequ		reg_t
reg_t					textequ		xmm_e
xmm_e					textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		xmm_temp3
xmm_temp3				textequ		reg_t
count                   =           count + 1
                        ENDM                                                ; RND_MAJ5_SSE2

SHA_Add5_SSE2			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD, _this4:DWORD, _Data4:DWORD, _this5:DWORD, _Data5:DWORD
                        pusha
__this1                 textequ     <[ebp+36]>
__Data1                 textequ     <[ebp+40]>
__this2                 textequ     <[ebp+44]>
__Data2                 textequ     <[ebp+48]>
__this3                 textequ     <[ebp+52]>
__Data3                 textequ     <[ebp+56]>
__this4                 textequ     <[ebp+60]>
__Data4                 textequ     <[ebp+64]>
__this5                 textequ     <[ebp+68]>
__Data5                 textequ     <[ebp+72]>
__Digest				textequ		<esp+320*5>
						mov			ebp, esp
                        sub         esp, 320*5+20*5+28
                        and			esp, 0fffffff0h						; align stack pointer
                        mov         eax, __Data1
                        mov			ebx, __Data2
                        mov			ecx, __Data3
                        mov			edx, __Data4
                        mov			esi, __Data5
__w						textequ		<esp>
__Data1					textequ		<[esp+320*5+20*5]>
__Data2					textequ		<[esp+320*5+20*5+4]>
__Data3					textequ		<[esp+320*5+20*5+8]>
__Data4					textequ		<[esp+320*5+20*5+12]>
__Data5					textequ		<[esp+320*5+20*5+16]>
__Frame					textequ		<[esp+320*5+20*5+20]>
__Length				textequ		<[esp+320*5+20*5+24]>
						mov			__Data1, eax
						mov			__Data2, ebx
						mov			__Data3, ecx
						mov			__Data4, edx
						mov			__Data5, esi
						mov			__Frame, ebp
                        mov			ebx, __this1
                        mov			edx, __this2
                        mov			esi, __this3
                        mov			edi, __this4
                        mov			ebp, __this5
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			ecx, ecx
                        add         [ebx+m_nCount0], eax
                        adc         [ebx+m_nCount1], ecx
                        add         [edx+m_nCount0], eax
                        adc         [edx+m_nCount1], ecx
                        add         [esi+m_nCount0], eax
                        adc         [esi+m_nCount1], ecx
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], ecx
                        add         [ebp+m_nCount0], eax
                        adc         [ebp+m_nCount1], ecx
                        movdqu		xmm0, [ebx+m_nHash0]
                        movdqu		xmm1, [edx+m_nHash0]
                        movdqu		xmm2, [esi+m_nHash0]
                        movdqu		xmm3, [edi+m_nHash0]
                        movdqa		xmm4, xmm0
                        movdqa		xmm5, xmm2
                        punpckldq	xmm0, xmm1
                        punpckldq	xmm2, xmm3
                        punpckhdq	xmm4, xmm1
                        punpckhdq	xmm5, xmm3
                        movdqa		xmm6, xmm0
                        movdqa		xmm7, xmm4
                        punpcklqdq	xmm0, xmm2
                        punpcklqdq	xmm4, xmm5
                        punpckhqdq	xmm6, xmm2
                        punpckhqdq	xmm7, xmm5
                        movdqa		[__Digest+0*16], xmm0
                        movdqa		[__Digest+1*16], xmm6
                        movdqa		[__Digest+2*16], xmm4
                        movdqa		[__Digest+3*16], xmm7
						mov			eax, [ebx+m_nHash4]
						mov			ecx, [edx+m_nHash4]
						mov			[__Digest+4*16+0], eax
						mov			[__Digest+4*16+4], ecx
						mov			eax, [esi+m_nHash4]
						mov			ecx, [edi+m_nHash4]
						mov			[__Digest+4*16+8], eax
						mov			[__Digest+4*16+12], ecx
						movdqu		xmm0, [ebp+m_nHash0]
						mov			eax, [ebp+m_nHash4]
						movdqa		[__Digest+20*4], xmm0
						mov			[__Digest+20*4+16], eax
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        mov			__Length, ecx
full_blocks:			mov			ebx, __Data1
						mov			edx, __Data2
						mov			esi, __Data3
						mov			edi, __Data4
						mov			ebp, __Data5
count					=			0
xmm_i_16				textequ		<xmm0>
xmm_i_15				textequ		<xmm1>
xmm_i_3					textequ		<xmm2>
xmm_i_2					textequ		<xmm3>
xmm_i_1					textequ		<xmm4>
xmm_t1					textequ		<xmm5>
xmm_t2					textequ		<xmm6>
xmm_t3					textequ		<xmm7>
                        REPEAT      16
                        IF          count eq 0
                        mov         eax, [ebx+count*4]
                        mov			ecx, [edx+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_i_16, eax
                        movd		xmm_t1, ecx
                        punpckldq	xmm_i_16, xmm_t1
                        mov         eax, [esi+count*4]
                        mov			ecx, [edi+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_t2, eax
                        movd		xmm_t3, ecx
                        punpckldq	xmm_t2, xmm_t3
                        shufps		xmm_i_16, xmm_t2, 01000100b
                        movdqa		[__w+count*16], xmm_i_16
                        ELSEIF      count eq 1
                        mov         eax, [ebx+count*4]
                        mov			ecx, [edx+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_i_15, eax
                        movd		xmm_t1, ecx
                        punpckldq	xmm_i_15, xmm_t1
                        mov         eax, [esi+count*4]
                        mov			ecx, [edi+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_t2, eax
                        movd		xmm_t3, ecx
                        punpckldq	xmm_t2, xmm_t3
                        shufps		xmm_i_15, xmm_t2, 01000100b
                        movdqa		[__w+count*16], xmm_i_15
                        ELSEIF      count eq 13
                        mov         eax, [ebx+count*4]
                        mov			ecx, [edx+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_i_3, eax
                        movd		xmm_t1, ecx
                        punpckldq	xmm_i_3, xmm_t1
                        mov         eax, [esi+count*4]
                        mov			ecx, [edi+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_t2, eax
                        movd		xmm_t3, ecx
                        punpckldq	xmm_t2, xmm_t3
                        shufps		xmm_i_3, xmm_t2, 01000100b
                        movdqa		[__w+count*16], xmm_i_3
                        ELSEIF      count eq 14
                        mov         eax, [ebx+count*4]
                        mov			ecx, [edx+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_i_2, eax
                        movd		xmm_t1, ecx
                        punpckldq	xmm_i_2, xmm_t1
                        mov         eax, [esi+count*4]
                        mov			ecx, [edi+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_t2, eax
                        movd		xmm_t3, ecx
                        punpckldq	xmm_t2, xmm_t3
                        shufps		xmm_i_2, xmm_t2, 01000100b
                        movdqa		[__w+count*16], xmm_i_2
                        ELSE
                        mov         eax, [ebx+count*4]
                        mov			ecx, [edx+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_i_1, eax
                        movd		xmm_t1, ecx
                        punpckldq	xmm_i_1, xmm_t1
                        mov         eax, [esi+count*4]
                        mov			ecx, [edi+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_t2, eax
                        movd		xmm_t3, ecx
                        punpckldq	xmm_t2, xmm_t3
                        shufps		xmm_i_1, xmm_t2, 01000100b
                        movdqa		[__w+count*16], xmm_i_1
                        ENDIF
count                   =           count + 1
                        ENDM
reg_i_1                 textequ     <eax>
reg_i_2                 textequ     <ebx>
reg_i_3                 textequ     <ecx>
reg_i_15                textequ     <edx>
reg_i_16                textequ     <esi>
count                   =           0
                        REPEAT      16
                        IF          count eq 0
                        mov         reg_i_16, [ebp+count*4]
                        bswap       reg_i_16
                        mov         [__w+count*4+320*4], reg_i_16
                        ELSEIF      count eq 1
                        mov         reg_i_15, [ebp+count*4]
                        bswap       reg_i_15
                        mov         [__w+count*4+320*4], reg_i_15
                        ELSEIF      count eq 13
                        mov         reg_i_3, [ebp+count*4]
                        bswap       reg_i_3
                        mov         [__w+count*4+320*4], reg_i_3
                        ELSEIF      count eq 14
                        mov         reg_i_2, [ebp+count*4]
                        bswap       reg_i_2
                        mov         [__w+count*4+320*4], reg_i_2
                        ELSE
                        mov         reg_i_1, [ebp+count*4]
                        bswap       reg_i_1
                        mov         [__w+count*4+320*4], reg_i_1
                        ENDIF
count                   =           count + 1
                        ENDM
                        REPEAT      64
                        pxor        xmm_i_3, xmm_i_16						; w[i-16]^w[i-3]
                        xor         reg_i_3, reg_i_16
xmm_i_14				textequ		xmm_i_16								; we forget w[i-16]
reg_i_14                textequ     reg_i_16
                        IF          count le 77
                        movdqa		xmm_i_14, [__w+(count-14)*16]
                        mov         reg_i_14, [__w+(count-14)*4+320*4]
                        pxor        xmm_i_3, xmm_i_14
                        xor         reg_i_3, reg_i_14
                        ELSE
                        pxor        xmm_i_3, [__w+(count-14)*16]
                        xor         reg_i_3, [__w+(count-14)*4+320*4]
                        ENDIF
                        pxor        xmm_i_3, [__w+(count-8)*16]
                        xor         reg_i_3, [__w+(count-8)*4+320*4]
                        movdqa		xmm_t1, xmm_i_3
                        rol         reg_i_3, 1
                        psrld		xmm_i_3, 31
                        mov         [__w+count*4+320*4], reg_i_3
                        pslld		xmm_t1, 1
                        por			xmm_i_3, xmm_t1
                        movdqa		[__w+count*16], xmm_i_3
xmm_i_0                 textequ     xmm_i_3
xmm_i_3                 textequ     xmm_i_2
xmm_i_2                 textequ     xmm_i_1
xmm_i_1                 textequ     xmm_i_0
xmm_i_16                textequ     xmm_i_15
xmm_i_15                textequ     xmm_i_14
reg_i_0                 textequ     reg_i_3
reg_i_3                 textequ     reg_i_2
reg_i_2                 textequ     reg_i_1
reg_i_1                 textequ     reg_i_0
reg_i_16                textequ     reg_i_15
reg_i_15                textequ     reg_i_14
count                   =           count + 1
                        ENDM
reg_a                   textequ     <eax>
reg_b                   textequ     <ebx>
reg_c                   textequ     <ecx>
reg_d                   textequ     <edx>
reg_e                   textequ     <esi>
reg_temp1               textequ     <edi>
reg_temp2               textequ     <ebp>
xmm_a					textequ		<xmm0>
xmm_b					textequ		<xmm1>
xmm_c					textequ		<xmm2>
xmm_d					textequ		<xmm3>
xmm_e					textequ		<xmm4>
xmm_temp1				textequ		<xmm5>
xmm_temp2				textequ		<xmm6>
xmm_temp3				textequ		<xmm7>
						movdqa		xmm_a, [__Digest+0*16]
						movdqa		xmm_b, [__Digest+1*16]
						movdqa		xmm_c, [__Digest+2*16]
						movdqa		xmm_d, [__Digest+3*16]
						movdqa		xmm_e, [__Digest+4*16]
                        mov         reg_a, [__Digest+20*4+0]
                        mov         reg_b, [__Digest+20*4+4]
                        mov         reg_c, [__Digest+20*4+8]
                        mov         reg_d, [__Digest+20*4+12]
                        mov         reg_e, [__Digest+20*4+16]
count                   =           0
                        REPEAT      20
                        RND_CH5_SSE2 05a827999H
                        ENDM
                        REPEAT      20
                        RND_PARITY5_SSE2 06ed9eba1H
                        ENDM
                        REPEAT      20
                        RND_MAJ5_SSE2 08f1bbcdcH
                        ENDM
                        REPEAT      20
                        RND_PARITY5_SSE2 0ca62c1d6H
                        ENDM
						paddd		xmm_a, [__Digest+0*16]
						paddd		xmm_b, [__Digest+1*16]
						paddd		xmm_c, [__Digest+2*16]
						paddd		xmm_d, [__Digest+3*16]
						paddd		xmm_e, [__Digest+4*16]
						movdqa		[__Digest+0*16], xmm_a
						movdqa		[__Digest+1*16], xmm_b
						movdqa		[__Digest+2*16], xmm_c
						movdqa		[__Digest+3*16], xmm_d
						movdqa		[__Digest+4*16], xmm_e
                        add         [__Digest+20*4+0], reg_a
                        add         [__Digest+20*4+4], reg_b
                        add         [__Digest+20*4+8], reg_c
                        add         [__Digest+20*4+12], reg_d
                        add         [__Digest+20*4+16], reg_e
						add			dword ptr __Data1, 64
						add			dword ptr __Data2, 64
						add			dword ptr __Data3, 64
						add			dword ptr __Data4, 64
						add			dword ptr __Data5, 64
						dec			dword ptr __Length
                        jnz			full_blocks
						mov			ebp, __Frame
						mov			ebx, __this1
						mov			edx, __this2
						mov			esi, __this3
						mov			edi, __this4
                        movdqa		xmm0, [__Digest+0*16]
                        movdqa		xmm1, [__Digest+1*16]
                        movdqa		xmm2, [__Digest+2*16]
                        movdqa		xmm3, [__Digest+3*16]
                        movdqa		xmm4, xmm0
                        movdqa		xmm5, xmm2
                        punpckldq	xmm0, xmm1
                        punpckldq	xmm2, xmm3
                        punpckhdq	xmm4, xmm1
                        punpckhdq	xmm5, xmm3
                        movdqa		xmm6, xmm0
                        movdqa		xmm7, xmm4
                        punpcklqdq	xmm0, xmm2
                        punpcklqdq	xmm4, xmm5
                        punpckhqdq	xmm6, xmm2
                        punpckhqdq	xmm7, xmm5
                        movdqu		[ebx+m_nHash0], xmm0
                        movdqu		[edx+m_nHash0], xmm6
                        movdqu		[esi+m_nHash0], xmm4
                        movdqu		[edi+m_nHash0], xmm7
						mov			eax, [__Digest+4*16+0]
						mov			ecx, [__Digest+4*16+4]
						mov			[ebx+m_nHash4], eax
						mov			[edx+m_nHash4], ecx
						mov			eax, [__Digest+4*16+8]
						mov			ecx, [__Digest+4*16+12]
						mov			[esi+m_nHash4], eax
						mov			[edi+m_nHash4], ecx
						mov			ebx, __this5
						movdqa		xmm0,[__Digest+20*4]
						mov			eax, [__Digest+20*4+16]
						movdqu		[ebx+m_nHash0], xmm0
						mov			[ebx+m_nHash4], eax
						mov			esp, ebp
                        popa
                        ret 40
SHA_Add5_SSE2			ENDP

RND_CH6_CH6_SSE2		MACRO       const1:REQ, const2:REQ, phase:REQ
; t=rotl32(a,5)+e+k+w[i]+((b&c)^(~b&d)); e=d; d=c; c=rotl32(b,30); b=a; a=t
; t=rotl32(a,5)+e+k+w[i]+(d^(b&(c^d)));
						IF			count2 eq 0
						movdqa		xmm_temp3, xmm_a
						mov			reg_a, [__Digest+4*20]
						mov			reg_b, [__Digest+4*20+4]
						movdqa		xmm_temp2, xmm_a
						mov			reg_c, [__Digest+4*20+8]
						mov			reg_d, [__Digest+4*20+12]
						movdqa		xmm_temp1, xmm_c
						mov			reg_e, [__Digest+4*20+32]
						ELSE
						movdqa		xmm_temp3, xmm_a
						movdqa		xmm_temp2, xmm_a
						movdqa		xmm_temp1, xmm_c
						ENDIF
						psrld		xmm_temp3, 27
                        mov         reg_temp1, reg_a
                        mov			reg_temp2, reg_c
						pslld		xmm_temp2, 5
                        rol         reg_a, 5
						pxor		xmm_temp1, xmm_d
                        add         reg_a, reg_e
                        xor			reg_temp2, reg_d
						por			xmm_temp3, xmm_temp2
                        add			reg_a, const2
                        and			reg_temp2, reg_b
						movdqa		xmm_temp2, xmm_b
                        add         reg_a, [__w+count2*8+320*4+phase]
						xor			reg_temp2, reg_d
						pand		xmm_temp1, xmm_b
                        ror			reg_b, 2
						paddd		xmm_temp3, xmm_e
                        add			reg_a, reg_temp2
						pslld		xmm_b, 30
                        mov         reg_e, reg_a
                        mov			reg_temp2, reg_b
						pxor		xmm_temp1, xmm_d
                        rol         reg_a, 5
						IF			const1 eq 05A827999H
						paddd		xmm_temp3, const_5A8279995A827999
						ELSE
						.ERR
						ENDIF
                        add         reg_a, reg_d
                        xor			reg_temp2, reg_c
						psrld		xmm_temp2, 2
                        add			reg_a, const2
                        and			reg_temp2, reg_temp1
						paddd		xmm_temp3, [__w+count1*16]
                        add         reg_a, [__w+(count2+1)*8+320*4+phase]
						xor			reg_temp2, reg_c
						por			xmm_b, xmm_temp2
                        ror			reg_temp1, 2
						paddd		xmm_temp3, xmm_temp1
                        add			reg_a, reg_temp2
reg_t					textequ		xmm_e
xmm_e					textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		xmm_temp3
xmm_temp3				textequ		reg_t
count1					=			count1 + 1
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d
reg_d                   textequ     reg_c
reg_c					textequ     reg_b
reg_b                   textequ     reg_temp1
reg_temp1               textequ     reg_t
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d
reg_d                   textequ     reg_c
reg_c					textequ     reg_b
reg_b                   textequ     reg_temp1
reg_temp1               textequ     reg_t
count2					=			count2 + 2
                        ENDM                                                ; RND_CH6_CH6_SSE2
RND_CH6_PARITY6_SSE2	MACRO       const1:REQ, const2:REQ, phase:REQ
; t=rotl32(a,5)+e+k+w[i]+((b&c)^(~b&d)); e=d; d=c; c=rotl32(b,30); b=a; a=t
; t=rotl32(a,5)+e+k+w[i]+(d^(b&(c^d)));
						movdqa		xmm_temp3, xmm_a
                        mov         reg_temp1, reg_a
                        mov			reg_temp2, reg_d
						movdqa		xmm_temp2, xmm_a
                        rol         reg_a, 5
						movdqa		xmm_temp1, xmm_c
                        add         reg_a, reg_e
                        xor			reg_temp2, reg_c
						psrld		xmm_temp3, 27
                        add			reg_a, const2
                        xor			reg_temp2, reg_b
						pslld		xmm_temp2, 5
                        add			reg_a, [__w+count2*8+320*4+phase]
						pxor		xmm_temp1, xmm_d
                        ror			reg_b, 2
						por			xmm_temp3, xmm_temp2
                        add			reg_a, reg_temp2
                        mov			reg_temp2, reg_c
						movdqa		xmm_temp2, xmm_b
                        mov         reg_e, reg_a
                        xor			reg_temp2, reg_b
						pand		xmm_temp1, xmm_b
                        rol         reg_a, 5
						paddd		xmm_temp3, xmm_e
                        add         reg_a, reg_d
                        xor			reg_temp2, reg_temp1
						pslld		xmm_b, 30
                        add			reg_a, const2
						pxor		xmm_temp1, xmm_d
                        add			reg_a, [__w+(count2+1)*8+320*4+phase]
						psrld		xmm_temp2, 2
                        add			reg_a, reg_temp2
						IF			const1 eq 05A827999H
						paddd		xmm_temp3, const_5A8279995A827999
						ELSE
						.ERR
						ENDIF
                        ror			reg_temp1, 2
						paddd		xmm_temp3, [__w+count1*16]
						por			xmm_b, xmm_temp2
						paddd		xmm_temp3, xmm_temp1
reg_t					textequ		xmm_e
xmm_e					textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		xmm_temp3
xmm_temp3				textequ		reg_t
count1					=			count1 + 1
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d                                   ; e=d
reg_d                   textequ     reg_c                                   ; d=c
reg_c                   textequ     reg_b                                   ; c=rotl(b,30)
reg_b                   textequ     reg_temp1                               ; b=t
reg_temp1               textequ     reg_t
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d                                   ; e=d
reg_d                   textequ     reg_c                                   ; d=c
reg_c                   textequ     reg_b                                   ; c=rotl(b,30)
reg_b                   textequ     reg_temp1                               ; b=t
reg_temp1               textequ     reg_t
count2					=			count2 + 2
                        ENDM                                                ; RND_CH6_PARITY6_SSE2

RND_PARITY6_MAJ6_SSE2	MACRO       const1:REQ, const2:REQ, phase:REQ
; t=rotl32(a,5)+e+k+w[i]+(b^c^d); e=d; d=c; c=rotl32(b,30); b=a; a=t
						movdqa		xmm_temp3, xmm_a
						mov			reg_temp2, reg_d
						mov			reg_temp1, reg_a
						movdqa		xmm_temp2, xmm_a
						rol			reg_a, 5
						movdqa		xmm_temp1, xmm_c
						add			reg_a, reg_e
						xor			reg_temp2, reg_c
						psrld		xmm_temp3, 27
						add			reg_a, const2
						mov			reg_e, reg_c
						pslld		xmm_temp2, 5
						add			reg_a, [__w+count2*8+320*4+phase]
						and			reg_temp2, reg_b
						pxor		xmm_temp1, xmm_b
						and			reg_e, reg_d
						por			xmm_temp3, xmm_temp2
						xor			reg_temp2, reg_e
						movdqa		xmm_temp2, xmm_b
						ror			reg_b, 2
						pxor		xmm_temp1, xmm_d
						add			reg_a, reg_temp2
						mov			reg_temp2, reg_c
						paddd		xmm_temp3, xmm_e
						mov			reg_e, reg_a
						xor			reg_temp2, reg_b
						IF			const1 eq 06ED9EBA1H
						paddd		xmm_temp3, const_6ED9EBA16ED9EBA1
						ELSEIF		const1 eq 0CA62C1D6H
						paddd		xmm_temp3, const_CA62C1D6CA62C1D6
						ELSE
						.ERR
						ENDIF
						rol			reg_a, 5
						psrld		xmm_temp2, 2
						add			reg_a, reg_d
						and			reg_temp2, reg_temp1
						pslld		xmm_b, 30
						add			reg_a, const2
						mov			reg_d, reg_b
						paddd		xmm_temp3, [__w+count1*16]
						add			reg_a, [__w+(count2+1)*8+320*4+phase]
						and			reg_d, reg_c
						paddd		xmm_temp3, xmm_temp1
						xor			reg_temp2, reg_d
						add			reg_a, reg_temp2
						por			xmm_b, xmm_temp2
						ror			reg_temp1, 2
reg_t					textequ		xmm_e
xmm_e					textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		xmm_temp3
xmm_temp3				textequ		reg_t
count1					=			count1 + 1
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d
reg_d                   textequ     reg_c
reg_c                   textequ     reg_b
reg_b					textequ		reg_temp1
reg_temp1				textequ		reg_t
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d
reg_d                   textequ     reg_c
reg_c                   textequ     reg_b
reg_b					textequ		reg_temp1
reg_temp1				textequ		reg_t
count2					=			count2 + 2
                        ENDM                                                ; RND_PARITY6_MAJ6_SSE2
RND_PARITY6_PARITY6_SSE2 MACRO       const1:REQ, const2:REQ, phase:REQ
; t=rotl32(a,5)+e+k+w[i]+(b^c^d); e=d; d=c; c=rotl32(b,30); b=a; a=t
						movdqa		xmm_temp3, xmm_a
                        mov         reg_temp1, reg_a                        ; t=a
                        mov			reg_temp2, reg_d
						movdqa		xmm_temp2, xmm_a
                        rol         reg_a, 5
						movdqa		xmm_temp1, xmm_c
                        add         reg_a, reg_e
                        xor			reg_temp2, reg_c
						psrld		xmm_temp3, 27
                        add			reg_a, const2
                        xor			reg_temp2, reg_b
						pslld		xmm_temp2, 5
                        add			reg_a, [__w+count2*8+320*4+phase]
						pxor		xmm_temp1, xmm_b
                        ror			reg_b, 2
						por			xmm_temp3, xmm_temp2
                        add			reg_a, reg_temp2
						movdqa		xmm_temp2, xmm_b
                        mov         reg_e, reg_a
                        mov			reg_temp2, reg_c
						pxor		xmm_temp1, xmm_d
                        rol         reg_a, 5
						paddd		xmm_temp3, xmm_e
                        add         reg_a, reg_d
                        xor			reg_temp2, reg_b
						IF			const1 eq 06ED9EBA1H
						paddd		xmm_temp3, const_6ED9EBA16ED9EBA1
						ELSEIF		const1 eq 0CA62C1D6H
						paddd		xmm_temp3, const_CA62C1D6CA62C1D6
						ELSE
						.ERR
						ENDIF
                        add			reg_a, const2
                        xor			reg_temp2, reg_temp1
						psrld		xmm_temp2, 2
                        add			reg_a, [__w+(count2+1)*8+320*4+phase]
						pslld		xmm_b, 30
                        ror			reg_temp1, 2
						paddd		xmm_temp3, [__w+count1*16]
                        add			reg_a, reg_temp2
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d                                   ; e=d
reg_d                   textequ     reg_c                                   ; d=c
reg_c                   textequ     reg_b                                   ; c=rotl(b,30)
reg_b                   textequ     reg_temp1                               ; b=t
reg_temp1               textequ     reg_t
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d                                   ; e=d
reg_d                   textequ     reg_c                                   ; d=c
reg_c                   textequ     reg_b                                   ; c=rotl(b,30)
reg_b                   textequ     reg_temp1                               ; b=t
reg_temp1               textequ     reg_t
count2					=			count2 + 2
						IF			count2 eq 80
                        add			[__Digest+4*20+32+phase], reg_e
						por			xmm_b, xmm_temp2
                        add			[__Digest+4*20+phase*4], reg_a
                        add			[__Digest+4*20+4+phase*4], reg_b
						paddd		xmm_temp3, xmm_temp1
                        add			[__Digest+4*20+8+phase*4], reg_c
                        add			[__Digest+4*20+12+phase*4], reg_d
						ELSE
						por			xmm_b, xmm_temp2
						paddd		xmm_temp3, xmm_temp1
						ENDIF
reg_t					textequ		xmm_e
xmm_e					textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		xmm_temp3
xmm_temp3				textequ		reg_t
count1					=			count1 + 1
                        ENDM                                                ; RND_PARITY6_PARITY6_SSE2
RND_MAJ6_CH6_SSE2		MACRO       const1:REQ, const2:REQ, phase:REQ
; t=rotl32(a,5)+e+k+w[i]+((b&c)^(b&d)^(c&d)); e=d; d=c; c=rotl32(b,30); b=a; a=t
; t=rotl32(a,5)+e+k+w[i]+((c&d)^(b&(c^d)))
						IF			count2 eq 0
						movdqa		xmm_temp3, xmm_a
						mov			reg_a, [__Digest+4*20+16]
						mov			reg_b, [__Digest+4*20+20]
						movdqa		xmm_temp2, xmm_a
						mov			reg_c, [__Digest+4*20+24]
						mov			reg_d, [__Digest+4*20+28]
						movdqa		xmm_temp1, xmm_c
						mov			reg_e, [__Digest+4*20+36]
						ELSE
						movdqa		xmm_temp3, xmm_a
						movdqa		xmm_temp2, xmm_a
						movdqa		xmm_temp1, xmm_c
						ENDIF
						psrld		xmm_temp3, 27
						pslld		xmm_temp2, 5
                        mov         reg_temp1, reg_a
                        mov			reg_temp2, reg_c
						pxor		xmm_temp1, xmm_d
                        rol         reg_a, 5
						por			xmm_temp3, xmm_temp2
                        add         reg_a, reg_e
                        xor			reg_temp2, reg_d
						movdqa		xmm_temp2, xmm_d
                        add			reg_a, const2
                        and			reg_temp2, reg_b
						pand		xmm_temp1, xmm_b
                        add         reg_a, [__w+count2*8+320*4+phase]
						xor			reg_temp2, reg_d
						paddd		xmm_temp3, xmm_e
                        ror			reg_b, 2
						pand		xmm_temp2, xmm_c
                        add			reg_a, reg_temp2
                        mov			reg_temp2, reg_b
						IF			const1 eq 08F1BBCDCH
						paddd		xmm_temp3, const_8F1BBCDC8F1BBCDC
						ELSE
						.ERR
						ENDIF
						pxor		xmm_temp2, xmm_temp1
                        mov         reg_e, reg_a
                        xor			reg_temp2, reg_c
						movdqa		xmm_temp1, xmm_b
                        rol         reg_a, 5
						paddd		xmm_temp3, [__w+count1*16]
                        add         reg_a, reg_d
                        and			reg_temp2, reg_temp1
						pslld		xmm_b, 30
                        add			reg_a, const2
						xor			reg_temp2, reg_c
						psrld		xmm_temp1, 2
                        ror			reg_temp1, 2
						paddd		xmm_temp3, xmm_temp2
                        add         reg_a, [__w+(count2+1)*8+320*4+phase]
						por			xmm_b, xmm_temp1
                        add			reg_a, reg_temp2
reg_t					textequ		xmm_e
xmm_e					textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		xmm_temp3
xmm_temp3				textequ		reg_t
count1					=			count1 + 1
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d
reg_d                   textequ     reg_c
reg_c					textequ     reg_b
reg_b                   textequ     reg_temp1
reg_temp1               textequ     reg_t
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d
reg_d                   textequ     reg_c
reg_c					textequ     reg_b
reg_b                   textequ     reg_temp1
reg_temp1               textequ     reg_t
count2					=			count2 + 2
                        ENDM                                                ; RND_MAJ6_CH6_SSE2
RND_MAJ6_PARITY6_SSE2	MACRO       const1:REQ, const2:REQ, phase:REQ
; t=rotl32(a,5)+e+k+w[i]+((b&c)^(b&d)^(c&d)); e=d; d=c; c=rotl32(b,30); b=a; a=t
; t=rotl32(a,5)+e+k+w[i]+((c&d)^(b&(c^d)))
						movdqa		xmm_temp3, xmm_a
                        mov         reg_temp1, reg_a
                        mov			reg_temp2, reg_d
						movdqa		xmm_temp2, xmm_a
                        rol         reg_a, 5
						movdqa		xmm_temp1, xmm_c
                        xor			reg_temp2, reg_c
						psrld		xmm_temp3, 27
                        add         reg_a, reg_e
                        xor			reg_temp2, reg_b
						pslld		xmm_temp2, 5
                        add			reg_a, const2
						pxor		xmm_temp1, xmm_d
                        add			reg_a, [__w+count2*8+320*4+phase]
						por			xmm_temp3, xmm_temp2
                        add			reg_a, reg_temp2
                        mov			reg_temp2, reg_c
						movdqa		xmm_temp2, xmm_d
                        mov         reg_e, reg_a
						pand		xmm_temp1, xmm_b
                        rol         reg_a, 5
						paddd		xmm_temp3, xmm_e
                        ror			reg_b, 2
						pand		xmm_temp2, xmm_c
                        add         reg_a, reg_d
                        xor			reg_temp2, reg_temp1
						IF			const1 eq 08F1BBCDCH
						paddd		xmm_temp3, const_8F1BBCDC8F1BBCDC
						ELSE
						.ERR
						ENDIF
                        add			reg_a, const2
                        xor			reg_temp2, reg_b
						pxor		xmm_temp2, xmm_temp1
                        add			reg_a, [__w+(count2+1)*8+320*4+phase]
						movdqa		xmm_temp1, xmm_b
                        ror			reg_temp1, 2
						paddd		xmm_temp3, [__w+count1*16]
                        add			reg_a, reg_temp2
						pslld		xmm_b, 30
						psrld		xmm_temp1, 2
						paddd		xmm_temp3, xmm_temp2
						por			xmm_b, xmm_temp1
reg_t					textequ		xmm_e
xmm_e					textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		xmm_temp3
xmm_temp3				textequ		reg_t
count1					=			count1 + 1
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d                                   ; e=d
reg_d                   textequ     reg_c                                   ; d=c
reg_c                   textequ     reg_b                                   ; c=rotl(b,30)
reg_b                   textequ     reg_temp1                               ; b=t
reg_temp1               textequ     reg_t
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d                                   ; e=d
reg_d                   textequ     reg_c                                   ; d=c
reg_c                   textequ     reg_b                                   ; c=rotl(b,30)
reg_b                   textequ     reg_temp1                               ; b=t
reg_temp1               textequ     reg_t
count2					=			count2 + 2
                        ENDM                                                ; RND_MAJ6_PARITY6_SSE2
SHA_Add6_SSE2			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD, _this4:DWORD, _Data4:DWORD, _this5:DWORD, _Data5:DWORD, _this6:DWORD, _Data6:DWORD
                        pusha
__this1                 textequ     <[ebp+36]>
__Data1                 textequ     <[ebp+40]>
__this2                 textequ     <[ebp+44]>
__Data2                 textequ     <[ebp+48]>
__this3                 textequ     <[ebp+52]>
__Data3                 textequ     <[ebp+56]>
__this4                 textequ     <[ebp+60]>
__Data4                 textequ     <[ebp+64]>
__this5                 textequ     <[ebp+68]>
__Data5                 textequ     <[ebp+72]>
__this6                 textequ     <[ebp+76]>
__Data6                 textequ     <[ebp+80]>
						mov			ebp, esp
                        sub         esp, 320*6+20*6+32
                        and			esp, 0fffffff0h						; align stack pointer
                        mov         eax, __Data1
                        mov			ebx, __Data2
                        mov			ecx, __Data3
                        mov			edx, __Data4
                        mov			esi, __Data5
                        mov			edi, __Data6
__w						textequ		<esp>
__Digest				textequ		<esp+320*6>
__Data1					textequ		<[esp+320*6+20*6]>
__Data2					textequ		<[esp+320*6+20*6+4]>
__Data3					textequ		<[esp+320*6+20*6+8]>
__Data4					textequ		<[esp+320*6+20*6+12]>
__Data5					textequ		<[esp+320*6+20*6+16]>
__Data6					textequ		<[esp+320*6+20*6+20]>
__Frame					textequ		<[esp+320*6+20*6+24]>
__Length				textequ		<[esp+320*6+20*6+28]>
						mov			__Data1, eax
						mov			__Data2, ebx
						mov			__Data3, ecx
						mov			__Data4, edx
						mov			__Data5, esi
						mov			__Data6, edi
						mov			__Frame, ebp
                        mov			ebx, __this5
                        mov			edx, __this6
                        mov			esi, __this3
                        mov			edi, __this4
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			ecx, ecx
                        add         [ebx+m_nCount0], eax
                        adc         [ebx+m_nCount1], ecx
                        add         [edx+m_nCount0], eax
                        adc         [edx+m_nCount1], ecx
                        mov			ebx, __this1
                        mov			edx, __this2
                        add         [esi+m_nCount0], eax
                        adc         [esi+m_nCount1], ecx
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], ecx
                        add         [ebx+m_nCount0], eax
                        adc         [ebx+m_nCount1], ecx
                        add         [edx+m_nCount0], eax
                        adc         [edx+m_nCount1], ecx
                        movdqu		xmm0, [ebx+m_nHash0]
                        movdqu		xmm1, [edx+m_nHash0]
                        movdqu		xmm2, [esi+m_nHash0]
                        movdqu		xmm3, [edi+m_nHash0]
                        movdqa		xmm4, xmm0
                        movdqa		xmm5, xmm2
                        punpckldq	xmm0, xmm1
                        punpckldq	xmm2, xmm3
                        punpckhdq	xmm4, xmm1
                        punpckhdq	xmm5, xmm3
                        movdqa		xmm6, xmm0
                        movdqa		xmm7, xmm4
                        punpcklqdq	xmm0, xmm2
                        punpcklqdq	xmm4, xmm5
                        punpckhqdq	xmm6, xmm2
                        punpckhqdq	xmm7, xmm5
                        movdqa		[__Digest+0*16], xmm0
                        movdqa		[__Digest+1*16], xmm6
                        movdqa		[__Digest+2*16], xmm4
                        movdqa		[__Digest+3*16], xmm7
						mov			eax, [ebx+m_nHash4]
						mov			ecx, [edx+m_nHash4]
						mov			[__Digest+4*16+0], eax
						mov			[__Digest+4*16+4], ecx
						mov			eax, [esi+m_nHash4]
						mov			ecx, [edi+m_nHash4]
						mov			[__Digest+4*16+8], eax
						mov			[__Digest+4*16+12], ecx
						mov			esi, __this5
						mov			edi, __this6
						movdqu		xmm0, [esi+m_nHash0]
						mov			eax, [esi+m_nHash4]
						movdqu		xmm1, [edi+m_nHash0]
						mov			ecx, [edi+m_nHash4]
						movdqa		[__Digest+20*4], xmm0
						movdqa		[__Digest+20*4+16], xmm1
						mov			[__Digest+20*4+32], eax
						mov			[__Digest+20*4+36], ecx
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        mov			__Length, ecx
full_blocks:			mov			ebx, __Data1
						mov			edx, __Data2
						mov			esi, __Data3
						mov			edi, __Data4
count					=			0
xmm_i_16				textequ		<xmm0>
xmm_i_15				textequ		<xmm1>
xmm_i_3					textequ		<xmm2>
xmm_i_2					textequ		<xmm3>
xmm_i_1					textequ		<xmm4>
xmm_t1					textequ		<xmm5>
xmm_t2					textequ		<xmm6>
xmm_t3					textequ		<xmm7>
                        REPEAT      16
                        IF          count eq 0
                        mov         eax, [ebx+count*4]
                        mov			ecx, [edx+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_i_16, eax
                        movd		xmm_t1, ecx
                        punpckldq	xmm_i_16, xmm_t1
                        mov         eax, [esi+count*4]
                        mov			ecx, [edi+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_t2, eax
                        movd		xmm_t3, ecx
                        punpckldq	xmm_t2, xmm_t3
                        shufps		xmm_i_16, xmm_t2, 01000100b
                        movdqa		[__w+count*16], xmm_i_16
                        ELSEIF      count eq 1
                        mov         eax, [ebx+count*4]
                        mov			ecx, [edx+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_i_15, eax
                        movd		xmm_t1, ecx
                        punpckldq	xmm_i_15, xmm_t1
                        mov         eax, [esi+count*4]
                        mov			ecx, [edi+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_t2, eax
                        movd		xmm_t3, ecx
                        punpckldq	xmm_t2, xmm_t3
                        shufps		xmm_i_15, xmm_t2, 01000100b
                        movdqa		[__w+count*16], xmm_i_15
                        ELSEIF      count eq 13
                        mov         eax, [ebx+count*4]
                        mov			ecx, [edx+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_i_3, eax
                        movd		xmm_t1, ecx
                        punpckldq	xmm_i_3, xmm_t1
                        mov         eax, [esi+count*4]
                        mov			ecx, [edi+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_t2, eax
                        movd		xmm_t3, ecx
                        punpckldq	xmm_t2, xmm_t3
                        shufps		xmm_i_3, xmm_t2, 01000100b
                        movdqa		[__w+count*16], xmm_i_3
                        ELSEIF      count eq 14
                        mov         eax, [ebx+count*4]
                        mov			ecx, [edx+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_i_2, eax
                        movd		xmm_t1, ecx
                        punpckldq	xmm_i_2, xmm_t1
                        mov         eax, [esi+count*4]
                        mov			ecx, [edi+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_t2, eax
                        movd		xmm_t3, ecx
                        punpckldq	xmm_t2, xmm_t3
                        shufps		xmm_i_2, xmm_t2, 01000100b
                        movdqa		[__w+count*16], xmm_i_2
                        ELSE
                        mov         eax, [ebx+count*4]
                        mov			ecx, [edx+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_i_1, eax
                        movd		xmm_t1, ecx
                        punpckldq	xmm_i_1, xmm_t1
                        mov         eax, [esi+count*4]
                        mov			ecx, [edi+count*4]
                        bswap       eax
                        bswap		ecx
                        movd		xmm_t2, eax
                        movd		xmm_t3, ecx
                        punpckldq	xmm_t2, xmm_t3
                        shufps		xmm_i_1, xmm_t2, 01000100b
                        movdqa		[__w+count*16], xmm_i_1
                        ENDIF
count                   =           count + 1
                        ENDM
                        mov			esi, __Data5
                        mov			edi, __Data6
count					=			0
mmx_i_16				textequ		<mm0>
mmx_i_15				textequ		<mm1>
mmx_i_3					textequ		<mm2>
mmx_i_2					textequ		<mm3>
mmx_i_1					textequ		<mm4>
mmx_t1					textequ		<mm5>
mmx_t2					textequ		<mm6>
mmx_t3					textequ		<mm7>
count                   =           0
                        REPEAT      16
                        IF          count eq 0
                        mov         eax, [esi+count*4]
                        mov			ecx, [edi+count*4]
                        bswap		eax
                        bswap       ecx
                        movd		mmx_i_16, eax
                        movd		mmx_t1, ecx
                        punpckldq	mmx_i_16, mmx_t1
                        movq		[__w+count*8+320*4], mmx_i_16
                        ELSEIF      count eq 1
                        mov         eax, [esi+count*4]
                        mov			ecx, [edi+count*4]
                        bswap		eax
                        bswap       ecx
                        movd		mmx_i_15, eax
                        movd		mmx_t1, ecx
                        punpckldq	mmx_i_15, mmx_t1
                        movq		[__w+count*8+320*4], mmx_i_15
                        ELSEIF      count eq 13
                        mov         eax, [esi+count*4]
                        mov			ecx, [edi+count*4]
                        bswap		eax
                        bswap       ecx
                        movd		mmx_i_3, eax
                        movd		mmx_t1, ecx
                        punpckldq	mmx_i_3, mmx_t1
                        movq		[__w+count*8+320*4], mmx_i_3
                        ELSEIF      count eq 14
                        mov         eax, [esi+count*4]
                        mov			ecx, [edi+count*4]
                        bswap		eax
                        bswap       ecx
                        movd		mmx_i_2, eax
                        movd		mmx_t1, ecx
                        punpckldq	mmx_i_2, mmx_t1
                        movq		[__w+count*8+320*4], mmx_i_2
                        ELSE
                        mov         eax, [esi+count*4]
                        mov			ecx, [edi+count*4]
                        bswap		eax
                        bswap       ecx
                        movd		mmx_i_1, eax
                        movd		mmx_t1, ecx
                        punpckldq	mmx_i_1, mmx_t1
                        movq		[__w+count*8+320*4], mmx_i_1
                        ENDIF
count                   =           count + 1
                        ENDM
                        REPEAT      64
                        pxor        xmm_i_3, xmm_i_16						; w[i-16]^w[i-3]
                        pxor        mmx_i_3, mmx_i_16
xmm_i_14				textequ		xmm_i_16								; we forget w[i-16]
mmx_i_14				textequ		mmx_i_16
                        IF          count le 77
                        movdqa		xmm_i_14, [__w+(count-14)*16]
                        movq		mmx_i_14, [__w+(count-14)*8+320*4]
                        pxor        xmm_i_3, xmm_i_14
                        pxor        mmx_i_3, mmx_i_14
                        ELSE
                        pxor        xmm_i_3, [__w+(count-14)*16]
                        pxor		mmx_i_3, [__w+(count-14)*8+320*4]
                        ENDIF
                        pxor        xmm_i_3, [__w+(count-8)*16]
                        pxor        mmx_i_3, [__w+(count-8)*8+320*4]
                        movdqa		xmm_t1, xmm_i_3
                        movq		mmx_t1, mmx_i_3
                        psrld		xmm_i_3, 31
                        psrld		mmx_i_3, 31
                        pslld		xmm_t1, 1
                        pslld		mmx_t1, 1
                        por			xmm_i_3, xmm_t1
                        por			mmx_i_3, mmx_t1
                        movdqa		[__w+count*16], xmm_i_3
                        movq		[__w+count*8+320*4], mmx_i_3
xmm_i_0                 textequ     xmm_i_3
xmm_i_3                 textequ     xmm_i_2
xmm_i_2                 textequ     xmm_i_1
xmm_i_1                 textequ     xmm_i_0
xmm_i_16                textequ     xmm_i_15
xmm_i_15                textequ     xmm_i_14
mmx_i_0                 textequ     mmx_i_3
mmx_i_3                 textequ     mmx_i_2
mmx_i_2                 textequ     mmx_i_1
mmx_i_1                 textequ     mmx_i_0
mmx_i_16                textequ     mmx_i_15
mmx_i_15                textequ     mmx_i_14
count                   =           count + 1
                        ENDM
xmm_a					textequ		<xmm0>
xmm_b					textequ		<xmm1>
xmm_c					textequ		<xmm2>
xmm_d					textequ		<xmm3>
xmm_e					textequ		<xmm4>
xmm_temp1				textequ		<xmm5>
xmm_temp2				textequ		<xmm6>
xmm_temp3				textequ		<xmm7>
reg_a                   textequ     <eax>
reg_b                   textequ     <ebx>
reg_c                   textequ     <ecx>
reg_d                   textequ     <edx>
reg_e                   textequ     <esi>
reg_temp1               textequ     <edi>
reg_temp2               textequ     <ebp>
						movdqa		xmm_a, [__Digest+0*16]
						movdqa		xmm_b, [__Digest+1*16]
						movdqa		xmm_c, [__Digest+2*16]
						movdqa		xmm_d, [__Digest+3*16]
						movdqa		xmm_e, [__Digest+4*16]
count1					=			0
count2					=			0
						REPEAT      10
                        RND_CH6_CH6_SSE2		05a827999H, 05a827999H, 0
                        ENDM
                        REPEAT      10
                        RND_CH6_PARITY6_SSE2	05a827999H, 06ed9eba1H, 0
                        ENDM
                        REPEAT      10
                        RND_PARITY6_MAJ6_SSE2	06ed9eba1H, 08f1bbcdcH, 0
                        ENDM
                        REPEAT      10
                        RND_PARITY6_PARITY6_SSE2 06ed9eba1H, 0ca62c1d6H, 0
                        ENDM
count2					=			0
                        REPEAT      10
                        RND_MAJ6_CH6_SSE2		08f1bbcdcH, 05a827999H, 4
                        ENDM
                        REPEAT      10
                        RND_MAJ6_PARITY6_SSE2	08f1bbcdcH, 06ed9eba1H, 4
                        ENDM
                        REPEAT      10
                        RND_PARITY6_MAJ6_SSE2	0ca62c1d6H, 08f1bbcdcH, 4
                        ENDM
                        REPEAT      10
                        RND_PARITY6_PARITY6_SSE2 0ca62c1d6H, 0ca62c1d6H, 4
                        ENDM
						paddd		xmm_a, [__Digest+0*16]
						paddd		xmm_b, [__Digest+1*16]
						paddd		xmm_c, [__Digest+2*16]
						paddd		xmm_d, [__Digest+3*16]
						paddd		xmm_e, [__Digest+4*16]
						movdqa		[__Digest+0*16], xmm_a
						movdqa		[__Digest+1*16], xmm_b
						movdqa		[__Digest+2*16], xmm_c
						movdqa		[__Digest+3*16], xmm_d
						movdqa		[__Digest+4*16], xmm_e
						add			dword ptr __Data1, 64
						add			dword ptr __Data2, 64
						add			dword ptr __Data3, 64
						add			dword ptr __Data4, 64
						add			dword ptr __Data5, 64
						add			dword ptr __Data6, 64
						dec			dword ptr __Length
                        jnz			full_blocks
						mov			ebp, __Frame
						mov			ebx, __this1
						mov			edx, __this2
						mov			esi, __this3
						mov			edi, __this4
                        movdqa		xmm0, [__Digest+0*16]
                        movdqa		xmm1, [__Digest+1*16]
                        movdqa		xmm2, [__Digest+2*16]
                        movdqa		xmm3, [__Digest+3*16]
						mov			eax, [__Digest+4*16+0]
						mov			ecx, [__Digest+4*16+4]
                        movdqa		xmm4, xmm0
                        movdqa		xmm5, xmm2
                        punpckldq	xmm0, xmm1
                        punpckldq	xmm2, xmm3
                        punpckhdq	xmm4, xmm1
                        punpckhdq	xmm5, xmm3
                        movdqa		xmm6, xmm0
                        movdqa		xmm7, xmm4
                        punpcklqdq	xmm0, xmm2
                        punpcklqdq	xmm4, xmm5
                        punpckhqdq	xmm6, xmm2
                        punpckhqdq	xmm7, xmm5
                        movdqu		[ebx+m_nHash0], xmm0
                        movdqu		[edx+m_nHash0], xmm6
 						mov			[ebx+m_nHash4], eax
						mov			[edx+m_nHash4], ecx
						mov			eax, [__Digest+4*16+8]
						mov			ecx, [__Digest+4*16+12]
	                    movdqu		[esi+m_nHash0], xmm4
                        movdqu		[edi+m_nHash0], xmm7
						mov			[esi+m_nHash4], eax
						mov			[edi+m_nHash4], ecx
						mov			ebx, __this5
						mov			edx, __this6
						movdqa		xmm0, [__Digest+20*4]
						movdqa		xmm1, [__Digest+20*4+16]
						mov			eax, [__Digest+20*4+32]
						mov			ecx, [__Digest+20*4+36]
						movdqu		[ebx+m_nHash0], xmm0
						mov			[ebx+m_nHash4], eax
						movdqu		[edx+m_nHash0], xmm1
						mov			[edx+m_nHash4], ecx
						mov			esp, ebp
                        popa
                        emms
                        ret 48
SHA_Add6_SSE2			ENDP
       end
