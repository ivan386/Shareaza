; #####################################################################################################################
;
; MD4_asm.asm
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
; MD4_asm - Implementation of MD4 for x86
;
; created              7.7.2004			by Camper			thetruecamper at gmx dot net
;
; modified             2.9.2004			by Camper			ICQ # 105491945
;
; #####################################################################################################################

                        .586p
                        .model      flat, stdcall 
                        option      casemap:none                    ; case sensitive
                        option      prologue:none                   ; we generate our own entry/exit code
                        option      epilogue:none

; #####################################################################################################################

						include		<common.inc>

m_nHash0				equ			0								; offsets as found in MD4.h
m_nHash1				equ			4
m_nHash2				equ			8
m_nHash3				equ			12

m_nCount0				equ			16
m_nCount1				equ			20

m_pBuffer				equ			24

; Some magic numbers for Transform...
MD4_S11                 equ         3
MD4_S12                 equ         7
MD4_S13                 equ         11
MD4_S14                 equ         19
MD4_S21                 equ         3
MD4_S22                 equ         5
MD4_S23                 equ         9
MD4_S24                 equ         13
MD4_S31                 equ         3
MD4_S32                 equ         9
MD4_S33                 equ         11
MD4_S34                 equ         15

						.data
						
const_5A827999			dq			2 dup (5A8279995A827999H)
const_6ED9EBA1			dq			2 dup (6ED9EBA16ED9EBA1H)

MD4FF                   MACRO       count:REQ,s:REQ
; a = (a+x[count]+((b&c)|(~b&d)))rol s
; a = (a+x[count]+(d^(b&(c^d))))rol s
						mov			reg_temp1, reg_c
						xor			reg_c, reg_d
						add			reg_a, [ebp+count*4]
						and			reg_c, reg_b
						xor			reg_c, reg_d
						add			reg_a, reg_c
						rol			reg_a, s
reg_t					textequ		reg_d
reg_d					textequ		reg_temp1
reg_temp1				textequ		reg_c
reg_c					textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
                        ENDM

MD4GG                   MACRO       count:REQ,s:REQ
; a = (a+x[count]+((b&c)|(b&d)|(c&d))+5A827999H) rol s
; a = (a+x[count]+((b&c)|(d&(b|c)))+5A827999H)rol s
						mov			reg_temp2, reg_b
						mov			reg_temp1, reg_b
						add			reg_a, [ebp+count*4]
						or			reg_b, reg_c
						and			reg_temp2, reg_c
						and			reg_b, reg_d
						add			reg_a, 5A827999H
						or			reg_b, reg_temp2
						add			reg_a, reg_b
						rol			reg_a, s
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_temp1
reg_temp1				textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
                        ENDM

MD4HH                   MACRO       count:REQ,s:REQ
; a = (a+x[count]+(b^c^d)+6ED9EBA1H)rol s
						add			reg_a, [ebp+count*4]
						mov			reg_temp1, reg_b
						xor			reg_b, reg_c
						add			reg_a, 6ED9EBA1H
						xor			reg_b, reg_d
						add			reg_a, reg_b
						rol			reg_a, s
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_temp1
reg_temp1				textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
                        ENDM

                        .code

MD4_Transform_p5        PROC                                            ; we expect ebp to point to the Data stream
                                                                        ; all other registers (eax,ebx,ecx,edx,esi,edi) will be destroyed
__this                  textequ     <[esp+32+2*4]>                      ; 1*pusha+2*call
; set alias for registers
reg_a                   textequ     <eax>
reg_b                   textequ     <ebx>
reg_c                   textequ     <ecx>
reg_d                   textequ     <edx>
reg_temp1               textequ     <esi>
reg_temp2               textequ     <edi>
                        mov         reg_temp1, __this
                        mov         reg_a, [reg_temp1+m_nHash0]
                        mov         reg_b, [reg_temp1+m_nHash1]
                        mov         reg_c, [reg_temp1+m_nHash2]
                        mov         reg_d, [reg_temp1+m_nHash3]
; round 1
                        MD4FF        0, MD4_S11
                        MD4FF        1, MD4_S12
                        MD4FF        2, MD4_S13
                        MD4FF        3, MD4_S14
                        MD4FF        4, MD4_S11
                        MD4FF        5, MD4_S12
                        MD4FF        6, MD4_S13
                        MD4FF        7, MD4_S14
                        MD4FF        8, MD4_S11
                        MD4FF        9, MD4_S12
                        MD4FF       10, MD4_S13
                        MD4FF       11, MD4_S14
                        MD4FF       12, MD4_S11
                        MD4FF       13, MD4_S12
                        MD4FF       14, MD4_S13
                        MD4FF       15, MD4_S14
; round 2
                        MD4GG        0, MD4_S21
                        MD4GG        4, MD4_S22
                        MD4GG        8, MD4_S23
                        MD4GG       12, MD4_S24
                        MD4GG        1, MD4_S21
                        MD4GG        5, MD4_S22
                        MD4GG        9, MD4_S23
                        MD4GG       13, MD4_S24
                        MD4GG        2, MD4_S21
                        MD4GG        6, MD4_S22
                        MD4GG       10, MD4_S23
                        MD4GG       14, MD4_S24
                        MD4GG        3, MD4_S21
                        MD4GG        7, MD4_S22
                        MD4GG       11, MD4_S23
                        MD4GG       15, MD4_S24
; round 3
                        MD4HH        0, MD4_S31
                        MD4HH        8, MD4_S32
                        MD4HH        4, MD4_S33
                        MD4HH       12, MD4_S34
                        MD4HH        2, MD4_S31
                        MD4HH       10, MD4_S32
                        MD4HH        6, MD4_S33
                        MD4HH       14, MD4_S34
                        MD4HH        1, MD4_S31
                        MD4HH        9, MD4_S32
                        MD4HH        5, MD4_S33
                        MD4HH       13, MD4_S34
                        MD4HH        3, MD4_S31
                        MD4HH       11, MD4_S32
                        MD4HH        7, MD4_S33
                        MD4HH       15, MD4_S34
                        mov         reg_temp1, __this
                        add         [reg_temp1+m_nHash0], reg_a
                        add         [reg_temp1+m_nHash1], reg_b
                        add         [reg_temp1+m_nHash2], reg_c
                        add         [reg_temp1+m_nHash3], reg_d
                        ret
MD4_Transform_p5        ENDP

MD4_Add_p5              PROC        PUBLIC, _this:DWORD, _Data:DWORD, _nLength:DWORD

                        pusha
__this                  textequ     <[esp+36]>                              ; different offset due to pusha
__Data                  textequ     <[esp+40]>
__nLength               textequ     <[esp+44]>

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
                        call        MD4_Transform_p5
                        add         ebp, 64
                        jmp         full_blocks

end_of_stream:          mov         edi, __this
                        mov         esi, ebp
                        lea         edi, [edi+m_pBuffer]
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
                        mov         byte ptr [edi+m_pBuffer+64+ecx], bl
                        inc         ecx
                        jnz         @B                                      ; offset = 64
                        mov         __Data, ebp
                        lea         ebp, [edi+m_pBuffer]
                        call        MD4_Transform_p5
                        mov         ebp, __Data
                        jmp         full_blocks

short_stream:           sub         ecx, eax                                ;  --> ecx=_nLength
                        mov         esi, ebp
                        lea         edi, [edi+m_pBuffer+eax]
                        rep movsb

get_out:                popa
                        ret 12

MD4_Add_p5              ENDP

MD4_Add1_SSE2			PROC		PUBLIC, _this:DWORD, _Data:DWORD
MD4_Add1_SSE2			ENDP
MD4_Add1_MMX			PROC		PUBLIC, _this:DWORD, _Data:DWORD
MD4_Add1_MMX			ENDP
MD4_Add1_p5				PROC		PUBLIC, _this:DWORD, _Data:DWORD
                        pusha
__this                  textequ     <[esp+36]>
__Data                  textequ     <[esp+40]>
__nLength				textequ		<dword ptr [esp+12]>
                        mov         ebp, __Data
                        mov         edi, __this
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			edx, edx
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], edx
						mov         __nLength, ecx
full_blocks:			call        MD4_Transform_p5
                        mov         ebp, __Data
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data, ebp
                        jnz			full_blocks
                        popa
                        ret 8
MD4_Add1_p5				ENDP

MD4_Add2_SSE2			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD
MD4_Add2_SSE2			ENDP
MD4_Add2_MMX			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD
MD4_Add2_MMX			ENDP
MD4_Add2_p5				PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD
                        pusha
__this1                 textequ     <[esp+36]>
__Data1                 textequ     <[esp+40]>
__this2                 textequ     <[esp+44]>
__Data2                 textequ     <[esp+48]>
__nLength				textequ		<dword ptr [esp+12]>
                        mov         ebp, __Data1
                        mov         edi, __this1
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			edx, edx
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], edx
						mov         __nLength, ecx
full_blocks1:			call        MD4_Transform_p5
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
full_blocks2:			call        MD4_Transform_p5
                        mov         ebp, __Data2
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data2, ebp
                        jnz			full_blocks2

                        popa
                        ret 16
MD4_Add2_p5				ENDP

MD4_Add3_p5				PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD
                        pusha
__this1                 textequ     <[esp+36]>
__Data1                 textequ     <[esp+40]>
__this2                 textequ     <[esp+44]>
__Data2                 textequ     <[esp+48]>
__this3                 textequ     <[esp+52]>
__Data3                 textequ     <[esp+56]>
__nLength				textequ		<dword ptr [esp+12]>
                        mov         ebp, __Data1
                        mov         edi, __this1
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			edx, edx
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], edx
						mov         __nLength, ecx
full_blocks1:			call        MD4_Transform_p5
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
full_blocks2:			call        MD4_Transform_p5
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
full_blocks3:			call        MD4_Transform_p5
                        mov         ebp, __Data3
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data3, ebp
                        jnz			full_blocks3

                        popa
                        ret 24
MD4_Add3_p5				ENDP

MD4_Add4_p5				PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD, _this4:DWORD, _Data4:DWORD
                        pusha
__this1                 textequ     <[esp+36]>
__Data1                 textequ     <[esp+40]>
__this2                 textequ     <[esp+44]>
__Data2                 textequ     <[esp+48]>
__this3                 textequ     <[esp+52]>
__Data3                 textequ     <[esp+56]>
__this4                 textequ     <[esp+60]>
__Data4                 textequ     <[esp+64]>
__nLength				textequ		<dword ptr [esp+12]>
                        mov         ebp, __Data1
                        mov         edi, __this1
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			edx, edx
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], edx
						mov         __nLength, ecx
