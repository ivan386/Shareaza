; #####################################################################################################################
;
; MD5_asm.asm
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
; MD5_asm - Implementation of MD5 for x86
;
; created				7.7.2004		by Camper			thetruecamper at gmx dot net
;
; modified				2.9.2004        by Camper			ICQ # 105491945
;
; #####################################################################################################################

                        .586p
                        .model      flat, stdcall 
                        option      casemap:none                    ; case sensitive
                        option      prologue:none                   ; we generate our own entry/exit code
                        option      epilogue:none

; #####################################################################################################################

						include		<common.inc>

m_nHash0				equ			0								; offsets as found in MD5.h
m_nHash1				equ			4
m_nHash2				equ			8
m_nHash3				equ			12

m_nCount0				equ			16
m_nCount1				equ			20

m_pBuffer				equ			24

; Some magic numbers for Transform...
MD5_S11                 equ         7
MD5_S12                 equ         12
MD5_S13                 equ         17
MD5_S14                 equ         22
MD5_S21                 equ         5
MD5_S22                 equ         9
MD5_S23                 equ         14
MD5_S24                 equ         20
MD5_S31                 equ         4
MD5_S32                 equ         11
MD5_S33                 equ         16
MD5_S34                 equ         23
MD5_S41                 equ         6
MD5_S42                 equ         10
MD5_S43                 equ         15
MD5_S44                 equ         21

						.data
						
						ALIGN		16
ac_const				dd			4 dup (0D76AA478H)
						dd			4 dup (0E8C7B756H)
						dd			4 dup (0242070DBH)
						dd			4 dup (0C1BDCEEEH)
						dd			4 dup (0F57C0FAFH)
						dd			4 dup (04787C62AH)
						dd			4 dup (0A8304613H)
						dd			4 dup (0FD469501H)
						dd			4 dup (0698098D8H)
						dd			4 dup (08B44F7AFH)
						dd			4 dup (0FFFF5BB1H)
						dd			4 dup (0895CD7BEH)
						dd			4 dup (06B901122H)
						dd			4 dup (0FD987193H)
						dd			4 dup (0A679438EH)
						dd			4 dup (049B40821H)
						dd			4 dup (0F61E2562H)
						dd			4 dup (0C040B340H)
						dd			4 dup (0265E5A51H)
						dd			4 dup (0E9B6C7AAH)
						dd			4 dup (0D62F105DH)
						dd			4 dup (002441453H)
						dd			4 dup (0D8A1E681H)
						dd			4 dup (0E7D3FBC8H)
						dd			4 dup (021E1CDE6H)
						dd			4 dup (0C33707D6H)
						dd			4 dup (0F4D50D87H)
						dd			4 dup (0455A14EDH)
						dd			4 dup (0A9E3E905H)
						dd			4 dup (0FCEFA3F8H)
						dd			4 dup (0676F02D9H)
						dd			4 dup (08D2A4C8AH)
						dd			4 dup (0FFFA3942H)
						dd			4 dup (08771F681H)
						dd			4 dup (06D9D6122H)
						dd			4 dup (0FDE5380CH)
						dd			4 dup (0A4BEEA44H)
						dd			4 dup (04BDECFA9H)
						dd			4 dup (0F6BB4B60H)
						dd			4 dup (0BEBFBC70H)
						dd			4 dup (0289B7EC6H)
						dd			4 dup (0EAA127FAH)
						dd			4 dup (0D4EF3085H)
						dd			4 dup (004881D05H)
						dd			4 dup (0D9D4D039H)
						dd			4 dup (0E6DB99E5H)
						dd			4 dup (01FA27CF8H)
						dd			4 dup (0C4AC5665H)
						dd			4 dup (0F4292244H)
						dd			4 dup (0432AFF97H)
						dd			4 dup (0AB9423A7H)
						dd			4 dup (0FC93A039H)
						dd			4 dup (0655B59C3H)
						dd			4 dup (08F0CCC92H)
						dd			4 dup (0FFEFF47DH)
						dd			4 dup (085845DD1H)
						dd			4 dup (06FA87E4FH)
						dd			4 dup (0FE2CE6E0H)
						dd			4 dup (0A3014314H)
						dd			4 dup (04E0811A1H)
						dd			4 dup (0F7537E82H)
						dd			4 dup (0BD3AF235H)
						dd			4 dup (02AD7D2BBH)
						dd			4 dup (0EB86D391H)
const_FFFFFFFFFFFFFFFF	dq			2 dup (0FFFFFFFFFFFFFFFFH)

MD5FF                   MACRO       count:REQ,s:REQ,ac:REQ
; a = b+(a+x[count]+ac+((b&c)|(~b&d)))rol s
; a = b+(a+x[count]+ax+(d^(b&(c^d))))rol s
                        mov         reg_temp1, reg_c
reg_t                   textequ     reg_temp1
reg_temp1               textequ     reg_c
reg_c                   textequ     reg_t
                        xor			reg_temp1, reg_d
                        add         reg_a, [reg_base+count*4]
                        and         reg_temp1, reg_b
                        add         reg_a, ac
                        xor			reg_temp1, reg_d
                        add         reg_a, reg_temp1
                        rol         reg_a, s
                        add         reg_a, reg_b
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
                        ENDM
                        
MD5GG                   MACRO       count:REQ,s:REQ,ac:REQ
; a = b+(a+x[count]+ac+((d&b)|(~d&c)))rol s
; a = b+(a+x[count]+ac+(c^(d&(b^c))))rols s
                        mov         reg_temp1, reg_b
reg_t                   textequ     reg_temp1
reg_temp1               textequ     reg_b
reg_b                   textequ     reg_t
                        xor         reg_temp1, reg_c
                        add         reg_a, [reg_base+count*4]
                        and         reg_temp1, reg_d
                        add         reg_a, ac
                        xor         reg_temp1, reg_c
                        add         reg_a, reg_temp1
                        rol         reg_a, s
                        add         reg_a, reg_b
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
                        ENDM

MD5HH                   MACRO       count:REQ,s:REQ,ac:REQ
; a = b+(a+x[count]+ac+(b^c^d)) rol s
                        mov         reg_temp1, reg_b
reg_t                   textequ     reg_temp1
reg_temp1               textequ     reg_b
reg_b                   textequ     reg_t
                        xor         reg_temp1, reg_c
                        add         reg_a, [reg_base+count*4]
                        xor         reg_temp1, reg_d
                        add         reg_a, ac
                        add         reg_a, reg_temp1
                        rol         reg_a, s
                        add         reg_a, reg_b
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
                        ENDM

MD5II                   MACRO       count:REQ,s:REQ,ac:REQ
; a = b+(a+x[count]+ac+(c^(~d|b))) rol s
                        mov         reg_temp1, reg_d
reg_t                   textequ     reg_temp1
reg_temp1               textequ     reg_d
reg_d                   textequ     reg_t
                        not         reg_temp1
                        add         reg_a, [reg_base+count*4]
                        or          reg_temp1, reg_b
                        add         reg_a, ac
                        xor         reg_temp1, reg_c
                        add         reg_a, reg_temp1
                        rol         reg_a, s
                        add         reg_a, reg_b
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
                        ENDM

                        .code

MD5_Transform_p5        PROC                                            ; we expect ebp to point to the Data stream
                                                                        ; all other registers (eax,ebx,ecx,edx,esi,edi) will be destroyed
__this                  textequ     <[esp+32+2*4]>                      ; 1*pusha+2*call

; set alias for registers
reg_a                   textequ     <eax>
reg_b                   textequ     <ebx>
reg_c                   textequ     <ecx>
reg_d                   textequ     <edx>
reg_temp1               textequ     <esi>
reg_temp2               textequ     <edi>
reg_base                textequ     <ebp>
                        mov         reg_temp2, __this
                        mov         reg_a, [reg_temp2+m_nHash0]
                        mov         reg_b, [reg_temp2+m_nHash1]
                        mov         reg_c, [reg_temp2+m_nHash2]
                        mov         reg_d, [reg_temp2+m_nHash3]
; round 1
                        MD5FF        0, MD5_S11,0D76AA478H  ;  1
                        MD5FF        1, MD5_S12,0E8C7B756H  ;  2
                        MD5FF        2, MD5_S13, 242070DBH  ;  3
                        MD5FF        3, MD5_S14,0C1BDCEEEH  ;  4
                        MD5FF        4, MD5_S11,0F57C0FAFH  ;  5
                        MD5FF        5, MD5_S12, 4787C62AH  ;  6
                        MD5FF        6, MD5_S13,0A8304613H  ;  7
                        MD5FF        7, MD5_S14,0FD469501H  ;  8
                        MD5FF        8, MD5_S11, 698098D8H  ;  9
                        MD5FF        9, MD5_S12, 8B44F7AFH  ; 10
                        MD5FF       10, MD5_S13,0FFFF5BB1H  ; 11
                        MD5FF       11, MD5_S14, 895CD7BEH  ; 12
                        MD5FF       12, MD5_S11, 6B901122H  ; 13
                        MD5FF       13, MD5_S12,0FD987193H  ; 14
                        MD5FF       14, MD5_S13,0A679438EH  ; 15
                        MD5FF       15, MD5_S14, 49B40821H  ; 16
; round 2
                        MD5GG        1, MD5_S21,0F61E2562H  ; 17
                        MD5GG        6, MD5_S22,0C040B340H  ; 18
                        MD5GG       11, MD5_S23, 265E5A51H  ; 19
                        MD5GG        0, MD5_S24,0E9B6C7AAH  ; 20
                        MD5GG        5, MD5_S21,0D62F105DH  ; 21
                        MD5GG       10, MD5_S22,  2441453H  ; 22
                        MD5GG       15, MD5_S23,0D8A1E681H  ; 23
                        MD5GG        4, MD5_S24,0E7D3FBC8H  ; 24
                        MD5GG        9, MD5_S21, 21E1CDE6H  ; 25
                        MD5GG       14, MD5_S22,0C33707D6H  ; 26
                        MD5GG        3, MD5_S23,0F4D50D87H  ; 27
                        MD5GG        8, MD5_S24, 455A14EDH  ; 28
                        MD5GG       13, MD5_S21,0A9E3E905H  ; 29
                        MD5GG        2, MD5_S22,0FCEFA3F8H  ; 30
                        MD5GG        7, MD5_S23, 676F02D9H  ; 31
                        MD5GG       12, MD5_S24, 8D2A4C8AH  ; 32
; round 3
                        MD5HH        5, MD5_S31,0FFFA3942H  ; 33
                        MD5HH        8, MD5_S32, 8771F681H  ; 34
                        MD5HH       11, MD5_S33, 6D9D6122H  ; 35
                        MD5HH       14, MD5_S34,0FDE5380CH  ; 36
                        MD5HH        1, MD5_S31,0A4BEEA44H  ; 37
                        MD5HH        4, MD5_S32, 4BDECFA9H  ; 38
                        MD5HH        7, MD5_S33,0F6BB4B60H  ; 39
                        MD5HH       10, MD5_S34,0BEBFBC70H  ; 40
                        MD5HH       13, MD5_S31, 289B7EC6H  ; 41
                        MD5HH        0, MD5_S32,0EAA127FAH  ; 42
                        MD5HH        3, MD5_S33,0D4EF3085H  ; 43
                        MD5HH        6, MD5_S34,  4881D05H  ; 44
                        MD5HH        9, MD5_S31,0D9D4D039H  ; 45
                        MD5HH       12, MD5_S32,0E6DB99E5H  ; 46
                        MD5HH       15, MD5_S33, 1FA27CF8H  ; 47
                        MD5HH        2, MD5_S34,0C4AC5665H  ; 48
