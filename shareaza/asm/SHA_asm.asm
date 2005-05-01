; #####################################################################################################################
;
; SHA_asm.asm
;
; Copyright (c) Shareaza Development Team, 2002-2005.
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
; SHA_asm - Implementation of SHA-1 for x86 - use together with SHA.cpp and SHA.h
;
; created              4.7.2004         by Camper
;
; last modified        20.7.2004         by Camper
;
; The integration into other projects than Shareaza is expressivly encouraged. Feel free to contact me about it.
;
; #####################################################################################################################

                        .586p
                        .model      flat, C 
                        option      casemap:none                    ; case sensitive
                        option      prologue:none                   ; we generate our own entry/exit code
                        option      epilogue:none

; #####################################################################################################################

m_nCount0                equ         0                              ; offsets as found in SHA.h
m_nCount1                equ         4

m_nHash0                 equ         8
m_nHash1                 equ         12
m_nHash2                 equ         16
m_nHash3                 equ         20
m_nHash4                 equ         24

m_nBuffer                equ         28

RND_CH                  MACRO       const:REQ

; t=a; a=rotl32(a,5)+e+k+w[i]+((b&c)^(~b&d)); e=d; d=c; c=rotl32(b,30); b=t

                        mov         reg_temp1, reg_a                        ; t=a
                        mov         reg_temp2, reg_c                        ; save c
                        rol         reg_a, 5
                        add         reg_a, reg_e
                        mov         reg_e, reg_b
                        and         reg_b, reg_c                            ; b&c
                        mov         reg_c, reg_e
                        not         reg_e
                        add         reg_a, const
                        ror         reg_c, 2                                ; c=rotl32(b,30)
                        and         reg_e, reg_d                            ; ~b&d
                        add         reg_a, [_w+count*4]
                        xor         reg_b, reg_e                            ; ( )^( )
                        add         reg_a, reg_b
reg_t                   textequ     reg_e
reg_e                   textequ     reg_d                                   ; e=d, saves mov reg_e,reg_d
reg_d                   textequ     reg_temp2                               ; d=c, saves mov reg_d,reg_temp2
reg_temp2               textequ     reg_t
reg_t                   textequ     reg_b                                   ; b=t
reg_b                   textequ     reg_temp1                               ; save mov reg_d,reg_temp1
reg_temp1               textequ     reg_t
count                   =           count + 1

                        ENDM                                                ; RND_CH

RND_PARITY              MACRO       const:REQ

; t=a; a=rotl32(a,5)+e+k+w[i]+(b^c^d); e=d; d=c; c=rotl32(b,30); b=t

                        mov         reg_temp1, reg_a                        ; t=a
                        rol         reg_a, 5
                        add         reg_a, reg_e
                        mov         reg_e, reg_b
                        add         reg_a, const
reg_t                   textequ     reg_b
reg_b                   textequ     reg_e                                   ; b<->e
reg_e                   textequ     reg_t
                        xor         reg_e, reg_c
                        add         reg_a, [_w+count*4]
                        xor         reg_e, reg_d
                        add         reg_a, reg_e
                        ror         reg_b, 2
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

                        mov         _t, reg_a
                        rol         reg_a, 5
                        mov         reg_temp1, reg_c
                        mov         reg_temp2, reg_d
                        add         reg_a, reg_e
                        mov         reg_e, reg_b
reg_t                   textequ     reg_b
reg_b                   textequ     reg_e
reg_e                   textequ     reg_t
                        and         reg_e, reg_c
                        add         reg_a, const
                        and         reg_c, reg_d
                        add         reg_a, [_w+count*4]
                        and         reg_d, reg_b
                        xor         reg_e, reg_c
                        xor         reg_e, reg_d
                        add         reg_a, reg_e
reg_t                   textequ     reg_c
reg_c                   textequ     reg_b                                   ; c=rotl32(b,30)
reg_b                   textequ     reg_t
                        mov         reg_b, _t                               ; b=t
                        ror         reg_c, 2
reg_t                   textequ     reg_e
reg_e                   textequ     reg_temp2                               ; e=d
reg_temp2               textequ     reg_t
reg_t                   textequ     reg_d
reg_d                   textequ     reg_temp1                               ; d=c
reg_temp1               textequ     reg_t
count                   =           count + 1

                        ENDM                                                ; RND_MAJ
                        
INIT_REG_ALIAS          MACRO
reg_accu                textequ     <eax>
reg_base                textequ     <ebp>
reg_i_1                 textequ     <ebx>
reg_i_2                 textequ     <ecx>
reg_i_3                 textequ     <edx>
reg_i_15                textequ     <esi>
reg_i_16                textequ     <edi>
                        ENDM
                        
                        .code

                        ALIGN       16

SHA_Compile_p5          PROC

__this                  textequ     <[esp+32+4+4+324]>                       ; pusha + 2 * ret addr in between
_w                      textequ     <esp+8>
_t                      textequ     <[esp+4]>

                        INIT_REG_ALIAS

count                   =           0
                        REPEAT      16
                        mov         reg_accu, [reg_base+count*4]
                        bswap       reg_accu
                        IF          count eq 0
                        mov         reg_i_16, reg_accu
                        ELSEIF      count eq 1
                        mov         reg_i_15, reg_accu
                        ELSEIF      count eq 13
                        mov         reg_i_3, reg_accu
                        ELSEIF      count eq 14
                        mov         reg_i_2, reg_accu
                        ELSEIF      count eq 15
                        mov         reg_i_1, reg_accu
                        ENDIF
                        mov         [_w+count*4], reg_accu
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

reg_a                   textequ     reg_accu
reg_b                   textequ     reg_i_1
reg_c                   textequ     reg_i_2
reg_d                   textequ     reg_i_3
reg_e                   textequ     reg_i_15
reg_temp1               textequ     reg_i_16
reg_temp2               textequ     reg_base

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
__this                  textequ     <[esp+36+324]>                              ; different offset due to pusha
__Data                  textequ     <[esp+40+324]>
__nLength               textequ     <[esp+44+324]>

                        sub         esp, 324

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

get_out:                add         esp, 324
                        popa
                        ret

SHA_Add_p5              ENDP

        end