full_blocks1:			call        MD4_Transform_p5
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
full_blocks2:			call        MD4_Transform_p5
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
full_blocks3:			call        MD4_Transform_p5
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
full_blocks4:			call        MD4_Transform_p5
                        mov         ebp, __Data4
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data4, ebp
                        jnz			full_blocks4

                        popa
                        ret 32
MD4_Add4_p5				ENDP

MD4_Add5_MMX			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD, _this4:DWORD, _Data4:DWORD, _this5:DWORD, _Data5:DWORD
MD4_Add5_MMX			ENDP
MD4_Add5_p5				PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD, _this4:DWORD, _Data4:DWORD, _this5:DWORD, _Data5:DWORD
                        pusha
__this1                 textequ     <[esp+36]>
__Data1                 textequ     <[esp+40]>
__this2                 textequ     <[esp+44]>
__Data2                 textequ     <[esp+48]>
__this3                 textequ     <[esp+52]>
__Data3                 textequ     <[esp+56]>
__this4                 textequ     <[esp+60]>
__Data4                 textequ     <[esp+64]>
__this5                 textequ     <[esp+68]>
__Data5                 textequ     <[esp+72]>
__nLength				textequ		<dword ptr [esp+12]>
                        mov         ebp, __Data1
                        mov         edi, __this1
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			edx, edx
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], edx
						mov         __nLength, ecx
full_blocks1:			call        MD4_Transform_p5
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
full_blocks2:			call        MD4_Transform_p5
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
full_blocks3:			call        MD4_Transform_p5
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
full_blocks4:			call        MD4_Transform_p5
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
full_blocks5:			call        MD4_Transform_p5
                        mov         ebp, __Data5
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data5, ebp
                        jnz			full_blocks5

                        popa
                        ret 40
MD4_Add5_p5				ENDP

MD4_Add6_MMX			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD, _this4:DWORD, _Data4:DWORD, _this5:DWORD, _Data5:DWORD, _this6:DWORD, _Data6:DWORD
MD4_Add6_MMX			ENDP
MD4_Add6_p5				PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD, _this4:DWORD, _Data4:DWORD, _this5:DWORD, _Data5:DWORD, _this6:DWORD, _Data6:DWORD
                        pusha
__this1                 textequ     <[esp+36]>
__Data1                 textequ     <[esp+40]>
__this2                 textequ     <[esp+44]>
__Data2                 textequ     <[esp+48]>
__this3                 textequ     <[esp+52]>
__Data3                 textequ     <[esp+56]>
__this4                 textequ     <[esp+60]>
__Data4                 textequ     <[esp+64]>
__this5                 textequ     <[esp+68]>
__Data5                 textequ     <[esp+72]>
__this6                 textequ     <[esp+76]>
__Data6                 textequ     <[esp+80]>
__nLength				textequ		<dword ptr [esp+12]>
                        mov         ebp, __Data1
                        mov         edi, __this1
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			edx, edx
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], edx
						mov         __nLength, ecx
full_blocks1:			call        MD4_Transform_p5
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
full_blocks2:			call        MD4_Transform_p5
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
full_blocks3:			call        MD4_Transform_p5
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
full_blocks4:			call        MD4_Transform_p5
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
full_blocks5:			call        MD4_Transform_p5
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
full_blocks6:			call        MD4_Transform_p5
                        mov         ebp, __Data6
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data6, ebp
                        jnz			full_blocks6

                        popa
                        ret 48
MD4_Add6_p5				ENDP

						.mmx

MD4FF3_MMX				MACRO		count:REQ,s:REQ
; a = (a+x[count]+((b&c)|(~b&d)))rol s
; a = (a+x[count]+(d^(b&(c^d))))rol s
						paddd		mmx_a, [__w+count*8]
						mov			reg_temp1, reg_c
						movq		mmx_temp1, mmx_c
						xor			reg_c, reg_d
						pxor		mmx_c, mmx_d
						add			reg_a, [ebp+count*4]
						pand		mmx_c, mmx_b
						and			reg_c, reg_b
						pxor		mmx_c, mmx_d
						xor			reg_c, reg_d
						paddd		mmx_a, mmx_c
						add			reg_a, reg_c
						movq		mmx_temp2, mmx_a
						rol			reg_a, s
						pslld		mmx_a, s
						psrld		mmx_temp2, 32-s
						por			mmx_a, mmx_temp2
mmx_t					textequ		mmx_d
mmx_d					textequ		mmx_temp1
mmx_temp1				textequ		mmx_c
mmx_c					textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		mmx_t
reg_t					textequ		reg_d
reg_d					textequ		reg_temp1
reg_temp1				textequ		reg_c
reg_c					textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD4FF3_MMX

MD4GG3_MMX				MACRO		count:REQ,s:REQ
; a = (a+x[count]+((b&c)|(b&d)|(c&d))+5A827999H) rol s
; a = (a+x[count]+((b&c)|(d&(b|c)))+5A827999H)rol s
						paddd		mmx_a, [__w+count*8]
						mov			reg_temp2, reg_b
						movq		mmx_temp2, mmx_b
						mov			reg_temp1, reg_b
						movq		mmx_temp1, mmx_b
						add			reg_a, [ebp+count*4]
						por			mmx_b, mmx_c
						or			reg_b, reg_c
						pand		mmx_temp2, mmx_c
						and			reg_temp2, reg_c
						pand		mmx_b, mmx_d
						and			reg_b, reg_d
						paddd		mmx_a, mmx_5A827999
						or			reg_b, reg_temp2
						por			mmx_b, mmx_temp2
						add			reg_a, 5A827999H
						paddd		mmx_a, mmx_b
						add			reg_a, reg_b
						movq		mmx_temp2, mmx_a
						rol			reg_a, s
						pslld		mmx_a, s
						psrld		mmx_temp2, 32-s
						por			mmx_a, mmx_temp2
mmx_t					textequ		mmx_d
mmx_d					textequ		mmx_c
mmx_c					textequ		mmx_temp1
mmx_temp1				textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		mmx_t
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_temp1
reg_temp1				textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD4GG3_MMX

MD4HH3_MMX				MACRO		count:REQ,s:REQ
; a = (a+x[count]+(b^c^d)+6ED9EBA1H)rol s
						movq		mmx_temp1, mmx_b
						add			reg_a, 6ED9EBA1H
						paddd		mmx_a, [__w+count*8]
						mov			reg_temp1, reg_b
						pxor		mmx_b, mmx_c
						xor			reg_b, reg_c
						paddd		mmx_a, mmx_6ED9EBA1
						add			reg_a, [ebp+count*4]
						pxor		mmx_b, mmx_d
						xor			reg_b, reg_d
						paddd		mmx_a, mmx_b
						add			reg_a, reg_b
						movq		mmx_temp2, mmx_a
						rol			reg_a, s
						pslld		mmx_a, s
						psrld		mmx_temp2, 32-s
						por			mmx_a, mmx_temp2
mmx_t					textequ		mmx_d
mmx_d					textequ		mmx_c
mmx_c					textequ		mmx_temp1
mmx_temp1				textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		mmx_t
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_temp1
reg_temp1				textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD4HH3_MMX

MD4_Add3_SSE2			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD
MD4_Add3_SSE2			ENDP
MD4_Add3_MMX			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD
                        pusha
__this1                 textequ     <[ebp+36]>
__Data1                 textequ     <[ebp+40]>
__this2                 textequ     <[ebp+44]>
__Data2                 textequ     <[ebp+48]>
__this3                 textequ     <[ebp+52]>
__Data3                 textequ     <[ebp+56]>
						mov			ebp, esp
                        sub         esp, 64*4+16*3+16
                        and			esp, 0ffffff80h						; align stack pointer
                        mov			eax, __Data1
                        mov			ebx, __Data2
__w						textequ		<esp>
__Digest				textequ		<esp+64*4>
__Data1					textequ		<[esp+64*4+16*3]>
__Data2					textequ		<[esp+64*4+16*3+4]>
__Count					textequ		<[esp+64*4+16*3+8]>
__Frame					textequ		<[esp+64*4+16*3+12]>
						mov			__Data1, eax
						mov			__Data2, ebx
						mov			__Frame, ebp
                        mov			ebx, __this1
                        mov			edx, __this2
                        mov			edi, __this3
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			ecx, ecx
                        add         [ebx+m_nCount0], eax
                        adc         [ebx+m_nCount1], ecx
                        add         [edx+m_nCount0], eax
                        adc         [edx+m_nCount1], ecx
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], ecx
                        movq		mm0, [ebx+m_nHash0]
                        movq		mm1, [ebx+m_nHash2]
                        movq		mm2, [edx+m_nHash0]
                        movq		mm3, [edx+m_nHash2]
                        movq		mm6, [edi+m_nHash0]
                        movq		mm7, [edi+m_nHash2]
                        movq		mm4, mm0
                        movq		mm5, mm1
                        punpckldq	mm0, mm2
                        punpckldq	mm1, mm3
                        punpckhdq	mm4, mm2
                        punpckhdq	mm5, mm3
                        movq		[__Digest+0*8], mm0
                        movq		[__Digest+1*8], mm4
                        movq		[__Digest+2*8], mm1
                        movq		[__Digest+3*8], mm5
                        movq		[__Digest+4*8], mm6
                        movq		[__Digest+5*8], mm7
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        mov			__Count, ecx
mmx_a					textequ		<mm0>
mmx_b					textequ		<mm1>
mmx_c					textequ		<mm2>
mmx_d					textequ		<mm3>
mmx_temp1				textequ		<mm4>
mmx_temp2				textequ		<mm5>
mmx_5A827999			textequ		<mm6>
mmx_6ED9EBA1			textequ		<mm7>
						movq		mmx_5A827999, const_5A827999
						movq		mmx_6ED9EBA1, const_6ED9EBA1
reg_a					textequ		<eax>
reg_b					textequ		<ebx>
reg_c					textequ		<ecx>
reg_d					textequ		<edx>
reg_temp1				textequ		<esi>
reg_temp2				textequ		<edi>
						mov			ebp, __Data3