; round 4
                        MD5II        0, MD5_S41,0F4292244H  ; 49
                        MD5II        7, MD5_S42, 432AFF97H  ; 50
                        MD5II       14, MD5_S43,0AB9423A7H  ; 51
                        MD5II        5, MD5_S44,0FC93A039H  ; 52
                        MD5II       12, MD5_S41, 655B59C3H  ; 53
                        MD5II        3, MD5_S42, 8F0CCC92H  ; 54
                        MD5II       10, MD5_S43,0FFEFF47DH  ; 55
                        MD5II        1, MD5_S44, 85845DD1H  ; 56
                        MD5II        8, MD5_S41, 6FA87E4FH  ; 57
                        MD5II       15, MD5_S42,0FE2CE6E0H  ; 58
                        MD5II        6, MD5_S43,0A3014314H  ; 59
                        MD5II       13, MD5_S44, 4E0811A1H  ; 60
                        MD5II        4, MD5_S41,0F7537E82H  ; 61
                        MD5II       11, MD5_S42,0BD3AF235H  ; 62
                        MD5II        2, MD5_S43, 2AD7D2BBH  ; 63
                        MD5II        9, MD5_S44,0EB86D391H  ; 64
                        add         [reg_temp2+m_nHash0], reg_a
                        add         [reg_temp2+m_nHash1], reg_b
                        add         [reg_temp2+m_nHash2], reg_c
                        add         [reg_temp2+m_nHash3], reg_d
                        ret
MD5_Transform_p5        ENDP

MD5_Add_p5              PROC        PUBLIC, _this:DWORD, _Data:DWORD, _nLength:DWORD
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
                        call        MD5_Transform_p5
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
                        call        MD5_Transform_p5
                        mov         ebp, __Data
                        jmp         full_blocks
short_stream:           sub         ecx, eax                                ;  --> ecx=_nLength
                        mov         esi, ebp
                        lea         edi, [edi+m_pBuffer+eax]
                        rep movsb
get_out:                popa
                        ret 12

MD5_Add_p5              ENDP

MD5_Add1_SSE2			PROC		PUBLIC, _this:DWORD, _Data:DWORD
MD5_Add1_SSE2			ENDP
MD5_Add1_MMX			PROC		PUBLIC, _this:DWORD, _Data:DWORD
MD5_Add1_MMX			ENDP
MD5_Add1_p5				PROC		PUBLIC, _this:DWORD, _Data:DWORD
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
full_blocks:			call        MD5_Transform_p5
                        mov         ebp, __Data
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data, ebp
                        jnz			full_blocks
                        popa
                        ret 8
MD5_Add1_p5				ENDP

MD5_Add2_SSE2			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD
MD5_Add2_SSE2			ENDP
MD5_Add2_MMX			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD
MD5_Add2_MMX			ENDP
MD5_Add2_p5				PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD
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
full_blocks1:			call        MD5_Transform_p5
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
full_blocks2:			call        MD5_Transform_p5
                        mov         ebp, __Data2
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data2, ebp
                        jnz			full_blocks2

                        popa
                        ret 16
MD5_Add2_p5				ENDP

MD5_Add3_p5				PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD
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
full_blocks1:			call        MD5_Transform_p5
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
full_blocks2:			call        MD5_Transform_p5
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
full_blocks3:			call        MD5_Transform_p5
                        mov         ebp, __Data3
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data3, ebp
                        jnz			full_blocks3

                        popa
                        ret 24
MD5_Add3_p5				ENDP

MD5_Add4_p5				PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD, _this4:DWORD, _Data4:DWORD
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
full_blocks1:			call        MD5_Transform_p5
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
full_blocks2:			call        MD5_Transform_p5
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
full_blocks3:			call        MD5_Transform_p5
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
full_blocks4:			call        MD5_Transform_p5
                        mov         ebp, __Data4
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data4, ebp
                        jnz			full_blocks4

                        popa
                        ret 32
MD5_Add4_p5				ENDP

MD5_Add5_MMX			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD, _this4:DWORD, _Data4:DWORD, _this5:DWORD, _Data5:DWORD
MD5_Add5_MMX			ENDP
MD5_Add5_p5				PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD, _this4:DWORD, _Data4:DWORD, _this5:DWORD, _Data5:DWORD
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
full_blocks1:			call        MD5_Transform_p5
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
full_blocks2:			call        MD5_Transform_p5
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
full_blocks3:			call        MD5_Transform_p5
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
full_blocks4:			call        MD5_Transform_p5
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
full_blocks5:			call        MD5_Transform_p5
                        mov         ebp, __Data5
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data5, ebp
                        jnz			full_blocks5

                        popa
                        ret 40
MD5_Add5_p5				ENDP

MD5_Add6_MMX			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD, _this4:DWORD, _Data4:DWORD, _this5:DWORD, _Data5:DWORD, _this6:DWORD, _Data6:DWORD
MD5_Add6_MMX			ENDP
MD5_Add6_p5				PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD, _this4:DWORD, _Data4:DWORD, _this5:DWORD, _Data5:DWORD, _this6:DWORD, _Data6:DWORD
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
full_blocks1:			call        MD5_Transform_p5
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
full_blocks2:			call        MD5_Transform_p5
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
full_blocks3:			call        MD5_Transform_p5
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
full_blocks4:			call        MD5_Transform_p5
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
full_blocks5:			call        MD5_Transform_p5
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
full_blocks6:			call        MD5_Transform_p5
                        mov         ebp, __Data6
                        add         ebp, 64
                        dec			__nLength
                        mov         __Data6, ebp
                        jnz			full_blocks6

                        popa
                        ret 48
MD5_Add6_p5				ENDP

						.mmx

MD5FF3_MMX				MACRO		count:REQ,s:REQ, ac:REQ
; a = b+(a+x[count]+ac+((b&c)|(~b&d)))rol s
; a = b+(a+x[count]+ax+(d^(b&(c^d))))rol s
						movq		mmx_temp1, mmx_c
                        mov         reg_temp1, reg_c
						paddd		mmx_a, [__w+count*8]
                        xor			reg_c, reg_d
						pxor		mmx_c, mmx_d
                        add         reg_a, [ebp+count*4]
						pand		mmx_c, mmx_b
                        and         reg_c, reg_b
						paddd		mmx_a, qword ptr [ac_const+ac_index*16]
                        add         reg_a, ac
						pxor		mmx_c, mmx_d
                        xor			reg_c, reg_d
						paddd		mmx_a, mmx_c
                        add         reg_a, reg_c
						movq		mmx_temp2, mmx_a
                        rol         reg_a, s
						pslld		mmx_a, s
                        add         reg_a, reg_b
						psrld		mmx_temp2, 32-s
						por			mmx_a, mmx_temp2
						paddd		mmx_a, mmx_b
reg_t					textequ		mmx_d
mmx_d					textequ		mmx_temp1
mmx_temp1				textequ		mmx_c
mmx_c					textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		reg_t
ac_index				=			ac_index + 1
reg_t					textequ		reg_d
reg_d					textequ		reg_temp1
reg_temp1				textequ		reg_c
reg_c					textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD5FF3_MMX
                        

MD5GG3_MMX				MACRO		count:REQ,s:REQ, ac:REQ
; a = b+(a+x[count]+ac+((d&b)|(~d&c)))rol s
; a = b+(a+x[count]+ac+(c^(d&(b^c))))rols s
						movq		mmx_temp1, mmx_b
                        mov         reg_temp1, reg_b
						paddd		mmx_a, [__w+count*8]
                        xor         reg_b, reg_c
						pxor		mmx_b, mmx_c
                        add         reg_a, [ebp+count*4]
						pand		mmx_b, mmx_d
                        and         reg_b, reg_d
						paddd		mmx_a, qword ptr [ac_const+ac_index*16]
                        add         reg_a, ac
						pxor		mmx_b, mmx_c
                        xor         reg_b, reg_c
						paddd		mmx_a, mmx_b
                        add         reg_a, reg_b
						movq		mmx_temp2, mmx_a
                        rol         reg_a, s
						pslld		mmx_a, s
                        add         reg_a, reg_temp1
						psrld		mmx_temp2, 32-s
						por			mmx_a, mmx_temp2
						paddd		mmx_a, mmx_temp1
reg_t					textequ		mmx_d
mmx_d					textequ		mmx_c
mmx_c					textequ		mmx_temp1
mmx_temp1				textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		reg_t
ac_index				=			ac_index + 1
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_temp1
reg_temp1				textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD5GG3_MMX

MD5HH3_MMX				MACRO       count:REQ,s:REQ, ac:REQ
; a = b+(a+x[count]+ac+(b^c^d)) rol s
						movq		mmx_temp1, mmx_b
                        mov         reg_temp1, reg_b
						paddd		mmx_a, [__w+count*8]
                        xor         reg_b, reg_c
						pxor		mmx_b, mmx_c
                        add         reg_a, [ebp+count*4]
						paddd		mmx_a, qword ptr [ac_const+ac_index*16]
                        xor         reg_b, reg_d
						pxor		mmx_b, mmx_d
                        add         reg_a, ac
						paddd		mmx_a, mmx_b
                        add         reg_a, reg_b
						movq		mmx_temp2, mmx_a
                        rol         reg_a, s
						pslld		mmx_a, s
                        add         reg_a, reg_temp1
						psrld		mmx_temp2, 32-s
						por			mmx_a, mmx_temp2
						paddd		mmx_a, mmx_temp1
reg_t					textequ		mmx_d
mmx_d					textequ		mmx_c
mmx_c					textequ		mmx_temp1
mmx_temp1				textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		reg_t
ac_index				=			ac_index + 1
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_temp1
reg_temp1				textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD5HH3_MMX

MD5II3_MMX				MACRO       count:REQ,s:REQ, ac:REQ
; a = b+(a+x[count]+ac+(c^(~d|b))) rol s
						movq		mmx_temp1, mmx_d
                        mov         reg_temp1, reg_d
						pxor		mmx_d, mmx_FFFF
                        not         reg_d
						paddd		mmx_a, [__w+count*8]
                        add         reg_a, [ebp+count*4]
						por			mmx_d, mmx_b
                        or          reg_d, reg_b
						paddd		mmx_a, qword ptr [ac_const+ac_index*16]
                        add         reg_a, ac
						pxor		mmx_d, mmx_c
                        xor         reg_d, reg_c
						paddd		mmx_a, mmx_d
                        add         reg_a, reg_d
						movq		mmx_temp2, mmx_a
                        rol         reg_a, s
						pslld		mmx_a, s
                        add         reg_a, reg_b
						psrld		mmx_temp2, 32-s
						por			mmx_a, mmx_temp2
						paddd		mmx_a, mmx_b
reg_t					textequ		mmx_temp1
mmx_temp1				textequ		mmx_d
mmx_d					textequ		mmx_c
mmx_c					textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		reg_t
ac_index				=			ac_index + 1
reg_t					textequ		reg_temp1
reg_temp1				textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD5II3_MMX

MD5_Add3_SSE2			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD
MD5_Add3_SSE2			ENDP
MD5_Add3_MMX			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD
                        pusha
__this1                 textequ     <[ebp+36]>
__Data1                 textequ     <[ebp+40]>
__this2                 textequ     <[ebp+44]>
__Data2                 textequ     <[ebp+48]>
__this3                 textequ     <[ebp+52]>
__Data3                 textequ     <[ebp+56]>
						mov			ebp, esp
                        sub         esp, 64*4+16*2+16
                        and			esp, 0ffffff80h						; align stack pointer
                        mov			eax, __Data1
                        mov			ebx, __Data2
__w						textequ		<esp>
__Digest				textequ		<esp+64*4>
__Data1					textequ		<[esp+64*4+16*2]>
__Data2					textequ		<[esp+64*4+16*2+4]>
__Count					textequ		<[esp+64*4+16*2+8]>
__Frame					textequ		<[esp+64*4+16*2+12]>
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
                        mov         ecx, LIBRARY_BUILDER_BLOCK_SIZE / 64
                        mov			__Count, ecx
mmx_a					textequ		<mm0>
mmx_b					textequ		<mm1>
mmx_c					textequ		<mm2>
mmx_d					textequ		<mm3>
mmx_temp1				textequ		<mm4>
mmx_temp2				textequ		<mm5>
mmx_FFFF				textequ		<mm7>
                        movq		mmx_FFFF, const_FFFFFFFFFFFFFFFF