full_blocks:
count					=			0
						mov			ebx, __Data1
						mov			edx, __Data2
						REPEAT		4
                        movq		mm0, [ebx+count*16]
                        movq		mm1, [ebx+count*16+8]
                        movq		mm2, [edx+count*16]
                        movq		mm3, [edx+count*16+8]
                        movq		mm4, mm0
                        movq		mm5, mm1
                        punpckldq	mm0, mm2
                        punpckldq	mm1, mm3
                        punpckhdq	mm4, mm2
                        punpckhdq	mm5, mm3
                        movq		[__w+count*4*8+0*8], mm0
                        movq		[__w+count*4*8+1*8], mm4
                        movq		[__w+count*4*8+2*8], mm1
                        movq		[__w+count*4*8+3*8], mm5
count					=			count + 1
						ENDM
                        movq		mmx_a, [__Digest+0*8]
                        movq		mmx_b, [__Digest+1*8]
                        movq		mmx_c, [__Digest+2*8]
                        movq		mmx_d, [__Digest+3*8]
                        mov			reg_a, [__Digest+4*8]
                        mov			reg_b, [__Digest+4*8+4]
                        mov			reg_c, [__Digest+4*8+8]
                        mov			reg_d, [__Digest+4*8+12]
; round 1
                        MD4FF3_MMX        0, MD4_S11
                        MD4FF3_MMX        1, MD4_S12
                        MD4FF3_MMX        2, MD4_S13
                        MD4FF3_MMX        3, MD4_S14
                        MD4FF3_MMX        4, MD4_S11
                        MD4FF3_MMX        5, MD4_S12
                        MD4FF3_MMX        6, MD4_S13
                        MD4FF3_MMX        7, MD4_S14
                        MD4FF3_MMX        8, MD4_S11
                        MD4FF3_MMX        9, MD4_S12
                        MD4FF3_MMX       10, MD4_S13
                        MD4FF3_MMX       11, MD4_S14
                        MD4FF3_MMX       12, MD4_S11
                        MD4FF3_MMX       13, MD4_S12
                        MD4FF3_MMX       14, MD4_S13
                        MD4FF3_MMX       15, MD4_S14
; round 2
                        MD4GG3_MMX        0, MD4_S21
                        MD4GG3_MMX        4, MD4_S22
                        MD4GG3_MMX        8, MD4_S23
                        MD4GG3_MMX       12, MD4_S24
                        MD4GG3_MMX        1, MD4_S21
                        MD4GG3_MMX        5, MD4_S22
                        MD4GG3_MMX        9, MD4_S23
                        MD4GG3_MMX       13, MD4_S24
                        MD4GG3_MMX        2, MD4_S21
                        MD4GG3_MMX        6, MD4_S22
                        MD4GG3_MMX       10, MD4_S23
                        MD4GG3_MMX       14, MD4_S24
                        MD4GG3_MMX        3, MD4_S21
                        MD4GG3_MMX        7, MD4_S22
                        MD4GG3_MMX       11, MD4_S23
                        MD4GG3_MMX       15, MD4_S24
; round 3
                        MD4HH3_MMX        0, MD4_S31
                        MD4HH3_MMX        8, MD4_S32
                        MD4HH3_MMX        4, MD4_S33
                        MD4HH3_MMX       12, MD4_S34
                        MD4HH3_MMX        2, MD4_S31
                        MD4HH3_MMX       10, MD4_S32
                        MD4HH3_MMX        6, MD4_S33
                        MD4HH3_MMX       14, MD4_S34
                        MD4HH3_MMX        1, MD4_S31
                        MD4HH3_MMX        9, MD4_S32
                        MD4HH3_MMX        5, MD4_S33
                        MD4HH3_MMX       13, MD4_S34
                        MD4HH3_MMX        3, MD4_S31
                        MD4HH3_MMX       11, MD4_S32
                        MD4HH3_MMX        7, MD4_S33
                        MD4HH3_MMX       15, MD4_S34
                        paddd		mmx_a, [__Digest+0*8]
                        paddd		mmx_b, [__Digest+1*8]
                        paddd		mmx_c, [__Digest+2*8]
                        paddd		mmx_d, [__Digest+3*8]
                        movq		[__Digest+0*8], mmx_a
                        movq		[__Digest+1*8], mmx_b
                        movq		[__Digest+2*8], mmx_c
                        movq		[__Digest+3*8], mmx_d
                        add			[__Digest+4*8], reg_a
                        add			[__Digest+4*8+4], reg_b
                        add			[__Digest+4*8+8], reg_c
                        add			[__Digest+4*8+12], reg_d
						add			dword ptr __Data1, 64
						add			dword ptr __Data2, 64
						add			ebp, 64
						dec			dword ptr __Count
                        jnz			full_blocks
                        mov			ebp, __Frame
						mov			ebx, __this1
						mov			edx, __this2
						mov			edi, __this3
                        movq		mm0, [__Digest+0*8]
                        movq		mm1, [__Digest+1*8]
                        movq		mm2, [__Digest+2*8]
                        movq		mm3, [__Digest+3*8]
                        movq		mm6, [__Digest+4*8]
                        movq		mm7, [__Digest+5*8]
                        movq		mm4, mm0
                        movq		mm5, mm2
                        punpckldq	mm0, mm1
                        punpckldq	mm2, mm3
                        punpckhdq	mm4, mm1
                        punpckhdq	mm5, mm3
                        movq		[ebx+m_nHash0], mm0
                        movq		[ebx+m_nHash2], mm2
                        movq		[edx+m_nHash0], mm4
                        movq		[edx+m_nHash2], mm5
                        movq		[edi+m_nHash0], mm6
                        movq		[edi+m_nHash2], mm7
						mov			esp, ebp
                        popa
                        emms
                        ret 24
MD4_Add3_MMX			ENDP

MD4FF4_FF4_MMX			MACRO		count:REQ, s:REQ, count1:REQ, s1:REQ, count2:REQ, s2:REQ
; a = (a+x[count]+((b&c)|(~b&d)))rol s
; a = (a+x[count]+(d^(b&(c^d))))rol s
						paddd		mmx_a, [__w+count*8]
						mov			reg_temp1, reg_c
						xor			reg_c, reg_d
						movq		mmx_temp1, mmx_c
						add			reg_a, [ebp+count1*4]
						and			reg_c, reg_b
						pxor		mmx_c, mmx_d
						xor			reg_c, reg_d
						pand		mmx_c, mmx_b
						add			reg_a, reg_c
						mov			reg_c, reg_b
						pxor		mmx_c, mmx_d
						rol			reg_a, s1
						paddd		mmx_a, mmx_c
						xor			reg_b, reg_temp1
						add			reg_d, [ebp+count2*4]
						movq		mmx_temp2, mmx_a
						and			reg_b, reg_a
						pslld		mmx_a, s
						xor			reg_b, reg_temp1
						psrld		mmx_temp2, 32-s
						add			reg_d, reg_b
						por			mmx_a, mmx_temp2
						rol			reg_d, s2
mmx_t					textequ		mmx_d
mmx_d					textequ		mmx_temp1
mmx_temp1				textequ		mmx_c
mmx_c					textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		mmx_t
reg_t					textequ		reg_d
reg_d					textequ		reg_temp1
reg_temp1				textequ		reg_c
reg_c					textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
reg_t					textequ		reg_d
reg_d					textequ		reg_temp1
reg_temp1				textequ		reg_c
reg_c					textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD4FF4_FF4_MMX

MD4FF4_GG4_MMX			MACRO		count:REQ, s:REQ, count1:REQ, s1:REQ, count2:REQ, s2:REQ
; a = (a+x[count]+((b&c)|(~b&d)))rol s
; a = (a+x[count]+(d^(b&(c^d))))rol s
						movq		mmx_temp1, mmx_c
						mov			reg_temp2, reg_b
						mov			reg_temp1, reg_b
						paddd		mmx_a, [__w+count*8]
						add			reg_a, 5A827999H
						or			reg_b, reg_c
						pxor		mmx_c, mmx_d
						and			reg_temp2, reg_c
						and			reg_b, reg_d
						pand		mmx_c, mmx_b
						add			reg_a, [ebp+count1*4]
						or			reg_b, reg_temp2
						pxor		mmx_c, mmx_d
						add			reg_a, reg_b
						paddd		mmx_a, mmx_c
						rol			reg_a, s1
						movq		mmx_temp2, mmx_a
						mov			reg_temp2, reg_a
						mov			reg_b, reg_a
						pslld		mmx_a, s
						add			reg_d, [ebp+count2*4]
						or			reg_a, reg_temp1
						psrld		mmx_temp2, 32-s
						and			reg_temp2, reg_temp1
						and			reg_a, reg_c
						por			mmx_a, mmx_temp2
						add			reg_d, 5A827999H
						or			reg_a, reg_temp2
						add			reg_d, reg_a
						rol			reg_d, s2
mmx_t					textequ		mmx_d
mmx_d					textequ		mmx_temp1
mmx_temp1				textequ		mmx_c
mmx_c					textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		mmx_t
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_temp1
reg_temp1				textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_temp1
reg_temp1				textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD4FF4_GG4_MMX
                        
MD4GG4_HH4_MMX			MACRO		count:REQ, s:REQ, count1:REQ, s1:REQ, count2:REQ, s2:REQ
; a = (a+x[count]+((b&c)|(b&d)|(c&d))+5A827999H) rol s
; a = (a+x[count]+((b&c)|(d&(b|c)))+5A827999H)rol s
						movq		mmx_temp2, mmx_b
						add			reg_a, [ebp+count1*4]
						mov			reg_temp1, reg_b
						movq		mmx_temp1, mmx_b
						xor			reg_b, reg_c
						add			reg_a, 6ED9EBA1H
						paddd		mmx_a, [__w+count*8]
						xor			reg_b, reg_d
						por			mmx_b, mmx_c
						add			reg_a, reg_b
						pand		mmx_temp2, mmx_c
						rol			reg_a, s1
						pand		mmx_b, mmx_d
						add			reg_d, [ebp+count2*4]
						mov			reg_b, reg_a
						paddd		mmx_a, mmx_5A827999
						xor			reg_a, reg_temp1
						add			reg_d, 6ED9EBA1H
						por			mmx_b, mmx_temp2
						xor			reg_a, reg_c
						paddd		mmx_a, mmx_b
						add			reg_d, reg_a
						movq		mmx_temp2, mmx_a
						rol			reg_d, s2
						pslld		mmx_a, s
						psrld		mmx_temp2, 32-s
						por			mmx_a, mmx_temp2