reg_a					textequ		<eax>
reg_b					textequ		<ebx>
reg_c					textequ		<ecx>
reg_d					textequ		<edx>
reg_temp1				textequ		<esi>
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
                        mov			reg_a, [edi+m_nHash0]
                        mov			reg_b, [edi+m_nHash1]
                        mov			reg_c, [edi+m_nHash2]
                        mov			reg_d, [edi+m_nHash3]
ac_index				=			0
; round 1
                        MD5FF3_MMX	 0, MD5_S11, 0D76AA478H				;  1
                        MD5FF3_MMX	 1, MD5_S12, 0E8C7B756H				;  2
                        MD5FF3_MMX	 2, MD5_S13, 0242070DBH				;  3
                        MD5FF3_MMX	 3, MD5_S14, 0C1BDCEEEH				;  4
                        MD5FF3_MMX	 4, MD5_S11, 0F57C0FAFH				;  5
                        MD5FF3_MMX	 5, MD5_S12, 04787C62AH				;  6
                        MD5FF3_MMX	 6, MD5_S13, 0A8304613H				;  7
                        MD5FF3_MMX	 7, MD5_S14, 0FD469501H				;  8
                        MD5FF3_MMX	 8, MD5_S11, 0698098D8H				;  9
                        MD5FF3_MMX	 9, MD5_S12, 08B44F7AFH				; 10
                        MD5FF3_MMX	10, MD5_S13, 0FFFF5BB1H				; 11
                        MD5FF3_MMX	11, MD5_S14, 0895CD7BEH				; 12
                        MD5FF3_MMX	12, MD5_S11, 06B901122H				; 13
                        MD5FF3_MMX	13, MD5_S12, 0FD987193H				; 14
                        MD5FF3_MMX	14, MD5_S13, 0A679438EH				; 15
                        MD5FF3_MMX	15, MD5_S14, 049B40821H				; 16
; round 2
                        MD5GG3_MMX	 1, MD5_S21, 0F61E2562H				; 17
                        MD5GG3_MMX	 6, MD5_S22, 0C040B340H				; 18
                        MD5GG3_MMX	11, MD5_S23, 0265E5A51H				; 19
                        MD5GG3_MMX	 0, MD5_S24, 0E9B6C7AAH				; 20
                        MD5GG3_MMX	 5, MD5_S21, 0D62F105DH				; 21
                        MD5GG3_MMX	10, MD5_S22, 002441453H				; 22
                        MD5GG3_MMX	15, MD5_S23, 0D8A1E681H				; 23
                        MD5GG3_MMX	 4, MD5_S24, 0E7D3FBC8H				; 24
                        MD5GG3_MMX	 9, MD5_S21, 021E1CDE6H				; 25
                        MD5GG3_MMX	14, MD5_S22, 0C33707D6H				; 26
                        MD5GG3_MMX	 3, MD5_S23, 0F4D50D87H				; 27
                        MD5GG3_MMX	 8, MD5_S24, 0455A14EDH				; 28
                        MD5GG3_MMX	13, MD5_S21, 0A9E3E905H				; 29
                        MD5GG3_MMX	 2, MD5_S22, 0FCEFA3F8H				; 30
                        MD5GG3_MMX	 7, MD5_S23, 0676F02D9H				; 31
                        MD5GG3_MMX	12, MD5_S24, 08D2A4C8AH				; 32
; round 3
                        MD5HH3_MMX	 5, MD5_S31, 0FFFA3942H				; 33
                        MD5HH3_MMX	 8, MD5_S32, 08771F681H				; 34
                        MD5HH3_MMX	11, MD5_S33, 06D9D6122H				; 35
                        MD5HH3_MMX	14, MD5_S34, 0FDE5380CH				; 36
                        MD5HH3_MMX	 1, MD5_S31, 0A4BEEA44H				; 37
                        MD5HH3_MMX	 4, MD5_S32, 04BDECFA9H				; 38
                        MD5HH3_MMX	 7, MD5_S33, 0F6BB4B60H				; 39
                        MD5HH3_MMX	10, MD5_S34, 0BEBFBC70H				; 40
                        MD5HH3_MMX	13, MD5_S31, 0289B7EC6H				; 41
                        MD5HH3_MMX	 0, MD5_S32, 0EAA127FAH				; 42
                        MD5HH3_MMX	 3, MD5_S33, 0D4EF3085H				; 43
                        MD5HH3_MMX	 6, MD5_S34, 004881D05H				; 44
                        MD5HH3_MMX	 9, MD5_S31, 0D9D4D039H				; 45
                        MD5HH3_MMX	12, MD5_S32, 0E6DB99E5H				; 46
                        MD5HH3_MMX	15, MD5_S33, 01FA27CF8H				; 47
                        MD5HH3_MMX	 2, MD5_S34, 0C4AC5665H				; 48
; round 4
                        MD5II3_MMX	 0, MD5_S41, 0F4292244H				; 49
                        MD5II3_MMX	 7, MD5_S42, 0432AFF97H				; 50
                        MD5II3_MMX	14, MD5_S43, 0AB9423A7H				; 51
                        MD5II3_MMX	 5, MD5_S44, 0FC93A039H				; 52
                        MD5II3_MMX	12, MD5_S41, 0655B59C3H				; 53
                        MD5II3_MMX	 3, MD5_S42, 08F0CCC92H				; 54
                        MD5II3_MMX	10, MD5_S43, 0FFEFF47DH				; 55
                        MD5II3_MMX	 1, MD5_S44, 085845DD1H				; 56
                        MD5II3_MMX	 8, MD5_S41, 06FA87E4FH				; 57
                        MD5II3_MMX	15, MD5_S42, 0FE2CE6E0H				; 58
                        MD5II3_MMX	 6, MD5_S43, 0A3014314H				; 59
                        MD5II3_MMX	13, MD5_S44, 04E0811A1H				; 60
                        MD5II3_MMX	 4, MD5_S41, 0F7537E82H				; 61
                        MD5II3_MMX	11, MD5_S42, 0BD3AF235H				; 62
                        MD5II3_MMX	 2, MD5_S43, 02AD7D2BBH				; 63
                        MD5II3_MMX	 9, MD5_S44, 0EB86D391H				; 64
                        paddd		mmx_a, [__Digest+0*8]
                        paddd		mmx_b, [__Digest+1*8]
                        paddd		mmx_c, [__Digest+2*8]
                        paddd		mmx_d, [__Digest+3*8]
                        movq		[__Digest+0*8], mmx_a
                        movq		[__Digest+1*8], mmx_b
                        movq		[__Digest+2*8], mmx_c
                        movq		[__Digest+3*8], mmx_d
                        add			[edi+m_nHash0], reg_a
                        add			[edi+m_nHash1], reg_b
                        add			[edi+m_nHash2], reg_c
                        add			[edi+m_nHash3], reg_d
						add			dword ptr __Data1, 64
						add			dword ptr __Data2, 64
						add			ebp, 64
						dec			dword ptr __Count
                        jnz			full_blocks
                        mov			ebp, __Frame
						mov			ebx, __this1
						mov			edx, __this2
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
						mov			esp, ebp
                        popa
                        emms
                        ret 24
MD5_Add3_MMX			ENDP
						
MD5FF4_FF4_MMX			MACRO		count:REQ,s:REQ, count1:REQ, s1:REQ, ac1:REQ, count2:REQ, s2:REQ, ac2:REQ
; a = b+(a+x[count]+ac+((b&c)|(~b&d)))rol s
; a = b+(a+x[count]+ax+(d^(b&(c^d))))rol s
						movq		mmx_temp1, mmx_c
                        mov         reg_temp1, reg_c
                        xor			reg_c, reg_d
						paddd		mmx_a, [__w+count*8]
                        add         reg_a, ac1
                        and         reg_c, reg_b
						pxor		mmx_c, mmx_d
                        add         reg_a, [reg_base+count1*4]
                        xor			reg_c, reg_d
						pand		mmx_c, mmx_b
                        add         reg_a, reg_c
						paddd		mmx_a, qword ptr [ac_const+ac_index*16]
                        rol         reg_a, s1
						pxor		mmx_c, mmx_d
                        add         reg_a, reg_b
                        mov         reg_c, reg_b
						paddd		mmx_a, mmx_c
                        xor			reg_b, reg_temp1
                        add         reg_d, [reg_base+count2*4]
						movq		mmx_temp2, mmx_a
                        add         reg_d, ac2
                        and         reg_b, reg_a
						pslld		mmx_a, s
                        xor			reg_b, reg_temp1
						psrld		mmx_temp2, 32-s
                        add         reg_d, reg_b
						por			mmx_a, mmx_temp2
                        rol         reg_d, s2
						paddd		mmx_a, mmx_b
                        add         reg_d, reg_a
reg_t					textequ		mmx_d
mmx_d					textequ		mmx_temp1
mmx_temp1				textequ		mmx_c
mmx_c					textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		reg_t
ac_index				=			ac_index + 1
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
						ENDM											; MD5FF4_FF4_MMX

MD5FF4_GG4_MMX			MACRO		count:REQ,s:REQ, count1:REQ, s1:REQ, ac1:REQ, count2:REQ, s2:REQ, ac2:REQ
; a = b+(a+x[count]+ac+((b&c)|(~b&d)))rol s
; a = b+(a+x[count]+ax+(d^(b&(c^d))))rol s
						movq		mmx_temp1, mmx_c
                        mov         reg_temp1, reg_b
                        xor         reg_b, reg_c
						paddd		mmx_a, [__w+count*8]
                        add         reg_a, ac1
                        and         reg_b, reg_d
						pxor		mmx_c, mmx_d
                        add         reg_a, [reg_base+count1*4]
                        xor         reg_b, reg_c
						pand		mmx_c, mmx_b
                        add         reg_a, reg_b
						paddd		mmx_a, qword ptr [ac_const+ac_index*16]
                        rol         reg_a, s1
						pxor		mmx_c, mmx_d
                        add         reg_a, reg_temp1
						paddd		mmx_a, mmx_c
                        mov         reg_b, reg_a
                        xor         reg_a, reg_temp1
						movq		mmx_temp2, mmx_a
                        add         reg_d, [reg_base+count2*4]
                        and         reg_a, reg_c
						pslld		mmx_a, s
                        add         reg_d, ac2
                        xor         reg_a, reg_temp1
						psrld		mmx_temp2, 32-s
                        add         reg_d, reg_a
						por			mmx_a, mmx_temp2
                        rol         reg_d, s2
						paddd		mmx_a, mmx_b
                        add         reg_d, reg_b
reg_t					textequ		mmx_d
mmx_d					textequ		mmx_temp1
mmx_temp1				textequ		mmx_c
mmx_c					textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		reg_t
ac_index				=			ac_index + 1
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
						ENDM											; MD5FF4_GG4_MMX
                        
MD5GG4_HH4_MMX			MACRO		count:REQ,s:REQ, count1:REQ, s1:REQ, ac1:REQ, count2:REQ, s2:REQ, ac2:REQ
; a = b+(a+x[count]+ac+((d&b)|(~d&c)))rol s
; a = b+(a+x[count]+ac+(c^(d&(b^c))))rols s
						movq		mmx_temp1, mmx_b
                        mov         reg_temp1, reg_b
                        xor         reg_b, reg_c
						paddd		mmx_a, [__w+count*8]
                        add         reg_a, ac1
                        xor         reg_b, reg_d
						pxor		mmx_b, mmx_c
                        add         reg_a, [reg_base+count1*4]
						pand		mmx_b, mmx_d
                        add         reg_a, reg_b
						pxor		mmx_b, mmx_c
                        rol         reg_a, s1
						paddd		mmx_a, qword ptr [ac_const+ac_index*16]
                        add         reg_a, reg_temp1
						paddd		mmx_a, mmx_b
                        mov         reg_b, reg_a
                        xor         reg_a, reg_temp1
						movq		mmx_temp2, mmx_a
                        add         reg_d, [reg_base+count2*4]
                        xor         reg_a, reg_c
						pslld		mmx_a, s
                        add         reg_d, ac2
						psrld		mmx_temp2, 32-s
                        add         reg_d, reg_a
						por			mmx_a, mmx_temp2
                        rol         reg_d, s2
						paddd		mmx_a, mmx_temp1
                        add         reg_d, reg_b