mmx_t					textequ		mmx_d
mmx_d					textequ		mmx_c
mmx_c					textequ		mmx_temp1
mmx_temp1				textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		mmx_t
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_temp1
reg_temp1				textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_temp1
reg_temp1				textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD4GG4_HH4_MMX

MD4GG4_FF4_MMX			MACRO		count:REQ, s:REQ, count1:REQ, s1:REQ, count2:REQ, s2:REQ
; a = (a+x[count]+((b&c)|(b&d)|(c&d))+5A827999H) rol s
; a = (a+x[count]+((b&c)|(d&(b|c)))+5A827999H)rol s
						movq		mmx_temp2, mmx_b
						mov			reg_temp1, reg_c
						xor			reg_c, reg_d
						paddd		mmx_a, [__w+count*8]
						and			reg_c, reg_b
						movq		mmx_temp1, mmx_b
						add			reg_a, [ebp+count1*4]
						xor			reg_c, reg_d
						por			mmx_b, mmx_c
						add			reg_a, reg_c
						mov			reg_c, reg_b
						pand		mmx_temp2, mmx_c
						rol			reg_a, s1
						pand		mmx_b, mmx_d
						xor			reg_b, reg_temp1
						add			reg_d, [ebp+count2*4]
						paddd		mmx_a, mmx_5A827999
						and			reg_b, reg_a
						por			mmx_b, mmx_temp2
						xor			reg_b, reg_temp1
						paddd		mmx_a, mmx_b
						add			reg_d, reg_b
						movq		mmx_temp2, mmx_a
						rol			reg_d, s2
						pslld		mmx_a, s
						psrld		mmx_temp2, 32-s
						por			mmx_a, mmx_temp2
mmx_t					textequ		mmx_d
mmx_d					textequ		mmx_c
mmx_c					textequ		mmx_temp1
mmx_temp1				textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		mmx_t
reg_t					textequ		reg_d
reg_d					textequ		reg_temp1
reg_temp1				textequ		reg_c
reg_c					textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
reg_t					textequ		reg_d
reg_d					textequ		reg_temp1
reg_temp1				textequ		reg_c
reg_c					textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD4GG4_FF4_MMX

MD4HH4_GG4_MMX			MACRO       count:REQ, s:REQ, count1:REQ, s1:REQ, count2:REQ, s2:REQ
; a = (a+x[count]+(b^c^d)+6ED9EBA1H)rol s
						movq		mmx_temp1, mmx_b
						mov			reg_temp2, reg_b
						mov			reg_temp1, reg_b
						paddd		mmx_a, [__w+count*8]
						add			reg_a, 5A827999H
						or			reg_b, reg_c
						pxor		mmx_b, mmx_c
						and			reg_temp2, reg_c
						and			reg_b, reg_d
						paddd		mmx_a, mmx_6ED9EBA1
						add			reg_a, [ebp+count1*4]
						or			reg_b, reg_temp2
						pxor		mmx_b, mmx_d
						add			reg_a, reg_b
						paddd		mmx_a, mmx_b
						rol			reg_a, s1
						movq		mmx_temp2, mmx_a
						mov			reg_temp2, reg_a
						mov			reg_b, reg_a
						pslld		mmx_a, s
						add			reg_d, [ebp+count2*4]
						or			reg_a, reg_temp1
						psrld		mmx_temp2, 32-s
						and			reg_temp2, reg_temp1
						and			reg_a, reg_c
						por			mmx_a, mmx_temp2
						add			reg_d, 5A827999H
						or			reg_a, reg_temp2
						add			reg_d, reg_a
						rol			reg_d, s2
mmx_t					textequ		mmx_d
mmx_d					textequ		mmx_c
mmx_c					textequ		mmx_temp1
mmx_temp1				textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		mmx_t
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_temp1
reg_temp1				textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_temp1
reg_temp1				textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD4HH4_GG4_MMX

MD4HH4_HH4_MMX			MACRO       count:REQ, s:REQ, count1:REQ, s1:REQ, count2:REQ, s2:REQ
; a = (a+x[count]+(b^c^d)+6ED9EBA1H)rol s
						movq		mmx_temp1, mmx_b
						add			reg_a, 6ED9EBA1H
						mov			reg_temp1, reg_b
						paddd		mmx_a, [__w+count*8]
						add			reg_a, [ebp+count1*4]
						xor			reg_b, reg_c
						pxor		mmx_b, mmx_c
						paddd		mmx_a, mmx_6ED9EBA1
						xor			reg_b, reg_d
						pxor		mmx_b, mmx_d
						add			reg_a, reg_b
						paddd		mmx_a, mmx_b
						rol			reg_a, s1
						add			reg_d, [ebp+count2*4]
						movq		mmx_temp2, mmx_a
						mov			reg_b, reg_a
						xor			reg_a, reg_temp1
						pslld		mmx_a, s
						add			reg_d, 6ED9EBA1H
						xor			reg_a, reg_c
						psrld		mmx_temp2, 32-s
						add			reg_d, reg_a
						por			mmx_a, mmx_temp2
						rol			reg_d, s2
mmx_t					textequ		mmx_d
mmx_d					textequ		mmx_c
mmx_c					textequ		mmx_temp1
mmx_temp1				textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		mmx_t
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_temp1
reg_temp1				textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_temp1
reg_temp1				textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD4HH4_HH4_MMX

MD4_Add4_SSE2			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD, _this4:DWORD, _Data4:DWORD
MD4_Add4_SSE2			ENDP
MD4_Add4_MMX			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD, _this4:DWORD, _Data4:DWORD
                        pusha
__this1                 textequ     <[ebp+36]>
__Data1                 textequ     <[ebp+40]>
__this2                 textequ     <[ebp+44]>
__Data2                 textequ     <[ebp+48]>
__this3                 textequ     <[ebp+52]>
__Data3                 textequ     <[ebp+56]>
__this4                 textequ     <[ebp+60]>
__Data4                 textequ     <[ebp+64]>
						mov			ebp, esp
                        sub         esp, 64*2+16*4+24
                        and			esp, 0ffffff80h						; align stack pointer
                        mov			eax, __Data1
                        mov			ebx, __Data2
                        mov			ecx, __Data3
                        mov			edx, __Data4
__w						textequ		<esp>
__Digest				textequ		<esp+64*2>
__Data1					textequ		<[esp+64*2+16*4]>
__Data2					textequ		<[esp+64*2+16*4+4]>
__Data3					textequ		<[esp+64*2+16*4+8]>
__Data4					textequ		<[esp+64*2+16*4+12]>
__Count					textequ		<[esp+64*2+16*4+16]>
__Frame					textequ		<[esp+64*2+16*4+20]>
						mov			__Data1, eax
						mov			__Data2, ebx
						mov			__Data3, ecx
						mov			__Data4, edx
						mov			__Frame, ebp
                        mov			ebx, __this1
                        mov			edx, __this2
                        mov			esi, __this3
                        mov			edi, __this4
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
                        movq		mm0, [ebx+m_nHash0]
                        movq		mm1, [ebx+m_nHash2]
                        movq		mm2, [edx+m_nHash0]
                        movq		mm3, [edx+m_nHash2]
                        movq		mm4, mm0
                        movq		mm5, mm1
                        punpckldq	mm0, mm2
                        punpckldq	mm1, mm3
                        punpckhdq	mm4, mm2
                        punpckhdq	mm5, mm3
                        movq		[__Digest+0*8], mm0
                        movq		[__Digest+1*8], mm4
                        movq		[__Digest+2*8], mm1
                        movq		[__Digest+3*8], mm5
                        movq		mm0, [esi+m_nHash0]
                        movq		mm1, [esi+m_nHash2]
                        movq		mm2, [edi+m_nHash0]
                        movq		mm3, [edi+m_nHash2]
                        movq		[__Digest+2*16+0*8], mm0
                        movq		[__Digest+2*16+1*8], mm1
                        movq		[__Digest+2*16+2*8], mm2
                        movq		[__Digest+2*16+3*8], mm3
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        mov			__Count, ecx
mmx_a					textequ		<mm0>
mmx_b					textequ		<mm1>
mmx_c					textequ		<mm2>
mmx_d					textequ		<mm3>
mmx_temp1				textequ		<mm4>
mmx_temp2				textequ		<mm5>
mmx_5A827999			textequ		<mm6>
mmx_6ED9EBA1			textequ		<mm7>
						movq		mmx_5A827999, const_5A827999
						movq		mmx_6ED9EBA1, const_6ED9EBA1
reg_a					textequ		<eax>
reg_b					textequ		<ebx>
reg_c					textequ		<ecx>
reg_d					textequ		<edx>
reg_temp1				textequ		<esi>
reg_temp2				textequ		<edi>
full_blocks:
count					=			0
						mov			ebx, __Data1
						mov			edx, __Data2
						REPEAT		4
                        movq		mm0, [ebx+count*16]
                        movq		mm1, [ebx+count*16+8]
                        movq		mm2, [edx+count*16]
                        movq		mm3, [edx+count*16+8]
                        movq		mm4, mm0
                        movq		mm5, mm1
                        punpckldq	mm0, mm2
                        punpckldq	mm1, mm3
                        punpckhdq	mm4, mm2
                        punpckhdq	mm5, mm3
                        movq		[__w+count*4*8+0*8], mm0
                        movq		[__w+count*4*8+1*8], mm4
                        movq		[__w+count*4*8+2*8], mm1
                        movq		[__w+count*4*8+3*8], mm5
count					=			count + 1
						ENDM
                        movq		mmx_a, [__Digest+0*8]
                        movq		mmx_b, [__Digest+1*8]
                        movq		mmx_c, [__Digest+2*8]
                        movq		mmx_d, [__Digest+3*8]
						mov			ebp, __Data3
                        mov			reg_a, [__Digest+2*16]
                        mov			reg_b, [__Digest+2*16+4]
                        mov			reg_c, [__Digest+2*16+8]
                        mov			reg_d, [__Digest+2*16+12]