reg_t					textequ		mmx_d
mmx_d					textequ		mmx_c
mmx_c					textequ		mmx_temp1
mmx_temp1				textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		reg_t
ac_index				=			ac_index + 1
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
						ENDM											; MD5GG4_HH4_MMX

MD5GG4_II4_MMX			MACRO		count:REQ,s:REQ, count1:REQ, s1:REQ, ac1:REQ, count2:REQ, s2:REQ, ac2:REQ
; a = b+(a+x[count]+ac+((d&b)|(~d&c)))rol s
; a = b+(a+x[count]+ac+(c^(d&(b^c))))rols s
						movq		mmx_temp1, mmx_b
                        mov         reg_temp1, reg_d
                        not         reg_d
						paddd		mmx_a, [__w+count*8]
                        add         reg_a, ac1
                        or          reg_d, reg_b
						pxor		mmx_b, mmx_c
                        add         reg_a, [reg_base+count1*4]
                        xor         reg_d, reg_c
						pand		mmx_b, mmx_d
                        add         reg_a, reg_d
						paddd		mmx_a, qword ptr [ac_const+ac_index*16]
                        rol         reg_a, s1
						pxor		mmx_b, mmx_c
                        add         reg_a, reg_b
                        mov         reg_d, reg_c
						paddd		mmx_a, mmx_b
                        not         reg_c
                        add         reg_temp1, [reg_base+count2*4]
						movq		mmx_temp2, mmx_a
                        or          reg_c, reg_a
                        add         reg_temp1, ac2
						pslld		mmx_a, s
                        xor         reg_c, reg_b
                        add         reg_temp1, reg_c
						psrld		mmx_temp2, 32-s
                        rol         reg_temp1, s2
						por			mmx_a, mmx_temp2
                        add         reg_temp1, reg_a
						paddd		mmx_a, mmx_temp1
reg_t					textequ		mmx_d
mmx_d					textequ		mmx_c
mmx_c					textequ		mmx_temp1
mmx_temp1				textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		reg_t
ac_index				=			ac_index + 1
reg_t					textequ		reg_temp1
reg_temp1				textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
reg_t					textequ		reg_temp1
reg_temp1				textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD5GG4_II4_MMX

MD5HH4_FF4_MMX			MACRO       count:REQ,s:REQ, count1:REQ, s1:REQ, ac1:REQ, count2:REQ, s2:REQ, ac2:REQ
; a = b+(a+x[count]+ac+(b^c^d)) rol s
						movq		mmx_temp1, mmx_b
                        mov         reg_temp1, reg_c
                        xor			reg_c, reg_d
						paddd		mmx_a, [__w+count*8]
                        add         reg_a, ac1
                        and         reg_c, reg_b
						pxor		mmx_b, mmx_c
                        add         reg_a, [reg_base+count1*4]
                        xor			reg_c, reg_d
						pxor		mmx_b, mmx_d
                        add         reg_a, reg_c
                        mov         reg_c, reg_b
						paddd		mmx_a, qword ptr [ac_const+ac_index*16]
                        rol         reg_a, s1
						paddd		mmx_a, mmx_b
                        add         reg_a, reg_c
                        xor			reg_b, reg_temp1
						movq		mmx_temp2, mmx_a
                        add         reg_d, [reg_base+count2*4]
                        and         reg_b, reg_a
						pslld		mmx_a, s
                        add         reg_d, ac2
                        xor			reg_b, reg_temp1
						psrld		mmx_temp2, 32-s
                        add         reg_d, reg_b
						por			mmx_a, mmx_temp2
                        rol         reg_d, s2
						paddd		mmx_a, mmx_temp1
                        add         reg_d, reg_a
reg_t					textequ		mmx_d
mmx_d					textequ		mmx_c
mmx_c					textequ		mmx_temp1
mmx_temp1				textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		reg_t
ac_index				=			ac_index + 1
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
						ENDM											; MD5HH4_FF4_MMX

MD5HH4_GG4_MMX			MACRO       count:REQ,s:REQ, count1:REQ, s1:REQ, ac1:REQ, count2:REQ, s2:REQ, ac2:REQ
; a = b+(a+x[count]+ac+(b^c^d)) rol s
						movq		mmx_temp1, mmx_b
                        mov         reg_temp1, reg_b
                        xor         reg_b, reg_c
						paddd		mmx_a, [__w+count*8]
                        add         reg_a, ac1
                        and         reg_b, reg_d
						pxor		mmx_b, mmx_c
                        add         reg_a, [reg_base+count1*4]
                        xor         reg_b, reg_c
						pxor		mmx_b, mmx_d
                        add         reg_a, reg_b
                        add         reg_d, ac2
						paddd		mmx_a, qword ptr [ac_const+ac_index*16]
                        rol         reg_a, s1
						paddd		mmx_a, mmx_b
                        add         reg_a, reg_temp1
                        add         reg_d, [reg_base+count2*4]
						movq		mmx_temp2, mmx_a
                        mov         reg_b, reg_a
                        xor         reg_a, reg_temp1
						pslld		mmx_a, s
                        and         reg_a, reg_c
						psrld		mmx_temp2, 32-s
                        xor         reg_a, reg_temp1
						por			mmx_a, mmx_temp2
                        add         reg_d, reg_a
						paddd		mmx_a, mmx_temp1
                        rol         reg_d, s2
                        add         reg_d, reg_b
reg_t					textequ		mmx_d
mmx_d					textequ		mmx_c
mmx_c					textequ		mmx_temp1
mmx_temp1				textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		reg_t
ac_index				=			ac_index + 1
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
						ENDM											; MD5HH4_GG4_MMX

MD5II4_HH4_MMX			MACRO       count:REQ,s:REQ, count1:REQ, s1:REQ, ac1:REQ, count2:REQ, s2:REQ, ac2:REQ
; a = b+(a+x[count]+ac+(c^(~d|b))) rol s
						movq		mmx_temp1, mmx_d
                        mov         reg_temp1, reg_b
                        xor         reg_b, reg_c
						paddd		mmx_a, [__w+count*8]
                        add         reg_a, ac1
                        xor         reg_b, reg_d
						pxor		mmx_d, mmx_FFFF
                        add         reg_a, [reg_base+count1*4]
						por			mmx_d, mmx_b
                        add         reg_a, reg_b
						paddd		mmx_a, qword ptr [ac_const+ac_index*16]
                        rol         reg_a, s1
						pxor		mmx_d, mmx_c
                        add         reg_a, reg_temp1
						paddd		mmx_a, mmx_d
                        mov         reg_b, reg_a
                        xor         reg_a, reg_temp1
						movq		mmx_temp2, mmx_a
                        add         reg_d, [reg_base+count2*4]
                        xor         reg_a, reg_c
						pslld		mmx_a, s
                        add         reg_d, ac2
						psrld		mmx_temp2, 32-s
                        add         reg_d, reg_a
						por			mmx_a, mmx_temp2
                        rol         reg_d, s2
						paddd		mmx_a, mmx_b
                        add         reg_d, reg_b
reg_t					textequ		mmx_temp1
mmx_temp1				textequ		mmx_d
mmx_d					textequ		mmx_c
mmx_c					textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		reg_t
ac_index				=			ac_index + 1
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
						ENDM											; MD5II4_HH4_MMX

MD5II4_II4_MMX			MACRO       count:REQ,s:REQ, count1:REQ, s1:REQ, ac1:REQ, count2:REQ, s2:REQ, ac2:REQ
; a = b+(a+x[count]+ac+(c^(~d|b))) rol s
						movq		mmx_temp1, mmx_d
                        mov         reg_temp1, reg_d
                        not         reg_d
						paddd		mmx_a, [__w+count*8]
                        add         reg_a, ac1
                        or          reg_d, reg_b
						pxor		mmx_d, mmx_FFFF
                        add         reg_a, [reg_base+count1*4]
                        xor         reg_d, reg_c
						por			mmx_d, mmx_b
                        add         reg_a, reg_d
						paddd		mmx_a, qword ptr [ac_const+ac_index*16]
                        rol         reg_a, s1
						pxor		mmx_d, mmx_c
                        add         reg_a, reg_b
                        mov         reg_d, reg_c
						paddd		mmx_a, mmx_d
                        not         reg_c
                        add         reg_temp1, [reg_base+count2*4]
						movq		mmx_temp2, mmx_a
                        or          reg_c, reg_a
                        add         reg_temp1, ac2
						pslld		mmx_a, s
                        xor         reg_c, reg_b
						psrld		mmx_temp2, 32-s
                        add         reg_temp1, reg_c
						por			mmx_a, mmx_temp2
                        rol         reg_temp1, s2
						paddd		mmx_a, mmx_b
                        add         reg_temp1, reg_a
reg_t					textequ		mmx_temp1
mmx_temp1				textequ		mmx_d
mmx_d					textequ		mmx_c
mmx_c					textequ		mmx_b
mmx_b					textequ		mmx_a
mmx_a					textequ		reg_t
ac_index				=			ac_index + 1
reg_t					textequ		reg_temp1
reg_temp1				textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
reg_t					textequ		reg_temp1
reg_temp1				textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD5II4_II4_MMX

MD5_Add4_SSE2			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD, _this4:DWORD, _Data4:DWORD
MD5_Add4_SSE2			ENDP
MD5_Add4_MMX			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD, _this4:DWORD, _Data4:DWORD
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
mmx_FFFF				textequ		<mm7>
                        movq		mmx_FFFF, const_FFFFFFFFFFFFFFFF
reg_a					textequ		<eax>
reg_b					textequ		<ebx>
reg_c					textequ		<edx>
reg_d					textequ		<esi>
reg_temp1				textequ		<edi>
						mov			ebp, __Data3
						mov			ecx, __Data4
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
                        mov			reg_a, [__Digest+2*16]
                        mov			reg_b, [__Digest+2*16+4]
                        mov			reg_c, [__Digest+2*16+8]
                        mov			reg_d, [__Digest+2*16+12]
reg_base				textequ		<ebp>
ac_index				=			0
; round 1
                        MD5FF4_FF4_MMX	 0, MD5_S11,  0, MD5_S11, 0D76AA478H,  1, MD5_S12, 0E8C7B756H			;  1
                        MD5FF4_FF4_MMX	 1, MD5_S12,  2, MD5_S13, 0242070DBH,  3, MD5_S14, 0C1BDCEEEH			;  2
                        MD5FF4_FF4_MMX	 2, MD5_S13,  4, MD5_S11, 0F57C0FAFH,  5, MD5_S12, 04787C62AH			;  3
                        MD5FF4_FF4_MMX	 3, MD5_S14,  6, MD5_S13, 0A8304613H,  7, MD5_S14, 0FD469501H			;  4
                        MD5FF4_FF4_MMX	 4, MD5_S11,  8, MD5_S11, 0698098D8H,  9, MD5_S12, 08B44F7AFH 			;  5
                        MD5FF4_FF4_MMX	 5, MD5_S12, 10, MD5_S13, 0FFFF5BB1H, 11, MD5_S14, 0895CD7BEH			;  6
                        MD5FF4_FF4_MMX	 6, MD5_S13, 12, MD5_S11, 06B901122H, 13, MD5_S12, 0FD987193H			;  7
                        MD5FF4_FF4_MMX	 7, MD5_S14, 14, MD5_S13, 0A679438EH, 15, MD5_S14, 049B40821H			;  8
                        MD5FF4_GG4_MMX	 8, MD5_S11,  1, MD5_S21, 0F61E2562H,  6, MD5_S22, 0C040B340H			;  9
                        MD5FF4_GG4_MMX	 9, MD5_S12, 11, MD5_S23, 0265E5A51H,  0, MD5_S24, 0E9B6C7AAH			; 10
                        MD5FF4_GG4_MMX	10, MD5_S13,  5, MD5_S21, 0D62F105DH, 10, MD5_S22, 002441453H			; 11
                        MD5FF4_GG4_MMX	11, MD5_S14, 15, MD5_S23, 0D8A1E681H,  4, MD5_S24, 0E7D3FBC8H			; 12
                        MD5FF4_GG4_MMX	12, MD5_S11,  9, MD5_S21, 021E1CDE6H, 14, MD5_S22, 0C33707D6H			; 13
                        MD5FF4_GG4_MMX	13, MD5_S12,  3, MD5_S23, 0F4D50D87H,  8, MD5_S24, 0455A14EDH			; 14
                        MD5FF4_GG4_MMX	14, MD5_S13, 13, MD5_S21, 0A9E3E905H,  2, MD5_S22, 0FCEFA3F8H			; 15
                        MD5FF4_GG4_MMX	15, MD5_S14,  7, MD5_S23, 0676F02D9H, 12, MD5_S24, 08D2A4C8AH			; 16