; round 1
                        MD4FF4_FF4_MMX		 0, MD4_S11,  0, MD4_S11,  1, MD4_S12
                        MD4FF4_FF4_MMX		 1, MD4_S12,  2, MD4_S13,  3, MD4_S14
                        MD4FF4_FF4_MMX		 2, MD4_S13,  4, MD4_S11,  5, MD4_S12
                        MD4FF4_FF4_MMX		 3, MD4_S14,  6, MD4_S13,  7, MD4_S14
                        MD4FF4_FF4_MMX		 4, MD4_S11,  8, MD4_S11,  9, MD4_S12
                        MD4FF4_FF4_MMX		 5, MD4_S12, 10, MD4_S13, 11, MD4_S14
                        MD4FF4_FF4_MMX		 6, MD4_S13, 12, MD4_S11, 13, MD4_S12
                        MD4FF4_FF4_MMX		 7, MD4_S14, 14, MD4_S13, 15, MD4_S14
                        MD4FF4_GG4_MMX		 8, MD4_S11,  0, MD4_S21,  4, MD4_S22
                        MD4FF4_GG4_MMX		 9, MD4_S12,  8, MD4_S23, 12, MD4_S24
                        MD4FF4_GG4_MMX		10, MD4_S13,  1, MD4_S21,  5, MD4_S22
                        MD4FF4_GG4_MMX		11, MD4_S14,  9, MD4_S23, 13, MD4_S24
                        MD4FF4_GG4_MMX		12, MD4_S11,  2, MD4_S21,  6, MD4_S22
                        MD4FF4_GG4_MMX		13, MD4_S12, 10, MD4_S23, 14, MD4_S24
                        MD4FF4_GG4_MMX		14, MD4_S13,  3, MD4_S21,  7, MD4_S22
                        MD4FF4_GG4_MMX		15, MD4_S14, 11, MD4_S23, 15, MD4_S24
; round 2
                        MD4GG4_HH4_MMX		 0, MD4_S21,  0, MD4_S31,  8, MD4_S32
                        MD4GG4_HH4_MMX		 4, MD4_S22,  4, MD4_S33, 12, MD4_S34
                        MD4GG4_HH4_MMX		 8, MD4_S23,  2, MD4_S31, 10, MD4_S32
                        MD4GG4_HH4_MMX		12, MD4_S24,  6, MD4_S33, 14, MD4_S34
                        MD4GG4_HH4_MMX		 1, MD4_S21,  1, MD4_S31,  9, MD4_S32
                        MD4GG4_HH4_MMX		 5, MD4_S22,  5, MD4_S33, 13, MD4_S34
                        MD4GG4_HH4_MMX		 9, MD4_S23,  3, MD4_S31, 11, MD4_S32
                        MD4GG4_HH4_MMX		13, MD4_S24,  7, MD4_S33, 15, MD4_S34
                        add			[__Digest+2*16], reg_a
                        add			[__Digest+2*16+4], reg_b
                        add			[__Digest+2*16+8], reg_c
                        add			[__Digest+2*16+12], reg_d
; round 3
						mov			ebp, __Data4
                        mov			reg_a, [__Digest+3*16]
                        mov			reg_b, [__Digest+3*16+4]
                        mov			reg_c, [__Digest+3*16+8]
                        mov			reg_d, [__Digest+3*16+12]
                        MD4GG4_FF4_MMX		 2, MD4_S21,  0, MD4_S11,  1, MD4_S12
                        MD4GG4_FF4_MMX		 6, MD4_S22,  2, MD4_S13,  3, MD4_S14
                        MD4GG4_FF4_MMX		10, MD4_S23,  4, MD4_S11,  5, MD4_S12
                        MD4GG4_FF4_MMX		14, MD4_S24,  6, MD4_S13,  7, MD4_S14
                        MD4GG4_FF4_MMX		 3, MD4_S21,  8, MD4_S11,  9, MD4_S12
                        MD4GG4_FF4_MMX		 7, MD4_S22, 10, MD4_S13, 11, MD4_S14
                        MD4GG4_FF4_MMX		11, MD4_S23, 12, MD4_S11, 13, MD4_S12
                        MD4GG4_FF4_MMX		15, MD4_S24, 14, MD4_S13, 15, MD4_S14
; round 3
                        MD4HH4_GG4_MMX		 0, MD4_S31,  0, MD4_S21,  4, MD4_S22
                        MD4HH4_GG4_MMX		 8, MD4_S32,  8, MD4_S23, 12, MD4_S24
                        MD4HH4_GG4_MMX		 4, MD4_S33,  1, MD4_S21,  5, MD4_S22
                        MD4HH4_GG4_MMX		12, MD4_S34,  9, MD4_S23, 13, MD4_S24
                        MD4HH4_GG4_MMX		 2, MD4_S31,  2, MD4_S21,  6, MD4_S22
                        MD4HH4_GG4_MMX		10, MD4_S32, 10, MD4_S23, 14, MD4_S24
                        MD4HH4_GG4_MMX		 6, MD4_S33,  3, MD4_S21,  7, MD4_S22
                        MD4HH4_GG4_MMX		14, MD4_S34, 11, MD4_S23, 15, MD4_S24
                        MD4HH4_HH4_MMX		 1, MD4_S31,  0, MD4_S31,  8, MD4_S32
                        MD4HH4_HH4_MMX		 9, MD4_S32,  4, MD4_S33, 12, MD4_S34
                        MD4HH4_HH4_MMX		 5, MD4_S33,  2, MD4_S31, 10, MD4_S32
                        MD4HH4_HH4_MMX		13, MD4_S34,  6, MD4_S33, 14, MD4_S34
                        MD4HH4_HH4_MMX		 3, MD4_S31,  1, MD4_S31,  9, MD4_S32
                        MD4HH4_HH4_MMX		11, MD4_S32,  5, MD4_S33, 13, MD4_S34
                        MD4HH4_HH4_MMX		 7, MD4_S33,  3, MD4_S31, 11, MD4_S32
                        MD4HH4_HH4_MMX		15, MD4_S34,  7, MD4_S33, 15, MD4_S34
                        paddd		mmx_a, [__Digest+0*8]
                        paddd		mmx_b, [__Digest+1*8]
                        paddd		mmx_c, [__Digest+2*8]
                        paddd		mmx_d, [__Digest+3*8]
                        movq		[__Digest+0*8], mmx_a
                        movq		[__Digest+1*8], mmx_b
                        movq		[__Digest+2*8], mmx_c
                        movq		[__Digest+3*8], mmx_d
                        add			[__Digest+3*16], reg_a
                        add			[__Digest+3*16+4], reg_b
                        add			[__Digest+3*16+8], reg_c
                        add			[__Digest+3*16+12], reg_d
						add			dword ptr __Data1, 64
						add			dword ptr __Data2, 64
						add			dword ptr __Data3, 64
						add			dword ptr __Data4, 64
						dec			dword ptr __Count
                        jnz			full_blocks
                        mov			ebp, __Frame
						mov			ebx, __this1
						mov			edx, __this2
						mov			esi, __this3
						mov			edi, __this4
                        movq		mm0, [__Digest+0*8]
                        movq		mm1, [__Digest+1*8]
                        movq		mm2, [__Digest+2*8]
                        movq		mm3, [__Digest+3*8]
                        movq		mm4, mm0
                        movq		mm5, mm2
                        punpckldq	mm0, mm1
                        punpckldq	mm2, mm3
                        punpckhdq	mm4, mm1
                        punpckhdq	mm5, mm3
                        movq		[ebx+m_nHash0], mm0
                        movq		[ebx+m_nHash2], mm2
                        movq		[edx+m_nHash0], mm4
                        movq		[edx+m_nHash2], mm5
                        movq		mm0, [__Digest+2*16+0*8]
                        movq		mm1, [__Digest+2*16+1*8]
                        movq		mm2, [__Digest+2*16+2*8]
                        movq		mm3, [__Digest+2*16+3*8]
                        movq		[esi+m_nHash0], mm0
                        movq		[esi+m_nHash2], mm1
                        movq		[edi+m_nHash0], mm2
                        movq		[edi+m_nHash2], mm3
						mov			esp, ebp
                        popa
                        emms
                        ret 32
MD4_Add4_MMX			ENDP

						.xmm

MD4FF5_SSE2				MACRO		count:REQ,s:REQ
; a = (a+x[count]+((b&c)|(~b&d)))rol s
; a = (a+x[count]+(d^(b&(c^d))))rol s
						paddd		xmm_a, [__w+count*16]
						mov			reg_temp1, reg_c
						movdqa		xmm_temp1, xmm_c
						xor			reg_c, reg_d
						pxor		xmm_c, xmm_d
						add			reg_a, [ebp+count*4]
						pand		xmm_c, xmm_b
						and			reg_c, reg_b
						pxor		xmm_c, xmm_d
						xor			reg_c, reg_d
						paddd		xmm_a, xmm_c
						add			reg_a, reg_c
						movdqa		xmm_temp2, xmm_a
						rol			reg_a, s
						pslld		xmm_a, s
						psrld		xmm_temp2, 32-s
						por			xmm_a, xmm_temp2
xmm_t					textequ		xmm_d
xmm_d					textequ		xmm_temp1
xmm_temp1				textequ		xmm_c
xmm_c					textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		xmm_t
reg_t					textequ		reg_d
reg_d					textequ		reg_temp1
reg_temp1				textequ		reg_c
reg_c					textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD4FF5_SSE2

MD4GG5_SSE2				MACRO		count:REQ,s:REQ
; a = (a+x[count]+((b&c)|(b&d)|(c&d))+5A827999H) rol s
; a = (a+x[count]+((b&c)|(d&(b|c)))+5A827999H)rol s
						paddd		xmm_a, [__w+count*16]
						mov			reg_temp2, reg_b
						movdqa		xmm_temp2, xmm_b
						mov			reg_temp1, reg_b
						movdqa		xmm_temp1, xmm_b
						add			reg_a, [ebp+count*4]
						por			xmm_b, xmm_c
						or			reg_b, reg_c
						pand		xmm_temp2, xmm_c
						and			reg_temp2, reg_c
						pand		xmm_b, xmm_d
						and			reg_b, reg_d
						paddd		xmm_a, xmm_5A827999
						or			reg_b, reg_temp2
						por			xmm_b, xmm_temp2
						add			reg_a, 5A827999H
						paddd		xmm_a, xmm_b
						add			reg_a, reg_b
						movdqa		xmm_temp2, xmm_a
						rol			reg_a, s
						pslld		xmm_a, s
						psrld		xmm_temp2, 32-s
						por			xmm_a, xmm_temp2
xmm_t					textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_temp1
xmm_temp1				textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		xmm_t
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_temp1
reg_temp1				textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD4GG5_SSE2