; round 2
                        MD5GG4_HH4_MMX	 1, MD5_S21,  5, MD5_S31, 0FFFA3942H,  8, MD5_S32, 08771F681H			; 17
                        MD5GG4_HH4_MMX	 6, MD5_S22, 11, MD5_S33, 06D9D6122H, 14, MD5_S34, 0FDE5380CH			; 18
                        MD5GG4_HH4_MMX	11, MD5_S23,  1, MD5_S31, 0A4BEEA44H,  4, MD5_S32, 04BDECFA9H			; 19
                        MD5GG4_HH4_MMX	 0, MD5_S24,  7, MD5_S33, 0F6BB4B60H, 10, MD5_S34, 0BEBFBC70H			; 20
                        MD5GG4_HH4_MMX	 5, MD5_S21, 13, MD5_S31, 0289B7EC6H,  0, MD5_S32, 0EAA127FAH			; 21
                        MD5GG4_HH4_MMX	10, MD5_S22,  3, MD5_S33, 0D4EF3085H,  6, MD5_S34, 004881D05H			; 22
                        MD5GG4_HH4_MMX	15, MD5_S23,  9, MD5_S31, 0D9D4D039H, 12, MD5_S32, 0E6DB99E5H			; 23
                        MD5GG4_HH4_MMX	 4, MD5_S24, 15, MD5_S33, 01FA27CF8H,  2, MD5_S34, 0C4AC5665H			; 24
                        MD5GG4_II4_MMX	 9, MD5_S21,  0, MD5_S41, 0F4292244H,  7, MD5_S42, 0432AFF97H			; 25
                        MD5GG4_II4_MMX	14, MD5_S22, 14, MD5_S43, 0AB9423A7H,  5, MD5_S44, 0FC93A039H			; 26
                        MD5GG4_II4_MMX	 3, MD5_S23, 12, MD5_S41, 0655B59C3H,  3, MD5_S42, 08F0CCC92H			; 27
                        MD5GG4_II4_MMX	 8, MD5_S24, 10, MD5_S43, 0FFEFF47DH,  1, MD5_S44, 085845DD1H			; 28
                        MD5GG4_II4_MMX	13, MD5_S21,  8, MD5_S41, 06FA87E4FH, 15, MD5_S42, 0FE2CE6E0H			; 29
                        MD5GG4_II4_MMX	 2, MD5_S22,  6, MD5_S43, 0A3014314H, 13, MD5_S44, 04E0811A1H			; 30
                        MD5GG4_II4_MMX	 7, MD5_S23,  4, MD5_S41, 0F7537E82H, 11, MD5_S42, 0BD3AF235H			; 31
                        MD5GG4_II4_MMX	12, MD5_S24,  2, MD5_S43, 02AD7D2BBH,  9, MD5_S44, 0EB86D391H			; 32
                        add			[__Digest+2*16], reg_a
                        add			[__Digest+2*16+4], reg_b
                        add			[__Digest+2*16+8], reg_c
                        add			[__Digest+2*16+12], reg_d
reg_base				textequ		<ecx>
; round 3
                        mov			reg_a, [__Digest+3*16]
                        mov			reg_b, [__Digest+3*16+4]
                        mov			reg_c, [__Digest+3*16+8]
                        mov			reg_d, [__Digest+3*16+12]
                        MD5HH4_FF4_MMX	 5, MD5_S31,  0, MD5_S11, 0D76AA478H,  1, MD5_S12, 0E8C7B756H			; 33
                        MD5HH4_FF4_MMX	 8, MD5_S32,  2, MD5_S13, 0242070DBH,  3, MD5_S14, 0C1BDCEEEH			; 34
                        MD5HH4_FF4_MMX	11, MD5_S33,  4, MD5_S11, 0F57C0FAFH,  5, MD5_S12, 04787C62AH			; 35
                        MD5HH4_FF4_MMX	14, MD5_S34,  6, MD5_S13, 0A8304613H,  7, MD5_S14, 0FD469501H			; 36
                        MD5HH4_FF4_MMX	 1, MD5_S31,  8, MD5_S11, 0698098D8H,  9, MD5_S12, 08B44F7AFH			; 37
                        MD5HH4_FF4_MMX	 4, MD5_S32, 10, MD5_S13, 0FFFF5BB1H, 11, MD5_S14, 0895CD7BEH			; 38
                        MD5HH4_FF4_MMX	 7, MD5_S33, 12, MD5_S11, 06B901122H, 13, MD5_S12, 0FD987193H			; 39
                        MD5HH4_FF4_MMX	10, MD5_S34, 14, MD5_S13, 0A679438EH, 15, MD5_S14, 049B40821H			; 40
                        MD5HH4_GG4_MMX	13, MD5_S31,  1, MD5_S21, 0F61E2562H,  6, MD5_S22, 0C040B340H			; 41
                        MD5HH4_GG4_MMX	 0, MD5_S32, 11, MD5_S23, 0265E5A51H,  0, MD5_S24, 0E9B6C7AAH			; 42
                        MD5HH4_GG4_MMX	 3, MD5_S33,  5, MD5_S21, 0D62F105DH, 10, MD5_S22, 002441453H			; 43
                        MD5HH4_GG4_MMX	 6, MD5_S34, 15, MD5_S23, 0D8A1E681H,  4, MD5_S24, 0E7D3FBC8H			; 44
                        MD5HH4_GG4_MMX	 9, MD5_S31,  9, MD5_S21, 021E1CDE6H, 14, MD5_S22, 0C33707D6H			; 45
                        MD5HH4_GG4_MMX	12, MD5_S32,  3, MD5_S23, 0F4D50D87H,  8, MD5_S24, 0455A14EDH			; 46
                        MD5HH4_GG4_MMX	15, MD5_S33, 13, MD5_S21, 0A9E3E905H,  2, MD5_S22, 0FCEFA3F8H			; 47
                        MD5HH4_GG4_MMX	 2, MD5_S34,  7, MD5_S23, 0676F02D9H, 12, MD5_S24, 08D2A4C8AH			; 48
; round 4
                        MD5II4_HH4_MMX	 0, MD5_S41,  5, MD5_S31, 0FFFA3942H,  8, MD5_S32, 08771F681H			; 49
                        MD5II4_HH4_MMX	 7, MD5_S42, 11, MD5_S33, 06D9D6122H, 14, MD5_S34, 0FDE5380CH			; 50
                        MD5II4_HH4_MMX	14, MD5_S43,  1, MD5_S31, 0A4BEEA44H,  4, MD5_S32, 04BDECFA9H			; 51
                        MD5II4_HH4_MMX	 5, MD5_S44,  7, MD5_S33, 0F6BB4B60H, 10, MD5_S34, 0BEBFBC70H			; 52
                        MD5II4_HH4_MMX	12, MD5_S41, 13, MD5_S31, 0289B7EC6H,  0, MD5_S32, 0EAA127FAH			; 53
                        MD5II4_HH4_MMX	 3, MD5_S42,  3, MD5_S33, 0D4EF3085H,  6, MD5_S34, 004881D05H			; 54
                        MD5II4_HH4_MMX	10, MD5_S43,  9, MD5_S31, 0D9D4D039H, 12, MD5_S32, 0E6DB99E5H			; 55
                        MD5II4_HH4_MMX	 1, MD5_S44, 15, MD5_S33, 01FA27CF8H,  2, MD5_S34, 0C4AC5665H			; 56
                        MD5II4_II4_MMX	 8, MD5_S41,  0, MD5_S41, 0F4292244H,  7, MD5_S42, 0432AFF97H			; 57
                        MD5II4_II4_MMX	15, MD5_S42, 14, MD5_S43, 0AB9423A7H,  5, MD5_S44, 0FC93A039H			; 58
                        MD5II4_II4_MMX	 6, MD5_S43, 12, MD5_S41, 0655B59C3H,  3, MD5_S42, 08F0CCC92H			; 59
                        MD5II4_II4_MMX	13, MD5_S44, 10, MD5_S43, 0FFEFF47DH,  1, MD5_S44, 085845DD1H			; 60
                        MD5II4_II4_MMX	 4, MD5_S41,  8, MD5_S41, 06FA87E4FH, 15, MD5_S42, 0FE2CE6E0H			; 61
                        MD5II4_II4_MMX	11, MD5_S42,  6, MD5_S43, 0A3014314H, 13, MD5_S44, 04E0811A1H			; 62
                        MD5II4_II4_MMX	 2, MD5_S43,  4, MD5_S41, 0F7537E82H, 11, MD5_S42, 0BD3AF235H			; 63
                        MD5II4_II4_MMX	 9, MD5_S44,  2, MD5_S43, 02AD7D2BBH,  9, MD5_S44, 0EB86D391H			; 64
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
						add			ebp, 64
						add			ecx, 64
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
MD5_Add4_MMX			ENDP

						.xmm

MD5FF5_SSE2				MACRO		count:REQ,s:REQ, ac:REQ
; a = b+(a+x[count]+ac+((b&c)|(~b&d)))rol s
; a = b+(a+x[count]+ax+(d^(b&(c^d))))rol s
						movdqa		xmm_temp1, xmm_c
                        mov         reg_temp1, reg_c
						paddd		xmm_a, [__w+count*16]
                        xor			reg_c, reg_d
						pxor		xmm_c, xmm_d
                        add         reg_a, [ebp+count*4]
						pand		xmm_c, xmm_b
                        and         reg_c, reg_b
						paddd		xmm_a, [ac_const+ac_index*16]
                        add         reg_a, ac
						pxor		xmm_c, xmm_d
                        xor			reg_c, reg_d
						paddd		xmm_a, xmm_c
                        add         reg_a, reg_c
						movdqa		xmm_temp2, xmm_a
                        rol         reg_a, s
						pslld		xmm_a, s
                        add         reg_a, reg_b
						psrld		xmm_temp2, 32-s
						por			xmm_a, xmm_temp2
						paddd		xmm_a, xmm_b
reg_t					textequ		xmm_d
xmm_d					textequ		xmm_temp1
xmm_temp1				textequ		xmm_c
xmm_c					textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		reg_t
ac_index				=			ac_index + 1
reg_t					textequ		reg_d
reg_d					textequ		reg_temp1
reg_temp1				textequ		reg_c
reg_c					textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD5FF5_SSE2
                        

MD5GG5_SSE2				MACRO		count:REQ,s:REQ, ac:REQ
; a = b+(a+x[count]+ac+((d&b)|(~d&c)))rol s
; a = b+(a+x[count]+ac+(c^(d&(b^c))))rols s
						movdqa		xmm_temp1, xmm_b
                        mov         reg_temp1, reg_b
						paddd		xmm_a, [__w+count*16]
                        xor         reg_b, reg_c
						pxor		xmm_b, xmm_c
                        add         reg_a, [ebp+count*4]
						pand		xmm_b, xmm_d
                        and         reg_b, reg_d
						paddd		xmm_a, [ac_const+ac_index*16]
                        add         reg_a, ac
						pxor		xmm_b, xmm_c
                        xor         reg_b, reg_c
						paddd		xmm_a, xmm_b
                        add         reg_a, reg_b
						movdqa		xmm_temp2, xmm_a
                        rol         reg_a, s
						pslld		xmm_a, s
                        add         reg_a, reg_temp1
						psrld		xmm_temp2, 32-s
						por			xmm_a, xmm_temp2
						paddd		xmm_a, xmm_temp1