MD4HH5_SSE2				MACRO		count:REQ,s:REQ
; a = (a+x[count]+(b^c^d)+6ED9EBA1H)rol s
						movdqa		xmm_temp1, xmm_b
						add			reg_a, 6ED9EBA1H
						paddd		xmm_a, [__w+count*16]
						mov			reg_temp1, reg_b
						pxor		xmm_b, xmm_c
						xor			reg_b, reg_c
						paddd		xmm_a, xmm_6ED9EBA1
						add			reg_a, [ebp+count*4]
						pxor		xmm_b, xmm_d
						xor			reg_b, reg_d
						paddd		xmm_a, xmm_b
						add			reg_a, reg_b
						movdqa		xmm_temp2, xmm_a
						rol			reg_a, s
						pslld		xmm_a, s
						psrld		xmm_temp2, 32-s
						por			xmm_a, xmm_temp2
xmm_t					textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_temp1
xmm_temp1				textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		xmm_t
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_temp1
reg_temp1				textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD4HH5_SSE2

MD4_Add5_SSE2			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD, _this4:DWORD, _Data4:DWORD, _this5:DWORD, _Data5:DWORD
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
						mov			ebp, esp
                        sub         esp, 64*4+16*5+28
                        and			esp, 0ffffff80h						; align stack pointer
                        mov			eax, __Data1
                        mov			ebx, __Data2
                        mov			ecx, __Data3
                        mov			edx, __Data4
                        mov			esi, __Data5
__w						textequ		<esp>
__Digest				textequ		<esp+64*4>
__Data1					textequ		<[esp+64*4+16*5]>
__Data2					textequ		<[esp+64*4+16*5+4]>
__Data3					textequ		<[esp+64*4+16*5+8]>
__Data4					textequ		<[esp+64*4+16*5+12]>
__Data5					textequ		<[esp+64*4+16*5+16]>
__Count					textequ		<[esp+64*4+16*5+20]>
__Frame					textequ		<[esp+64*4+16*5+24]>
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
                        movdqu		xmm3, [ebp+m_nHash0]
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
                        movdqa		[__Digest+4*16], xmm3
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        mov			__Count, ecx
xmm_a					textequ		<xmm0>
xmm_b					textequ		<xmm1>
xmm_c					textequ		<xmm2>
xmm_d					textequ		<xmm3>
xmm_temp1				textequ		<xmm4>
xmm_temp2				textequ		<xmm5>
xmm_5A827999			textequ		<xmm6>
xmm_6ED9EBA1			textequ		<xmm7>
						movdqa		xmm_5A827999, const_5A827999
						movdqa		xmm_6ED9EBA1, const_6ED9EBA1
reg_a					textequ		<eax>
reg_b					textequ		<ebx>
reg_c					textequ		<ecx>
reg_d					textequ		<edx>
reg_temp1				textequ		<esi>
reg_temp2				textequ		<edi>
						mov			ebp, __Data5
full_blocks:
count					=			0
						mov			ebx, __Data1
						mov			edx, __Data2
						mov			esi, __Data3
						mov			edi, __Data4
						REPEAT		4
                        movdqu		xmm0, [ebx+count*16]
                        movdqu		xmm1, [edx+count*16]
                        movdqu		xmm2, [esi+count*16]
                        movdqu		xmm3, [edi+count*16]
                        movdqa		xmm4, xmm0
                        movdqa		xmm5, xmm2
                        punpckldq	xmm0, xmm1
                        punpckldq	xmm2, xmm3
                        punpckhdq	xmm4, xmm1
                        punpckhdq	xmm5, xmm3
                        movdqa		xmm1, xmm0
                        movdqa		xmm3, xmm4
                        punpcklqdq	xmm0, xmm2
                        punpcklqdq	xmm4, xmm5
                        punpckhqdq	xmm1, xmm2
                        punpckhqdq	xmm3, xmm5
                        movdqa		[__w+count*4*16+0*16], xmm0
                        movdqa		[__w+count*4*16+1*16], xmm1
                        movdqa		[__w+count*4*16+2*16], xmm4
                        movdqa		[__w+count*4*16+3*16], xmm3
count					=			count + 1
						ENDM
                        movdqa		xmm_a, [__Digest+0*16]
                        movdqa		xmm_b, [__Digest+1*16]
                        movdqa		xmm_c, [__Digest+2*16]
                        movdqa		xmm_d, [__Digest+3*16]
                        mov			reg_a, [__Digest+4*16]
                        mov			reg_b, [__Digest+4*16+4]
                        mov			reg_c, [__Digest+4*16+8]
                        mov			reg_d, [__Digest+4*16+12]
; round 1
                        MD4FF5_SSE2        0, MD4_S11
                        MD4FF5_SSE2        1, MD4_S12
                        MD4FF5_SSE2        2, MD4_S13
                        MD4FF5_SSE2        3, MD4_S14
                        MD4FF5_SSE2        4, MD4_S11
                        MD4FF5_SSE2        5, MD4_S12
                        MD4FF5_SSE2        6, MD4_S13
                        MD4FF5_SSE2        7, MD4_S14
                        MD4FF5_SSE2        8, MD4_S11
                        MD4FF5_SSE2        9, MD4_S12
                        MD4FF5_SSE2       10, MD4_S13
                        MD4FF5_SSE2       11, MD4_S14
                        MD4FF5_SSE2       12, MD4_S11
                        MD4FF5_SSE2       13, MD4_S12
                        MD4FF5_SSE2       14, MD4_S13
                        MD4FF5_SSE2       15, MD4_S14
; round 2
                        MD4GG5_SSE2        0, MD4_S21
                        MD4GG5_SSE2        4, MD4_S22
                        MD4GG5_SSE2        8, MD4_S23
                        MD4GG5_SSE2       12, MD4_S24
                        MD4GG5_SSE2        1, MD4_S21
                        MD4GG5_SSE2        5, MD4_S22
                        MD4GG5_SSE2        9, MD4_S23
                        MD4GG5_SSE2       13, MD4_S24
                        MD4GG5_SSE2        2, MD4_S21
                        MD4GG5_SSE2        6, MD4_S22
                        MD4GG5_SSE2       10, MD4_S23
                        MD4GG5_SSE2       14, MD4_S24
                        MD4GG5_SSE2        3, MD4_S21
                        MD4GG5_SSE2        7, MD4_S22
                        MD4GG5_SSE2       11, MD4_S23
                        MD4GG5_SSE2       15, MD4_S24
; round 3
                        MD4HH5_SSE2        0, MD4_S31
                        MD4HH5_SSE2        8, MD4_S32
                        MD4HH5_SSE2        4, MD4_S33
                        MD4HH5_SSE2       12, MD4_S34
                        MD4HH5_SSE2        2, MD4_S31
                        MD4HH5_SSE2       10, MD4_S32
                        MD4HH5_SSE2        6, MD4_S33
                        MD4HH5_SSE2       14, MD4_S34
                        MD4HH5_SSE2        1, MD4_S31
                        MD4HH5_SSE2        9, MD4_S32
                        MD4HH5_SSE2        5, MD4_S33
                        MD4HH5_SSE2       13, MD4_S34
                        MD4HH5_SSE2        3, MD4_S31
                        MD4HH5_SSE2       11, MD4_S32
                        MD4HH5_SSE2        7, MD4_S33
                        MD4HH5_SSE2       15, MD4_S34
                        paddd		xmm_a, [__Digest+0*16]
                        paddd		xmm_b, [__Digest+1*16]
                        paddd		xmm_c, [__Digest+2*16]
                        paddd		xmm_d, [__Digest+3*16]
                        movdqa		[__Digest+0*16], xmm_a
                        movdqa		[__Digest+1*16], xmm_b
                        movdqa		[__Digest+2*16], xmm_c
                        movdqa		[__Digest+3*16], xmm_d
                        add			[__Digest+4*16], reg_a
                        add			[__Digest+4*16+4], reg_b
                        add			[__Digest+4*16+8], reg_c
                        add			[__Digest+4*16+12], reg_d
						add			dword ptr __Data1, 64
						add			dword ptr __Data2, 64
						add			dword ptr __Data3, 64
						add			dword ptr __Data4, 64
						add			ebp, 64
						dec			dword ptr __Count
                        jnz			full_blocks
                        mov			ebp, __Frame
						mov			ebx, __this1
						mov			edx, __this2
						mov			esi, __this3
						mov			edi, __this4
						mov			ecx, __this5
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
                        movdqa		xmm3, [__Digest+4*16]
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
                        movdqu		[ecx+m_nHash0], xmm3
						mov			esp, ebp
                        popa
                        ret 40
MD4_Add5_SSE2			ENDP

MD4FF6_FF6_SSE2			MACRO		count:REQ, s:REQ, count1:REQ, s1:REQ, count2:REQ, s2:REQ
; a = (a+x[count]+((b&c)|(~b&d)))rol s
; a = (a+x[count]+(d^(b&(c^d))))rol s
						paddd		xmm_a, [__w+count*16]
						mov			reg_temp1, reg_c
						xor			reg_c, reg_d
						movdqa		xmm_temp1, xmm_c
						add			reg_a, [ebp+count1*4]
						and			reg_c, reg_b
						pxor		xmm_c, xmm_d
						xor			reg_c, reg_d
						pand		xmm_c, xmm_b
						add			reg_a, reg_c
						mov			reg_c, reg_b
						pxor		xmm_c, xmm_d
						rol			reg_a, s1
						paddd		xmm_a, xmm_c
						xor			reg_b, reg_temp1
						add			reg_d, [ebp+count2*4]
						movdqa		xmm_temp2, xmm_a
						and			reg_b, reg_a
						pslld		xmm_a, s
						xor			reg_b, reg_temp1
						psrld		xmm_temp2, 32-s
						add			reg_d, reg_b
						por			xmm_a, xmm_temp2
						rol			reg_d, s2
xmm_t					textequ		xmm_d
xmm_d					textequ		xmm_temp1
xmm_temp1				textequ		xmm_c
xmm_c					textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		xmm_t
reg_t					textequ		reg_d
reg_d					textequ		reg_temp1
reg_temp1				textequ		reg_c
reg_c					textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
reg_t					textequ		reg_d
reg_d					textequ		reg_temp1
reg_temp1				textequ		reg_c
reg_c					textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD4FF6_FF6_SSE2

MD4FF6_GG6_SSE2			MACRO		count:REQ, s:REQ, count1:REQ, s1:REQ, count2:REQ, s2:REQ
; a = (a+x[count]+((b&c)|(~b&d)))rol s
; a = (a+x[count]+(d^(b&(c^d))))rol s
						movdqa		xmm_temp1, xmm_c
						mov			reg_temp2, reg_b
						mov			reg_temp1, reg_b
						paddd		xmm_a, [__w+count*16]
						add			reg_a, 5A827999H
						or			reg_b, reg_c
						pxor		xmm_c, xmm_d
						and			reg_temp2, reg_c
						and			reg_b, reg_d
						pand		xmm_c, xmm_b
						add			reg_a, [ebp+count1*4]
						or			reg_b, reg_temp2
						pxor		xmm_c, xmm_d
						add			reg_a, reg_b
						paddd		xmm_a, xmm_c
						rol			reg_a, s1
						movdqa		xmm_temp2, xmm_a
						mov			reg_temp2, reg_a
						mov			reg_b, reg_a
						pslld		xmm_a, s
						add			reg_d, [ebp+count2*4]
						or			reg_a, reg_temp1
						psrld		xmm_temp2, 32-s
						and			reg_temp2, reg_temp1
						and			reg_a, reg_c
						por			xmm_a, xmm_temp2
						add			reg_d, 5A827999H
						or			reg_a, reg_temp2
						add			reg_d, reg_a
						rol			reg_d, s2
xmm_t					textequ		xmm_d
xmm_d					textequ		xmm_temp1
xmm_temp1				textequ		xmm_c
xmm_c					textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		xmm_t
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_temp1
reg_temp1				textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_temp1
reg_temp1				textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD4FF6_GG6_SSE2
                        
MD4GG6_HH6_SSE2			MACRO		count:REQ, s:REQ, count1:REQ, s1:REQ, count2:REQ, s2:REQ
; a = (a+x[count]+((b&c)|(b&d)|(c&d))+5A827999H) rol s
; a = (a+x[count]+((b&c)|(d&(b|c)))+5A827999H)rol s
						movdqa		xmm_temp2, xmm_b
						add			reg_a, [ebp+count1*4]
						mov			reg_temp1, reg_b
						movdqa		xmm_temp1, xmm_b
						xor			reg_b, reg_c
						add			reg_a, 6ED9EBA1H
						paddd		xmm_a, [__w+count*16]
						xor			reg_b, reg_d
						por			xmm_b, xmm_c
						add			reg_a, reg_b
						pand		xmm_temp2, xmm_c
						rol			reg_a, s1
						pand		xmm_b, xmm_d
						add			reg_d, [ebp+count2*4]
						mov			reg_b, reg_a
						paddd		xmm_a, xmm_5A827999
						xor			reg_a, reg_temp1
						add			reg_d, 6ED9EBA1H
						por			xmm_b, xmm_temp2
						xor			reg_a, reg_c
						paddd		xmm_a, xmm_b
						add			reg_d, reg_a
						movdqa		xmm_temp2, xmm_a
						rol			reg_d, s2
						pslld		xmm_a, s
						psrld		xmm_temp2, 32-s
						por			xmm_a, xmm_temp2
xmm_t					textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_temp1
xmm_temp1				textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		xmm_t
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_temp1
reg_temp1				textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_temp1
reg_temp1				textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD4GG6_HH6_SSE2

MD4GG6_FF6_SSE2			MACRO		count:REQ, s:REQ, count1:REQ, s1:REQ, count2:REQ, s2:REQ
; a = (a+x[count]+((b&c)|(b&d)|(c&d))+5A827999H) rol s
; a = (a+x[count]+((b&c)|(d&(b|c)))+5A827999H)rol s
						movdqa		xmm_temp2, xmm_b
						mov			reg_temp1, reg_c
						xor			reg_c, reg_d
						paddd		xmm_a, [__w+count*16]
						and			reg_c, reg_b
						movdqa		xmm_temp1, xmm_b
						add			reg_a, [ebp+count1*4]
						xor			reg_c, reg_d
						por			xmm_b, xmm_c
						add			reg_a, reg_c
						mov			reg_c, reg_b
						pand		xmm_temp2, xmm_c
						rol			reg_a, s1
						pand		xmm_b, xmm_d
						xor			reg_b, reg_temp1
						add			reg_d, [ebp+count2*4]
						paddd		xmm_a, xmm_5A827999
						and			reg_b, reg_a
						por			xmm_b, xmm_temp2
						xor			reg_b, reg_temp1
						paddd		xmm_a, xmm_b
						add			reg_d, reg_b
						movdqa		xmm_temp2, xmm_a
						rol			reg_d, s2
						pslld		xmm_a, s
						psrld		xmm_temp2, 32-s
						por			xmm_a, xmm_temp2
xmm_t					textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_temp1
xmm_temp1				textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		xmm_t
reg_t					textequ		reg_d
reg_d					textequ		reg_temp1
reg_temp1				textequ		reg_c
reg_c					textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
reg_t					textequ		reg_d
reg_d					textequ		reg_temp1
reg_temp1				textequ		reg_c
reg_c					textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD4GG6_FF6_SSE2

MD4HH6_GG6_SSE2			MACRO       count:REQ, s:REQ, count1:REQ, s1:REQ, count2:REQ, s2:REQ
; a = (a+x[count]+(b^c^d)+6ED9EBA1H)rol s
						movdqa		xmm_temp1, xmm_b
						mov			reg_temp2, reg_b
						mov			reg_temp1, reg_b
						paddd		xmm_a, [__w+count*16]
						add			reg_a, 5A827999H
						or			reg_b, reg_c
						pxor		xmm_b, xmm_c
						and			reg_temp2, reg_c
						and			reg_b, reg_d
						paddd		xmm_a, xmm_6ED9EBA1
						add			reg_a, [ebp+count1*4]
						or			reg_b, reg_temp2
						pxor		xmm_b, xmm_d
						add			reg_a, reg_b
						paddd		xmm_a, xmm_b
						rol			reg_a, s1
						movdqa		xmm_temp2, xmm_a
						mov			reg_temp2, reg_a
						mov			reg_b, reg_a
						pslld		xmm_a, s
						add			reg_d, [ebp+count2*4]
						or			reg_a, reg_temp1
						psrld		xmm_temp2, 32-s
						and			reg_temp2, reg_temp1
						and			reg_a, reg_c
						por			xmm_a, xmm_temp2
						add			reg_d, 5A827999H
						or			reg_a, reg_temp2
						add			reg_d, reg_a
						rol			reg_d, s2
xmm_t					textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_temp1
xmm_temp1				textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		xmm_t
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_temp1
reg_temp1				textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_temp1
reg_temp1				textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD4HH6_GG6_SSE2

MD4HH6_HH6_SSE2			MACRO       count:REQ, s:REQ, count1:REQ, s1:REQ, count2:REQ, s2:REQ
; a = (a+x[count]+(b^c^d)+6ED9EBA1H)rol s
						movdqa		xmm_temp1, xmm_b
						add			reg_a, 6ED9EBA1H
						mov			reg_temp1, reg_b
						paddd		xmm_a, [__w+count*16]
						add			reg_a, [ebp+count1*4]
						xor			reg_b, reg_c
						pxor		xmm_b, xmm_c
						xor			reg_b, reg_d
						paddd		xmm_a, xmm_6ED9EBA1
						add			reg_a, reg_b
						pxor		xmm_b, xmm_d
						rol			reg_a, s1
						add			reg_d, [ebp+count2*4]
						paddd		xmm_a, xmm_b
						mov			reg_b, reg_a
						xor			reg_a, reg_temp1
						movdqa		xmm_temp2, xmm_a
						add			reg_d, 6ED9EBA1H
						xor			reg_a, reg_c
						pslld		xmm_a, s
						add			reg_d, reg_a
						psrld		xmm_temp2, 32-s
						rol			reg_d, s2
						por			xmm_a, xmm_temp2
xmm_t					textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_temp1
xmm_temp1				textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		xmm_t
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_temp1
reg_temp1				textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_temp1
reg_temp1				textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD4HH6_HH6_SSE2

MD4_Add6_SSE2			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD, _this4:DWORD, _Data4:DWORD, _this5:DWORD, _Data5:DWORD, _this6:DWORD, _Data6:DWORD
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
                        sub         esp, 64*4+16*6+32
                        and			esp, 0ffffff80h						; align stack pointer
                        mov			eax, __Data1
                        mov			ebx, __Data2
                        mov			ecx, __Data3
                        mov			edx, __Data4
                        mov			esi, __Data5
                        mov			edi, __Data6
__w						textequ		<esp>
__Digest				textequ		<esp+64*4>
__Data1					textequ		<[esp+64*4+16*6]>
__Data2					textequ		<[esp+64*4+16*6+4]>
__Data3					textequ		<[esp+64*4+16*6+8]>
__Data4					textequ		<[esp+64*4+16*6+12]>
__Data5					textequ		<[esp+64*4+16*6+16]>
__Data6					textequ		<[esp+64*4+16*6+20]>
__Count					textequ		<[esp+64*4+16*6+24]>
__Frame					textequ		<[esp+64*4+16*6+28]>
						mov			__Data1, eax
						mov			__Data2, ebx
						mov			__Data3, ecx
						mov			__Data4, edx
						mov			__Data5, esi
						mov			__Data6, edi
						mov			__Frame, ebp
                        mov			ebx, __this1
                        mov			edx, __this2
                        mov			esi, __this3
                        mov			edi, __this4
                        mov         eax, LIBRARY_BUILDER_BLOCK_SIZE
                        xor			ecx, ecx
                        movdqu		xmm0, [ebx+m_nHash0]
                        movdqu		xmm1, [edx+m_nHash0]
                        movdqu		xmm2, [esi+m_nHash0]
                        movdqu		xmm3, [edi+m_nHash0]
						add         [ebx+m_nCount0], eax
                        adc         [ebx+m_nCount1], ecx
                        add         [edx+m_nCount0], eax
                        adc         [edx+m_nCount1], ecx
                        mov			ebx, __this6
                        mov			ebp, __this5
                        add         [esi+m_nCount0], eax
                        adc         [esi+m_nCount1], ecx
                        add         [edi+m_nCount0], eax
                        adc         [edi+m_nCount1], ecx
                        add         [ebp+m_nCount0], eax
                        adc         [ebp+m_nCount1], ecx
                        add         [ebx+m_nCount0], eax
                        adc         [ebx+m_nCount1], ecx
                        movdqa		xmm4, xmm0
                        movdqa		xmm5, xmm2
                        punpckldq	xmm0, xmm1
                        punpckldq	xmm2, xmm3
                        punpckhdq	xmm4, xmm1
                        punpckhdq	xmm5, xmm3
                        movdqu		xmm3, [ebp+m_nHash0]
                        movdqu		xmm1, [ebx+m_nHash0]
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
                        movdqa		[__Digest+4*16], xmm3
                        movdqa		[__Digest+5*16], xmm1
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        mov			__Count, ecx
xmm_a					textequ		<xmm0>
xmm_b					textequ		<xmm1>
xmm_c					textequ		<xmm2>
xmm_d					textequ		<xmm3>
xmm_temp1				textequ		<xmm4>
xmm_temp2				textequ		<xmm5>
xmm_5A827999			textequ		<xmm6>
xmm_6ED9EBA1			textequ		<xmm7>
						movdqa		xmm_5A827999, const_5A827999
						movdqa		xmm_6ED9EBA1, const_6ED9EBA1