reg_t					textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_temp1
xmm_temp1				textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		reg_t
ac_index				=			ac_index + 1
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_temp1
reg_temp1				textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD5GG5_SSE2

MD5HH5_SSE2				MACRO       count:REQ,s:REQ, ac:REQ
; a = b+(a+x[count]+ac+(b^c^d)) rol s
						movdqa		xmm_temp1, xmm_b
                        mov         reg_temp1, reg_b
						paddd		xmm_a, [__w+count*16]
                        xor         reg_b, reg_c
						pxor		xmm_b, xmm_c
                        add         reg_a, [ebp+count*4]
						paddd		xmm_a, [ac_const+ac_index*16]
                        xor         reg_b, reg_d
						pxor		xmm_b, xmm_d
                        add         reg_a, ac
						paddd		xmm_a, xmm_b
                        add         reg_a, reg_b
						movdqa		xmm_temp2, xmm_a
                        rol         reg_a, s
						pslld		xmm_a, s
                        add         reg_a, reg_temp1
						psrld		xmm_temp2, 32-s
						por			xmm_a, xmm_temp2
						paddd		xmm_a, xmm_temp1
reg_t					textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_temp1
xmm_temp1				textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		reg_t
ac_index				=			ac_index + 1
reg_t					textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_temp1
reg_temp1				textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD5HH5_SSE2

MD5II5_SSE2				MACRO       count:REQ,s:REQ, ac:REQ
; a = b+(a+x[count]+ac+(c^(~d|b))) rol s
						movdqa		xmm_temp1, xmm_d
                        mov         reg_temp1, reg_d
						pxor		xmm_d, xmm_FFFF
                        not         reg_d
						paddd		xmm_a, [__w+count*16]
                        add         reg_a, [ebp+count*4]
						por			xmm_d, xmm_b
                        or          reg_d, reg_b
						paddd		xmm_a, [ac_const+ac_index*16]
                        add         reg_a, ac
						pxor		xmm_d, xmm_c
                        xor         reg_d, reg_c
						paddd		xmm_a, xmm_d
                        add         reg_a, reg_d
						movdqa		xmm_temp2, xmm_a
                        rol         reg_a, s
						pslld		xmm_a, s
                        add         reg_a, reg_b
						psrld		xmm_temp2, 32-s
						por			xmm_a, xmm_temp2
						paddd		xmm_a, xmm_b
reg_t					textequ		xmm_temp1
xmm_temp1				textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		reg_t
ac_index				=			ac_index + 1
reg_t					textequ		reg_temp1
reg_temp1				textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD5II5_SSE2

MD5_Add5_SSE2			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD, _this4:DWORD, _Data4:DWORD, _this5:DWORD, _Data5:DWORD
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
xmm_FFFF				textequ		<xmm7>
                        movdqa		xmm_FFFF, const_FFFFFFFFFFFFFFFF
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
                        movdqa		xmm6, xmm0
                        movdqa		xmm3, xmm4
                        punpcklqdq	xmm0, xmm2
                        punpcklqdq	xmm4, xmm5
                        punpckhqdq	xmm6, xmm2
                        punpckhqdq	xmm3, xmm5
                        movdqa		[__w+count*4*16+0*16], xmm0
                        movdqa		[__w+count*4*16+1*16], xmm6
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
ac_index				=			0
; round 1
                        MD5FF5_SSE2	 0, MD5_S11, 0D76AA478H				;  1
                        MD5FF5_SSE2	 1, MD5_S12, 0E8C7B756H				;  2
                        MD5FF5_SSE2	 2, MD5_S13, 0242070DBH				;  3
                        MD5FF5_SSE2	 3, MD5_S14, 0C1BDCEEEH				;  4
                        MD5FF5_SSE2	 4, MD5_S11, 0F57C0FAFH				;  5
                        MD5FF5_SSE2	 5, MD5_S12, 04787C62AH				;  6
                        MD5FF5_SSE2	 6, MD5_S13, 0A8304613H				;  7
                        MD5FF5_SSE2	 7, MD5_S14, 0FD469501H				;  8
                        MD5FF5_SSE2	 8, MD5_S11, 0698098D8H				;  9
                        MD5FF5_SSE2	 9, MD5_S12, 08B44F7AFH				; 10
                        MD5FF5_SSE2	10, MD5_S13, 0FFFF5BB1H				; 11
                        MD5FF5_SSE2	11, MD5_S14, 0895CD7BEH				; 12
                        MD5FF5_SSE2	12, MD5_S11, 06B901122H				; 13
                        MD5FF5_SSE2	13, MD5_S12, 0FD987193H				; 14
                        MD5FF5_SSE2	14, MD5_S13, 0A679438EH				; 15
                        MD5FF5_SSE2	15, MD5_S14, 049B40821H				; 16
; round 2
                        MD5GG5_SSE2	 1, MD5_S21, 0F61E2562H				; 17
                        MD5GG5_SSE2	 6, MD5_S22, 0C040B340H				; 18
                        MD5GG5_SSE2	11, MD5_S23, 0265E5A51H				; 19
                        MD5GG5_SSE2	 0, MD5_S24, 0E9B6C7AAH				; 20
                        MD5GG5_SSE2	 5, MD5_S21, 0D62F105DH				; 21
                        MD5GG5_SSE2	10, MD5_S22, 002441453H				; 22
                        MD5GG5_SSE2	15, MD5_S23, 0D8A1E681H				; 23
                        MD5GG5_SSE2	 4, MD5_S24, 0E7D3FBC8H				; 24
                        MD5GG5_SSE2	 9, MD5_S21, 021E1CDE6H				; 25
                        MD5GG5_SSE2	14, MD5_S22, 0C33707D6H				; 26
                        MD5GG5_SSE2	 3, MD5_S23, 0F4D50D87H				; 27
                        MD5GG5_SSE2	 8, MD5_S24, 0455A14EDH				; 28
                        MD5GG5_SSE2	13, MD5_S21, 0A9E3E905H				; 29
                        MD5GG5_SSE2	 2, MD5_S22, 0FCEFA3F8H				; 30
                        MD5GG5_SSE2	 7, MD5_S23, 0676F02D9H				; 31
                        MD5GG5_SSE2	12, MD5_S24, 08D2A4C8AH				; 32
; round 3
                        MD5HH5_SSE2	 5, MD5_S31, 0FFFA3942H				; 33
                        MD5HH5_SSE2	 8, MD5_S32, 08771F681H				; 34
                        MD5HH5_SSE2	11, MD5_S33, 06D9D6122H				; 35
                        MD5HH5_SSE2	14, MD5_S34, 0FDE5380CH				; 36
                        MD5HH5_SSE2	 1, MD5_S31, 0A4BEEA44H				; 37
                        MD5HH5_SSE2	 4, MD5_S32, 04BDECFA9H				; 38
                        MD5HH5_SSE2	 7, MD5_S33, 0F6BB4B60H				; 39
                        MD5HH5_SSE2	10, MD5_S34, 0BEBFBC70H				; 40
                        MD5HH5_SSE2	13, MD5_S31, 0289B7EC6H				; 41
                        MD5HH5_SSE2	 0, MD5_S32, 0EAA127FAH				; 42
                        MD5HH5_SSE2	 3, MD5_S33, 0D4EF3085H				; 43
                        MD5HH5_SSE2	 6, MD5_S34, 004881D05H				; 44
                        MD5HH5_SSE2	 9, MD5_S31, 0D9D4D039H				; 45
                        MD5HH5_SSE2	12, MD5_S32, 0E6DB99E5H				; 46
                        MD5HH5_SSE2	15, MD5_S33, 01FA27CF8H				; 47
                        MD5HH5_SSE2	 2, MD5_S34, 0C4AC5665H				; 48
; round 4
                        MD5II5_SSE2	 0, MD5_S41, 0F4292244H				; 49
                        MD5II5_SSE2	 7, MD5_S42, 0432AFF97H				; 50
                        MD5II5_SSE2	14, MD5_S43, 0AB9423A7H				; 51
                        MD5II5_SSE2	 5, MD5_S44, 0FC93A039H				; 52
                        MD5II5_SSE2	12, MD5_S41, 0655B59C3H				; 53
                        MD5II5_SSE2	 3, MD5_S42, 08F0CCC92H				; 54
                        MD5II5_SSE2	10, MD5_S43, 0FFEFF47DH				; 55
                        MD5II5_SSE2	 1, MD5_S44, 085845DD1H				; 56
                        MD5II5_SSE2	 8, MD5_S41, 06FA87E4FH				; 57
                        MD5II5_SSE2	15, MD5_S42, 0FE2CE6E0H				; 58
                        MD5II5_SSE2	 6, MD5_S43, 0A3014314H				; 59
                        MD5II5_SSE2	13, MD5_S44, 04E0811A1H				; 60
                        MD5II5_SSE2	 4, MD5_S41, 0F7537E82H				; 61
                        MD5II5_SSE2	11, MD5_S42, 0BD3AF235H				; 62
                        MD5II5_SSE2	 2, MD5_S43, 02AD7D2BBH				; 63
                        MD5II5_SSE2	 9, MD5_S44, 0EB86D391H				; 64
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
MD5_Add5_SSE2			ENDP

MD5FF6_FF6_SSE2			MACRO		count:REQ,s:REQ, count1:REQ, s1:REQ, ac1:REQ, count2:REQ, s2:REQ, ac2:REQ
; a = b+(a+x[count]+ac+((b&c)|(~b&d)))rol s
; a = b+(a+x[count]+ax+(d^(b&(c^d))))rol s
						movdqa		xmm_temp1, xmm_c
                        mov         reg_temp1, reg_c
                        xor			reg_c, reg_d
						paddd		xmm_a, [__w+count*16]
                        add         reg_a, ac1
                        and         reg_c, reg_b
						pxor		xmm_c, xmm_d
                        add         reg_a, [reg_base+count1*4]
                        xor			reg_c, reg_d
						pand		xmm_c, xmm_b
                        add         reg_a, reg_c
						paddd		xmm_a, [ac_const+ac_index*16]
                        rol         reg_a, s1
						pxor		xmm_c, xmm_d
                        add         reg_a, reg_b
                        mov         reg_c, reg_b
						paddd		xmm_a, xmm_c
                        xor			reg_b, reg_temp1
                        add         reg_d, [reg_base+count2*4]
						movdqa		xmm_temp2, xmm_a
                        add         reg_d, ac2
                        and         reg_b, reg_a
						pslld		xmm_a, s
                        xor			reg_b, reg_temp1
						psrld		xmm_temp2, 32-s
                        add         reg_d, reg_b
						por			xmm_a, xmm_temp2
                        rol         reg_d, s2
						paddd		xmm_a, xmm_b
                        add         reg_d, reg_a
reg_t					textequ		xmm_d
xmm_d					textequ		xmm_temp1
xmm_temp1				textequ		xmm_c
xmm_c					textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		reg_t
ac_index				=			ac_index + 1
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
						ENDM											; MD5FF6_FF6_SSE2

MD5FF6_GG6_SSE2			MACRO		count:REQ,s:REQ, count1:REQ, s1:REQ, ac1:REQ, count2:REQ, s2:REQ, ac2:REQ
; a = b+(a+x[count]+ac+((b&c)|(~b&d)))rol s
; a = b+(a+x[count]+ax+(d^(b&(c^d))))rol s
						movdqa		xmm_temp1, xmm_c
                        mov         reg_temp1, reg_b
                        xor         reg_b, reg_c
						paddd		xmm_a, [__w+count*16]
                        add         reg_a, ac1
                        and         reg_b, reg_d
						pxor		xmm_c, xmm_d
                        add         reg_a, [reg_base+count1*4]
                        xor         reg_b, reg_c
						pand		xmm_c, xmm_b
                        add         reg_a, reg_b
						paddd		xmm_a, [ac_const+ac_index*16]
                        rol         reg_a, s1
						pxor		xmm_c, xmm_d
                        add         reg_a, reg_temp1
						paddd		xmm_a, xmm_c
                        mov         reg_b, reg_a
                        xor         reg_a, reg_temp1
						movdqa		xmm_temp2, xmm_a
                        add         reg_d, [reg_base+count2*4]
                        and         reg_a, reg_c
						pslld		xmm_a, s
                        add         reg_d, ac2
                        xor         reg_a, reg_temp1
						psrld		xmm_temp2, 32-s
                        add         reg_d, reg_a
						por			xmm_a, xmm_temp2
                        rol         reg_d, s2
						paddd		xmm_a, xmm_b
                        add         reg_d, reg_b
reg_t					textequ		xmm_d
xmm_d					textequ		xmm_temp1
xmm_temp1				textequ		xmm_c
xmm_c					textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		reg_t
ac_index				=			ac_index + 1
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
						ENDM											; MD5FF6_GG6_SSE2
                        
MD5GG6_HH6_SSE2			MACRO		count:REQ,s:REQ, count1:REQ, s1:REQ, ac1:REQ, count2:REQ, s2:REQ, ac2:REQ
; a = b+(a+x[count]+ac+((d&b)|(~d&c)))rol s
; a = b+(a+x[count]+ac+(c^(d&(b^c))))rols s
						movdqa		xmm_temp1, xmm_b
                        mov         reg_temp1, reg_b
                        xor         reg_b, reg_c
						paddd		xmm_a, [__w+count*16]
                        add         reg_a, ac1
                        xor         reg_b, reg_d
						pxor		xmm_b, xmm_c
                        add         reg_a, [reg_base+count1*4]
						pand		xmm_b, xmm_d
                        add         reg_a, reg_b
						pxor		xmm_b, xmm_c
                        rol         reg_a, s1
						paddd		xmm_a, [ac_const+ac_index*16]
                        add         reg_a, reg_temp1
						paddd		xmm_a, xmm_b
                        mov         reg_b, reg_a
                        xor         reg_a, reg_temp1
						movdqa		xmm_temp2, xmm_a
                        add         reg_d, [reg_base+count2*4]
                        xor         reg_a, reg_c
						pslld		xmm_a, s
                        add         reg_d, ac2
						psrld		xmm_temp2, 32-s
                        add         reg_d, reg_a
						por			xmm_a, xmm_temp2
                        rol         reg_d, s2
						paddd		xmm_a, xmm_temp1
                        add         reg_d, reg_b
reg_t					textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_temp1
xmm_temp1				textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		reg_t
ac_index				=			ac_index + 1
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
						ENDM											; MD5GG6_HH6_SSE2

MD5GG6_II6_SSE2			MACRO		count:REQ,s:REQ, count1:REQ, s1:REQ, ac1:REQ, count2:REQ, s2:REQ, ac2:REQ
; a = b+(a+x[count]+ac+((d&b)|(~d&c)))rol s
; a = b+(a+x[count]+ac+(c^(d&(b^c))))rols s
						movdqa		xmm_temp1, xmm_b
                        mov         reg_temp1, reg_d
                        not         reg_d
						paddd		xmm_a, [__w+count*16]
                        add         reg_a, ac1
                        or          reg_d, reg_b
						pxor		xmm_b, xmm_c
                        add         reg_a, [reg_base+count1*4]
                        xor         reg_d, reg_c
						pand		xmm_b, xmm_d
                        add         reg_a, reg_d
						paddd		xmm_a, [ac_const+ac_index*16]
                        rol         reg_a, s1
						pxor		xmm_b, xmm_c
                        add         reg_a, reg_b
                        mov         reg_d, reg_c
						paddd		xmm_a, xmm_b
                        not         reg_c
                        add         reg_temp1, [reg_base+count2*4]
						movdqa		xmm_temp2, xmm_a
                        or          reg_c, reg_a
                        add         reg_temp1, ac2
						pslld		xmm_a, s
                        xor         reg_c, reg_b
                        add         reg_temp1, reg_c
						psrld		xmm_temp2, 32-s
                        rol         reg_temp1, s2
						por			xmm_a, xmm_temp2
                        add         reg_temp1, reg_a
						paddd		xmm_a, xmm_temp1
reg_t					textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_temp1
xmm_temp1				textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		reg_t
ac_index				=			ac_index + 1
reg_t					textequ		reg_temp1
reg_temp1				textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
reg_t					textequ		reg_temp1
reg_temp1				textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD5GG6_II6_SSE2

MD5HH6_FF6_SSE2			MACRO       count:REQ,s:REQ, count1:REQ, s1:REQ, ac1:REQ, count2:REQ, s2:REQ, ac2:REQ
; a = b+(a+x[count]+ac+(b^c^d)) rol s
						movdqa		xmm_temp1, xmm_b
                        mov         reg_temp1, reg_c
                        xor			reg_c, reg_d
						paddd		xmm_a, [__w+count*16]
                        add         reg_a, ac1
                        and         reg_c, reg_b
						pxor		xmm_b, xmm_c
                        add         reg_a, [reg_base+count1*4]
                        xor			reg_c, reg_d
						pxor		xmm_b, xmm_d
                        add         reg_a, reg_c
                        mov         reg_c, reg_b
						paddd		xmm_a, [ac_const+ac_index*16]
                        rol         reg_a, s1
						paddd		xmm_a, xmm_b
                        add         reg_a, reg_c
                        xor			reg_b, reg_temp1
						movdqa		xmm_temp2, xmm_a
                        add         reg_d, [reg_base+count2*4]
                        and         reg_b, reg_a
						pslld		xmm_a, s
                        add         reg_d, ac2
                        xor			reg_b, reg_temp1
						psrld		xmm_temp2, 32-s
                        add         reg_d, reg_b
						por			xmm_a, xmm_temp2
                        rol         reg_d, s2
						paddd		xmm_a, xmm_temp1
                        add         reg_d, reg_a
reg_t					textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_temp1
xmm_temp1				textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		reg_t
ac_index				=			ac_index + 1
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
						ENDM											; MD5HH6_FF6_SSE2

MD5HH6_GG6_SSE2			MACRO       count:REQ,s:REQ, count1:REQ, s1:REQ, ac1:REQ, count2:REQ, s2:REQ, ac2:REQ
; a = b+(a+x[count]+ac+(b^c^d)) rol s
						movdqa		xmm_temp1, xmm_b
                        mov         reg_temp1, reg_b
                        xor         reg_b, reg_c
						paddd		xmm_a, [__w+count*16]
                        add         reg_a, ac1
                        and         reg_b, reg_d
						pxor		xmm_b, xmm_c
                        add         reg_a, [reg_base+count1*4]
                        xor         reg_b, reg_c
						pxor		xmm_b, xmm_d
                        add         reg_a, reg_b
                        add         reg_d, ac2
						paddd		xmm_a, [ac_const+ac_index*16]
                        rol         reg_a, s1
						paddd		xmm_a, xmm_b
                        add         reg_a, reg_temp1
                        add         reg_d, [reg_base+count2*4]
						movdqa		xmm_temp2, xmm_a
                        mov         reg_b, reg_a
                        xor         reg_a, reg_temp1
						pslld		xmm_a, s
                        and         reg_a, reg_c
						psrld		xmm_temp2, 32-s
                        xor         reg_a, reg_temp1
						por			xmm_a, xmm_temp2
                        add         reg_d, reg_a
						paddd		xmm_a, xmm_temp1
                        rol         reg_d, s2
                        add         reg_d, reg_b
reg_t					textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_temp1
xmm_temp1				textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		reg_t
ac_index				=			ac_index + 1
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
						ENDM											; MD5HH6_GG6_SSE2

MD5II6_HH6_SSE2			MACRO       count:REQ,s:REQ, count1:REQ, s1:REQ, ac1:REQ, count2:REQ, s2:REQ, ac2:REQ
; a = b+(a+x[count]+ac+(c^(~d|b))) rol s
						movdqa		xmm_temp1, xmm_d
                        mov         reg_temp1, reg_b
                        xor         reg_b, reg_c
						paddd		xmm_a, [__w+count*16]
                        add         reg_a, ac1
                        xor         reg_b, reg_d
						pxor		xmm_d, xmm_FFFF
                        add         reg_a, [reg_base+count1*4]
						por			xmm_d, xmm_b
                        add         reg_a, reg_b
						paddd		xmm_a, [ac_const+ac_index*16]
                        rol         reg_a, s1
						pxor		xmm_d, xmm_c
                        add         reg_a, reg_temp1
						paddd		xmm_a, xmm_d
                        mov         reg_b, reg_a
                        xor         reg_a, reg_temp1
						movdqa		xmm_temp2, xmm_a
                        add         reg_d, [reg_base+count2*4]
                        xor         reg_a, reg_c
						pslld		xmm_a, s
                        add         reg_d, ac2
						psrld		xmm_temp2, 32-s
                        add         reg_d, reg_a
						por			xmm_a, xmm_temp2
                        rol         reg_d, s2
						paddd		xmm_a, xmm_b
                        add         reg_d, reg_b
reg_t					textequ		xmm_temp1
xmm_temp1				textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		reg_t
ac_index				=			ac_index + 1
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
						ENDM											; MD5II6_HH6_SSE2

MD5II6_II6_SSE2			MACRO       count:REQ,s:REQ, count1:REQ, s1:REQ, ac1:REQ, count2:REQ, s2:REQ, ac2:REQ
; a = b+(a+x[count]+ac+(c^(~d|b))) rol s
						movdqa		xmm_temp1, xmm_d
                        mov         reg_temp1, reg_d
                        not         reg_d
						paddd		xmm_a, [__w+count*16]
                        add         reg_a, ac1
                        or          reg_d, reg_b
						pxor		xmm_d, xmm_FFFF
                        add         reg_a, [reg_base+count1*4]
                        xor         reg_d, reg_c
						por			xmm_d, xmm_b
                        add         reg_a, reg_d
						paddd		xmm_a, [ac_const+ac_index*16]
                        rol         reg_a, s1
						pxor		xmm_d, xmm_c
                        add         reg_a, reg_b
                        mov         reg_d, reg_c
						paddd		xmm_a, xmm_d
                        not         reg_c
                        add         reg_temp1, [reg_base+count2*4]
						movdqa		xmm_temp2, xmm_a
                        or          reg_c, reg_a
                        add         reg_temp1, ac2
						pslld		xmm_a, s
                        xor         reg_c, reg_b
						psrld		xmm_temp2, 32-s
                        add         reg_temp1, reg_c
						por			xmm_a, xmm_temp2
                        rol         reg_temp1, s2
						paddd		xmm_a, xmm_b
                        add         reg_temp1, reg_a
reg_t					textequ		xmm_temp1
xmm_temp1				textequ		xmm_d
xmm_d					textequ		xmm_c
xmm_c					textequ		xmm_b
xmm_b					textequ		xmm_a
xmm_a					textequ		reg_t
ac_index				=			ac_index + 1
reg_t					textequ		reg_temp1
reg_temp1				textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
reg_t					textequ		reg_temp1
reg_temp1				textequ		reg_d
reg_d					textequ		reg_c
reg_c					textequ		reg_b
reg_b					textequ		reg_a
reg_a					textequ		reg_t
						ENDM											; MD5II6_II6_SSE2

MD5_Add6_SSE2			PROC		PUBLIC, _this1:DWORD, _Data1:DWORD, _this2:DWORD, _Data2:DWORD, _this3:DWORD, _Data3:DWORD, _this4:DWORD, _Data4:DWORD, _this5:DWORD, _Data5:DWORD, _this6:DWORD, _Data6:DWORD
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
xmm_FFFF				textequ		<xmm7>
                        movdqa		xmm_FFFF, const_FFFFFFFFFFFFFFFF