reg_a					textequ		<eax>
reg_b					textequ		<ebx>
reg_c					textequ		<ecx>
reg_d					textequ		<edx>
reg_temp1				textequ		<esi>
reg_temp2				textequ		<edi>
full_blocks:
count					=			0
						mov			ebx, __Data1
						mov			edx, __Data2
						mov			esi, __Data3
						mov			edi, __Data4
						REPEAT		4
                        movdqu		xmm0, [ebx+count*16]
                        movdqu		xmm1, [edx+count*16]
                        movdqu		xmm2, [esi+count*16]
                        movdqu		xmm3, [edi+count*16]
                        movdqa		xmm4, xmm0
                        movdqa		xmm5, xmm2
                        punpckldq	xmm0, xmm1
                        punpckldq	xmm2, xmm3
                        punpckhdq	xmm4, xmm1
                        punpckhdq	xmm5, xmm3
                        movdqa		xmm1, xmm0
                        movdqa		xmm3, xmm4
                        punpcklqdq	xmm0, xmm2
                        punpcklqdq	xmm4, xmm5
                        punpckhqdq	xmm1, xmm2
                        punpckhqdq	xmm3, xmm5
                        movdqa		[__w+count*4*16+0*16], xmm0
                        movdqa		[__w+count*4*16+1*16], xmm1
                        movdqa		[__w+count*4*16+2*16], xmm4
                        movdqa		[__w+count*4*16+3*16], xmm3
count					=			count + 1
						ENDM
                        movdqa		xmm_a, [__Digest+0*16]
                        movdqa		xmm_b, [__Digest+1*16]
                        movdqa		xmm_c, [__Digest+2*16]
                        movdqa		xmm_d, [__Digest+3*16]
						mov			ebp, __Data5
                        mov			reg_a, [__Digest+4*16]
                        mov			reg_b, [__Digest+4*16+4]
                        mov			reg_c, [__Digest+4*16+8]
                        mov			reg_d, [__Digest+4*16+12]
; round 1
                        MD4FF6_FF6_SSE2		 0, MD4_S11,  0, MD4_S11,  1, MD4_S12
                        MD4FF6_FF6_SSE2		 1, MD4_S12,  2, MD4_S13,  3, MD4_S14
                        MD4FF6_FF6_SSE2		 2, MD4_S13,  4, MD4_S11,  5, MD4_S12
                        MD4FF6_FF6_SSE2		 3, MD4_S14,  6, MD4_S13,  7, MD4_S14
                        MD4FF6_FF6_SSE2		 4, MD4_S11,  8, MD4_S11,  9, MD4_S12
                        MD4FF6_FF6_SSE2		 5, MD4_S12, 10, MD4_S13, 11, MD4_S14
                        MD4FF6_FF6_SSE2		 6, MD4_S13, 12, MD4_S11, 13, MD4_S12
                        MD4FF6_FF6_SSE2		 7, MD4_S14, 14, MD4_S13, 15, MD4_S14
                        MD4FF6_GG6_SSE2		 8, MD4_S11,  0, MD4_S21,  4, MD4_S22
                        MD4FF6_GG6_SSE2		 9, MD4_S12,  8, MD4_S23, 12, MD4_S24
                        MD4FF6_GG6_SSE2		10, MD4_S13,  1, MD4_S21,  5, MD4_S22
                        MD4FF6_GG6_SSE2		11, MD4_S14,  9, MD4_S23, 13, MD4_S24
                        MD4FF6_GG6_SSE2		12, MD4_S11,  2, MD4_S21,  6, MD4_S22
                        MD4FF6_GG6_SSE2		13, MD4_S12, 10, MD4_S23, 14, MD4_S24
                        MD4FF6_GG6_SSE2		14, MD4_S13,  3, MD4_S21,  7, MD4_S22
                        MD4FF6_GG6_SSE2		15, MD4_S14, 11, MD4_S23, 15, MD4_S24
; round 2
                        MD4GG6_HH6_SSE2		 0, MD4_S21,  0, MD4_S31,  8, MD4_S32
                        MD4GG6_HH6_SSE2		 4, MD4_S22,  4, MD4_S33, 12, MD4_S34
                        MD4GG6_HH6_SSE2		 8, MD4_S23,  2, MD4_S31, 10, MD4_S32
                        MD4GG6_HH6_SSE2		12, MD4_S24,  6, MD4_S33, 14, MD4_S34
                        MD4GG6_HH6_SSE2		 1, MD4_S21,  1, MD4_S31,  9, MD4_S32
                        MD4GG6_HH6_SSE2		 5, MD4_S22,  5, MD4_S33, 13, MD4_S34
                        MD4GG6_HH6_SSE2		 9, MD4_S23,  3, MD4_S31, 11, MD4_S32
                        MD4GG6_HH6_SSE2		13, MD4_S24,  7, MD4_S33, 15, MD4_S34
                        add			[__Digest+4*16], reg_a
                        add			[__Digest+4*16+4], reg_b
                        add			[__Digest+4*16+8], reg_c
                        add			[__Digest+4*16+12], reg_d
; round 3
						mov			ebp, __Data6
                        mov			reg_a, [__Digest+5*16]
                        mov			reg_b, [__Digest+5*16+4]
                        mov			reg_c, [__Digest+5*16+8]
                        mov			reg_d, [__Digest+5*16+12]
                        MD4GG6_FF6_SSE2		 2, MD4_S21,  0, MD4_S11,  1, MD4_S12
                        MD4GG6_FF6_SSE2		 6, MD4_S22,  2, MD4_S13,  3, MD4_S14
                        MD4GG6_FF6_SSE2		10, MD4_S23,  4, MD4_S11,  5, MD4_S12
                        MD4GG6_FF6_SSE2		14, MD4_S24,  6, MD4_S13,  7, MD4_S14
                        MD4GG6_FF6_SSE2		 3, MD4_S21,  8, MD4_S11,  9, MD4_S12
                        MD4GG6_FF6_SSE2		 7, MD4_S22, 10, MD4_S13, 11, MD4_S14
                        MD4GG6_FF6_SSE2		11, MD4_S23, 12, MD4_S11, 13, MD4_S12
                        MD4GG6_FF6_SSE2		15, MD4_S24, 14, MD4_S13, 15, MD4_S14
; round 3
                        MD4HH6_GG6_SSE2		 0, MD4_S31,  0, MD4_S21,  4, MD4_S22
                        MD4HH6_GG6_SSE2		 8, MD4_S32,  8, MD4_S23, 12, MD4_S24
                        MD4HH6_GG6_SSE2		 4, MD4_S33,  1, MD4_S21,  5, MD4_S22
                        MD4HH6_GG6_SSE2		12, MD4_S34,  9, MD4_S23, 13, MD4_S24
                        MD4HH6_GG6_SSE2		 2, MD4_S31,  2, MD4_S21,  6, MD4_S22
                        MD4HH6_GG6_SSE2		10, MD4_S32, 10, MD4_S23, 14, MD4_S24
                        MD4HH6_GG6_SSE2		 6, MD4_S33,  3, MD4_S21,  7, MD4_S22
                        MD4HH6_GG6_SSE2		14, MD4_S34, 11, MD4_S23, 15, MD4_S24
                        MD4HH6_HH6_SSE2		 1, MD4_S31,  0, MD4_S31,  8, MD4_S32
                        MD4HH6_HH6_SSE2		 9, MD4_S32,  4, MD4_S33, 12, MD4_S34
                        MD4HH6_HH6_SSE2		 5, MD4_S33,  2, MD4_S31, 10, MD4_S32
                        MD4HH6_HH6_SSE2		13, MD4_S34,  6, MD4_S33, 14, MD4_S34
                        MD4HH6_HH6_SSE2		 3, MD4_S31,  1, MD4_S31,  9, MD4_S32
                        MD4HH6_HH6_SSE2		11, MD4_S32,  5, MD4_S33, 13, MD4_S34
                        MD4HH6_HH6_SSE2		 7, MD4_S33,  3, MD4_S31, 11, MD4_S32
                        MD4HH6_HH6_SSE2		15, MD4_S34,  7, MD4_S33, 15, MD4_S34
                        paddd		xmm_a, [__Digest+0*16]
                        paddd		xmm_b, [__Digest+1*16]
                        paddd		xmm_c, [__Digest+2*16]
                        paddd		xmm_d, [__Digest+3*16]
                        movdqa		[__Digest+0*16], xmm_a
                        movdqa		[__Digest+1*16], xmm_b
                        movdqa		[__Digest+2*16], xmm_c
                        movdqa		[__Digest+3*16], xmm_d
                        add			[__Digest+5*16], reg_a
                        add			[__Digest+5*16+4], reg_b
                        add			[__Digest+5*16+8], reg_c
                        add			[__Digest+5*16+12], reg_d
						add			dword ptr __Data1, 64
						add			dword ptr __Data2, 64
						add			dword ptr __Data3, 64
						add			dword ptr __Data4, 64
						add			dword ptr __Data5, 64
						add			dword ptr __Data6, 64
						dec			dword ptr __Count
                        jnz			full_blocks
                        mov			ebp, __Frame
						mov			ebx, __this1
						mov			edx, __this2
						mov			esi, __this3
						mov			edi, __this4
						mov			ecx, __this5
						mov			eax, __this6
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
                        movdqa		xmm3, [__Digest+4*16]
                        movdqa		xmm1, [__Digest+5*16]
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
                        movdqu		[ecx+m_nHash0], xmm3
                        movdqu		[eax+m_nHash0], xmm1
						mov			esp, ebp
                        popa
                        ret 48
MD4_Add6_SSE2			ENDP

               end