reg_a					textequ		<eax>
reg_b					textequ		<ebx>
reg_c					textequ		<edx>
reg_d					textequ		<esi>
reg_temp1				textequ		<edi>
						mov			ebp, __Data5
						mov			ecx, __Data6
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
                        movdqa		xmm6, xmm0
                        movdqa		xmm3, xmm4
                        punpcklqdq	xmm0, xmm2
                        punpcklqdq	xmm4, xmm5
                        punpckhqdq	xmm6, xmm2
                        punpckhqdq	xmm3, xmm5
                        movdqa		[__w+count*4*16+0*16], xmm0
                        movdqa		[__w+count*4*16+1*16], xmm6
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
reg_base				textequ		<ebp>
ac_index				=			0
; round 1
                        MD5FF6_FF6_SSE2	 0, MD5_S11,  0, MD5_S11, 0D76AA478H,  1, MD5_S12, 0E8C7B756H			;  1
                        MD5FF6_FF6_SSE2	 1, MD5_S12,  2, MD5_S13, 0242070DBH,  3, MD5_S14, 0C1BDCEEEH			;  2
                        MD5FF6_FF6_SSE2	 2, MD5_S13,  4, MD5_S11, 0F57C0FAFH,  5, MD5_S12, 04787C62AH			;  3
                        MD5FF6_FF6_SSE2	 3, MD5_S14,  6, MD5_S13, 0A8304613H,  7, MD5_S14, 0FD469501H			;  4
                        MD5FF6_FF6_SSE2	 4, MD5_S11,  8, MD5_S11, 0698098D8H,  9, MD5_S12, 08B44F7AFH 			;  5
                        MD5FF6_FF6_SSE2	 5, MD5_S12, 10, MD5_S13, 0FFFF5BB1H, 11, MD5_S14, 0895CD7BEH			;  6
                        MD5FF6_FF6_SSE2	 6, MD5_S13, 12, MD5_S11, 06B901122H, 13, MD5_S12, 0FD987193H			;  7
                        MD5FF6_FF6_SSE2	 7, MD5_S14, 14, MD5_S13, 0A679438EH, 15, MD5_S14, 049B40821H			;  8
                        MD5FF6_GG6_SSE2	 8, MD5_S11,  1, MD5_S21, 0F61E2562H,  6, MD5_S22, 0C040B340H			;  9
                        MD5FF6_GG6_SSE2	 9, MD5_S12, 11, MD5_S23, 0265E5A51H,  0, MD5_S24, 0E9B6C7AAH			; 10
                        MD5FF6_GG6_SSE2	10, MD5_S13,  5, MD5_S21, 0D62F105DH, 10, MD5_S22, 002441453H			; 11
                        MD5FF6_GG6_SSE2	11, MD5_S14, 15, MD5_S23, 0D8A1E681H,  4, MD5_S24, 0E7D3FBC8H			; 12
                        MD5FF6_GG6_SSE2	12, MD5_S11,  9, MD5_S21, 021E1CDE6H, 14, MD5_S22, 0C33707D6H			; 13
                        MD5FF6_GG6_SSE2	13, MD5_S12,  3, MD5_S23, 0F4D50D87H,  8, MD5_S24, 0455A14EDH			; 14
                        MD5FF6_GG6_SSE2	14, MD5_S13, 13, MD5_S21, 0A9E3E905H,  2, MD5_S22, 0FCEFA3F8H			; 15
                        MD5FF6_GG6_SSE2	15, MD5_S14,  7, MD5_S23, 0676F02D9H, 12, MD5_S24, 08D2A4C8AH			; 16
; round 2
                        MD5GG6_HH6_SSE2	 1, MD5_S21,  5, MD5_S31, 0FFFA3942H,  8, MD5_S32, 08771F681H			; 17
                        MD5GG6_HH6_SSE2	 6, MD5_S22, 11, MD5_S33, 06D9D6122H, 14, MD5_S34, 0FDE5380CH			; 18
                        MD5GG6_HH6_SSE2	11, MD5_S23,  1, MD5_S31, 0A4BEEA44H,  4, MD5_S32, 04BDECFA9H			; 19
                        MD5GG6_HH6_SSE2	 0, MD5_S24,  7, MD5_S33, 0F6BB4B60H, 10, MD5_S34, 0BEBFBC70H			; 20
                        MD5GG6_HH6_SSE2	 5, MD5_S21, 13, MD5_S31, 0289B7EC6H,  0, MD5_S32, 0EAA127FAH			; 21
                        MD5GG6_HH6_SSE2	10, MD5_S22,  3, MD5_S33, 0D4EF3085H,  6, MD5_S34, 004881D05H			; 22
                        MD5GG6_HH6_SSE2	15, MD5_S23,  9, MD5_S31, 0D9D4D039H, 12, MD5_S32, 0E6DB99E5H			; 23
                        MD5GG6_HH6_SSE2	 4, MD5_S24, 15, MD5_S33, 01FA27CF8H,  2, MD5_S34, 0C4AC5665H			; 24
                        MD5GG6_II6_SSE2	 9, MD5_S21,  0, MD5_S41, 0F4292244H,  7, MD5_S42, 0432AFF97H			; 25
                        MD5GG6_II6_SSE2	14, MD5_S22, 14, MD5_S43, 0AB9423A7H,  5, MD5_S44, 0FC93A039H			; 26
                        MD5GG6_II6_SSE2	 3, MD5_S23, 12, MD5_S41, 0655B59C3H,  3, MD5_S42, 08F0CCC92H			; 27
                        MD5GG6_II6_SSE2	 8, MD5_S24, 10, MD5_S43, 0FFEFF47DH,  1, MD5_S44, 085845DD1H			; 28
                        MD5GG6_II6_SSE2	13, MD5_S21,  8, MD5_S41, 06FA87E4FH, 15, MD5_S42, 0FE2CE6E0H			; 29
                        MD5GG6_II6_SSE2	 2, MD5_S22,  6, MD5_S43, 0A3014314H, 13, MD5_S44, 04E0811A1H			; 30
                        MD5GG6_II6_SSE2	 7, MD5_S23,  4, MD5_S41, 0F7537E82H, 11, MD5_S42, 0BD3AF235H			; 31
                        MD5GG6_II6_SSE2	12, MD5_S24,  2, MD5_S43, 02AD7D2BBH,  9, MD5_S44, 0EB86D391H			; 32
                        add			[__Digest+4*16], reg_a
                        add			[__Digest+4*16+4], reg_b
                        add			[__Digest+4*16+8], reg_c
                        add			[__Digest+4*16+12], reg_d
reg_base				textequ		<ecx>
; round 3
                        mov			reg_a, [__Digest+5*16]
                        mov			reg_b, [__Digest+5*16+4]
                        mov			reg_c, [__Digest+5*16+8]
                        mov			reg_d, [__Digest+5*16+12]
                        MD5HH6_FF6_SSE2	 5, MD5_S31,  0, MD5_S11, 0D76AA478H,  1, MD5_S12, 0E8C7B756H			; 33
                        MD5HH6_FF6_SSE2	 8, MD5_S32,  2, MD5_S13, 0242070DBH,  3, MD5_S14, 0C1BDCEEEH			; 34
                        MD5HH6_FF6_SSE2	11, MD5_S33,  4, MD5_S11, 0F57C0FAFH,  5, MD5_S12, 04787C62AH			; 35
                        MD5HH6_FF6_SSE2	14, MD5_S34,  6, MD5_S13, 0A8304613H,  7, MD5_S14, 0FD469501H			; 36
                        MD5HH6_FF6_SSE2	 1, MD5_S31,  8, MD5_S11, 0698098D8H,  9, MD5_S12, 08B44F7AFH			; 37
                        MD5HH6_FF6_SSE2	 4, MD5_S32, 10, MD5_S13, 0FFFF5BB1H, 11, MD5_S14, 0895CD7BEH			; 38
                        MD5HH6_FF6_SSE2	 7, MD5_S33, 12, MD5_S11, 06B901122H, 13, MD5_S12, 0FD987193H			; 39
                        MD5HH6_FF6_SSE2	10, MD5_S34, 14, MD5_S13, 0A679438EH, 15, MD5_S14, 049B40821H			; 40
                        MD5HH6_GG6_SSE2	13, MD5_S31,  1, MD5_S21, 0F61E2562H,  6, MD5_S22, 0C040B340H			; 41
                        MD5HH6_GG6_SSE2	 0, MD5_S32, 11, MD5_S23, 0265E5A51H,  0, MD5_S24, 0E9B6C7AAH			; 42
                        MD5HH6_GG6_SSE2	 3, MD5_S33,  5, MD5_S21, 0D62F105DH, 10, MD5_S22, 002441453H			; 43
                        MD5HH6_GG6_SSE2	 6, MD5_S34, 15, MD5_S23, 0D8A1E681H,  4, MD5_S24, 0E7D3FBC8H			; 44
                        MD5HH6_GG6_SSE2	 9, MD5_S31,  9, MD5_S21, 021E1CDE6H, 14, MD5_S22, 0C33707D6H			; 45
                        MD5HH6_GG6_SSE2	12, MD5_S32,  3, MD5_S23, 0F4D50D87H,  8, MD5_S24, 0455A14EDH			; 46
                        MD5HH6_GG6_SSE2	15, MD5_S33, 13, MD5_S21, 0A9E3E905H,  2, MD5_S22, 0FCEFA3F8H			; 47
                        MD5HH6_GG6_SSE2	 2, MD5_S34,  7, MD5_S23, 0676F02D9H, 12, MD5_S24, 08D2A4C8AH			; 48
; round 4
                        MD5II6_HH6_SSE2	 0, MD5_S41,  5, MD5_S31, 0FFFA3942H,  8, MD5_S32, 08771F681H			; 49
                        MD5II6_HH6_SSE2	 7, MD5_S42, 11, MD5_S33, 06D9D6122H, 14, MD5_S34, 0FDE5380CH			; 50
                        MD5II6_HH6_SSE2	14, MD5_S43,  1, MD5_S31, 0A4BEEA44H,  4, MD5_S32, 04BDECFA9H			; 51
                        MD5II6_HH6_SSE2	 5, MD5_S44,  7, MD5_S33, 0F6BB4B60H, 10, MD5_S34, 0BEBFBC70H			; 52
                        MD5II6_HH6_SSE2	12, MD5_S41, 13, MD5_S31, 0289B7EC6H,  0, MD5_S32, 0EAA127FAH			; 53
                        MD5II6_HH6_SSE2	 3, MD5_S42,  3, MD5_S33, 0D4EF3085H,  6, MD5_S34, 004881D05H			; 54
                        MD5II6_HH6_SSE2	10, MD5_S43,  9, MD5_S31, 0D9D4D039H, 12, MD5_S32, 0E6DB99E5H			; 55
                        MD5II6_HH6_SSE2	 1, MD5_S44, 15, MD5_S33, 01FA27CF8H,  2, MD5_S34, 0C4AC5665H			; 56
                        MD5II6_II6_SSE2	 8, MD5_S41,  0, MD5_S41, 0F4292244H,  7, MD5_S42, 0432AFF97H			; 57
                        MD5II6_II6_SSE2	15, MD5_S42, 14, MD5_S43, 0AB9423A7H,  5, MD5_S44, 0FC93A039H			; 58
                        MD5II6_II6_SSE2	 6, MD5_S43, 12, MD5_S41, 0655B59C3H,  3, MD5_S42, 08F0CCC92H			; 59
                        MD5II6_II6_SSE2	13, MD5_S44, 10, MD5_S43, 0FFEFF47DH,  1, MD5_S44, 085845DD1H			; 60
                        MD5II6_II6_SSE2	 4, MD5_S41,  8, MD5_S41, 06FA87E4FH, 15, MD5_S42, 0FE2CE6E0H			; 61
                        MD5II6_II6_SSE2	11, MD5_S42,  6, MD5_S43, 0A3014314H, 13, MD5_S44, 04E0811A1H			; 62
                        MD5II6_II6_SSE2	 2, MD5_S43,  4, MD5_S41, 0F7537E82H, 11, MD5_S42, 0BD3AF235H			; 63
                        MD5II6_II6_SSE2	 9, MD5_S44,  2, MD5_S43, 02AD7D2BBH,  9, MD5_S44, 0EB86D391H			; 64
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
						add			ebp, 64
						add			ecx, 64
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
MD5_Add6_SSE2			ENDP

		end